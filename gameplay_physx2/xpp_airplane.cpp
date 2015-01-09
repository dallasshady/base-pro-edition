
#include "headers.h"
#include "xpp.h"
#include "callback.h"

void xpp::preprocessAirplane(engine::IClump* clump)
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

        if( strstr( (*atomicI)->getFrame()->getName(), "alpha" ) )
        {            
            for( unsigned int i=0; i<numShaders; i++ )
            {
                unsigned int flags = geometry->getShader(i)->getFlags();
                flags = flags | engine::sfAlphaBlending;
                flags = flags & ~engine::sfCulling;
                geometry->getShader(i)->setFlags( flags );
            }
        }
        else
        {
            for( unsigned int i=0; i<numShaders; i++ )
            {
                unsigned int flags = geometry->getShader(i)->getFlags();
                flags = flags & ~engine::sfCulling;
                geometry->getShader(i)->setFlags( flags );
            }
        }
    }
}