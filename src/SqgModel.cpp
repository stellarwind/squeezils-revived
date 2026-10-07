// SqgModel: see SqgModel.h. Reconstructed from SQZ.EXE 0x472560-0x473710.
#include "SqgModel.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <new>

// The game's allocator, error reporter and texture lookup, still the original code:
extern void *SqgAlloc(size_t size);                      // function pointer stored at 0x4c0f78 (the game's malloc)
extern void  SqgError(const char *message);              // @00473710
typedef IDirect3DRMTexture *(*SqgTextureLookupFn)(const char *name);
extern SqgTextureLookupFn SqgGetTextureLookup();         // pointer stored at 0x4c0f84, NULL until the game sets it
extern void *Operator_New(size_t size);                  // @0048ea00

static const char kHeader[] = "SQG Format 1.0";          // @004c0f64, the 15 bytes including the terminator are compared

// Reads `count` items; the original only treats the stream's error flag as failure, so a short read goes unnoticed.
static bool ReadOk(FILE *f) { return ferror(f) == 0; }

// @00472560
// File layout: header, texture count + names, vertex count + 0x24-byte vertices, face count, index count + indices,
// group count + groups, delta list count + lists (count + 0x10-byte deltas each).
BOOL SqgModel::Load(const char *path, int mirror)
{
    FILE *f = fopen(path, "rb");
    if (f == NULL)
        return FALSE;

    char header[20];
    fread(header, sizeof kHeader, 1, f);
    if (!ReadOk(f)) { SqgError("Can't read file header"); fclose(f); return FALSE; }
    if (strcmp(header, kHeader) != 0) { SqgError("File header is wrong for SQG file %s"); fclose(f); return FALSE; }

    fread(&textureCount, 4, 1, f);
    if (!ReadOk(f)) { SqgError("Can't read texture count"); fclose(f); return FALSE; }
    if (textureCount > 0) {
        textureNames = (char **)SqgAlloc(textureCount * 4);
        for (int i = 0; i < textureCount; i++) {
            size_t len;
            fread(&len, 4, 1, f);
            if (!ReadOk(f)) { SqgError("Can't read texture name length"); fclose(f); return FALSE; }
            textureNames[i] = (char *)SqgAlloc(len);
            fread(textureNames[i], len, 1, f);
            if (!ReadOk(f)) { SqgError("Can't read texture name %d"); fclose(f); return FALSE; }
        }
    }

    fread(&vertexCount, 4, 1, f);
    if (!ReadOk(f)) { SqgError("Can't read vertex count"); fclose(f); return FALSE; }
    if (vertexCount < 1) { SqgError("Vertex count is <= 0"); fclose(f); return FALSE; }
    vertices = (D3DRMVERTEX *)SqgAlloc(vertexCount * sizeof(D3DRMVERTEX));
    fread(vertices, sizeof(D3DRMVERTEX), vertexCount, f);
    if (!ReadOk(f)) { SqgError("Can't read vertices"); fclose(f); return FALSE; }

    fread(&faceCount, 4, 1, f);
    if (!ReadOk(f)) { SqgError("Can't read index count"); fclose(f); return FALSE; }
    if (faceCount < 1) { SqgError("Face count is <= 0"); fclose(f); return FALSE; }
    fread(&indexCount, 4, 1, f);
    if (!ReadOk(f)) { SqgError("Can't read index count"); fclose(f); return FALSE; }
    indices = (int *)SqgAlloc(indexCount * 4);
    fread(indices, 4, indexCount, f);
    if (!ReadOk(f)) { SqgError("Can't read vertex indices"); fclose(f); return FALSE; }

    fread(&groupCount, 4, 1, f);
    if (!ReadOk(f)) { SqgError("Can't read group count"); fclose(f); return FALSE; }
    if (groupCount < 1) { SqgError("Triangle count is <= 0"); fclose(f); return FALSE; }
    groups = (SqgGroup *)SqgAlloc(groupCount * sizeof(SqgGroup));
    fread(groups, sizeof(SqgGroup), groupCount, f);
    if (!ReadOk(f)) { SqgError("Can't read groups"); fclose(f); return FALSE; }

    if (mirror) {
        // reverse the vertex order of every face list and flip X
        for (int i = 0; i < indexCount; ) {
            int n = indices[i];
            i++;
            for (int a = i, b = i + n - 1; a < b; a++, b--) {
                int t = indices[a];
                indices[a] = indices[b];
                indices[b] = t;
            }
            i += n;
        }
        for (int v = 0; v < vertexCount; v++)
            vertices[v].position.x = -vertices[v].position.x;
    }

    fread(&deltaListCount, 4, 1, f);
    if (!ReadOk(f)) { SqgError("Can't read delta list count"); fclose(f); return FALSE; }
    if (deltaListCount == 0) {
        fclose(f);
        return TRUE;
    }

    deltaLists = (SqgDeltaList *)SqgAlloc(deltaListCount * sizeof(SqgDeltaList));
    for (int i = 0; i < deltaListCount; i++) {
        int n;
        fread(&n, 4, 1, f);
        if (!ReadOk(f)) { SqgError("Can't read delta count for delta list %d"); fclose(f); return FALSE; }
        deltaLists[i].count = n;
        deltaLists[i].deltas = (SqgDelta *)SqgAlloc(n * sizeof(SqgDelta));
        fread(deltaLists[i].deltas, sizeof(SqgDelta), n, f);
        if (!ReadOk(f)) { SqgError("Can't read deltas for delta list %d"); fclose(f); return FALSE; }
        if (mirror)
            for (int k = 0; k < n; k++)
                deltaLists[i].deltas[k].dx = -deltaLists[i].deltas[k].dx;
    }
    fclose(f);
    return TRUE;
}

