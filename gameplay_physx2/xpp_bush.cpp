
#include "headers.h"
#include "xpp.h"
#include "callback.h"

void xpp::preprocessBush(engine::IClump* clump)
{
    callback::AtomicL atomicL;    
    clump->forAllAtomics( callback::enumerateAtomics, &atomicL );
    engine::IGeometry* geometry;
    for( callback::AtomicI atomicI = atomicL.begin(); atomicI != atomicL.end(); atomicI++ )
    {
        geometry = (*atomicI)->getGeometry();
        // process shaders
        engine::IShader* shader;
        int numShaders = geometry->getNumShaders();
        int i,j;
        for( i=0; i<numShaders; i++ )
        {
            shader = geometry->getShader( i );
            shader->setFlags( shader->getFlags() | engine::sfAlphaBlending | engine::sfAlphaTesting );
            shader->setFlags( shader->getFlags() & ~engine::sfLighting );
            shader->setFlags( shader->getFlags() & ~engine::sfCulling );
            shader->setAlphaTestFunction( engine::cfGreater );
            shader->setAlphaTestRef( 16 );
            // for all textures
            for( j=0; j<geometry->getShader(i)->getNumLayers(); j++ )
            {
                shader->getLayerTexture(j)->setMinFilter( engine::ftAnisotropic );
                shader->getLayerTexture(j)->setMagFilter( engine::ftLinear );
                shader->getLayerTexture(j)->setMipFilter( engine::ftLinear );
                shader->getLayerTexture(j)->setMaxAnisotropy( 8 );
            }
        }
    }
}