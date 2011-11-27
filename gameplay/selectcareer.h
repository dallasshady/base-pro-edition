
#ifndef SELECTCAREER_ACTIVITY_INCLUDED
#define SELECTCAREER_ACTIVITY_INCLUDED

#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/engine.h"
#include "../shared/gui.h"

#include "activity.h"
#include "career.h"

using namespace ccor;

#define SCGUI_NUM_ITEMS 4

class SelectCareer : public Activity
{
private:
    struct Item
    {
    public:
        gui::IGuiWindow*     itemWindow;
        gui::IGuiWindow*     itemPicture;
        gui::IGuiStaticText* itemCareer;
        gui::IGuiButton*     itemSelectButton;
        gui::IGuiButton*     itemDeleteButton;
        Career*              career;
    public:
        Item();
        Item(gui::IGuiWindow* root, 
             const char* windowName,
             const char* pictureName,
             const char* careerName,
             const char* selectButtonName,
             const char* deleteButtonName);
    public:
        void enable(bool flag);
    };
private:
    gui::IGuiWindow* _window;
    gui::IGuiSlider* _slider;
    Item             _items[SCGUI_NUM_ITEMS];
    int              _topCareer;
    bool             _endOfActivity;
private:
    void updateGUI(void);
    void createCareer(void);
protected:
    virtual ~SelectCareer();
private:
    static void messageCallback(gui::Message* message, void* userData);
public:
    SelectCareer();
public:
    virtual void updateActivity(float dt);
    virtual bool endOfActivity(void);
    virtual void onBecomeActive(void);
    virtual void onBecomeInactive(void);
};

#endif