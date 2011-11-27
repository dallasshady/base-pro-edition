
#include "headers.h"
#include "jumper.h"
#include "imath.h"

/**
 * jump animation
 */

static engine::AnimSequence standingJumpSequence = 
{ 
    FRAMETIME(225), 
    FRAMETIME(251),  
    engine::ltNone, 
    0.0f 
};

static engine::AnimSequence standingJumpSequenceWings = 
{ 
    FRAMETIME(1946), 
    FRAMETIME(1985),  
    engine::ltNone, 
    0.0f 
};

/**
 * class implementation
 */

Jumper::StandingJump::StandingJump(Jumper* jumper, NxActor* actor, MatrixConversion* mc) :
    JumperAction( jumper )
{
    // set action properties
    _actionTime = 0.0f;
    _blendTime = 0.1f;
    _endOfAction = false;
    _phActor = actor;
    _matrixConversion = mc;

    engine::IAnimationController* animCtrl = _clump->getAnimationController();

    // capture blend source
    animCtrl->captureBlendSrc();

    // reset animation mixer
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( animCtrl->getTrackAnimation( i ) ) animCtrl->setTrackActivity( i, false );
    }

    bool useWingsuit = database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->wingsuit;

    // setup animation cycle
    if( useWingsuit )
    {
        animCtrl->setTrackAnimation( 0, &standingJumpSequenceWings );
    }
    else
    {
        animCtrl->setTrackAnimation( 0, &standingJumpSequence );
		getCore()->logMessage("animation time: %2.5f", FRAMETIME(251) - FRAMETIME(225));
    }
    animCtrl->setTrackActivity( 0, true );
    animCtrl->setTrackSpeed( 0, 0.75f );
    animCtrl->setTrackWeight( 0, 1.0f );
    animCtrl->resetTrackTime( 0 );    
    animCtrl->advance( 0.0f );

    // capture blend destination
    animCtrl->captureBlendDst();
    animCtrl->blend( 0.0f );
}

/**
 * Action implementation
 */

void Jumper::StandingJump::update(float dt)
{
    updateAnimation( dt );
    _clump->getFrame()->getLTM();

    if( _actionTime < _blendTime ) return;

    if( _actionTime > _blendTime + ( FRAMETIME(245) - FRAMETIME(225) ) )
    {
        if( _phActor->isSleeping() )
        {
            Matrix4f sampleLTM = Jumper::getCollisionFF( _clump )->getFrame()->getLTM();
            _phActor->setGlobalPose( wrap( sampleLTM ) );
            _phActor->wakeUp();
            NxVec3 velH = wrap( _clump->getFrame()->getAt() );
            velH.normalize();
            velH *= 3.0f;
            NxVec3 velV = wrap( _clump->getFrame()->getUp() );
            velV.normalize();
            velV *= 1.5f;
            _phActor->setLinearVelocity( velH + velV );
            _jumper->initOverburdenCalculator( velH + velV );
        }
        else
        {
            _clump->getFrame()->setMatrix( _matrixConversion->convert( wrap( _phActor->getGlobalPose() ) ) );
        }
		if( _jumper->getSpinalCord()->modifier) _endOfAction = true;
    }

    if( _clump->getAnimationController()->isEndOfAnimation( 0 ) )
    {
        _endOfAction = true;
    }
	getCore()->logMessage("actiontime: %2.5f", _actionTime);
}