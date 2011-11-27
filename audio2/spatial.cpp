
#include "headers.h"
#include "spatial.h"
#include "errorreport.h"
#include "compability.h"
#include "../shared/mainwnd.h"
#include "../common/istring.h"

const Vector3f defaultDir( 0,1,0 );
const Vector3f defaultVel( 0,0,0 );

SpatialSound::SpatialSounds SpatialSound::_spatialSounds;

SpatialSound::SpatialSound()
{
    _lastListenerPos = _lastListenerVel = Vector3f( 0,0,0 );
    ZeroMemory( &_absoluteParameters, sizeof(DS3DBUFFER) );
    _absoluteParameters.dwSize = sizeof(DS3DBUFFER);
    _absoluteParameters.dwMode = DS3DMODE_DISABLE;
    _absoluteParameters.flMinDistance = DS3D_DEFAULTMINDISTANCE;
    _absoluteParameters.flMaxDistance = DS3D_DEFAULTMAXDISTANCE;
    _absoluteParameters.dwOutsideConeAngle = 360;
    _absoluteParameters.dwInsideConeAngle  = 360;
    _absoluteParameters.lConeOutsideVolume = 0;
    _absoluteParameters.vConeOrientation.x = 0;
    _absoluteParameters.vConeOrientation.y = 1;
    _absoluteParameters.vConeOrientation.z = 0;
    _relativeParameters = _absoluteParameters;

    // register instance of spatial sound
    _spatialSounds.push_back( this );
}

SpatialSound::~SpatialSound()
{
    for( SpatialSoundI spatialSoundI = _spatialSounds.begin();
                       spatialSoundI != _spatialSounds.end();
                       spatialSoundI++ )
    {
        if( *spatialSoundI == this )
        {
            _spatialSounds.erase( spatialSoundI );
            break;
        }
    }
}

void SpatialSound::onListener(const Vector3f& listenerPos, const Vector3f& listenerVel)
{
    _lastListenerPos = listenerPos * 0.01f;
    _lastListenerVel = listenerVel * 0.01f;
    IDirectSound3DBuffer8* iDirectSound3DBuffer8 = getIDirectSound3DBuffer8();
    if( iDirectSound3DBuffer8 )
    {
        // convert absolute spatial parameters to relative
        _relativeParameters = _absoluteParameters;
        _relativeParameters.vPosition.x -= listenerPos[0] * 0.01f;
        _relativeParameters.vPosition.y -= listenerPos[1] * 0.01f;
        _relativeParameters.vPosition.z -= listenerPos[2] * 0.01f;
        _relativeParameters.vVelocity.x -= listenerVel[0] * 0.01f;
        _relativeParameters.vVelocity.y -= listenerVel[1] * 0.01f;
        _relativeParameters.vVelocity.z -= listenerVel[2] * 0.01f;
        // update spatial parameters of sound
        DS3DBUFFER temp;
        ZeroMemory( &temp, sizeof(DS3DBUFFER) );
        temp.dwSize = sizeof(DS3DBUFFER);
        _dsCR( iDirectSound3DBuffer8->GetAllParameters( &temp ) );
        _dsCR( iDirectSound3DBuffer8->SetAllParameters( &_relativeParameters, DS3D_IMMEDIATE ) );
    }
}

void SpatialSound::setDistanceModel(float refDist, float maxDist, float rolloff)
{
    _absoluteParameters.dwMode = DS3DMODE_NORMAL;
    _absoluteParameters.flMinDistance = refDist * 0.01f;
    _absoluteParameters.flMaxDistance = maxDist * 0.01f;
}

