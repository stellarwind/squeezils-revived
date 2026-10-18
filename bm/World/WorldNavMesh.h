// Navigation mesh interface; allocation size confirmed at SQZ.EXE 0x436410.
#pragma once
struct NavMesh {
    char storage[0x2030];
    NavMesh(); // 0x47aec0
    ~NavMesh(); // 0x47af00
    int Load(const char *name); // 0x47c8e0
};
