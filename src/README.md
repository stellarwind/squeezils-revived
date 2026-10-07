# src: readable C++ reconstruction

Hand-translated from the original game binary, function by function, with real names and the original data layout.
It is not byte-identical to the original; the goal is source that behaves the same and recompiles into something very similar.
Every function here has been verified inside the real, unmodified game with `hook/` (see below), except the ones listed under "Not verified in play".

## Contents
- `Renderer.h/.cpp`: DirectDraw + Direct3D Retained Mode setup (display modes, windowing, device selection, surfaces, palettes, frames, viewport).
- `ListenerTable.h/.cpp`: the window object's registry of 16 callback/context pairs.
- `BodyPart.h/.cpp`: the character classes' D3DRM side: `BodyPart` (places its frame from a pose and applies the morph weights, builds its mesh) and `AnimatedBody` (root frame transform, building and tearing down its parts). The animation timing and blending is game logic and not reconstructed.
- `Texture.h/.cpp`: GIF to D3DRM texture: builds the `D3DRMIMAGE` and palette, makes the transparent colour palette index 0, calls `CreateTexture`.
- `SqgModel.h/.cpp`: the `.sqg` model format (vertices, face lists, groups, morph deltas): the loader, the `MeshBuilder` frame builder the game uses for its models, the `Mesh` builder for morphing parts and the per-frame morph update.
- `Warp.h/.cpp`: the sky object. Loads the six-view cube map GIFs and, each frame, picks the cube face the camera looks at and drives the software renderer.
- `Sky.h/.cpp`: `SkyRenderer`, the software renderer behind the sky: per-row viewing rays projected into the fisheye cube faces, then a parabola per row walked in fixed point (8 and 16 bit output). Only the mode the game uses.
- `Gif.h/.cpp`: the game's GIF reader (header, global palette, LZW decoder) giving palette indices.
- `Image.h/.cpp`: `ImageLoad` (GIF to pixels in the display format, rows padded to 4 bytes) and `ImageConvertRow` (palette indices to 8/16/24/32 bit through the colour masks). Used by Texture and Warp.
- `Scene.h/.cpp`: the per-frame draw path on top of the renderer, complete (z-buffer and back-buffer clear, `Viewport::Render`, present blit, scene frames, decal sprites, frame transform helpers).
- `LayoutCheck.cpp`: compile-time asserts that the structs have the original field offsets and sizes (`Renderer` 0x9cc, `Scene`, `Warp`, `WarpFace`, `BodyPart`/`AnimatedBody`/`BodyPose`, the `Sqg` structs, `DisplayFormat`).

## Not reconstructed
- The sky renderer's modes 0 and 2 (the game only uses mode 1) and its least-squares table.
- The original binary is still used for everything else: the game logic, and the modules still reach each other through the original addresses
  (for example `Warp` calls the sky library and image loader there). The hook redirects those calls to our code.

## Not verified in play
- `Scene::SetQuality`, `Warp::Resize` and `SkyRenderer::Resize` never ran in a normal session (checked against the disassembly only).
- `CheckDesktopDepth` and a few trivial setters are untested. The windowed 8 bit palette paths need an 8 bit desktop.

## Syntax check
Needs the DirectX 7 SDK headers (not included; set `DX7_INCLUDE`):
```
g++ -m32 -std=c++17 -fsyntax-only -Wno-invalid-offsetof -Wno-deprecated-enum-enum-conversion -I %DX7_INCLUDE% Renderer.cpp ListenerTable.cpp Scene.cpp Warp.cpp SqgModel.cpp Texture.cpp BodyPart.cpp Sky.cpp Gif.cpp Image.cpp LayoutCheck.cpp
```
