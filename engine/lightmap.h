/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description internal auxiliary methods for lightmap usage
 *
 * @author bad3p
 */

#ifndef LIGHTMAP_AUX_INCLUDED
#define LIGHTMAP_AUX_INCLUDED

#include "headers.h"
#include "engine.h"
#include "atomic.h"
#include "light.h"
#include "clump.h"
#include "shader.h"
#include "bsp.h"

struct LightmapChunk
{
public:
    auid atomicId;
    auid sectorId;
    auid textureId;
};

typedef std::vector<LightmapChunk> LightmapChunks;

void collectLightmaps(BSP* bsp, LightmapChunks* lightmaps);
void collectLightmaps(Clump* clump, LightmapChunks* lightmaps);

void writeLightmaps(LightmapChunks* lightmaps, IResource* resource);

#endif