
#include "headers.h"
#include "mission.h"
#include "imath.h"
#include "database.h"
#include "npc.h"
#include "npccameraman.h"
#include "smokeball.h"
#include "goal.h"
#include "interrupt.h"
#include "forest.h"
#include "version.h"

/**
 * Mission::ThirdPersonCamera
 */

#define TPSM_INITIAL_DISTANCE 500.0f
#define TPSM_MINIMAL_DISTANCE 150.0f

Mission::ThirdPersonCamera::ThirdPersonCamera(Scene* scene, Actor* target) :
    Actor( scene )
{
    _name                = "ThirdPersonCamera";
    _positionMode        = false;
    _positionModeTimeout = 0.0f;
    _target              = target;
    _cameraTilt          = 0;
    _cameraTurn          = 0;
    _cameraFOV           = 60* CAMERA_FOV_MULTIPLIER;
    _cameraDistance      = TPSM_INITIAL_DISTANCE;
    _cameraMatrix        = Matrix4f( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );    
}

Mission::ThirdPersonCamera::~ThirdPersonCamera()
{
}

void Mission::ThirdPersonCamera::onUpdateActivity(float dt)
{
    assert( _scene );

    _positionModeTimeout -= dt;
    _positionModeTimeout = _positionModeTimeout < 0 ? 0 : _positionModeTimeout;

    // retrieve action channels
    ActionChannel* headLeft  = Gameplay::iGameplay->getActionChannel( iaHeadLeft );
    ActionChannel* headRight = Gameplay::iGameplay->getActionChannel( iaHeadRight );
    ActionChannel* headUp    = Gameplay::iGameplay->getActionChannel( iaHeadUp );
    ActionChannel* headDown  = Gameplay::iGameplay->getActionChannel( iaHeadDown );
    ActionChannel* zoomIn    = Gameplay::iGameplay->getActionChannel( iaZoomIn );
    ActionChannel* zoomOut   = Gameplay::iGameplay->getActionChannel( iaZoomOut );    
    ActionChannel* changeTarget   = Gameplay::iGameplay->getActionChannel( iaWLO );  

	if (false && changeTarget->getTrigger()) {
        if( dynamic_cast<Jumper*>( _target ) )
        {
            Jumper* j = dynamic_cast<Jumper*>( _target );

			if (j->getAirplane()) {
				_target = j->getAirplane();
			} else if (j->getParent()) {
				_target = j->getParent();
			}
		}
	}

    if( _positionMode )
    {
        // field of view
        _cameraFOV = ( 60.0f - 55.0f * ( zoomIn->getAmplitude() ) ) * CAMERA_FOV_MULTIPLIER;

        // target position
        Matrix4f targetPose( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );
        if( _target ) targetPose = _target->getPose();    
        Vector3f targetPos( targetPose[3][0], targetPose[3][1], targetPose[3][2] );

        if( dynamic_cast<Jumper*>( _target ) )
        {
            Jumper* j = dynamic_cast<Jumper*>( _target );
            j->getClump()->getFrame()->getLTM();
            engine::IFrame* backBone = Jumper::getBackBone( j->getClump() );
            targetPos = backBone->getPos();
        }

        // camera direction
        Vector3f cameraAt = _cameraPos - targetPos;
        cameraAt.normalize();        

        // camera right
        Vector3f cameraRight; 
        cameraRight.cross( Vector3f(0,1,0), cameraAt );
        cameraRight.normalize();

        // camera up
        Vector3f cameraUp;
        cameraUp.cross( cameraAt, cameraRight );
        cameraUp.normalize();

        // camera matrix
        _cameraMatrix.set( 
            cameraRight[0], cameraRight[1], cameraRight[2], 0.0f,
            cameraUp[0], cameraUp[1], cameraUp[2], 0.0f,
            cameraAt[0], cameraAt[1], cameraAt[2], 0.0f,
            _cameraPos[0], _cameraPos[1], _cameraPos[2], 1.0f
        );
    }
    else
    {
        // camera offset 
        _cameraDistance -= dt * 500.0f * zoomIn->getAmplitude();
        _cameraDistance += dt * 500.0f * zoomOut->getAmplitude();
        if( _cameraDistance < TPSM_MINIMAL_DISTANCE ) _cameraDistance = TPSM_MINIMAL_DISTANCE;

        // rotate camera    
        _cameraTurn += 180 * dt * headLeft->getAmplitude();
        _cameraTurn -= 180 * dt * headRight->getAmplitude();
        _cameraTilt += 180 * dt * headUp->getAmplitude();
        _cameraTilt -= 180 * dt * headDown->getAmplitude();
        if( _cameraTilt < -89 ) _cameraTilt = -89;
        if( _cameraTilt > 89 ) _cameraTilt = 89;

        Matrix4f targetPose( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );
        if( _target ) targetPose = _target->getPose();
    
        Vector3f targetPos( targetPose[3][0], targetPose[3][1], targetPose[3][2] );
        Vector3f targetOffset( 0,150,0 );
    
        if( dynamic_cast<Jumper*>( _target ) )
        {
            Jumper* j = dynamic_cast<Jumper*>( _target );
            j->getClump()->getFrame()->getLTM();
            engine::IFrame* backBone = Jumper::getBackBone( j->getClump() );
            targetPos = backBone->getPos();
            targetOffset.set( 0,75, 0 );
        }

        // calculate camera matrix        
        _cameraMatrix.set( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );
        _cameraMatrix = Gameplay::iEngine->rotateMatrix( _cameraMatrix, Vector3f( 1,0,0 ), _cameraTilt );
        _cameraMatrix = Gameplay::iEngine->rotateMatrix( _cameraMatrix, Vector3f( 0,1,0 ), _cameraTurn );
        Vector3f at( _cameraMatrix[2][0], _cameraMatrix[2][1], _cameraMatrix[2][2] );

        // possible camera position
        Vector3f cameraPos = targetPos + targetOffset + at * _cameraDistance;

        // clip camera distance
        float clipDistance = _cameraDistance;
        if( _scene->clipCameraRay( targetPos, cameraPos, clipDistance ) )
        {
            _cameraDistance = clipDistance;
            cameraPos = targetPos + targetOffset + at * _cameraDistance;
        }        
        
        // finalize camera matrix (translation component)
        _cameraMatrix = Gameplay::iEngine->translateMatrix( _cameraMatrix, cameraPos );
    }

    // camera is actual now
    Gameplay::iEngine->getDefaultCamera()->setFOV( _cameraFOV );
    Gameplay::iEngine->getDefaultCamera()->getFrame()->setMatrix( _cameraMatrix );
    _scene->getScenery()->happen( this, EVENT_CAMERA_IS_ACTUAL );
    if( _scene->getTopMode() ) _scene->getTopMode()->happen( this, EVENT_CAMERA_IS_ACTUAL );

    // RT-RS pass
    bool flares = ( _scene->getLocation()->getWeather() == ::wtSunny ) || ( _scene->getLocation()->getWeather() == ::wtVariable );
    Gameplay::iGameplay->getRenderTarget()->render( _scene, _cameraMatrix, _cameraFOV, flares, false );
    // GUI
    Gameplay::iEngine->getDefaultCamera()->beginScene( 0, Vector4f( 0,0,0,0 ) );    
    if( _scene->isHUDEnabled() ) Gameplay::iGui->render();
    Gameplay::iEngine->getDefaultCamera()->endScene();
    // present result
    Gameplay::iEngine->present();
}

