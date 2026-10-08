// Resources: loading and sharing the level's textures and geometry. Reconstructed from SQZ.EXE 0x424010-0x425870.
//
// The ini files list what a place or region needs: [InputFiles] names other ini files, [TextureResources] and
// [GeometryResources] map an entry name to a file (a .gif / .ppm texture, a .wrl or .sqg model) or, when the value is not a
// file, to the name of another entry to share. Everything loaded goes into the top table of a stack (one table per level);
// leaving the level pops the table and releases what is in it. Lookups search the stack from the top down, so a place can use
// what the game loaded before it.
//
// The stacks themselves are globals the start-up code built (textures at 0x4c7180, geometry at 0x4c71c0), the tables hold the
// original vtables (Container.h), and several loaders switch the process's current directory into the folder of the ini file
// being read, because the file names inside are relative to it. The folder they came from is parked in kSavedDirectory so
// TextureLookup can step back into it for texture files.
#pragma once
#include "IniReader.h"
#include "Original.h"

// ---- textures ----
IDirect3DRMTexture *LoadTexture(const char *name, const char *path);   // @00425070 loads the file and files it under name in the top table (replacing an entry of that name); NULL when it can't
IDirect3DRMTexture *FindTexture(const char *name);                     // @00425220 searches the stack from the top; NULL when missing
void LoadTextureResources(IniReader *ini);                             // @00425310 pushes a new table, reads every list named in [InputFiles] into it, then the ini file's own [TextureResources]
void ReadTextureList(IniReader *ini);                                  // @00425500 [TextureResources]: load or share each entry, then the default prop sprite
void ReadDefaultPropSprite(IniReader *ini);                            // @00425870 the "DefaultPropSprite" decal settings over the current defaults
void PopTextureTable();                                                // @00425ab0 releases the top table's textures and drops the table
IDirect3DRMTexture *TextureLookup(const char *name);                   // @00424010 the model loaders' texture callback: strips the extension and folder, reuses a loaded texture or loads it

// ---- geometry ----
IDirect3DRMFrame *LoadGeometry(const char *name, const char *spec);    // @004242b0 one model (spec is "file[;options]"): .sqg if there is one, else sqgifies the .wrl; filed under name
IDirect3DRMFrame *FindGeometry(const char *name);                      // @004246f0 searches the stack from the top; NULL when missing
void LoadGeometryResources(IniReader *ini);                            // @004247e0 like LoadTextureResources, for the geometry stack
void ReadGeometryList(IniReader *ini);                                 // @004249a0 [GeometryResources]: load or share each entry
void PopGeometryTable();                                               // @00424c80
void InitGeometryLoading();                                            // @004241d0 start-up: error callbacks, the perspective setting, no saved directory
void SqgErrorLog(const char *message);                                 // @004241b0 the model loaders' error callback: logs it
