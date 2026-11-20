# Hyperactivity

A decentralized safe superintelligent operating system language.

Knowledge graphs as interaction networks. Mathematical knowledge graphs as cubical interaction networks.

## The whole picture

There is no primitive thing, only a primitive law of interaction. A term is a net of cells, and the only
thing that ever happens is that two cells meet and are replaced.

### The pieces

- **Cell.** A node with one *principal port* and some other ports.
- **Value.** A cell that offers itself at its principal port: a λ, a pair, a type, a constructor, an
  interval point, a system.
- **Operation.** A cell that wants one value at its principal port: apply, first, second, path-apply,
  compose, case, substitute, duplicate, erase.
- **Wire.** Connects two ports. A wire can be empty for a while: whoever arrives first leaves what it has,
  the second finds it.
- **Active pair.** An operation's principal port wired to a value's principal port. This is the only place
  anything happens.
- **Step.** The two cells of an active pair are removed and replaced by a small net on the wires they were
  holding. Nothing else is looked at.

Two values wired together, or two operations wired together, do nothing.

```
   arg ──┐
         app ●────● λ ── body      ⟶      out ── body (with arg wired in where the parameter was)
   out ──┘
```

### The rules

Read each line as: *operation* meets *value* ⟶ what replaces them.

Functions and pairs

```
apply(arg)      meets  λ             ⟶  the body, with arg wired in for the parameter
first           meets  pair(a, b)    ⟶  a          (b is erased)
second          meets  pair(a, b)    ⟶  b          (a is erased)
force           meets  delayed body  ⟶  the body
```

Paths

```
path-apply(r)   meets  path-λ i. t   ⟶  substitute [i := r] facing t
path-apply(0|1) with the endpoints known  ⟶  that endpoint, the path itself never read
path-apply(0|1) meets  a blocked path     ⟶  the endpoint, read from the path's type
```

Substitution of a dimension (a cell too; it moves one level at a time)

```
subst  meets  interval or face          ⟶  the substituted interval or face
subst  meets  λ, pair, Π, Σ, path type  ⟶  the same cell, a subst facing each part
subst  meets  path-λ j. t               ⟶  path-λ j. (subst facing t), j renamed if it would clash
subst  meets  system                    ⟶  the branch whose condition became true; else the system
                                            with substituted conditions
subst  meets  a held value              ⟶  stays pending on the reference until someone reads
```

Sharing

```
duplicate  meets  value with parts   ⟶  the value is held once; each side gets a reference
duplicate  meets  value, no parts    ⟶  two copies
erase      meets  value              ⟶  gone; an erase faces each part
anything   meets  reference          ⟶  the top cell of the held value is copied out; each part with
                                         parts of its own becomes a reference
a read that arrives before the value exists waits. Waiting is not a step.
```

Blocked: nothing to react with yet

```
apply / first / second / path-apply / compose / case
           meets  a variable or another blocked thing  ⟶  a neutral cell holding the operands
subst      meets  neutral            ⟶  the original operation again, on substituted parts: it retries
```

Systems (a value defined only where its conditions hold)

```
any eliminator  meets  system    ⟶  a system of that eliminator on each branch, each done only when read
add-branch(φ, t) with φ true     ⟶  t
add-branch(φ, t) with φ false    ⟶  the rest; t is dropped
```

Composition (filling: given a line of types, a partial tube and a base, produce the far end)

```
compose  meets  a tube that covers everything  ⟶  the tube's end
compose  with a partial tube: look at the head of the type line
   head Π           ⟶  the Π clause
   head Σ           ⟶  the Σ clause
   head path type   ⟶  the path clause
   head universe    ⟶  the universe clause
   head Glue        ⟶  the Glue clause
   head data type   ⟶  that type's own clause, generated from its declaration
   head neutral     ⟶  a neutral composition
```

Each clause is itself a term of the language, not machine code.

Glue (what makes an equivalence a path)

```
Glue type, condition true     ⟶  the type on that face
glued value, condition true   ⟶  the value on that face
unglue on the face            ⟶  the equivalence's map applied
unglue  meets  glued value    ⟶  its base
```

Data types

```
constructor with a dimension at 0 or 1   ⟶  its declared boundary
compose in a data type, partial tube     ⟶  a formal composition: itself a value of the type
transport  meets  constructor            ⟶  that constructor's clause
transport  where the type line is constant  ⟶  the identity
case  meets  constructor c(fields)       ⟶  branch c built with the fields; each recursive field's
                                             hypothesis is the same case on that field, not yet run
case  meets  formal composition          ⟶  a composition in the motive
```

### How it runs

Running a term is rewriting active pairs until none is left facing the place being looked at. Steps in
different places do not touch each other, so they run at the same time on different workers. A value is
computed only as far as someone reads it.

### How the foundation shows up

- *No primitive thing:* a cell has no content except which rules fire when something meets it.
- *Open boundary:* a free port. A neutral is a cell waiting on one.
- *Every step preserves meaning:* each line above is one of the calculus's equations, read left to right.
- *Identity is a path, and transport computes:* `ua` is a Glue type; transport along it runs the
  composition rules and ends at "unglue on the face ⟶ the equivalence's map applied".

### Where this comes from

These are the rules of the running machine, read line by line on 2026-11-20 from its core (the cell
vocabulary in `core.h`, the rules in `net.c`). Three groups of the machine's operations are not in this
list: arithmetic on words, machine-level values and bytes, and the operations that let the type checker,
itself a term, inspect values.

## Setup

Requires a C compiler (gcc or clang) and make.

```
git clone https://github.com/avikj/hyperactivity.git
cd hyperactivity
make
./hyperactive
```

`make` builds the machine (`build/cubical`); `./hyperactive` is the machine run as the loop. The code
and the documents are not files: they are the source, `store/`, read and written only through
`./hyperactive`. From there, `hyperactive` teaches you the rest.
