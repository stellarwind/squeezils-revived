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
struct IDirect3DRMFrame;
struct IDirect3DRM;

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
static char *const kSaveGameName        = (char *)0x4c63d0;           // the name typed in the save-game dialog (same buffer kLoadGameTitle clears)
static char *const kKeyName             = (char *)0x4c6468;           // the key name shown and typed in the "press a key" dialog

// ---- the input object (global 0x4badac, the object GetScreenInfo returns): key and joystick bindings the dialogs edit ----
struct InputSystem;
InputSystem *GetInputSystem();                                          // the global at 0x4badac
int   InputJoystickMode(InputSystem *in);                               // @0043aee0 0 off, 1, 2
int   InputSetJoystickMode(InputSystem *in, int mode);                  // @0043aef0
int   InputJoystickPresent(InputSystem *in);                            // @0043c7a0
int   InputActionName(InputSystem *in, int index, char *line);         // @0043c7b0 text line of binding number index; > 0 when there is one
void  InputRevertBindings(InputSystem *in, int which);                 // @0043ad30
void  InputBindingLabel(InputSystem *in, const char *line, char *keyName);   // @0043ad00
int   InputAssignKey(InputSystem *in, char *line, const char *keyName);      // virtual slot 0
int   InputSetKeySlot(InputSystem *in, char *line, int slot);                // virtual slot 1
int   InputSetButtonSlot(InputSystem *in, char *line, int slot);             // @0043ae70
int   InputSaveBindings(InputSystem *in);                                    // @0043c4d0
void *InputCalibration(InputSystem *in);                                // the joystick calibration object at input+0x10
void  CalibrationStart(void *cal);                                      // @00447880
int   CalibrationCheck(void *cal);                                      // @004479c0 nonzero: skip the calibration run
int   CalibrationRun(void *cal, HWND dlg);                              // @00447f00 nonzero when it worked
int   SimulationCanSave(void *sim);                                     // virtual slot 9 of the current simulation
int   SimulationSave(void *sim, const char *path);                      // virtual slot 11

// the three sensitivity settings the sensitivity dialog edits (floats 0.0 .. 1.0) and the functions that apply them
static float *const kSensitivity1 = (float *)0x4c77c8;                  // set by @0044aa10, slider 1
static float *const kSensitivity2 = (float *)0x4c77a0;                  // set by @0044aab0, slider 2
static float *const kSensitivity3 = (float *)0x4c77cc;                  // set by @0044a940, slider 3
void  SetSensitivity1(float value);
void  SetSensitivity2(float value);
void  SetSensitivity3(float value);

// ---- the panel ----
void *GetPanel();                                                     // @004206c0 the in-game panel object (global 0x4badb0); its ten digit pictures start at +0x428


struct Panel;
void  PanelLoadFonts(Panel *panel);                                   // @0042e7a0 reads the message font settings and makes the font (PanelText.cpp)

BOOL  GoodieCount(const char *name, int *count);                        // the player's count of a goodie by name (SpitNutGoodie ...); FALSE when there is no player
void  StoreSetting(const char *name, void *data, int size);           // the global settings store (object at 0x4c7060, virtual slot 0): remember a value
class Bitmap;
class String;
BOOL  CurrentGoodieName(String *name);                              // the goodie the player has selected, from the player's inventory table (+8); FALSE when there is no player
static int *const kMessageShown = (int *)0x4bbea4;                    // set while the dead / won message is on screen

static int *const kQuestItemsLoaded = (int *)0x4bc01c;                      // set once the left/middle/right key pictures are loaded

// ---- the player ----
void *GetPlayerSoul();                                                // @00420690 + 00414080 the player's actor (a Soul), or NULL
int   SoulMood(void *soul);                                           // the player soul's mood (dynamic_cast to SqueezilPlayerSoul, then its virtual at +0x64)
static const void *const kSoulType                = (const void *)0x4b93e8;   // .?AVSoul
static const void *const kSqueezilPlayerSoulType  = (const void *)0x4b9490;   // .?AVSqueezilPlayerSoul

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

// ---- the panel's remaining original code ----
static int *const kPanelDrawn = (int *)0x4bbe90;                       // set after the panel picture was first drawn
float *PlayerMatrix();                                                 // the player's 4x4 matrix as floats (@00414050 then its virtual slot 3), or NULL
void *PlayerInventory();                                               // the player's inventory table (its four byte entries start at +4: goodie counts by name, quest piece flags), or NULL

