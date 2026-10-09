# Byte-match plan for the remaining code

Assessment of the "compiler-in-the-loop search" architecture (compile cache → constraint extraction → feedback-guided
search) against what the byte-match track actually does, plus the plan for the remaining ~4480 functions / ~502 KB.
The how-to is [BYTEMATCH_GUIDE.md](BYTEMATCH_GUIDE.md); the numbers live in [BYTEMATCH_STATUS.md](BYTEMATCH_STATUS.md).

## The one-sentence verdict

The recommended architecture is sound **in general** but was written for the wrong regime: it assumes the bottleneck is
"compilation + blind enumeration of C syntax." Here compilation is *not* the bottleneck (human disassembly-reading is),
the search is *not* blind (a first draft is usually within 0–25 lines), and a large share of the remaining near-misses are
*provably not fixable by any source change* (register allocation, x87 scheduler leak, EH-funclet boundary). So: adopt the
cheap, low-risk pieces (a TU-aware compile cache, divergence triage, a small diverse beam), and **skip** the heavy pieces
(warm compiler workers, a typed-AST mutation engine, a surrogate compiler) until a measurement says otherwise.

## What the recommendation assumes vs what we have

| Recommendation assumes | Reality here (evidence) |
|---|---|
| Compilation is the bottleneck | A candidate costs ~0.6 s of `cl`+`objdump` (the 864-combo flag matrix ran in ~45 s over 12 jobs); the bottleneck is reading `nm/full.asm` and writing the right dialect |
| Blind enumeration of many candidates | Human-first: a function is usually 0–25 lines off on the first try; 4–8 hand-authored variants, ~3 rounds |
| Need a byte matcher | Already exists: `flagsearch.pl`/`match.sh` give an instruction-level match with relocations masked — that *is* the "final correctness oracle" the recommendation wants |
| Per-function caching/search is safe | It is not: codegen of a function depends on what precedes it in the TU (x87 scheduler leak) *and* on unrelated header content. Any cache/search must be TU-aware and address-order-aware |

## Map: recommendation → what we already have

| Recommendation item | Already present |
|---|---|
| Compile + binary matcher as oracle | `match.sh` (`cl -GX -GR -O2 -G5 -ML` + extract + normalize + diff). Done and settled |
| Cache / dedupe candidates | Only an in-run memo in `flagsearch.pl` (`md5(label+extracted code) → score`). **No disk cache, no cross-run cache, no compile cache** |
| Candidate ranking | `tryfn.sh` (one function), `ptry.sh` (header/template functions, parallel) |
| Targeted mutation | `greedy.pl` (textual rewrite rules, coordinate descent), `typesearch.pl` (`short`↔`unsigned short` per variable) |
| Divergence feedback | Manual: a human reads the side-by-side diff. **No automatic first-divergence classification** |
| Beam / diversity | Only "keep the single best, repeat" |

## What to adopt (in priority order), and why

### 1. Divergence triage (highest value, currently missing)
The single most useful automation is *classifying the diff*, not generating more candidates. A small script should take the
`match.sh` side-by-side output and label the mismatch so the human spends compiles only on what is fixable:

- **source-fixable**: operand order, branch inversion, `!` vs plain test, `float` vs `double`, intrinsic vs call, missing/inlined helper, wrong `ret N`, constant width.
- **allocator/scheduler artifact (leave it)**: `this`/vptr/`esi`/`edi` register swap, x87 `fld` order that leaks from the preceding function, "match in isolation but drift in a bigger file", the EH-cleanup-stub `jmp +N` boundary.

The Close table in BYTEMATCH_STATUS.md already records these verdicts by hand ("allocator choice, not source shape" etc.);
the automation is to make that classification **mechanical and first**, so nobody re-tries a register swap a tenth time. The
triage should also report *where* the first divergence is (prologue/frame vs late arithmetic), which is the cheap half of the
recommendation's two-stage scoring.

### 2. Disk-backed, TU-aware compile cache (adopt with the right key)
Add a content-addressed cache in front of `match.sh`/`flagsearch.pl` that stores the **extracted function bytes** (or the
normalized instruction list), keyed conservatively on:

```text
hash( full translation-unit source · content of every included header · defines · flags · cl identity )
```

- Key on the **whole TU + all headers + defines**, never on one function: the docs prove unrelated declarations in a shared
  header shift unrelated functions, and a function's bytes depend on what precedes it in the same file.
- Cache failed compiles/diagnostics too.
- This makes `status.sh` re-runs near-instant (it recompiles all ~126 units on every run) and skips re-testing a candidate that
  was already tried in an earlier session. It does **not** fix the near-miss search (every new candidate is a different TU),
  so it is a throughput win, not a search win.

### 3. Small diverse beam + reject-before-compile (light version of P3/P4/P5)
- Extend `ptry.sh`/`tryfn.sh` to keep the top-**K structurally distinct** candidates (not just the best), where "structurally
  distinct" is judged by the divergence classifier's label, so a register-swap variant does not crowd out a genuinely different
  loop shape. The docs already ask for "structurally different candidates"; this just makes the pool explicit.
