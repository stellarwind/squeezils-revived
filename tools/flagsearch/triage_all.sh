#!/bin/bash
# triage_all.sh -- run the divergence triager over every unit in Repo/bm/manifest.txt.
# Prints, per unit, one classification line for each non-zero function.
cd "$(dirname "$0")/../.." || exit 1
HERE="$(pwd)"
BM="$HERE/Repo/bm"
while read -r tgt rest; do
  [ -n "$tgt" ] || continue
  case "$tgt" in \#*) continue;; esac
  srcs=($rest)
  for i in "${!srcs[@]}"; do srcs[$i]="$BM/${srcs[$i]}"; done
  out=$(FS_OUT="/tmp/tg_$(basename "$(dirname "$tgt")")" "$HERE/tools/flagsearch/triage.sh" "$BM/$tgt" "${srcs[@]}" 2>/dev/null)
  if [ -n "$out" ]; then
    echo "== $tgt =="
    echo "$out"
  fi
done < "$BM/manifest.txt"
