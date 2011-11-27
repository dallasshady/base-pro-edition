
#ifndef STATIC_SOUND_INCLUDED
#define STATIC_SOUND_INCLUDED

#include "headers.h"
#include "../shared/audio.h"
#include "ogg.h"
#include "imem.h"

class StaticSound : public audio::ISound,
                    virtual public Chain
{
private:
    std::string _resourceName;
    ALuint      _bufferId;
    ALuint      _sourceId;
public:
    // class implementation
    StaticSound(const char* resourceName);
    virtual ~StaticSound();
    // ISound
    virtual void __stdcall release(void);
    virtual bool __stdcall isPlaying(void);
    virtual void __stdcall play(void);
    virtual void __stdcall stop(void);
    virtual void __stdcall setGain(float value);
    virtual void __stdcall setLoop(bool loop);
    virtual void __stdcall setPitchShift(float value);
    virtual void __stdcall setDistanceModel(float refDist, float maxDist, float rolloff);
    virtual void __stdcall setGainLimits(float minGain, float maxGain);
    virtual void __stdcall place(const Vector3f& pos, const Vector3f& dir, float coneInnerAngle, float coneOuterAngle, float coneOuterGain, const Vector3f& vel);
    virtual void __stdcall place(const Vector3f& pos, const Vector3f& dir, float coneInnerAngle, float coneOuterAngle, float coneOuterGain);
    virtual void __stdcall place(const Vector3f& pos, const Vector3f& vel);
    virtual void __stdcall place(const Vector3f& pos);
    virtual void __stdcall setReverberation(float inGain, float reverbMixDB, float reverbTime, float hfTimeRatio) {}
public:
    // inlines
    inline const char* getResourceName(void) { return _resourceName.c_str(); }
};

#endif