
#include "headers.h"
#include "animation.h"
#include "fastquat.h"

/**
 * SRT animation support
 */

Animation::Animation(const char* name, unsigned int numKeys)
{
    assert( numKeys > 0 );

    _name    = name;
    _numKeys = numKeys;    
    _keys    = new SRT[numKeys];
    _period  = 0.0f;
    memset( _keys, 0, sizeof(SRT)*numKeys );
}

Animation* Animation::clone(void)
{
    Animation* result = new Animation( _name.c_str(), _numKeys );
    result->_period = _period;
    memcpy( result->_keys, _keys, sizeof(SRT)*_numKeys );
    return result;
}

Animation::~Animation(void)
{
    delete[] _keys;
}

void Animation::setKey(unsigned int kId, const SRT* srt)
{
    assert( kId>=0 && kId<_numKeys );
    assert( srt->time >= 0 );    
    _keys[kId] = *srt;
    _period = _period > srt->time ? _period : srt->time;
}

bool Animation::validateKeys(void)
{
    for( unsigned int i=0; i<_numKeys; i++ )
    {
        if( i && _keys[i].time <= _keys[i-1].time ) return false;
    }    
    return true;
}

unsigned int Animation::getSRT(float time, SRT* srt, unsigned int cacheId)
{
    if( _numKeys == 1 ) 
    {
        *srt = _keys[0];
        return 0;
    }

    // trim time value to animation period
    if( time > _period )
    {
        time = time - ( int( time / _period ) * _period );
    }

    // check boundary condition
    if( time == _keys[0].time ) 
    {
        *srt = _keys[0];
        return 0;
    }
    else if( time == _keys[_numKeys-1].time )
    {
        *srt = _keys[_numKeys-1];
        return _numKeys-2;
    }

    // search for animation key
    unsigned int kId;

    // sequential access using cache
    if( ( cacheId != 0xFFFFFFFF ) && ( _keys[cacheId].time <= time && _keys[cacheId+1].time > time ) )
    {
        kId = cacheId;
    }
    else if( ( cacheId != 0xFFFFFFFF ) && ( cacheId < _numKeys-2 ) && ( _keys[cacheId+1].time <= time && _keys[cacheId+2].time > time ) )
    {
        cacheId++;
        kId = cacheId;
    }
    else
    {
        // fast binary search for time specified
        unsigned int startId = 0;
        unsigned int endId   = _numKeys-1;
        do
        {
            kId = startId + (endId - startId)/2;
            if( ( _keys[kId].time <= time && _keys[kId+1].time > time ) ) break;
            if( _keys[kId].time > time ) endId = kId;
            else if( _keys[kId].time < time ) startId = kId;
        }
        while( true );
        cacheId = kId;
    }

    // linear interpolation    
    float interpolator = ( time - _keys[kId].time ) / ( _keys[kId+1].time - _keys[kId].time );
    D3DXVec3Lerp( &srt->scale, &_keys[kId].scale, &_keys[kId+1].scale, interpolator );
    D3DXVec3Lerp( &srt->translation, &_keys[kId].translation, &_keys[kId+1].translation, interpolator );
    D3DXQuaternionSlerp( &srt->rotation, &_keys[kId].rotation, &_keys[kId+1].rotation, interpolator );    

    // no interpolation (+5% performance)
    /*
    *srt = _keys[kId];
    */

    return cacheId;
}

/**
 * SRT animation set
 */

AnimationSet::AnimationSet(unsigned int numAnimations)
{
    _numReferences = 1;
    _numAnimations = numAnimations;
    _animations = new Animation*[_numAnimations];
    memset( _animations, 0, sizeof(Animation*)*_numAnimations );
}

AnimationSet::~AnimationSet()
{
    for( unsigned int i=0; i<_numAnimations; i++ )
    {
        delete _animations[i];
    }
    delete[] _animations;
}

void AnimationSet::release(void)
{
    _numReferences--;
    if( _numReferences == 1 ) delete this;
}

/**
 * animation track
 */

