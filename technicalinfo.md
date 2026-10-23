# Squeezil forwarding runtime

The 32-bit `ddraw.dll` shim routes through `sqz-runtime.dll`, which also intercepts
the game's imported `Direct3DRMCreate` factory. The game's original `d3drm.dll`
stays in place. Original DirectDraw/Direct3DRM implementations still render.
The default SDL3 platform path owns a resizable native window and presents a
fixed-resolution virtual game primary with aspect-correct scaling.

The retained scene is rendered by SDL3 GPU / Direct3D12 by default.

Requirements: Windows, Python 3, and i686-w64-mingw32 GCC on PATH. Put the supplied
game ZIP and original DLLs in `../SqueezilsGameFiles` relative to this checkout.

```powershell
python tools/build.py --stage --sdl
python tools/test.py
pwsh -File tools/play.ps1
```

Build output, downloaded binaries, extracted assets and per-process traces are
under `build/forward` and are not committed. The build pins official SDL 3.4.18
x86 archives and verifies published SHA-256 digests. Provenance is recorded in
`third_party/SDL3/source.json`; its license is staged beside SDL3.dll.

`build/forward/squeezils-patch.zip` contains only `ddraw.dll`, `sqz-runtime.dll`,
`SDL3.dll`, SDL's license and the native `d3drm.dll` dependency. Extract it beside
the executable. Windows DirectDraw is loaded by absolute system path. The runtime
reports a 16-bit Z-buffer when its legacy GetCaps depth mask is empty, replacing
the old compatibility wrapper's depth-initialization fix.
The separately built `d3drm.dll` proxy and `legacy` directory are used only by
export-shim tests and are not part of the patch package.

SDL is enabled when `SQZ_SDL_WINDOW` is unset; `SQZ_SDL_WINDOW=0` opts out.
`tools/play.ps1 -OriginalWindow` also retains original window creation. The launch
script removes inherited 16BITCOLOR/256COLOR environment tokens for its child.
For ordinary launch, disable Reduced color mode in the executable's Windows
Compatibility settings; existing saved settings are not changed by the runtime.
The runtime does not restart the game. The original executable on disk stays unchanged.

The source build instruments native vtable entries inside the test process,
preserving object/vtable addresses, COM identity and original callbacks. There
is no GPU swapchain. Unknown interfaces and failed
surface recovery remain visible in the trace rather than silently succeeding.

Two narrowly scoped color fixes are enabled at build time with
`SQZ_FIX_16BPP_RGB_INDEX_REMAP` and `SQZ_FIX_16BPP_DESKTOP_GATE`. Whole-function
fingerprints check the known original code before process-memory edits. The
texture loader branches through an x86 guard that retains its index-remapping
path only for indexed 8bpp images. The desktop gate accepts depths up to 32
without falsifying GetDeviceCaps/GetDisplayMode or the actual surface formats.
Set `SQZ_FIX_RGB_REMAP=0` to compare with the original remapping while retaining
32bpp startup acceptance; `SQZ_ALLOW_32BPP=0` preserves the original depth gate.
Diagnostic logs report installation status and actual image/surface formats.

The 32bpp build also enables `SQZ_FIX_16BPP_COLORKEY_MASK` for the original
HUD key helper's shift-by-32 bug, and `SQZ_FIX_16BPP_WARP_BRIDGE` for the CPU
sky renderer, which only accepts 8/16bpp. Warp images and their private output
surface use RGB565; a scoped GDI StretchBlt converts/scales that surface into
the 32-bit frame. All other color surfaces remain at the actual display format.
Native tests cover original mask instructions and real RGB565-to-32 pixel output.

SDL3 GPU rendering is the default; `SQZ_RENDERER=native` selects native RM
rendering for comparison. The Viewport::Render hook reads live frame/mesh/decal
data, uploads the existing sky background, renders to an offscreen target and
synchronously reads back into the original DD surface so HUD/GDI composition
continues. Texture sampling is bilinear with premultiplied cutout filtering.

Lights are collected from native RM frames before geometry extraction. Ambient
colors and directional frame +Z directions are evaluated in camera coordinates.
Vertex normals use the inverse-transpose frame transform. The vertex shader
evaluates Lambert diffuse lighting and multiplies group color; the rasterizer
smoothly interpolates the result for Gouraud shading. Point
lights support range/attenuation; enable-frame scope and unlit/flat/Gouraud group
quality are respected. Decals remain unlit. Specular, fog, material inheritance,
translucent sorting and exact RM render-quality equivalence remain unimplemented.

