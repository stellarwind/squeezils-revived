// PlayMovieFunction (vtable 0x49e7d4): the script command that plays a movie file.
#pragma once
#include "FunctionArgs/FunctionArgs.h"
#include "FunctionMisc/MiscClasses.h"

struct MoviePlayer {
    MoviePlayer();                                       // 0x418b90 (out of line: state = 0)
    int Play(const char *name, float fadeSeconds);       // 0x418aa0 (ignores this; uses the global player)
    int state;
    char data[0x44];
};
extern MoviePlayer *g_moviePlayer;                       // 0x4c75e4

struct MovieArgList {
    virtual String ReadString(int flag);                 // slot 0
};

class PlayMovieFunction : public Function {
public:
    PlayMovieFunction() { fade = 0; }
    ~PlayMovieFunction();                                // 0x4418a0
    virtual int Execute(ScriptCall *args, int extra);    // 0x441aa0
    virtual Function *Create(ArgList *list);             // 0x441970
    virtual void Print();                                // 0x441a60
    virtual const char *Name();                          // 0x441940
    FloatFunctionArgument *fade;                         // 0x04
    String file;                                         // 0x08
};
