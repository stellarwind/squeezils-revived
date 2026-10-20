// SQZ.EXE 0x422010 (cdecl): build a CompoundFunction from the ini section [name]: every key but Class holds one
// script command, parsed into a function and appended. Returns 0 (and frees the compound) when a command fails.
#include <string.h>
#pragma intrinsic(strcmp, strlen)
#include "IniReader/IniReader.h"
#include "FunctionArgs/FunctionArgs.h"

class LoadedCompound : public Function {
public:
    LoadedCompound() {}
    ~LoadedCompound();                                   // 0x421b00
    virtual int Execute(ScriptCall *args, int extra);    // 0x421bd0
    virtual Function *Create(ArgList *list);             // 0x421c30
    virtual void Print();                                // 0x421c40
    virtual const char *Name();                          // 0x421ba0
    void Append(Function *item);                         // 0x421c90
    struct List { List(); ~List(); char body[0x10]; } items;   // 0x04 (ctor 0x46b780, dtor 0x46b7b0)
};
StringList *SplitWords(const char *text);              // 0x421db0
Function *ParseFunction(ArgList *list);                // 0x421f50
void DebugLog(const char *format, ...);                // 0x419290
extern const char kParsingFunction[];                  // 0x4bb1d0
extern const char kClass[];                            // 0x4b9e28
extern const char kFunctionClass[];                    // 0x4bb1c4
extern const char kNotAFunction[];                     // 0x4bb190
extern const char kParseFailed[];                      // 0x4ba08c
extern const char kParsedOk[];                         // 0x4bb180

Function *LoadFunction(IniReader *ini, const char *name)
{
    DebugLog(kParsingFunction, name);
    String section(ini->section);
    ini->SetSection(name);
    if (strcmp(ini->GetString(kClass), kFunctionClass))
        DebugLog(kNotAFunction, name);
    LoadedCompound *compound = new LoadedCompound;
    StringList *keys = ini->GetKeys();
    int i = 0;
    for (; i < keys->count; i++) {
        int isClass = !strcmp(keys->Get(i).c_str(), kClass);
        if (!isClass) {
            String text(ini->GetString(keys->Get(i).c_str()));
            if (!strlen(text.c_str()))
                break;
            StringList *words = SplitWords(text.c_str());
            Function *fn = ParseFunction((ArgList *)words);
            if (words)
                delete words;
            if (!fn) {
                DebugLog(kParseFailed);
                if (compound)
                    delete compound;
                ini->SetSection(section.c_str());
                if (keys)
                    delete keys;
                return 0;
            }
            compound->Append(fn);
        }
    }
    if (keys)
        delete keys;
    ini->SetSection(section.c_str());
    DebugLog(kParsedOk);
    compound->Print();
    return compound;
}