Run `tools/play.ps1` locally or launch the patched SQZ.EXE directly. A usable SDL
Direct3D12 backend and Windows d3dcompiler_47.dll are required. A small first-scene
result file distinguishes successful replacement from initialization/extraction
failure. Unsupported visuals fail explicitly. The Windows 11 scene test has been
confirmed by the user; complete game coverage remains to be tested.

Tests cover native forwarding vs original DLLs, export names/ordinals, scene and
mesh APIs, callbacks, float ABI, reference concurrency, surface recovery, pointer
geometry, and a real hidden SDL-window/virtual-primary resize checkpoint.
Game-window GDI text joins the virtual primary, including nested DC leases for
the HUD timer. Canvas writes are coalesced for presentation, and resize/paint
events schedule cached redraws without requiring another gameplay/UI action.
The completed canvas is scaled through GDI into the SDL HWND to avoid the slow
DirectDraw fullscreen stretch under 16-bit compatibility. Native DD/RM still
render the canvas; the DirectDraw presentation path remains a failure fallback.
Set `SQZ_PRESENT_BENCH=1` with the same launch environment to run the optional
visible size-comparison benchmark in `build/forward/probe/window_probe.exe`
after compiling it with `tools/test.py`.
Manual gameplay, focus changes and fullscreen transitions need playtesting.

Project evidence and current limitations live in the workspace's
`../META/FORWARDING_SDL_2026-10-22.md`. Do not replace DLLs while the test game is
running. The files in test `legacy` folders are private dependencies, not redistributable
source artifacts. Normal launches do not create logs. Set `SQZ_TRACE=1` only
when diagnosing a problem; detailed per-call tracing has substantial overhead.

Native color-source verification: the original RM DLL ignores per-vertex source
colors when rendering these Mesh groups and uses SetGroupColor instead. The GPU
extractor follows that behavior. A native offscreen fixture checks white/blue/
gray groups with contradictory and mixed source colors, plus a Gouraud gradient.
RGB565 ambient/directional shading samples agree within one component step;
complete scene and render-state equivalence is not claimed.

HUD DPI: the game converts its INI font point sizes using GetDeviceCaps(LOGPIXELSY)
before creating GDI fonts. The import hook returns 96 for LOGPIXELSX/Y only on the
logical canvas DC and the game window DC used before the primary is available.
The full canvas is scaled at presentation, so HUD text no longer scales twice at
200% Windows UI scaling. Other device metrics and unrelated DCs remain native.
A fixture substitutes 96/144/192/288 host DPI and checks identical font extents;
the window integration test checks the hooked canvas reports 96 DPI.

Mesh culling: the native offscreen oracle discards a reverse-wound triangle.
The GPU mesh pipeline now culls back faces with clockwise front-face winding;
a separate decal pipeline keeps billboards double-sided. A regression compares
both native/GPU front-facing and reversed geometry. No depth bias is applied.

Placed frame visuals: resource frames attached through AddVisual need the
placement frame's transform. Extraction composes local GetTransform matrices
along the traversed path instead of relying on world transforms initialized by
native Render. Visitation is keyed by frame identity and placement basis, so
shared resources render at each placement. Tests extract the original nut/spit
assets before any native rendering and verify shared sprite placements.

Billboard sizing: native RM SetDecalSize stores world-space dimensions; inherited
frame scale affects anchor placement but does not resize the billboard. A native
oracle at frame scales 1, 2, (3,.5,1), and .5 produces the same 256 covered pixels.
GPU extraction now matches, avoiding oversized overworld tree sprites.

Testing saves: original region/place serializers 0x4188c0/0x439db0 write the same
ASCII v1.1 header, region/location pair and Inventory serializer 0x416600 data:
name, scalar, six four-integer region rows, then inventory count/items. The
original Inventory constructor 0x415ef0 initializes its scalar to -1. Generated
profiles retain that scalar, unlock the six region rows and use an empty
inventory. The loader 0x439b70 creates RegionSimulation (0x416e50), even when
loading a save written inside a level: no mid-level position is serialized.
The generator enumerates LocationList place definitions and validates each
Destination's PLACE.INI/LevelFile before writing short alphanumeric filenames.

Save creation: the original dialog's helper 0x406dc0 calls CreateFileA with
CREATE_NEW and handles overwrite confirmation itself; it never creates the save
directory. Staging now preserves empty archive directories, including SQZ/SAVE.
The EXE's CreateFileA import also retries a failed save creation after creating
that exact directory beside the executable, only for ERROR_PATH_NOT_FOUND.
Existing-file and access-denied errors retain the original behavior. The runtime
does not redirect saves or modify their format. The save compatibility fixture
checks creation from a missing directory, saved contents, existing-file protection,
and an unrelated missing path.

