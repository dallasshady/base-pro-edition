
#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/product_version.h"
#include "../shared/audio.h"
#include "../shared/mainwnd.h"

using namespace ccor;

/**
 * ISound implementation : no sound support
 */

class NoSound : public audio::ISound
{
private:
    bool _isPlaying;
public:
    NoSound() : _isPlaying(false) {}
public:
    virtual void __stdcall release(void)
    {
        delete this;
    }
    virtual bool __stdcall isPlaying(void)
    {
        return _isPlaying;
    }
    virtual void __stdcall play(void)
    {
        _isPlaying = true;
    }
    virtual void __stdcall stop(void)
    {
        _isPlaying = false;
    }
    virtual void __stdcall setGain(float value)
    {
    }
    virtual void __stdcall setLoop(bool loop)
    {
    }
    virtual void __stdcall setPitchShift(float value)
    {
    }
    virtual void __stdcall setDistanceModel(float refDist, float maxDist, float rolloff)
    {
    }
    virtual void __stdcall setGainLimits(float minGain, float maxGain)
    {
    }
    virtual void __stdcall place(const Vector3f& pos, const Vector3f& dir, float coneInnerAngle, float coneOuterAngle, float coneOuterGain, const Vector3f& vel)
    {
    }
    virtual void __stdcall place(const Vector3f& pos, const Vector3f& dir, float coneInnerAngle, float coneOuterAngle, float coneOuterGain)
    {
    }
    virtual void __stdcall place(const Vector3f& pos, const Vector3f& vel)
    {
    }
    virtual void __stdcall place(const Vector3f& pos)
    {
    }
    virtual void __stdcall setReverberation(float inGain, float reverbMixDB, float reverbTime, float hfTimeRatio)
    {
    }
};

/**
 * IAudio implementation : no sound support
 */

const Vector3f zeroV( 0,0,0 );

class Audio : public EntityBase, 
              virtual public audio::IAudio
{
public:
    Audio()
    {
    } 
    ~Audio() 
    {
    }
public:
    // EntityBase
    virtual void __stdcall entityInit(Object * p) 
    {
    }
    virtual void __stdcall entityAct(float dt) 
    {        
    }
    virtual IBase * __stdcall entityAskInterface(iid_t id) 
    { 
        if( id == audio::IAudio::iid ) return this;
        return NULL;
    }
    static EntityBase* creator() 
    { 
        return new Audio;
    }
    virtual void __stdcall entityDestroy() 
    { 
        delete this; 
    }
public:
    // IAudio
    virtual void __stdcall setListener(const Vector3f& pos, const Vector3f& up, const Vector3f& at, const Vector3f& vel)
    {
    }
    virtual void __stdcall setListener(const Matrix4f& matrix, const Vector3f& vel)
    {
    }
    virtual void __stdcall setListener(const Vector3f& pos, const Vector3f& up, const Vector3f& at)
    {
    }
    virtual void __stdcall setListener(const Matrix4f& matrix)
    {
    }
    virtual void __stdcall setListenerGain(float value)
    {
    }
    virtual float __stdcall getListenerGain(void)
    {
        return 1.0f;
    }
    virtual void __stdcall setSpeedOfSound(float value)
    {
    }
    virtual void __stdcall setDopplerFactor(float value)
    {
    }
    virtual audio::ISound* __stdcall createStaticSound(const char* resourceName)
    {
        return new NoSound;
    }
    virtual audio::ISound* __stdcall createReverbSound(const char* resourceName, audio::Reverb* reverbs)
    {
        return new NoSound;
    }
    virtual audio::ISound* __stdcall createStreamSound(const char* resourceName, unsigned int numBuffers, unsigned int bufferSize)
    {
        return new NoSound;
    }
};

SINGLE_ENTITY_COMPONENT(Audio);
