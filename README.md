<p align="center">
  <img src="https://hyperactivity.ai/logo.svg" alt="hyperactivity" width="76" />
</p>

<h1 align="center">hyperactivity</h1>

<p align="center">
  <a href="https://hyperactivity.ai">hyperactivity.ai</a> ·
  <a href="https://hyperactivity.ai/kg">Knowledge Graph</a> ·
  <a href="https://hyperactivity.ai/kernel">Browser Kernel (WASM)</a> ·
  <a href="https://hyperactivity.ai/nets">Interaction Net Lab</a> ·
  <a href="https://hyperactivity.ai/paper">Paper</a>
</p>

---

Modern computing runs on untyped bytes, mutable filesystems, and centralized servers. Because raw bytes carry no meaning and equality is broken, every field invents workarounds on top: token predictors that hallucinate, scalar preference averages that erase disagreement, canonical string IDs (like InChI in chemistry) that collapse distinct substances, blockchains to coordinate state, and manual rewrites between languages and hardware.

Hyperactivity is a self-hosted cubical interaction-net operating system language and content-addressed knowledge graph built from four primitives:

1. **Interaction nets (`src/core.h`, `src/net.c`).** Every program — like a neural circuit, a molecule, or a network of agents — is a graph of cells connected by wires. Each cell has one *principal port* and zero or more auxiliary ports. Two cells react when an operation's principal port meets a value's principal port (an *active pair*). Disjoint active pairs reduce in parallel across cores with no locks and no global clock, and values compute only as far as they are read.
2. **Cubical univalence (`ua`).** Byte equality, definitional equality, and semantic equivalence are distinct. An equivalence between two representations is a path, and transporting along that path computes — any theorem, optimization, or physical identity proved for one representation transports automatically to every equivalent representation.
3. **Observation fibers & warrant.** An agent observes $y = o(h)$. Its exact state is the fiber $\mathrm{Fib}_o(y) = \{ h \mid o(h) = y \}$ of every world-history consistent with $y$. A claim is warranted when it holds across the entire fiber. When two compatible histories disagree, the system produces an explicit split witness instead of a hallucination or a scalar preference average.
4. **Content-addressed set union (`store/`).** Every definition, proof, document, and web route is an immutable SHA-256 object. Independent machines and browsers converge by set union — no mutable filesystems, no canonical server state, and no blockchain.

---

## In the store

- **Kernel, lifting & silicon (`src/`, `lib/`, `lib/silicon/`):** Self-hosted cubical type checker; Brunerie's number (`-2`) computed directly in two evaluators; C++, Rust, and x86-64 lifted into host values, specialized along equivalence paths, and lowered to native x86-64 binaries and 3nm GDSII v6.0 silicon layouts (including GALS cubical interaction-net tile RTL).
- **Compression (`lib/compress.hypercubical`, `lib/compress-stream.hypercubical`):** Streaming $O(N)$ renormalized rANS causal context-fiber compression with zero search, zero probability axioms, and zero serialized lookup tables, beating `gzip -9`, `bzip2 -9`, and `lzma` ([`/compression`](https://hyperactivity.ai/compression)).
- **AI & credit (`lib/observation.hypercubical`, `lib/credit.hypercubical`):** Standpoint-preserving observation fibers, active querying, and conserved multi-dimensional credit ([`/ai`](https://hyperactivity.ai/ai)).
- **Neuroscience & neural co-processors (`neuro/`):** FitzHugh–Nagumo excitable membranes, cortical output-null fibers, two-learner brain–decoder co-adaptation, and interface-reshaping processes (`RProc`), with symbolic Sum-of-Squares Lyapunov and Control Barrier proofs over $\mathbb{Q}$ holding for all parameter values simultaneously ([`/neuro`](https://hyperactivity.ai/neuro)).
- **Physics, chemistry & living matter (`lqg/`, `phys/`, `chem/`, `fold/`):** Relational Quantum Mechanics, Loop Quantum Gravity $SU(2)$ spin networks, Thiemann Hamiltonian vertex & Big Bounce cosmology, Dirac–Foldy–Wouthuysen fine structure, zero-float ab initio $\text{H}_2$ and Bareiss–Sylvester Helium, superconductivity (BdG, Ginzburg–Landau SOS, Hubbard superexchange), exact rational $SO(3)/SE(3)$ peptide backbone kinematics, chemical identity without InChI's string collapse, and bioelectric morphogenesis ([`/physics`](https://hyperactivity.ai/physics)).
- **Decentralized web (`lib/server.hypercubical`, `hyperactivity.ai`):** Serves its own website directly out of `store/`. Nodes replicate `store/` without Git (`/packs`, `/roots`, `/root`, `/bootstrap.tgz`). Browsers cache SHA-256 objects in `IndexedDB`, exchange objects peer-to-peer over WebRTC (`RTCDataChannel` via stateless `/api/p2p/signal`), and execute `cubical.wasm` locally at [`/kernel`](https://hyperactivity.ai/kernel).

---

## Interaction laws

Every reduction step in `src/net.c` is one of these local active-pair rewrites:

```text
apply(arg)      ⋈  λ              ⟶  body [param := arg]
first / second  ⋈  pair(a, b)     ⟶  a / b  (other wire erased)
force           ⋈  delay(t)       ⟶  t
path-apply(r)   ⋈  path-λ i. t    ⟶  subst [i := r] t
duplicate       ⋈  value          ⟶  shared reference (atomic values copied)
erase           ⋈  value          ⟶  erase facing each subpart
compose         ⋈  partial tube   ⟶  type-directed Kan clause (Π, Σ, Path, U, Glue, Data)
unglue          ⋈  glued value    ⟶  base (equivalence map applied on true face)
transport       ⋈  constructor    ⟶  constructor clause (identity on constant line)
```

Every composition clause is a term of the language. `ua` is a `Glue` type: transporting along an equivalence computes directly through these rules.

---

## Build & run

Requires a C11 compiler (`gcc` or `clang`) and `make`.

```sh
git clone https://github.com/avikj/hyperactivity.git
cd hyperactivity
make
./hyperactive
```

`store/` holds immutable SHA-256 objects:

```sh
./hyperactive def two-learner-asym-sos-id       # inspect a definition and its key
./build/cubical neuro/lyapunov.hypercubical     # run a module (.hypercubical and .cub both resolve)
./build/cubical lib/server.hypercubical 8080    # boot a local mirror of hyperactivity.ai
```
