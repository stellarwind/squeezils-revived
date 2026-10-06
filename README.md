# squeezils-revived

A work in progress to revive the old classic, **Squeezils**: a 1997 3D action game by Protozoa, published by Inscape. 

Game files are not distributed here. The game is abandonware and easy to find, but you need to bring your own files.

## Install
1. Copy `ddraw.dll` into the folder that contains `SQZ.EXE` and `d3drm.dll`.
2. Launch `SQZ.EXE`

Get the built DLL from the Releases or build it yourself (32-bit MinGW-w64 required):

```
gcc -shared -static -m32 -s -o ddraw.dll ddraw.c ddraw.def -luser32
```

## Status
- Launching and the "depth surface failed to create" error are fixed.
- 16bpp color compatibility mode is silently forced because I have no energy to fight D3D retained mode that produces magenta in some color mixing cases (GUI transparent blend, backgroound plane etc)

## Next steps
- Fix movie playback
- Fix GUI font blend
- Remove D3D retained mode and add implement modern graphics API calls

## License
MIT, for the code in this repository only. The game and its assets belong to their respective owners.
