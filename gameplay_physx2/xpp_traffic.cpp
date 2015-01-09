
#include "headers.h"
#include "xpp.h"
#include "callback.h"

static engine::IAtomic* preprocessTrafficAtomicCB(engine::IAtomic* atomic, void* data)
{
    int          i,j;
    unsigned int flags;

    for( i=0; i<atomic->getGeometry()->getNumShaders(); i++ )
    {
        for( j=0; j<atomic->getGeometry()->getShader(i)->getNumLayers(); j++ )
        {
            char* textureName = strdup( atomic->getGeometry()->getShader(i)->getLayerTexture(j)->getName() );
            assert( textureName );
            strlwr( textureName );
            if( strstr( textureName, "shadow" ) )
            {
                flags = atomic->getGeometry()->getShader(i)->getFlags();
                flags = flags | engine::sfAlphaBlending | engine::sfAlphaTesting;
                flags = flags & ~engine::sfCulling;
                atomic->getGeometry()->getShader(i)->setFlags( flags );
                atomic->getGeometry()->getShader(i)->setAlphaTestFunction( engine::cfGreater );
                atomic->getGeometry()->getShader(i)->setAlphaTestRef( 0 );
            }
            free( textureName );
        }        
    }
    return atomic;
}

void xpp::preprocessTraffic(engine::IClump* clump)
{
    clump->forAllAtomics( preprocessTrafficAtomicCB, NULL );
}