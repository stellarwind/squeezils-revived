# Missing-symbol resolution: optimization review

Reviewed 2026-10-21. This is a tooling and work-selection plan, not an
implemented change or permission to weaken ownership/ABI checks. Work remains
one function or small family per block under WORKFLOW.md budgets. No polling,
orchestrator chains, automatic status regeneration, commits or pushes.

## Evidence and snapshot boundaries

The analysis used completed pipeline `9ed43249-1aa5-4d6b-a12b-96f518346bd6`:

| Measure | Reviewed run |
| --- | ---: |
| Manifest sources compiled | 1,137/1,137 |
| Duplicate linker errors | 0 |
| LNK2001 diagnostic messages | 1,614 |
| Unique unresolved names | 1,323 |
| Unresolved extracted code-pointer names | 0 |
| Qv / template-signature / ordinary-other names | 3 / 36 / 1,284 |

The report hashes agreed with the pipeline's generated hashes, and the
snapshot-covered inputs had not changed when checked during the analysis.
The diagnostic completed without an internal linker error or omitted sources.
Cached data/resources/QvLib provenance remained unverified; there was no linked
EXE or runtime validation. Source coverage is a separate scoreboard measure.

At write-up time, a newer completed run exists:
`cd8ec253-803b-4a78-9cae-2e82b5c158f2`, with 1,138/1,138 manifest sources compiled,
zero duplicate errors and 1,317 unresolved names. Its clusters are 3 Qv,
30 template-signature and 1,284 ordinary-other. The guide's restart point already
records this run. Preserve the earlier numbers as analysis evidence, not current
totals. Different manifest inventories prevent treating the totals alone as an
equivalent-input improvement comparison.

Generated evidence lives under `Repo/build/integration/`: pipeline-summary.json,
owner-clusters.json, ownership.json, exact-call-candidates.json,
destructor-candidates.json, data_alias.json and containers/partial-link-summary.json.
Those files are mutable outputs; identify the run and hashes when relying on them.

## Main conclusion

The reproducible compile -> inventory -> aliases -> projection -> diagnostic
pipeline already exists in `Repo/tools/integration_pipeline.py`. Do not spend a
new block recreating it. Improve ownership discovery and actionable triage first,
then reduce repeated compilation and proof work without losing freshness checks.

### 1. Repair ownership metadata before investigating aliases individually

The reviewed ownership.json contains 90 malformed target lines across 45 files,
all reported as "missing fields", and eight unmatched target lines. For example,
`Repo/src/Container/gen/targets_Array05.txt` splits the Grow target and its label
across two lines. The ownership parser requires expression, address and label on
one line. Check the matcher's actual interpretation before changing this family;
do not silently reinterpret or ignore malformed inputs.

The exact-call report rejects 234 currently unresolved names with
"callee lacks unique complete exact-body owner". Missing mappings can hide owners
that already exist, but these 234 are not a promised resolution count: absent
implementations, nonexact bodies and ambiguous owners can produce the same reason.

Bounded tooling block:
- Validate target syntax consistently with the matcher and identify malformed rows.
- Repair the split-target metadata family and regenerate ownership.
- Distinguish missing mappings from missing bodies and competing owners.
- Check affected matching results; retain explicit unique surviving-owner checks.
- Report mapping changes separately from actual linker resolutions.

### 2. Replace the ordinary-other bucket with actionable classifications

`integration_clusters.py` places 1,284 of the reviewed 1,323 names in ordinary-other.
It classifies a name as a container when a template token occurs anywhere in its
signature. Consequently ordinary constructors such as NavProp and FrameProp enter
the container bucket just because they accept template arguments. The bucket is
a triage hint, not an implementation-family count.

Produce two independent fields for each full decorated name:
- Implementation family: owning class, original address, candidate canonical owner
  and shared implementation where evidence supports that grouping.
- Blocker: missing mapping, missing implementation, ambiguous owner, conflicting
  declaration/ABI, unproven caller, data identity/layout conflict, or an accepted
  candidate awaiting registry review.

Demangle for display and family grouping; retain the full decorated symbol as the
identity. Group by the owning entity rather than types appearing in arguments.
Join current diagnostic names, referring relocations/sources, ownership mappings,
discovery rejection reasons and reviewed registries. Unknown evidence must remain
explicit rather than becoming a guessed owner.

Rank coherent families by distinct unresolved names, affected sources, evidence
already available, likely work and dependencies introduced. Diagnostic-message
count is useful impact evidence but not sufficient priority: one heavily called
name can outrank a family that removes more work. Separate linker messages from
actual relocation-site counts.

### 3. Surface accepted-but-unregistered candidates

