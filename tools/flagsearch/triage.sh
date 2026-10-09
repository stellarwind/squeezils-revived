#!/bin/bash
# triage.sh targets.txt src.cpp...   -- run the byte-match and classify every non-zero divergence.
# Same usage as match.sh; the classifications come from triage.pl.
HERE="$(cd "$(dirname "$0")" && pwd)"
T="$1"; shift
[ -f "$T" ] || { echo "usage: triage.sh targets.txt src.cpp..."; exit 1; }
case "$T" in /*) ;; *) T="$(pwd)/$T";; esac
SRC=(); for s in "$@"; do case "$s" in /*) SRC+=("$s");; *) SRC+=("$(pwd)/$s");; esac; done
OUT="${FS_OUT:-$(mktemp -d /tmp/triage_out.XXXXXX)}"; mkdir -p "$OUT"; rm -f "$OUT"/show_*.txt
cd "$HERE"
perl flagsearch.pl -t "$T" -m one -o "$OUT" -j 1 "${SRC[@]}" 2>&1 | tail -n +3 | cut -f1,2,4 | tr ',' '\n' | sed 's/^[0-9]*\t[0-9]*\t//' > "$OUT/scores.txt"
for f in "$OUT"/show_*_ref.txt; do [ -e "$f" ] || continue
  l="${f##*/show_}"; l="${l%_ref.txt}"
  s=$(grep "^$l=" "$OUT/scores.txt" | cut -d= -f2)
  [ "$s" != "0" ] || continue
  perl "$HERE/triage.pl" "$f" "$OUT/show_${l}_got.txt" "$l"
done
