// SQZ.EXE 0x458130: load the seven model parts of one body color.
#include <stdio.h>
#include <string.h>
#include "IniReader/IniReader.h"

struct ProtoLoadModel {
    char data[0x2c];
    ProtoLoadModel(); // 0x4719d0
    ~ProtoLoadModel(); // 0x471a00
    int Load(const char *filename, int flag); // 0x472560
};
struct ProtoLoadPart {
    char data[0x10];
    ProtoLoadPart(ProtoLoadModel *model); // 0x41d870
};
struct AnimatedBody {
    virtual void AddPart(const char *name, ProtoLoadPart *part); // slot 0
    char data[0x50 - 4];
    AnimatedBody(); // 0x41d9e0
    ~AnimatedBody(); // 0x41dc70
};
struct ProtoLoadModels {
    void Insert(ProtoLoadModel *const &model, int at); // 0x46b180
};
extern ProtoLoadModels gBodyModels; // 0x4c7b98
void GameLog(const char *format, ...); // 0x419290

AnimatedBody *LoadProtoBody(const char *color)
{
    IniReader ini;
    ini.SetSection("Cycles");
    StringList parts;
    parts.Insert(String("body"), -1);
    parts.Insert(String("foot_l"), -1);
    parts.Insert(String("foot_r"), -1);
    parts.Insert(String("hand_l"), -1);
    parts.Insert(String("hand_r"), -1);
    parts.Insert(String("head"), -1);
    parts.Insert(String("tail"), -1);
    AnimatedBody *body = new AnimatedBody;
    for (int i=0; i<parts.count; ++i) {
        ProtoLoadModel *model = new ProtoLoadModel;
        char key[80];
        const char *partName = parts.Get(i).c_str();
        sprintf(key, "%s_%s", color, partName);
        GameLog("\tLoading morph body part %s... ", key);
        const char *filename = ini.GetString(key);
        if (strlen(filename) == 0) {
            GameLog("ERROR: no filename entry!\n");
            delete body;
            delete model;
            return 0;
        }
        if (!model->Load(filename, 0)) {
            GameLog("ERROR: Failed to load %s from %s!\n", key, filename);
            delete body;
            delete model;
            return 0;
        }
        ProtoLoadPart *part = new ProtoLoadPart(model);
        body->AddPart(parts.Get(i).c_str(), part);
        GameLog("OK.\n");
        gBodyModels.Insert(model, -1);
    }
    return body;
}
