
#ifndef STREAM_SOUND_INCLUDED
#define STREAM_SOUND_INCLUDED

#include "headers.h"
#include "../shared/audio.h"
#include "ogg.h"
#include "spatial.h"

class StreamSound :  public SpatialSound
{
private:
    typedef std::list<StreamSound*> StreamSounds;
    typedef StreamSounds::iterator StreamSoundI;
private:
    static StreamSounds _streamSounds;
private:
    std::string            _resourceName;
    unsigned int           _notifyCount;
    unsigned int           _notifySize;
    OggFile*               _oggFile;
    WAVEFORMATEX           _bufferFormat;
    DSBUFFERDESC           _bufferDesc;
    IDirectSound8*         _iDirectSound8;
    IDirectSoundBuffer*    _iDirectSoundBuffer;
    IDirectSoundBuffer8*   _iDirectSoundBuffer8;
    IDirectSound3DBuffer8* _iDirectSound3DBuffer8;    
    bool                   _isLooping;
    bool                   _isPlaying;
    DWORD                  _writeCursor;
    DWORD                  _playProgress;
private:
    void uploadSound(BYTE* buffer, DWORD size);
    void onUpdateStreamBuffer(void);
protected:
    // SpatialSound
    virtual IDirectSound3DBuffer8* getIDirectSound3DBuffer8(void) { return _iDirectSound3DBuffer8; }
    virtual IDirectSoundFXWavesReverb8* getIDirectSoundFXWavesReverb8(void) { return NULL; }
public:
    // class implementation
    StreamSound(const char* resourceName, IDirectSound8* iDirectSound8, unsigned int notifyCount, unsigned int notifySize);
    virtual ~StreamSound();
    // ISound
    virtual void __stdcall release(void);
    virtual bool __stdcall isPlaying(void);
    virtual void __stdcall play(void);
    virtual void __stdcall stop(void);
    virtual void __stdcall setGain(float value);
    virtual void __stdcall setLoop(bool loop);
    virtual void __stdcall setPitchShift(float value);
public:
    static void updateStreamBuffers(void);
public:
    // frequency mixing capabilities (setup externally by Audio entity)
    static DWORD minSecondarySampleRate;
    static DWORD maxSecondarySampleRate;
};

#endif