void AnimationController::Track::updateAbsoluteTime(void)
{
    switch( animation->loopType )
    {
    case engine::ltNone:
    {
        absoluteTime = animation->startTime + time;
        if( absoluteTime < animation->startTime ) absoluteTime = animation->startTime;
        if( absoluteTime > animation->endTime ) absoluteTime = animation->endTime;
        break;
    }
    case engine::ltPeriodic:
    {
        if( time <= animation->loopStartTime - animation->startTime )
        {
            absoluteTime = animation->startTime + time;
        }
        else
        {
            float period = animation->endTime - animation->loopStartTime;
            float periodicTime = time - (animation->loopStartTime - animation->startTime);
            float rangedTime = periodicTime - period * int( periodicTime / period );
            absoluteTime = animation->loopStartTime + rangedTime;
        }
        break;
    }
    case engine::ltMirror:
    {
        if( time <= animation->loopStartTime - animation->startTime )
        {
            absoluteTime = animation->startTime + time;
        }
        else
        {
            float period = animation->endTime - animation->loopStartTime;
            float periodicTime = time - (animation->loopStartTime - animation->startTime);
            int pquantity = int( periodicTime / period );
            float rangedTime = periodicTime - period * pquantity;
            if( pquantity % 2 == 0 )
            {
                absoluteTime = animation->loopStartTime + rangedTime;
            }
            else
            {
                absoluteTime = animation->endTime - rangedTime;
            }
        }
        break;
    }
    default:
        assert( !"shouldn't be here!" );
        absoluteTime = 0;
    }
}

/**
 * SRT animation controller
 */

AnimationController::AnimationController(Frame* hierarchy, AnimationSet* animationSet, Frame* hierarchyRoot)
{
    _hierarchy = hierarchy;
    _animationSet = animationSet;
    _animationSet->_numReferences++;
    _animationOutput = new Matrix*[_animationSet->getNumAnimations()];

    for( unsigned int i=0; i<_animationSet->getNumAnimations(); i++ )
    {
        Frame* targetFrame = hierarchyRoot->findFrame( _animationSet->getAnimationByIndex( i )->getName() );
        if( targetFrame )
        {
            _animationOutput[ i ] = &targetFrame->TransformationMatrix;
        }
    }

    memset( _track, 0, sizeof(Track) * engine::maxAnimationTracks );

    _defaultAnimation.startTime = 0;
    _defaultAnimation.endTime = _animationSet->getAnimationByIndex( 0 )->getPeriod();
    _defaultAnimation.loopStartTime = 0;
    _defaultAnimation.loopType = engine::ltPeriodic;

    setTrackAnimation( 0, &_defaultAnimation );
    setTrackSpeed( 0, 1.0f );
    setTrackWeight( 0, 1.0f );
    setTrackActivity( 0, true );

    _blendSrc = new SRT[_animationSet->getNumAnimations()];
    _blendDst = new SRT[_animationSet->getNumAnimations()];

    _activeTrack.reserve( engine::maxAnimationTracks );
}

AnimationController::~AnimationController()
{
    while( _weightSetMap.size() )
    {
        delete[] _weightSetMap.begin()->second;
        _weightSetMap.erase( _weightSetMap.begin() );
    }

    _animationSet->release();
    delete[] _blendDst;
    delete[] _blendSrc;
    delete[] _animationOutput;
}

/**
 * IAnimationController : track control
 */

engine::AnimSequence* AnimationController::getDefaultAnimation(void)
{
    return &_defaultAnimation;
}

bool AnimationController::isBeginOfAnimation(unsigned int trackId)
{
    assert( trackId>=0 && trackId<engine::maxAnimationTracks );
    _track[trackId].updateAbsoluteTime();
    return _track[trackId].animation->loopType == engine::ltNone && 
           _track[trackId].absoluteTime <= _track[trackId].animation->startTime;
}

bool AnimationController::isEndOfAnimation(unsigned int trackId)
{
    assert( trackId>=0 && trackId<engine::maxAnimationTracks );
    _track[trackId].updateAbsoluteTime();
    return _track[trackId].animation->loopType == engine::ltNone && 
           _track[trackId].absoluteTime >= _track[trackId].animation->endTime;
}

void AnimationController::resetTrackTime(unsigned int trackId)
{
    assert( trackId>=0 && trackId<engine::maxAnimationTracks );
    _track[trackId].time = 0;
}

bool AnimationController::getTrackActivity(unsigned int trackId)
{
    assert( trackId>=0 && trackId<engine::maxAnimationTracks );
    for( unsigned int i=0; i<_activeTrack.size(); i++ )
    {
        if( _activeTrack[i] == trackId ) return true;
    }
    return false;
}

void AnimationController::setTrackActivity(unsigned int trackId, bool flag)
{
    assert( trackId>=0 && trackId<engine::maxAnimationTracks );
    assert( _track[trackId].animation != NULL );

    if( flag )
    {
        // check track activity
        for( unsigned int i=0; i<_activeTrack.size(); i++ )
        {
            if( _activeTrack[i] == trackId ) return;
        }
        // add track to active track list
        _activeTrack.push_back( trackId );
    }
    else
    {
        // find track in active track list
        for( unsigned int i=0; i<_activeTrack.size(); i++ )
        {
            if( _activeTrack[i] == trackId )
            {
                _activeTrack.erase( _activeTrack.begin() + i );
                break;
            }
        }
    }
}

