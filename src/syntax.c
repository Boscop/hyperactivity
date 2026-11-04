/* syntax.c — the surface language: tokens, layout, parsing into the core forms, and printing the
 * core forms back in the surface language. Included by lang.c (it uses its Node and tables).
 *
 * The surface language is mathematical notation: application by juxtaposition, declared infix
 * operators, indentation for structure, λ x → e, (x : A) → B, (x : A) × B, a ≡ b, definitions by
 * equations. Every Unicode symbol has an ASCII spelling (→ ->, λ \, ↦ |->, ≡ ==, ∧ /\, ∨ \/, × **,
 * ℕ Nat, S¹ S1, ⊥ Empty, ⊤ Unit); the lexer reads both as one token. */

/* ---------------- tokens ---------------- */
enum { X_ID, X_NUM, X_STR, X_LP, X_RP, X_LB, X_RB, X_LC, X_RC, X_COMMA, X_SEMI, X_DOT, X_VO, X_VS, X_VC, X_EOF };
typedef struct { int k; char *s; int line, col; } Tok;
static Tok *xt; static int nxt, capxt;
static const char *x_file;
static void x_push(int k, const char *s, size_t n, int line, int col) {
    if (nxt == capxt) { capxt = capxt ? capxt * 2 : 1024; xt = realloc(xt, (size_t)capxt * sizeof(Tok)); }
    char *b = malloc(n + 1); memcpy(b, s, n); b[n] = 0;
    xt[nxt++] = (Tok){ k, b, line, col };
}
/* the one spelling of each symbol */
static const struct { const char *a, *u; } X_ALIAS[] = {
    { "->", "→" }, { "\\", "λ" }, { "|->", "↦" }, { "==", "≡" }, { "∧", "/\\" }, { "∨", "\\/" }, { "**", "×" },
    { "ℕ", "Nat" }, { "S¹", "S1" }, { "⊥", "Empty" }, { "⊤", "Unit" }, { "=/=", "≢" }, { "forall", "∀" },
    { NULL, NULL } };
static const char *x_canon(const char *s) {
    for (int i = 0; X_ALIAS[i].a; i++) if (!strcmp(s, X_ALIAS[i].a)) return X_ALIAS[i].u;
    return s;
}
static bool x_delim(int c) { return c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == ',' || c == ';' || c == '"'; }
static bool x_identstart(unsigned char c) { return isalpha(c) || c == '_' || c >= 0x80 || c == '%'; }
static void x_die(int line, int col, const char *m, const char *x) {
    fprintf(stderr, "%s:%d:%d: %s%s%s\n", x_file ? x_file : "?", line, col, m, x ? " " : "", x ? x : "");
    exit(1);
}
/* a word (a maximal run of non-blank, non-delimiter characters) as tokens: projections .f split off,
 * a leading ~ or λ split off */
static void x_word(const char *b, size_t n, int line, int col) {
    if (n > 1 && b[0] == '~' && b[1] != '~') { x_push(X_ID, "~", 1, line, col); x_word(b + 1, n - 1, line, col + 1); return; }
    if (n > 2 && (unsigned char)b[0] == 0xCE && (unsigned char)b[1] == 0xBB) { x_push(X_ID, "λ", 2, line, col); x_word(b + 2, n - 2, line, col + 1); return; }
    if (n > 1 && b[0] == '\\' && isalpha((unsigned char)b[1])) { x_push(X_ID, "λ", 2, line, col); x_word(b + 1, n - 1, line, col + 1); return; }
    if (n > 1 && b[0] == '.' && x_identstart((unsigned char)b[1])) {
        size_t e = 1; while (e < n && b[e] != '.') e++;
        x_push(X_DOT, b + 1, e - 1, line, col);
        if (e < n) x_word(b + e, n - e, line, col + (int)e);
        return;
    }
    bool num = true; for (size_t i = 0; i < n; i++) if (!isdigit((unsigned char)b[i])) num = false;
    if (num) { x_push(X_NUM, b, n, line, col); return; }
    /* s.f: a projection of an identifier */
    for (size_t i = 1; i + 1 < n; i++)
        if (b[i] == '.' && b[i - 1] != '.' && x_identstart((unsigned char)b[i + 1]) && b[0] != '#') {
            x_word(b, i, line, col); x_word(b + i, n - i, line, col + (int)i); return;
        }
    char *w = malloc(n + 1); memcpy(w, b, n); w[n] = 0;
    const char *c = x_canon(w);
    x_push(X_ID, c, strlen(c), line, col);
    free(w);
}
static void x_lex(const char *text) {
    nxt = 0;
    const char *p = text; int line = 1; const char *ls = text;
#define COL ((int)(p - ls) + 1)
    while (*p) {
        if (*p == '\n') { p++; line++; ls = p; continue; }
        if (isspace((unsigned char)*p)) { p++; continue; }
        if (p[0] == '-' && p[1] == '-') { while (*p && *p != '\n') p++; continue; }
        if (p[0] == '{' && p[1] == '-') {
            int depth = 0;
            while (*p) {
                if (p[0] == '{' && p[1] == '-') { depth++; p += 2; continue; }
                if (p[0] == '-' && p[1] == '}') { depth--; p += 2; if (!depth) break; continue; }
                if (*p == '\n') { line++; ls = p + 1; }
                p++;
            }
            continue;
        }
        int col = COL;
        if (*p == '"') {
            const char *b = ++p; while (*p && *p != '"' && *p != '\n') p++;
            if (*p != '"') x_die(line, col, "unclosed string", NULL);
            x_push(X_STR, b, (size_t)(p - b), line, col); p++; continue;
        }
        if (x_delim(*p)) {
            static const char D[] = "()[]{},;"; static const int K[] = { X_LP, X_RP, X_LB, X_RB, X_LC, X_RC, X_COMMA, X_SEMI };
            x_push(K[strchr(D, *p) - D], p, 1, line, col); p++; continue;
        }
        const char *b = p;
        while (*p && !isspace((unsigned char)*p) && !x_delim(*p)) p++;
        x_word(b, (size_t)(p - b), line, col);
    }
    x_push(X_EOF, "<end of file>", 13, line, COL);
#undef COL
}
/* ---------------- layout ----------------
 * A block opens after where, of, let, mutual, postulate (and the file is one): its column is that of
 * its first token. A line starting at the block's column starts a new item (X_VS); one starting left
 * of it closes the block (X_VC); one starting right of it continues the item. `in` closes a let's
 * block; a closing bracket closes the blocks opened inside it. */
static Tok *lt; static int nlt, caplt;
static void l_push(Tok t) { if (nlt == caplt) { caplt = caplt ? caplt * 2 : 1024; lt = realloc(lt, (size_t)caplt * sizeof(Tok)); } lt[nlt++] = t; }
static bool x_is(Tok *t, const char *s) { return t->k == X_ID && !strcmp(t->s, s); }
static bool x_opens(Tok *t) { return x_is(t, "where") || x_is(t, "of") || x_is(t, "let") || x_is(t, "mutual") || x_is(t, "postulate"); }
static void x_layout(void) {
    nlt = 0;
    /* the open blocks: column, bracket depth where opened, whether a let's; the lets closed by
     * indentation that still await their `in` (by bracket depth) */
    int stk[256], depthat[256], islet[256], ns = 0, depth = 0;
    int await[256], na = 0;
    int pending = 0, pending_let = 0, prevline = 0;
    for (int i = 0; i < nxt; i++) {
        Tok t = xt[i];
        Tok v = t; v.s = "";
#define CLOSE() do { v.k = X_VC; l_push(v); if (islet[ns - 1]) { if (na == 256) x_die(t.line, t.col, "too deep", NULL); await[na++] = depthat[ns - 1]; } ns--; } while (0)
        if (t.k == X_EOF) { while (ns > 0) { v.k = X_VC; l_push(v); ns--; } l_push(t); break; }
        if (i == 0 || pending) {
            if (ns == 256) x_die(t.line, t.col, "blocks nested too deeply", NULL);
            if (pending && ns > 0 && t.col <= stk[ns - 1] && t.line != prevline) {
                /* an empty block */
                v.k = X_VO; l_push(v); v.k = X_VC; l_push(v); pending = 0;
            } else {
                stk[ns] = t.col; depthat[ns] = depth; islet[ns] = pending_let; ns++;
                if (i) { v.k = X_VO; l_push(v); }
                pending = 0; prevline = t.line;
                goto emit;
            }
        }
        if (t.line != prevline) {
            while (ns > 1 && t.col < stk[ns - 1] && depthat[ns - 1] == depth) CLOSE();
            if (ns > 0 && t.col == stk[ns - 1] && depthat[ns - 1] == depth) { v.k = X_VS; l_push(v); }
            prevline = t.line;
        }
        if (x_is(&t, "in")) {
            if (na > 0 && await[na - 1] == depth) na--;
            else {
                bool found = false;
                for (int q = ns - 1; q >= 1 && depthat[q] == depth; q--) if (islet[q]) { found = true; break; }
                if (!found) x_die(t.line, t.col, "`in` without its let", NULL);
                while (!islet[ns - 1]) CLOSE();
                v.k = X_VC; l_push(v); ns--;
            }
        }
        if (t.k == X_RP || t.k == X_RB || t.k == X_RC) {
            while (ns > 1 && depthat[ns - 1] >= depth) CLOSE();
            while (na > 0 && await[na - 1] >= depth) na--;
            depth--;
        }
    emit:
        l_push(t);
        if (t.k == X_LP || t.k == X_LB || t.k == X_LC) depth++;
        if (x_opens(&t)) { pending = 1; pending_let = x_is(&t, "let"); }
#undef CLOSE
    }
}

/* ---------------- operators ---------------- */
typedef struct { char *op; int prec, assoc; } Fix;   /* assoc: 0 none, 1 left, 2 right */
static Fix *FIX; static int nfix, capfix;
static void fix_set(const char *op, int prec, int assoc) {
    for (int i = 0; i < nfix; i++) if (!strcmp(FIX[i].op, op)) { FIX[i].prec = prec; FIX[i].assoc = assoc; return; }
    GROW(FIX, capfix, (size_t)nfix + 1); FIX[nfix++] = (Fix){ strdup(op), prec, assoc };
}
/* the builtin operators: → (0, right), × (2, right), ≡ ≡[] (4), = (5, faces), \/ (6), /\ (7), @ (30, left) */
static Fix *fix_of(Tok *t) {
    static Fix B[] = { { "→", 0, 2 }, { "×", 2, 2 }, { "≡", 4, 0 }, { "\\/", 6, 1 }, { "/\\", 7, 1 }, { "@", 30, 1 } };
    if (t->k != X_ID) return NULL;
    for (size_t i = 0; i < sizeof B / sizeof *B; i++) if (!strcmp(B[i].op, t->s)) return &B[i];
    for (int i = nfix - 1; i >= 0; i--) if (!strcmp(FIX[i].op, t->s)) return &FIX[i];
    return NULL;
}
/* an operator's name as a definition: _+_ */
static char *op_name(const char *op) { char *r = malloc(strlen(op) + 3); sprintf(r, "_%s_", op); return r; }

/* ---------------- the parser ---------------- */
static int xp;                    /* position in lt */
static bool x_typed;              /* the checker is loaded: data fields are types, signatures check */
static bool x_untyped_file;       /* converting a file that was loaded before the checker */
static Tok *P0(void) { return &lt[xp]; }
static Tok *P1(void) { return &lt[xp + 1 < nlt ? xp + 1 : nlt - 1]; }
static Tok *NX(void) { Tok *t = &lt[xp]; if (t->k != X_EOF) xp++; return t; }
static void x_err(const char *m, Tok *t) { x_die(t->line, t->col, m, t->k == X_VS ? "(at a new line)" : t->k == X_VC ? "(at the end of a block)" : t->k == X_VO ? "(at a new block)" : t->s); }
static bool at(const char *s) { return x_is(P0(), s); }
static void want(const char *s) { if (!at(s)) { char m[64]; snprintf(m, sizeof m, "expected %s, found", s); x_err(m, P0()); } NX(); }
static void wantk(int k, const char *what) { if (P0()->k != k) { char m[64]; snprintf(m, sizeof m, "expected %s, found", what); x_err(m, P0()); } NX(); }
static const char *RESERVED[] = { "=", ":", "→", "λ", "↦", "|", "of", "where", "let", "in", "if", "then", "else", "case", "return",
    "data", "codata", "record", "mutual", "postulate", "import", "infix", "infixl", "infixr", "by", "calc", "∀", "decreasing", NULL };
static bool reserved(const char *s) { for (int i = 0; RESERVED[i]; i++) if (!strcmp(RESERVED[i], s)) return true; return false; }
static int nfresh_x;
static Node *xfresh(const char *p) { char b[32]; snprintf(b, sizeof b, "%%%s%d", p, nfresh_x++); return nsym(b); }
static Node *lst(int n, ...) {
    va_list ap; va_start(ap, n);
    Node *x = nlist(n);
    for (int i = 0; i < n; i++) x->it[i] = va_arg(ap, Node *);
    va_end(ap);
    return x;
}
static Node *sym2(const char *h, Node *a) { return lst(2, nsym(h), a); }
static Node *sym3(const char *h, Node *a, Node *b) { return lst(3, nsym(h), a, b); }
static Node *sym4(const char *h, Node *a, Node *b, Node *c) { return lst(4, nsym(h), a, b, c); }

/* the primitives written as prefix functions: their number of operands (min, max) */
static const struct { const char *n; int lo, hi; } PRIM[] = {
    { "fst", 1, 1 }, { "snd", 1, 1 }, { "force", 1, 1 }, { "thunk", 1, 1 }, { "delay", 1, 1 }, { "rec", 1, 1 }, { "the", 2, 2 },
    { "code", 1, 1 }, { "PathP", 3, 3 }, { "faceof", 1, 1 }, { "sysmap", 2, 2 }, { "sys+", 3, 3 }, { "comp", 3, 3 }, { "Glue", 2, 2 },
    { "glue", 2, 2 }, { "unglue", 2, 2 }, { "hcompD", 2, 2 }, { "%view", 1, 1 }, { "%ieq", 2, 2 }, { "%feq", 2, 2 }, { "%clauses", 1, 1 },
    { "%dnf", 1, 1 }, { "%motive", 1, 1 }, { "%restrict", 2, 2 }, { "%abs", 2, 2 }, { "%branch", 3, 3 }, { "%clos", 2, 2 },
    { "%wsucc", 1, 1 }, { "%weq", 2, 2 }, { "%wlt", 2, 2 }, { "%wadd", 2, 2 }, { "%wmax", 2, 2 }, { "%wsub", 2, 2 }, { "%welim", 3, 3 },
    { "%welimP", 4, 4 }, { "%box", 1, 1 }, { "%gval", 1, 1 }, { "%gty", 1, 1 }, { "%dtab", 1, 1 }, { "%dent", 1, 1 }, { "%dtrans", 1, 1 },
    { "%var", 1, 2 }, { "%dim", 0, 1 }, { "%@", 4, 4 }, { "%fand", 2, 2 }, { "%for", 2, 2 }, { "pair", 2, 2 }, { NULL, 0, 0 } };
static int prim_ix(const char *s) { for (int i = 0; PRIM[i].n; i++) if (!strcmp(PRIM[i].n, s)) return i; return -1; }

