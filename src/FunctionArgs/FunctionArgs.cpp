// FunctionArgument constructors (SQZ.EXE 0x420c80-0x420e70).
#include "FunctionArgs/FunctionArgs.h"

FunctionArgument::FunctionArgument()
{
    literal = 1;
}

FunctionArgument::FunctionArgument(const char *variable)
{
    literal = 0;
    name = String(variable);
}


void FunctionArgument::Print() {}

IntFunctionArgument::IntFunctionArgument(int v)
{
    literal = 1;
    value = v;
}

IntFunctionArgument::IntFunctionArgument(const char *variable) : FunctionArgument(variable)
{
}

void IntFunctionArgument::Print() {}
