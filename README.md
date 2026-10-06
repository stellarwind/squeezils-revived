# squeezils-revived

A work in progress to revive the old classic, **Squeezils**: a 1997 3D action game by Protozoa, published by Inscape. 

Game files are not distributed here. The game is abandonware and easy to find, but you need to bring your own files.

## Install
1. Copy `ddraw.dll` into the folder that contains `SQZ.EXE` and `d3drm.dll`.
2. Set `SQZ.EXE` to Windows 98 compatibility mode and 16-bit colors.

Get the built DLL from the Releases or build it yourself (32-bit MinGW-w64 required):

```
gcc -shared -static -m32 -s -o ddraw.dll ddraw.c ddraw.def -luser32
```

## Status
Only launching and the "depth surface failed to create" error are fixed so far.

## Next steps
- Remove the 16-bit color requirement
- Fix movie playback
- Add modern window creation and modern graphics API support

## License
MIT, for the code in this repository only. The game and its assets belong to their respective owners.
