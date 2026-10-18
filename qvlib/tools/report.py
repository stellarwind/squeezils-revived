"""Validate artifacts and regenerate the human-readable vendoring report."""
import collections
import difflib
import hashlib
import json
from pathlib import Path

here=Path(__file__).resolve().parents[1]
root=here.parents[1]
def load(name): return json.loads((here/name).read_text())
m=load('mapping.json');c=load('obj/mapped-corpus.json');o=load('outside.json')
unity=load('obj/corpus.json')
scores={int(n.removeprefix('qv_'),16):int(v) for n,v in (s.split('=') for s in (here/'scores.txt').read_text().splitlines())}
targets=[s.split() for s in (here/'targets_qvlib.txt').read_text().splitlines() if s and not s.startswith('#')]
assert {int(s[1],16) for s in targets}==set(scores)
assert len(scores)==len(m['mapped']) and max(scores.values())<999
assert not m['conflicts']
assert {n for n,v in m['mapped'].items() if scores[v['address']]==0}==set(m['byte_exact'])
source=here/'src'
upstream=root/'archive/qvlib_download/src/QvLib-1.0'
verified=0
for row in (here/'upstream.sha256').read_text().splitlines():
    digest,name=row.split('  ',1)
    b=(here/name).read_bytes()
    assert hashlib.sha256(b).hexdigest()==digest,name
    if upstream.exists(): assert b==(upstream/name).read_bytes(),name
    verified+=1
assert (here/'obj/QvLib.lib').exists()
assert 'QvLib smoke passed' in (here/'obj/smoke.log').read_text()
shapes={
0x480490:'Same brace-skipping loop; esi/edi roles and argument load scheduling differ.',
0x480840:'ecx/edx swapped while saving the enum values/names pointers.',
0x480a50:'Whitespace scanner uses different index/limit registers and compare order.',
0x480b50:'Reversed cmp operands with the equivalent jge/jle condition.',
0x480ee0:'One pointer/index register choice; unity compilation scores 0.',
0x480f60:'Header/version scan uses different counters, base registers and load order.',
0x481d90:'Diagnostic stack-print loop uses different registers and initialization order.',
0x482340:'Reversed cmp operands with the equivalent jg/jl condition.',
0x482750:'Allocation/copy loop scheduling and registers differ; unity scores 0.',
0x482810:'Capacity comparison/scheduling differs; unity scores 0.',
0x482b10:'Reversed capacity comparison and different copy-loop registers.',
0x484530:'esi/edi object/field-data roles swapped throughout SpotLight construction.',
0x485740:'esi/edi object/field-data roles swapped throughout PointLight construction.',
0x4864b0:'Identity-matrix loop/frame differs; EXE loads float constants, source loads double constants.'}
def short(n):
    if n.startswith('??0'): return n[3:].split('@@')[0]+'::constructor'
    if n.startswith('??1'): return n[3:].split('@@')[0]+'::destructor'
    if n.startswith('??_G'): return n[4:].split('@@')[0]+'::deleting destructor'
    parts=n.lstrip('?').split('@')
    return (parts[1]+'::' if len(parts)>1 and parts[1] else '')+parts[0]
inverse={v['address']:n for n,v in m['mapped'].items()}
uncovered=sorted(a for a in m['unmapped_exe'] if str(a) not in m['covered_funclets'])
unlocated=sorted(set(m['unmapped_source'])-set(o)-set(m['outside_range']))
main_zero=sum(s==0 for s in scores.values())
outside_zero=sum(v['score']==0 for v in o.values())
original_mapped=len(m['mapped'])-len(m['discovered_addresses'])
assert original_mapped+len(m['covered_funclets'])+len(uncovered)==555
assert len(c['functions'])==len(m['mapped'])+len(o)+len(set(m['outside_range'])-set(o))+len(unlocated)
rows=['region\taddress\tscore\tsymbol\tevidence']
for a,n in sorted(inverse.items()): rows.append(f'main\t{a:08x}\t{scores[a]}\t{n}\t{m["mapped"][n]["reason"]}')
for n,v in sorted(o.items(),key=lambda p:p[1]['address']): rows.append(f'outside\t{v["address"]:08x}\t{v["score"]}\t{n}\tvtable/call propagation')
(here/'score_table.tsv').write_text('\n'.join(rows)+'\n')
main_table=['| Address | Function | Score | Diff shape |','|---|---|---:|---|']
diffs=[]
for a,n in sorted(inverse.items()):
    if not scores[a]: continue
    assert a in shapes,hex(a)
    main_table.append(f'| `{a:08x}` | {short(n)} | {scores[a]} | {shapes[a]} |')
    sz=int(next(t[3] for t in targets if int(t[1],16)==a))
    ref=next(v for v in c['exe'][str(a)]['variants'] if v['size']==sz)['norm']
    diff=list(difflib.unified_diff(ref,c['functions'][n]['norm'],fromfile='SQZ.EXE',tofile='SGI source',n=1))
    diffs.extend([f'=== {a:08x} {short(n)} score={scores[a]}']+diff[:26]+(['... (truncated)'] if len(diff)>26 else [])+[''])
(here/'diffs.txt').write_text('\n'.join(diffs).rstrip()+'\n')
out_table=['| Address | Function | Score |','|---|---|---:|']
for n,v in sorted(o.items(),key=lambda p:p[1]['address']):
    out_table.append(f'| `{v["address"]:08x}` | {short(n)} | {v["score"]} |')
