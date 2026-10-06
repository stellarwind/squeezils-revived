# hook: runtime validation of the reconstructed code

`sqzhook.dll` is loaded by the proxy `ddraw.dll` (it is optional: if the file is absent nothing happens). It detours functions in the
original `SQZ.EXE` in memory (the EXE on disk is never modified) and either checks or replaces them with the code in `../src`.

`SQZ_HOOK_MODE`: `compare` (default; run the original, run ours on a snapshot, log differences), `replace`, or `off`.
`SQZ_REPLACE`: comma list of heavy functions to replace with our code: `quality planes frames viewport enum surfaces shutdown qdm enterdisplay uploadpal setpal init`
(`enumcmp` / `qdmcmp` run those two in compare mode). The log is `sqzhook_log.txt`; an access violation writes a `CRASH` block with registers and stack.

Lessons baked into the engine and checks:
- Decompilers often show `void` for functions that return a value in EAX; check the callers (hook wrappers pass the original's return through).
- A detour must not steal bytes that are a branch target (loop back-edge); `check_hooks.sh` scans for that and tiny functions are copied whole.

Build (32-bit MinGW-w64, DirectX 7 SDK headers via `DX7_INCLUDE`): see `build.bat`.
