
#ifndef DEMOVERSION_ACTIVITY_INCLUDED
#define DEMOVERSION_ACTIVITY_INCLUDED

#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/engine.h"
#include "../shared/gui.h"

#include "activity.h"
#include "career.h"
#include "geoscape.h"

using namespace ccor;

/**
 * demo activity
 */

class Demo : public Activity
{
private:
    bool      _endOfActivity;
    Career*   _career;
    Location* _location;
protected:
    // class implementation
    virtual ~Demo();
private:
    // message handling
    static void messageCallback(gui::Message* message, void* userData);
public:
    // class implementation
    Demo(Career* career);
public:
    // Activity
    virtual void updateActivity(float dt);
    virtual bool endOfActivity(void);
    virtual void onBecomeActive(void);
    virtual void onBecomeInactive(void);
    virtual void onReturnFromActivity(Activity* activity);
};

#endif