float AnimationController::getTrackSpeed(unsigned int trackId)
{
    assert( trackId>=0 && trackId<engine::maxAnimationTracks );
    return _track[trackId].speed;
}

void AnimationController::setTrackSpeed(unsigned int trackId, float speed)
{
    assert( trackId>=0 && trackId<engine::maxAnimationTracks );
    assert( speed>=0 );
    _track[trackId].speed = speed;
}

float AnimationController::getTrackWeight(unsigned int trackId)
{
    assert( trackId>=0 && trackId<engine::maxAnimationTracks );
    return _track[trackId].weight;
}

void AnimationController::setTrackWeight(unsigned int trackId, float weight)
{
    assert( trackId>=0 && trackId<engine::maxAnimationTracks );
    _track[trackId].weight = weight;
}

engine::AnimSequence* AnimationController::getTrackAnimation(unsigned int trackId)
{
    assert( trackId>=0 && trackId<engine::maxAnimationTracks );
    return _track[trackId].animation;
}

void AnimationController::setTrackAnimation(unsigned int trackId, engine::AnimSequence* sequence)
{
    assert( trackId>=0 && trackId<engine::maxAnimationTracks );
    _track[trackId].animation = sequence;
}

void AnimationController::copyTrack(unsigned int srcTrackId, unsigned int dstTrackId)
{
    assert( srcTrackId>=0 && srcTrackId<engine::maxAnimationTracks );
    assert( dstTrackId>=0 && dstTrackId<engine::maxAnimationTracks );
    _track[dstTrackId] = _track[srcTrackId];
}

inline void slerpQuaternions(SRT* dst, SRT* src, engine::WeightSet* weightSet, std::vector<unsigned int>& activeTrack)
{   
    SRT temp;
    float weight1 = weightSet->weight[activeTrack[0]];
    float weight2;
    float weightSum;
    unsigned int j;
    DWORD* testZero = reinterpret_cast<DWORD*>( &weight2 );
    dst->rotation = src[activeTrack[0]].rotation;
    for( unsigned int i=1; i<activeTrack.size(); i++ )
    {
        j = activeTrack[i];
        weight2 = weightSet->weight[j];
        if( (*testZero) )
        {
            weightSum = weight1 + weight2;        
            weight2 /= ( weightSum>0 ) ? ( weight1 + weight2 ) : 1.0f;        
            D3DXQuaternionSlerp( &temp.rotation, &dst->rotation, &src[j].rotation, weight2 );
            dst->rotation = temp.rotation;
            weight1 += weightSet->weight[j];
        }        
    }
}

void AnimationController::advance(float dt)
{
    if( _activeTrack.size() == 0 ) 
    {
        return;
    }
    else if( _activeTrack.size() == 1 )
    {
        advanceSingleTrack( dt );
    }
    else
    {
        advanceMultipleTracks( dt );
    }
    _hierarchy->dirty();
}

void AnimationController::advanceSingleTrack(float dt)
{
    unsigned int i;
    Matrix s,r,rs,t;

    unsigned int activeTrack = _activeTrack[0];

    // advance time for active track
    _track[activeTrack].time += dt * _track[activeTrack].speed;
    _track[activeTrack].updateAbsoluteTime();    

    // mix animation tracks
    for( i=0; i<_animationSet->getNumAnimations(); i++ )
    {
        // calculate SRT values for active track
        _track[activeTrack].cacheId = _animationSet->getAnimationByIndex( i )->getSRT( 
            _track[activeTrack].absoluteTime, 
            &_mixerDst,
            _track[activeTrack].cacheId
        );

        // calculate final transformation matrix from mixer destination SRT
        D3DXMatrixRotationQuaternion( &r, &_mixerDst.rotation );
        D3DXMatrixScaling( &s, _mixerDst.scale.x, _mixerDst.scale.y, _mixerDst.scale.z );
        D3DXMatrixTranslation( &t, _mixerDst.translation.x, _mixerDst.translation.y, _mixerDst.translation.z );
        dxMultiply( &rs, &s, &r );
        dxMultiply( _animationOutput[i], &rs, &t );
    }
}

