
#include "headers.h"
#include "static.h"
#include "errorreport.h"
#include "compability.h"

/**
 * class implementation
 */

const Vector3f defaultDir( 0,1,0 );
const Vector3f defaultVel( 0,0,0 );

DWORD StaticSound::minSecondarySampleRate = 0;
DWORD StaticSound::maxSecondarySampleRate = 0;

StaticSound::StaticSound(const char* resourceName, IDirectSound8* iDirectSound8)
{
    _resourceName          = resourceName;
    _iDirectSound8         = iDirectSound8;
    _iDirectSoundBuffer    = NULL;
    _iDirectSoundBuffer8   = NULL;
    _iDirectSound3DBuffer8 = NULL;

    // open sound file
    OggFile* oggFile = new OggFile( resourceName );

    // define sound buffer
    ZeroMemory( &_bufferFormat, sizeof(WAVEFORMATEX) ); 
    ZeroMemory( &_bufferDesc, sizeof(DSBUFFERDESC) );
    _bufferFormat.wFormatTag      = WAVE_FORMAT_PCM;
    _bufferFormat.nChannels       = oggFile->getNumChannels();
    _bufferFormat.nSamplesPerSec  = oggFile->getSamplingRate();
    _bufferFormat.wBitsPerSample  = 16;
    _bufferFormat.nBlockAlign     = _bufferFormat.wBitsPerSample / 8 * _bufferFormat.nChannels;
    _bufferFormat.nAvgBytesPerSec = _bufferFormat.nSamplesPerSec * _bufferFormat.nBlockAlign;
    _bufferDesc.dwSize  = sizeof( DSBUFFERDESC );
    _bufferDesc.dwFlags = DSBCAPS_CTRLFX | 
                          DSBCAPS_CTRLFREQUENCY | 
                          DSBCAPS_CTRLVOLUME;
    if( oggFile->getNumChannels() == 1 )
    {
        _bufferDesc.dwFlags = _bufferDesc.dwFlags | DSBCAPS_CTRL3D;// | DSBCAPS_MUTE3DATMAXDISTANCE;
    }
    _bufferDesc.dwBufferBytes   = oggFile->getSize();
    _bufferDesc.lpwfxFormat     = &_bufferFormat;
    _bufferDesc.guid3DAlgorithm = DS3DALG_DEFAULT;

    // create sound buffer
    _dsCR( _iDirectSound8->CreateSoundBuffer( &_bufferDesc, &_iDirectSoundBuffer, NULL ) );

    // obtain sound manager from sound buffer
    _dsCR( _iDirectSoundBuffer->QueryInterface( 
        IID_IDirectSoundBuffer8, 
        (VOID**)&_iDirectSoundBuffer8
    ) );

    // fill sound buffer with data
    void* bufferData1 = NULL;
    void* bufferData2 = NULL;
    DWORD bufferSize1 = 0;
    DWORD bufferSize2 = 0;
    _dsCR( _iDirectSoundBuffer->Lock(
        0,
        _bufferDesc.dwBufferBytes,
        &bufferData1,
        &bufferSize1,
        &bufferData2,
        &bufferSize2,
        0
    ) );
    assert( bufferSize2 == 0 );
    oggFile->readBlock( bufferData1, bufferSize1 );
    _dsCR( _iDirectSoundBuffer->Unlock( bufferData1, bufferSize1, bufferData2, bufferSize2 ) );

    // obtain 3D manager from sound manager
    if( oggFile->getNumChannels() == 1 )
    {
        _dsCR( _iDirectSoundBuffer8->QueryInterface( 
            IID_IDirectSound3DBuffer, 
            (VOID**)&_iDirectSound3DBuffer8
        ) );

        // setup buffer mode
        _dsCR( _iDirectSound3DBuffer8->SetMode( DS3DMODE_NORMAL, DS3D_IMMEDIATE ) );
    }

    // delete OGG file
    delete oggFile;

    // reset management
    _isPlaying = false;
    _isLooping = false;
    _iDirectSoundFXWavesReverb8 = NULL;
    _fxWavesReverbIsActual = false;
}

StaticSound::~StaticSound()
{
    if( isPlaying() ) stop();
    if( _iDirectSound3DBuffer8 ) _iDirectSound3DBuffer8->Release();
    _iDirectSoundBuffer8->Release();    
    _iDirectSoundBuffer->Release();
}

/**
 * ISound
 */

void StaticSound::release(void)
{
    delete this;
}

bool StaticSound::isPlaying(void)
{
    if( _isPlaying )
    {
        DWORD status;
        _dsCR( _iDirectSoundBuffer8->GetStatus( &status ) );
        if( !(status & DSBSTATUS_PLAYING) )
        {
            _isPlaying = false;
        }
    }
    return _isPlaying;
}

void StaticSound::play(void)
{
    _dsCR( _iDirectSoundBuffer8->Play( 0, 0, _isLooping ? DSBPLAY_LOOPING : 0 ) );    
    _isPlaying = true;
}

void StaticSound::stop(void)
{
    _dsCR( _iDirectSoundBuffer8->Stop() );
    _isPlaying = false;
}

void StaticSound::setGain(float value)
{
    _dsCR( _iDirectSoundBuffer8->SetVolume( convertToDecibels( value ) ) );
}

void StaticSound::setLoop(bool loop)
{
    _isLooping = loop;
}

void StaticSound::setPitchShift(float value)
{
    // TEST_ISSUE
    // return;

    // reset frequency
    _dsCR( _iDirectSoundBuffer8->SetFrequency( DSBFREQUENCY_ORIGINAL ) );

    // retrieve default frequency    
    DWORD defaultFrequency = 1;
    _dsCR( _iDirectSoundBuffer8->GetFrequency( &defaultFrequency ) );

    // calculate frequency
    DWORD frequency = DWORD( defaultFrequency * value );
    
    // saturate frequency
    if( frequency < minSecondarySampleRate ) frequency = minSecondarySampleRate;
    if( frequency > maxSecondarySampleRate ) frequency = maxSecondarySampleRate;
    
    // override frequency
    _dsCR( _iDirectSoundBuffer8->SetFrequency( frequency ) );
}

IDirectSoundFXWavesReverb8* StaticSound::getIDirectSoundFXWavesReverb8(void)
{
    if( _fxWavesReverbIsActual )
    {
        return _iDirectSoundFXWavesReverb8;
    }
    else
    {
        // now effect interface becomes an actual
        _fxWavesReverbIsActual = true;

        // setup reverberation effect on buffer
        DSEFFECTDESC dsEffectDesc;
        ZeroMemory( &dsEffectDesc, sizeof(DSEFFECTDESC) );
        dsEffectDesc.dwSize  = sizeof(DSEFFECTDESC);
        dsEffectDesc.dwFlags = DSFX_LOCSOFTWARE;
        dsEffectDesc.guidDSFXClass = GUID_DSFX_WAVES_REVERB;
        DWORD setFxResult;
        if( _iDirectSoundBuffer8->SetFX( 1, &dsEffectDesc, &setFxResult ) == DS_OK )
        {
            _dsCR( _iDirectSoundBuffer8->GetObjectInPath(
                GUID_DSFX_WAVES_REVERB,
                0,
                IID_IDirectSoundFXWavesReverb,
                (VOID**)&_iDirectSoundFXWavesReverb8 
            ) );
        }

        // return interface
        return _iDirectSoundFXWavesReverb8;
    }
}