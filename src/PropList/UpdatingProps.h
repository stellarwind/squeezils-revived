#pragma once
struct Prop;
struct UpdatingPropArray {
    void *vtable;
    int count, capacity;
    Prop **data;
    void Remove(int index); // 0x468fd0
};
struct RemovedPropIndices {
    void *vtable;
    int count, capacity;
    int *data;
    int Insert(const int &index, int hint); // 0x45f000
    void Remove(int index); // 0x45d990
};
extern UpdatingPropArray gUpdatingProps; // 0x4c71f8
extern RemovedPropIndices gRemovedPropIndices; // 0x4c72a8
void FlushRemovedProps(); // 0x42d2b0
