# SGI QvLib 1.0 vendoring and SQZ.EXE comparison

The upstream `include/`, `src/`, `COPYING`, `README` and `QvLib.lsm` are unmodified: **154 files**, verified by `upstream.sha256` and against the local download. `.gitattributes` disables text conversion for these files, preserving their bytes in Git as well. This particular archive contains **76 .c++ files**, rather than the estimated 77: 71 leaf units and five aggregate units (`Qv`, `QvLib`, `QvNodes`, `QvFields`, `QvErrors`). The original notice and distribution metadata are retained verbatim.

## Build and verification

Run from the worktree root in Git Bash:

```sh
bash qvlib/build.sh --parts
bash qvlib/test.sh
bash qvlib/map.sh
bash qvlib/score.sh
"/c/Users/jans.margevics/AppData/Local/Programs/Python/Python315/python.exe" qvlib/tools/report.py
```

`PYTHON` overrides the Python executable used by the shell scripts. These commands need the existing project layout, `tools/flagsearch/env.sh`, VC++ 4.1 (cl 10.10.6038), Perl, objdump and the original EXE. `build.sh` first compiles the untouched `QvLib.c++` unity TU and creates `obj/QvLib.lib`; `--parts` also compiles all 71 leaf TUs through generated .cpp wrappers. Include-order repairs for QvLists, QvTraverse and QvWWWInline stay in those wrappers. `shim/libc.h` supplies the missing libc types and pi constants. All outputs and generated wrappers are ignored under `obj/`.

Flags: `-GX -Gy -O2 -G3 -ML -D__ANSI_CPP__`, with explicit shim/include/src paths. No `-GR`. Each targets file carries its `#FLAGS` line. The targets below use the separate-TU build; unity compilation also succeeds, but changes inlining and register allocation. Three separate-TU near misses (482750, 482810, 480ee0) are exact in the unity object.

The linked VC++ 4.1 smoke program **passed**: nested Separator/Cube parsing, numeric fields, DEF/USE identity, reference-count cleanup, and rejection of an invalid field. This is a parser integration check; game integration and comparison of parsed results against the running original game are not performed here.

## Mapping and scores

- **350 named routines in the requested range: 336 score 0, 14 nonzero.** All 336 zero-score routines also have identical SHA-256 hashes of their relocation-masked bytes, not just identical normalized instruction text.
- The 555 original function-table entries partition into **349 named starts + 181 embedded EH cleanup/fragment entries + 25 routines without QvLib source**. The fragments are covered by their parent targets, not independently scored as duplicate functions. See `mapping.json` for their exact parent/offset assignments.
- One additional named start, QvCube's deleting destructor at **00488260**, was discovered by its vtable. `fns_all.txt` folds it into the constructor. Explicit target sizes separate it correctly.
- **39 additional library routines outside the requested range: 5 score 0, 34 nonzero.** These are 34 traversal methods and five QvElement/diagnostic-helper functions at 0049bf10–0049cd60, beyond the nominal CRT boundary. They are separately listed in `targets_qvlib_outside.txt` and `scores_outside.txt`.
- **403 source functions emitted** by the separate build: 350 mapped in the main range, 39 in the later block, three traversal overrides in game code, QvNode::init at 0047e5d0, and **10 without an identified standalone EXE body**. Those ten may be unused or inlined; this does not imply missing parser functionality.
- No upstream leaf source file is wholly absent: every one of the 71 leaf units has at least one identified function. In particular, **QvTraverse and QvElement are present**, mostly after 0049bf10.

`tools/corpus.pl` loads `obj_functions`, `dis_norm`, PE relocation masking and size helpers directly from the project harness. `static.pl` adds its three internal-linkage functions; the stock parser only considers external symbols. `offsets.py` batches instruction decoding. `map.py` imports all specified legacy seed files, resolves their renamed classes by body/call/vtable evidence, hash-checks relocation-masked bytes, propagates aligned calls and vtables to a fixed point, and records fuzzy leftovers without silently assigning ambiguous candidates. The final mapping has **zero conflicting proposals**. No address/name mapping is hand-entered.

