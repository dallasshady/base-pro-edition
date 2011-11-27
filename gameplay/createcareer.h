
#ifndef CREATECAREER_ACTIVITY_INCLUDED
#define CREATECAREER_ACTIVITY_INCLUDED

#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/engine.h"
#include "../shared/gui.h"

#include "activity.h"
#include "career.h"
#include "virtues.h"
#include "database.h"

using namespace ccor;

class CreateCareer : public Activity
{
private:
    bool                 _endOfActivity;
    Career*              _career;
    gui::IGuiWindow*     _window;
    gui::IGuiEdit*       _heightBox;
    gui::IGuiEdit*       _weightBox;
    gui::IGuiWindow*     _facePicture;
    gui::IGuiStaticText* _faceDescription;
    gui::IGuiPanel*      _predPool;
    gui::IGuiPanel*      _predPerception;
    gui::IGuiPanel*      _predEndurance;
    gui::IGuiPanel*      _predTracking;
    gui::IGuiPanel*      _predRigging;
    gui::IGuiPanel*      _malfunctions;
    TiXmlDocument*       _index;
protected:
    virtual ~CreateCareer();
private:
    static void messageCallback(gui::Message* message, void* userData);
private:
    void updateFaceGui(void);
    void updatePredisposition(gui::IGuiPanel* origin);
    void generateStartupGearStock(void);
public:
    CreateCareer(Career* career);
public:
    virtual void updateActivity(float dt);
    virtual bool endOfActivity(void);
    virtual void onBecomeActive(void);
    virtual void onBecomeInactive(void);
};

#endif