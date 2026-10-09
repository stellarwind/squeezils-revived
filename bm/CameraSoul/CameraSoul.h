// CameraSoul "do nothing" virtual slots (0x40b350-0x40b6c0): trivial stubs of the CameraSoul/InputEventTarget vtables.
// Each is a leaf (returns 0 / 5, or an empty void) with a fixed number of stack args. Generic names; only the signature matters.
#pragma once
#include "compat41.h"

struct CameraSoulStubs {
    int  return0arg1(int a);                            // 0x40b350 ret 4, returns 0
    int  return5();                                     // 0x40b490 ret, returns 5
    int  return0();                                     // 0x40b5e0 ret, returns 0
    int  return0b();                                    // 0x40b5f0 ret, returns 0
    void void1(int a);                                  // 0x40b600 ret 4
    void void0();                                       // 0x40b610 ret
    void void1b(int a);                                 // 0x40b620 ret 4
    void void0b();                                      // 0x40b630 ret
    void void0c();                                      // 0x40b640 ret
    void void2(int a, int b);                           // 0x40b650 ret 8
    void void6(int a, int b, int c, int d, int e, int f); // 0x40b660 ret 0x18
    void void1c(int a);                                 // 0x40b670 ret 4
    void void4(int a, int b, int c, int d);             // 0x40b680 ret 0x10
    void void0d();                                      // 0x40b690 ret
    void void1d(int a);                                 // 0x40b6a0 ret 4
    void void0e();                                      // 0x40b6b0 ret
    void void1e(int a);                                 // 0x40b6c0 ret 4
    void void6b(int a, int b, int c, int d, int e, int f); // 0x40a620 ret 0x18
    void void6c(int a, int b, int c, int d, int e, int f); // 0x40b330 ret 0x18
    int  return0arg1b(int a);                           // 0x404fa0 ret 4, returns 0
    int  return0arg1c(int a);                           // 0x45a7f0 ret 4, returns 0
    int  return0arg2(int a, int b);                     // 0x45a820 ret 8, returns 0
    int  return1arg1(int a);                            // 0x41fb00 ret 4, returns 1
};