Level transitions: renderer helpers 0x4764c0/0x4765d0 rewrite window styles and
call SetWindowPos to center/resize the original window. SDL-window EXE imports
preserve the native position/size and minimize/maximize style bits instead.
ShowWindow(SW_SHOWNORMAL) becomes SW_SHOW when already maximized. SDL and Windows
continue to handle user resizing and title-bar actions directly; the logical
render canvas still follows the game's display-mode dimensions. Other windows
and SDL-disabled runs retain their native API behavior.

Overworld placement precision: recovering camera axes by subtracting transformed
unit points rounded their components at large camera translations. Multiplying
those axes by distant object coordinates amplified the error near the camera.
Extraction now composes root/camera parent matrices and inverts the camera's
linear matrix in double precision, keeping translation separate from directions.
The original retained transforms and simulation are unchanged. The scene fixture
uses coordinates around (522, 89, 547), small camera movements and a transformed
scene root; its maximum position error against direct native point transforms is
below 0.0002 world units (observed 0.0000551). The initial reproduction with the
old unit-point method measured 0.02861691 world units on the X axis alone.

Optional CPU stage profiling: launch with SQZ_GPU_PROFILE=1 to aggregate Render
stages during process seconds 15 through 25. The runtime writes one
gpu-profile-PID.json beside the EXE after that interval. Normal launches disable
the profiler. Stages separate extraction, CPU background conversion/upload,
texture resolution, vertex setup, draw/download command encoding, submission,
existing fence wait, readback mapping and CPU output conversion. Command encoding
is CPU time; fence wait includes outstanding GPU work and is not a GPU timestamp.
The original HUD drawn after Viewport::Render is outside these profiler stages.

Texture caching compares packed source rows and indexed palettes with retained
byte snapshots before allocating/converting RGBA. Format masks, dimensions,
color-key and transparency changes invalidate the cache too. Each texture/decal
pair is resolved once per draw, so repeated batches reuse its binding. Unchanged
textures retain their GPU allocation without conversion or upload. The source
scan remains necessary because native image buffers can be edited directly;
this does not yet rely on dirty notifications. Snapshots and native references
are released with the GPU cache. Profiler reports include source-check,
conversion and reuse counts. Tests cover warm/shared bindings and live pixel,
palette, color-key and transparency edits through the native RM image.

The scene color target and its pipelines use BGRA8. Standard 32-bit DirectDraw
RGB masks (00ff0000/0000ff00/000000ff, unused alpha byte) already have that byte
layout, so background upload preparation and readback writeback use pitched row
copies. The original CPU warp and GDI HUD ordering are retained. RGB555, RGB565
and uncommon 32-bit masks use the generic conversion fallback with BGRA channel
ordering. Sampled material textures remain RGBA8; the GPU handles target channel
layout. The frame-pixel fixture verifies primary colors, padding preservation and
negative pitch in all four formats; the native/GPU oracle checks actual draws.

Local geometry caching: mesh groups retain triangulated positions, UVs and
normals in persistent CPU/GPU buffers. Per-frame traversal reads current frame,
camera, light and group state; per-draw vertex uniforms carry object-to-camera
and normal transforms, group color and a 256-light scope mask. A shared view
uniform carries projection and ambient/directional/point lights. Moving props
therefore update transforms without rebuilding or uploading their meshes.

Successful native mesh/builder mutation observers invalidate cached owners.
Builder face edits conservatively invalidate cached builders. Changed groups,
including Squeezil morphs, rebuild and upload; unchanged groups keep their GPU
contents. All changed groups and CPU billboard quads share one transfer buffer
and one copy pass per frame. Cache owners retain native references; cache data
and GPU buffers are released with the last bound retained device. Unhooked
oracle fixtures compare source geometry to detect edits without observers.

The CPU projected-vertex path remains a test reference. Regression fixtures
compare cached draws for camera/prop movement, morph vertices/normals, flat and
Gouraud shading, scaled normals, moving lights and point-light scope. The stock
RM oracle runs against both paths, including original nut/spit assets and
builder geometry. DLL integration tests verify real observer invalidation for
mesh SetVertices and builder SetVertex. Scene traversal and decal quad generation
remain per-frame; spatial culling is not part of this change.
