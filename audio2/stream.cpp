
#include "headers.h"
#include "stream.h"
#include "errorreport.h"
#include "../common/istring.h"
#include "../shared/ccor.h"
#include "compability.h"

/**
 * class implementation
 */

const Vector3f defaultDir( 0,1,0 );
const Vector3f defaultVel( 0,0,0 );

StreamSound::StreamSounds StreamSound::_streamSounds;

DWORD StreamSound::minSecondarySampleRate = 0;
DWORD StreamSound::maxSecondarySampleRate = 0;

StreamSound::StreamSound(const char* resourceName, IDirectSound8* iDirectSound8, unsigned int notifyCount, unsigned int notifySize)
{
    assert( notifyCount );
    assert( notifySize );

    _resourceName          = resourceName;
    _iDirectSound8         = iDirectSound8;
    _iDirectSoundBuffer    = NULL;
    _iDirectSoundBuffer8   = NULL;
    _iDirectSound3DBuffer8 = NULL;
    _notifySize  = notifySize;
    _notifyCount = notifyCount;

    // open OGG file
    _oggFile = new OggFile( resourceName );

    // define sound buffer
    ZeroMemory( &_bufferFormat, sizeof(WAVEFORMATEX) ); 
    ZeroMemory( &_bufferDesc, sizeof(DSBUFFERDESC) );
    _bufferFormat.wFormatTag      = WAVE_FORMAT_PCM;
    _bufferFormat.nChannels       = _oggFile->getNumChannels();
    _bufferFormat.nSamplesPerSec  = _oggFile->getSamplingRate();
    _bufferFormat.wBitsPerSample  = 16;
    _bufferFormat.nBlockAlign     = _bufferFormat.wBitsPerSample / 8 * _bufferFormat.nChannels;
    _bufferFormat.nAvgBytesPerSec = _bufferFormat.nSamplesPerSec * _bufferFormat.nBlockAlign;
    _bufferDesc.dwSize  = sizeof( DSBUFFERDESC );
    _bufferDesc.dwFlags = DSBCAPS_CTRLFX | 
                          DSBCAPS_CTRLFREQUENCY | 
                          DSBCAPS_CTRLVOLUME | 
                          DSBCAPS_CTRLPOSITIONNOTIFY | 
                          DSBCAPS_GETCURRENTPOSITION2;/* | 
                          DSBCAPS_LOCSOFTWARE;*/
    if( _oggFile->getNumChannels() == 1 )
    {
        _bufferDesc.dwFlags = _bufferDesc.dwFlags | DSBCAPS_CTRL3D;// | DSBCAPS_MUTE3DATMAXDISTANCE;
    }
    _bufferDesc.dwBufferBytes   = notifyCount * notifySize;
    _bufferDesc.lpwfxFormat     = &_bufferFormat;
    _bufferDesc.guid3DAlgorithm = DS3DALG_DEFAULT;

    // create sound buffer
    _dsCR( _iDirectSound8->CreateSoundBuffer( &_bufferDesc, &_iDirectSoundBuffer, NULL ) );

    // obtain sound manager from sound buffer
    _dsCR( _iDirectSoundBuffer->QueryInterface( 
        IID_IDirectSoundBuffer8, 
        (VOID**)&_iDirectSoundBuffer8
    ) );

    // obtain 3D manager from sound manager
    if( _oggFile->getNumChannels() == 1 )
    {
        _dsCR( _iDirectSoundBuffer8->QueryInterface( 
            IID_IDirectSound3DBuffer, 
            (VOID**)&_iDirectSound3DBuffer8
        ) );

        // setup buffer mode
        _dsCR( _iDirectSound3DBuffer8->SetMode( DS3DMODE_NORMAL, DS3D_IMMEDIATE ) );
    }

    // reset management
    _isLooping = false;
    _isPlaying = false;
    _writeCursor = 0;
    _playProgress = 0;

    // register in management list
    _streamSounds.push_back( this );
}

StreamSound::~StreamSound()
{
    // stop sound
    if( isPlaying() ) stop();
    // release sound interfaces
    if( _iDirectSound3DBuffer8 ) _iDirectSound3DBuffer8->Release();
    _iDirectSoundBuffer8->Release();    
    _iDirectSoundBuffer->Release();
    // release OGG file
    delete _oggFile;    
    // unregister
    for( StreamSoundI streamSoundI = _streamSounds.begin();
                      streamSoundI != _streamSounds.end();
                      streamSoundI++ )
    {
        if( *streamSoundI == this )
        {
            _streamSounds.erase( streamSoundI );
            break;
        }
    }
}


void StreamSound::release(void)
{
    delete this;
}

bool StreamSound::isPlaying(void)
{
    return _isPlaying;
}

void StreamSound::play(void)
{
    // reset ogg file
    _oggFile->reset();

    // setup streaming
    _writeCursor = 0;
    _playProgress = 0;

    // initialize buffer
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
    uploadSound( reinterpret_cast<BYTE*>(bufferData1), bufferSize1 );
    _dsCR( _iDirectSoundBuffer->Unlock( bufferData1, bufferSize1, bufferData2, bufferSize2 ) );

    // play
    _dsCR( _iDirectSoundBuffer8->Play( 0, 0, DSBPLAY_LOOPING ) );
    _isPlaying = true;
}

