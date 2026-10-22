# Squeezil forwarding runtime

32-bit `ddraw.dll` and `d3drm.dll` entry points route through `sqz-runtime.dll`
and log calls while the original DirectDraw/Direct3DRM implementations render.
The optional SDL3 platform path owns a resizable native window and presents a
fixed-resolution virtual game primary with aspect-correct scaling.

This is an experimental forwarding checkpoint, not a modern GPU renderer.

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

`tools/play.ps1 -OriginalWindow` retains original window creation. Both modes
preserve the supplied DirectDraw compatibility wrapper behind our shim; tests
select the system DirectDraw backend explicitly. The launch script scopes the
16-bit-color compatibility setting to the child process. No persistent system
compatibility settings or original executable bytes are changed.

The source build instruments native vtable entries inside the test process,
preserving object/vtable addresses, COM identity and original callbacks. There
is no game-function trampoline or GPU swapchain. Unknown interfaces and failed
surface recovery remain visible in the trace rather than silently succeeding.

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
running. The files in `legacy` are private test dependencies, not redistributable
source artifacts. Detailed per-call tracing has substantial debug overhead.
