// SQZ.EXE 0x422730 (cdecl): push a new function table, then read the script functions from every input file listed
// in [InputFiles] (their [FunctionList] sections) and from this file's own [FunctionList], stepping the splash bar.
#include <string.h>
#include "IniReader/IniReader.h"

struct FunctionTable { FunctionTable(); char body[0x28]; };   // 0x46b930
struct FunctionTableStack { void Insert(FunctionTable *const &item, int at); };   // 0x46c4a0
extern FunctionTableStack gFunctionTables;             // 0x4c7090
struct Splash {
    virtual void v0();
    virtual void Step();                               // slot 1
};
extern Splash *g_splash;                               // 0x4c75e8
struct ScriptFunction;
ScriptFunction *LoadFunction(IniReader *ini, const char *name);   // 0x422010 (cdecl)
void RegisterFunction(const char *name, ...);          // 0x422be0
void DebugLog(const char *format, ...);                // 0x419290
extern const char kInputFiles[];                       // 0x4b9d64
extern const char kFunctionList[];                     // 0x4bb1e8
extern const char kReadingFunctionList[];              // 0x4bb1f8
extern const char kNoInputFile[];                      // 0x4b9d40

void LoadFunctions(IniReader *ini)
{
    gFunctionTables.Insert(new FunctionTable, -1);
    ini->SetSection(kInputFiles);
    StringList *files = ini->GetKeys();
    int loaded = 0;
    int step = files->count < 6 ? 1 : files->count / 6;
    int i = 0;
    for (; i < files->count; i++) {
        String file(ini->GetString(files->Get(i).c_str()));
        if (strlen(file.c_str())) {
            IniReader sub(file.c_str());
            DebugLog(kReadingFunctionList, sub.GetPath());
            sub.SetSection(kFunctionList);
            StringList *names = sub.GetKeys();
            int j = 0;
            for (; j < names->count; j++) {
                sub.SetSection(kFunctionList);
                String name(sub.GetString(names->Get(j).c_str()));
                ScriptFunction *fn = LoadFunction(&sub, name.c_str());
                if (fn)
                    RegisterFunction(name.c_str(), fn);
                if (loaded % step == 0 || step == 1)
                    g_splash->Step();
                loaded++;
            }
            if (names)
                delete names;
        } else {
            DebugLog(kNoInputFile, i);
        }
    }
    if (files)
        delete files;
    ini->SetSection(kFunctionList);
    StringList *names = ini->GetKeys();
    int k = 0;
    for (; k < names->count; k++) {
        String name(ini->GetString(names->Get(k).c_str()));
        ScriptFunction *fn = LoadFunction(ini, name.c_str());
        if (fn)
            RegisterFunction(name.c_str(), fn);
    }
    if (names)
        delete names;
}
