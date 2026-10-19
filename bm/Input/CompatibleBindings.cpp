// 0x43c650: determine whether two command bindings can share an input.
#include "compat41.h"

struct BindingKinds {
    int Type(int command);     // 0x43dbe0
    int Category(int command); // 0x43dc00
};
struct InputCompatibility {
    void *pad0;
    BindingKinds *bindings;
    bool Compatible(int first, int second);
};

bool InputCompatibility::Compatible(int first, int second)
{
    if (bindings->Type(first) == 2 || bindings->Type(second) == 2 ||
        bindings->Type(first) == 3 || bindings->Type(second) == 3)
        return false;
    if (bindings->Type(first) == 4 || bindings->Type(second) == 4)
        return false;
    int a = bindings->Category(first);
    int b = bindings->Category(second);
    if (a == b)
        return true;
    bool compatible = false;
    if (a == 0 && b != 3)
        return true;
    if (b == 0 && a != 3)
        return true;
    switch (a) {
    case 1:
        if (b == 0) compatible = true;
        if (b == 3) compatible = true;
        break;
    case 2:
        if (b == 0) compatible = true;
        if (b == 3) compatible = true;
        break;
    }
    if (!compatible) {
        switch (b) {
        case 1:
            if (a == 0) compatible = true;
            if (a == 3) return true;
            break;
        case 2:
            if (a == 0) compatible = true;
            if (a == 3) return true;
            break;
        }
    }
    return compatible;
}