// @00472f60
// Builds a Mesh with one group per model group. Face indices in the file are global; D3DRM wants them relative to the
// group's first vertex, so they are shifted down around the AddGroup call and shifted back afterwards.
PZMorphMesh *SqgModel::CreateMesh(IDirect3DRM *d3drm)
{
    IDirect3DRMMesh *mesh;
    d3drm->CreateMesh(&mesh);
    mesh->SetName("PZMorph Mesh");

    int *groupIds = (int *)SqgAlloc(groupCount * 4);
    for (int g = 0; g < groupCount; g++) {
        SqgGroup &grp = groups[g];
        if (grp.vertexCount == 0)
            continue;
        int skip = 0;                               // shared by both loops below, as in the original

        for (int i = 0; i < grp.indexCount; i++) {
            int *p = &indices[grp.indexStart + i];
            if (skip == 0) skip = *p;
            else { skip--; *p -= grp.firstVertex; }
        }

        D3DRMGROUPINDEX id;
        mesh->AddGroup(grp.vertexCount, grp.faceCount, 0, (unsigned *)&indices[grp.indexStart], &id);
        groupIds[g] = (int)id;
        mesh->SetVertices(id, 0, grp.vertexCount, &vertices[grp.firstVertex]);

        for (int i = 0; i < grp.indexCount; i++) {
            int *p = &indices[grp.indexStart + i];
            if (skip == 0) skip = *p;
            else { skip--; *p += grp.firstVertex; }
        }

        mesh->SetGroupColor(id, grp.textureIndex == -1 ? (D3DCOLOR)grp.color : (D3DCOLOR)0xffffffff);
        SqgTextureLookupFn lookup = SqgGetTextureLookup();
        if (grp.textureIndex != -1 && lookup)
            mesh->SetGroupTexture(id, lookup(textureNames[grp.textureIndex]));
    }

    D3DRMVERTEX *copy = (D3DRMVERTEX *)SqgAlloc(vertexCount * sizeof(D3DRMVERTEX));
    memcpy(copy, vertices, vertexCount * sizeof(D3DRMVERTEX));

    PZMorphMesh *inst = (PZMorphMesh *)Operator_New(sizeof(PZMorphMesh));
    inst->groupIds = groupIds;
    inst->vertices = copy;
    inst->mesh = mesh;
    return inst;
}