void Mission::ThirdPersonCamera::switchMode(void)
{
    if( _positionModeTimeout > 0 ) return;

    _positionMode = !_positionMode;
    if( _positionMode )
    {
        _cameraPos[0] = _cameraMatrix[3][0];
        _cameraPos[1] = _cameraMatrix[3][1];
        _cameraPos[2] = _cameraMatrix[3][2];
        _cameraFOV    = 60 * CAMERA_FOV_MULTIPLIER;
    }
    else
    {
        _cameraTilt     = 0;
        _cameraTurn     = 0;
        _cameraFOV      = 60 * CAMERA_FOV_MULTIPLIER;
        _cameraDistance = TPSM_INITIAL_DISTANCE;
    }
    _positionModeTimeout = 0.125f;
}

void Mission::ThirdPersonCamera::resetSwitchTimer(void)
{
    _positionModeTimeout = 0.125f;
}

/**
 * Mission::FirstPersonCamera
 */

Mission::FirstPersonCamera::FirstPersonCamera(Scene* scene, engine::IFrame* headFrame) :
    Actor( scene )
{
    _name           = "FirstPersonCamera";
    _headFrame      = headFrame;
    _cameraTilt     = 0;
    _cameraTurn     = 0;
    _cameraFOV      = 60 * CAMERA_FOV_MULTIPLIER;
    _cameraMatrix   = Matrix4f( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );
}

