#pragma once
// SIZE 0x0c: navigation point storage; construction deliberately leaves values alone.
struct Vector3fEmpty {
    float x, y, z; // 0x00, 0x04, 0x08
    Vector3fEmpty(); // 0x457040, owner Vector3fEmpty.cpp
    ~Vector3fEmpty(); // 0x401660, owner Vector3fEmpty.cpp
};
