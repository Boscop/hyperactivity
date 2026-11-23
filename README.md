<p align="center">
  <a href="https://hyperactivity.ai">
    <picture>
      <source media="(prefers-color-scheme: dark)" srcset="https://hyperactivity.ai/logo.svg?v=lv2" />
      <source media="(prefers-color-scheme: light)" srcset="https://hyperactivity.ai/logo-light.svg?v=lv2" />
      <img src="https://hyperactivity.ai/logo.svg?v=lv2" alt="hyperactivity — Coherent Lightvirus" width="108" />
    </picture>
  </a>
</p>

<p align="center">
  <a href="https://hyperactivity.ai/nets">
    <img src="https://hyperactivity.ai/readme-hero.svg?v=2" alt="Hyperactivity — Active Pair Reduction and Guarded Corecursion Interaction Net" width="100%" />
  </a>
</p>

<h1 align="center">hyperactivity</h1>

<p align="center">
  <strong>A cubical interaction-net operating system language and decentralized knowledge graph.</strong><br/>
  <a href="https://hyperactivity.ai">hyperactivity.ai</a> ·
  <a href="https://hyperactivity.ai/kg">Knowledge Graph (<code>/kg</code>)</a> ·
  <a href="https://hyperactivity.ai/kernel">Browser Kernel (<code>/kernel</code>)</a> ·
  <a href="https://hyperactivity.ai/nets">Interaction Net Lab (<code>/nets</code>)</a> ·
  <a href="https://hyperactivity.ai/ai"><code>/ai</code></a> ·
  <a href="https://hyperactivity.ai/neuro"><code>/neuro</code></a> ·
  <a href="https://hyperactivity.ai/physics"><code>/physics</code></a> ·
  <a href="https://hyperactivity.ai/compression"><code>/compression</code></a> ·
  <a href="https://hyperactivity.ai/silicon"><code>/silicon</code></a> ·
  <a href="https://hyperactivity.ai/chip-design"><code>/chip-design</code></a> ·
  <a href="https://hyperactivity.ai/pdf">Monograph (PDF)</a> ·
  <a href="LICENSE">License</a>
</p>

---

## 0. Enter the System

Everything in this document is live, self-hosted, and executable: hyperactivity is written in hyperactivity. You can enter from a browser with zero install, or build it locally with `make`.

### In Your Browser (Zero Install, Zero Server Compute)

