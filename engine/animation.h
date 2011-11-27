/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description animation routine
 *
 * @author bad3p
 */

#pragma once

#include "headers.h"
#include "engine.h"
#include "frame.h"

/**
 * STR animation
 */

struct SRT
{
public:
    float      time;
    Vector     scale;
    Quaternion rotation;
    Vector     translation;
};

class Animation
{
private:
    std::string  _name;
    unsigned int _numKeys;
    SRT*         _keys;
    float        _period;
public:
    inline const char* getName(void) { return _name.c_str(); }
    inline float getPeriod(void) { return _period; }
    inline SRT* getKey(unsigned int kId) 
    {
        assert( kId>=0 && kId<_numKeys );
        return _keys+kId;
    }
public:
    Animation(const char* name, unsigned int numKeys);
    virtual ~Animation(void);
public:
    Animation* clone(void);
    void setKey(unsigned int kId, const SRT* srt);    
    bool validateKeys(void);
    unsigned int getSRT(float time, SRT* srt, unsigned int cacheId=0xFFFFFFFF);
};

/**
 * SRT animation set
 */

class AnimationSet
{
private:
    friend class AnimationController;
private:
    unsigned int _numReferences;
    unsigned int _numAnimations;
    Animation**  _animations;
private:
    virtual ~AnimationSet();
public:
    AnimationSet(unsigned int numAnimations);    
    void release(void);
public:
    inline unsigned int getNumAnimations(void)
    {
        return _numAnimations;
    }
    inline Animation* getAnimationByIndex(unsigned int animId)
    {
        assert( animId>=0 && animId<_numAnimations );
        return _animations[animId];
    }
    inline Animation* getAnimationByName(const char* animName)
    {
        for( unsigned int i=0; i<_numAnimations; i++ )
        {
            if( strcmp( _animations[i]->getName(), animName ) == 0 ) return _animations[i];
        }
        assert( !"getAnimationByName() : animation not found!" );
        return NULL;
    }
    inline unsigned int getAnimationIndexByName(const char* animName)
    {
        for( unsigned int i=0; i<_numAnimations; i++ )
        {
            if( strcmp( _animations[i]->getName(), animName ) == 0 ) return i;
        }
        assert( !"getAnimationIndexByName() : animation not found!" );
        return 0xFFFFFFFF;
    }
    inline void setAnimation(unsigned int animId, Animation* animation)
    {
        assert( animId>=0 && animId<_numAnimations );
        _animations[animId] = animation;
    }
};

/**
 * SRT animation controller
 */

class AnimationController : public engine::IAnimationController
{   
private:
    struct Track
    {
    public:
        float                 time;         // track local time
        float                 speed;        // track speed
        float                 weight;       // track weight
        engine::AnimSequence* animation;    // specified animation sequence
        float                 absoluteTime; // track absolute time
        unsigned int          cacheId;      // cache for SRT search in animation
    public:
        Track() : time(0), speed(0), weight(0), animation(NULL), absoluteTime(0), cacheId(0xFFFFFFFF) {}
        void updateAbsoluteTime(void);
    };
private:
    /**
     * weight set
     */
    typedef std::pair<std::string, engine::WeightSet*> WeightSetT;
    typedef std::map<std::string, engine::WeightSet*> WeightSetMap;
    typedef WeightSetMap::iterator WeightSetI;
private:
    Frame*                    _hierarchy;
    AnimationSet*             _animationSet;
    Matrix**                  _animationOutput;
    engine::AnimSequence      _defaultAnimation;
    Track                     _track[engine::maxAnimationTracks];
    std::vector<unsigned int> _activeTrack;    
    WeightSetMap              _weightSetMap;
    WeightSetI                _activeWeightSetI;
    SRT                       _mixerSrc[engine::maxAnimationTracks];
    SRT                       _mixerDst;
    SRT*                      _blendSrc;
    SRT*                      _blendDst;
private:
    void advanceSingleTrack(float dt);
    void advanceMultipleTracks(float dt);
public:
    // class implementation
    AnimationController(Frame* hierarchy, AnimationSet* animationSet, Frame* hierarchyRoot);
    virtual ~AnimationController();
public:
    // IAnimationController
    virtual engine::AnimSequence* __stdcall getDefaultAnimation(void);
    virtual bool __stdcall isBeginOfAnimation(unsigned int trackId);
    virtual bool __stdcall isEndOfAnimation(unsigned int trackId);
    virtual void __stdcall resetTrackTime(unsigned int trackId);
    virtual bool __stdcall getTrackActivity(unsigned int trackId);
    virtual void __stdcall setTrackActivity(unsigned int trackId, bool flag);
    virtual float __stdcall getTrackSpeed(unsigned int trackId);
    virtual void __stdcall setTrackSpeed(unsigned int trackId, float speed);
    virtual float __stdcall getTrackWeight(unsigned int trackId);
    virtual void __stdcall setTrackWeight(unsigned int trackId, float weight);
    virtual engine::AnimSequence* __stdcall getTrackAnimation(unsigned int trackId);
    virtual void __stdcall setTrackAnimation(unsigned int trackId, engine::AnimSequence* sequence);
    virtual void __stdcall copyTrack(unsigned int srcTrackId, unsigned int dstTrackId);
    virtual void __stdcall advance(float dt);
    virtual void __stdcall captureBlendSrc(void);
    virtual void __stdcall captureBlendDst(void);
    virtual void __stdcall blend(float interpolator);
    // IAnimationController : weight set manipulation
    virtual unsigned int __stdcall getNumAnimations(void);
    virtual void __stdcall createWeightSet(const char* weightSetName);
    virtual void __stdcall enableWeightSet(const char* weightSetName);
    virtual void __stdcall disableWeightSet(void);
    virtual engine::WeightSet* __stdcall getWeightSet(const char* weightSetName);
    virtual engine::WeightSet* __stdcall getAnimationWeightSet(const char* weightSetName, const char* animationName);
    virtual void __stdcall forAllAnimationWeightSets(const char* weightSetName, engine::WeightSetCallBack callBack, void* data);
public:
    // module local : inlines
    inline AnimationSet* getAnimationSet(void) { return _animationSet; }
};