void SpatialSound::place(const Vector3f& pos, const Vector3f& dir, float coneInnerAngle, float coneOuterAngle, float coneOuterGain, const Vector3f& vel)
{
    _absoluteParameters.dwMode = DS3DMODE_NORMAL;
    _absoluteParameters.vPosition.x = pos[0] * 0.01f;
    _absoluteParameters.vPosition.y = pos[1] * 0.01f;
    _absoluteParameters.vPosition.z = pos[2] * 0.01f;
    _absoluteParameters.vVelocity.x = vel[0] * 0.01f;
    _absoluteParameters.vVelocity.y = vel[1] * 0.01f;
    _absoluteParameters.vVelocity.z = vel[2] * 0.01f;
    Vector3f normalDir = dir; normalDir.normalize();
    _absoluteParameters.vConeOrientation.x = normalDir[0];
    _absoluteParameters.vConeOrientation.y = normalDir[1];
    _absoluteParameters.vConeOrientation.z = normalDir[2];
    _absoluteParameters.dwInsideConeAngle = static_cast<DWORD>( coneInnerAngle );
    _absoluteParameters.dwOutsideConeAngle = static_cast<DWORD>( coneOuterAngle );
    _absoluteParameters.lConeOutsideVolume = convertToDecibels( coneOuterGain );    
    onListener( _lastListenerPos * 100, _lastListenerVel * 100 );
}

void SpatialSound::place(const Vector3f& pos, const Vector3f& dir, float coneInnerAngle, float coneOuterAngle, float coneOuterGain)
{
    place( pos, dir, coneInnerAngle, coneOuterAngle, coneOuterGain, defaultVel );
}

void SpatialSound::place(const Vector3f& pos, const Vector3f& vel)
{
    place( pos, defaultDir, 360, 360, 1.0f, vel );
}

void SpatialSound::place(const Vector3f& pos)
{
    place( pos, defaultDir, 360, 360, 1.0f, defaultVel );
}

void SpatialSound::setReverberation(float inGain, float reverbMixDB, float reverbTime, float hfTimeRatio)
{
    IDirectSoundFXWavesReverb8* iDirectSoundFXWavesReverb8 = getIDirectSoundFXWavesReverb8();
    if( iDirectSoundFXWavesReverb8 )
    {
        DSFXWavesReverb parameters;

        // due to convertToDecibels() return value in hundreds of decibels,
        // we shall just constantly multiply this value by 0.01 to
        // obtains decibels
        parameters.fInGain = 0.01f * static_cast<float>( convertToDecibels( inGain ) );
        if( parameters.fInGain < DSFX_WAVESREVERB_INGAIN_MIN ) parameters.fInGain = DSFX_WAVESREVERB_INGAIN_MIN;
        if( parameters.fInGain > DSFX_WAVESREVERB_INGAIN_MAX ) parameters.fInGain = DSFX_WAVESREVERB_INGAIN_MAX;

        parameters.fReverbMix = 0.01f * static_cast<float>( convertToDecibels( reverbMixDB ) );
        if( parameters.fReverbMix < DSFX_WAVESREVERB_REVERBMIX_MIN ) parameters.fReverbMix = DSFX_WAVESREVERB_REVERBMIX_MIN;
        if( parameters.fReverbMix > DSFX_WAVESREVERB_REVERBMIX_MAX ) parameters.fReverbMix = DSFX_WAVESREVERB_REVERBMIX_MAX;

        parameters.fReverbTime = 1000 * reverbTime;
        if( parameters.fReverbTime < DSFX_WAVESREVERB_REVERBTIME_MIN ) parameters.fReverbTime = DSFX_WAVESREVERB_REVERBTIME_MIN;
        if( parameters.fReverbTime > DSFX_WAVESREVERB_REVERBTIME_MAX ) parameters.fReverbTime = DSFX_WAVESREVERB_REVERBTIME_MAX;

        parameters.fHighFreqRTRatio = hfTimeRatio;
        if( parameters.fHighFreqRTRatio < DSFX_WAVESREVERB_HIGHFREQRTRATIO_MIN ) parameters.fHighFreqRTRatio = DSFX_WAVESREVERB_HIGHFREQRTRATIO_MIN;
        if( parameters.fHighFreqRTRatio > DSFX_WAVESREVERB_HIGHFREQRTRATIO_MAX ) parameters.fHighFreqRTRatio = DSFX_WAVESREVERB_HIGHFREQRTRATIO_MAX;

        _dsCR( iDirectSoundFXWavesReverb8->SetAllParameters( &parameters ) );
    }
}

void SpatialSound::setListenerParameters(const Vector3f& listenerPos, const Vector3f& listenerVel)
{
    for( SpatialSoundI spatialSoundI = _spatialSounds.begin();
                       spatialSoundI != _spatialSounds.end();
                       spatialSoundI++ )
    {
        (*spatialSoundI)->onListener( listenerPos, listenerVel );
    }
}