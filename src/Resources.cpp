// Resources: see Resources.h. Reconstructed from SQZ.EXE 0x424010-0x425870.
#include <string.h>
#include <windows.h>
#include "Resources.h"
#include "TextureDefaults.h"
#include "Texture.h"
#include "SqgModel.h"
#include "Container.h"
#include "Renderer.h"
#include "Scene.h"
#include "Splash.h"
#include "World.h"

// The containers have no virtual destructor in the original either (Container.h), and the objects deleted here are exactly the
// classes the original deletes, so the warning does not apply.
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"

// ---- the game's globals ----
static Array<TextureTable *> *const kTextureStack  = (Array<TextureTable *> *)0x4c7180;
static Array<GeometryTable *> *const kGeometryStack = (Array<GeometryTable *> *)0x4c71c0;
static char *const kSavedDirectory = (char *)0x4c70e8;       // 128 bytes: the folder the process was in before a loader moved into an ini file's folder, "" when it did not
static int  *const kTextureBytes   = (int *)0x4bb5f4;        // running total of the textures' memory, counted by CreateTextureFromGif
static int  *const kUsePerspective = (int *)0x4bb5f0;        // [Render] UsePerspectiveCorrection (always 1 with a hardware device)
static Splash **const kSplash      = (Splash **)0x4c75e8;
static DecalSettings *const kDefaultDecal = (DecalSettings *)0x4c7190;
static TexturePixelFormat *const kTexturePixelFormat = (TexturePixelFormat *)0x4c7168;

static_assert(sizeof(TextureTable) == 0x28 && sizeof(GeometryTable) == 0x28, "the original allocates 0x28 bytes for a resource table");

static TextureTable *TopTextureTable()   { return kTextureStack->At(kTextureStack->count - 1); }     // @00425bf0
static GeometryTable *TopGeometryTable() { return kGeometryStack->At(kGeometryStack->count - 1); }   // @00424dc0

// A value ending in the extension (the original compares the last four characters, case sensitive)
static bool EndsWith(const char *text, size_t length, const char *extension)
{
    return length >= 4 && strcmp(text + length - 4, extension) == 0;
}

// Loaders that read files named relative to an ini file move into that folder for the duration (only for a plain file name:
// a name with a backslash is taken as it stands). The folder they leave is parked in kSavedDirectory.
static bool EnterIniFolder(IniReader *ini)
{
    bool moved = true;
    if (GetCurrentDirectoryA(128, kSavedDirectory) == 0)
        moved = false;                                   // the original still switches below, but will not switch back
    String folder;
    SetCurrentDirectoryA(ini->GetDirectory(&folder)->c_str());
    return moved;
}

static void LeaveIniFolder()
{
    SetCurrentDirectoryA(kSavedDirectory);
    kSavedDirectory[0] = 0;
}

// ---- textures ----

// @00425070
IDirect3DRMTexture *LoadTexture(const char *name, const char *path)
{
    GameLog("Loading texture %s (%s): ", name, path);
    IDirect3DRMTexture *texture = NULL;
    if (strstr(path, ".ppm"))
        GetRenderer()->d3drm->LoadTexture(path, &texture);
    if (texture == NULL) {
        float transparent[3] = { 1.0f, 0.0f, 1.0f };     // magenta
        texture = CreateTextureFromGif(path, transparent, (const int *)kTexturePixelFormat, kTextureBytes);
    }
    if (texture == NULL) {
        GameLog("failed!\n");
        return NULL;
    }
    GameLog("OK.\n");

    TextureTable *table = TopTextureTable();
    if (table->Contains(String(name)))
        table->Get(String(name))->Release();             // the entry is replaced: drop the old texture
    table->Insert(String(name), texture);
    return texture;
}

// @00425220
IDirect3DRMTexture *FindTexture(const char *name)
{
    for (int i = kTextureStack->count - 1; i >= 0; i--) {
        TextureTable *table = kTextureStack->At(i);
        if (table->Contains(String(name)))
            return table->Get(String(name));
    }
    return NULL;
}

// @00425310
void LoadTextureResources(IniReader *ini)
{
    TextureTable *table = new TextureTable();
    kTextureStack->Insert(table);

    ini->SetSection("InputFiles");
    StringList *lists = ini->GetKeys();
    for (int i = 0; i < lists->count; i++) {
        String file(ini->GetString(lists->At(i).c_str()));
        if (strlen(file.c_str()) != 0) {
            IniReader list(file.c_str());
            GameLog("Reading texture resource list from %s...\n", list.GetPath());
            ReadTextureList(&list);
            if (i % 2 == 0)
                (*kSplash)->Step();
        }
    }
    delete lists;

    ReadTextureList(ini);
    GameLog("MELTDOWN DEBUG: total texture size now %d (%x).\n", *kTextureBytes, *kTextureBytes);
}