static Node *p_expr(int minprec);
static Node *p_face(void);
static bool atom_start(Tok *t) {
    switch (t->k) {
    case X_ID:
        if (x_is(t, "~") || x_is(t, "λ") || x_is(t, "let") || x_is(t, "if") || x_is(t, "case") || x_is(t, "∀")) return true;
        return !reserved(t->s) && !fix_of(t);
    case X_NUM: case X_STR: case X_LP: case X_LB: case X_LC: return true;
    default: return false;
    }
}
/* the binders of λ: x, _, (x y : A), (i : I), {x} */
typedef struct { Node *name; Node *ty; bool dim, imp; } Bnd;
static int p_binders(Bnd *bs, int max, bool need_type) {
    int n = 0;
    for (;;) {
        Tok *t = P0();
        if (t->k == X_ID && !reserved(t->s) && !fix_of(t)) { if (n == max) x_err("too many binders", t); NX(); bs[n++] = (Bnd){ nsym(t->s), NULL, false, false }; continue; }
        if (t->k == X_LP || t->k == X_LC) {
            /* (x y : A) */
            int save = xp; bool imp = t->k == X_LC; NX();
            int k0 = n;
            while (P0()->k == X_ID && !reserved(P0()->s) && !fix_of(P0())) { if (n == max) x_err("too many binders", P0()); bs[n++] = (Bnd){ nsym(NX()->s), NULL, false, imp }; }
            if (n > k0 && at(":")) {
                NX(); Node *A = p_expr(0);
                wantk(imp ? X_RC : X_RP, imp ? "}" : ")");
                for (int q = k0; q < n; q++) { bs[q].ty = A; bs[q].dim = is(A, "I"); }
                continue;
            }
            if (n > k0 && imp && P0()->k == X_RC) { NX(); continue; }
            n = k0; xp = save;
        }
        break;
    }
    if (need_type) for (int i = 0; i < n; i++) if (!bs[i].ty) x_err("a binder needs its type here:", P0());
    return n;
}
static Node *lam_of(Bnd *bs, int n, Node *body) {
    for (int i = n - 1; i >= 0; i--) body = sym3(bs[i].dim ? "plam" : "lam", bs[i].name, body);
    return body;
}
/* faces: φ ∨ ψ, φ ∧ ψ, r = 0, r = 1, (φ), ∀ i → φ, tt, ff */
static Node *p_iexpr(void);
static bool group_has_eq(void);
static Node *p_fatom(void) {
    if (P0()->k == X_LP && group_has_eq()) {
        int save = xp; NX();
        Node *f = p_face();
        if (P0()->k == X_RP && !x_is(P1(), "=")) { NX(); return f; }
        xp = save;
    }
    if (at("∀")) { NX(); Tok *i = NX(); want("→"); return sym3("forall", nsym(i->s), p_face()); }
    if (at("tt") || at("ff")) return nsym(NX()->s);
    Node *r = p_iexpr();
    if (!at("=")) return r;   /* a face given by an expression */
    NX();
    Tok *e = NX();
    const char *v = (e->k == X_NUM && (!strcmp(e->s, "0") || !strcmp(e->s, "1"))) ? e->s : x_is(e, "i0") ? "0" : x_is(e, "i1") ? "1" : NULL;
    if (!v) x_err("a face is r = 0 or r = 1, found", e);
    return sym3("=", r, nsym(v));
}
static Node *p_fconj(void) { Node *a = p_fatom(); while (at("/\\")) { NX(); a = sym3("&", a, p_fatom()); } return a; }
static Node *p_face(void) { Node *a = p_fconj(); while (at("\\/")) { NX(); a = sym3("|", a, p_fconj()); } return a; }
/* an interval: application terms joined by /\ \/ (no =) */
static Node *p_app(void);
static Node *p_iexpr(void) {
    Node *a = p_app();
    while (at("/\\") || at("\\/")) {
        /* a face's ∧ ∨ join faces, not intervals: stop where the next operand is a face */
        int save = xp; const char *op = NX()->s;
        Node *b = p_app();
        if (at("=")) { xp = save; break; }
        a = sym3(op, a, b);
    }
    return a;
}
/* does a bracketed group hold a face (an = at its top level)? */
static bool group_has_eq(void) {
    int d = 0;
    for (int i = xp + 1; i < nlt; i++) {
        int k = lt[i].k;
        if (k == X_LP || k == X_LB || k == X_LC) d++;
        else if (k == X_RP || k == X_RB || k == X_RC) { if (--d < 0) return false; }
        else if (d == 0 && x_is(&lt[i], "=")) return true;
        else if (d == 0 && (x_is(&lt[i], "let") || x_is(&lt[i], "↦") || k == X_COMMA || k == X_EOF)) return false;
    }
    return false;
}
/* a system: [ φ ↦ e , ... ] */
static Node *p_system(void) {
    wantk(X_LB, "[");
    Node **bs = NULL; size_t cap = 0; int n = 0;
    while (P0()->k != X_RB) {
        Node *f = p_face(); want("↦"); Node *e = p_expr(0);
        GROW(bs, cap, (size_t)n + 1); bs[n++] = lst(2, f, e);
        if (P0()->k == X_COMMA) NX(); else break;
    }
    wantk(X_RB, "]");
    Node *s = nlist(n + 1); s->it[0] = nsym("sys");
    for (int i = 0; i < n; i++) s->it[i + 1] = bs[i];
    free(bs);
    return s;
}
static Node *p_block_lets(Node *body_after);
static Node *p_case(void);
static Node *p_lambda(void) {
    NX();
    Bnd bs[64]; int n = p_binders(bs, 64, false);
    if (!n) x_err("λ needs a binder, found", P0());
    want("→");
    return lam_of(bs, n, p_expr(0));
}
/* an atom: a name, a number, (e), (a , b), [system], ~ r, λ, let, if, case */
static Node *p_atom(void) {
    Tok *t = P0();
    if (x_is(t, "~")) { NX(); return sym2("~", p_atom()); }
    if (x_is(t, "λ")) return p_lambda();
    if (x_is(t, "let")) return p_block_lets(NULL);
    if (x_is(t, "if")) { NX(); Node *c = p_expr(0); want("then"); Node *a = p_expr(0); want("else"); Node *b = p_expr(0); return sym4("if", c, a, b); }
    if (x_is(t, "case")) return p_case();
    if (x_is(t, "∀")) { NX(); Tok *i = NX(); want("→"); return sym3("forall", nsym(i->s), p_face()); }
    if (t->k == X_NUM) { NX(); return sym2("%num", nsym(t->s)); }
    if (t->k == X_LB) return p_system();
    if (t->k == X_LP) {
        if (group_has_eq()) { NX(); Node *f = p_face(); wantk(X_RP, ")"); return f; }
        NX();
        if (P0()->k == X_RP) { NX(); return nsym("%absurd"); }
        /* an operator section name: (_+_) or (+) */
        if (fix_of(P0()) && P1()->k == X_RP) { Node *r = nsym(op_name(NX()->s)); NX(); return r; }
        Node *e = p_expr(0);
        if (P0()->k == X_COMMA) {
            Node *xs[256]; int n = 0; xs[n++] = e;
            while (P0()->k == X_COMMA) { NX(); if (n == 256) x_err("tuple too long", P0()); xs[n++] = p_expr(0); }
            wantk(X_RP, ")");
            Node *r = xs[n - 1];
            for (int i = n - 2; i >= 0; i--) r = sym3("pair", xs[i], r);
            return r;
        }
        wantk(X_RP, ")");
        return e;
    }
    if (t->k == X_LC) { NX(); Node *e = p_expr(0); wantk(X_RC, "}"); return sym2("%imp", e); }
    if (t->k == X_ID && !reserved(t->s)) {
        NX();
        if (!strcmp(t->s, "i0")) return nsym("0");
        if (!strcmp(t->s, "i1")) return nsym("1");
        if (!strcmp(t->s, "Type")) return nsym("U");
        if (!strcmp(t->s, "_")) return nsym("%hole");
        return nsym(t->s);
    }
    x_err("unexpected", t);
    return NULL;
}
/* a postfix projection .f */
static Node *p_post(void) {
    Node *a = p_atom();
    while (P0()->k == X_DOT) a = lst(2, nsym(NX()->s), a);
    return a;
}
/* application: a head and its operands; a primitive takes its operands into its own form */
static Node *p_app(void) {
    Node *h = p_post();
    Node *args[256]; int n = 0;
    while (atom_start(P0()) && !x_is(P0(), "λ") && !x_is(P0(), "let") && !x_is(P0(), "if") && !x_is(P0(), "case")) {
        if (n == 256) x_err("too many operands", P0());
        args[n++] = p_post();
    }
    /* a λ, let, if or case as the last operand */
    if (x_is(P0(), "λ") || x_is(P0(), "let") || x_is(P0(), "if") || x_is(P0(), "case")) { if (n == 256) x_err("too many operands", P0()); args[n++] = p_atom(); }
    int pi = !h->list ? prim_ix(h->s) : -1;
    if (!n) return pi >= 0 && PRIM[pi].lo == 0 ? lst(1, h) : h;
    if (!h->list && !strcmp(h->s, "U")) {
        /* Type ℓ */
        Node *u = sym2("U", args[0]); Node *r = u;
        for (int i = 1; i < n; i++) { Node *a = lst(2, r, args[i]); r = a; }
        return r;
    }
    if (pi >= 0) {
        int k = n < PRIM[pi].hi ? n : PRIM[pi].hi;
        if (k < PRIM[pi].lo) x_err("too few operands for", P0());
        const char *nm = !strcmp(h->s, "%@") ? "@" : !strcmp(h->s, "%fand") ? "&" : !strcmp(h->s, "%for") ? "|" : h->s;
        Node *r = nlist(k + 1); r->it[0] = nsym(nm);
        for (int i = 0; i < k; i++) r->it[i + 1] = args[i];
        if (k == n) return r;
        Node *a = nlist(n - k + 1); a->it[0] = r;
        for (int i = k; i < n; i++) a->it[i - k + 1] = args[i];
        return a;
    }
    Node *r = nlist(n + 1); r->it[0] = h;
    for (int i = 0; i < n; i++) r->it[i + 1] = args[i];
    return r;
}
static Node *mk_pi(const char *k, Node *x, Node *A, Node *B) { return sym4(k, x, A, B); }
/* a dependent arrow or product: (x y : A) → B, {x : A} → B, (x : A) × B */
static Node *p_telescope_type(void) {
    int save = xp;
    Bnd bs[64]; int n = p_binders(bs, 64, false);
    bool typed = n > 0; for (int i = 0; i < n; i++) if (!bs[i].ty) typed = false;
    if (typed && (at("→") || at("×"))) {
        const char *k = at("→") ? "Pi" : "Sigma"; NX();
        Node *B = p_expr(k[0] == 'P' ? 0 : 2);
        for (int i = n - 1; i >= 0; i--) {
            B = mk_pi(k, bs[i].name, bs[i].ty, B);
            if (bs[i].imp) B = sym2("%impPi", B);
        }
        return B;
    }
    xp = save;
    return NULL;
}
static Node *p_expr(int minprec) {
    Node *lhs = NULL;
    if (P0()->k == X_LP || P0()->k == X_LC) lhs = p_telescope_type();
    if (!lhs) lhs = p_app();
    for (;;) {
        Tok *t = P0();
        if (x_is(t, "≡") && P1()->k == X_LB) {
            /* a ≡[ i ↦ A ] b */
            if (4 < minprec) break;
            NX(); NX();
            Tok *i = NX(); want("↦"); Node *A = p_expr(0); wantk(X_RB, "]");
            Node *b = p_expr(5);
            lhs = sym4("PathP", sym3("plam", nsym(i->s), A), lhs, b);
            continue;
        }
        Fix *f = fix_of(t);
        if (!f || f->prec < minprec) break;
        NX();
        Node *rhs = p_expr(f->assoc == 2 ? f->prec : f->prec + 1);
        if (!strcmp(f->op, "→")) lhs = mk_pi("Pi", xfresh("_"), lhs, rhs);
        else if (!strcmp(f->op, "×")) lhs = mk_pi("Sigma", xfresh("_"), lhs, rhs);
        else if (!strcmp(f->op, "@")) lhs = sym3("@", lhs, rhs);
        else if (!strcmp(f->op, "/\\") || !strcmp(f->op, "\\/")) lhs = sym3(f->op, lhs, rhs);
        else lhs = lst(3, nsym(op_name(f->op)), lhs, rhs);
        if (f->assoc == 0) { Fix *g = fix_of(P0()); if (g && g->prec == f->prec && g->assoc == 0) x_err("a non-associative operator chained:", P0()); }
    }
    return lhs;
}
/* case e [return P] of alternatives (pattern → body) */
static Node *p_pattern_head(void);
static Node *p_case(void) {
    want("case");
    Node *e = p_expr(0);
    Node *P = NULL;
    if (at("return")) { NX(); P = p_expr(0); }
    want("of");
    wantk(X_VO, "a block of alternatives");
    Node **alts = NULL; size_t cap = 0; int n = 0;
    while (P0()->k != X_VC) {
        Node *pat = p_pattern_head();
        want("→");
        Node *b = p_expr(0);
        GROW(alts, cap, (size_t)n + 1); alts[n++] = lst(2, pat, b);
        if (P0()->k == X_VS || P0()->k == X_SEMI) NX(); else if (P0()->k != X_VC) x_err("expected a new alternative, found", P0());
    }
    NX();
    Node *r = nlist(n + (P ? 3 : 2)); int k = 0;
    r->it[k++] = nsym(P ? "elim" : "case");
    if (P) r->it[k++] = P;
    r->it[k++] = e;
    for (int i = 0; i < n; i++) r->it[k++] = alts[i];
    free(alts);
    return r;
}
/* a case alternative's pattern: c x y, _, x */
static Node *p_pattern_head(void) {
    Tok *t = NX();
    if (t->k != X_ID) x_err("a pattern starts with a constructor or a variable, found", t);
    if (!strcmp(t->s, "_")) return nsym("_");
    int c = con_lookup(t->s);
    if (c < 0 || (CON[c].nfields + CON[c].ndims > 0 && at("→"))) {
        /* a variable: the scrutinee, named (a constructor's name with no fields, of a constructor
         * that has fields, is a variable) */
        if (atom_start(P0()) && !at("→")) x_err("not a constructor:", t);
        return nsym(t->s);
    }
    Node *xs[64]; int n = 0;
    xs[n++] = nsym(t->s);
    while (P0()->k == X_ID && !at("→")) { if (n == 64) x_err("too many fields", P0()); Tok *f = NX(); xs[n++] = nsym(!strcmp(f->s, "_") ? "_" : f->s); }
    Node *r = nlist(n);
    for (int i = 0; i < n; i++) r->it[i] = xs[i];
    return r;
}
/* let x = e; f x y = e; ... in body */
static Node *p_block_lets(Node *after) {
    want("let");
    wantk(X_VO, "a block of definitions");
    Node *names[256], *vals[256]; int n = 0;
    while (P0()->k != X_VC) {
        Tok *t = NX();
        if (t->k != X_ID || reserved(t->s)) x_err("a let defines a name, found", t);
        Bnd bs[64]; int nb = p_binders(bs, 64, false);
        Node *T = NULL;
        if (at(":")) { NX(); T = p_expr(0); if (P0()->k == X_VS) NX(); Tok *u = NX(); if (!x_is(u, t->s)) x_err("a signature is followed by its definition, found", u); nb = p_binders(bs, 64, false); }
        want("=");
        Node *v = lam_of(bs, nb, p_expr(0));
        if (T) v = sym3("the", T, v);
        if (n == 256) x_err("too many definitions in one let", t);
        names[n] = nsym(t->s); vals[n++] = v;
        if (P0()->k == X_VS || P0()->k == X_SEMI) NX(); else if (P0()->k != X_VC) x_err("expected a new definition, found", P0());
    }
    NX();
    Node *body = after;
    if (!body) { want("in"); body = p_expr(0); }
    for (int i = n - 1; i >= 0; i--) body = sym4("let", names[i], vals[i], body);
    return body;
}

/* ---------------- numerals and intervals ----------------
 * A numeral n is an interval endpoint where an interval stands (an operand of @, ~, ∧, ∨, a face,
 * a constructor's dimension), else the natural number suc (... zero). */
static Node *numeral(const char *s) {
    long n = atol(s);
    if (con_lookup("zero") < 0 || con_lookup("suc") < 0) die("a numeral needs the natural numbers (zero, suc):", s);
    Node *r = nsym("zero");
    for (long i = 0; i < n; i++) r = lst(2, nsym("suc"), r);
    return r;
}
static Node *resolve(Node *t, bool iv) {
    if (!t->list) return t;
    if (t->n == 2 && is(t->it[0], "%num")) {
        if (iv) { if (!strcmp(t->it[1]->s, "0") || !strcmp(t->it[1]->s, "1")) return t->it[1]; die("an interval is 0, 1, a dimension, ~, /\\ or \\/; found", t->it[1]->s); }
        return numeral(t->it[1]->s);
    }
    if (t->n == 0) return t;
    Node *h = t->it[0];
    Node *r = nlist(t->n); r->it[0] = h->list ? resolve(h, false) : h;
    int c = !h->list ? con_lookup(h->s) : -1;
    for (int i = 1; i < t->n; i++) {
        bool v = false;
        if (!h->list) {
            const char *k = h->s;
            if (!strcmp(k, "@")) v = i >= 2 && i <= 2;
            else if (!strcmp(k, "~") || !strcmp(k, "/\\") || !strcmp(k, "\\/")) v = true;
            else if (!strcmp(k, "=")) v = i == 1;
            else if (c >= 0) v = i - 1 >= CON[c].nfields;
        }
        r->it[i] = resolve(t->it[i], v);
    }
    return r;
}

/* ---------------- names in terms ---------------- */
/* t with the free name x replaced by the name y (y is fresh or not bound inside t) */
static Node *x_rename(Node *t, const char *x, const char *y) {
    if (!t->list) return !strcmp(t->s, x) ? nsym(y) : t;
    if (t->n == 0 || t->it[0]->list) { Node *r = nlist(t->n); for (int i = 0; i < t->n; i++) r->it[i] = x_rename(t->it[i], x, y); return r; }
    const char *k = t->it[0]->s;
    if ((!strcmp(k, "lam") || !strcmp(k, "plam") || !strcmp(k, "forall")) && t->n == 3 && !t->it[1]->list && !strcmp(t->it[1]->s, x)) return t;
    if ((!strcmp(k, "Pi") || !strcmp(k, "Sigma")) && t->n == 4 && !t->it[1]->list && !strcmp(t->it[1]->s, x))
        return sym4(k, t->it[1], x_rename(t->it[2], x, y), t->it[3]);
    if (!strcmp(k, "let") && t->n == 4 && !strcmp(t->it[1]->s, x)) return sym4("let", t->it[1], x_rename(t->it[2], x, y), t->it[3]);
    Node *r = nlist(t->n); r->it[0] = t->it[0];
    bool cs = !strcmp(k, "case") || !strcmp(k, "elim");
    int from = !strcmp(k, "elim") ? 3 : 2;
    for (int i = 1; i < t->n; i++) {
        Node *a = t->it[i];
        if (cs && i >= from && a->list && a->n == 2) {
            /* an alternative: its pattern binds */
            Node *pat = a->it[0]; bool binds = false;
            if (pat->list) { for (int q = 1; q < pat->n; q++) if (!strcmp(pat->it[q]->s, x)) binds = true; }
            else if (!strcmp(pat->s, x)) binds = true;
            r->it[i] = binds ? a : lst(2, pat, x_rename(a->it[1], x, y));
            continue;
        }
        r->it[i] = (!strcmp(k, "=") && i == 2) ? a : x_rename(a, x, y);
    }
    return r;
}

