# src: readable C++ reconstruction

Hand-translated from the original game binary, function by function, with real names and the original data layout.
It is not byte-identical to the original; the goal is source that behaves the same and recompiles into something very similar.
Every function here has been verified inside the real, unmodified game with `hook/` (see below).

## Contents
- `Renderer.h/.cpp`: DirectDraw + Direct3D Retained Mode setup (display modes, windowing, device selection, surfaces, palettes, frames, viewport).
- `ListenerTable.h/.cpp`: the window object's registry of 16 callback/context pairs.
- `SqgModel.h/.cpp`: the `.sqg` model format (vertices, face lists, groups, morph deltas): the loader, the `MeshBuilder` frame builder the game uses for its models, the `Mesh` builder for morphing parts and the per-frame morph update.
- `Warp.h/.cpp`: the sky object. Loads the six-view cube map GIFs and, each frame, picks the cube face the camera looks at and drives the software renderer.
- `Scene.h/.cpp`: the per-frame draw path on top of the renderer, complete (z-buffer and back-buffer clear, `Viewport::Render`, present blit, scene frames, decal sprites, frame transform helpers).
- `LayoutCheck.cpp`: compile-time asserts that `Renderer` has the original field offsets and size (0x9cc). Also checks `Scene` (0x1c bytes).

## Not reconstructed yet
- The software renderer behind the sky (`0x48bb30`-`0x48d300`) and the GIF decoder (`0x479290`-`0x479b70`): `Warp` calls the original code for them. The renderer only supports 8 and 16 bit output.
- `Scene::SetQuality` and `Warp::Resize` never ran in a normal session, so they are unverified.
- `CheckDesktopDepth`, the `Error`/`FatalError` boxes' guessed title/strings and a few trivial setters are untested.

## Syntax check
Needs the DirectX 7 SDK headers (not included; set `DX7_INCLUDE`):
```
g++ -m32 -std=c++17 -fsyntax-only -Wno-invalid-offsetof -Wno-deprecated-enum-enum-conversion -I %DX7_INCLUDE% Renderer.cpp ListenerTable.cpp Scene.cpp Warp.cpp SqgModel.cpp LayoutCheck.cpp
```
