
#include "headers.h"
#include "xpp.h"
#include "callback.h"

void xpp::preprocessOstankinoCutscene(engine::IClump* clump)
{
    callback::AtomicL atomicL;
    clump->forAllAtomics( callback::enumerateAtomics, &atomicL );    
    engine::IGeometry* geometry;
    engine::IShader* shader;
    int i;
    for( callback::AtomicI atomicI = atomicL.begin(); atomicI != atomicL.end(); atomicI++ )
    {
        geometry = (*atomicI)->getGeometry();
        // blurred blades
        if( strcmp( (*atomicI)->getFrame()->getName(), "blur01" ) == 0 || 
            strcmp( (*atomicI)->getFrame()->getName(), "blur02" ) == 0 )
        {
            for( i=0; i<geometry->getNumShaders(); i++ )
            {
                shader = geometry->getShader( i );
                shader->setFlags( geometry->getShader(i)->getFlags() | engine::sfAlphaBlending );
                shader->setFlags( geometry->getShader(i)->getFlags() & ~engine::sfCulling );
            }
        }
    }
}