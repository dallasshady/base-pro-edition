
#include "headers.h"
#include "xpp.h"
#include "callback.h"

void xpp::preprocessHelicopter(engine::IClump* clump)
{
    callback::AtomicL atomics;
    clump->forAllAtomics( callback::enumerateAtomics, &atomics );

    // enable alpha blending for corresponding meshes    
    for( callback::AtomicI atomicI = atomics.begin();
                           atomicI != atomics.end();
                           atomicI++ )
    {
        engine::IGeometry* geometry = (*atomicI)->getGeometry();
        unsigned int numShaders = geometry->getNumShaders();
        for( unsigned int i=0; i<numShaders; i++ )
        {
            if( geometry->getShader(i)->getNumLayers() && 
                strcmp( geometry->getShader(i)->getLayerTexture(0)->getName(), "Lopast01" ) == 0 )
            {
                unsigned int flags = geometry->getShader(i)->getFlags();
                flags = flags | engine::sfAlphaBlending;
                flags = flags & ~engine::sfCulling;
                geometry->getShader(i)->setFlags( flags );
            }
        }
    }
}