// @004734e0
// Applies the weighted morph targets to the instance's vertices and uploads them to the Mesh, group by group.
void SqgModel::Morph(PZMorphMesh *inst, int count, const float *weights)
{
    for (int v = 0; v < vertexCount; v++)           // base positions (normals and texture coordinates stay)
        inst->vertices[v].position = vertices[v].position;

    for (int k = 0; k < count; k++) {
        float w = weights[k];
        if (fabsf(w) == 0.0f)
            continue;
        const SqgDeltaList &list = deltaLists[k];
        for (int d = 0; d < list.count; d++) {
            const SqgDelta &delta = list.deltas[d];
            D3DVECTOR &pos = inst->vertices[delta.vertex].position;
            pos.x = pos.x + delta.dx * w;
            pos.y = pos.y + delta.dy * w;
            pos.z = pos.z + delta.dz * w;
        }
    }

    for (int g = 0; g < groupCount; g++) {
        const SqgGroup &grp = groups[g];
        if (grp.vertexCount > 0)
            inst->mesh->SetVertices((D3DRMGROUPINDEX)inst->groupIds[g], 0, grp.vertexCount, &inst->vertices[grp.firstVertex]);
    }
}

// @004731f0
// One MeshBuilder per group, each filled vertex by vertex and face by face, all attached to a new parentless frame.
// Face indices are shifted to group-local around the face loop, as in CreateMesh. (The original never releases the
// builder of a group without vertices; that leak is kept.)
IDirect3DRMFrame *SqgModel::BuildFrame(IDirect3DRM *d3drm, BOOL perspective)
{
    IDirect3DRMFrame *frame;
    d3drm->CreateFrame(NULL, &frame);

    for (int g = 0; g < groupCount; g++) {
        SqgGroup &grp = groups[g];

        IDirect3DRMMeshBuilder *builder;
        d3drm->CreateMeshBuilder(&builder);
        builder->SetName("PZMorph MeshBuilder");
        builder->SetPerspective(perspective);
        builder->ReserveSpace(grp.vertexCount, grp.vertexCount, grp.faceCount);
        if (grp.vertexCount == 0)
            continue;

        int skip = 0;
        for (int i = 0; i < grp.indexCount; i++) {
            int *p = &indices[grp.indexStart + i];
            if (skip == 0) skip = *p;
            else { skip--; *p -= grp.firstVertex; }
        }

        for (int v = 0; v < grp.vertexCount; v++) {
            const D3DRMVERTEX &vert = vertices[grp.firstVertex + v];
            builder->AddVertex(vert.position.x, vert.position.y, vert.position.z);
            builder->AddNormal(vert.normal.x, vert.normal.y, vert.normal.z);
            builder->SetTextureCoordinates(v, vert.tu, vert.tv);
        }

        int pos = 0;
        for (int f = 0; f < grp.faceCount; f++) {
            int n = indices[grp.indexStart + pos];
            pos++;
            IDirect3DRMFace *face;
            builder->CreateFace(&face);
            for (; n > 0; n--) {
                int idx = indices[grp.indexStart + pos];
                pos++;
                face->AddVertexAndNormalIndexed(idx, idx);
            }
            face->SetColor(grp.textureIndex == -1 ? (D3DCOLOR)grp.color : (D3DCOLOR)0xffffffff);
            face->Release();
        }

        for (int i = 0; i < grp.indexCount; i++) {
            int *p = &indices[grp.indexStart + i];
            if (skip == 0) skip = *p;
            else { skip--; *p += grp.firstVertex; }
        }

        SqgTextureLookupFn lookup = SqgGetTextureLookup();
        if (grp.textureIndex != -1 && lookup)
            builder->SetTexture(lookup(textureNames[grp.textureIndex]));

        frame->AddVisual(builder);
        builder->Release();
    }
    return frame;
}
