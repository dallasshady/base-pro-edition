
#ifndef GEOSCAPE_ACTIVITY_INCLUDED
#define GEOSCAPE_ACTIVITY_INCLUDED

#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/engine.h"
#include "../shared/gui.h"

#include "activity.h"
#include "career.h"

using namespace ccor;

class Geoscape;
class GeoscapeMode;
class Location;

/**
 * location is geoscape menu item, provided with certain behaviour
 */

class Location
{
protected:
    struct LocationData
    {
        WeatherType weather;     // weather in location
        Vector3f    wind;        // wind direction
        float       windAmbient; // value of ambient wind
        float       windBlast;   // value of wind blasts
        bool        player;      // true if player in location        
    };
protected:
    Geoscape*        _geoscape;    // subj.
    unsigned int     _databaseId;  // location database id    
    LocationData*    _gameData;    // gamedata (serializable)
    gui::IGuiWindow* _window;      // location window
    GameData*        _demoGameData;// for demoversion purpose
public:
    inline unsigned int getDatabaseId(void) { return _databaseId; }
    inline WeatherType getWeather(void) { return _gameData->weather; }
    inline bool getPlayer(void) { return _gameData->player; }
    inline gui::IGuiWindow* getWindow(void) { return _window; }
    inline Vector3f getWindDirection(void) { return _gameData->wind; }
    inline float getWindAmbient(void) { return _gameData->windAmbient; }
    inline float getWindBlast(void) { return _gameData->windBlast; }
public:
    // class implementation
    Location(Geoscape* geoscape, unsigned int databaseId);
    virtual ~Location();
public:
    // common behaviour
    void setWeather(WeatherType weather);
    void setWind(Vector3f dir, float ambient, float blast);
    void setPlayer(bool player);
    Vector2f getLocationCoords(void);
    const wchar_t* getWindForecast(void);
    const wchar_t* getWeatherForecast(void);
};

/**
 * mode is sub-activity, running under terms of geoscape activity
 *  - mode can manage their own gui resource and handle gui messages
 *  - modes are managed by geoscape
 */

class GeoscapeMode
{
protected:
    friend class Geoscape;
protected:
    Geoscape* _geoscape;
protected:
    GeoscapeMode(Geoscape* geoscape) : _geoscape(geoscape) {}
    virtual ~GeoscapeMode() {}
protected:
    virtual bool endOfMode(void) = 0;
    virtual void onGuiMessage(gui::Message* message) = 0;
    virtual void onUpdateMode(float dt) = 0;
};

/**
 * geoscape is root career activity, that cause all other activities
 */

struct DateTime
{
public:
    int year;
    int month;
    int week;
    int day;
    int hour;
    int minute;
public:
    DateTime() : year(0), month(0), week(0), day(0), hour(0), minute(0) {}
    DateTime(float timeInMinutes)
    {
        minute = int( timeInMinutes );
        hour   = minute / 60; minute -= hour * 60;
        day    = hour / 24; hour -= day * 24;
        week   = day / 7; day -= week * 7;
        month  = week / 4; week -= month * 4;
        year   = month / 12; month -= year * 12;
        year++, month++, week++, day++;
    }
};

class Geoscape : public Activity
{
private:
    struct HistoryEntry
    {
    public:
        std::wstring message;
        Vector4f     color;
        float        lifetime;
    public:
        HistoryEntry(const wchar_t* m, const Vector4f& c, float lt) : 
            message(m), color(c), lifetime(lt) 
        {}
    };
private:
    Career*                   _career;
    bool                      _endOfActivity;
    gui::IGuiWindow*          _geoscape;
    bool                      _moveMap;
    bool                      _lastPosIsValid;
    int                       _lastX;
    int                       _lastY;
    int                       _currX;
    int                       _currY;
    GeoscapeMode*             _geoscapeMode;
    std::vector<Location*>    _locations;
    Activity*                 _requestedActivity;
    float                     _passedTime;
    std::vector<unsigned int> _blinkingLocations;
    float                     _blinkingTime;
    std::vector<HistoryEntry> _history;
    std::vector<Gear>         _market; // market simulation
protected:
    // class implementation
    virtual ~Geoscape();
private:
    // message handling
    static void messageCallback(gui::Message* message, void* userData);
    // gui render callback
    static void panelRenderCallback(gui::IGuiPanel* panel, void* data);
public:
    // class implementation
    Geoscape(Career* career);
public:
    // Activity
    virtual void updateActivity(float dt);
    virtual bool endOfActivity(void);
    virtual void onBecomeActive(void);
    virtual void onBecomeInactive(void);
    virtual void onReturnFromActivity(Activity* activity);
    // class behaviour
    void onUpdateMapScrolling(float dt);
public:
    // geoscape behaviour
    gui::IGuiWindow* getWindow(void);
    Career* getCareer(void);
    Vector3f getCursorLatitude(void);
    Vector3f getCursorLongitude(void);
    int getCursorX(void);
    int getCursorY(void);
    unsigned int getNumLocations(void);
    Location* getLocation(unsigned int id);
    Location* getLocation(gui::IGuiPanel* panel);
    Location* getPlayerLocation(void);
    Location* getHomeLocation(void);
    DateTime getDateTime(void);
    void requestActivity(Activity* activity);
    void setBlinkingLocations(Event* event);
    void setNoBlinkingLocations(void);
    unsigned int getHistorySize(void);
    void addHistory(const wchar_t* message, const Vector4f color);
    void addGearToMarket(Gear gear);
    void removeGearFromMarket(unsigned int id);
    unsigned int getMarketSize(void);
    Gear getGearFromMarket(unsigned int id);
    Gear generateGear(bool generateUsedGear);
    void centerGeoscape(int x, int y);
    void closeGeoscapeMode(void);
};

#endif