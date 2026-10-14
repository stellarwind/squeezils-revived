# Squeezil decomp

Decompilation of **Squeezils** (Protozoa, 1997): readable C++ for `SQZ.EXE`, checked byte-for-byte against the original with Visual C++ 4.1 + DirectX 2 SDK.

## Current matching

`███████████░░░░░░░░░░░░░░░░░░░` **38.1%**

**1757 of 4615** game-logic functions (0x401000-0x47e600) are byte-exact (30.1% of the code bytes).

## Map of the EXE

```
SQZ.EXE   Win32 executable, Visual C++ 4.1, 1996
┌──────────────────────────────────────────────────────────────────┐
│ .text    0x401000 - 0x49cfa0     CODE   638 KB                   │
│ ┌──────────────────────────────────────────────────────────────┐ │
│ │ ◆ MAIN GAME LOGIC            0x401000 - 0x47e600   502 KB    │ │
│ │   Protozoa's own code: souls, behaviours, physics, scenes,   │ │
│ │   menus, rendering glue.  4,615 functions.                   │ │
│ │   GOAL: byte-match.   ████████░░░░░░░░░░░░ 38.1%             │ │
│ ├──────────────────────────────────────────────────────────────┤ │
│ │ ◇ QvLib (VRML importer)      0x47e600 - 0x48e000    63 KB    │ │
│ │   3rd-party parser, linked in.  Only its OUTPUT must match.  │ │
│ ├──────────────────────────────────────────────────────────────┤ │
│ │ ✗ C RUNTIME (LIBC.LIB)       0x48e000 - 0x49d000    60 KB    │ │
│ │   Microsoft's malloc, printf, startup. Re-linked as is.      │ │
│ └──────────────────────────────────────────────────────────────┘ │
│ .rdata   0x49d000   111 KB   strings, vtables, constant tables   │
│ .data    0x4b9000   215 KB   globals (53 KB initialised)         │
│ .idata   0x4ef000     5 KB   import table (DLL function names)   │
│ .rsrc    0x4f1000     8 KB   resources                           │
│ .reloc   0x4f3000    56 KB   relocations (linker bookkeeping)    │
└──────────────────────────────────────────────────────────────────┘
      │  imports, resolved at run time
      ▼  NOT part of the EXE, we never touch these
  D3DRM.DLL   DDRAW.DLL   KERNEL32.DLL   USER32.DLL   WINMM.DLL ...
```

## Leftovers: 2858 functions

| State | Functions |
|---|---|
| Nearly there (1-4 differing instructions) | 6 |
| Close (5-20) | 14 |
| Far (more than 20) | 44 |
| Not started | 2794 |

_Updated Wed Oct 14 14:33:25 2026. Not counted: the VRML importer (QvLib), which is only checked by its output, and the Microsoft C runtime, which is linked from the original LIBC.LIB. Game files are not included._