Mission::FirstPersonCamera::~FirstPersonCamera()
{
}


void Mission::FirstPersonCamera::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
}

void Mission::FirstPersonCamera::onUpdateActivity(float dt)
{
    assert( _scene );

    // retrieve action channels
    ActionChannel* headLeft  = Gameplay::iGameplay->getActionChannel( iaHeadLeft );
    ActionChannel* headRight = Gameplay::iGameplay->getActionChannel( iaHeadRight );
    ActionChannel* headUp    = Gameplay::iGameplay->getActionChannel( iaHeadUp );
    ActionChannel* headDown  = Gameplay::iGameplay->getActionChannel( iaHeadDown );

    // rotate camera    
    _cameraTurn += 180 * dt * headLeft->getAmplitude();
    _cameraTurn -= 180 * dt * headRight->getAmplitude();
    _cameraTilt += 180 * dt * headUp->getAmplitude();
    _cameraTilt -= 180 * dt * headDown->getAmplitude();
    if( _cameraTilt < -89 ) _cameraTilt = -89;
    if( _cameraTilt > 89 ) _cameraTilt = 89;

    // put constraint on turn
    if( _cameraTurn < -120 ) _cameraTurn = -120;
    if( _cameraTurn > 120 ) _cameraTurn = 120;

    // put constraint on tilt
    float maxTurn = 120.0f;
    float maxTilt = -20;
    float minTurn = 0;
    float minTilt = -75;
    float factor = ( fabs( _cameraTurn ) - minTurn ) / ( maxTurn - minTurn );
    factor = factor < 0 ? 0 : ( factor > 1 ? 1 : factor );
    float tiltLimit = minTilt * ( 1 - factor ) + maxTilt * factor;
    if( _cameraTilt < tiltLimit ) _cameraTilt = tiltLimit;    

    // calculate camera matrix
    Matrix4f ltm = _headFrame->getLTM();
    ::orthoNormalize( ltm );
    _cameraMatrix.set( -1,0,0,0,
                       0,1,0,0,
                       0,0,-1,0,
                       0,0,0,1 );
    _cameraMatrix = Gameplay::iEngine->rotateMatrix( _cameraMatrix, Vector3f( 1,0,0 ), -_cameraTilt );
    _cameraMatrix = Gameplay::iEngine->rotateMatrix( _cameraMatrix, Vector3f( 0,1,0 ), _cameraTurn );
    _cameraMatrix = Gameplay::iEngine->transformMatrix( _cameraMatrix, ltm );

    // camera is actual now
    Gameplay::iEngine->getDefaultCamera()->setFOV( _cameraFOV );
    Gameplay::iEngine->getDefaultCamera()->getFrame()->setMatrix( _cameraMatrix );
    _scene->getScenery()->happen( this, EVENT_CAMERA_IS_ACTUAL );
    if( _scene->getTopMode() ) _scene->getTopMode()->happen( this, EVENT_CAMERA_IS_ACTUAL );

    // RT-RS pass
    Gameplay::iGameplay->getRenderTarget()->render( _scene, _cameraMatrix, _cameraFOV, false, false );
    // GUI
    Gameplay::iEngine->getDefaultCamera()->beginScene( 0, Vector4f( 0,0,0,0 ) );
    if( _scene->isHUDEnabled() ) Gameplay::iGui->render();
    Gameplay::iEngine->getDefaultCamera()->endScene();
    // present result
    Gameplay::iEngine->present();
}

