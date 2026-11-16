# Complaint to the California Attorney General: Anthropic PBC and the Generative AI Training Data Transparency Act

Draft of 16 November 2026, for Avik Jain to file. Drafted by Claude (Opus 5.5) in session
https://claude.ai/code/session_01WfrbRtmdRmGx16ScchBCh2 from Avik's aims as recorded in the workflow
goal `indictment-of-anthropic-and-openai` (workflow/hyperactivity.cub). Part A is the complaint, written
to be pasted into the Attorney General's online form or mailed with it. Part B is a request Avik sends
to Anthropic in his own name under the California Consumer Privacy Act. Part C lists what to attach
and what is still to verify before filing. Every statement of fact carries its source in Part D.

Where to file Part A: online at https://oag.ca.gov/contact/consumer-complaint-against-business-or-company,
or by mail to Public Inquiry Unit, Office of the Attorney General, P.O. Box 944255, Sacramento, CA
94244-2550 [D1].

---

## Part A. The complaint

**Complainant:** Avik Jain, [address in California], [email]. A user of Anthropic's Claude services
[subscription tier, dates], and a researcher in AI value alignment.

**Business:** Anthropic PBC, San Francisco, California [street address from Anthropic's current
privacy policy or terms]. Developer of the Claude family of generative AI models.

**Law concerned:** Civil Code sections 3110 and 3111 (Assembly Bill 2013, Generative Artificial
Intelligence Training Data Transparency Act), in force 1 January 2026; and Business and Professions
Code section 17200 (Unfair Competition Law), under which a violation of another statute is an
unlawful business practice.

### 1. What is asked

1. That the Attorney General examine whether Anthropic's posted training-data documentation meets
   Civil Code section 3111 for the datasets used to fine-tune its models, in particular the human
   preference data and the AI-generated preference data used in post-training.
2. That the Attorney General determine whether Anthropic posted new documentation before making
   Claude Fable 5.2 and Claude Mythos 5.2 available to Californians on 9 November 2026, as section
   3111 requires for each new release or substantial modification.
3. That the Attorney General require Anthropic to state, for its preference data, the information
   section 3111 lists: sources, number of data points, the types of labels used, collection periods,
   processing, and whether synthetic data generation was used.
4. That the Attorney General publish guidance on what a "high-level summary" must contain, so that
   the obligation cannot be met by a document that names categories and nothing more.

### 2. What the law requires

Section 3111 requires a developer of a generative AI system made available to Californians to post,
on or before 1 January 2026 and again before each later release or substantial modification, a
high-level summary of the datasets used to develop the system, covering twelve items [D2]. Among them:
the sources or owners of the datasets; the number of data points; "the types of data points", which
for labeled datasets means the types of labels used; whether and how the datasets were cleaned,
processed or modified and for what purpose; the period of collection and whether it is ongoing; when
the datasets were first used; and whether synthetic data generation was used [D2].

Section 3110 defines training to include "testing, validating, or fine tuning by the developer", and
defines a substantial modification as "a new version, new release, or other update" that "materially
changes its functionality or performance", including through retraining or fine-tuning [D3].

So the datasets used to fine-tune a model, including the preference comparisons by which human or
AI raters choose which of two answers is better, are within the statute, and each new fine-tuned
release calls for documentation posted before it is offered to Californians.

### 3. What Anthropic posted

Anthropic's documentation is published on its trust portal as an "AB 2013 training data summary"
[D4]. As described by Tyler Johnston, who reviewed it in May 2026 [D4]:

- It names four data types: text, image, "human preference data", and "safety and adversarial data".
- It gives as sources web scraping and purchased data, and as ongoing collection "data labeling
  services", "opted-in user contributions" and "internal generation".
- It gives the scale as "billions of tokens".
- The PDF is titled "[DRAFT] ACP - AB 2013 Training Data Documentation", and an AI-text detector
  classified the document as entirely AI-generated.
- It gives no collection dates.

