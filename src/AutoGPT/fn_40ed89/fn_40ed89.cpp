// The destructor at 0x401660 is empty; Vector3f owns no resources.
struct Vector3f {
    float x, y, z;
    ~Vector3f();
};

struct CleanupObject {
    unsigned char precedingMembers[0xdc];
    Vector3f vectorAtDC;
};

// 0x40ed89 is a destructor cleanup within 0x40ec90, not an ordinary
// callable function. Its implicit object comes from [parent EBP - 0x10].
// Expose that context as a parameter in this standalone implementation.
// The original inherited-frame calling convention cannot be expressed by
// this C++ entry point without reconstructing the enclosing lifetime.
extern "C" void __cdecl fn_40ed89(CleanupObject *object)
{
    object->vectorAtDC.~Vector3f();
}

// Vector3f::~Vector3f (0x401660) is defined in AutoGPT/fn_40ed7b.
