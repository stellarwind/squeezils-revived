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
template<> struct OriginalVtables<PrimitiveArray<String, 21> >     { enum : unsigned { primary = 0x4a0214, bufferObject = 0x4a01fc }; };
template<> struct OriginalVtables<Set<String, 21> >                { enum : unsigned { primary = 0x4a024c, bufferObject = 0x4a0234 }; };
template<> struct OriginalVtables<Table<String, int, PrimitiveArray<int, 1>, 21> >
                                                                   { enum : unsigned { primary = 0x4a0274 }; };
template<> struct OriginalVtables<BufferObjectTable<String, int, PrimitiveArray<int, 1>, 21> >
                                                                   { enum : unsigned { primary = 0x49d4dc, bufferObject = 0x49d4c4 }; };
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
void  ClearWorldObjects();                                             // @0042d010 frees the level's prop lists (reconstructed: PropList.cpp)

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
void  AddUpdatingProp(Prop *prop);                                     // @0042d180 (reconstructed: PropList.cpp)
void  RemoveUpdatingProp(Prop *prop);                                  // @0042d1a0 (reconstructed: PropList.cpp)
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

// ---- the place's walkable surfaces (NavProp.h) ----
// The PlaceSimulation keeps the level's walkable surfaces (loaded from .nat files) and the copies of them that NavProps place.
// That class is not reconstructed; these are the functions of its surface list that NavProp calls.
class IniReader;
struct NavProp;
struct LoadedSurface;
void *GetPlaceSurfaces(void *place);                                    // @00439b60 the place simulation's surface list (its +0x3c)
// "Loading surface %s (.nat file %s) from directory %s" (the ini file's directory); returns the surface's number
int   LoadSurface(void *surfaces, IniReader *ini, const char *name, const char *natFile);   // @0040fc10
LoadedSurface *LoadedSurfaceAt(void *surfaces, int surface);             // @00412c10 (reconstructed in NavSurface.cpp)
// A copy of a surface placed with a prop's matrix; returns the copy's number
int   AddSurfaceInstance(void *surfaces, NavProp *prop, int surface, const float *matrix, IniValueTable *properties);   // @004100b0
void  RemoveSurfaceInstance(void *surfaces, int instance);              // @004102f0 only adds the number to a list at +0x144 (presumably removed later)
void  MoveSurfaceInstance(void *surfaces, int instance, const float *matrix);   // @00410310

// ---- CycleProp's step events (CycleProp.h): a table from a child number to a script function name ----
template<> struct OriginalVtables<PrimitiveArray<int, 24> >        { enum : unsigned { primary = 0x4a03c4, bufferObject = 0x4a03ac }; };
template<> struct OriginalVtables<Set<int, 24> >                   { enum : unsigned { primary = 0x4a03fc, bufferObject = 0x4a03e4 }; };
template<> struct OriginalVtables<Table<int, String, PrimitiveArray<String, 3>, 24> >
                                                                   { enum : unsigned { primary = 0x4a0424 }; };
template<> struct OriginalVtables<BufferObjectTable<int, String, PrimitiveArray<String, 3>, 24> >
                                                                   { enum : unsigned { primary = 0x49dd2c, bufferObject = 0x49dd14 }; };

// ---- prop loader (PropLoader.h) ----
class IniReader;
struct MorphCycle;
typedef Table<String, Prop *, Array<Prop *>, 0>             PropTable;     // prop templates by name (the level's tables: 0x4c7208, 0x4c7230, 0x4c72c0)
typedef Table<String, MorphCycle *, Array<MorphCycle *>, 0> PathTable;     // animation paths (clips) by name (0x4c7280, 0x4c7258, 0x4c71d0)
int   ReadPathText(MorphCycle *clip, IniReader *ini, const char *file);   // @0041c530 (thiscall) fills the clip from a text path file; 0 on failure

// ---- simulation (Simulation.h) ----
struct CreatureTable;
void  ConstructIdList(void *ids);                       // @0045eef0 (thiscall) the simulation's list of ids: empty, room for 2
void  DestructIdList(void *ids);                        // @0045efa0 (thiscall)
void  ConstructCreatureTable(CreatureTable *table);     // @00466590 (thiscall) the table part of the creature table; the vtables are set by the caller
void *GetApplication();                                 // @004206b0 the global at 0x4badac
void  FlushInputMessages(void *application);            // @0043c970 (thiscall) throws away the key and mouse messages waiting

// ---- SqueezilBody (SqueezilBody.h) ----
struct Contact;
unsigned FrameMilliseconds();                                   // @004206d0 the game clock (the global at 0x4badc0), then @0041ec00 its +0xc: the milliseconds the last frame took
int   MorphCycleLoad(MorphCycle *clip, const char *file);       // @0041bd60 (thiscall) fills the clip from a cycle file; 0 on failure
void  MatrixToAngles(const float *m, float *pitch, float *yaw, float *roll);   // @0047e500 (thiscall on the matrix) radians: atan2 of the matrix's rows
void *NavSurfaceFaceOf(void *surfaces, const Contact *contact);   // @00412bf0 the face object of contact->face (the list at +0x134, virtual slot 0)
float *FaceRelativeMatrix(void *face, float *out, const float *matrix);   // @004110b0 (thiscall) the matrix relative to the face; returns out