/* ---------------- definitions by clauses (untyped) ----------------
 * Clauses f p1 ... pn = e, tried in order. Each column whose patterns are all the same variable
 * keeps its name; a split column is cased on, constructor by constructor: the clauses with that
 * constructor (its field patterns as new columns) and those with a variable there (renamed to the
 * scrutinee); the constructors no clause names share one default alternative. */
typedef struct { Node **p; Node *rhs; } Row;
static bool pat_wild(Node *p) { return !p->list && (!strcmp(p->s, "%hole") || !strcmp(p->s, "_")); }
/* a pattern's constructor: (c p...), or a constructor without fields (a name of a constructor with
 * fields, alone, is a variable) */
static int pat_con(Node *p) {
    if (!p->list) { int c = con_lookup(p->s); return c >= 0 && CON[c].nfields + CON[c].ndims == 0 ? c : -1; }
    return p->it[0]->list ? -1 : con_lookup(p->it[0]->s);
}
static Node *pat_norm(Node *p) {
    /* numerals as constructors */
    if (p->list && p->n == 2 && is(p->it[0], "%num")) return numeral(p->it[1]->s);
    if (p->list) { Node *r = nlist(p->n); for (int i = 0; i < p->n; i++) r->it[i] = i ? pat_norm(p->it[i]) : p->it[i]; return r; }
    return p;
}
static Node *cc_match(Node **vars, int nv, Row *rows, int nr, const char *fname);
static Node *cc_match(Node **vars, int nv, Row *rows, int nr, const char *fname) {
    if (!nr) die("the clauses do not cover every case of", fname);
    /* the first row matches when all its patterns are variables */
    int j = -1;
    for (int q = 0; q < nv; q++) if (!pat_wild(rows[0].p[q]) && pat_con(rows[0].p[q]) >= 0) { j = q; break; }
    if (j < 0) {
        Node *e = rows[0].rhs;
        for (int q = 0; q < nv; q++) if (!pat_wild(rows[0].p[q]) && strcmp(rows[0].p[q]->s, vars[q]->s)) e = x_rename(e, rows[0].p[q]->s, vars[q]->s);
        return e;
    }
    int d = CON[pat_con(rows[0].p[j])].data;
    int N = DATA[d].ncons;
    Node **alts = calloc((size_t)N + 2, sizeof(Node *)); int na = 0;
    bool need_default = false;
    for (int ci = 0; ci < N; ci++) {
        int c = DATA[d].cons[ci];
        int nf = CON[c].nfields + CON[c].ndims;
        bool named = false;
        for (int r = 0; r < nr; r++) if (pat_con(rows[r].p[j]) == c) named = true;
        if (!named) { need_default = true; continue; }
        /* the field names: the first row's variables there, else fresh */
        Node **fv = calloc((size_t)nf + 1, sizeof(Node *));
        for (int r = 0; r < nr && nf; r++) if (pat_con(rows[r].p[j]) == c) {
            Node *p = rows[r].p[j];
            for (int q = 0; q < nf; q++) if (!fv[q] && p->list && q + 1 < p->n && !p->it[q + 1]->list && !pat_wild(p->it[q + 1]) && con_lookup(p->it[q + 1]->s) < 0) {
                bool dup = false; for (int z = 0; z < q; z++) if (fv[z] && !strcmp(fv[z]->s, p->it[q + 1]->s)) dup = true;
                bool clash = false; for (int z = 0; z < nv; z++) if (z != j && !strcmp(vars[z]->s, p->it[q + 1]->s)) clash = true;
                if (!dup && !clash) fv[q] = p->it[q + 1];
            }
            break;
        }
        for (int q = 0; q < nf; q++) if (!fv[q]) fv[q] = xfresh("f");
        /* the rows for c: its fields as columns in place of column j */
        int nv2 = nv - 1 + nf;
        Node **v2 = calloc((size_t)nv2 + 1, sizeof(Node *));
        for (int q = 0; q < j; q++) v2[q] = vars[q];
        for (int q = 0; q < nf; q++) v2[j + q] = fv[q];
        for (int q = j + 1; q < nv; q++) v2[q - 1 + nf] = vars[q];
        Row *r2 = calloc((size_t)nr + 1, sizeof(Row)); int n2 = 0;
        for (int r = 0; r < nr; r++) {
            Node *p = rows[r].p[j];
            int pc = pat_con(p);
            if (!pat_wild(p) && pc >= 0 && pc != c) continue;
            Row x; x.p = calloc((size_t)nv2 + 1, sizeof(Node *)); x.rhs = rows[r].rhs;
            for (int q = 0; q < j; q++) x.p[q] = rows[r].p[q];
            for (int q = j + 1; q < nv; q++) x.p[q - 1 + nf] = rows[r].p[q];
            if (pc == c) {
                int got = p->list ? p->n - 1 : 0;
                if (got != nf) die("a pattern has the wrong number of fields for", CON[c].name);
                for (int q = 0; q < nf; q++) x.p[j + q] = p->it[q + 1];
            } else {
                for (int q = 0; q < nf; q++) x.p[j + q] = nsym("_");
                if (!pat_wild(p) && strcmp(p->s, vars[j]->s)) x.rhs = x_rename(x.rhs, p->s, vars[j]->s);
            }
            r2[n2++] = x;
        }
        Node *body = cc_match(v2, nv2, r2, n2, fname);
        Node *pat = nlist(nf + 1); pat->it[0] = nsym(CON[c].name);
        for (int q = 0; q < nf; q++) pat->it[q + 1] = fv[q];
        alts[na++] = lst(2, pat, body);
    }
    if (need_default) {
        /* the rows with a variable in column j */
        Node **v2 = calloc((size_t)nv + 1, sizeof(Node *));
        for (int q = 0; q < nv; q++) v2[q] = vars[q];
        Row *r2 = calloc((size_t)nr + 1, sizeof(Row)); int n2 = 0;
        for (int r = 0; r < nr; r++) if (pat_wild(rows[r].p[j]) || pat_con(rows[r].p[j]) < 0) {
            Row x = rows[r]; x.p = calloc((size_t)nv + 1, sizeof(Node *));
            memcpy(x.p, rows[r].p, (size_t)nv * sizeof(Node *));
            if (!pat_wild(x.p[j]) && strcmp(x.p[j]->s, vars[j]->s)) x.rhs = x_rename(x.rhs, x.p[j]->s, vars[j]->s);
            x.p[j] = nsym("_");
            r2[n2++] = x;
        }
        alts[na++] = lst(2, nsym("_"), cc_match(v2, nv, r2, n2, fname));
    }
    Node *cs = nlist(na + 2); cs->it[0] = nsym("case"); cs->it[1] = vars[j];
    for (int i = 0; i < na; i++) cs->it[i + 2] = alts[i];
    free(alts);
    return cs;
}
/* a clause's left side as its function and patterns: f p..., a op b */
typedef struct { const char *f; Node **ps; int np; Node *field; } Lhs;
static Lhs lhs_of(Node *e) {
    Lhs l = { 0 };
    if (!e->list) { l.f = e->s; return l; }
    /* (fld (f args)): a copattern */
    if (e->n == 2 && !e->it[0]->list && fld_lookup(e->it[0]->s) >= 0 && e->it[1]->list) { l = lhs_of(e->it[1]); l.field = e->it[0]; return l; }
    if (e->n == 2 && !e->it[0]->list && fld_lookup(e->it[0]->s) >= 0 && !e->it[1]->list) { l.f = e->it[1]->s; l.field = e->it[0]; return l; }
    if (e->it[0]->list) die("a clause's left side is a name applied to patterns", NULL);
    l.f = e->it[0]->s; l.np = e->n - 1;
    l.ps = calloc((size_t)l.np + 1, sizeof(Node *));
    for (int i = 0; i < l.np; i++) l.ps[i] = pat_norm(e->it[i + 1]);
    return l;
}
static Node *clauses_untyped(const char *f, Lhs *ls, Node **rhs, int n) {
    int np = ls[0].np;
    for (int i = 1; i < n; i++) if (ls[i].np != np) die("clauses with different numbers of patterns:", f);
    Node **vars = calloc((size_t)np + 1, sizeof(Node *));
    for (int q = 0; q < np; q++) {
        /* a column whose patterns are one variable keeps its name */
        const char *nm = NULL; bool same = true;
        for (int i = 0; i < n; i++) {
            Node *p = ls[i].ps[q];
            if (pat_wild(p)) continue;
            if (pat_con(p) >= 0 || p->list) { same = false; break; }
            if (!nm) nm = p->s; else if (strcmp(nm, p->s)) same = false;
        }
        bool dup = false; for (int z = 0; z < q && nm; z++) if (!strcmp(vars[z]->s, nm)) dup = true;
        vars[q] = same && nm && !dup ? nsym(nm) : xfresh("a");
    }
    Row *rows = calloc((size_t)n + 1, sizeof(Row));
    for (int i = 0; i < n; i++) rows[i] = (Row){ ls[i].ps, rhs[i] };
    Node *body = cc_match(vars, np, rows, n, f);
    for (int q = np - 1; q >= 0; q--) body = sym3("lam", vars[q], body);
    return body;
}

/* ---------------- declarations ---------------- */
static void toplevel(Node *f);
static void x_top(Node *f);
static Node *core(Node *t) { return resolve(t, false); }
static Node *resolve_keep(Node *t) { return t; }
typedef struct { int np; Node **pn, **pty; bool *imp, *rev; Node *R; } TSig;
static TSig tsig;
static Node *autobind(Node *sig);
static void tele(Node *sig);
static Node *clauses_typed(const char *f, Lhs *ls, Node **rhs, int n);
static void x_checked(const char *name, Node *sig, Node *body, bool expect, uint64_t mask, int mlen);
/* the names defined without a signature later in the file: a use before the definition declares it */
static char **xlater; static int nxlater, capxlater;
static void free_syms(Node *t, Names *bound, Names *out) {
    if (!t->list) { if (!names_has(bound, t->s)) names_add(out, t->s); return; }
    if (t->n == 0) return;
    if (!t->it[0]->list) {
        const char *k = t->it[0]->s;
        if ((!strcmp(k, "lam") || !strcmp(k, "plam") || !strcmp(k, "forall")) && t->n == 3) {
            int save = bound->n; names_add(bound, t->it[1]->s); free_syms(t->it[2], bound, out); bound->n = save; return;
        }
        if ((!strcmp(k, "Pi") || !strcmp(k, "Sigma")) && t->n == 4) {
            free_syms(t->it[2], bound, out); int save = bound->n; names_add(bound, t->it[1]->s); free_syms(t->it[3], bound, out); bound->n = save; return;
        }
        if (!strcmp(k, "let") && t->n == 4) {
            free_syms(t->it[2], bound, out); int save = bound->n; names_add(bound, t->it[1]->s); free_syms(t->it[3], bound, out); bound->n = save; return;
        }
        if (!strcmp(k, "case") || !strcmp(k, "elim")) {
            int from = !strcmp(k, "elim") ? 3 : 2;
            for (int i = 1; i < from && i < t->n; i++) free_syms(t->it[i], bound, out);
            for (int i = from; i < t->n; i++) {
                Node *a = t->it[i]; if (!a->list || a->n != 2) continue;
                int save = bound->n;
                if (a->it[0]->list) for (int q = 1; q < a->it[0]->n; q++) names_add(bound, a->it[0]->it[q]->s);
                else names_add(bound, a->it[0]->s);
                free_syms(a->it[1], bound, out); bound->n = save;
            }
            return;
        }
    }
    for (int i = 0; i < t->n; i++) free_syms(t->it[i], bound, out);
}
static void declare_forward(Node *body) {
    Names b = { 0 }, out = { 0 };
    free_syms(body, &b, &out);
    for (int i = 0; i < out.n; i++)
        for (int k = 0; k < nxlater; k++) if (xlater[k] && !strcmp(xlater[k], out.v[i]) && global_lookup(out.v[i]) < 0) {
            x_top(sym2("declare", nsym(out.v[i])));
        }
    free(b.v); free(out.v);
}
static void def_untyped(const char *name, Node *body) {
    body = core(body);
    declare_forward(body);
    for (int k = 0; k < nxlater; k++) if (xlater[k] && !strcmp(xlater[k], name)) { xlater[k] = NULL; break; }
    x_top(sym3("def", nsym(name), body));
}
/* a data declaration: data D params = c fields | ...  A field is (x : A), (i : I) a dimension, a
 * system [ φ ↦ e , ... ] the boundary; a bare atom is the field's type (or, in an unchecked type,
 * its name). */
static Node *p_data_ctor(void) {
    Tok *c = NX();
    if (c->k != X_ID || reserved(c->s)) x_err("a constructor's name, found", c);
    Node *items[128]; int n = 0;
    items[n++] = nsym(c->s);
    Node *dims[16]; int nd = 0; Node *bdy = NULL;
    while (atom_start(P0()) || P0()->k == X_LB) {
        if (n >= 120) x_err("too many fields", P0());
        if (P0()->k == X_LB) { bdy = p_system(); continue; }
        if (P0()->k == X_LP) {
            Bnd bs[16]; int save = xp; int nb = p_binders(bs, 16, false);
            if (nb && bs[0].ty) {
                for (int q = 0; q < nb; q++) {
                    if (bs[q].dim) { if (nd == 16) x_err("too many dimensions", P0()); dims[nd++] = bs[q].name; }
                    else items[n++] = lst(2, bs[q].name, core(bs[q].ty));
                }
                continue;
            }
            xp = save;
        }
        Node *a = p_post();
        if (!x_typed && !a->list) items[n++] = a;
        else { static int nfld0; char nm[32]; snprintf(nm, sizeof nm, "x%d", nfld0++); items[n++] = lst(2, nsym(nm), core(a)); }
    }
    if (nd) {
        Node *dl = nlist(nd + 1); dl->it[0] = nsym("dims");
        for (int q = 0; q < nd; q++) dl->it[q + 1] = dims[q];
        items[n++] = dl;
        if (bdy) for (int q = 1; q < bdy->n; q++) items[n++] = core(bdy->it[q]);
    } else if (bdy) x_err("a boundary needs dimensions (i : I)", P0());
    Node *r = nlist(n);
    for (int i = 0; i < n; i++) r->it[i] = items[i];
    return r;
}
static Node *p_data(void) {
    want("data");
    Tok *d = NX();
    if (d->k != X_ID) x_err("a data type's name, found", d);
    Node *ps[32]; int np = 0;
    while (!at("=") && P0()->k != X_VS && P0()->k != X_VC && P0()->k != X_EOF) {
        if (np == 32) x_err("too many parameters", P0());
        if (P0()->k == X_LP) {
            Bnd bs[16]; int nb = p_binders(bs, 16, true);
            for (int q = 0; q < nb; q++) ps[np++] = lst(2, bs[q].name, core(bs[q].ty));
            continue;
        }
        Tok *t = NX();
        if (t->k != X_ID) x_err("a parameter, found", t);
        ps[np++] = x_typed ? lst(2, nsym(t->s), nsym("U")) : nsym(t->s);
    }
    Node *head = nsym(d->s);
    if (np) { head = nlist(np + 1); head->it[0] = nsym(d->s); for (int i = 0; i < np; i++) head->it[i + 1] = ps[i]; }
    Node *cs[256]; int nc = 0;
    if (at("=")) {
        NX();
        if (at("|")) NX();
        for (;;) {
            if (nc == 256) x_err("too many constructors", P0());
            cs[nc++] = p_data_ctor();
            if (at("|")) { NX(); continue; }
            break;
        }
    }
    Node *r = nlist(nc + 2); r->it[0] = nsym("data"); r->it[1] = head;
    for (int i = 0; i < nc; i++) r->it[i + 2] = cs[i];
    return r;
}
/* the end of a declaration */
static void decl_end(void) {
    if (P0()->k == X_VS) { NX(); return; }
    if (P0()->k == X_EOF || P0()->k == X_VC) return;
    x_err("expected the end of the declaration, found", P0());
}
/* a clause's left side up to = (or : for a signature) */
static Node *p_lhs(void) { return p_expr(1); }
static bool x_fail_next;
static void x_test(const char *h, Node *a, Node *b) {
    Node *f = b ? sym3(h, a, b) : sym2(h, a);
    x_top(f);
}
static void p_decl(void);
static void p_decls_block(void) {
    wantk(X_VO, "a block");
    while (P0()->k != X_VC && P0()->k != X_EOF) { p_decl(); }
    if (P0()->k == X_VC) NX();
}
static void p_decl(void) {
    Tok *t = P0();
    bool fail = x_fail_next; x_fail_next = false;
    if (x_is(t, "#fail")) { NX(); x_fail_next = true; return; }
    if (x_is(t, "import")) {
        NX(); Tok *f = NX();
        if (f->k != X_STR && f->k != X_ID) x_err("import needs a file, found", f);
        char *q = malloc(strlen(f->s) + 3); sprintf(q, "\"%s\"", f->s);
        x_top(sym2("import", nsym(q)));
        decl_end(); return;
    }
    if (x_is(t, "infix") || x_is(t, "infixl") || x_is(t, "infixr")) {
        int assoc = x_is(t, "infixl") ? 1 : x_is(t, "infixr") ? 2 : 0; NX();
        Tok *n = NX(); if (n->k != X_NUM) x_err("a fixity needs a precedence, found", n);
        while (P0()->k == X_ID) {
            const char *o = NX()->s;
            if (o[0] == '_' && strlen(o) > 2 && o[strlen(o) - 1] == '_') { char *b = strdup(o + 1); b[strlen(b) - 1] = 0; o = b; }
            fix_set(o, atoi(n->s), assoc);
        }
        decl_end(); return;
    }
    if (x_is(t, "data")) {
        Node *d = p_data();
        if (x_typed) x_top(sym2(fail ? "checked-data-fail" : "checked-data", d));
        else x_top(d);
        decl_end(); return;
    }
    if (x_is(t, "postulate")) {
        NX();
        wantk(X_VO, "a block of postulates");
        while (P0()->k != X_VC) {
            Node *ns[64]; int n = 0;
            while (P0()->k == X_ID && !reserved(P0()->s)) { if (n == 64) x_err("too many names", P0()); ns[n++] = nsym(NX()->s); }
            if (!n) x_err("a postulate names something, found", P0());
            Node *ax = nlist(n + 1); ax->it[0] = nsym("axiom");
            if (at(":")) { NX(); Node *T = core(p_expr(0)); for (int i = 0; i < n; i++) ax->it[i + 1] = lst(2, ns[i], T); }
            else for (int i = 0; i < n; i++) ax->it[i + 1] = ns[i];
            x_top(ax);
            if (P0()->k == X_VS || P0()->k == X_SEMI) NX(); else if (P0()->k != X_VC) x_err("expected a new postulate, found", P0());
        }
        NX();
        decl_end(); return;
    }
    if (x_is(t, "#show")) { NX(); x_test("show", core(p_expr(0)), NULL); decl_end(); return; }
    if (x_is(t, "#observe")) { NX(); Tok *n = NX(); x_test("observe", nsym(n->s), core(p_expr(0))); decl_end(); return; }
    if (x_is(t, "#same")) { NX(); Node *a = core(p_expr(0)); want("="); Node *b = core(p_expr(0)); x_test("same", a, b); decl_end(); return; }
    if (x_is(t, "#check")) {
        NX(); Node *e = p_expr(0); want(":"); Node *T = p_expr(0);
        if (x_typed) { tsig.np = 0; x_checked(NULL, T, e, !fail, 0, 0); }
        else x_test(fail ? "check-fail" : "check", core(T), core(e));
        decl_end(); return;
    }
    if (x_is(t, "#check-data")) { NX(); x_test("check-data", nsym(NX()->s), NULL); decl_end(); return; }
    if (x_is(t, "#input")) { NX(); Node *ns = nlist(1); ns->it[0] = nsym("input"); while (P0()->k == X_ID) { Node *a = nlist(ns->n + 1); memcpy(a->it, ns->it, (size_t)ns->n * sizeof(Node *)); a->it[ns->n] = nsym(NX()->s); ns = a; } x_top(ns); decl_end(); return; }
    if (x_is(t, "#supply")) { NX(); Node *n = nsym(NX()->s); want("="); x_test("supply", n, core(p_expr(0))); decl_end(); return; }
    /* clauses (and a signature): every clause of one name, consecutive */
    Node *lhs = p_lhs();
    Node *sig = NULL;
    if (at(":")) {
        if (lhs->list) x_err("a signature names one thing, found", P0());
        if (!x_typed) x_err("a signature, before the checker is loaded:", P0());
        NX(); sig = p_expr(0);
        if (P0()->k != X_VS) x_err("a signature is followed by its definition, found", P0());
        NX();
        lhs = p_lhs();
    }
    want("=");
    Node *rhs = p_expr(0);
    Lhs l0 = lhs_of(lhs);
    Lhs *ls = malloc(sizeof(Lhs)); Node **rs = malloc(sizeof(Node *)); int n = 1;
    ls[0] = l0; rs[0] = rhs;
    for (;;) {
        int save = xp;
        if (P0()->k != X_VS) break;
        NX();
        if (P0()->k != X_ID || reserved(P0()->s) || P0()->s[0] == '#') { xp = save; break; }
        Node *l2 = p_lhs();
        if (!at("=")) { xp = save; break; }
        Lhs l = lhs_of(l2);
        if (strcmp(l.f, l0.f)) { xp = save; break; }
        NX();
        ls = realloc(ls, (size_t)(n + 1) * sizeof(Lhs)); rs = realloc(rs, (size_t)(n + 1) * sizeof(Node *));
        ls[n] = l; rs[n] = p_expr(0); n++;
    }
    if (sig) {
        if (strcmp(l0.f, lhs->list ? lhs_of(lhs).f : lhs->s)) {}
        Node *sg = autobind(resolve_keep(sig));
        tele(sg);
        uint64_t mask = 0; for (int q = 0; q < tsig.np && q < 64; q++) if (tsig.imp[q]) mask |= 1ull << q;
        for (int i = 0; i < n; i++) if (strcmp(ls[i].f, l0.f)) x_err("a clause of another definition after a signature:", P0());
        Node *body = clauses_typed(l0.f, ls, rs, n);
        x_checked(l0.f, sg, body, !fail, mask, tsig.np);
    } else def_untyped(l0.f, clauses_untyped(l0.f, ls, rs, n));
    decl_end();
}
/* a file in the surface language. Loading is reentrant (an import loads another file): each file
 * has its own tokens. skip_imports: its imports were loaded already (load_file loads them first). */
