// Vector3f::~Vector3f at 0x401660 has no side effects.
struct Vector3f {
    float x, y, z;
    ~Vector3f();
};

// Only the layout needed by this cleanup is known here.
struct CleanupObject {
    unsigned char precedingMembers[0xe8];
    Vector3f vectorAtE8;
};

// Original: compiler-generated destructor cleanup, entered with its parent's
// EBP intact. The object pointer lives at [EBP-0x10]; the funclet destroys
// the member at object+0xe8. Expose that implicit context as a C++ parameter.
// This preserves the member destruction, but is not the original funclet ABI.
extern "C" void __cdecl fn_40ed7b(CleanupObject *object)
{
    object->vectorAtE8.~Vector3f();
}

Vector3f::~Vector3f() {}
