param([switch]$OriginalWindow)
$ErrorActionPreference = 'Stop'
$gameDirectory = Join-Path $PSScriptRoot '../build/forward/game'
$priorWindow = $env:SQZ_SDL_WINDOW
$priorCompat = $env:__COMPAT_LAYER
try {
    $env:SQZ_SDL_WINDOW = if ($OriginalWindow) { '0' } else { '1' }
    $env:__COMPAT_LAYER = '16BITCOLOR'
    Start-Process -FilePath (Join-Path $gameDirectory 'SQZ.EXE') -WorkingDirectory $gameDirectory -WindowStyle Normal
} finally {
    $env:SQZ_SDL_WINDOW = $priorWindow
    $env:__COMPAT_LAYER = $priorCompat
}
