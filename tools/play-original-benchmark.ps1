$ErrorActionPreference = 'Stop'
$gameDirectory = Join-Path $PSScriptRoot '../build/native-benchmark'
if (!(Test-Path -LiteralPath (Join-Path $gameDirectory 'benchmark-build.json'))) {
    throw 'Run python tools/prepare_native_benchmark.py first.'
}
$priorCompat = $env:__COMPAT_LAYER
try {
    # The unmodified startup still requires a 16-bit desktop. Use Windows' own
    # reduced-color mode rather than changing that check or adding a DLL shim.
    $env:__COMPAT_LAYER = '16BITCOLOR'
    Start-Process -FilePath (Join-Path $gameDirectory 'SQZ.EXE') -WorkingDirectory $gameDirectory -WindowStyle Normal
} finally {
    $env:__COMPAT_LAYER = $priorCompat
}
