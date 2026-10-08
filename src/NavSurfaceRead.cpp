// NavSurface, part two: reading the surface types from the ini files, the list's lifetime, and the once-a-frame work.
// Reconstructed from SQZ.EXE 0x40bee0, 0x40d530-0x40e2e0 (+ the one at 0x40e160), 0x40f450, 0x410330, 0x4106e0 and 0x412920.
// The classes and the loading of the surfaces themselves are in NavSurface.h / NavSurface.cpp.
#include "NavSurface.h"
#include "NavMesh.h"
#include "SqueezilSoulInternal.h"
#include "FrameProp.h"
#include "Simulation.h"
#include "Script.h"
#include "IniReader.h"
#include <string.h>

static const char *const kNoneName = "none";     // the string at 0x4b9cf4

// ---- lifetime ----

// @0040bee0
void LoadedSurface::Destruct()
{
    delete mesh;
    typeNumbers.~SurfaceTypeNumbers();
    typeNames.~StringList();
    file.~String();
    directory.~String();
}

// What the original inlines wherever it frees an instance (@0040d7a8-0040d7eb): the flags, then the property table. The matrices
// and the positions have nothing to destroy.
void InstancedSurface::Destruct()
{
    flags.~IntList();
    properties.~IniValueTable();
}

// @0040d530
NavSurfaceList *NavSurfaceList::Construct()
{
    ::new (&knownNames) NameSet();
    ::new (&fileNames) Array<NameSet *>();
    ::new (&loaded) Array<LoadedSurface *>();
    ::new (&instances) Array<InstancedSurface *>();
    ConstructIdList(&removed);
    ConstructIdList(&removedLater);
    ::new (&types) SurfaceTypeTable();
    return this;
}

// Frees one instance: its members, then its memory
void NavSurfaceList::DestroyInstance(int number)
{
    InstancedSurface *copy = instances.At(number);
    if (copy) {
        copy->Destruct();
        Operator_Delete(copy);
    }
}

// @0040d640 Every loaded surface and every instance goes (the descriptions in the type table and the name sets in fileNames are
// left to the heap, as in the original), then the members in the opposite order of Construct.
void NavSurfaceList::Destruct()
{
    for (int i = loaded.count - 1; i >= 0; i--) {
        LoadedSurface *l = loaded.At(i);
        if (l) {
            l->Destruct();
            Operator_Delete(l);
        }
        loaded.RemoveAt(i);
    }
    for (int i = instances.count - 1; i >= 0; i--) {
        if (instances.At(i))
            DestroyInstance(i);
        instances.RemoveAt(i);
    }
    types.~SurfaceTypeTable();
    DestructIdList(&removedLater);
    DestructIdList(&removed);
    instances.~Array<InstancedSurface *>();
    loaded.~Array<LoadedSurface *>();
    fileNames.~Array<NameSet *>();
    knownNames.~NameSet();
}

// ---- reading the ini ----

// @0040d870 The [InputFiles] section lists more ini files (any key; the value is the file name) whose [SurfaceList] sections are
// read first; then the [SurfaceList] section of `ini` itself. Every name read is remembered in a new set of fileNames, so that
// UnloadLastFile can forget them again.
int NavSurfaceList::ReadSurfaceList(IniReader *ini)
{
    String none(kNoneName);
    if (!knownNames.Contains(none)) {
        String again(kNoneName);
        knownNames.Insert(again, -1);
    }
    NameSet *names = new NameSet;
    fileNames.Insert(names, -1);

    ini->SetSection("InputFiles");
    StringList *keys = ini->GetKeys();
    for (int i = 0; i < keys->count; i++) {
        ini->SetSection("InputFiles");
        String file(ini->GetString(keys->At(i).c_str()));
        if (file.c_str()[0] == '\0') {
            GameLog("ERROR: no input file numbered %d.\n", i);
        } else {
            IniReader other(file.c_str());
            ReadSurfaceNames(&other, names);
        }
    }
    delete keys;
    ReadSurfaceNames(ini, names);
    return 1;
}

// @0040dad0 Each key of the [SurfaceList] section names (in its value) a surface type; every type has its own section, read by
// ReadSurfaceType. A name defined twice is logged (and read again). 0 when the section has no keys.
int NavSurfaceList::ReadSurfaceNames(IniReader *ini, NameSet *names)
{
    ini->SetSection("SurfaceList");
    StringList *keys = ini->GetKeys();
    String none(kNoneName);
    if (!knownNames.Contains(none)) {
        String again(kNoneName);
        knownNames.Insert(again, -1);
    }
    if (keys->count == 0) {
        delete keys;
        return 0;
    }
    for (int i = 0; i < keys->count; i++) {
        String name;
        name = String(ini->GetString(keys->At(i).c_str()));
        if (knownNames.Contains(name))
            GameLog("ERROR: surface name %s already present in surface type list!\n", name.c_str());
        knownNames.Insert(name, -1);
        names->Insert(name, -1);
        ReadSurfaceType(ini, name.c_str());
    }
    delete keys;
    return 1;
}

