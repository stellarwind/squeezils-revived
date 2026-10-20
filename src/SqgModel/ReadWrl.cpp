// 0x47a800: read a top-level VRML node from a file or standard input.
#include <stdio.h>
#include <string.h>
#include <errno.h>

struct QvNode;
struct WrlInput {
    char state[0x48];
    WrlInput();                   // 0x480fc0
    ~WrlInput();                  // 0x481050
    void Init(FILE *file);        // 0x4810c0
};
int ReadWrlTop(WrlInput *input, QvNode **node); // 0x481c30
void WrlError(const char *format, ...);        // 0x47a7b0
extern const char kWrlReadMode[];             // 0x4ba008
extern const char kWrlOpenError[];            // 0x4c3c54

QvNode *ReadWrlFile(const char *path)
{
    FILE *file = stdin;
    if (path) {
        file = fopen(path, kWrlReadMode);
        if (!file) {
            WrlError(kWrlOpenError, path, strerror(errno));
            return 0;
        }
    }
    WrlInput input;
    input.Init(file);
    QvNode *node;
    int ok = ReadWrlTop(&input, &node);
    if (file && file != stdin)
        fclose(file);
    if (ok && node)
        return node;
    return 0;
}