The stock `match.sh` was run on all 349 external targets in the background. The final pass uses its unchanged scoring code through a generated local adapter that additionally accepts COFF storage class 3 for static `stripWhite`; project harness files are untouched. `scores.txt` is that complete 350-function result. `score_table.tsv` supplies mangled names and evidence. `diffs.txt` contains short main-range diffs. The later-block verifier also loads `dis_norm` and `diffcount` from the same harness; game overrides below 0047e600 are never scored by it.

## Main-range nonzero functions

| Address | Function | Score | Diff shape |
|---|---|---:|---|
| `00480490` | QvNode::flushInput | 12 | Same brace-skipping loop; esi/edi roles and argument load scheduling differ. |
| `00480840` | QvFieldData::addEnumValue | 8 | ecx/edx swapped while saving the enum values/names pointers. |
| `00480a50` | stripWhite | 14 | Whitespace scanner uses different index/limit registers and compare order. |
| `00480b50` | QvFieldData::getEnumData | 4 | Reversed cmp operands with the equivalent jge/jle condition. |
| `00480ee0` | QvFieldData::read | 4 | One pointer/index register choice; unity compilation scores 0. |
| `00480f60` | QvInput::isASCIIHeader | 18 | Header/version scan uses different counters, base registers and load order. |
| `00481d90` | QvState::print | 20 | Diagnostic stack-print loop uses different registers and initialization order. |
| `00482340` | QvNameEntry::initClass | 4 | Reversed cmp operands with the equivalent jg/jl condition. |
| `00482750` | QvString::expand | 12 | Allocation/copy loop scheduling and registers differ; unity scores 0. |
| `00482810` | QvString::makeEmpty | 6 | Capacity comparison/scheduling differs; unity scores 0. |
| `00482b10` | QvPList::expand | 24 | Reversed capacity comparison and different copy-loop registers. |
| `00484530` | QvSpotLight::constructor | 116 | esi/edi object/field-data roles swapped throughout SpotLight construction. |
| `00485740` | QvPointLight::constructor | 74 | esi/edi object/field-data roles swapped throughout PointLight construction. |
| `004864b0` | QvMatrixTransform::constructor | 36 | Identity-matrix loop/frame differs; EXE loads float constants, source loads double constants. |

These differences do not by themselves prove functional source changes. Most are equivalent register/compare/scheduling choices; the identity-matrix constructor also changes floating-point constant width and loop scheduling. The unchanged SGI source is retained as requested. The three cases that become exact in the unity TU demonstrate that translation-unit composition explains some differences.

## Traversal modifications and later code

Protozoa **did modify traversal**: all 34 surviving stock traversal methods omit the unconditional ANNOUNCE call in SGI's QvTraverse.c++. The underlying property/state logic remains recognizable. Group/switch methods additionally differ in loop scheduling/register use. The five QvElement/printProperties routines score zero.

Three virtual slots instead reach game code: **QvSeparator::traverse → 0047aa50**, **QvIndexedFaceSet::traverse → 0047aad0**, **QvUnknownNode::traverse → 0047ac90**. These are overrides, not absent source methods; the D3DRM-building path remains out of scope and was not changed or byte-matched. QvNode::init is immediately before the requested range at **0047e5d0**, identified by its call relocation.

