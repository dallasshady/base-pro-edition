
#ifndef DELETECAREER_ACTIVITY_INCLUDED
#define DELETECAREER_ACTIVITY_INCLUDED

#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/engine.h"
#include "../shared/gui.h"

#include "activity.h"
#include "career.h"

using namespace ccor;

class DeleteCareer : public Activity
{
private:
    Career*          _career;
    gui::IGuiWindow* _window;
    bool             _endOfActivity;
protected:
    virtual ~DeleteCareer();
private:
    static void messageCallback(gui::Message* message, void* userData);
public:
    DeleteCareer(Career* career);
public:
    virtual void updateActivity(float dt);
    virtual bool endOfActivity(void);
    virtual void onBecomeActive(void);
    virtual void onBecomeInactive(void);
};

#endif