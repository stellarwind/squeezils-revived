// NavSurfaceList::DestroyLoaded (SQZ.EXE 0x40ede0): reset the list, then delete every loaded surface from the end and empty the array.
struct LoadedSurface {
    int first;                      // 0x00
    ~LoadedSurface();               // 0x40bee0
};
class LoadedArray {
public:
    virtual LoadedSurface *Get(int i);     // slot 0
    virtual int Insert(LoadedSurface *const &v, int at);
    virtual void Remove(int i);            // slot 2
    int count;                             // 0x04
    int capacity;
    LoadedSurface **data;
};
struct NavSurfaceListD {
    char pad[0x124];
    LoadedArray loaded;                    // 0x124
    void Reset();                          // 0x40ec90
    void DestroyLoaded();
};

// Declarations of the neighbouring NavSurfaceList functions of the original unit (the compiler's register choice depends on the symbol count).
void DeclPad1();
void DeclPad2();
void DeclPad3();
void DeclPad4();
void DeclPad5();
void DeclPad6();
void DeclPad7();
void DeclPad8();

void NavSurfaceListD::DestroyLoaded()
{
    Reset();
    for (int i = loaded.count - 1; i >= 0; i--) {
        if (i == 0)
            loaded.Get(i)->first = 0;
        LoadedSurface *s = loaded.Get(i);
        if (s)
            { s->~LoadedSurface(); operator delete(s); }
        loaded.Remove(i);
    }
}
