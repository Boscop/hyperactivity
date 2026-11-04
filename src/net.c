/* net.c — cells, wiring, the interaction rules, and building a net from
 * compiled syntax. */
#include "core.h"
#include <string.h>
#include <pthread.h>
#include <sched.h>

long stat_steps, stat_kind[K_END], stat_cells;
#define ADD(x, v) __atomic_fetch_add(&(x), (v), __ATOMIC_RELAXED)

/* ---------------- type-stable cells ----------------
 * A cell's memory is never returned: a thread holding a stale pointer can
 * still take the lock word, and revalidation then finds the wire changed.
 * Freed cells go to the freeing thread's list after its locks are released. */
#define MAXFREE 4096
typedef struct { Cell *op, *v; } Redex;
typedef struct { Cell *free[MAXFREE + 1]; Cell *defer; Redex *made; size_t nmade, capmade; struct Box **dead; size_t ndead, capdead; } Local;
static __thread Local *L;
static Local *local(void) { if (!L) L = calloc(1, sizeof(Local)); return L; }

Cell *cell(int kind, int n) {
    Local *l = local();
    Cell *c;
    if (n <= MAXFREE && l->free[n]) { c = l->free[n]; l->free[n] = (Cell *)(uintptr_t)c->a; }
    else {
        c = malloc(sizeof(Cell) + (size_t)n * sizeof(Port));
        if (!c) { fputs("out of memory\n", stderr); exit(2); }
        __atomic_store_n(&c->lk, 0, __ATOMIC_RELAXED);
        c->n = (uint16_t)n;   /* never changes: a freed cell is reused only for a cell of the same size */
    }
    /* other workers read ports without locks (a lock set is read, then read again under the
     * locks), and a cell they read may be reused here: its ports are cleared atomically */
    for (int k = 0; k < n; k++) __atomic_store_n(&c->port[k], 0, __ATOMIC_RELAXED);
    c->kind = (uint16_t)kind; c->x = 0; c->ty = 0; c->a = 0; c->p = NULL; c->key = NULL; c->e = NULL;
    ADD(stat_cells, 1);
    return c;
}
/* the payload a cell owns */
static bool owns_dnf(int k) { return k == O_PEND || k == V_IV || k == V_FACE || k == V_SYS || k == V_NPAPP || k == O_PAPPP || k == O_BIN2 || k == O_SYSCONS2 || k == V_NTRANS || k == O_TRANSD2 || k == O_IEQB || k == O_FEQLB; }
static bool owns_subst(int k) { return k == O_SUBST; }
static void box_release(Box *b);
static void payload_retain(Cell *c) {
    if (owns_dnf(c->kind)) dnf_retain(c->p); else if (owns_subst(c->kind)) subst_retain(c->p);
    else if (c->kind == V_REF) { __atomic_add_fetch(&((Box *)c->p)->rc, 1, __ATOMIC_RELAXED); if (c->a) subst_retain((Subst *)(uintptr_t)c->a); }
    else if (c->kind == V_WORD || c->kind == V_WPLUS) big_retain(c->p);
    else if (c->kind == V_THUNK && c->a) subst_retain((Subst *)(uintptr_t)c->a);
}
static void payload_release(Cell *c) {
    if (owns_dnf(c->kind)) dnf_release(c->p); else if (owns_subst(c->kind)) subst_release(c->p);
    else if (c->kind == V_REF && c->p) { box_release(c->p); if (c->a) subst_release((Subst *)(uintptr_t)c->a); c->a = 0; }
    else if (c->kind == V_WORD || c->kind == V_WPLUS) big_release(c->p);
    else if (c->kind == V_THUNK && c->a) { subst_release((Subst *)(uintptr_t)c->a); c->a = 0; }
    c->p = NULL;
}
static void memo_leave(Cell *c);
void cell_free(Cell *c) { if (c->e) memo_leave(c); if (c->key) { key_release(c->key); c->key = NULL; } payload_release(c); Local *l = local(); c->a = (uint64_t)(uintptr_t)l->defer; l->defer = c; ADD(stat_cells, -1); }
static void flush_frees(void) {
    Local *l = local();
    while (l->defer) {
        Cell *c = l->defer; l->defer = (Cell *)(uintptr_t)c->a;
        if (c->n <= MAXFREE) { c->a = (uint64_t)(uintptr_t)l->free[c->n]; l->free[c->n] = c; }
    }
}
static Cell *clone(const Cell *v) { Cell *c = cell(v->kind, v->n); c->x = v->x; c->ty = v->ty; c->a = v->a; c->p = v->p; c->key = v->key ? key_retain(v->key) : NULL; payload_retain(c); return c; }

/* a wire is published with release and read with acquire, so a cell is
 * initialized before any thread reaching it through a wire touches it */
static inline Port rd(Cell *c, int k) { return __atomic_load_n(&c->port[k], __ATOMIC_ACQUIRE); }
static void lock(Cell *c) {
    while (__atomic_exchange_n(&c->lk, 1, __ATOMIC_ACQUIRE))
        while (__atomic_load_n(&c->lk, __ATOMIC_RELAXED)) __builtin_ia32_pause();
}
static void unlock(Cell *c) { __atomic_store_n(&c->lk, 0, __ATOMIC_RELEASE); }

/* ---------------- active pairs: per-worker queues ---------------- */
typedef struct { pthread_mutex_t mu; Redex *q; size_t head, len, cap; } Queue;
static Queue *Q; static int NQ;
static __thread int myq;
static long pending;                       /* queued or being rewritten */
static void queues_init(int n) {
    NQ = n; Q = calloc((size_t)n, sizeof(Queue));
    for (int i = 0; i < n; i++) pthread_mutex_init(&Q[i].mu, NULL);
}
void net_push(Cell *op, Cell *v) {
    if (!Q) queues_init(1);
    Queue *q = &Q[myq];
    pthread_mutex_lock(&q->mu);
    if (q->len == q->cap) {
        if (q->head > 0) { memmove(q->q, q->q + q->head, (q->len - q->head) * sizeof(Redex)); q->len -= q->head; q->head = 0; }
        if (q->len == q->cap) { q->cap = q->cap ? q->cap * 2 : 1024; q->q = realloc(q->q, q->cap * sizeof(Redex)); }
    }
    q->q[q->len++] = (Redex){ op, v };
    ADD(pending, 1);
    pthread_mutex_unlock(&q->mu);
}
static bool pop_from(Queue *q, Redex *r) {
    bool got = false;
    pthread_mutex_lock(&q->mu);
    if (q->head < q->len) { *r = q->q[q->head++]; if (q->head == q->len) q->head = q->len = 0; got = true; }
    pthread_mutex_unlock(&q->mu);
    return got;
}
static bool pop(Redex *r) {
    if (!Q) queues_init(1);
    if (pop_from(&Q[myq], r)) return true;
    for (int k = 1; k < NQ; k++) if (pop_from(&Q[(myq + k) % NQ], r)) return true;
    return false;
}

void link(Port a, Port b) {
    if (a) __atomic_store_n(&PC(a)->port[PI(a)], b, __ATOMIC_RELEASE);
    if (b) __atomic_store_n(&PC(b)->port[PI(b)], a, __ATOMIC_RELEASE);
    if (a && b && PI(a) == 0 && PI(b) == 0) {
        Cell *x = PC(a), *y = PC(b);
        Cell *op = NULL, *val = NULL;
        if (is_value(x->kind) && !is_value(y->kind)) { op = y; val = x; }
        else if (is_value(y->kind) && !is_value(x->kind)) { op = x; val = y; }
        if (op && op->kind != O_ROOT) {
            /* held until the replacement creating it is complete */
            Local *l = local();
            if (l->nmade == l->capmade) { l->capmade = l->capmade ? l->capmade * 2 : 64; l->made = realloc(l->made, l->capmade * sizeof(Redex)); }
            l->made[l->nmade++] = (Redex){ op, val };
        }
    }
}
/* publish the active pairs this thread has made */
static void publish(void) {
    Local *l = local();
    for (size_t i = 0; i < l->nmade; i++) net_push(l->made[i].op, l->made[i].v);
    l->nmade = 0;
}
/* move whatever is wired to c's port k onto port `to` */
static void mv(Cell *c, int k, Port to) { link(c->port[k], to); }
static Port out_of(Cell *o) { return o->port[o->n - 1]; }


/* ---------------- keys ----------------
 * Interned terms (core.h Key). One table and one mutex serve interning and the memo below; the
 * mutex is taken inside rules and never while a cell lock is taken, so it is innermost. */
static void era(Port prov);
long stat_hits, stat_keys;
static pthread_mutex_t kmu = PTHREAD_MUTEX_INITIALIZER;
static Key **KT; static size_t KTcap, KTn;
static void key_print_d(FILE *o, const Key *k, int d);
static void key_print(FILE *o, const Key *k) { key_print_d(o, k, 0); }
static void key_print_d(FILE *o, const Key *k, int d) {
    static const char *T[] = { "?", "W", "I", "F", "V", "G", "CON", "DATA", "LAM", "PAIR", "PI", "SIGMA", "PATHP", "U", "APP", "FST", "SND", "PAPP", "CASE", "FORCE", "SUB", "SUBST", "DIM", "TERM", "WTY", "UNFOLD" };
    if (d > 12) { fprintf(o, "…"); return; }
    fprintf(o, "(%s", T[k->tag]);
    if (k->tag == K_GLOBAL) fprintf(o, " %s", name_of((long)k->a));
    else if (k->tag == K_CON) fprintf(o, " %s", CON[k->a].name);
    else if (k->tag == K_WORD || k->tag == K_VAR || k->tag == K_DIM) fprintf(o, " %llu", (unsigned long long)k->a);
    else if (k->tag == K_CASE || k->tag == K_LAM) fprintf(o, " @%p", k->p);
    for (uint32_t i = 0; i < k->n; i++) { fputc(' ', o); key_print_d(o, k->c[i], d + 1); }
    fputc(')', o);
}
static bool key_trace;   /* CUBICAL_KEYS: the keys registered, fired and hit, on stderr */
static bool no_memo;     /* CUBICAL_NOMEMO: no producer is pending on its key (every value computed where it is read) */
static __thread Key *scrut_key;   /* set by a case rule for the branch it builds; taken by the branch's frame */
static uint64_t mix(uint64_t h, uint64_t x) { h ^= x + 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2); return h * 0x100000001b3ull; }
static uint64_t dnf_hash(const Dnf *d) {
    uint64_t h = 7;
    if (!d) return h;
    for (uint32_t i = 0; i < d->n; i++) { h = mix(h, d->c[i]->n); for (uint32_t k = 0; k < d->c[i]->n; k++) h = mix(h, d->c[i]->l[k]); }
    return h;
}
static uint64_t key_hash(int tag, uint64_t a, const void *p, int n, Key *const *c) {
    uint64_t h = mix(mix(17, (uint64_t)tag), a);
    if (tag == K_IV || tag == K_FACE) h = mix(h, dnf_hash(p));
    else if (tag == K_WORD) { const Big *b = p; if (b) for (uint32_t i = 0; i < b->n; i++) h = mix(h, b->l[i]); }
    else h = mix(h, (uint64_t)(uintptr_t)p);
    for (int i = 0; i < n; i++) h = mix(h, (uint64_t)(uintptr_t)c[i]);
    return h;
}
static bool key_same(const Key *k, int tag, uint64_t a, const void *p, int n, Key *const *c) {
    if (k->tag != tag || k->a != a || k->n != (uint32_t)n) return false;
    for (int i = 0; i < n; i++) if (k->c[i] != c[i]) return false;
    if (tag == K_IV || tag == K_FACE) return dnf_equal(k->p, p);
    if (tag == K_WORD) { WN x = { a, k->p }, y = { a, (Big *)p }; return wn_cmp(&x, &y) == 0; }
    return k->p == p;
}
Key *key_retain(Key *k) { __atomic_add_fetch(&k->rc, 1, __ATOMIC_RELAXED); return k; }
static void key_free(Key *k);
static void key_release_locked(Key *k) {   /* under kmu */
    if (__atomic_sub_fetch(&k->rc, 1, __ATOMIC_ACQ_REL) == 0) key_free(k);
}
/* A count that reaches zero is never raised again: interning takes a reference only to a key whose
 * count is not zero (key() below), and every other retain is by a holder of a reference. So the
 * last releaser frees the key; until it does, interning passes over it */
void key_release(Key *k) {
    if (__atomic_sub_fetch(&k->rc, 1, __ATOMIC_ACQ_REL) == 0) {
        pthread_mutex_lock(&kmu);
        key_free(k);
        pthread_mutex_unlock(&kmu);
    }
}
static void key_free(Key *k) {
    size_t i = k->h & (KTcap - 1);
    while (KT[i] != k) i = (i + 1) & (KTcap - 1);
    /* open addressing: re-insert the run after the hole */
    KT[i] = NULL; KTn--;
    for (size_t j = (i + 1) & (KTcap - 1); KT[j]; j = (j + 1) & (KTcap - 1)) {
        Key *m = KT[j]; KT[j] = NULL;
        size_t q = m->h & (KTcap - 1);
        while (KT[q]) q = (q + 1) & (KTcap - 1);
        KT[q] = m;
    }
    if (k->tag == K_IV || k->tag == K_FACE) dnf_release(k->p);
    else if (k->tag == K_WORD) big_release(k->p);
    for (uint32_t i2 = 0; i2 < k->n; i2++) key_release_locked(k->c[i2]);
    free(k);
}
/* the interned key (a reference the caller owns); children are borrowed (retained here); a Dnf or
 * Big payload is retained, others held by pointer */
