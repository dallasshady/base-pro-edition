
#include "headers.h"
#include "sound.h"
#include "gameplay.h"

/**
 * class implementation
 */

FootstepsSound::FootstepsSound(Actor* parent) : Actor( parent )
{
    _walkSound = parent->getScene()->createWalkSound();
    _turnSound = parent->getScene()->createTurnSound();
    if( getScene()->getReverberation() )
    {
        _walkSound->setReverberation(
            getScene()->getReverberation()->inGain,
            getScene()->getReverberation()->reverbMixDB,
            getScene()->getReverberation()->reverbTime,
            getScene()->getReverberation()->hfTimeRatio
        );
        _turnSound->setReverberation(
            getScene()->getReverberation()->inGain,
            getScene()->getReverberation()->reverbMixDB,
            getScene()->getReverberation()->reverbTime,
            getScene()->getReverberation()->hfTimeRatio
        );
    }
    _pitchShiftFactor = 1.0f;
}

FootstepsSound::~FootstepsSound()
{
    _walkSound->release();
    _turnSound->release();
}

/**
 * abstraction layer
 */

void FootstepsSound::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
    if( eventId == EVENT_SCENE_REVERBERATION_IS_CHANGED )
    {
        if( getScene()->getReverberation() )
        {
            getCore()->logMessage( "FootstepsSound reverberation changed" );
            _walkSound->setReverberation(
                getScene()->getReverberation()->inGain,
                getScene()->getReverberation()->reverbMixDB,
                getScene()->getReverberation()->reverbTime,
                getScene()->getReverberation()->hfTimeRatio
            );
            _turnSound->setReverberation(
                getScene()->getReverberation()->inGain,
                getScene()->getReverberation()->reverbMixDB,
                getScene()->getReverberation()->reverbTime,
                getScene()->getReverberation()->hfTimeRatio
            );
        }
    }

    if( initiator == _parent )
    {
        Matrix4f parentPose = _parent->getPose();
        Vector3f soundPos( parentPose[3][0], parentPose[3][1], parentPose[3][2] );

        switch( eventId )
        {        
        case EVENT_JUMPER_BEGIN_WALKFWD:
            if( !_walkSound->isPlaying() )
            {
                _pitchMultiplier = _scene->getWalkPitch();
                if( Gameplay::iGameplay->pitchShiftIsEnabled() )
                {
                    _walkSound->setPitchShift( _pitchShiftFactor * _pitchMultiplier * _scene->getTimeSpeed() );
                }
                _walkSound->place( soundPos );
                _walkSound->play();
            }
            break;
        case EVENT_JUMPER_END_WALKFWD:
            if( _walkSound->isPlaying() ) _walkSound->stop();
            break;
        case EVENT_JUMPER_BEGIN_WALKBCK:
            if( !_walkSound->isPlaying() )
            {
                _pitchMultiplier = _scene->getBackPitch();
                if( Gameplay::iGameplay->pitchShiftIsEnabled() )
                {
                    _walkSound->setPitchShift( _pitchShiftFactor * _pitchMultiplier * _scene->getTimeSpeed() );
                }
                _walkSound->place( soundPos );
                _walkSound->play();
            }
            break;
        case EVENT_JUMPER_END_WALKBCK:
            if( _walkSound->isPlaying() ) _walkSound->stop();
            break;
        case EVENT_JUMPER_BEGIN_TURN:
            if( !_turnSound->isPlaying() )
            {
                if( Gameplay::iGameplay->pitchShiftIsEnabled() )
                {
                    _turnSound->setPitchShift( _scene->getTurnPitch() * _scene->getTimeSpeed() );
                }
                _turnSound->place( soundPos );
                _turnSound->play();
            }
            break;
        case EVENT_JUMPER_END_TURN:
            if( _turnSound->isPlaying() ) _turnSound->stop();
            break;
        case EVENT_JUMPER_PITCHSHIFT:
            assert( eventData );
            _pitchShiftFactor = *( (float*)( eventData ) );
            if( _walkSound->isPlaying() ) 
            {
                if( Gameplay::iGameplay->pitchShiftIsEnabled() )
                {
                    _walkSound->setPitchShift( _pitchShiftFactor * _pitchMultiplier * _scene->getTimeSpeed() );
                }
            }
            break;
        }
    }
}

void FootstepsSound::onUpdateActivity(float dt)
{
    Matrix4f parentPose = _parent->getPose();
    Vector3f soundPos( parentPose[3][0], parentPose[3][1], parentPose[3][2] );

    if( _walkSound->isPlaying() )
    {
        _walkSound->place( soundPos );
        if( Gameplay::iGameplay->pitchShiftIsEnabled() )
        {
            _walkSound->setPitchShift( _pitchShiftFactor * _pitchMultiplier * _scene->getTimeSpeed() );
        }
    }

    if( _turnSound->isPlaying() )
    {
        _turnSound->place( soundPos );
        if( Gameplay::iGameplay->pitchShiftIsEnabled() )
        {
            _turnSound->setPitchShift( _scene->getTurnPitch() * _scene->getTimeSpeed() );
        }
    }
}