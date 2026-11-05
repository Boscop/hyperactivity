# scoped-machine

A single normalizer for cubical type theory. Every term is compiled to a graph of cells and reduced
by local rewrites. Programs, proofs, the type checker, data structures and the rules for transport
all run on that one mechanism.

```
make test       # all suites; the same output with 0, 1, 2, 4, 8, 16 workers; again under ASan/UBSan
make library    # tests/boxes.cub, tests/sort.cub, tests/memo.cub
./build/cubical lib/prelude.cub lib/code.cub lib/check.cub lib/std.cub FILE.cub
```

Code: `src/net.c` (cells, rewrite rules, parallel scheduler), `src/dnf.c` (intervals, faces,
substitutions), `src/big.c` (exact naturals), `src/lang.c` (core forms, compiler to cells,
readback, top-level forms, modules), `src/syntax.c` (the surface language: layout, parsing,
elaboration, clause compilation, printing). Language files: `lib/prelude.cub` (core data types,
composition clauses), `lib/code.cub` (codes, `View`), `lib/check.cub` (the type checker),
`lib/std.cub` (checked library), `lib/sort.cub` (9.15 over any ordered type; `tests/sort.cub`
instantiates and runs it). Coinduction: §7, `tests/codata.cub`; guarded recursion: §8,
`tests/guarded.cub`. Earlier plans and specs: `archive/`.

## 0. The language

```
data List (A : Type) = nil | cons (x : A) (xs : List A)
data S¹ = base | loop : base ≡ base

length : {A : Type} → List A → Nat
length nil = 0
length (cons x xs) = suc (length xs)

twist : S¹ → S¹
twist base = base
twist (loop i) = loop (~ i)

plus-zero : (n : Nat) → plus n zero ≡ n
plus-zero zero = λ i → zero
plus-zero (suc k) = λ i → suc (plus-zero k i)

#same length (cons 1 (cons 2 nil)) = 2
```

Application is juxtaposition; structure is indentation (a block opens after `where`, `of`, `let`,
`mutual`, `postulate`; `;` separates items on one line). `(x : A) → B`, `{x : A} → B` (implicit),
`(x : A) × B`, `A → B`, `A × B`, `λ x y → e`, `(a , b)`, `let x = e in b`, `if b then t else e`,
`case e of c x → t; d → u`, `case e return P of ...`, `e where` a block of definitions, `Type`,
`Type l` (`Level`, `lzero`, `lsuc`, `lmax`, or a numeral). Numerals are natural numbers. Every
Unicode symbol has an ASCII spelling: `->` `\` `|->` `==` `/\` `\/` `**` `forall`, `Nat` for `ℕ`,
`S1` for `S¹`, `Empty` for `⊥`, `Unit` for `⊤`. Operators are declared: `infix 4 ≡`, `infixr 5 ∙`,
and defined as `_≡_` or by an equation `x ≡ y = ...`. A name used in a signature and bound nowhere
is an implicit parameter (a type, or a level when it stands as a universe's level).

**Paths are equality.** `a ≡ b` is the type of paths between `a` and `b` (in a type found from
them; `Path A a b` names the type), `a ≡[ i ↦ A ] b` a path over a line of types. A path is
`λ i → e`, applied as `p i`; the interval has `0`, `1`, `~ r`, `r ∧ s`, `r ∨ s`. A partial value is
`[ i = 0 ↦ a , i = 1 ↦ b ]`, composition `comp (λ i → A) (λ j → [ ... ]) a0`, transport
`transp (λ i → A) a`. Whether a λ or an application is a path's or a function's is decided by the
checker (below), so it is written the same way. `lib/std.cub` has `refl`, `sym`, `_∙_`, `cong`,
`subst`, `transport`, `funExt`, equivalences and `ua`, with `calc` chains:
`calc a ≡ b by p ≡ c by q` (each step on its own indented line if long), composed by `_∙_`.

**Declarations.** A data type lists its constructors by fields (`cons (x : A) (xs : List A)`) or by
type (`cons : A → List A → List A`); a constructor whose type ends in `a ≡ b` is a path
constructor of a higher inductive type. A definition is a signature and equations; its equations
may match constructors at any depth, path constructors with their dimensions (`twist (loop i)`),
and implicit arguments as `{p}`. A codata type lists its fields (`codata Stream A where head : A;
tail : Stream A`), and a value of it is defined by copatterns: `(smap f s).head = f s.head`.
`record R A = { f : A ; g : B }` is a Σ with named projections, and `record { f = a ; g = b }` or
`(a , b)` a value of it; `x.f` projects. `mutual` groups definitions that call each other;
`postulate x y` declares names without values.

**Recursion.** A call on a field of the matched argument is structural; a definition at a type with
a clock recurses through any functions where a later value is expected (§8). A call on a field's field,
at any depth (`fib (suc (suc n)) = plus (fib n) (fib (suc n))`), is course-of-values: the split
computes each value together with the values below it (a Σ of the value and `Below v`, `Below`
itself an elimination into types), and a split of a variable below reverts that pair, so that in
its branches `Below` unfolds to the pairs of its own recursive fields. Any other terminating
recursion is written with a measure into `Nat` and, where the obligation does not compute to
`Unit`, a proof of it:

```
zig : (a b : Nat) → Nat
zig zero b = b
zig (suc a) b = suc (zig b a)
decreasing by plus a b using <-suc-eq (plus b a) (plus a b) (plus-comm b a)
```

Each call `f as` must satisfy `m as < m ps` (`_<_` of `lib/std.cub`, which computes to `Unit` or
`Empty` on numerals); the `using` term is checked at that type in the scope of the call's clause.
The definition becomes recursion on a bound of the measure, whose hypothesis is reverted through
the clauses' matches. A proof about such a definition does not unfold it by its equations
definitionally (the recursion's bound is part of the term), only by computation on values.

**Elaboration.** An implicit argument left out is a hole. Holes are solved first syntactically:
from the arguments' types as their heads' signatures give them, from the type an application is
expected to have (a λ's binder from the Π expected, a branch's from the motive at its constructor,
a definition's body from its signature), and from a solved argument's own type. What remains is
solved by the checker, which is itself a term (`lib/check.cub`): it checks the elaborated term
with its holes and its guesses (a λ or an application of unknown kind, function or path) and
answers with solutions or a request to redo; an undetermined level is `lzero`. A case written
where a type is expected takes that type as its motive, so its stuck values read back with their
type. `{_}` asks for a hole where an argument must be given positionally.

**Commands.** `#check e : T`, `#same a = b` (equal readings, at a type when either side has one),
`#show e`, `#observe n e` (a codata value unfolded n levels), `#input x` and `#supply x = e` (§6),
`#fail` before a declaration or command (it must be rejected). `import "file.cub"` loads a module
once. Results and failures print in the surface syntax. The core forms the language compiles to
(`(lam x b)`, `(elim P e ((c x) b))`, …) are still read from files of other extensions, and
`--print FILE` prints such a file in the surface syntax.

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
dimensions), partial values (systems), composition, Glue types and their elements, compositions
of types and their elements, and higher inductive types (data types with path constructors).

