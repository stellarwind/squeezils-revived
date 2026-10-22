param([switch]$OriginalWindow,[switch]$Gpu,[switch]$Native)
$ErrorActionPreference = 'Stop'
$gameDirectory = Join-Path $PSScriptRoot '../build/forward/game'
$priorWindow = $env:SQZ_SDL_WINDOW
$priorCompat = $env:__COMPAT_LAYER
$priorRenderer = $env:SQZ_RENDERER
try {
    $env:SQZ_SDL_WINDOW = if ($OriginalWindow) { '0' } else { '1' }
    $env:SQZ_RENDERER = if ($Native) { 'native' } else { 'sdlgpu' }
    $env:__COMPAT_LAYER = (($priorCompat -split '\s+' | Where-Object { $_ -and $_ -notmatch '^(16BITCOLOR|256COLOR)$' }) -join ' ')
    Start-Process -FilePath (Join-Path $gameDirectory 'SQZ.EXE') -WorkingDirectory $gameDirectory -WindowStyle Normal
} finally {
    $env:SQZ_SDL_WINDOW = $priorWindow
    $env:__COMPAT_LAYER = $priorCompat
    $env:SQZ_RENDERER = $priorRenderer
}
