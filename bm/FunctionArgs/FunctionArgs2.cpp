// The other FunctionArgument constructors (SQZ.EXE 0x421170-0x423400).
#include "FunctionArgs/FunctionArgs.h"

// Three spare declarations: the compiler's register choices depend on how many symbols the unit has declared (padscan; ParseBehaviorArgument).
void PadDecl1(); void PadDecl2(); void PadDecl3();

struct FaVec { float x, y, z; FaVec() {} FaVec(const FaVec &o) : x(o.x), y(o.y), z(o.z) {} ~FaVec(); };            // dtor 0x401660

struct ArgContext;
class FloatFunctionArgument : public FunctionArgument {
public:
    float value;                                                 // 0x0c
    FloatFunctionArgument(float v);                              // 0x421170
    FloatFunctionArgument(const char *variable);                 // 0x4211e0
    virtual void Print();
};
class Vector3fFunctionArgument : public FunctionArgument {
public:
    FaVec value;                                                 // 0x0c
    Vector3fFunctionArgument(const FaVec &v);                    // 0x421460
    Vector3fFunctionArgument(const char *variable);              // 0x4214f0
    FaVec Get(ArgContext *context);                      // 0x421800
    virtual void Print();
};
class StringFunctionArgument : public FunctionArgument {
public:
    String value;                                                // 0x0c
    StringFunctionArgument(const char *text);                    // 0x421890
    virtual void Print();
};
class BehaviorTypeFunctionArgument : public FunctionArgument {
public:
    int value;                                                   // 0x0c
    BehaviorTypeFunctionArgument(int v);                         // 0x423230
    virtual void Print();
};
class StimulusTypeFunctionArgument : public FunctionArgument {
public:
    int value;                                                   // 0x0c
    StimulusTypeFunctionArgument(int v);                         // 0x423400
    virtual void Print();
};

FloatFunctionArgument::FloatFunctionArgument(float v)
{
    literal = 1;
    value = v;
}

FloatFunctionArgument::FloatFunctionArgument(const char *variable) : FunctionArgument(variable) {}

Vector3fFunctionArgument::Vector3fFunctionArgument(const FaVec &v)
{
    literal = 1;
    value.x = v.x;
    value.y = v.y;
    value.z = v.z;
}

Vector3fFunctionArgument::Vector3fFunctionArgument(const char *variable) : FunctionArgument(variable) {}

StringFunctionArgument::StringFunctionArgument(const char *text)
{
    value = String(text);
}

BehaviorTypeFunctionArgument::BehaviorTypeFunctionArgument(int v)
{
    value = v;
}

StimulusTypeFunctionArgument::StimulusTypeFunctionArgument(int v)
{
    value = v;
}

extern const char *const kBehaviorNames[];                       // 0x49ec60
extern const char *const kStimulusNames[];                       // 0x49ec10
void BehaviorTypeFunctionArgument::Print() { DebugLog("%s ", kBehaviorNames[value]); }     // 0x4232a0
void StimulusTypeFunctionArgument::Print() { DebugLog("%s ", kStimulusNames[value]); }     // 0x423470

struct ArgList {
    virtual String ReadString(int flag);                         // slot 0
    virtual void a1();
    virtual void Skip(int n);                                    // slot 2
    void *cursor;                                                // 0x04 0 when the script text ran out
};

FloatFunctionArgument *ParseFloatArgument(ArgList *list)         // 0x421250
{
    if (!list->cursor) {
        DebugLog("WARNING: missing float argument\n");
        return new FloatFunctionArgument(0.0f);
    }
    float v;
    FloatFunctionArgument *arg;
    int ok = list->ReadString(0).ToFloat(&v);
    if (!ok)
        arg = new FloatFunctionArgument(list->ReadString(0).c_str());
    else
        arg = new FloatFunctionArgument(v);
    list->Skip(0);
    return arg;
}

