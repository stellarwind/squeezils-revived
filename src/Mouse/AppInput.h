#pragma once
#include "compat41.h"

struct AppInputSink {
    virtual void v0();
    virtual void Command(int code, const char *text); // +0x04
    virtual void v2(); virtual void v3(); virtual void v4();
    virtual void BeginFirst();                      // +0x14
    virtual void EndFirst(float strength);          // +0x18
    virtual void BeginSecond();                     // +0x1c
    virtual void EndSecond(float strength);         // +0x20
};
struct AppInputAxis { unsigned down, up; int direction; };
struct App {
    char pad[0x18];
    AppInputSink *sink;                             // +0x18
    char pad1[0x30 - 0x1c];
    AppInputAxis axisA;                             // +0x30
    int heldA;                                     // +0x3c
    float valueA;                                  // +0x40
    AppInputAxis axisB;                             // +0x44
    int heldB;                                     // +0x50
    float valueB;                                  // +0x54
    int f58;
    float firstDuration, secondDuration;            // +0x5c, +0x60
    int firstButton, secondButton;                  // +0x64, +0x68
    int firstHeld, secondHeld;                      // +0x6c, +0x70
    int f74;
    int modal;                                     // +0x78
    char pad2[0x88 - 0x7c];
    unsigned firstDown, firstUp;                    // +0x88, +0x8c
    unsigned secondDown, secondUp;                  // +0x90, +0x94
    char pad3[0xb4 - 0x98];
    int mapped;                                    // +0xb4
    int Mapped(int code, int arg);                  // 0x43b120
    int Default(int code, int arg);                 // 0x43b5a0
    int Fallback(int code, int arg);                // 0x43b7c0
    int Dispatch(int code, int arg);                // 0x43b0d0
    void FirstDown();                              // 0x43bb90
    void FirstUp();                                // 0x43bba0
    void SecondDown();                             // 0x43bbb0
    void SecondUp();                               // 0x43bbc0
};
