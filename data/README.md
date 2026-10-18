# SQZ global data extraction

Worktree: `D:/Decomp/Squeezil/Repo-data`, branch `extract-global-data`.
All changes belong to `data/`. The shared Repo checkout stays on main.

Run `bash data/build.sh` from this worktree, or `bash build.sh` here, in Git Bash.
Requirements: Python 3 (stdlib only), ghidra-cli on PATH, Ghidra 12.1.4, and the
VC++ 4.1 setup in ../../tools/flagsearch/env.sh. Set GHIDRA_INSTALL_DIR if needed;
build.sh defaults to the existing D:/Decomp/ghidra_12.1.4_PUBLIC installation.

The user's original ../../SqueezilsGameFiles/SQZ.EXE and ../../tools/rtti.tsv are
read-only inputs. On a clean build, build.sh imports the executable into ignored
analysis.gpr/analysis.rep and exports snapshot.tsv using Snapshot.java. Changes
to the script or EXE refresh the snapshot. All generated game content stays local.
The documented ../../Squeezil.gpr is an empty marker without a backing .rep;
the extraction therefore uses fresh analysis rather than unavailable custom labels.

Tracked files: .gitignore, extract.py, check.py, rc_decode.py, Snapshot.java,
build.sh, README.md, function_targets.tsv, skipped.tsv, ambiguous.tsv. The tables
contain only addresses, names, sizes and reasons. No game bytes or resource text
belong in git. Ignored artifacts: data.cpp/data.h, resources.rc, resources/,
resources.json, manifest.json, snapshot*.tsv, compile.log, verification.log,
*.obj, *.res, __pycache__/, analysis.*. They regenerate from the user's EXE.

extract.py reads PE relocations, Ghidra boundaries and references, and propagates
code ownership through data references. It skips compiler RTTI/vtables and data
exclusively rooted in library code; uncertain ownership stays included. It emits
symbolic addresses, normal printable C string literals, and exact-round-trip
float/double literals. Snapshot.java supplies Ghidra types and FPU/integer memory
operand widths (with p-code fallback). Clear integer reads become unsigned int
or unsigned short unless Ghidra establishes a signed type. Floating reads take
priority over bitwise integer access. Known fields inside mixed objects use
packed structs, retaining original object boundaries; unknown regions stay bytes.
String fragments lacking a terminator use readable character initializers because
VC4 rejects an exact-size unterminated string literal. BSS uses plain declarations.

rc_decode.py reconstructs menus, dialogs, string tables and the original icon as
VC4 RC syntax/ICO. Original resource payloads are retained locally for comparison.
check.py compares each compiled COFF object to the EXE, masks only pointer slots,
and requires a relocation at each slot. All compiled resources compare exactly,
including type, name and language. It enforces the original coverage counts.

Coverage: 7,885 objects, 255,153 bytes (129,142 initialized, 126,011 BSS), and 7,032
symbolic pointer relocations. Object categories: 1,242 strings, 133 float/double,
321 integer, 2,376 unknown byte arrays, 3,813 pointer-bearing structures. Pointer
structures may also contain typed scalar fields. Skipped: 75,887 bytes in the
331,040 virtual-byte .rdata/.data ranges; .idata separately excludes 5,374 bytes.
Resources: 19 entries, 7,088 payload bytes. File-alignment padding is not payload.

Ambiguities: 152,295 emitted bytes lack a classified code-reference root (mostly
BSS); ambiguous.tsv records these conservatively retained ranges. Target names
are stable address-derived identifiers. function_targets.tsv lists all 5,859 code
addresses referenced by input relocations, including skipped tables and interior
EH/jump labels. fn_ signatures are placeholders; a complete link must supply
matching code labels and skipped-data/import dependencies. This extraction unit
compiles independently; it is not a complete executable.
