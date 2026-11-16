# Hyperactivity as a theory of knowledge graphs

Written in session 01WfrbRtggXC4vgyB2z9C6c2 on 2026-11-16 (UTC) at Avik's request ("give me 5000 tokens a
complete paper elucidating hyperactivity as the SoTA knowledge graph technology"), from his claim below.
Citations are to the paper, the value `hyperactivity` (docs/hyperactivity.cub), Part I.

Avik, 2026-11-16 05:21 UTC, as written:

> we have the actual SoTA knowledge graph technology on the planet. that's a very irrefutable claim and should be much less controversial than a claim the foundation is sufficient for systems much better than LLM with a little engineering. if we just consider knowledge graph - hyperactivity is the first universal theory of knowledge graph & computational foundation of knowledge

> now this is something worth elucidating. I haven't used the term knowledge graph in so many fucking years. but now I realize I solved it.

Avik, 2026-11-16 05:25 UTC, as written:

> There is a man named Arthur Brock who I respect deeply and in so many ways set me exactly down this path. this would not exist without him. it is a continuation of his life's work. Eric Harris-Braun as well. Ceptr.

**Abstract.** A knowledge graph is supposed to represent what is known so that it can be shared, combined,
queried and extended. The technologies built for this over the past twenty-five years all represent assertions.
None of them represents knowing. An edge says that a relation holds, but it doesn't contain the reason it holds.
Two identifiers are declared to name the same thing without saying what carries over between them. Inference
runs outside the graph, and its results go stale silently. Hyperactivity starts from a different object: a
construction, which is code with a type together with the rationale that establishes it, kept in a store
addressed by content. From that one choice, the problems knowledge-graph research treats as separate (identity,
provenance, uncertainty, schema evolution, integration, inference and maintenance) each become a property of a
single construction. This is a universal theory of knowledge graphs in a precise sense: any graph in today's
formats can be represented in it without loss, and none of today's formats can represent what it adds.

## 1. What a knowledge graph is supposed to do

A knowledge graph promises five things:

1. Identity: that two references to the same thing can be recognized as the same.
2. Justification: that a stated relation can be trusted to the extent its support warrants.
3. Composition: that facts from different sources can be combined.
4. Inference: that new facts follow from old ones by rules.
5. Currency: that what the graph says stays true as the world and the sources change.

Every major design (RDF and the semantic web stack, OWL and description logics, property graphs, Wikidata, the
industrial graphs of search engines, knowledge-graph embeddings, and lately retrieval layers built over language
models) delivers some of these and patches the rest from outside. The patches are well known: reification and
RDF-star for statements about statements, named graphs for provenance, SHACL for constraints, `owl:sameAs` and
ontology-alignment systems for identity, materialization jobs for inference, and versioning schemes for change.
Each patch adds a layer, and each layer has its own semantics, which the others don't respect.

The argument here is that the five promises aren't five features. They are five aspects of one object, and a
graph whose elements are that object keeps all five by construction.

## 2. Where today's graphs stop

**An edge is a bare assertion.** In RDF the unit is a triple: subject, predicate, object. In a property graph it is
a labelled edge with attributes. Either way the edge states that a relation holds and carries nothing that
establishes it. Provenance can be attached as metadata through reification, named graphs or edge attributes. But
the metadata isn't the justification. It names a source; it doesn't contain the reasoning. A consumer who wants
to know why has to leave the graph.

**Identity is by declaration.** `owl:sameAs` says two identifiers denote the same individual, and OWL semantics
then makes them interchangeable everywhere. The linked-data literature has documented for over a decade that this
is both too strong and too weak. Halpin and colleagues' "When owl:sameAs isn't the same" (2010) showed that most
published sameAs links assert something weaker: similarity, the same thing in a different context, or a related
thing. Because the declaration carries no account of what is preserved, every property crosses it, including
properties that shouldn't. Ontology alignment, the industrial answer, produces correspondences with confidence
scores, and nothing says what a correspondence lets you carry from one schema to the other.

