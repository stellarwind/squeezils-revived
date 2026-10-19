// Standalone forwarding thunks, in original address order.
// Keep callees out of this translation unit so VC4.1 emits tail jumps.

void IniCloseAll();                         // 0x4430e0
void RegisterBufferPrototype_45d270();        // 0x45d270: builds a buffer prototype, calls 0x448ce0
void RegisterBufferPrototype_45d430();        // 0x45d430: builds a buffer prototype, calls 0x448ce0

// @00443260 Used by location teardown (0x4130b0).
void IniCloseAllThunk()
{
    IniCloseAll();
}

// @0045d260 Startup forwarding entry for the buffer prototype registration routine.
void RegisterBufferPrototypeThunk_45d260()
{
    RegisterBufferPrototype_45d270();
}

// @0045d420 Startup forwarding entry for the buffer prototype registration routine.
void RegisterBufferPrototypeThunk_45d420()
{
    RegisterBufferPrototype_45d430();
}
