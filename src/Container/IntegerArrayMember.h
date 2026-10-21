// Integer-array member complete destructor, original 0x40c420.
#pragma once
#include "Container/ContainerArray.h"

// SIZE 0x14: Array<int> owns backing storage; trailing BufferObject vptr has no lifetime work.
struct C40c420 {
    Array<int> array; // 0x00: vptr/count/capacity/data at +0/+4/+8/+c
    void *bufferVirtualTable; // 0x10
    ~C40c420(); // 0x40c420: complete owner Misc17/Dt2.cpp
};
