// QvLib parser helper (0x480490): skip the rest of a {...} block, counting nested braces.
struct QvInput { int Read(char *c); };      // 0x4810f0
void SkipBlock(QvInput *in)
{
    char c;
    int depth = 1;
    do {
        if (!in->Read(&c))
            return;
        if (c == '}')
            depth--;
        else if (c == '{')
            depth++;
    } while (depth > 0);
}
