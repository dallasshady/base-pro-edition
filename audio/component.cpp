
#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/product_version.h"
#include "../shared/audio.h"
#include "../shared/mainwnd.h"
#include "static.h"
#include "stream.h"
#include "reverb.h"
#include "imem.h"

using namespace ccor;

/**
 * IAudio implementation
 */

const Vector3f zeroV( 0,0,0 );

class OpenALAudio : public EntityBase, 
              virtual public audio::IAudio
{
private:
    ALCdevice*   _device;
    ALCcontext*  _context;
public:
    OpenALAudio() : _device(NULL), _context(NULL) 
    {
    } 
    ~OpenALAudio() 
    {
        // release debugging chain links
        if( Chain::first )
        {
            FILE* f = fopen( "audio.garbage", "wt" );
            std::string className;        
            std::string objectName;
            while( Chain::first )
            {
                className  = "class";
                objectName = "object";
                if( dynamic_cast<StaticSound*>( Chain::first ) )
                {
                    className = "StaticSound";
                    objectName = dynamic_cast<StaticSound*>( Chain::first )->getResourceName();
                }
                else if( dynamic_cast<StreamSound*>( Chain::first ) )
                {
                    className = "StreamSound";
                    objectName = dynamic_cast<StreamSound*>( Chain::first )->getResourceName();
                }
                else if( dynamic_cast<ReverbSound*>( Chain::first ) )
                {
                    className = "ReverbSound";
                    objectName = dynamic_cast<ReverbSound*>( Chain::first )->getResourceName();
                }
                fprintf( f, "\"%s\" \"%s\" %s\n", className.c_str(), objectName.c_str(), Chain::first->getContext() );
                delete Chain::first;
            }
            fclose( f );
        }

        if( _context ) alcDestroyContext( _context );
        if( _device ) alcCloseDevice( _device );
    }
public:
    // EntityBase
    virtual void __stdcall entityInit(Object * p) 
    {
        // enumerate devices
        const ALCchar* deviceList = alcGetString( NULL, ALC_DEVICE_SPECIFIER );
        const ALCchar* deviceName;
        while( *deviceList != 0 )
        {
            deviceName = deviceList;
            deviceList = deviceList + strlen( deviceList ) + 1;
        }

        // create device
        _device = alcOpenDevice( "Generic Hardware" );
        if( !_device ) 
        {
            _device = alcOpenDevice( "Generic Software" );
            if( !_device ) throw Exception( "Cannot create audio device" );
        }

        // enumerate extensions
        const ALCchar* extensionList = alcGetString( _device, ALC_EXTENSIONS );

        // create context
        _context = alcCreateContext( _device, NULL );
        if( !_context ) throw Exception( "Cannot create audio context" );
        alcMakeContextCurrent( _context );

        // setup 3D-sound simulation
        alDistanceModel( AL_LINEAR_DISTANCE );
        alSpeedOfSound( 343.3f );
        alDopplerFactor( 3.0f );

        // setup default listener position
        setListener( zeroV, zeroV, zeroV, zeroV );
        setListenerGain( 1.0f );

        // reset error handling
        alGetError();
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
        return new OpenALAudio; 
    }
    virtual void __stdcall entityDestroy() 
    { 
        delete this; 
    }
public:
    // IAudio
    virtual void __stdcall setListener(const Vector3f& pos, const Vector3f& up, const Vector3f& at, const Vector3f& vel)
    {
        Vector3f temp;
        float    fv[6];

        // position
        fv[0] = pos[0] * 0.01f, fv[1] = pos[1] * 0.01f, fv[2] = -pos[2] * 0.01f;
        alListenerfv( AL_POSITION, fv );
        assert( alGetError() == AL_NO_ERROR );

        // orientation        
        temp = at; temp.normalize();
        fv[0] = temp[0], fv[1] = temp[1], fv[2] = -temp[2];
        temp = up; temp.normalize();
        fv[3] = temp[0], fv[4] = temp[1], fv[5] = -temp[2];
        alListenerfv( AL_ORIENTATION, fv );
        assert( alGetError() == AL_NO_ERROR );

        // velocity
        fv[0] = vel[0] * 0.01f, fv[1] = vel[1] * 0.01f, fv[2] = -vel[2] * 0.01f;
        alListenerfv( AL_VELOCITY, fv );
        assert( alGetError() == AL_NO_ERROR );
    }
    virtual void __stdcall setListener(const Matrix4f& matrix, const Vector3f& vel)
    {
        setListener(
            Vector3f( matrix[3][0], matrix[3][1], matrix[3][2] ),
            Vector3f( matrix[1][0], matrix[1][1], matrix[1][2] ),
            Vector3f( matrix[2][0], matrix[2][1], matrix[2][2] ),
            vel
        );
    }
    virtual void __stdcall setListener(const Vector3f& pos, const Vector3f& up, const Vector3f& at)
    {
        setListener( pos, up, at, zeroV );
    }
    virtual void __stdcall setListener(const Matrix4f& matrix)
    {
        setListener( matrix, zeroV );
    }
    virtual void __stdcall setListenerGain(float value)
    {
        alListenerf( AL_GAIN, value );
        assert( alGetError() == AL_NO_ERROR );
    }
    virtual float __stdcall getListenerGain(void)
    {
        ALfloat result;
        alGetListenerf( AL_GAIN, &result );
        assert( alGetError() == AL_NO_ERROR );
        return result;
    }
    virtual void __stdcall setSpeedOfSound(float value)
    {
        alSpeedOfSound( value );
        assert( alGetError() == AL_NO_ERROR );
    }
    virtual void __stdcall setDopplerFactor(float value)
    {
        alDopplerFactor( value );
        assert( alGetError() == AL_NO_ERROR );
    }
    virtual audio::ISound* __stdcall createStaticSound(const char* resourceName)
    {
        return new StaticSound( resourceName );
    }
    virtual audio::ISound* __stdcall createReverbSound(const char* resourceName, audio::Reverb* reverbs)
    {
        return new ReverbSound( resourceName, reverbs );
    }
    virtual audio::ISound* __stdcall createStreamSound(const char* resourceName, unsigned int numBuffers, unsigned int bufferSize)
    {
        assert( numBuffers > 1 );
        return new StreamSound( resourceName, numBuffers, bufferSize );
    }
};

SINGLE_ENTITY_COMPONENT(OpenALAudio);