static void load_surface(const char *text, const char *path, bool imports_only) {
    Tok *s_xt = xt, *s_lt = lt; int s_nxt = nxt, s_capxt = capxt, s_nlt = nlt, s_caplt = caplt, s_xp = xp;
    char **s_later = xlater; int s_nlater = nxlater, s_caplater = capxlater;
    const char *s_file = x_file; bool s_typed = x_typed, s_fail = x_fail_next;
    xt = NULL; nxt = capxt = 0; lt = NULL; nlt = caplt = 0; xlater = NULL; nxlater = capxlater = 0;
    x_file = path; x_fail_next = false;
    x_lex(text); x_layout();
    for (int i = 0; i < nlt; i++) if ((i == 0 || lt[i - 1].k == X_VS) && lt[i].k == X_ID && !reserved(lt[i].s) && lt[i].s[0] != '#') {
        GROW(xlater, capxlater, (size_t)nxlater + 1); xlater[nxlater++] = lt[i].s;
    }
    xp = 0;
    /* the declarations of a file loaded once the checker is are checked */
    x_typed = global_lookup("chk-def") >= 0 && !x_untyped_file;
    while (P0()->k != X_EOF) {
        while (P0()->k == X_VS || P0()->k == X_VC) NX();
        if (P0()->k == X_EOF) break;
        bool imp = at("import");
        if (imp != imports_only) {
            /* skip this declaration: to the next item of the file's block */
            NX(); int d = 0;
            while (P0()->k != X_EOF && !(d == 0 && P0()->k == X_VS)) { if (P0()->k == X_VO) d++; else if (P0()->k == X_VC) d--; NX(); }
            continue;
        }
        p_decl();
    }
    xt = s_xt; nxt = s_nxt; capxt = s_capxt; lt = s_lt; nlt = s_nlt; caplt = s_caplt; xp = s_xp;
    xlater = s_later; nxlater = s_nlater; capxlater = s_caplater; x_file = s_file; x_typed = s_typed; x_fail_next = s_fail;
}

/* ---------------- printing the core forms in the surface language ----------------
 * pr(t, prec) is t's text at the context precedence prec (parenthesized when t binds more loosely),
 * on one line or broken over lines indented from the column it starts at. Precedences: 0 λ let if
 * case (they extend to the right), 0 →, 2 ×, operators by their fixity, 6 ∨, 7 ∧, 30 @, 100 an
 * application, 101 an operand. */
typedef struct { char *b; size_t n, cap; } Buf;
static void bput(Buf *o, const char *s) { size_t l = strlen(s); if (o->n + l + 1 > o->cap) { o->cap = (o->n + l + 1) * 2 + 256; o->b = realloc(o->b, o->cap); } memcpy(o->b + o->n, s, l + 1); o->n += l; }
static void bputc(Buf *o, char c) { char s[2] = { c, 0 }; bput(o, s); }
static void bnl(Buf *o, int col) { bputc(o, '\n'); for (int i = 1; i < col; i++) bputc(o, ' '); }
static int pr_width = 100;
static bool pr_typed;        /* a path λ prints as λ i → (the checker tells it apart), else λ (i : I) → */
static Buf pr_lost;          /* comments of subterms printed within a line */
static int vlen(const char *s) { int n = 0; for (; *s; s++) if (((unsigned char)*s & 0xC0) != 0x80) n++; return n; }
static bool has_nl(const char *s) { return strchr(s, '\n') != NULL; }
static void pr(Buf *o, Node *t, int prec, int col);
static char *pr_str(Node *t, int prec, int col) { Buf b = { 0 }; bput(&b, ""); pr(&b, t, prec, col); return b.b; }
static char *pr_flat(Node *t, int prec) { int w = pr_width; pr_width = 1 << 30; char *s = pr_str(t, prec, 1); pr_width = w; return s; }
static void lost(Node *t) { if (t->cmt) { bput(&pr_lost, t->cmt); t->cmt = NULL; } }
static bool is_face(Node *t) { return t->list && t->n >= 1 && !t->it[0]->list && (is(t->it[0], "=") || is(t->it[0], "&") || is(t->it[0], "|") || is(t->it[0], "forall")); }
static bool face_has_eq(Node *t) {
    if (!t->list || t->n < 1 || t->it[0]->list) return false;
    if (is(t->it[0], "=")) return true;
    if (is(t->it[0], "&") || is(t->it[0], "|")) { for (int i = 1; i < t->n; i++) if (face_has_eq(t->it[i])) return true; }
    if (is(t->it[0], "forall")) return true;
    return false;
}
/* a face inside a face: φ ∧ ψ, φ ∨ ψ, r = 0, ∀ i → φ */
static void pr_face(Buf *o, Node *t, int prec, int col) {
    if (t->list && t->n >= 1 && !t->it[0]->list) {
        const char *h = t->it[0]->s;
        if (!strcmp(h, "=")) {
            char *r = pr_str(t->it[1], 101, col);
            if (prec > 8) bputc(o, '(');
            bput(o, r); bput(o, " = "); bput(o, t->it[2]->s);
            if (prec > 8) bputc(o, ')');
            return;
        }
        if (!strcmp(h, "&") || !strcmp(h, "|")) {
            int p = h[0] == '&' ? 7 : 6;
            if (prec > p) bputc(o, '(');
            for (int i = 1; i < t->n; i++) { if (i > 1) bput(o, p == 7 ? " /\\ " : " \\/ "); pr_face(o, t->it[i], i == 1 ? p : p + 1, col); }
            if (prec > p) bputc(o, ')');
            return;
        }
        if (!strcmp(h, "forall")) { if (prec > 0) bputc(o, '('); bput(o, "∀ "); bput(o, t->it[1]->s); bput(o, " → "); pr_face(o, t->it[2], 0, col); if (prec > 0) bputc(o, ')'); return; }
    }
    pr(o, t, 101, col);
}
static bool mentions_x(Node *t, const char *x) { return mentions_name(t, x); }
static const char *keyw_name(const char *s) {
    /* names that would read as something else: written with a % prefix is not possible, so refuse */
    return s;
}
static void pr_sym(Buf *o, const char *s, bool iv) {
    if (!strcmp(s, "0")) { bput(o, iv ? "0" : "i0"); return; }
    if (!strcmp(s, "1")) { bput(o, iv ? "1" : "i1"); return; }
    if (!strcmp(s, "U")) { bput(o, "Type"); return; }
    bput(o, keyw_name(s));
}
/* the binders of a chain of λs and lines */
static Node *pr_binders(Buf *o, Node *t) {
    bool first = true;
    while (t->list && t->n >= 3 && !t->it[0]->list && (is(t->it[0], "lam") || is(t->it[0], "plam"))) {
        bool dim = is(t->it[0], "plam");
        for (int i = 1; i < t->n - 1; i++) {
            if (!first) bputc(o, ' ');
            first = false;
            if (dim && !pr_typed) { bput(o, "("); bput(o, param_name(t->it[i])->s); bput(o, " : I)"); }
            else bput(o, param_name(t->it[i])->s);
        }
        t = t->it[t->n - 1];
    }
    return t;
}
static void pr_alts(Buf *o, Node **alts, int n, int col, bool flat) {
    for (int i = 0; i < n; i++) {
        Node *a = alts[i];
        if (flat) { if (i) bput(o, "; "); }
        else {
            bnl(o, col);
            if (a->cmt) { char *c = a->cmt; while (*c) { char *e = strchr(c, '\n'); bput(o, "-- "); size_t l = e ? (size_t)(e - c) : strlen(c); char *ln = strndup(c, l); bput(o, ln); free(ln); bnl(o, col); c = e ? e + 1 : c + l; } a->cmt = NULL; }
        }
        if (!a->list || a->n != 2) die("print: a malformed alternative", NULL);
        Node *p = a->it[0];
        if (p->list) { for (int q = 0; q < p->n; q++) { if (q) bputc(o, ' '); bput(o, p->it[q]->s); } }
        else bput(o, p->s);
        bput(o, " → ");
        int c2 = col + vlen(o->b + (strrchr(o->b, '\n') ? strrchr(o->b, '\n') - o->b + 1 : 0)) - (col - 1);
        (void)c2;
        /* the body: on this line, or the next, indented */
        size_t ls = strrchr(o->b, '\n') ? (size_t)(strrchr(o->b, '\n') - o->b + 1) : 0;
        int here = vlen(o->b + ls) + 1;
        /* a case as the body of an alternative that others follow on its line is parenthesized */
        Node *bd = a->it[1];
        bool opencase = bd->list && bd->n >= 1 && (is(bd->it[0], "case") || is(bd->it[0], "elim"));
        char *f = pr_flat(bd, opencase && i < n - 1 ? 1 : 0);
        if (opencase && !flat && here + vlen(f) <= pr_width) { free(f); f = pr_flat(bd, 1); }
        if (flat || here + vlen(f) <= pr_width) { if (!flat && has_nl(f)) { free(f); pr(o, a->it[1], 0, here); } else { bput(o, f); free(f); } }
        else { free(f); bnl(o, col + 2); pr(o, a->it[1], 0, col + 2); }
    }
}
static int cur_col(Buf *o) { size_t ls = strrchr(o->b, '\n') ? (size_t)(strrchr(o->b, '\n') - o->b + 1) : 0; return vlen(o->b + ls) + 1; }
static void pr(Buf *o, Node *t, int prec, int col) {
    lost(t);
    if (!t->list) { pr_sym(o, t->s, false); return; }
    if (t->n == 0) die("print: an empty list", NULL);
    Node *h = t->it[0];
    /* fits on the line: print it flat */
    if (pr_width < (1 << 29)) {
        char *f = pr_flat(t, prec);
        if (!has_nl(f) && col + vlen(f) <= pr_width) { bput(o, f); free(f); return; }
        free(f);
    }
    bool flat = pr_width >= (1 << 29);
    const char *k = h->list ? "" : h->s;
#define OPEN(p) do { if (prec > (p)) { bputc(o, '('); col++; } } while (0)
#define CLOSE(p) do { if (prec > (p)) bputc(o, ')'); } while (0)
    if (!strcmp(k, "lam") || !strcmp(k, "plam")) {
        OPEN(0);
        bput(o, "λ "); Node *b = pr_binders(o, t); bput(o, " →");
        if (flat) { bputc(o, ' '); pr(o, b, 0, col); }
        else { char *f = pr_flat(b, 0); if (cur_col(o) + 1 + vlen(f) <= pr_width) { bputc(o, ' '); bput(o, f); } else { bnl(o, col + 2); pr(o, b, 0, col + 2); } free(f); }
        CLOSE(0); return;
    }
    if (!strcmp(k, "forall") || !strcmp(k, "=") || !strcmp(k, "&") || !strcmp(k, "|")) {
        if (face_has_eq(t)) { bputc(o, '('); pr_face(o, t, 0, col + 1); bputc(o, ')'); return; }
        /* a face of face variables */
        if (!strcmp(k, "&") || !strcmp(k, "|")) {
            Node *r = t->it[1];
            for (int i = 2; i < t->n; i++) r = lst(3, nsym(k), r, t->it[i]);
            OPEN(99); bput(o, k[0] == '&' ? "%fand " : "%for "); pr(o, r->it[1], 101, col); bputc(o, ' '); pr(o, r->it[2], 101, col); CLOSE(99);
            return;
        }
    }
    if (!strcmp(k, "let")) {
        OPEN(0);
        bput(o, "let ");
        Node *x = t; bool first = true;
        int ic = col + 4;
        while (x->list && x->n == 4 && is(x->it[0], "let")) {
            if (!first) { if (flat) bput(o, " in let "); else bnl(o, ic); }
            first = false;
            bput(o, x->it[1]->s); bput(o, " = ");
            pr(o, x->it[2], 0, cur_col(o));
            x = x->it[3];
            if (flat) break;
        }
        if (flat) { bput(o, " in "); pr(o, x, 0, col); }
        else { bnl(o, col); bput(o, "in "); pr(o, x, 0, col + 3); }
        CLOSE(0); return;
    }
    if (!strcmp(k, "if") && t->n == 4) {
        OPEN(0);
        bput(o, "if "); pr(o, t->it[1], 0, col + 3);
        if (flat) { bput(o, " then "); pr(o, t->it[2], 0, col); bput(o, " else "); pr(o, t->it[3], 0, col); }
        else { bnl(o, col + 2); bput(o, "then "); pr(o, t->it[2], 0, col + 7); bnl(o, col + 2); bput(o, "else "); pr(o, t->it[3], 0, col + 7); }
        CLOSE(0); return;
    }
    if (!strcmp(k, "case") || !strcmp(k, "elim")) {
        bool el = !strcmp(k, "elim");
        OPEN(0);
        bput(o, "case "); pr(o, t->it[el ? 2 : 1], 0, col + 5);
        if (el) { bput(o, " return "); pr(o, t->it[1], 0, cur_col(o)); }
        bput(o, " of");
        int from = el ? 3 : 2;
        if (t->n - from == 1 && !t->it[from]->list) { /* (case e D): no alternatives */ bput(o, " ⊥"); }
        if (flat) { bputc(o, ' '); pr_alts(o, t->it + from, t->n - from, col, true); }
        else pr_alts(o, t->it + from, t->n - from, col + 2, false);
        CLOSE(0); return;
    }
    if ((!strcmp(k, "Pi") || !strcmp(k, "Sigma"))) {
        bool pi = k[0] == 'P';
        Node *x, *A, *B;
        if (t->n == 4) { x = t->it[1]; A = t->it[2]; B = t->it[3]; }
        else if (t->n == 3 && t->it[1]->list && t->it[1]->n >= 1 && t->it[1]->it[0]->list) {
            /* a telescope */
            Node *tel = t->it[1], *r = t->it[2];
            for (int q = tel->n - 1; q >= 0; q--) r = sym4(k, tel->it[q]->it[0], tel->it[q]->it[1], r);
            pr(o, r, prec, col); return;
        }
        else { x = t->it[1]->it[0]; A = t->it[1]->it[1]; B = t->it[2]; }
        int p = pi ? 0 : 2;
        OPEN(p);
        if (mentions_x(B, x->s) || is(A, "I")) {
            bputc(o, '('); bput(o, x->s);
            /* binders of one type, together */
            while (B->list && B->n == 4 && is(B->it[0], k) && node_eq(B->it[2], A) && !mentions_x(A, B->it[1]->s) && mentions_x(B->it[3], B->it[1]->s)) { bputc(o, ' '); bput(o, B->it[1]->s); B = B->it[3]; }
            bput(o, " : "); pr(o, A, 0, cur_col(o)); bputc(o, ')');
        } else pr(o, A, p + 1, col);
        bput(o, pi ? " →" : " ×");
        char *f = pr_flat(B, p);
        if (flat || cur_col(o) + 1 + vlen(f) <= pr_width) { bputc(o, ' '); pr(o, B, p, cur_col(o)); }
        else { bnl(o, col); pr(o, B, p, col); }
        free(f);
        CLOSE(p); return;
    }
    if (!strcmp(k, "pair") && t->n == 3) {
        bputc(o, '(');
        Node *x = t; bool first = true;
        while (x->list && x->n == 3 && is(x->it[0], "pair")) {
            if (!first) { char *f = pr_flat(x->it[1], 0); if (flat || cur_col(o) + 2 + vlen(f) <= pr_width) bput(o, " , "); else { bnl(o, col); bput(o, ", "); } free(f); }
            first = false;
            pr(o, x->it[1], 0, cur_col(o));
            x = x->it[2];
        }
        { char *f = pr_flat(x, 0); if (flat || cur_col(o) + 3 + vlen(f) <= pr_width) bput(o, " , "); else { bnl(o, col); bput(o, ", "); } free(f); }
        pr(o, x, 0, cur_col(o));
        bputc(o, ')'); return;
    }
    if (!strcmp(k, "sys")) {
        bputc(o, '[');
        for (int i = 1; i < t->n; i++) {
            if (i > 1) { if (flat) bput(o, " ,"); else { bnl(o, col); bputc(o, ','); } }
            bputc(o, ' ');
            pr_face(o, t->it[i]->it[0], 0, cur_col(o)); bput(o, " ↦ ");
            pr(o, t->it[i]->it[1], 0, cur_col(o));
        }
        bput(o, t->n > 1 ? " ]" : "]"); return;
    }
    if ((!strcmp(k, "/\\") || !strcmp(k, "\\/")) && t->n >= 3) {
        int p = k[0] == '/' ? 7 : 6;
        OPEN(p);
        for (int i = 1; i < t->n; i++) {
            if (i > 1) { bputc(o, ' '); bput(o, k); bputc(o, ' '); }
            Node *a = t->it[i];
            if (!a->list && (!strcmp(a->s, "0") || !strcmp(a->s, "1"))) pr_sym(o, a->s, true); else pr(o, a, i == 1 ? p : p + 1, cur_col(o));
        }
        CLOSE(p); return;
    }
    if (!strcmp(k, "~") && t->n == 2) {
        OPEN(100);
        bput(o, "~ "); Node *a = t->it[1];
        if (!a->list) pr_sym(o, a->s, true); else pr(o, a, 101, cur_col(o));
        CLOSE(100); return;
    }
    if (!strcmp(k, "@") && t->n == 3) {
        OPEN(30);
        pr(o, t->it[1], 30, col); bput(o, " @ ");
        Node *a = t->it[2];
        if (!a->list) pr_sym(o, a->s, true); else pr(o, a, 31, cur_col(o));
        CLOSE(30); return;
    }
    if (!strcmp(k, "@") && t->n == 5) { Node *r = nlist(5); r->it[0] = nsym("%@"); for (int i = 1; i < 5; i++) r->it[i] = t->it[i]; t = r; h = r->it[0]; k = "%@"; }
    if (!strcmp(k, "U") && t->n == 1) { bput(o, "Type"); return; }
    if (!strcmp(k, "U") && t->n == 2) { Node *r = lst(2, nsym("Type"), t->it[1]); t = r; h = r->it[0]; }
    /* an application (a primitive's operands included) */
    OPEN(100);
    {
        int c0 = cur_col(o);
        if (h->list) pr(o, h, 100, c0); else bput(o, h->s);
        for (int i = 1; i < t->n; i++) {
            Node *a = t->it[i];
            char *f;
            bool iv = !h->list && ((!strcmp(h->s, "%@") && i == 2));
            if (!a->list && (!strcmp(a->s, "0") || !strcmp(a->s, "1"))) { Buf b = { 0 }; bput(&b, ""); pr_sym(&b, a->s, iv); f = b.b; }
            else f = pr_flat(a, 101);
            if (flat || (!has_nl(f) && cur_col(o) + 1 + vlen(f) <= pr_width)) { bputc(o, ' '); bput(o, f); }
            else { bnl(o, c0 + 2); if (!a->list) bput(o, f); else pr(o, a, 101, c0 + 2); }
            free(f);
        }
    }
    CLOSE(100);
#undef OPEN
#undef CLOSE
}
/* comment lines */
static void pr_cmt(Buf *o, const char *c) {
    while (c && *c) {
        const char *e = strchr(c, '\n'); size_t l = e ? (size_t)(e - c) : strlen(c);
        bput(o, l ? "-- " : "--"); char *ln = strndup(c, l); bput(o, ln); free(ln); bputc(o, '\n');
        c = e ? e + 1 : c + l;
    }
}

