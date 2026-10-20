#pragma once
struct IPoint { int x, y; };
struct Pointer {
    short x, y;
    char pad[0x10 - 4];
    int width, height;
    int flag;
    void Center(int how); // 0x4591b0
    void MoveTo(IPoint p); // 0x459150
};
