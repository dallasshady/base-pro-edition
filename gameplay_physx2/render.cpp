
#include "render.h"
#include "gameplay.h"

/**
 * SimpleRT implementation
 */

SimpleRT::SimpleRT()
{
}
    
SimpleRT::~SimpleRT()
{
}

void SimpleRT::render(RenderSource* source, const Matrix4f& view, float fov, bool flares, bool wideScreen)
{
    // setup cameras   
    Gameplay::iEngine->getDefaultCamera()->getFrame()->setMatrix( view );
    Gameplay::iEngine->getDefaultCamera()->setFOV( fov );

    // pass source
    for( unsigned int i=0; i<source->getNumPasses(); i++ )
    {
        Gameplay::iEngine->getDefaultCamera()->setNearClipPlane( 
            source->getPassNearClip( i ) 
        );
        Gameplay::iEngine->getDefaultCamera()->setFarClipPlane( 
            source->getPassFarClip( i ) 
        );
        Gameplay::iEngine->getDefaultCamera()->beginScene(
            source->getPassClearFlag( i ),
            source->getClearColor()
        );
        source->renderPass( i );
        Gameplay::iEngine->getDefaultCamera()->endScene();
    }

    // lens flares
    if( flares ) 
    {
        Gameplay::iEngine->getDefaultCamera()->beginScene( 0, Vector4f( 0,0,0,1 ) );
        source->renderLensFlares();
        Gameplay::iEngine->getDefaultCamera()->endScene();
    }

    // widescreen
    if( wideScreen )
    {
        float aspectRatio  = 1.85f;
        float screenWidth  = Gameplay::iEngine->getScreenSize()[0];
        float screenHeight = Gameplay::iEngine->getScreenSize()[1];
        float aspectedHeight = screenWidth / aspectRatio;
        if( aspectedHeight > screenHeight ) aspectedHeight = screenHeight;
        float tapeHeight = 0.5f * ( screenHeight - aspectedHeight );
        engine::ITexture* blacktexture = Gameplay::iEngine->getTexture( "black" ); assert( blacktexture );
        Gameplay::iEngine->getDefaultCamera()->beginScene( 0, Vector4f( 0,0,0,1 ) );
        Gameplay::iEngine->renderRect2d( Vector2f( 0, 0 ), Vector2f( screenWidth, tapeHeight ), Vector4f( 1,1,1,1 ), blacktexture );
        Gameplay::iEngine->renderRect2d( Vector2f( 0, screenHeight - tapeHeight ), Vector2f( screenWidth, tapeHeight ), Vector4f( 1,1,1,1 ), blacktexture );
        Gameplay::iEngine->getDefaultCamera()->endScene();
    }
}

/**
 * AfterFxRT implemetation
 */

AfterFxRT::AfterFxRT()
{
    // create motion blur rendeing resources
    Vector3f screenSize = Gameplay::iEngine->getScreenSize();
    _motionBlurRT = Gameplay::iEngine->createRenderTarget( 
        unsigned int( screenSize[0] ),
        unsigned int( screenSize[1] ),
        unsigned int( screenSize[2] ),
        "motionBlurRT"
    );
    _motionBlurRT->setMagFilter( engine::ftLinear );
    _motionBlurRT->setMinFilter( engine::ftLinear );
    _motionBlurRT->setMipFilter( engine::ftLinear );
    _motionBlurCamera = Gameplay::iEngine->createCameraEffect( _motionBlurRT );
    engine::IFrame* frame = Gameplay::iEngine->createFrame( "motionBlurCameraFrame" );
    _motionBlurCamera->getCamera()->setFrame( frame );
    _motionBlurCamera->setPfx( engine::pfxMotionBlur );
    _motionBlurCamera->setWeight( 0.0f );

    // create bloom rendeing resources
    _bloomRT = Gameplay::iEngine->createRenderTarget( 
        unsigned int( screenSize[0]/4 ),
        unsigned int( screenSize[1]/4 ),
        32,
        "bloomRT"
    );
    _bloomRT->setMagFilter( engine::ftLinear );
    _bloomRT->setMinFilter( engine::ftLinear );
    _bloomRT->setMipFilter( engine::ftLinear );
	_bloomCamera = Gameplay::iEngine->createCameraEffect( _bloomRT );
	frame = Gameplay::iEngine->createFrame( "bloomCameraFrame" );
	_bloomCamera->getCamera()->setFrame( frame );
	_bloomCamera->setPfx( engine::pfxBloom );
}