**Partial values.** `[ i = 0 ↦ a , i = 1 ↦ b ]` is a value defined only where its faces hold.
Each branch is kept unbuilt, as syntax with its environment, and is built with its face's
substitution applied (`a` with `i := 0`) only when an operation reads it. If a substitution makes a
face true (here `i := 0`), the whole partial value becomes that branch.

**Composition.** `comp A u a0` takes a line of types `A(i)`, a starting element `a0 : A(0)` and a
partial value `u` (where it is given, what the result must be along the line), and produces an
element of `A(1)` matching `u`. Transport is composition with nothing given. How it is computed
depends on what kind of type `A(i)` is; each case is a term in `lib/prelude.cub`, not C:
- functions (`%comp-pi`): to apply the result to `x : A(1)`, move `x` back to each `A(i)`, apply,
  and compose the results in the codomain;
- pairs (`%comp-sigma`): compose the first components, then the second components along the line
  the first one traced;
- paths (`%comp-path`): compose pointwise, adding the path's two endpoints to what is given;
- Glue (`%comp-glue`): this is where univalence computes, so transport along `ua e` applies `e`;
  on the end's faces the fibre over the composed base comes from the equivalence's contractibility;
- universes (`%comp-u`): a composition of types is a type of its own, `%hcompU A S` (Cubical Agda's
  `hcomp` in `Type`). Its faces `S` are the tube's lines of types from `A`, and the complement of
  the tube's face carries `A` itself, so where the tube is empty the composition is `A` and a
  closed composition of types is a type. Its elements are `glue` values; on a face, `%unglueU`
  transports back along that face's line. Composition through it (`%comp-gluel`) composes the
  unglued base and then, on the end's faces, takes the fibre of that transport over it from
  `%tproof`: the transport fillers along the line and one composition back along it. No
  equivalence and no contractibility proof is involved, so eliminating a higher inductive type
  into the universe and transporting through the result costs transports along the face lines,
  at every nesting depth.

