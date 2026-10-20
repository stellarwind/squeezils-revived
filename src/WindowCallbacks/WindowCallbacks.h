// Callback registry layout from SQZ.EXE 0x477e60 onward.
#pragma once
struct WindowCallbackSlot {
    void *handler;
    void *context;
};
struct WindowCallbacks {
    WindowCallbackSlot slots[16];
    void *accelerators; // HACCEL, 0x80
    unsigned long quitStatus; // WM_QUIT wParam, 0x84
    WindowCallbacks();  // 0x477e60
    ~WindowCallbacks(); // 0x477ea0
    int PumpPending();  // 0x477fd0: 0 quit, 1 idle, 2 messages processed
};
extern WindowCallbacks *gWindowCallbacks; // 0x4c1afc
