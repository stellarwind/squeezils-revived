// 0x4249a0: ReadGeometryList - load or share the entries of the [GeometryResources] section of an ini file.
#include <windows.h>
#include <d3drm.h>
#include <string.h>
#include "IniReader/IniReader.h"
void GameLog(const char *format, ...);                         // 0x419290
extern char gSavedDirectory[];                                 // 0x4c70e8
struct GeomTable { virtual int Insert(const String &k, IDirect3DRMFrame **v, int at); };   // slot 0
GeomTable *TopGeometryTable();                                 // 0x424dc0
IDirect3DRMFrame *FindGeometry(const char *name);              // 0x4246f0
void LoadGeometry(const char *key, const char *value);         // 0x4242b0
void ReadGeometryList(IniReader *ini)
{
    int bare, share;
    ini->SetSection("GeometryResources");
    StringList *keys = ini->GetKeys();
    int i = 0;
    if (keys->count > 0) {
        do {
            const char *key = keys->Get(i).c_str();
            const char *value = ini->GetString(key);
            if (strlen(value) == 0)
                GameLog("ERROR: geom resource %s has no filename entry!\n", key);
            bare = strchr(value, 92) == 0;
            if (bare && ((strlen(value) == 0) > 5 || stricmp(value + strlen(value) - 4, ".wrl") != 0))
                share = 1;
            else
                share = 0;
            String dir;
            if (bare) {
                if (GetCurrentDirectory(0x80, gSavedDirectory) == 0)
                    bare = 0;
                SetCurrentDirectory(ini->GetDirectory().c_str());
            }
            if (share) {
                GameLog("Shared geom entry: %s shares %s.\n", key, value);
                IDirect3DRMFrame *frame = FindGeometry(value);
                if (frame == 0) {
                    GameLog("ERROR: no entry %s to share!\n");
                } else {
                    TopGeometryTable()->Insert(String(key), &frame, -1);
                    frame->AddRef();
                }
            } else {
                LoadGeometry(keys->Get(i).c_str(), value);
            }
            if (bare) {
                SetCurrentDirectory(gSavedDirectory);
                strcpy(gSavedDirectory, "");
            }
            i++;
        } while (i < keys->count);
    }
}
