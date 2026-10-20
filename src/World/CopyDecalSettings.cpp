// SQZ.EXE 0x425970: copy the decal's fields, including all three vectors.
// Its vector copies were inline in this source unit.
#define WORLD_DECAL_INLINE_VECTOR_COPY
#include "WorldDecal.h"

WorldDecalSettings::WorldDecalSettings(const WorldDecalSettings &other)
    : colors(other.colors), size(other.size), origin(other.origin),
      useTransparency(other.useTransparency), transparentColor(other.transparentColor)
{
}
