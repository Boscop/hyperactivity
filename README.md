<p align="center">
  <img src="https://hyperactivity.ai/logo.svg" alt="hyperactivity" width="84" />
</p>

<h1 align="center">hyperactivity</h1>

<p align="center">
  <a href="https://hyperactivity.ai">hyperactivity.ai</a> ·
  <a href="https://hyperactivity.ai/kg">Knowledge Graph (<code>/kg</code>)</a> ·
  <a href="https://hyperactivity.ai/kernel">Browser Kernel (<code>/kernel</code>)</a> ·
  <a href="https://hyperactivity.ai/nets">Interaction Net Lab (<code>/nets</code>)</a> ·
  <a href="https://hyperactivity.ai/paper">Monograph (<code>/paper</code>)</a> ·
  <a href="https://hyperactivity.ai/pdf">PDF</a>
</p>

---

## Table of Contents

1. [The Defect of 1970s Computing](#1-the-defect-of-1970s-computing)
2. [The Execution Primitive: Interaction Nets](#2-the-execution-primitive-interaction-nets)
3. [Identity as Computation: Cubical Type Theory & Univalence](#3-identity-as-computation-cubical-type-theory--univalence)
4. [Knowledge as Lack of Ignorance: Observation Fibers, Warrant & Standpoint Alignment](#4-knowledge-as-lack-of-ignorance-observation-fibers-warrant--standpoint-alignment)
5. [No Filesystems, No Canonical State: Content-Addressed Set Union & Conserved Credit](#5-no-filesystems-no-canonical-state-content-addressed-set-union--conserved-credit)
6. [Absorption Without Rewrite: C++/Rust/x86-64 Lifting, Specialization & 3nm Silicon](#6-absorption-without-rewrite-crustx86-64-lifting-specialization--3nm-silicon)
7. [Zero-Search Causal Context-Fiber Compression](#7-zero-search-causal-context-fiber-compression)
8. [Neuroscience & Closed-Loop Neural Co-Processors](#8-neuroscience--closed-loop-neural-co-processors)
9. [Relational Physics, Exact Quantum Chemistry & Living Matter](#9-relational-physics-exact-quantum-chemistry--living-matter)
10. [The Decentralized Web Seed](#10-the-decentralized-web-seed)
11. [Build, Run & Inspect](#11-build-run--inspect)

---

## 1. The Defect of 1970s Computing

Everything built on the 1970s computing stack — Unix filesystems, POSIX byte streams, TCP/IP location addressing, mutable databases, and centralized servers — shares a single structural defect: **raw bytes have no meaning, and identity is collapsed into mutable location or string equality.**

Because the substrate cannot express types, equivalence, or partial observation, every domain invents a brittle workaround on top of it:

- **AI:** Large language models predict raw byte tokens $p_\theta(x_t \mid x_{<t})$ and sample a single point from an uncollapsed space of compatible worlds, producing structural hallucination. Preference alignment (RLHF) collapses incompatible human standpoints into a single 1D scalar reward, erasing disagreement into a muddled average. Lexical safety filters key refusals to surface words rather than semantic meaning.
- **Chemistry:** Entity resolution — deciding when two records describe the same substance — collapses digest equality, definitional equality, and semantic/tautomeric equivalence into string equality of a canonical identifier (InChI), which fails across tautomeric crystals in the Cambridge Structural Database (IUPAC project #2012-023-2-800).
- **Distributed systems & finance:** Location-addressed mutable state forces global consensus protocols, central servers, or blockchains to arbitrate a single "canonical state," while economies collapse multi-dimensional human contribution into a single 1D scalar currency.
- **Systems & hardware:** Software is rewritten endlessly across C, C++, Rust, Python, CUDA, Verilog, and GDSII because compilers discard semantic equivalence paths the moment code is lowered to bytes.

Hyperactivity replaces that substrate with one self-hosted mathematical and computational system: **lock-free local interaction nets executing cubical type theory over an immutable content-addressed store with no canonical state.**

---

## 2. The Execution Primitive: Interaction Nets

There is no primitive thing, only a primitive law of interaction. Real systems — a cortical circuit, a spin-network vertex, a biochemical reaction complex, an asynchronous hardware tile, or a network of communicating agents — have no global clock and no central address bus. They are **interaction networks** (Lafont, 1990).

In `src/core.h` and `src/net.c`, every term is a net of **cells** connected by **wires**:

- **Cell:** A node with one *principal port* and zero or more auxiliary ports.
- **Value:** A cell that offers itself at its principal port (`λ`, pair, type, constructor, interval endpoint, partial system).
- **Operation:** A cell that demands a value at its principal port (`apply`, `first`, `second`, `path-apply`, `compose`, `case`, `subst`, `duplicate`, `erase`).
- **Wire:** Connects two ports. A wire may be empty while one side computes; whoever arrives first leaves what it has, and the second finds it. Waiting is not a step.
- **Active pair:** An operation's principal port wired directly to a value's principal port. This is the only place computation happens.
- **Step:** The two cells of an active pair are removed and replaced by a small local net on the auxiliary wires they held. Nothing else in memory is inspected.

```text
   arg ──┐
         app ●────● λ ── body      ⟶      out ── body (arg wired into parameter)
   out ──┘
```

Because every rewrite touches only the two cells of an active pair and their incident wires, **disjoint active pairs reduce simultaneously across CPU cores, GPU warps, or asynchronous silicon tiles with zero locks.** Optimal sharing (`duplicate` / `erase`) prevents duplicating work, and demand-driven evaluation computes a value only as far as a reader demands it.

### Core Interaction Laws (`src/net.c`)

```text
-- Functions, pairs & guarded corecursion (▸ κ)
apply(arg)      ⋈  λ               ⟶  body [param := arg]
first / second  ⋈  pair(a, b)      ⟶  a / b   (other wire erased)
force           ⋈  delay(t)        ⟶  t

-- Cubical paths & dimension substitution
path-apply(r)   ⋈  path-λ i. t     ⟶  subst [i := r] facing t
path-apply(0|1) with known ends    ⟶  that endpoint directly (path never read)
path-apply(0|1) ⋈  blocked path    ⟶  endpoint read from the path's type
subst           ⋈  interval/face   ⟶  substituted interval or face
subst           ⋈  λ, pair, Π, Σ   ⟶  same cell, subst facing each part
subst           ⋈  path-λ j. t     ⟶  path-λ j. (subst facing t), α-safe
subst           ⋈  system          ⟶  branch whose face became true, else reduced system
subst           ⋈  held value      ⟶  stays pending on the reference until read

-- Optimal sharing
duplicate       ⋈  value w/ parts  ⟶  value held once; each side gets a reference
duplicate       ⋈  atomic value    ⟶  two copies
erase           ⋈  value           ⟶  gone; erase facing each part
anything        ⋈  reference       ⟶  top cell copied out; subparts stay references

-- Partial systems
eliminator      ⋈  system          ⟶  system of eliminator on each branch (lazy)
add-branch(φ,t) with φ = true      ⟶  t
add-branch(φ,t) with φ = false     ⟶  rest (t dropped)

-- Kan composition, Glue & Univalence (ua)
compose         ⋈  total tube      ⟶  tube's far end
compose         with partial tube  ⟶  type-directed Kan clause (Π, Σ, Path, U, Glue, Data)
Glue / glued    when face is true  ⟶  type / value on that face
unglue          on true face       ⟶  equivalence map applied
unglue          ⋈  glued value     ⟶  base
transport       ⋈  constructor     ⟶  constructor's transport clause
transport       on constant line   ⟶  identity
```

Every Kan composition clause is itself a term of the language, not ad-hoc C runtime code.

> **Interactive:** Watch and step through Peano arithmetic, LSB-first binary carry, parallel certified merge sort, cubical paths, Kan box filling, univalence, and bounded $O(1)$ guarded-corecursive artificial life live at [**hyperactivity.ai/nets**](https://hyperactivity.ai/nets) — or run the real C kernel compiled to WebAssembly at [**hyperactivity.ai/kernel**](https://hyperactivity.ai/kernel).

---

## 3. Identity as Computation: Cubical Type Theory & Univalence

Hyperactivity implements **CCHM cubical type theory** (Cohen, Coquand, Huber, & Mörtberg, 2018) with a self-hosted type checker written inside the language (`lib/check.hypercubical`).

In ordinary type theory or mainstream programming, equality is either raw bit-identity or an opaque axiom that blocks computation. In cubical type theory, an interval $I$ with endpoints $0, 1 : I$ and De Morgan algebra $(\wedge, \vee, 1 - \cdot)$ turns equality into a function out of the interval:

$$\mathrm{Path}_A(a, b) \;=\; \{ p : I \to A \mid p(0) \equiv a,\; p(1) \equiv b \}$$

Two types $A$ and $B$ connected by an equivalence $e : A \simeq B$ yield a path $\mathrm{ua}(e) : \mathrm{Path}_{\mathcal{U}}(A, B)$ constructed via `Glue` types (Voevodsky's Univalence Axiom, now a theorem that computes). Transporting any program, invariant, or proof $x : P(A)$ along $\mathrm{ua}(e)$ reduces mechanically through the Glue and Kan composition laws to $P(B)$:

$$\mathrm{transport}\;(\lambda i.\; P(\mathrm{ua}(e)\;i))\;x \;\longrightarrow^*\; e_*(x)$$

Three distinct notions of identity are never collapsed:
1. **Digest equality:** identical SHA-256 content hash in `store/`.
2. **Definitional equality:** identical normal form under the interaction-net laws.
3. **Path / semantic equivalence ($\mathrm{Path}$ / `ua`):** constructive witness that two different representations (e.g. an unoptimized specification and a specialized x86-64 binary, or two tautomeric graphs of the same molecule) behave identically and transport all proofs between them.

**Concrete benchmark:** The evaluator computes **Brunerie's number** $\beta \in \pi_4(S^3) \cong \mathbb{Z}/2\mathbb{Z}$ (`experiments/brunerie/typed.cub`, Ljungström's typed port) directly to normal form `negsuc (suc zero)` ($-2$) in both single-worker and 4-worker parallel execution (`make brunerie`).

---

## 4. Knowledge as Lack of Ignorance: Observation Fibers, Warrant & Standpoint Alignment

> **Full interactive treatise:** [**hyperactivity.ai/ai**](https://hyperactivity.ai/ai) · Source: [`lib/observation.hypercubical`](https://hyperactivity.ai/kg?mod=lib/observation.hypercubical), [`lib/obs.hypercubical`](https://hyperactivity.ai/kg?mod=lib/obs.hypercubical)

An agent never sees the full state of the world $h \in H$; it receives a partial observation $y = o(h) \in O$. Its exact epistemic state is the **observation fiber** — the preimage of $y$ under $o$:

$$\mathrm{Fib}_o(y) \;=\; \sum_{h : H} \big(o(h) \equiv y\big)$$

Knowledge is literally the *absence of ignorance* (Greek *a-lētheia*, un-concealment): each new observation intersects the fiber, removing histories incompatible with reality without ever fabricating a point inside it. A claim $P : H \to \mathrm{Type}$ is **warranted** at observation $y$ if and only if it holds across the *entire* fiber:

$$\mathrm{Warranted}(o, y, P) \;:\equiv\; \prod_{h : H} \big(o(h) \equiv y\big) \to P(h)$$

This resolves three structural failures of contemporary AI simultaneously:

1. **Hallucination becomes structurally unrepresentable:** Asserting $P$ when $\mathrm{Fib}_o(y)$ contains two compatible histories $(h_1, h_2)$ with $P(h_1)$ true and $\neg P(h_2)$ false yields an explicit **split witness**. Rather than guessing, the agent issues an **active query** (`WorldAfter`) that separates $h_1$ from $h_2$ until every surviving history agrees.
2. **Standpoint alignment replaces scalar RLHF averaging:** Aggregating incompatible human values into a single scalar reward $r(x, y)$ erases every standpoint into a sycophantic mean that satisfies nobody. In Hyperactivity, standpoints are carried explicitly as typed perspectives; merging agents takes the content-addressed **set union** of warranted claims together with explicit split witnesses wherever perspectives diverge.
3. **Framing-invariant safety:** Safety gates are typed over semantic equivalence classes rather than surface lexical tokens, so harmless rephrasing never triggers false refusals and adversarial rephrasing never bypasses a semantic invariant.

---

## 5. No Filesystems, No Canonical State: Content-Addressed Set Union & Conserved Credit

There are no mutable source files in Hyperactivity. Every declaration, module, proof, document, and web route is stored as an immutable SHA-256 object inside `store/` (inspired by Unison's content-addressed code, generalized to a full metacircular OS and knowledge graph).

- **Convergence without consensus:** Because objects are immutable and addressed by content hash, two independent machines, agents, or browsers never fight over a mutable file path or a "canonical server state." Exchanging objects converges by pure **set union** — no central server, no distributed locks, and no blockchain.
- **Conserved multi-dimensional credit (`lib/credit.hypercubical`):** Instead of collapsing value into a 1D scalar currency, contributions across the knowledge graph carry exact dependency provenance and conserved credit flows directly inside the type system.

---

## 6. Absorption Without Rewrite: C++/Rust/x86-64 Lifting, Specialization & 3nm Silicon

> **Full exposition:** [**hyperactivity.ai/chip-design**](https://hyperactivity.ai/chip-design) · Source: [`lib/specialize.hypercubical`](https://hyperactivity.ai/kg?mod=lib/specialize.hypercubical), [`lib/silicon/export.hypercubical`](https://hyperactivity.ai/kg?mod=lib/silicon/export.hypercubical)

A new computational foundation that requires rewriting the world's existing software and hardware is dead on arrival. Hyperactivity absorbs existing ecosystems as values of the host:

- **Zero-rewrite lifting (`tools/clang2cub.py`, `tools/mir2cub.py`, `tools/sail2cub.py`, `tools/agda2cub.py`):** Real C++, Rust MIR, x86-64 ISA semantics (with TSO/RC11 memory models), and Agda modules lift into typed values inside `store/`.
- **Verified specialization (`lib/specialize.hypercubical`):** A high-level program is specialized to concrete hardware or inputs *together with an equivalence path* proving the lowered code computes the same result as the source, then emitted directly to standalone static x86-64 ELF64 binaries (`lib/elf.hypercubical`) or JIT machine code (`lib/x86.hypercubical`).
- **Full-stack silicon (`lib/silicon/`):** Exact circuit light-cone lower bounds, Landauer reversible computing, $[8, 4, 4]$ SECDED ECC, DPA masking, Tomasulo out-of-order execution, MESI cache coherence, `[[5, 1, 3]]` quantum error-correction compilation, byte-level GDSII v6.0 round-trip proofs, 5-layer 3nm layout, and GALS cubical interaction-net tile RTL.

---

## 7. Zero-Search Causal Context-Fiber Compression

> **Full benchmarks & interactive report:** [**hyperactivity.ai/compression**](https://hyperactivity.ai/compression) · Source: [`lib/compress.hypercubical`](https://hyperactivity.ai/kg?mod=lib/compress.hypercubical), [`lib/compress-stream.hypercubical`](https://hyperactivity.ai/kg?mod=lib/compress-stream.hypercubical), [`lib/optimality.hypercubical`](https://hyperactivity.ai/kg?mod=lib/optimality.hypercubical)

Intelligence is compression: discovering exact structural identities that eliminate ignorance. Applying the exact same observation-fiber machinery directly to ordered byte streams yields a **zero-search causal context-fiber lossless compressor** over streaming $O(N)$ renormalized rANS (Duda, 2009) with:

- Zero probability axioms,
- Zero serialized lookup tables,
- Universal `refl`-verified round-trip decoding (`decode (encode x) ≡ x`), and
- Exact Shannon-rate, Kraft inequality, and pigeonhole incompressibility proofs (`lib/optimality.hypercubical`).

On real byte streams, this zero-search causal context-fiber codec outperforms `gzip -9`, `bzip2 -9`, and `lzma`.

---

## 8. Neuroscience & Closed-Loop Neural Co-Processors

> **Full interactive 101 → Frontier → Closure treatise with live phase-space & two-learner simulations:** [**hyperactivity.ai/neuro**](https://hyperactivity.ai/neuro) · Source: [`neuro/`](https://hyperactivity.ai/kg?mod=neuro/lyapunov.hypercubical)

A nervous system is a massively parallel local interaction network. Starting from ion-channel electrochemistry (Hodgkin & Huxley, 1952) and the 2D **FitzHugh–Nagumo** excitable membrane oscillator (FitzHugh, 1961; Nagumo, 1962),

$$\frac{dv}{dt} = v - \frac{v^3}{3} - w + I_{\text{ext}}, \qquad \frac{dw}{dt} = \varepsilon\,(v + a - b\,w)$$

cortical populations $x(t) \in \mathbb{R}^N$ evolve on low-dimensional **intrinsic manifolds** $\mathcal{M} \subset \mathbb{R}^N$, where a motor or BCI readout $y = Cx$ induces an **output-null fiber** $\mathrm{Fib}_C(y) = x_0 + \ker C$ (`neuro/descent.hypercubical`).

In a closed-loop neural co-processor (the Berkeley two-learner lineage: Carmena, Orsborn, Burden, & Madduri, 2023), *both* the biological brain ($B$) and the electronic decoder ($C$) adapt online:

$$\frac{de_B}{dt} = -\eta_B e_B + \gamma_1 e_C, \qquad \frac{de_C}{dt} = \gamma_2 e_B - \eta_C e_C$$

Numerical simulation can test millions of trajectories and still miss the parameter corner that diverges inside an implanted BCI. Hyperactivity decides polynomial **Sum-of-Squares (SOS)** stability and safety certificates in exact normal form over the universal polynomial ring $\mathbb{Q}[x_1, \dots, x_n]$ via [`chem/algebra.hypercubical`](https://hyperactivity.ai/kg?mod=chem/algebra.hypercubical) (`al-id`), so every proof holds for *all* real parameter values simultaneously with zero floating-point error:

- **Asymmetric Two-Learner Lyapunov SOS ([`neuro/lyapunov.hypercubical`](https://hyperactivity.ai/kg?mod=neuro/lyapunov.hypercubical), `two-learner-asym-sos-id`):** For weighted energy $V = p_1 e_B^2 + p_2 e_C^2$,

$$4 p_1 \eta_B \left(-\frac{dV}{dt}\right) = \big(2 p_1 \eta_B e_B - (p_1 \gamma_1 + p_2 \gamma_2)\,e_C\big)^2 + \Big(4 p_1 p_2 \eta_B \eta_C - (p_1 \gamma_1 + p_2 \gamma_2)^2\Big)\,e_C^2$$

  recovering the sharp Hurwitz stability boundary $\eta_B \eta_C > \gamma_1 \gamma_2$ constructively.
- **Forward-Invariant Control Barrier Functions ([`neuro/barrier.hypercubical`](https://hyperactivity.ai/kg?mod=neuro/barrier.hypercubical)):** Exact SOS Nagumo invariance proofs for both the two-learner safe set (`cbf-2learner-safe-sos-id`) and the externally driven FitzHugh–Nagumo trapping ellipsoid (`fhn-driven-trapping-sos-id`).
- **Dynamic Interface-Reshaping Co-Processors ([`lib/polyproc.hypercubical`](https://hyperactivity.ai/kg?mod=lib/polyproc.hypercubical), [`neuro/reshaping-coproc.hypercubical`](https://hyperactivity.ai/kg?mod=neuro/reshaping-coproc.hypercubical)):** Guarded corecursive processes generalized to indexed interface-reshaping processes `RProc κ D A i` (channels opening/closing and decoders rebinding mid-run, with exact cubical round-trip equivalence `gproc-rproc-id`), proof-gated activation (`SafeGateDS`, `disagreement-blocks-gate`), REBUS cognitive prior relaxation (`neuro/inhibition.hypercubical`; Carhart-Harris & Friston, 2019), and synthetic synesthesia (`neuro/synesthesia.hypercubical`).

---

## 9. Relational Physics, Exact Quantum Chemistry & Living Matter

> **Full interactive 101 → Frontier → Closure treatise with live $SU(2)$ spin-network & bioelectric morphogenesis simulations:** [**hyperactivity.ai/physics**](https://hyperactivity.ai/physics) · Source: [`lqg/`](https://hyperactivity.ai/kg?mod=lqg/cosmology-bounce.hypercubical), [`phys/`](https://hyperactivity.ai/kg?mod=phys/relativistic-atom.hypercubical), [`chem/`](https://hyperactivity.ai/kg?mod=chem/algebra.hypercubical), [`fold/`](https://hyperactivity.ai/kg?mod=fold/backbone-kinematics.hypercubical)

At the fundamental scale, spacetime, atoms, molecules, and morphogenetic tissues are relational interaction networks:

- **Relational Quantum Mechanics & Loop Quantum Gravity (`lqg/`):** Formalization of RQM (Rovelli, 1996; Di Biagio & Rovelli, 2021; `lqg/rqm.hypercubical`), discrete $SU(2)$ spin networks with exact area spectra $A(\mathcal{S}) = 8\pi\gamma\ell_P^2 \sum_{e \cap \mathcal{S}} \sqrt{j_e(j_e+1)}$, Thiemann QSD gravitational/electromagnetic Hamiltonian vertices (`lqg/hamiltonian-vertex.hypercubical`), $U(1)$ magnetic loop flux sectors (`chem/magloop.hypercubical`), semiclassical bridges (`lqg/semiclassical-bridge.hypercubical`), 4D Pachner invariants, and the Ashtekar–Pawlowski–Singh Loop Quantum Cosmology Big Bounce $H^2 = \frac{8\pi G}{3}\rho(1 - \rho/\rho_c)$ (`lqg/cosmology-bounce.hypercubical`).
- **Relativistic Atomic Structure & Superconductivity (`phys/`, `chem/dirac.hypercubical`):** Exact Pauli–Dirac Clifford algebra, Foldy–Wouthuysen relativistic expansion, spin–orbit center-of-gravity invariance, the $Z \le 137$ critical coupling threshold (`phys/relativistic-atom.hypercubical`), Bogoliubov–de Gennes quasiparticle dispersion, Ginzburg–Landau SOS free-energy positivity, and Hubbard superexchange (`phys/superconductivity.hypercubical`).
- **Zero-Float Ab Initio Quantum Chemistry & Chemical Identity (`chem/`):** Exact rational variational Ritz bounds for $\text{H}_2$ and Bareiss–Sylvester Helium, many-body fermionic canonical anticommutation (CAR) algebra, and structural chemical entity resolution where tautomeric and conformational relations are carried as cubical paths instead of lossy InChI strings.
- **Peptide Kinematics & Bioelectric Morphogenesis (`fold/`, `chem/`):** Exact rational $SO(3)/SE(3)$ all-atom peptide backbone kinematics with zero floating-point/quaternion drift (`fold/backbone-kinematics.hypercubical`), and gap-junction bioelectric interaction networks relaxing into stable isopotential anatomical domains (Levin's morphogenetic paradigm).

---

## 10. The Decentralized Web Seed

[hyperactivity.ai](https://hyperactivity.ai) is served directly by [`lib/server.hypercubical`](https://hyperactivity.ai/kg?mod=lib/server.hypercubical) out of `store/` and functions as a seed node for a decentralized peer-to-peer knowledge web:

- **Git-free pool replication:** Any node replicates the content-addressed store directly over HTTP via `/packs`, `/roots`, `/root`, and `/bootstrap.tgz`.
- **Browser-local content-addressed cache (`IndexedDB`):** Visiting [`/kg`](https://hyperactivity.ai/kg) maintains a local `IndexedDB` store (`hx-cas-v1`) keyed by SHA-256 hash. Every module fetched once is stored locally and served to peers.
- **Stateless WebRTC P2P mesh & ambient co-presence:** `/api/p2p/signal` and `/api/live/pulse` hold zero open sockets and run zero server compute — the server only introduces peers for WebRTC `RTCDataChannel` handshakes (`WANT` / `OBJ` set-union sync) and lightweight presence beacons. Once connected, all object exchange flows browser-to-browser.
- **Client-side execution (`cubical.wasm`):** [`/kernel`](https://hyperactivity.ai/kernel) runs the real cubical kernel directly inside the user's browser.
- **Universal Knowledge Graph portals:** Every module or term referenced on any page of `hyperactivity.ai` is a live clickable portal into its exact content-addressed `.hypercubical` source and dependency neighborhood.

---

## 11. Build, Run & Inspect

Requires a C11 compiler (`gcc` or `clang`) and `make`.

```sh
git clone https://github.com/avikj/hyperactivity.git
cd hyperactivity
make
./hyperactive
```

`store/` holds immutable SHA-256 objects read and written through `./hyperactive` and `./build/cubical` (both `.hypercubical` and `.cub` module extensions resolve natively):

```sh
./hyperactive def two-learner-asym-sos-id       # inspect any definition & its content-addressed key
./build/cubical neuro/lyapunov.hypercubical     # run / verify a module
./build/cubical lib/guard.hypercubical          # run the repository guard
make brunerie                                   # compute Brunerie's number (-2) on 1 and 4 workers
./build/cubical lib/server.hypercubical 8080    # boot a local mirror of hyperactivity.ai
```

### Repository Layout

```text
src/            C11 interaction-net evaluator, cubical normalizer, content-addressed store & drivers
store/          immutable SHA-256 objects (lib/, neuro/, chem/, phys/, lqg/, fold/, docs/)
tools/          lifters (clang2cub, mir2cub, sail2cub, agda2cub) & assembly helpers
experiments/    Brunerie's number (-2), C++ & Rust lifting suites
Makefile        native kernel, library, and Brunerie targets
hyperactive     entrypoint symlink to build/cubical
```
