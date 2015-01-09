
#include "headers.h"
#include "mission.h"
#include "imath.h"
#include "database.h"
#include "version.h"

void Mission::FlywayCamera::onUpdateActivity(float dt)
{
    if( _target )
    {
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

        // target velocity
        Vector3f targetVel = _target->getVel();

        // move camera
        Vector3f moveN = ( targetPos + targetVel * _timeReserve ) - _cameraPos;
        moveN.normalize();
        _cameraPos += moveN * targetVel.length() * _lag * dt;

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
    if( _scene->isHUDEnabled() )  Gameplay::iGui->render();
    Gameplay::iEngine->getDefaultCamera()->endScene();
    // present result
    Gameplay::iEngine->present();
}

/**
 * class implementation
 */

Mission::FlywayCamera::FlywayCamera(Scene* scene) : Actor( scene )
{
    _name        = "FlywayCamera";
    _target      = NULL;
    _timeReserve = 0.0f;
    _lag         = 0.0f;
    _cameraFOV   = 60.0f * CAMERA_FOV_MULTIPLIER;
    _cameraPos.set( 0,0,0 );
    _cameraMatrix.set( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );
}

Mission::FlywayCamera::~FlywayCamera()
{
}

/**
 * class behaviour
 */

void Mission::FlywayCamera::setupCamera(Actor* target, float timeReserve, float lag)
{
    assert( target );
    _target = target;
    _timeReserve = timeReserve;
    _lag = lag;

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

    // target velocity
    Vector3f targetVel = _target->getVel();

    // setup camera
    _cameraPos = targetPos + targetVel * _timeReserve;
}