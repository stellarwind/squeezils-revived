#!/bin/bash
# Run once after mapping. Detailed diffs stay in ignored obj/match.log.
set -eu
HERE="$(cd "$(dirname "$0")" && pwd)"
PYTHON="${PYTHON:-/c/Users/jans.margevics/AppData/Local/Programs/Python/Python315/python.exe}"
"$PYTHON" "$HERE/tools/prepare-score.py"
# Derive the -I paths from this checkout; FS_FLAGS overrides the targets file's #FLAGS line in match.sh.
M="$(cygpath -m "$HERE")"
export FS_FLAGS="-GX -Gy -O2 -G3 -ML -D__ANSI_CPP__ -I$M/shim -I$M/include -I$M/src"
FS_OUT="$HERE/obj/match" FS_DIFF_LINES=4 bash "$HERE/obj/harness/match.sh" "$HERE/targets_qvlib.txt" "$HERE"/obj/parts/*.cpp > "$HERE/obj/match.log" 2>&1
cp "$HERE/obj/match/scores.txt" "$HERE/scores.txt"
echo "Saved scores.txt; details in obj/match.log"
