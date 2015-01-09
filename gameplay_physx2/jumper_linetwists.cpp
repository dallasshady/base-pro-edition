
#include "headers.h"
#include "jumper.h"
#include "imath.h"

/**
 * related animations
 */

static engine::AnimSequence linetwistsSequence = 
{ 
    FRAMETIME(2146), 
    FRAMETIME(2169), 
    engine::ltMirror,
    FRAMETIME(2146)
};

/**
 * class implementation
 */

Jumper::Linetwists::Linetwists(Jumper* jumper, NxActor* phFlight, MatrixConversion* mcFlight) :
    JumperAction( jumper )
{
    // set action properties
    _actionTime = 0.0f;
    _blendTime = 0.1f;
    _endOfAction = false;
    _phActor = phFlight;
    _matrixConversion = mcFlight;

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
    animCtrl->setTrackAnimation( 0, &linetwistsSequence );
    animCtrl->setTrackActivity( 0, true );
    animCtrl->setTrackSpeed( 0, 0.55f );
    animCtrl->setTrackWeight( 0, 1.0f );
    animCtrl->resetTrackTime( 0 );
    animCtrl->advance( 0.0f );

    // capture blend destination
    animCtrl->captureBlendDst();
    animCtrl->blend( 0.0f );
}

void Jumper::Linetwists::update(float dt)
{
    updateAnimation( dt );

    // synchronize physics & render
    _clump->getFrame()->setMatrix( _matrixConversion->convert( wrap( _phActor->getGlobalPose() ) ) );
    _clump->getFrame()->getLTM();
}

void Jumper::Linetwists::updatePhysics(void)
{
    SpinalCord* spinalCord = _jumper->getSpinalCord();
    Virtues* virtues = _jumper->getVirtues();
    CanopySimulator* canopy = _jumper->getCanopySimulator();

    // shock penalty
    float penalty = _jumper->getVirtues()->getControlPenalty( _jumper->getShock() );
    penalty = ( 1 - penalty );

    // update canopy controls
    canopy->setLeftWarpDeep( spinalCord->leftWarp * penalty );
    canopy->setRightWarpDeep( spinalCord->rightWarp * penalty );
}