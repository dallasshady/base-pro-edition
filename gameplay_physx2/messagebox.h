
#ifndef MESSAGEBOX_ACTIVITY_INCLUDED
#define MESSAGEBOX_ACTIVITY_INCLUDED

#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/engine.h"
#include "../shared/gui.h"

#include "activity.h"

using namespace ccor;

class Messagebox : public Activity
{
private:    
    gui::IGuiWindow* _window;
    bool             _endOfActivity;
protected:
    virtual ~Messagebox();
private:
    static void messageCallback(gui::Message* message, void* userData);
public:
    Messagebox(const wchar_t* message);
public:
    virtual void updateActivity(float dt);
    virtual bool endOfActivity(void);
    virtual void onBecomeActive(void);
    virtual void onBecomeInactive(void);
};

#endif