- [**`hyperactivity.ai/kernel`**](https://hyperactivity.ai/kernel) — The interaction-net machine compiled to WebAssembly (`cubical.wasm`), executing directly inside your browser tab.
- [**`hyperactivity.ai/kg`**](https://hyperactivity.ai/kg) — Interactive explorer for the content-addressed Knowledge Graph (`lib/`, `neuro/`, `chem/`, `phys/`, `lqg/`, `fold/`), backed by a browser-local `IndexedDB` content-addressed object store (`hx-cas-v1`) and stateless WebRTC peer-to-peer object sync.
- [**`hyperactivity.ai/nets`**](https://hyperactivity.ai/nets) — Multi-scale visual compiler (`Macro` / `Structural` / `Atomic`) for interaction nets: arithmetic, LSB-first binary carry, parallel merge sort, cubical paths, Kan box filling, computational univalence, and bounded $O(1)$ guarded-corecursive artificial life (`life ▸ κ`).
- **Interactive domain treatises with live simulations and clickable source portals:**
  - [**`hyperactivity.ai/ai`**](https://hyperactivity.ai/ai) — Observation fibers, split witnesses, `WorldAfter` active queries, standpoint networks, and why RLHF and lexical refusal gates fail structurally.
  - [**`hyperactivity.ai/neuro`**](https://hyperactivity.ai/neuro) — Nernst/Hodgkin–Huxley/FitzHugh–Nagumo phase planes, intrinsic cortical manifolds, two-learner neural co-processor SOS Lyapunov & Control Barrier certificates, and interface-reshaping `RProc`.
  - [**`hyperactivity.ai/physics`**](https://hyperactivity.ai/physics) — Relational Quantum Mechanics, $SU(2)$ spin networks, Loop Quantum Cosmology Big Bounce, relativistic Dirac atoms ($Z \le 137$), superconductivity, zero-float quantum chemistry, peptide kinematics, and bioelectric morphogenesis.
  - [**`hyperactivity.ai/compression`**](https://hyperactivity.ai/compression) — Zero-search causal context-fiber rANS compression with exact `refl` benchmarks vs `deflate -9`, `gzip -9`, `bzip2 -9`, and `lzma`.
  - [**`hyperactivity.ai/silicon`**](https://hyperactivity.ai/silicon) ([paper: `/chip-design`](https://hyperactivity.ai/chip-design)) — Cycle-accurate 16-tile `HyperNet-16` (`4×4`) vs. pipelined RISC-V simulator (`69.0×` fewer cycles / `43.0×` faster 7nm latency on `5-Queens`), official 128-bit EPFL SkyWater 130nm synthesis (`-50.1%` cells on `bar`, `13.0×` faster `adder`), Tiny Tapeout Sky130 `tt_um_hyperinet`, and `18,664,712`-polygon Calma GDSII v6.0 die inspector.
  - [**`hyperactivity.ai/learn`**](https://hyperactivity.ai/learn) — Ground-up interactive textbook on interaction nets with live inline programs.

### On Your Machine (Requires `gcc` or `clang` and `make`)

```sh
git clone https://github.com/avikj/hyperactivity.git && cd hyperactivity && make
./hyperactive                                   # interactive loop over store/
./hyperactive def two-learner-asym-sos-id       # inspect any definition, its SHA-256 key, and dependents
./build/cubical neuro/lyapunov.hypercubical     # run any module (.hypercubical and .cub both resolve)
make brunerie                                   # compute Brunerie's number (-2) on 1 and 4 workers
./build/cubical lib/server.hypercubical 8080    # boot your own local mirror of hyperactivity.ai
```

### Table of Contents

1. [The Structural Failure of the 1970s Computing Stack](#1-the-structural-failure-of-the-1970s-computing-stack)
2. [Interaction Nets: The Local Execution Primitive (`lib/kernel.hypercubical`)](#2-interaction-nets-the-local-execution-primitive)
3. [Cubical Type Theory, Kan Composition & Computational Univalence (`lib/check.hypercubical`)](#3-cubical-type-theory-kan-composition--computational-univalence)
4. [Observation Fibers, Warrant, Split Witnesses & Standpoint Alignment (`lib/observation.hypercubical`)](#4-observation-fibers-warrant-split-witnesses--standpoint-alignment)
5. [No Filesystems, No Canonical State: Content-Addressed Set Union & Conserved Credit (`store/`, `lib/credit.hypercubical`)](#5-no-filesystems-no-canonical-state-content-addressed-set-union--conserved-credit)
6. [Zero-Rewrite Language Lifting, Specialization Equivalence & Native Codegen (`tools/`, `lib/cxx/`, `lib/specialize.hypercubical`)](#6-zero-rewrite-language-lifting-specialization-equivalence--native-codegen)
7. [Full-Stack Silicon: Circuit Lower Bounds, 3nm GDSII v6.0 & Quantum Error Correction (`lib/silicon/`)](#7-full-stack-silicon-circuit-lower-bounds-3nm-gdsii-v60--quantum-error-correction)
8. [Zero-Search Causal Context-Fiber Compression (`lib/compress.hypercubical`)](#8-zero-search-causal-context-fiber-compression)
9. [Neuroscience & Closed-Loop Neural Co-Processors (`neuro/`, `lib/polyproc.hypercubical`)](#9-neuroscience--closed-loop-neural-co-processors)
10. [Relational Physics, Exact Quantum Chemistry & Living Matter (`lqg/`, `phys/`, `chem/`, `fold/`)](#10-relational-physics-exact-quantum-chemistry--living-matter)
11. [The Decentralized Web Seed (`lib/server.hypercubical`)](#11-the-decentralized-web-seed)
12. [Reference: Commands, Store Mechanics & Primary Literature](#12-reference-commands-store-mechanics--primary-literature)

---

## 1. The Structural Failure of the 1970s Computing Stack

Modern computing — operating systems, web browsers, compilers, cloud databases, frontier AI, cheminformatics, and neural interfaces — still runs on four architectural decisions frozen in the 1970s:

1. **Untyped byte streams.** Files, pipes, sockets, HTTP bodies, and memory pages are flat sequences of bytes with no intrinsic type, invariant, or proof attached. Once a compiler lowers a program to bytes, its mathematical meaning is thrown away. Every downstream parser, serializer, runtime, and hardware block must re-parse, guess, or trust an out-of-band specification.
2. **Mutable location-addressed storage.** A filesystem path like `/etc/config`, a raw memory pointer `0x7fff...`, or a URL `https://host/api/v1` names a mutable box rather than an immutable value. Every edit silently mutates the meaning of every reference pointing into that box, forcing lock hierarchies, version-pinning hacks, fragile container images, and irreproducible builds.
3. **Collapsed equality.** 1970s software has only raw byte equality (`memcmp`) or pointer identity (`==`). It cannot distinguish three fundamentally different relations:
   - **Digest equality** — identical byte sequence / cryptographic hash;
   - **Definitional equality** — identical normal form under computation;
   - **Semantic / path equivalence** — two structurally distinct representations (a linked list and a packed array, two tautomers of a crystal, a C++ class method and its vectorized x86-64 specialization, or two coordinate charts on a manifold) that compute the same structure and transport every proof and program between them.
4. **Centralized canonical state.** Because state is mutable and location-addressed, two independent machines cannot converge simply by exchanging facts. Coordination requires either a central authority (cloud platforms, centralized registries, trusted APIs) or an expensive global ordering protocol (blockchains, Paxos/Raft consensus, distributed lock managers).

Every major crisis across contemporary computing and science is a downstream symptom of those four defects:

| Domain | Observed Crisis | Underlying Structural Defect |
| --- | --- | --- |
| **Artificial Intelligence** | Hallucination, sycophancy, brittle lexical refusals | Token predictors sample a single point from an uncollapsed space of compatible histories without checking fiber-wide invariance; population-marginal RLHF collapses incompatible human standpoints into a 1D scalar average; lexical safety filters match surface byte strings rather than semantic equivalence classes. |
| **Chemistry & Cheminformatics** | Chemical entity-resolution failure (IUPAC Project #2012-023-2-800) | Standard InChI collapses digest equality, definitional equality, and tautomeric/conformational equivalence into string equality of a single canonical identifier, failing systematically across tautomeric crystals in the Cambridge Structural Database. |
| **Systems & Hardware** | Endless rewrites across C, C++, Rust, CUDA, Verilog, GDSII | Compilers discard equivalence proofs on lowering, blocking cross-language proof transport, invariant-driven specialization, and end-to-end silicon certification. |
| **Distributed Systems & Economics** | Platform lock-in, blockchain energy/latency overhead, 1D scalar money | Mutable location-addressed state forces global ordering; economic ledgers collapse multi-dimensional provenance and contribution into a single unindexed scalar. |
| **Neural Interfaces (BCI)** | Closed-loop co-adaptation instability & unsafe stimulation | Numerical simulation over IEEE-754 floats cannot certify stability or forward-invariance across all continuous parameter corners of a coupled biological-electronic system. |
| **Fundamental Physics** | Coordinate/gauge artifacts, floating-point drift, background dependence | Floating-point discretization breaks exact conservation laws and gauge equivalences; global-clock simulation contradicts the local relational structure of physical interactions. |

Hyperactivity replaces all four 1970s assumptions simultaneously with one self-hosted architecture:

1. **Local interaction nets** (`lib/kernel.hypercubical`) as the lock-free, clockless execution primitive;
2. **CCHM cubical type theory** (`lib/check.hypercubical`) with computational univalence (`ua`) as the universal language of programs, types, proofs, and equivalences;
3. **Observation fibers & split witnesses** (`lib/observation.hypercubical`) as the epistemic foundation of knowledge, compression, alignment, and control;
4. **An immutable SHA-256 content-addressed store** (`store/`) with no mutable filesystems, no canonical state, convergence by pure set union, and conserved multi-dimensional credit (`lib/credit.hypercubical`).

---

## 2. Interaction Nets: The Local Execution Primitive

There is no primitive static "thing" in nature, only a primitive law of local interaction. A cortical microcircuit, an $SU(2)$ spin-network vertex, a biochemical reaction complex, an asynchronous hardware tile, and a peer-to-peer network of agents have no global clock, no central address bus, and no global call stack. They are **interaction networks** (Lafont, 1990).

### The Machine, Written in Hyperactivity ([`lib/kernel.hypercubical`](https://hyperactivity.ai/kg?mod=lib/kernel.hypercubical))

The machine is a definition in the language, checked like every other definition. It has one rule: **an eliminator meets a value, and a table (`nk-act`) says what is built on the wires the two held.** Nothing else in it belongs to any particular calculus:

- **Bodies** (`NkTm`): variables by index into the environment, constructors with their arguments, and numbered eliminators (`nk-op`) carrying the body they eliminate and their other arguments. Application, increment and case are eliminators `#0`, `#1`, `#2`.
- **Wires** (`NkW`): a wire's name is a binary number, lowest digit first; `nk-s` gives the next name.
- **Slots** (`NkSlot`): a wire is empty, holds a value, holds a waiting operation, or holds the pair that meets: `nks-none | nks-val | nks-op | nks-pair`.
- **Changes** (`NkCh`): a step asks for changes to wires. A value arrives, an operation arrives, or a slot is set. A value waits or meets the operation waiting; an operation waits or meets the value waiting. Waiting is not a step.
- **Sharing**: a value with parts is held once and read by reference (`nkv-ref`). Duplication (`nko-dup`), erasure (`nko-era`) and forwarding (`nko-fwd`) are operations like any other.
- **Blocked values**: an eliminator meeting a blocked value becomes a blocked value, so open terms reduce under binders during type checking.
- **Quotation** (`nko-quote`, `nko-gather`): reads a value back as a body.
- **The executor** (`NkExec`) stands outside the rule. It only chooses which wire steps next and hands out fresh names.

A new type former is rows of the table plus bodies written with `nk-con` and `nk-op`: no new rule and no new proof. The interval rows (`nk-act-i`) are how De Morgan intervals, paths, Kan composition, Glue and univalence enter the same machine.

- **Active pair:** an operation and a value meeting on one wire. **This is the only site where computation occurs.**
- **Rewrite step** (`nk-step-at`): the pair is replaced by what the table builds on the wires it held. No other slot is read or written.

```text
   arg ──┐
         app ●────● λ ── body      ⟶      out ── body   (arg wired to param)
   out ──┘
```

### Parallelism Is a Theorem of the Machine

Lock-free, clockless execution is proved in `lib/kernel.hypercubical`, not assumed of an implementation:

- **`nk-meet-comm`**: a value arriving and an operation arriving at the two ends of one wire commute, whatever the wire held.
- **`nk-safe-commute`**: whenever the executor's test `nk-safe?` accepts two wires, stepping them in either order leaves memories that read the same at every wire.
- **`nk-get-set`, `nk-get-set-other`, `nk-upd-swap-at`**: the memory laws both rest on.

An executor may therefore step any two wires `nk-safe?` accepts in either order. `lib/silicon/inet.hypercubical` states the corresponding facts for the hardware tile in its own model (`wslot-order-indep`, `net-step-commute`).

### The Machine Checks Its Own Computation

- Two evaluators written in the language, the net (`nk-nf`) and an environment machine (`nk-ev-nf`), are checked by `refl` to reach the same normal form on worked terms (`nk-ev-agree-*`) covering sharing, open bodies, De Morgan interval laws and path endpoints.
- Kan composition and univalence compute inside the machine: `nk-kan-ua` transports the pair `(1, 2)` along `ua swap` to `(2, 1)`, and `nk-ty-ua-over` types the glued value over the univalence path.
- [`lib/check.hypercubical`](https://hyperactivity.ai/kg?mod=lib/check.hypercubical) is the type checker, written in the language.
- [`lib/singularity.hypercubical`](https://hyperactivity.ai/kg?mod=lib/singularity.hypercubical) crawls `store/` cold from disk, checks each definition's body against its type with the definition's own slot masked (so a recursive definition cannot justify itself), and emits itself both as ISO C (`sg-c`) and as a static x86-64 ELF written byte by byte by [`lib/elf.hypercubical`](https://hyperactivity.ai/kg?mod=lib/elf.hypercubical), with no `gcc` and no libc (`sg-elf`). The pool's verifier is a binary hyperactivity wrote.

`src/` is the bootstrap that runs programs natively today; [`/kernel`](https://hyperactivity.ai/kernel) runs it compiled to WebAssembly.

### Guarded Corecursion in Bounded Memory

Infinite reactive processes (event loops, HTTP servers, excitable neural membranes, closed-loop neural co-processors, and artificial life, `life ▸ κ` at [`hyperactivity.ai/nets?preset=life`](https://hyperactivity.ai/nets?preset=life)) run forever in constant memory: recursive unfolding is guarded by a clock modality `▸ κ` and steps only when an external observation forces it.

### The Table (`nk-act`)

The machine's whole calculus is these rows. An eliminator meeting a value either builds a body on the wires (with the eliminator's wires innermost, `act-ap`; with the value's parts innermost, `act-rp`; or with the value itself innermost, `act-self`), returns a value outright (`act-hd`), or is kept whole holding the value (`act-stuck`), read back and computed on bodies.

```text
apply (0)          ⋈  closure                  ⟶  its body, the argument first
increment (1)      ⋈  number m                 ⟶  m + 1
case (2)           ⋈  constructor c            ⟶  body of branch c, its fields bound
first (9)          ⋈  pair                     ⟶  the first part
second (10)        ⋈  pair                     ⟶  the second part
path at a point (8) ⋈ end of the interval      ⟶  that end of the path; the path is not read
path at a point (8) ⋈ blocked point            ⟶  the path applied to the point
reversal (5)       ⋈  interval point           ⟶  its De Morgan negation, in normal form
meet (6), join (7) ⋈  interval point           ⟶  wait on the second point carrying the first (16, 17)
meet (16)          ⋈  second point             ⟶  the meet, in normal form
join (17)          ⋈  second point             ⟶  the join, in normal form
transport, composition (20 and up)             ⟶  kept whole, computed on the read-back body
```

An interval point is a join of meets of literals, each meet sorted, no meet containing another, so two points equal in the De Morgan algebra have one form.

### The Bootstrap's Memory Layout, Sharing & Zero Ambient Context

Choices in the bootstrap make cubical interaction nets fast enough to self-host a full operating system and compute Brunerie's number in seconds:

1. **Dual-mode reference counting (`net_solo`):**
   When running single-threaded (`net_solo = true`, set once at `net_start`), reference counts and step counters use plain integer arithmetic with zero atomic bus-lock overhead. When worker threads are active (`net_solo = false`), the exact same code switches to lock-free `__atomic_fetch_add(..., __ATOMIC_RELAXED)` and `__atomic_sub_fetch(..., __ATOMIC_ACQ_REL)`. There is no global heap lock and no stop-the-world garbage collector.
2. **Immutable DNF payloads & zero ambient face context:**
   Every dimension name `Dim` is a monotonically allocated `uint64_t` (never reused). Intervals (`V_IV`) and faces (`V_FACE`) carry immutable, reference-counted Disjunctive Normal Form (`Dnf`) arrays of sorted `Clause`s, where each literal packs `dim << 1 | polarity`. Because a face clause is literally a substitution of endpoints (`0` or `1`) for dimensions, working under a face is purely local reindexing via `O_SUBST` — **no cell ever consults an ambient typing or face context.**
3. **Optimal lazy sharing (`O_DUP`, `V_REF`, `V_THUNK`, `V_DEFER`, `V_TEXT`):**
   - Duplicating a compound value (`O_DUP ⋈ value`) never deep-copies its subgraph; it wraps the value once in `V_REF` and hands each consumer a reference. When an operation meets `V_REF`, only the top cell is copied out while all subparts remain shared `V_REF` pointers.
   - Deferred operations (`V_DEFER`) avoid eagerly duplicating eliminators across every branch of a partial system `V_SYS`: each branch holds a deferred operation that only runs if that branch is demanded.
   - `V_TEXT` represents a string of bytes as a `List Word` (`cons #b (cons #b ... nil)`) lazily one character at a time from a shared `Bytes` buffer, so copying a text shares the underlying bytes in $O(1)$.

### The Bootstrap's Laws

The bootstrap in `src/` represents these value forms:

- **Value (`V_*`):** A cell with a *positive* principal port — it offers data:
  - Core values: `V_LAM` (closure with `Record` + environment ports), `V_PAIR`, `V_PI`, `V_SIGMA`, `V_U` (universe level), `V_PATHP`, `V_PLAM` (path abstraction binding a `Dim`), `V_IV` (interval `Dnf`), `V_FACE` (face `Dnf`), `V_SYS` (partial system of branches), `V_GLUE`, `V_GLUEV`.
  - Data & codata values: `V_DATA`, `V_CON` (constructor with fields and interval dimensions), `V_HCOMP` (formal homogeneous composition), `V_TRANSH` (formal transported composition), `V_CASEF` (elimination as a function), `V_REC` (copattern body for guarded corecursion).
  - Sharing & laziness values: `V_REF` (shared box — copying it copies the reference; an operation meeting it copies only the top cell out), `V_THUNK` (unbuilt subterm), `V_DEFER` (deferred operation, built on demand), `V_SUSP` / `V_WSUSP` (lazy induction hypotheses).
  - Exact machine values: `V_WORD` (64-bit fast path or arbitrary-precision `Big` limb array from `src/big.c`), `V_PVAL` (`I64`, `U32`, `F64`, `F32`), `V_BYTES` (refcounted immutable byte buffer), `V_TEXT` (lazy `List Word` view over `Bytes` at offset `a`, costing zero cells until characters are read).
  - Blocked neutrals (for open terms during type checking and specialization): `V_NVAR`, `V_NAPP`, `V_NFST`, `V_NSND`, `V_NPAPP`, `V_NCOMP`, `V_NUNGLUE`, `V_NCASE`, `V_NTRANS`, `V_NPROJ`, `V_WPLUS`, `V_NWOP`, `V_NWELIM`, `V_NPRIM`.
- **Operation (`O_*`):** A cell with a *negative* principal port — it demands one value at port `0` and writes its output to its last port: `O_APP`, `O_CALL`, `O_FORCE`, `O_FST`, `O_SND`, `O_PAPPR`, `O_PAPPP`, `O_SUBST`, `O_DUP`, `O_ERA`, `O_COMP`, `O_COMPA`, `O_COMPH`, `O_COMPW`, `O_MKPLAM`, `O_MKGLUE`, `O_MKGLUEV`, `O_UNGLUE`, `O_CASE`, `O_HCOMP`, `O_TRANSD1`, `O_TRANSD2`, `O_VIEW`, `O_WEQ..O_WSHR`, `O_LOAD`, `O_STORE`, `O_NATIVE`, `O_FFI`.

Its law listing; read each law as `operation ⋈ value ⟶ replacement net`:

**Functions, pairs & guarded corecursion**
```text
O_APP(arg)      ⋈  V_LAM(body, env)       ⟶  body with arg wired in for parameter
O_CALL(args)    ⋈  global definition      ⟶  body built in one interaction with all parameters
O_FST           ⋈  V_PAIR(a, b)           ⟶  a   (b wired to O_ERA)
O_SND           ⋈  V_PAIR(a, b)           ⟶  b   (a wired to O_ERA)
O_FORCE         ⋈  V_THUNK / V_REC        ⟶  unfolded body (one step of guarded corecursion)
```

**Cubical paths & dimension substitution (`O_SUBST`)**
```text
O_PAPPP(r)      ⋈  V_PLAM(i, t)           ⟶  O_SUBST [i := r] facing t
O_PAPPP(0 | 1)  with known endpoints      ⟶  endpoint directly (path body never forced)
O_PAPPP(0 | 1)  ⋈  blocked neutral path   ⟶  endpoint read from the path's V_PATHP type
O_SUBST(σ)      ⋈  V_IV / V_FACE          ⟶  substituted canonical Dnf interval or face
O_SUBST(σ)      ⋈  V_LAM, V_PAIR, Π, Σ    ⟶  same cell with O_SUBST(σ) facing each part
O_SUBST(σ)      ⋈  V_PLAM(j, t)           ⟶  V_PLAM(j', O_SUBST(σ') facing t), fresh j' if clash
O_SUBST(σ)      ⋈  V_SYS                  ⟶  branch whose face became 1; else V_SYS w/ substituted faces
O_SUBST(σ)      ⋈  V_REF (held value)     ⟶  stays pending on reference until a reader arrives
```

**Optimal sharing & local erasure**
```text
O_DUP           ⋈  compound value         ⟶  value held once in V_REF; both outputs get references
O_DUP           ⋈  atomic value           ⟶  two independent copies
O_ERA           ⋈  value                  ⟶  cell freed; O_ERA wired to each auxiliary port
any operation   ⋈  V_REF                  ⟶  top cell copied out; subparts remain V_REF
```

**Blocked neutrals (open terms during type checking & specialization)**
```text
O_APP / O_FST / O_SND / O_PAPPP / O_COMP / O_CASE
                ⋈  V_NVAR or neutral      ⟶  neutral cell (V_NAPP, V_NFST, ..., V_NCASE)
O_SUBST(σ)      ⋈  neutral                ⟶  original operation re-triggered on substituted parts
```

**Partial systems (`V_SYS`)**
```text
eliminator      ⋈  V_SYS                  ⟶  V_SYS of V_DEFER(eliminator) on each branch (lazy)
O_SYSCONS(φ, t) when φ = 1 (true)         ⟶  t   (rest wired to O_ERA)
O_SYSCONS(φ, t) when φ = 0 (false)        ⟶  rest (t wired to O_ERA)
```

**Kan composition, Glue, Univalence & Higher Inductive Data Types**
```text
O_COMP          ⋈  total tube (φ = 1)     ⟶  tube evaluated at i = 1 (base wired to O_ERA)
O_COMP          with partial tube         ⟶  clause for head of type line A(i):
                                             V_PI, V_SIGMA, V_PATHP, V_U, V_GLUE, V_DATA, or Neutral
V_GLUE          when face φ = 1           ⟶  type T on that face
V_GLUEV         when face φ = 1           ⟶  value t on that face
O_UNGLUE        when face φ = 1           ⟶  equivalence map e applied to value
O_UNGLUE        ⋈  V_GLUEV(S, a)          ⟶  base a
V_CON (HIT)     at dim 0 or 1             ⟶  declared boundary term
O_HCOMP         in V_DATA w/ partial tube ⟶  V_HCOMP formal composition (or pushed into fields)
O_TRANSD        ⋈  V_CON                  ⟶  constructor's transport clause
O_TRANSD        on constant line (ψ = 1)  ⟶  identity (zero allocation)
O_CASE          ⋈  V_CON                  ⟶  branch built with fields (recursive hypotheses as V_SUSP)
O_CASE          ⋈  V_HCOMP                ⟶  composition in the elimination motive
```

**Machine words, exact bignums, primitives & native boundaries**
```text
O_WADD..O_WSHR  ⋈  V_WORD (64-bit / Big)  ⟶  exact arbitrary-precision natural arithmetic
O_POP1 / O_POP2 ⋈  V_PVAL (I64/U32/F64)   ⟶  exact IEEE/machine primitive operations
O_LOAD / O_STORE ⋈ V_BYTES                ⟶  bounds-checked immutable byte-region access
O_NATIVE        ⋈  crossing arguments     ⟶  direct jump into verified #jit native x86-64 code
```

Every Kan composition clause is itself a term of the language, not ad-hoc compiler magic, and open-term normalization for type checking and closed-term execution run on the exact same interaction-net machine.

> **Watch active pairs reduce live:** Open [**`hyperactivity.ai/nets`**](https://hyperactivity.ai/nets) and switch between `Macro`, `Structural`, and `Atomic` views.

---

## 3. Cubical Type Theory, Kan Composition & Computational Univalence

Hyperactivity implements **CCHM cubical type theory** (Cohen, Coquand, Huber, & Mörtberg, 2018) with a **self-hosted type checker** written inside the language itself ([`lib/check.hypercubical`](https://hyperactivity.ai/kg?mod=lib/check.hypercubical)). The machine `build/cubical` does not embed a separate trusted type-checking engine; it provides the interaction-net evaluator and reflection operations (`O_VIEW`, `O_MKVAR`, `O_MKDIM`, `O_IEQ`, `O_FEQL`, `O_RESTRICT`, `O_NTYPE`, `O_CLOS`) over which `lib/check.hypercubical` runs directly.

### The De Morgan Interval $I$ and Dependent Path Types

The interval $I$ carries two endpoints $0, 1 : I$ and a De Morgan algebra $(\wedge, \vee, 1 - \cdot)$ satisfying associativity, commutativity, idempotence, distributivity, boundary laws ($0 \wedge r = 0$, $1 \wedge r = r$, $0 \vee r = r$, $1 \vee r = 1$), and De Morgan involutions:

```math
1 - (1 - r) = r, \qquad 1 - (r \wedge s) = (1 - r) \vee (1 - s), \qquad 1 - (r \vee s) = (1 - r) \wedge (1 - s)
```

In the machine, every interval expression is held in Disjunctive Normal Form (`nk-dnf-of`, `nk-and-d`, and De Morgan negation `nk-not-d` in `lib/kernel.hypercubical`), so interval equality is decided in normal form.

Given a line of types $A : I \to \mathcal{U}$ and endpoints $a_0 : A(0)$, $a_1 : A(1)$, a **dependent path** is a continuous map out of $I$ agreeing definitionally with its endpoints:

```math
\mathrm{PathP}\;A\;a_0\;a_1 = \{\, p : (i : I) \to A(i) \mid p(0) \equiv a_0,\; p(1) \equiv a_1 \,\}
```

When $A$ is constant, $\mathrm{Path}_A(a, b) \equiv \mathrm{PathP}\;(\lambda \_.\, A)\;a\;b$ (written `a ≡ b`). Core principles that require uncomputable axioms in Martin-Löf type theory reduce directly by interaction:

- **Reflexivity:** $\mathrm{refl}_a = \lambda i.\, a$.
- **Symmetry:** $\mathrm{sym}\;p = \lambda i.\, p(1 - i)$.
- **Congruence (`cong`):** $\mathrm{cong}\;f\;p = \lambda i.\, f(p(i))$.
- **Function extensionality (`funExt`):**

```math
\mathrm{funExt} : \big(\forall x : A.\; f(x) \equiv g(x)\big) \to (f \equiv g), \qquad \mathrm{funExt}\;h = \lambda i.\,\lambda x.\; h\;x\;i
```

- **Contractibility of singletons (`sing-contr` in `lib/observation.hypercubical`):** A value paired with an equality witness to itself carries zero additional information; it contracts continuously to `(a, refl)` via the De Morgan connection $i \wedge j$:

```haskell
Sing : {A : Type} → A → Type
Sing {A} a = (x : A) × a ≡ x

sing-contr : {A : Type} → (a : A) → isContr (Sing a)
sing-contr a = ((a , λ i → a) , λ s → λ i → (snd s i , λ j → snd s (i /\ j)))
```

### Partial Systems and Kan Composition (`comp`)

A **face formula** $\varphi \in \mathbb{F}$ is generated by endpoint equations $(i = 0)$ and $(i = 1)$, conjunction $\wedge$, and disjunction $\vee$, with $(i = 0) \wedge (i = 1) = 0_{\mathbb{F}}$. A **partial element** $u : \mathrm{Partial}\;\varphi\;A$ is defined only where $\varphi$ holds, written `[ φ₁ ↦ t₁ , φ₂ ↦ t₂ ]`, where overlapping branches agree on $\varphi_1 \wedge \varphi_2$.

Given a line of types $A : I \to \mathcal{U}$, a face formula $\varphi \in \mathbb{F}$, a partial tube $u : (i : I) \to \mathrm{Partial}\;\varphi\;(A(i))$, and a base $a_0 : A(0)$ agreeing with $u(0)$ on $\varphi$, **Kan composition** `comp A [φ ↦ u] a₀ : A(1)` computes the lid of the open box at $i = 1$ agreeing definitionally with $u(1)$ on $\varphi$:

```text
          u(i=1)  ──▶  comp A [φ ↦ u] a₀   : A(1)   (agrees with u(1) on φ)
         ▲                      ▲
         │  partial tube u(i)   │  (defined wherever φ holds)
         │                      │
        a₀ ───────────────────▶ base : A(0)          (agrees with u(0) on φ)
```

From `comp`:
- **Transport** along any line of types $A : I \to \mathcal{U}$ is composition with the empty tube ($\varphi = 0_{\mathbb{F}}$):

```math
\mathrm{transport}\;A\;a_0 = \mathrm{comp}\;A\;[\,]\;a_0 : A(1)
```

- **Path concatenation** (`concat p q` for $p : a \equiv b$, $q : b \equiv c$) is a single composition in the constant type:

```haskell
cm-catb : {x y z : Bool} → x ≡ y → y ≡ z → x ≡ z
cm-catb {x} {y} {z} p q = λ i → comp (λ j → Bool) (λ j → [ i = 0 ↦ x , i = 1 ↦ q j ]) (p i)
```

- **Type-directed computation of `comp`:**
  - **$\Pi$-types:** $\mathrm{comp}\;(\lambda i.\,(x : A(i)) \to B(i, x))\;[\varphi \mapsto u]\;f_0$ fills the argument $x : A(1)$ backward along $A$ to $A(0)$, applies $f_0$, and composes in $B$.
  - **$\Sigma$-types:** fills the first component in $A$ to obtain a line $p(i)$ from $a_0$ to $a_1$, then composes the second component along the dependent line $B(i, p(i))$.
  - **Path types:** pushes `comp` under the path binder $\lambda j$, adjoining $(j = 0) \vee (j = 1)$ to the tube face $\varphi$.
  - **Data / Higher Inductive Types:** reduces into constructor fields when the cap and tube share a constructor (`O_HCM1..O_HCM3`), or forms a canonical `V_HCOMP` value whose eliminator (`O_CASE`) computes by composition in the elimination motive.
  - **Universes ($\mathcal{U}$):** computes via **Glue types**.

### Glue Types, Univalence (`ua`), and the Three-Way Equality Discipline

Given a base type $B$, a face $\varphi$, a partial type $A : \mathrm{Partial}\;\varphi\;\mathcal{U}$, and a partial equivalence $e : \mathrm{Partial}\;\varphi\;(A \simeq B)$, the type $\mathrm{Glue}\;[\varphi \mapsto (A, e)]\;B$ equals $A$ definitionally when $\varphi = 1$ and is equivalent to $B$ globally via `unglue`.

Choosing $\varphi = (i = 0) \vee (i = 1)$ with $(A, e)$ on $(i = 0)$ and $(B, \mathrm{id}_B)$ on $(i = 1)$ defines **Voevodsky's Univalence Map** directly as a computable term with zero axioms:

```math
\mathrm{ua} : (A \simeq B) \to \mathrm{Path}_{\mathcal{U}}(A, B), \qquad \mathrm{ua}(e) = \lambda i.\; \mathrm{Glue}\;[\, (i = 0) \mapsto (A, e),\; (i = 1) \mapsto (B, \mathrm{id}_B) \,]\;B
```

Transporting any proof, invariant, or program $x : P(A)$ along $\mathrm{ua}(e)$ executes directly through the `V_GLUE` / `O_UNGLUE` / `O_COMP` interaction laws:

```math
\mathrm{transport}\;(\lambda i.\; P(\mathrm{ua}(e)\,i))\;x \;\longrightarrow^{\ast}\; e_{\ast}(x)
```

This enforces the exact three-way distinction that 1970s computing collapses:
1. **Digest equality:** identical SHA-256 hash in `store/`.
2. **Definitional equality (`refl`):** identical normal form under the interaction-net laws.
3. **Path / semantic equivalence (`Path`, `ua`):** constructive witness that two structurally distinct representations behave identically and transport every proof and program between them.

### Stress Benchmark: Computing Brunerie's Number ($\pi_4(S^3) \cong \mathbb{Z}/2\mathbb{Z}$)

Computing **Brunerie's number** $\beta$ — the generator of the fourth homotopy group of the 3-sphere $\pi_4(S^3) \cong \mathbb{Z}/\beta\mathbb{Z}$, whose value is $\pm 2$ (Brunerie, 2016) — is the canonical stress test of cubical evaluators because it exercises nested higher inductive types ($S^1, S^2, S^3$, join, smash product, James construction), deep multi-dimensional Kan compositions, and Glue/univalence transport simultaneously.

Hyperactivity computes Ljungström's typed port (`experiments/brunerie/typed.cub`) directly to normal form `negsuc (suc zero)` ($-2$) in both single-worker and 4-worker parallel runs:

```sh
make brunerie
# 1 worker:  brunerie = negsuc (suc zero)   (358,603 interaction steps, ~6.6s)
# 4 workers: brunerie = negsuc (suc zero)
```

---

## 4. Observation Fibers, Warrant, Split Witnesses & Standpoint Alignment

<p align="center">
  <a href="https://hyperactivity.ai/ai">
    <img src="https://hyperactivity.ai/readme-fiber.svg?v=2" alt="Observation Fiber Fib_o(y) and Split Witness Collapse" width="100%" />
  </a>
</p>

> **Full interactive treatise with live Observation-Fiber Refinement & Standpoint Network labs:** [**`hyperactivity.ai/ai`**](https://hyperactivity.ai/ai) · Source: [`lib/observation.hypercubical`](https://hyperactivity.ai/kg?mod=lib/observation.hypercubical), [`lib/obs.hypercubical`](https://hyperactivity.ai/kg?mod=lib/obs.hypercubical)

### Knowledge as Lack of Ignorance

Consider a world whose complete history belongs to a type $H$. No finite observer accesses $h \in H$ directly; an observer receives a reading $y = o(h) \in O$ through an observation map $o : H \to O$.

A reading $y$ does **not** select a point $h \in H$. Its exact epistemic content is the **observation fiber** — the space of all world-histories compatible with $y$:

```math
\mathrm{Fib}_o(y) = \sum_{h : H} \big(o(h) \equiv y\big)
```

Every observation decomposes $H$ cleanly into disjoint fibers (`fiberEquiv` in [`lib/fiber.hypercubical`](https://hyperactivity.ai/kg?mod=lib/fiber.hypercubical)):

```math
H \;\simeq\; \sum_{y : O} \mathrm{Fib}_o(y)
```

Receiving a second observation $o_2(h) = y_2$ refines the fiber by intersecting compatible histories: it eliminates possibilities ruled out by reality without ever fabricating a point inside the remaining fiber. Knowledge is literally the *absence of ignorance* (Greek *a-lētheia*, un-concealment).

A conclusion $P : H \to \mathrm{Type}$ is **warranted** by reading $y$ of observation $o$ if and only if $P(h)$ holds across **every** history in the fiber:

```math
\mathrm{Warrants}(o, y, P) :\equiv \prod_{(h, p) : \mathrm{Fib}_o(y)} P(h)
```

Any refinement $o' : H \to O'$ factoring $o = g \circ o'$ preserves every claim warranted by the coarser observation — directly in [`lib/observation.hypercubical`](https://hyperactivity.ai/kg?mod=lib/observation.hypercubical):

```haskell
Warrants : {H O : Type} → (H → O) → O → (H → Type) → Type
Warrants o y P = (hp : fiber o y) → P (fst hp)

warrants-refine : {H O O' : Type} → (o' : H → O') → (g : O' → O) → (y' : O') → (P : H → Type) →
  Warrants (λ h → g (o' h)) (g y') P → Warrants o' y' P
warrants-refine o' g y' P w hp = w (fst hp , cong g (snd hp))
```

### Why Partial Observations Have No Self-Contained Law (`no-observed-law`)

Even when the total universe evolves by a deterministic, reversible law $s : J \to J$, the observed coordinate $o(j)$ generally obeys **no** closed transition law of its own. [`lib/observation.hypercubical`](https://hyperactivity.ai/kg?mod=lib/observation.hypercubical) proves this on the minimal reversible universe $J = \mathrm{Bool} \times \mathrm{Bool}$ with involution $s(a, b) = (a \oplus b, b)$ observed by $o(a, b) = a$:

```haskell
bxor : Bool → Bool → Bool
bxor a b = if a then not b else b

whole-step : Bool × Bool → Bool × Bool
whole-step j = (bxor (fst j) (snd j) , snd j)

whole-step-involution : (a b : Bool) → whole-step (whole-step (a , b)) ≡ (a , b)
whole-step-involution true  true  = λ i → (true , true)
whole-step-involution true  false = λ i → (true , false)
whole-step-involution false true  = λ i → (false , true)
whole-step-involution false false = λ i → (false , false)

no-observed-law : (g : Bool → Bool) → ((j : Bool × Bool) → g (fst j) ≡ fst (whole-step j)) → Empty
no-observed-law g h =
  true≢false (concat (sym (h (the (Bool × Bool) (true , false))))
                     (h (the (Bool × Bool) (true , true))))
```

At `(true, false)` any candidate law $g$ must satisfy $g(\mathrm{true}) = \mathrm{true}$, while at `(true, true)` it must satisfy $g(\mathrm{true}) = \mathrm{false}$ — a contradiction. Whenever an agent only observes $o(j)$, the unobserved fiber coordinate $b$ is real, and pretending $g(o(j))$ predicts the next observation is mathematically impossible.

### Why Token-Stream LLMs, RLHF & Lexical Safety Gates Fail Structurally

Viewed through observation fibers, the three defining failures of contemporary AI are structural impossibilities of the 1970s byte-stream paradigm:

1. **Hallucination is selecting a point inside an uncollapsed fiber.**
   An autoregressive language model samples a single token sequence from $p_\theta(x_t \mid x_{<t})$. Even when the prompt $y = o(h)$ leaves $\mathrm{Fib}_o(y)$ containing two compatible histories $(h_1, h_2)$ with $P(h_1)$ true and $\neg P(h_2)$ false, the sampler commits to one point and emits it with identical syntactic confidence to a warranted fact.
   - **Hyperactivity's resolution:** Whenever $\mathrm{Fib}_o(y)$ is not invariant under $P$, the system produces an explicit **split witness** $(h_1, h_2, p_1, p_2)$ demonstrating two compatible histories that disagree on $P$. Instead of guessing, the agent executes an **active query** (`WorldAfter`) that partitions $\mathrm{Fib}_o(y)$ until every surviving history agrees.

2. **Population-marginal RLHF erases standpoint.**
   Reinforcement Learning from Human Feedback (Christiano et al., 2017; Ouyang et al., 2022) fits a single scalar reward $r_\phi(x, y) \in \mathbb{R}$ across pooled pairwise preferences from annotators with incompatible values, goals, and epistemologies. Projecting a multi-perspective fiber onto a 1D scalar average destroys every distinct standpoint and yields sycophantic, muddled compromises.
   - **Hyperactivity's resolution:** Standpoints are explicit typed perspectives. Each participant carries a reception relation `Receive : S → M → S → A → Type` (`readings` in `lib/observation.hypercubical`):

```haskell
readings : {S M A : Type} → (S → M → S → A → Type) → S → M → Type
readings {S} {M} {A} R s u = (s' : S) × (a : A) × R s u s' a
```

   Combining multiple agents never averages their standpoints into a scalar; it takes the content-addressed **set union** of warranted claims together with explicit **split witnesses** wherever standpoints genuinely disagree.

3. **Lexical refusal gates violate framing invariance.**
   Safety classifiers and keyword filters operate on surface byte strings $w \in \Sigma^\ast$. Because surface syntax is not invariant under semantic equivalence ($\equiv_{\mathrm{sem}}$), two prompts $t_1 \equiv_{\mathrm{sem}} t_2$ with identical meaning receive opposite classifications under benign rephrasing (false refusals), while adversarial rephrasing trivially bypasses the filter (jailbreaks).
   - **Hyperactivity's resolution:** Safety invariants are typed predicates over semantic equivalence classes and observation fibers (such as `SafeGateDS` and `disagreement-blocks-gate` in `neuro/reshaping-coproc.hypercubical`), strictly invariant under path transport (`ua`).

---

## 5. No Filesystems, No Canonical State: Content-Addressed Set Union & Conserved Credit

There are no mutable source files in Hyperactivity. Every declaration, module, proof, treatise, SVG, and web route lives as an immutable SHA-256 content-addressed object inside `store/` (generalizing Unison's content-addressed code to an entire operating system and decentralized knowledge graph).

### Anatomy of `store/`

1. **Declaration-level items:** Every top-level declaration in a module is hashed individually by SHA-256 into two immutable objects:
   - `store/<item_hash>/text` — the exact source text of the declaration;
   - `store/<item_hash>/forms` — its parsed forms and content-addressed dependency hashes.
   Editing one definition creates a new item hash without mutating any existing hash anywhere in the world.
2. **Module manifests:** Each logical module (e.g. `lib/observation.hypercubical`) is a content-addressed sequence of item hashes (`store/<mod_hash>/module`).
3. **Root manifest (`store/ROOT/root`):** A single 64-hex-character SHA-256 root pointing to:
   - the `paths` map (hex-encoded logical paths $\to$ module hashes);
   - the 256-way sharded `names` tree indexing every definition name to its item hash and reverse dependents.

### Convergence by Pure Set Union (Zero Consensus Overhead)

Because every object in `store/` is immutable and addressed by the SHA-256 hash of its content, two independent machines, autonomous agents, or web browsers never conflict over mutable file paths, never need a central database, and never burn energy on a global blockchain to agree on a canonical state.

Replicating knowledge between node $A$ and node $B$ is literally **set union** of their content-addressed object sets:

```math
S_{A \cup B} = S_A \cup S_B
```

Set union is associative, commutative, and idempotent (a join-semilattice). Any subset of nodes can exchange objects in any order — over HTTP packs (`/packs`, `/roots`, `/bootstrap.tgz`), WebRTC `RTCDataChannel`s (`WANT`/`OBJ`), USB drives, or air-gapped relays — and converge to the exact same knowledge graph with zero merge conflicts.

### Conserved Multi-Dimensional Credit Without a Blockchain (`lib/credit.hypercubical`)

Traditional economies and blockchains collapse multi-dimensional provenance and contribution into a single unindexed 1D scalar token, and then require global ordering to prevent double-spending.

[`lib/credit.hypercubical`](https://hyperactivity.ai/kg?mod=lib/credit.hypercubical) defines distributed credit conservation directly over the network state with **no global scheduler as an axiom**:
- A network state `CrState = List Nat × List Nat` records every peer's held credit $c_i$ and every directed channel's in-flight credit $m_{i \to j}$, with total:

```math
\mathrm{total}(S) = \sum_i c_i + \sum_{i, j} m_{i \to j}
```

- **Allocation** moves $q$ units from peer $i$'s held balance ($k + q = c_i$) onto channel $i \to j$; **delivery** moves $q$ units from channel $i \to j$ ($k + q = m_{ij}$) to peer $j$'s held balance:

```haskell
data CrMove = cr-allocate (i : Nat) (j : Nat) (q : Nat) (k : Nat)
            | cr-deliver  (i : Nat) (j : Nat) (q : Nat) (k : Nat)

cr-move : CrMove → CrState → CrState
cr-move (cr-allocate i j q k) s = (cr-set (fst s) i k , cr-add (snd s) (cr-chan s i j) q)
cr-move (cr-deliver  i j q k) s = (cr-add (fst s) j q , cr-set (snd s) (cr-chan s i j) k)

cr-move-total : (mv : CrMove) → (s : CrState) → cr-allowed mv s →
                cr-total (cr-move mv s) ≡ cr-total s
cr-move-total (cr-allocate i j q k) s h =
  cr-alloc-sum (cr-sum (fst s)) (cr-sum (cr-set (fst s) i k))
               (cr-sum (snd s)) (cr-sum (cr-add (snd s) (cr-chan s i j) q)) q
               (cr-take (fst s) i k q (fst h) (snd (snd h)))
               (cr-give (snd s) (cr-chan s i j) q (fst (snd h)))
cr-move-total (cr-deliver i j q k) s h =
  cr-deliv-sum (cr-sum (fst s)) (cr-sum (cr-add (fst s) j q))
               (cr-sum (snd s)) (cr-sum (cr-set (snd s) (cr-chan s i j) k)) q
               (cr-give (fst s) j q (fst (snd h)))
               (cr-take (snd s) (cr-chan s i j) k q (fst h) (snd (snd h)))
```

- `lib/credit.hypercubical` further proves conservation over arbitrary finite sequences of allowed local transitions, models an observer's partial view of the network as an observation fiber, and adds receipt-bearing delivery where every transfer carries an identifier so no run can deliver the same transfer twice.
- Because every derived theorem, specialization, or artifact in `store/` permanently carries its exact SHA-256 dependency DAG, credit flows along the actual mathematical dependency graph rather than an unindexed scalar ledger.

---

## 6. Zero-Rewrite Language Lifting, Specialization Equivalence & Native Codegen

A new computing foundation cannot demand rewriting billions of lines of existing C++, Rust, and hardware specifications. Hyperactivity lifts existing ecosystems directly into typed values of the host language, specializes them along explicit cubical equivalence paths, and compiles straight to native x86-64 machine code or custom silicon.

### C++ Lifting (`tools/clang2cub.py`, `lib/cxx/`, `experiments/cxx/`)

`tools/clang2cub.py` ingests any C++ translation unit *after* Clang 18's full semantic analysis (`-ast-dump=json`, `-fdump-record-layouts`, `-fdump-vtable-layouts`, `-ast-dump`) and emits it as a typed first-order value `<unit>-tu : CTU` with zero `"unknown"` nodes:
- All names are resolved to unique declaration IDs;
- Overload resolution, template instantiation, implicit casts, and constructor/destructor calls are completely explicit;
- Alignment (`alignas`), qualified calls (`p->Base::f()`), virtual inheritance, and exact byte-level struct/vtable layouts match Clang Sema byte for byte;
- [`lib/cxx/ast.hypercubical`](https://hyperactivity.ai/kg?mod=lib/cxx/ast.hypercubical), [`lib/cxx/machine.hypercubical`](https://hyperactivity.ai/kg?mod=lib/cxx/machine.hypercubical), and [`lib/cxx/read-expr.hypercubical`](https://hyperactivity.ai/kg?mod=lib/cxx/read-expr.hypercubical) define the C++ abstract machine and expression evaluator directly inside `store/` (exercised on 7 C++ units in `experiments/cxx/`, including `book.cpp`, a real financial order book).

### Rust MIR & Full x86-64 ISA Lifting (`tools/mir2cub.py`, `tools/sail2cub.py`)

- **Rust MIR:** `tools/mir2cub.py` lifts Rust's Mid-level Intermediate Representation (`experiments/rust/book.rs`, `more.rs`) into typed host values.
- **Full Sail x86-64 ISA:** `tools/sail2cub.py` lifts the official Sail x86-64 ISA specification into [`lib/isa/x86/all.hypercubical`](https://hyperactivity.ai/kg?mod=lib/isa/x86/all.hypercubical), covering instruction decoding, linear memory, EVEX dispatch, arithmetic/logic, CPU flags, jumps/loops, and TSO/RC11 weak memory models.

### Specialization Along Explicit Equivalence Paths (`lib/specialize.hypercubical`)

In [`lib/specialize.hypercubical`](https://hyperactivity.ai/kg?mod=lib/specialize.hypercubical), compilation is **not** an opaque black-box pass you have to trust. Given a code `c` at type `Tc`, a schedule `sched` specifying held definitions, and a set of rewrite theorems from `store/` (`lib/rules.hypercubical`), `specialize Tc sched rules c` normalizes `c` on the interaction-net machine and rewrites it into specialized code `c'` **paired with an explicit path `π`** proving `eval c ≡ eval c'`:

```text
specialize(Tc, sched, rules, c)  ⟶  (c', π)    where  π : eval(c) ≡ eval(c')
```

The specializer itself never needs to be trusted or proved correct as a monolithic compiler: **if any rewrite step is wrong, the generated path `π` fails to type-check.** Normalization contributes `refl` by the quote/eval law, and every algebraic or invariant-driven rewrite carries its exact theorem witness.

### Native x86-64 JIT (`#jit`) and Standalone ELF64 (`#build`)

- [`lib/x86.hypercubical`](https://hyperactivity.ai/kg?mod=lib/x86.hypercubical) emits native x86-64 machine code for `#jit` definitions: when all arguments cross the machine-value boundary, the interaction net jumps directly into the native bytes; when an argument is symbolic/neutral, it falls back seamlessly to the cubical term.
- [`lib/elf.hypercubical`](https://hyperactivity.ai/kg?mod=lib/elf.hypercubical) packages native machine code into standalone Linux ELF64 executables (`#build`) with zero external assembler or linker required.
- [`lib/first-order.hypercubical`](https://hyperactivity.ai/kg?mod=lib/first-order.hypercubical), [`lib/emit-c.hypercubical`](https://hyperactivity.ai/kg?mod=lib/emit-c.hypercubical), [`lib/emit-c-proof.hypercubical`](https://hyperactivity.ai/kg?mod=lib/emit-c-proof.hypercubical), and [`lib/singularity.hypercubical`](https://hyperactivity.ai/kg?mod=lib/singularity.hypercubical) lower higher-order definitions and `WCmd` world commands to both standalone ISO C11 (`sg-c`, UBSan-clean, operational UB-freedom and simulation proofs `cs-compile-closed-sound`, `cs-compile-closed-no-ub`) and a zero-`gcc`, zero-libc static Linux x86-64 ELF64 binary (`sg-elf`, 699 KB via `lib/elf.hypercubical`). Crawling `store/<d>/decl` and `store/<g>/forms` cold from disk over pure-`Word` Linux syscalls (`WCmd` tags 4, 11–14) with zero baked-in signatures, both binaries verify kernel definitions, `chem/lithium-exact.hypercubical`, and `lib/singularity.hypercubical`'s own definitions cold (`sg-c` through 80.9 KB `nk-ty-s`), reject forged `forms` (`rc=1`), and pair with productive `Clock`/`Tick` Lawvere fixed points (`sg-lawvere-unfold`) and metacircular theory transport (`sg-theory-transport` via `lib/portal.hypercubical`).

### Lifting C++ → Hyperactivity → Compiling vs. `clang++ -O3` Directly

| Dimension | Compiling C++ directly (`clang++ -O3`) | Lifting C++ to Hyperactivity → Specializing → Compiling |
| --- | --- | --- |
| **Correctness & Undefined Behavior** | Opaque binary; zero proof of memory safety, absence of UB, or domain invariants. | Prove absence of UB, bounds safety, and domain invariants (e.g., exact share/cash conservation in `book.cpp`) directly over the lifted `CTU` **without rewriting a single line of C++**. |
| **Optimization Power** | Limited to conservative local/syntactic passes (`clang++` cannot assume domain invariants it cannot prove). | Invariant-driven specialization (`lib/specialize.hypercubical`) eliminates virtual dispatch, bounds checks, redundant allocations, and abstraction layers *along an explicit equivalence path* `π`. |
| **Cross-Language Composition** | Locked to C++ and fragile `extern "C"` FFI boundaries. | Prove a C++ unit (`experiments/cxx/book.cpp`) and a Rust unit (`experiments/rust/book.rs`) path-equivalent (`ua`) and transport proofs across both ecosystems. |
| **Target Flexibility** | Locked to whatever LLVM backend flags exist. | Lower the exact same term to native x86-64 ELF64 (`#build`), in-kernel `#jit`, WebAssembly (`cubical.wasm`), or custom silicon RTL/GDSII (`lib/silicon/`). |

---

## 7. Full-Stack Silicon: Circuit Lower Bounds, 3nm GDSII v6.0 & Quantum Error Correction

> **Interactive silicon lab & full technical paper:** [**`hyperactivity.ai/silicon`**](https://hyperactivity.ai/silicon) · [**`hyperactivity.ai/chip-design`**](https://hyperactivity.ai/chip-design) · Source: [`lib/silicon/hypernet16.hypercubical`](https://hyperactivity.ai/kg?mod=lib/silicon/hypernet16.hypercubical), [`lib/silicon/inet-fabric.hypercubical`](https://hyperactivity.ai/kg?mod=lib/silicon/inet-fabric.hypercubical), [`lib/bench/epfl-128.hypercubical`](https://hyperactivity.ai/kg?mod=lib/bench/epfl-128.hypercubical), [`lib/silicon/export.hypercubical`](https://hyperactivity.ai/kg?mod=lib/silicon/export.hypercubical)

The same cubical kernel extends all the way down from high-level types to physical foundry masks and fault-tolerant quantum circuits:

1. **Zero-Postulate Circuit & Layout Lower Bounds (`lib/optimality.hypercubical`, `lib/silicon/optimal.hypercubical`):**
   - Exact proof of the **4-NAND2 XOR circuit lower bound** (`xor4-strictly-optimal`, proving no 3-gate NAND2 network among all 576 topologies computes 2-input XOR);
   - Fan-in-2 causality light-cone bounds (`fanin2-lightcone-bound`, `no-64input-depth5-circuit`) and 2D/3D Design-Rule-Check (DRC) packing & $O(N)$ sweep-line bounds (`xor4-2d-area-optimal`, `sweep-linear-drc-sound`).
2. **Microarchitecture, Reliability & Security (`lib/silicon/`):**
   - Landauer $\Delta S = 0$ Bennett reversible computing (`bennett-zero-garbage`);
   - $[8, 4, 4]$ SECDED cosmic-ray ECC (`Pos8 ≡ Bit3`, `lib/silicon/ecc.hypercubical`);
   - 1-probe Boolean/ISW Differential Power Analysis masking (`MaskShareEquiv`) and Spectre SLH non-interference (`slh-noninterference`);
   - Tomasulo out-of-order execution, MESI cache coherence, and univalent zero-bank-conflict SRAM swizzling (`TensorSwizzleEquiv`).
3. **Official 128-Bit EPFL SkyWater 130nm & Quantum Compilation Benchmarks (`lib/bench/epfl-128.hypercubical`, `lib/bench/eda-quantum.hypercubical`):**
   - **Official 128-Bit EPFL Benchmarks on SkyWater 130nm (`sky130_fd_sc_hd`):** Constructive algebraic synthesis verified 100% SAT-equivalent against `lsils/benchmarks` via `yosys-abc cec`: `hy_bar128` uses **`-50.1%` Sky130 standard cells** (`1,158` vs. `2,319`, `-14.5%` area), `hy_adder128_ks` achieves **`13.0×` faster timing** (`859.4 MHz` vs. `66.1 MHz`, `17` vs. `255` AIG levels), and `hy_adder128_area` uses **`892` AIG nodes** (`-12.5%` vs. `ABC resyn2`); plus ISCAS-85 `c17` (6 NAND2 gates) and `epfl-adder8` (`-20%` vs. Yosys+ABC):

```haskell
-- Exact ISCAS-85 `c17` standard benchmark (6 NAND2 gates, 5 inputs -> 2 outputs)
c17-n22-spec : Bool → Bool → Bool → Bool → Bool → Bool
c17-n22-spec n1 n2 n3 n6 n7 =
  let n10 = nand2 n1 n3
      n11 = nand2 n3 n6
      n16 = nand2 n2 n11
  in nand2 n10 n16

c17-n23-spec : Bool → Bool → Bool → Bool → Bool → Bool
c17-n23-spec n1 n2 n3 n6 n7 =
  let n11 = nand2 n3 n6
      n16 = nand2 n2 n11
      n19 = nand2 n11 n7
  in nand2 n16 n19
```

   - **Quantum Compilation:** `[[5, 1, 3]]` quantum error-correction compiler (`PauliErr16 ≡ Syndrome4`, `no-4qubit-qec-possible`) and the **Amy/Nam Clifford+T T-count benchmark suite** (`28 → 24` T-gates on 4-Toffoli, $-14.3\%$, unitary error $1.46 \times 10^{-15}$) with executable **OpenQASM 3.0** emission.
4. **Post-von-Neumann Interaction-Net Silicon & Hierarchical GDSII Signoff (`lib/silicon/inet-fabric.hypercubical`, `hypernet.hypercubical`, `hypernet16.hypercubical`, `export.hypercubical`):**
   - **Tiny Tapeout SkyWater 130nm `tt_um_hyperinet`:** 1-cycle Lafont interaction-net processor (`3,826` Sky130 cells, `24,529.78 µm²` — **`2.63×` smaller silicon area** and **`3.99×` fewer flip-flops** than pipelined von Neumann `hypercore_soc` on the same library).
   - **16-Tile (`4×4`) GALS `HyperNet-16` vs. Pipelined RISC-V (`Verilator 5.020`, `Yosys + ABC` 7nm NLDM):** Zero central switch via low-4-bit striped `VAR_SRAM` `vxchg`, solving **`5-Queens = 10` in `29` clock cycles (`40.90 ns`) vs. `2,001` cycles (`1,758.82 ns`) on pipelined RISC-V (`69.0×` fewer cycles / `43.0×` faster physical latency)** with `84.1%` tile-cycles clock-gated, **`-18.8%` smaller per-tile silicon area**, and 1-cycle §4.5 cubical dead-tree pruning (`hn16-cubical-nq5-cert`).
   - **Byte-Level Lossless Calma GDSII v6.0 & `18,664,712`-Polygon Die (`KLayout 0.28.16`):**

```haskell
data GdsByte2 = gb2 (hi : Word) (lo : Word)
data GdsByte4 = gb4 (c3 : Word) (c2 : Word) (c1 : Word) (c0 : Word)

gb2-eta : (x : GdsByte2) → gb2 (gb2-hi x) (gb2-lo x) ≡ x
gb2-eta (gb2 hi lo) = refl
```

   - Universal lossless binary Calma GDSII v6.0 round-trip (`gds6-file-roundtrip-sound`) plus hierarchical `AREF 4×4` × `AREF 45×30` 11-layer `HYPERNET16_4X4_DIE` (`5,386` bytes representing **`129,600` placed CMOS7 cells** and **`18,664,712` polygons** with `0` DRC violations in `KLayout`).

---

## 8. Zero-Search Causal Context-Fiber Compression

> **Full report & benchmarks:** [**`hyperactivity.ai/compression`**](https://hyperactivity.ai/compression) · Source: [`lib/compress.hypercubical`](https://hyperactivity.ai/kg?mod=lib/compress.hypercubical), [`lib/compress-stream.hypercubical`](https://hyperactivity.ai/kg?mod=lib/compress-stream.hypercubical)

Intelligence is compression: discovering exact structural regularities that collapse ignorance. Applying the exact same observation-fiber machinery of Section 4 directly to ordered byte streams yields a **zero-search causal context-fiber lossless compressor** built over streaming $O(N)$ renormalized **rANS** (asymmetric numeral systems; Duda, 2009).

### How Causal Context-Fiber Coding Works

At byte position $t$ of a stream $x_0, x_1, \dots, x_{N-1}$, the causal prefix $R_{<t} = (x_0, \dots, x_{t-1})$ induces a nested tower of **context fibers** $\mathrm{Fib}_k(c_t)$ across context depths $k \in \{4, 3, 2, 1, 0\}$:

1. **Zero search, zero serialized tables:** Unlike LZ77/DEFLATE/LZMA (which search sliding windows and serialize Huffman/range tables) or neural compressors (which serialize megabytes of weights), the causal prefix $R_{<t}$ *itself* deterministically fixes the exact integer frequency counts at every depth $k$ in $O(1)$ per symbol.
2. **Exact fiber exclusion:** If symbol $x_t$ has not yet appeared in depth $k$, the encoder emits an escape and drops to depth $k - 1$, excluding all symbols already ruled out at higher depths, all the way down to earliest-content ground coding.
3. **Zero probability axioms:** Every theorem in `lib/compress.hypercubical` uses purely discrete integer frequencies, cumulative measures, equivalence fibers, and exact bignum `wdiv-mod` / `wdiv-unique` / `wmod-unique` bijections — zero measure theory, zero floats, zero postulates.
4. **Compositional telescoping & higher-order transforms:** `lib/compress.hypercubical` proves exact compositional telescoping (`cstate-feed-append`), universal round-trip losslessness (`decode (encode x) ≡ x`) for all `List Word`, higher-order DAG/$\lambda$-template factoring (`MTerm`), bidirectional context-class quotienting (`compress-record-upto`), 1D/2D cubical boundary operators ($\Delta_x, \Delta_{xy}$), and group-orbit symmetry factoring (`GroupAction`), alongside Kraft's inequality, the exact rANS Shannon-rate upper bound, and pigeonhole incompressibility (`lib/optimality.hypercubical`).

### Exact `refl` Benchmarks vs. `deflate -9`, `gzip -9`, `bzip2 -9`, `lzma`

Every byte count below (including all framing) reduces to `refl` inside `build/cubical` on the exact same 7 byte streams (compared against Python 3.12 `zlib 1.3`, `bz2`, and `lzma` on identical inputs):

| # | Dataset (exact byte stream in `lib/compress.hypercubical`) | Raw | **`lib/compress`** | Ratio (bits/B) | `deflate -9` | `gzip -9` | `bzip2 -9` | `lzma2 -9` | `xz/lzma -9` |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | `sample-record-prose` (UTF-8 technical prose) | 846 B | **455 B** | 1.86× (4.30) | 466 B | 484 B | 519 B | 512 B | 568 B |
| 2 | `sample-fiber-code` (Unicode `.hypercubical` source) | 863 B | **239 B** | 3.61× (2.22) | 272 B | 290 B | 307 B | 276 B | 332 B |
| 3a | `sample-sec4` (standalone section) | 630 B | **335 B** | 1.88× (4.25) | 335 B | 353 B | 385 B | 380 B | 436 B |
| 3b | `sample-sec4` given `sample-record-prose` (`compress-given`) | 630 B | **280 B** | 2.25× (3.56) | 312 B (`zdict`) | — | — | — | — |
| 4a | `sample-quotient` (literal `compress-record`) | 134 B | **56 B** | 2.39× (3.34) | 59 B | 77 B | 88 B | 60 B | 116 B |
| 4b | `sample-quotient` (`compress-record-upto digit-class`) | 134 B | **46 B** | 2.91× (2.75) | 59 B | 77 B | 88 B | 60 B | 116 B |
| 5 | `sample-grid` (12×12 bilinear 2D surface, $\Delta_{xy}$) | 144 B | **32 B** | 4.50× (1.78) | 72 B | 90 B | 133 B | 76 B | 132 B |
| 6 | `sample-skewed` (64 B 7:1 periodic stream) | 64 B | **10 B** | 6.40× (1.25) | 9 B | 27 B | 43 B | 17 B | 72 B |
| 7 | `sample-hash` (32 B SHA-256 hex store root prefix) | 32 B | **30 B** | 1.07× (7.50) | 31 B | 49 B | 61 B | 36 B | 88 B |

---

## 9. Neuroscience & Closed-Loop Neural Co-Processors

> **Full 101 → Modern Frontier → Computational Closure interactive treatise with live FitzHugh–Nagumo phase-plane & two-learner BCI labs:** [**`hyperactivity.ai/neuro`**](https://hyperactivity.ai/neuro) · Run locally: `./build/cubical neuro/lyapunov.hypercubical`

### From Ion Channels to Excitable Active Pairs

Every cortical action potential begins with electrochemical equilibrium across a lipid bilayer. The **Nernst potential** for ion species $s$ and the **Hodgkin–Huxley** (1952) 4D membrane conductance equations govern single-neuron electrophysiology:

```math
E_s = \frac{RT}{z_s F}\ln\frac{[s]_{\text{out}}}{[s]_{\text{in}}}, \qquad C_m \frac{dV}{dt} = I_{\text{ext}} - \bar{g}_{\text{Na}} m^3 h (V - E_{\text{Na}}) - \bar{g}_{\text{K}} n^4 (V - E_{\text{K}}) - g_L (V - E_L)
```

Because fast $\text{Na}^+$ activation $m(t)$ equilibrates almost instantaneously while $\text{K}^+$ activation $n$ and $\text{Na}^+$ inactivation $h$ covary along a 1D slow manifold, the 4D conductance dynamics reduce cleanly to the 2D **FitzHugh–Nagumo** oscillator (FitzHugh, 1961; Nagumo, 1962) — a local active pair between fast membrane voltage $v$ and slow recovery $w$:

```math
\frac{dv}{dt} = v - \frac{v^3}{3} - w + I_{\text{ext}}, \qquad \frac{dw}{dt} = \varepsilon\,(v + a - b\,w)
```

### Intrinsic Manifolds, Output-Null Fibers & Two-Learner Co-Adaptation

Large cortical populations $x(t) \in \mathbb{R}^N$ do not roam freely in $\mathbb{R}^N$; recurrent synaptic weights confine neural activity to a low-dimensional **intrinsic manifold** $\mathcal{M} \subset \mathbb{R}^N$ ($d \ll N$; Sadtler et al., 2014). A downstream motor readout or brain–computer interface (BCI) decoder $y = C x$ induces an **output-null observation fiber** ([`neuro/descent.hypercubical`](https://hyperactivity.ai/kg?mod=neuro/descent.hypercubical)):

```math
\mathrm{Fib}_C(y) = x_0 + \ker C
```

Motor preparation, internal planning, and trial-to-trial variability live inside $\ker C$ without prematurely driving the actuator.

In a modern **closed-loop neural co-processor** (Carmena, Orsborn, Burden, & Madduri, 2023), *both* the biological brain ($B$) and the electronic co-processor ($C$) adapt simultaneously online with learning rates $\eta_B, \eta_C > 0$ and cross-coupling gains $\gamma_1, \gamma_2$:

```math
\frac{de_B}{dt} = -\eta_B e_B + \gamma_1 e_C, \qquad \frac{de_C}{dt} = \gamma_2 e_B - \eta_C e_C
```

If cross-coupling outpaces individual damping, the brain and decoder chase each other into runaway instability.

### Universal Symbolic SOS Lyapunov & Control Barrier Certificates Over $\mathbb{Q}$

Numerical simulation over IEEE-754 floats can only test finite sample points; it can never rule out an adversarial parameter corner in continuous space. Instead, Hyperactivity decides polynomial **Sum-of-Squares (SOS)** identities in exact normal form over the universal polynomial ring $\mathbb{Q}[x_1, \dots, x_n]$ via [`chem/algebra.hypercubical`](https://hyperactivity.ai/kg?mod=chem/algebra.hypercubical) (`al-id`), so every stability and safety theorem holds in **every commutative ring containing $\mathbb{Q}$** for **all** real parameter values simultaneously:

1. **Asymmetric Two-Learner Lyapunov SOS ([`neuro/lyapunov.hypercubical`](https://hyperactivity.ai/kg?mod=neuro/lyapunov.hypercubical), `two-learner-asym-sos-id`):**
   For weighted Lyapunov energy $V(e_B, e_C) = p_1 e_B^2 + p_2 e_C^2$ ($p_1, p_2 > 0$),

```math
4 p_1 \eta_B \left(-\frac{1}{2}\frac{dV}{dt}\right) = \big(2 p_1 \eta_B e_B - (p_1 \gamma_1 + p_2 \gamma_2)\,e_C\big)^2 + \Big(4 p_1 p_2 \eta_B \eta_C - (p_1 \gamma_1 + p_2 \gamma_2)^2\Big)\,e_C^2
```

   Setting $p_1 \gamma_1 = p_2 \gamma_2$ recovers the sharp Hurwitz stability threshold $\eta_B \eta_C > \gamma_1 \gamma_2$ constructively. Here is the exact code from [`neuro/lyapunov.hypercubical`](https://hyperactivity.ai/kg?mod=neuro/lyapunov.hypercubical) (decided directly to `λ i → true` by the kernel):

```haskell
two-learner-sym-sos : (A : Type) → (A → A → A) → (A → A → A) → (BQ → A) → List A → A × A
two-learner-sym-sos A ad mu q vs =
  (λ g sub sq →
    let eb = g 0; ec = g 1; gm = g 2; x = g 3; y = g 4
        lhs = mu eb (ad (sub (mu eb (sq x)) (mu (q (fn-q 2 1)) (mu gm (mu x y)))) (mu ec (sq y)))
        rhs = ad (sq (sub (mu eb x) (mu gm y))) (mu (sub (mu eb ec) (sq gm)) (sq y))
    in (lhs , rhs))
  (λ i → al-nth A (q (fn-q 0 1)) i vs) (al-gsub A ad mu q) (al-gsq A mu)

two-learner-sym-sos-id : al-id 5 two-learner-sym-sos ≡ true
two-learner-sym-sos-id = λ i → true
```

2. **Forward-Invariant Control Barrier Functions ([`neuro/barrier.hypercubical`](https://hyperactivity.ai/kg?mod=neuro/barrier.hypercubical)):**
   For the safe ball $h(e_B, e_C) = R^2 - e_B^2 - e_C^2 \ge 0$, Nagumo forward-invariance requires $\dot{h} + \alpha h \ge 0$. Letting $d_1 = 2\eta_B - \alpha$ and $d_2 = 2\eta_C - \alpha$, `cbf-2learner-safe-sos-id` proves:

```math
d_1 \cdot \big(\dot{h} + \alpha h\big) = d_1 \alpha R^2 + (d_1 e_B - 2\gamma e_C)^2 + (d_1 d_2 - 4\gamma^2)\,e_C^2
```

   guaranteeing that whenever $2\eta_B > \alpha > 0$ and $(2\eta_B - \alpha)(2\eta_C - \alpha) \ge 4\gamma^2$, the coupled brain–machine state can never exit the safe ball. In the same module, `fhn-driven-trapping-sos-id` proves a global trapping-region SOS certificate for the full externally driven FitzHugh–Nagumo neuron for *every* stimulation current $I_{\text{ext}}$, recovery offset $a$, and $b > 0$.

3. **Interface-Reshaping Guarded Co-Processors (`lib/polyproc.hypercubical`, `neuro/reshaping-coproc.hypercubical`):**
   Real neural interfaces do not keep a fixed channel layout: electrodes drift, channels open and close, and decoders rebind mid-run. [`lib/polyproc.hypercubical`](https://hyperactivity.ai/kg?mod=lib/polyproc.hypercubical) generalizes guarded processes to indexed **interface-reshaping processes** `RProc κ D A i` over dynamic signatures `DynSig` (with exact round-trip path `gproc-rproc-id`). [`neuro/reshaping-coproc.hypercubical`](https://hyperactivity.ai/kg?mod=neuro/reshaping-coproc.hypercubical) wraps stimulation commands in `SafeGateDS`, proving `disagreement-blocks-gate`: whenever two histories in the observation fiber disagree on safety, the split witness structurally blocks stimulation from firing.

4. **Cognitive Geometry, Synesthesia, Addiction & Dyadic Co-Processing:**
   - [`neuro/inhibition.hypercubical`](https://hyperactivity.ai/kg?mod=neuro/inhibition.hypercubical) — REBUS (Relaxed Beliefs Under Psychedelics; Carhart-Harris & Friston, 2019) precision-weighted prior relaxation over hierarchical predictive fibers.
   - [`neuro/opioid-addiction.hypercubical`](https://hyperactivity.ai/kg?mod=neuro/opioid-addiction.hypercubical) — Opponent-process withdrawal trough, pseudoaddiction escalation trap, and self-continuation attractor SOS.
   - [`neuro/synesthesia.hypercubical`](https://hyperactivity.ai/kg?mod=neuro/synesthesia.hypercubical) — Cross-modal synthetic synesthesia as path transport between sensory representations (`hyperactivity.ai/synesthesia`).
   - [`neuro/eye-contact.hypercubical`](https://hyperactivity.ai/kg?mod=neuro/eye-contact.hypercubical) — Zero-latency dyadic eye-contact co-processing at the speed of light.

---

## 10. Relational Physics, Exact Quantum Chemistry & Living Matter

> **Full 101 → Modern Frontier → Computational Closure interactive treatise with live $SU(2)$ spin-network & bioelectric morphogenesis labs:** [**`hyperactivity.ai/physics`**](https://hyperactivity.ai/physics)

### Relational Quantum Mechanics & Loop Quantum Gravity (`lqg/`)

1. **Relational Quantum Mechanics ([`lqg/rqm.hypercubical`](https://hyperactivity.ai/kg?mod=lqg/rqm.hypercubical)):**
   In RQM (Rovelli, 1996; Di Biagio & Rovelli, 2021), physical variables take values only relative to local interactions between systems — never relative to an unphysical God's-eye global state. This is the exact physical counterpart of Hyperactivity's observation fibers and interaction-net active pairs.
2. **$SU(2)$ Spin Networks & Discrete Quantum Geometry:**
   In Loop Quantum Gravity (Rovelli & Smolin, 1995; Ashtekar & Lewandowski, 2004), spacetime at the Planck scale is a spin network — a graph $\Gamma = (V, E)$ whose edges carry irreducible $SU(2)$ representations $j_e \in \frac{1}{2}\mathbb{Z}_{\ge 0}$ and whose vertices carry intertwiners obeying Gauss closure (triangle inequalities) $|j_1 - j_2| \le j_3 \le j_1 + j_2$. Any surface $\mathcal{S}$ punctured by edges $e \cap \mathcal{S}$ has a discrete quantum area spectrum:

```math
A(\mathcal{S}) = 8\pi \gamma \ell_P^2 \sum_{e \cap \mathcal{S}} \sqrt{j_e(j_e + 1)}
```

   [`lqg/hamiltonian-vertex.hypercubical`](https://hyperactivity.ai/kg?mod=lqg/hamiltonian-vertex.hypercubical) implements Thiemann's Quantum Spin Dynamics (QSD) gravitational and electromagnetic Hamiltonian vertex actions; [`chem/magloop.hypercubical`](https://hyperactivity.ai/kg?mod=chem/magloop.hypercubical) encodes $U(1)$ magnetic loop flux sectors; and [`lqg/semiclassical-bridge.hypercubical`](https://hyperactivity.ai/kg?mod=lqg/semiclassical-bridge.hypercubical) constructs the semiclassical bridge to continuum geometry.
3. **Ashtekar–Pawlowski–Singh Loop Quantum Cosmology Big Bounce ([`lqg/cosmology-bounce.hypercubical`](https://hyperactivity.ai/kg?mod=lqg/cosmology-bounce.hypercubical)):**
   Holonomy corrections replace the singular classical Friedmann equation with the LQC modified Friedmann equation:

```math
H^2 = \frac{8\pi G}{3}\,\rho\!\left(1 - \frac{\rho}{\rho_c}\right)
```

   `lqg-lqc-hubble-max-sos-id` proves the exact universal SOS identity over $\mathbb{Q}$:

```math
\rho_c^2 - 4\rho(\rho_c - \rho) = (2\rho - \rho_c)^2 \ge 0
```

   proving universally that $H^2 \le \frac{2\pi G}{3}\rho_c$ (eliminating the Big Bang singularity) and vanishes at $\rho = \rho_c$:

```haskell
lqg-lqc-hubble-poly : (A : Type) → (A → A → A) → (A → A → A) → (BQ → A) → List A → A × A
lqg-lqc-hubble-poly A ad mu q vs =
  (λ g sub sq →
    let rho = g 0; rhoc = g 1
        q2 = q (fn-q 2 1); q4 = q (fn-q 4 1)
        lhs = sub (sq rhoc) (mu q4 (mu rho (sub rhoc rho)))
        rhs = sq (sub (mu q2 rho) rhoc)
    in (lhs , rhs))
  (λ n → al-nth A (q (fn-q 0 1)) n vs) (al-gsub A ad mu q) (al-gsq A mu)

lqg-lqc-hubble-max-sos-id : al-id 2 lqg-lqc-hubble-poly ≡ true
lqg-lqc-hubble-max-sos-id = λ i → true
```

   At the bounce $\rho = \rho_c$, `lqg-lqc-bounce-accel-id` proves that the holonomy-corrected Raychaudhuri bracket equals $-3(\rho_c + P)$, yielding strictly repulsive quantum-geometry acceleration $\ddot{a}/a = +4\pi G(\rho_c + P) > 0$ for *every* matter equation of state with $\rho_c + P > 0$, alongside exact 4D Pachner $1 \leftrightarrow 5$ and $2 \leftrightarrow 4$ simplicial invariance of $\chi(S^3) = V - E + F - T = 0$.

### Relativistic Atomic Structure & Superconductivity (`phys/`)

- **Pauli–Dirac Clifford Algebra & the $Z \le 137$ Heavy-Element Stability Limit ([`phys/relativistic-atom.hypercubical`](https://hyperactivity.ai/kg?mod=phys/relativistic-atom.hypercubical)):** Proves $(\mathbf{p} \cdot \boldsymbol{\sigma})^2 = (p_x^2 + p_y^2 + p_z^2) I_2$ over any commutative ring containing $\mathbb{Q}$, derives the Foldy–Wouthuysen relativistic Hamiltonian expansion, proves spin–orbit center-of-gravity conservation, and certifies the exact $Z\alpha < 1$ ($Z \le 137$) Dirac Coulomb stability threshold.
- **Superconductivity ([`phys/superconductivity.hypercubical`](https://hyperactivity.ai/kg?mod=phys/superconductivity.hypercubical)):**
  - **Bogoliubov–de Gennes (BdG) Nambu–Gorkov Hamiltonian:** Exact gapped quasiparticle dispersion $E_k^2 = \xi_k^2 + \Delta^2$ (`phys-bdg-dispersion-id`) and Bogoliubov unitary diagonalization norm identity $E^2 + O^2 = (u^2 + v^2)^2(\xi^2 + \Delta^2)$:

```haskell
phys-bdg-char-poly : (A : Type) → (A → A → A) → (A → A → A) → (BQ → A) → List A → A × A
phys-bdg-char-poly A ad mu q vs =
  (λ g sub sq →
    let ev = g 0; xi = g 1; del = g 2
        lhs = sub (mu (sub ev xi) (ad ev xi)) (sq del)
        rhs = sub (sq ev) (ad (sq xi) (sq del))
    in (lhs , rhs))
  (λ n → al-nth A (q (fn-q 0 1)) n vs) (al-gsub A ad mu q) (al-gsq A mu)

phys-bdg-dispersion-id : al-id 3 phys-bdg-char-poly ≡ true
phys-bdg-dispersion-id = λ i → true
```

  - **Ginzburg–Landau Mexican-Hat Free Energy SOS (`phys-gl-mexican-hat-sos-id`):** $2\beta(-a|\psi|^2 + \frac{\beta}{2}|\psi|^4 + \frac{a^2}{2\beta}) = (\beta|\psi|^2 - a)^2 \ge 0$, proving spontaneous $U(1)$ symmetry breaking at $|\psi|^2 = a/\beta$ with exact condensation energy $-a^2/(2\beta)$.
  - **Two-Site Hubbard Superexchange (`phys-hubbard-exchange-id`):** Exact characteristic polynomial $(2\lambda - U)^2 = U^2 + 16 t^2$, proving antiferromagnetic singlet stabilization over the $E_T = 0$ triplet for all $t \ne 0, U > 0$.

### Zero-Float Ab Initio Quantum Chemistry & Chemical Identity (`chem/`)

- **Exact Rational Variational Quantum Chemistry (`chem/`):** Exact rational variational Ritz upper bounds for $\text{H}_2$, Bareiss–Sylvester exact rational Helium ground-state energy, $N = 3$ Lithium pair-repulsion base (`chem/pair-base-n.hypercubical`), and many-body fermionic Canonical Anticommutation Relations (CAR) algebra — all computed with zero floating-point rounding.
- **Structural Chemical Identity:** Resolves IUPAC's chemical entity-resolution crisis by separating digest equality, definitional equality, and tautomeric/conformational equivalences as explicit cubical paths (`ua`) rather than collapsing them into lossy InChI strings.

### Peptide Kinematics, Autonomous Sequence-to-Fold & Bioelectric Morphogenesis (`fold/`, `chem/`)

- **Zero-Drift Rational $SO(3)/SE(3)$ Peptide Kinematics & Autonomous Folding ([`fold/backbone-kinematics.hypercubical`](https://hyperactivity.ai/kg?mod=fold/backbone-kinematics.hypercubical), `fold/se3-monoid.hypercubical`, `fold/amino-ramachandran.hypercubical`, `fold/sequence-folder.hypercubical`):** Constructs all-atom 3D protein backbones using exact rational Cayley/Rodrigues parameterizations — eliminating floating-point drift and quaternion renormalization error permanently. Hyperfactoring 3D folding onto $SE(3)$ monoid hinge invariance and a 20-amino-acid discrete Ramachandran quotient drives an autonomous sequence-to-fold engine selecting native $\beta$-hairpin and $\alpha$-helical folds on **Chignolin (`GYDPETGTWG`, PDB 1UAO)** and **Trp-cage (PDB 1L2Y)** with exact steric and hydrogen-bond verification.
- **Bioelectric Morphogenesis (`neuro/barrier.hypercubical`, `chem/`):** Models Michael Levin's morphogenetic bioelectric networks as gap-junction interaction nets ($I_{ij} = g_{\text{gj}}(V_j - V_i)$) relaxing bistable cellular potentials into stable isopotential anatomical domains, with exact mean-field + isopotential-difference dissipation splitting (`bioelectric-gap-dissipation-sos-id`).

---

## 11. The Decentralized Web Seed & Sovereign P2P Mesh

<p align="center">
  <a href="https://hyperactivity.ai/kg">
    <img src="https://hyperactivity.ai/readme-kg.svg?v=2" alt="Decentralized Content-Addressed P2P Web Seed" width="100%" />
  </a>
</p>

[hyperactivity.ai](https://hyperactivity.ai) is not a separate JavaScript frontend talking to a microservice backend. The entire site is served directly out of `store/` by [`lib/server.hypercubical`](https://hyperactivity.ai/kg?mod=lib/server.hypercubical) — under a single canonical chrome layer (`s-chrome-html`) unifying every route — and acts as a live seed node for a decentralized peer-to-peer knowledge web:

1. **Zero-Git Pool Replication (`/packs`, `/roots`, `/root`, `/bootstrap.tgz`):**
   Any machine can clone and synchronize the entire `store/` directly over plain HTTP without GitHub, verifying every received object locally by its SHA-256 hash.
2. **Browser-Local Content-Addressed Store (`IndexedDB`):**
   Every visitor to [`hyperactivity.ai/kg`](https://hyperactivity.ai/kg) runs a local `IndexedDB` object store (`hx-cas-v1`) keyed by SHA-256 hash, synchronized across open tabs via `BroadcastChannel`. Once an object is fetched, it lives locally in your browser and never needs to hit the origin server again.
3. **Stateless WebRTC P2P Object Mesh, Ambient Co-Presence & Distributed Cut Reduction (`lib/distnet.hypercubical`):**
   `/api/p2p/signal` and `/api/live/pulse` in `lib/server.hypercubical` hold **zero open sockets** and execute **zero server compute** — the seed only relays lightweight WebRTC SDP/ICE handshakes and presence pulses. Once two browsers complete their handshake, `WANT` / `OBJ` set-union object sync flows directly peer-to-peer over `RTCDataChannel`, while `lib/distnet.hypercubical` (`/peer/manifest`, `/peer/reduce`, CLI `peers` / `peer-pull` / `delegate`) partitions and reduces interaction-net cuts across sovereign peer nodes and `lib/portal.hypercubical` delivers results by mathematical transport.
4. **Client-Side Kernel Execution (`cubical.wasm`):**
   [`hyperactivity.ai/kernel`](https://hyperactivity.ai/kernel) runs the interaction-net machine compiled to WebAssembly directly inside your browser.
5. **Universal Knowledge Graph Portals:**
   Every `.hypercubical` module or definition referenced on any page of `hyperactivity.ai` is a live clickable portal that opens its exact content-addressed source, SHA-256 key, and dependency neighborhood right over the page.

---

## 12. Reference: Commands, Store Mechanics & Primary Literature

### Build, Inspect & Run Locally

Both `.hypercubical` and `.cub` extensions resolve bidirectionally across the bootstrap's module reader, `lib/guard.hypercubical`, and `/api/kg/module`:

```sh
# Build the machine (gcc or clang, and make)
make

# Interactive loop over store/
./hyperactive

# Inspect any definition, its SHA-256 item key, and reverse dependents
./hyperactive def two-learner-asym-sos-id
./hyperactive def warrants-refine
./hyperactive def lqg-lqc-hubble-max-sos-id

# Execute any module with its imports
./build/cubical neuro/lyapunov.hypercubical
./build/cubical lqg/cosmology-bounce.hypercubical
./build/cubical lib/observation.hypercubical

# Run the repository guard (verifies frozen bootstrap hashes + store integrity)
./build/cubical lib/guard.hypercubical

# Check the full core + library
make library

# Compute Brunerie's number (-2) on 1 and 4 parallel workers
make brunerie

# Boot your own local mirror of hyperactivity.ai on port 8080
./build/cubical lib/server.hypercubical 8080
```

### Repository Layout

```text
src/            Bootstrap: the interaction laws and the store, compiled once by make
store/          Immutable SHA-256 content-addressed objects (read & written via ./hyperactive)
  lib/          Self-hosted cubical checker, observation fibers, rANS compression, specialization,
                C++/Rust/x86 lifting, conserved credit, interface-reshaping RProc, distnet, portal,
                silicon/GDSII/QEC, web server
  neuro/        Two-learner BCI Lyapunov & CBF SOS certificates, FitzHugh–Nagumo, RProc co-processors,
                REBUS inhibition, opioid opponent-process SOS, synthetic synesthesia, eye-contact co-processing
  chem/         Universal polynomial algebra over Q, zero-float ab initio H2 / He / Li, CAR algebra,
                magnetic loops, chemical identity, bioelectric morphogenesis
  phys/         Relativistic atomic structure (Dirac / Foldy–Wouthuysen, Z <= 137), superconductivity (BdG/GL/Hubbard)
  lqg/          Relational Quantum Mechanics, SU(2) spin networks, Thiemann QSD vertex, semiclassical bridge, LQC bounce
  fold/         Exact rational SO(3)/SE(3) peptide kinematics, Ramachandran quotient & autonomous sequence-to-fold engine
  docs/         Monograph, interactive domain treatises (/ai, /neuro, /physics, /compression, /chip-design), web apps
tools/          Lifters (clang2cub.py, mir2cub.py, sail2cub.py, agda2cub.py) & assembly helpers
experiments/    Brunerie's number (-2), C++ (book.cpp..seventh.cpp) & Rust (book.rs, more.rs) lifting suites
Makefile        Machine, library, and Brunerie build targets
hyperactive     Entrypoint symlink to build/cubical
LICENSE         Hyperactive Anti-Capitalist Commons & Commercial License v1.0
```

### Primary Literature & Intellectual Lineage

- **Interaction Nets:** Y. Lafont, *"Interaction Nets"*, POPL 1990.
- **Cubical Type Theory & Univalence:** C. Cohen, T. Coquand, S. Huber, A. Mörtberg, *"Cubical Type Theory: a constructive interpretation of the univalence axiom"*, TYPES 2015 / FLAP 2018; G. Brunerie, *"On the homotopy groups of spheres in homotopy type theory"*, PhD Thesis 2016.
- **Asymmetric Numeral Systems:** J. Duda, *"Asymmetric numeral systems"*, arXiv:0902.0271, 2009.
- **Excitable Membranes & Neural Co-Processors:** A. L. Hodgkin & A. F. Huxley, *J. Physiol.* 1952; R. FitzHugh, *Biophys. J.* 1961; J. Nagumo et al., *Proc. IRE* 1962; P. T. Sadtler et al., *Nature* 2014; A. D. Madduri, S. A. Burden, A. L. Orsborn, *"A game-theoretic model for co-adaptive brain-machine interfaces"*, 2023; R. L. Carhart-Harris & K. J. Friston, *"REBUS and the Anarchic Brain"*, *Pharmacol. Rev.* 2019.
- **Relational Physics & Loop Quantum Gravity:** C. Rovelli, *"Relational Quantum Mechanics"*, *Int. J. Theor. Phys.* 1996; A. Di Biagio & C. Rovelli, *Found. Phys.* 2021; C. Rovelli & L. Smolin, *Nucl. Phys. B* 1995; A. Ashtekar, T. Pawlowski, P. Singh, *Phys. Rev. Lett.* 96, 141301 (2006).
- **Bioelectric Morphogenesis:** M. Levin, *"Bioelectric signaling: Reprogrammable circuits underlying embryogenesis, regeneration, and cancer"*, *Cell* 2021.

---

## 13. License

Copyright © 2026 **Hyperactive Labs, Inc.** and **Avik Jain**. Governed by the [**Hyperactive Anti-Capitalist Commons & Commercial License v1.0 (`LICENSE`)**](LICENSE) — a **Copyfarleft** dual license:

- **Free & Copyfarleft for the Non-Capitalist Commons:** Natural persons acting outside capitalist enterprise, students, educators, independent scientists, non-commercial public/academic research groups, charitable nonprofits, and democratically governed 100% worker-owned cooperatives have full royalty-free rights to read, compile, execute, verify, modify, and share — conditioned on **strong network share-alike copyleft** (every shared or network-served derivative remains under [`LICENSE`](LICENSE) with full corresponding machine-readable source disclosed).
- **Paid Commercial License Required for Capitalist Entities & Commercial AI:** Zero rights are granted under [`LICENSE`](LICENSE) to any for-profit corporation, venture-backed or investor-owned enterprise, hedge fund or trading firm, commercial cloud host, semiconductor vendor, or commercial AI laboratory — including internal corporate R&D, evaluation, hardware fabrication, and AI/LLM scraping, pre-training, fine-tuning, distillation, or RAG ingestion. Commercial licensing inquiries: [**root@hyperactivity.ai**](mailto:root@hyperactivity.ai).