The reviewed destructor discovery accepts three names still unresolved; they also
remain pending in the newer write-up-time report:

| Full decorated name | Existing candidate owner |
| --- | --- |
| `??1LaunchParams@@QAE@XZ` | `??1Pair@@QAE@XZ` |
| `??1M405260@@QAE@XZ` | `??1Pair@@QAE@XZ` |
| `??1PhysCore@@QAE@XZ` | `??1Pair@@QAE@XZ` |

All map to original 0x4052b0 in `Fam68/TwoVecDtor.cpp`. These are discovery
results, not bindings approved by this note. Review current declarations, complete
destructor ABI, original body and owner selection, then record valid decisions in
the tracked registry. Revalidate through projection and one diagnostic link.

Add a pipeline report for accepted candidates absent from registries, intersected
with current unresolved names. Deduplicate discoveries across generators. This
prevents useful proof from remaining buried in JSON while harder cases are chosen.
Do not automatically register or emit a candidate merely because discovery accepts it.

### 4. Resolve declaration and lifecycle families at their source

A coarse split by decorated-symbol spelling in the reviewed run finds 197 data
names, 120 constructors, 116 destructors, eight allocation/deallocation operators,
18 other operators and 864 remaining methods/functions. These categories partition
that snapshot; they do not prove ownership or estimate safe alias counts.

`gAmbientNames` has three incompatible declared types: AmbientNameArray,
AmbientNames and PrimitiveArray<String,2>. Other examples include gBodyModels,
whose declarations use ProtoBodyModelArray and ProtoLoadModels. Repeated aliases
can preserve conflicting local views instead of fixing the underlying interface.

For each small family, prove original storage identity and layout; select one
implementation/storage owner and shared declaration; update the referring units;
rerun affected matches. Treat constructor/destructor families similarly, including
vtable identity, element stride/lifetime, hidden return buffers, this adjustments,
and complete versus deleting-destructor distinctions. Retain reviewed aliases
where source unification is inappropriate for matching, with explicit ownership.

The reviewed exact-data discovery rejects 173 unresolved names as unproven or
conflicting references, nonzero addends, or no unique extracted entry. Split these
reasons before picking work rather than relaxing the proof to make a batch pass.
Zero unresolved code-pointer names does not establish that all runtime pointer
identity and relocation behavior have been tested.

### 5. Reuse compilation and proof inputs with dependency-aware freshness

`integration.py` currently deletes and recompiles each manifest object's output
on every run, sequentially. A registry-only change still recompiles the whole
inventory. Several proof generators also reread the same objects and original
instruction evidence. These are opportunities; no timing benchmark was collected
in this review, so no speedup factor is claimed.

Add explicit invalidation rules:
- Registry/policy change: reuse verified compiled objects; regenerate dependent
  aliases/projection and perform one diagnostic link for the completed family.
- Source/header change: rebuild affected translation units and invalidate their
  dependent inventories/proofs and projections.
- Compiler, flags, include environment or support-runtime change: invalidate
  affected compilation entries rather than relying on object existence/timestamps.
- Manifest change: rebuild the source/object inventory, including removals and
  ownership changes, so old inputs cannot survive unnoticed.

Compilation cache keys need source and transitive header contents, compiler and
support dependencies, flags/defines and relevant environment. Extend provenance
to external INCLUDE headers and compiler support files currently outside snapshot
coverage before treating cache reuse as verified. Keep the existing cached binary
provenance limitations explicit.

Cache parsed COFF inventories and original instruction indexes by content hash.
Keep proof-result keys dependent on original image, relevant objects, target maps,
declarations, registry/policy and proof-tool version. Never skip proof regeneration
solely because a previous alias object exists. A safe registry-only pipeline mode
is preferable to ad hoc manual stage skipping.

## Recommended bounded work order and acceptance criteria

1. Target-map repair: consistent parsing, malformed-family repair, refreshed owner
   inventory and affected match checks; quantify recovered mappings.
2. Blocker-aware triage: every current unresolved name appears exactly once in the
   primary classification, while family memberships may cross-reference it;
   counts and report hashes reconcile with the completed diagnostic.
3. Pending destructor family: review the three candidates above, register only
   proven bindings, retain one owner and complete one diagnostic pass.
4. Incremental build/proof reuse: demonstrate unchanged-input reuse and proper
   invalidation for source, shared header, flags, registry and manifest changes;
   compare against a clean diagnostic on equivalent inputs.
5. Use the improved queue for subsequent small data/lifecycle/method families.

