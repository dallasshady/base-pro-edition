/**
 * This source code is a part of Metathrone game project. 
 * (c) Perfect Play 2003.
 *
 * @description Trigger manager implementation
 *
 * @author Sergey Alekhin
 */

#ifndef H4076DCE9_5EEE_43A4_85F7_85A06961AD2C
#define H4076DCE9_5EEE_43A4_85F7_85A06961AD2C
#include "../shared/ccor.h"
namespace ccor {

class EntityMgr;
struct TriggerObj;    

class TriggerMgr {
public:
    TriggerMgr();
    ~TriggerMgr();

    void createTrigger(trigid_t id, int flags, Object * value);

    void destroyTrigger(trigid_t id);

    void trigAddListener(trigid_t idTrig, entid_t id);

    void trigRemoveListener(trigid_t idTrig, entid_t id);

    void activate(EntityMgr * emgr, trigid_t idTrig, Object * data);

    void setTriggerValue(trigid_t id, Object * value);

    Object * getTriggerValue(trigid_t id);

    // Process checking-triggers cycle
    void checkTriggers(EntityMgr * emgr);

    // Notify that entity has been destroyed
    void onEntityDestroyed(entid_t id);

    void destroyAll();

private:

    typedef std::map<trigid_t, TriggerObj*> MapId;

    MapId mapId;

    std::set<entid_t> tmpListeners;

    std::set<entid_t> processedListeners;

};

}
#endif
