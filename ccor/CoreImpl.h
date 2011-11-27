/**
 * This source code is a part of Metathrone game project. 
 * (c) Perfect Play 2003.
 *
 * @author Sergey Alekhin
 */

#ifndef H1E3EB54B_E4EB_457E_AF98_6E1B6D060152
#define H1E3EB54B_E4EB_457E_AF98_6E1B6D060152
#include "../shared/ccor.h"
#include "EntityMgr.h"
#include "TriggerMgr.h"
#include "Resource.h"
#include "Idset.h"
#include "RandToolkit.h"
#include "TimeMgr.h"
namespace ccor {

class CoreImpl;


struct CoreTerminateException {
public:
    CoreTerminateException(int exitCode) : code(exitCode) {
    }

    int code;
};


class SingleCore {
public:

    static ICore * getInstance();
    static void releaseInstance();

private:
    static CoreImpl * instance;
};




class CoreImpl : public ICore {
public:

    /**
     * @link
     * @shapeType PatternLink
     * @pattern Singleton
     * @supplierRole Singleton factory 
     */
    /*# SingleCore _singleCore; */

    CoreImpl();
    ~CoreImpl();
    void init();
    void release();

    virtual IParamPack * __stdcall getCoreParamPack() {
        return coreParamPack;
    }

    virtual IParamPackFactory * __stdcall getParamPackFactory() {
        return paramPackFactory;
    }

    virtual void __stdcall exit(int code) {
        throw CoreTerminateException(code);
    }

    virtual void __stdcall logMessageV(const char * fmt, va_list vl);

    // Execute core entities
    void __stdcall act();

    virtual entid_t __stdcall createEntity(const char * type, entid_t idParent, Object * param) { return em.createEntity(type,idParent,param); }

    virtual void __stdcall destroyEntity(entid_t id) { tm.onEntityDestroyed(id); em.destroyEntity(id); }

    virtual int __stdcall findByType(const char * type) { return em.findByType(type); }

    virtual entid_t __stdcall getByType(const char * type) { return em.getByType(type); }

    virtual void __stdcall copyEntityResult(entid_t * array) { em.copyEntityResult(array); }

    virtual IBase * __stdcall askInterface(entid_t eid, iid_t iid) { return em.queryInterface(eid,iid,true); }

    virtual IBase * __stdcall askInterface(const char * entityType, iid_t iid) { return em.queryInterface(entityType,iid,true); }

    virtual IBase * __stdcall queryInterface(entid_t eid, iid_t iid) { return em.queryInterface(eid,iid,false); }

    virtual IBase * __stdcall queryInterface(const char * entityType, iid_t iid) { return em.queryInterface(entityType,iid,false); }

    virtual int __stdcall findTypesForInterface(iid_t iid) { return em.findTypesForInterface(iid); }

    virtual void __stdcall copyStrResult(const char * * array) { em.copyStrResult(array); }

    virtual const char * __stdcall getEntityType(entid_t id) { return em.getEntityType(id); }

    virtual void __stdcall sendEventEntity(entid_t entity, evtid_t event, Object * param) { em.sendEventEntity(entity,event,param); }

    virtual void __stdcall sendEventType(const char * type, evtid_t event, Object * param) { em.sendEventType(type,event,param); }

    virtual void __stdcall setActivity(entid_t id, float activityRate) { em.setActivity(id, activityRate); }

    virtual void __stdcall createTrigger(trigid_t id, int flags, Object * value) { tm.createTrigger(id,flags,value); }

    virtual void __stdcall destroyTrigger(trigid_t id) { tm.destroyTrigger(id); }

    virtual void __stdcall trigAddListener(trigid_t trigId, entid_t id) { tm.trigAddListener(trigId, id); }

    virtual void __stdcall trigRemoveListener(trigid_t trigId, entid_t id) { tm.trigRemoveListener(trigId,id); }

    virtual void __stdcall activate(trigid_t trigId, Object * param) { tm.activate(&em,trigId,param); }

    virtual void __stdcall setTriggerValue(trigid_t id, Object * value) { tm.setTriggerValue(id,value); }

    virtual Object * __stdcall getTriggerValue(trigid_t id) { return tm.getTriggerValue(id); }

    virtual globid_t __stdcall findId(const char * name) { return idset.find(name); }

    virtual const char * __stdcall getIdName(globid_t id) { return idset.getName(id); }

    virtual IResource * __stdcall getResource(const char * resourceName, const char * mode, const char * type) {
        return resMgr.getResource(resourceName,mode,type);
    }

    virtual time_t __stdcall getResourceTime(const char * resourceName) { return resMgr.getLastModified(resourceName); }

    virtual void __stdcall processSML(const char * smlText, SMLListener * lis);

    virtual IRandToolkit * __stdcall getRandToolkit() { return &randToolkit; }

    virtual ITime *  __stdcall createTime(const char * name, entid_t eid, TimeEventType evType) { return timeMgr.createTime(name,eid,evType); }

    virtual ITime * __stdcall getTime(const char * name) { return timeMgr.getTime(name); }

    virtual ITime * __stdcall getSystemTime() { return timeMgr.getSystemTime(); }

    virtual Object * __stdcall getXData() { return xdata; }

    virtual void __stdcall setXData(Object * p) { xdata = p; }

private:
    /** @link aggregation */
    EntityMgr em;

    /** @link aggregation */
    TriggerMgr tm;

    /** @link aggregation */
    IParamPackFactory * paramPackFactory;

    /** @link aggregation */
    IParamPack * coreParamPack;

    /** @link aggregation */
    ResourceMgr resMgr;

    /** @link aggregation */
    Idset idset;

    /** @link aggregation */
    RandToolkit randToolkit;

    /** @link aggregation */
    TimeMgr timeMgr;
    
    std::vector<std::string> logBuffered;

    Object * xdata;

    FILE * flog;

    bool htmlLog;

    void processSystemMessages();

    void startLog();

};


}

#endif
