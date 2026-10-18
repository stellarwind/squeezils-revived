#!/bin/bash
# Mechanical mapping pass. Build first with: bash qvlib/build.sh --parts
set -eu
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
PYTHON="${PYTHON:-/c/Users/jans.margevics/AppData/Local/Programs/Python/Python315/python.exe}"
perl "$HERE/tools/corpus.pl" "$ROOT" > "$HERE/obj/corpus.log" 2>&1
perl "$HERE/tools/corpus.pl" "$ROOT" parts > "$HERE/obj/corpus-parts.log" 2>&1
perl "$HERE/tools/static.pl" "$ROOT"
"$PYTHON" "$HERE/tools/offsets.py" --parts
"$PYTHON" "$HERE/tools/map.py" --parts
perl "$HERE/tools/outside.pl" "$ROOT"
