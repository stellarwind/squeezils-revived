#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
# Optional GHIDRA_INSTALL_DIR and ghidra on PATH select the local installation.
if [[ -z "${GHIDRA_INSTALL_DIR:-}" && -d /d/Decomp/ghidra_12.1.4_PUBLIC ]]; then
    export GHIDRA_INSTALL_DIR='D:/Decomp/ghidra_12.1.4_PUBLIC'
fi
analysis_project="$(cygpath -w "$PWD/analysis")"
if [[ ! -f snapshot.tsv || Snapshot.java -nt snapshot.tsv || ../../SqueezilsGameFiles/SQZ.EXE -nt snapshot.tsv ]]; then
    if [[ ! -d analysis.rep ]]; then
        ghidra import "$(cd ../.. && pwd)/SqueezilsGameFiles/SQZ.EXE" --project "$analysis_project"
    fi
    ghidra script run "$PWD/Snapshot.java" --project "$analysis_project" -- "$PWD/snapshot.tsv"
fi
python extract.py
source ../../tools/flagsearch/env.sh
cl -nologo -c -GX -GR -O2 -G5 -ML -Fodata.obj data.cpp
rc -r -fo resources.res resources.rc
python check.py
