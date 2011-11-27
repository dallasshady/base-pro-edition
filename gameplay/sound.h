
#ifndef SOUND_ACTORS_INCLUDED
#define SOUND_ACTORS_INCLUDED

#include "headers.h"
#include "scene.h"
#include "../shared/audio.h"

/**
 * ambient sound plays repeately
 */

enum AmbientProcessingMode
{    
    apmAltitude, // sound gain interpolated by altitude value
    apmDistance  // sound gain interpolated by distance to certain position
};

struct AmbientSoundDesc
{
public:
    AmbientProcessingMode processingMode; // processing mode
    Vector3f              position;       // processing position (for distance mode)
    float                 minValue;       // minimal processing value (alt. or dist.)
    float                 minValueGain;   // sound gain for minimal processing value
    float                 maxValue;       // maximal processing value (alt. or dist.)
    float                 maxValueGain;   // sound gain for maximal processing value
};

class AmbientSound : public Actor
{
private:
    audio::ISound*    _sound;
    AmbientSoundDesc  _desc;
public:
    // actor abstracts
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    AmbientSound(Actor* parent, const char* resource, AmbientSoundDesc* desc);
    virtual ~AmbientSound();
};

/**
 * footsteps sound
 */

class FootstepsSound : public Actor
{
private:
    audio::ISound* _walkSound;
    audio::ISound* _turnSound;
    float          _pitchMultiplier;
    float          _pitchShiftFactor;
public:
    // actor abstracts
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
    virtual void onUpdateActivity(float dt);
public:
    FootstepsSound(Actor* parent);
    virtual ~FootstepsSound();
};

/**
 * freefall sound
 */

class FreefallSound : public Actor
{
private:
    audio::ISound* _clothesSound;
    audio::ISound* _jetSound;
    float          _clothesGain;
    float          _clothesPitch;
    float          _jetGain;
    float          _jetPitch;
    float          _velocity;
    float          _modifier;
public:
    // actor abstracts
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
    virtual void onUpdateActivity(float dt);
public:
    FreefallSound(Actor* parent);
    virtual ~FreefallSound();
};

/**
 * flight sound
 */

class FlightSound : public Actor
{
private:
    audio::ISound* _openSound;
    audio::ISound* _flappingSound;
    float          _flappingGain;
    float          _flappingPitch;
    float          _velocity;
public:
    // actor abstracts
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
    virtual void onUpdateActivity(float dt);
public:
    FlightSound(Actor* parent);
    virtual ~FlightSound();
};

/**
 * cry sound
 */

class CrySound : public Actor
{
private:
    audio::ISound* _easyCry;
    audio::ISound* _mediumCry;
    audio::ISound* _hardCry;
public:
    // actor abstracts
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
    virtual void onUpdateActivity(float dt);
public:
    CrySound(Actor* parent);
    virtual ~CrySound();
};

#endif