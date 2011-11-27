
#include "headers.h"
#include "xpp.h"
#include "callback.h"

void xpp::preprocessSlider(engine::IClump* clump)
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
            geometry->getShader(i)->setFlags( geometry->getShader(i)->getFlags() | engine::sfAlphaBlending | engine::sfAlphaTesting );
            geometry->getShader(i)->setFlags( geometry->getShader(i)->getFlags() & ~engine::sfCulling );
            geometry->getShader(i)->setAlphaTestFunction( engine::cfGreater );
            geometry->getShader(i)->setAlphaTestRef( 0 );
        }
    }
}