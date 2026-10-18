# QvLib parser baseline

**16/16 inputs pass**: VC++ 4.1 parts with silent traversal (A), MinGW g++ at
`-O0` (B), and VC++ 4.1 unity (C) produce identical canonical dumps, parser return
values and exact read-error messages. The 15 non-game B dumps are committed as
goldens. Only the dump hash is committed for NUT2.WRL.
golden/manifest.json also preserves B's exit status and dump hash for every input.

This is a parser comparison against the same SGI reference source compiled by
another compiler, not an independent VRML implementation or a full-game test.
Passing this finite corpus does not establish equivalence for every VRML file.

## Run

From the worktree root in Git Bash:

```sh
bash qvlib/test/run.sh
bash qvlib/test/run.sh --a-only
```

The default builds A/B/C once, runs each on all inputs and compares A/B, A/C and
A/golden. `--a-only` builds and runs A against the committed golden files/hashes;
it does not invoke MinGW or build/run the C executable. Stock build.sh also
creates the unity library while building the parts objects. Compiler output,
executables and local input copies stay under ignored `work/` or `../obj/`.
Dumps are `out/{A,B,C}/<filename>.txt`, with LF line endings. A failure prints
its first differing line and saves the full unified diff in `out/`.

`--run-only` rechecks existing A/B/C executables without rebuilding. Internal
`QV_REUSE_LIBS=1` skips the unchanged VC library build during a dumper repair;
`QV_REUSE_VC=1` skips the VC stages during a MinGW compatibility repair. Neither
is needed for normal runs. Slow compiler work is a single finite scripted batch;
there is no polling or retry loop.

The usual project VC4 environment is required at ../../tools/flagsearch/env.sh,
relative to the repository root; MinGW is `D:/Decomp/mingw32/bin/g++`. Python 3
and Git Bash are also required. Each compiler include directory is passed with
`-I` because env.sh replaces INCLUDE. The checked-in SGI fixtures allow tests to
run without the download archive. The original game file is required at the
source path in sources.json. No original input is modified. Source SHA-256s are
checked on every run and all 154 vendor hashes are verified.

## Per-file results

PASS for an invalid input means all builds reject it identically and A matches
the B golden, including messages, source line numbers and overall return value.

| Input | Read result | Unique nodes | A/B | A/C | A/golden |
|---|---|---:|---|---|---|
| AllNodes.wrl (SGI) | Accept | 40 | PASS | PASS | PASS |
| Sample.wrl (SGI) | Accept | 26 | PASS | PASS | PASS |
| NUT2.WRL (game) | Accept | 2 | PASS | PASS | PASS (hash) |
| all_fields.wrl | Accept | 5 | PASS | PASS | PASS |
| def_use.wrl | Accept | 4 | PASS | PASS | PASS |
| unknown_fields.wrl | Accept | 3 | PASS | PASS | PASS |
| whitespace.wrl | Accept | 3 | PASS | PASS | PASS |
| float_edges.wrl | Accept | 3 | PASS | PASS | PASS |
| empty.wrl | Reject: invalid header | 0 | PASS | PASS | PASS |
| use_before_def.wrl | Reject: unknown reference | 0 | PASS | PASS | PASS |
| unknown_no_fields.wrl | Reject: unknown class | 0 | PASS | PASS | PASS |
| bad_header.wrl | Reject: invalid header | 0 | PASS | PASS | PASS |
| unknown_field.wrl | Reject: unknown field | 0 | PASS | PASS | PASS |
| unterminated_brace.wrl | Reject: expected closing brace | 0 | PASS | PASS | PASS |
| bad_number.wrl | Reject: cannot read width | 0 | PASS | PASS | PASS |
| mf_missing_bracket.wrl | Reject: expected comma or bracket | 0 | PASS | PASS | PASS |

The 13 hand-written fixtures cover DEF/USE across nested Separators and across
top-level reads, USE before DEF, defaults and explicit values, unknown classes
with populated/empty/absent fields[], comments, tabs, blank lines, empty input
and five other malformed cases. all_fields.wrl exercises all **18 field types**:
12 SF types and 6 MF types. SFEnum and SFBitMask use built-in nodes with declared
enumerations; the remaining types include an unknown-node schema. SFImage tests
two four-component pixels and the full component byte stream. float_edges adds
rounding boundaries, subnormal-boundary scale, a large finite value and negative
zero. The SGI samples retain their exact source bytes and are covered by the
vendored SGI license in ../COPYING.

## Dumper contract

