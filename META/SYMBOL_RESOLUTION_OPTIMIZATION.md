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


Surface accessor/transform family completed: 27 reviewed declarations now bind to
ten complete byte-exact owners via surface_aliases.py and surface_registry.json.
Five focused tests pass. Completed pipeline cbf6a1b4-0b47-445c-a516-4563dd46b9fe
compiles 1,139/1,139 sources, zero duplicates and 1,285 unique unresolved names
(1,312 -> 1,285 on the same source inventory); all 27 disappear from diagnostics.
No source body edits or relaxed proof gates. Two nonexact IsNear bindings remain
withheld. See integration guide for return-buffer/forwarder ABI evidence.


Navigation contact-query extension completed: nine additional reviewed bindings
across five exact owners. Six surface tests pass. Completed pipeline
f1d4a985-7730-4ad7-bbb6-daf835e0a827 compiles 1,139/1,139 sources with zero
duplicates and 1,276 unique unresolved names. Saved name-set comparison proves
nine removals and no additions (1,285 -> 1,276); no body edits or relaxed checks.
Larger nonexact movement/loading methods remain outside the exact-alias batch.


Application family completed with eleven reviewed bindings removing ten existing
unresolved names. A conflicting SetFlag declaration was split: RegionInit clears
+b4 at 43aa20; PlaceStart sets it at 43aa10. Original calls and all ten exact owner
bodies are rechecked. Caller scores remain 61/87; four application and six shared
surface tests pass. Completed pipeline 98bf731b-b29c-44bc-8277-446b7dd002be compiles
1,139/1,139 sources, zero duplicates, 1,266 unique unresolved names. Name-set delta
proves ten removals and zero additions. See integration guide for full ABI evidence.


Integer ID-list ownership completed: the duplicate constructor unit was retired
in favor of the existing exact Set<int,10> body; two reviewed lifecycle bindings
preserve callers. Both complete vtables now have one verified owner and all 13
slots agree with original identity, correcting the inherited wrong factory slot.
Five tests and three byte-exact targets pass. Completed pipeline
42841610-e40e-4be6-8a31-5de655772d85 compiles 1,138/1,138 sources, zero duplicates,
1,248 unique unresolved names. Saved sets prove 18 removals and no additions
(1,266 -> 1,248). Manifest changed only by removing the proven duplicate. See the
integration guide for complete-object ABI, typed destruction and projection proof.


LevelBrainTable::KeyAt is now reviewed and registered to the sole addressed
Table<String,RoboBrain*,Array<RoboBrain*>,0> owner at 0044de90. Hidden String
result buffer, ECX/table/key offsets, virtual key access and ret 8 agree with the
original; caller and owner matches are zero and eleven proof tests pass. Completed
pipeline f0a8730c-4600-4bd6-9968-e67fc835207e compiles 1,138/1,138 sources, zero
duplicates, 1,531 LNK2001 messages and 1,247 unique unresolved names. Saved sets
prove this one removal and no additions (1,248 -> 1,247); no pending accepted
unregistered candidates remain. Snapshot-covered inputs were unchanged during
this run and report hashes reconcile. The prior baseline also differs in
Tiny4/Names.cpp, which this block did not edit; no registry-only equivalent-input
comparison is claimed. Source coverage unchanged; cached provenance unverified,
EXE/runtime pending. See integration guide for ABI and ownership evidence.


Character/Creature pointer-array interface family completed: shared ObjArrayC/D
declarations now own the existing TypeName, Read and Write bodies, and factories
use the same declarations. Original secondary vtable slots and all eight affected
match targets agree exactly; five object audits find no unresolved externals.
Pipeline 643b1fca-66f9-4b2d-a5da-1c310f9fc1ad compiles 1,138/1,138 sources, zero
duplicates, 1,507 LNK2001 messages and 1,237 unique unresolved names. Saved sets
prove ten removals and no additions (1,247 -> 1,237). Inventory unchanged; snapshot
inputs unchanged during the run and report hashes reconcile. Source coverage is
unchanged; cached provenance unverified and EXE/runtime pending. See integration
guide for vtable identity, stream ABI and single-owner evidence.


Integer-array member lifetime ownership is now shared at source: C40c420 uses
Array<int> directly and the standalone cleanup uses the same 0x14-byte declaration.
Both unowned destructor spellings disappear without new aliases. All twelve
wrapper-unit targets remain byte-exact; cleanup retains its known score-2 frame
difference. Six proof tests pass; 18 existing registry decisions refresh hashes
only, with identity/ABI/address evidence unchanged. Completed pipeline
16d810bf-e77f-4754-a1c8-ea67265824d4 compiles 1,138/1,138 sources, zero duplicates,
1,505 LNK2001 messages, 1,235 unresolved names. Saved sets prove two removals and
no additions (1,237 -> 1,235); snapshot/report hashes verify this run. Cached
provenance unverified, EXE/runtime pending. Integration guide also records the
incomplete LandSimulation primary/secondary vtables that require a shared-class
repair; no unsafe placeholder-slot aliases were added.