- Add a cheap pre-screen before compiling a hand-authored candidate batch: calling convention/`ret N`, intrinsic-vs-call,
  EH-frame present/absent, and the already-known commutative/canonicalizations. The candidate sets are small, so this is a
  minor win, but it is cheap.

## What to skip or defer (and why)

| Recommendation item | Decision | Why |
|---|---|---|
| Warm compiler workers / process orchestration | **defer** | `cl` 10.10 is a fast 1996 binary and has no persistent mode; startup is not the bottleneck; parallel `ptry.sh` already hides it |
| Full type-directed AST + mutation engine | **skip** | Solves an enumeration problem we do not have; the remaining near-misses are mostly non-source-fixable, and the bulk of remaining work is *translation*, not *optimization* |
| Surrogate compiler / learned cost model | **not yet** (agrees) | compiler throughput is not limiting; validate caching+pruning first |
| Heavy per-candidate instrumentation | **light only** | we already know compile isn't the bottleneck; measure cache hit-rate and compiles-per-exact-match, not startup breakdowns |

## The plan for the remaining ~4480 functions / ~502 KB (~90 %)

Ordered by leverage. The top of the list is *not* more search machinery.

**A. Keep the translation engine running (dominant ROI).** The remaining work is mostly functions with no source yet, and a
first draft already lands near-exact. Do, in address order and one original source file per `.cpp`, with minimal includes,
scoring in isolation first:
1. **Mine the trivial leaves first** — `{ return field; }` getters, field setters, no-op/`return 0` vtable slots. Already proven
   first-try exact and cheap (the 2026-10-09 sweep gained +28 exact this way). There are more in `Repo/src/*.h` and in the
   still-unmapped regions.
2. Port the already-identified independent areas in BYTEMATCH_STATUS.md "Not written yet" / "Next candidates": remaining
   container instantiations, the INI classes (0x4426d0–0x443700), Matrix4 turns/`TurnAxis`, `BufferObject` slots, `Array<String>`
   copy ctor, `Set::Insert`, `Table::Get`, `Set<int,N>`/`Set<float,25>` copies.
3. Keep the "one template source exact for every instantiation" check and the `status.sh -g` regression gate after each header change.

**B. Triage the Close list, do not chase artifacts.** For each close function, run the divergence classifier once and record
the verdict. Only spend further compiles on "source-fixable". For "allocator/scheduler/context artifact", either leave it
(most register swaps) or reproduce the TU context (the x87-leak cases, e.g. `Vector3f::Length` needs the FP shape of the
navmesh box-test/tree-walk that precedes it). Never loop on a register swap.

**C. Add the three automations from above, in order:** (1) divergence triager, (2) TU-aware compile cache, (3) diverse beam +
pre-screen. Implement each as a script next to the existing `tools/flagsearch/` helpers, and update this doc + BYTEMATCH_STATUS.md
with what each one measurably saves.

**D. Revisit warm workers / batching / a surrogate only if** compilation becomes the measured dominant cost after A–C. If
batching is ever tried, only with modes verified against the per-function baseline — combining functions into one TU changes
codegen (the docs already depend on that fact for exact matches).

## Non-negotiables carried over from the guide

- The compiler and flags are settled; never touch them.
- Compare in isolation *and* in address order; a function can match alone and drift in context.
- Keep each unit's includes minimal; guard unfinished/extra declarations behind a macro so they cannot shift other units.
- Relocations are masked already; treat relocation handling separately from true instruction mismatches (already done).

## Automation status (2026-10-09)

- **C1 divergence triager — done.** `tools/flagsearch/triage.pl` (classify one ref/got pair), `triage.sh` (run match.sh then classify every non-zero function), `triage_all.sh` (loop the whole manifest). Labels: `allocator-register-swap`, `allocator-register-move`, `eh-funclet-boundary`, `reorder` (x87/operand), `length±N` (helper/intrinsic/EH-frame), `structural` (source-fixable candidate); each line also reports the first divergence line and prologue-vs-body. First concrete win: it flagged `Body::SetMatrix`/`SetContact` as `length-N` (ours `rep movs` vs the original's unrolled `mov` stores) → unrolled them to explicit per-field stores → 46/49 → 0. The full `triage_all.sh` sweep confirmed the rest of the Close list is allocator/reorder/EH artifacts (documented in BYTEMATCH_STATUS.md), except a few still-open `length-N` leads.
- **C2 compile cache — done (v1).** `tools/flagsearch/cache.sh` = match.sh with a disk cache keyed on the targets file + source files + `env.sh` + `FS_FLAGS` (md5). `FS_CACHE=` sets the cache dir (default `tools/flagsearch/.cache`). Limitation: it does **not** hash the shared `Repo/bm` headers, so a header-only edit needs the cache cleared (`rm -rf tools/flagsearch/.cache`); the header-tree hash turned out to be non-deterministic/slow, so it was dropped. Verified miss→compile+cache→hit (instant).
- **C3 diverse beam + pre-screen — not yet.** `ptry.sh`/`tryfn.sh` still keep only the single best candidate.
