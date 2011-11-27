
#ifndef FATALITY_LIST_ACTIVITY_INCLUDED
#define FATALITY_LIST_ACTIVITY_INCLUDED

#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/engine.h"
#include "../shared/gui.h"

#include "activity.h"
#include "career.h"

using namespace ccor;

class Incident
{
private:
    std::string  _name;
    Virtues      _virtues;
    unsigned int _locationId;
public:
    // class implementation
    Incident(Career* career, unsigned int locationId);
    Incident(TiXmlElement* node);
    virtual ~Incident();
public:
    // class behaviour
    void save(TiXmlDocument* xmlDocument);
    // properties
    inline const char* getName(void) { return _name.c_str(); }
    inline Virtues* getVirtues(void) { return &_virtues; }
    inline unsigned int getLocationId(void) { return _locationId; }
};

class FatalityList : public Activity
{
private:
    typedef std::vector<Incident*> Incidents;
    typedef Incidents::iterator IncidentI;
private:
    typedef std::vector<unsigned int> Indices;
private:
    bool             _endOfActivity;
    gui::IGuiWindow* _window;
    Incident*        _freshIncident;
    Incidents        _incidents;
    Indices          _sortedIndices;
    int              _topIndex;
private:
    // internal behaviour
    void initialize(void);
    void updateGui(void);
    void sortByScore(void);
    // message callback
    static void messageCallback(gui::Message* message, void* userData);
public:
    // class implementation
    FatalityList();
    FatalityList(Career* career, unsigned int locationId);
    virtual ~FatalityList();
public:
    // Activity abstracts
    virtual void updateActivity(float dt);
    virtual bool endOfActivity(void);
    virtual void onBecomeActive(void);
    virtual void onBecomeInactive(void);
};

#endif