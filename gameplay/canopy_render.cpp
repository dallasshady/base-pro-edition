
#include "headers.h"
#include "canopy.h"
#include "imath.h"
#include "gear.h"
#include "database.h"

engine::IAtomic* CanopyRenderCallback::onRenderCanopy(engine::IAtomic* atomic, void* data)
{
    assert( reinterpret_cast<CanopyRenderCallback*>(data)->canopyTexture );

    // for all shaders - apply face texture    
    for( int i=0; i<atomic->getGeometry()->getNumShaders(); i++ )
    {
        assert( atomic->getGeometry()->getShader(i)->getNumLayers() == 1 );
        atomic->getGeometry()->getShader(i)->setLayerTexture( 
            0, 
            reinterpret_cast<CanopyRenderCallback*>(data)->canopyTexture
        );
    }
    return atomic;
}

void CanopyRenderCallback::restoreAtomic(engine::IAtomic* atomic)
{
    engine::ITexture* texture = Gameplay::iEngine->getTexture( "white" );
    assert( texture );
    for( int i=0; i<atomic->getGeometry()->getNumShaders(); i++ )
    {        
        atomic->getGeometry()->getShader(i)->setLayerTexture( 0, texture );        
    }
}

CanopyRenderCallback::~CanopyRenderCallback()
{
    if( canopyTexture ) canopyTexture->release();
}

void CanopyRenderCallback::setTexture(const char* textureName, const char* resourceName)
{
    if( canopyTexture )
    {
        canopyTexture->release();
        canopyTexture = NULL;
    }
    canopyTexture = Gameplay::iEngine->getTexture( textureName );
    if( canopyTexture == NULL )
    {
        canopyTexture = Gameplay::iEngine->createTexture( resourceName );
        assert( canopyTexture );
        canopyTexture->setMinFilter( engine::ftAnisotropic );
        canopyTexture->setMagFilter( engine::ftLinear );
        canopyTexture->setMipFilter( engine::ftLinear );
    }
    // +1 to mark textures as used by this object
    canopyTexture->addReference();
}

void CanopyRenderCallback::setTexture(database::Canopy* gearRecord)
{
    database::GearTexture* textureInfo = database::GearTexture::getRecord( gearRecord->textureId );
    setTexture( textureInfo->textureName, textureInfo->resourceName );
}

void CanopyRenderCallback::setTexture(Gear* gear)
{
    setTexture( database::Canopy::getRecord( gear->id ) );
}

void CanopyRenderCallback::apply(engine::IClump* clump)
{
    callback::AtomicL atomics;
    clump->forAllAtomics( callback::enumerateAtomics, &atomics );
    for( callback::AtomicI atomicI = atomics.begin(); atomicI != atomics.end(); atomicI++ )
    {
        if( CanopySimulator::getCollisionGeometry( clump ) != (*atomicI) )
        {
            (*atomicI)->setRenderCallback( onRenderCanopy, this );
        }
    }
}

void CanopyRenderCallback::restore(engine::IClump* clump)
{
    callback::AtomicL atomics;
    clump->forAllAtomics( callback::enumerateAtomics, &atomics );
    for( callback::AtomicI atomicI = atomics.begin(); atomicI != atomics.end(); atomicI++ )
    {
        if( CanopySimulator::getCollisionGeometry( clump ) != (*atomicI) )
        {
            restoreAtomic( (*atomicI) );
        }
    }
}