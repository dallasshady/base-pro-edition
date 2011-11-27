
#include "headers.h"
#include "static.h"

/**
 * class implementation
 */

const Vector3f defaultDir( 0,0,0 );
const Vector3f defaultVel( 0,0,0 );

StaticSound::StaticSound(const char* resourceName)
{
    _resourceName = resourceName;

    // open sound file
    OggFile* oggFile = new OggFile( resourceName );

    // create buffer
    alGenBuffers( 1, &_bufferId );
    assert( alGetError() == AL_NO_ERROR );

    // read sound data from file
    ALuint size = oggFile->getSize();
    char* data = new char[size];
    oggFile->readBlock( data, size );    

    // fill buffer with sound data
    ALenum format = ( oggFile->getNumChannels() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16 );
    alBufferData( _bufferId, format, data, oggFile->getSize(), oggFile->getSamplingRate() );
    delete data;

    // create source
    alGenSources( 1, &_sourceId );
    assert( alGetError() == AL_NO_ERROR );

    // attach buffer to source
    alSourcei( _sourceId, AL_BUFFER, _bufferId );
    assert( alGetError() == AL_NO_ERROR );

    // setup source
    alSourcei( _sourceId, AL_SOURCE_RELATIVE, false );
    assert( alGetError() == AL_NO_ERROR );

    alSourcef( _sourceId, AL_MIN_GAIN, 0 );
    assert( alGetError() == AL_NO_ERROR );
    alSourcef( _sourceId, AL_MAX_GAIN, 1 );
    assert( alGetError() == AL_NO_ERROR );

    delete oggFile;
}

StaticSound::~StaticSound()
{
    // stop playing
    alSourceStop( _sourceId );
    assert( alGetError() == AL_NO_ERROR );
    alSourcei( _sourceId, AL_BUFFER, AL_NONE );    
    assert( alGetError() == AL_NO_ERROR );

    // delete source
    alDeleteSources( 1, &_sourceId );
    assert( alGetError() == AL_NO_ERROR );

    // delete buffer
    alDeleteBuffers( 1, &_bufferId );
    assert( alGetError() == AL_NO_ERROR );
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
    ALint state;
    alGetSourcei( _sourceId, AL_SOURCE_STATE, &state );
    return state == AL_PLAYING;
}

void StaticSound::play(void)
{
    if( !isPlaying() ) 
    {
        alSourcePlay( _sourceId );
        assert( alGetError() == AL_NO_ERROR );
    }
}

void StaticSound::stop(void)
{
    alSourceStop( _sourceId );
    assert( alGetError() == AL_NO_ERROR );
}

void StaticSound::setGain(float value)
{
    alSourcef( _sourceId, AL_GAIN, value );
    assert( alGetError() == AL_NO_ERROR );
}

void StaticSound::setLoop(bool loop)
{
    alSourcei( _sourceId, AL_LOOPING, loop );
    assert( alGetError() == AL_NO_ERROR );
}

void StaticSound::setPitchShift(float value)
{
    alSourcef( _sourceId, AL_PITCH, value );
    assert( alGetError() == AL_NO_ERROR );
}

void StaticSound::setGainLimits(float minGain, float maxGain)
{
    alSourcef( _sourceId, AL_MIN_GAIN, minGain );
    assert( alGetError() == AL_NO_ERROR );
    alSourcef( _sourceId, AL_MAX_GAIN, maxGain );
    assert( alGetError() == AL_NO_ERROR );
}

void StaticSound::place(const Vector3f& pos, const Vector3f& dir, float coneInnerAngle, float coneOuterAngle, float coneOuterGain, const Vector3f& vel)
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

void StaticSound::setDistanceModel(float refDist, float maxDist, float rolloff)
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

void StaticSound::place(const Vector3f& pos, const Vector3f& dir, float coneInnerAngle, float coneOuterAngle, float coneOuterGain)
{
    place( pos, dir, coneInnerAngle, coneOuterAngle, coneOuterGain, defaultVel );
}

void StaticSound::place(const Vector3f& pos, const Vector3f& vel)
{
    place( pos, defaultDir, 360, 360, 1.0f, vel );
}

void StaticSound::place(const Vector3f& pos)
{
    place( pos, defaultDir, 360, 360, 1.0f, defaultVel );
}