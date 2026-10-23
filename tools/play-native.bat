@echo off
setlocal
set "SQZ_RENDERER=native"
set "SQZ_TRACE=0"
set "SQZ_SDL_WINDOW=1"
start "" /D "%~dp0" "%~dp0SQZ.EXE"