// @0040dd30 Reads the section called `name`: Navigable (false / true / bounce / slippery / flyThrough), RebudGoodies, Activate and
// Deactivate. (The Class key is read and ignored.) A description the table already has for the name is freed and replaced.
// The ini's current section is put back afterwards.
int NavSurfaceList::ReadSurfaceType(IniReader *ini, const char *name)
{
    String saved(ini->section);
    ini->SetSection(name);
    ini->GetString("Class");
    const char *navigable = ini->GetString("Navigable");

    int walkable = 1, bounce = 0, slippery = 0, flyThrough = 0;
    if (strcmp(navigable, "false") == 0) {
        walkable = 0;
    } else if (strcmp(navigable, "true") != 0) {
        if (strcmp(navigable, "bounce") == 0)
            bounce = 1;
        else if (strcmp(navigable, "slippery") == 0)
            slippery = 1;
        else if (strcmp(navigable, "flyThrough") == 0)
            flyThrough = 1;
        else
            walkable = 1;
    }
    float rebud = ini->GetFloat("RebudGoodies", 0.1f);
    String activate(ini->GetString("Activate"));
    String deactivate(ini->GetString("Deactivate"));

    String key(name);
    if (types.Contains(key)) {
        String again(name);
        SurfaceTypeDescription *old = types.Get(again);
        if (old) {
            old->deactivate.~String();
            old->activate.~String();
            Operator_Delete(old);
        }
    }

    SurfaceTypeDescription *d = (SurfaceTypeDescription *)Operator_New(sizeof(SurfaceTypeDescription));
    if (d) {
        ::new (&d->activate) String();
        ::new (&d->deactivate) String();
        d->walkable = walkable;
        d->bounce = bounce;
        d->slippery = slippery;
        d->flyThrough = flyThrough;
        d->rebudGoodies = rebud;
        d->deactivate = String(deactivate.c_str());
        d->activate = String(activate.c_str());
    }
    String slot(name);
    types.Insert(slot, d, -1);
    GameLog("OK.\n");
    ini->SetSection(saved.c_str());
    return 1;
}

// @0040e160 The names the file read last defined are forgotten again: out of the known names, and out of the type table, where
// their descriptions are freed.
void NavSurfaceList::UnloadLastFile()
{
    NameSet *names = fileNames.At(fileNames.count - 1);
    for (int i = 0; i < names->count; i++) {
        String name = names->At(i);
        knownNames.RemoveKey(name);
        if (types.Contains(name)) {
            SurfaceTypeDescription *d = types.Get(name);
            if (d) {
                d->deactivate.~String();
                d->activate.~String();
                Operator_Delete(d);
            }
            types.RemoveKey(name);
        }
    }
    if (names) {
        names->~NameSet();
        Operator_Delete(names);
    }
    fileNames.RemoveAt(fileNames.count - 1);
}

// @0040e2e0 [SurfaceBindingList]: every key is a surface type of a surface, its value the type it is bound to. Logs the start, and
// "OK." when the section had keys. The ini is left at that section.
void ReadSurfaceBindings(IniReader *ini, IniValueTable *out)
{
    GameLog("Reading surface bindings: ");
    ini->SetSection("SurfaceBindingList");
    StringList *keys = ini->GetKeys();
    if (keys->count == 0) {
        delete keys;
        return;
    }
    for (int i = 0; i < keys->count; i++) {
        String key;
        String value;
        key = keys->At(i);
        value = String(ini->GetString(key.c_str()));
        out->Insert(key, value, -1);
    }
    GameLog("OK.\n");
    delete keys;
}

// ---- binding a face set ----

// @0040f450 (the script function SetSurfaceType gets here) Binds a type of the contact's surface copy to another surface type of
// the place: the copy's property table says so, and so does its flag for that type. The two Strings are the callee's to release.
void NavSurfaceList::SetFaceSet(String faceSet, String type, const Contact *c)
{
    InstancedSurface *inst = Instance(c->face);
    IniValueTable *table = &inst->properties;
    const char *faceSetText = faceSet.c_str();
    const char *typeText = type.c_str();
    if (!table->Contains(faceSet)) {
        GameLog("ERROR: can't set nonexistent face set %s to surface type %s.\n", faceSetText, typeText);
        return;
    }
    if (!types.Contains(type)) {
        GameLog("ERROR: can't set face set %s to nonexistent surface type %s.\n", faceSetText, typeText);
        return;
    }
    table->Insert(faceSet, type, -1);
    LoadedSurface *l = Instance(c->face)->surface;
    for (int i = 0; i < l->typeNames.count; i++) {
        String name = l->typeNames.At(i);
        if (StringEqual(&name, &faceSet)) {
            Instance(c->face)->flags.Replace(i, types.Get(type)->walkable);
            break;
        }
    }
}

