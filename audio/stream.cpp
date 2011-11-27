
#include "headers.h"
#include "stream.h"

/**
 * class implementation
 */

const Vector3f defaultDir( 0,0,0 );
const Vector3f defaultVel( 0,0,0 );

StreamSound::StreamSound(const char* resourceName, unsigned int numBuffers, unsigned int bufferSize)
{
    // open sound file
    _oggFile = new OggFile( resourceName );

    // setup stream
    _bufferSize = bufferSize;
    _numBuffers = _numBuffersAvaiable = numBuffers;
    _unqueueCount = 0;
    _queueCount = 0;
    _dataToRead = 0;
    _loop = false;

    // create buffers
    _bufferIds = new ALuint[_numBuffers];
    alGenBuffers( _numBuffers, _bufferIds );
    assert( alGetError() == AL_NO_ERROR );

    _sourceId = 0;
    _event = NULL;
    _thread = NULL;
}

StreamSound::~StreamSound()
{
    stop();

    // delete buffers
    alDeleteBuffers( _numBuffers, _bufferIds );
    assert( alGetError() == AL_NO_ERROR );

    delete _oggFile;
}

/**
 * ISound
 */

void StreamSound::release(void)
{
    delete this;
}

bool StreamSound::isPlaying(void)
{
    return ( _event != NULL );
}

void StreamSound::play(void)
{
    if( !isPlaying() )
    {
        // create source
        alGenSources( 1, &_sourceId );
        assert( alGetError() == AL_NO_ERROR );

        // setup source
        alSourcei( _sourceId, AL_SOURCE_RELATIVE, false );
        assert( alGetError() == AL_NO_ERROR );
        alSourcef( _sourceId, AL_MIN_GAIN, 0 );
        assert( alGetError() == AL_NO_ERROR );
        alSourcef( _sourceId, AL_MAX_GAIN, 1 );
        assert( alGetError() == AL_NO_ERROR );

        // create thread event
        sprintf( _eventName, "StreamSoundEvent_%X", (unsigned int)(this) );
        _event = CreateEvent( NULL, TRUE, FALSE, _eventName );
        assert( _event );

        // create thread 
        _thread = CreateThread( NULL, 0, threadProc, this, 0, NULL );
        assert( _thread ); 
    }
}

void StreamSound::stop(void)
{
    if( isPlaying() )
    {
        // stop thread
        SetEvent( _event );
        WaitForSingleObject( _thread, INFINITE );
        CloseHandle( _event );
        CloseHandle( _thread );
        _event = NULL;
        _thread = NULL;
    
        // delete source
        alDeleteSources( 1, &_sourceId );
        assert( alGetError() == AL_NO_ERROR );
    }
}

void StreamSound::setGain(float value)
{
    alSourcef( _sourceId, AL_GAIN, value );
    assert( alGetError() == AL_NO_ERROR );
}

void StreamSound::setLoop(bool loop)
{
    _loop = loop;
}

void StreamSound::setPitchShift(float value)
{
    alSourcef( _sourceId, AL_PITCH, value );
    assert( alGetError() == AL_NO_ERROR );
}

void StreamSound::setGainLimits(float minGain, float maxGain)
{
    alSourcef( _sourceId, AL_MIN_GAIN, minGain );
    assert( alGetError() == AL_NO_ERROR );
    alSourcef( _sourceId, AL_MAX_GAIN, maxGain );
    assert( alGetError() == AL_NO_ERROR );
}

void StreamSound::place(const Vector3f& pos, const Vector3f& dir, float coneInnerAngle, float coneOuterAngle, float coneOuterGain, const Vector3f& vel)
{
    Vector3f temp;
    float    fv[6];

    // position
    fv[0] = pos[0] * 0.01f, fv[1] = pos[1] * 0.01f, fv[2] = -pos[2] * 0.01f;
    alSourcefv( _sourceId, AL_POSITION, fv );
    assert( alGetError() == AL_NO_ERROR );

    // velocity
    fv[0] = vel[0] * 0.01f, fv[1] = vel[1] * 0.01f, fv[2] = -vel[2] * 0.01f;
    alSourcefv( _sourceId, AL_VELOCITY, fv );
    assert( alGetError() == AL_NO_ERROR );

    // direction
    temp = dir; temp.normalize();
    fv[0] = temp[0], fv[1] = temp[1], fv[2] = -temp[2];
    alSourcefv( _sourceId, AL_DIRECTION, fv );
    assert( alGetError() == AL_NO_ERROR );

    // cone properties
    alSourcef( _sourceId, AL_CONE_INNER_ANGLE, coneInnerAngle );
    assert( alGetError() == AL_NO_ERROR );
    alSourcef( _sourceId, AL_CONE_OUTER_ANGLE, coneOuterAngle );
    assert( alGetError() == AL_NO_ERROR );
    alSourcef( _sourceId, AL_CONE_OUTER_GAIN, coneOuterGain );
    assert( alGetError() == AL_NO_ERROR );
}

