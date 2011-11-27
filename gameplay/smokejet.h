
#ifndef SMOKEJET_ACTORS_INCLUDED
#define SMOKEJET_ACTORS_INCLUDED

#include "headers.h"
#include "scene.h"
#include "jumper.h"

enum SmokeJetMode
{
    sjmLeft,
    sjmRight
};

class SmokeJet : public Actor
{
private:
    SmokeJetMode             _mode;
    Jumper*                  _jumper;
    engine::SmokeTrailScheme _scheme;
    engine::IShader*         _shader;
    engine::IRendering*      _smokeTrail;
    bool                     _enabled;
public:
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    SmokeJet(Jumper* jumper, Vector4f color, SmokeJetMode mode);
    virtual ~SmokeJet();
public:
    // class behaviour
    bool isEnabled(void);
    void enable(bool flag);
    void setColor(Vector4f color);
};

#endif