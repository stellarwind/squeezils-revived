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
- `Bitmap.h/.cpp`: `Bitmap`, a DirectDraw offscreen surface for the 2D interface art (HUD icons, digits, load bar): colour keyed blits with retry, and the loader for the game's 8 bit `.bmp` files, which converts to the display format with `ImageConvertRow`. Its seven virtual methods are called by the original code through the original vtable.
- `GameString.h/.cpp`: `String` (the original's `SqzString`), the reference counted, copy-on-write string handle with case-insensitive compares.
- `Container.h`: the game's container templates, `Array`, `PrimitiveArray`, `Set` (a sorted array with binary search), `Table` (a key set plus a parallel value array) and `BufferObjectTable`. One template covers the many per-instantiation copies in the binary. Original code also uses these objects and runs `dynamic_cast` on them, so constructors end by switching to the original vtables (listed in `Original.h`). See `CONTAINERS.md` in the project folder for the class map.
- `IniFile.h/.cpp`: the parsed INI files: the parser, `IniFileSection` (key -> value plus the keys in file order), `IniFileCache` (one parsed file) and the global cache by path (`IniOpen`, `IniCloseAll`).
- `IniReader.h/.cpp`: `IniReader`, the settings cursor (file + current section) the game reads nearly every setting through, including the key and int lists.
- `Original.h`: the boundary with the original binary. Every function or global the code above still takes from `SQZ.EXE` (allocators, the error reporters, the renderer and scene globals, the window callback) is declared there, with its address. Nothing else in `src/` refers to the original, so that list is what a standalone build would still have to provide. The modules call each other directly (`Warp` and `Scene` drive `SkyRenderer`, `Texture` and `Warp` use `ImageLoad`).
- `LayoutCheck.cpp`: compile-time asserts that the structs have the original field offsets and sizes (`Renderer` 0x9cc, `Scene`, `Warp`, `WarpFace`, `BodyPart`/`AnimatedBody`/`BodyPose`, the `Sqg` structs, `DisplayFormat`).

- `VectorMath.cpp`: `Slerp` (@0047da80) and `MatrixToAngles` (@0047e500), self-contained maths with no game state. Written ahead of time: **not in `hook/build.bat`, not hooked, not linked**, checked only by reading the disassembly and a small numeric test (not compared against the original in play). `Original.h` still declares `MatrixToAngles` as an original call; switch it to this one when linking.

## Not reconstructed
- The sky renderer's modes 0 and 2 (the game only uses mode 1) and its least-squares table.
- The original binary is still used for everything else: the game logic, which calls into this code through the original addresses (the hook redirects those calls).

## Not verified in play
- `Scene::SetQuality`, `Warp::Resize` and `SkyRenderer::Resize` never ran in a normal session (checked against the disassembly only).
- `CheckDesktopDepth` and a few trivial setters are untested. The windowed 8 bit palette paths need an 8 bit desktop.
- Containers: `Set::RemoveKey`, `Set::Replace`, `Table::RemoveKey` and `Table::IndexOf` were never called by the original code in play. The parser's duplicate-key warning and the missing-`]` header case did not occur in the game's files.

## Syntax check
Needs the DirectX 7 SDK headers (not included; set `DX7_INCLUDE`):
```
g++ -m32 -std=c++17 -fsyntax-only -Wno-invalid-offsetof -Wno-deprecated-enum-enum-conversion -I %DX7_INCLUDE% Renderer.cpp ListenerTable.cpp Scene.cpp Warp.cpp SqgModel.cpp Texture.cpp BodyPart.cpp Sky.cpp Gif.cpp Image.cpp Bitmap.cpp GameString.cpp IniFile.cpp IniReader.cpp LayoutCheck.cpp
```
