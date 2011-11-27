#ifndef H7BF3DE64_1658_4f3e_9B3D_1132FAB5F0AA
#define H7BF3DE64_1658_4f3e_9B3D_1132FAB5F0AA
#include "../shared/ccor.h"
namespace ccor {

class Time : public ITime {
public:

    Time(entid_t _parentEid, TimeEventType evType) : value(0), timeout(1), parentEid(_parentEid) {
    }

    virtual void __stdcall setTimeout(float value) { }
    
    virtual float __stdcall getValue() { }

    virtual void __stdcall setValue(float value) { }
    
    virtual void __stdcall enable(bool enable) { }

    virtual void __stdcall advance(float dt) { }
    
    virtual void __stdcall release() { delete this; }

private:

    entid_t parentEid;

    float value;

    float timeout;

};




class SystemTime : public ITime {
public:

    SystemTime() { instance = this; value = 0; }

    ~SystemTime() { instance = 0; }

    virtual void __stdcall setTimeout(float value) { }
    
    virtual float __stdcall getValue() { return value; }

    virtual void __stdcall setValue(float value) { }
    
    virtual void __stdcall enable(bool enable) { }

    virtual void __stdcall advance(float dt) { }
    
    virtual void __stdcall release() { }

    void advanceSystem(float dt) { value+=dt; }

    static SystemTime * instance;

private:

    float value;

};



class TimeMgr {
public:

    TimeMgr() { instance = this; }

    ~TimeMgr() { instance = 0; }

    ITime * createTime(const char * name, entid_t eid, TimeEventType evType) {
        return 0;
    }

    ITime * getTime(const char * name) {
        if (stricmp(name, "system")==0)
            return &systemTime;
               
        // try to find named object in a map
        MapTime::const_iterator it = mapTime.find(name);
        if (it==mapTime.end()) return NULL;

        return it->second; 
    }

    ITime * getSystemTime() { return &systemTime; }

    static TimeMgr * instance;

private:

    typedef std::map<std::string, Time *> MapTime;

    MapTime mapTime;

    SystemTime systemTime;

};


}
#endif
