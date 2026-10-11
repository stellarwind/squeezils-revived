// QvLib input readers (0x481490 ... 0x481590): skip blanks (0x481740), read a number (0x481820 / 0x481890-style), store it narrowed.
class QvInput {
public:
    int Next();                       // 0x481740
    int ReadInt(int *v);              // 0x481820
    int ReadDouble(double *v);        // float reader
    int GetInt(int *out);
    int GetShort(short *out);
    int GetUInt(unsigned *out);
    int GetLong(long *out);
    int GetFloat(float *out);
};
int QvInput::GetInt(int *out)
{
    int v;
    if (!Next()) return 0;
    int r = ReadInt(&v);
    if (r) *out = v;
    return r;
}
int QvInput::GetShort(short *out)
{
    int v;
    if (!Next()) return 0;
    int r = ReadInt(&v);
    if (r) *out = (short)v;
    return r;
}
int QvInput::GetUInt(unsigned *out)
{
    int v;
    if (!Next()) return 0;
    int r = ReadInt(&v);
    if (r) *out = v;
    return r;
}
int QvInput::GetLong(long *out)
{
    int v;
    if (!Next()) return 0;
    int r = ReadInt(&v);
    if (r) *out = v;
    return r;
}
int QvInput::GetFloat(float *out)
{
    double v;
    if (!Next()) return 0;
    int r = ReadDouble(&v);
    if (r) *out = (float)v;
    return r;
}
