
#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/product_version.h"
#include "../shared/audio.h"
#include "../shared/mainwnd.h"
#include "../shared/input.h"
#include "../common/istring.h"

#include "errorreport.h"
#include "static.h"
#include "stream.h"
#include "silence.h"

using namespace ccor;

/**
 * IAudio implementation
 */

const Vector3f zeroV( 0,0,0 );

audio::ISound* testSound = NULL;
input::IInputDevice* testInput = NULL;
Vector3f soundPos(0,0,0);

class Audio : public EntityBase, 
              virtual public audio::IAudio
{
public:
    typedef std::pair<std::string,LPGUID> DirectSoundDevice;
    typedef std::vector<DirectSoundDevice> DirectSoundDevices;
private:
    DirectSoundDevices       devices;
    IDirectSound8*           iDirectSound8;
    IDirectSound3DListener8* iDirectSoundListener8;
public:
    Audio() : iDirectSound8(NULL), iDirectSoundListener8(NULL)
    {
    } 
    ~Audio() 
    {
        if( testSound ) testSound->release();
        // release sound device
        if( iDirectSound8 ) iDirectSound8->Release();
        // reset COM 
        CoUninitialize();
    }
public:
    static BOOL CALLBACK directSoundEnumerationCallback(
        LPGUID  lpGuid,    
        LPCSTR  lpcstrDescription,  
        LPCSTR  lpcstrModule,   
        LPVOID  lpContext
    )
    {
        Audio* __this = reinterpret_cast<Audio*>( lpContext );
        __this->devices.push_back( DirectSoundDevice( lpcstrDescription, lpGuid ) );
        return TRUE;
    }
public:
    virtual void __stdcall entityInit(Object * p) 
    {
        // enumerate sound devices
        DirectSoundEnumerate( directSoundEnumerationCallback, this );
        assert( devices.size() );

        // create default DirectSound device
        _dsCR( DirectSoundCreate8( 
            devices[0].second, 
            &iDirectSound8,
            NULL
        ) );

        // setup cooperative level of sound library
        mainwnd::IMainWnd* iMainWnd;
        queryInterface( "MainWnd", &iMainWnd );
        _dsCR( iDirectSound8->SetCooperativeLevel(
            HWND( iMainWnd->getHandle() ),
            DSSCL_NORMAL
        ) );

        // obtain primary sound buffer
        IDirectSoundBuffer* primaryBuffer = NULL;
        DSBUFFERDESC        primaryBufferDesc;
        ZeroMemory( &primaryBufferDesc, sizeof(DSBUFFERDESC) );
        primaryBufferDesc.dwSize = sizeof(DSBUFFERDESC);
        primaryBufferDesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
        _dsCR( iDirectSound8->CreateSoundBuffer( &primaryBufferDesc, &primaryBuffer, NULL ) );

        // obtain 3D listener interface from primary buffer
        _dsCR( primaryBuffer->QueryInterface( 
            IID_IDirectSound3DListener, 
            (VOID**)(&iDirectSoundListener8)
        ) );

        // release primary buffer interface
        primaryBuffer->Release();

        // setup listener
        _dsCR( iDirectSoundListener8->SetDistanceFactor(
            1.0f,
            DS3D_IMMEDIATE
        ) );
        _dsCR( iDirectSoundListener8->SetDopplerFactor( 
            3.0f,
            DS3D_IMMEDIATE
        ) );
        _dsCR( iDirectSoundListener8->SetRolloffFactor( 
            1.0f,
            DS3D_IMMEDIATE
        ) );

        // obtain device capabilities
        DSCAPS dsCaps;
        ZeroMemory( &dsCaps, sizeof(DSCAPS) );
        dsCaps.dwSize = sizeof(DSCAPS);
        _dsCR( iDirectSound8->GetCaps( &dsCaps ) );

        // setup frequency mixing capabilities
        StaticSound::minSecondarySampleRate = dsCaps.dwMinSecondarySampleRate;
        StaticSound::maxSecondarySampleRate = dsCaps.dwMaxSecondarySampleRate;
        StreamSound::minSecondarySampleRate = dsCaps.dwMinSecondarySampleRate;
        StreamSound::maxSecondarySampleRate = dsCaps.dwMaxSecondarySampleRate;

        CoInitialize( NULL );
    }
    virtual void __stdcall entityAct(float dt) 
    {
        StreamSound::updateStreamBuffers();

        /*if( !testSound )
        {
            testSound = createStreamSound( "./res/sounds/cry/cry01.ogg", 4, 64 * 1024 );
            testSound->setPitchShift( 1.0f );
            testSound->setLoop( false );
            testSound->setGain( 0.5f );
            testSound->play();
        }*/
    }
    virtual IBase* __stdcall entityAskInterface(iid_t id) 
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
        Vector3f normalUp = up; normalUp.normalize();
        Vector3f normalAt = at; normalAt.normalize();
        _dsCR( iDirectSoundListener8->SetOrientation(
            normalAt[0], normalAt[1], normalAt[2],
            normalUp[0], normalUp[1], normalUp[2],
            DS3D_IMMEDIATE 
        ) );

        SpatialSound::setListenerParameters( pos, vel );
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
        // not supported
    }
    virtual float __stdcall getListenerGain(void)
    {
        return 1.0f;
    }
    virtual void __stdcall setSpeedOfSound(float value)
    {
        // not supported
    }
    virtual void __stdcall setDopplerFactor(float value)
    {
        _dsCR( iDirectSoundListener8->SetDopplerFactor( 
            value,
            DS3D_IMMEDIATE
        ) );
    }
    virtual audio::ISound* __stdcall createStaticSound(const char* resourceName)
    {
        return new StaticSound( resourceName, iDirectSound8 );
    }
    virtual audio::ISound* __stdcall createReverbSound(const char* resourceName, audio::Reverb* reverbs)
    {
        return new StaticSound( resourceName, iDirectSound8 );
    }
    virtual audio::ISound* __stdcall createStreamSound(const char* resourceName, unsigned int numBuffers, unsigned int bufferSize)
    {
        //return new Silence();
        //return new StaticSound( resourceName, iDirectSound8 );
        return new StreamSound( resourceName, iDirectSound8, numBuffers, bufferSize );
    }
    virtual void __stdcall updateStreamSounds(void)
    {
        StreamSound::updateStreamBuffers();
    }
};

SINGLE_ENTITY_COMPONENT(Audio);
