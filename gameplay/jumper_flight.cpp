
#include "headers.h"
#include "jumper.h"
#include "imath.h"

/**
 * related animations
 */

static engine::AnimSequence passiveFlightSequence = 
{ 
    FRAMETIME(592), 
    FRAMETIME(652), 
    engine::ltPeriodic, 
    FRAMETIME(592) 
};

static engine::AnimSequence steerRightSequence = 
{
    FRAMETIME(652), 
    FRAMETIME(667), 
    engine::ltNone, 
    0.0f 
};

static engine::AnimSequence steerLeftSequence = 
{
    FRAMETIME(712), 
    FRAMETIME(721), 
    engine::ltNone, 
    0.0f
};

static engine::AnimSequence groupingSequence =
{
    FRAMETIME(832),
    FRAMETIME(851),
    engine::ltNone,
    0.0f
};

/**
 * class implementation
 */

Jumper::Flight::Flight(Jumper* jumper, NxActor* phFlight, MatrixConversion* mcFlight) :
    JumperAction( jumper )
{
    // set action properties
    _actionTime = 0.0f;
    _blendTime = 0.2f;
    _endOfAction = false;
    _phActor = phFlight;
    _matrixConversion = mcFlight;
    _targetSequence = &passiveFlightSequence;

    // setup action channels according deep brake setting
    Gameplay::iGameplay->getActionChannel( iaLeft )->setAmplitude( 0.0f );
    Gameplay::iGameplay->getActionChannel( iaRight )->setAmplitude( 0.0f );

    // setup animation
    setAnimation( &passiveFlightSequence );
}

void Jumper::Flight::setAnimation(engine::AnimSequence* sequence)
{
    _actionTime = 0.0f;

    // animation controller
    engine::IAnimationController* animCtrl = _clump->getAnimationController();

    // remove procedural influence
    animCtrl->advance( 0.0f );
    _clump->getFrame()->getLTM();

    // capture blend source
    animCtrl->captureBlendSrc();

    // reset animation mixer
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( animCtrl->getTrackAnimation( i ) ) animCtrl->setTrackActivity( i, false );
    }

    // setup animation
    animCtrl->setTrackAnimation( 0, sequence );
    animCtrl->setTrackActivity( 0, true );
    animCtrl->setTrackSpeed( 0, 1.0f );
    animCtrl->setTrackWeight( 0, 1.0f );
    animCtrl->resetTrackTime( 0 );
    animCtrl->advance( 0.0f );

    // capture blend destination
    animCtrl->captureBlendDst();
    animCtrl->blend( 0.0f );
}

void Jumper::Flight::update(float dt)
{
    // animation controller
    engine::IAnimationController* animCtrl = _clump->getAnimationController();

    // determine animation playing direction
    float animDir;
    if( _targetSequence == animCtrl->getTrackAnimation( 0 ) )
    {
        if( &groupingSequence == animCtrl->getTrackAnimation( 0 ) )
        {
            animDir = 0.75f;
        }
        else if( &steerRightSequence == animCtrl->getTrackAnimation( 0 ) ||
                 &steerLeftSequence == animCtrl->getTrackAnimation( 0 ) )
        {
            animDir = 0.25f;
        }
        else
        {
            animDir = 0.75f;
        }
    }
    else
    {
        if( &groupingSequence == animCtrl->getTrackAnimation( 0 ) )
        {
            animDir = -0.75f;
        }
        else if( &steerRightSequence == animCtrl->getTrackAnimation( 0 ) ||
                 &steerLeftSequence == animCtrl->getTrackAnimation( 0 ) )
        {
            animDir = -0.75f;
        }
        else
        {
            animDir = -0.75f;
        }
    }
   
    // determine animation switch
    if( _targetSequence != _clump->getAnimationController()->getTrackAnimation( 0 ) &&
        ( &passiveFlightSequence == _clump->getAnimationController()->getTrackAnimation( 0 ) ||
          _clump->getAnimationController()->isBeginOfAnimation( 0 ) ) )
    {
        setAnimation( _targetSequence );
    }

    // blend phase?
    if( _actionTime < _blendTime )
    {
        if( _actionTime + dt < _blendTime )
        {
            _actionTime += dt;
            _clump->getAnimationController()->blend( _actionTime/_blendTime );
        }
        else
        {
            // pass blend, move to animation
            dt -= ( _blendTime - _actionTime );
            _actionTime = _blendTime;
            _actionTime += dt;
            // advance animation
            _clump->getAnimationController()->advance( animDir * dt );
        }
    }
    else
    {              
        // advance animation
        _actionTime += dt;
        if( animDir < 0 || ( animDir > 0 && !_clump->getAnimationController()->isEndOfAnimation( 0 ) ) )
        {
            _clump->getAnimationController()->advance( animDir * dt );
        }
        else
        {
            _clump->getAnimationController()->advance( 0.0f );
        }
    }

    // synchronize physics & render
    _clump->getFrame()->setMatrix( _matrixConversion->convert( wrap( _phActor->getGlobalPose() ) ) );
    _clump->getFrame()->getLTM();
}