AfterFxRT::~AfterFxRT()
{
    // remove rendering resources
    _motionBlurCamera->getCamera()->release();
	_bloomCamera->getCamera()->release();
}

void AfterFxRT::render(RenderSource* source, const Matrix4f& view, float fov, bool flares, bool wideScreen)
{
    // setup cameras   
    _motionBlurCamera->getCamera()->getFrame()->setMatrix( view );
    _motionBlurCamera->getCamera()->setFOV( fov );
    Gameplay::iEngine->getDefaultCamera()->getFrame()->setMatrix( view );
    Gameplay::iEngine->getDefaultCamera()->setFOV( fov );

    // pass source
    for( unsigned int i=0; i<source->getNumPasses(); i++ )
    {
        _motionBlurCamera->getCamera()->setNearClipPlane( 
            source->getPassNearClip( i ) 
        );
        _motionBlurCamera->getCamera()->setFarClipPlane( 
            source->getPassFarClip( i ) 
        );
        _motionBlurCamera->getCamera()->beginScene(
            source->getPassClearFlag( i ),
            source->getClearColor()
        );
        source->renderPass( i );
        _motionBlurCamera->getCamera()->endScene();
    }

    // apply motion blur
    _motionBlurCamera->setWeight( source->getBlur() );
    _motionBlurCamera->applyEffect();

    // render blurred image on to bloom render target
    _bloomCamera->getCamera()->getFrame()->setMatrix( view );
    _bloomCamera->getCamera()->setFOV( fov );
    _bloomCamera->getCamera()->setNearClipPlane( 1.0f );
    _bloomCamera->getCamera()->setFarClipPlane( 100.0f );
    _bloomCamera->getCamera()->beginScene(
		engine::cmClearColor | engine::cmClearDepth,
        source->getClearColor()
    );
    _bloomCamera->getCamera()->renderTexture( _motionBlurRT );
    _bloomCamera->getCamera()->endScene();

    // apply bloom
    _bloomCamera->setWeight( source->getBrightPass() );
    _bloomCamera->setVector( Vector4f( source->getBloom(),0,0,0 ) );
    _bloomCamera->applyEffect();

    // render image on to screen surface, add lens flares effect
    Gameplay::iEngine->getDefaultCamera()->beginScene( 
        engine::cmClearColor | engine::cmClearDepth,
        source->getClearColor()
    );
    Gameplay::iEngine->getDefaultCamera()->renderTexture( _motionBlurRT );
    Gameplay::iEngine->getDefaultCamera()->renderTextureAdditive( _bloomRT );
    if( flares ) source->renderLensFlares();
    Gameplay::iEngine->getDefaultCamera()->endScene();

    // widescreen
    if( wideScreen )
    {
        float aspectRatio  = 1.85f;
        float screenWidth  = Gameplay::iEngine->getScreenSize()[0];
        float screenHeight = Gameplay::iEngine->getScreenSize()[1];
        float aspectedHeight = screenWidth / aspectRatio;
        if( aspectedHeight > screenHeight ) aspectedHeight = screenHeight;
        float tapeHeight = 0.5f * ( screenHeight - aspectedHeight );
        engine::ITexture* blacktexture = Gameplay::iEngine->getTexture( "black" ); assert( blacktexture );
        Gameplay::iEngine->getDefaultCamera()->beginScene( 0, Vector4f( 0,0,0,1 ) );
        Gameplay::iEngine->renderRect2d( Vector2f( 0, 0 ), Vector2f( screenWidth, tapeHeight ), Vector4f( 1,1,1,1 ), blacktexture );
        Gameplay::iEngine->renderRect2d( Vector2f( 0, screenHeight - tapeHeight ), Vector2f( screenWidth, tapeHeight ), Vector4f( 1,1,1,1 ), blacktexture );
        Gameplay::iEngine->getDefaultCamera()->endScene();
    }
}

