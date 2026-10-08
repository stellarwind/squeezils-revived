// PropLoader: see PropLoader.h. Reconstructed from SQZ.EXE 0x429dc0-0x42a520.
#include <string.h>
#include "PropLoader.h"
#include "Prop.h"
#include "Animation.h"
#include "Scene.h"
#include "Splash.h"

#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"

static PropTable *const kLevelProps  = (PropTable *)0x4c7208;
static PathTable *const kLevelPaths  = (PathTable *)0x4c7280;
static PropTable *const kSecondProps = (PropTable *)0x4c7230;
static PathTable *const kSecondPaths = (PathTable *)0x4c7258;
static PropTable *const kThirdProps  = (PropTable *)0x4c72c0;
static PathTable *const kThirdPaths  = (PathTable *)0x4c71d0;
static Splash **const kSplash        = (Splash **)0x4c75e8;

// @00429dc0 (the original also made a String of every key on the way: no effect)
void ClearPropTables()
{
    ClearWorldObjects();
    for (int i = 0; i < kLevelProps->keys.count; i++) {
        Prop *prop = kLevelProps->values.At(i);
        if (prop)
            prop->VDestroy(1);
    }
    for (int i = kLevelProps->keys.count - 1; i >= 0; i--)
        kLevelProps->RemoveAt(i);

    for (int i = 0; i < kLevelPaths->keys.count; i++) {
        MorphCycle *path = kLevelPaths->values.At(i);
        if (path)
            delete path;
    }
    for (int i = kLevelPaths->keys.count - 1; i >= 0; i--)
        kLevelPaths->RemoveAt(i);
}

// @00429fa0
void ReadPropFiles(IniReader *ini, PropTable *props, PathTable *paths)
{
    ini->SetSection("InputFiles");
    StringList *files = ini->GetKeys();

    // paths: the input files' first, then this file's own
    for (int i = 0; i < files->count; i++) {
        ini->SetSection("InputFiles");
        String name(ini->GetString(files->At(i).c_str()));
        if (strlen(name.c_str()) != 0) {
            IniReader input(name.c_str());
            GameLog("Reading paths from %s...\n", input.GetPath());
            ReadPaths(&input, paths);
        } else {
            GameLog("ERROR: no input file numbered %d.\n", i);
        }
    }
    ReadPaths(ini, paths);

    // props: the same files, the splash screen steps about ten times
    int every = files->count < 10 ? 1 : files->count / 10;
    int read = 0;
    for (int i = 0; i < files->count; i++) {
        ini->SetSection("InputFiles");
        String name(ini->GetString(files->At(i).c_str()));
        if (strlen(name.c_str()) != 0) {
            IniReader input(name.c_str());
            GameLog("Reading props from %s...\n", input.GetPath());
            ReadProps(&input, props);
            if (read % every == 0 || every == 1)
                (*kSplash)->Step();
            read++;
        } else {
            GameLog("ERROR: no input file numbered %d.\n", i);
        }
    }
    delete files;
    ReadProps(ini, props);
}

// @00429f40
void ReadPropFiles1(IniReader *ini) { ReadPropFiles(ini, kLevelProps, kLevelPaths); }
// @00429f60
void ReadPropFiles2(IniReader *ini) { ReadPropFiles(ini, kSecondProps, kSecondPaths); }
// @00429f80
void ReadPropFiles3(IniReader *ini) { ReadPropFiles(ini, kThirdProps, kThirdPaths); }

// @0042c700 The table key is the file name (the entry's value), not the entry's own name. A path file that can't be read is dropped.
void ReadPaths(IniReader *ini, PathTable *paths)
{
    ini->SetSection("PathList");
    StringList *entries = ini->GetKeys();
    for (int i = 0; i < entries->count; i++) {
        MorphCycle *clip = new MorphCycle(0, 1.0f, -1.0f);
        ini->SetSection("PathList");
        String file(ini->GetString(entries->At(i).c_str()));
        if (ReadPathText(clip, ini, file.c_str())) {
            if (paths->Contains(file)) {
                MorphCycle *old = paths->Get(file);
                if (old)
                    delete old;
            }
            paths->Insert(file, clip, -1);
        }
    }
    delete entries;
}

// @0042a290
void ReadProps(IniReader *ini, PropTable *props)
{
    ini->SetSection("BackstagePropList");
    StringList *entries = ini->GetKeys();
    for (int i = 0; i < entries->count; i++) {
        ini->SetSection("BackstagePropList");
        String propName(ini->GetString(entries->At(i).c_str()));
        if (LoadProp(ini, props, propName.c_str(), NULL))
            GameLog(" OK.\n");
        else
            GameLog("ERROR: couldn't find prop %s in BackstagePropList.\n", propName.c_str());
    }
    delete entries;

    ini->SetSection("PropList");
    entries = ini->GetKeys();
    for (int i = 0; i < entries->count; i++) {
        ini->SetSection("PropList");
        String propName(ini->GetString(entries->At(i).c_str()));
        Prop *prop = LoadProp(ini, props, propName.c_str(), NULL);
        if (prop) {
            prop->VShow(GetScene()->GetSceneChild());
            GameLog(" OK.\n");
        } else {
            GameLog("ERROR: couldn't find prop %s in PropList.\n", propName.c_str());
        }
    }
    delete entries;
}

// @00436370
void WorldReadPropFiles(IniReader *ini, IniReader *other)
{
    if (other)
        ReadPropFiles3(other);
    else
        ReadPropFiles2(ini);
}