// ---- SqueezilSoul (SqueezilSoul.h) ----
struct Float3;
struct Vector3f;
struct Inventory;
class World;
template<class T> class Array;
struct Matrix16 { float m[16]; };                               // a 4x4 matrix passed by value (the original's matrix class)
void  DebugLog(const char *format, ...);                        // @00419360 printf style; written only while the game's debug output is on
int   ClockValueA();                                            // @004206d0 then @0041ec10: the game clock's +0x20 (meaning not identified)
int   ClockValueB();                                            // @004206d0 then @0041ebf0: the game clock's +0x24 (meaning not identified)
void  PlayEffect(const char *name, const Float3 &position);     // @0044a250 (cdecl, a String and a vector by value) plays the named sound effect there
void  StopLoopingSounds();                                      // @0044a8f0 (only called while the global at 0x4bd960 is set)
static int *const kLoopingSoundPlaying = (int *)0x4bd960;
int   PhysicalCollision(int id, int otherId);                   // @00447590 (cdecl, PhysicalSoul's) two creatures bumped: "attackEffect" / "bumpEffect"
int   InputHeldAttack();                                        // the application object (0x4badac) +0x68: set while the attack is held (inferred)
void  PhysicalSoulInit(void *soul);                             // @00446e30 (thiscall) PhysicalSoul's constructor: the base vtables, and the first one made reads Gravity / StaticFriction / KineticFriction
static const void *const kMessageMapType      = (const void *)0x4b93c8;   // .?AVMessageMap
static const void *const kGrantMessageMapType = (const void *)0x4b93a8;   // .?AVGrantMessageMap
// the inventory (class Inventory, 0x416010-0x4165e0): one list of values per goodie name, plus the one goodie held
void  InventoryAdd(Inventory *inv, const char *goodie, int value);       // @00416110
void  InventoryRemove(Inventory *inv, const char *goodie, int index);    // @00416280
int   InventoryCount(Inventory *inv, const char *goodie);                // @00416300
Array<int> *InventoryItems(Inventory *inv, const char *goodie);          // @004163d0 NULL when there are none
void  InventoryRemoveNuts(Inventory *inv);                               // @004164a0 empties the food nut and spit nut lists (and more, not read)
String *InventoryHeldName(Inventory *inv);                               // its +8: the goodie held ("" for none)
int  *InventoryHeldValue(Inventory *inv);                                // its +0xc
// the place (PlaceSimulation, not reconstructed) and its goodie list (its +0x38, @00439b50)
void *GetPlaceGoodies(void *place);                                       // @00439b50
void  DropGoodie(void *goodies, int value, const char *goodie, Matrix16 at);      // @0044d4f0 (thiscall) puts a goodie into the place
void  ThrowGoodie(void *goodies, int value, Matrix16 at, Float3 direction, float speed);   // @0044dc30 (thiscall) a spat nut
void  GoodieGranted(void *goodies, const char *goodie, void *soul, void *place);  // @0044bc40 (thiscall) tells the place a goodie was picked up
// the place's surfaces (GetPlaceSurfaces) as a squeezil moves over them -- reconstructed in NavSurface.cpp (NavSurfaceList)
int   NavSurfaceIsGone(void *surfaces, const Contact *contact);              // @00410890 the contact's surface copy was removed (its two lists, virtual slot 6)
String NavSurfaceTypeName(void *surfaces, const Contact *contact);           // @0040f3c0 (thiscall, String returned through a hidden pointer)
IniValueTable *NavSurfaceProperties(void *surfaces, const Contact *contact); // @0040f3a0
int   NavSurfaceIsSlippery(void *surfaces, const Contact *contact);          // @0040f770 a property of the face's type (inferred from its use)
int   NavSurfaceBlocksWalking(void *surfaces, const Contact *contact);       // @0040f610 a property of the face's type (inferred from its use)
int   NavSurfaceHasTouchScript(void *surfaces, const Contact *contact);      // @0040f8d0 a property of the face's type (inferred from its use)
String NavSurfaceTouchScript(void *surfaces, const Contact *contact);        // @0040f9b0 the script function named by the face's type
void  NavSurfaceTypeChanged(void *surfaces, void *soul, const Contact *contact, const void *oldInfo, const void *newInfo);   // @00412920
// @00411120 Moves a squeezil `distance` along the surface from the first five vectors and the contact, writing the new ones into
// the second set (the original passes the same buffers for both). 0 when it ran off the surface.
int   MoveAlongSurface(void *surfaces, void *soul, Float3 *pos, Float3 *right, Float3 *up, Float3 *forward, Float3 *direction,
                       float distance, Contact *contact, Float3 *newPos, Float3 *newRight, Float3 *newUp, Float3 *newForward,
                       Float3 *newDirection, Contact *newContact);
// @004108e0 Whether a flight of `distance` from pos along direction hits a surface; if so, where and how the squeezil lands
int   FlightHitsSurface(void *surfaces, Float3 *pos, Float3 *right, Float3 *up, Float3 *forward, Float3 *direction,
                        float distance, Float3 *hitPos, Float3 *newRight, Float3 *newUp, Float3 *newForward, Contact *newContact);
void *WorldSurface(World *world, int surface);                  // @004365a0 the surface copy `surface` of the current place (its +0xc: face records)
// The lowest height a squeezil can reach (set by @00450c00): one that would sink below it is held at it, and on reaching it the
// script function named by the string (set by @00450b90, both not reconstructed) runs once
static float *const kFloorLevel = (float *)0x4c7ac8;
static String *const kFloorLevelScript = (String *)0x4c7acc;
// @00436640 (thiscall on the world, 3 stack arguments) The matrix `matrix`, given relative to a surface copy's face, in the world:
// the face's own transform applied. `surface` is the surface copy number (the first field of a Contact). Returns out.
float *WorldFaceToWorld(World *world, float *out, const float *matrix, int surface);
