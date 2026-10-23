@echo off
setlocal
set "SQZ_RENDERER=sdlgpu"
set "SQZ_SDL_WINDOW=1"
start "" /D "%~dp0" "%~dp0SQZ.EXE"
