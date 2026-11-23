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

Computing since 1970 is built on untyped bytes, mutable filesystems, and centralized servers. Because raw bytes carry no meaning and equality is broken, every field builds brittle workarounds on top: token predictors that hallucinate, scalar preference averages that erase disagreement, canonical string IDs (like InChI in chemistry) that collapse distinct substances, blockchains to fake consensus, and endless manual rewrites between languages and hardware.

**Hyperactivity** replaces that substrate with one self-hosted system where mathematics, software, hardware, and distributed knowledge are the same substance.

---

## The four ideas

1. **Local interaction, no global clock (Interaction Nets).**
   A program — like a neural circuit, a molecule, or a network of agents — is a graph of cells connected by wires (`src/core.h`, `src/net.c`). Each cell has one *principal port* and zero or more auxiliary ports. Two cells react if and only if an operation's principal port meets a value's principal port (an *active pair*). Disjoint active pairs reduce concurrently across cores with no locks and no central coordinator, and values compute only as far as they are read.

2. **Identity is a path that computes (Cubical Univalence).**
   Byte equality, definitional equality, and semantic equivalence are different things. In cubical type theory, an equivalence between two representations *is* a path (`ua`), and transporting along that path computes. A theorem, optimization, or physical identity proved once for one representation automatically transports to every equivalent representation.

3. **Knowledge is a fiber of compatible histories (Observation & Warrant).**
   No agent sees the whole world; it sees an observation $y = o(h)$. Its exact epistemic state is the preimage fiber $\mathrm{Fib}_o(y) = \{ h \mid o(h) = y \}$ of every world-history consistent with what it observed. A claim is warranted only if it holds across the *entire* fiber. Whenever two compatible histories disagree, you get an explicit split witness — making hallucination structurally impossible to hide and eliminating scalar RLHF preference averaging.

4. **No filesystems, no canonical state (Content-Addressed Set Union).**
   Every definition, proof, document, and web page in this repository is an immutable SHA-256 object inside `store/`. Independent machines and browsers converge by pure set union — no mutable paths, no central authority, and no blockchain.

---

## What is built and running inside this single system

Because those four laws are a universal substrate rather than a narrow programming language, one self-hosted kernel closes real problems end-to-end across computing and science:

- **Self-hosted kernel, lifting & silicon (`src/`, `lib/`, `lib/silicon/`):** The cubical type checker is written in the language itself. Brunerie's number (`-2`) computes directly in two evaluators. Existing C++, Rust, and x86-64 code lifts into values of the host without rewriting, specializes along equivalence paths, and lowers to native x86-64 binaries and 3nm GDSII v6.0 silicon layouts (including GALS cubical interaction-net tile RTL).
- **Zero-search lossless compression (`lib/compress.hypercubical`, `lib/compress-stream.hypercubical`):** Applying observation fibers directly to byte streams produces a streaming $O(N)$ renormalized rANS causal context-fiber compressor with no probability axioms and no serialized lookup tables that beats `gzip -9`, `bzip2 -9`, and `lzma` ([`/compression`](https://hyperactivity.ai/compression)).
- **AI alignment & conserved credit (`lib/observation.hypercubical`, `lib/credit.hypercubical`):** Standpoint-preserving observation fibers, active querying that shrinks fibers until a warranted action exists, and conserved multi-dimensional credit replacing 1D scalar money without a blockchain ([`/ai`](https://hyperactivity.ai/ai)).
- **Neuroscience & closed-loop neural co-processors (`neuro/`):** Excitable membranes (FitzHugh–Nagumo), cortical output-null fibers, two-learner brain–decoder co-adaptation, and dynamic interface-reshaping processes (`RProc`), with exact symbolic Sum-of-Squares Lyapunov and Control Barrier safety proofs over $\mathbb{Q}$ that hold for all parameter values simultaneously ([`/neuro`](https://hyperactivity.ai/neuro)).
- **Physics, quantum chemistry & living matter (`lqg/`, `phys/`, `chem/`, `fold/`):** Relational Quantum Mechanics, Loop Quantum Gravity $SU(2)$ spin networks, Thiemann Hamiltonian vertex & Big Bounce cosmology, Dirac–Foldy–Wouthuysen relativistic fine structure, zero-float ab initio $\text{H}_2$ and Bareiss–Sylvester Helium, superconductivity (BdG, Ginzburg–Landau SOS, Hubbard superexchange), exact rational $SO(3)/SE(3)$ peptide backbone kinematics, chemical identity without InChI's string collapse, and bioelectric morphogenesis ([`/physics`](https://hyperactivity.ai/physics)).
- **Decentralized web seed (`lib/server.hypercubical`, `hyperactivity.ai`):** The repository serves its own website directly out of `store/`. Any node can replicate `store/` without Git (`/packs`, `/roots`, `/root`, `/bootstrap.tgz`). Browsers cache SHA-256 objects locally in `IndexedDB`, exchange objects peer-to-peer over WebRTC (`RTCDataChannel` via stateless `/api/p2p/signal`), and execute `cubical.wasm` client-side at [`/kernel`](https://hyperactivity.ai/kernel) with zero server compute.

---

## The interaction laws

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

Every composition clause is itself a term of the language. `ua` is a `Glue` type: transporting along an equivalence computes directly through these rules.

---

## Build, run & inspect

Requires a C11 compiler (`gcc` or `clang`) and `make`.

```sh
git clone https://github.com/avikj/hyperactivity.git
cd hyperactivity
make
./hyperactive
```

`store/` holds immutable SHA-256 objects (not mutable source files). Read, run, or serve directly from `store/`:

```sh
./hyperactive def two-learner-asym-sos-id       # inspect any definition & its content-addressed key
./build/cubical neuro/lyapunov.hypercubical     # run / verify a module (.hypercubical and .cub both resolve)
./build/cubical lib/server.hypercubical 8080    # boot a local mirror of hyperactivity.ai
```

Or explore everything in your browser with zero install:
- **[hyperactivity.ai/kg](https://hyperactivity.ai/kg)** — interactive Knowledge Graph & live source inspector
- **[hyperactivity.ai/kernel](https://hyperactivity.ai/kernel)** — live `cubical.wasm` kernel in the browser
- **[hyperactivity.ai/nets](https://hyperactivity.ai/nets)** — interactive interaction-net visual compiler
