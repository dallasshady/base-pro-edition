/**
 * This source code is a part of AqWARium game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description Main window definitions
 *
 * @author bad3p
 */

#ifndef MAINWND_INCLUDED
#define MAINWND_INCLUDED
#include "../shared/ccor.h"

namespace mainwnd 
{

/**
 * Native main window trigger
 */
struct TriggerMainwnd 
{
    typedef ccor::Object Value;
    typedef ccor::Object Param;
    DECLARE_TRIGGER_ID(0x80000);
};

/**
 * Events
 */

struct evMainwndEvent : public ccor::Object
{
    DECLARE_EVENT_ID(0x80001);
};

struct evScreenSize : public evMainwndEvent
{
public:
    DECLARE_EVENT_ID(0x80004);
public:
    int width;
    int height;
public:
    evScreenSize(int w, int h) : width(w), height(h) {}
};


/**
 * Interface for Application main window
 */
class IMainWnd : public ccor::IBase {
public:
    DECLARE_INTERFACE_ID(0x80002);
public:
    virtual void __stdcall setWindowText(const char* text) = 0;
    virtual int __stdcall getHandle(void) = 0;
};

}

#endif