void StreamSound::setDistanceModel(float refDist, float maxDist, float rolloff)
{
    refDist *= 0.01f;
    maxDist *= 0.01f;

    alSourcef( _sourceId, AL_REFERENCE_DISTANCE, refDist );
    assert( alGetError() == AL_NO_ERROR );    
    alSourcef( _sourceId, AL_MAX_DISTANCE, maxDist );
    assert( alGetError() == AL_NO_ERROR );
    alSourcef( _sourceId, AL_ROLLOFF_FACTOR, rolloff );
    assert( alGetError() == AL_NO_ERROR );
}

void StreamSound::place(const Vector3f& pos, const Vector3f& dir, float coneInnerAngle, float coneOuterAngle, float coneOuterGain)
{
    place( pos, dir, coneInnerAngle, coneOuterAngle, coneOuterGain, defaultVel );
}

void StreamSound::place(const Vector3f& pos, const Vector3f& vel)
{
    place( pos, defaultDir, 360, 360, 1.0f, vel );
}

void StreamSound::place(const Vector3f& pos)
{
    place( pos, defaultDir, 360, 360, 1.0f, defaultVel );
}

/**
 * thread proc
 */

DWORD StreamSound::threadProc(LPVOID lpParameter)
{
    StreamSound* __this = reinterpret_cast<StreamSound*>( lpParameter );
    char* buffer = new char[__this->_bufferSize];

    // reset data
    __this->_oggFile->reset();
    __this->_dataToRead = __this->_oggFile->getSize();

    while( true )
    {
        if( WaitForSingleObject( __this->_event, STREAM_THREAD_PERIOD ) == WAIT_OBJECT_0 ) break;
        
        ALint numProcessed;
        ALuint bufferId;
        alGetSourcei( __this->_sourceId, AL_BUFFERS_PROCESSED, &numProcessed );
        while( numProcessed )
        {
            alSourceUnqueueBuffers( __this->_sourceId, 1, &bufferId );
            if( ++__this->_unqueueCount == __this->_numBuffers ) __this->_unqueueCount = 0;
            ++__this->_numBuffersAvaiable;
            --numProcessed;
        }
        if( __this->_dataToRead && __this->_numBuffersAvaiable )
        {                
            ALsizei blockSize = ( __this->_dataToRead > __this->_bufferSize ) ? __this->_bufferSize : __this->_dataToRead;
            __this->_oggFile->readBlock( buffer, blockSize );
            __this->_dataToRead -= blockSize;
            ALenum format = ( __this->_oggFile->getNumChannels() == 1 ) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
            alBufferData( __this->_bufferIds[__this->_queueCount], format, buffer, blockSize, __this->_oggFile->getSamplingRate() );
            alSourceQueueBuffers( __this->_sourceId, 1, &__this->_bufferIds[__this->_queueCount] );
            if( ++__this->_queueCount == __this->_numBuffers ) __this->_queueCount = 0;
            --__this->_numBuffersAvaiable;
        }
        if( __this->_numBuffersAvaiable == __this->_numBuffers )
        {
            if( __this->_loop )
            {
                __this->_oggFile->reset();
                __this->_dataToRead = __this->_oggFile->getSize();
            }
        }
        else
        {
            ALint sourceState;
            alGetSourcei( __this->_sourceId, AL_SOURCE_STATE, &sourceState );
            if( sourceState != AL_PLAYING )
            {   
                alGetSourcei( __this->_sourceId, AL_BUFFERS_PROCESSED, &numProcessed );
                while( numProcessed )
                {
                    alSourceUnqueueBuffers( __this->_sourceId, 1, &bufferId );
                    if( ++__this->_unqueueCount == __this->_numBuffers )
                    __this->_unqueueCount = 0;
                    ++__this->_numBuffersAvaiable;
                    --numProcessed;
                }
                alSourcePlay( __this->_sourceId );
            }
        }
    }

    alSourceStop( __this->_sourceId );
    assert( alGetError() == AL_NO_ERROR );

    ALuint bufferId = 0;
    while( __this->_numBuffersAvaiable < __this->_numBuffers )
    {
        alSourceUnqueueBuffers( __this->_sourceId, 1, &bufferId );
        assert( alGetError() == AL_NO_ERROR );
        if( ++__this->_unqueueCount == __this->_numBuffers ) __this->_unqueueCount = 0;
        ++__this->_numBuffersAvaiable;
    }
    assert( __this->_queueCount == __this->_unqueueCount );

    alSourcei( __this->_sourceId, AL_BUFFER, NULL );
    assert( alGetError() == AL_NO_ERROR );

    delete[] buffer;
    return 0;
}