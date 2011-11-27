
#include "headers.h"
#include "reverb.h"

/**
 * class implementation
 */

const Vector3f defaultDir( 0,0,0 );
const Vector3f defaultVel( 0,0,0 );

ReverbSound::ReverbSound(const char* resourceName, audio::Reverb* reverbs)
{
    _resourceName = resourceName;

    unsigned int numReverbs = 0;
    audio::Reverb* reverb = reverbs;
    while( ( reverb->shift != 0.0f ) && ( reverb->gain != 0.0f ) ) numReverbs++, reverb++;

    unsigned int i,j;

    // open sound file
    OggFile* oggFile = new OggFile( resourceName );

    // read sound data from file
    ALuint size = oggFile->getSize();
    char* originalData = new char[size];
    oggFile->readBlock( originalData, size );

    // typecast sound data 
    short int* originalSamples = (short int*)(originalData);
    unsigned int numOriginalSamples = size / 2;

    // reference data size (in samples)
    ALuint refSamples = oggFile->getSamplingRate() * oggFile->getNumChannels();

    // make storage for reverberation data and copy original data
    char* reverbData = new char[size];
    short int* reverbSamples = (short int*)(reverbData);
    memset( reverbData, 0, size );
    memcpy( reverbData, originalData, size );

    // make reverberation effect
    ALuint offset, rj;
    float  sample;
    for( i=0; i<numReverbs; i++ )
    {
        offset = ALuint( refSamples * reverbs[i].shift );
        if( oggFile->getNumChannels() == 2 && offset % 2 ) offset--;
        for( j=0; j<numOriginalSamples; j++ )
        {
            rj = offset + j;
            rj = ( rj >= numOriginalSamples ) ? rj - numOriginalSamples : rj;
            sample = float( reverbSamples[rj] ) + float( originalSamples[j] ) * reverbs[i].gain;
            sample = sample > 32767 ? 32767 : sample;
            sample = sample < -32768 ? -32768 : sample;
            reverbSamples[rj] = short int( sample );
        }
    }

    // create buffer
    alGenBuffers( 1, &_bufferId );
    assert( alGetError() == AL_NO_ERROR );

    // fill buffer with reverbetation data
    ALenum format = ( oggFile->getNumChannels() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16 );
    alBufferData( _bufferId, format, reverbData, size, oggFile->getSamplingRate() );
    assert( alGetError() == AL_NO_ERROR );
    delete reverbData;
    delete originalData;

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

ReverbSound::~ReverbSound()
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

void ReverbSound::release(void)
{
    delete this;
}

bool ReverbSound::isPlaying(void)
{
    ALint state;
    alGetSourcei( _sourceId, AL_SOURCE_STATE, &state );
    return state == AL_PLAYING;
}

void ReverbSound::play(void)
{
    if( !isPlaying() ) 
    {
        alSourcePlay( _sourceId );
        assert( alGetError() == AL_NO_ERROR );
    }
}

void ReverbSound::stop(void)
{
    alSourceStop( _sourceId );
    assert( alGetError() == AL_NO_ERROR );
}

void ReverbSound::setGain(float value)
{
    alSourcef( _sourceId, AL_GAIN, value );
    assert( alGetError() == AL_NO_ERROR );
}

void ReverbSound::setLoop(bool loop)
{
    alSourcei( _sourceId, AL_LOOPING, loop );
    assert( alGetError() == AL_NO_ERROR );
}

void ReverbSound::setPitchShift(float value)
{
    alSourcef( _sourceId, AL_PITCH, value );
    assert( alGetError() == AL_NO_ERROR );
}

void ReverbSound::setGainLimits(float minGain, float maxGain)
{
    alSourcef( _sourceId, AL_MIN_GAIN, minGain );
    assert( alGetError() == AL_NO_ERROR );
    alSourcef( _sourceId, AL_MAX_GAIN, maxGain );
    assert( alGetError() == AL_NO_ERROR );
}

void ReverbSound::place(const Vector3f& pos, const Vector3f& dir, float coneInnerAngle, float coneOuterAngle, float coneOuterGain, const Vector3f& vel)
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

void ReverbSound::setDistanceModel(float refDist, float maxDist, float rolloff)
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

void ReverbSound::place(const Vector3f& pos, const Vector3f& dir, float coneInnerAngle, float coneOuterAngle, float coneOuterGain)
{
    place( pos, dir, coneInnerAngle, coneOuterAngle, coneOuterGain, defaultVel );
}

void ReverbSound::place(const Vector3f& pos, const Vector3f& vel)
{
    place( pos, defaultDir, 360, 360, 1.0f, vel );
}

void ReverbSound::place(const Vector3f& pos)
{
    place( pos, defaultDir, 360, 360, 1.0f, defaultVel );
}