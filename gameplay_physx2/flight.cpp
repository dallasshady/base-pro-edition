
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

	// update revebation based on altitude
	Matrix4f parentPose = _parent->getPose();
	updateReverbation(_openSound, parentPose[3][1]);

    // flapping sound
    _flappingSound = Gameplay::iAudio->createStaticSound( "./res/sounds/parachute/flapping.ogg" ); assert( _flappingSound );    
    _flappingSound->setGainLimits( 0.0f, 1.0f );
    _flappingSound->setLoop( true );
    _flappingSound->setDistanceModel( 1000.0f, 50000.0f, 1.0f );    

	// update revebation based on altitude
	updateReverbation(_flappingSound, parentPose[3][1]);

	// wind sound
    _jetSound = Gameplay::iAudio->createStaticSound( "./res/sounds/freefall/jet2.ogg" ); assert( _jetSound );    
    _jetSound->setGainLimits( 0.0f, 1.0f );
    _jetSound->setLoop( true );
    _jetSound->setDistanceModel( 2000.0f, 50000.0f, 1.0f );    

	// update revebation based on altitude
	updateReverbation(_jetSound, parentPose[3][1]);


    // flapping dynamics
    _flappingGain  = 0.0f;
    _flappingPitch = 0.0f;
    _jetGain      = 1.0f;
    _jetPitch     = 1.0f;
    _velocity     = 0.0f;
    _modifier     = 1.0f;
}

FlightSound::~FlightSound()
{
    _openSound->release();
    _flappingSound->release();
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

void FlightSound::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
    if( true || initiator == _parent )
    {
        Matrix4f parentPose = _parent->getPose();
        Vector3f soundPos( parentPose[3][0], parentPose[3][1], parentPose[3][2] );

        switch( eventId )
        {
        case EVENT_CANOPY_OPEN:
            if( !_flappingSound->isPlaying() )
            {
				_openSound->setGain(100.0f);
                _openSound->play();
                _flappingSound->play();
				_jetSound->play();
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

		_flappingGain  = _velocity;
		_flappingPitch = 0.0f;

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
        _jetPitch = lerp( jetMinPitchVel, jetMinPitchValue, jetMaxPitchVel, jetMaxPitchValue, _velocity );
        _jetGain  = lerp( jetMinGainVel, jetMinGainValue, jetMaxGainVel, jetMaxGainValue, _velocity );
        _jetPitch     *= _modifier;
    }
}

void FlightSound::updateReverbation(audio::ISound *sound, float altitude) {
	// get altitude and reverbation (if altitude is above maxAltitude of reverbation, dont play it
	if (altitude == 0.0f) return;

	database::LocationInfo::Reverberation *reverbation = getScene()->getReverberation();
	altitude /= 100.f;
	if(reverbation) {
		if (reverbation->maxAltitude >= altitude) {
			sound->setReverberation(
				reverbation->inGain,
				reverbation->reverbMixDB,
				reverbation->reverbTime,
				reverbation->hfTimeRatio
			);
			//getCore()->logMessage("Flight Altitude: %2.2f; Reverb: ON", altitude);
		} else {
			//getCore()->logMessage("Flight Altitude: %2.2f; Reverb: OFF", altitude);
			//sound->setReverberation(0, 0, 0, 0);
		}
    }
}

void FlightSound::onUpdateActivity(float dt)
{
    Matrix4f parentPose = _parent->getPose();
    Vector3f soundPos( parentPose[3][0], parentPose[3][1], parentPose[3][2] );
    Vector3f soundVel = _parent->getVel();

    if( _flappingSound->isPlaying() )
    {
		// update revebation based on altitude
		updateReverbation(_flappingSound, parentPose[3][1]);

        _flappingSound->place( soundPos, soundVel );
		
		//getCore()->logMessage("soundvel: %2.2f", soundVel.length());
		//_flappingSound->setPitchShift(soundVel.length() / 2000.0f);

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

		// jet sound
		// update revebation based on altitude
		updateReverbation(_flappingSound, parentPose[3][1]);

		_jetSound->place( soundPos, soundVel );
		_jetSound->setPitchShift( soundVel.length() / 1000.0f);
        if( Gameplay::iGameplay->pitchShiftIsEnabled() )
        {
            _jetSound->setPitchShift( _jetPitch * _scene->getTimeSpeed() );
        }
        _jetSound->setGain( _jetGain );
    }
}