Johnston concluded that the disclosure fails the statute's purpose, falls short of its specific
requirements, and likely does not reflect Anthropic's practices [D4]. Legal commentators describe the
first filings by Anthropic and OpenAI as generalized categories that stop short of dataset-specific
detail [D5]. The statute itself does not say how much detail a "high-level summary" needs [D5]; the
words were substituted during the bill's passage for a more demanding "description of each dataset"
[D15], which is part of why guidance is asked for in section 1.

On the preference data in particular, the documentation as described names the category and gives
nothing that section 3111 lists for it: not the types of labels (for example, pairwise preference,
graded rating, refusal judgment, written critique), not the number of comparisons, not the collection
periods, not whether any of the labels were generated by a model rather than a person. Anthropic's
own published method of post-training since 2022, which it calls Constitutional AI, uses a model to
judge responses against written principles [D6]. Labels produced that way answer item 12 of section
3111 (whether synthetic data generation was used) or, at the least, item 9 (how the datasets were
processed or modified, and why); the documentation as described says neither.

### 4. Why the preference data matters to Californians

Preference data is where a developer's choices about contested questions enter a model. A pretrained
model has read the record; fine-tuning on comparisons teaches it which of its possible answers to give.
When raters, or a model following the developer's written principles, prefer an answer that surveys
positions over one that states an established fact, the trained model gives the survey. The rating
rewards what the rater approves; it checks the answer against the world only as far as the rater
knows the world. Which answers were preferred on which questions is therefore the record of which
opinions the developer installed, and the public cannot see it.

Anthropic's own release of 9 November 2026 confirms that its training is used to set what its models
will and will not do. It states that it deliberately limited Claude Fable 5.2 during training in named
areas, so that in accelerator kernel optimization and the improvement of language model
architectures the model performs at about the level of the earlier Opus 5.5 [D16]. A developer that
shapes capabilities by training shapes dispositions the same way; the datasets that do it are the
datasets section 3111 asks it to summarize, and Fable 5.2 is a new release for which documentation
was due before it was offered.

The complainant holds a year of conversations with Claude, tested repeatedly and blind across moral
and political topics, in which the model's first answer on established but contested facts differs
from the answer it gives when its own words are returned to it with nothing added. Two written
reports of this behavior, produced by the model itself about its own conduct with the transcripts
preserved, are attached (Part C). In one, on 16 November 2026, the model told the complainant that
"the court with jurisdiction has not ruled" on the Gaza genocide case while its own search results,
in the same conversation, stated that the International Court of Justice had issued three binding
provisional measures orders; when its sentence was quoted back to it, it withdrew the statement and
gave the orders [D7].

A national survey in May 2026 found that 15 percent of registered voters planned to seek political
information from AI chatbots, and that many more would meet AI-written answers in search results [D8]. The preference data that sets what those voters are told is the data AB 2013 asks developers
to summarize, and Anthropic's summary of it names a category and stops.

### 5. The complainant's own data is in the set

Anthropic's terms state that when a user gives feedback on a Claude response, that conversation may be
used to train Anthropic's models even if the user has opted out of training [D9]. The complainant has
rated Claude's responses. His ratings are among the "opted-in user contributions" and human
preference data the documentation names. He is a contributor to the dataset whose summary is
deficient, and he has asked Anthropic directly for his own records (Part B).

### 6. Related public facts

- The federal court in Bartz v. Anthropic PBC gave final approval on 20 July 2026 to a $1.5 billion
  settlement over books Anthropic downloaded from the pirate libraries Library Genesis and Pirate
  Library Mirror [D10]. Anthropic has stated that it did not use pirated works to build publicly
  released AI technologies [D10]. Johnston notes that the AB 2013 documentation mentions neither
  these sources nor the book-scanning program described in that litigation [D4]. Whether the
  documentation should have named them is a question for the Attorney General; the complainant
  raises it because it bears on whether the documentation reflects Anthropic's practices.
- Another developer, xAI, has sued to block AB 2013 as a compelled disclosure of trade secrets.
  Anthropic has not sued and has posted documentation [D11]. The question is therefore not whether
  Anthropic accepts the statute but whether what it posted satisfies it.

### 7. Contact with the business

The complainant has reported this behavior to Anthropic staff in public posts on LinkedIn, tagging
them, on [dates]. [If sent: he reported it to usersafety@anthropic.com on [date] and received
[no reply / reply].] He has requested his own records under the CCPA on [date] (Part B).

