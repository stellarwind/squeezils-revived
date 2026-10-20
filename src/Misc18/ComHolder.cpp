// SQZ.EXE 0x47a0f0 (constructor) and 0x47a100 (Release): a colour plus a COM object pointer.
#include <windows.h>
struct ComHolder {
    unsigned color;             // +0
    IUnknown *obj;              // +4
    ComHolder();
    void Release();
};
ComHolder::ComHolder() { color = 0xff000000; obj = 0; }
void ComHolder::Release()
{
    if (obj)
        obj->Release();
    obj = 0;
}
