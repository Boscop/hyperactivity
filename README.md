# scoped-machine

A single normalizer for cubical type theory. Every term is compiled to a graph of cells and reduced
by local rewrites. Programs, proofs, the type checker, data structures and the rules for transport
all run on that one mechanism.

```
make test       # all suites; the same output with 0, 1, 2, 4, 8, 16 workers; again under ASan/UBSan
make library    # tests/boxes.ctt and tests/sort.ctt
./build/cubical lib/prelude.ctt lib/code.ctt lib/check.ctt lib/std.ctt FILE.ctt
```

Code: `src/net.c` (cells, rewrite rules, parallel scheduler), `src/dnf.c` (intervals, faces,
substitutions), `src/big.c` (exact naturals), `src/lang.c` (reader, compiler to cells, readback,
top-level forms, modules). Language files: `lib/prelude.ctt` (core data types, composition
clauses), `lib/code.ctt` (codes, `View`), `lib/check.ctt` (the type checker), `lib/std.ctt`
(checked library). Earlier plans and specs: `archive/`.

## 1. Rewrites see only the two cells involved

Each cell has one principal port; a step rewrites two cells whose principal ports meet. No rule
consults an environment or context. Being under a face such as `i = 0` is a substitution of the
dimension by the endpoint. Applying a dimension binder puts a substitution cell on its body, and
that substitution re-activates exactly the stuck values it unblocks: `p @ i` under `i := 0` becomes
the left endpoint of `p`. The step count is fixed by the term: identical with any number of worker
threads (`make test` compares every suite's output at 0, 1, 2, 4, 8 and 16). Reading a stored value
costs the same whether or not the value has arrived: a read that arrives first waits on the wire
and is served the same one-level copy, and waiting is not a step. Every reading is taken over the
complete normalization of what was posed. A worker locks the active pair and the cells it rewires,
in address order, and revalidates before rewriting.

## 2. Values include equality

Besides functions, pairs and data, values are: the interval (`0`, `1`, dimensions, `~`, `/\`,
`\/`, kept in normal form), paths (a value with a dimension abstracted), faces (conditions on
dimensions), partial values (systems), composition, Glue types and their elements, and higher
inductive types (data types with path constructors).

**Partial values.** `(sys ((= i 0) a) ((= i 1) b))` is a value defined only where its faces hold.
Each branch is kept unbuilt, as syntax with its environment, and is built with its face's
substitution applied (`a` with `i := 0`) only when an operation reads it. If a substitution makes a
face true (here `i := 0`), the whole partial value becomes that branch.

**Composition.** `comp A u a0` takes a line of types `A(i)`, a starting element `a0 : A(0)` and a
partial value `u` (where it is given, what the result must be along the line), and produces an
element of `A(1)` matching `u`. Transport is composition with nothing given. How it is computed
depends on what kind of type `A(i)` is; each case is a term in `lib/prelude.ctt`, not C:
- functions (`%comp-pi`): to apply the result to `x : A(1)`, move `x` back to each `A(i)`, apply,
  and compose the results in the codomain;
- pairs (`%comp-sigma`): compose the first components, then the second components along the line
  the first one traced;
- paths (`%comp-path`): compose pointwise, adding the path's two endpoints to what is given;
- universes (`%comp-u`) and Glue (`%comp-glue`): via Glue types; this is where univalence
  computes, so transport along `ua e` applies `e`.

**Data types.** One mechanism serves every declared data type, with nothing written per type:
- moving a constructor value along a line of data types (e.g. `List A(i)`) moves each of its
  fields along its field type; this is derived from the declaration;
- a path constructor such as `loop i` must still meet its declared endpoints after moving, so the
  result is corrected by a composition that glues those endpoints back on;
- a composition in a data type that cannot be computed further is kept as a value, `hcompD`; a case
  analysis meeting such a value applies itself to the pieces and composes the results in the
  result type.

## 3. Work is done once, and only where it is read

A shared subterm is held by a duplicating cell that copies lazily, one level at a time as it is
read. A stored value lives once, and readers copy its top cell. A substitution of dimensions is
recorded on the reference, not applied by copying. A subterm in a position that might not be read
is held unbuilt until an operation meets it: `(fst (pair a b))` never computes `b`, even if `b`
diverges. A λ's subterms that do not mention its variable are computed once, however often the λ is
applied. A case is one cell, and selecting a branch is one step.

## 4. Open terms normalize, with types

A stuck computation is one with no rule to fire because the thing it inspects is unknown: `f x`
with `f` a variable, `case n ...` with `n` a variable, `p @ i` with `p` a path variable and `i` a
dimension variable. It is kept as a value (a neutral) holding its parts; reduction continues
everywhere else, including inside the branches of a stuck case. When a substitution later
supplies the unknown, it fires. A neutral's type is computed from the variable at its head along
the operations applied to it, and a stuck path's endpoints are read from that type. So the runtime
returns full normal forms under binders, not just evaluation results.

## 5. Reflection: `%view`

`(%view v)` turns the outermost form of a computed value into an ordinary data constructor in one
step, whose fields are the value's own parts: a Π type gives `(wpi A B)` with `A` the domain and
`B` the codomain as a function, a stuck application gives `(wapp f a T)`. A program cases on this
like any data, so a program in the language can examine a value's structure. What it sees is the
computed value, not the source text that produced it; a function shows only as `(wlam f)`.
`(%var T)` and `(%dim)` make fresh variables. The type checker (`lib/check.ctt`) is written with
these and runs on this runtime, over the values the runtime already computed for each subterm:
there is no second evaluator, and checking a definition is running that program on it.

## 6. Inputs

`(input x)` declares a definition without a value. An operation that needs `x`'s value (applying
it, a case on it, a projection) is held at exactly that point. Everything that does not depend on
`x` reduces fully, and the parts of a result that do not need `x` are produced. `(supply x t)`
connects the value, and the held operations proceed. Unlike a variable, an input is not computed
with symbolically: what depends on it waits for it.

## Measurements

Every reading reports the steps taken. The sorted list of 9.15 reads in 1.3k steps. The library plus the paper's terms check in 1.5M steps.
