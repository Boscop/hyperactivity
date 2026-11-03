# scoped-machine

A single normalizer for cubical type theory. Every term is compiled to a graph of cells and reduced
by local rewrites. Programs, proofs, the type checker, data structures and the rules for transport
all run on that one mechanism.

```
make test       # all suites; the same output with 0, 1, 2, 4, 8, 16 workers; again under ASan/UBSan
make library    # tests/boxes.ctt, tests/sort.ctt, tests/memo.ctt
./build/cubical lib/prelude.ctt lib/code.ctt lib/check.ctt lib/std.ctt FILE.ctt
```

Code: `src/net.c` (cells, rewrite rules, parallel scheduler), `src/dnf.c` (intervals, faces,
substitutions), `src/big.c` (exact naturals), `src/lang.c` (reader, compiler to cells, readback,
top-level forms, modules). Language files: `lib/prelude.ctt` (core data types, composition
clauses), `lib/code.ctt` (codes, `View`), `lib/check.ctt` (the type checker), `lib/std.ctt`
(checked library), `lib/sort.ctt` (9.15 over any ordered type; `tests/sort.ctt` instantiates and
runs it). Coinduction: §7, `tests/codata.ctt`. Earlier plans and specs: `archive/`.

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

Equal terms reached by different routes are also computed once. Every cell that will produce a
value (a thunk, a saturated call, an application of a known λ, a case) carries a key: its compiled
body over the keys of the values bound in it, interned, so one term is one pointer. A value's key
is its constructor over its parts' keys; a held value (a box) is named by what was put in it, and a
copy out of a box, a definition unfolded, and the scrutinee a case branch rebuilds from the fields
it took all carry that name, so one value has one name by whatever route it is reached. A producer
is pending on its key from its creation; when one fires while another of the same key is pending,
its value goes into a box held by the entry, and the other reads the box instead of computing
(waiting on the wire, uncounted, if the value has not arrived). Retention is reachability: an
entry lives exactly while a producer is pending or a reader holds the box; the table holds nothing
on its own, and two boxes with equal values but different origins are not identified (a numeral
computed by `add` and the same numeral written out are two values). Naive Fibonacci is the test
(`tests/memo.ctt`): `fib (n+2)` demands `fib (n+1)`, which demands `fib n` while the outer `fib n`
is pending, so every `fib k` is computed once and the work is linear in the calls (unary addition
dominates the count), whether the argument is a numeral the machine computed or one written out.

A reading reports its steps, and with them the memo hits (values found instead of computed) and
the values shared. The steps are the work this reading did, given what earlier readings left held
and the order the rewrites ran in: a term's second reading can cost less than its first, and with
workers a reading can find a value shared that it would have computed alone. Without the memo
(`CUBICAL_NOMEMO=1`) the count is a property of the term; `make test` checks that it does not
depend on the number of workers, and that with the memo the answers do not.

## 4. Open terms normalize, with types

A stuck computation is one with no rule to fire because the thing it inspects is unknown: `f x`
with `f` a variable, `case n ...` with `n` a variable, `p @ i` with `p` a path variable and `i` a
dimension variable. It is kept as a value (a neutral) holding its parts; reduction continues
everywhere else, including inside the branches of a stuck case. When a substitution later
supplies the unknown, it fires. A neutral's type is computed from the variable at its head along
the operations applied to it, and a stuck path's endpoints are read from that type.

A reading of a checked definition, or of `(the T t)`, is taken at its type: `quote` in
`lib/check.ctt` reads the value as a code (`lib/code.ctt`), applying a function to a fresh
variable of its domain, reading a pair by its projections and a path at a fresh dimension, and a
stuck elimination's branches at fresh fields typed by the constructor's telescope, at the motive.
So `(lam p (@ p 0))` at `Π p : Path Nat zero zero. Nat` reads `(lam x0 zero)`, and a variable of
a Σ type reads `(pair (fst x0) (snd x0))`. Typed readings give full normal forms, except inside an
elimination written without a motive that is the head of a stuck term (applied, projected, or
eliminated), whose type is not known there; reached at a known type, its branches are read at that
type. A stuck word elimination carries no motive, so its branches are read without types: the
type `T = P w` it is quoted at does not determine an arbitrary motive `P`, so reading them at `T`
would be unsound (a motiveless case can be read at `T` because its motive is constant). The
code satisfies two laws, checked in `tests/quote.ctt`: evaluating it (`eval-g`) gives a value
convertible with the original, and quoting that value gives the same code. A failure report
prints its types (expected and actual, a type that was eliminated) the same way.

`same` of two terms, either of them a checked definition or `(the T t)`, reads both at that
one type, so equal values compare equal however they are written. Other readings (`show` and
`same` of any other terms) are taken without a type: under a binder,
a variable's type is not known, so a path variable stays stuck at an endpoint and nothing is
η-expanded. `same` compares two readings as text: on well-typed terms equal texts are equal
values; two readings can differ while their values are equal.

