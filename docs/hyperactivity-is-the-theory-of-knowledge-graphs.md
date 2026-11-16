# Hyperactivity is the theory of knowledge graphs

Avik, 2026-11-16 05:21 UTC (session 01WfrbRtggXC4vgyB2z9C6c2), as written:

> we have the actual SoTA knowledge graph technology on the planet. that's a very irrefutable claim and should be much less controversial than a claim the foundation is sufficient for systems much better than LLM with a little engineering. if we just consider knowledge graph - hyperactivity is the first universal theory of knowledge graph & computational foundation of knowledge

> now this is something worth elucidating. I haven't used the term knowledge graph in so many fucking years. but now I realize I solved it.

## What a knowledge graph is today

RDF and OWL, property graphs, Wikidata, and the industrial graphs of search and social companies store labelled
edges between identifiers. An edge asserts a relation; it does not carry the evidence establishing it. Two
descriptions of one thing are identified by fiat (`owl:sameAs`), with no account of what transfers across the
identification. Computation over the graph happens outside it, in queries and pipelines whose results are not
part of the graph. Provenance is metadata attached after the fact, and nothing determines which derived
conclusions become stale when a fact changes.

## What each becomes in hyperactivity

Citations are to the paper, the value `hyperactivity` (docs/hyperactivity.cub), Part I.

- A node is code with a type: what kind of thing it is is checked, not labelled.
- An edge is a construction: a relation together with the rationale establishing it (§1 "The universe and its
  descriptions", "Routines, relations, and rationales", equation 1.5, q : Π_{a:A} P(a, f(a))).
- Identity is computational: an equivalence of representations is a path, and transport carries every dependent
  operation, property and proof along it (§4 "Equivalence, univalence, and structure").
- What evidence supports is exact: a construction over every history compatible with what was observed, not a
  confidence (§1, equation 1.3, Warrants(y, P) := Π over Fib_o(y) of P(h)).
- Derived knowledge is a maintained view, a proof about an update (§10.2 "A maintained view is a proof about an
  update"), and dependency certificates determine exactly what is recomputed when a fact changes, absence
  included (§10.5 "Dependency certificates and persistent evaluation"): nothing is stale.
- The graph's computation, including the language it is written in, is inside the graph (§6.3 "Reflection
  applied to a language definition itself").
- An utterance is an event whose meaning is a relation between participants and their histories, not a payload
  (§1 "Language, history, and inherited knowledge", equation 1.4, Receive).

## Shown on 2026-11-15

Organizing about 50 knowledge-work targets and what they rest on needed no new primitive: the task graph
(lib/task-graph.cub), documents by content (lib/document.cub) and projections of one value (lib/paper.cub)
already were the system, and the workflow graph and the knowledge graph turned out to be one graph (Avik: "workflow
dag and knowledge dag are totally indivisible as soon as I start noting knowledge work intentions in the
workflow"). A theory of knowledge that is also the theory of the work and reasoning that change it is the
universal case.