## String-container member destructor ownership (2026-10-21)

Container/StringContainerMembers.h gives C40be40 its actual Array<String> member
(base destructor 45e250), and C40c3d0 its actual Table<String,String,
PrimitiveArray<String,3>,23> member (base destructor 462b50). Misc17/Dt2.cpp uses
these shared declarations, retiring the two unowned M40be40/M40c3d0 names. Original
cleanup tail jumps establish both destinations; existing RTTI gives the table's
BufferObject offset +2c. Compile-time guards check complete sizes 14/30 hex.
All twelve wrapper targets remain exact; six destructor tests pass. Eighteen
existing registry decisions refresh only owner/source hashes; script assertions
preserve addresses, ABI, owners and declaration evidence. No new aliases.
Completed pipeline 052e239a-b67d-49d0-a7d4-f18afc596b21 compiles 1,138/1,138 sources,
zero duplicates, 1,503 LNK2001 messages and 1,233 unique unresolved names. Saved
string-members-name-delta.json proves two removals and zero additions (1,235 ->
1,233); snapshot inputs unchanged during run and report hashes verified. This
supersedes 16d810bf-e77f-4754-a1c8-ea67265824d4. Source inventory/coverage unchanged;
remaining wrapper-unit unowned names are M40be90 and M450910. Cached provenance
unverified; EXE/runtime pending. STATUS.md/HANDOFF.md untouched; no commit/push.


## Functional source priority and animation ownership (2026-10-21)

User explicitly superseded byte matching: no match.sh, scores, padscan or register
tuning. WORKFLOW.md records the override. Changed functions are explicitly func
in states.txt; no new score measurements were made. The old generic Owner41
blend owner's score is obsolete: it was replaced by typed functional source,
not preserved as a bm target. Existing scores for other edited units may change.
STATUS.md/FUNCTIONS.tsv were not regenerated.

Interrupted dispatch run de708b27-aa7d-47c7-8e2a-c52af841ea56 had completed:
1,138/1,138 sources, zero duplicates, 1,492 LNK2001 messages, 1,227 unique names
(1,233 -> 1,227). Script/CallFunction.h supplies one cdecl dispatch declaration;
15 direct callers and 47 transitive translation units use the canonical entry.
Three now-unused dispatch aliases were retired. This is prior evidence, not the
current restart point.

Animation source now shares MorphCycle execution/creation declarations,
PoseTable lifetimes/accessors and PartCycle/Execution.h. Advance, SetCycle,
blend-fraction, air-animation and pose-application callers name these source
owners directly. MorphCycle::BlendPose replaces the generic Owner41 integer-
argument view at 41bcf0, forwarding actual float weight, time and hint to the
existing PartCycleB::Blend. Empty tracks return the incoming hint. BodyUpdate.h
now shares pitch/roll/character fields at +210/+218/+22c with AnimateInAir;
the implementation is directly UpdatingSqueezilBody::AnimateInAir. No substitute
stubs or no-op implementations were introduced.

Seven global bindings formerly depended on exact caller bodies. Their already-
reviewed addresses are now explicit individual extern declarations in Fn458cd0;
normal data ownership resolves them to the same extracted symbols, asserted in
animation-declared-data-owners.json. The old multi-variable comment misleadingly
implied declaration order: FlyUp=b4, FlyLeft=b8, FlyRight=bc, FlyDown=c0 relative
to 4bf000. No address was inferred from that ordering. Obsolete exact aliases
for AdvancePose/CPose lifetimes, Clip blend/length, PoseEntries::Entry and the
air-animation method were removed as their callers now use the real definitions.
Old failed pipeline attempts are superseded below; no original-body proof gate
was bypassed or made to accept a nonexact binding.

Authoritative completed pipeline 2ffa28ea-790e-4b8e-a5fe-920ec74ff9c5 compiles
1,138/1,138 sources with no failures/omissions, zero LNK2005, 1,479 LNK2001
messages and 1,214 unique unresolved names; no internal linker error. Snapshot
inputs stayed unchanged and report hashes reconcile. Saved animation-name-delta
proves 13 removals and zero additions (1,227 -> 1,214). The manifest replaces
one generic blend source with its typed owner; original-address coverage remains
unchanged. Four native VC4 functional cases link the production BlendPose object
against an instrumented part-track callback: empty/nonempty tracks, order,
pose pointers, time offset, float weight, hint and returned last result pass.
Evidence: animation-blend-test-result.json. This is focused runtime validation,
not gameplay validation. Cached binary/external-header provenance remains
unverified, EXE still does not link. No commits/pushes; HANDOFF.md unchanged.


