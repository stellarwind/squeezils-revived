// Original.h: everything the reconstructed rendering code still takes from the original game binary (SQZ.EXE).
//
// This is the whole boundary between the reconstruction and the rest of the game. Every function here is called through its
// address in the original (see hook/hook.cpp, which defines them); once the code behind it is reconstructed, its entry moves
// out of this list. Nothing else in src/ refers to an original address, so this list is exactly what a standalone build would
// still have to provide.
#pragma once
#include <windows.h>
#include <stddef.h>

class Renderer;
class Scene;
struct IDirect3DRMTexture;

// ---- memory ----
// Objects the original code frees itself must come from the heap it frees them to.
void *Lib_Malloc(size_t n);                // @004904c0 the C runtime's malloc (GIF pixels, sky faces)
void  Lib_Free(void *p);                   // @00490540 and its free
void *GameAlloc(size_t n);                 // @00448350 the game's allocator wrapper (texture images)
void  GameFree(void *p);                   // @00448360
void *Operator_New(size_t n);              // @0048ea00 operator new (Warp, PZMorphMesh, Scene)
void  Operator_Delete(void *p);            // @0048e9f0
void *SqgAlloc(size_t n);                  // calls the function pointer stored at 0x4c0f78 (model data)
void  SqgFree(void *p);                    // calls the function pointer stored at 0x4c0f80

// ---- reporting ----
const char *HResultToString(HRESULT hr);   // @00478430 text for a DirectDraw / Direct3D result code
int   PrepareForDialog();                  // @004753c0 does nothing, returns 0 (called before the message boxes)
void  SqgError(const char *format, ...);   // @00473710 printf style error report for model files
void  NumericError();                      // @0041eb80 "number out of range" report used by the sky's fixed point conversion

// ---- game state ----
Renderer *GetRenderer();                   // the global at 0x4c75dc
Scene    *GetScene();                      // the global at 0x4c75e0
typedef IDirect3DRMTexture *(*SqgTextureLookupFn)(const char *name);
SqgTextureLookupFn SqgGetTextureLookup();  // the pointer stored at 0x4c0f84 (NULL until the game sets it)

// ---- data ----
// The shared empty string (one NUL byte) the String class hands out for an unset string and copies into new reps.
static const char *const kEmptyString = (const char *)0x4b9488;

// ---- vtables ----
// The vtable of Bitmap (Bitmap.h). Bitmaps are also made and used by original code, which calls the same virtual methods.
static void *const kBitmapVtable = (void *)0x4a1060;

// ---- callbacks ----
// The window hook's callback (0x476630): the registry in ListenerTable holds it so the original window code can reach the
// Renderer. It is original code that has not been reconstructed.
static void *const kWindowCallback = (void *)0x476630;
