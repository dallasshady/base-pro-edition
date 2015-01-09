
#ifndef HUD_ACTORS_INCLUDED
#define HUD_ACTORS_INCLUDED

#include "headers.h"
#include "scene.h"
#include "callback.h"
#include "sensor.h"
#include "scene.h"

const gui::Rect hud_digit_1( 0, 0, 48, 80 );
const gui::Rect hud_digit_2( 48, 0, 96, 80 );
const gui::Rect hud_digit_3( 96, 0, 144, 80 );
const gui::Rect hud_digit_4( 144, 0, 192, 80 );
const gui::Rect hud_digit_5( 192, 0, 240, 80 );
const gui::Rect hud_digit_6( 0, 80, 48, 160 );
const gui::Rect hud_digit_7( 48, 80, 96, 160 );
const gui::Rect hud_digit_8( 96, 80, 144, 160 );
const gui::Rect hud_digit_9( 144, 80, 192, 160 );
const gui::Rect hud_digit_0( 192, 80, 240, 160 );
const gui::Rect hud_digit_minus( 0, 160, 48, 240 );
const gui::Rect hud_digit_reset( 48, 160, 96, 240 );
const gui::Rect hud_digit_null( 96, 160, 144, 240 );

/**
 * HUD actor
 */

class HUD : public Actor
{
protected:
    void resetIndicator(gui::IGuiPanel* panel);
    void setIndicator(gui::IGuiPanel* panel, unsigned int digit);
    void setSignumIndicator(gui::IGuiPanel* panel, float value);
public:
    HUD(Actor* parent);
    virtual ~HUD();
};

/**
 * altimeter actor
 */

class Altimeter : public HUD
{
private:
    struct AltimeterState
    {
    public:
        bool  mode;
        float altitude;
    };
private:
    gui::IGuiWindow* _window;
    float            _timeout;
    GameData*        _gameData;
    AltimeterState*  _state;
    float            _acAMTimeout;
    float            _acIWATimeout;
    float            _acDWATimeout;
    audio::ISound*   _toneSound;
public:
    // actor abstracts
    virtual void onUpdateActivity(float dt);
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
public:
    // class implemetation
    Altimeter(Actor* parent);
    virtual ~Altimeter();
public:
    static void setAudibleAltimeter(Career* career, bool mode, float altitude);
    static bool getAAMode(Career* career);
    static float getAAAltitude(Career* career);
};

/**
 * variometer actor
 */

class Variometer : public HUD
{
private:
    gui::IGuiWindow* _window;
    float            _timeout;
public:
    // actor abstracts
    virtual void onUpdateActivity(float dt);
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
public:
    // class implemetation
    Variometer(Actor* parent);
    virtual ~Variometer();
};

/**
 * timer actor
 */

class Timer : public HUD
{
private:
    gui::IGuiWindow* _window;
    bool             _active;
    float            _time;
    unsigned int     _startEvent;
    unsigned int     _stopEvent;
public:
    // actor abstracts
    virtual void onUpdateActivity(float dt);
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
public:
    // class implemetation
    Timer(Actor* parent, unsigned int startEvent, unsigned int stopEvent, const wchar_t* caption);
    virtual ~Timer();
};

/**
 * health status
 */

class HealthStatus : public Actor
{
private:
    gui::IGuiWindow* _healthWindow;
    gui::IGuiWindow* _skillsWindow;
public:
    // actor abstracts
    virtual void onUpdateActivity(float dt);
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
public:
    // class implemetation
    HealthStatus(Actor* parent);
    virtual ~HealthStatus();
};

#endif