FunctionArgument *ParseArgument(ArgList *list)                   // 0x420ee0
{
    if (!list->cursor) {
        DebugLog("WARNING: missing int argument, using 0.\n");
        return new IntFunctionArgument(0);
    }
    float v;
    IntFunctionArgument *arg;
    int ok = list->ReadString(0).ToFloat(&v);
    if (!ok) {
        arg = new IntFunctionArgument(list->ReadString(0).c_str());
    } else {
        if ((float)(int)v != v)
            DebugLog("WARNING: truncating expected integer argument %s\n", list->ReadString(0).c_str());
        arg = new IntFunctionArgument((int)v);
    }
    list->Skip(0);
    return arg;
}

struct VarTable { FaVec Lookup(const String &name); };      // 0x46b700
struct ArgContext { char pad[0x60]; VarTable vars; };                    // 0x60

FaVec Vector3fFunctionArgument::Get(ArgContext *context)                 // 0x421800
{
    if (literal)
        return value;
    return context->vars.Lookup(name);
}

int BehaviorIndex(const char *name);                             // 0x448850 (-1: unknown)

BehaviorTypeFunctionArgument *ParseBehaviorArgument(ArgList *list)   // 0x4232c0
{
    if (!list->cursor) {
        DebugLog("WARNING: missing Robo Behavior arguments, using \"BEH_NONE\".\n");
        return new BehaviorTypeFunctionArgument(7);
    }
    String word = list->ReadString(0);
    int type = BehaviorIndex(word.c_str());
    if (type == -1)
        DebugLog("WARNING: unrecognized Robo Behavior name \"%s\".\n", word.rep);
    BehaviorTypeFunctionArgument *arg = new BehaviorTypeFunctionArgument(type);
    list->Skip(0);
    return arg;
}

int StimulusIndex(const char *name);                             // 0x448890 (-1: unknown)

StimulusTypeFunctionArgument *ParseStimulusArgument(ArgList *list)   // 0x423490
{
    if (!list->cursor) {
        DebugLog("WARNING: missing Stimulus arguments, using \"STIM_NONE\".\n");
        return new StimulusTypeFunctionArgument(0x13);
    }
    String word = list->ReadString(0);
    list->Skip(0);
    int type = StimulusIndex(word.c_str());
    if (type == -1)
        DebugLog("WARNING: unrecognized Robo Stimulus name \"%s\".\n", word.rep);
    StimulusTypeFunctionArgument *arg = new StimulusTypeFunctionArgument(type);
    return arg;
}

StringFunctionArgument *ParseStringArgument(ArgList *list)      // 0x421940
{
    if (!list->cursor) {
        DebugLog("WARNING: missing string arguments, using \"BLANK\".\n");
        return new StringFunctionArgument("BLANK");
    }
    StringFunctionArgument *arg = new StringFunctionArgument(list->ReadString(0).c_str());
    list->Skip(0);
    return arg;
}

Vector3fFunctionArgument *ParseVectorArgument(ArgList *list, FaVec fallback)   // 0x421560 (the caller passes the default by value)
{
    FaVec v;
    Vector3fFunctionArgument *arg;
    if (!list->cursor) {
        arg = new Vector3fFunctionArgument(fallback);
    } else {
        float probe;
        int ok = list->ReadString(0).ToFloat(&probe);
        if (!ok) {
            arg = new Vector3fFunctionArgument(list->ReadString(0).c_str());
        } else {
            for (float *p = &v.x; p < &v.x + 3; p++) {
                if (!list->cursor) {
                    DebugLog("WARNING: missing vector element, using 0.\n");
                    *p = 0;
                } else {
                    float f;
                    int good = list->ReadString(0).ToFloat(&f);
                    if (!good) {
                        DebugLog("WARNING: non-numeric vector element, using 0.\n");
                        *p = 0;
                    } else {
                        *p = f;
                    }
                    list->Skip(0);
                }
            }
            arg = new Vector3fFunctionArgument(v);
        }
    }
    return arg;
}