void AnimationController::advanceMultipleTracks(float dt)
{    
    unsigned int i,j,k;
    Matrix s,r,rs,t;

    unsigned int numActiveTracks = _activeTrack.size();

    // advance time for active tracks
    for( j=0; j<numActiveTracks; j++ )
    {
        _track[_activeTrack[j]].time += dt * _track[_activeTrack[j]].speed;
        _track[_activeTrack[j]].updateAbsoluteTime();
    }

    engine::WeightSet weightSet;

    // mix animation tracks
    for( i=0; i<_animationSet->getNumAnimations(); i++ )
    {
        // calculate SRT values for active tracks
        for( j=0; j<numActiveTracks; j++ ) 
        {
            k = _activeTrack[j]; 
            _track[k].cacheId = _animationSet->getAnimationByIndex( i )->getSRT( 
                _track[k].absoluteTime, 
                _mixerSrc+k,
                _track[k].cacheId
            );
        }

        // reset destination SRT
        memset( &_mixerDst, 0, sizeof(SRT) );

        // calculate weights taking into account a weight set
        float weightSum = 0.0f;
        for( j=0; j<numActiveTracks; j++ )
        {
            k = _activeTrack[j];
            weightSet.weight[k] = _track[k].weight;
            weightSum += weightSet.weight[k];
        }
        if( _activeWeightSetI != NULL )
        {
            weightSum = 0.0f;
            for( j=0; j<numActiveTracks; j++ )
            {
                k = _activeTrack[j];
                weightSet.weight[k] *= _activeWeightSetI->second[i].weight[k];
                weightSum += weightSet.weight[k];
            }
        }

        if( weightSum == 0 ) continue;

        // rescale weights to achieve a weight sum of 1.0f
        for( j=0; j<numActiveTracks; j++ )
        {
            k = _activeTrack[j];
            weightSet.weight[k] *= 1.0f / weightSum; 
        }

        // mix scale and translational components
        for( j=0; j<numActiveTracks; j++ )
        {
            k = _activeTrack[j];
            _mixerDst.scale += _mixerSrc[k].scale * weightSet.weight[k];
            _mixerDst.translation += _mixerSrc[k].translation * weightSet.weight[k];
        }
        
        // mix rotational components
        slerpQuaternions( &_mixerDst, _mixerSrc, &weightSet, _activeTrack );

        // calculate final transformation matrix from mixer destination SRT
        D3DXMatrixRotationQuaternion( &r, &_mixerDst.rotation );
        D3DXMatrixScaling( &s, _mixerDst.scale.x, _mixerDst.scale.y, _mixerDst.scale.z );
        D3DXMatrixTranslation( &t, _mixerDst.translation.x, _mixerDst.translation.y, _mixerDst.translation.z );
        dxMultiply( &rs, &s, &r );
        dxMultiply( _animationOutput[i], &rs, &t );
    }
}

/**
 * IAnimationController : animation blender
 */

void AnimationController::captureBlendSrc(void)
{
    SRT* srt;
    Matrix pureRotation;    

    for( unsigned int i=0; i<_animationSet->getNumAnimations(); i++ )
    {
        srt = _blendSrc + i;
        srt->scale.x = D3DXVec3Length( &dxRight( _animationOutput[i] ) );
        srt->scale.y = D3DXVec3Length( &dxUp( _animationOutput[i] ) );
        srt->scale.z = D3DXVec3Length( &dxAt( _animationOutput[i] ) );
        srt->translation = dxPos( _animationOutput[i] );
        pureRotation = *_animationOutput[i];
        pureRotation._11 /= srt->scale.x, 
        pureRotation._12 /= srt->scale.x, 
        pureRotation._13 /= srt->scale.x, 
        pureRotation._14 = 0.0f;
        pureRotation._21 /= srt->scale.y, 
        pureRotation._22 /= srt->scale.y, 
        pureRotation._23 /= srt->scale.y, 
        pureRotation._24 = 0.0f;
        pureRotation._31 /= srt->scale.z, 
        pureRotation._32 /= srt->scale.z, 
        pureRotation._33 /= srt->scale.z, 
        pureRotation._34 = 0.0f;
        pureRotation._41 = 0.0f, 
        pureRotation._42 = 0.0f, 
        pureRotation._43 = 0.0f, 
        pureRotation._44 = 1.0f;
        D3DXQuaternionRotationMatrix( &srt->rotation, &pureRotation );
        D3DXQuaternionNormalize( &srt->rotation, &srt->rotation );
    }
}

