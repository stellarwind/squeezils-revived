// 0x425310: read the texture resource lists named in [InputFiles], then the main ini's own.
#include <string.h>
#include "String/String.h"
#include "IniReader/IniReader.h"

struct TextureTable {
    char pad[0x28];
    TextureTable();                          // 0x467a70
    void *operator new(unsigned size);       // 0x48ea00
};
struct TextureStack {
    int Insert(TextureTable *const &t, int hint);             // 0x466c80 (Array<TextureTable *>)
};
struct Splash {
    virtual void v0();
    virtual void Step();                     // slot 1
};
extern TextureStack g_textureStack;          // 0x4c7180
extern Splash *g_splash;                     // 0x4c75e8
extern int g_textureBytes;
void GameLog(const char *fmt, ...);          // 0x419290
void ReadTextureList(IniReader *ini);        // 0x425500

void LoadTextureResources(IniReader *ini)
{
    g_textureStack.Insert(new TextureTable, -1);

    ini->SetSection("InputFiles");
    StringList *lists = ini->GetKeys();
    for (int i = 0; i < lists->count; i++) {
        String file(ini->GetString(lists->Get(i).c_str()));
        if (strlen(file.c_str()) != 0) {
            IniReader list(file.c_str());
            GameLog("Reading texture resource list from %s...\n", list.GetPath());
            ReadTextureList(&list);
            if (i % 2 == 0)
                g_splash->Step();
        }
    }
    delete lists;

    ReadTextureList(ini);
    GameLog("MELTDOWN DEBUG: total texture size now %d (%x).\n", g_textureBytes, g_textureBytes);
}
