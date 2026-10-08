// VirtualCall: call a virtual function of an object from the original game by its slot number.
//
// Objects the original code made carry a pointer to a table of function addresses (the vtable) in their first four bytes. A
// virtual call looks up the function for this object's own class in that table and calls it, so the right version runs for a
// mesh, a sprite, and so on. Most of our reconstructed classes are shared with original code and keep that original table, so
// where we need the same lookup we do it by hand: slot N of the object's table, called with the 32-bit Windows "thiscall"
// convention the original compiler used (the object in ECX, the other arguments on the stack, the callee cleans them up).
//
// Why not just declare the functions `virtual`? Our compiler would build tables of its own, which the original code can't use,
// and it passes some arguments differently (a String by value, for one), which has crashed the game before. Once everything is
// compiled together these can become ordinary virtual functions.
//
// Usage: CallVirtual<ReturnType>(object, slot, arguments...);   For example, slot 3 of a Prop builds its frame:
//     IDirect3DRMFrame *frame = CallVirtual<IDirect3DRMFrame *>(prop, 3, parentFrame);
// The argument types are taken from the arguments as written, so pass exactly what the original function takes (a Float3 by
// value is copied onto the stack, as the original does).
#pragma once

template<class Result, class... Arguments>
Result CallVirtual(void *object, int slot, Arguments... arguments)
{
    typedef Result (__attribute__((thiscall)) *Function)(void *, Arguments...);
    void **vtable = *(void ***)object;
    return ((Function)vtable[slot])(object, arguments...);
}
