# hook: runtime validation of the reconstructed code

`sqzhook.dll` is loaded by the proxy `ddraw.dll` (it is optional: if the file is absent nothing happens). It detours functions in the
original `SQZ.EXE` in memory (the EXE on disk is never modified) and either checks or replaces them with the code in `../src`.

`SQZ_HOOK_MODE`: `compare` (default; run the original, run ours on a snapshot, log differences), `replace`, or `off`.
`SQZ_REPLACE`: comma list of heavy functions to replace with our code: `quality planes frames viewport enum surfaces shutdown qdm enterdisplay uploadpal setpal init fade` the `Scene` ones `swarp sinit sbegin srender spresent squality sxform schild sdel cframe cframeu cdecal dframe gmatrix sload scam sview fparent fvisual fchild fdel`, the `Warp` ones `wctor wdtor winit woutput wresize wupdate wload`, the model ones `sqgload sqgmesh sqgmorph sqgframe` the texture one `texgif`, the lights and frame helpers `lightamb lightdir fxform fdecal quad`, the character ones `bpapply bpbuild abroot abbuild abdestroy`, the teardown ones `pzdestroy sdtor`, the image loading ones `img` (`imgcmp` compares), and the sky renderer library `sky` (`skycmp` runs the original first, then ours, and compares return values, tables and drawn pixels)
(`enumcmp` / `qdmcmp` run those two in compare mode). The log is `sqzhook_log.txt`; an access violation writes a `CRASH` block with registers and stack.
Both settings can also be given as `mode=` and `replace=` lines in a `sqzhook.cfg` next to the DLL, used when the environment variables are empty (they did not reach the game process when a compatibility layer was applied to `SQZ.EXE`). The log starts with the process ID and exe path and is written next to the DLL.

Lessons baked into the engine and checks:
- Decompilers often show `void` for functions that return a value in EAX; check the callers (hook wrappers pass the original's return through).
- A detour must not steal bytes that are a branch target (loop back-edge); `check_hooks.sh` scans for that (it only looks at branches within 0x400 bytes after each hooked address, and needs `objdump -d -M intel SQZ.EXE` saved as `/tmp/sqz.dis`) and tiny functions are copied whole. The id `all` enables every heavy hook. Since the reconstructed modules call each other directly, the original sky library and image loader are no longer reached when `wload winit wupdate sbegin srender` and the texture ids are replaced: listing `sky` and `img` then shows `calls=0` for their hooks in the summary, which proves that nothing calls the original. To compare them against the original again, leave the Warp, Scene and texture ids out so the original code drives them (`skycmp`, `imgcmp`).

Build (32-bit MinGW-w64, DirectX 7 SDK headers via `DX7_INCLUDE`): see `build.bat`.