/* ---------------- converting a file of core forms ---------------- */
static Node *x_rename_all(Node *t, const char *a, const char *b) {
    if (!t->list) return strcmp(t->s, a) ? t : nsym(b);
    Node *r = nlist(t->n); r->cmt = t->cmt;
    for (int i = 0; i < t->n; i++) r->it[i] = x_rename_all(t->it[i], a, b);
    return r;
}
/* (def f (lam params... (case v alts))): as clauses when every alternative is a constructor with
 * variable fields (or _), v is a parameter not used in the alternatives, and no name is a constructor */
static bool clause_form(Node *body, Node **ps, int *np, int *vi, Node **cs) {
    *np = 0;
    Node *b = body;
    while (b->list && b->n >= 3 && is(b->it[0], "lam")) { for (int i = 1; i < b->n - 1; i++) { if (*np == 64) return false; ps[(*np)++] = param_name(b->it[i]); } b = b->it[b->n - 1]; }
    if (!*np || !b->list || b->n < 3 || !is(b->it[0], "case") || b->it[1]->list) return false;
    *cs = b; *vi = -1;
    for (int i = 0; i < *np; i++) {
        if (con_lookup(ps[i]->s) >= 0 || reserved(ps[i]->s)) return false;
        for (int j = 0; j < i; j++) if (!strcmp(ps[i]->s, ps[j]->s)) return false;
        if (!strcmp(ps[i]->s, b->it[1]->s)) *vi = i;
    }
    if (*vi < 0) return false;
    bool any = false;
    for (int i = 2; i < b->n; i++) {
        Node *a = b->it[i];
        if (!a->list || a->n != 2) return false;
        if (mentions_name(a->it[1], b->it[1]->s)) return false;
        Node *p = a->it[0];
        if (!p->list) { if (strcmp(p->s, "_")) return false; continue; }
        if (p->n < 1 || p->it[0]->list || con_lookup(p->it[0]->s) < 0) return false;
        any = true;
        for (int q = 1; q < p->n; q++) {
            if (p->it[q]->list || con_lookup(p->it[q]->s) >= 0 || reserved(p->it[q]->s) || !strcmp(p->it[q]->s, "_")) return false;
            for (int r = 0; r < *np; r++) if (r != *vi && !strcmp(ps[r]->s, p->it[q]->s)) return false;
            for (int r = 1; r < q; r++) if (!strcmp(p->it[r]->s, p->it[q]->s)) return false;
        }
    }
    return any;
}
static void conv_def(Buf *o, Node *f) {
    const char *name = f->it[1]->s; Node *body = f->it[2];
    Node *ps[64]; int np, vi; Node *cs;
    if (clause_form(body, ps, &np, &vi, &cs)) {
        /* the default alternative last */
        for (int pass = 0; pass < 2; pass++)
        for (int i = 2; i < cs->n; i++) {
            Node *a = cs->it[i];
            if ((pass == 0) != (a->it[0]->list != 0)) continue;
            if (a->cmt) { pr_cmt(o, a->cmt); a->cmt = NULL; }
            bput(o, name);
            for (int q = 0; q < np; q++) {
                bputc(o, ' ');
                if (q != vi) { bput(o, ps[q]->s); continue; }
                Node *p = a->it[0];
                if (!p->list) bput(o, "_");
                else if (p->n == 1) bput(o, p->it[0]->s);
                else { bputc(o, '('); for (int r = 0; r < p->n; r++) { if (r) bputc(o, ' '); bput(o, p->it[r]->s); } bputc(o, ')'); }
            }
            bput(o, " =");
            char *fl = pr_flat(a->it[1], 0);
            if (cur_col(o) + 1 + vlen(fl) <= pr_width && !has_nl(fl)) { bputc(o, ' '); bput(o, fl); }
            else { bnl(o, 3); pr(o, a->it[1], 0, 3); }
            free(fl);
            bputc(o, '\n');
        }
        return;
    }
    bput(o, name);
    Node *b = body;
    while (b->list && b->n >= 3 && is(b->it[0], "lam")) { for (int i = 1; i < b->n - 1; i++) { bputc(o, ' '); bput(o, param_name(b->it[i])->s); } b = b->it[b->n - 1]; }
    bput(o, " =");
    char *fl = pr_flat(b, 0);
    if (cur_col(o) + 1 + vlen(fl) <= pr_width && !has_nl(fl)) { bputc(o, ' '); bput(o, fl); }
    else { bnl(o, 3); pr(o, b, 0, 3); }
    free(fl);
    bputc(o, '\n');
}
static void conv_data(Buf *o, Node *f) {
    bput(o, "data ");
    Node *hd = f->it[1];
    if (!hd->list) bput(o, hd->s);
    else {
        bput(o, hd->it[0]->s);
        for (int i = 1; i < hd->n; i++) {
            Node *p = hd->it[i];
            if (p->list) { bput(o, " ("); bput(o, p->it[0]->s); bput(o, " : "); pr(o, p->it[1], 0, cur_col(o)); bputc(o, ')'); }
            else { bputc(o, ' '); bput(o, p->s); }
        }
    }
    int start = (int)o->n;
    for (int i = 2; i < f->n; i++) {
        Node *c = f->it[i];
        bput(o, i == 2 ? " = " : " | ");
        bput(o, c->it[0]->s);
        int k = 1;
        for (; k < c->n; k++) {
            Node *x = c->it[k];
            if (x->list && x->n >= 1 && is(x->it[0], "dims")) break;
            if (!x->list) { bputc(o, ' '); bput(o, x->s); }
            else { bput(o, " ("); bput(o, x->it[0]->s); bput(o, " : "); pr(o, x->it[1], 0, cur_col(o)); bputc(o, ')'); }
        }
        if (k < c->n) {
            Node *dl = c->it[k++];
            for (int q = 1; q < dl->n; q++) { bput(o, " ("); bput(o, dl->it[q]->s); bput(o, " : I)"); }
            Node *sy = nlist(c->n - k + 1); sy->it[0] = nsym("sys");
            for (int q = k; q < c->n; q++) sy->it[q - k + 1] = c->it[q];
            bputc(o, ' '); pr(o, sy, 101, cur_col(o));
        }
    }
    /* too long for a line: one constructor per line */
    if (vlen(o->b + start) + 20 > pr_width) {
        Buf b = { 0 }; bput(&b, "");
        const char *s = o->b + start;
        for (const char *p = s; *p; p++) {
            if ((p == s && !strncmp(p, " = ", 3)) || !strncmp(p, " | ", 3)) {
                /* a separator at the top level of the declaration */
                int d = 0; for (const char *q = s; q < p; q++) { if (*q == '(' || *q == '[') d++; else if (*q == ')' || *q == ']') d--; }
                if (!d) { bput(&b, "\n  "); bputc(&b, p[1]); bputc(&b, ' '); p += 2; continue; }
            }
            bputc(&b, *p);
        }
        o->n = (size_t)start; o->b[start] = 0; bput(o, b.b); free(b.b);
    }
    bputc(o, '\n');
}
/* a core form in the surface language */
static void conv_form(Buf *o, Node *f) {
    if (f->cmt) pr_cmt(o, f->cmt);
    const char *h = f->it[0]->s;
    if (!strcmp(h, "def")) { conv_def(o, f); }
    else if (!strcmp(h, "declare")) { /* a later name is declared where it is first used */ }
    else if (!strcmp(h, "data")) conv_data(o, f);
    else if (!strcmp(h, "axiom")) {
        bput(o, "postulate");
        bool typed = false; for (int i = 1; i < f->n; i++) if (f->it[i]->list) typed = true;
        if (!typed) { for (int i = 1; i < f->n; i++) { bputc(o, ' '); bput(o, f->it[i]->s); } bputc(o, '\n'); }
        else for (int i = 1; i < f->n; i++) {
            bput(o, "\n  ");
            if (!f->it[i]->list) { bput(o, f->it[i]->s); continue; }
            bput(o, f->it[i]->it[0]->s); bput(o, " : "); pr(o, f->it[i]->it[1], 0, cur_col(o));
        }
        if (typed) bputc(o, '\n');
    }
    else if (!strcmp(h, "import")) {
        char *q = strdup(f->it[1]->s[0] == '"' ? f->it[1]->s + 1 : f->it[1]->s);
        size_t l = strlen(q); if (l && q[l - 1] == '"') q[--l] = 0;
        if (l > 4 && !strcmp(q + l - 4, ".ctt")) strcpy(q + l - 4, ".cub");
        bput(o, "import \""); bput(o, q); bput(o, "\"\n");
    }
    else if (!strcmp(h, "show")) { bput(o, "#show "); pr(o, f->it[1], 0, 7); bputc(o, '\n'); }
    else if (!strcmp(h, "same")) { bput(o, "#same "); pr(o, f->it[1], 0, 7); bput(o, " = "); pr(o, f->it[2], 0, cur_col(o)); bputc(o, '\n'); }
    else if (!strcmp(h, "check") || !strcmp(h, "check-fail")) {
        if (h[5]) bput(o, "#fail\n");
        bput(o, "#check "); pr(o, f->it[2], 0, 8); bput(o, " : "); pr(o, f->it[1], 0, cur_col(o)); bputc(o, '\n');
    }
    else if (!strcmp(h, "check-data")) { bput(o, "#check-data "); bput(o, f->it[1]->s); bputc(o, '\n'); }
    else if (!strcmp(h, "input")) { bput(o, "#input"); for (int i = 1; i < f->n; i++) { bputc(o, ' '); bput(o, f->it[i]->s); } bputc(o, '\n'); }
    else if (!strcmp(h, "supply")) { bput(o, "#supply "); bput(o, f->it[1]->s); bput(o, " = "); pr(o, f->it[2], 0, cur_col(o)); bputc(o, '\n'); }
    else if (!strcmp(h, "observe")) { bput(o, "#observe "); bput(o, f->it[1]->s); bputc(o, ' '); pr(o, f->it[2], 0, cur_col(o)); bputc(o, '\n'); }
    else { bput(o, "-- (not converted) "); char *b; size_t l; FILE *m = open_memstream(&b, &l); print_node(m, f); fclose(m); bput(o, b); free(b); bputc(o, '\n'); }
    if (pr_lost.n) { Buf c = { 0 }; bput(&c, ""); pr_cmt(&c, pr_lost.b); /* comments from inside the form, after it */ bput(o, c.b); free(c.b); pr_lost.n = 0; pr_lost.b[0] = 0; }
}
/* alpha-equivalence of desugared terms */
typedef struct { const char *a, *b; } APair;
static bool alpha(Node *x, Node *y, APair *env, int n) {
    if (x->list != y->list) return false;
    if (!x->list) {
        for (int i = n - 1; i >= 0; i--) {
            bool ma = !strcmp(env[i].a, x->s), mb = !strcmp(env[i].b, y->s);
            if (ma || mb) return ma && mb;
        }
        return !strcmp(x->s, y->s);
    }
    if (x->n != y->n) return false;
    if (x->n >= 3 && !x->it[0]->list && !y->it[0]->list && !strcmp(x->it[0]->s, y->it[0]->s)) {
        const char *k = x->it[0]->s;
        if ((!strcmp(k, "lam") || !strcmp(k, "plam") || !strcmp(k, "forall")) && x->n == 3) {
            env[n] = (APair){ x->it[1]->s, y->it[1]->s };
            return alpha(x->it[2], y->it[2], env, n + 1);
        }
        if (!strcmp(k, "let") && x->n == 4) {
            if (!alpha(x->it[2], y->it[2], env, n)) return false;
            env[n] = (APair){ x->it[1]->s, y->it[1]->s };
            return alpha(x->it[3], y->it[3], env, n + 1);
        }
    }
    for (int i = 0; i < x->n; i++) if (!alpha(x->it[i], y->it[i], env, n)) return false;
    return true;
}
static Node **xcollect; static int nxcollect, capxcollect; static bool x_collecting;
static void x_top(Node *f) {
    if (x_collecting) { GROW(xcollect, capxcollect, (size_t)nxcollect + 1); xcollect[nxcollect++] = f; return; }
    toplevel(f);
}
/* print a file of core forms in the surface language; with check, each form's printing is parsed
 * back and compared with it (desugared, up to the names of bound variables) as the form is loaded */
