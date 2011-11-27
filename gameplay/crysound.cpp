
#include "headers.h"
#include "sound.h"
#include "gameplay.h"
#include "../common/istring.h"

CrySound::CrySound(Actor* parent) : Actor( parent )
{
    _easyCry = Gameplay::iAudio->createStaticSound( "./res/sounds/cry/cry03.ogg" ); assert( _easyCry );    
    _easyCry->setLoop( false );
    _easyCry->setGainLimits( 0.0f, 1.0f );    
    _easyCry->setDistanceModel( 200.0f, 10000.0f, 1.0f );    

    _mediumCry = Gameplay::iAudio->createStaticSound( "./res/sounds/cry/cry02.ogg" ); assert( _mediumCry );    
    _mediumCry->setLoop( false );
    _mediumCry->setGainLimits( 0.0f, 1.0f );
    _mediumCry->setDistanceModel( 300.0f, 15000.0f, 1.0f );    

    _hardCry = Gameplay::iAudio->createStaticSound( "./res/sounds/cry/cry01.ogg" ); assert( _hardCry );    
    _hardCry->setLoop( false );
    _hardCry->setGainLimits( 0.0f, 1.0f );
    _hardCry->setDistanceModel( 400.0f, 20000.0f, 1.0f );    

    if( getScene()->getReverberation() )
    {
        _easyCry->setReverberation(
            getScene()->getReverberation()->inGain,
            getScene()->getReverberation()->reverbMixDB,
            getScene()->getReverberation()->reverbTime,
            getScene()->getReverberation()->hfTimeRatio
        );
        _mediumCry->setReverberation(
            getScene()->getReverberation()->inGain,
            getScene()->getReverberation()->reverbMixDB,
            getScene()->getReverberation()->reverbTime,
            getScene()->getReverberation()->hfTimeRatio
        );
        _hardCry->setReverberation(
            getScene()->getReverberation()->inGain,
            getScene()->getReverberation()->reverbMixDB,
            getScene()->getReverberation()->reverbTime,
            getScene()->getReverberation()->hfTimeRatio
        );
    }
}

CrySound::~CrySound()
{
    _hardCry->release();
    _mediumCry->release();
    _easyCry->release();
}

void CrySound::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
    if( initiator == _parent )
    {
        float defaultGain = 0.5f;
        bool isCrying = _easyCry->isPlaying() || _mediumCry->isPlaying() || _hardCry->isPlaying();
        if( eventId == EVENT_EASY_CRY && !isCrying )
        {
            _easyCry->play();
            _easyCry->setGain( defaultGain );
            onUpdateActivity( 0.0f );
        }
        else if( eventId == EVENT_MEDIUM_CRY && !isCrying )
        {
            _mediumCry->play();
            _mediumCry->setGain( defaultGain );
            onUpdateActivity( 0.0f );
        }
        else if( eventId == EVENT_HARD_CRY && !isCrying )
        {
            _hardCry->play();
            _hardCry->setGain( defaultGain );
            onUpdateActivity( 0.0f );
        }
    }
}

void CrySound::onUpdateActivity(float dt)
{
    Matrix4f parentPose = _parent->getPose();
    Vector3f soundPos( parentPose[3][0], parentPose[3][1], parentPose[3][2] );
    Vector3f soundVel = _parent->getVel();

    if( _easyCry->isPlaying() ) _easyCry->place( soundPos, soundVel );
    if( _mediumCry->isPlaying() ) _mediumCry->place( soundPos, soundVel );
    if( _hardCry->isPlaying() ) _hardCry->place( soundPos, soundVel );
}