
#ifndef ANIMATED_OBJECTS_INCLUDED
#define ANIMATED_OBJECTS_INCLUDED

#include "headers.h"
#include "scene.h"
#include "callback.h"

/**
 * troupe of animated object 
 */

struct AnimatedObjectDesc
{
public:
    engine::IClump* source;       // source for animated objects
    engine::IClump* map;          // each atomic of map will be replaced by copy of source
    float           animDistance; // distance limit for animation update
    float           animSpeed;    // animation speed
};

class AnimatedObject : public Actor
{
private:
    AnimatedObjectDesc _desc;
    callback::ClumpL   _objects;
protected:
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    AnimatedObject(Actor* parent, AnimatedObjectDesc* desc);
    virtual ~AnimatedObject();
};

#endif