static void convert_file(const char *path, bool check) {
    char *text; { FILE *f = fopen(path, "rb"); if (!f) die("cannot open", path); fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET); text = malloc((size_t)n + 1); text[fread(text, 1, (size_t)n, f)] = 0; fclose(f); }
    const char *saved = src; src = text; keep_cmt = true;
    Node *f; Node **fs = NULL; size_t cap = 0; int nf = 0;
    while ((f = read_node())) { GROW(fs, cap, (size_t)nf + 1); fs[nf++] = f; }
    keep_cmt = false; src = saved;
    bput(&pr_lost, "");
    /* a name that is a word of the surface language is renamed */
    for (int i = 0; i < nf; i++) fs[i] = x_rename_all(fs[i], "then", "and-then");
    bool saved_ut = x_untyped_file; x_untyped_file = global_lookup("chk-def") < 0;
    int bad = 0;
    for (int i = 0; i < nf; i++) {
        Buf o = { 0 }; bput(&o, "");
        conv_form(&o, fs[i]);
        if (!check) fputs(o.b, stdout);
        else if (!is(fs[i]->it[0], "declare")) {
            /* parse it back */
            nxcollect = 0; x_collecting = true;
            load_surface(o.b, "<printed>", false);
            x_collecting = false;
            Node *g = NULL;
            for (int k = 0; k < nxcollect; k++) if (!is(xcollect[k]->it[0], "declare")) g = xcollect[k];
            bool same = false;
            if (getenv("X_TRACE")) fprintf(stderr, "form %d %s\n", i, fs[i]->n > 1 && !fs[i]->it[1]->list ? fs[i]->it[1]->s : "");
            if (g && is(fs[i]->it[0], "def") && is(g->it[0], "def")) {
                APair env[4096];
                same = !strcmp(fs[i]->it[1]->s, g->it[1]->s) && alpha(ds(fs[i]->it[2]), ds(g->it[2]), env, 0);
            } else if (g) same = node_eq(fs[i], g);
            if (!same && g && is(fs[i]->it[0], "def")) {
                /* the first subterm that differs */
                Node *a = ds(fs[i]->it[2]), *b = ds(g->it[2]);
                for (int depth = 0; depth < 200 && a->list && b->list && a->n == b->n; depth++) {
                    int k = -1; APair env[4096];
                    for (int q = 0; q < a->n; q++) if (!alpha(a->it[q], b->it[q], env, 0)) { k = q; break; }
                    if (k < 0) break;
                    a = a->it[k]; b = b->it[k];
                }
                printf("FIRST: "); print_node(stdout, a); printf("\n  VS: "); print_node(stdout, b); printf("\n");
            }
            if (!same) {
                bad++;
                printf("DIFFERS: "); print_node(stdout, fs[i]); printf("\n  printed: %s  parsed: ", o.b);
                if (g) print_node(stdout, g); printf("\n");
            }
        }
        free(o.b);
        if (check) toplevel(fs[i]);
    }
    if (check) printf("%s: %d forms, %d differ\n", path, nf, bad);
    x_untyped_file = saved_ut;
}

/* ---------------- codes read back as core terms ----------------
 * A code (lib/code.cub) as printed: de Bruijn variables over the binders entered, checked
 * definitions and data types by index, a free variable by its source name. NULL: no term (a
 * variable without a name, made by the checker inside the solution). */
static const char **cn_names; static int ncn, capcn; static int cn_fresh;
static Node *cn_push(const char *p) { char b[32]; snprintf(b, sizeof b, "%%%s%d", p, cn_fresh++); GROW(cn_names, capcn, (size_t)ncn + 1); cn_names[ncn++] = strdup(b); return nsym(cn_names[ncn - 1]); }
static long cn_word(Node *w) { return !w->list && w->s[0] == '#' ? atol(w->s + 1) : -1; }
static int cn_list(Node *l, Node **out, int max) {
    int n = 0;
    while (l->list && l->n == 3 && is(l->it[0], "lcons")) { if (n == max) return -1; out[n++] = l->it[1]; l = l->it[2]; }
    return is(l, "lnil") ? n : -1;
}
static Node *cn(Node *c);
static Node *cn_bound(const char *p, Node *body, Node **x) { *x = cn_push(p); Node *r = cn(body); ncn--; return r; }
static Node *cn(Node *c) {
    if (!c->list) {
        const char *s = c->s;
        static const struct { const char *c, *t; } K0[] = { { "ci0", "0" }, { "ci1", "1" }, { "cftt", "tt" }, { "cfff", "ff" }, { "clevel", "Level" },
            { "clzero", "lzero" }, { "cword", "Word" }, { "csys0", "%sys0" }, { NULL, NULL } };
        for (int i = 0; K0[i].c; i++) if (!strcmp(s, K0[i].c)) return !strcmp(K0[i].t, "%sys0") ? lst(1, nsym("sys")) : nsym(K0[i].t);
        return NULL;
    }
    const char *h = c->it[0]->s;
    Node *x, *a, *b, *d;
#define R(e) do { Node *r_ = (e); if (!r_) return NULL; } while (0)
    if (!strcmp(h, "cvar")) { long k = cn_word(c->it[1]); if (k < 0 || k >= ncn) return NULL; const char *nm = cn_names[ncn - 1 - k]; return !strncmp(nm, "%rec ", 5) ? lst(2, nsym("rec"), nsym(nm + 5)) : nsym(nm); }
    if (!strcmp(h, "cforce") && c->it[1]->list && is(c->it[1]->it[0], "cvar")) return cn(c->it[1]);
    if (!strcmp(h, "celim") || !strcmp(h, "ccase")) {
        /* an elimination: each branch binds its fields, a hypothesis per recursive field (written
         * (rec x)), its dimensions */
        bool el = h[1] == 'e';
        long di = cn_word(c->it[1]); if (di < 0 || di >= nchkdata) return NULL;
        int D = CHKDATA[di];
        Node *P = el ? cn(c->it[2]) : NULL; if (el && !P) return NULL;
        Node *x = cn(c->it[el ? 4 : 3]); if (!x) return NULL;
        Node *bs[64]; int nb = cn_list(c->it[el ? 3 : 2], bs, 64); if (nb != DATA[D].ncons) return NULL;
        Node *r = nlist(nb + (el ? 3 : 2)); int k = 0;
        r->it[k++] = nsym(el ? "elim" : "case"); if (el) r->it[k++] = P; r->it[k++] = x;
        for (int i = 0; i < nb; i++) {
            ConInfo *ci = &CON[DATA[D].cons[i]];
            int nf = ci->nfields, nr = ci->nrec, ndm = ci->ndims, save = ncn;
            Node *pat = nlist(nf + ndm + 1); pat->it[0] = nsym(ci->name);
            Node *b = bs[i]; const char *recf[64]; int nrf = 0;
            for (int j = 0; j < nf + nr + ndm; j++) {
                if (!b->list || b->n != 2 || !(is(b->it[0], "clam") || is(b->it[0], "cdlam"))) { ncn = save; return NULL; }
                if (j < nf) { Node *xn = cn_push("q"); pat->it[j + 1] = xn; if (ci->recmask >> j & 1) recf[nrf++] = xn->s; }
                else if (j < nf + nr) { char nm[64]; snprintf(nm, sizeof nm, "%%rec %s", j - nf < nrf ? recf[j - nf] : "?"); GROW(cn_names, capcn, (size_t)ncn + 1); cn_names[ncn++] = strdup(nm); }
                else pat->it[nf + (j - nf - nr) + 1] = cn_push("j");
                b = b->it[1];
            }
            Node *body = cn(b);
            ncn = save;
            if (!body) return NULL;
            r->it[k++] = lst(2, pat, body);
        }
        return r;
    }
    if (!strcmp(h, "cfree")) { Node *v = c->it[1]; return !v->list && !(v->s[0] == 'v' && isdigit((unsigned char)v->s[1])) && strcmp(v->s, "?") ? v : NULL; }
    if (!strcmp(h, "cglob")) { long k = cn_word(c->it[1]); return k >= 0 && k < nchecked && !CHKHIDE[k] ? nsym(CHECKED[k]) : NULL; }
    if (!strcmp(h, "clam") || !strcmp(h, "cdlam")) { b = cn_bound("q", c->it[1], &x); return b ? sym3("lam", x, b) : NULL; }
    if (!strcmp(h, "cplam")) { b = cn_bound("j", c->it[1], &x); return b ? sym3("plam", x, b) : NULL; }
    if (!strcmp(h, "cforall")) { b = cn_bound("j", c->it[1], &x); return b ? sym3("forall", x, b) : NULL; }
    if (!strcmp(h, "cpi") || !strcmp(h, "csigma")) { a = cn(c->it[1]); if (!a) return NULL; b = cn_bound("q", c->it[2], &x); return b ? sym4(h[1] == 'p' ? "Pi" : "Sigma", x, a, b) : NULL; }
    if (!strcmp(h, "clet")) { a = cn(c->it[1]); if (!a) return NULL; b = cn_bound("q", c->it[2], &x); return b ? sym4("let", x, a, b) : NULL; }
    if (!strcmp(h, "cuniv")) { a = cn(c->it[1]); if (!a) return NULL; return is(a, "lzero") ? nsym("U") : sym2("U", a); }
    if (!strcmp(h, "cdata")) {
        long di = cn_word(c->it[1]); if (di < 0 || di >= nchkdata) return NULL;
        Node *ps[64]; int n = cn_list(c->it[2], ps, 64); if (n < 0) return NULL;
        if (!n) return nsym(DATA[CHKDATA[di]].name);
        Node *r = nlist(n + 1); r->it[0] = nsym(DATA[CHKDATA[di]].name);
        for (int i = 0; i < n; i++) { R(r->it[i + 1] = cn(ps[i])); }
        return r;
    }
    if (!strcmp(h, "ccon")) {
        long di = cn_word(c->it[1]), k = cn_word(c->it[2]); if (di < 0 || di >= nchkdata || k < 0 || k >= DATA[CHKDATA[di]].ncons) return NULL;
        Node *fs[64], *ds_[16]; int n = cn_list(c->it[3], fs, 64), m = cn_list(c->it[4], ds_, 16); if (n < 0 || m < 0) return NULL;
        const char *cname = CON[DATA[CHKDATA[di]].cons[k]].name;
        if (!n && !m) return nsym(cname);
        Node *r = nlist(n + m + 1); r->it[0] = nsym(cname);
        for (int i = 0; i < n; i++) { R(r->it[i + 1] = cn(fs[i])); }
        for (int i = 0; i < m; i++) { R(r->it[n + i + 1] = cn(ds_[i])); }
        return r;
    }
    if (!strcmp(h, "cproj")) {
        long di = cn_word(c->it[1]), k = cn_word(c->it[2]); if (di < 0 || di >= nchkdata || k < 0 || k >= DATA[CHKDATA[di]].nflds) return NULL;
        a = cn(c->it[3]); return a ? lst(2, nsym(FLD[DATA[CHKDATA[di]].flds[k]].name), a) : NULL;
    }
    if (!strcmp(h, "ceq0") || !strcmp(h, "ceq1")) { a = cn(c->it[1]); return a ? sym3("=", a, nsym(h[3] == '0' ? "0" : "1")) : NULL; }
    if (!strcmp(h, "csysc")) {
        /* a system: its branches in order */
        Node *bs[64]; int n = 0; Node *s = c;
        while (s->list && is(s->it[0], "csysc")) { if (n == 64) return NULL; a = cn(s->it[1]); b = cn(s->it[2]); if (!a || !b) return NULL; bs[n++] = lst(2, a, b); s = s->it[3]; }
        if (!is(s, "csys0")) return NULL;
        Node *r = nlist(n + 1); r->it[0] = nsym("sys");
        for (int i = 0; i < n; i++) r->it[i + 1] = bs[i];
        return r;
    }
    static const struct { const char *c, *t; int n; } F[] = {
        { "capp", "", 2 }, { "cpair", "pair", 2 }, { "cfst", "fst", 1 }, { "csnd", "snd", 1 }, { "cpathp", "PathP", 3 },
        { "cpapp", "@", 2 }, { "cneg", "~", 1 }, { "cmeet", "/\\", 2 }, { "cjoin", "\\/", 2 }, { "cfand", "&", 2 }, { "cfor", "|", 2 },
        { "ccomp", "comp", 3 }, { "cgluet", "Glue", 2 }, { "cglue", "glue", 2 }, { "cunglue", "unglue", 2 }, { "chcomp", "hcompD", 2 },
        { "cforce", "force", 1 }, { "clsuc", "lsuc", 1 }, { "clmax", "lmax", 2 }, { "cann", "the", 2 }, { NULL, NULL, 0 } };
    for (int q = 0; F[q].c; q++) if (!strcmp(h, F[q].c)) {
        if (c->n != F[q].n + 1) return NULL;
        Node *r = nlist(F[q].n + (F[q].t[0] ? 1 : 0)); int k = 0;
        if (F[q].t[0]) r->it[k++] = nsym(F[q].t);
        for (int i = 0; i < F[q].n; i++) { d = cn(c->it[i + 1]); if (!d) return NULL; r->it[k++] = d; }
        if (!strcmp(h, "cann")) { Node *t = r->it[1]; r->it[1] = r->it[2]; r->it[2] = t; }
        return r;
    }
#undef R
    return NULL;
}

/* ---------------- elaboration ----------------
 * A term of a checked definition becomes a core term in which each implicit argument is a hole
 * (%hole #k) inside the application holding it (%spine ...), and each λ or application whose kind
 * the syntax does not tell is a guess (%guess #k #g t). The checker finds the holes and tells the
 * wrong guesses; holes and guesses are keyed by where they stand in the source, so a decision holds
 * from one run to the next. */
typedef struct { char *name; uint64_t m; int len; } IMask;
static IMask *IM; static int nim, capim;
static IMask *imask_of(const char *s) { for (int i = nim - 1; i >= 0; i--) if (!strcmp(IM[i].name, s)) return IM[i].m ? &IM[i] : NULL; return NULL; }
static void imask_set(const char *s, uint64_t m, int len) { GROW(IM, capim, (size_t)nim + 1); IM[nim++] = (IMask){ strdup(s), m, len }; }
typedef struct { Node *key; int idx; Node *sol; } EHole;
typedef struct { Node *key; int kind; } EGuess;
static EHole *EH; static int neh, capeh;
static EGuess *EG; static int neg, capeg;
static int e_hole(Node *key, int idx) {
    for (int i = 0; i < neh; i++) if (EH[i].key == key && EH[i].idx == idx) return i;
    GROW(EH, capeh, (size_t)neh + 1); EH[neh] = (EHole){ key, idx, NULL }; return neh++;
}
static int e_guess(Node *key, int deflt) {
    for (int i = 0; i < neg; i++) if (EG[i].key == key) return i;
    GROW(EG, capeg, (size_t)neg + 1); EG[neg] = (EGuess){ key, deflt }; return neg++;
}
static Node *wordn(long n) { char b[32]; snprintf(b, sizeof b, "#%ld", n); return nsym(b); }
/* the scope: local names, each a term or a dimension variable */
typedef struct { const char *n; bool dim; } Sc;
static Sc *esc; static int nesc, capesc;
static void e_push(const char *n, bool dim) { GROW(esc, capesc, (size_t)nesc + 1); esc[nesc++] = (Sc){ n, dim }; }
static Sc *e_find(const char *n) { for (int i = nesc - 1; i >= 0; i--) if (!strcmp(esc[i].n, n)) return &esc[i]; return NULL; }
static bool e_head_kw(const char *s) {
    return keyword(s) || prim_ix(s) >= 0 || !strcmp(s, "%impPi") || !strcmp(s, "%num") || !strcmp(s, "%imp") || !strcmp(s, "%lam") || !strcmp(s, "sys");
}
/* does x stand where an interval does, in a surface term? */
static bool dim_use(Node *t, const char *x) {
    if (!t->list || t->n == 0) return false;
    Node *h = t->it[0];
    if (!h->list) {
        const char *k = h->s;
        if (((!strcmp(k, "lam") || !strcmp(k, "plam") || !strcmp(k, "%lam") || !strcmp(k, "forall")) && t->n == 3 && !strcmp(t->it[1]->s, x))) return false;
        bool iv_all = !strcmp(k, "~") || !strcmp(k, "/\\") || !strcmp(k, "\\/");
        for (int i = 1; i < t->n; i++) {
            Node *a = t->it[i];
            bool iv = iv_all || (!strcmp(k, "=") && i == 1) || (!strcmp(k, "@") && i == 2);
            int c = con_lookup(k); if (c >= 0 && !e_find(k) && i - 1 >= CON[c].nfields) iv = true;
            if (iv && !a->list && !strcmp(a->s, x)) return true;
            if (dim_use(a, x)) return true;
        }
        return false;
    }
    for (int i = 0; i < t->n; i++) if (dim_use(t->it[i], x)) return true;
    return false;
}
static bool iv_certain(Node *a) {
    if (!a->list) { if (!strcmp(a->s, "0") || !strcmp(a->s, "1")) return true; Sc *s = e_find(a->s); return s && s->dim; }
    return a->n >= 1 && !a->it[0]->list && (is(a->it[0], "~") || is(a->it[0], "/\\") || is(a->it[0], "\\/"));
}
static bool lit01(Node *a) { return a->list && a->n == 2 && is(a->it[0], "%num") && (is(a->it[1], "0") || is(a->it[1], "1")); }
static Node *el(Node *t);
static Node *hole_or_sol(Node *key, int idx, bool *pend) {
    int k = e_hole(key, idx);
    if (EH[k].sol) return EH[k].sol;
    *pend = true;
    return sym2("%hole", wordn(k));
}
/* an application: the head's implicit arguments inserted (eagerly, up to the next explicit one),
 * each operand a function's or a path's by what it is */
