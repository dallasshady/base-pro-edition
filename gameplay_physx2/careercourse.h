
#ifndef CAREERCOURSE_MODE_INCLUDED
#define CAREERCOURSE_MODE_INCLUDED

#include "geoscape.h"

enum TimeSpeed
{
    tsPause,
    tsFast,
    tsFastest
};

#define HGUI_NUM_ITEMS 9

/**
 * career course dialog box
 */

class CareerDialog
{
public:
    virtual ~CareerDialog() {}
public:
    virtual bool isComplete(void) = 0;
    virtual void onGuiMessage(gui::Message* message) = 0;
};

class MessageDialog : public CareerDialog
{
private:
    gui::IGuiWindow* _window;
    bool             _isComplete;
public:
    // class implementation
    MessageDialog(const wchar_t* message);
    virtual ~MessageDialog();
    // CareerDialog implementation
    virtual bool isComplete(void);
    virtual void onGuiMessage(gui::Message* message);
};

class TravelDialog : public CareerDialog
{
private:
    Geoscape*        _geoscape;
    Location*        _location;
    float            _travelCost;
    float            _travelTime;
    float            _returnCost;
    float            _returnTime;
    float            _totalCost;
    gui::IGuiWindow* _window;
    bool             _isComplete;
public:
    // class implementation
    TravelDialog(Geoscape* geoscape, Location* location, float travelCost, float travelTime, float returnCost, float returnTime);
    virtual ~TravelDialog();
    // CareerDialog implementation
    virtual bool isComplete(void);
    virtual void onGuiMessage(gui::Message* message);
};

class EnterLocationDialog : public CareerDialog
{
private:
    Geoscape*        _geoscape;
    Location*        _location;
    gui::IGuiWindow* _window;
    bool             _isComplete;
public:
    // class implementation
    EnterLocationDialog(Geoscape* geoscape, Location* location);
    virtual ~EnterLocationDialog();
    // CareerDialog implementation
    virtual bool isComplete(void);
    virtual void onGuiMessage(gui::Message* message);
};

/**
 * geoscape mode : career course
 */

enum EventDisplayMode
{
    edmActive, // displays active events
    edmComing  // displays coming events
};

#define NUM_HISTORY_SLOTS 4
#define NUM_SHOP_ITEMS    10

class CareerCourse : public GeoscapeMode
{
protected:
    friend class Geoscape;
protected:
    gui::IGuiWindow*          _closeCareer;
    gui::IGuiWindow*          _careerCourse;
    gui::IGuiWindow*          _skillsScreen;
    gui::IGuiWindow*          _fundsScreen;
    gui::IGuiWindow*          _shopScreen;    
    gui::IGuiPanel*           _shopSlider;
    gui::IGuiPanel*           _playerSlider;
    GearType                  _gearFilter;
    std::vector<unsigned int> _marketOffer;
    std::vector<unsigned int> _playerOffer;
    std::vector<unsigned int> _unsorted;
    engine::ITexture* _comingEventsTexture;
    gui::IGuiWindow*  _comingEventsIcon;
    gui::IGuiWindow*  _activeEventsIcon;
    EventDisplayMode  _displayMode;
    TimeSpeed         _timeSpeed;
    float             _timeBlink;
    CareerDialog*     _careerDialog;   
protected:
    void updateEventProgress(Event* event);
    void updateGui(void);
    gui::IGuiPanel* getRankProgressBox(unsigned int id);
    gui::IGuiPanel* getPromoProgressBox(unsigned int id);
    void updateSkillsScreen(void);
    void updateFundsScreen(void);
    void updateShopScreen(void);
    bool isBestOffer(Gear* candidate, Gear* nominant);
    void updateBlinking(float dt);
    void updateBlinkingButton(gui::IGuiPanel* buttonPanel, float alpha);
    void resetBlinking(void);
    void setCareerDialog(CareerDialog* dialog);
    void setTimeSpeed(TimeSpeed timeSpeed);
    void travelToLocation(Location* location);
    void sellItem(unsigned int itemId);
    void buyItem(unsigned int itemId);
    float simulateCourse(float minutes, bool guiFeedback);
    float getTreatmentEffectivity(void);
    float getTreatmentFee(void);
protected:
    static void eventCallback(Event* event, void* data);
protected:
    // class implementation
    CareerCourse(Geoscape* geoscape);
    virtual ~CareerCourse();
    // GeoscapeMode
    virtual bool endOfMode(void);
    virtual void onGuiMessage(gui::Message* message);
    virtual void onUpdateMode(float dt);
};

#endif