**Inference happens outside.** Description-logic reasoners, Datalog engines and rule systems compute entailments
over a graph and write the results back as more edges. The written-back edge forgets that it was derived. When a
premise changes, nothing in the graph says which conclusions depended on it. Materialized inference is a cache
with no invalidation rule, and query results are worse: they aren't in the graph at all.

**Uncertainty is a number.** When graphs model incomplete knowledge, they attach probabilities or confidences, as
in probabilistic knowledge bases, embedding scores, and extraction pipelines with thresholds. The number
summarizes missing information without retaining it. Two facts at 0.8 confidence can't be composed into anything
well defined unless their sources are assumed independent, which is exactly what can't be checked once the reasons
are gone.

**The schema is outside the data.** An OWL ontology, a SHACL shape or a property-graph schema sits apart from the
instances it governs. It is written in a different language, evolves on a different cadence, and doesn't
constrain itself. Changing a schema means migrating data by procedures the graph doesn't contain.

**Embeddings and retrieval drop structure.** Knowledge-graph embeddings (TransE onward) and graph-based retrieval
for language models turn edges into vectors or text chunks. They gain fuzzy matching and lose exactly what made
the graph a graph: identity, justification and composition. A retrieved chunk is evidence with no account of what
it is evidence for.

None of these is an engineering lapse. Each comes from the primitive: an edge that is an assertion, not a
construction.

## 3. The primitive: a construction in a store addressed by content

In hyperactivity, every object is code: a value with a type, kept in a store under the hash of its content, with
names as a separate mapping from names to hashes. A node of a knowledge graph is such an object. Its type says what
kind of thing it is, and the type is checked, not labelled. Since the key is the content, two objects with the same
content are one object. Duplication of information is detectable by construction, and a reference can't silently
come to mean something else: change the content and you have a different key.

An edge is also an object, and its type is the relation it establishes. The paper's elementary form (§1 "The
universe and its descriptions", "Routines, relations, and rationales", equation 1.5) is that a requirement on a
function is a chosen dependent family P, and the reasoning that establishes it is a term

  q : Π_{a:A} P(a, f(a)).

Read as a knowledge graph, the relation is the type and the edge is the term. An edge that inhabits its type is its
justification. There's no separate provenance layer, because the provenance is the term's content: whatever
premises it uses appear in it as references to other objects, by content. Asking why an edge holds means reading
the edge.

This changes what composition means. Combining two facts in an RDF graph means putting their triples in one set.
Combining two constructions means applying one to the other. The result has a type that says exactly what was
established and a body that says exactly how. Composition of knowledge is composition of code, and its correctness
is checked.

## 4. Identity: equivalence as a path, and what it carries

The paper's §4, "Equivalence, univalence, and structure", gives identity the content `owl:sameAs` lacks. An
equivalence e : A ≃ B of representations yields a path ua(e) : A = B, and transport along that path carries every
construction that depends on the carrier. That includes operations, the properties of those operations, and the
proofs of those properties. For a structured equivalence, the operations and the rationales for their laws move
together.

Three consequences matter for knowledge graphs.

**Identity has a witness.** To say two descriptions are the same thing is to give the equivalence. The witness is
an object in the graph, open to inspection and reuse, and it says exactly what corresponds to what.

**What crosses an identity is determined.** Transport carries exactly what depends on the carrier through the
stated structure, and nothing else. The failure mode Halpin documented can't arise. A weaker relation (a
projection that forgets information, a refinement, a correspondence in some respects) has a different type, and it
supports exactly the transfers its type licenses. The paper says this directly: an observation that forgets
information, a refinement of behavior, and an equivalence of complete structures have different types and
consequences, and calling two things "the same" supplies none of the evidence.