missing_table=['| Address | Size |','|---|---:|']+[f'| `{a:08x}` | {c["exe"][str(a)]["size"]} |' for a in uncovered]
text=f'''# SGI QvLib 1.0 vendoring and SQZ.EXE comparison

The upstream `include/`, `src/`, `COPYING`, `README` and `QvLib.lsm` are unmodified: **{verified} files**, verified by `upstream.sha256` and against the local download. `.gitattributes` disables text conversion for these files, preserving their bytes in Git as well. This particular archive contains **76 .c++ files**, rather than the estimated 77: 71 leaf units and five aggregate units (`Qv`, `QvLib`, `QvNodes`, `QvFields`, `QvErrors`). The original notice and distribution metadata are retained verbatim.

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

- **{len(scores)} named routines in the requested range: {main_zero} score 0, {len(scores)-main_zero} nonzero.** All {main_zero} zero-score routines also have identical SHA-256 hashes of their relocation-masked bytes, not just identical normalized instruction text.
- The 555 original function-table entries partition into **{original_mapped} named starts + {len(m['covered_funclets'])} embedded EH cleanup/fragment entries + {len(uncovered)} routines without QvLib source**. The fragments are covered by their parent targets, not independently scored as duplicate functions. See `mapping.json` for their exact parent/offset assignments.
- One additional named start, QvCube's deleting destructor at **00488260**, was discovered by its vtable. `fns_all.txt` folds it into the constructor. Explicit target sizes separate it correctly.
- **{len(o)} additional library routines outside the requested range: {outside_zero} score 0, {len(o)-outside_zero} nonzero.** These are 34 traversal methods and five QvElement/diagnostic-helper functions at 0049bf10–0049cd60, beyond the nominal CRT boundary. They are separately listed in `targets_qvlib_outside.txt` and `scores_outside.txt`.
- **403 source functions emitted** by the separate build: 350 mapped in the main range, 39 in the later block, three traversal overrides in game code, QvNode::init at 0047e5d0, and **{len(unlocated)} without an identified standalone EXE body**. Those ten may be unused or inlined; this does not imply missing parser functionality.
- No upstream leaf source file is wholly absent: every one of the 71 leaf units has at least one identified function. In particular, **QvTraverse and QvElement are present**, mostly after 0049bf10.

`tools/corpus.pl` loads `obj_functions`, `dis_norm`, PE relocation masking and size helpers directly from the project harness. `static.pl` adds its three internal-linkage functions; the stock parser only considers external symbols. `offsets.py` batches instruction decoding. `map.py` imports all specified legacy seed files, resolves their renamed classes by body/call/vtable evidence, hash-checks relocation-masked bytes, propagates aligned calls and vtables to a fixed point, and records fuzzy leftovers without silently assigning ambiguous candidates. The final mapping has **zero conflicting proposals**. No address/name mapping is hand-entered.

The stock `match.sh` was run on all 349 external targets in the background. The final pass uses its unchanged scoring code through a generated local adapter that additionally accepts COFF storage class 3 for static `stripWhite`; project harness files are untouched. `scores.txt` is that complete 350-function result. `score_table.tsv` supplies mangled names and evidence. `diffs.txt` contains short main-range diffs. The later-block verifier also loads `dis_norm` and `diffcount` from the same harness; game overrides below 0047e600 are never scored by it.

## Main-range nonzero functions

{chr(10).join(main_table)}

These differences do not by themselves prove functional source changes. Most are equivalent register/compare/scheduling choices; the identity-matrix constructor also changes floating-point constant width and loop scheduling. The unchanged SGI source is retained as requested. The three cases that become exact in the unity TU demonstrate that translation-unit composition explains some differences.

## Traversal modifications and later code

Protozoa **did modify traversal**: all 34 surviving stock traversal methods omit the unconditional ANNOUNCE call in SGI's QvTraverse.c++. The underlying property/state logic remains recognizable. Group/switch methods additionally differ in loop scheduling/register use. The five QvElement/printProperties routines score zero.

Three virtual slots instead reach game code: **QvSeparator::traverse → 0047aa50**, **QvIndexedFaceSet::traverse → 0047aad0**, **QvUnknownNode::traverse → 0047ac90**. These are overrides, not absent source methods; the D3DRM-building path remains out of scope and was not changed or byte-matched. QvNode::init is immediately before the requested range at **0047e5d0**, identified by its call relocation.

{chr(10).join(out_table)}

## EXE functions without upstream QvLib source

All remaining {len(uncovered)} entries are in **0048bb30–0048e000**, after the parser's field/node implementation block. They have no mapping to this upstream library; the project context places non-Qv game/math helpers in this tail. They are not evidence that the vendor snapshot lacks parser classes.

{chr(10).join(missing_table)}

## Source functions without an identified standalone EXE copy

{chr(10).join('- `'+n+'`' for n in unlocated)}

The traversal announcer is explained by the removed announcements. The small accessors, read overloads, container helpers and compiler-generated array deletion helpers may have been discarded or inlined. `mapping.json` retains unresolved candidates and all imported seed hints for future investigation. No source edits were made to force scores to zero.
'''
(here/'README.md').write_text(text)
print(f'Validated {verified} vendor files; main {main_zero}/{len(scores)} exact; later {outside_zero}/{len(o)} exact; {len(uncovered)} EXE and {len(unlocated)} source without a body mapping')