/**
 * Mission : class implementation
 */

Mission::Mission(Scene* scene, database::MissionInfo* missionInfo, unsigned int wttid, unsigned int wtmid) : Mode(scene)
{
    _missionInfo         = missionInfo;
    _wttid               = wttid;
    _wtmid               = wtmid;
    _endOfMission        = false;
    _endOfPlayerActivity = false;
    scene->setTimeSpeed( 1.0f );
    scene->setTimeSpeedMultiplier( 1.0f );
    _fadTimeout = 0.0f;
    _fadValue = _fadTargetValue = 1.0f;
    _phTimeLeft = 0.0f;
    _interruptTimeout = 0.0f;

    // location info
    database::LocationInfo* locationInfo = database::LocationInfo::getRecord( _scene->getLocation()->getDatabaseId() );

    // exit point info
    database::LocationInfo::ExitPoint* exitPoint = NULL;
    if( missionInfo->exitPointId != AIRPLANE_EXIT )
    {
        exitPoint = locationInfo->exitPoints + missionInfo->exitPointId;
    }
   
    // enclosure
    Enclosure* enclosure = NULL;
    if( exitPoint )
    {    
        _scene->getExitPointEnclosure( missionInfo->exitPointId );
    }

    // reset mission actors
    _player = NULL;
    _tpcamera = NULL;
    _fpcamera = NULL;
    _fwcamera = NULL;
    _frcamera = NULL;

    // mission specific casting 
    if( missionInfo->castingCallback )
    {
        missionInfo->castingCallback( this );
    }

    // cannot continiue properly with no player
    assert( _player );

    // statistics
	if( missionInfo->exitPointId == AIRPLANE_EXIT || _player->getAirplane() != NULL )
    {
        _player->getVirtues()->statistics.numSkydives++;
    }
    else
    {
        _player->getVirtues()->statistics.numBaseJumps++;
    }

    // create camera actors
    _tpcamera = new ThirdPersonCamera( scene, _player );
    _fpcamera = new FirstPersonCamera( scene, Jumper::getFirstPersonFrame( _player->getClump() ) );
    _fwcamera = new FlywayCamera( scene );
    _frcamera = new FreeCamera( scene );

    // prompt window
    _promptWindow = Gameplay::iGui->createWindow( "Prompt" ); assert( _promptWindow );
}

Mission::~Mission()
{
    if( _tpcamera ) delete _tpcamera;
    if( _fpcamera ) delete _fpcamera;
    if( _fwcamera ) delete _fwcamera;
    if( _frcamera ) delete _frcamera;
    _promptWindow->getPanel()->release();

    // play menu music
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_execution.ogg" );
}

/**
 * Mission : abstraction layer
 */

