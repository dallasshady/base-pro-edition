
#include "headers.h"
#include "sound.h"
#include "gameplay.h"
#include "../common/istring.h"

/**
 * class implementation
 */

FlightSound::FlightSound(Actor* parent) : Actor( parent )
{
    _openSound = Gameplay::iAudio->createStaticSound( "./res/sounds/parachute/open.ogg" ); assert( _openSound );    
    _openSound->setLoop( false );
    _openSound->setGainLimits( 0.0f, 1.0f );
    _openSound->setDistanceModel( 1000.0f, 50000.0f, 1.0f );

    if( getScene()->getReverberation() )
    {    
        _openSound->setReverberation(
            getScene()->getReverberation()->inGain,
            getScene()->getReverberation()->reverbMixDB,
            getScene()->getReverberation()->reverbTime,
            getScene()->getReverberation()->hfTimeRatio
        );
    }

    // flapping sound
    _flappingSound = Gameplay::iAudio->createStaticSound( "./res/sounds/parachute/flapping.ogg" ); assert( _flappingSound );    
    _flappingSound->setGainLimits( 0.0f, 1.0f );
    _flappingSound->setLoop( true );
    _flappingSound->setDistanceModel( 1000.0f, 50000.0f, 1.0f );    

    if( getScene()->getReverberation() )
    {    
        _flappingSound->setReverberation(
            getScene()->getReverberation()->inGain,
            getScene()->getReverberation()->reverbMixDB,
            getScene()->getReverberation()->reverbTime,
            getScene()->getReverberation()->hfTimeRatio
        );
    }

    // flapping dynamics
    _flappingGain  = 0.0f;
    _flappingPitch = 0.0f;
    _velocity      = 0.0f;
}

FlightSound::~FlightSound()
{
    _openSound->release();
    _flappingSound->release();
}

/**
 * abstraction layer
 */

static float lerp(float minVel, float minVal, float maxVel, float maxVal, float vel)
{
    // interpolation coeff.
    float i = ( vel - minVel ) / ( maxVel - minVel );
    i = i < 0 ? 0 : ( i > 1 ? 1 : i );
    // result
    return minVal * ( 1.0f - i ) + maxVal * i;
}

void FlightSound::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
    if( initiator == _parent )
    {
        Matrix4f parentPose = _parent->getPose();
        Vector3f soundPos( parentPose[3][0], parentPose[3][1], parentPose[3][2] );

        switch( eventId )
        {
        case EVENT_CANOPY_OPEN:
            if( !_flappingSound->isPlaying() )
            {
                _openSound->play();
                _flappingSound->play();
            }
            break;
        case EVENT_CANOPY_VELOCITY:
            _velocity = *( (float*)( eventData ) );
            break;
        }

        // canopy pitch lerp
        float canopyMinPitchVel   = 0.0f;
        float canopyMinPitchValue = 0.5f;
        float canopyMaxPitchVel   = 14.0f;
        float canopyMaxPitchValue = 2.0f;
        // canopy gain lerp
        float canopyMinGainVel   = 0.0f;
        float canopyMinGainValue = 0.0f;
        float canopyMaxGainVel   = 14.0f;
        float canopyMaxGainValue = 3.0f;

        // result
        _flappingPitch = lerp( canopyMinPitchVel, canopyMinPitchValue, canopyMaxPitchVel, canopyMaxPitchValue, _velocity );
        _flappingGain  = lerp( canopyMinGainVel, canopyMinGainValue, canopyMaxGainVel, canopyMaxGainValue, _velocity );        
    }
}

void FlightSound::onUpdateActivity(float dt)
{
    Matrix4f parentPose = _parent->getPose();
    Vector3f soundPos( parentPose[3][0], parentPose[3][1], parentPose[3][2] );
    Vector3f soundVel = _parent->getVel();

    if( _flappingSound->isPlaying() )
    {
        _flappingSound->place( soundPos, soundVel );
        if( Gameplay::iGameplay->pitchShiftIsEnabled() )
        {
            _flappingSound->setPitchShift( _flappingPitch * _scene->getTimeSpeed() );
        }
        _flappingSound->setGain( _flappingGain );
        _openSound->place( soundPos, soundVel );
        if( Gameplay::iGameplay->pitchShiftIsEnabled() )
        {
            _openSound->setPitchShift( _scene->getTimeSpeed() );
        }
    }
}