**Schema alignment becomes integration by transport.** Two ontologies for the same domain are two types. An
alignment is an equivalence, or a weaker typed relation, between them. Data, queries and inferences written against
one move to the other along it, with their correctness intact. The paper's sorting development (§4.4 "Ordering: a
declaration that computes") shows this at small scale: a sorted answer with its multiplicity evidence is built by
elimination over a bag and the identities the bag carries. Integrating sources that describe the same things
differently is the general case of the same operation.

## 5. Evidence and observation: knowledge without priors

The paper's §1 gives the account of what an observation warrants that knowledge graphs have lacked. Let H be the
admissible histories of a process and o : H → O an observation. A reading y doesn't identify a history. It
determines the fiber of compatible histories (equation 1.1):

  Fib_o(y) := Σ_{h:H} (o(h) = y),

and what the reading warrants about a proposed conclusion P is a construction over that whole fiber (equation 1.3):

  Warrants(y, P) := Π_{(h,p):Fib_o(y)} P(h).

This is how a knowledge graph should represent incomplete knowledge. A source that reports something is an
observation. What follows from the report is whatever holds for every state of affairs compatible with it. Missing
information is kept as the family of possibilities, not compressed into a number. Combining two sources intersects
their fibers, and refining an observation shrinks the fiber, so a conclusion established on the coarser fiber stays
established on the finer one. When a probability structure really is given, it is extra information that further
reasoning can use. It isn't a stand-in for the information that's absent.

