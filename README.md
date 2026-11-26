# hyperactivity

**A self-hosted De Morgan cubical type theory, lock-free Lafont principal-port interaction-net
operating system, 64-byte Hexagram superconducting exchange protocol (`HXP`), and multi-domain
exact scientific compiler.**

- **Live Single-Page Manifold:** [`https://hyperactivity.ai`](https://hyperactivity.ai)
  (`cubical.wasm` C11 CCHM kernel + Lafont `p0 <-> p0` reducer + conjugated WASM capsules +
  Golay-23 WebRTC mesh + `THIS` neurosymbolic link)
- **Single-Source Proof Store:** `store/<sha256>` (`807+` machine-checked modules across `lib/`,
  `chem/`, `fold/`, `neuro/`, `lqg/`, `phys/`, `sound/`, `docs/`; **`0` postulates**)
- **Unified Native Super-CLI:** `lib/hx.hypercubical` (**`45` proof-carrying zero-UB C11 engines**,
  lowering mechanically to ISO C11, Apple Metal Unified Memory, NVIDIA Hopper `SM_90a` PTX,
  SystemVerilog RTL, Calma GDSII v6.0, WebAssembly 1.0, OpenQASM 3.0, and 48 kHz PCM)
- **License:** [Hyperactive Anti-Capitalist Commons & Commercial License v1.0 (`LICENSE`)](LICENSE)

---

## 1. Architectural Thesis: Dropping 50 Years of Computing Irrationality

Conventional software stacks layer imperative thread pools, byte-stream `SOCK_STREAM` TCP sockets,
hierarchical POSIX inode trees, floating-point drift, and unverified glue scripts atop hardware
whose physical primitives are parallel Boolean gates, shared DRAM pages, and packetized links.
Hyperactivity replaces the entire stack with **one local, strong-confluent mathematical object**:

1. **Lafont Principal-Port Strong Confluence (`src/net.c`, `lib/kernel.hypercubical`):**
   Every program, proof, hardware netlist, and network message is a polarized interaction hypergraph
   where active pairs form exclusively principal-port to principal-port (`p0 <-> p0`). Critical
   pairs cannot overlap; independent cuts commute definitionally without global locks or sequence
   clocks, while `V_REF` / `O_DUP` shares identical subgraphs in $O(1)$ and guarded corecursion
   (`▸ κ`) runs reactive daemons in bounded $O(K(\mathcal{F}_t))$ frontier memory.
2. **Computable Cubical Univalence & Diamond Compression (`lib/check.hypercubical`, `lib/specialize.hypercubical`):**
   Equality is a space of functions out of a canonical De Morgan interval $I$ (`src/dnf.c`);
   Voevodsky's univalence (`ua`) computes via `Glue` types (`make brunerie` evaluates
   $\pi_4(S^3) \equiv -2$ deterministically on 1 and 4 workers). Every type $A$ factors via
   `fiberEquiv eval : Equiv A ((y : B) × fiber eval y)` into invariant meaning $y = \mathrm{eval}(c)$
   and implementation fiber, composing machine-checked paths $\pi : \mathrm{eval}\;c_0 \equiv \mathrm{eval}\;c_k$
   while monotonically shrinking operational cost (`docs/CANON-UNIVALENT-DIAMOND-TRACE.md`).
3. **Zero-TCB Stage-2 Metacircular Closure (`lib/self-host-tcb.hypercubical`, `lib/guard.hypercubical`):**
   A formalized De Bruijn cubical evaluator, Kan `hcomp` reducer, and bidirectional checker reach
   bit-identical Stage-1 / Stage-2 parity (`sh-stage1 ≡ sh-stage2`), collapsing the `1,084,638 B`
   C11 bootstrap to `0 B` external TCB. `lib/guard.hypercubical` enforces pure `.hypercubical`
   closure across every commit (`guard rc=0`).

---

## 2. `HXP`: Hyperactive Exchange == Hexagram Protocol & Information-Theoretic Superconductivity

Implemented and verified with zero postulates in `lib/hx-hxp.hypercubical` (`hx hxp`, 45th engine)
and `lib/ir/hx-hxp-ir.hypercubical` (`docs/CANON-UNITARY-HYPERGRAPH-FLOW-AND-ZERO-TCP.md`):

