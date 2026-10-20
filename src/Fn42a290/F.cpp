// 0x42a290: ReadProps - load the props of [BackstagePropList] and [PropList]; the second list shows each prop in the scene.
#include "IniReader/IniReader.h"
struct Prop { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void Show(void *where); };   // +0x10
struct PropTable;
struct SceneH { void *GetChild(); };                              // 0x477290
extern SceneH *gScene;                                            // 0x4c75e0
Prop *LoadProp(IniReader *ini, PropTable *props, const char *name, int extra);   // 0x42a520 (cdecl)
void GameLog(const char *format, ...);                            // 0x419290
void ReadProps(IniReader *ini, PropTable *props)
{
    StringList *entries;
    int i;
    ini->SetSection("BackstagePropList");
    entries = ini->GetKeys();
    for (i = 0; i < entries->count; i++) {
        {
            ini->SetSection("BackstagePropList");
            const char *key = entries->Get(i).c_str();
            String name(ini->GetString(key));
            if (LoadProp(ini, props, name.c_str(), 0))
                GameLog(" OK.\n");
            else
                GameLog("ERROR: couldn't find prop %s in BackstagePropList.\n", name.c_str());
        }
    }
    if (entries)
        delete entries;
    ini->SetSection("PropList");
    entries = ini->GetKeys();
    for (i = 0; i < entries->count; i++) {
        {
            ini->SetSection("PropList");
            const char *key = entries->Get(i).c_str();
            String name(ini->GetString(key));
            Prop *prop = LoadProp(ini, props, name.c_str(), 0);
            if (prop) {
                prop->Show(gScene->GetChild());
                GameLog(" OK.\n");
            } else {
                GameLog("ERROR: couldn't find prop %s in PropList.\n", name.c_str());
            }
        }
    }
    if (entries)
        delete entries;
}
