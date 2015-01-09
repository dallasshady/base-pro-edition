
#ifndef ABSTRACT_CHARACTER_INCLUDED
#define ABSTRACT_CHARACTER_INCLUDED

#include "headers.h"
#include "scene.h"

/**
 * action analyser
 */

#define actionIs(A) ( NULL != dynamic_cast<A*>( _action ) )

/**
 * character are actors, that can perform some actions
 */

class Character : public Actor
{
protected:
    /**
     * abstract action
     */
    class Action
    {
    protected:
        engine::IClump* _clump;
        bool            _endOfAction;
        float           _blendTime;
        float           _actionTime;
    protected:
        void updateAnimation(float dt);
    public:
        Action(engine::IClump* clump) : _clump(clump), _endOfAction(false), _blendTime(0.0f), _actionTime(0.0f) {}
        virtual ~Action() {}
    public:
        virtual void update(float dt) = 0;
        virtual void updatePhysics(void) = 0;
    public:
        // some actions has certain damage factor (multiplier)
        virtual float getDamageFactor(void) { return 1.0f; }
    public:
        inline bool isEndOfAction(void) { return _endOfAction; }
        inline float getActionTime(void) { return _actionTime; }
        inline float getBlendTime(void) { return _blendTime; }
    };
    /**
     * idle action (playing given animation sequence)
     */
    class Idle : public Action
    {
    public:
        // class implementation
        Idle(engine::IClump* clump, engine::AnimSequence* sequence, float blendTime, float animSpeed);
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void) {}
    };
protected:
    engine::IClump* _clump;
    Action*         _action;
    bool            _active;
public:
    // actor abstracts
    virtual void onUpdateActivity(float dt);
    virtual void onUpdatePhysics(void);
    virtual Matrix4f getPose(void) { return _clump->getFrame()->getLTM(); }
public:
    // class implementation
    Character(Actor* parent, engine::IClump* clump);
    virtual ~Character();
public:
    // inlines
    inline engine::IClump* getClump(void) { return _clump; }
};

#endif