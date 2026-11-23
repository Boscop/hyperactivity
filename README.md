<p align="center">
  <a href="https://hyperactivity.ai/nets">
    <img src="https://hyperactivity.ai/readme-hero.svg" alt="Hyperactivity — Active Pair Reduction and Guarded Corecursion Interaction Net" width="100%" />
  </a>
</p>

<h1 align="center">hyperactivity</h1>

<p align="center">
  <strong>A cubical interaction-net operating system language and decentralized knowledge graph.</strong><br/>
  <a href="https://hyperactivity.ai">hyperactivity.ai</a> ·
  <a href="https://hyperactivity.ai/kg">Knowledge Graph (<code>/kg</code>)</a> ·
  <a href="https://hyperactivity.ai/kernel">Browser Kernel (<code>/kernel</code>)</a> ·
  <a href="https://hyperactivity.ai/nets">Interaction Net Lab (<code>/nets</code>)</a> ·
  <a href="https://hyperactivity.ai/paper">Paper</a> ·
  <a href="https://hyperactivity.ai/pdf">PDF</a> ·
  <a href="LICENSE">License</a>
</p>

---

### Run it now

**In your browser (zero install, zero server compute):**
- [**`hyperactivity.ai/kernel`**](https://hyperactivity.ai/kernel) — run the C11 cubical kernel compiled to WebAssembly (`cubical.wasm`) locally in your browser.
- [**`hyperactivity.ai/kg`**](https://hyperactivity.ai/kg) — explore the live content-addressed Knowledge Graph with browser-local `IndexedDB` caching and WebRTC peer-to-peer object sync.
- [**`hyperactivity.ai/nets`**](https://hyperactivity.ai/nets) — step through active-pair reductions, parallel certified sort, cubical paths, Kan boxes, univalence, and guarded artificial life (`life ▸ κ`).

**On your machine (requires `gcc`/`clang` + `make`):**

```sh
git clone https://github.com/avikj/hyperactivity.git && cd hyperactivity && make
./build/cubical neuro/lyapunov.hypercubical     # verify a module (.hypercubical and .cub both resolve)
make brunerie                                   # compute Brunerie's number (-2) on 1 and 4 workers
./hyperactive def two-learner-asym-sos-id       # inspect any definition & its SHA-256 content key
./build/cubical lib/server.hypercubical 8080    # boot your own local mirror of hyperactivity.ai
./hyperactive                                   # full interactive onboarding walk over store/
```

---

## 1. Why this exists — and how it runs

Computing since 1970 runs on untyped bytes, mutable filesystems, and centralized servers. Because raw bytes carry no meaning and equality is broken, every field builds brittle workarounds on top: token predictors that hallucinate, scalar preference averages that erase disagreement, canonical string IDs (like InChI in chemistry) that collapse distinct substances, blockchains to fake consensus, and endless manual rewrites between languages and hardware.

Hyperactivity replaces that substrate with one self-hosted machine built on **local interaction networks** (Lafont, 1990). There is no primitive thing, only a primitive law of interaction. In `src/core.h` and `src/net.c`, every term is a net of **cells** connected by **wires**:

- Every cell has one **principal port** and zero or more auxiliary ports.
- A **value** offers itself at its principal port (`λ`, pair, type, constructor, interval endpoint, partial system). An **operation** demands a value at its principal port (`apply`, `first`, `second`, `path-apply`, `compose`, `case`, `subst`, `duplicate`, `erase`).
- Computation happens only at an **active pair** — an operation's principal port wired directly to a value's principal port. Disjoint active pairs reduce concurrently across cores with no locks and no global clock, and values compute only as far as a reader demands them.

```text
-- Functions, pairs & guarded corecursion (▸ κ)
apply(arg)      ⋈  λ               ⟶  body [param := arg]
first / second  ⋈  pair(a, b)      ⟶  a / b   (other wire erased)
force           ⋈  delay(t)        ⟶  t

-- Cubical paths & dimension substitution
path-apply(r)   ⋈  path-λ i. t     ⟶  subst [i := r] facing t
path-apply(0|1) with known ends    ⟶  that endpoint directly (path never read)
subst           ⋈  λ, pair, Π, Σ   ⟶  same cell, subst facing each part
subst           ⋈  system          ⟶  branch whose face became true, else reduced system

-- Optimal sharing
duplicate       ⋈  value w/ parts  ⟶  value held once; each side gets a reference
duplicate       ⋈  atomic value    ⟶  two copies
erase           ⋈  value           ⟶  gone; erase facing each part

-- Kan composition, Glue & Univalence (ua)
compose         ⋈  total tube      ⟶  tube's far end
compose         with partial tube  ⟶  type-directed Kan clause (Π, Σ, Path, U, Glue, Data)
Glue / glued    when face is true  ⟶  type / value on that face
unglue          on true face       ⟶  equivalence map applied
unglue          ⋈  glued value     ⟶  base
transport       ⋈  constructor     ⟶  constructor's transport clause
```

Every Kan composition clause is itself a term of the language.

---

## 2. Identity as computation: cubical paths & univalence (`ua`)

Hyperactivity implements **CCHM cubical type theory** (Cohen, Coquand, Huber, & Mörtberg, 2018) with a self-hosted type checker (`lib/check.hypercubical`). An interval $I$ with endpoints $0, 1 : I$ turns equality into a function out of the interval:

```math
\mathrm{Path}_A(a, b) = \{\, p : I \to A \mid p(0) \equiv a,\; p(1) \equiv b \,\}
```

Any equivalence $e : A \simeq B$ produces a path $\mathrm{ua}(e) : \mathrm{Path}_{\mathcal{U}}(A, B)$ via `Glue` types (Voevodsky's Univalence Axiom, computing directly through the active-pair laws above):

```math
\mathrm{transport}\;(\lambda i.\; P(\mathrm{ua}(e)\,i))\;x \;\longrightarrow^{\ast}\; e_{\ast}(x)
```

Digest equality (identical SHA-256 hash in `store/`), definitional equality (identical normal form), and path/semantic equivalence (`ua`) stay distinct — so a proof, optimization, or chemical/physical identity established once transports automatically across every equivalent representation.

Reproduce **Brunerie's number** $\beta \in \pi_4(S^3) \cong \mathbb{Z}/2\mathbb{Z}$ (`negsuc (suc zero)` = $-2$, Ljungström's typed port) on 1 and 4 parallel workers directly:

```sh
make brunerie
```

---

## 3. Observation fibers, warrant & zero-search compression

<p align="center">
  <a href="https://hyperactivity.ai/ai">
    <img src="https://hyperactivity.ai/readme-fiber.svg" alt="Observation Fiber Fib_o(y) and Split Witness Collapse" width="100%" />
  </a>
</p>

An agent never sees the full world $h \in H$; it sees an observation $y = o(h) \in O$. Its exact epistemic state is the **observation fiber** — every world-history compatible with $y$ ([`lib/observation.hypercubical`](https://hyperactivity.ai/kg?mod=lib/observation.hypercubical)):

```math
\mathrm{Fib}_o(y) = \sum_{h : H} \big(o(h) \equiv y\big), \qquad \mathrm{Warranted}(o, y, P) :\equiv \prod_{h : H} \big(o(h) \equiv y\big) \to P(h)
```

- **AI alignment & standpoint ([`hyperactivity.ai/ai`](https://hyperactivity.ai/ai)):** Claiming $P$ when two histories $(h_1, h_2)$ in $\mathrm{Fib}_o(y)$ disagree yields an explicit **split witness** instead of a hallucination. Active querying (`WorldAfter`) shrinks the fiber until every surviving history agrees. Merging agents takes the content-addressed set union of warranted claims with explicit split witnesses rather than collapsing incompatible human standpoints into a 1D RLHF scalar average.
- **Zero-search lossless compression ([`hyperactivity.ai/compression`](https://hyperactivity.ai/compression)):** Applying the same causal observation-fiber machinery directly to ordered byte streams produces a streaming $O(N)$ renormalized rANS codec (Duda, 2009; [`lib/compress.hypercubical`](https://hyperactivity.ai/kg?mod=lib/compress.hypercubical), [`lib/compress-stream.hypercubical`](https://hyperactivity.ai/kg?mod=lib/compress-stream.hypercubical), [`lib/optimality.hypercubical`](https://hyperactivity.ai/kg?mod=lib/optimality.hypercubical)) with zero search, zero probability axioms, zero serialized lookup tables, universal `refl` round-trip proofs, and Shannon-rate / Kraft / pigeonhole bounds — outperforming `gzip -9`, `bzip2 -9`, and `lzma`.

---

## 4. Lifting, specialization & 3nm silicon

A foundation that requires rewriting existing software and hardware is dead on arrival. Hyperactivity lifts real C++, Rust MIR, x86-64 ISA semantics (with TSO/RC11 memory models), and Agda modules into typed values of the host (`tools/clang2cub.py`, `tools/mir2cub.py`, `tools/sail2cub.py`, `tools/agda2cub.py`).

- **Specialization & native lowering ([`lib/specialize.hypercubical`](https://hyperactivity.ai/kg?mod=lib/specialize.hypercubical)):** Programs specialize together with an equivalence path back to source and emit standalone static x86-64 ELF64 binaries (`lib/elf.hypercubical`) or JIT machine code (`lib/x86.hypercubical`).
- **Full-stack silicon ([`hyperactivity.ai/chip-design`](https://hyperactivity.ai/chip-design), `lib/silicon/`):** Circuit light-cone lower bounds, Landauer reversible computing, $[8,4,4]$ SECDED ECC, DPA masking, Tomasulo out-of-order execution, MESI cache coherence, `[[5,1,3]]` quantum error-correction compilation, byte-level GDSII v6.0 round-trip proofs, 5-layer 3nm layout, and GALS cubical interaction-net tile RTL (`lib/silicon/export.hypercubical`).

---

## 5. Neuroscience & closed-loop neural co-processors

> **Interactive phase-space & two-learner simulations:** [**`hyperactivity.ai/neuro`**](https://hyperactivity.ai/neuro) · Run locally: `./build/cubical neuro/lyapunov.hypercubical`

From ion-channel electrochemistry (Hodgkin & Huxley, 1952) and the 2D **FitzHugh–Nagumo** excitable membrane oscillator (FitzHugh, 1961; Nagumo, 1962),

```math
\frac{dv}{dt} = v - \frac{v^3}{3} - w + I_{\text{ext}}, \qquad \frac{dw}{dt} = \varepsilon\,(v + a - b\,w)
```

cortical populations evolve on intrinsic manifolds where a readout $y = Cx$ induces an **output-null fiber** $\mathrm{Fib}_C(y) = x_0 + \ker C$ ([`neuro/descent.hypercubical`](https://hyperactivity.ai/kg?mod=neuro/descent.hypercubical)). In a closed-loop brain–machine co-processor (Carmena, Orsborn, Burden, & Madduri, 2023), brain ($B$) and decoder ($C$) co-adapt:

```math
\frac{de_B}{dt} = -\eta_B e_B + \gamma_1 e_C, \qquad \frac{de_C}{dt} = \gamma_2 e_B - \eta_C e_C
```

Hyperactivity decides symbolic **Sum-of-Squares (SOS)** stability and safety certificates in exact normal form over $\mathbb{Q}[x_1,\dots,x_n]$ via [`chem/algebra.hypercubical`](https://hyperactivity.ai/kg?mod=chem/algebra.hypercubical), so every proof holds for *all* real parameter values simultaneously:

- **Weighted Lyapunov SOS ([`neuro/lyapunov.hypercubical`](https://hyperactivity.ai/kg?mod=neuro/lyapunov.hypercubical)):** For $V = p_1 e_B^2 + p_2 e_C^2$,

```math
4 p_1 \eta_B \left(-\frac{dV}{dt}\right) = \big(2 p_1 \eta_B e_B - (p_1 \gamma_1 + p_2 \gamma_2)\,e_C\big)^2 + \Big(4 p_1 p_2 \eta_B \eta_C - (p_1 \gamma_1 + p_2 \gamma_2)^2\Big)\,e_C^2
```

  recovering the sharp Hurwitz boundary $\eta_B \eta_C > \gamma_1 \gamma_2$ constructively.
- **Forward-invariant Control Barrier Functions ([`neuro/barrier.hypercubical`](https://hyperactivity.ai/kg?mod=neuro/barrier.hypercubical)):** Exact Nagumo invariance for the two-learner safe set (`cbf-2learner-safe-sos-id`) and the externally driven FitzHugh–Nagumo trapping ellipsoid (`fhn-driven-trapping-sos-id`).
- **Dynamic interface-reshaping co-processors ([`lib/polyproc.hypercubical`](https://hyperactivity.ai/kg?mod=lib/polyproc.hypercubical), [`neuro/reshaping-coproc.hypercubical`](https://hyperactivity.ai/kg?mod=neuro/reshaping-coproc.hypercubical)):** Indexed guarded processes `RProc κ D A i` (channels opening/closing and decoders rebinding mid-run, with exact round-trip path `gproc-rproc-id`), proof-gated stimulation (`SafeGateDS`, `disagreement-blocks-gate`), REBUS prior relaxation ([`neuro/inhibition.hypercubical`](https://hyperactivity.ai/kg?mod=neuro/inhibition.hypercubical); Carhart-Harris & Friston, 2019), and synthetic synesthesia ([`neuro/synesthesia.hypercubical`](https://hyperactivity.ai/kg?mod=neuro/synesthesia.hypercubical)).

---

## 6. Relational physics, exact quantum chemistry & living matter

> **Interactive $SU(2)$ spin-network & bioelectric morphogenesis labs:** [**`hyperactivity.ai/physics`**](https://hyperactivity.ai/physics)

- **Relational Quantum Mechanics & Loop Quantum Gravity (`lqg/`):** RQM (Rovelli, 1996; Di Biagio & Rovelli, 2021; [`lqg/rqm.hypercubical`](https://hyperactivity.ai/kg?mod=lqg/rqm.hypercubical)), discrete $SU(2)$ spin networks with area spectrum $A(\mathcal{S}) = 8\pi\gamma\ell_P^2 \sum_{e \cap \mathcal{S}} \sqrt{j_e(j_e+1)}$, Thiemann QSD Hamiltonian vertices ([`lqg/hamiltonian-vertex.hypercubical`](https://hyperactivity.ai/kg?mod=lqg/hamiltonian-vertex.hypercubical)), $U(1)$ magnetic loop flux sectors ([`chem/magloop.hypercubical`](https://hyperactivity.ai/kg?mod=chem/magloop.hypercubical)), semiclassical bridges ([`lqg/semiclassical-bridge.hypercubical`](https://hyperactivity.ai/kg?mod=lqg/semiclassical-bridge.hypercubical)), 4D Pachner invariants, and the Ashtekar–Pawlowski–Singh Loop Quantum Cosmology Big Bounce $H^2 = \frac{8\pi G}{3}\rho(1 - \rho/\rho_c)$ ([`lqg/cosmology-bounce.hypercubical`](https://hyperactivity.ai/kg?mod=lqg/cosmology-bounce.hypercubical)).
- **Relativistic atoms & superconductivity (`phys/`):** Pauli–Dirac Clifford algebra, Foldy–Wouthuysen expansion, spin–orbit center-of-gravity invariance, $Z \le 137$ critical coupling ([`phys/relativistic-atom.hypercubical`](https://hyperactivity.ai/kg?mod=phys/relativistic-atom.hypercubical)), Bogoliubov–de Gennes dispersion, Ginzburg–Landau SOS positivity, and Hubbard superexchange ([`phys/superconductivity.hypercubical`](https://hyperactivity.ai/kg?mod=phys/superconductivity.hypercubical)).
- **Zero-float ab initio chemistry, folding & morphogenesis (`chem/`, `fold/`):** Exact rational variational Ritz bounds for $\text{H}_2$ and Bareiss–Sylvester Helium, fermionic CAR algebra, structural chemical identity carrying tautomeric equivalence as cubical paths rather than lossy InChI strings (IUPAC #2012-023-2-800), exact rational $SO(3)/SE(3)$ all-atom peptide backbone kinematics ([`fold/backbone-kinematics.hypercubical`](https://hyperactivity.ai/kg?mod=fold/backbone-kinematics.hypercubical)), and gap-junction bioelectric morphogenesis (Levin).

---

## 7. No filesystems, conserved credit & the decentralized web seed

<p align="center">
  <a href="https://hyperactivity.ai/kg">
    <img src="https://hyperactivity.ai/readme-kg.svg" alt="Decentralized Content-Addressed P2P Web Seed" width="100%" />
  </a>
</p>

Every definition, proof, document, and web route lives as an immutable SHA-256 object inside `store/` (generalizing Unison's content-addressed code to a full OS and knowledge graph). Independent machines and browsers converge by pure **set union** — no canonical server state and no blockchain — while [`lib/credit.hypercubical`](https://hyperactivity.ai/kg?mod=lib/credit.hypercubical) tracks conserved multi-dimensional credit across contributions.

[`lib/server.hypercubical`](https://hyperactivity.ai/kg?mod=lib/server.hypercubical) serves [hyperactivity.ai](https://hyperactivity.ai) directly out of `store/` as a peer-to-peer seed node:
- **Git-free pool sync:** `/packs`, `/roots`, `/root`, and `/bootstrap.tgz` replicate `store/` over HTTP without GitHub.
- **Browser-local `IndexedDB` CAS + WebRTC P2P:** [`/kg`](https://hyperactivity.ai/kg) caches SHA-256 objects locally (`hx-cas-v1`), coordinates peer handshakes via stateless `/api/p2p/signal` and `/api/live/pulse` (zero open sockets, zero server compute), and syncs objects directly over `RTCDataChannel`.
- **Client-side execution:** [`/kernel`](https://hyperactivity.ai/kernel) runs `cubical.wasm` in the browser.

```text
src/            C11 interaction-net evaluator, cubical normalizer, content-addressed store & drivers
store/          immutable SHA-256 objects (lib/, neuro/, chem/, phys/, lqg/, fold/, docs/)
tools/          lifters (clang2cub, mir2cub, sail2cub, agda2cub) & assembly helpers
experiments/    Brunerie's number (-2), C++/Rust lifters & WIP experimental suites
Makefile        native kernel, library, and Brunerie targets
hyperactive     entrypoint symlink to build/cubical
LICENSE         Hyperactive Anti-Capitalist Commons & Commercial License v1.0
```

---

## 8. License

Copyright © 2026 **Hyperactive Labs, Inc.** and **Avik Jain**. Governed by the [**Hyperactive Anti-Capitalist Commons & Commercial License v1.0 (`LICENSE`)**](LICENSE) — a **Copyfarleft** dual license:

- **Free & Copyfarleft for the Non-Capitalist Commons:** Natural persons acting outside capitalist enterprise, students, educators, independent scientists, non-commercial public/academic research groups, charitable nonprofits, and democratically governed 100% worker-owned cooperatives have full royalty-free rights to read, compile, execute, verify, modify, and share — conditioned on **strong network share-alike copyleft** (every shared or network-served derivative remains under [`LICENSE`](LICENSE) with full corresponding machine-readable source disclosed).
- **Paid Commercial License Required for Capitalist Entities & Commercial AI:** Zero rights are granted under [`LICENSE`](LICENSE) to any for-profit corporation, venture-backed or investor-owned enterprise, hedge fund or trading firm, commercial cloud host, semiconductor vendor, or commercial AI laboratory — including internal corporate R&D, evaluation, hardware fabrication, and AI/LLM scraping, pre-training, fine-tuning, distillation, or RAG ingestion. Commercial licensing inquiries: [**root@hyperactivity.ai**](mailto:root@hyperactivity.ai).