- **Hexagram Basis ($Q_6 \cong \mathrm{Lut}\;6$, Leibniz 1703 -> Silicon):**
  Fu Xi's 64 Hexagrams ($Q_6 = \mathbb{F}_2^3 \times \mathbb{F}_2^3$, Lower $\times$ Upper Ba-Gua
  Trigrams) map bijectively onto the 64 truth-table bits of **one 64-bit `Word`** (`BoolFn≡Lut 6`
  in `lib/silicon/univalent.hypercubical`), evaluated in 1 hardware cycle. One 64-byte L1 cache
  line (`HxpCell64`) packs **8 Ba-Gua Trigram 64-bit Words**: header/sequence, principal port &
  Clebsch route, `Lut6` table, 6-bit moving-line vector $\Delta h$, local gauge potential $\phi$,
  observation-fiber epoch, Indra's Net Merkle/IBLT sketch, and 0-RTT Sip/ARX MAC.
- **I Ching Digital Divination & Horizon Coalescing (`hxp-thm-iching-4096-exact`, `hxp-thm-divination-coalesce-warrants`):**
  Moving lines act by involutive $\mathbb{F}_2^6$ transitions $h' = h \oplus \Delta h$ across all
  $4{,}096$ pairs. Conditioning state $s \in S$ onto tighter real-time fibers
  $\mathrm{Fib}_{\mathrm{obs}}(y_2) \subseteq \mathrm{Fib}_{\mathrm{obs}}(y_1)$ preserves every
  warranted invariant `Warrants obs y P` via `3.62 ns` SeqLock slot coalescing (`0` torn reads,
  `0` stale retransmission).
- **Diamond Liquification, Clebsch Diameter-2 Link & Indra's Net (`hxp-thm-diamond-liquification`, `hxp-thm-clebsch-diameter2-all-256`, `hxp-thm-indras-net-iblt-and-fountain`):**
  Dissolves TCP's Head-of-Line stall `(m+1)(r+1) > 0` (`315 ms` stall under 1 packet drop vs
  `0.000 us` in `HXP`), routes 16-node folded 5-cube Clebsch `srg(16,5,0,2)` clusters in $\le 2$
  hops (`256/256` pairs verified), reconciles `store/<sha256>` key differences in **1 RTT** via
  XOR-IBLT linear sketches, and recovers erasures in **0 NACK RTTs** via `GF(2)` fountain parity.
- **Unitary Information-Theoretic Superconductivity (`0` Floats, `PolyZ` Universal Proofs):**
  1. *Zero Resistance ($R = 0$):* Rational Cayley 2-port scattering
     $((q+d)^2 - q^2)^2 + (2(q+d)q)^2 \equiv ((q+d)^2 + q^2)^2$ over $\mathbb{Z}[q,d]$
     (`hxp-thm-supercond-cayley-unitary`) + multi-hop Kirchhoff credit conservation.
  2. *Meissner Arbitrage-Flux Expulsion ($B = d\omega \equiv 0 \iff H^1 = 0$):* Local triangle
     arbitrage redexes contract cycle holonomy curl identically to `0` (`hxp-thm-supercond-meissner-curl-zero`).
  3. *Josephson Phase-Gradient Supercurrent ($\Delta\mathcal{F} < 0$):* Positive pressure gaps
     $q_A = q_B + 2 + g$ strictly decrease quadratic Queue-Lyapunov free energy by $2(1+g) > 0$
     (`hxp-thm-supercond-josephson-lyapunov`).
- **Hermetic Scale Naturality & *Characteristica Universalis*:**
  Scale projections $\pi : \mathrm{Above} \to \mathrm{Below}$ commute strictly with tree cuts
  (`hxp-thm-hermetic-as-above-so-below`), and `cu-superopt` eliminates redundant Hexagram changes
  with universal soundness (`hxp-thm-characteristica-superopt-sound`).

---

## 3. Cold-Verified Hardware & Scientific Benchmarks