// ---- the scripts of the types ----

// @00412920 A squeezil's surface changed: the script the old type names as its Deactivate runs, then the new type's Activate. The
// type is the value of the SurfaceInfo (what the face's type is bound to here). A script is only run when it has a name.
void NavSurfaceList::TypeChanged(void *soul, const Contact *, const SurfaceInfo *oldInfo, const SurfaceInfo *newInfo)
{
    LocationSimulation *simulation = (LocationSimulation *)GetCurrentSimulation();
    simulation->FindIdBySoul(soul);

    if (types.Contains(oldInfo->value)) {
        SurfaceTypeDescription *d = types.Get(oldInfo->value);
        String script(d->deactivate);
        if (script.c_str()[0]) {
            ScriptCaller caller;
            ScriptCallerInitSoul(&caller, soul, ((LocationSimulation *)GetCurrentSimulation())->FindIdBySoul(soul), NULL);
            String again(d->deactivate);
            PostFunction(again.c_str(), &caller, 0, 0);
        }
    }
    if (types.Contains(newInfo->value)) {
        SurfaceTypeDescription *d = types.Get(newInfo->value);
        String script(d->activate);
        if (script.c_str()[0]) {
            ScriptCaller caller;
            ScriptCallerInitSoul(&caller, soul, ((LocationSimulation *)GetCurrentSimulation())->FindIdBySoul(soul), NULL);
            String again(d->activate);
            PostFunction(again.c_str(), &caller, 0, 0);
        }
    }
}

// ---- once a frame ----

// @00410330 A body standing on a surface copy that moved (Changed) moves with it: its place relative to the face is turned into
// the world with the copy's new matrix, and its facing (the way it was heading, scaled by its speed) is carried the same way: the
// point it would have reached is put into the copy's space with the matrix from before the move and out again with the new one;
// what is left after taking the new position away is the new heading.
void NavSurfaceList::MoveCarried()
{
    for (int i = 0;; i++) {
        CreatureTable *creatures = ((LocationSimulation *)GetCurrentSimulation())->GetCreatures();
        if (creatures->keys.count <= i)
            break;
        void *body = ((LocationSimulation *)GetCurrentSimulation())->GetCreatures()->values.data[i]->actor;
        if (!CallVirtual<int>(body, BODY_IS_ON_SURFACE))
            continue;
        InstancedSurface *inst = Instance(CallVirtual<Contact *>(body, BODY_GET_CONTACT)->face);
        if (!inst->Changed())
            continue;

        float relative[16];
        memcpy(relative, CallVirtual<Contact *>(body, BODY_GET_CONTACT)->matrix, sizeof relative);
        inst = Instance(CallVirtual<Contact *>(body, BODY_GET_CONTACT)->face);
        float world[16];
        inst->MatrixToWorld(world, relative);

        float speed = CallVirtual<float>(body, BODY_GET_FACING_VALUE);
        const float *bodyMatrix = CallVirtual<float *>(body, BODY_GET_MATRIX);
        Float3 *facing = CallVirtual<Float3 *>(body, BODY_GET_FACING);
        Vector3f heading = Vector3f(facing->x, facing->y, facing->z) * speed;
        Vector3f reached = heading + Vector3f(bodyMatrix[12], bodyMatrix[13], bodyMatrix[14]);

        float before[3], after[3];
        float reachedPoint[3] = { reached.x, reached.y, reached.z };
        inst->ToLocalBefore(before, reachedPoint);
        inst->ToWorld(after, before);
        Vector3f direction = Vector3f(after[0], after[1], after[2]) - Vector3f(world[12], world[13], world[14]);
        float length = Normalize(direction);

        CallVirtual<void>(body, BODY_SET_MATRIX, (const float *)world);
        Float3 facingNow = { direction.x, direction.y, direction.z };
        CallVirtual<void>(body, BODY_SET_FACING, &facingNow, length);
    }
}

// @004106e0 Instances are removed in two steps, so that the code that was told about them has a frame to stop using them: what was
// queued a frame ago (removedLater) is destroyed now, and what was queued since (removed) moves over to wait for the next frame.
void NavSurfaceList::FlushRemoved()
{
    for (int k = removedLater.ids.count - 1; k >= 0; k--) {
        int number = removedLater.At(k);
        DestroyInstance(number);
        InstancedSurface *none = NULL;
        instances.Replace(number, none);
        removedLater.RemoveAt(k);
    }
    for (int k = removed.ids.count - 1; k >= 0; k--) {
        int number = removed.At(k);
        removed.RemoveAt(k);
        removedLater.Insert(number);
    }
}
