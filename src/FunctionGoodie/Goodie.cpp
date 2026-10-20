// CreateGoodieAtPositionFunction small slots (SQZ.EXE 0x440c10-0x440f80).
#include "FunctionGoodie/Goodie.h"

void CreateGoodieAtPositionFunction::ParseExtra(ArgList *list) {}
void CreateGoodieAtPositionFunction::DumpExtra() {}

CreateGoodieAtPositionFunction *CreateGoodieAtPositionFunction::Make()
{
    return new CreateGoodieAtPositionFunction;
}

Function *CreateGoodieAtPositionFunction::Create(ArgList *list)
{
    CreateGoodieAtPositionFunction *f = Make();
    f->name = ParseStringArgument(list);
    f->position = ParseVectorArgument(list, Vector3f(0, 0, 0));
    f->direction = ParseVectorArgument(list, Vector3f(0, 1, 0));
    f->ParseExtra(list);
    return f;
}

void CreateGoodieAtPositionFunction::Print()
{
    DebugLogAll("%s", Name());
    name->Print();
    position->Print();
    direction->Print();
    DumpExtra();
    DebugLogAll("\n");
}

const char *CreateGoodieAtPositionFunction::Name() { return "CreateGoodieAtPosition"; }

const char *CreateGoodieAtPositionWithVelocityFunction::Name() { return "CreateGoodieAtPositionWithVelocity"; }

void CreateGoodieAtPositionWithVelocityFunction::ParseExtra(ArgList *list)
{
    velocity = ParseVectorArgument(list, Vector3f(0, 0, 0));
    speed = ParseFloatArgument(list);
}

void CreateGoodieAtPositionWithVelocityFunction::DumpExtra()
{
    velocity->Print();
    speed->Print();
}

CreateGoodieAtPositionFunction *CreateGoodieAtPositionWithVelocityFunction::Make()
{
    return new CreateGoodieAtPositionWithVelocityFunction;
}
