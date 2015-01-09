
#include "headers.h"
#include "sound.h"
#include "gameplay.h"

/**
 * class implementation
 */ 

AmbientSound::AmbientSound(Actor* parent, const char* resource, AmbientSoundDesc* desc) : Actor( parent )
{
    _sound = Gameplay::iAudio->createStreamSound( resource, 8, 1024*64 ); assert( _sound );
    _sound->setLoop( true );
    _desc  = *desc;
}

AmbientSound::~AmbientSound()
{
    _sound->release();
}

/**
 * actor abstracts
 */

void AmbientSound::onUpdateActivity(float dt)
{
    // retrieve camera position
    Actor* camera = _scene->getCamera();
    if( !camera ) return;
    Matrix4f pose = camera->getPose();

    float lerpValue = 0;
    switch( _desc.processingMode )
    {
    case apmAltitude: 
        // extract camera height
        lerpValue = pose[3][1];        
        break;
    case apmDistance:
        lerpValue = ( Vector3f( pose[3][0], pose[3][1], pose[3][2] ) - _desc.position ).length();
        break;
    }

    // make LERP factor
    float lerpFactor = ( lerpValue - _desc.minValue ) / ( _desc.maxValue - _desc.minValue );
    lerpFactor = lerpFactor < 0 ? 0 : lerpFactor;
    lerpFactor = lerpFactor > 1 ? 1 : lerpFactor;
    // make gain by camera height
    float gain = _desc.minValueGain * ( 1.0f - lerpFactor ) + _desc.maxValueGain * lerpFactor;

    // update sound properties
    if( !_sound->isPlaying() ) _sound->play();
    _sound->setGain( gain );
    if( Gameplay::iGameplay->pitchShiftIsEnabled() )
    {
        _sound->setPitchShift( _scene->getTimeSpeed() );
    }
}