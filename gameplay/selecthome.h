
#ifndef SELECTHOME_MODE_INCLUDED
#define SELECTHOME_MODE_INCLUDED

#include "geoscape.h"
#include "easybmp.h"

/**
 * mode : select home location
 */

class SelectHome : public GeoscapeMode
{
protected:
    friend class Geoscape;
protected:
    BMP*             _worldMap;
    gui::IGuiWindow* _messageBox;
    bool             _endOfMode;
    int              _homeX;
    int              _homeY;
protected:
    // class implementation
    SelectHome(Geoscape* geoscape);
    virtual ~SelectHome();
    // GeoscapeMode
    virtual bool endOfMode(void);
    virtual void onGuiMessage(gui::Message* message);
    virtual void onUpdateMode(float dt);
protected:
    // mode result
    inline int getHomeX(void) { return _homeX; }
    inline int getHomeY(void) { return _homeY; }
};

#endif