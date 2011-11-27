
#include "headers.h"
#include "lightmap.h"
#include "asset.h"

engine::IBSPSector* collectSectorLightmapsCB(engine::IBSPSector* sector, void* data)
{
    BSPSector* bspSector = dynamic_cast<BSPSector*>( sector );
    LightmapChunks* lightmaps = reinterpret_cast<LightmapChunks*>( data );

    if( bspSector )
    {
        if( bspSector->lightmap() )
        {
            LightmapChunk lightmapChunk;
            lightmapChunk.atomicId  = 0;
            lightmapChunk.sectorId  = (auid)( bspSector );
            lightmapChunk.textureId = (auid)( bspSector->lightmap() );
            lightmaps->push_back( lightmapChunk );
        }
    }
    return sector;
}

engine::IAtomic* collectAtomicLightmapsCB(engine::IAtomic* atomic, void* data)
{
    Atomic* a = dynamic_cast<Atomic*>( atomic );
    LightmapChunks* lightmaps = reinterpret_cast<LightmapChunks*>( data );

    if( a )
    {
        if( a->lightmap() )
        {
            LightmapChunk lightmapChunk;
            lightmapChunk.atomicId  = (auid)( a );
            lightmapChunk.sectorId  = 0;
            lightmapChunk.textureId = (auid)( a->lightmap() );
            lightmaps->push_back( lightmapChunk );
        }
    }
    
    return atomic;
}

engine::IClump* collectClumpLightmapsCB(engine::IClump* clump, void* data)
{
    clump->forAllAtomics( collectAtomicLightmapsCB, data );
    return clump;
}

void collectLightmaps(BSP* bsp, LightmapChunks* lightmaps)
{
    bsp->forAllSectors( collectSectorLightmapsCB, lightmaps );
    // bsp->forAllClumps( collectClumpLightmapsCB, lightmaps );
}

void collectLightmaps(Clump* clump, LightmapChunks* lightmaps)
{
    collectClumpLightmapsCB( clump, lightmaps );
}

void writeLightmaps(LightmapChunks* lightmaps, IResource* resource)
{
    for( unsigned int i=0; i<lightmaps->size(); i++ )
    {
        // write chunk header
        ChunkHeader binaryHeader( BA_BINARY, sizeof( LightmapChunk ) );
        binaryHeader.write( resource );

        // write lightmap chunk
        LightmapChunk lightmapChunk = (*lightmaps)[i];
        fwrite( &lightmapChunk, sizeof(LightmapChunk), 1, resource->getFile() );
    }
}