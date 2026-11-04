# Brunerie number experiment

`brunerie.cub` is an untyped port of Ljungström's `brunerie'` from agda/cubical
(`Cubical/Homotopy/Group/Pi4S3/BrunerieExperiments.agda`). The file's comment says it computes in about 1 second in Agda.
The pipeline is the same: `g10 (g9 (g8 λ i j → f7' (η₃ (push (loop i) (loop j)))))`.

The port includes the h-level proofs that the pipeline evaluates:
- `TL2` and `TL3`: isOfHLevelTypeOfHLevel 2 and 3, via the Σ≡Prop retract and the univalence retract (uaη, transpEquiv).
- `P4`: isOfHLevelΠ 4.
- `help`: the S²ToSetElim equivalence proof.

Two proofs are still placeholders, `HOLE_E4` (surfc on `squash4`) and `HOLE_SZ` (isSetℤ). If either one is evaluated, its name shows in the result.

Run from `scoped-machine/`:

    CUBICAL_NOCACHE=1 ./build/cubical lib/prelude.cub lib/code.cub lib/check.cub lib/std.cub \
      experiments/brunerie/brunerie.cub experiments/brunerie/run-<name>.cub

| driver | what it shows | measured |
|---|---|---|
| `run-circle.cub` | g9 on `loop` and on `loop·loop` | `t2inc (pos 1)` and `t2inc (pos 2)` |
| `run-square.cub` | F, the η₃-on-push square through f7' | 19.4M steps without the patch, 2.4M steps with it; contains one `squash4` |
| `run-cube4.cub` | encodeTruncS2 along a genuine `squash4` 4-cube, which exercises TL3 | does not finish in 400 s within 3 GB |
| `run-brunerie.cub` | the number | runs out of memory (more than 6.5 GB, about 3 min); no numeral yet |

The measurements were taken at commit c5aab2d. They were not re-measured after 11907e3 and 04f761c.

The parser reads a literal `0` or `1` given as a constructor's interval argument as Nat `zero`. Bind the interval with `λ (a : I)` and apply it with `@` instead.

`empty-hcomp-experiment.patch`, made against c5aab2d, makes `hcompD` with an empty system reduce to its base in every data type.
It is a stand-in for measuring only, and it is not coherent for higher inductive types: eliminating into the universe gives `Glue A []`, not `A`.

## The two causes

1. **Transport is `comp A [] a0`.** In a data type with path constructors, this leaves a formal `hcompD []` that never goes away.
   F carries 20 such layers around values that are really constant.
   `codeTruncS2` turns each layer into a composition in the universe of groupoids.
   Transport through n layers took 779, 2036, 6755 and 30385 steps for n = 1, 2, 3, 4.
   The fix is a transport primitive that is the identity on closed data and acts on constructors for data with parameters (`%transD`), with comp = hcomp ∘ transp.
2. **`%comp-u` encodes a composition of types as Glue with `lineequiv` equivalences.**
   Transporting through such a Glue (`%comp-glue`) needs `snd (snd S1) a1`, the fibre contraction of each equivalence on a true face.
   For a `lineequiv`, that contraction is itself a composition in the fibre types (Σ of a path) along the face line, one Glue level down.
   TL3 on the 4-cube nests about four such levels (retract hcomp, univalence retract, uaη, Hopf Glue), with up to 8 faces each.
   Agda instead treats a composition of types as its own type former, and transports through it using only transport along the faces.