static Node *el_app(Node *t, Node *h, Node **args, int n) {
    Node *out[512]; Node *orig[512]; int no = 0;
    bool pend = false;
    Node *hd;
    IMask *im = !h->list && !e_find(h->s) ? imask_of(h->s) : NULL;
    if (im) {
        hd = h;
        int pos = 0, i = 0;
        while (pos < im->len && no < 500) {
            if (im->m >> pos & 1) {
                if (i < n && args[i]->list && args[i]->n == 2 && is(args[i]->it[0], "%imp")) { orig[no] = args[i]->it[1]; out[no++] = el(args[i]->it[1]); i++; }
                else { orig[no] = NULL; out[no++] = hole_or_sol(t, pos, &pend); }
            } else {
                if (i >= n) break;
                if (args[i]->list && args[i]->n == 2 && is(args[i]->it[0], "%imp")) die("an implicit argument {..} where an explicit one is due, applying", h->s);
                orig[no] = args[i]; out[no++] = el(args[i]); i++;
            }
            pos++;
        }
        for (; i < n && no < 500; i++) {
            if (args[i]->list && args[i]->n == 2 && is(args[i]->it[0], "%imp")) die("an implicit argument {..} beyond the implicit ones of", h->s);
            orig[no] = args[i]; out[no++] = el(args[i]);
        }
    } else {
        hd = el(h);
        for (int i = 0; i < n && no < 500; i++) {
            if (args[i]->list && args[i]->n == 2 && is(args[i]->it[0], "%imp")) die("an implicit argument {..} applied to something without implicit arguments", NULL);
            orig[no] = args[i]; out[no++] = !args[i]->list && !strcmp(args[i]->s, "%hole") ? hole_or_sol(args[i], 0, &pend) : el(args[i]);
        }
    }
    bool is_con = !h->list && !e_find(h->s) && (con_lookup(h->s) >= 0 || fld_lookup(h->s) >= 0);
    Node *cur = hd; bool sp = pend; pend = false;
    (void)sp;
    /* holes among the arguments so far make the spine one to solve */
    bool holes = false;
    for (int i = 0; i < no; i++) {
        Node *a = out[i];
        if (a->list && a->n == 2 && is(a->it[0], "%hole")) holes = true;
        int kind = 0;   /* 0 application, 1 path application, 2 a guess */
        if (!is_con) { if (iv_certain(a)) kind = 1; else if (orig[i] && lit01(orig[i])) kind = 2; }
        if (kind && holes) { cur = sym2("%spine", cur); holes = false; }
        if (kind == 0) cur = lst(2, cur, a);
        else if (kind == 1) cur = sym3("@", cur, a);
        else {
            int g = e_guess(orig[i], 2);
            cur = EG[g].kind == 3 ? sym4("%guess", wordn(g), wordn(3), sym3("@", cur, a)) : sym4("%guess", wordn(g), wordn(2), lst(2, cur, a));
        }
    }
    if (holes) cur = sym2("%spine", cur);
    return cur;
}
static Node *el_alts(Node *t, int from) {
    Node *r = nlist(t->n); r->it[0] = t->it[0];
    for (int i = 1; i < from; i++) r->it[i] = el(t->it[i]);
    for (int i = from; i < t->n; i++) {
        Node *a = t->it[i];
        if (!a->list || a->n != 2) { r->it[i] = a; continue; }
        int save = nesc;
        Node *p = a->it[0];
        if (p->list) { for (int q = 1; q < p->n; q++) e_push(p->it[q]->s, CON[con_lookup(p->it[0]->s)].nfields < q); }
        else e_push(p->s, false);
        r->it[i] = lst(2, p, el(a->it[1]));
        nesc = save;
    }
    return r;
}
static bool rt_is_path(Node *R);
static Node *el(Node *t) {
    if (!t->list) {
        if (!strcmp(t->s, "%hole")) { bool p = false; Node *h = hole_or_sol(t, 0, &p); return p ? sym2("%spine", h) : h; }
        if (e_find(t->s)) return t;
        if (imask_of(t->s)) return el_app(t, t, NULL, 0);
        return t;
    }
    if (t->n == 0) return t;
    Node *h = t->it[0];
    if (h->list || !e_head_kw(h->s)) return el_app(t, h, t->it + 1, t->n - 1);
    const char *k = h->s;
    if (!strcmp(k, "%num")) return t;
    if (!strcmp(k, "%imp")) die("an implicit argument {..} outside an application", NULL);
    if (!strcmp(k, "%impPi")) return el(t->it[1]);
    if (!strcmp(k, "%lam")) { int save = nesc; e_push(t->it[1]->s, false); Node *b = el(t->it[2]); nesc = save; return sym3("lam", t->it[1], b); }
    if (!strcmp(k, "lam") && t->n == 3) {
        bool certain = dim_use(t->it[2], t->it[1]->s);
        int g = certain ? -1 : e_guess(t, 0);
        bool dim = certain || (g >= 0 && EG[g].kind == 1);
        int save = nesc; e_push(t->it[1]->s, dim); Node *b = el(t->it[2]); nesc = save;
        Node *r = sym3(dim ? "plam" : "lam", t->it[1], b);
        return g >= 0 ? sym4("%guess", wordn(g), wordn(EG[g].kind), r) : r;
    }
    if ((!strcmp(k, "plam") || !strcmp(k, "forall")) && t->n == 3) { int save = nesc; e_push(t->it[1]->s, true); Node *b = el(t->it[2]); nesc = save; return sym3(k, t->it[1], b); }
    if ((!strcmp(k, "Pi") || !strcmp(k, "Sigma")) && t->n == 4) {
        Node *A = el(t->it[2]); int save = nesc; e_push(t->it[1]->s, false); Node *B = el(t->it[3]); nesc = save;
        return sym4(k, t->it[1], A, B);
    }
    if (!strcmp(k, "let") && t->n == 4) { Node *e = el(t->it[2]); int save = nesc; e_push(t->it[1]->s, false); Node *b = el(t->it[3]); nesc = save; return sym4("let", t->it[1], e, b); }
    if (!strcmp(k, "case")) return el_alts(t, 2);
    if (!strcmp(k, "elim")) return el_alts(t, 3);
    if (!strcmp(k, "=")) return sym3("=", el(t->it[1]), t->it[2]);
    if (!strcmp(k, "sys")) {
        Node *r = nlist(t->n); r->it[0] = h;
        for (int i = 1; i < t->n; i++) r->it[i] = lst(2, el(t->it[i]->it[0]), el(t->it[i]->it[1]));
        return r;
    }
    Node *r = nlist(t->n); r->it[0] = h;
    for (int i = 1; i < t->n; i++) r->it[i] = el(t->it[i]);
    return r;
}
static bool rt_is_path(Node *R) {
    if (!R->list || R->n < 1 || R->it[0]->list) return false;
    return is(R->it[0], "PathP") || is(R->it[0], "_≡_");
}

/* ---------------- checked definitions by clauses ----------------
 * The signature's telescope gives the parameters (their names, types, which are implicit) and the
 * result type R. A split on a variable v is the elimination of v with the motive
 *     λ v → let (the variables rebuilt from v) in Π (the later parameters) → R,
 * applied to the later parameters; its alternative for a constructor c binds c's fields, rebuilds
 * v (and what was rebuilt from v) by let, and binds the later parameters again. A recursive call on
 * a recursive field of a split is that split's induction hypothesis (rec x), applied to the later
 * parameters. */
typedef struct { const char *v; Node *def; int gp; Node *ty; } CVar;   /* a variable rebuilt by a let, its type */
typedef struct { const char *v; int gp; Node *E; Node **rf; int nrf; } Frame;   /* a split's IH */
static const char *tfname;
static CVar *cdefs; static int ncdefs, capcdefs;
static Frame *frames; static int nframes, capframes;
static Node *subst_sym(Node *t, const char *x, Node *e);
static Node *ty_of(const char *v) { for (int i = ncdefs - 1; i >= 0; i--) if (!strcmp(cdefs[i].v, v)) return cdefs[i].ty; for (int i = 0; i < tsig.np; i++) if (!strcmp(tsig.pn[i]->s, v)) return tsig.pty[i]; return NULL; }
/* the types of constructor c's fields, the variable it splits being of type T (D at its parameters):
 * the declaration's field types, its parameters and earlier fields replaced */
static Node **field_types(int c, Node *T, Node **fv) {
    int d = CON[c].data, nf = CON[c].nfields;
    Node **r = calloc((size_t)nf + 1, sizeof(Node *));
    Node *dd = DATA[d].decl;
    if (!T || !dd) return r;
    Node *hd = T; Node *args[64]; int na = 0;
    if (T->list) { hd = T->it[0]; for (int i = 1; i < T->n && na < 64; i++) args[na++] = T->it[i]; }
    if (hd->list || strcmp(hd->s, DATA[d].name) || na != DATA[d].nparams) return r;
    Node *cd = dd->it[2 + CON[c].index];
    Node *dh = dd->it[1];
    for (int q = 0; q < nf; q++) {
        Node *f = cd->it[q + 1];
        if (!f->list || f->n != 2) return r;
        Node *A = f->it[1];
        for (int k = 0; k < na; k++) A = subst_sym(A, param_name(dh->it[k + 1])->s, args[k]);
        for (int k = 0; k < q; k++) { Node *pf = cd->it[k + 1]; A = subst_sym(A, pf->it[0]->s, fv[k]); }
        r[q] = A;
    }
    return r;
}
static int gp_of(const char *v) { for (int i = ncdefs - 1; i >= 0; i--) if (!strcmp(cdefs[i].v, v)) return cdefs[i].gp; for (int i = 0; i < tsig.np; i++) if (!strcmp(tsig.pn[i]->s, v)) return i; return -1; }
static Node *subst_sym(Node *t, const char *x, Node *e) {
    if (!t->list) return !strcmp(t->s, x) ? e : t;
    if (t->n >= 3 && !t->it[0]->list && (is(t->it[0], "lam") || is(t->it[0], "plam") || is(t->it[0], "%lam")) && !strcmp(t->it[1]->s, x)) return t;
    if (t->n == 4 && !t->it[0]->list && is(t->it[0], "let") && !strcmp(t->it[1]->s, x)) return sym4("let", t->it[1], subst_sym(t->it[2], x, e), t->it[3]);
    Node *r = nlist(t->n);
    for (int i = 0; i < t->n; i++) r->it[i] = subst_sym(t->it[i], x, e);
    return r;
}
/* an expression with the rebuilt variables replaced by what they are */
static Node *expand(Node *e) {
    for (int k = 0; k < 64; k++) {
        Node *r = e;
        for (int i = ncdefs - 1; i >= 0; i--) if (cdefs[i].def) r = subst_sym(r, cdefs[i].v, cdefs[i].def);
        if (node_eq(r, e)) return r;
        e = r;
    }
    return e;
}
static Node *rec_xform(Node *e);
static Node *rec_call(Node *e, Node **args, int n) {
    /* the call's arguments at the parameters' positions (implicit ones not given) */
    Node *at[256] = { 0 }; int i = 0;
    for (int q = 0; q < tsig.np && i <= n; q++) {
        if (tsig.imp[q]) { if (i < n && args[i]->list && args[i]->n == 2 && is(args[i]->it[0], "%imp")) at[q] = args[i++]->it[1]; continue; }
        if (i < n) at[q] = args[i++];
    }
    if (i < n) die("a recursive call with more arguments than parameters:", tfname);
    for (int s = nframes - 1; s >= 0; s--) {
        Frame *f = &frames[s];
        int p = f->gp;
        bool ok = at[p] != NULL;
        for (int q = 0; q < p && ok; q++) if (at[q] && !(!at[q]->list && !strcmp(at[q]->s, tsig.pn[q]->s))) ok = false;
        if (!ok) continue;
        Node *want = expand(rec_xform(at[p]));
        for (int r = 0; r < f->nrf; r++) {
            Node *cand = expand(subst_sym(f->E, f->v, f->rf[r]));
            if (!node_eq(cand, want)) continue;
            /* the hypothesis at the later parameters */
            int nl = 0; for (int q = p + 1; q < tsig.np; q++) if (tsig.rev[q]) nl++;
            Node *call = lst(2, nsym("rec"), f->rf[r]);
            if (!nl) return call;
            Node *ap = nlist(nl + 1); ap->it[0] = call; int k = 1;
            for (int q = p + 1; q < tsig.np; q++) if (tsig.rev[q]) {
                if (!at[q] && !tsig.imp[q]) die("a recursive call must give every argument after the one it recurses on:", tfname);
                ap->it[k++] = at[q] ? rec_xform(at[q]) : nsym("%hole");
            }
            return ap;
        }
    }
    (void)e;
    char m[256]; snprintf(m, sizeof m, "%s: a recursive call is not on a constructor field from one of the definition's own matches (structural recursion), in", tfname);
    print_node(stderr, e); fputc('\n', stderr);
    die(m, tfname);
    return NULL;
}
static Node *rec_xform(Node *e) {
    if (!e->list) { if (!strcmp(e->s, tfname) && !e_find(e->s)) return rec_call(e, NULL, 0); return e; }
    if (e->n == 0) return e;
    Node *h = e->it[0];
    if (!h->list && !strcmp(h->s, tfname)) {
        Node **a = calloc((size_t)e->n, sizeof(Node *));
        for (int i = 1; i < e->n; i++) a[i - 1] = e->it[i];
        return rec_call(e, a, e->n - 1);
    }
    if (!h->list && (is(h, "lam") || is(h, "plam") || is(h, "%lam")) && e->n == 3 && !strcmp(e->it[1]->s, tfname)) return e;
    Node *r = nlist(e->n);
    for (int i = 0; i < e->n; i++) r->it[i] = rec_xform(e->it[i]);
    return r;
}
/* the later parameters (after p) as Π types around R; as λs; as arguments */
/* the later parameters a split reverts: those the recursive calls change, or whose types depend on an
 * earlier parameter */
