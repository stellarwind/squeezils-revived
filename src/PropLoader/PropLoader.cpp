// ReadPropFiles1/2/3 (0x429f40/0x429f60/0x429f80): ReadPropFiles(ini, props, paths).
// The two global-table addresses are masked immediates, so one source matches all three.
extern void ReadPropFiles(void *ini, void *props, void *paths);   // 0x429fa0

void ReadPropFiles1(void *ini)
{
    ReadPropFiles(ini, (void *)0x4c7208, (void *)0x4c7280);
}

extern void ReadPropFiles2(void *ini);   // 0x429f60
extern void ReadPropFiles3(void *ini);   // 0x429f80

int __stdcall WorldReadPropFiles(void *ini, void *other)
{
    if (other) {
        ReadPropFiles3(other);
        return 1;
    }
    ReadPropFiles2(ini);
    return 1;
}
