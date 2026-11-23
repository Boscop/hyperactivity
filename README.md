# Hyperactivity

**A decentralized operating system language for collective superintelligence.**

*Knowledge graphs as interaction networks. Mathematical knowledge graphs as cubical interaction networks.*

> **Live decentralized web seed:** [hyperactivity.ai](https://hyperactivity.ai)

| Portal | What It Is |
| --- | --- |
| [**Knowledge Graph (`/kg`)**](https://hyperactivity.ai/kg) | Interactive graph & live source explorer backed by browser-local `IndexedDB` content-addressed storage and WebRTC P2P object sync |
| [**Browser Kernel (`/kernel`)**](https://hyperactivity.ai/kernel) | Real cubical interaction-net kernel (`cubical.wasm`) running directly in the browser with zero server compute |
| [**Interaction Net Lab (`/nets`)**](https://hyperactivity.ai/nets) | Live multi-scale visual compiler (`Macro` / `Structural` / `Atomic`) for arithmetic, binary carry, parallel certified sort, cubical paths, Kan boxes, univalence & guarded artificial life |
| [**The Paper (`/paper` · `/pdf`)**](https://hyperactivity.ai/paper) | Full monograph + 102-page vector PDF export |
| [**Compression (`/compression`)**](https://hyperactivity.ai/compression) | Zero-search causal context-fiber compression (`lib/compress.hypercubical`, `lib/compress-stream.hypercubical`) beating `gzip -9`, `bzip2 -9`, and `lzma` |
| [**AI (`/ai`)**](https://hyperactivity.ai/ai) | 101 → Frontier → Closure treatise on observation fibers, warrant, split witnesses, active querying, and standpoint alignment |
| [**Neuroscience (`/neuro`)**](https://hyperactivity.ai/neuro) | 101 → Frontier → Closure treatise on FitzHugh–Nagumo membranes, intrinsic manifolds, two-learner BCI co-adaptation, symbolic SOS Lyapunov/CBF safety over ℚ, and interface-reshaping `RProc` |
| [**Physics & Chemistry (`/physics`)**](https://hyperactivity.ai/physics) | 101 → Frontier → Closure treatise on Relational Quantum Mechanics, LQG spin networks & Big Bounce, Dirac fine structure, zero-float ab initio chemistry, superconductivity, exact peptide kinematics, and bioelectric morphogenesis |
| [**Chip Design (`/chip-design`)**](https://hyperactivity.ai/chip-design) | Full-stack silicon from algebraic circuit lower bounds to GDSII v6.0 3nm layout and GALS cubical inet-tile RTL |
| [**Tour (`/learn`)**](https://hyperactivity.ai/learn) · [**Music (`/music`)**](https://hyperactivity.ai/music) | Ground-up interactive textbook with inline live nets & Pythagorean harmonic engine compiled directly from Hyperactivity ratios |

---

## Why Everything Is an Interaction Network

1970s computing built the world on untyped byte streams, broken equality, global clocks, and location-addressed servers. Real physical, biological, neural, and cognitive systems do not work that way: synapses, ion channels, $SU(2)$ spin-network vertices, biochemical reaction complexes, and communicating agents are **local, concurrent interaction networks** of active pairs.

Hyperactivity gives interaction networks universal cubical semantics, univalent identity (`ua`), guarded corecursion (`▸ κ`), and content-addressed convergence by pure set union — with no canonical server state and no blockchain.

---

## The Core Machine

There is no primitive thing, only a primitive law of interaction. A term is a net of cells, and the only thing that ever happens is that two cells meet and are replaced.

### Vocabulary

- **Cell:** A node with one *principal port* and zero or more auxiliary ports.
- **Value:** A cell that offers itself at its principal port (`λ`, pair, type, constructor, interval point, system).
- **Operation:** A cell that demands a value at its principal port (`apply`, `first`, `second`, `path-apply`, `compose`, `case`, `subst`, `duplicate`, `erase`).
- **Wire:** Connects two ports. Whoever arrives first leaves what it has; the second finds it.
- **Active pair:** An operation's principal port wired to a value's principal port. This is the only place anything happens.
- **Step:** The two cells of an active pair are removed and replaced by a small net on the wires they held. Nothing else in the universe is inspected — disjoint active pairs reduce concurrently across workers without locks.

```text
   arg ──┐
         app ●────● λ ── body      ⟶      out ── body (arg wired into parameter)
   out ──┘
```

### Interaction Laws

Read each line as: *operation* meets *value* ⟶ replacement net (from `src/core.h` and `src/net.c`).

**Functions, pairs & guarded corecursion**

```text
apply(arg)      meets  λ               ⟶  body with arg wired in for parameter
first           meets  pair(a, b)      ⟶  a   (b erased)
second          meets  pair(a, b)      ⟶  b   (a erased)
force           meets  delayed body    ⟶  body
```

**Cubical paths & dimension substitution**

```text
path-apply(r)   meets  path-λ i. t     ⟶  subst [i := r] facing t
path-apply(0|1) with known endpoints   ⟶  endpoint directly (path never read)
path-apply(0|1) meets  blocked path    ⟶  endpoint read from path's type
subst           meets  interval/face   ⟶  substituted interval or face
subst           meets  λ, pair, Π, Σ   ⟶  same cell, subst facing each part
subst           meets  path-λ j. t     ⟶  path-λ j. (subst facing t), α-safe
subst           meets  system          ⟶  branch whose condition became true
subst           meets  held value      ⟶  stays pending until read
```

**Optimal sharing & demand-driven evaluation**

```text
duplicate       meets  value w/ parts  ⟶  value held once; each side gets a reference
duplicate       meets  atomic value    ⟶  two copies
erase           meets  value           ⟶  gone; erase faces each part
anything        meets  reference       ⟶  top cell copied out; subparts stay references
read before value exists               ⟶  waits (waiting is not a step)
```

**Kan composition, Glue & Univalence (`ua`)**

```text
compose         meets  total tube      ⟶  tube's far end
compose         with partial tube      ⟶  clause for head type (Π, Σ, Path, U, Glue, Data)
Glue / glued    when face is true      ⟶  type / value on that face
unglue          on face                ⟶  equivalence map applied
unglue          meets  glued value     ⟶  base
transport       meets  constructor     ⟶  constructor's transport clause
transport       on constant type line  ⟶  identity
```

Every composition clause is itself a term of the language, not ad-hoc runtime code. `ua` is a `Glue` type: transporting any proof or program along an equivalence computes directly via the Glue and composition rules above.

---

## Inside the Content-Addressed Store (`store/`)

Source code and documents are not mutable files — they are immutable SHA-256 objects inside `store/`, read and written through `./hyperactive`:

- **Core, Lifting & Compression (`lib/`)**: Cubical type checker written in the language itself, zero-rewrite C++/Rust/x86-64 lifting, verified C-first specialization, streaming $O(N)$ renormalized rANS causal context-fiber compression, conserved multi-dimensional credit, interface-reshaping guarded processes (`RProc`), and full-stack silicon (`lib/silicon/`) down to GDSII v6.0 3nm layout and GALS cubical inet-tile RTL.
- **Neuroscience & Neural Co-Processors (`neuro/`)**: Output-null fibers & intrinsic manifolds, symbolic Sum-of-Squares (SOS) Lyapunov stability certificates for symmetric and asymmetric two-learner brain–machine co-adaptation over $\mathbb{Q}$, forward-invariant Control Barrier Functions for driven FitzHugh–Nagumo membranes, proof-gated reshaping co-processors (`SafeGateDS`), REBUS cognitive inhibition, and synthetic synesthesia.
- **Physics, Quantum Chemistry & Living Matter (`lqg/`, `phys/`, `chem/`, `fold/`)**: Relational Quantum Mechanics, Loop Quantum Gravity $SU(2)$ spin networks, Thiemann Hamiltonian vertex, 4D Pachner invariants & Big Bounce cosmology, Dirac–Foldy–Wouthuysen relativistic fine structure, zero-float ab initio $\text{H}_2$ and Bareiss–Sylvester Helium, superconductivity (BdG, Ginzburg–Landau SOS, Hubbard superexchange), exact rational $SO(3)/SE(3)$ peptide backbone kinematics, chemical identity beyond InChI, and bioelectric morphogenesis.

---

## Decentralized Web Seed Architecture

`hyperactivity.ai` is served directly by `lib/server.hypercubical` out of `store/` and acts as a seed node for a peer-to-peer web:

1. **Zero-Git Pool Sync**: `/packs`, `/roots`, `/root`, and `/bootstrap.tgz` allow any machine to replicate the content-addressed store directly over HTTP without GitHub.
2. **Browser-Local Content-Addressed Cache**: `/kg` maintains an `IndexedDB` store (`hx-cas-v1`) keyed by SHA-256 hash.
3. **Stateless WebRTC Rendezvous**: `/api/p2p/signal` and `/api/live/pulse` coordinate peer handshakes and ambient co-presence with zero open sockets and zero server compute; once connected, browsers exchange content-addressed objects directly over `RTCDataChannel`.
4. **Client-Side Execution**: `/kernel` runs `cubical.wasm` directly in the browser.

---

## Build & Run

Requires a C11 compiler (`gcc` or `clang`) and `make`.

```bash
git clone https://github.com/avikj/hyperactivity.git
cd hyperactivity
make
./hyperactive
```

- `make` builds the native kernel (`build/cubical`).
- `./hyperactive` runs the interactive loop over `store/`.
- `./build/cubical lib/server.hypercubical 8080` boots your own local mirror of `hyperactivity.ai` directly from `store/`.
- Both `.hypercubical` and `.cub` module extensions resolve natively.