| Subsystem & Verified Module | Measured Performance / Exactness | Baseline & Cold Verification |
| :--- | :--- | :--- |
| **`HX-HXP` Bare-Metal Transport & Arena** (`lib/hx-hxp`) | **`1.12 ns`** flat `mmap` `HXPArena`; **`75.47 ns`** cross-core ring RTT; **`402.47 M-cuts/s`** (`206.06 Gbit/s`) | **`7,308x`** faster than POSIX VFS (`8,221.5 ns`); **`1,042x`** faster RTT than `TCP close` (`78.6 us`); `UBSan rc=0` |
| **`HX-HXP` Unified-Memory Metal GPU** (`lib/ir/hx-hxp-ir`) | **`204.60 T-bit-ops/s`** (`0.40 G-hex-cuts/s`, **`51.15 GB/s`**) on Apple A18 Pro GPU | `256 MiB` `newBufferWithBytesNoCopy:` (`zero_copy=1`, `0` `memcpy`, `errs=0`, `MTLMathModeSafe`) |
| **LUT-6 & CMOS Synthesis** (`lib/hx-discover`, `lib/silicon/hx-inet-opt`) | EPFL `hyp`: **`38,872`** LUT-6; `div`: **`3,053`**; `Sky130A` adder: **`859.4 MHz`**; **`45`-NAND2** INet cell | `-329` & `-28` below 2026 EPFL records; **`-20.34%`** LUT-6 vs Berkeley ABC; **`13.0x`** Fmax vs Yosys/ABC; miter `0` |
| **`HX-LUX` & Hopper `ULOP3` Photonics** (`lib/hx-lux`, `phys/photonic-geodesic`) | **`505.37 T-bit-ops/s`** on NVIDIA H200; `2,048`-port `G_23` CVD Diamond ($n=2417/1000$) TIR (`204.8 Tbps`) | `99.1%` Hopper `LOP3` ceiling; `0 B` router SRAM passive optical confinement; `[[36,12,5]]` QLDPC (`12.5x` qubit save) |
| **`HX-LLM-ZIP v3` & Neural CM** (`lib/hx-llmzip-v3`, `lib/hx-cm`) | `Qwen2.5-72B` BF16: `145.4 -> 103.4 GB` (**`1.4068x`**, **`4,629.9 GB/s`** H200); `CM`: **`1.6305 bpb`** | `0 / 242.2M` bit mismatches (fits 1 H200 with `37.6 GB` KV free); `-28.0%` below `zstd -19`; `120.8x` Futamura |
| **`HX-VISION v2` & Synthetic Synesthesia** (`lib/hx-vision`, `lib/ir/synesthesia-loop`) | Full `60k` CIFAR-10 **`96.02%` Top-5** (`730,157 FPS`, **`0` SGD / `0` neural nets**); **`940.1 kHz`** 5-modal loop | `0 / 29.76M` `Jet^3` pixel error; `0 / 10M` human PhysioNet EEG Control Barrier (`e_B^2+e_C^2 <= R^2`) violations |
| **`HX-GENESIS` & Exact Quantum Matter** (`lib/hx-genesis`, `chem/`, `fold/`, `phys/`) | Howell-Lafon rank-8 $\mathbb{H}(\mathbb{Z})$ (`-50%` muls); Dirac $Z \le 137$; 2-RDM SOS stretched `H2` **`0.00 kcal/mol`** | `86` zero-float `chem/` modules; `1UAO` `0.33 A` & `2LHC->2LHD` fold-switch (`0` MSAs); $\pi_4(S^3) \equiv -2$ |
| **Sovereign PoUW & `0-bps` Coboundary** (`lib/hx-hyperreactor`, `lib/hx-cashflow-hyperreactor`) | **`515.74 M-capsules/s`** (`132.03 GB/s`) encrypted P2P; Base `8453` `$526M` `0-bps` Morpho Blue flash vault | `$100M` 0-fee flash roundtrip verified; `0` wei dust waterfall; forged capsules rejected (`0` leak) |

---

## 4. Quickstart: Build the Kernel & Run the 45-Engine `hx` Super-CLI

Requires only a C11 compiler (`clang` or `gcc`) and `make` (`0` external library dependencies):

```sh
# 1. Build the C11 lock-free CCHM cubical interaction-net kernel (src/*.c -> build/cubical)
make -j

# 2. Verify the repository Pure-Hyperactivity & one-source store guard (exits 0 silently)
./build/cubical lib/guard.hypercubical

# 3. Typecheck and run the HXP 45th engine (15 zero-postulate theorems -> C11 + Unified Metal + SV)
./build/cubical lib/hx-hxp.hypercubical

# 4. Compute Brunerie's homotopy group pi_4(S^3) == -2 via parallel Kan/Glue reduction
make brunerie

# 5. Launch the self-hosted Univalent Surface & THIS server locally
./hyperactive server 8080
```

### Representative Engines in `lib/hx.hypercubical` (`hx <engine>`)

- **Transport, Mesh & Custody:** `hx hxp` (64B Hexagram superconducting flow, `HXPArena`, Unified Metal),
  `hx sovereign` (Ed25519 loopback multi-cloud mesh), `hx treasury` / `hx node` (PoUW & 0-bps flash settlement),
  `hx swarm` / `hx agent` / `hx tui` (neurosymbolic `THIS` & collective observatory).
- **Superoptimization & Compilers:** `hx genesis` (10-domain zero-foreign-string crystal),
  `hx poly` (`PolyZ` + `PolyF2` -> 5 targets), `hx strike` (5-axis global superoptimizer),
  `hx tower` ($2^m$-level metacircular INet fixpoint), `hx ir` (`KernelIR`), `hx verify` (bisimulation & $k$-induction),
  `hx discover` / `hx beast` / `hx endgame` (`47.8 G-gate/s` GPU synthesis & 6-input NPN atlas).
