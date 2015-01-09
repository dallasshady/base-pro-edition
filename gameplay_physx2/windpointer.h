
#ifndef WIND_POINTER_INCLUDED
#define WIND_POINTER_INCLUDED

#include "headers.h"
#include "scene.h"

/**
 * wind pointer actor
 */

class WindPointer : public Actor
{
private:
    gui::IGuiWindow* _signature;
    gui::IGuiPanel*  _windSpeed;
public:
    // actor abstracts
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
    virtual void onUpdateActivity(float dt);
public:
    WindPointer(Actor* parent);
    virtual ~WindPointer();
};

#endif