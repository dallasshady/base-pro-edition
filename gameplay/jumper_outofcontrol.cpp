
#include "headers.h"
#include "jumper.h"
#include "imath.h"

/**
 * related animations
 */

static engine::AnimSequence rollingSequence = 
{ 
    FRAMETIME(2172), 
    FRAMETIME(2190),  
    engine::ltPeriodic,
    FRAMETIME(2172)
};

static engine::AnimSequence swingingSequence = 
{ 
    FRAMETIME(1418), 
    FRAMETIME(1428),  
    engine::ltPeriodic,
    FRAMETIME(1418)
};

/**
 * class implementation
 */

Jumper::OutOfControl::OutOfControl(Jumper* jumper, NxActor* actor, MatrixConversion* mc, float time) :
    JumperAction( jumper )
{
    // set action properties
    _actionTime = 0.0f;
    _blendTime = 0.1f;
    _endOfAction = false;
    _phActor = actor;
    _matrixConversion = mc;
    _time = time;

    engine::IAnimationController* animCtrl = _clump->getAnimationController();

    // capture blend source
    animCtrl->captureBlendSrc();

    // reset animation mixer
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( animCtrl->getTrackAnimation( i ) ) animCtrl->setTrackActivity( i, false );
    }

    animCtrl->setTrackAnimation( 0, &rollingSequence );
    animCtrl->setTrackAnimation( 1, &swingingSequence );
    animCtrl->setTrackActivity( 0, true );
    animCtrl->setTrackActivity( 1, true );
    animCtrl->setTrackSpeed( 0, 0.25f );
    animCtrl->setTrackSpeed( 1, 0.5f );
    animCtrl->setTrackWeight( 0, 1.0f );
    animCtrl->setTrackWeight( 1, 0.0f );
    animCtrl->resetTrackTime( 0 );
    animCtrl->resetTrackTime( 1 );
    animCtrl->advance( 0.0f );

    // capture blend destination
    animCtrl->captureBlendDst();
    animCtrl->blend( 0.0f );
}

/**
 * Action implementation
 */

void Jumper::OutOfControl::update(float dt)
{
    // detect animation velocity
    float minVelocity  = 0.25f;
    float minAnimSpeed = 0.0f;
    float maxVelocity  = 5.0f;
    float maxAnimSpeed = 1.0f;
    float factor = ( _phActor->getLinearVelocity().magnitude() - minVelocity ) / ( maxVelocity - minVelocity );
    factor = factor < 0 ? 0 : ( factor > 1 ? 1 : factor );
    float animSpeed = minAnimSpeed * ( 1 - factor ) + maxAnimSpeed * factor;

    // update animation
    updateAnimation( dt * animSpeed );
    
    // synchronize physics & render
    _clump->getFrame()->setMatrix( _matrixConversion->convert( wrap( _phActor->getGlobalPose() ) ) );
    _clump->getFrame()->getLTM();

    // sequence blending 
    float blendVelocity = 1.0f;
    engine::IAnimationController* animCtrl = _clump->getAnimationController();

    // detect ground under jumper
    float maxDist = 1.0f;
    NxRay worldRay;         
    worldRay.orig = _phActor->getGlobalPosition();
    worldRay.dir  = NxVec3( 0,-maxDist,0 );
    NxShape** terrainShapes = _jumper->getScene()->getPhTerrain()->getShapes();
    NxTriangleMeshShape* triMeshShape = terrainShapes[0]->isTriangleMesh();
    assert( triMeshShape );   
    NxRaycastHit raycastHit;
    if( triMeshShape->raycast( worldRay, maxDist, NX_RAYCAST_DISTANCE | NX_RAYCAST_NORMAL, raycastHit, true ) )
    {
        float weight = animCtrl->getTrackWeight( 0 );
        weight += blendVelocity * dt;
        if( weight > 1 ) weight = 1;
        animCtrl->setTrackWeight( 0, weight );
        animCtrl->setTrackWeight( 1, 1 - weight );
    }
    else
    {
        float weight = animCtrl->getTrackWeight( 0 );
        weight -= blendVelocity * dt;
        if( weight < 0 ) weight = 0;
        animCtrl->setTrackWeight( 0, weight );
        animCtrl->setTrackWeight( 1, 1 - weight );
    }

    // decrease time of out of control
    _time -= dt;

    // finalize action
    if( _time < 0 && _jumper->getVirtues()->evolution.health > 0 ) _endOfAction = true;
}

static float getAirResistancePower(float i)
{
    return ( exp( pow( i, 1.4f ) )  - 1.0f ) / 1.718f;
}

void Jumper::OutOfControl::updatePhysics(void)
{
    // velocity of base jumper's body
    NxVec3 velocity = _phActor->getLinearVelocity();

    // vectical velocity of base jumper's body
    NxVec3 velocityV( 0, velocity.y, 0 );

    // air resistance coefficients
    float AR = _jumper->getVirtues()->getTrackingAirResistance();

    // terminal velocity
    float Vt = sqrt( 9.8f * _phActor->getMass() / AR );
    float It = velocityV.magnitude() / Vt;

    // air resistance force
    NxVec3 Far = NxVec3(0,1,0) * getAirResistancePower( velocityV.magnitude() / Vt ) * _phActor->getMass() * 9.8f;

    // gyration resistance
    float GR = 40.0f;
    float GRV = velocity.magnitude() / 25.0f;
    if( GRV > 1.0f ) GRV = 1.0f;
    GR *= GRV;
    NxVec3 Tr = _phActor->getAngularVelocity() * -GR;

    // finalize motion equation    
    _phActor->addForce( Far );
    _phActor->addTorque( Tr );
}