A `glue` value's base is held unbuilt. Where the system is total (on the end's faces of a
composition through a Glue type or a composition of types, the usual case) the value is the face's
element and the base is never read; built, it is a further composition whose tube carries the
fibre paths, and those compose generically one level down, so each level of nesting would pay for
every level below it. Transport along the double cover of the circle over a composite of `n`
loops (`tests/codata.cub`, the composites defined one on another):

| loops | composed on the left | composed on the right |
|---|---|---|
| 4 | 4.2k steps | 5.1k |
| 8 | 9.1k | 12.0k |
| 16 | 19.0k | 27.0k |
| 64 | 78.4k | 146k |

With compositions of types encoded as Glue types over transported identity equivalences and the
base built (commit 04f761c and before) the same transports took 20.5k, 46.9k and 99.8k steps on
the left and 51.9k, 340k and 2.49M on the right at 4, 8 and 16 loops.

**Data types.** One mechanism serves every declared data type, with nothing written per type:
- moving a constructor value along a line of data types (e.g. `List A(i)`) moves each of its
  fields along its field type; this is derived from the declaration;
- a path constructor such as `loop i` must still meet its declared endpoints after moving, so the
  result is corrected by a composition that glues those endpoints back on;
- composition is generalized (Cubical Agda's `gcomp`): the tube gets the complement of its face,
  carrying the transported starting value. At every closed point the tube is then total, so a
  composition never leaves a formal composition behind where nothing was to be composed: a
  composition with an empty tube is transport, transport along a constant line of a closed type
  is the identity (`transp (λ i → S1) base` is `base`), and transport along `ua not` of `true` is
  `false`;
- in a data type without path constructors, a composition whose starting value and sides are
  all one constructor goes into the fields, each composed along its type at the earlier fields'
  fillers (CCHM's rule), so `λ j → comp Nat [ j = 0 ↦ 1 , j = 1 ↦ 1 ] 1` is `λ j → 1` and a path
  field transported along `ua` is a constant path again. The machine looks at the starting value
  and then at each side at a fresh dimension; a side that is not that constructor (a variable)
  keeps the composition formal, and a substitution decides again. The checker accepts a formal
  composition in such a type only when its tube covers every closed point, as the machine's own
  do, so a closed value of such a type is a constructor;
- a composition in a data type that cannot be computed further is kept as a value, `hcompD`; a case
  analysis meeting such a value applies itself to the pieces and composes the results in the
  result type.

## 3. Work is done once, and only where it is read

A shared subterm is held by a duplicating cell that copies lazily, one level at a time as it is
read. A stored value lives once, and readers copy its top cell. A substitution of dimensions is
recorded on the reference, not applied by copying. A subterm in a position that might not be read
is held unbuilt until an operation meets it: `fst (a , b)` never computes `b`, even if `b`
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
(`tests/memo.cub`): `fib (n+2)` demands `fib (n+1)`, which demands `fib n` while the outer `fib n`
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

A reading of a checked definition, or of `the T t`, is taken at its type: `quote` in
`lib/check.cub` reads the value as a code (`lib/code.cub`), applying a function to a fresh
variable of its domain, reading a pair by its projections and a path at a fresh dimension, and a
stuck elimination's branches at fresh fields typed by the constructor's telescope, at the motive.
So `λ p → p 0` at `Path Nat zero zero → Nat` reads `λ x0 → zero`, and a variable of a Σ type
reads `(fst x0 , snd x0)`. Typed readings give full normal forms, except inside an
elimination written without a motive that is the head of a stuck term (applied, projected, or
eliminated), whose type is not known there; reached at a known type, its branches are read at that
type. A stuck word elimination carries no motive, so its branches are read without types: the
type `T = P w` it is quoted at does not determine an arbitrary motive `P`, so reading them at `T`
would be unsound (a motiveless case can be read at `T` because its motive is constant). The
code satisfies two laws, checked in `tests/quote.cub`: evaluating it (`eval-g`) gives a value
convertible with the original, and quoting that value gives the same code. A failure report
prints its types (expected and actual, a type that was eliminated) the same way.

`#same` of two terms, either of them a checked definition or `the T t`, reads both at that
one type, so equal values compare equal however they are written. Other readings (`#show` and
`#same` of any other terms) are taken without a type: under a binder,
a variable's type is not known, so a path variable stays stuck at an endpoint and nothing is
η-expanded. `#same` compares two readings as text: on well-typed terms equal texts are equal
values; two readings can differ while their values are equal.

## 5. Reflection: `%view`

`(%view v)` turns the outermost form of a computed value into an ordinary data constructor in one
step, whose fields are the value's own parts: a Π type gives `(wpi A B)` with `A` the domain and
`B` the codomain as a function, a stuck application gives `(wapp f a T)`. A program cases on this
like any data, so a program in the language can examine a value's structure. What it sees is the
computed value, not the source text that produced it; a function shows only as `(wlam f)`.
`(%var T)` and `(%dim)` make fresh variables, `(%dnf r)` gives an interval's or a face's clauses
as lists of literals, and `(%motive f)` an elimination's motive. The type checker (`lib/check.cub`) is written with
these and runs on this runtime, over the values the runtime already computed for each subterm:
there is no second evaluator, and checking a definition is running that program on it.

## 6. Inputs

`#input x` declares a definition without a value. An operation that needs `x`'s value (applying
it, a case on it, a projection) is held at exactly that point. Everything that does not depend on
`x` reduces fully, and the parts of a result that do not need `x` are produced. `#supply x = t`
connects the value, and the held operations proceed. Unlike a variable, an input is not computed
with symbolically: what depends on it waits for it.

## 7. Coinduction

`codata Orbit (A : Type) where here : A; next : Orbit A` declares a coinductive record type by
its fields. A field's type may mention the parameters, the earlier fields, and the type itself
strictly positively (at any parameters not mentioning it, under Π codomains and Σ components);
the checker checks this. A field applied to a value is its projection: `here o`, or `o.here`.

A value of a codata type is made by a copattern definition, a definition whose body is λs and
lines over a record:

```
unfold : {A : Type} → (A → A) → A → Orbit A
(unfold f a).here = a
(unfold f a).next = unfold f (f a)
```

Such a definition may call itself, and the definitions of a `mutual` group may call each
other. Its value is its stuck call: a neutral of its declared type, holding the definition and its
arguments. A projection unfolds the call once (the body applied to the same arguments, at the same
dimensions) and takes the field. Nothing else unfolds it: an application, a comparison, a reading,
and a path application at 0 or 1, which reads the endpoint from the declared type as for any
neutral path. So a process that never ends is a finite value, an observation of depth n costs n
unfoldings, and a definition that is its own tail (`zeros`, whose `next` is `zeros`) is held as
its call. A record stands only as the body of a copattern definition.

The checker (`chk-codef` in `lib/check.cub`) requires every call of the group in a field's term to
be guarded: it is the head of the field's term (applied to arguments not mentioning the group, at
any dimensions), or stands there under a λ, a line, a pair, a constructor, a case or system
branch, or a let's body; never as an argument or eliminated. Every body of a group is found guarded
before any is checked, since checking one can unfold another. Each body is then checked at its
type, the group's calls at their declared types. A guarded call is productive, so the value exists at
every depth. At a path type the boundary is checked field by field (a record is compared with a
value by its fields), so it holds at every depth. Two stuck calls are convertible when their
definitions and arguments are; there is no η for codata, so two definitions whose unfoldings are
bisimilar are not convertible. Their equality is a path, and a path between orbits is a
bisimulation: `tests/codata.cub` proves `(x ≡ y) ≃ (x ≈ y)` by four corecursive definitions
(`punaragamana/Orbit.agda`), and proves `mapO id o ≡ o` by a corecursive bisimulation.

