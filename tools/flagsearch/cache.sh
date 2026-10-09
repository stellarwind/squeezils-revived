#!/bin/bash
# cache.sh targets.txt src.cpp...  -- like match.sh, but with a disk-backed compile cache.
# Prints the same "label=score" lines as match.sh. The key covers the targets file, the source files,
# the build env (env.sh, which pins the compiler + SDK + include roots) and FS_FLAGS, so any of those
# changing invalidates the entry. Note: it does NOT hash the shared headers under Repo/bm, so a header-only
# edit needs `rm -rf tools/flagsearch/.cache` (or FS_CACHE=... elsewhere). It also does not cache the diffs.
HERE="$(cd "$(dirname "$0")" && pwd)"
CACHE="${FS_CACHE:-$HERE/.cache}"; mkdir -p "$CACHE"
T="$1"; shift
[ -f "$T" ] || { echo "usage: cache.sh targets.txt src.cpp..."; exit 1; }
case "$T" in /*) ;; *) T="$(pwd)/$T";; esac
SRC=(); for s in "$@"; do case "$s" in /*) SRC+=("$s");; *) SRC+=("$(pwd)/$s");; esac; done

key=$( { cat "$T" "${SRC[@]}"; cat "$HERE/env.sh"; printf '%s' "${FS_FLAGS:-}"; } | md5sum | cut -d' ' -f1 )

if [ -f "$CACHE/$key" ]; then
  cat "$CACHE/$key"
  exit 0
fi

OUT="${FS_OUT:-$(mktemp -d /tmp/cache_out.XXXXXX)}"; mkdir -p "$OUT"; rm -f "$OUT"/show_*.txt
cd "$HERE"
perl flagsearch.pl -t "$T" -m one -o "$OUT" -j 1 "${SRC[@]}" 2>&1 | tail -n +3 | cut -f1,2,4 | tr ',' '\n' | sed 's/^[0-9]*\t[0-9]*\t//' > "$OUT/scores.txt"
cat "$OUT/scores.txt" | tee "$CACHE/$key"