void Mission::onUpdateActivity(float dt)
{
    _fadTimeout -= dt;
    _interruptTimeout -= dt;

    // important action: ESC - interrupt mission
    if( Gameplay::iGameplay->getKeyboardState()->keyState[0x01] & 0x80 )
    {
        if( _endOfPlayerActivity )
        {
            _scene->getCareer()->setMissionWalkthroughFlag( _wttid, _wtmid, true );
            _endOfMission = true;
        }
        else if( _interruptTimeout < 0 )
        {
            assert( _scene->getCamera() );
            _scene->getCamera()->updateActivity( 0.0f );
            new Interrupt( _scene, _scene->getCamera()->getPose(), &_endOfMission, ( _player->getPhase() != jpRoaming ) );
            _interruptTimeout = 0.25f;
            return;
        }
    }

    // soft time acceleration
    if( _fadValue != _fadTargetValue )
    {
        float fadSpeed =     1.0f;
        float direction = _fadTargetValue - _fadValue;
        if( direction < 0 )
        {
            _fadValue -= fadSpeed * dt;
            if( _fadValue < _fadTargetValue ) _fadValue = _fadTargetValue;
        }
        else
        {
            _fadValue += fadSpeed * dt;
            if( _fadValue > _fadTargetValue ) _fadValue = _fadTargetValue;
        }
        _scene->setTimeSpeedMultiplier( _fadValue );
    }

    // action: first person mode
    if( Gameplay::iGameplay->getActionChannel( iaCameraMode0 )->getTrigger() )
    {
        setFirstPersonCamera();
    }
    // action: third person mode
    if( Gameplay::iGameplay->getActionChannel( iaCameraMode1 )->getTrigger() )
    {
        setThirdPersonCamera();
    }
    // action : flyway mode
    if( Gameplay::iGameplay->getActionChannel( iaCameraMode2 )->getTrigger() )
    {
        _player->happen( this, EVENT_JUMPER_THIRDPERSON, _player );
        _scene->setCamera( _fwcamera );
        _fwcamera->setupCamera( _player, 2.0f, 0.75f );
    }
    // action : free camera
    if( Gameplay::iGameplay->getActionChannel( iaCameraMode3 )->getTrigger() )
    {
        _player->happen( this, EVENT_JUMPER_THIRDPERSON, _player );
        _scene->setCamera( _frcamera );
    }    
    // action: time acceleration
    if( Gameplay::iGameplay->getActionChannel( ::iaAccelerateFlightTime )->getTrigger() && 
        _fadTimeout < 0 && ( _player->getPhase() == ::jpFlight || _player->getPhase() == ::jpRoaming ) )
    {
        _fadTargetValue *= 2.0f;
        if( _fadTargetValue > 4.0f ) _fadTargetValue = 4.0f;
        _fadTimeout = 0.25f;
    }
    // action: time deceleration
    if( Gameplay::iGameplay->getActionChannel( ::iaDecelerateFlightTime )->getTrigger() && 
        _fadTimeout < 0 && ( _player->getPhase() == ::jpFlight || _player->getPhase() == ::jpRoaming ) )
    {
        _fadTargetValue *= 0.5f;
        if( _fadTargetValue < 1.0f ) _fadTargetValue = 1.0f;
        _fadTimeout = 0.25f;
    }

    // reset time deceleration/acceleration
    if( _player->getPhase() == ::jpFreeFalling )
    {
        _fadTargetValue = 1.0f;
    }
    
    // collect forest actors
    unsigned int forestId;
    std::vector<Forest*> forests;
    getScene()->getScenery()->happen( this, EVENT_FOREST_ENUMERATE, &forests );

    // update physics
    _phTimeLeft += dt;
    while( _phTimeLeft > simulationStepTime )
    {
        // begin to simulate physics
        _scene->getPhScene()->simulate( simulationStepTime );
        _scene->getPhScene()->flushStream();
        _scene->getPhScene()->fetchResults( NX_RIGID_BODY_FINISHED, true );
        updatePhysics();
        _phTimeLeft -= simulationStepTime;

        // forest interaction
        for( forestId=0; forestId<forests.size(); forestId++ )
        {
            forests[forestId]->simulateInteraction( _player );
        }
    }
    _scene->getPhScene()->visualize();

    // player is over its activity?
    if( !_endOfPlayerActivity && _player->isOverActivity() )
    {
        // raise local switch
        _endOfPlayerActivity = true;  

        // hide HUD
        happen( this, EVENT_HUD_HIDE );
    }

    // important action: TAB - show goals
    if( ( Gameplay::iGameplay->getKeyboardState()->keyState[0x0F] & 0x80 ) || _endOfPlayerActivity )
    {
        if( !goalsIsVisible() )
        {
            showGoals( true );
        }
    }
    else
    {
        if( goalsIsVisible() )
        {
            showGoals( false );
        }
    }
}