Composition in a codata type is a copattern definition generated from the declaration (`%comp-D`):
each field composed along its type, at the parameters' lines and each earlier field's filler. A
transported process is again a process, unfolded field by field when it is observed.

`#show t` reads a stuck call as the call. `#observe n t` reads it unfolded n levels deep, as
records. An unfolding is named by the call it unfolds, so two unfoldings of one call pending at
once share the body (§3).

## 8. Guarded recursion

```
nats : Stream κ Nat
nats = 0 ∷ smap suc nats

fibs : Stream κ Nat
fibs = 0 ∷ 1 ∷ zipWith plus fibs (stail fibs)

#same nth 10 (λ κ → fibs {κ}) = 55
```

`▸ κ A` is an `A` one tick of the clock `κ` later: the Π type `(α : Tick κ) → A`. A tick is bound
by λ and given by application, and nothing else: `Clock` and `Tick κ` are the domains of Π types
only, in no universe, so a tick cannot be stored, returned, or let-bound. The checker's
timelessness rule makes "later" mean later: in the body of a tick λ, a function applied to the
tick α may use no variable bound at or after α. So `λ α → x α α` is rejected (one tick spent
twice) and `λ α β → x α β` accepted. Interval variables are exempt: `λ α (i : I) → p i α` turns a
path of later values into a later path, and `λ (i : I) α → p α i` turns it back. This is sound
because the interval does not vary in time: the "earlier" of a context extended by an interval
is the earlier context extended by the interval, ◁(Γ × I) ≅ ◁Γ × I, as in the cubical models of
guarded type theory.

