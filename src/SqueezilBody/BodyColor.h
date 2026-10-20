#pragma once
class IniReader;
const char *BodyColorName(int color); // 0x458fd0
int FindBodyColor(const char *name);  // 0x458f20
int ReadBodyColor(IniReader *ini);    // 0x458f60
