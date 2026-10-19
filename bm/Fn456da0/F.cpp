// DirectionHistory::DirectionHistory(int n) (0x456da0): allocates n+1 empty vectors, then clears.
struct Vector3fEmpty { float x, y, z; Vector3fEmpty(); ~Vector3fEmpty(); };
struct TailVec { float x, y, z; ~TailVec(); };
struct DirectionHistory {
    int count;             // 0x00
    Vector3fEmpty *items;  // 0x04
    TailVec last;            // 0x08
    char pad[0x38 - 0x14];
    DirectionHistory(int n);
    void Clear();          // 0x456ef0
};
DirectionHistory::DirectionHistory(int n)
{
    count = n + 1;
    items = new Vector3fEmpty[n + 1];
    Clear();
}