// @00425500 An entry whose value is a .gif or .ppm file name is loaded; any other value names an entry to share.
void ReadTextureList(IniReader *ini)
{
    ini->SetSection("TextureResources");
    StringList *keys = ini->GetKeys();
    for (int i = 0; i < keys->count; i++) {
        const char *value = ini->GetString(keys->At(i).c_str());
        size_t length = strlen(value);
        bool isFile = length > 5 && (EndsWith(value, length, ".gif") || EndsWith(value, length, ".ppm"));
        if (isFile) {
            bool moved = strchr(value, '\\') == NULL;
            if (moved)
                moved = EnterIniFolder(ini);
            if (i % 20 == 0)
                (*kSplash)->Step();
            LoadTexture(keys->At(i).c_str(), value);
            if (moved)
                LeaveIniFolder();
        } else {
            String key = keys->At(i);
            GameLog("Shared texture entry: %s shares %s.\n", key.c_str(), value);
            IDirect3DRMTexture *texture = FindTexture(value);
            if (texture == NULL) {
                GameLog("ERROR: no entry %s to share!\n", value);
                continue;
            }
            TopTextureTable()->Insert(keys->At(i), texture);
            texture->AddRef();
        }
    }
    delete keys;
    ReadDefaultPropSprite(ini);
}

// @00425870
void ReadDefaultPropSprite(IniReader *ini)
{
    DecalSettings settings;
    GetWorld()->ReadDecalSettings(&settings, ini, "Defaults", "DefaultPropSprite", *kDefaultDecal);
    *kDefaultDecal = settings;
}

// @00425ab0 (the original also built and dropped a String of every key on the way: no effect)
void PopTextureTable()
{
    TextureTable *table = TopTextureTable();
    for (int i = 0; i < table->keys.count; i++)
        table->values.At(i)->Release();
    delete table;
    kTextureStack->RemoveAt(kTextureStack->count - 1);
}

// @00424010 Models name their textures with a path and usually a .gif extension; the tables are keyed by the bare name.
IDirect3DRMTexture *TextureLookup(const char *name)
{
    String copy(name);
    char *base = (char *)copy.c_str();                   // changed in place below (the copy's own buffer)
    size_t length = strlen(base);
    if (_stricmp(base + length - 4, ".gif") == 0)        // as in the original, no check that the name is long enough
        base[length - 4] = 0;
    char *slash = strrchr(base, '\\');
    if (slash)
        base = slash + 1;

    IDirect3DRMTexture *texture = NULL;
    if (strlen(base) != 0)
        texture = FindTexture(base);
    if (texture) {
        GameLog("Reusing texture %s\n", base);
        return texture;
    }
    texture = FindTexture(name);
    if (texture) {
        GameLog("Reusing texture %s\n", name);
        return texture;
    }

    // Not loaded yet: the file is looked for from the folder the process was in before the loaders moved into the ini's.
    char current[128];
    bool back = strlen(kSavedDirectory) != 0;
    if (back) {
        GetCurrentDirectoryA(128, current);
        SetCurrentDirectoryA(kSavedDirectory);
    }
    texture = LoadTexture(name, name);
    if (strlen(kSavedDirectory) != 0)
        SetCurrentDirectoryA(current);
    if (texture == NULL)
        GameLog("WARNING: can't load texture %s\n", name);
    return texture;
}

// ---- geometry ----

// @004241b0
void SqgErrorLog(const char *message)
{
    GameLog("%s", message);
}

// Both ways of loading a model end here: the frame holding its meshes (@004242b0 builds it twice in the same way).
static IDirect3DRMFrame *BuildModelFrame(SqgModel *model, int useMesh)
{
    IDirect3DRM *d3drm = GetRenderer()->d3drm;
    if (useMesh) {
        IDirect3DRMFrame *frame = GetScene()->CreateFrameUnder(NULL);
        PZMorphMesh *mesh = model->CreateMesh(d3drm);
        frame->AddVisual(mesh->mesh);
        if (mesh) {
            mesh->Destroy();
            Operator_Delete(mesh);
        }
        return frame;
    }
    return model->BuildFrame(d3drm, *kUsePerspective);
}

