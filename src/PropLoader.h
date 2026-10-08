// PropLoader: reading the props (and the animation paths they use) a level lists in its ini files. Reconstructed from SQZ.EXE
// 0x429dc0-0x42a290 (the top level) and LoadProp.cpp (0x42a520-0x42c700). The path reader 0x42c700 is still original code
// (Original.h).
//
// A level's ini names other ini files in [InputFiles]. Each of them can hold [PathList] (animation paths, read first for every
// file) and the prop lists: [BackstagePropList] (props loaded but not shown: templates other props copy) and [PropList] (props
// loaded and shown in the scene at once). A prop is a template stored by name in a table; FindPropTemplate looks it up there.
//
// The game keeps three pairs of tables (a table of props and a table of paths each): 0x4c7208 / 0x4c7280 for the level itself,
// 0x4c7230 / 0x4c7258 and 0x4c72c0 / 0x4c71d0 for two other sets that 0x436370 (a Soul / region function) asks for. Only the
// first pair is ever cleared here (ClearPropTables).
#pragma once
#include "Container.h"
#include "IniReader.h"
#include "Original.h"
#include "Float3.h"

struct Prop;
struct MorphCycle;


// @00429dc0 frees the level's props and paths (first pair of tables) and empties the tables
void ClearPropTables();

// @00429fa0 reads every ini file named in ini's [InputFiles], then ini itself: paths first, then props. The splash screen is
// stepped about ten times on the way.
void ReadPropFiles(IniReader *ini, PropTable *props, PathTable *paths);
void ReadPropFiles1(IniReader *ini);   // @00429f40 into the first pair of tables (the level's own)
void ReadPropFiles2(IniReader *ini);   // @00429f60 into the second pair
void ReadPropFiles3(IniReader *ini);   // @00429f80 into the third pair

// @0042a290 [BackstagePropList] then [PropList] of one ini file: every entry names a prop, loaded by LoadProp; the ones in
// [PropList] are shown in the scene at once
void ReadProps(IniReader *ini, PropTable *props);

// @0042a520 Builds the prop template called name from its section in ini (what its Type key says) and files it in table (NULL:
// the second pair's table), replacing a template of that name. kind is NULL from the prop lists; "Prop" (what the sim code
// passes) makes the entry a prop of type Prop whose placement keys carry that prefix ("PropPosition" ...). Returns NULL, after
// logging why, when it can't.
//
// Types: Mesh (a loaded geometry), Sprite (a loaded texture as a decal), and the group types Prop and Frame (FrameProp),
// LODProp, Cycle (CycleProp) and Nav (NavProp), whose children are the props named by the "Prop" key and Child1..ChildCount.
// Without a Type, the section's own name is tried as a geometry, then as a texture. Placement: Position, Scale, then Rotation
// (three angles in degrees) or RotationZdir and RotationYup (direction and up vectors).
Prop *LoadProp(IniReader *ini, PropTable *table, const char *name, const char *kind);

// @0042bfd0 the placement keys of ini's current section, each with the prefix in front (NULL: none): Position (default 0 0 0,
// warns without), Scale (1 1 1), then Rotation, or else RotationZdir (0 0 1) and RotationYup (0 1 0), both made unit length.
void ReadPlacement(IniReader *ini, const char *prefix, Float3 *position, Float3 *scale, Float3 *direction, Float3 *up);

// @00421db0 splits text at spaces into a new list (runs of spaces count as one)
StringList *SplitWords(const char *text);

// @0042cc10 the animation path with that name: looked for in the third, second, then the level's table; NULL if none
MorphCycle *FindPath(const char *name);
