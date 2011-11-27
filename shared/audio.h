/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description audio module
 *
 * @author bad3p
 */

#ifndef AUDIO_INTERFACE_INCLUDED
#define AUDIO_INTERFACE_INCLUDED

#include "../shared/ccor.h"

namespace audio
{

/**
 * reverberation descriptor
 */

struct Reverb
{
public:    
    float shift; // echo latency, in seconds
    float gain;  // echo attenuation
};

/**
 * Sound 
 */

class ISound
{
public:
    DECLARE_INTERFACE_ID(0x90002);
public:
    virtual void __stdcall release(void) = 0;
    virtual bool __stdcall isPlaying(void) = 0;
    virtual void __stdcall play(void) = 0;
    virtual void __stdcall stop(void) = 0;
    virtual void __stdcall setGain(float value) = 0;
    virtual void __stdcall setLoop(bool loop) = 0;
    virtual void __stdcall setPitchShift(float value) = 0;
    virtual void __stdcall setDistanceModel(float refDist, float maxDist, float rolloff) = 0;
    virtual void __stdcall setGainLimits(float minGain, float maxGain) = 0;
    virtual void __stdcall place(const Vector3f& pos, const Vector3f& dir, float coneInnerAngle, float coneOuterAngle, float coneOuterGain, const Vector3f& vel) = 0;
    virtual void __stdcall place(const Vector3f& pos, const Vector3f& dir, float coneInnerAngle, float coneOuterAngle, float coneOuterGain) = 0;
    virtual void __stdcall place(const Vector3f& pos, const Vector3f& vel) = 0;
    virtual void __stdcall place(const Vector3f& pos) = 0;    
public:
    /**
     * wave reverberation
     *  - inGain; volume of input signal (0...1)
     *  - reverbGain; volume of reverberation mix (0...1, but actually ~0.9....1)
     *  - reverbTime; reverberation time, in seconds
     *  - hfTimeRatio; high-frequency time ratio (0.001...0.999)
     */
    virtual void __stdcall setReverberation(float inGain, float reverbMixDB, float reverbTime, float hfTimeRatio) = 0;
};

/**
 * Audio entity
 */

class IAudio : public ccor::IBase 
{
public:
    DECLARE_INTERFACE_ID(0x90000);
public:
    /**
     * listener properties
     */
    virtual void __stdcall setListener(const Vector3f& pos, const Vector3f& up, const Vector3f& at, const Vector3f& vel) = 0;
    virtual void __stdcall setListener(const Matrix4f& matrix, const Vector3f& vel) = 0;
    virtual void __stdcall setListener(const Vector3f& pos, const Vector3f& up, const Vector3f& at) = 0;
    virtual void __stdcall setListener(const Matrix4f& matrix) = 0;
    virtual void __stdcall setListenerGain(float value) = 0;
    virtual float __stdcall getListenerGain(void) = 0;
    /**
     * 3d sound simulation
     */
    virtual void __stdcall setSpeedOfSound(float value) = 0;
    virtual void __stdcall setDopplerFactor(float value) = 0;
    /**
     * sound creation
     */
    virtual ISound* __stdcall createStaticSound(const char* resourceName) = 0;
    virtual ISound* __stdcall createReverbSound(const char* resourceName, Reverb* reverbs) = 0;
    virtual ISound* __stdcall createStreamSound(const char* resourceName, unsigned int numBuffers, unsigned int bufferSize) = 0;
    /**
     * streaming management
     * use this method in case of long delays in entity acting routing
     */
    virtual void __stdcall updateStreamSounds(void) = 0;
};

}

#endif
