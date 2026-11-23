# hyperactivity

A cubical interaction-net operating system language and content-addressed knowledge graph.

Live: [hyperactivity.ai](https://hyperactivity.ai) · [graph](https://hyperactivity.ai/kg) · [kernel](https://hyperactivity.ai/kernel) · [nets](https://hyperactivity.ai/nets) · [paper](https://hyperactivity.ai/paper) · [pdf](https://hyperactivity.ai/pdf)

## Build

Requires a C11 compiler (`gcc` or `clang`) and `make`.

```sh
git clone https://github.com/avikj/hyperactivity.git
cd hyperactivity
make
./hyperactive
```

Run a local node of the website and content-addressed pool:

```sh
./build/cubical lib/server.hypercubical 8080
```

Run the guard or verify a module:

```sh
./build/cubical lib/guard.hypercubical
./build/cubical neuro/lyapunov.hypercubical
```

## Layout

```text
src/            C11 interaction-net evaluator, cubical normalizer, store & drivers
store/          content-addressed immutable objects (read/written via ./hyperactive)
  lib/          cubical checker, specialization, C++/Rust/x86 lifting, rANS compression, credit, silicon/GDSII, web server
  neuro/        two-learner BCI Lyapunov/CBF SOS certificates, FitzHugh-Nagumo, interface-reshaping RProc
  chem/         zero-float ab initio H2 & Helium, Dirac algebra, chemical identity, bioelectric morphogenesis
  phys/         relativistic atomic structure, superconductivity (BdG, Ginzburg-Landau, Hubbard)
  lqg/          relational quantum mechanics, SU(2) spin networks, Thiemann vertex, LQC bounce
  fold/         exact rational SO(3)/SE(3) peptide backbone kinematics
  docs/         monograph, treatises (/ai, /neuro, /physics, /compression, /chip-design), web apps (/kg, /nets, /kernel)
tools/          lifters (clang2cub, mir2cub, sail2cub, agda2cub) and assembly helpers
experiments/    Brunerie number, C++/Rust lifting tests
```

## Execution model

Terms are interaction nets (`src/core.h`, `src/net.c`). Every cell has one principal port and zero or more auxiliary ports. Computation is local rewriting of active pairs (an operation's principal port wired to a value's principal port). Disjoint active pairs reduce in parallel without locks.

```text
apply(arg)      ⋈  λ              ⟶  body [param := arg]
first / second  ⋈  pair(a, b)     ⟶  a / b  (other wire erased)
force           ⋈  delay(t)       ⟶  t
path-apply(r)   ⋈  path-λ i. t    ⟶  subst [i := r] t
duplicate       ⋈  value          ⟶  shared reference (atomic values copied)
erase           ⋈  value          ⟶  erase on each subpart
compose         ⋈  partial tube   ⟶  type-directed Kan clause (Π, Σ, Path, U, Glue, Data)
unglue          ⋈  glued value    ⟶  base (equivalence map on true face)
transport       ⋈  constructor    ⟶  constructor clause (identity on constant line)
```

## Decentralized sync

Nodes replicate `store/` without Git via `/packs`, `/roots`, `/root`, and `/bootstrap.tgz`. Browsers cache content-addressed objects in `IndexedDB`, exchange objects directly over WebRTC (`RTCDataChannel` via stateless `/api/p2p/signal`), and execute `cubical.wasm` locally at `/kernel`.
