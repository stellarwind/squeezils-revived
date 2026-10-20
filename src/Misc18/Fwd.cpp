// SQZ.EXE 0x421c90: forwards to the list object at +4: Insert(v, -1) (append).
struct ListB { virtual void s0(); virtual void Insert(const int &v, int at); };
struct Holder7 {
    int pad;
    ListB b;                    // +4
    void Append(int v);
};
void Holder7::Append(int v) { b.Insert(v, -1); }
