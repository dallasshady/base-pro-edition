
#ifndef SMOKEBALL_ACTORS_INCLUDED
#define SMOKEBALL_ACTORS_INCLUDED

#include "headers.h"
#include "scene.h"

struct SmokeBallDesc
{
public:
    Vector3f     center;         // ball center
    float        radius;         // ball radius
    Vector4f     color;          // ball color
    unsigned int numParticles;   // number of particles
    float        particleMass;   // mass of particle
    float        particleRadius; // radius of particle
public:
    SmokeBallDesc()
    {
        center.set( 0,0,0 );
        radius = 0.0f;
        color.set( 1,1,1,1 );
        numParticles = 0;
        particleMass = 1.0f;
    }
};

class SmokeBall : public Actor
{
private:
    SmokeBallDesc            _desc;
    engine::IShader*         _shader;
    engine::IParticleSystem* _particleSystem;
    Vector3f                 _sphereCenter;
    float                    _sphereRadius;
    bool                     _userFlag;    
public:
    // class behaviour
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    SmokeBall(Actor* parent, SmokeBallDesc* desc);
    virtual ~SmokeBall();
public:
    // class behaviour
    bool isInside(Vector3f point);
    bool getUserFlag(void);
    void setUserFlag(bool value);
    SmokeBallDesc* getDescriptor(void);    
};

#endif