
#include "headers.h"
#include "xpp.h"
#include "callback.h"

void xpp::preprocessCanopy02(engine::IClump* clump)
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
            geometry->getShader(i)->setFlags(
                geometry->getShader(i)->getFlags() & ~engine::sfCulling
            );
            geometry->getShader(i)->setSpecularColor( Vector4f( 0.99f, 0.93f, 0.51f, 1.0f ) );
            geometry->getShader(i)->setSpecularPower( 10.0f );
        }
    }
}