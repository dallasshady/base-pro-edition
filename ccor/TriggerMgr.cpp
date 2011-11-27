/**
 * This source code is a part of Metathrone game project. 
 * (c) Perfect Play 2003.
 *
 * @author Sergey Alekhin
 */

#include "headers.h"
#include "CoreImpl.h"
#include "TriggerMgr.h"
#include "EntityMgr.h"
namespace ccor {

#define INVALID_TRIGGER(id) throw Exception("ccor : Invalid trigger id 0x%X", id)


struct TriggerObj : public Object {
    ITrigChecker * checker;
    char flags;
    std::set<entid_t> listenerEntities;
    Object * value;
    TriggerObj() {
        checker = NULL;
        flags = 0;
        value = NULL;
    }
};


TriggerMgr::TriggerMgr() {

}


TriggerMgr::~TriggerMgr() {
    destroyAll();
}


void TriggerMgr::destroyAll() {

    for (MapId::iterator it=mapId.begin(); it!=mapId.end(); ++it) {
        delete it->second;
    }

    mapId.clear();
}


void TriggerMgr::createTrigger(trigid_t id, int flags, Object * value) {
    TriggerObj * t = new TriggerObj;
    t->checker = NULL;
    t->flags = (char) flags;
    t->value = value;
    mapId[id] = t;
}


void TriggerMgr::destroyTrigger(trigid_t id) {
    MapId::iterator it = mapId.find(id);
    if (it==mapId.end()) INVALID_TRIGGER(id);
    delete it->second;
    mapId.erase(it);
}


void TriggerMgr::trigAddListener(trigid_t idTrig, entid_t id) {
    MapId::iterator it = mapId.find(idTrig);
    if (it==mapId.end()) INVALID_TRIGGER(idTrig);
    TriggerObj * t=it->second;
    t->listenerEntities.insert(id);
}


void TriggerMgr::trigRemoveListener(trigid_t idTrig, entid_t id) { 
    MapId::iterator it = mapId.find(idTrig);
    if (it==mapId.end()) INVALID_TRIGGER(idTrig);
    TriggerObj * t=it->second;
    t->listenerEntities.erase(id);
}


void TriggerMgr::activate(EntityMgr * emgr, trigid_t idTrig, Object * param) {
    MapId::iterator tr = mapId.find(idTrig);
    if (tr==mapId.end()) INVALID_TRIGGER(idTrig);
    bool immFlag=(tr->second->flags & trigImmediate)!=0;
    // Process all listeners
    tmpListeners = tr->second->listenerEntities;
    processedListeners.clear();
    while (!tmpListeners.empty()) {
        entid_t id = *tmpListeners.begin();
        tmpListeners.erase(id);
        if (processedListeners.find(id)==processedListeners.end()) {
            processedListeners.insert(id);
            emgr->handleTrigger(id, idTrig, param, immFlag);
        }
    }
    processedListeners.clear();
    tmpListeners.clear();
}


void TriggerMgr::setTriggerValue(trigid_t id, Object * value) {
    MapId::iterator it = mapId.find(id);
    if (it==mapId.end()) INVALID_TRIGGER(id);
    if (!(it->second->flags | trigValueable)) {
        throw Exception("ccor : Can't %s value of non-valuable trigger 0x%04X. "
            "It should be created with trigValueable flag.", "set", id);
    }
    it->second->value = value;
}


Object * TriggerMgr::getTriggerValue(trigid_t id) {
    MapId::iterator it = mapId.find(id);
    if (it==mapId.end()) INVALID_TRIGGER(id);
    if (!(it->second->flags | trigValueable)) {
        throw Exception("ccor : Can't %s value of non-valuable trigger 0x%04X. "
            "It should be created with trigValueable flag.", "get", id);
    }
    return it->second->value;
}


void TriggerMgr::onEntityDestroyed(entid_t id) {

    tmpListeners.erase(id);

    MapId::const_iterator it = mapId.begin();

    for (; it!=mapId.end(); ++it) {
        it->second->listenerEntities.erase(id);
    }

}


void TriggerMgr::checkTriggers(EntityMgr * emgr) {

    MapId::const_iterator it = mapId.begin();

    for (; it!=mapId.end(); ++it) {
        TriggerObj * obj = it->second;
        if (NULL!=obj->checker) {
            Object * param = NULL;
            if (obj->checker->onCheckTrigger(it->first,param)) {
                activate(emgr,it->first,param);
            }
        }
    }
}


}