This settles the open-world versus closed-world split that divided the semantic web from databases. Open-world
reasoning says absence of a fact isn't its negation. Closed-world reasoning says it is. In hyperactivity, absence is
itself an observation with a fiber. A dependency certificate (§10.5 "Dependency certificates and persistent
evaluation") records a lookup that found nothing as a dependency on that absence at a given revision. So a
conclusion drawn from absence is valid exactly while the absence holds, and the graph knows when it stops holding.

The same construction governs action (§10.4 "Observations, information, and locally available actions", equation
10.11). An action is safe at an observation when its condition holds over every compatible state:

  SafeAt(y) := Π_{(s,p):Fiber_o(y)} P(s).

A knowledge graph that drives decisions (in an organization, an agent or an instrument) needs exactly this: the
evidence that a decision is justified by what is actually known, not by a guess about the hidden state.

## 6. Change: maintained views and dependency certificates

Knowledge changes. Sources are corrected, the world moves, and derived conclusions have to follow. The paper's §10
makes this part of the theory, not an operational afterthought.

A derived result is kept with its correctness witness (equation 10.2):

  Maintained(q, d) := Σ_{r:R} (r = q(d)),

and an incremental update is correct when it turns a maintained result for d into one for the updated d′ with the
witness intact (§10.2 "A maintained view is a proof about an update", equation 10.4). The theorem is a property of
the functions. A database's materialized view, a spreadsheet's dependency, a rendered visualization and a knowledge
graph's inferred edges are all instances of it.

Dependency certificates (§10.5, equation 10.13) record exactly what a derivation read: each key and version, and
each absence at its epoch. A derivation whose certificate is still valid can be reused without recomputing. One
whose certificate is broken is recomputed, and only those. That's the invalidation rule materialized inference
never had. A knowledge graph built this way can't hold a stale inference, because staleness is detectable: an
inference whose certificate fails is known not to be current.

Two smaller consequences follow. Versioning is free: a changed fact is a new object with a new key, and the old one
stays; the history of a fact is the sequence of bindings of its name to keys. Reproducibility is by content: a
conclusion names, by content, exactly the premises it was drawn from.

## 7. Reflection: the schema, the queries and the language inside the graph

In current systems the ontology, the constraint language, the query language and the reasoner are all outside the
graph. In hyperactivity they're objects inside it. The paper's §6 "Reflective programming and language extension"
shows a language's own definitions (its grammar, judgments, interpreters and transformations) becoming operands of
the same activity as the expressions they interpret. §6.3 "Reflection applied to a language definition itself"
applies reflection to a language definition itself: transforming its represented clauses changes how later inputs
are interpreted.

For knowledge graphs this means a schema is a value in the graph, and evolving the schema is a construction over
that value. Migrating data across a schema change is transport along the relation between the old schema and the
new one. A query is a function over the graph's values, and its result can be kept as a maintained view with a
certificate. A reasoning rule is a construction, and its applications are terms that cite it. Nothing about the
graph's semantics is outside the graph.

## 8. Interpretation: utterances, sources and the people in them

A knowledge graph is built from what people and instruments say. Current systems treat a statement as a payload
that gets extracted, normalized and stored, and its meaning as fixed by the vocabulary. The paper's §1, "Language,
history, and inherited knowledge", rejects that model and gives the alternative an executable form. A reception
relates a participant's retained history, the expression received, the updated state and the interpreted response
(equation 1.4):

  Receive_i : S_i → U → S_i → A_i → 𝒰.

What an expression means depends on who receives it, with what history, under which interpretation. A claim about
what a participant understood has to be established relative to its available history.

For knowledge graphs this means three things. A statement is an event: its source, its time and the context it
answered are part of it, and extraction keeps the event and builds what it warrants as a construction over it.
Disagreement between sources is information, not noise: two sources whose statements conflict are two
observations, and the fiber compatible with both says what the conflict leaves open. The provenance of human
knowledge becomes first-class: a person's words, kept whole and addressed by content, are the inputs, and what the
graph concludes from them is a construction that cites them.

## 9. Work and knowledge are one graph

Today's knowledge graphs model the world and leave out the work that produces and uses knowledge. In hyperactivity
the task graph (lib/task-graph.cub) is a graph of the same kind. A node is a goal named by an English sentence; it
exists because of what it serves (its parts-of list); and it is resolved when its children are. Once
knowledge-work intentions go into this graph (a publication to write, a field's ignorance to dispel), the workflow
graph and the knowledge graph are one graph. The workflow is the record of work done, work pending, and what serves
what. Everything written is work done: code for some interpreter, serving some purpose. A document is a projection
of values, computed whenever it is read, so it can't be stale. The paper itself exists once, as a value, with its
human and pure presentations as projections.

This closes the loop current systems leave open. A knowledge graph should know what its knowledge is for, which
questions it was assembled to answer, and which conclusions are still missing. In hyperactivity, purposes are
nodes, evidence is objects, and the paths from evidence to purpose are constructions. On 2026-11-15, organizing
about 50 knowledge-work targets and what they rest on needed no new primitive: the task graph, documents by content
(lib/document.cub) and projections of one value (lib/paper.cub) already were the system.

## 10. Universality

The claim to universality has a precise content.

**Every existing graph embeds.** A triple is a degenerate construction: a relation type whose inhabitant carries
no rationale beyond its assertion. A property graph is a family of such types with attributes. An OWL ontology is a
type theory of a restricted kind, and its entailments are constructions. A probabilistic knowledge base is a graph
plus a supplied probability structure, which §1 treats as additional information. Every current format can be
represented in hyperactivity without loss, and its semantics become explicit where they were implicit.

**None embeds the other way.** No current format can represent a justification as the content of an edge, an
identity that determines what it carries, a conclusion over a fiber without a prior, a derived fact that knows when
it is stale, or a schema that is a value of the graph it governs. Each of these is a structure the triple can't
hold. So the gap isn't one of degree.

**The theory is computational.** A theory of knowledge graphs that couldn't execute would be one more ontology.
Hyperactivity's constructions run. One machine-realized evaluator executes every represented term, including the
interpreters and transformations (§9.2 "Universal native realization by one evaluator"). The repository's checked
code realizes the paper's constructions, and the same language holds the task graph and the paper.

## 11. Consequences

**Science.** A field's knowledge becomes a graph of constructions over its observations. Two schools that disagree
are two families of constructions, and the fiber compatible with both observations shows what is actually
unsettled. Replication is reuse under a valid certificate. A retracted result invalidates exactly what depended on
it.

**Organizations.** The whole of an institution (its systems, its records, its reasoning and its decisions) becomes
one program with end-to-end verifiability of any provable property. The knowledge graph of the organization and the
organization's computation are the same object.

**Machine intelligence.** A language model's weights hold the usage history of the human record without identity,
justification or currency. A knowledge graph of constructions gives what the model lacks: claims tied to their
warrants, identities with witnesses, and conclusions that know their dependencies. A model working inside it is an
executor of code over that graph, and its outputs become objects whose correctness is relative to what they serve.

**Communication.** Two people who disagree hold two constructions. The system can show the shared premises and
where inference over them diverges, so disagreement becomes navigable.

## 12. Antecedents: Ceptr, the MetaCurrency Project and Holochain

Arthur Brock and Eric Harris-Braun, through Ceptr and the MetaCurrency Project and later Holochain, worked on the
question this answers: how to make meaning travel with data, between agents, with no central authority deciding what
anything means. Avik credits this work as setting him on the path; hyperactivity is a continuation of it.

**Ceptr: data that carries its own meaning.** Ceptr's semantic trees made every datum a structure tagged with a
symbol saying what it is, so a receiver could interpret it without an out-of-band schema. Semtrex, its regular
expressions over semantic trees, made matching on meaning a primitive. In hyperactivity, a value's type is its
meaning and the checker enforces it: Ceptr's symbols and structures become types, and matching on meaning becomes
elimination over a type and its identities. What hyperactivity adds is that two structures meaning the same thing
are related by an equivalence, and transport carries everything across it.

**Receptors and protocols: computing as agents interacting.** Ceptr treated computation as receptors exchanging
signals under explicit protocols, not as applications wrapped around a database. That is the subject of the paper's
§1 and §10 "Persistent, interactive, and distributed programs": processes as interaction trees, handlers as
interpretations of requests, composition of participants internalizing their interactions, and the meaning of a
message as a relation between participants' histories (equation 1.4).

**Holochain: agent-centric validity with no global ledger.** Holochain gave each application shared validation
rules (its "DNA") and let each agent keep its own chain, with peers validating rather than a global consensus
deciding. The paper's §10.3, "A distributed credit invariant, with no global scheduler as an axiom", states the same
stance mathematically: an invariant over a global state no participant can see, proved conserved by every allowed
local transition. Validation rules are types and their checks are proofs.

**MetaCurrency: currencies as ways of seeing flows.** Brock's idea that a currency is a "current-see", a way of making
some flow of value visible and acting on it, is where hyperactivity's credit invariant, order ledger and view of
markets as the exposure of present information come from. The dollar is one scalar instrument for seeing one kind of
flow, and a richer currency is a richer observation of the same activity.

**What the line was building toward.** Ceptr aimed at distributed sense-making: a substrate where meaning, agreement
and value could be computed among people without silos. A knowledge graph whose objects carry their meaning, whose
edges carry their rationale, whose identities carry what they preserve, and whose validity is local and provable is
the completion of that program, with the mathematics it pointed at: univalence for meaning across representations,
warrants for knowledge under partial observation, and maintained views for currency over time.

## 13. Conclusion

Knowledge-graph technology has spent twenty-five years adding layers to an assertion: provenance, identity links,
rules, constraints, confidences, versions and retrieval. Hyperactivity replaces the assertion with a construction.
An object is code with a type, addressed by content. An edge is a relation together with its rationale. Identity is
a path that determines what it transports. Evidence warrants exactly what holds over every compatible history.
Derived knowledge is a maintained view whose certificate knows when it stops being current. The schema, the queries
and the language are values of the graph they govern. The work that produces and uses knowledge is part of the same
graph. Each of the five promises of §1 is a property of this object, and every existing format embeds in it.
