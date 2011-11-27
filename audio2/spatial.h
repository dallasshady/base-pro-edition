
#ifndef SPATIAL_SOUND_INCLUDED
#define SPATIAL_SOUND_INCLUDED

#include "headers.h"
#include "../shared/audio.h"

class SpatialSound : public audio::ISound
{
protected:
    typedef std::list<SpatialSound*> SpatialSounds;
    typedef SpatialSounds::iterator SpatialSoundI;
protected:
    static SpatialSounds _spatialSounds;
protected:
    Vector3f   _lastListenerPos;
    Vector3f   _lastListenerVel;
    DS3DBUFFER _absoluteParameters; // absolute spatial parameters
    DS3DBUFFER _relativeParameters; // relative spatial parameters
protected:
    // abstraction
    virtual IDirectSound3DBuffer8* getIDirectSound3DBuffer8(void) = 0;
    virtual IDirectSoundFXWavesReverb8* getIDirectSoundFXWavesReverb8(void) = 0;
protected:
    // class internal behaviour
    void onListener(const Vector3f& listenerPos, const Vector3f& listenerVel);
public:
    // class implementation
    SpatialSound();
    virtual ~SpatialSound();
public:
    // audio::ISound
    virtual void __stdcall setGainLimits(float minGain, float maxGain) {}
    virtual void __stdcall setDistanceModel(float refDist, float maxDist, float rolloff);
    virtual void __stdcall place(const Vector3f& pos, const Vector3f& dir, float coneInnerAngle, float coneOuterAngle, float coneOuterGain, const Vector3f& vel);
    virtual void __stdcall place(const Vector3f& pos, const Vector3f& dir, float coneInnerAngle, float coneOuterAngle, float coneOuterGain);
    virtual void __stdcall place(const Vector3f& pos, const Vector3f& vel);
    virtual void __stdcall place(const Vector3f& pos);
    virtual void __stdcall setReverberation(float inGain, float reverbMixDB, float reverbTime, float hfTimeRatio);
public:
    // common interface
    static void setListenerParameters(const Vector3f& listenerPos, const Vector3f& listenerVel);
};

#endif