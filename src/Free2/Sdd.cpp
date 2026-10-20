// Slot-0 scalar deleting destructor of a script command class whose complete destructor is a separate function:
//   push esi; mov esi,ecx; call ~T; test byte ptr [esp+8],1; je skip; push esi; call operator delete; add esp,4; skip: mov eax,esi; pop esi; ret 4
// 58 classes share this body at 0x47e6f0-0x48b860 (the ~T call is a masked relocation). The byte test (instead of
// mov eax,[esp+8]; test al,1) comes from testing the flags through a char cast.
struct T {
    void Dtor();
    T *Sdd(int flags);
};
T *T::Sdd(int flags)
{
    Dtor();
    if ((char)flags & 1)
        delete this;
    return this;
}
