// LoadPaletteFile (0x474ca0): reads a JASC-PAL palette file (256 entries) into RGBA quads.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int LoadPaletteFile(const char *path, unsigned char *rgba)
{
    char line[256];
    int r, g, b;
    if (path != 0 && strcmp(path, "") != 0) {
        FILE *f = fopen(path, "r");
        if (f == 0)
            return 0;
        fgets(line, 255, f);
        if (strncmp(line, "JASC-PAL", 8) != 0)
            return 0;
        fgets(line, 255, f);
        fgets(line, 255, f);
        if (atoi(line) != 256)
            return 0;
        for (int i = 0; i < 256; i++) {
            fgets(line, 255, f);
            sscanf(line, "%d %d %d", &r, &g, &b);
            rgba[i * 4] = (unsigned char)r;
            rgba[i * 4 + 1] = (unsigned char)g;
            rgba[i * 4 + 2] = (unsigned char)b;
        }
        fclose(f);
        return 1;
    }
    return 0;
}
