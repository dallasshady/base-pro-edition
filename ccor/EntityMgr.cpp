/**
 * This source code is a part of Metathrone game project. 
 * (c) Perfect Play 2003.
 *
 * @author Sergey Alekhin
 */

#include "headers.h"
#include <ctime>
#include "CoreImpl.h"
#include "EntityMgr.h"
#include "ComponentMgr.h"
#include "Windows.h"

namespace ccor {

static const int ACTIVITY_RATE_NORMAL       = 1000;
static const int ACTIVITY_RATE_MAX          = 2 * ACTIVITY_RATE_NORMAL;
static const int ACTIVITY_MAX_DELAY_CLOCK   = 5 * CLOCKS_PER_SEC;


entid_t EntityMgr::createEntity(const char * type, entid_t idParent, Object * param) {

    if (NULL==type) throw Exception("core: Invalid entity type [NULL]");
    if (-1!=idParent) checkId(idParent);
    // Get component
    ComponentMgr * comgr = SingleComponentMgr::getInstance();
    IComponent * c = comgr->getComponentForType(type);
    // Try to create entity
    EntityBase * e = NULL;
    try {
        // Find creator
        const EntityTypeInfo * t = c->getTypeInfo();
        for (; t && t->typeName; ++t) {
            if (stricmp(t->typeName, type)==0) {
                if (t->creator==NULL) break;
                e = t->creator();
                break;
            }
        }
        if (NULL==e) throw Exception("core: Unable to find entity creator");
    }
    catch(const Exception& e) {
        SingleCore::getInstance()->logMessage("core: Exception while creating entity '%s' -->>\n%s", type, (const char*)e);
        throw;
    }
    catch(...) {
        SingleCore::getInstance()->logMessage("core: Unknown exception while creating entity '%s'", type);
        throw;
    }
    if (e==NULL) throw Exception("core: Error creating entity %s", type);
    // Find type chunk if the one exists; if not, create it
    typeid_t typeId = -1;
    MapTypeId::const_iterator it=mapTypeId.find(type);
    if (it==mapTypeId.end()) {
        typeId = createChunkType(c, type);
        mapTypeId[type]=typeId;
    }
    else typeId = it->second;
    // Add entity information
    entid_t id = createChunkEntity(e, typeId);
    chunkType[typeId].instances.push_back(id);
    if (-1!=idParent) {
        EntityChunk& ecParent=chunkEntity[idParent];
        ecParent.children.back()=id;
        ecParent.children.push_back(-1);
        const_cast<const entid_t*&>(ecParent.entity->childId) = &ecParent.children[0];
    }
    EntityChunk& ec = chunkEntity[id];
    const_cast<entid_t&>(e->entityId) = id;
    const_cast<entid_t&>(e->parentId) = idParent;
    const_cast<const entid_t*&>(e->childId) = &ec.children[0];
    const_cast<ICore*&>(e->icore) = SingleCore::getInstance();
    // Try to init entity
    bool init=false;
    try {
        e->entityInit(param);
        init=true;
    }
    catch(const Exception& e) {
        SingleCore::getInstance()->logMessage("core: Exception in %s::entityInit() -->>\n%s", type, (const char*)e);
        destroyEntity(id);
        throw;
    }
    catch(...) {
        SingleCore::getInstance()->logMessage("core: Unknown exception in %s::entityInit()", type);
        destroyEntity(id);
        throw;
    }
    // Notify subscribers that entity has been created
    TrigEntityLife::Param trigParam;
    trigParam.eid = id;
    trigParam.lifeTime = entityCreated;
    trigParam.param = param;
    trigParam.entityType = chunkType[chunkEntity[id].typeId].name;
    getCore()->activate(TrigEntityLife::tid,&trigParam);
    return id;
}


void EntityMgr::destroyEntity(entid_t id) {
    if (id==-1) return;
    // checkId(id);
    // Notify subscribers that entity is to be destroyed
    TrigEntityLife::Param trigParam;
    trigParam.eid = id;
    trigParam.lifeTime = entityPreDestroy;
    trigParam.param = NULL;
    trigParam.entityType = chunkType[chunkEntity[id].typeId].name;
    getCore()->activate(TrigEntityLife::tid,&trigParam);
    // Begin destroying
    EntityChunk& ec = chunkEntity[id];
    entid_t parentId=ec.entity->parentId;
    // Destroy children
    while (ec.children.size()>1) {
        entid_t idChild = ec.children[ec.children.size()-2];
        ec.children.erase(ec.children.begin()+ec.children.size()-2);
        getCore()->destroyEntity(idChild);
    }
    // Destroy entity itself
    EntityTypeChunk& tc = chunkType[ec.typeId];
    for (unsigned int i=0; i < tc.instances.size(); ++i) {
        if (tc.instances[i]==id) {
            tc.instances.erase(tc.instances.begin()+i);
            break;
        }
    }
    // physically destroy entity
    ec.entity->entityDestroy();
    // remove id from parent entity
    if (parentId>=0) {
        EntityChunk& ecParent=chunkEntity[parentId];
        for (unsigned int i=0; i < ecParent.children.size(); ++i) {
            if (ecParent.children[i]==id) {
                ecParent.children.erase(ecParent.children.begin()+i);
                --i;
            }
        }
    }
    // free entity's chunk
    ec.typeId=-1;
    ec.entity = NULL;
    freeChunkEntity.push(id);
    // Notify subscribers that entity has been destroyed
    trigParam.lifeTime = entityDestroyed;
    getCore()->activate(TrigEntityLife::tid,&trigParam);
}


IBase * EntityMgr::queryInterface(entid_t eid, iid_t iid, bool softQuery) {
    IBase * pInt = NULL;
    if (eid>=0) {
        checkId(eid);
        pInt = chunkEntity[eid].entity->entityAskInterface(iid);
    }
    if (NULL==pInt && !softQuery) {
        const char * iname = getCore()->getIdName(iid);
        throw Exception("core: Entity {eid=%d} doesn't support Iterface{iid=0x%08X, name='%s'}", eid, iid, iname);
    }
    return (IBase*)vtableDisplace.displaceInterface(pInt);
}


IBase * EntityMgr::queryInterface(const char * entityType, iid_t iid, bool softQuery) {
    int numItems = findByType(entityType);
    IBase * pInt = NULL;
    if (numItems > 0) {
        pInt = queryInterface(entityResult.front(), iid, true);
    }
    if (NULL==pInt && !softQuery) {
        if (numItems==0) throw Exception("core: Entity {type='%s'} not exists", entityType);
        const char * iname = getCore()->getIdName(iid);
        throw Exception("core: Entity {type='%s'} doesn't support Iterface{iid=0x%08X, name='%s'}", entityType, iid, iname);
    }
    return pInt;
}


int EntityMgr::findTypesForInterface(iid_t iid) {
    static std::vector<const char*> result;
    result.clear();
    for (unsigned int i=0; i < chunkType.size(); ++i) {
        const char * typeName = chunkType[i].name;
        if (NULL==typeName || chunkType[i].instances.empty()) continue;
        entid_t id = chunkType[i].instances[0];
        IBase * p = queryInterface(id, iid, true);
        if (NULL!=p) result.push_back(typeName);
    }
    strResult = result;
    return strResult.size();
}


void EntityMgr::copyStrResult(const char * * array) const {
    for (int i=0; i < strResult.size(); ++i) {
        array[i] = strResult[i];
    }
}


entid_t EntityMgr::getByType(const char * type) {
    MapTypeId::const_iterator it = mapTypeId.find(type);
    if (it==mapTypeId.end()) return -1;
    const EntityTypeChunk& tc=chunkType[it->second];
    if(tc.instances.empty()) return -1;
    return tc.instances[0];
}


int EntityMgr::findByType(const char * type) {
    entityResult = find_entity_t();
    if (NULL==type) return 0;
    MapTypeId::const_iterator it = mapTypeId.find(type);
    if (it==mapTypeId.end()) return 0;
    const EntityTypeChunk& tc=chunkType[it->second];
    entityResult = tc.instances;
    return entityResult.size();
}


void EntityMgr::copyEntityResult(entid_t * array) const {
    for (int i=0; i < entityResult.size(); ++i) {
        array[i] = entityResult[i];
    }
}


const char * EntityMgr::getEntityType(entid_t id) {
    checkId(id);
    return chunkType[chunkEntity[id].typeId].name;
}


void EntityMgr::sendEventEntity(entid_t entity, evtid_t event, Object * param) {
    if (entity<0) return;
    checkId(entity);
    const EntityChunk& ec=chunkEntity[entity];
    ec.entity->entityHandleEvent(event,-1,param);
}


void EntityMgr::sendEventType(const char * type, evtid_t event, Object * param) {
    if (NULL==type) {
        for (unsigned int i=0; i < chunkEntity.size(); ++i) {
            if (NULL!=chunkEntity[i].entity) sendEventEntity(i,event,param);
        }
    }
    else {
        MapTypeId::const_iterator it = mapTypeId.find(type);
        if (it==mapTypeId.end()) return;
        const EntityTypeChunk& tc=chunkType[it->second];
        for (unsigned int i=0; i < tc.instances.size(); ++i) {
            entid_t id = tc.instances[i];
            if (NULL!=chunkEntity[id].entity) sendEventEntity(id,event,param);
        }
    }
}


typeid_t EntityMgr::createChunkType(IComponent * c, const char* typeName) {

    // Validate type name
    const char * validTypeName = NULL;
    const EntityTypeInfo * s = c->getTypeInfo();
    for ( ; s && s->typeName; ++s ) {
        if (::stricmp(s->typeName, typeName)==0) {
            validTypeName = s->typeName;
            break;
        }
    }
    if (NULL==validTypeName) throw Exception("core: Can't determine entity type");

    entid_t id;
    if (freeChunkType.empty()) {
        // create new chunk
        id = (entid_t) chunkType.size();
        chunkType.push_back(EntityTypeChunk());
    }
    else {
        id = freeChunkEntity.top();
        freeChunkEntity.pop();
    }

    chunkType[id].comp = c;
    chunkType[id].name = validTypeName;
    chunkType[id].actMilli = 0;
    return id;

}


entid_t EntityMgr::createChunkEntity(EntityBase * entity, typeid_t typeId) {

    entid_t id;
    if (freeChunkEntity.empty()) {
        // create new chunk
        id = (entid_t) chunkEntity.size();
        chunkEntity.push_back(EntityChunk());
    }
    else {
        id = freeChunkEntity.top();
        freeChunkEntity.pop();
    }

    chunkEntity[id].entity = entity;
    chunkEntity[id].typeId = typeId;
    chunkEntity[id].lastMilli = ::clock();
    chunkEntity[id].actMilli = 0;
    chunkEntity[id].actRate  = ACTIVITY_RATE_NORMAL;
    chunkEntity[id].actAccumulate = 0;
    chunkEntity[id].children.push_back(-1);
    return id;
}


EntityMgr::~EntityMgr() {
    destroyAll();
}


void EntityMgr::destroyAll() {
    // Building list of entities
    std::vector<entid_t> ids;
    for (unsigned int i=0; i < chunkEntity.size(); ++i) {
        if (chunkEntity[i].entity!=NULL)
            ids.push_back(i);
    }
    while (!ids.empty()) {
        for (int i=ids.size()-1; i>=0; --i) {
            if (chunkEntity[ids[i]].entity!=NULL && chunkEntity[ids[i]].children.size()==1) {
                getCore()->destroyEntity(ids[i]);
                ids.erase(ids.begin()+i);
                break;
            }
        }
    }
}

//float dtConst = 1.0f/12.5f;

void EntityMgr::actEntities() {

    // Process trigger handlers for all entities
    for (unsigned i=0; i < chunkEntity.size(); ++i) {
        if (NULL!=chunkEntity[i].entity) {
            for (unsigned int j=0; i<chunkEntity.size() && j<chunkEntity[i].trigevt.size(); ++j) {
                EntityTriggerEvent evt=chunkEntity[i].trigevt[j];
                chunkEntity[i].entity->entityHandleEvent(-1,evt.id,evt.param);
            }
        }
    }
    // Act all entities
    // Also count time statistics for entity and for it's type
    // Also take into account entity's activity rate
    clock_t curMilli = -1;
    for (unsigned i=0; i < chunkEntity.size(); ++i) {
        EntityChunk& ec = chunkEntity[i];
        if (NULL!=ec.entity) {
            assert(ec.actRate >= 0);
            assert(ec.actRate <= ACTIVITY_RATE_MAX);
            if ((ec.actAccumulate += ec.actRate) >= ACTIVITY_RATE_MAX) {
                ec.actAccumulate -= ACTIVITY_RATE_MAX;
                if (curMilli<0) curMilli = ::clock();
                clock_t dc = curMilli - ec.lastMilli;
                if (dc>ACTIVITY_MAX_DELAY_CLOCK) dc=ACTIVITY_MAX_DELAY_CLOCK;
                float dt = dc * (1.0f/CLOCKS_PER_SEC);
                /*
                if( dt < dtConst )
                {
                    Sleep( DWORD( ( dtConst - dt ) * 1000.0f ) );
                    dt = dtConst;
                }
                */
                ec.lastMilli = curMilli;
                if (!ec.entity->entityEverActed()) {
                    getCore()->logMessage("core: acting '%s'[%d]",
                        chunkType[ec.typeId].name, ec.entity->getid());
                }
                ec.entity->entityAct(dt);
                const_cast<unsigned&>(chunkEntity[i].entity->entityNumActs)++;
                clock_t curMilli2 = ::clock();
                clock_t actTime = curMilli2 - curMilli;
                chunkEntity[i].actMilli += actTime;
                chunkType[chunkEntity[i].typeId].actMilli += actTime;
                curMilli = curMilli2;
                // update system timer
                TimeMgr::instance->getSystemTime()->advance(actTime * (1.0f/CLOCKS_PER_SEC));
            }
        }
    }
    /*
    // Manage vtbl transforms
    if (rand() % 16 == 0) {
        vtableDisplace.reset();
    }
    if (rand() % 16 == 0) {
        vtableDisplace.clear();
    }
    */
}


void EntityMgr::setActivity(entid_t id, float activityRate) {
    checkId(id);
    if (activityRate < 0) activityRate = 0;
    if (activityRate > 1) activityRate = 1;
    chunkEntity[id].actRate = (int)(2 * activityRate * ACTIVITY_RATE_NORMAL);
}


void EntityMgr::handleTrigger(entid_t id, trigid_t trigId, Object * param, bool immediate) {
    checkId(id);
    if (immediate) {
        const EntityChunk& ec=chunkEntity[id];
        ec.entity->entityHandleEvent(EvtTrigger::eventId,trigId,param);
    }
    else {
        EntityChunk& ec=chunkEntity[id];
        ec.trigevt.push_back(EntityTriggerEvent());
        ec.trigevt.back().id = trigId;
        ec.trigevt.back().param = param;
    }
}


}
