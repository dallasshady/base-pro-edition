/**
 * This source code is a part of Metathrone game project. 
 * (c) Perfect Play 2003.
 *
 * @author Sergey Alekhin
 */

#ifndef H522C7A53_BA9D_4201_AD86_7D20074643B2
#define H522C7A53_BA9D_4201_AD86_7D20074643B2
#include "../shared/ccor.h"
#include <ctime>
#include "ComponentMgr.h"
#include "VtableDisplace.h"
#include "find_result_t.h"
namespace ccor {

struct EntityTriggerEvent {
    trigid_t id;
    Object * param;
};

struct EntityTypeChunk {
    IComponent * comp;
    const char * name;
    std::vector<entid_t> instances;
    clock_t actMilli;
};

struct EntityChunk {
    EntityBase * entity;
    std::vector<entid_t> children;
    typeid_t typeId;
    clock_t lastMilli;
    clock_t actMilli;
    int actRate;
    int actAccumulate;
    std::vector<EntityTriggerEvent> trigevt;
};


/**
 * Manages entities creation, destroying, sending events
 */
class EntityMgr {
public:    

    EntityMgr() { }

    ~EntityMgr();

    entid_t createEntity(const char * type, entid_t idParent, Object * param);

    void destroyEntity(entid_t id);

    int findByType(const char * type);

    entid_t getByType(const char * type);

    void copyEntityResult(entid_t * array) const;

    IBase * queryInterface(entid_t eid, iid_t iid, bool softQuery);

    IBase * queryInterface(const char * entityType, iid_t iid, bool softQuery);

    int findTypesForInterface(iid_t iid);

    void copyStrResult(const char * * array) const;

    const char * getEntityType(entid_t id);

    void sendEventEntity(entid_t entity, evtid_t event, Object * param);

    void sendEventType(const char * type, evtid_t event, Object * param);

    void setActivity(entid_t id, float activityRate);

    void actEntities();

    void handleTrigger(entid_t id, trigid_t trigId, Object * param, bool immediate);

    void checkId(entid_t id) const { 
        if (unsigned(id)>=chunkEntity.size() || NULL==chunkEntity[id].entity)
            throw Exception("core: No such entity with id=%d", id); 
        assert(chunkEntity[id].typeId>=0);
    }

    void destroyAll();

private:

    /** @link dependency */
    /*# ComponentMgr lnkSingleComponentMgr; */

    typedef std::map<std::string, typeid_t> MapTypeId; 
    MapTypeId mapTypeId;
    std::vector<EntityChunk> chunkEntity;
    std::vector<EntityTypeChunk> chunkType;
    std::stack<entid_t, std::vector<entid_t> > freeChunkEntity;
    std::stack<typeid_t, std::vector<typeid_t> > freeChunkType;
    VtableDisplace vtableDisplace;
    find_entity_t entityResult;
    find_str_t strResult;

    typeid_t createChunkType(IComponent * c, const char* typeName);
    entid_t createChunkEntity(EntityBase * entity, typeid_t typeId);

};

}
#endif
