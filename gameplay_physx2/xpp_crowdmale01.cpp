
#include "headers.h"
#include "xpp.h"
#include "callback.h"

void xpp::preprocessCrowdMale01(engine::IClump* clump)
{
    // preprocessor rules:
    //  - hides atomic attached to "character01_head01_04_Walk_dummy"
    //  - hides atomic attached to "character01_head01_04_dummy00"
    //  - for all shaders, disables caching 
    //  - for "shadowPlane" atomic - enable subtractive blending

    callback::AtomicL atomicL;
    clump->forAllAtomics( callback::enumerateAtomics, &atomicL );
    for( callback::AtomicI atomicI = atomicL.begin(); atomicI != atomicL.end(); atomicI++ )
    {
        if( 0 == strcmp( (*atomicI)->getFrame()->getName(), "character01_head01_04_Walk_dummy" ) ||
            0 == strcmp( (*atomicI)->getFrame()->getName(), "character01_head01_04_dummy00" ) )
        {
            (*atomicI)->setFlags( 0 );
        }

        // process shaders
        engine::IShader* shader;
        int numShaders = (*atomicI)->getGeometry()->getNumShaders();
        for( int i=0; i<numShaders; i++ )
        {
            shader = (*atomicI)->getGeometry()->getShader( i );
            shader->setFlags( shader->getFlags() & ~engine::sfCaching );
        }

        // shadow plane?
        if( strcmp( (*atomicI)->getFrame()->getName(), "shadowPlane" ) == 0 )
        {
            for( int i=0; i<numShaders; i++ )
            {
                shader = (*atomicI)->getGeometry()->getShader( i );
                shader->setFlags( shader->getFlags() & ~engine::sfCaching );
                shader->setFlags( shader->getFlags() | engine::sfAlphaBlending );
                shader->setBlendOp( engine::bpRevSubtract );
                shader->setSrcBlend( engine::bmOne );
                shader->setDestBlend( engine::bmOne );
            }
        }
    }
}