| Address | Function | Score |
|---|---|---:|
| `0049bf10` | QvGroup::traverse | 12 |
| `0049bf60` | QvLOD::traverse | 8 |
| `0049bfa0` | QvSwitch::traverse | 36 |
| `0049c010` | QvTransformSeparator::traverse | 15 |
| `0049c100` | QvCoordinate3::traverse | 5 |
| `0049c190` | QvFontStyle::traverse | 5 |
| `0049c220` | QvMaterial::traverse | 5 |
| `0049c2b0` | QvMaterialBinding::traverse | 5 |
| `0049c340` | QvNormal::traverse | 5 |
| `0049c3d0` | QvNormalBinding::traverse | 5 |
| `0049c460` | QvShapeHints::traverse | 5 |
| `0049c4f0` | QvTextureCoordinate2::traverse | 5 |
| `0049c580` | QvTexture2::traverse | 5 |
| `0049c610` | QvTexture2Transform::traverse | 5 |
| `0049c6a0` | QvDirectionalLight::traverse | 5 |
| `0049c730` | QvPointLight::traverse | 5 |
| `0049c7c0` | QvSpotLight::traverse | 5 |
| `0049c850` | QvOrthographicCamera::traverse | 5 |
| `0049c8e0` | QvPerspectiveCamera::traverse | 5 |
| `0049c970` | QvTransform::traverse | 5 |
| `0049ca00` | QvRotation::traverse | 5 |
| `0049ca90` | QvMatrixTransform::traverse | 5 |
| `0049cb20` | QvTranslation::traverse | 5 |
| `0049cbb0` | QvScale::traverse | 5 |
| `0049cc40` | QvAsciiText::traverse | 8 |
| `0049cc50` | printProperties | 0 |
| `0049cc80` | QvCone::traverse | 5 |
| `0049cc90` | QvCube::traverse | 5 |
| `0049cca0` | QvCylinder::traverse | 5 |
| `0049ccb0` | QvIndexedLineSet::traverse | 5 |
| `0049ccc0` | QvPointSet::traverse | 5 |
| `0049ccd0` | QvSphere::traverse | 5 |
| `0049cce0` | QvWWWAnchor::traverse | 3 |
| `0049ccf0` | QvWWWInline::traverse | 3 |
| `0049cd00` | QvInfo::traverse | 3 |
| `0049cd10` | QvElement::constructor | 0 |
| `0049cd30` | QvElement::deleting destructor | 0 |
| `0049cd50` | QvElement::destructor | 0 |
| `0049cd60` | QvElement::print | 0 |

## EXE functions without upstream QvLib source

All remaining 25 entries are in **0048bb30–0048e000**, after the parser's field/node implementation block. They have no mapping to this upstream library; the project context places non-Qv game/math helpers in this tail. They are not evidence that the vendor snapshot lacks parser classes.

| Address | Size |
|---|---:|
| `0048bb30` | 112 |
| `0048bba0` | 32 |
| `0048bbc0` | 368 |
| `0048bd30` | 48 |
| `0048bd60` | 2240 |
| `0048c620` | 96 |
| `0048c680` | 144 |
| `0048c710` | 288 |
| `0048c830` | 64 |
| `0048c870` | 48 |
| `0048c8a0` | 432 |
| `0048ca50` | 1536 |
| `0048d050` | 224 |
| `0048d130` | 112 |
| `0048d1a0` | 160 |
| `0048d240` | 224 |
| `0048d320` | 800 |
| `0048d640` | 352 |
| `0048d7a0` | 192 |
| `0048d860` | 80 |
| `0048d8b0` | 96 |
| `0048d910` | 160 |
| `0048d9b0` | 496 |
| `0048dba0` | 544 |
| `0048ddc0` | 592 |

## Source functions without an identified standalone EXE copy

- `??_EQvName@@QAEPAXI@Z`
- `??_EQvString@@QAEPAXI@Z`
- `?announce@@YAXPBD@Z`
- `?getVersion@QvInput@@QAEMXZ`
- `?read@QvInput@@QAEHAAG@Z`
- `?read@QvInput@@QAEHAAI@Z`
- `?read@QvInput@@QAEHAAN@Z`
- `?remove@QvDict@@QAEHK@Z`
- `?remove@QvNodeList@@QAEXH@Z`
- `?unrefNoDelete@QvNode@@QBEXXZ`

The traversal announcer is explained by the removed announcements. The small accessors, read overloads, container helpers and compiler-generated array deletion helpers may have been discarded or inlined. `mapping.json` retains unresolved candidates and all imported seed hints for future investigation. No source edits were made to force scores to zero.
