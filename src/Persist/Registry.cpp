// BufferObject registry (0x448ce0 / 0x448d20): a lazily allocated array of 60 prototype pointers.
#include <string.h>
#include "Persist/Stream.h"

int g_bufferObjectCount;
BufferObject **g_bufferObjects;

void RegisterBufferObject(BufferObject *proto)
{
    if (!g_bufferObjects)
        g_bufferObjects = (BufferObject **)operator new(240);
    g_bufferObjects[g_bufferObjectCount] = proto;
    g_bufferObjectCount++;
}

BufferObject *FindBufferObject(const char *name)
{
    for (int i = 0; i < g_bufferObjectCount; i++)
        if (strcmp(g_bufferObjects[i]->TypeName(), name) == 0)
            return g_bufferObjects[i];
    return 0;
}
