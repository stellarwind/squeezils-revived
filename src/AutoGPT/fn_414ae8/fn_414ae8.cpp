// 0x414ae8: cleanup for the enclosing function's temporary string object.
// The original funclet has no stack arguments: it inherits its parent's EBP.
// A standalone C++ function must receive that hidden context explicitly.
// In the enclosing C++ function this cleanup is the implicit destructor call
// for an automatic TemporaryString, on both return and exception unwinding.
// Matching the inherited EBP requires reconstructing that enclosing lifetime;
// the explicit frame parameter below preserves the cleanup's operation.
// The destructor at 0x443240 releases the object's two allocated buffers.
class TemporaryString {
public:
    ~TemporaryString(); // Original implementation: 0x443240 (__thiscall).
};

extern "C" void __cdecl fn_414ae8(void *parentFrame)
{
    TemporaryString *temporary =
        reinterpret_cast<TemporaryString *>(
            static_cast<char *>(parentFrame) - 0x24);
    temporary->~TemporaryString();
}
