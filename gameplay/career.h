
#ifndef CAREER_DESCRIPTOR_INCLUDED
#define CAREER_DESCRIPTOR_INCLUDED

#include "headers.h"
#include "virtues.h"
#include "database.h"
#include "../shared/gui.h"

/**
 * forward declaration
 */

class Geoscape;

/**
 * time casting
 */

#define HOURS_TO_MINUTES(H) ( 60 * H )
#define DAYS_TO_MINUTES(D) ( HOURS_TO_MINUTES( D * 24 ) )
#define WEEKS_TO_MINUTES(W) ( DAYS_TO_MINUTES( W * 7 ) )
#define MONTHS_TO_MINUTES(M) ( WEEKS_TO_MINUTES( M * 4 ) )

/**
 * career event
 */

class Career;

enum EventFlag
{
    efSystem   = 0x0001, // event is for system internal usage
    efActive   = 0x0002, // event is active
    efFinished = 0x0004  // event is finished
};

class Event
{
protected:
    Career*           _career;        // career owned the event
    unsigned int      _flags;         // event flags
    float             _duration;      // event duration (in minutes)
    float             _timeTo;        // time to event occurence (in minutes)
    unsigned int      _databaseId;    // internal database id for this event
    engine::ITexture* _buttonTexture; // button texture
    gui::IGuiWindow*  _window;        // window
protected:
    void initializeGui(void);
    void setEventCaption(const wchar_t* caption);
public:
    Event(Career* career, unsigned int databaseId);
    Event(Career* career, unsigned int databaseId, TiXmlElement* element);
    virtual ~Event();
public:
    inline float& getDuration(void) { return _duration; }
    inline float& getTimeTo(void) { return _timeTo; }
    inline unsigned int& getFlags(void) { return _flags; }
    inline unsigned int getDatabaseId(void) { return _databaseId; }
    inline gui::IGuiWindow* getWindow(void) { return _window; }
public:
    void saveToXml(TiXmlNode* node);
    static Event* createFromXml(Career* career, TiXmlElement* element);
public:
    virtual void onSaveToXml(TiXmlElement* element);
    virtual const char* getClassName(void) = 0;
    virtual void onBeginEvent(Geoscape* geoscape) = 0;
    virtual void onEndEvent(Geoscape* geoscape) = 0;
    virtual void onUpdateEvent(Geoscape* geoscape, float dt) = 0;
    virtual void enumerateLocationIds(std::vector<unsigned int>& ids) = 0;
};

/**
 * game data for common serialization usage
 */

class GameData
{
private:
    unsigned int _size;
    char*        _data;
public:
    GameData(unsigned int size)
    {
        _size = size;
        _data = new char[size];
        memset( _data, 0, size );
    }
    ~GameData()
    {
        delete[] _data;
    }
public:
    inline unsigned int getSize(void) { return _size; }
    inline void* getData(void) { return _data; }
};

/**
 * career
 */

enum Acrobatics
{
    acroJumpFromRun   = 0x00000001,
    acroFreeflyFlip   = 0x00000002,
    acroFreeflySitfly = 0x00000004,
    acroFrontFlip     = 0x00000008,
    acroFrontBackFlip = 0x00000010,
    acroBackFlip      = 0x00000020,
    acroBackFrontFlip = 0x00000040,
    acroReserved01    = 0x00000080,
    acroReserved02    = 0x00000100,
    acroReserved03    = 0x00000200,
    acroReserved04    = 0x00000400,
    acroReserved05    = 0x00000800,
    acroReserved06    = 0x00001000,
    acroReserved07    = 0x00002000,
    acroReserved08    = 0x00004000,
    acroReserved09    = 0x00008000,
    acroReserved10    = 0x00010000,
    acroReserved11    = 0x00020000,
    acroReserved12    = 0x00040000,
    acroReserved13    = 0x00080000,
    acroReserved14    = 0x00100000,
    acroReserved15    = 0x00200000,
    acroReserved16    = 0x00400000,
    acroReserved17    = 0x00800000,
    acroReserved18    = 0x01000000,
    acroReserved19    = 0x02000000,
    acroReserved20    = 0x04000000,
    acroReserved21    = 0x08000000,
    acroReserved22    = 0x10000000,
    acroReserved23    = 0x20000000,
    acroReserved24    = 0x40000000,
    acroReserved25    = 0x80000000
};

typedef void (*EventCallback)(Event* event, void* data);

class Career
{
private:
    typedef std::pair<std::string, GameData*> GameDataT;
    typedef std::map<std::string, GameData*> GameDataM;
    typedef GameDataM::iterator GameDataI;
private:
    std::string          _name;
    Virtues              _virtues;
    std::vector<Gear>    _gears;
    std::vector<Event*>  _events;    
    GameDataM            _gameDataM;
    bool                 _isHomeDefined;
    int                  _homeX;
    int                  _homeY;
    EventCallback        _eventCallback;
    void*                _eventCallbackData;
private:
    void initializeWalkthroughMeter(void);
public:
    // class implementation
    Career(const char* name);
    Career(TiXmlElement* node);
    virtual ~Career();
public:
    // career properties
    inline const char* getName(void) { return _name.c_str(); }
    inline Virtues* getVirtues(void) { return &_virtues; }
    inline unsigned int getNumGears(void) { return (unsigned int)_gears.size(); }
    inline Gear getGear(unsigned int id) { assert( id>=0 && id<_gears.size() ); return _gears[id]; }
    inline bool isHomeDefined(void) { return _isHomeDefined; }
    inline int getHomeX(void) { return _homeX; }
    inline int getHomeY(void) { return _homeY; }
    inline unsigned int getNumEvents(void) { return (unsigned int)_events.size(); }
    inline Event* getEvent(unsigned int id) { assert( id>=0 && id<_events.size() ); return _events[id]; }        
public:
    // gear management
    void addGear(Gear gear);
    void removeGear(unsigned int id);
    void equipGear(unsigned int id);
	bool equipGear();						// equip gear in all unequipped slots
	bool unequipGear(GearType type);		// unequip certain gear type. returns true if slot was not empty
	bool unequipGear();			// unequip all gear types. returns true if at least one slot was not empty
public:
    // event management
    void addEvent(Event* event);
    void removeEvent(Event* event);    
    void setEventCallback(EventCallback eventCallback, void* data);
public:
    // game data management
    void addGameData(const char* name, GameData* gameData);
    GameData* getGameData(const char* name);
public:
    // career management
    void setHome(int homeX, int homeY);
    void save(TiXmlDocument* xmlDocument);
public:
    // career walk-through management
    float getWalkthroughPercent(void);
    bool getMissionWalkthroughFlag(unsigned int tournamentId, unsigned int missionId);
    void setMissionWalkthroughFlag(unsigned int tournamentId, unsigned int missionId, bool flag);
public:
    // baggage of acrobatics 
    bool getAcrobaticsSkill(Acrobatics skill);
    void setAcrobaticsSkill(Acrobatics skill, bool isLearned);
public:
    // other properties
    bool getLicensedFlag(void);
    void setLicensedFlag(bool flag);
    bool getHomePlacementFlag(void);
    void setHomePlacementFlag(bool flag);
};

#endif