void StreamSound::stop(void)
{
    // stop
    _dsCR( _iDirectSoundBuffer8->Stop() );
    _isPlaying = false;
}

void StreamSound::setGain(float value)
{
    _dsCR( _iDirectSoundBuffer8->SetVolume( convertToDecibels( value ) ) );
}

void StreamSound::setLoop(bool loop)
{
    _isLooping = loop;
}

void StreamSound::setPitchShift(float value)
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

/**
 * streaming routine
 */

void StreamSound::uploadSound(BYTE* buffer, DWORD size)
{    
    DWORD bytesRestToUpload = size;
    DWORD uploadOffset = 0;
    DWORD oggBytes;
    while( bytesRestToUpload > 0 )
    {
        oggBytes = _oggFile->readBlock( buffer + uploadOffset, bytesRestToUpload );
        bytesRestToUpload -= oggBytes;
        uploadOffset += oggBytes;
        if( bytesRestToUpload > 0 )
        {
            if( _isLooping )
            {
                _oggFile->reset();
            }
            else
            {
                FillMemory( buffer + uploadOffset, bytesRestToUpload, 0 );
                bytesRestToUpload = 0;
            }
        }
    }
    _playProgress += size;
}

void StreamSound::onUpdateStreamBuffer(void)
{
    void* bufferData1;
    void* bufferData2;
    DWORD bufferSize1;
    DWORD bufferSize2;

    // obtain play cursor
    DWORD playCursor = 0;
    DWORD safeCursor = 0;
    _dsCR( _iDirectSoundBuffer8->GetCurrentPosition( &playCursor, &safeCursor ) );

    if( playCursor > _writeCursor )
    {
        // the simplest case - just lock & write
        _dsCR( _iDirectSoundBuffer->Lock(
            _writeCursor,
            playCursor - _writeCursor,
            &bufferData1,
            &bufferSize1,
            &bufferData2,
            &bufferSize2,
            0
        ) );

        // buffer size and _nextWriteOffset are both multiples of _notifySize, 
        // it should the second buffer, so it should never be valid
        assert( bufferData2 == NULL );

        // load buffer with sound data
        uploadSound( reinterpret_cast<BYTE*>(bufferData1), bufferSize1 );

        // unlock buffer
        _dsCR( _iDirectSoundBuffer->Unlock( bufferData1, bufferSize1, NULL, NULL ) );

        // update cursor 
        _writeCursor = playCursor;
    }
    else if( playCursor < _writeCursor )
    {
        // maintain safety
        if( _writeCursor < safeCursor ) 
        {
            _writeCursor = safeCursor;
        }

        // lock entire buffer from write cursor
        _dsCR( _iDirectSoundBuffer->Lock(
            _writeCursor,
            _bufferDesc.dwBufferBytes - _writeCursor,
            &bufferData1,
            &bufferSize1,
            &bufferData2,
            &bufferSize2,
            0
        ) );

        // buffer size and _nextWriteOffset are both multiples of _notifySize, 
        // it should the second buffer, so it should never be valid
        assert( bufferData2 == NULL );

        // load buffer with sound data
        uploadSound( reinterpret_cast<BYTE*>(bufferData1), bufferSize1 );

        // unlock buffer
        _dsCR( _iDirectSoundBuffer->Unlock( bufferData1, bufferSize1, NULL, NULL ) );

        if( playCursor > 0 )
        {
            // lock buffer from beginning up to playing cursor
            _dsCR( _iDirectSoundBuffer->Lock(
                0,
                playCursor,
                &bufferData1,
                &bufferSize1,
                &bufferData2,
                &bufferSize2,
                0
            ) );

            // buffer size and _nextWriteOffset are both multiples of _notifySize, 
            // it should the second buffer, so it should never be valid
            assert( bufferData2 == NULL );

            // load buffer with sound data
            uploadSound( reinterpret_cast<BYTE*>(bufferData1), bufferSize1 );

            // unlock buffer
            _dsCR( _iDirectSoundBuffer->Unlock( bufferData1, bufferSize1, NULL, NULL ) );
        }

        // update cursor 
        _writeCursor = playCursor;
    }

    // cycle write cursor
    if( _writeCursor == _bufferDesc.dwBufferBytes ) _writeCursor = 0;

    // terminate playing of non-looping stream 
    if( !_isLooping && _playProgress >= _oggFile->getSize() + _bufferDesc.dwBufferBytes )
    {
        stop();
    }
}

void StreamSound::updateStreamBuffers(void)
{
    for( StreamSoundI streamSoundI = _streamSounds.begin();
                      streamSoundI != _streamSounds.end();
                      streamSoundI++ )
    {
        ( *streamSoundI )->onUpdateStreamBuffer();
    }
}