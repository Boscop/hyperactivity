<p align="center">
  <img src="https://hyperactivity.ai/logo.svg" alt="hyperactivity" width="84" />
</p>

<h1 align="center">hyperactivity</h1>

<p align="center">
  <strong>A cubical interaction-net operating system language and decentralized knowledge graph.</strong><br/>
  <a href="https://hyperactivity.ai">hyperactivity.ai</a> ·
  <a href="https://hyperactivity.ai/kg">Knowledge Graph</a> ·
  <a href="https://hyperactivity.ai/kernel">Browser Kernel (WASM)</a> ·
  <a href="https://hyperactivity.ai/nets">Interaction Net Lab</a> ·
  <a href="https://hyperactivity.ai/paper">Paper</a> ·
  <a href="https://hyperactivity.ai/pdf">PDF</a>
</p>

---

**There are no files, no global clock, and no canonical server.** Every definition, theorem, document, and web route in this repository lives as an immutable SHA-256 object inside `store/`. Computation is lock-free local rewriting of active pairs in an interaction net (`src/net.c`). Equality is cubical path equality — and univalence (`ua`) computes.

The same kernel that runs its own type checker, serves [hyperactivity.ai](https://hyperactivity.ai), compiles to x86-64 / 3nm GDSII silicon, and computes Brunerie's number (`-2`) in two evaluators also proves:
- **Zero-search lossless compression** (`lib/compress.hypercubical`) beating `gzip -9`, `bzip2 -9`, and `lzma` via causal context-fiber coding ([`/compression`](https://hyperactivity.ai/compression)).
- **Observation fibers & standpoint alignment** (`lib/observation.hypercubical`) where warrant holds across every compatible history, eliminating hallucination structurally ([`/ai`](https://hyperactivity.ai/ai)).
- **Two-learner brain–machine co-processor safety** (`neuro/lyapunov.hypercubical`, `neuro/barrier.hypercubical`) via exact symbolic Sum-of-Squares Lyapunov and Control Barrier certificates over ℚ and dynamic interface-reshaping processes `RProc` ([`/neuro`](https://hyperactivity.ai/neuro)).
- **Exact physics, quantum chemistry & living matter** (`lqg/`, `phys/`, `chem/`, `fold/`) — Relational Quantum Mechanics, $SU(2)$ spin networks & Loop Quantum Cosmology bounce, Dirac fine structure, zero-float ab initio $\text{H}_2$ and Helium, superconductivity, exact rational $SO(3)/SE(3)$ peptide kinematics, and bioelectric morphogenesis ([`/physics`](https://hyperactivity.ai/physics)).

---

## Try it in 5 seconds

```sh
git clone https://github.com/avikj/hyperactivity.git
cd hyperactivity
make
./hyperactive
```

Boot your own local mirror of `hyperactivity.ai` (with live Knowledge Graph, WebRTC P2P rendezvous, and git-free pool sync):

```sh
./build/cubical lib/server.hypercubical 8080
```

Or run the kernel directly in your browser with zero install at **[hyperactivity.ai/kernel](https://hyperactivity.ai/kernel)** (`cubical.wasm`), and explore all definitions & dependency graphs live at **[hyperactivity.ai/kg](https://hyperactivity.ai/kg)**.

---

## A taste of `.hypercubical`

Exact asymmetric two-learner neural co-processor stability (`neuro/lyapunov.hypercubical`) — decided in normal form over the universal polynomial ring $\mathbb{Q}[x_1,\dots,x_n]$, so it holds for all real parameter values simultaneously with zero floating-point approximation:

```haskell
-- V = p₁ e_B² + p₂ e_C²   →   4 p₁ η_B (-dV/dt) = (2 p₁ η_B e_B - (p₁ γ₁ + p₂ γ₂) e_C)² + (4 p₁ p₂ η_B η_C - (p₁ γ₁ + p₂ γ₂)²) e_C²
two-learner-asym-sos-id : al-id 8 two-learner-asym-sos ≡ true
two-learner-asym-sos-id = λ i → true
```

---

## How the machine works

Every term is a net of cells (`src/core.h`, `src/net.c`). Each cell has one **principal port** and zero or more auxiliary ports. Two cells react if and only if an operation's principal port meets a value's principal port — an **active pair**. Disjoint active pairs reduce concurrently across workers without locks, and values are evaluated only as far as they are read.

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

Every composition clause is itself a term of the language. `ua` is a `Glue` type: transporting a proof or program along an equivalence computes directly through these rules.

---

## Decentralized web & store layout

`store/` is a content-addressed pool — concurrent nodes converge by pure set union. Nodes sync without Git via `/packs`, `/roots`, `/root`, and `/bootstrap.tgz`; browsers cache objects locally in `IndexedDB` (`hx-cas-v1`), exchange objects peer-to-peer over WebRTC (`RTCDataChannel` via stateless `/api/p2p/signal`), and execute `cubical.wasm` client-side.

```text
src/            C11 interaction-net evaluator, cubical normalizer, content-addressed store & drivers
store/          immutable SHA-256 objects (read & written via ./hyperactive)
  lib/          cubical checker, specialization, C++/Rust/x86 lifting, rANS compression, credit, silicon/GDSII, server
  neuro/        two-learner BCI Lyapunov & CBF SOS certificates, FitzHugh-Nagumo, reshaping co-processors (RProc)
  chem/         zero-float ab initio H2 & Helium, Dirac algebra, chemical identity, bioelectric morphogenesis
  phys/         relativistic atomic structure, superconductivity (BdG, Ginzburg-Landau, Hubbard)
  lqg/          relational quantum mechanics, SU(2) spin networks, Thiemann vertex, LQC Big Bounce
  fold/         exact rational SO(3)/SE(3) peptide backbone kinematics
  docs/         monograph, domain treatises (/ai, /neuro, /physics, /compression, /chip-design), web apps (/kg, /nets, /kernel)
tools/          lifters (clang2cub, mir2cub, sail2cub, agda2cub) & assembly helpers
experiments/    Brunerie's number (-2), C++ & Rust lifting tests
```
