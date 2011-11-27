
#ifndef STATIC_SOUND_INCLUDED
#define STATIC_SOUND_INCLUDED

#include "headers.h"
#include "../shared/audio.h"
#include "ogg.h"
#include "spatial.h"

class StaticSound : public SpatialSound
{
private:
    std::string                 _resourceName;
    WAVEFORMATEX                _bufferFormat;
    DSBUFFERDESC                _bufferDesc;
    IDirectSound8*              _iDirectSound8;
    IDirectSoundBuffer*         _iDirectSoundBuffer;
    IDirectSoundBuffer8*        _iDirectSoundBuffer8;
    IDirectSound3DBuffer8*      _iDirectSound3DBuffer8;
    IDirectSoundFXWavesReverb8* _iDirectSoundFXWavesReverb8;
    bool                        _fxWavesReverbIsActual;
    bool                        _isPlaying;
    bool                        _isLooping;
protected:
    // SpatialSound
    virtual IDirectSound3DBuffer8* getIDirectSound3DBuffer8(void) { return _iDirectSound3DBuffer8; }
    virtual IDirectSoundFXWavesReverb8* getIDirectSoundFXWavesReverb8(void);
public:
    // class implementation
    StaticSound(const char* resourceName, IDirectSound8* iDirectSound8);
    virtual ~StaticSound();
    // ISound
    virtual void __stdcall release(void);
    virtual bool __stdcall isPlaying(void);
    virtual void __stdcall play(void);
    virtual void __stdcall stop(void);
    virtual void __stdcall setGain(float value);
    virtual void __stdcall setLoop(bool loop);
    virtual void __stdcall setPitchShift(float value);
public:
    // frequency mixing capabilities (setup externally by Audio entity)
    static DWORD minSecondarySampleRate;
    static DWORD maxSecondarySampleRate;
};

#endif