// ---- the world (World.h) ----
class World;
World *GetWorld();                                                     // the global at 0x4bada8
// The nav surface is an original class (0x2030 bytes) the world only creates, loads and frees.
void *NavMeshConstruct(void *memory);                                  // @0047aec0 clears the object, returns it
int   NavMeshLoad(void *nav, const char *path);                        // @0047c8e0 nonzero when the file loaded
void  NavMeshDestroy(void *nav);                                       // @0047af00 (the caller frees the memory)
int   LoadPaletteFile(const char *path, unsigned char *rgba);          // @00474ca0 (cdecl) reads a palette into a 256 entry RGBA table, nonzero on success
void  ClearWorldObjects();                                             // @0042d010 frees the level's prop lists

// ---- resource tables (Resources.h) ----
// The texture and geometry tables are the same container templates as the ini files use (Container.h), with other element
// types. The stacks that hold them are globals the game's start-up code built (0x4c7180 and 0x4c71c0).
typedef Table<String, IDirect3DRMTexture *, Array<IDirect3DRMTexture *>, 0> TextureTable;
typedef Table<String, IDirect3DRMFrame *, Array<IDirect3DRMFrame *>, 0>     GeometryTable;
template<> struct OriginalVtables<PrimitiveArray<String, 0> >      { enum : unsigned { primary = 0x4a0764, bufferObject = 0x4a074c }; };
template<> struct OriginalVtables<Set<String, 0> >                 { enum : unsigned { primary = 0x4a079c, bufferObject = 0x4a0784 }; };
template<> struct OriginalVtables<Array<IDirect3DRMTexture *> >    { enum : unsigned { primary = 0x4a07dc }; };
template<> struct OriginalVtables<Array<IDirect3DRMFrame *> >      { enum : unsigned { primary = 0x4a072c }; };
template<> struct OriginalVtables<TextureTable>                    { enum : unsigned { primary = 0x4a07c4 }; };
template<> struct OriginalVtables<GeometryTable>                   { enum : unsigned { primary = 0x4a0714 }; };
template<> struct OriginalVtables<Array<TextureTable *> >          { enum : unsigned { primary = 0x4a06f4 }; };
template<> struct OriginalVtables<Array<GeometryTable *> >         { enum : unsigned { primary = 0x4a06d4 }; };

// ---- animation containers (Animation.h, BodyPart.h) ----
struct Vector3f;
struct PartOrientation;
struct PartPose;
struct PartCycle;
struct BodyPart;
typedef Table<String, PartPose *, Array<PartPose *>, 0>   PoseTableBase;
typedef Table<String, PartCycle *, Array<PartCycle *>, 0> PartCycleTable;
typedef Table<String, BodyPart *, Array<BodyPart *>, 0>   BodyPartTable;
template<> struct OriginalVtables<Array<float> >                   { enum : unsigned { primary = 0x49ffec }; };
template<> struct OriginalVtables<PrimitiveArray<float, 2> >       { enum : unsigned { primary = 0x49d75c, bufferObject = 0x49d744 }; };
template<> struct OriginalVtables<Array<Vector3f> >                { enum : unsigned { primary = 0x4a002c }; };
template<> struct OriginalVtables<PrimitiveArray<Vector3f, 4> >    { enum : unsigned { primary = 0x49d7a4, bufferObject = 0x49d78c }; };
template<> struct OriginalVtables<Array<PartOrientation> >         { enum : unsigned { primary = 0x4a0a2c }; };
template<> struct OriginalVtables<Array<PrimitiveArray<float, 2> *> > { enum : unsigned { primary = 0x4a0a4c }; };
template<> struct OriginalVtables<Array<PartPose *> >              { enum : unsigned { primary = 0x4a0a84 }; };
template<> struct OriginalVtables<PoseTableBase>                       { enum : unsigned { primary = 0x4a0a6c }; };
template<> struct OriginalVtables<Array<BodyPart *> >              { enum : unsigned { primary = 0x4a0abc }; };
template<> struct OriginalVtables<BodyPartTable>                   { enum : unsigned { primary = 0x4a0aa4 }; };
template<> struct OriginalVtables<Array<PartCycle *> >             { enum : unsigned { primary = 0x4a0b14 }; };
template<> struct OriginalVtables<PartCycleTable>                  { enum : unsigned { primary = 0x4a0afc }; };

