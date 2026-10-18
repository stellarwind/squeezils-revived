// SQZ.EXE 0x456e60: destructor of a counted array of 12-byte parts (count +0, data +4) that also holds a part member at +8.
struct Part { int v[3]; Part() {} ~Part(); };
struct C10 {
    int count;                  // +0
    Part *data;                 // +4
    Part tail;                  // +8
    ~C10();
};
C10::~C10()
{
    if (count > 0 && data) {
        delete [] data;
        data = 0;
        count = 0;
    }
}