static float getAirResistancePower(float i)
{
    return ( exp( pow( i, 1.4f ) )  - 1.0f ) / 1.718f;
}

void Jumper::Flight::updatePhysics(void)
{
    SpinalCord* spinalCord = _jumper->getSpinalCord();
    Virtues* virtues = _jumper->getVirtues();
	CanopySimulator* canopy = _jumper->getDominantCanopy();

    // velocity of base jumper's body
    NxVec3 velocity = _phActor->getLinearVelocity();
    
    // horizontal velocity (including wind)
    NxVec3 velocityH = velocity; 
    velocityH += _jumper->getScene()->getWindAtPoint( _phActor->getGlobalPosition() );    
    velocityH.y = 0;

    // shock penalty
    float penalty = _jumper->getVirtues()->getControlPenalty( _jumper->getShock() );
    penalty = ( 1 - penalty );

    // update canopy controls
	// modifier to 50% input
	if (spinalCord->modifier) {
		float dt = _jumper->getDeltaTime();
		if (spinalCord->left > 0.5f && Gameplay::iGameplay->getActionChannel(iaLeft)->getTrigger()) Gameplay::iGameplay->getActionChannel(iaLeft)->downAmplitude(dt, 0.5f);
		if (spinalCord->right > 0.5f && Gameplay::iGameplay->getActionChannel(iaRight)->getTrigger()) Gameplay::iGameplay->getActionChannel(iaRight)->downAmplitude(dt, 0.5f);
		if (spinalCord->leftWarp > 0.5f && Gameplay::iGameplay->getActionChannel(iaLeftWarp)->getTrigger()) Gameplay::iGameplay->getActionChannel(iaLeftWarp)->downAmplitude(dt, 0.5f);
		if (spinalCord->rightWarp > 0.5f && Gameplay::iGameplay->getActionChannel(iaRightWarp)->getTrigger()) Gameplay::iGameplay->getActionChannel(iaRightWarp)->downAmplitude(dt, 0.5f);
		if (spinalCord->leftRearRiser > 0.5f && Gameplay::iGameplay->getActionChannel(iaLeftRearRiser)->getTrigger()) Gameplay::iGameplay->getActionChannel(iaLeftRearRiser)->downAmplitude(dt, 0.5f);
		if (spinalCord->rightRearRiser > 0.5f && Gameplay::iGameplay->getActionChannel(iaRightRearRiser)->getTrigger()) Gameplay::iGameplay->getActionChannel(iaRightRearRiser)->downAmplitude(dt, 0.5f);

		if (spinalCord->leftReserve > 0.5f && Gameplay::iGameplay->getActionChannel(iaReserveLeft)->getTrigger()) Gameplay::iGameplay->getActionChannel(iaReserveLeft)->downAmplitude(dt, 0.5f);
		if (spinalCord->rightReserve > 0.5f && Gameplay::iGameplay->getActionChannel(iaReserveRight)->getTrigger()) Gameplay::iGameplay->getActionChannel(iaReserveRight)->downAmplitude(dt, 0.5f);
		if (spinalCord->leftReserveWarp > 0.5f && Gameplay::iGameplay->getActionChannel(iaReserveLeftWarp)->getTrigger()) Gameplay::iGameplay->getActionChannel(iaReserveLeftWarp)->downAmplitude(dt, 0.5f);
		if (spinalCord->rightReserveWarp > 0.5f && Gameplay::iGameplay->getActionChannel(iaReserveRightWarp)->getTrigger()) Gameplay::iGameplay->getActionChannel(iaReserveRightWarp)->downAmplitude(dt, 0.5f);
		if (spinalCord->leftReserveRearRiser > 0.5f && Gameplay::iGameplay->getActionChannel(iaReserveLeftRearRiser)->getTrigger()) Gameplay::iGameplay->getActionChannel(iaReserveLeftRearRiser)->downAmplitude(dt, 0.5f);
		if (spinalCord->rightReserveRearRiser > 0.5f && Gameplay::iGameplay->getActionChannel(iaReserveRightRearRiser)->getTrigger()) Gameplay::iGameplay->getActionChannel(iaReserveRightRearRiser)->downAmplitude(dt, 0.5f);
	}

	// set toggles
	canopy->setLeftDeep( spinalCord->left * penalty );
	canopy->setRightDeep( spinalCord->right * penalty );
	// set risers
    canopy->setLeftWarpDeep( spinalCord->leftWarp * penalty );
    canopy->setRightWarpDeep( spinalCord->rightWarp * penalty );
    canopy->setLeftRearRiser( spinalCord->leftRearRiser * penalty );
    canopy->setRightRearRiser( spinalCord->rightRearRiser * penalty );

    canopy->setWLOToggles( spinalCord->wlo );
    canopy->setHookKnife( spinalCord->hook );

    // update reserve canopy controls
	if (_jumper->isPlayer() &&
		_jumper->getCanopyReserveSimulator() &&
		_jumper->getDominantCanopy() != _jumper->getCanopyReserveSimulator() && 
		_jumper->getCanopyReserveSimulator()->isOpened()) {

		// set toggles
		CanopySimulator *reserve_canopy = _jumper->getCanopyReserveSimulator();
		reserve_canopy->setLeftDeep( spinalCord->leftReserve * penalty );
		reserve_canopy->setRightDeep( spinalCord->rightReserve * penalty );
		// set risers
		reserve_canopy->setLeftWarpDeep( spinalCord->leftReserveWarp * penalty );
		reserve_canopy->setRightWarpDeep( spinalCord->rightReserveWarp * penalty );
		reserve_canopy->setLeftRearRiser( spinalCord->leftReserveRearRiser * penalty );
		reserve_canopy->setRightRearRiser( spinalCord->rightReserveRearRiser * penalty );

		reserve_canopy->setWLOToggles( spinalCord->wlo );
		reserve_canopy->setHookKnife( spinalCord->hook );
	}
    // determine animation sequence to be played
    _targetSequence = &passiveFlightSequence;
    if( spinalCord->left != 0 && spinalCord->right == 0 ) _targetSequence = &steerLeftSequence;
    if( spinalCord->right != 0 && spinalCord->left == 0 ) _targetSequence = &steerRightSequence;
    if( spinalCord->left != 0 && spinalCord->right != 0 ) _targetSequence = &groupingSequence;

    // local coordinate system of base jumper
    NxMat34 pose = _phActor->getGlobalPose();
    NxVec3 x = pose.M.getColumn(0);
    NxVec3 y = pose.M.getColumn(1);
    NxVec3 z = pose.M.getColumn(2);

    // air resistance coefficient
    float ARmult = 2.5f;
    float AR = ARmult * virtues->getTrackingAirResistance();
    if( database::Suit::getRecord( virtues->equipment.suit.id )->wingsuit )
    {
        AR = ARmult * virtues->getFrogAirResistance();
    }

    // terminal velocity
    float Vt = sqrt( 9.8f * _phActor->getMass() / AR );
    float It = velocity.magnitude() / Vt;

    NxVec3 dir = velocity * -1;
    dir.normalize();

    // air resistance force
    NxVec3 Far = dir * getAirResistancePower( velocity.magnitude() / Vt ) * _phActor->getMass() * 9.8f;

    // finalize motion equation    
    _phActor->addForce( Far );
}