// The model loader's pieces that are still original code (SqgModel.h has Load, CreateMesh and BuildFrame).
class SqgModel;
SqgModel *SqgModelConstruct(void *memory);                             // @004719d0 clears the 0x2c byte object, returns it
void  SqgModelDestruct(SqgModel *model);                               // @00471a00
int   SqgModelLoadWrl(SqgModel *model, const char *path, int flag);    // @00472480 reads a .wrl file into the model ("sqgify"), nonzero on success
void  SqgModelSave(SqgModel *model, const char *path);                 // @00472b90 writes the model as an .sqg file
void  SqgSetErrorCallback(void (__cdecl *fn)(const char *));           // @004736f0 stores into 0x4c0f88 (the function SqgError reports through)
void  SqgSetTextureLookup(IDirect3DRMTexture *(__cdecl *fn)(const char *));   // @00473700 stores into 0x4c0f84
IDirect3DRMFrame *LoadWrlFrame(IDirect3DRM *d3drm, const char *path, IDirect3DRMTexture *(__cdecl *lookup)(const char *));   // @0047a580 builds a frame straight from a .wrl file
void  WrlSetLogCallback(void (__cdecl *fn)(const char *));             // @0047a790 stores into 0x4c3c4c (its meaning is a guess: the game passes its error logger to both)
void  WrlSetErrorCallback(void (__cdecl *fn)(const char *));           // @0047a7a0 stores into 0x4c3c50

// ---- props (Prop.h, FrameProp.h) ----
struct Prop;
struct ScriptCaller;
template<> struct OriginalVtables<Array<Prop *> >                  { enum : unsigned { primary = 0x4a0874 }; };
template<> struct OriginalVtables<PrimitiveArray<float, 25> >      { enum : unsigned { primary = 0x4a04ac, bufferObject = 0x4a0494 }; };
template<> struct OriginalVtables<Set<float, 25> >                 { enum : unsigned { primary = 0x4a0454, bufferObject = 0x4a043c }; };
template<> struct OriginalVtables<Table<float, String, PrimitiveArray<String, 3>, 25> >
                                                                   { enum : unsigned { primary = 0x4a047c }; };
template<> struct OriginalVtables<BufferObjectTable<float, String, PrimitiveArray<String, 3>, 25> >
                                                                   { enum : unsigned { primary = 0x49dc9c, bufferObject = 0x49dc84 }; };
// The props whose Update runs every frame (the Array<Prop *> at 0x4c71f8). Removing only queues the index (in 0x4c72a8).
void  AddUpdatingProp(Prop *prop);                                     // @0042d180
void  RemoveUpdatingProp(Prop *prop);                                  // @0042d1a0
// The prop template with this name from the level's prop tables (the loader: also reads "name:..." forms). The two numbers are
// not identified; FrameProp passes 0 and 0.
Prop *FindPropTemplate(const char *name, int unknown1, int unknown2);  // @0042c8b0 (cdecl, the name is a String by value)
// The current simulation's actors by index (0 is the player)
void *SimulationActor(int index);                                      // @00414050 (thiscall on the simulation)
void *SimulationSoul(int index);                                       // @00414080
float *ActorMatrix(void *actor);                                       // the actor's virtual slot 3: its 4x4 matrix
float *CameraMatrix();                                                  // @00420690 + @00414150: entry 1 of the simulation's actor table, its actor's 4x4 matrix (the camera; LODProp reads it)
static const void *const kSqueezilSoulType = (const void *)0x4b9368;   // .?AVSqueezilSoul
// The two constructors of the record a prop hands a script function. Both fill it directly when prop is not NULL (all FrameProp
// does); with a NULL prop they look the prop up through the actor, which is not reconstructed.
ScriptCaller *ScriptCallerInit(ScriptCaller *caller, int actorIndex, Prop *prop);               // @004207a0
ScriptCaller *ScriptCallerInitSoul(ScriptCaller *caller, void *soul, int actorIndex, Prop *prop);   // @00420900

// ---- CycleProp's step events (CycleProp.h): a table from a child number to a script function name ----
template<> struct OriginalVtables<PrimitiveArray<int, 24> >        { enum : unsigned { primary = 0x4a03c4, bufferObject = 0x4a03ac }; };
template<> struct OriginalVtables<Set<int, 24> >                   { enum : unsigned { primary = 0x4a03fc, bufferObject = 0x4a03e4 }; };
template<> struct OriginalVtables<Table<int, String, PrimitiveArray<String, 3>, 24> >
                                                                   { enum : unsigned { primary = 0x4a0424 }; };
template<> struct OriginalVtables<BufferObjectTable<int, String, PrimitiveArray<String, 3>, 24> >
                                                                   { enum : unsigned { primary = 0x49dd2c, bufferObject = 0x49dd14 }; };