dump.cpp calls `QvDB::init`, `QvInput::setFilePointer` and `QvDB::read` repeatedly
until null/EOF or failure, following the upstream test/readTest.c++ calling
pattern. Every returned root is ref-counted and retained until the last read.
Output is depth-first, with stable first-visit IDs, actual class names, DEF names,
USE identities and every field in metadata declaration order. It prints the
field type, isDefault flag and full value. Enums/masks are numeric values;
rotations include axis and angle, matrices contain all 16 values, MF values
include their element count, and images include dimensions, component count and
all bytes. Strings are quoted with deterministic escaping. No raw pointer values
are printed. The C locale is fixed. Both output streams are unbuffered and the
runner merges stderr into stdout, preserving the exact SGI read-error text.
Every read's return value and the aggregate result are printed. A parser error
exits 1, success exits 0, and an observer/internal error exits 2 or 3.

No traverse() method is called. Without `-GR`, node/field types are identified
by comparing vtable identities with stock prototype objects, without reading
those prototypes' field values. Only known group subclasses are inspected for
children; QvNode does not initialize children for non-groups. Unknown nodes use
their public instanceFieldData, since the stock generated getFieldData accessor
returns shared metadata that may belong to a later unknown node. This observer
correction was necessary for AllNodes' two Squid instances; all three builds had
the same initial observer failure, not a parser difference.

## The observed A/B difference and its cause

The initial raw `%.9g` dumps differed only in float_edges.wrl. The trace is
`dump.cpp::number` → the compiler runtime's `sprintf("%.9g", ...)`:

| Stored value | VC4 CRT | MinGW/UCRT |
|---|---|---|
| 1e-8 rounded to float | 9.99999994e-009 | 9.99999994e-09 |
| IEEE negative zero | 0 | -0 |
| Small positive normal | 1.17549435e-038 | 1.17549435e-38 |
| Large finite float | 3.40282347e+038 | 3.40282347e+38 |

diagnostics/float-format.diff retains the raw output differences with IEEE bits.
diagnostics/float-bits.json records all 22 floats in that fixture: **every bit
agrees in A/B/C**, including `0x80000000` for negative zero. Reproduce this check
with `python qvlib/test/diagnose-floats.py`. `QV_RAW_FLOATS=1` enables raw CRT
rendering for a manual dumper run; `QV_FLOAT_BITS=1` also appends the IEEE bits.
The normal runner clears these diagnostic variables.

This is a **compiler/runtime float-formatting difference**, not a QvLib build
bug, uninitialized value, char-signedness difference or parsed-value discrepancy.
The formatting function belongs to the test observer/CRT and is not in
score_table.tsv. The relevant parser path contains these score-0 functions:

| Function | SQZ address | Score |
|---|---|---:|
| QvInput::read(float &) | 00481590 | 0 |
| QvInput::readReal(double &) | 00481950 | 0 |
| QvSFVec3f::readValue | 00489c70 | 0 |
| QvSFFloat::readValue | 0048a3f0 | 0 |
| QvMFFloat::read1Value | 0048b340 | 0 |

The canonical observer still formats with `%.9g`, then removes only redundant
exponent zero padding and preserves the IEEE negative-zero sign explicitly.
It changes no significant digits, read errors or parser values. This makes all
canonical outputs identical without editing or masking differences in QvLib.

## Compiler compatibility and baseline provenance

A uses the exact `build.sh --parts` objects and flags
`-GX -Gy -O2 -G3 -ML -D__ANSI_CPP__`, substituting only a generated silent
QvTraverse wrapper. Unlike the earlier game link test, no game overrides or
stubs are needed: all stock virtual definitions are retained and never invoked.
C uses the untouched unity QvLib.lib and the same VC4 dump.obj. B uses MinGW-W64
i686 GCC 16.2.0, `-O0 -std=gnu++98 -fpermissive -D__ANSI_CPP__`, the same
dump.cpp, and the vendored unity source composed through a generated wrapper.

The separate shim-mingw/libc.h supplies the old libc typedefs and math constants.
`-fpermissive` accepts the old extra-qualified constructor declarations. Two
pre-standard for-loop scopes in QvFieldData require generated compatibility
copies: shim-mingw/prepare.py hoists the loop index declaration in stripWhite
and in the named-field QvFieldData::read overload. This preserves the scope that
VC4 assigns to the existing variable; no operations or parser decisions change.
All other source bytes are used unchanged. The upstream files are never edited.
These two functions are existing near misses (00480a50: 14, 00480ee0: 4) in
score_table.tsv, but produce **no observed A/B output difference** here.

Goldens are copied only from the successful B run by the explicit maintenance
tool accept-baseline.py, which refuses incomplete/failing three-way results.
Normal run.sh never regenerates or overwrites them. NUT2's source hash/path is
in sources.json and its canonical B dump hash is in golden/NUT2.WRL.sha256;
neither its original content nor its dump content is tracked. To deliberately
refresh a baseline, run `run.sh --bootstrap`, inspect all results, then invoke
`python qvlib/test/accept-baseline.py`. The default run continues to compare with
the committed goldens.
