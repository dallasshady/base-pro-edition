
#ifndef TRAFFIC_ACTORS_INCLUDED
#define TRAFFIC_ACTORS_INCLUDED

#include "headers.h"
#include "scene.h"

/**
 * traffic actor
 */

struct TrafficDesc
{
public:
    engine::IClump* source;
    float           animSpeed;
    float           animOffset;
};

class Traffic : public Actor
{
private:
    TrafficDesc     _desc;
    engine::IClump* _clump;
protected:
    // actor abstracts
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    Traffic(Actor* parent, TrafficDesc* desc);
    virtual ~Traffic();
};

#endif