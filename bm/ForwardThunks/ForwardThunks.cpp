// Standalone forwarding thunks, in original address order.
// Keep callees out of this translation unit so VC4.1 emits tail jumps.

void IniCloseAll();                         // 0x4430e0
// Each startup callee constructs a buffer prototype and registers it at 0x448ce0.
void RegisterBufferPrototype_45c2e0();        // 0x45c2e0
void RegisterBufferPrototype_45c3c0();        // 0x45c3c0
void RegisterBufferPrototype_45c4a0();        // 0x45c4a0
void RegisterBufferPrototype_45c640();        // 0x45c640
void RegisterBufferPrototype_45c760();        // 0x45c760
void RegisterBufferPrototype_45c850();        // 0x45c850
void RegisterBufferPrototype_45c980();        // 0x45c980
void RegisterBufferPrototype_45cb00();        // 0x45cb00
void RegisterBufferPrototype_45ccd0();        // 0x45ccd0
void RegisterBufferPrototype_45cea0();        // 0x45cea0
void RegisterBufferPrototype_45d0b0();        // 0x45d0b0
void RegisterBufferPrototype_45d270();        // 0x45d270
void RegisterBufferPrototype_45d430();        // 0x45d430
void RegisterBufferPrototype_45d5c0();        // 0x45d5c0

// @00443260 Used by location teardown (0x4130b0).
void IniCloseAllThunk()
{
    IniCloseAll();
}

// @0045c2d0 Startup forwarding entry for the buffer prototype registration routine.
void RegisterBufferPrototypeThunk_45c2d0()
{
    RegisterBufferPrototype_45c2e0();
}

// @0045c3b0 Startup forwarding entry for the buffer prototype registration routine.
void RegisterBufferPrototypeThunk_45c3b0()
{
    RegisterBufferPrototype_45c3c0();
}

// @0045c490 Startup forwarding entry for the buffer prototype registration routine.
void RegisterBufferPrototypeThunk_45c490()
{
    RegisterBufferPrototype_45c4a0();
}

// @0045c630 Startup forwarding entry for the buffer prototype registration routine.
void RegisterBufferPrototypeThunk_45c630()
{
    RegisterBufferPrototype_45c640();
}

// @0045c750 Startup forwarding entry for the buffer prototype registration routine.
void RegisterBufferPrototypeThunk_45c750()
{
    RegisterBufferPrototype_45c760();
}

// @0045c840 Startup forwarding entry for the buffer prototype registration routine.
void RegisterBufferPrototypeThunk_45c840()
{
    RegisterBufferPrototype_45c850();
}

// @0045c970 Startup forwarding entry for the buffer prototype registration routine.
void RegisterBufferPrototypeThunk_45c970()
{
    RegisterBufferPrototype_45c980();
}

// @0045caf0 Startup forwarding entry for the buffer prototype registration routine.
void RegisterBufferPrototypeThunk_45caf0()
{
    RegisterBufferPrototype_45cb00();
}

// @0045ccc0 Startup forwarding entry for the buffer prototype registration routine.
void RegisterBufferPrototypeThunk_45ccc0()
{
    RegisterBufferPrototype_45ccd0();
}

// @0045ce90 Startup forwarding entry for the buffer prototype registration routine.
void RegisterBufferPrototypeThunk_45ce90()
{
    RegisterBufferPrototype_45cea0();
}

// @0045d0a0 Startup forwarding entry for the buffer prototype registration routine.
void RegisterBufferPrototypeThunk_45d0a0()
{
    RegisterBufferPrototype_45d0b0();
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

// @0045d5b0 Startup forwarding entry for the buffer prototype registration routine.
void RegisterBufferPrototypeThunk_45d5b0()
{
    RegisterBufferPrototype_45d5c0();
}
