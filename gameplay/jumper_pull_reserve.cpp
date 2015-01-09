
#include "headers.h"
#include "jumper.h"
#include "imath.h"

/**
 * process constants
 */

const float trackBlendTime = 0.4f;

/**
 * related animations
 */

static engine::AnimSequence pullAndDropPilotchuteSequence = 
{
    FRAMETIME(1002),
    FRAMETIME(1033),
    engine::ltNone, 
    0.0f
};

static engine::AnimSequence putHandsOverHeadSequence = 
{
    FRAMETIME(1219),
    FRAMETIME(1229),
    engine::ltPeriodic, 
    FRAMETIME(1219)
};

/**
 * class implementation
 */

Jumper::PullReserve::PullReserve(Jumper* jumper, PxRigidDynamic* phActor, MatrixConversion* mc,  PilotchuteSimulator* pc, PxVec3 localAnchor) :
    Tracking( jumper, phActor, mc )
{
    // set action properties
    _actionTime = 0.0f;
    _blendTime = 0.2f;
    _endOfAction = false;
    _pilotchute = pc;
    _wing_area = 0.0f;
    _tracking = 0.0f;
    _leg_pitch = 0.0f;
	_carving_forward = _carving_sideways = 0.0f;

    // connect pilot chute
    if( !_pilotchute->isPulled() )
    {
        _pilotchute->connect( _phActor, Jumper::getLineRigJoint( _clump ), localAnchor );
    }

    engine::IAnimationController* animCtrl = _clump->getAnimationController();

    // capture blend source
    animCtrl->captureBlendSrc();

    // reset animation mixer
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( animCtrl->getTrackAnimation( i ) ) animCtrl->setTrackActivity( i, false );
    }

    // setup animation cycles
    animCtrl->setTrackAnimation( 0, &pullAndDropPilotchuteSequence );
    animCtrl->setTrackAnimation( 1, &putHandsOverHeadSequence );
    animCtrl->setTrackActivity( 0, true );
    animCtrl->setTrackActivity( 1, true );
    animCtrl->setTrackSpeed( 0, 1.0f );
    animCtrl->setTrackSpeed( 1, 1.0f );
    animCtrl->setTrackWeight( 0, 1.0f );
    animCtrl->setTrackWeight( 1, 0.0f );
    animCtrl->resetTrackTime( 0 );
    animCtrl->resetTrackTime( 1 );
    animCtrl->advance( 0.0f );

    // capture blend destination
    animCtrl->captureBlendDst();
    animCtrl->blend( 0.0f );

	_pilotchute->pull( Jumper::getLineHandJoint( _clump ) );
	_pilotchute->updateActivity(0.0f);

	PxTransform pose = _phActor->getGlobalPose();
	PxVec3 x = pose.q.getBasisVector0();
    PxVec3 y = pose.q.getBasisVector1();
    PxVec3 z = pose.q.getBasisVector2();
    _pilotchute->drop( z * 10.0f - x * 5.0f );
	_pilotchute->setInflation(0.6f);
}

void Jumper::PullReserve::update(float dt)
{
    updateAnimation( dt );

    // synchronize physics & render
    _clump->getFrame()->setMatrix( _matrixConversion->convert( wrap( _phActor->getGlobalPose() ) ) );
    _clump->getFrame()->getLTM();

    // look for opening
	if( _pilotchute->isOpened() ) {
		_endOfAction = true;
	}
    
    engine::IAnimationController* animCtrl = _clump->getAnimationController();
    if( animCtrl->isEndOfAnimation( 0 ) || _pilotchute->isDropped() )
    {
        // blend to second animation track (put hands over head)
        float weight = animCtrl->getTrackWeight( 0 );
        weight -= dt / trackBlendTime;
        if( weight < 0 ) weight = 0.0f;
        animCtrl->setTrackWeight( 0, weight );
        animCtrl->setTrackWeight( 1, 1.0f - weight );
    }
}