void AnimationController::captureBlendDst(void)
{
    SRT* srt;
    Matrix pureRotation;    

    for( unsigned int i=0; i<_animationSet->getNumAnimations(); i++ )
    {
        srt = _blendDst + i;
        srt->scale.x = D3DXVec3Length( &dxRight( _animationOutput[i] ) );
        srt->scale.y = D3DXVec3Length( &dxUp( _animationOutput[i] ) );
        srt->scale.z = D3DXVec3Length( &dxAt( _animationOutput[i] ) );
        srt->translation = dxPos( _animationOutput[i] );
        pureRotation = *_animationOutput[i];
        pureRotation._11 /= srt->scale.x, 
        pureRotation._12 /= srt->scale.x, 
        pureRotation._13 /= srt->scale.x, 
        pureRotation._14 = 0.0f;
        pureRotation._21 /= srt->scale.y, 
        pureRotation._22 /= srt->scale.y, 
        pureRotation._23 /= srt->scale.y, 
        pureRotation._24 = 0.0f;
        pureRotation._31 /= srt->scale.z, 
        pureRotation._32 /= srt->scale.z, 
        pureRotation._33 /= srt->scale.z, 
        pureRotation._34 = 0.0f;
        pureRotation._41 = 0.0f, 
        pureRotation._42 = 0.0f, 
        pureRotation._43 = 0.0f, 
        pureRotation._44 = 1.0f;
        D3DXQuaternionRotationMatrix( &srt->rotation, &pureRotation );
        D3DXQuaternionNormalize( &srt->rotation, &srt->rotation );
    }
}

void AnimationController::blend(float interpolator)
{
    Matrix r,s,rs,t;
    SRT    srt;

    // trim interpolator range
    if( interpolator < 0.0f ) interpolator = 0.0f;
    if( interpolator > 1.0f ) interpolator = 1.0f;

    for( unsigned int i=0; i<_animationSet->getNumAnimations(); i++ )
    {
        D3DXVec3Lerp( &srt.scale, &_blendSrc[i].scale, &_blendDst[i].scale, interpolator );        
        D3DXVec3Lerp( &srt.translation, &_blendSrc[i].translation, &_blendDst[i].translation, interpolator );
        D3DXQuaternionSlerp( &srt.rotation, &_blendSrc[i].rotation, &_blendDst[i].rotation, interpolator );

        D3DXMatrixRotationQuaternion( &r, &srt.rotation );
        D3DXMatrixScaling( &s, srt.scale.x, srt.scale.y, srt.scale.z );
        D3DXMatrixTranslation( &t, srt.translation.x, srt.translation.y, srt.translation.z );
        dxMultiply( &rs, &s, &r );
        dxMultiply( _animationOutput[i], &rs, &t );
    }

    _hierarchy->dirty();
}

/**
 * weight set manipulation
 */

unsigned int AnimationController::getNumAnimations(void)
{
    return _animationSet->getNumAnimations();
}

void AnimationController::createWeightSet(const char* weightSetName)
{
    engine::WeightSet* weightSet = new engine::WeightSet[_animationSet->getNumAnimations()];
    memset( weightSet, 0, sizeof(engine::WeightSet) * _animationSet->getNumAnimations() );
    _weightSetMap.insert( WeightSetT( weightSetName, weightSet ) );
}

void AnimationController::enableWeightSet(const char* weightSetName)
{
    _activeWeightSetI = _weightSetMap.find( weightSetName );
    assert( _activeWeightSetI != _weightSetMap.end() );
    if( _activeWeightSetI == _weightSetMap.end() ) _activeWeightSetI = NULL;
}

void AnimationController::disableWeightSet(void)
{
    _activeWeightSetI = NULL;
}

engine::WeightSet* AnimationController::getWeightSet(const char* weightSetName)
{
    WeightSetI weightSetI = _weightSetMap.find( weightSetName ); 
    assert( weightSetI != _weightSetMap.end() );
    return weightSetI->second;
}   

engine::WeightSet* AnimationController::getAnimationWeightSet(const char* weightSetName, const char* animationName)
{
    WeightSetI weightSetI = _weightSetMap.find( weightSetName ); 
    assert( weightSetI != _weightSetMap.end() );
    unsigned int animationIndex = _animationSet->getAnimationIndexByName( animationName ); 
    assert( animationIndex != 0xFFFFFFFF );
    return weightSetI->second + animationIndex;
}

void AnimationController::forAllAnimationWeightSets(const char* weightSetName, engine::WeightSetCallBack callBack, void* data)
{
    WeightSetI weightSetI = _weightSetMap.find( weightSetName ); 
    assert( weightSetI != _weightSetMap.end() );    

    for( unsigned int i=0; i<_animationSet->getNumAnimations(); i++ )
    {
        callBack( 
            _animationSet->getAnimationByIndex( i )->getName(),
            weightSetI->second + i,
            data
        );
    }
}