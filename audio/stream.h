
#ifndef STREAM_SOUND_INCLUDED
#define STREAM_SOUND_INCLUDED

#include "headers.h"
#include "../shared/audio.h"
#include "ogg.h"
#include "imem.h"

#define STREAM_THREAD_PERIOD (25)

class StreamSound : public audio::ISound,
                    virtual public Chain
{
private:
    OggFile* _oggFile;
    ALuint   _bufferSize;
    ALuint   _numBuffers;
    ALuint   _numBuffersAvaiable;
    ALuint   _unqueueCount;
    ALuint   _queueCount;
    ALuint   _dataToRead;
    ALuint*  _bufferIds;
    ALuint   _sourceId;
    bool     _loop;
    char     _eventName[256];
    HANDLE   _event;
    HANDLE   _thread;
private:
    static DWORD WINAPI threadProc(LPVOID lpParameter);
public:
    // class implementation
    StreamSound(const char* resourceName, unsigned int numBuffers, unsigned int bufferSize);
    virtual ~StreamSound();
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
    inline const char* getResourceName(void) { return _oggFile->getFileName(); }
};

#endif