void Mission::showGoals(bool flag)
{
    if( flag && !goalsIsVisible() )
    {
        // show prompt
        Gameplay::iGui->getDesktop()->insertPanel( _promptWindow->getPanel() );
        _promptWindow->align( gui::atTop, 0, gui::atRight, 0 );
        // enumerate goals
        ActorV goals;
        happen( this, EVENT_GOAL_ENUMERATE, &goals );
        // show & arrange goals
        Goal* goal;
        int x = 8;
        int y = _promptWindow->getPanel()->getRect().bottom + 8;
        gui::Rect desktopRect = Gameplay::iGui->getDesktop()->getRect();
        gui::Rect rect;
        for( unsigned int i=0; i<goals.size(); i++ )
        {
            goal = dynamic_cast<Goal*>( goals[i] ); assert( goal );
            Gameplay::iGui->getDesktop()->insertPanel( goal->getWindow()->getPanel() );
            // place goal
            rect.left   = x;
            rect.right  = x + goal->getWindow()->getPanel()->getRect().getWidth();
            rect.top    = y;
            rect.bottom = y + goal->getWindow()->getPanel()->getRect().getHeight();
            // check rect is not overlaps the desktop
            if( rect.bottom > desktopRect.bottom )
            {
                x = rect.right + 8;
                y = _promptWindow->getPanel()->getRect().bottom + 8;
                rect.left   = x;
                rect.right  = x + goal->getWindow()->getPanel()->getRect().getWidth();
                rect.top    = y;
                rect.bottom = y + goal->getWindow()->getPanel()->getRect().getHeight();
            }
            goal->getWindow()->getPanel()->setRect( rect );
            y = rect.bottom + 8;
        }
    }
    else if( !flag && goalsIsVisible() )
    {
        // hide prompt
        Gameplay::iGui->getDesktop()->removePanel( _promptWindow->getPanel() );
        // enumerate goals
        ActorV goals;
        happen( this, EVENT_GOAL_ENUMERATE, &goals );
        // hide goals
        Goal* goal;        
        for( unsigned int i=0; i<goals.size(); i++ )
        {
            goal = dynamic_cast<Goal*>( goals[i] ); assert( goal );
            Gameplay::iGui->getDesktop()->removePanel( goal->getWindow()->getPanel() );
        }
    }
}

bool Mission::goalsIsVisible(void)
{
    return ( _promptWindow->getPanel()->getParent() != NULL );
}

void Mission::onSuspend(void)
{
    Gameplay::iGui->setCursorVisible( true );
    Gameplay::iGui->getDesktop()->setColor( Vector4f( 1,1,1,1 ) );

    if( _scene->getCamera() )
    {
        if( _scene->getCamera() == _tpcamera ||
            _scene->getCamera() == _fpcamera ||
            _scene->getCamera() == _fwcamera ||
            _scene->getCamera() == _frcamera )
        {
            _scene->setCamera( NULL );
        }
    }
}

void Mission::onResume(void)
{
    Gameplay::iGui->setCursorVisible( false );
    Gameplay::iGui->getDesktop()->setColor( Vector4f( 0,0,0,0 ) );
    if( _tpcamera ) _scene->setCamera( _tpcamera );
}

bool Mission::endOfMode(void)
{
    return _endOfMission;
}

/**
 * complex behaviour
 */

void Mission::setFirstPersonCamera(void)
{
    _player->happen( this, EVENT_JUMPER_FIRSTPERSON, _player );
    _scene->setCamera( _fpcamera );
}

void Mission::setThirdPersonCamera(void)
{
    _player->happen( this, EVENT_JUMPER_THIRDPERSON, _player );

    Actor* currentCamera = _scene->getCamera();
    ThirdPersonCamera* currentTPSCamera = dynamic_cast<ThirdPersonCamera*>( currentCamera );
    if( currentTPSCamera )
    {
        currentTPSCamera->switchMode();
    }
    else
    {
        _scene->setCamera( _tpcamera );
        _tpcamera->resetSwitchTimer();
    }
}

bool Mission::cameraIsFirstPerson(void)
{
    return ( _fpcamera == _scene->getCamera() );
}

bool Mission::cameraIsThirdPerson(void)
{
    return ( _tpcamera == _scene->getCamera() );
}