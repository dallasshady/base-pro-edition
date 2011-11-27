
#ifndef SILENCE_INCLUDED
#define SILENCE_INCLUDED

#include "headers.h"
#include "../shared/audio.h"

class Silence : public audio::ISound
{
public:
    bool _isPlaying;
public:
    // class implementation
    Silence() : _isPlaying(false) {}
    virtual ~Silence() {}
    // ISound
    virtual void __stdcall release(void) { delete this; }
    virtual bool __stdcall isPlaying(void) { return _isPlaying; }
    virtual void __stdcall play(void) { _isPlaying = true; }
    virtual void __stdcall stop(void) { _isPlaying = false; }
    virtual void __stdcall setGain(float value) {}
    virtual void __stdcall setLoop(bool loop) {}
    virtual void __stdcall setPitchShift(float value) {}
    virtual void __stdcall setDistanceModel(float refDist, float maxDist, float rolloff) {}
    virtual void __stdcall setGainLimits(float minGain, float maxGain) {}
    virtual void __stdcall place(const Vector3f& pos, const Vector3f& dir, float coneInnerAngle, float coneOuterAngle, float coneOuterGain, const Vector3f& vel) {}
    virtual void __stdcall place(const Vector3f& pos, const Vector3f& dir, float coneInnerAngle, float coneOuterAngle, float coneOuterGain) {}
    virtual void __stdcall place(const Vector3f& pos, const Vector3f& vel) {}
    virtual void __stdcall place(const Vector3f& pos) {}
    virtual void __stdcall setReverberation(float inGain, float reverbMixDB, float reverbTime, float hfTimeRatio) {}
};

#endif