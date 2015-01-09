
#include "headers.h"
#include "xpp.h"
#include "callback.h"
#include "jumper.h"

void xpp::preprocessBaseJumper01(engine::IClump* clump)
{
    // preprocessor rules:
    //  - applies no chaching on all shaders
    //  - applies no mipfilter on all texture
    //  - applies no culling "Parachute_risers03_Parashute_risers_polyShape_shader_0"

    callback::AtomicL atomicL;    
    clump->forAllAtomics( callback::enumerateAtomics, &atomicL );
    engine::IGeometry* geometry;
    engine::IShader* shader;
    int numShaders;
    int i,j;
    for( callback::AtomicI atomicI = atomicL.begin(); atomicI != atomicL.end(); atomicI++ )
    {
        geometry = (*atomicI)->getGeometry();
        // process shaders        
        numShaders = geometry->getNumShaders();        
        for( i=0; i<numShaders; i++ )
        {
            shader = geometry->getShader( i );
            shader->setFlags( shader->getFlags() & ~engine::sfCaching );
            // for all textures
            for( j=0; j<geometry->getShader(i)->getNumLayers(); j++ )
            {
                shader->getLayerTexture(j)->setMinFilter( engine::ftAnisotropic );
                shader->getLayerTexture(j)->setMagFilter( engine::ftLinear );
                shader->getLayerTexture(j)->setMipFilter( engine::ftNone );
                shader->getLayerTexture(j)->setMaxAnisotropy( 8 );
            }
            // risers shader?
            if( strcmp( geometry->getShader(i)->getName(), "Parachute_risers03_Parashute_risers_polyShape_shader_0" ) == 0 )
            {
                unsigned int flags = geometry->getShader(i)->getFlags();
                flags = flags & ~engine::sfCulling;
                geometry->getShader(i)->setFlags( flags );
            }
        }
    }

    // preprocess risers atomic
    engine::IAtomic* risersAtomic = Jumper::getRisers( clump );
    assert( risersAtomic );
    geometry = risersAtomic->getGeometry();
    numShaders = geometry->getNumShaders();
    for( i=0; i<numShaders; i++ )
    {
        shader = geometry->getShader( i );
        shader->setFlags( shader->getFlags() & ~engine::sfCulling );
        shader->setFlags( shader->getFlags() | engine::sfAlphaBlending );
        shader->setFlags( shader->getFlags() | engine::sfAlphaTesting );
        shader->setFlags( shader->getFlags() | engine::sfCaching );
        shader->setAlphaTestFunction( engine::cfGreater );
        shader->setAlphaTestRef( 0 );
    }

    // preprocess wingsuit (no cull)
    engine::IAtomic* bodyAtomic = Jumper::getBody( clump, 1 );
    assert( bodyAtomic );
    if( bodyAtomic )
    {
        geometry = bodyAtomic->getGeometry();
        numShaders = geometry->getNumShaders();
        for( i=0; i<numShaders; i++ )
        {
            shader = geometry->getShader( i );
            shader->setFlags( shader->getFlags() & ~engine::sfCulling );
        }
    }

    // preprocess Tensor helmet
    engine::IAtomic* helmetAtomic = Jumper::getHelmet( clump, 1 );    
    assert( helmetAtomic );
    if( helmetAtomic )
    {
        geometry = helmetAtomic->getGeometry();
        numShaders = geometry->getNumShaders();
        assert( numShaders > 1 );
        if( numShaders > 1 )
        {
            shader = geometry->getShader( 1 );
            shader->setFlags( shader->getFlags() & ~engine::sfCulling | engine::sfAlphaBlending );
        }
    }
}