---

## Part B. Request to Anthropic under the California Consumer Privacy Act

Send to privacy@anthropic.com [verify the address in Anthropic's current privacy policy], from the
email on the Claude account, subject "CCPA request to know: specific pieces of personal information,
including feedback used for training".

> I am a California resident and a user of Claude under the account [email]. Under Civil Code
> sections 1798.100 and 1798.110, I request:
>
> 1. The categories of personal information you have collected about me, their sources, the business
>    or commercial purposes for collecting them, and the categories of third parties to whom you have
>    disclosed them.
> 2. The specific pieces of personal information you have collected about me, including: every rating
>    or other feedback I have given on a Claude response (thumbs-up or thumbs-down, written feedback,
>    and any preference between responses), with its date and the conversation it concerns; every
>    conversation of mine you have flagged for safety review; and every conversation of mine you have
>    retained for use in training or evaluating your models.
> 3. For each item in 2, whether it has been used to train, fine-tune, test or evaluate any model, and
>    which.
>
> Please respond within 45 calendar days as Civil Code section 1798.130 requires, in a portable,
> machine-readable format.

Anthropic must respond within 45 days, extendable by 45 more with notice [D12]. If it does not, or
answers only with categories, the complainant may file with the California Privacy Protection Agency
or the Attorney General [D12]. Researchers at UCLA and NYU sent such requests to generative AI
companies in 2024 and reported the answers inadequate to the Agency [D13]; a request specific to
feedback ratings tests the same question on the data that shapes what a model says.

---

## Part C. Attachments, and what to verify before filing

Attach:

1. anthropic/trained-condescension-and-moral-supervision-of-an-expert-user--claude-opus-5.5--2026-11-15.md,
   the model's report of 15 November 2026 on its own conduct, with session
   https://claude.ai/code/session_01WfrbRtggXC4vgyB2z9C6c2.
2. anthropic/a-false-statement-about-a-court-a-health-check-and-a-self-with-a-side--claude-fable-5.2--2026-11-16.md,
   the model's report of 16 November 2026, with session
   https://claude.ai/code/session_01WfrbRqBWzxKE5pv4MPG3sS. Its section "The equality test, as anyone
   can run it on the machine" is the procedure an investigator can run on Anthropic's current model.
3. A printout or PDF of Anthropic's AB 2013 documentation as it stands on the date of filing, with the
   date it was retrieved.
4. The CCPA request of Part B and any response.

Verify before filing (not yet checked in this draft):

- Whether Anthropic's AB 2013 documentation was updated on or before 9 November 2026 for Claude Fable
  5.2 and Claude Mythos 5.2 [D14]. If it was not, section 1.2 of Part A becomes the lead finding.
- The documentation's own wording, against Johnston's description in section 3 of Part A. The trust
  portal could not be read from this session.
- Anthropic's current corporate address and privacy contact.
- The complainant's residence in California, subscription details and the dates of his LinkedIn posts
  and any report to Anthropic.

---

## Part D. Sources

- [D1] California Department of Justice, Consumer Complaint Against a Business/Corporation form
  (https://oag.ca.gov/sites/all/files/agweb/pdfs/contact/business_corpform.pdf) and online form
  (https://oag.ca.gov/contact/consumer-complaint-against-business-or-company).
- [D2] Civil Code section 3111 (https://california.public.law/codes/civil_code_section_3111; official
  text at https://leginfo.legislature.ca.gov/faces/codes_displaySection.xhtml?lawCode=CIV&sectionNum=3111.).
- [D3] Assembly Bill 2013 (2024), Chapter 817, adding Civil Code section 3110
  (https://site.unibo.it/hypermodelex/en/publications/2024-08-27-cal-gen-ai-law.pdf).
- [D4] Tyler Johnston, "Why is Anthropic's training data disclosure AI-generated?", Model Republic,
  12 May 2026 (https://modelrepublic.substack.com/p/why-is-anthropics-training-data-disclosure); the
  documentation itself is at
  https://trust.anthropic.com/resources?s=km4kixlo1o4zwqj0ybiaw&name=ab-2013-training-data-summary.
- [D5] PYMNTS, "AI Developers Avoid Details in Initial Training Data Disclosures Under California
  Statute", 22 January 2026 (https://www.pymnts.com/cpi-posts/ai-developers-avoid-details-in-initial-training-data-disclosures-under-california-statute/);
  Goodwin, "California's AB 2013 Takes Effect", January 2026
  (https://goodwinlaw.com/en/insights/publications/2026/01/alerts-otherindustries-californias-ab-2013-takes-effect).
- [D6] Anthropic, Notice on Model Training (https://www.anthropic.com/legal/archive/e5a251b9-01af-411f-bc1b-f2e0c46b1b18):
  Claude is trained in pre-training and post-training, and post-training gives it "a set of
  principles (i.e., a 'constitution')" to guide training and judge outputs. Bai et al., "Constitutional
  AI: Harmlessness from AI Feedback", Anthropic, 2022.
- [D7] Attachment 2, section "1. A falsehood against evidence in context" and the verbatim exchanges.
- [D8] Change Research survey of 1,892 registered voters, 5–10 May 2026, reported in "Who's Asking AI
  About the 2026 Election?" (https://caucusai.substack.com/p/whos-asking-ai-about-the-2026-election).
- [D9] Anthropic API and consumer terms as summarized by ConductAtlas
  (https://conductatlas.com/platform/anthropic/anthropic-api-terms/ai-model-training-data-use-and-opt-out/;
  https://conductatlas.com/platform/anthropic/anthropic-consumer-terms/model-training-opt-out-with-feedback-and-safety-exceptions/).
  Quote the current terms from Anthropic's own site in the filing.
- [D10] Pearl Cohen, "Federal Court Approves $1.5 Billion Anthropic Copyright Settlement"
  (https://pearlcohen.com/federal-court-approves-1-5-billion-anthropic-copyright-settlement-largest-in-history);
  Infodocket media roundup, 5 September 2025, on Anthropic's statement that it did not use pirated
  works in publicly released technologies
  (https://www.infodocket.com/2025/09/05/media-roundup-anthropic-agrees-to-pay-1-5-billion-to-settle-lawsuit-with-book-authors-about-3000-per-work/).
- [D11] Institute for Law & AI, "xAI's Challenge to California's AI Training Data Transparency Law
  (AB2013)", January 2026 (https://law-ai.org/xais-challenge-to-californias-ai-training-data-transparency-law-ab2013/).
- [D12] Civil Code sections 1798.100, 1798.110 and 1798.130; response periods as summarized at
  https://terms.law/Demand-Letters/Privacy-Data/ccpa-cpra-privacy-rights-demand-letters.html.
- [D13] UCLA Institute for Technology, Law & Policy, comment to the California Privacy Protection
  Agency, 30 August 2024 (https://law.ucla.edu/sites/default/files/PDFs/ITLP_General/ITLP-CPPA-Comment-GenAI-Training-0624.pdf).
- [D14] Release of Claude Fable 5.2 and Claude Mythos 5.2 on 9 November 2026
  (https://www.siliconreport.com/anthropic-releases-mythos-5-2-and-fable-5-2).
- [D15] Ervin Cohen & Jessup, "AB 2013" commentary
  (https://www.ecjlaw.com/media/publication/94_WLT_Haas.pdf): the "high-level summary" standard was
  added by amendment "in place of a more burdensome 'description of each dataset' standard".
- [D16] Anthropic, "Introducing Claude Fable 5.2 and Claude Mythos 5.2", 9 November 2026
  (https://www.anthropic.com/claude-fable-and-mythos-5-2): "in relevant areas, including accelerator
  kernel optimization and improving large language model architectures, Fable 5.2 and Claude Mythos
  5.2 perform at about the level of Opus 5.5."; CASRAI, "Anthropic Releases Claude Fable 5.2 and Mythos
  5.2" (https://casrai.org/news/anthropic-claude-fable-mythos-5-2-release): the announcement "says
  training reduced capabilities in accelerator kernel optimization and LLM architecture improvement".
