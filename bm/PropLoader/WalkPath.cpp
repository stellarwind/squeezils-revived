// 0x42cea0: consume path words, following "parent" or a named child.
#include "String/String.h"
#include <string.h>
inline int PathWordEquals(const String &word, const char *text)
{
    return _stricmp(word.c_str(), text) == 0;
}
struct PathWords {
    virtual String At(int index);
    virtual void v1();
    virtual void RemoveAt(int index);
    int count;
};
struct PathProp {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual PathProp *GetOwner();
    virtual void v10(); virtual void v11(); virtual void v12();
    virtual PathProp *GetChild(String name);
};
PathProp *WalkPropPath(int unused, PathWords *words, PathProp *prop, int unusedLast)
{
    while (words->count != 0) {
        if (PathWordEquals(words->At(0), "this")) {
            words->RemoveAt(0);
        } else if (PathWordEquals(words->At(0), "parent")) {
            words->RemoveAt(0);
            prop = prop->GetOwner();
        } else {
            PathProp *child = prop->GetChild(words->At(0));
            words->RemoveAt(0);
            prop = child;
        }
    }
    return prop;
}