// @004242b0 spec is "file" or "file;options" (the options are cut off and not used here).
IDirect3DRMFrame *LoadGeometry(const char *name, const char *spec)
{
    String specCopy(spec);
    char *file = (char *)specCopy.c_str();
    char *semicolon = strchr(file, ';');
    if (semicolon)
        *semicolon = 0;
    GameLog("Loading geom %s (%s): ", name, file);
    (*kSplash)->Step();

    IniReader ini;
    int sqgify = ini.GetInt("SqgifyAllFiles", 0);
    int useMesh = ini.GetInt("UseMeshForGeomResources", 0);

    IDirect3DRMFrame *frame;
    if (!sqgify) {
        frame = LoadWrlFrame(GetRenderer()->d3drm, file, TextureLookup);
    } else {
        SqgSetErrorCallback(SqgErrorLog);
        SqgSetTextureLookup(TextureLookup);
        void *memory = Operator_New(0x2c);
        SqgModel *model = memory ? SqgModelConstruct(memory) : NULL;

        // the .sqg file sits next to the .wrl with the same name: swap the three letter extension
        String sqgName(file);
        char *sqgFile = (char *)sqgName.c_str();
        strcpy(sqgFile + strlen(sqgFile) - 4, ".sqg");

        if (model->Load(sqgFile, 0)) {
            GameLog("loaded sqg: ");
            frame = BuildModelFrame(model, useMesh);
        } else if (SqgModelLoadWrl(model, file, 0)) {
            GameLog("saved to sqg: ");
            SqgModelSave(model, sqgFile);
            frame = BuildModelFrame(model, useMesh);
        } else {
            GameLog("couldn't sqgify, loading wrl: ");
            frame = LoadWrlFrame(GetRenderer()->d3drm, file, TextureLookup);
        }
        if (model) {
            SqgModelDestruct(model);
            Operator_Delete(model);
        }
    }

    if (frame == NULL) {
        GameLog("failed!\n");
        return NULL;
    }
    GameLog("OK.\n");

    GeometryTable *table = TopGeometryTable();
    if (table->Contains(String(name)))
        table->Get(String(name))->Release();             // the entry is replaced
    table->Insert(String(name), frame);
    return frame;
}

// @004246f0
IDirect3DRMFrame *FindGeometry(const char *name)
{
    for (int i = kGeometryStack->count - 1; i >= 0; i--) {
        GeometryTable *table = kGeometryStack->At(i);
        if (table->Contains(String(name)))
            return table->Get(String(name));
    }
    return NULL;
}

// @004247e0
void LoadGeometryResources(IniReader *ini)
{
    GeometryTable *table = new GeometryTable();
    kGeometryStack->Insert(table);

    ini->SetSection("InputFiles");
    StringList *lists = ini->GetKeys();
    for (int i = 0; i < lists->count; i++) {
        String file(ini->GetString(lists->At(i).c_str()));
        if (strlen(file.c_str()) != 0) {
            IniReader list(file.c_str());
            GameLog("Reading geom resource list from %s...\n", list.GetPath());
            ReadGeometryList(&list);
        }
    }
    delete lists;

    ReadGeometryList(ini);
}

// @004249a0 An entry with a folder in its value, or a .wrl value, is a file to load; a bare name that is not a .wrl names an
// entry to share. A bare file name is looked for next to the ini file.
void ReadGeometryList(IniReader *ini)
{
    ini->SetSection("GeometryResources");
    StringList *keys = ini->GetKeys();
    for (int i = 0; i < keys->count; i++) {
        String keyString = keys->At(i);
        const char *key = keyString.c_str();
        const char *value = ini->GetString(key);
        if (strlen(value) == 0)
            GameLog("ERROR: geom resource %s has no filename entry!\n", key);

        size_t length = strlen(value);
        bool bare = strchr(value, '\\') == NULL;
        bool share = bare && !(length >= 4 && _stricmp(value + length - 4, ".wrl") == 0);
        bool moved = bare;
        if (moved)
            moved = EnterIniFolder(ini);

        if (share) {
            GameLog("Shared geom entry: %s shares %s.\n", key, value);
            IDirect3DRMFrame *frame = FindGeometry(value);
            if (frame == NULL) {
                // the original passes no argument for the %s here; the entry name is what the message is about
                GameLog("ERROR: no entry %s to share!\n", value);
            } else {
                TopGeometryTable()->Insert(String(key), frame);
                frame->AddRef();
            }
        } else {
            LoadGeometry(key, value);
        }

        if (moved)
            LeaveIniFolder();
    }
    delete keys;
}

// @00424c80
void PopGeometryTable()
{
    GeometryTable *table = TopGeometryTable();
    for (int i = 0; i < table->keys.count; i++)
        table->values.At(i)->Release();
    delete table;
    kGeometryStack->RemoveAt(kGeometryStack->count - 1);
}

// @004241d0
void InitGeometryLoading()
{
    kSavedDirectory[0] = 0;
    WrlSetLogCallback(SqgErrorLog);
    WrlSetErrorCallback(SqgErrorLog);

    IniReader ini;
    ini.SetSection("Render");
    *kUsePerspective = ini.GetInt("UsePerspectiveCorrection", 0);
    if (GetRenderer()->hardware)
        *kUsePerspective = 1;
}
