$ErrorActionPreference = 'Stop'
$gameDirectory = $PSScriptRoot
$priorWindow = $env:SQZ_SDL_WINDOW
$priorCompat = $env:__COMPAT_LAYER
$priorTrace = $env:SQZ_TRACE
try {
    $env:SQZ_SDL_WINDOW = '1'
    $env:SQZ_TRACE = '1'
    $env:__COMPAT_LAYER = (($priorCompat -split '\s+' | Where-Object { $_ -and $_ -notmatch '^(16BITCOLOR|256COLOR)$' }) -join ' ')
    Write-Host 'Play until the magenta leaves and clock/info text are visible, then exit normally.'
    $game = Start-Process -FilePath (Join-Path $gameDirectory 'SQZ.EXE') -WorkingDirectory $gameDirectory -PassThru
    $game.WaitForExit()
    $log = Join-Path $gameDirectory "sqz-diagnostic-$($game.Id).log"
    if (!(Test-Path -LiteralPath $log)) { throw "Diagnostic log was not created: $log" }
    $zip = Join-Path $gameDirectory "squeezils-diagnostics-$($game.Id).zip"
    Compress-Archive -LiteralPath $log -DestinationPath $zip -Force
    Write-Host "Send this file back: $zip"
} catch {
    Write-Host $_ -ForegroundColor Red
} finally {
    $env:SQZ_SDL_WINDOW = $priorWindow
    $env:__COMPAT_LAYER = $priorCompat
    $env:SQZ_TRACE = $priorTrace
    Read-Host 'Press Enter to close'
}
