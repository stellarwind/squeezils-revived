// LoadProtoBodies, SQZ.EXE 0x458050.
struct AnimatedBody;
extern const char *g_bodyColorNames[]; // 0x49fdd8
extern AnimatedBody *gProtoBodies[];   // 0x4bf138
AnimatedBody *LoadProtoBody(const char *color); // 0x458130
int DebugLogAll(const char *format, ...); // 0x419290

int LoadProtoBodies()
{
    for (int color = 0; color < 7; ++color) {
        const char *name = g_bodyColorNames[color];
        DebugLogAll("Loading %s proto body...\n", name);
        gProtoBodies[color] = LoadProtoBody(name);
        if (!gProtoBodies[color])
            return 0;
        DebugLogAll("OK.\n");
    }
    return 1;
}
