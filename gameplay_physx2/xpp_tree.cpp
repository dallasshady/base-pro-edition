
#include "headers.h"
#include "xpp.h"
#include "callback.h"

void xpp::preprocessTree(engine::IClump* clump)
{
    callback::AtomicL atomicL;    
    clump->forAllAtomics( callback::enumerateAtomics, &atomicL );
    engine::IGeometry* geometry;
    for( callback::AtomicI atomicI = atomicL.begin(); atomicI != atomicL.end(); atomicI++ )
    {
        geometry = (*atomicI)->getGeometry();
        // process shaders
        engine::IShader* shader;
        shader = geometry->getShader( 0 );
        shader->setFlags( shader->getFlags() | engine::sfAlphaBlending | engine::sfAlphaTesting );
        shader->setFlags( shader->getFlags() & ~engine::sfLighting );
        shader->setFlags( shader->getFlags() & ~engine::sfCulling );
        shader->setAlphaTestFunction( engine::cfGreater );
        shader->setAlphaTestRef( 16 );
        // for all textures
        for( int j=0; j<geometry->getShader(0)->getNumLayers(); j++ )
        {
            shader->getLayerTexture(j)->setMinFilter( engine::ftAnisotropic );
            shader->getLayerTexture(j)->setMagFilter( engine::ftLinear );
            shader->getLayerTexture(j)->setMipFilter( engine::ftLinear );
            shader->getLayerTexture(j)->setMaxAnisotropy( 8 );
            shader->getLayerTexture(j)->setMipmapLODBias( -2 );
        }
    }
}