## 5. Reflection: `%view`

`(%view v)` turns the outermost form of a computed value into an ordinary data constructor in one
step, whose fields are the value's own parts: a Π type gives `(wpi A B)` with `A` the domain and
`B` the codomain as a function, a stuck application gives `(wapp f a T)`. A program cases on this
like any data, so a program in the language can examine a value's structure. What it sees is the
computed value, not the source text that produced it; a function shows only as `(wlam f)`.
`(%var T)` and `(%dim)` make fresh variables, `(%dnf r)` gives an interval's or a face's clauses
as lists of literals, and `(%motive f)` an elimination's motive. The type checker (`lib/check.ctt`) is written with
these and runs on this runtime, over the values the runtime already computed for each subterm:
there is no second evaluator, and checking a definition is running that program on it.

## 6. Inputs

`(input x)` declares a definition without a value. An operation that needs `x`'s value (applying
it, a case on it, a projection) is held at exactly that point. Everything that does not depend on
`x` reduces fully, and the parts of a result that do not need `x` are produced. `(supply x t)`
connects the value, and the held operations proceed. Unlike a variable, an input is not computed
with symbolically: what depends on it waits for it.

## 7. Coinduction

`(codata (Orbit (A U)) (here A) (next (Orbit A)))` declares a coinductive record type by its
fields. A field's type may mention the parameters, the earlier fields, and the type itself
strictly positively (at any parameters not mentioning it, under Π codomains and Σ components);
`checked-codata` checks this. A field applied to a value is its projection: `(here o)`.

A value of a codata type is made by a copattern definition, a definition whose body is λs and
lines over a record:

```
(checked unfold (Pi ((A U) (f (Pi a A A)) (a A)) (Orbit A))
  (lam A f a (record (here a) (next (unfold A f (f a))))))
```

Such a definition may call itself, and the definitions of a `(mutual ...)` group may call each
other. Its value is its stuck call: a neutral of its declared type, holding the definition and its
arguments. A projection unfolds the call once (the body applied to the same arguments, at the same
dimensions) and takes the field. Nothing else unfolds it: an application, a comparison, a reading,
and a path application at 0 or 1, which reads the endpoint from the declared type as for any
neutral path. So a process that never ends is a finite value, an observation of depth n costs n
unfoldings, and a definition that is its own tail (`zeros`, whose `next` is `zeros`) is held as
its call. A record stands only as the body of a copattern definition.

The checker (`chk-codef` in `lib/check.ctt`) requires every call of the group in a field's term to
be guarded: it is the head of the field's term (applied to arguments not mentioning the group, at
any dimensions), or stands there under a λ, a line, a pair, a constructor, a case or system
branch, or a let's body; never as an argument or eliminated. Every body of a group is found guarded
before any is checked, since checking one can unfold another. Each body is then checked at its
type, the group's calls at their declared types. A guarded call is productive, so the value exists at
every depth. At a path type the boundary is checked field by field (a record is compared with a
value by its fields), so it holds at every depth. Two stuck calls are convertible when their
definitions and arguments are; there is no η for codata, so two definitions whose unfoldings are
bisimilar are not convertible. Their equality is a path, and a path between orbits is a
bisimulation: `tests/codata.ctt` proves `(x ≡ y) ≃ (x ≈ y)` by four corecursive definitions
(`punaragamana/Orbit.agda`), and proves `mapO id o ≡ o` by a corecursive bisimulation.

Composition in a codata type is a copattern definition generated from the declaration (`%comp-D`):
each field composed along its type, at the parameters' lines and each earlier field's filler. A
transported process is again a process, unfolded field by field when it is observed.

`(show t)` reads a stuck call as the call. `(observe n t)` reads it unfolded n levels deep, as
records. An unfolding is named by the call it unfolds, so two unfoldings of one call pending at
once share the body (§3).

## Measurements

Every reading reports the steps taken. The sorted list of 9.15 reads in 1.2k steps (eight words). The library plus the paper's terms check in 1.5M steps. Naive `fib` of a computed 20 reads in 36k steps, of 25 in 278k, and of 20 written out in 36k; the iterative `fib` reads 21k and 226k (`tests/memo.ctt`; without the memo, naive `fib` takes 294k and 3.8M). `CUBICAL_KEYS=1` traces the keys registered, fired and hit on stderr, and the entries left at the end. The development of `Orbit.agda` (the codata types, `unfold`, `mapO`, the four corecursive maps between paths and bisimulations, and `(x ≡ y) ≃ (x ≈ y)`) checks in 66k steps, `iso1` (a corecursive square) in 11k.
