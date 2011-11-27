
#ifndef CROWD_ACTORS_INCLUDED
#define CROWD_ACTORS_INCLUDED

#include "headers.h"
#include "scene.h"
#include "character.h"
#include "callback.h"
#include "sensor.h"

/**
 * crowd troupe
 */

struct CrowdDesc
{
public:
    engine::IClump* extras;
    unsigned int    numActors;
    unsigned int    numWalkingActors;
};

class Crowd : public Actor
{
private:
    CrowdDesc    _desc;
    Enclosure*   _enclosure;
    unsigned int _numWalkingActors;
protected:
    // actor abstracts
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    Crowd(Actor* parent, CrowdDesc* crowdDesc);
    virtual ~Crowd();
    // class behaviour
    bool beginWalk(void);
    void endWalk(void);
};

/**
 * crowd spectator
 */

class Spectator : public Character
{
private:
    /**
     * turn action
     */
    class Turn : public Character::Action
    {
    private:
        Vector3f _dir;
    public:
        // class implementation
        Turn(engine::IClump* clump, Vector3f dir);
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void) {}
    };
    /**
     * move action
     */
    class Move : public Character::Action
    {
    private:
        Vector3f   _pos;
        Enclosure* _enclosure;
        bool       _fast;
    public:
        // class implementation
        Move(engine::IClump* clump, Enclosure* enclosure, const Vector3f& pos, bool fast);
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void) {}
    };
private:
    enum Wish
    {
        wishRelax,
        wishGoto,
        wishWatch
    };
private:
    Enclosure*        _enclosure;
    engine::ITexture* _texture;
    Wish              _wish;
    bool              _endOfWish;
    float             _relaxTime;
    Vector3f          _gotoPos;
    float             _gotoError;
    float             _watchDelay;
    float             _watchTime;
    Vector3f          _watchPos;
protected:
    // atomic rendering
    static engine::IAtomic* onRenderAtomic(engine::IAtomic* atomic, void* data);
    // actor abstracts
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    Spectator(Crowd* crowd, engine::IClump* cloneSource, Enclosure* enclosure);
    virtual ~Spectator();
};

#endif