static Key *key(int tag, uint64_t a, void *p, int n, Key *const *c) {
    for (int i = 0; i < n; i++) if (!c[i]) return NULL;
    uint64_t h = key_hash(tag, a, p, n, c);
    pthread_mutex_lock(&kmu);
    if (KTn * 2 >= KTcap) {
        size_t nc = KTcap ? KTcap * 2 : 4096;
        Key **nt = calloc(nc, sizeof(Key *));
        for (size_t i = 0; i < KTcap; i++) if (KT[i]) { size_t q = KT[i]->h & (nc - 1); while (nt[q]) q = (q + 1) & (nc - 1); nt[q] = KT[i]; }
        free(KT); KT = nt; KTcap = nc;
    }
    size_t i = h & (KTcap - 1);
    for (; KT[i]; i = (i + 1) & (KTcap - 1))
        if (KT[i]->h == h && key_same(KT[i], tag, a, p, n, c)) {
            /* a reference to it, unless its last reference is gone (its releaser frees it) */
            Key *k = KT[i];
            uint32_t r = __atomic_load_n(&k->rc, __ATOMIC_ACQUIRE);
            while (r && !__atomic_compare_exchange_n(&k->rc, &r, r + 1, true, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) { }
            if (r) { pthread_mutex_unlock(&kmu); return k; }
        }
    Key *k = malloc(sizeof(Key) + (size_t)n * sizeof(Key *));
    k->rc = 1; k->n = (uint32_t)n; k->h = h; k->tag = (uint16_t)tag; k->a = a;
    k->p = (tag == K_IV || tag == K_FACE) ? dnf_retain(p) : tag == K_WORD ? big_retain(p) : p;
    for (int j = 0; j < n; j++) k->c[j] = key_retain(c[j]);
    KT[i] = k; KTn++;
    pthread_mutex_unlock(&kmu);
    return k;
}
static Key *key0(int tag, uint64_t a, void *p) { return key(tag, a, p, 0, NULL); }
static Key *key1(int tag, uint64_t a, void *p, Key *x) { return key(tag, a, p, 1, &x); }
static Key *key2(int tag, uint64_t a, void *p, Key *x, Key *y) { Key *c[2] = { x, y }; return key(tag, a, p, 2, c); }
Key *key_global(long g) { return key0(K_GLOBAL, (uint64_t)g, NULL); }
Key *key_con0(int c) { return key0(K_CON, (uint64_t)c, NULL); }
/* a substitution as a key: its (dimension, interval) pairs, in order */
static Key *subst_key(const Subst *s) {
    int n = (int)(2 * s->n);
    Key **c = calloc((size_t)n + 1, sizeof(Key *)); int m = 0;
    for (uint32_t i = 0; i < s->n; i++) { c[m++] = key0(K_DIM, s->d[i], NULL); c[m++] = key0(K_IV, 0, s->v[i]); }
    Key *r = key(K_SUBST, 0, NULL, m, c);
    for (int i = 0; i < m; i++) if (c[i]) key_release(c[i]);
    free(c);
    return r;
}
/* the key of a cell as it stands: its term, under the substitution a thunk or reference carries (a
 * reference carries its box's key, or the key of the box it was itself held in). The caller owns
 * the result */
static Key *eff_key(Cell *c) {
    Key *k = c->key;
    if (!k) return NULL;
    if ((c->kind == V_THUNK || c->kind == V_REF) && c->a) {
        Key *sk = subst_key((Subst *)(uintptr_t)c->a);
        if (!sk) return NULL;
        Key *r = key2(K_SUB, 0, NULL, k, sk); key_release(sk);
        return r;
    }
    return key_retain(k);
}
/* the key of what a port provides: a value or reference at its principal port, or the output of a
 * keyed operation (a duplicator's outputs carry their input's key). The cell is a neighbour of the
 * firing pair or made by this thread, so it is locked or owned. The caller owns the result */
static Key *key_of_port(Port p) {
    if (!p) return NULL;
    Cell *c = PC(p);
    if (PI(p) == 0) return is_value(c->kind) ? eff_key(c) : NULL;
    if (!c->key) return NULL;
    if (c->kind == O_DUP || c->kind == O_FWD || c->kind == O_SHARE) return (PI(p) == 1 || PI(p) == 2) ? key_retain(c->key) : NULL;
    return PI(p) == c->n - 1 ? key_retain(c->key) : NULL;
}

/* ---------------- memo: one value per live key ----------------
 * A key names a term over the values it is built on: a held value (a box) is named by what was
 * put in it, and a value copied out of a box, or rebuilt from the fields a case took from it,
 * carries the box's key, so one value reached by any route has one name. A cell that will
 * produce the value of a term (a thunk, a saturated call, an application of a known λ, a case) is
 * pending on its key from its creation. When one fires while another is pending, the value goes
 * into a box and both read it (the second waits on the wire for it, uncounted, as any read of a
 * value not yet there); the entry and its box live exactly while a producer is pending or a reader
 * holds the box (HANDOFF §36: retention is reachability). When the firing producer is the only
 * one pending, it builds directly: nothing holds the value. */
typedef struct Entry { Key *k; Box *box; long pending; bool hold; struct Entry *next; } Entry;   /* hold: the entry holds one reference on its box while a producer is pending */
static Entry **MT; static size_t MTcap, MTn;
static Entry *memo_find(Key *k, bool make) {
    if (MTcap == 0) { MTcap = 4096; MT = calloc(MTcap, sizeof(Entry *)); }
    size_t i = k->h & (MTcap - 1);
    for (Entry *e = MT[i]; e; e = e->next) if (e->k == k) return e;
    if (!make) return NULL;
    if (MTn * 2 >= MTcap) {
        size_t nc = MTcap * 2; Entry **nt = calloc(nc, sizeof(Entry *));
        for (size_t j = 0; j < MTcap; j++) for (Entry *e = MT[j], *nx; e; e = nx) { nx = e->next; size_t q = e->k->h & (nc - 1); e->next = nt[q]; nt[q] = e; }
        free(MT); MT = nt; MTcap = nc; i = k->h & (MTcap - 1);
    }
    Entry *e = calloc(1, sizeof(Entry)); e->k = key_retain(k); e->next = MT[i]; MT[i] = e; MTn++;
    return e;
}
static void memo_drop(Entry *e) {   /* under kmu: no producer pending, no box */
    size_t i = e->k->h & (MTcap - 1);
    for (Entry **pp = &MT[i]; *pp; pp = &(*pp)->next) if (*pp == e) { *pp = e->next; break; }
    MTn--; key_release_locked(e->k); free(e);
}
/* a producer cell with a key: pending on it */
static void memo_register(Cell *c) {
    if (no_memo) return;
    Key *k = eff_key(c);
    if (!k) return;
    if (key_trace) { fprintf(stderr, "register %d ", c->kind); key_print(stderr, k); fputc('\n', stderr); }
    pthread_mutex_lock(&kmu);
    Entry *e = memo_find(k, true); e->pending++; c->e = e;
    if (e->box && !e->hold) { __atomic_add_fetch(&e->box->rc, 1, __ATOMIC_RELAXED); e->hold = true; }
    pthread_mutex_unlock(&kmu);
    key_release(k);
}
static void box_release(Box *b);
static void memo_leave(Cell *c) {   /* a pending producer consumed without producing (erased) or re-keyed */
    Entry *e = c->e; c->e = NULL;
    pthread_mutex_lock(&kmu);
    if (--e->pending == 0) {
        if (e->box) { Box *b = e->box; bool h = e->hold; e->hold = false; pthread_mutex_unlock(&kmu); if (h) box_release(b); return; }
        memo_drop(e);
    }
    pthread_mutex_unlock(&kmu);
}
/* a pending producer fires. Returns the port to give its consumer when the value is (or will be)
 * shared, and *out = the box to build into (NULL when the value is already there or being built);
 * returns 0 when the producer builds directly */
static Port memo_begin(Cell *c, Box **out, bool builder) {
    *out = NULL;
    Entry *e = c->e;
    if (!e) return 0;
    c->e = NULL;
    pthread_mutex_lock(&kmu);
    e->pending--;
    if (key_trace) { fprintf(stderr, "fire %d %s pending=%ld box=%d ", c->kind, builder ? "builder" : "thunk", e->pending, e->box != NULL); key_print(stderr, e->k); fputc('\n', stderr); }
    if (e->box) {
        /* the value, there or being built: read it */
        Box *b = e->box;
        __atomic_add_fetch(&b->rc, 1, __ATOMIC_RELAXED);
        bool last = e->pending == 0 && e->hold;
        if (last) e->hold = false;
        pthread_mutex_unlock(&kmu);
        ADD(stat_hits, 1);
        Cell *r = cell(V_REF, 1); r->p = b; r->key = key_retain(b->key);
        if (last) box_release(b);   /* the entry's own hold */
        return P(r, 0);
    }
    if (!builder || e->pending == 0) {
        /* a thunk: its body's producer, which has the same key, decides (it registers as pending);
         * a builder nobody else waits for builds directly */
        if (e->pending == 0) memo_drop(e);
        pthread_mutex_unlock(&kmu);
        return 0;
    }
    /* others wait for this value: it is held once and read by reference. Its root is locked until the
     * value is linked into it (memo_end): a reader locks the root, so none reads the box before its
     * wire exists (the root is fresh, so taking its lock here never waits) */
    Box *b = malloc(sizeof(Box)); b->rc = 2; b->root = cell(O_ROOT, 1); b->key = key_retain(e->k); b->e = e;
    lock(b->root);
    e->box = b; e->hold = true;
    pthread_mutex_unlock(&kmu);
    ADD(stat_keys, 1);
    *out = b;
    Cell *r = cell(V_REF, 1); r->p = b; r->key = key_retain(b->key);
    return P(r, 0);
}
/* CUBICAL_KEYS: the entries still live at the end (what holds values across readings) */
void memo_dump(void) {
    if (!key_trace) return;
    pthread_mutex_lock(&kmu);
    for (size_t i = 0; i < MTcap; i++) for (Entry *e = MT[i]; e; e = e->next) {
        fprintf(stderr, "entry pending=%ld box=%d ", e->pending, e->box != NULL);
        key_print(stderr, e->k); fputc('\n', stderr);
    }
    fprintf(stderr, "%zu entries, %zu keys\n", MTn, KTn);
    pthread_mutex_unlock(&kmu);
}
/* the box of a memo entry is gone: the entry no longer has a value */
static void memo_box_gone(Box *b) {
    pthread_mutex_lock(&kmu);
    Entry *e = b->e; b->e = NULL;
    if (e) { e->box = NULL; e->hold = false; if (e->pending == 0) memo_drop(e); }
    pthread_mutex_unlock(&kmu);
}
/* ---------------- small constructors ---------------- */
/* profile: where each duplicator and eraser comes from (a record's definition, or a rule) */
static __thread long dup_origin;
long prof_odup[65536], prof_oera[65536], prof_rdup[256], prof_rera[256];
/* a value held once: references copy the box; the last one erases the value */
static void box_hold(Box *b);
static Key *key_of_port(Port p);
static Box *box_new(Port val) {
    Box *b = malloc(sizeof(Box)); b->rc = 1; b->root = cell(O_ROOT, 1); b->e = NULL;
    b->key = key_of_port(val);
    link(val, P(b->root, 0));
    return b;
}
static Cell *ref_to(Box *b) { Cell *r = cell(V_REF, 1); r->p = b; r->key = b->key ? key_retain(b->key) : NULL; return r; }
/* reading a stored value (held by root): its top cell is copied, each part a reference to the
 * stored part (wrapped in a box when first read), so a part is copied only when an operation
 * reads it, one level per interaction. A reader of a value not yet there waits for it (O_FWD)
 * and gets the same copy when it arrives. The rewrite holds the stored value and its neighbours (lockset). */
static void box_hold(Box *b) { __atomic_add_fetch(&b->rc, 1, __ATOMIC_RELAXED); }
Port thunk_build(Cell *v);
Port unbuilt_build(Cell *v);
static bool unbuilt(int k);
static Port sub(Port prov, Subst *s);
static Port copy_top(Key *as, Cell *v);
/* as: the key of what is held (the box's; a definition's name): a copy carries it */
Port lazy_copy(Key *as, Cell *root) {
    Port p = root->port[0];
    Cell *v = p ? PC(p) : NULL;
    if (v && PI(p) == 0 && unbuilt(v->kind)) {
        /* held unbuilt: built in place, once; this and later readers wait for its value */
        link(unbuilt_build(v), P(root, 0));
        p = root->port[0]; v = PC(p);
    }
    if (!v || PI(p) != 0 || !is_value(v->kind)) {
        /* the value is not there yet: the reader waits on the wire for it (O_FWD), and is served
         * the same one-level copy when it arrives. Waiting costs no step, so what a read costs does
         * not depend on whether the value had arrived */
        Cell *d = cell(O_FWD, 3); d->key = as ? key_retain(as) : NULL;
        link(root->port[0], P(d, 0)); link(P(d, 1), P(root, 0));
        return P(d, 2);
    }
    return copy_top(as, v);
}
/* one level of a stored value: its top cell copied, each part with parts of its own a reference to a
 * box around the stored part (the stored value's port then refers to that box too). The copy is
 * named by what is held (as) */
static Port copy_top(Key *as, Cell *v) {
    Cell *c = clone(v);
    if (as && c->key != as) { if (c->key) key_release(c->key); c->key = key_retain(as); }
    for (int k = 1; k < v->n; k++) {
        Port q = v->port[k];
        Cell *qc = q ? PC(q) : NULL;
        if (qc && PI(q) == 0 && is_value(qc->kind) && qc->n == 1 && !unbuilt(qc->kind)) {
            /* a part with no parts of its own (a reference, a word, a constant): itself */
            Cell *q2 = clone(qc);
            link(P(q2, 0), P(c, k));
            continue;
        }
        if (q && !(PI(q) == 0 && is_value(qc->kind))) {
            /* a part not there yet: shared as it will be when it arrives (O_SHARE) — copied if it
             * has no parts, else held once with a reference on each side — so the copy does not
             * depend on whether it had arrived */
            Cell *d = cell(O_SHARE, 3);
            link(q, P(d, 0)); link(P(d, 1), P(v, k)); link(P(d, 2), P(c, k));
            continue;
        }
        Box *b = box_new(q);
        Cell *r1 = ref_to(b);
        Cell *r2 = ref_to(b); box_hold(b);
        link(P(r1, 0), P(v, k));
        link(P(r2, 0), P(c, k));
    }
    return P(c, 0);
}

/* a subterm held unbuilt, built (its environment moved into the body) */
/* an unbuilt subterm (a thunk, an induction hypothesis, a word's hypothesis), built */
static bool unbuilt(int k) { return k == V_THUNK || k == V_SUSP || k == V_WSUSP; }
static Port case_of_susp(Cell *v);
static Port welim_of_wsusp(Cell *v);
Port unbuilt_build(Cell *v) {
    if (v->kind == V_SUSP) return case_of_susp(v);
    if (v->kind == V_WSUSP) return welim_of_wsusp(v);
    return thunk_build(v);
}
static Port memo_begin(Cell *c, Box **out, bool builder);
/* a pending producer's result: built into the memo's box when the value is shared */
static Port memo_end(Port shared, Box *box, Port built) {
    if (!shared) return built;
    if (box) { link(built, P(box->root, 0)); unlock(box->root); }
    return shared;
}
Port thunk_build(Cell *v) {
    if (prof_on) ADD(prof_count[((Record *)v->p)->where], 1);
    Box *bx; Port sh = memo_begin(v, &bx, false);
    if (sh && !bx) { for (int k = 1; k < v->n; k++) era(v->port[k]); cell_free(v); return sh; }
    Port r = record_build(v->p, 0, &v->port[1]);
    /* a substitution that reached it applies to its value */
    if (v->a) r = sub(r, (Subst *)(uintptr_t)v->a);
    cell_free(v);
    return memo_end(sh, bx, r);
}
static Port word_of(uint64_t n) { Cell *c = cell(V_WORD, 1); c->a = n; return P(c, 0); }
/* the number a word cell holds (a copy), and a word cell holding a number (taking it) */
static WN cell_wn(const Cell *c) { WN r = { c->a, c->p }; return wn_copy(&r); }
static Port word_wn(WN k) { Cell *c = cell(V_WORD, 1); c->a = k.s; c->p = k.b; return P(c, 0); }
Port word_port(WN k) { return word_wn(k); }
/* a reference reads its value under the substitution it carries (a=Subst or 0): the top cell
 * copied and substituted, each part a reference carrying the substitution on */
static Port ref_read(Cell *r) { Port p = lazy_copy(r->key, ((Box *)r->p)->root); return r->a ? sub(p, (Subst *)(uintptr_t)r->a) : p; }
Port ref_copy(Cell *c) { return ref_read(c); }
/* a word's suspended elimination, computed (for readback) */
Port wsusp_force(Cell *c) {
    Cell *e = cell(O_WELIM, 4);
    link(c->port[3], P(e, 0)); link(c->port[1], P(e, 1)); link(c->port[2], P(e, 2));
    return P(e, 3);
}
static Port ref_of(Port val) { Cell *c = ref_to(box_new(val)); return P(c, 0); }
static void era(Port prov);
/* The last reference to a box gone: what it holds is erased. Not here: the release can happen where
 * the box's root and the cell its value comes from are not locked (a substitution holding the box
 * freed by an unrelated rule), and erasing rewires that cell. So the box is retired after the
 * rewrite's locks are released (retire_boxes), under the locks of exactly the two cells involved. */
static void memo_box_gone(Box *b);
static void box_release(Box *b) {
    if (__atomic_sub_fetch(&b->rc, 1, __ATOMIC_ACQ_REL)) return;
    if (b->e) memo_box_gone(b);
    Local *l = local();
    if (l->ndead == l->capdead) { l->capdead = l->capdead ? l->capdead * 2 : 16; l->dead = realloc(l->dead, l->capdead * sizeof(Box *)); }
    l->dead[l->ndead++] = b;
}
static void lock(Cell *c);
static void unlock(Cell *c);
static void retire_boxes(void) {
    Local *l = local();
    while (l->ndead) {
        Box *b = l->dead[--l->ndead];
        Cell *r = b->root, *w;
        Port p;
        for (;;) {
            p = __atomic_load_n(&r->port[0], __ATOMIC_ACQUIRE);
            w = p ? PC(p) : NULL;
            if (w && w < r) { lock(w); lock(r); } else { lock(r); if (w) lock(w); }
            if (__atomic_load_n(&r->port[0], __ATOMIC_ACQUIRE) == p) break;
            unlock(r); if (w) unlock(w);
        }
        if (p) era(p);
        unlock(r); if (w) unlock(w);
        cell_free(r);
        if (b->key) key_release(b->key);
        free(b);
    }
}
static void era(Port prov) { Cell *e = cell(O_ERA, 1); e->a = (uint64_t)dup_origin; link(prov, P(e, 0)); }
static Port sub(Port prov, Subst *s) { Cell *c = cell(O_SUBST, 2); c->p = subst_retain(s); link(prov, P(c, 0)); return P(c, 1); }  /* borrows s */
static Port mkplam_of(Port prov, Dim i) { Cell *m = cell(O_MKPLAM, 2); m->a = i; link(prov, P(m, 0)); return P(m, 1); }
static Port iv_cell(Dnf *r) { Cell *c = cell(V_IV, 1); c->p = r; return P(c, 0); }  /* takes r */
/* m providers sharing one value (a chain of duplicators) */
static void split(Port prov, int m, Port *out) {
    if (m == 0) { era(prov); return; }
    if (m == 1) { out[0] = prov; return; }
    Key *k = key_of_port(prov);
    for (int i = 0; i < m - 1; i++) { Cell *d = cell(O_DUP, 3); d->a = (uint64_t)dup_origin; d->key = k ? key_retain(k) : NULL; link(prov, P(d, 0)); out[i] = P(d, 1); prov = P(d, 2); }
    out[m - 1] = prov;
    if (k) key_release(k);
}
void port_split(Port prov, int m, Port *out) { split(prov, m, out); }
static void split_restrict(Port prov, const Dnf *cl, int m, Port *out) {
    split(prov, m, out);
    for (int k = 0; k < m; k++) { Subst *t = subst_of_clause(cl->c[k]); out[k] = sub(out[k], t); subst_release(t); }
}

/* ---------------- types of neutrals ----------------
 * A neutral carries its type on its last port. Eliminating a neutral of
 * known type computes the result's type from it (CCHM: the endpoints of a
 * path are part of its type, so t 0 = u0 for t : Path A u0 u1 even when t
 * is neutral). The type is known when the neutral's head was declared with
 * one (ty = 1); otherwise it is V_UNK. */
static Port unk(void) { return P(cell(V_UNK, 1), 0); }
static bool is_neutral(int k) { return k >= V_NVAR && k <= V_NPROJ; }
static Port tpart(Port T, int k) { Cell *c = cell(O_TPART, 2); c->a = (uint64_t)k; link(T, P(c, 0)); return P(c, 1); }
static Port app2(Port f, Port x) { Cell *a = cell(O_APP, 3); link(f, P(a, 0)); link(x, P(a, 1)); return P(a, 2); }
static Port ntype_of(Port neu) { Cell *c = cell(O_NTYPE, 2); link(neu, P(c, 0)); return P(c, 1); }
static Port papp_at(Port line, Dnf *r) { Cell *a = cell(O_PAPPR, 3); link(iv_cell(r), P(a, 0)); link(line, P(a, 1)); return P(a, 2); }  /* takes r */
static void split2(Port prov, Port *a, Port *b) { Cell *d = cell(O_DUP, 3); d->a = (uint64_t)dup_origin; d->key = key_of_port(prov); link(prov, P(d, 0)); *a = P(d, 1); *b = P(d, 2); }

/* the stuck head of a copattern definition: a neutral of its declared type (when it has one), named
 * by the definition; projection unfolds it */
Port cofix_head(long g, long tyg) {
    Cell *c = cell(V_NVAR, 2); c->a = (uint64_t)g; c->ty = TY_COFIX; c->key = key_global(g);
    if (tyg >= 0) { Cell *t = cell(V_GLOBAL, 1); t->a = (uint64_t)tyg; t->key = key_global(tyg); link(P(t, 0), P(c, 1)); c->ty |= TY_KNOWN; }
    else link(unk(), P(c, 1));
    return P(c, 0);
}
Port apply_gid(long g, const Port *args, int n) {
    Cell *gc = cell(V_GLOBAL, 1); gc->a = (uint64_t)g; gc->key = key_global(g);
    Port f = P(gc, 0);
    Key *fk = key_retain(gc->key);
    for (int k = 0; k < n; k++) {
        Cell *a = cell(O_APP, 3); link(f, P(a, 0)); link(args[k], P(a, 1)); f = P(a, 2);
        if (fk) { Key *ak = key_of_port(args[k]); Key *nk = key2(K_APP, 0, NULL, fk, ak); key_release(fk); if (ak) key_release(ak); fk = nk; }
        if (fk) { a->key = key_retain(fk); memo_register(a); }
    }
    if (fk) key_release(fk);
    return f;
}
static Port apply_global(const char *name, Port *args, int n) {
    long g = global_lookup(name);
    if (g < 0) { fprintf(stderr, "missing library definition %s\n", name); exit(2); }
    return apply_gid(g, args, n);
}
static Port face_cell(Dnf *f) { Cell *c = cell(V_FACE, 1); c->p = f; return P(c, 0); }  /* takes f */

/* a constructor applied to its fields and dimensions: c v r is its boundary
 * e(v, r) where the boundary's face holds at r (judgmentally), and otherwise
 * a constructor value. Built again after every substitution. */
Port con_build(int c, const Port *f, const Port *d) {
    ConInfo *k = &CON[c];
    int nf = k->nfields, nd = k->ndims, n = nf + nd;
    if (k->bdy < 0) {
        Cell *v = cell(V_CON, n + 1); v->a = (uint64_t)c;
        for (int j = 0; j < nf; j++) link(f[j], P(v, 1 + j));
        for (int j = 0; j < nd; j++) link(d[j], P(v, 1 + nf + j));
        return P(v, 0);
    }
    Port *a = malloc((size_t)(2 * n + 1) * sizeof(Port));
    for (int j = 0; j < n; j++) split2(j < nf ? f[j] : d[j - nf], &a[j], &a[n + j]);
    Port S = apply_gid(k->bdy, a, n), s1, s2;
    split2(S, &s1, &s2);
    Cell *fo = cell(O_FACEOF, 2); link(s1, P(fo, 0));
    Cell *ck = cell(O_CONCHK, n + 3); ck->a = (uint64_t)c;
    link(P(fo, 1), P(ck, 0)); link(s2, P(ck, 1));
    for (int j = 0; j < n; j++) link(a[n + j], P(ck, 2 + j));
    free(a);
    return P(ck, n + 2);
}

/* ---------------- structural rules ---------------- */
static void rule_dup(Cell *o, Cell *v) {
    Port o1 = o->port[1], o2 = o->port[2];
    if (prof_on) {
        ADD(prof_dupk[v->kind & 63], 1);
        if (v->kind == V_LAM) ADD(prof_dup[((Record *)v->p)->where], 1);
        if (v->kind == V_CON && v->a < 4096) ADD(prof_con[v->a], 1);
        if (v->kind == V_GLOBAL && v->a < 65536) ADD(prof_glob[v->a], 1);
    }
    if (v->n > 1 || unbuilt(v->kind)) {
        /* a value with parts is held once: each side a reference, read one level at a time (a
         * subterm held unbuilt is built once, by its first reader) */
        Box *bx = box_new(P(v, 0));
        Cell *r1 = ref_to(bx), *r2 = ref_to(bx); box_hold(bx);
        link(P(r1, 0), o1); link(P(r2, 0), o2);
        cell_free(o);
        return;
    }
    Cell *c1 = clone(v), *c2 = clone(v);
    for (int k = 1; k < v->n; k++) {
        Cell *d = cell(O_DUP, 3); d->a = o->a; d->key = key_of_port(v->port[k]);
        link(v->port[k], P(d, 0)); link(P(d, 1), P(c1, k)); link(P(d, 2), P(c2, k));
    }
    link(P(c1, 0), o1); link(P(c2, 0), o2);
    cell_free(o); cell_free(v);
}
static void rule_era(Cell *o, Cell *v) {
    if (prof_on) ADD(prof_erak[v->kind & 63], 1);
    dup_origin = (long)o->a;
    for (int k = 1; k < v->n; k++) era(v->port[k]);
    cell_free(o); cell_free(v);
}

/* substitution of dimensions */
static void rule_subst(Cell *o, Cell *v) {
    Subst *s = o->p;
    Port out = o->port[1];
    switch (v->kind) {
    case V_GLOBAL: case V_UNK:
        link(P(v, 0), out); cell_free(o); return;
    case V_THUNK: case V_REF: {
        /* the reference (the unbuilt subterm) carries the substitution: nothing is copied */
        Subst *t = v->a ? subst_compose((Subst *)(uintptr_t)v->a, s) : subst_retain(s);
        if (v->a) subst_release((Subst *)(uintptr_t)v->a);
        v->a = (uint64_t)(uintptr_t)t;
        if (v->e) { memo_leave(v); memo_register(v); }
        link(P(v, 0), out); cell_free(o); return;
    }
    case V_NVAR:
        link(sub(v->port[1], s), P(v, 1));
        link(P(v, 0), out); cell_free(o); return;
    case V_IV: { Dnf *n2 = iv_subst(v->p, s); dnf_release(v->p); v->p = n2; link(P(v, 0), out); cell_free(o); return; }
    case V_FACE: { Dnf *n2 = face_subst(v->p, s); dnf_release(v->p); v->p = n2; link(P(v, 0), out); cell_free(o); return; }
    case V_PLAM: {
        Dim j = (Dim)v->a, j2 = j;
        Subst *t = subst_without(s, j);
        if (subst_range_mentions(t, j)) {
            j2 = fresh_dim();
            Dnf *at = iv_atom(j2); Subst *t2 = subst_with(t, j, at);
            dnf_release(at); subst_release(t); t = t2;
        }
        link(mkplam_of(sub(v->port[1], t), j2), out);
        subst_release(t);
        cell_free(o); cell_free(v); return;
    }
    case V_SYS: {
        int nb = v->n - 1;
        Dnf *cl = v->p;
        Dnf **f = malloc((size_t)(nb + 1) * sizeof(Dnf *));
        int total = 0;
        for (int k = 0; k < nb; k++) { Dnf *fc = face_of_clause(cl->c[k]); f[k] = face_subst(fc, s); dnf_release(fc); total += (int)f[k]->n; }
        for (int k = 0; k < nb; k++) if (dnf_is_one(f[k])) {
            /* the substitution entails this branch's clause: the system is that value */
            link(sub(v->port[k + 1], s), out);
            for (int m = 0; m < nb; m++) if (m != k) era(v->port[m + 1]);
            for (int m = 0; m < nb; m++) dnf_release(f[m]);
            free(f); cell_free(o); cell_free(v); return;
        }
        Cell *ns = cell(V_SYS, total + 1);
        Clause **ncl = malloc((size_t)(total + 1) * sizeof(Clause *));
        int idx = 0;
        for (int k = 0; k < nb; k++) {
            int m = (int)f[k]->n;
            Port *cp = malloc((size_t)(m + 1) * sizeof(Port));
            split(v->port[k + 1], m, cp);
            for (int q = 0; q < m; q++) {
                ncl[idx] = f[k]->c[q];
                Subst *t = subst_then_clause(s, f[k]->c[q]);
                link(sub(cp[q], t), P(ns, idx + 1));
                subst_release(t);
                idx++;
            }
            free(cp);
        }
        ns->p = clause_list(ncl, (uint32_t)total);
        free(ncl);
        link(P(ns, 0), out);
        for (int m = 0; m < nb; m++) dnf_release(f[m]);
        free(f); cell_free(o); cell_free(v); return;
    }
    /* neutrals and reducible constructors become their operations again */
    case V_NAPP: { Cell *a = cell(O_APP, 3); link(sub(v->port[1], s), P(a, 0)); link(sub(v->port[2], s), P(a, 1)); link(P(a, 2), out); era(v->port[3]); break; }
    case V_NFST: case V_NSND: { Cell *a = cell(v->kind == V_NFST ? O_FST : O_SND, 2); link(sub(v->port[1], s), P(a, 0)); link(P(a, 1), out); era(v->port[2]); break; }
    case V_NPROJ: { Cell *a = cell(O_PROJ, 2); a->a = v->a; link(sub(v->port[1], s), P(a, 0)); link(P(a, 1), out); era(v->port[2]); break; }
    case V_NPAPP: {
        Cell *a = cell(O_PAPPR, v->x ? 5 : 3); a->x = v->x;
        link(iv_cell(iv_subst(v->p, s)), P(a, 0));
        link(sub(v->port[1], s), P(a, 1));
        if (v->x) { link(sub(v->port[2], s), P(a, 2)); link(sub(v->port[3], s), P(a, 3)); }
        link(P(a, a->n - 1), out);
        era(v->port[v->n - 1]);
        break;
    }
    case V_NCOMP: { Cell *a = cell(O_COMP, 4); link(sub(v->port[2], s), P(a, 0)); link(sub(v->port[1], s), P(a, 1)); link(sub(v->port[3], s), P(a, 2)); link(P(a, 3), out); era(v->port[4]); break; }
    case V_NUNGLUE: { Cell *a = cell(O_UNGLUE, 3); link(sub(v->port[1], s), P(a, 0)); link(sub(v->port[2], s), P(a, 1)); link(P(a, 2), out); era(v->port[3]); break; }
    case V_CON: {
        ConInfo *k = &CON[v->a];
        int n = k->nfields + k->ndims;
        Port *a = malloc((size_t)(n + 1) * sizeof(Port));
        for (int j = 0; j < n; j++) a[j] = sub(v->port[1 + j], s);
        link(con_build((int)v->a, a, a + k->nfields), out);
        free(a);
        break;
    }
    case V_WPLUS: { Cell *a = cell(O_WADD, 3); link(sub(v->port[1], s), P(a, 0)); link(word_wn(cell_wn(v)), P(a, 1)); link(P(a, 2), out); break; }
    case V_NWOP: { Cell *a = cell((int)v->x, 3); link(sub(v->port[1], s), P(a, 0)); link(sub(v->port[2], s), P(a, 1)); link(P(a, 2), out); break; }
    case V_NWELIM: { Cell *a = cell(O_WELIM, 4); for (int k = 1; k <= 3; k++) link(sub(v->port[k], s), P(a, k - 1)); link(P(a, 3), out); break; }
    case V_HCOMP: { Cell *a = cell(O_HCOMP, 3); link(sub(v->port[1], s), P(a, 0)); link(sub(v->port[2], s), P(a, 1)); link(P(a, 2), out); break; }
    case V_NCASE: {
        /* 1=scrutinee [2=motive] then the environment, type → 0=scrutinee then the environment, out:
         * the motive is the table's, rebuilt where it is read */
        int mb = v->x ? 1 : 0, ne = v->n - 3 - mb;
        Cell *a = cell(O_CASE, ne + 2); a->x = 0; a->p = v->p;
        link(sub(v->port[1], s), P(a, 0));
        for (int e = 0; e < ne; e++) link(sub(v->port[2 + mb + e], s), P(a, 1 + e));
        link(P(a, a->n - 1), out);
        if (mb) era(v->port[2]);
        era(v->port[v->n - 1]);
        break;
    }
    case V_NTRANS: {
        int np = v->n - 3;
        Cell *a = cell(O_TRANSD1, np + 3); a->a = v->a;
        link(face_cell(face_subst(v->p, s)), P(a, 0));
        for (int j = 0; j < np; j++) link(sub(v->port[2 + j], s), P(a, 1 + j));
        link(sub(v->port[1], s), P(a, np + 1));
        link(P(a, np + 2), out);
        era(v->port[v->n - 1]);
        break;
    }
    case V_GLUE: { Cell *a = cell(O_MKGLUE, 3); link(sub(v->port[2], s), P(a, 0)); link(sub(v->port[1], s), P(a, 1)); link(P(a, 2), out); break; }
    case V_GLUEV: { Cell *a = cell(O_MKGLUEV, 3); link(sub(v->port[1], s), P(a, 0)); link(sub(v->port[2], s), P(a, 1)); link(P(a, 2), out); break; }
    default: {
        /* constructors: λ (its environment), pair, Π, Σ, PathP */
        Cell *c = clone(v);
        for (int k = 1; k < v->n; k++) link(sub(v->port[k], s), P(c, k));
        link(P(c, 0), out);
        break;
    }
    }
    cell_free(o); cell_free(v);
}

/* a value is put under a dimension binder; a system whose clauses avoid the
 * dimension is a system of lines */
static void rule_mkplam(Cell *o, Cell *v) {
    Dim i = (Dim)o->a;
    if (v->kind == V_REF) { link(ref_read(v), P(o, 0)); cell_free(v); return; }
    if (unbuilt(v->kind)) {
        if (o->kind == O_FORCE) { link(unbuilt_build(v), o->port[1]); cell_free(o); }
        else link(unbuilt_build(v), P(o, 0));
        return;
    }
    Port out = o->port[1];
    if (v->kind == V_SYS) {
        Dnf *cl = v->p;
        int nb = v->n - 1;
        bool avoid = true;
        for (int k = 0; k < nb; k++) if (clause_mentions(cl->c[k], i)) avoid = false;
        if (avoid) {
            Cell *ns = cell(V_SYS, nb + 1); ns->p = dnf_retain(cl);
            for (int k = 0; k < nb; k++) link(mkplam_of(v->port[k + 1], i), P(ns, k + 1));
            link(P(ns, 0), out);
            cell_free(o); cell_free(v); return;
        }
    }
    Cell *pl = cell(V_PLAM, 2); pl->a = i;
    link(P(v, 0), P(pl, 1));
    link(P(pl, 0), out);
    cell_free(o);
}

/* an eliminator meets a system: it acts branchwise, every other operand
 * restricted to the branch's clause */
static void distribute(Cell *o, Cell *v) {
    int nb = v->n - 1;
    Dnf *cl = v->p;
    Cell *ns = cell(V_SYS, nb + 1); ns->p = dnf_retain(cl);
    Port ex[o->n]; int ne = 0;
    switch (o->kind) {
    case O_APP: case O_UNGLUE2: case O_SYSMAP: case O_DAPPLY: ex[ne++] = o->port[1]; break;
    case O_PAPPP: if (o->x) { ex[ne++] = o->port[1]; ex[ne++] = o->port[2]; } break;
    case O_COMPA: ex[ne++] = o->port[1]; ex[ne++] = o->port[2]; break;
    case O_CASE: for (int k = 1; k < o->n - 1; k++) ex[ne++] = o->port[k]; break;
    case O_TRANSD2: for (int k = 1; k < o->n - 1; k++) ex[ne++] = o->port[k]; break;
    default: break;
    }
    Port out = out_of(o);
    Port *cp[o->n];
    for (int e = 0; e < ne; e++) { cp[e] = malloc((size_t)(nb + 1) * sizeof(Port)); split_restrict(ex[e], cl, nb, cp[e]); }
    for (int b = 0; b < nb; b++) {
        Port val = v->port[b + 1], res = P(ns, b + 1);
        Cell *c;
        switch (o->kind) {
        case O_APP: c = cell(O_APP, 3); link(val, P(c, 0)); link(cp[0][b], P(c, 1)); link(P(c, 2), res); break;
        case O_SYSMAP: c = cell(O_APP, 3); link(cp[0][b], P(c, 0)); link(val, P(c, 1)); link(P(c, 2), res); break;
        case O_FST: case O_SND: case O_FORCE: case O_TPART: case O_PROJ: case O_UNFOLD: c = cell(o->kind, 2); c->a = o->a; link(val, P(c, 0)); link(P(c, 1), res); break;
        case O_DAPPLY: c = cell(O_DAPPLY, 3); link(val, P(c, 0)); link(cp[0][b], P(c, 1)); link(P(c, 2), res); break;
        case O_FACEOF: c = cell(O_FACEOF, 2); link(val, P(c, 0)); link(P(c, 1), res); break;
        case O_PAPPP:
            c = cell(O_PAPPR, o->x ? 5 : 3); c->x = o->x;
            { Subst *t = subst_of_clause(cl->c[b]); link(iv_cell(iv_subst(o->p, t)), P(c, 0)); subst_release(t); }
            link(val, P(c, 1));
            if (o->x) { link(cp[0][b], P(c, 2)); link(cp[1][b], P(c, 3)); }
            link(P(c, c->n - 1), res);
            break;
        case O_UNGLUE2: c = cell(O_UNGLUE, 3); link(cp[0][b], P(c, 0)); link(val, P(c, 1)); link(P(c, 2), res); break;
        case O_COMPA: c = cell(O_COMP, 4); link(cp[0][b], P(c, 0)); link(val, P(c, 1)); link(cp[1][b], P(c, 2)); link(P(c, 3), res); break;
        case O_CASE:
            c = cell(O_CASE, o->n); c->x = o->x; c->p = o->p;
            link(val, P(c, 0));
            for (int e = 0; e < ne; e++) link(cp[e][b], P(c, 1 + e));
            link(P(c, o->n - 1), res);
            break;
        case O_TRANSD2:
            c = cell(O_TRANSD2, o->n); c->a = o->a;
            { Subst *t = subst_of_clause(cl->c[b]); c->p = face_subst(o->p, t); subst_release(t); }
            link(val, P(c, 0));
            for (int e = 0; e < ne; e++) link(cp[e][b], P(c, 1 + e));
            link(P(c, o->n - 1), res);
            break;
        default: fprintf(stderr, "distribute: kind %d\n", o->kind); exit(2);
        }
    }
    for (int e = 0; e < ne; e++) free(cp[e]);
    link(P(ns, 0), out);
    cell_free(o); cell_free(v);
}

/* a stuck projection is a neutral retaining its operand; its type is the
 * Σ's first component, or its second at the first projection */
static void neutral_proj(Cell *o, Cell *v) {
    bool first = o->kind == O_FST;
    Cell *n = cell(first ? V_NFST : V_NSND, 3);
    Port out = o->port[1];
    /* its type is computed from the spine when needed (O_NTYPE): stored, the type of a projection
     * would hold the earlier projections each with its own type, exponential in the depth */
    (void)first;
    link(P(v, 0), P(n, 1)); link(unk(), P(n, 2));
    link(P(n, 0), out);
    cell_free(o);
}

/* ---------------- values as data ----------------
 * %view gives a value's head as a constructor of View (lib/code.cub) whose fields are the
 * value's own ports: nothing is copied, and a part is read further only by viewing it. */
enum { W_LAM, W_PAIR, W_PI, W_SIGMA, W_U, W_PATH, W_PLAM, W_IV, W_FACE, W_SYS, W_GLUE, W_GLUEV, W_DATA, W_CON,
       W_HCOMP, W_VAR, W_APP, W_FST, W_SND, W_PAPP, W_COMP, W_UNGLUE, W_ELIM, W_TRANS, W_LEVEL, W_OTHER,
       W_WORD, W_WLIT, W_WPLUS, W_WOP, W_WELIM, W_COFIX, W_PROJ, W_REC, W_N };
static const char *WNAME[W_N] = { "wlam", "wpair", "wpi", "wsigma", "wu", "wpath", "wplam", "wiv", "wface", "wsys", "wglue",
    "wgluev", "wdata", "wcon", "whcomp", "wvar", "wapp", "wfst", "wsnd", "wpapp", "wcomp", "wunglue", "welim", "wtrans",
    "wlevel", "wother", "wword", "wwlit", "wwplus", "wwop", "wwelim", "wcofix", "wproj", "wrec" };
static int WC[W_N], C_LNIL = -1, C_LCONS, C_TRUE, C_FALSE, C_NONE, C_SOME;
static int B_TRUE = -1, B_FALSE;
static void need_bool(void) {
    if (B_TRUE >= 0) return;
    B_FALSE = con_index("false");
    int t = con_index("true");
    if (t < 0 || B_FALSE < 0) { fprintf(stderr, "Bool is missing (lib/prelude.cub)\n"); exit(2); }
    __atomic_store_n(&B_TRUE, t, __ATOMIC_RELEASE);
}
static void need_cons(void) {
    if (C_LNIL >= 0) return;
    for (int w = 0; w < W_N; w++) if ((WC[w] = con_index(WNAME[w])) < 0) { fprintf(stderr, "%%view needs the data type View (lib/code.cub)\n"); exit(2); }
    C_LCONS = con_index("lcons");
    C_TRUE = con_index("true"); C_FALSE = con_index("false");
    C_NONE = con_index("none"); C_SOME = con_index("some");
    int nil = con_index("lnil");
    if (nil < 0 || C_LCONS < 0 || C_TRUE < 0 || C_FALSE < 0 || C_NONE < 0 || C_SOME < 0) { fprintf(stderr, "the checker's data types are missing\n"); exit(2); }
    __atomic_store_n(&C_LNIL, nil, __ATOMIC_RELEASE);
}
static Port mk(int c, Port a, Port b, Port d, Port e) {
    Port f[4] = { a, b, d, e };
    return con_build(c, f, NULL);
}
static Port list_of(const Port *xs, int n) {
    Port l = con_build(C_LNIL, NULL, NULL);
    for (int k = n - 1; k >= 0; k--) l = mk(C_LCONS, xs[k], l, 0, 0);
    return l;
}
static Port pair_of(Port a, Port b) { Cell *c = cell(V_PAIR, 3); link(a, P(c, 1)); link(b, P(c, 2)); return P(c, 0); }
static Port face_cell(Dnf *f);
static void rule_view(Cell *o, Cell *v) {
    need_cons();
    Port out = o->port[1], r = 0;
    int vk = v->kind;
    bool keep = false;   /* the value itself is the field */
    switch (vk) {
    case V_LAM: case V_CASEF: r = mk(WC[W_LAM], P(v, 0), 0, 0, 0); keep = true; break;
    case V_PLAM: r = mk(WC[W_PLAM], P(v, 0), 0, 0, 0); keep = true; break;
    case V_IV: r = mk(WC[W_IV], P(v, 0), 0, 0, 0); keep = true; break;
    case V_FACE: r = mk(WC[W_FACE], P(v, 0), 0, 0, 0); keep = true; break;
    case V_PAIR: r = mk(WC[W_PAIR], v->port[1], v->port[2], 0, 0); break;
    case V_WTY: r = con_build(WC[W_WORD], NULL, NULL); break;
    case V_WORD: r = mk(WC[W_WLIT], P(v, 0), 0, 0, 0); keep = true; break;
    case V_WPLUS: r = mk(WC[W_WPLUS], word_wn(cell_wn(v)), v->port[1], 0, 0); break;
    case V_NWOP: r = mk(WC[W_WOP], word_of((uint64_t)(v->x - O_WEQ)), v->port[1], v->port[2], 0); break;
    case V_NWELIM: r = mk(WC[W_WELIM], v->port[1], v->port[2], v->port[3], 0); break;
    case V_PI: r = mk(WC[W_PI], v->port[1], v->port[2], 0, 0); break;
    case V_SIGMA: r = mk(WC[W_SIGMA], v->port[1], v->port[2], 0, 0); break;
    case V_U: r = mk(WC[W_U], v->port[1], 0, 0, 0); break;
    case V_PATHP: r = mk(WC[W_PATH], v->port[1], v->port[2], v->port[3], 0); break;
    case V_GLUE: r = mk(WC[W_GLUE], v->port[1], v->port[2], 0, 0); break;
    case V_GLUEV: r = mk(WC[W_GLUEV], v->port[1], v->port[2], 0, 0); break;
    case V_HCOMP: r = mk(WC[W_HCOMP], v->port[1], v->port[2], 0, 0); break;
    case V_SYS: {
        int nb = v->n - 1; Dnf *cl = v->p;
        Port *bs = malloc((size_t)(nb + 1) * sizeof(Port));
        for (int k = 0; k < nb; k++) bs[k] = pair_of(face_cell(face_of_clause(cl->c[k])), v->port[k + 1]);
        r = mk(WC[W_SYS], list_of(bs, nb), 0, 0, 0);
        free(bs);
        break;
    }
    case V_DATA: {
        int d = data_checked_index((int)v->a);
        if (!strcmp(DATA[v->a].name, "Level")) { r = con_build(WC[W_LEVEL], NULL, NULL); break; }
        if (d < 0) { for (int k = 1; k < v->n; k++) era(v->port[k]); r = con_build(WC[W_OTHER], NULL, NULL); break; }
        r = mk(WC[W_DATA], word_of((uint64_t)d), list_of(&v->port[1], v->n - 1), 0, 0);
        break;
    }
    case V_CON: {
        /* a constructor of a checked data type, or of Level (as data type 0: levels are read by
         * their constructors only); of any other, nothing the checker reads */
        ConInfo *k = &CON[v->a];
        int d = data_checked_index(k->data);
        if (d < 0 && strcmp(DATA[k->data].name, "Level")) { for (int q = 1; q < v->n; q++) era(v->port[q]); r = con_build(WC[W_OTHER], NULL, NULL); break; }
        r = mk(WC[W_CON], word_of((uint64_t)(d < 0 ? 0 : d)), word_of((uint64_t)k->index), list_of(&v->port[1], k->nfields), list_of(&v->port[1 + k->nfields], k->ndims));
        break;
    }
    case V_NVAR:
        if (v->a & LVL_TAG) r = mk(WC[W_VAR], word_of(v->a & ~LVL_TAG), v->port[1], 0, 0);
        else if ((v->ty & TY_COFIX) && global_checked((long)v->a) >= 0)
            /* the stuck head of a checked copattern definition: its index in the signature, its type */
            r = mk(WC[W_COFIX], word_of((uint64_t)global_checked((long)v->a)), v->port[1], 0, 0);
        else { era(v->port[1]); r = con_build(WC[W_OTHER], NULL, NULL); }
        break;
    case V_NPROJ: {
        FldInfo *fl = &FLD[v->a];
        int d = data_checked_index(fl->data);
        if (d < 0) { for (int q = 1; q < v->n; q++) era(v->port[q]); r = con_build(WC[W_OTHER], NULL, NULL); break; }
        r = mk(WC[W_PROJ], word_of((uint64_t)d), word_of((uint64_t)fl->index), v->port[1], v->port[2]);
        break;
    }
    case V_REC: {
        int d = data_checked_index((int)v->a);
        if (d < 0) { for (int q = 1; q < v->n; q++) era(v->port[q]); r = con_build(WC[W_OTHER], NULL, NULL); break; }
        r = mk(WC[W_REC], word_of((uint64_t)d), list_of(&v->port[1], v->n - 1), 0, 0);
        break;
    }
    case V_NAPP: r = mk(WC[W_APP], v->port[1], v->port[2], v->port[3], 0); break;
    case V_NFST: r = mk(WC[W_FST], v->port[1], v->port[2], 0, 0); break;
    case V_NSND: r = mk(WC[W_SND], v->port[1], v->port[2], 0, 0); break;
    case V_NPAPP: {
        Cell *iv = cell(V_IV, 1); iv->p = dnf_retain(v->p);
        if (v->x) { era(v->port[2]); era(v->port[3]); }
        r = mk(WC[W_PAPP], v->port[1], P(iv, 0), v->port[v->n - 1], 0);
        break;
    }
    case V_NCOMP: r = mk(WC[W_COMP], v->port[1], v->port[2], v->port[3], v->port[4]); break;
    case V_NUNGLUE: r = mk(WC[W_UNGLUE], v->port[1], v->port[2], v->port[3], 0); break;
    case V_NCASE: {
        /* the elimination as a function of its scrutinee: its table and environment (its motive,
         * when it has one, is the table's over that environment: %motive) */
        CaseTab *tb = v->p;
        if (data_checked_index(tb->d) < 0) { for (int q = 1; q < v->n; q++) era(v->port[q]); r = con_build(WC[W_OTHER], NULL, NULL); break; }
        int mb = v->x ? 1 : 0, ne = v->n - 3 - mb;
        Cell *f = cell(V_CASEF, ne + 1); f->x = 0; f->p = tb;
        if (mb) era(v->port[2]);
        for (int e = 0; e < ne; e++) link(v->port[2 + mb + e], P(f, 1 + e));
        r = mk(WC[W_ELIM], word_of((uint64_t)data_checked_index(tb->d)), v->port[1], P(f, 0), v->port[v->n - 1]);
        break;
    }
    case V_NTRANS: {
        int np = v->n - 3;
        if (data_checked_index((int)v->a) < 0) { for (int q = 1; q < v->n; q++) era(v->port[q]); r = con_build(WC[W_OTHER], NULL, NULL); break; }
        Cell *fc = cell(V_FACE, 1); fc->p = dnf_retain(v->p);
        Port l = list_of(&v->port[2], np);
        Port f[5] = { word_of((uint64_t)data_checked_index((int)v->a)), P(fc, 0), l, v->port[1], v->port[v->n - 1] };
        r = con_build(WC[W_TRANS], f, NULL);
        break;
    }
    default:
        for (int k = 1; k < v->n; k++) era(v->port[k]);
        r = con_build(WC[W_OTHER], NULL, NULL);
        break;
    }
    link(r, out);
    cell_free(o);
    if (!keep) cell_free(v);
}

/* an induction hypothesis: the elimination of its field (1=field [2=motive] environment) */
static Port case_of_susp(Cell *v) {
    Cell *c = cell(O_CASE, v->n); c->x = v->x; c->p = v->p;
    for (int k = 1; k < v->n; k++) link(v->port[k], P(c, k - 1));
    if (v->key) { c->key = key_retain(v->key); memo_register(c); }
    cell_free(v);
    return P(c, c->n - 1);
}
/* a word's hypothesis: the elimination of its word (1=z 2=s 3=w) */
static Port welim_of_wsusp(Cell *v) {
    Cell *e = cell(O_WELIM, 4);
    link(v->port[3], P(e, 0)); link(v->port[1], P(e, 1)); link(v->port[2], P(e, 2));
    cell_free(v);
    return P(e, 3);
}
/* words: a literal #k, k successors of a neutral (V_WPLUS), or a neutral. wsplit reads a word as
 * (k, base), base 0 for a literal; it returns the cell to free (the literal or V_WPLUS) */
static Cell *wsplit(Cell *c, WN *k, Port *base) {
    if (c->kind == V_WORD) { *k = cell_wn(c); *base = 0; return c; }
    if (c->kind == V_WPLUS) { *k = cell_wn(c); *base = c->port[1]; return c; }
    *k = wn_small(0); *base = P(c, 0); return NULL;
}
/* k ⊕ base (taking k) */
static Port wplus(WN k, Port base) {
    if (!base) return word_wn(k);
    if (wn_zero(&k)) return base;
    Cell *w = cell(V_WPLUS, 2); w->a = k.s; w->p = k.b; link(base, P(w, 1));
    return P(w, 0);
}
static Port wstuck(int op, Port a, Port b) {
    Cell *n = cell(V_NWOP, 3); n->x = (uint32_t)op;
    link(a, P(n, 1)); link(b, P(n, 2));
    return P(n, 0);
}

/* an operation on a value it has no rule for (an ill-typed term, run unchecked): the result is
 * unknown, the operands erased — the machine does not stop, and a checker reading the value of a
 * term it rejected reads ? there */
static void stuck(Cell *o, Cell *v) {
    for (int k = 1; k < v->n; k++) era(v->port[k]);
    for (int k = 1; k < o->n - 1; k++) era(o->port[k]);
    link(unk(), o->port[o->n - 1]);
    cell_free(o); cell_free(v);
}


/* CUBICAL_TRACE: one line per interaction during a (show ...) reading — the operation, the value
 * it met, and what each names (the definition a λ or thunk was compiled in, a constructor, a global,
 * a word, the data type an elimination is over) */
FILE *trace_out;
static const char *const KIND_NAME[K_END] = {
    [V_LAM] = "LAM",
    [V_PAIR] = "PAIR",
    [V_PI] = "PI",
    [V_SIGMA] = "SIGMA",
    [V_U] = "U",
    [V_PATHP] = "PATHP",
    [V_PLAM] = "PLAM",
    [V_IV] = "IV",
    [V_FACE] = "FACE",
    [V_SYS] = "SYS",
    [V_GLUE] = "GLUE",
    [V_GLUEV] = "GLUEV",
    [V_NVAR] = "NVAR",
    [V_NAPP] = "NAPP",
    [V_NFST] = "NFST",
    [V_NSND] = "NSND",
    [V_NPAPP] = "NPAPP",
    [V_NCOMP] = "NCOMP",
    [V_NUNGLUE] = "NUNGLUE",
    [V_NCASE] = "NCASE",
    [V_NTRANS] = "NTRANS",
    [V_NPROJ] = "NPROJ",
    [V_REC] = "REC",
    [V_GLOBAL] = "GLOBAL",
    [V_UNK] = "UNK",
    [V_DATA] = "DATA",
    [V_CON] = "CON",
    [V_HCOMP] = "HCOMP",
    [V_CASEF] = "CASEF",
    [V_REF] = "REF",
    [V_WORD] = "WORD",
    [V_WSUSP] = "WSUSP",
    [V_WTY] = "WTY",
    [V_THUNK] = "THUNK",
    [V_WPLUS] = "WPLUS",
    [V_NWOP] = "NWOP",
    [V_NWELIM] = "NWELIM",
    [V_SUSP] = "SUSP",
    [O_APP] = "APP",
    [O_FORCE] = "FORCE",
    [O_FST] = "FST",
    [O_SND] = "SND",
    [O_PAPPR] = "PAPPR",
    [O_PAPPP] = "PAPPP",
    [O_SUBST] = "SUBST",
    [O_DUP] = "DUP",
    [O_ERA] = "ERA",
    [O_COMP] = "COMP",
    [O_COMPA] = "COMPA",
    [O_COMPH] = "COMPH",
    [O_MKPLAM] = "MKPLAM",
    [O_MKGLUE] = "MKGLUE",
    [O_MKGLUEV] = "MKGLUEV",
    [O_UNGLUE] = "UNGLUE",
    [O_UNGLUE2] = "UNGLUE2",
    [O_FACEOF] = "FACEOF",
    [O_NEG] = "NEG",
    [O_BIN] = "BIN",
    [O_BIN2] = "BIN2",
    [O_FEQ] = "FEQ",
    [O_FORALL] = "FORALL",
    [O_SYSCONS] = "SYSCONS",
    [O_SYSCONS2] = "SYSCONS2",
    [O_SYSMAP] = "SYSMAP",
    [O_TPART] = "TPART",
    [O_CONCHK] = "CONCHK",
    [O_HCOMP] = "HCOMP",
    [O_TRANSD1] = "TRANSD1",
    [O_TRANSD2] = "TRANSD2",
    [O_CASE] = "CASE",
    [O_VIEW] = "VIEW",
    [O_MKVAR] = "MKVAR",
    [O_MKDIM] = "MKDIM",
    [O_IEQ] = "IEQ",
    [O_IEQB] = "IEQB",
    [O_FEQL] = "FEQL",
    [O_FEQLB] = "FEQLB",
    [O_CLAUSES] = "CLAUSES",
    [O_DNF] = "DNF",
    [O_MOTIVE] = "MOTIVE",
    [O_RESTRICT] = "RESTRICT",
    [O_GLOB] = "GLOB",
    [O_ABS] = "ABS",
    [O_NTYPE] = "NTYPE",
    [O_CALL] = "CALL",
    [O_WSUCC] = "WSUCC",
    [O_WEQ] = "WEQ",
    [O_WLT] = "WLT",
    [O_WADD] = "WADD",
    [O_WMAX] = "WMAX",
    [O_WSUB] = "WSUB",
    [O_WOP2] = "WOP2",
    [O_WELIM] = "WELIM",
    [O_PEND] = "PEND",
    [O_CLOS] = "CLOS",
    [O_CLOS2] = "CLOS2",
    [O_BRANCH] = "BRANCH",
    [O_BRANCHC] = "BRANCHC",
    [O_BRANCHL] = "BRANCHL",
    [O_FWD] = "FWD",
    [O_SHARE] = "SHARE",
    [O_ROOT] = "ROOT",
    [O_PROJ] = "PROJ",
    [O_UNFOLD] = "UNFOLD",
    [O_DAPPLY] = "DAPPLY",
};

static void trace_side(Cell *c) {
    switch (c->kind) {
    case V_LAM: case V_THUNK: { Record *r = c->p; if (r && r->where >= 0 && r->where < prof_n) fprintf(trace_out, "<%s>", prof_name[r->where]); break; }
    case V_SUSP: case V_CASEF: case V_NCASE: case O_CASE: { CaseTab *t = c->p; if (t) fprintf(trace_out, "<%s>", DATA[t->d].name); break; }
    case V_GLOBAL: fprintf(trace_out, "<%s>", name_of((long)c->a)); break;
    case V_CON: fprintf(trace_out, "<%s>", CON[c->a].name); break;
    case V_WORD: if (!c->p) fprintf(trace_out, "<#%llu>", (unsigned long long)c->a); break;
    default: break;
    }
}
static void trace_step(Cell *o, Cell *v) {
    fprintf(trace_out, "%ld %s", stat_steps, KIND_NAME[o->kind] ? KIND_NAME[o->kind] : "?");
    trace_side(o);
    fprintf(trace_out, " · %s", KIND_NAME[v->kind] ? KIND_NAME[v->kind] : "?");
    trace_side(v);
    fputc('\n', trace_out);
}
/* ---------------- the rules ---------------- */
static void interact(Cell *o, Cell *v) {
    if (o->kind == O_FWD) {
        /* a read waiting for its value (lazy_copy): not a step of its own. An unbuilt value is
         * built, as the reader would have built it in place; a value is passed on to where it is
         * held, and the waiting reader gets its one-level copy */
        if (unbuilt(v->kind)) { link(unbuilt_build(v), P(o, 0)); return; }
        Port c = copy_top(o->key, v);
        link(c, o->port[2]);
        link(P(v, 0), o->port[1]);
        cell_free(o);
        return;
    }
    if (o->kind == O_SHARE) {
        /* a part of a read value that arrived after the read (copy_top): shared exactly as the
         * read would have shared it had it been there; not a step of its own */
        rule_dup(o, v);
        return;
    }
    ADD(stat_steps, 1); ADD(stat_kind[o->kind], 1);
    if (trace_out) trace_step(o, v);
    if (prof_on) {
        long a = (long)o->a;
        if (o->kind == O_DUP || o->kind == O_ERA) {
            long *rec = o->kind == O_DUP ? prof_odup : prof_oera, *rul = o->kind == O_DUP ? prof_rdup : prof_rera;
            if (a > 0 && a < 65536) ADD(rec[a], 1); else if (a < 0 && -a < 256) ADD(rul[-a], 1); else ADD(rec[0], 1);
        }
        dup_origin = -(long)o->kind;
    }
    switch (o->kind) {
    case O_ERA: rule_era(o, v); return;
    case O_DUP: rule_dup(o, v); return;
    case O_SUBST: rule_subst(o, v); return;
    case O_MKPLAM: rule_mkplam(o, v); return;
    default: break;
    }
    if (v->kind == V_GLOBAL && o->kind == O_CALL) {
        long g = (long)v->a;
        Record *r = global_nary(g);
        int na = o->n - 2;
        if (r && r->nparam <= na) {
            /* all the parameters: the body at once; any further arguments applied to it */
            if (prof_on) ADD(prof_count[r->where], 1);
            Box *bx; Port sh = memo_begin(o, &bx, true);
            if (sh && !bx) { for (int k = 0; k < na; k++) era(o->port[1 + k]); link(sh, o->port[o->n - 1]); cell_free(o); cell_free(v); return; }
            Port res = record_build_n(r, &o->port[1], NULL);
            for (int k = r->nparam; k < na; k++) res = app2(res, o->port[1 + k]);
            link(memo_end(sh, bx, res), o->port[o->n - 1]);
            cell_free(o); cell_free(v);
            return;
        }
        /* otherwise applications, one argument at a time */
        Port f = P(v, 0);
        for (int k = 0; k < na; k++) f = app2(f, o->port[1 + k]);
        link(f, o->port[o->n - 1]);
        cell_free(o);
        return;
    }
    if (v->kind == V_GLOBAL) {
        /* a definition is used: its value is copied in */
        Port r = global_copy((long)v->a, v->key);
        if (!r) { global_park((long)v->a, o, v); return; }
        link(r, P(o, 0));
        cell_free(v);
        return;
    }
    /* an unbuilt subterm meeting an operation is built (an explicit force of a hypothesis too) */
    if (unbuilt(v->kind)) { link(unbuilt_build(v), P(o, 0)); return; }
    if (v->kind == V_REF) {
        /* the value is copied out of its box */
        link(ref_read(v), P(o, 0));
        cell_free(v);
        return;
    }
    int vk = v->kind;
    if (vk == V_UNK && o->kind != O_VIEW && o->kind != O_CLOS && o->kind != O_CLOS2 && o->kind != O_PEND) {
        /* an eliminator of an unknown type gives an unknown type */
        for (int k = 1; k < o->n - 1; k++) era(o->port[k]);
        link(P(v, 0), out_of(o));
        cell_free(o);
        return;
    }
    switch (o->kind) {
    case O_APP:
        if (vk == V_CASEF) {
            /* the elimination as a function, applied: the case on the argument */
            Cell *c = cell(O_CASE, v->n + 1); c->x = v->x; c->p = v->p;
            mv(o, 1, P(c, 0));
            for (int k = 1; k < v->n; k++) link(v->port[k], P(c, k));
            link(P(c, c->n - 1), o->port[2]);
            cell_free(o); cell_free(v);
            return;
        }
        if (vk == V_LAM && ((Record *)v->p)->nparam == 1) {
            if (prof_on) ADD(prof_count[((Record *)v->p)->where], 1);
            Box *bx; Port sh = memo_begin(o, &bx, true);
            if (sh && !bx) { era(o->port[1]); era(P(v, 0)); link(sh, o->port[2]); cell_free(o); return; }
            Port r = record_build(v->p, o->port[1], &v->port[1]);
            link(memo_end(sh, bx, r), o->port[2]);
            cell_free(o); cell_free(v);
        } else if (vk == V_SYS) distribute(o, v);
        else {
            /* stuck: a neutral application (its type computed from the spine when needed). A stuck
             * call of a copattern definition stays one, and is named by its head and argument */
            Cell *n = cell(V_NAPP, 4);
            if (is_neutral(vk) && (v->ty & TY_COFIX)) {
                n->ty = TY_COFIX;
                if (v->key) { Key *ak = key_of_port(o->port[1]); n->key = key2(K_APP, 0, NULL, v->key, ak); if (ak) key_release(ak); }
            }
            mv(o, 1, P(n, 2)); link(unk(), P(n, 3));
            link(P(v, 0), P(n, 1));
            link(P(n, 0), o->port[2]);
            cell_free(o);
        }
        return;
    case O_FORCE:
        if (vk == V_LAM && ((Record *)v->p)->nparam == 0) {
            if (prof_on) ADD(prof_count[((Record *)v->p)->where], 1);
            link(record_build(v->p, 0, &v->port[1]), o->port[1]);
            cell_free(o); cell_free(v);
        } else if (vk == V_SYS) distribute(o, v);
        else { link(P(v, 0), o->port[1]); cell_free(o); }
        return;
    case O_FST: case O_SND:
        if (vk == V_PAIR) {
            int keep = o->kind == O_FST ? 1 : 2;
            link(v->port[keep], o->port[1]); era(v->port[3 - keep]);
            cell_free(o); cell_free(v);
        } else if (vk == V_SYS) distribute(o, v);
        else neutral_proj(o, v);
        return;
    case O_PAPPR: {
        if (vk != V_IV) { stuck(o, v); return; }
        Dnf *r = v->p;
        if (o->x && (dnf_is_zero(r) || dnf_is_one(r))) {
            /* an endpoint: answered without the path */
            int z = dnf_is_zero(r);
            link(o->port[z ? 2 : 3], o->port[4]);
            era(o->port[1]); era(o->port[z ? 3 : 2]);
        } else {
            Cell *pp = cell(O_PAPPP, o->x ? 4 : 2); pp->x = o->x; pp->p = dnf_retain(r);
            mv(o, 1, P(pp, 0));
            if (o->x) { mv(o, 2, P(pp, 1)); mv(o, 3, P(pp, 2)); }
            link(P(pp, pp->n - 1), out_of(o));
        }
        cell_free(o); cell_free(v);
        return;
    }
    case O_PAPPP:
        if (vk == V_PLAM) {
            Subst *t = subst_single((Dim)v->a, o->p);
            link(sub(v->port[1], t), out_of(o));
            subst_release(t);
            if (o->x) { era(o->port[1]); era(o->port[2]); }
            cell_free(o); cell_free(v);
        } else if (vk == V_SYS) distribute(o, v);
        else {
            Dnf *r = o->p;
            if (!o->x && is_neutral(vk) && (dnf_is_zero(r) || dnf_is_one(r))) {
                /* an endpoint of a neutral path, read from its type (computed from a copy of the
                 * spine): t 0 = u0 for t : Path A u0 u1 */
                Port p1, p2; split2(P(v, 0), &p1, &p2);
                Cell *e = cell(O_PEND, 3); e->p = dnf_retain(r);
                Cell *nt = cell(O_NTYPE, 2); link(p1, P(nt, 0)); link(P(nt, 1), P(e, 0));
                link(p2, P(e, 1)); link(P(e, 2), out_of(o));
                cell_free(o);
                return;
            }
            /* stuck: a neutral path application */
            Cell *n = cell(V_NPAPP, o->x ? 5 : 3); n->x = o->x; n->p = dnf_retain(r);
            if (is_neutral(vk) && (v->ty & TY_COFIX)) {
                n->ty = TY_COFIX;
                if (v->key) { Key *rk = key0(K_IV, 0, r); n->key = key2(K_PAPP, 0, NULL, v->key, rk); if (rk) key_release(rk); }
            }
            link(unk(), P(n, n->n - 1));
            link(P(v, 0), P(n, 1));
            if (o->x) { mv(o, 1, P(n, 2)); mv(o, 2, P(n, 3)); }
            link(P(n, 0), out_of(o));
            cell_free(o);
        }
        return;
    case O_COMP:
        if (vk == V_SYS) {
            /* the tube is partial: look at the family */
            Cell *ca = cell(O_COMPA, 4);
            mv(o, 1, P(ca, 0)); link(P(v, 0), P(ca, 1)); mv(o, 2, P(ca, 2)); link(P(ca, 3), o->port[3]);
        } else {
            /* the tube covers: its end */
            Cell *pr = cell(O_PAPPR, 3);
            link(iv_cell(dnf_one()), P(pr, 0)); link(P(v, 0), P(pr, 1)); link(P(pr, 2), o->port[3]);
            era(o->port[1]); era(o->port[2]);
        }
        cell_free(o);
        return;
    case O_COMPA:
        if (vk == V_PLAM) {
            /* the family at its own fresh dimension; its head decides */
            Cell *ch = cell(O_COMPH, 4); ch->a = v->a;
            link(v->port[1], P(ch, 0)); mv(o, 1, P(ch, 1)); mv(o, 2, P(ch, 2)); link(P(ch, 3), o->port[3]);
            cell_free(o); cell_free(v);
        } else if (vk == V_SYS) distribute(o, v);
        else {
            /* a neutral family: a neutral composition, of type the family at 1 */
            Cell *n = cell(V_NCOMP, 5);
            link(P(v, 0), P(n, 1)); mv(o, 1, P(n, 2)); mv(o, 2, P(n, 3)); link(unk(), P(n, 4));
            link(P(n, 0), o->port[3]);
            cell_free(o);
        }
        return;
    case O_COMPH: {
        Dim i = (Dim)o->a;
        Port u = o->port[1], a0 = o->port[2], out = o->port[3];
        Port args[5];
        const char *clause = NULL; int n = 0;
        switch (vk) {
        case V_PI: case V_SIGMA:
            args[0] = mkplam_of(v->port[1], i); args[1] = mkplam_of(v->port[2], i); args[2] = u; args[3] = a0; n = 4;
            clause = vk == V_PI ? "%comp-pi" : "%comp-sigma"; break;
        case V_PATHP:
            args[0] = mkplam_of(v->port[1], i); args[1] = mkplam_of(v->port[2], i); args[2] = mkplam_of(v->port[3], i);
            args[3] = u; args[4] = a0; n = 5; clause = "%comp-path"; break;
        case V_U: era(v->port[1]); args[0] = u; args[1] = a0; n = 2; clause = "%comp-u"; break;
        case V_DATA: {
            /* CHM: comp = hcomp after transport; %comp-D is generated by the declaration */
            int np = v->n - 1;
            Port *da = malloc((size_t)(np + 3) * sizeof(Port));
            for (int k = 0; k < np; k++) da[k] = mkplam_of(v->port[k + 1], i);
            da[np] = u; da[np + 1] = a0;
            link(apply_gid(DATA[v->a].comp, da, np + 2), out);
            free(da);
            cell_free(o); cell_free(v);
            return;
        }
        case V_GLUE:
            args[0] = mkplam_of(v->port[1], i); args[1] = mkplam_of(v->port[2], i); args[2] = u; args[3] = a0; n = 4;
            clause = "%comp-glue"; break;
        case V_SYS: {
            /* a system of families: each branch whose clause holds along the
             * whole line, ∀i, composes in its own family */
            int nb = v->n - 1; Dnf *cl = v->p;
            int keep = 0;
            for (int k = 0; k < nb; k++) if (!clause_mentions(cl->c[k], i)) keep++;
            Cell *ns = cell(V_SYS, keep + 1);
            Clause **kc = malloc((size_t)(keep + 1) * sizeof(Clause *));
            Port *uc = malloc((size_t)(keep + 1) * sizeof(Port)), *ac = malloc((size_t)(keep + 1) * sizeof(Port));
            int q = 0;
            for (int k = 0; k < nb; k++) if (!clause_mentions(cl->c[k], i)) kc[q++] = cl->c[k];
            Dnf *kl = clause_list(kc, (uint32_t)keep); free(kc);
            ns->p = kl;
            split_restrict(u, kl, keep, uc); split_restrict(a0, kl, keep, ac);
            q = 0;
            for (int k = 0; k < nb; k++) {
                if (clause_mentions(cl->c[k], i)) { era(v->port[k + 1]); continue; }
                Cell *c = cell(O_COMP, 4);
                link(uc[q], P(c, 0)); link(mkplam_of(v->port[k + 1], i), P(c, 1)); link(ac[q], P(c, 2)); link(P(c, 3), P(ns, q + 1));
                q++;
            }
            free(uc); free(ac);
            link(P(ns, 0), out);
            cell_free(o); cell_free(v);
            return;
        }
        default: {
            /* a neutral head: a neutral composition, of type the head at i = 1 */
            Cell *nc = cell(V_NCOMP, 5);
            link(mkplam_of(P(v, 0), i), P(nc, 1)); link(u, P(nc, 2)); link(a0, P(nc, 3)); link(unk(), P(nc, 4)); link(P(nc, 0), out);
            cell_free(o);
            return;
        }
        }
        link(apply_global(clause, args, n), out);
        cell_free(o); cell_free(v);
        return;
    }
    case O_MKGLUE:
        if (vk == V_SYS) { Cell *g = cell(V_GLUE, 3); mv(o, 1, P(g, 1)); link(P(v, 0), P(g, 2)); link(P(g, 0), o->port[2]); }
        else { Cell *f = cell(O_FST, 2); link(P(v, 0), P(f, 0)); link(P(f, 1), o->port[2]); era(o->port[1]); }
        cell_free(o);
        return;
    case O_MKGLUEV:
        if (vk == V_SYS) { Cell *g = cell(V_GLUEV, 3); link(P(v, 0), P(g, 1)); mv(o, 1, P(g, 2)); link(P(g, 0), o->port[2]); }
        else { link(P(v, 0), o->port[2]); era(o->port[1]); }
        cell_free(o);
        return;
    case O_UNGLUE:
        if (vk == V_SYS) {
            Cell *u2 = cell(O_UNGLUE2, 3);
            mv(o, 1, P(u2, 0)); link(P(v, 0), P(u2, 1)); link(P(u2, 2), o->port[2]);
        } else {
            /* on the face: the equivalence's map */
            Cell *sn = cell(O_SND, 2), *f = cell(O_FST, 2), *a = cell(O_APP, 3);
            link(P(v, 0), P(sn, 0)); link(P(sn, 1), P(f, 0)); link(P(f, 1), P(a, 0)); mv(o, 1, P(a, 1)); link(P(a, 2), o->port[2]);
        }
        cell_free(o);
        return;
    case O_UNGLUE2:
        if (vk == V_GLUEV) {
            link(v->port[2], o->port[2]); era(v->port[1]); era(o->port[1]);
            cell_free(o); cell_free(v);
        } else if (vk == V_SYS) distribute(o, v);
        else {
            /* stuck: a neutral unglue, of type the Glue type's base */
            Cell *n = cell(V_NUNGLUE, 4);
            link(unk(), P(n, 3));
            mv(o, 1, P(n, 1)); link(P(v, 0), P(n, 2)); link(P(n, 0), o->port[2]);
            cell_free(o);
        }
        return;
    case O_FACEOF: {
        Cell *f = cell(V_FACE, 1);
        if (vk == V_SYS) {
            f->p = face_of_clauses(v->p);
            for (int k = 1; k < v->n; k++) era(v->port[k]);
            cell_free(v);
        } else { f->p = dnf_one(); era(P(v, 0)); }
        link(P(f, 0), o->port[1]);
        cell_free(o);
        return;
    }
    case O_NEG:
        if (vk != V_IV) { stuck(o, v); return; }
        { Dnf *n2 = iv_neg(v->p); dnf_release(v->p); v->p = n2; }
        link(P(v, 0), o->port[1]); cell_free(o);
        return;
    case O_BIN: {
        if (vk != V_IV && vk != V_FACE) { stuck(o, v); return; }
        Cell *b = cell(O_BIN2, 2); b->x = o->x; b->p = dnf_retain(v->p);
        mv(o, 1, P(b, 0)); link(P(b, 1), o->port[2]);
        cell_free(o); cell_free(v);
        return;
    }
    case O_BIN2: {
        bool face = o->x == B_FAND || o->x == B_FOR;
        if (vk != (face ? V_FACE : V_IV)) { stuck(o, v); return; }
        Dnf *n2 = (o->x == B_MEET || o->x == B_FAND) ? dnf_meet(o->p, v->p, face) : dnf_join(o->p, v->p, face);
        dnf_release(v->p); v->p = n2;
        link(P(v, 0), o->port[1]); cell_free(o);
        return;
    }
    case O_FEQ: {
        if (vk != V_IV) { stuck(o, v); return; }
        Cell *f = cell(V_FACE, 1); f->p = face_eq(v->p, (int)o->a);
        link(P(f, 0), o->port[1]); cell_free(o); cell_free(v);
        return;
    }
    case O_FORALL:
        if (vk != V_FACE) { stuck(o, v); return; }
        { Dnf *n2 = face_forall(v->p, (Dim)o->a); dnf_release(v->p); v->p = n2; }
        link(P(v, 0), o->port[1]); cell_free(o);
        return;
    case O_SYSCONS: {
        if (vk != V_FACE) { stuck(o, v); return; }
        Dnf *f = v->p;
        if (dnf_is_one(f)) {
            Cell *fo = cell(O_FORCE, 2); mv(o, 1, P(fo, 0)); link(P(fo, 1), o->port[3]); era(o->port[2]);
        } else if (dnf_is_zero(f)) {
            link(o->port[2], o->port[3]); era(o->port[1]);
        } else {
            Cell *s2 = cell(O_SYSCONS2, 3); s2->p = dnf_retain(f);
            mv(o, 2, P(s2, 0)); mv(o, 1, P(s2, 1)); link(P(s2, 2), o->port[3]);
        }
        cell_free(o); cell_free(v);
        return;
    }
    case O_SYSCONS2:
        if (vk == V_SYS) {
            /* the branch, forced under each clause of its face, joins the system */
            Dnf *f = o->p;
            int nb = v->n - 1, m = (int)f->n;
            Cell *ns = cell(V_SYS, nb + m + 1);
            Clause **ncl = malloc((size_t)(nb + m + 1) * sizeof(Clause *));
            Dnf *cl = v->p;
            for (int k = 0; k < nb; k++) { ncl[k] = cl->c[k]; mv(v, k + 1, P(ns, k + 1)); }
            Port *cp = malloc((size_t)(m + 1) * sizeof(Port));
            split_restrict(o->port[1], f, m, cp);
            for (int q = 0; q < m; q++) {
                ncl[nb + q] = f->c[q];
                Cell *fo = cell(O_FORCE, 2); link(cp[q], P(fo, 0)); link(P(fo, 1), P(ns, nb + q + 1));
            }
            ns->p = clause_list(ncl, (uint32_t)(nb + m));
            free(ncl);
            free(cp);
            link(P(ns, 0), o->port[2]);
            cell_free(o); cell_free(v);
        } else {
            /* the rest is total */
            link(P(v, 0), o->port[2]); era(o->port[1]);
            cell_free(o);
        }
        return;
    case O_TPART: {
        int k = (int)o->a;
        if ((vk == V_PI || vk == V_SIGMA || vk == V_PATHP || vk == V_GLUE) && k < v->n) {
            link(v->port[k], o->port[1]);
            for (int m = 1; m < v->n; m++) if (m != k) era(v->port[m]);
            cell_free(o); cell_free(v);
        } else if (vk == V_SYS) distribute(o, v);
        else { link(unk(), o->port[1]); era(P(v, 0)); cell_free(o); }
        return;
    }
    case O_CONCHK: {
        int n = o->n - 3;
        if (vk == V_FACE && dnf_is_one(v->p)) {
            /* the boundary's face holds: the constructor is its boundary */
            link(o->port[1], o->port[n + 2]);
            for (int j = 0; j < n; j++) era(o->port[2 + j]);
        } else {
            Cell *c = cell(V_CON, n + 1); c->a = o->a;
            for (int j = 0; j < n; j++) mv(o, 2 + j, P(c, 1 + j));
            era(o->port[1]);
            link(P(c, 0), o->port[n + 2]);
        }
        cell_free(o); cell_free(v);
        return;
    }
    case O_HCOMP:
        if (vk == V_SYS) {
            /* a partial tube: a formal hcomp, a value of the data type */
            Cell *h = cell(V_HCOMP, 3);
            link(P(v, 0), P(h, 1)); mv(o, 1, P(h, 2)); link(P(h, 0), o->port[2]);
        } else {
            Cell *pr = cell(O_PAPPR, 3);
            link(iv_cell(dnf_one()), P(pr, 0)); link(P(v, 0), P(pr, 1)); link(P(pr, 2), o->port[2]);
            era(o->port[1]);
        }
        cell_free(o);
        return;
    case O_TRANSD1: {
        int np = o->n - 3;
        if (vk != V_FACE) { stuck(o, v); return; }
        if (dnf_is_one(v->p)) {
            /* on ψ the family is constant: transport is the identity */
            link(o->port[np + 1], o->port[np + 2]);
            for (int k = 1; k <= np; k++) era(o->port[k]);
        } else {
            Cell *t = cell(O_TRANSD2, np + 2); t->a = o->a; t->p = dnf_retain(v->p);
            mv(o, np + 1, P(t, 0));
            for (int k = 1; k <= np; k++) mv(o, k, P(t, k));
            link(P(t, np + 1), o->port[np + 2]);
        }
        cell_free(o); cell_free(v);
        return;
    }
    case O_TRANSD2: {
        int np = o->n - 2;
        Port out = o->port[np + 1];
        if (vk == V_CON) {
            ConInfo *k = &CON[v->a];
            int n = k->nfields + k->ndims;
            Port *a = malloc((size_t)(np + n + 2) * sizeof(Port));
            for (int j = 0; j < np; j++) a[j] = o->port[1 + j];
            a[np] = face_cell(dnf_retain(o->p));
            for (int j = 0; j < n; j++) a[np + 1 + j] = v->port[1 + j];
            link(apply_gid(k->trans, a, np + 1 + n), out);
            free(a);
            cell_free(o); cell_free(v);
        } else if (vk == V_HCOMP) {
            Port *a = malloc((size_t)(np + 4) * sizeof(Port));
            for (int j = 0; j < np; j++) a[j] = o->port[1 + j];
            a[np] = face_cell(dnf_retain(o->p)); a[np + 1] = v->port[1]; a[np + 2] = v->port[2];
            link(apply_gid(DATA[o->a].transh, a, np + 3), out);
            free(a);
            cell_free(o); cell_free(v);
        } else if (vk == V_SYS) distribute(o, v);
        else {
            Cell *n = cell(V_NTRANS, np + 3); n->a = o->a; n->p = dnf_retain(o->p);
            link(P(v, 0), P(n, 1));
            for (int j = 0; j < np; j++) mv(o, 1 + j, P(n, 2 + j));
            link(unk(), P(n, np + 2));
            link(P(n, 0), out);
            cell_free(o);
        }
        return;
    }
    case O_CASE: {
        /* 0=scrutinee [1=motive] then the environment, out */
        CaseTab *tb = o->p;
        int ne = tb->nenv, mb = o->x ? 1 : 0, e0 = 1 + mb;
        if (vk == V_CON) {
            /* the selected branch's body is built with the constructor's fields and dimensions
             * and the shared variables: one interaction. A branch taking induction hypotheses
             * gets, per recursive field, its elimination by the same case, suspended */
            ConInfo *ci = &CON[v->a];
            int k = ci->index, nf = ci->nfields, nd = ci->ndims, nr = ci->nrec;
            bool ih = nr && (tb->mask >> k & 1);
            int np = nf + (ih ? nr : 0) + nd;
            Port prm[np + 1], env[ne + 1];
            if (!ih) {
                for (int j = 0; j < nf + nd; j++) prm[j] = v->port[1 + j];
                for (int e = 0; e < ne; e++) env[e] = o->port[e0 + e];
                if (mb) era(o->port[1]);
            } else {
                Port mc[nr + 1], ec[ne + 1][nr + 2];
                if (mb) split(o->port[1], nr, mc);
                for (int e = 0; e < ne; e++) { split(o->port[e0 + e], nr + 1, ec[e]); env[e] = ec[e][0]; }
                int q = 0;
                for (int j = 0; j < nf; j++) {
                    if (ci->recmask >> j & 1) {
                        Port fa, fb; split2(v->port[1 + j], &fa, &fb);
                        prm[j] = fa;
                        /* 1=field [2=motive] then the environment */
                        Cell *h = cell(V_SUSP, ne + 2 + mb); h->x = o->x; h->p = tb;
                        link(fb, P(h, 1));
                        if (mb) link(mc[q], P(h, 2));
                        for (int e = 0; e < ne; e++) link(ec[e][q + 1], P(h, 2 + mb + e));
                        /* its key: the case term on the field (the motive is not part of the term) */
                        { Key **ck = malloc(((size_t)ne + 2) * sizeof(Key *)); ck[0] = key_of_port(fb);
                          for (int e = 0; e < ne; e++) ck[1 + e] = key_of_port(ec[e][q + 1]);
                          h->key = key(K_CASE, 0, tb, ne + 1, ck);
                          for (int e = 0; e < ne + 1; e++) if (ck[e]) key_release(ck[e]);
                          free(ck); }
                        prm[nf + q] = P(h, 0);
                        q++;
                    } else prm[j] = v->port[1 + j];
                }
                for (int j = 0; j < nd; j++) prm[nf + nr + j] = v->port[1 + nf + j];
            }
            Box *bx; Port sh = memo_begin(o, &bx, true);
            if (sh && !bx) { for (int j = 0; j < np; j++) era(prm[j]); for (int e = 0; e < ne; e++) era(env[e]); link(sh, o->port[o->n - 1]); cell_free(o); cell_free(v); return; }
            scrut_key = v->key ? key_retain(v->key) : NULL;   /* the branch's rebuilt scrutinee is named by this value */
            link(memo_end(sh, bx, record_build_n(tb->br[k], prm, env)), o->port[o->n - 1]);
            if (scrut_key) { key_release(scrut_key); scrut_key = NULL; }
            cell_free(o); cell_free(v);
        } else if (vk == V_HCOMP && tb->motive) {
            /* f (hcomp^i [φ ↦ u] u0) = comp^i P(hfill) [φ ↦ f u] (f u0), f this elimination; the
             * motive P is built here, from the environment, since this rule reads it */
            Port me[ne + 1];
            Cell *f = cell(V_CASEF, ne + 1); f->x = 0; f->p = tb;
            for (int e = 0; e < ne; e++) { Port a1, a2; split2(o->port[e0 + e], &a1, &a2); me[e] = a1; link(a2, P(f, 1 + e)); }
            Port a[4] = { record_build_n(tb->motive, NULL, me), P(f, 0), v->port[1], v->port[2] };
            link(apply_gid(case_hcomp_global, a, 4), o->port[o->n - 1]);
            cell_free(o); cell_free(v);
        } else if (vk == V_SYS) distribute(o, v);
        else {
            /* stuck: a neutral elimination, of type the motive at the scrutinee */
            /* 1=scrutinee [2=motive] then the environment, type */
            /* its motive, which types it, is built here from the environment */
            int nm = tb->motive ? 1 : 0;
            Cell *n = cell(V_NCASE, ne + 3 + nm); n->x = (uint32_t)nm; n->p = tb;
            link(P(v, 0), P(n, 1)); link(unk(), P(n, n->n - 1));
            if (nm) {
                Port me[ne + 1];
                for (int e = 0; e < ne; e++) { Port a1, a2; split2(o->port[e0 + e], &a1, &a2); me[e] = a1; link(a2, P(n, 3 + e)); }
                link(record_build_n(tb->motive, NULL, me), P(n, 2));
            } else for (int e = 0; e < ne; e++) mv(o, e0 + e, P(n, 2 + e));
            link(P(n, 0), o->port[o->n - 1]);
            cell_free(o);
        }
        return;
    }
    case O_VIEW: rule_view(o, v); return;
    case O_WEQ: case O_WLT: case O_WADD: case O_WMAX: case O_WSUB: {
        /* a is read: wait for b */
        Cell *c = cell(O_WOP2, 3); c->x = o->kind;
        mv(o, 1, P(c, 0)); link(P(v, 0), P(c, 1)); mv(o, 2, P(c, 2));
        cell_free(o);
        return;
    }
    case O_WSUCC: {
        WN k; Port b; Cell *fr = wsplit(v, &k, &b);
        WN k1 = wn_succ(&k); wn_drop(&k);
        link(wplus(k1, b), o->port[1]);
        if (fr) cell_free(fr);
        cell_free(o);
        return;
    }
    case O_WELIM: {
        WN k; Port b; Cell *fr = wsplit(v, &k, &b);
        if (fr) cell_free(fr);
        if (wn_zero(&k) && !b) { link(o->port[1], o->port[3]); era(o->port[2]); }
        else if (wn_zero(&k)) {
            /* a neutral word: the elimination is stuck */
            Cell *e = cell(V_NWELIM, 4);
            link(b, P(e, 1)); mv(o, 1, P(e, 2)); mv(o, 2, P(e, 3)); link(P(e, 0), o->port[3]);
        } else {
            /* s (k-1 ⊕ b) h, h the elimination of k-1 ⊕ b, computed when used */
            WN one = wn_small(1), km = wn_sub(&k, &one);
            Port s1, s2, p1, p2;
            split2(o->port[2], &s1, &s2);
            split2(wplus(km, b), &p1, &p2);
            Cell *h = cell(V_WSUSP, 4);
            link(o->port[1], P(h, 1)); link(s2, P(h, 2)); link(p2, P(h, 3));
            link(app2(app2(s1, p1), P(h, 0)), o->port[3]);
        }
        wn_drop(&k);
        cell_free(o);
        return;
    }
    case O_WOP2: {
        need_bool();
        Cell *ac = PC(o->port[1]);
        WN ka, kb; Port ba, bb;
        Cell *fa = wsplit(ac, &ka, &ba), *fb = wsplit(v, &kb, &bb);
        if (fa) cell_free(fa);
        if (fb) cell_free(fb);
        int op = (int)o->x;
        Port r = 0;
        if (!ba && !bb) {
            int c = wn_cmp(&ka, &kb);
            switch (op) {
            case O_WEQ: r = con_build(c == 0 ? B_TRUE : B_FALSE, NULL, NULL); break;
            case O_WLT: r = con_build(c < 0 ? B_TRUE : B_FALSE, NULL, NULL); break;
            case O_WADD: r = word_wn(wn_add(&ka, &kb)); break;
            case O_WMAX: r = word_wn(wn_copy(c > 0 ? &ka : &kb)); break;
            default: r = word_wn(wn_sub(&ka, &kb)); break;
            }
        } else if (op == O_WADD) {
            /* by recursion on b */
            if (!bb) r = wplus(wn_add(&ka, &kb), ba);
            else r = wplus(wn_copy(&kb), wstuck(O_WADD, wplus(wn_copy(&ka), ba), bb));
        } else {
            /* by recursion on both: successors cancel */
            WN c = wn_cmp(&ka, &kb) < 0 ? wn_copy(&ka) : wn_copy(&kb);
            WN a1 = wn_sub(&ka, &c), b1 = wn_sub(&kb, &c);
            bool za = wn_zero(&a1) && !ba, zb = wn_zero(&b1) && !bb;   /* literally zero */
            bool pa = !wn_zero(&a1), pb = !wn_zero(&b1);               /* a successor */
            switch (op) {
            case O_WEQ:
                if ((za && pb) || (zb && pa)) { r = con_build(B_FALSE, NULL, NULL); if (ba) era(ba); if (bb) era(bb); break; }
                r = wstuck(op, wplus(wn_copy(&a1), ba), wplus(wn_copy(&b1), bb)); break;
            case O_WLT:
                if (zb) { r = con_build(B_FALSE, NULL, NULL); if (ba) era(ba); break; }
                if (za && pb) { r = con_build(B_TRUE, NULL, NULL); if (bb) era(bb); break; }
                r = wstuck(op, wplus(wn_copy(&a1), ba), wplus(wn_copy(&b1), bb)); break;
            case O_WMAX:
                if (za) { r = wplus(wn_add(&c, &b1), bb); break; }
                if (zb) { r = wplus(wn_add(&c, &a1), ba); break; }
                r = wplus(wn_copy(&c), wstuck(op, wplus(wn_copy(&a1), ba), wplus(wn_copy(&b1), bb))); break;
            default:
                if (zb) { r = wplus(wn_copy(&a1), ba); break; }
                if (za) { r = word_of(0); if (bb) era(bb); break; }
                r = wstuck(op, wplus(wn_copy(&a1), ba), wplus(wn_copy(&b1), bb)); break;
            }
            wn_drop(&c); wn_drop(&a1); wn_drop(&b1);
        }
        wn_drop(&ka); wn_drop(&kb);
        link(r, o->port[2]);
        cell_free(o);
        return;
    }
    case O_MKVAR: case O_MKDIM: {
        /* a fresh variable (of the type at port 1) or dimension, each time; 0 is its source name (a
         * word) or only the trigger */
        uint32_t nm = vk == V_WORD && !v->p ? (uint32_t)v->a + 1 : 0;
        for (int k = 1; k < v->n; k++) era(v->port[k]);
        if (o->kind == O_MKVAR) {
            static uint64_t next_var;
            /* its type is held once: every copy of the variable refers to it */
            Cell *n = cell(V_NVAR, 2); n->a = LVL_TAG | __atomic_fetch_add(&next_var, 1, __ATOMIC_RELAXED); n->ty = 1; n->x = nm;
            n->key = key0(K_VAR, n->a, NULL);   /* fresh for the whole run: its own key */
            link(ref_of(o->port[1]), P(n, 1)); link(P(n, 0), o->port[2]);
        } else {
            Dim d = fresh_dim();
            if (nm) dim_named(d, nm - 1);
            Cell *iv = cell(V_IV, 1); iv->p = iv_atom(d); link(P(iv, 0), o->port[1]);
        }
        cell_free(o); cell_free(v);
        return;
    }
    case O_ABS: {
        /* (%abs i v): the line binding the dimension i */
        Dnf *d = v->p;
        if (vk != V_IV || !d || d->n != 1 || d->c[0]->n != 1 || (d->c[0]->l[0] & 1)) { fprintf(stderr, "%%abs: not a dimension\n"); exit(2); }
        Cell *m = cell(O_MKPLAM, 2); m->a = d->c[0]->l[0] >> 1;
        mv(o, 1, P(m, 0)); link(P(m, 1), o->port[2]);
        cell_free(o); cell_free(v);
        return;
    }
    case O_GLOB: {
        if (vk != V_WORD || v->p) { fprintf(stderr, "%%gval/%%gty/%%dtab/%%dent: an index is a word\n"); exit(2); }
        o->a = v->a;
        long g = table_global((int)o->x, (long)o->a);
        if (g < 0) { fprintf(stderr, "checker table %u has no entry %llu\n", o->x, (unsigned long long)o->a); exit(2); }
        if (o->x == TB_VAL) {
            /* a checked definition's own value */
            Port val = global_copy(g, NULL);
            if (!val) { fprintf(stderr, "a checked definition without a value\n"); exit(2); }
            link(val, o->port[1]);
        } else { Cell *r = cell(V_GLOBAL, 1); r->a = (uint64_t)g; link(P(r, 0), o->port[1]); }
        cell_free(o); cell_free(v);
        return;
    }
    case O_NTYPE: {
        Port out = o->port[1], T;
        switch (vk) {
        case V_NVAR: T = (v->ty & TY_KNOWN) ? v->port[1] : (era(v->port[1]), unk()); break;
        case V_NPROJ: {
            /* the field's type at the parameters of the neutral's type, the earlier fields its own */
            era(v->port[2]);
            Port n1, n2; split2(v->port[1], &n1, &n2);
            Cell *da = cell(O_DAPPLY, 3); link(ntype_of(n1), P(da, 0));
            Cell *g = cell(V_GLOBAL, 1); g->a = (uint64_t)FLD[v->a].fty; g->key = key_global(FLD[v->a].fty);
            link(P(g, 0), P(da, 1));
            T = app2(P(da, 2), n2);
            break;
        }
        case V_NAPP: era(v->port[3]); T = app2(tpart(ntype_of(v->port[1]), 2), v->port[2]); break;
        case V_NFST: era(v->port[2]); T = tpart(ntype_of(v->port[1]), 1); break;
        case V_NSND: {
            era(v->port[2]);
            Port p1, p2; split2(v->port[1], &p1, &p2);
            Cell *f = cell(O_FST, 2); link(p2, P(f, 0));
            T = app2(tpart(ntype_of(p1), 2), P(f, 1));
            break;
        }
        case V_NPAPP:
            if (v->x) { era(v->port[2]); era(v->port[3]); }
            era(v->port[v->n - 1]);
            T = papp_at(tpart(ntype_of(v->port[1]), 1), dnf_retain(v->p));
            break;
        case V_NCOMP: era(v->port[2]); era(v->port[3]); era(v->port[4]); T = papp_at(v->port[1], dnf_one()); break;
        case V_NUNGLUE: era(v->port[1]); era(v->port[3]); T = tpart(ntype_of(v->port[2]), 1); break;
        case V_NCASE:
            for (int k = 2 + (v->x ? 1 : 0); k < v->n; k++) era(v->port[k]);
            T = v->x ? app2(v->port[2], v->port[1]) : (era(v->port[1]), unk());
            break;
        case V_NTRANS: {
            int np = v->n - 3;
            Cell *d = cell(V_DATA, np + 1); d->a = v->a;
            for (int j = 0; j < np; j++) link(papp_at(v->port[2 + j], dnf_one()), P(d, 1 + j));
            era(v->port[1]); era(v->port[v->n - 1]);
            T = P(d, 0);
            break;
        }
        default: for (int k = 1; k < v->n; k++) era(v->port[k]); T = unk(); break;
        }
        link(T, out);
        cell_free(o);
        cell_free(v);
        return;
    }
    case O_PEND: {
        /* 0=the type, 1=the neutral path, 2=out */
        if (vk == V_PATHP) {
            int z = dnf_is_zero(o->p);
            link(v->port[z ? 2 : 3], o->port[2]); era(v->port[1]); era(v->port[z ? 3 : 2]); era(o->port[1]);
            cell_free(o); cell_free(v);
            return;
        }
        for (int k = 1; k < v->n; k++) era(v->port[k]);
        Cell *n = cell(V_NPAPP, 3); n->p = dnf_retain(o->p);
        mv(o, 1, P(n, 1)); link(unk(), P(n, 2)); link(P(n, 0), o->port[2]);
        cell_free(o); cell_free(v);
        return;
    }
    case O_CLOS: {
        Cell *c = cell(O_CLOS2, 3);
        mv(o, 1, P(c, 0)); link(P(v, 0), P(c, 1)); mv(o, 2, P(c, 2));
        cell_free(o);
        return;
    }
    case O_CLOS2: {
        need_cons();
        static int C_SAME = -1, C_DIFF;
        if (C_SAME < 0) { C_DIFF = con_index("cdiff"); __atomic_store_n(&C_SAME, con_index("csame"), __ATOMIC_RELEASE); }
        if (C_SAME < 0 || C_DIFF < 0) { fprintf(stderr, "%%clos needs CloCmp (lib/code.cub)\n"); exit(2); }
        Cell *f = PC(o->port[1]);
        /* two λs of one record, or two eliminations of one table */
        bool fl = PI(o->port[1]) == 0 && (f->kind == V_LAM || f->kind == V_CASEF);
        if (fl && vk == f->kind && f->p == v->p && f->n == v->n && (vk == V_LAM || f->x == v->x)) {
            /* copies of the two environments, paired; the λs themselves are kept for applying
             * when the environments differ */
            int ne = v->n - 1;
            Port *ps = malloc((size_t)(ne + 1) * sizeof(Port));
            for (int k = 0; k < ne; k++) {
                Port a1, a2, b1, b2;
                split2(f->port[k + 1], &a1, &a2); split2(v->port[k + 1], &b1, &b2);
                link(a2, P(f, k + 1)); link(b2, P(v, k + 1));
                ps[k] = pair_of(a1, b1);
            }
            Port fs[3] = { list_of(ps, ne), P(f, 0), P(v, 0) };
            link(con_build(C_SAME, fs, NULL), o->port[2]);
            free(ps);
            cell_free(o);
            return;
        }
        Port fs[2] = { o->port[1], P(v, 0) };
        link(con_build(C_DIFF, fs, NULL), o->port[2]);
        cell_free(o);
        return;
    }
    case O_BRANCH: {
        /* the branch of an elimination at given fields, hypotheses and dimensions */
        if (vk != V_CASEF) { fprintf(stderr, "%%branch: not an elimination\n"); exit(2); }
        CaseTab *tb = v->p;
        int mb = v->x ? 1 : 0, ne = tb->nenv;
        Cell *c = cell(O_BRANCHC, ne + 3); c->p = tb; c->a = 0;
        mv(o, 1, P(c, 0)); mv(o, 2, P(c, 1));
        if (mb) era(v->port[1]);
        for (int e = 0; e < ne; e++) link(v->port[1 + mb + e], P(c, 2 + e));
        link(P(c, ne + 2), o->port[3]);
        cell_free(o); cell_free(v);
        return;
    }
    case O_BRANCHC: {
        if (vk != V_WORD || v->p) { fprintf(stderr, "%%branch: a constructor index is a word\n"); exit(2); }
        /* 0=rest of the list, environment, out */
        Cell *c = cell(O_BRANCHL, o->n - 1); c->p = o->p; c->a = v->a; c->x = 0;
        for (int k = 2; k < o->n; k++) mv(o, k, P(c, k - 1));
        mv(o, 1, P(c, 0));
        cell_free(o); cell_free(v);
        return;
    }
    case O_BRANCHL: {
        need_cons();
        CaseTab *tb = o->p;
        int ne = tb->nenv, np = (int)o->x;
        if (vk != V_CON) { fprintf(stderr, "%%branch: parameters are a list\n"); exit(2); }
        if ((int)v->a == C_LCONS) {
            Cell *c = cell(O_BRANCHL, o->n + 1); c->p = tb; c->a = o->a; c->x = (uint32_t)np + 1;
            for (int k = 1; k <= ne + np; k++) mv(o, k, P(c, k));
            link(v->port[1], P(c, ne + np + 1));
            mv(o, o->n - 1, P(c, c->n - 1));
            link(v->port[2], P(c, 0));
            cell_free(o); cell_free(v);
            return;
        }
        /* the parameters are fields, a hypothesis per recursive field, dimensions; a branch not
         * taking hypotheses is given the others */
        ConInfo *ci = &CON[DATA[tb->d].cons[o->a]];
        bool ih = ci->nrec && (tb->mask >> o->a & 1);
        Port env[ne + 1], prm[np + 1];
        int m = 0;
        for (int e = 0; e < ne; e++) env[e] = o->port[1 + e];
        for (int q = 0; q < np; q++) {
            Port x = o->port[1 + ne + q];
            if (!ih && q >= ci->nfields && q < ci->nfields + ci->nrec) era(x); else prm[m++] = x;
        }
        if (m != tb->br[o->a]->nparam) { fprintf(stderr, "%%branch: %d parameters for a branch of %d\n", m, tb->br[o->a]->nparam); exit(2); }
        link(record_build_n(tb->br[o->a], prm, env), o->port[o->n - 1]);
        cell_free(o); cell_free(v);
        return;
    }
    case O_IEQ: case O_FEQL: {
        if (vk != (o->kind == O_IEQ ? V_IV : V_FACE)) { fprintf(stderr, "%%ieq/%%feq: not an interval or face\n"); exit(2); }
        Cell *c = cell(o->kind == O_IEQ ? O_IEQB : O_FEQLB, 2); c->p = dnf_retain(v->p);
        mv(o, 1, P(c, 0)); mv(o, 2, P(c, 1));
        cell_free(o); cell_free(v);
        return;
    }
    case O_IEQB: case O_FEQLB: {
        need_cons();
        if (vk != (o->kind == O_IEQB ? V_IV : V_FACE)) { fprintf(stderr, "%%ieq/%%feq: not an interval or face\n"); exit(2); }
        link(con_build(dnf_equal(o->p, v->p) ? C_TRUE : C_FALSE, NULL, NULL), o->port[1]);
        cell_free(o); cell_free(v);
        return;
    }
    case O_CLAUSES: {
        need_cons();
        if (vk != V_FACE) { fprintf(stderr, "%%clauses: not a face\n"); exit(2); }
        Dnf *d = v->p; int n = d ? (int)d->n : 0;
        Port *cs = malloc((size_t)(n + 1) * sizeof(Port));
        for (int k = 0; k < n; k++) cs[k] = face_cell(face_of_clause(d->c[k]));
        link(list_of(cs, n), o->port[1]);
        free(cs);
        cell_free(o); cell_free(v);
        return;
    }
    case O_MOTIVE: {
        /* an elimination as a function (V_CASEF): its motive built over its environment, (some P),
         * or none when it was written without one */
        need_cons();
        if (vk != V_CASEF || v->x) { fprintf(stderr, "%%motive: not an elimination\n"); exit(2); }
        CaseTab *tb = v->p; int ne = v->n - 1;
        Port r;
        if (tb->motive) { Port me[ne + 1]; for (int e = 0; e < ne; e++) me[e] = v->port[1 + e]; r = mk(C_SOME, record_build_n(tb->motive, NULL, me), 0, 0, 0); }
        else { for (int e = 0; e < ne; e++) era(v->port[1 + e]); r = con_build(C_NONE, NULL, NULL); }
        link(r, o->port[1]);
        cell_free(o); cell_free(v);
        return;
    }
    case O_DNF: {
        /* an interval or a face as data: its clauses (a list), each a list of its literals
         * (pair atom b), atom the dimension as an interval; b: negated (an interval), or the
         * endpoint 1 (a face). No clauses: 0, ff; a clause without literals: 1, tt */
        need_cons(); need_bool();
        if (vk != V_IV && vk != V_FACE) { fprintf(stderr, "%%dnf: not an interval or a face\n"); exit(2); }
        Dnf *d = v->p; int n = d ? (int)d->n : 0;
        Port *cs = malloc((size_t)(n + 1) * sizeof(Port));
        for (int k = 0; k < n; k++) {
            Clause *cl = d->c[k]; int m = (int)cl->n;
            Port *ls = malloc((size_t)(m + 1) * sizeof(Port));
            for (int j = 0; j < m; j++) {
                Cell *a = cell(V_IV, 1); a->p = iv_atom((Dim)(cl->l[j] >> 1));
                ls[j] = pair_of(P(a, 0), con_build((cl->l[j] & 1) ? B_TRUE : B_FALSE, NULL, NULL));
            }
            cs[k] = list_of(ls, m);
            free(ls);
        }
        link(list_of(cs, n), o->port[1]);
        free(cs);
        cell_free(o); cell_free(v);
        return;
    }
    case O_RESTRICT: {
        if (vk != V_FACE || !v->p || ((Dnf *)v->p)->n != 1) { fprintf(stderr, "%%restrict: not a clause\n"); exit(2); }
        Subst *t = subst_of_clause(((Dnf *)v->p)->c[0]);
        link(sub(o->port[1], t), o->port[2]);
        subst_release(t);
        cell_free(o); cell_free(v);
        return;
    }
    case O_SYSMAP:
        if (vk == V_SYS) distribute(o, v);
        else { Cell *a = cell(O_APP, 3); mv(o, 1, P(a, 0)); link(P(v, 0), P(a, 1)); link(P(a, 2), o->port[2]); cell_free(o); }
        return;
    case O_PROJ: {
        /* a field: of a copattern definition's body, the field's term (held unbuilt); of a stuck
         * call of a copattern definition, the call unfolded and then the field; of any other
         * neutral, a neutral projection */
        Port out = o->port[1];
        int k = FLD[o->a].index;
        if (vk == V_REC && k + 1 < v->n) {
            for (int m = 1; m < v->n; m++) if (m != k + 1) era(v->port[m]);
            link(v->port[k + 1], out);
            cell_free(o); cell_free(v);
        } else if (vk == V_SYS) distribute(o, v);
        else if (is_neutral(vk) && (v->ty & TY_COFIX)) {
            Cell *u = cell(O_UNFOLD, 2);
            link(P(v, 0), P(u, 0)); link(P(u, 1), P(o, 0));
        } else if (is_neutral(vk)) {
            Cell *n = cell(V_NPROJ, 3); n->a = o->a;
            link(P(v, 0), P(n, 1)); link(unk(), P(n, 2)); link(P(n, 0), out);
            cell_free(o);
        } else stuck(o, v);
        return;
    }
    case O_UNFOLD: {
        /* a stuck call taken apart from its head: the body of the definition applied to the same
         * arguments, at the same dimensions. The application is named by the call it unfolds, so two
         * unfoldings of one call while it is pending share the body */
        Port out = o->port[1];
        if (vk == V_NAPP) {
            Cell *u = cell(O_UNFOLD, 2), *a = cell(O_APP, 3);
            link(v->port[1], P(u, 0)); link(P(u, 1), P(a, 0)); link(v->port[2], P(a, 1)); link(P(a, 2), out);
            era(v->port[3]);
            if (v->key) { a->key = key1(K_UNFOLD, 0, NULL, v->key); if (a->key) memo_register(a); }
            cell_free(o); cell_free(v);
        } else if (vk == V_NPAPP) {
            Cell *u = cell(O_UNFOLD, 2), *pr = cell(O_PAPPR, 3);
            link(v->port[1], P(u, 0));
            link(iv_cell(dnf_retain(v->p)), P(pr, 0)); link(P(u, 1), P(pr, 1)); link(P(pr, 2), out);
            if (v->x) { era(v->port[2]); era(v->port[3]); }
            era(v->port[v->n - 1]);
            cell_free(o); cell_free(v);
        } else if (vk == V_NVAR && (v->ty & TY_COFIX)) {
            long b = global_unfold((long)v->a);
            Cell *g = cell(V_GLOBAL, 1); g->a = (uint64_t)b; g->key = key_global(b);
            link(P(g, 0), out); era(v->port[1]);
            cell_free(o); cell_free(v);
        } else if (vk == V_SYS) distribute(o, v);
        else { link(P(v, 0), out); cell_free(o); }
        return;
    }
    case O_DAPPLY:
        if (vk == V_DATA) {
            Port f = o->port[1];
            for (int k = 1; k < v->n; k++) f = app2(f, v->port[k]);
            link(f, o->port[2]);
            cell_free(o); cell_free(v);
        } else if (vk == V_SYS) distribute(o, v);
        else stuck(o, v);
        return;
    default:
        stuck(o, v); return;
    }
}

/* ---------------- rewriting one active pair (HANDOFF §20) ----------------
 * Lock the pair and every cell whose port the rule may rewrite (the pair's
 * neighbours; for a definition, the cell holding its value and that value's
 * cell), in address order; revalidate that the neighbours are unchanged;
 * rewrite; release; then retire the consumed cells. */
static int cell_cmp(const void *x, const void *y) {
    uintptr_t a = (uintptr_t)*(Cell *const *)x, b = (uintptr_t)*(Cell *const *)y;
    return a < b ? -1 : a > b;
}
/* the cells to lock, sorted and distinct; when they do not fit in cap, minus the number needed (the
 * stored value read without locks may have been replaced by a larger one since cap was estimated) */
static int lockset(Cell *o, Cell *v, Cell **S, int cap) {
    int n = 0;
#define ADDC(c) do { Cell *c_ = (c); if (n < cap) S[n] = c_; n++; } while (0)
#define ADDPARTS(w) do { for (int k = 1; k < (w)->n; k++) { Port q = rd((w), k); if (q) ADDC(PC(q)); } } while (0)
    ADDC(o); ADDC(v);
    for (int k = 1; k < o->n; k++) { Port p = rd(o, k); if (p) ADDC(PC(p)); }
    for (int k = 1; k < v->n; k++) { Port p = rd(v, k); if (p) ADDC(PC(p)); }
    if (v->kind == V_GLOBAL) { Cell *r = global_res((long)v->a); ADDC(r); Port p = rd(r, 0); if (p) ADDC(PC(p)); }
    if (o->kind == O_WOP2 || o->kind == O_CLOS2) {
        /* the first operand's value, held at port 1: its parts are moved */
        Port p = rd(o, 1);
        if (p) { Cell *w = PC(p); ADDPARTS(w); }
    }
    if (v->kind == V_REF || v->kind == V_GLOBAL) {
        /* the stored value and its parts' neighbours: reading it may wrap a part in a box */
        Cell *r = v->kind == V_REF ? ((Box *)v->p)->root : global_res((long)v->a);
        if (v->kind == V_REF) ADDC(r);
        Port p = rd(r, 0);
        if (p) {
            Cell *w = PC(p);
            if (v->kind == V_REF) ADDC(w);
            ADDPARTS(w);
        }
    }
#undef ADDPARTS
#undef ADDC
    if (n > cap) return -n;
    qsort(S, (size_t)n, sizeof(Cell *), cell_cmp);
    int m = 0;
    for (int i = 0; i < n; i++) if (m == 0 || S[m - 1] != S[i]) S[m++] = S[i];
    return m;
}
static void rewrite(Redex r) {
    Cell *o = r.op, *v = r.v;
    int cap = o->n + v->n + 8;
    if (o->kind == O_WOP2 || o->kind == O_CLOS2) { Port p = rd(o, 1); if (p) cap += PC(p)->n; }
    if (v->kind == V_REF || v->kind == V_GLOBAL) {
        /* room for the stored value's parts (read again under the locks) */
        Cell *rt = v->kind == V_REF ? ((Box *)v->p)->root : global_res((long)v->a);
        Port p = rd(rt, 0);
        if (p) cap += PC(p)->n + 2;
    }
    Cell *buf1[64], *buf2[64];
    Cell **S = cap <= 64 ? buf1 : malloc((size_t)cap * sizeof(Cell *));
    Cell **T = cap <= 64 ? buf2 : malloc((size_t)cap * sizeof(Cell *));
    int n;
    for (;;) {
        n = lockset(o, v, S, cap);
        if (n < 0) {
            /* more cells than estimated: room for them, and the set read again */
            if (cap > 64) { free(S); free(T); }
            cap = -n;
            S = cap <= 64 ? buf1 : malloc((size_t)cap * sizeof(Cell *));
            T = cap <= 64 ? buf2 : malloc((size_t)cap * sizeof(Cell *));
            continue;
        }
        for (int i = 0; i < n; i++) lock(S[i]);
        int m = lockset(o, v, T, cap), j = 0;
        bool ok = m >= 0;
        for (int i = 0; i < m && ok; i++) { while (j < n && S[j] < T[i]) j++; if (j == n || S[j] != T[i]) ok = false; }
        if (ok) break;
        for (int i = n - 1; i >= 0; i--) unlock(S[i]);
    }
    interact(o, v);
    for (int i = n - 1; i >= 0; i--) unlock(S[i]);
    retire_boxes();
    flush_frees();
    publish();
    if (cap > 64) { free(S); free(T); }
    ADD(pending, -1);
}

/* ---------------- workers ---------------- */
static int nworkers;
static pthread_t *workers;
static pthread_mutex_t wmu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t wcv = PTHREAD_COND_INITIALIZER, pcv = PTHREAD_COND_INITIALIZER;
static int running, parked;
static void *worker(void *arg) {
    myq = (int)(intptr_t)arg;
    for (;;) {
        if (!__atomic_load_n(&running, __ATOMIC_ACQUIRE)) {
            pthread_mutex_lock(&wmu);
            parked++; pthread_cond_broadcast(&pcv);
            while (!__atomic_load_n(&running, __ATOMIC_ACQUIRE)) pthread_cond_wait(&wcv, &wmu);
            parked--;
            pthread_mutex_unlock(&wmu);
        }
        Redex r;
        if (pop(&r)) rewrite(r); else sched_yield();
    }
    return NULL;
}
void net_start(int n) {
    key_trace = getenv("CUBICAL_KEYS") != NULL;
    no_memo = getenv("CUBICAL_NOMEMO") != NULL;
    queues_init(n + 1);
    nworkers = n;
    workers = calloc((size_t)n + 1, sizeof(pthread_t));
    for (int i = 0; i < n; i++) pthread_create(&workers[i], NULL, worker, (void *)(intptr_t)(i + 1));
    pthread_mutex_lock(&wmu);
    while (parked < nworkers) pthread_cond_wait(&pcv, &wmu);
    pthread_mutex_unlock(&wmu);
}
static bool observed(Cell *root) {
    if (!root) return false;
    lock(root);
    Port q = root->port[0];
    bool ok = q && PI(q) == 0 && is_value(PC(q)->kind);
    unlock(root);
    return ok;
}
/* an observation: reduce, every worker taking pairs in arrival order, until
 * the observed port holds a value; the rest of the net stands */
bool net_observe(Cell *root, long limit) {
    retire_boxes(); flush_frees();
    publish();
    if (nworkers) { pthread_mutex_lock(&wmu); __atomic_store_n(&running, 1, __ATOMIC_RELEASE); pthread_cond_broadcast(&wcv); pthread_mutex_unlock(&wmu); }
    long n = 0;
    bool ok;
    for (;;) {
        if ((ok = observed(root))) break;
        if (__atomic_load_n(&pending, __ATOMIC_ACQUIRE) == 0 || (limit >= 0 && n >= limit)) break;
        Redex r;
        if (pop(&r)) { rewrite(r); n++; } else sched_yield();
    }
    if (nworkers) {
        pthread_mutex_lock(&wmu);
        __atomic_store_n(&running, 0, __ATOMIC_RELEASE);
        while (parked < nworkers) pthread_cond_wait(&pcv, &wmu);
        pthread_mutex_unlock(&wmu);
    }
    if (!ok && root && limit < 0 && key_trace) {
        /* the reading stopped short of a value: what the root waits on, operation by operation */
        Port q = root->port[0]; Cell *c = q ? PC(q) : NULL;
        fprintf(stderr, "stuck: root -> kind %d port %d\n", c ? c->kind : -1, q ? PI(q) : -1);
        for (int d = 0; d < 12 && c && PI(q) != 0; d++) {
            if (c->kind == O_FWD && c->key) { fprintf(stderr, "  a read waiting for the value of "); key_print(stderr, c->key); fputc('\n', stderr); }
            q = c->port[0]; c = q ? PC(q) : NULL;
            fprintf(stderr, "  waits on kind %d port %d", c ? c->kind : -1, q ? PI(q) : -1);
            if (c && c->key) { fputc(' ', stderr); key_print(stderr, c->key); }
            if (c && PI(q) == 0 && c->kind == V_REF) { Box *b = c->p; Port r = b->root->port[0]; fprintf(stderr, " -> the box of "); if (b->key) key_print(stderr, b->key); q = r; c = q ? PC(q) : NULL; }
            fputc('\n', stderr);
        }
    }
    return ok || observed(root);
}
/* reduce every pending pair: the workload's normal form, or what waits on an input */
void net_drain(void) { net_observe(NULL, -1); }

/* ---------------- building a net from syntax ---------------- */
typedef struct { Port *sup; int cur, n; Key *key; } Supply;
typedef struct { Supply *s; int nslots; const int *uses; Key *scrut; } Frame;   /* scrut: a case branch's scrutinee's key (the value it rebuilds is named by it) */

static void bind(Frame *f, int slot, Port prov) {
    int u = f->uses[slot];
    Supply *s = &f->s[slot];
    s->sup = malloc((size_t)(u + 1) * sizeof(Port)); s->n = u; s->cur = 0;
    s->key = key_of_port(prov);
    split(prov, u, s->sup);
}
static void bind_dim(Frame *f, int slot, Dim i) {
    int u = f->uses[slot];
    Supply *s = &f->s[slot];
    s->sup = malloc((size_t)(u + 1) * sizeof(Port)); s->n = u; s->cur = 0;
    Dnf *at = iv_atom(i); s->key = key0(K_IV, 0, at); dnf_release(at);
    for (int k = 0; k < u; k++) s->sup[k] = iv_cell(iv_atom(i));
}
/* the key of a compiled term in a frame, without building it: the term with its slots' keys.
 * NULL where a part has no key or the form is not keyed. The caller owns the result */
static Key *synkey(Syn *t, Frame *f) {
    Key *r = NULL, *x = NULL, *y = NULL, *z = NULL;
    switch (t->tag) {
    case S_VAR: { Key *k = f->s[t->slot].key; return k ? key_retain(k) : NULL; }
    case S_LAM: case S_THUNK: {
        /* a thunk is its body's term; a λ is its record over its environment */
        Key **env = malloc(((size_t)t->n + 1) * sizeof(Key *)); bool ok = true;
        for (int k = 0; k < t->n; k++) { env[k] = synkey(t->k[k], f); if (!env[k]) ok = false; }
        if (ok) {
            if (t->tag == S_LAM) r = key(K_LAM, 0, t->rec, t->n, env);
            else {
                Record *rc = t->rec;
                Frame g = { calloc((size_t)rc->nslots + 1, sizeof(Supply)), rc->nslots, rc->uses, NULL };
                for (int k = 0; k < rc->nenv; k++) g.s[rc->env_slot[k]].key = env[k];
                r = synkey(rc->body, &g);
                free(g.s);
            }
        }
        for (int k = 0; k < t->n; k++) if (env[k]) key_release(env[k]);
        free(env);
        return r;
    }
    case S_APP: x = synkey(t->k[0], f); y = x ? synkey(t->k[1], f) : NULL; r = key2(K_APP, 0, NULL, x, y); break;
    case S_CALL: {
        r = key0(K_GLOBAL, (uint64_t)t->id, NULL);
        for (int k = 0; k < t->n && r; k++) { Key *a = synkey(t->k[k], f); Key *n = key2(K_APP, 0, NULL, r, a); key_release(r); if (a) key_release(a); r = n; }
        return r;
    }
    case S_PAIR: case S_PI: case S_SIGMA: case S_PATHP: case S_DATA: case S_CON: {
        if (t->tag == S_CON && t->x) return f->scrut ? key_retain(f->scrut) : NULL;   /* the scrutinee rebuilt: its key */
        int tag = t->tag == S_PAIR ? K_PAIR : t->tag == S_PI ? K_PI : t->tag == S_SIGMA ? K_SIGMA : t->tag == S_PATHP ? K_PATHP : t->tag == S_DATA ? K_DATA : K_CON;
        Key **c = malloc(((size_t)t->n + 1) * sizeof(Key *)); bool ok = true;
        for (int k = 0; k < t->n; k++) { c[k] = synkey(t->k[k], f); if (!c[k]) ok = false; }
        if (ok) r = key(tag, (uint64_t)t->id, NULL, t->n, c);
        for (int k = 0; k < t->n; k++) if (c[k]) key_release(c[k]);
        free(c);
        return r;
    }
    case S_FST: x = synkey(t->k[0], f); r = key1(K_FST, 0, NULL, x); break;
    case S_SND: x = synkey(t->k[0], f); r = key1(K_SND, 0, NULL, x); break;
    case S_FORCE: return synkey(t->k[0], f);   /* forcing a suspension is the suspended term */
    case S_PAPP: if (t->x) return NULL; x = synkey(t->k[0], f); y = x ? synkey(t->k[1], f) : NULL; r = key2(K_PAPP, 0, NULL, x, y); break;
    case S_U: { if (t->n) { x = synkey(t->k[0], f); r = key1(K_U, 0, NULL, x); } else r = key0(K_U, 0, NULL); break; }
    case S_WORD: { Big *b = t->big; r = key0(K_WORD, (uint64_t)t->id, b); return r; }
    case S_WORDTY: return key0(K_WTY, 0, NULL);
    case S_IV0: { Dnf *d = dnf_zero(); r = key0(K_IV, 0, d); dnf_release(d); return r; }
    case S_IV1: { Dnf *d = dnf_one(); r = key0(K_IV, 0, d); dnf_release(d); return r; }
    case S_FTT: { Dnf *d = dnf_one(); r = key0(K_FACE, 0, d); dnf_release(d); return r; }
    case S_FFF: { Dnf *d = dnf_zero(); r = key0(K_FACE, 0, d); dnf_release(d); return r; }
    case S_GLOBAL: return key0(K_GLOBAL, (uint64_t)t->id, NULL);
    case S_AXIOM: return key0(K_VAR, (uint64_t)t->id, NULL);
    case S_CASE: {
        /* the table, the scrutinee and the shared variables */
        int ne = t->tab->nenv;
        Key **c = malloc(((size_t)ne + 2) * sizeof(Key *)); bool ok = true;
        c[0] = synkey(t->k[0], f); if (!c[0]) ok = false;
        for (int e = 0; e < ne; e++) { c[1 + e] = synkey(t->k[2 + e], f); if (!c[1 + e]) ok = false; }
        if (ok) r = key(K_CASE, 0, t->tab, ne + 1, c);
        for (int e = 0; e < ne + 1; e++) if (c[e]) key_release(c[e]);
        free(c);
        return r;
    }
    case S_LET: {
        Key *saved = f->s[t->slot].key;
        f->s[t->slot].key = synkey(t->k[0], f);
        r = synkey(t->k[1], f);
        if (f->s[t->slot].key) key_release(f->s[t->slot].key);
        f->s[t->slot].key = saved;
        return r;
    }
    default: return NULL;
    }
    if (x) key_release(x);
    if (y) key_release(y);
    if (z) key_release(z);
    return r;
}
static Port build(Syn *t, Frame *f);
static Port op1(int kind, Syn *a, Frame *f) { Cell *c = cell(kind, 2); link(build(a, f), P(c, 0)); return P(c, 1); }

static Port build(Syn *t, Frame *f) {
    Cell *c;
    switch (t->tag) {
    case S_VAR: {
        Supply *s = &f->s[t->slot];
        if (s->cur >= s->n) { fprintf(stderr, "internal: slot %d used beyond its count\n", t->slot); exit(2); }
        return s->sup[s->cur++];
    }
    case S_LAM: case S_THUNK:
        c = cell(t->tag == S_LAM ? V_LAM : V_THUNK, t->n + 1); c->p = t->rec;
        for (int k = 0; k < t->n; k++) link(build(t->k[k], f), P(c, k + 1));
        c->key = synkey(t, f);
        if (t->tag == S_THUNK && c->key) memo_register(c);
        return P(c, 0);
    case S_APP:
        c = cell(O_APP, 3); link(build(t->k[0], f), P(c, 0)); link(build(t->k[1], f), P(c, 1));
        c->key = synkey(t, f); if (c->key) memo_register(c);
        return P(c, 2);
    case S_PAIR: c = cell(V_PAIR, 3); goto fields;
    case S_PI: c = cell(V_PI, 3); goto fields;
    case S_SIGMA: c = cell(V_SIGMA, 3); goto fields;
    case S_PATHP: c = cell(V_PATHP, 4); goto fields;
    fields:
        for (int k = 0; k < c->n - 1; k++) link(build(t->k[k], f), P(c, k + 1));
        c->key = synkey(t, f);
        return P(c, 0);
    case S_FST: { Port r = op1(O_FST, t->k[0], f); PC(r)->key = synkey(t, f); return r; }
    case S_SND: { Port r = op1(O_SND, t->k[0], f); PC(r)->key = synkey(t, f); return r; }
    case S_U: {
        /* U at its level: lzero when none is written */
        c = cell(V_U, 2);
        if (t->n) link(build(t->k[0], f), P(c, 1));
        else { int z = con_index("lzero"); if (z < 0) { fprintf(stderr, "U needs Level (lib/prelude.cub)\n"); exit(2); } link(con_build(z, NULL, NULL), P(c, 1)); }
        return P(c, 0);
    }
    case S_BOX: return ref_of(build(t->k[0], f));
    case S_WORD: c = cell(V_WORD, 1); c->a = (uint64_t)t->id; c->p = big_retain(t->big); c->key = synkey(t, f); return P(c, 0);
    case S_WORDTY: c = cell(V_WTY, 1); c->key = synkey(t, f); return P(c, 0);
    case S_CALL: {
        c = cell(O_CALL, t->n + 2);
        for (int k = 0; k < t->n; k++) link(build(t->k[k], f), P(c, 1 + k));
        Cell *gv = cell(V_GLOBAL, 1); gv->a = (uint64_t)t->id; gv->key = key_global(t->id);
        link(P(gv, 0), P(c, 0));
        c->key = synkey(t, f); if (c->key) memo_register(c);
        return P(c, t->n + 1);
    }
    case S_GLOBTAB:
        c = cell(O_GLOB, 2); c->x = t->x;
        link(build(t->k[0], f), P(c, 0));
        return P(c, 1);
    case S_PRIM:
        c = cell((int)t->x, t->n + 1);
        for (int k = 0; k < t->n; k++) link(build(t->k[k], f), P(c, k));
        return P(c, t->n);
    case S_PLAM: {
        Dim i = fresh_dim();
        bind_dim(f, t->slot, i);
        Cell *m = cell(O_MKPLAM, 2); m->a = i;
        link(build(t->k[0], f), P(m, 0));
        return P(m, 1);
    }
    case S_FORALL: {
        Dim i = fresh_dim();
        bind_dim(f, t->slot, i);
        Cell *m = cell(O_FORALL, 2); m->a = i;
        link(build(t->k[0], f), P(m, 0));
        return P(m, 1);
    }
    case S_PAPP:
        c = cell(O_PAPPR, t->x ? 5 : 3); c->x = t->x;
        link(build(t->k[1], f), P(c, 0)); link(build(t->k[0], f), P(c, 1));
        if (t->x) { link(build(t->k[2], f), P(c, 2)); link(build(t->k[3], f), P(c, 3)); }
        return P(c, c->n - 1);
    case S_IV0: { Port r = iv_cell(dnf_zero()); PC(r)->key = synkey(t, f); return r; }
    case S_IV1: { Port r = iv_cell(dnf_one()); PC(r)->key = synkey(t, f); return r; }
    case S_NEG: return op1(O_NEG, t->k[0], f);
    case S_BIN: c = cell(O_BIN, 3); c->x = t->x; link(build(t->k[0], f), P(c, 0)); link(build(t->k[1], f), P(c, 1)); return P(c, 2);
    case S_FEQ: c = cell(O_FEQ, 2); c->a = t->x; link(build(t->k[0], f), P(c, 0)); return P(c, 1);
    case S_FTT: c = cell(V_FACE, 1); c->p = dnf_one(); c->key = synkey(t, f); return P(c, 0);
    case S_FFF: c = cell(V_FACE, 1); c->p = dnf_zero(); c->key = synkey(t, f); return P(c, 0);
    case S_FACEOF: return op1(O_FACEOF, t->k[0], f);
    case S_SYS0: c = cell(V_SYS, 1); c->p = NULL; return P(c, 0);
    case S_SYSCONS:
        c = cell(O_SYSCONS, 4);
        link(build(t->k[0], f), P(c, 0)); link(build(t->k[1], f), P(c, 1)); link(build(t->k[2], f), P(c, 2));
        return P(c, 3);
    case S_SYSMAP: c = cell(O_SYSMAP, 3); link(build(t->k[1], f), P(c, 0)); link(build(t->k[0], f), P(c, 1)); return P(c, 2);
    case S_COMP:
        c = cell(O_COMP, 4);
        link(build(t->k[1], f), P(c, 0)); link(build(t->k[0], f), P(c, 1)); link(build(t->k[2], f), P(c, 2));
        return P(c, 3);
    case S_GLUE: c = cell(O_MKGLUE, 3); link(build(t->k[1], f), P(c, 0)); link(build(t->k[0], f), P(c, 1)); return P(c, 2);
    case S_GLUEV: c = cell(O_MKGLUEV, 3); link(build(t->k[0], f), P(c, 0)); link(build(t->k[1], f), P(c, 1)); return P(c, 2);
    case S_UNGLUE: c = cell(O_UNGLUE, 3); link(build(t->k[0], f), P(c, 0)); link(build(t->k[1], f), P(c, 1)); return P(c, 2);
    case S_GLOBAL: c = cell(V_GLOBAL, 1); c->a = (uint64_t)t->id; c->key = key_global(t->id); return P(c, 0);
    case S_AXIOM:
        c = cell(V_NVAR, 2); c->a = (uint64_t)t->id; c->key = synkey(t, f);
        if (t->slot >= 0) { Cell *g = cell(V_GLOBAL, 1); g->a = (uint64_t)t->slot; link(P(g, 0), P(c, 1)); c->ty = 1; }
        else link(unk(), P(c, 1));
        return P(c, 0);
    case S_FORCE: { Port r = op1(O_FORCE, t->k[0], f); PC(r)->key = synkey(t, f); return r; }
    case S_DATA: c = cell(V_DATA, t->n + 1); c->a = (uint64_t)t->id; goto fields;
    case S_CON: {
        Port *a = malloc((size_t)(t->n + 1) * sizeof(Port));
        for (int k = 0; k < t->n; k++) a[k] = build(t->k[k], f);
        Port r = con_build((int)t->id, a, a + CON[t->id].nfields);
        free(a);
        if (PI(r) == 0) PC(r)->key = synkey(t, f);   /* a plain constructor (one with a boundary is checked first) */
        return r;
    }
    case S_PROJ: c = cell(O_PROJ, 2); c->a = (uint64_t)t->id; link(build(t->k[0], f), P(c, 0)); return P(c, 1);
    case S_REC:
        c = cell(V_REC, t->n + 1); c->a = (uint64_t)t->id;
        for (int k = 0; k < t->n; k++) link(build(t->k[k], f), P(c, k + 1));
        return P(c, 0);
    case S_HCOMPD: c = cell(O_HCOMP, 3); link(build(t->k[0], f), P(c, 0)); link(build(t->k[1], f), P(c, 1)); return P(c, 2);
    case S_TRANSD:
        c = cell(O_TRANSD1, t->n + 1); c->a = (uint64_t)t->id;
        for (int k = 0; k < t->n; k++) link(build(t->k[k], f), P(c, k));
        return P(c, t->n);
    case S_CASE: {
        /* k[0]=scrutinee k[1]=motive (or none) k[2..]=the shared variables */
        int ne = t->tab->nenv;
        c = cell(O_CASE, ne + 2); c->x = 0; c->p = t->tab;
        link(build(t->k[0], f), P(c, 0));
        for (int e = 0; e < ne; e++) link(build(t->k[2 + e], f), P(c, 1 + e));
        c->key = synkey(t, f); if (c->key) memo_register(c);
        return P(c, c->n - 1);
    }
    case S_LET: {
        bind(f, t->slot, build(t->k[0], f));
        return build(t->k[1], f);
    }
    }
    fprintf(stderr, "build: tag %d\n", t->tag); exit(2);
}

static Port run_frame(Syn *body, int nslots, const int *uses, int nparam, const Port *params, int nenv, const int *env_slot, const Port *env, int where) {
    Frame f = { calloc((size_t)nslots + 1, sizeof(Supply)), nslots, uses, scrut_key };
    scrut_key = NULL;
    long saved = dup_origin;
    if (prof_on) dup_origin = where + 1;
    for (int k = 0; k < nparam; k++) bind(&f, k, params[k]);
    for (int k = 0; k < nenv; k++) bind(&f, env_slot[k], env[k]);
    Port r = build(body, &f);
    for (int k = 0; k < nslots; k++) {
        if (f.s[k].cur != f.s[k].n) { fprintf(stderr, "internal: slot %d built %d of %d uses\n", k, f.s[k].cur, f.s[k].n); exit(2); }
        free(f.s[k].sup);
        if (f.s[k].key) key_release(f.s[k].key);
    }
    if (f.scrut) key_release(f.scrut);
    free(f.s);
    dup_origin = saved;
    return r;
}
Port record_build(const Record *r, Port param, const Port *env) {
    return run_frame(r->body, r->nslots, r->uses, r->nparam, &param, r->nenv, r->env_slot, env, r->where);
}
Port record_build_n(const Record *r, const Port *params, const Port *env) {
    return run_frame(r->body, r->nslots, r->uses, r->nparam, params, r->nenv, r->env_slot, env, r->where);
}
Port top_build(Syn *body, int nslots, const int *uses) {
    return run_frame(body, nslots, uses, 0, NULL, 0, NULL, NULL, -1);
}
