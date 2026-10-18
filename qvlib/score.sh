#!/bin/bash
# Run once after mapping. Detailed diffs stay in ignored obj/match.log.
set -eu
HERE="$(cd "$(dirname "$0")" && pwd)"
PYTHON="${PYTHON:-/c/Users/jans.margevics/AppData/Local/Programs/Python/Python315/python.exe}"
"$PYTHON" "$HERE/tools/prepare-score.py"
FS_OUT="$HERE/obj/match" FS_DIFF_LINES=4 bash "$HERE/obj/harness/match.sh" "$HERE/targets_qvlib.txt" "$HERE"/obj/parts/*.cpp > "$HERE/obj/match.log" 2>&1
cp "$HERE/obj/match/scores.txt" "$HERE/scores.txt"
echo "Saved scores.txt; details in obj/match.log"