Keep the cheap external-reference accounting per function and one diagnostic link
per completed family. Report source coverage, compiled-source coverage, duplicates,
unique unresolved names, input verification and runtime validation independently.
Do not weaken original-address, ABI/layout, full-body or symbolic-relocation checks
to reduce totals. Improvements should find and reuse proof more efficiently.

## Target-map repair completed (2026-10-21)

The first bounded block is implemented. Rejoined 45 split rows (31 Array Grow,
13 Set RemoveKey and one Table RemoveKey) without changing expressions, original
addresses, labels or source bodies. The generator already emits single-line rows
and required no change. Python ownership inventory and the Perl matcher now enforce
the same row grammar; malformed metadata stops ownership/alias generation. The
match.sh wrapper also preserves matcher failures and their diagnostics. The shared
Perl parser is included in pipeline input hashes.

Using the same compiled inventory before/after repair, invalid rows fell from 90
to zero, mapped addresses rose from 2,962 to 2,994, and mapped symbols rose from
2,625 to 2,670. Eight unrelated unmatched rows remain. All 45 recovered symbols
have one original-address mapping and compiled bodies; 23 have one compiled
definition and 22 have competing definitions that still require projection and
surviving-owner checks. These are recovered mappings, not approved aliases.

Checked all 256 targets in the 45 repaired files: 189 exact and 67 nonexact,
no missing scores and no regression against existing numeric FUNCTIONS.tsv
scores. Recovered targets score 0 (28), 10 (2), 19 (6), or 25 (9). Four parser,
cross-language grammar, manifest-validation and fail-closed inventory tests pass.
Generated evidence: build/integration/target-repair-rows.json,
ownership-before-target-repair.json, target-repair-checks.json and
target-repair-matches/. STATUS.md was not regenerated.

Pipeline 820ebfce-8303-4de9-ae1d-3040e02f78df compiled 1,139/1,139 manifest
sources and refreshed ownership, then stopped before projection/link because
entry_aliases.py rejected the existing exact-call registry decision for
`?Find@Table2@@QAEPAUEntry@@ABVString@@@Z`. The regenerated shared-leaf evidence
no longer includes original entry 0046d2a0 in declared_original_entries. This
same registry failure was already present in run
d1972e14-7ed5-42c8-9759-601cb87330cd before this repair. No registry decision was
weakened or automatically refreshed. No new duplicate/unresolved totals are
available; source coverage is unchanged and runtime remains untested. This is an
incomplete pipeline run, not a successful diagnostic or freshness verification.

The Table2::Find blocker is now resolved by reviewing 23 existing TableGet
registry entries: six use the addressed 0046d2a0 owner and 17 retain their old
owner with that shared entry removed. Destinations and caller/ABI proofs did not
change. All 18 accessor match targets score zero; 11 proof tests pass. Completed
pipeline 08c1b003-7d33-4a9a-86bc-ee61e521e01a compiled 1,139/1,139 sources,
zero duplicates, 1,604 LNK2001 messages and 1,315 unique unresolved names;
snapshot-covered inputs were unchanged. This supersedes the failed run above.
Earlier diagnostics used different inventories, so no equivalent-input reduction
is claimed. Cached provenance remains unverified; EXE linking/runtime are pending.
See EXE_INTEGRATION_GUIDE.md final section for details. Next bounded block:
blocker-aware triage. Remaining optimization recommendations are still planned.


The pending complete-destructor family in section 3 is now reviewed and registered:
LaunchParams, M405260 and PhysCore all select the sole complete owner at 004052b0.
Fresh proofs, six destructor tests and the byte-exact owner match pass. Completed
pipeline 28d367f4-1779-46e3-a364-dc405a6b7af5 compiles 1,139/1,139 sources,
zero duplicates, 1,601 LNK2001 messages and 1,312 unique unresolved names.
Only the destructor registry changed relative to the preceding equivalent-source
run; all three names disappeared. Snapshot-covered inputs stayed unchanged.
EXE/runtime and cached provenance remain unverified. See the integration guide.


Blocker-aware triage and pending-candidate reporting (sections 2/3) are implemented
in integration_clusters.py schema 2. Completed pipeline
2a8fcf02-13f4-42db-8cf4-82fea1fd6f64 verifies the pipeline hook and report hash:
1,139/1,139 compiled, zero duplicates, 1,312 unique unresolved names, 1,601 messages.
All names appear exactly once; 2,412 compiled relocation sites are counted separately.
Eight focused tests pass. Conservative classifications leave 774 unknowns explicit;
effort/dependency estimates remain for review. One accepted unregistered unresolved
candidate is surfaced: LevelBrainTable::KeyAt at 0044de90. Review its addressed
owner and String return ABI next. See the integration guide's final section.