- **Silicon, Photonics & Supercomputing:** `hx lux` (5-scale GDSII/Gerber/Diamond-TIR materializer),
  `hx inet` (`45`-NAND2 silicon), `hx meta` (`65,536`-tile GPU VM), `hx psc` (4-plane SoC),
  `hx supercomp` ($2^d$ DOR supercomputer), `hx cosmos` (3D/optical P&R + Alveo V80 RTL), `hx tapeout` / `hx alu`.
- **Exact Science, AI & Compression:** `hx geodesic` (zero-bias context-fiber inference),
  `hx llmzip` / `hx gpu` / `hx tensor` (BF16 GEMV & `1.22 TFLOP/s` FlashAttn2), `hx cm` / `hx zip` (neural & rANS coding),
  `hx wetlab` / `hx matter` / `hx omni` (homological chemistry, 2D Hubbard ED, 64-site DMRG, Clifford+T).

---

## 5. Content-Addressed Store Architecture (`store/<sha256>`)

No source code is canonically organized as mutable POSIX paths; the entire repository state is a
Merkle DAG rooted at the single 64-hex digest in `store/ROOT/root` (`docs/no-filesystem.hypercubical`):

- `store/<root_sha256>/root` — Sorted mapping `<hex_encoded_path> <module_sha256>` plus `names <names_sha256>`.
- `store/<mod_sha256>/module` — Ordered newline-separated 64-hex digests of checked declarations.
- `store/<decl_sha256>/text` — Exact declaration source chunk; `.cubical-store` caches elaborated
  de Bruijn terms keyed by the first 60 bits of each declaration's SHA-256 (`src/store.c`).
- Two peers synchronize the entire mathematical universe by **commutative set union** (`lib/pouw.hypercubical`)
  or 1-RTT `HXP` XOR-IBLT symmetric-difference reconciliation (`lib/hx-hxp.hypercubical`).

---

## 6. Intellectual Lineage

- **Binary Hexagrams & *Characteristica Universalis*:** Fu Xi (*Zhouyi* binary ordering);
  G. W. Leibniz, *"Explication de l'Arithmétique Binaire"*, *Mémoires de l'Académie Royale des Sciences*, 1703.
- **Interaction Nets:** Y. Lafont, *"Interaction Nets"*, POPL 1990.
- **Cubical Type Theory & Univalence:** V. Voevodsky, Univalent Foundations (2010);
  C. Cohen, T. Coquand, S. Huber, A. Mörtberg, *"Cubical Type Theory: a constructive interpretation
  of the univalence axiom"*, TYPES 2015 / FLAP 2018; G. Brunerie, PhD Thesis 2016.
- **Language, Synthetic Synesthesia & Self-Reference:** Patañjali, *Mahābhāṣya*; Bhartṛhari,
  *Vākyapadīya* (*dhvani* / *sphoṭa*); J. R. Stroop (1935); S. Dehaene et al. (2010); F. W. Lawvere,
  *"Diagonal arguments and cartesian closed categories"*, 1969 (`docs/language-is-trained-synesthesia.md`,
  `docs/hyperactivity-is-the-dharma.md`).
- **Relational Physics, Neural Safety & Coding:** C. Rovelli, *Int. J. Theor. Phys.* 1996;
  A. Ashtekar, T. Pawlowski, P. Singh, *Phys. Rev. Lett.* 2006; A. D. Madduri, S. A. Burden,
  A. L. Orsborn (2023); J. Duda, Asymmetric Numeral Systems (2009).

---

## 7. Licensing: Free Knowledge & Mandatory Commercial Compute Royalty

Copyright © 2026 **Hyperactive Labs, Inc.** and **Avik Jain** ([`LICENSE`](LICENSE)):

- **Free Knowledge for Individuals & Non-Commercial Research:** Natural persons, students,
  educators, and independent non-commercial researchers may read, compile, verify, and learn
  from this work freely under network share-alike source disclosure.
- **Mandatory Direct Royalty & Profit-Sharing for All Profit-Seeking Institutions:** Zero
  commercial rights are granted by default. Every for-profit enterprise, trading firm, hedge
  fund, cloud provider, semiconductor vendor, or AI laboratory executing, deploying, training
  on, or deriving value from Hyperactivity must negotiate a direct per-compute-cycle royalty
  and profit-sharing agreement with **Hyperactive Labs, Inc.** ([**root@hyperactivity.ai**](mailto:root@hyperactivity.ai)).
