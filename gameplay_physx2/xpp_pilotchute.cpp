
#include "headers.h"
#include "xpp.h"
#include "callback.h"

void xpp::preprocessPilotchute(engine::IClump* clump)
{
    callback::AtomicL atomicL;    
    clump->forAllAtomics( callback::enumerateAtomics, &atomicL );
    engine::IGeometry* geometry;
    for( callback::AtomicI atomicI = atomicL.begin(); atomicI != atomicL.end(); atomicI++ )
    {
        geometry = (*atomicI)->getGeometry();
        // process shaders
        for( int i=0; i<geometry->getNumShaders(); i++ )
        {
            // mesh shader?
            if( geometry->getShader(i)->getNumLayers() == 1 &&
                strcmp( geometry->getShader(i)->getLayerTexture( 0 )->getName(), "risers01_01" ) == 0 )
            {
                // enablre anisotropy
                geometry->getShader(i)->getLayerTexture( 0 )->setMinFilter( engine::ftAnisotropic );
                geometry->getShader(i)->getLayerTexture( 0 )->setMagFilter( engine::ftLinear );
                geometry->getShader(i)->getLayerTexture( 0 )->setMaxAnisotropy( 8 );
                // enable alpha blending
                geometry->getShader(i)->setFlags(
                    geometry->getShader(i)->getFlags() | engine::sfAlphaBlending 
                );
            }
            // disable culling for all shaders
            geometry->getShader(i)->setFlags(
                geometry->getShader(i)->getFlags() & ~engine::sfCulling 
            );
        }
    }
}