/**
 * StereoRT : class implemetation
 */

StereoRT::StereoRT()
{
    // create motion blur rendeing resources
    Vector3f screenSize = Gameplay::iEngine->getScreenSize();
    _stereoRT = Gameplay::iEngine->createRenderTarget( 
        unsigned int( screenSize[0] ),
        unsigned int( screenSize[1] ),
        unsigned int( screenSize[2] ),
        "motionBlurRT"
    );
    _stereoRT->setMagFilter( engine::ftLinear );
    _stereoRT->setMinFilter( engine::ftLinear );
    _stereoRT->setMipFilter( engine::ftLinear );
    _stereoCamera = Gameplay::iEngine->createCameraEffect( _stereoRT );
    engine::IFrame* frame = Gameplay::iEngine->createFrame( "stereoCameraFrame" );
    _stereoCamera->getCamera()->setFrame( frame );
    _stereoCamera->setPfx( engine::pfxMotionBlur );
    _stereoCamera->setWeight( 0.0f );
}

StereoRT::~StereoRT()
{
    _stereoCamera->getCamera()->release();
}

void StereoRT::render(RenderSource* source, const Matrix4f& view, float fov, bool flares, bool wideScreen)
{
    // first pass, left eye
    Matrix4f identity( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );
    Matrix4f leftEyeM = Gameplay::iEngine->rotateMatrix( identity, Vector3f(0,1,0), -0.125f );
    Matrix4f rightEyeM = Gameplay::iEngine->rotateMatrix( identity, Vector3f(0,1,0), 0.125f );
    Matrix4f leftEyeLTM = Gameplay::iEngine->transformMatrix( leftEyeM, view );
    Matrix4f rightEyeLTM = Gameplay::iEngine->transformMatrix( rightEyeM, view );

    // first pass, left eye
    _stereoCamera->getCamera()->getFrame()->setMatrix( leftEyeLTM );
    _stereoCamera->getCamera()->setFOV( fov );
 
    // pass source
    for( unsigned int i=0; i<source->getNumPasses(); i++ )
    {
        _stereoCamera->getCamera()->setNearClipPlane( 
            source->getPassNearClip( i ) 
        );
        _stereoCamera->getCamera()->setFarClipPlane( 
            source->getPassFarClip( i ) 
        );
        _stereoCamera->getCamera()->beginScene(
            source->getPassClearFlag( i ),
            source->getClearColor()
        );
        source->renderPass( i );
        _stereoCamera->getCamera()->endScene();
    }

    // apply first pass blending
    _stereoCamera->setWeight( 0.0f );
    _stereoCamera->applyEffect();

    // second pass, right eye
    _stereoCamera->getCamera()->getFrame()->setMatrix( rightEyeLTM );
    _stereoCamera->getCamera()->setFOV( fov );

    // pass source
    for( unsigned int i=0; i<source->getNumPasses(); i++ )
    {
        _stereoCamera->getCamera()->setNearClipPlane( 
            source->getPassNearClip( i ) 
        );
        _stereoCamera->getCamera()->setFarClipPlane( 
            source->getPassFarClip( i ) 
        );
        _stereoCamera->getCamera()->beginScene(
            source->getPassClearFlag( i ),
            source->getClearColor()
        );
        source->renderPass( i );
        _stereoCamera->getCamera()->endScene();
    }

    // apply second pass blending
    _stereoCamera->setWeight( 0.5f );
    _stereoCamera->applyEffect();

    // render image on to screen surface, add lens flares effect
    Gameplay::iEngine->getDefaultCamera()->beginScene( 
        engine::cmClearColor | engine::cmClearDepth,
        source->getClearColor()
    );
    Gameplay::iEngine->getDefaultCamera()->renderTexture( _stereoRT );
    if( flares ) source->renderLensFlares();
    Gameplay::iEngine->getDefaultCamera()->endScene();
}