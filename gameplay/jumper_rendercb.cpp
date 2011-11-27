
#include "headers.h"
#include "jumper.h"
#include "imath.h"
#include "gear.h"
#include "database.h"

/**
 * jumper render callbacks
 */

engine::IAtomic* JumperRenderCallback::onRenderFace(engine::IAtomic* atomic, void* data)
{
    assert( reinterpret_cast<JumperRenderCallback*>(data)->faceTexture );

    // for all shaders - apply face texture    
    for( int i=0; i<atomic->getGeometry()->getNumShaders(); i++ )
    {
        assert( atomic->getGeometry()->getShader(i)->getNumLayers() == 1 );
        atomic->getGeometry()->getShader(i)->setLayerTexture( 
            0, 
            reinterpret_cast<JumperRenderCallback*>(data)->faceTexture
        );
    }
    return atomic;
}

engine::IAtomic* JumperRenderCallback::onRenderHand(engine::IAtomic* atomic, void* data)
{
    assert( reinterpret_cast<JumperRenderCallback*>(data)->faceTexture );

    // for all shaders - apply face texture
    for( int i=0; i<atomic->getGeometry()->getNumShaders(); i++ )
    {
        assert( atomic->getGeometry()->getShader(i)->getNumLayers() == 1 );
        atomic->getGeometry()->getShader(i)->setLayerTexture( 
            0, 
            reinterpret_cast<JumperRenderCallback*>(data)->faceTexture
        );
    }
    return atomic;
}

engine::IAtomic* JumperRenderCallback::onRenderHelmet(engine::IAtomic* atomic, void* data)
{
    assert( reinterpret_cast<JumperRenderCallback*>(data)->helmetTexture );

    // for all shaders - apply helmet texture
    for( int i=0; i<atomic->getGeometry()->getNumShaders(); i++ )
    {
        assert( atomic->getGeometry()->getShader(i)->getNumLayers() == 1 );
        atomic->getGeometry()->getShader(i)->setLayerTexture( 
            0, 
            reinterpret_cast<JumperRenderCallback*>(data)->helmetTexture
        );
    }
    return atomic;
}

engine::IAtomic* JumperRenderCallback::onRenderBody(engine::IAtomic* atomic, void* data)
{
    assert( reinterpret_cast<JumperRenderCallback*>(data)->suitTexture );
    assert( reinterpret_cast<JumperRenderCallback*>(data)->rigTexture );

    // for all shaders - apply suit texture
    for( int i=0; i<atomic->getGeometry()->getNumShaders(); i++ )
    {
        assert( atomic->getGeometry()->getShader(i)->getNumLayers() == 1 );
        if( i == 0 || i == 2 )
        {
            atomic->getGeometry()->getShader(i)->setLayerTexture( 
                0, 
                reinterpret_cast<JumperRenderCallback*>(data)->suitTexture
            );
        }
        else if( i == 1 )
        {
            atomic->getGeometry()->getShader(i)->setLayerTexture( 
                0, 
                reinterpret_cast<JumperRenderCallback*>(data)->rigTexture
            );
        }
    }
    return atomic;
}

void JumperRenderCallback::apply(engine::IClump* clump)
{
    for( unsigned int i=0; i<Jumper::getNumHeads( clump ); i++ )
    {
        Jumper::getHead( clump, i )->setRenderCallback( onRenderFace, this );
    }
    for( i=0; i<Jumper::getNumBodies( clump ); i++ )
    {
        Jumper::getBody( clump, i )->setRenderCallback( onRenderBody, this );
    }
    for( i=0; i<Jumper::getNumHelmets( clump ); i++ )
    {
        Jumper::getHelmet( clump, i )->setRenderCallback( onRenderHelmet, this );
    }
    Jumper::getLeftEye( clump )->setRenderCallback( onRenderFace, this );
    Jumper::getRightEye( clump )->setRenderCallback( onRenderFace, this );
    Jumper::getLeftHand( clump )->setRenderCallback( onRenderHand, this );
    Jumper::getRightHand( clump )->setRenderCallback( onRenderHand, this );
}

void JumperRenderCallback::setFace(unsigned int faceId)
{
    if( faceTexture )
    {
        faceTexture->release();
        faceTexture = NULL;
    }
    database::Face* face = database::Face::getRecord( faceId );
    faceTexture = Gameplay::iEngine->getTexture( face->textureName );
    if( faceTexture == NULL )
    {
        faceTexture = Gameplay::iEngine->createTexture( face->resourceName );
        assert( faceTexture );
        faceTexture->setMinFilter( engine::ftAnisotropic );
        faceTexture->setMagFilter( engine::ftLinear );
        faceTexture->setMipFilter( engine::ftLinear );
    }
    // +1 to mark textures as used by this object
    faceTexture->addReference();
}

void JumperRenderCallback::setFace(const char* textureName, const char* resourceName)
{
    if( faceTexture )
    {
        faceTexture->release();
        faceTexture = NULL;
    }
    faceTexture = Gameplay::iEngine->getTexture( textureName );
    if( faceTexture == NULL )
    {
        faceTexture = Gameplay::iEngine->createTexture( resourceName );
        assert( faceTexture );
        faceTexture->setMinFilter( engine::ftAnisotropic );
        faceTexture->setMagFilter( engine::ftLinear );
        faceTexture->setMipFilter( engine::ftLinear );
    }
    // +1 to mark textures as used by this object
    faceTexture->addReference();
}