static Node *later_pi(int p) { Node *r = tsig.R; for (int q = tsig.np - 1; q > p; q--) if (tsig.rev[q]) r = sym4("Pi", tsig.pn[q], tsig.pty[q], r); return r; }
static Node *later_lam(int p, Node *b) { for (int q = tsig.np - 1; q > p; q--) if (tsig.rev[q]) b = sym3("%lam", tsig.pn[q], b); return b; }
static Node *later_app(int p, Node *f) {
    int nl = 0; for (int q = p + 1; q < tsig.np; q++) if (tsig.rev[q]) nl++;
    if (!nl) return f;
    Node *r = nlist(nl + 1); r->it[0] = f; int k = 1;
    for (int q = p + 1; q < tsig.np; q++) if (tsig.rev[q]) r->it[k++] = tsig.pn[q];
    return r;
}
static bool mentions_any(Node *t, const char *v) { return mentions_name(t, v); }
/* a rebuilt variable's value, annotated with its type when it is known */
static Node *typed_def(CVar *c) { return c->ty ? sym3("the", c->ty, c->def) : c->def; }
static Node *ccm_t(Node **vars, int nv, Row *rows, int nr);
static Node *ccm_t(Node **vars, int nv, Row *rows, int nr) {
    if (!nr) die("the clauses do not cover every case of", tfname);
    int j = -1;
    for (int q = 0; q < nv; q++) if (!pat_wild(rows[0].p[q]) && pat_con(rows[0].p[q]) >= 0) { j = q; break; }
    if (j < 0) {
        Node *e = rows[0].rhs;
        for (int q = 0; q < nv; q++) if (!pat_wild(rows[0].p[q]) && strcmp(rows[0].p[q]->s, vars[q]->s)) e = x_rename(e, rows[0].p[q]->s, vars[q]->s);
        return rec_xform(e);
    }
    const char *v = vars[j]->s;
    int gp = gp_of(v);
    if (gp < 0) die("internal: a split on an unknown variable", v);
    int d = CON[pat_con(rows[0].p[j])].data, N = DATA[d].ncons;
    /* the rebuilt variables that depend on v, in order */
    int dep[256], nd = 0;
    for (int i = 0; i < ncdefs; i++) {
        if (!cdefs[i].def) continue;
        bool m = mentions_any(cdefs[i].def, v);
        for (int q = 0; q < nd && !m; q++) if (mentions_any(cdefs[i].def, cdefs[dep[q]].v)) m = true;
        if (m && nd < 256) dep[nd++] = i;
    }
    Node *mot = later_pi(gp);
    for (int q = nd - 1; q >= 0; q--) if (mentions_name(mot, cdefs[dep[q]].v)) mot = sym4("let", nsym(cdefs[dep[q]].v), typed_def(&cdefs[dep[q]]), mot);
    mot = sym3("%lam", nsym(v), mot);
    /* the frame of this split: x_gp in terms of v */
    Node *E = expand(tsig.pn[gp]);
    Node **alts = calloc((size_t)N + 1, sizeof(Node *)); int na = 0;
    for (int ci = 0; ci < N; ci++) {
        int c = DATA[d].cons[ci];
        int nf = CON[c].nfields + CON[c].ndims;
        Node **fv = calloc((size_t)nf + 1, sizeof(Node *));
        for (int r = 0; r < nr && nf; r++) if (pat_con(rows[r].p[j]) == c) {
            Node *p = rows[r].p[j];
            for (int q = 0; q < nf; q++) if (!fv[q] && p->list && q + 1 < p->n && !p->it[q + 1]->list && !pat_wild(p->it[q + 1]) && pat_con(p->it[q + 1]) < 0) {
                bool clash = false;
                for (int z = 0; z < q; z++) if (fv[z] && !strcmp(fv[z]->s, p->it[q + 1]->s)) clash = true;
                for (int z = 0; z < nv; z++) if (!strcmp(vars[z]->s, p->it[q + 1]->s)) clash = true;
                for (int z = 0; z < tsig.np; z++) if (!strcmp(tsig.pn[z]->s, p->it[q + 1]->s)) clash = true;
                if (!clash) fv[q] = p->it[q + 1];
            }
            break;
        }
        for (int q = 0; q < nf; q++) if (!fv[q]) fv[q] = xfresh("f");
        int nv2 = nv - 1 + nf;
        Node **v2 = calloc((size_t)nv2 + 1, sizeof(Node *));
        for (int q = 0; q < j; q++) v2[q] = vars[q];
        for (int q = 0; q < nf; q++) v2[j + q] = fv[q];
        for (int q = j + 1; q < nv; q++) v2[q - 1 + nf] = vars[q];
        Row *r2 = calloc((size_t)nr + 1, sizeof(Row)); int n2 = 0;
        for (int r = 0; r < nr; r++) {
            Node *p = rows[r].p[j]; int pc = pat_con(p);
            if (!pat_wild(p) && pc >= 0 && pc != c) continue;
            Row x; x.p = calloc((size_t)nv2 + 1, sizeof(Node *)); x.rhs = rows[r].rhs;
            for (int q = 0; q < j; q++) x.p[q] = rows[r].p[q];
            for (int q = j + 1; q < nv; q++) x.p[q - 1 + nf] = rows[r].p[q];
            if (pc == c) {
                if ((p->list ? p->n - 1 : 0) != nf) die("a pattern has the wrong number of fields for", CON[c].name);
                for (int q = 0; q < nf; q++) x.p[j + q] = p->it[q + 1];
            } else {
                for (int q = 0; q < nf; q++) x.p[j + q] = nsym("_");
                if (!pat_wild(p) && strcmp(p->s, v)) x.rhs = x_rename(x.rhs, p->s, v);
            }
            r2[n2++] = x;
        }
        if (!n2) die("the clauses do not cover every case of", tfname);
        /* v rebuilt, and what was rebuilt from it */
        Node *cv = nsym(CON[c].name);
        if (nf) { cv = nlist(nf + 1); cv->it[0] = nsym(CON[c].name); for (int q = 0; q < nf; q++) cv->it[q + 1] = fv[q]; }
        int save_defs = ncdefs, save_frames = nframes;
        Node *vty = ty_of(v);
        Node **fty = field_types(c, vty, fv);
        CVar vdef = { v, cv, gp, vty };
        GROW(cdefs, capcdefs, (size_t)ncdefs + 1); cdefs[ncdefs++] = vdef;
        for (int q = 0; q < nd; q++) { GROW(cdefs, capcdefs, (size_t)ncdefs + 1); cdefs[ncdefs++] = cdefs[dep[q]]; }
        for (int q = 0; q < nf; q++) { GROW(cdefs, capcdefs, (size_t)ncdefs + 1); cdefs[ncdefs++] = (CVar){ fv[q]->s, NULL, gp, q < CON[c].nfields ? fty[q] : NULL }; }
        /* the recursive fields' hypotheses */
        Frame fr = { v, gp, E, calloc((size_t)nf + 1, sizeof(Node *)), 0 };
        for (int q = 0; q < CON[c].nfields; q++) if (CON[c].recmask >> q & 1) fr.rf[fr.nrf++] = fv[q];
        GROW(frames, capframes, (size_t)nframes + 1); frames[nframes++] = fr;
        Node *body = ccm_t(v2, nv2, r2, n2);
        nframes = save_frames;
        /* the field entries are scoping only */
        ncdefs = save_defs;
        body = later_lam(gp, body);
        /* v rebuilt (and what depends on it) where the branch mentions it */
        for (int q = nd - 1; q >= 0; q--) if (mentions_name(body, cdefs[dep[q]].v)) body = sym4("let", nsym(cdefs[dep[q]].v), typed_def(&cdefs[dep[q]]), body);
        if (mentions_name(body, v)) body = sym4("let", nsym(v), typed_def(&vdef), body);
        Node *pat = nlist(nf + 1); pat->it[0] = nsym(CON[c].name);
        for (int q = 0; q < nf; q++) pat->it[q + 1] = fv[q];
        alts[na++] = lst(2, pat, body);
    }
    /* a motive not depending on v: a case */
    bool dep_mot = mentions_name(mot->it[2], v);
    Node *el_ = nlist(na + (dep_mot ? 3 : 2)); int k0 = 0;
    el_->it[k0++] = nsym(dep_mot ? "elim" : "case"); if (dep_mot) el_->it[k0++] = mot; el_->it[k0++] = nsym(v);
    for (int i = 0; i < na; i++) el_->it[k0++] = alts[i];
    return later_app(gp, el_);
}
/* free names of a signature not bound anywhere: its implicit type (and level) parameters */
static bool bindable_free(const char *s) {
    if (e_find(s) || global_lookup(s) >= 0 || con_lookup(s) >= 0 || fld_lookup(s) >= 0 || keyword(s) || constant_sym(s) || reserved(s)) return false;
    if (s[0] == '%' || s[0] == '#' || isdigit((unsigned char)s[0]) || !strcmp(s, "I") || !strcmp(s, "Level") || !strcmp(s, "_")) return false;
    return true;
}
static void free_in_sig(Node *t, Names *bound, Names *out, Names *lvl, bool under_U) {
    if (!t->list) { if (!names_has(bound, t->s) && bindable_free(t->s)) { names_add(out, t->s); if (under_U) names_add(lvl, t->s); } return; }
    if (t->n == 0) return;
    if (!t->it[0]->list) {
        const char *k = t->it[0]->s;
        if ((!strcmp(k, "Pi") || !strcmp(k, "Sigma")) && t->n == 4) {
            free_in_sig(t->it[2], bound, out, lvl, false);
            int save = bound->n; names_add(bound, t->it[1]->s); free_in_sig(t->it[3], bound, out, lvl, false); bound->n = save; return;
        }
        if ((!strcmp(k, "lam") || !strcmp(k, "plam") || !strcmp(k, "forall")) && t->n == 3) { int save = bound->n; names_add(bound, t->it[1]->s); free_in_sig(t->it[2], bound, out, lvl, false); bound->n = save; return; }
        if (!strcmp(k, "let") && t->n == 4) { free_in_sig(t->it[2], bound, out, lvl, false); int save = bound->n; names_add(bound, t->it[1]->s); free_in_sig(t->it[3], bound, out, lvl, false); bound->n = save; return; }
        if (!strcmp(k, "U") && t->n == 2) { free_in_sig(t->it[1], bound, out, lvl, true); return; }
        if (!strcmp(k, "%num")) return;
        if (!strcmp(k, "=") && t->n == 3) { free_in_sig(t->it[1], bound, out, lvl, false); return; }
        for (int i = 1; i < t->n; i++) free_in_sig(t->it[i], bound, out, lvl, false);
        return;
    }
    for (int i = 0; i < t->n; i++) free_in_sig(t->it[i], bound, out, lvl, false);
}
static Node *autobind(Node *sig) {
    Names b = { 0 }, out = { 0 }, lvl = { 0 };
    free_in_sig(sig, &b, &out, &lvl, false);
    for (int i = out.n - 1; i >= 0; i--) if (!names_has(&lvl, out.v[i])) sig = sym2("%impPi", sym4("Pi", nsym(out.v[i]), nsym("U"), sig));
    for (int i = lvl.n - 1; i >= 0; i--) sig = sym2("%impPi", sym4("Pi", nsym(lvl.v[i]), nsym("Level"), sig));
    free(b.v); free(out.v); free(lvl.v);
    return sig;
}
/* the telescope of a signature: its leading Πs */
static void tele(Node *sig) {
    int n = 0; Node *t = sig;
    for (;;) {
        bool imp = false;
        if (t->list && t->n == 2 && is(t->it[0], "%impPi")) { imp = true; t = t->it[1]; }
        if (!(t->list && t->n == 4 && is(t->it[0], "Pi"))) break;
        n++; t = t->it[3]; (void)imp;
    }
    tsig.np = n; tsig.pn = calloc((size_t)n + 1, sizeof(Node *)); tsig.pty = calloc((size_t)n + 1, sizeof(Node *)); tsig.imp = calloc((size_t)n + 1, sizeof(bool));
    t = sig; n = 0;
    for (;;) {
        bool imp = false;
        if (t->list && t->n == 2 && is(t->it[0], "%impPi")) { imp = true; t = t->it[1]; }
        if (!(t->list && t->n == 4 && is(t->it[0], "Pi"))) break;
        tsig.pn[n] = t->it[1]; tsig.pty[n] = t->it[2]; tsig.imp[n] = imp; n++;
        t = t->it[3];
    }
    tsig.R = t;
}
/* does every call of f in e give, at parameter q, the clause's own variable there? */
static bool uniform_in(Node *e, const char *f, int q, Node *pat) {
    if (!e->list) return strcmp(e->s, f) != 0;
    if (e->n >= 1 && !e->it[0]->list && !strcmp(e->it[0]->s, f)) {
        int i = 1;
        for (int k = 0; k < tsig.np; k++) {
            if (tsig.imp[k]) { if (i < e->n && e->it[i]->list && e->it[i]->n == 2 && is(e->it[i]->it[0], "%imp")) i++; continue; }
            if (k == q) { if (i >= e->n) return false; Node *a = e->it[i]; if (a->list || pat->list || pat_wild(pat) || strcmp(a->s, pat->s)) return false; }
            i++;
        }
        for (int k = 1; k < e->n; k++) if (!uniform_in(e->it[k], f, q, pat)) return false;
        return true;
    }
    for (int i = 0; i < e->n; i++) if (!uniform_in(e->it[i], f, q, pat)) return false;
    return true;
}
/* the checked definition's body from its clauses */
static Node *clauses_typed(const char *f, Lhs *ls, Node **rhs, int n) {
    tfname = f;
    /* the patterns at the parameters: implicit ones given as {p} or else the parameter itself; the
     * patterns past the parameters are dimensions (the result is a path) */
    int extra = -1;
    Row *rows = calloc((size_t)n + 1, sizeof(Row));
    for (int i = 0; i < n; i++) {
        Node **ps = calloc((size_t)tsig.np + 64, sizeof(Node *)); int k = 0;
        for (int q = 0; q < tsig.np; q++) {
            if (tsig.imp[q]) { if (k < ls[i].np && ls[i].ps[k]->list && ls[i].ps[k]->n == 2 && is(ls[i].ps[k]->it[0], "%imp")) ps[q] = pat_norm(ls[i].ps[k++]->it[1]); else ps[q] = tsig.pn[q]; continue; }
            if (k >= ls[i].np) die("a clause with fewer patterns than parameters:", f);
            ps[q] = ls[i].ps[k++];
        }
        int ex = ls[i].np - k;
        if (extra < 0) extra = ex; else if (extra != ex) die("clauses with different numbers of patterns:", f);
        for (int q = 0; q < ex; q++) ps[tsig.np + q] = ls[i].ps[k + q];
        rows[i] = (Row){ ps, rhs[i] };
    }
    if (extra < 0) extra = 0;
    /* a parameter whose patterns are all one variable is named by it, when the signature does not
     * use its name */
    for (int q = 0; q < tsig.np; q++) {
        const char *nm = NULL; bool same = true;
        for (int i = 0; i < n; i++) { Node *p = rows[i].p[q]; if (pat_wild(p)) continue; if (p->list || pat_con(p) >= 0) { same = false; break; } if (!nm) nm = p->s; else if (strcmp(nm, p->s)) same = false; }
        if (!same || !nm || !strcmp(nm, tsig.pn[q]->s)) continue;
        bool used = mentions_name(tsig.R, tsig.pn[q]->s);
        for (int z = q + 1; z < tsig.np; z++) if (mentions_name(tsig.pty[z], tsig.pn[q]->s)) used = true;
        bool taken = false; for (int z = 0; z < tsig.np; z++) if (!strcmp(tsig.pn[z]->s, nm)) taken = true;
        if (!used && !taken && bindable_free(nm)) tsig.pn[q] = nsym(nm);
    }
    /* the dimensions' names */
    Node **dn = calloc((size_t)extra + 1, sizeof(Node *));
    for (int q = 0; q < extra; q++) {
        Node *p = rows[0].p[tsig.np + q];
        dn[q] = !p->list && !pat_wild(p) ? p : xfresh("i");
        for (int i = 0; i < n; i++) { Node *pp = rows[i].p[tsig.np + q]; if (pp->list || pat_con(pp) >= 0) die("a dimension is matched only by a variable in", f); if (!pat_wild(pp) && strcmp(pp->s, dn[q]->s)) rows[i].rhs = x_rename(rows[i].rhs, pp->s, dn[q]->s); rows[i].p[tsig.np + q] = nsym("_"); }
    }
    /* which later parameters a split reverts */
    tsig.rev = calloc((size_t)tsig.np + 1, sizeof(bool));
    for (int q = 0; q < tsig.np; q++) {
        for (int r = 0; r < q; r++) if (mentions_name(tsig.pty[q], tsig.pn[r]->s)) tsig.rev[q] = true;
        if (!tsig.imp[q]) for (int i = 0; i < n && !tsig.rev[q]; i++) if (!uniform_in(rows[i].rhs, f, q, rows[i].p[q])) tsig.rev[q] = true;
    }
    ncdefs = 0; nframes = 0;
    Node **vars = calloc((size_t)tsig.np + 1, sizeof(Node *));
    for (int q = 0; q < tsig.np; q++) vars[q] = tsig.pn[q];
    /* each clause's right side under its dimensions (a path's λs, guessed: a line by default) */
    for (int i = 0; i < n; i++) for (int q = extra - 1; q >= 0; q--) rows[i].rhs = sym3("lam", dn[q], rows[i].rhs);
    Node *body = ccm_t(vars, tsig.np, rows, n);
    for (int q = tsig.np - 1; q >= 0; q--) body = sym3("%lam", tsig.pn[q], body);
    return body;
}
/* a checked definition (or, name NULL, a check): elaborated, checked, its holes and guesses
 * settled by the checker, until the checker's answer is final */
static void x_checked(const char *name, Node *sig, Node *body, bool expect, uint64_t mask, int mlen) {
    neh = neg = 0;
    Node *T = NULL, *t = NULL;
    for (int round = 0; ; round++) {
        nesc = 0; T = core(el(sig));
        nesc = 0; t = core(el(body));
        if (trusted) break;
        Node *dT = ds(T), *dt = ds(t);
        Node *q = nl3("%%chk-def", nl2("%raw", aclosed(dT)), nl2("%raw", aclosed(dt)));
        long steps = 0;
        char *r = readback(q, &steps);
        const char *saved = src; src = r; Node *rn = read_node(); src = saved;
        bool again = false;
        if (rn && rn->list && rn->n == 2 && is(rn->it[0], "elab")) {
            Node *xs[256]; int nx = cn_list(rn->it[1], xs, 256);
            for (int i = 0; i < nx; i++) {
                Node *pr_ = xs[i];
                if (!pr_->list || pr_->n != 3 || !is(pr_->it[0], "pair")) continue;
                long k = cn_word(pr_->it[1]);
                Node *o = pr_->it[2];
                if (k < 0 || k >= neh || EH[k].sol || !o->list || o->n != 2 || !is(o->it[0], "some")) continue;
                ncn = 0; Node *sol = cn(o->it[1]);
                if (sol) { EH[k].sol = sol; again = true; }
            }
            if (!again) { free(r); r = strdup("(no opq #32 opq opq)"); }
        } else if (rn && rn->list && rn->n == 2 && is(rn->it[0], "redo")) {
            long k = cn_word(rn->it[1]);
            if (k >= 0 && k < neg && round < 200) { EG[k].kind ^= 1; again = true; }
        }
        if (again && round < 400) { free(r); continue; }
        forced_result = r; forced_steps = steps;
        break;
    }
    if (getenv("CUBICAL_SHOW_ELAB")) { fprintf(stderr, "%s : ", name ? name : "(check)"); print_node(stderr, T); fprintf(stderr, "\n  = "); print_node(stderr, t); fprintf(stderr, "\n"); }
    if (name) x_top(lst(4, nsym(expect ? "checked" : "checked-fail"), nsym(name), T, t));
    else x_top(lst(3, nsym(expect ? "check" : "check-fail"), T, t));
    if (forced_result) { free((char *)forced_result); forced_result = NULL; }
    if (name && mask && global_lookup(name) >= 0) imask_set(name, mask, mlen);
}
