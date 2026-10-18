// By-value decal argument layouts used by World and Scene.
#pragma once
struct WorldVector {
    float x, y, z;
    WorldVector() {}
    WorldVector Cross(const WorldVector &other) const; // 0x41a350
    void operator=(const WorldVector &other) { x = other.x; y = other.y; z = other.z; }
    WorldVector(const WorldVector &other); // 0x4183c0
    ~WorldVector();                       // 0x401660
};
struct WorldDecalSettings {
    int colors;
    WorldVector size;
    WorldVector origin;
    int useTransparency;
    WorldVector transparentColor;
    WorldDecalSettings(const WorldDecalSettings &other); // 0x425970
    ~WorldDecalSettings();                             // 0x425a30
};
