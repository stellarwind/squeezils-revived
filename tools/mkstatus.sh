#!/usr/bin/env bash
# Refresh and publish only the README progress bar. No builds or byte scoring.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec "${PYTHON:-python}" "$ROOT/tools/progress.py" "$@"
