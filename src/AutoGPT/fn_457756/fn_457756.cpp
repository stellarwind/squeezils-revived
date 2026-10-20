// 0x457756: Vector3f lifetime cleanup in the function at 0x4574b0.
// The original compiler-generated funclet takes no stack arguments and
// inherits its parent's EBP. Its sole operation is to destroy the automatic
// vector at EBP - 0x54, using the empty destructor at 0x401660.
// Standalone C++ needs that implicit parent-frame context passed explicitly.
// In the enclosing source this operation belongs to the vector's automatic
// lifetime, including exception unwinding, rather than a separate API call.
struct Vector3f {
    float x, y, z;
    ~Vector3f(); // Original out-of-line destructor: 0x401660 (__thiscall).
};

extern "C" void __cdecl fn_457756(void *parentFrame)
{
    Vector3f *vector = reinterpret_cast<Vector3f *>(
        static_cast<char *>(parentFrame) - 0x54);
    vector->~Vector3f();
}