`dfix : (▸ κ A → A) → ▸ κ A` (at every level: `next`, `⊛`, `dfix`, `fix`, `pfix` and `forceκ`
take `{l : Level}` first) is a stuck call of the kind copattern definitions make (§7), with one
difference: it unfolds, `dfix f ⋄ = f (dfix f)`, only when applied to the tick constant `⋄`, and at
a tick variable it stays stuck. `fix f = f (dfix f)` is then the guarded fixed point, and every
recursion whose recursive use is under `▸` is well typed, whatever functions it passes through.
`⋄` is not written in a checked term; it is the computation of `forceκ : ((κ : Clock) → ▸ κ (A κ))
→ (κ : Clock) → A κ`, a later value at every clock being a value now at every clock (the clock is
fresh, so its tick constant may be given). A total stream is a stream at every clock,
`Str A = (κ : Clock) → Stream κ A`, observed through `forceκ` (`tl`, `nth`); `κ0` is the closed
clock. `pfix : dfix f ≡ next (fix f)` is the unfolding at a tick variable. It is a stuck call
too: at `⋄` both its ends are `fix f`, and it computes there, `pfix f i ⋄ = fix f` for every `i`
(the tick-unfolding definitions count path parameters among theirs). So transport along `pfix`
computes once `forceκ` gives the tick, and a guarded recursive type, a fixed point in the universe
such as `G κ = fix (λ X → Nat × ((α : Tick κ) → X α))`, can be unfolded and folded by transport
along `pfix` and still read canonical numerals (`tests/guarded.cub`). `pfix` being a path rather
than a judgmental equality is a property of the theory (`dfix f` at a tick variable must not
unfold, or normalization fails); the path computes wherever the equality would.
Normalization terminates because the only unfolding fires on `⋄`, which only `forceκ` gives.

Guarded types are ordinary data whose recursive fields are later:
`data Stream (κ : Clock) A = _∷_ (head : A) (tail : ▸ κ (Stream κ A))`. Such a field is not a
recursive field (an elimination takes no hypothesis for it). Composition in `▸ κ A` is pointwise
(the composition of ticks and clocks is the identity), so transport along a line of stream types
moves each tail at its own tick, and the heads read are numerals again (composition with an empty
tube in a data type without path constructors goes into the fields, §2).

The elaborator writes the ticks. A clock named in a signature (an argument of `Tick` or of a
data type's `Clock` parameter) is an implicit parameter. A definition with a clock whose
recursion is not structural is the guarded fixed point over its explicit parameters after the
clock. A term given where `▸ κ X` is expected and not itself later is a λ over a fresh tick, inside
which a later subterm standing where a present value is due is applied to that tick, and a
recursive call is the fixed point's self at the outermost such tick. A tick λ written by hand
(`1 ∷ λ α → ones`) is treated the same way, and so is the later component of a pair. A clock
given to a defined family (`G κ` with `G : Clock → Type`) is bound as well. A clock argument nothing
determines is the one clock in scope. A recursive call needed now is reported where it stands:
`bad = smap suc bad` is rejected with "a recursive call needed now, not under ▸".

## Measurements

Every reading reports the steps taken. The sorted list of 9.15 reads in 1.6k steps (eight words).
The library (`lib/std.cub`, elaborated and checked) loads in 1.4M steps, and with the paper's terms
(`tests/boxes.cub`) in 3.1M; elaboration runs the checker again when it has solved holes, and a
case given a motive from its expected type carries it at run time. Naive `fib` of a computed 20
reads in 36k steps, of 25 in 278k, and of 20 written out in 36k; the iterative `fib` reads 21k and
226k (`tests/memo.cub`; without the memo, naive `fib` takes 294k and 3.8M). `CUBICAL_KEYS=1` traces
the keys registered, fired and hit on stderr, and the entries left at the end. The development of
`Orbit.agda` (the codata types, `unfold`, `mapO`, the four corecursive maps between paths and
bisimulations, and `(x ≡ y) ≃ (x ≈ y)`) checks in 60k steps, `iso1` (a corecursive square) in 13k.

Brunerie's number, by an untyped port of Ljungström's `brunerie'` from agda/cubical
(`experiments/brunerie/`, about 1 s in Agda), reads `negsuc (suc zero)`, that is -2, in 1.26M
steps (about 3 s with the library); `make test` checks it with one and four workers. The pipeline
eliminates the circle and S² into truncations, the Hopf fibration and `Code` into the universe,
and transports through the proofs that the types of sets and of groupoids have the next h-level.
Before compositions of types were a type former and the glue base was held unbuilt it ran out of
7 GB of memory.
