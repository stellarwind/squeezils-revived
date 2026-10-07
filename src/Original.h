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
void  GameLog(const char *format, ...);    // @00419290 printf style message to the game's log file and console
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

// ---- ini files ----
static const char *const kCurrentIniPath = (const char *)0x4c74d8;        // the game's global current ini file name
static const char *const kDefaultSectionName = (const char *)0x4b9e60;    // "Defaults"
static const char *const kCurrentDirectory = (const char *)0x4bd220;      // "."
static const char *const kIniParsingMessage = (const char *)0x4bd20c;     // "PARSING INI %s...\n"
// "  PARSING ERROR! duplicate entry '%s' in section '%s'.\n    Old text: '%s'.\n    New text: '%s'.\n"
static const char *const kIniDuplicateMessage = (const char *)0x4bd1ac;

// ---- containers ----
// The container templates (Container.h) and the ini file classes (IniFile.h) are reconstructed, but original code builds and
// uses the same objects (and runs dynamic_cast on them), so the objects keep the original vtables. One entry per
// instantiation the reconstruction constructs: the vtable at +0 and, for save/load capable classes, BufferObject's.
class String;
class IniFileSection;
class IniFileCache;
template<class T> class Array;
template<class T, int ID> class PrimitiveArray;
template<class T, int ID> class Set;
template<class K, class V, class VArray, int ID> class Table;
template<class K, class V, class VArray, int ID> class BufferObjectTable;
typedef Table<String, IniFileCache *, Array<IniFileCache *>, 51> IniFileTable;
typedef BufferObjectTable<String, String, PrimitiveArray<String, 3>, 23> IniValueTable;

template<class C> struct OriginalVtables;
template<> struct OriginalVtables<Array<String> >                  { enum : unsigned { primary = 0x4a000c }; };
template<> struct OriginalVtables<Array<int> >                     { enum : unsigned { primary = 0x49ffcc }; };
template<> struct OriginalVtables<Array<IniFileSection *> >        { enum : unsigned { primary = 0x4a0d6c }; };
template<> struct OriginalVtables<Array<IniFileCache *> >          { enum : unsigned { primary = 0x4a0e1c }; };
template<> struct OriginalVtables<PrimitiveArray<String, 3> >      { enum : unsigned { primary = 0x49d50c, bufferObject = 0x49d4f4 }; };
template<> struct OriginalVtables<PrimitiveArray<String, 23> >     { enum : unsigned { primary = 0x4a0334, bufferObject = 0x4a031c }; };
template<> struct OriginalVtables<PrimitiveArray<String, 50> >     { enum : unsigned { primary = 0x4a0cf4, bufferObject = 0x4a0cdc }; };
template<> struct OriginalVtables<PrimitiveArray<String, 51> >     { enum : unsigned { primary = 0x4a0da4, bufferObject = 0x4a0d8c }; };
template<> struct OriginalVtables<PrimitiveArray<int, 1> >         { enum : unsigned { primary = 0x49d5a4, bufferObject = 0x49d58c }; };
template<> struct OriginalVtables<Set<String, 23> >                { enum : unsigned { primary = 0x4a036c, bufferObject = 0x4a0354 }; };
template<> struct OriginalVtables<Set<String, 50> >                { enum : unsigned { primary = 0x4a0d2c, bufferObject = 0x4a0d14 }; };
template<> struct OriginalVtables<Set<String, 51> >                { enum : unsigned { primary = 0x4a0ddc, bufferObject = 0x4a0dc4 }; };
template<> struct OriginalVtables<Table<String, String, PrimitiveArray<String, 3>, 23> >
                                                                   { enum : unsigned { primary = 0x4a0394 }; };
template<> struct OriginalVtables<IniValueTable>                   { enum : unsigned { primary = 0x49d574, bufferObject = 0x49d55c }; };
template<> struct OriginalVtables<Table<String, IniFileSection *, Array<IniFileSection *>, 50> >
                                                                   { enum : unsigned { primary = 0x4a0d54 }; };
template<> struct OriginalVtables<IniFileTable>                    { enum : unsigned { primary = 0x4a0e04 }; };

IniFileTable *GetIniFiles();               // the global at 0x4c7738: every ini file parsed so far, by path (built by the game's startup code)

// ---- the running game ----
void *GetCurrentSimulation();                         // @00420690 the global at 0x4badbc: the simulation (place or region) being run
const char *GetPlaceName(int index);                  // @00420680 + 0041e8c0 the game's list of place folder names
void *DynamicCast(void *obj, const void *from, const void *to);   // @0048f060 __RTDynamicCast with RTTI type descriptors
static const void *const kLocationSimulationType = (const void *)0x4ba1d0;   // .?AVLocationSimulation
static const void *const kPlaceSimulationType    = (const void *)0x4bae38;   // .?AVPlaceSimulation
static const void *const kRegionSimulationType   = (const void *)0x4ba370;   // .?AVRegionSimulation

// ---- application, input and menu state ----
struct ScreenInfo { char pad[0x28]; int width; int height; };       // the object behind the global at 0x4badac (window size)
ScreenInfo *GetScreenInfo();                                          // @004206b0
int   GetKeyCode(const char *name);                                   // @0043ace0 key name ("Return", "F1" ...) -> virtual key
int   GameLoadGame(const char *path);                                 // the current simulation's load-game virtual (slot 10); 0 on failure
HINSTANCE GetInstance();                                              // the global at 0x4c73c8
HWND      GetMainWindow();                                            // the global at 0x4c73cc
static const char *const kGameDirectory = (const char *)0x4c73d8;     // the game folder
static char *const kSaveGameDirectory   = (char *)0x4c6400;           // <exe folder>\sqz\save\ (built by the Menu constructor)
static char *const kLoadGameName        = (char *)0x4c63e8;           // what the load-game dialog picked
static char *const kLoadGameTitle       = (char *)0x4c63d0;
static void *const kLoadGameDialogProc  = (void *)0x406eb0;           // dialog window procedures still in original code
static void *const kAboutDialogProc     = (void *)0x406cb0;
static void *const kQuitDialogProc      = (void *)0x406d60;

// ---- vtables ----
// The vtable of Bitmap (Bitmap.h). Bitmaps are also made and used by original code, which calls the same virtual methods.
static void *const kBitmapVtable = (void *)0x4a1060;
// The vtable of Splash (Splash.h): slot 0 SetProgress, slot 1 Step.
static void *const kSplashVtable = (void *)0x49ed34;
// The vtable of Menu (Menu.h): slot 0 SetSelected, slot 1 OnMouseMove.
static void *const kMenuVtable = (void *)0x49d28c;

// ---- callbacks ----
// The window hook's callback (0x476630): the registry in ListenerTable holds it so the original window code can reach the
// Renderer. It is original code that has not been reconstructed.
static void *const kWindowCallback = (void *)0x476630;
