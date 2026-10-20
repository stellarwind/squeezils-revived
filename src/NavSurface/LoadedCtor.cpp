// LoadedSurface constructor (0x40bc60): retain names and index the surface types.
#define ARRAY_COPY_CTOR
#include "Ini/IniFile.h"
typedef BufferObjectTable<String, int, PrimitiveArray<int, 1>, 21> LoadedTypeNumbers;
struct LoadedMesh {
    float GetSizeSquared(); // 0x47af20
};
struct LoadedSurfaceCtor {
    LoadedMesh *mesh;
    float sizeSquared;
    int faceCount;
    String directory, file;
    StringList typeNames;
    LoadedTypeNumbers typeNumbers;
    LoadedSurfaceCtor(LoadedMesh *navMesh, int faces, const char *folder,
        const char *natFile, StringList *types);
};
LoadedSurfaceCtor::LoadedSurfaceCtor(LoadedMesh *navMesh, int faces, const char *folder,
    const char *natFile, StringList *types) : typeNames(*types)
{
    mesh = navMesh;
    sizeSquared = navMesh->GetSizeSquared();
    faceCount = faces;
    { String value(folder); directory = value; }
    { String value(natFile); file = value; }
    for (int i = 0; i < typeNames.count; ++i)
        typeNumbers.Insert(typeNames.Get(i), i, -1);
}
