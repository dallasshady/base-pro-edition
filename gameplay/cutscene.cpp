
#include "headers.h"
#include "cutscene.h"
#include "imath.h"
#include "database.h"
#include "missionbrowser.h"
#include "version.h"

/**
 * aggregated Cutscene::Camera
 */

void Cutscene::Camera::onUpdateActivity(float dt)
{
    // update title
    _titleTimeout -= dt;
    if( _titleTimeout < 0 ) _titleTimeout = 0;

    // RT-RS pass
    bool flares = ( _scene->getLocation()->getWeather() == ::wtSunny ) || ( _scene->getLocation()->getWeather() == ::wtVariable );
    Gameplay::iGameplay->getRenderTarget()->render( _scene, _cameraMatrix, _cameraFOV, flares, false );

    // render title
    if( _titleTimeout > 0 )
    {
        Vector3f screenSize = Gameplay::iEngine->getScreenSize();
        float alpha = _titleTimeout / _titleFadeTime;
        if( alpha > 1 ) alpha = 1;
        if( alpha < 0 ) alpha = 0;
        gui::Rect rect0( 0, 0, int(screenSize[0]), int(screenSize[1]) );
        gui::Rect rect1( 1, 1, 1+int(screenSize[0]), 1+int(screenSize[1]) );
        Gameplay::iEngine->getDefaultCamera()->beginScene( 0, Vector4f(0,0,0,0) );
        Gameplay::iGui->renderUnicodeText(
            rect1, "title", Vector4f(0,0,0,alpha), gui::atCenter, gui::atCenter, true,
            _titleText.c_str()
        );
        Gameplay::iGui->renderUnicodeText(
            rect0, "title", Vector4f(1,1,1,alpha), gui::atCenter, gui::atCenter, true,
            _titleText.c_str()
        );
        Gameplay::iEngine->getDefaultCamera()->endScene();
    }

    // present result
    Gameplay::iEngine->present();
}

Cutscene::Camera::Camera(Scene* scene, float fov) : Actor( scene )
{
    _name         = "CutsceneCamera";
    _cameraFOV    = fov;
    _cameraMatrix = Matrix4f( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );
}

Cutscene::Camera::~Camera()
{
}

void Cutscene::Camera::setMatrix(const Matrix4f& matrix)
{
    _cameraMatrix = matrix;
}

void Cutscene::Camera::setTitle(const wchar_t* text, float timeout, float fadeTime)
{
    _titleText     = text;
    _titleTimeout  = timeout;
    _titleFadeTime = fadeTime;
}

/**
 * Cutscene
 */

Cutscene::Cutscene(Scene* scene, CutsceneDesc* desc) : Mode( scene )
{
    assert( scene );
    _name = "CutsceneMode";
    _desc = *desc;
    _clump = NULL;
    _camera = new Camera( scene, 60.0f * CAMERA_FOV_MULTIPLIER );

    // enumerate animation tracks
    for( CutsceneTrackI trackI = _desc.tracks.begin();
                        trackI != _desc.tracks.end();
                        trackI++ )
    {
        engine::AnimSequence animSequence;
        animSequence.startTime = trackI->begin;        
        animSequence.endTime = trackI->end;
        animSequence.loopStartTime = 0.0f;
        animSequence.loopType = engine::ltNone;
        _animSequenceL.push_back( animSequence );
    }
    
    // setup sequence playing
    if( _animSequenceL.size() == 0 )
    {
        _animSequenceI = NULL;
    }
    else
    {
        _animSequenceI = _animSequenceL.begin();
    }

    // set title
    database::LocationInfo* locationInfo = database::LocationInfo::getRecord( scene->getLocation()->getDatabaseId() );
    const wchar_t* locationName = Gameplay::iLanguage->getUnicodeString(locationInfo->nameId);
    _camera->setTitle( locationName, 15, 5 );
}

Cutscene::~Cutscene()
{    
    delete _camera;
}

/**
 * actor abstracts
 */

void Cutscene::onUpdateActivity(float dt)
{
    assert( _clump );
    
    // update animation
    _clump->getAnimationController()->advance( dt );
    // if there is end of animation sequence
    if( _clump->getAnimationController()->isEndOfAnimation( 0 ) )
    {
        // look for another sequence
        if( _animSequenceI != NULL &&
            _animSequenceI != _animSequenceL.end() )
        {
            _animSequenceI++;
            if( _animSequenceI == _animSequenceL.end() )
            {
                _animSequenceI = _animSequenceL.begin();
            }
            _clump->getAnimationController()->setTrackAnimation( 0, &(*_animSequenceI) );
            _clump->getAnimationController()->setTrackSpeed( 0, _desc.speed );
            _clump->getAnimationController()->resetTrackTime( 0 );
        }
    }

    // search for camera tracker
    engine::IFrame* frame = Gameplay::iEngine->findFrame( _clump->getFrame(), "Camera" ); assert( frame );
    _clump->getFrame()->getLTM();
    Matrix4f cameraLTM = frame->getLTM();
    orthoNormalize( cameraLTM );

    // setup camera pose
    _camera->setMatrix( cameraLTM );

    // wait for any key
    bool anykey = false;
    input::KeyboardState* keyboardState = Gameplay::iGameplay->getKeyboardState();
    for( unsigned int i=0; i<255; i++ )
    {
        if( keyboardState->keyState[i] & 0x80 )
        {
            anykey = true;
            break;
        }
    }

    if( anykey )
    {
        new MissionBrowser( _scene, _camera->getPose(), 60 * CAMERA_FOV_MULTIPLIER );
    }
}

/**
 * mode abstracts
 */

void Cutscene::onSuspend(void)
{
    assert( _clump != NULL );
    
    _scene->setCamera( NULL );
    _scene->getStage()->remove( _clump );
    _clump->release();
    _clump = NULL;
    if( _animSequenceI != NULL ) _animSequenceI = _animSequenceL.begin();
}

void Cutscene::onResume(void)
{
    assert( _clump == NULL );

    // setup camera
    _scene->setCamera( _camera );
    
    // create clone of source
    _clump = _desc.source->clone( "CutsceneClump" ); assert( _clump );

    // setup animation
    if( _animSequenceI != NULL )
    {
        _clump->getAnimationController()->setTrackAnimation( 0, &(*_animSequenceI) );
        _clump->getAnimationController()->setTrackSpeed( 0, _desc.speed );
        _clump->getAnimationController()->resetTrackTime( 0 );
    }
        
    // add clump to scene
    _scene->getStage()->add( _clump );
}

bool Cutscene::endOfMode(void)
{
    return false;
}