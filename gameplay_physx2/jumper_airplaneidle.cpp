
#include "headers.h"
#include "jumper.h"
#include "imath.h"

static engine::AnimSequence airplaneIdleSequence =
{
    FRAMETIME(1633),
    FRAMETIME(1669),
    engine::ltMirror,
    FRAMETIME(1633)
};

static engine::AnimSequence airplaneIdleSequenceWings =
{
    FRAMETIME(2073),
    FRAMETIME(2104),
    engine::ltMirror,
    FRAMETIME(2073)
};

/**
 * class implementation
 */

Jumper::AirplaneIdle::AirplaneIdle(Jumper* jumper) : JumperAction( jumper )
{
    // set action properties
    _actionTime = 0.0f;
    _blendTime = 0.1f;
    _endOfAction = false;

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
        animCtrl->setTrackAnimation( 0, &airplaneIdleSequenceWings );
    }
    else
    {
        animCtrl->setTrackAnimation( 0, &airplaneIdleSequence );
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

Jumper::AirplaneIdle::~AirplaneIdle()
{
}

/**
 * Action
 */

void Jumper::AirplaneIdle::update(float dt)
{
    updateAnimation( dt );

    // place jumper to airplane exit
    Matrix4f clumpLTM = _clump->getFrame()->getLTM();
    Vector3f clumpScale = calcScale( clumpLTM );

    Matrix4f exitLTM = _jumper->getAirplaneExit()->getLTM();
    orthoNormalize( exitLTM );
    exitLTM[0][0] *= clumpScale[0], exitLTM[0][1] *= clumpScale[0], exitLTM[0][2] *= clumpScale[0];
    exitLTM[1][0] *= clumpScale[1], exitLTM[1][1] *= clumpScale[1], exitLTM[1][2] *= clumpScale[1];
    exitLTM[2][0] *= clumpScale[2], exitLTM[2][1] *= clumpScale[2], exitLTM[2][2] *= clumpScale[2];

    _clump->getFrame()->setMatrix( exitLTM );
}