## Navigation storage/loading shared source (2026-10-21)

NavMesh/NavMesh.h now provides the actual 0x2030-byte mesh layout, construction,
allocation/cleanup, file-format detection and ASCII/binary loader interfaces.
Compile-time checks cover 0x0c points and 0x28 faces. Misc15/Mesh.cpp owns the
constructor and allocators under NavMesh; the redundant NavMeshCtor translation
unit/target is retired from the manifest. The existing NavMesh destructor calls
FreeArrays/FreeTree directly, retiring the old Free1/Free2 aliases. Point arrays
use the existing Vector3fEmpty owner via a shared header instead of an undefined
MVert constructor. ASCII/binary input writes the same face fields and now agrees
with the BoxNode** tree-reader declarations. World and surface-loading callers
include the shared mesh declaration instead of incomplete private storage views.
No replacement stubs, byte-match runs, padding sweeps or score preservation.
Touched original addresses are explicitly func; old scores may change and were
not measured. STATUS.md/FUNCTIONS.tsv were not regenerated.

Existing math and face-destructor declaration evidence moved from Mesh.cpp to
the shared header; one mesh-destructor object hash changed after direct helper
calls. Three reviewed records were refreshed with unchanged addresses, targets,
ABI and per-source declaration-address identity. Generated evidence is in
nav-loading-proof-refresh.json. Tree-node lifetime implementation and geometric
raycast helper declarations remain separate unresolved families.

Authoritative completed pipeline 0adb4bd0-ddb2-44e3-806d-8bba095c287e compiles
1,137/1,137 active manifest sources without failures/omissions; zero duplicate
errors, 1,466 LNK2001 messages and 1,205 unique unresolved names. No internal
linker error; snapshot inputs unchanged and report hashes reconcile. This
supersedes 2ffa28ea-790e-4b8e-a5fe-920ec74ff9c5 and the failed intermediate
1bdb8b1c-91e0-480f-95be-0de12e18a88c. nav-loading-name-delta.json proves nine
removals and no additions (1,214 -> 1,205). Source inventory decreases only by
retiring the duplicate constructor; original-address source coverage is unchanged.
Fourteen native VC4 checks exercise production allocator, face constructor,
empty-point and mesh-destructor objects (with test-only vector/face lifetime
and tree-clear leaf dependencies): initial fields, allocation/reallocation to
zero, face defaults, tree cleanup, repeated array cleanup and destruction pass.
Evidence: nav-storage-test-result.json. File parsing/gameplay are not runtime-
validated; cached binary/external-header provenance remains unverified. EXE
still cannot link. No commit/push; HANDOFF.md unchanged.


## Navigation tree lifetime owner (2026-10-21)

NavMesh/BoxNode.h now shares the complete 0x40-byte node layout across mesh
cleanup and both box readers. Misc15/Octree.cpp supplies BoxNode construction,
recursive complete destruction, face-index allocation and FreeItems. Child
deletion invokes its destructor exactly once; face storage uses delete[]. Mesh
FreeTree likewise performs one typed delete. Retired the duplicate Node8::Release
body/target from Misc6 and its now-unreferenced Clear alias. No stubs or binary
wrappers; all four tree addresses remain func. Scores may change and were not
measured/preserved. One existing face-destructor record refreshed its shared-
header hash only; ownership and ABI decisions are unchanged.

Completed pipeline 66078bb4-17a1-4e39-8a83-aa616d8a0d68 compiles 1,137/1,137
sources without failures/omissions; zero duplicates, 1,460 LNK2001 messages,
1,202 unique unresolved symbols, no internal linker error. Snapshot inputs stayed
unchanged; report hashes reconcile. nav-tree-name-delta.json proves three removed
BoxNode names, no additions (1,205 -> 1,202). This supersedes
0adb4bd0-ddb2-44e3-806d-8bba095c287e and the failed intermediate proof-refresh run.
Sixteen native VC4 checks link the production Octree object: initialization,
allocation/reallocation/empty allocation, repeated release and nested child
destruction pass, with eight tracked allocations and eight releases. Evidence:
nav-tree-test-result.json. Source inventory/address coverage unchanged; cached
binary/external-header provenance unverified, EXE/gameplay pending. No byte-match
commands, STATUS regeneration, commit or push; HANDOFF.md unchanged. Remaining
NavMesh raycast helper declarations are a separate next family.
