
#include "headers.h"
#include "sound.h"
#include "gameplay.h"
#include "../common/istring.h"

/**
 * class implementation
 */

FreefallSound::FreefallSound(Actor* parent) : Actor( parent )
{
    // create sound of flapping clothes 
    _clothesSound = Gameplay::iAudio->createStaticSound( "./res/sounds/freefall/clothes.ogg" ); assert( _clothesSound );    
    _clothesSound->setLoop( true );
    _clothesSound->setGainLimits( 0.0f, 1.0f );
    _clothesSound->setDistanceModel( 1000.0f, 25000.0f, 1.0f );    

    if( getScene()->getReverberation() )
    {    
        _clothesSound->setReverberation(
            getScene()->getReverberation()->inGain,
            getScene()->getReverberation()->reverbMixDB,
            getScene()->getReverberation()->reverbTime,
            getScene()->getReverberation()->hfTimeRatio
        );
    }

    // create specific jet noise that is falling human body produced
    _jetSound = Gameplay::iAudio->createStaticSound( "./res/sounds/freefall/jet2.ogg" ); assert( _jetSound );    
    _jetSound->setGainLimits( 0.0f, 1.0f );
    _jetSound->setLoop( true );
    _jetSound->setDistanceModel( 2000.0f, 50000.0f, 1.0f );    

    if( getScene()->getReverberation() )
    {    
        _jetSound->setReverberation(
            getScene()->getReverberation()->inGain,
            getScene()->getReverberation()->reverbMixDB,
            getScene()->getReverberation()->reverbTime,
            getScene()->getReverberation()->hfTimeRatio
        );
    }
    
    _clothesGain  = 1.0f;
    _clothesPitch = 1.0f;
    _jetGain      = 1.0f;
    _jetPitch     = 1.0f;
    _velocity     = 0.0f;
    _modifier     = 1.0f;
}

FreefallSound::~FreefallSound()
{
    if( _clothesSound ) _clothesSound->release();
    if( _jetSound ) _jetSound->release();
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

void FreefallSound::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
    if( initiator == _parent )
    {
        Matrix4f parentPose = _parent->getPose();
        Vector3f soundPos( parentPose[3][0], parentPose[3][1], parentPose[3][2] );

        switch( eventId )
        {
        case EVENT_JUMPER_BEGIN_FREEFALL:
            if( !_clothesSound->isPlaying() )
            {
                _clothesSound->play();
                _jetSound->play();
            }
            break;
        case EVENT_JUMPER_END_FREEFALL:
            if( _clothesSound->isPlaying() )
            {
                _clothesSound->stop();
                _jetSound->stop();
            }
            break;
        case EVENT_JUMPER_FREEFALL_VELOCITY:
            _velocity = *( (float*)( eventData ) );
            break;
        case EVENT_JUMPER_FREEFALL_MODIFIER:
            _modifier = *( (float*)( eventData ) );
            break;
        }

        // clothes pitch lerp
        float clothesMinPitchVel   = 0.0f;
        float clothesMinPitchValue = 1.0f;
        float clothesMaxPitchVel   = 50.0f;
        float clothesMaxPitchValue = 5.0f;
        // clothes gain lerp
        float clothesMinGainVel   = 0.0f;
        float clothesMinGainValue = 0.0f;
        float clothesMaxGainVel   = 10.0f;
        float clothesMaxGainValue = 1.0f;

        // jet pitch lerp
        float jetMinPitchVel   = 0.0f;
        float jetMinPitchValue = 1.0f;
        float jetMaxPitchVel   = 50.0f;
        float jetMaxPitchValue = 2.0f;
        // jet gain lerp
        float jetMinGainVel   = 0.0f;
        float jetMinGainValue = 0.0f;
        float jetMaxGainVel   = 50.0f;
        float jetMaxGainValue = 2.0f;

        // result
        _clothesPitch = lerp( clothesMinPitchVel, clothesMinPitchValue, clothesMaxPitchVel, clothesMaxPitchValue, _velocity );
        _clothesGain  = lerp( clothesMinGainVel, clothesMinGainValue, clothesMaxGainVel, clothesMaxGainValue, _velocity );
        _jetPitch = lerp( jetMinPitchVel, jetMinPitchValue, jetMaxPitchVel, jetMaxPitchValue, _velocity );
        _jetGain  = lerp( jetMinGainVel, jetMinGainValue, jetMaxGainVel, jetMaxGainValue, _velocity );
        _clothesPitch *= _modifier;
        _jetPitch     *= _modifier;
    }
}

void FreefallSound::onUpdateActivity(float dt)
{
    Matrix4f parentPose = _parent->getPose();
    Vector3f soundPos( parentPose[3][0], parentPose[3][1], parentPose[3][2] );
    Vector3f soundVel = _parent->getVel();

    if( _clothesSound->isPlaying() )
    {
        _clothesSound->place( soundPos, soundVel );
        if( Gameplay::iGameplay->pitchShiftIsEnabled() )
        {
            _clothesSound->setPitchShift( _clothesPitch * _scene->getTimeSpeed() );
        }
        _clothesSound->setGain( _clothesGain );
        _jetSound->place( soundPos, soundVel );
        if( Gameplay::iGameplay->pitchShiftIsEnabled() )
        {
            _jetSound->setPitchShift( _jetPitch * _scene->getTimeSpeed() );
        }
        _jetSound->setGain( _jetGain );
    }
}