void JumperRenderCallback::setHelmet(unsigned int helmetId)
{
    if( helmetTexture )
    {
        helmetTexture->release();
        helmetTexture = NULL;
    }
    database::Helmet* helmet = database::Helmet::getRecord( helmetId );
    database::GearTexture* gearTexture = database::GearTexture::getRecord( helmet->textureId );
    helmetTexture = Gameplay::iEngine->getTexture( gearTexture->textureName );
    if( helmetTexture == NULL )
    {
        helmetTexture = Gameplay::iEngine->createTexture( gearTexture->resourceName );
        assert( helmetTexture );
        helmetTexture->setMinFilter( engine::ftAnisotropic );
        helmetTexture->setMagFilter( engine::ftLinear );
        helmetTexture->setMipFilter( engine::ftLinear );
    }
    // +1 to mark textures as used by this object
    helmetTexture->addReference();
}

void JumperRenderCallback::setSuit(unsigned int suitId)
{
    if( suitTexture )
    {
        suitTexture->release();
        suitTexture = NULL;
    }
    database::Suit* suit = database::Suit::getRecord( suitId );
    database::GearTexture* gearTexture = database::GearTexture::getRecord( suit->textureId );
    suitTexture = Gameplay::iEngine->getTexture( gearTexture->textureName );
    if( suitTexture == NULL )
    {
        suitTexture = Gameplay::iEngine->createTexture( gearTexture->resourceName );
        assert( suitTexture );
        suitTexture->setMinFilter( engine::ftAnisotropic );
        suitTexture->setMagFilter( engine::ftLinear );
        suitTexture->setMipFilter( engine::ftLinear );
    }
    // +1 to mark textures as used by this object
    suitTexture->addReference();
}

void JumperRenderCallback::setRig(unsigned int rigId)
{
    if( rigTexture )
    {
        rigTexture->release();
        rigTexture = NULL;
    }
    database::Rig* rig = database::Rig::getRecord( rigId );
    database::GearTexture* gearTexture = database::GearTexture::getRecord( rig->textureId );
    rigTexture = Gameplay::iEngine->getTexture( gearTexture->textureName );
    if( rigTexture == NULL )
    {
        rigTexture = Gameplay::iEngine->createTexture( gearTexture->resourceName );
        assert( rigTexture );
        rigTexture->setMinFilter( engine::ftAnisotropic );
        rigTexture->setMagFilter( engine::ftLinear );
        rigTexture->setMipFilter( engine::ftLinear );
    }
    // +1 to mark textures as used by this object
    rigTexture->addReference();
}

JumperRenderCallback::~JumperRenderCallback()
{
    if( faceTexture ) faceTexture->release();
    if( helmetTexture ) helmetTexture->release();
    if( suitTexture ) suitTexture->release();
    if( rigTexture ) rigTexture->release();
}

void JumperRenderCallback::restoreAtomic(engine::IAtomic* atomic)
{
    engine::ITexture* texture = Gameplay::iEngine->getTexture( "white" );
    assert( texture );
    for( int i=0; i<atomic->getGeometry()->getNumShaders(); i++ )
    {        
        atomic->getGeometry()->getShader(i)->setLayerTexture( 0, texture );        
    }
}

void JumperRenderCallback::restore(engine::IClump* clump)
{
    engine::ITexture* texture = Gameplay::iEngine->getTexture( "white" );
    assert( texture );
    texture->addReference();
    
    restoreAtomic( Jumper::getLeftHand( clump ) );
    restoreAtomic( Jumper::getRightHand( clump ) );
    for( unsigned int i=0; i<Jumper::getNumHelmets( clump ); i++ )
    {
        restoreAtomic( Jumper::getHelmet( clump, i ) );
    }
    for( i=0; i<Jumper::getNumBodies( clump ); i++ )
    {
        restoreAtomic( Jumper::getBody( clump, i ) );
    }
}

/**
 * riser render callback
 */

engine::IAtomic* RiserRenderCallback::onRenderRiser(engine::IAtomic* atomic, void* data)
{
    if( reinterpret_cast<RiserRenderCallback*>(data)->showSlider )
    {
        // turn on slider rendering
        assert( atomic->getGeometry()->getNumShaders() == 2 );
        engine::IShader* sliderShader = atomic->getGeometry()->getShader( 1 );
        sliderShader->setAlphaTestFunction( engine::cfGreater );
        sliderShader->setAlphaTestRef( 0 );
    }
    else
    {
        // turn off slider rendering
        assert( atomic->getGeometry()->getNumShaders() == 2 );
        engine::IShader* sliderShader = atomic->getGeometry()->getShader( 1 );
        sliderShader->setAlphaTestFunction( engine::cfNever );
    }
    return atomic;
}

void RiserRenderCallback::apply(engine::IClump* clump)
{
    Jumper::getRisers( clump )->setRenderCallback( onRenderRiser, this ); 
}