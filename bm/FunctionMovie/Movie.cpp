// PlayMovieFunction (SQZ.EXE 0x441940-0x441b70).
#include "FunctionMovie/Movie.h"

Function *PlayMovieFunction::Create(ArgList *list)
{
    PlayMovieFunction *f = new PlayMovieFunction;
    f->fade = ParseFloatArgument(list);
    f->file = ((MovieArgList *)list)->ReadString(0);
    return f;
}

int PlayMovieFunction::Execute(ScriptCall *args, int extra)
{
    delete g_moviePlayer;
    g_moviePlayer = new MoviePlayer;
    if (g_moviePlayer->Play(file.c_str(), fade->Get(extra))) {
        delete g_moviePlayer;
        g_moviePlayer = 0;
    }
    return 1;
}

void PlayMovieFunction::Print()
{
    DebugLog("%s", Name());
    fade->Print();
    DebugLog("%s", file.c_str());
}

const char *PlayMovieFunction::Name() { return "PlayMovie"; }

PlayMovieFunction::~PlayMovieFunction()
{
    delete fade;
}
