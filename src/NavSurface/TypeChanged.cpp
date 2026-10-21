// SQZ.EXE 0x412920: dispatch the old type's Deactivate and new type's Activate scripts.
#include <string.h>
#include "NavSurface/NavSurfaceList.h"
#include "String/String.h"
#include "NavSurface/SurfaceTypeDescription.h"

struct SurfaceTypeInfo {
    int type;
    String typeName, value;
};
struct SurfaceTypeRegistry {
    virtual void slot0();
    virtual int Contains(const String &key); // slot 1
    SurfaceTypeDescription *Get(const String &key); // 0x46d2a0
};
struct SurfaceScriptSimulation {
    int FindIdBySoul(void *soul); // 0x4140b0
};
SurfaceScriptSimulation *GetSimulation(); // 0x420690
struct SurfaceScriptCaller {
    int storage[6];
    SurfaceScriptCaller(void *soul, int id, void *owner); // 0x420900
};
#include "Script/CallFunction.h"

void NavSurfaceList::TypeChanged(void *soul, const Contact *,
    const SurfaceTypeInfo *oldInfo, const SurfaceTypeInfo *newInfo)
{
    GetSimulation()->FindIdBySoul(soul);
    SurfaceTypeRegistry &types = *(SurfaceTypeRegistry *)((char *)this + 0x16c);
    oldInfo->typeName.c_str();
    oldInfo->value.c_str();
    if (types.Contains(oldInfo->value)) {
        SurfaceTypeDescription *description = types.Get(oldInfo->value);
        if (strlen(description->Deactivate().c_str()) > 0) {
            SurfaceScriptCaller caller(soul, GetSimulation()->FindIdBySoul(soul), 0);
            CallFunction(description->Deactivate().c_str(), &caller, 0, 0);
        }
    }
    newInfo->typeName.c_str();
    newInfo->value.c_str();
    if (types.Contains(newInfo->value)) {
        SurfaceTypeDescription *description = types.Get(newInfo->value);
        if (strlen(description->Activate().c_str()) > 0) {
            SurfaceScriptCaller caller(soul, GetSimulation()->FindIdBySoul(soul), 0);
            CallFunction(description->Activate().c_str(), &caller, 0, 0);
        }
    }
}
