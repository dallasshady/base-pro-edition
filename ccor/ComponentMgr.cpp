/**
 * This source code is a part of Metathrone game project. 
 * (c) Perfect Play 2003.
 *
 * @description Component manager implementation.
 *
 * @author Sergey Alekhin
 */

#include "Headers.h"
#include "ComponentMgr.h"
#include "CoreImpl.h"
namespace ccor {


ComponentMgr * SingleComponentMgr::instance= 0;

ComponentMgr * SingleComponentMgr::getInstance() {
    if (instance == 0) {
        instance = new ComponentMgr();
    }
    return instance;
}

void SingleComponentMgr::releaseInstance() {
    delete instance;
    instance = 0;
}


ComponentMgr::ComponentMgr() {
    loadStaticComponents();
    refreshMapComp();
}


ComponentMgr::~ComponentMgr() {

    int cc = chunkComp.size();
    while (cc>0) {
        ComponentChunk& c = chunkComp[--cc];
        if (NULL!=c.comp) {
            removeComponent(c);
        }
    }

}

IComponent * ComponentMgr::getComponentForType(const char * entityType) {

    if (NULL==entityType) throw Exception("core: Invalid entity type [NULL]");

    // (**)++ Find component that supports specified entity type
    MapTypename::const_iterator it = mapTypename.find(entityType);
    if (it != mapTypename.end()) {
        return chunkComp[it->second].comp;
    }
    // (**)--

    // We've not found loaded component.
    // Try to load it; find component path first
    MapCstrStr::const_iterator it1 = mapCompPath.find(entityType);
    if (it1 == mapCompPath.end()) 
        throw Exception("core: No component found for entity type %s", entityType);

    // Try to load it
    const char * compPath = it1->second.c_str();
    if (!loadComponent(compPath, NULL)) {
        throw Exception("core: Can't load component %s", compPath);
    }

    // Find component chunk which should exist
    it = mapTypename.find(entityType);
    if (it == mapTypename.end()) 
        throw Exception("core: Component %s doesn't support entity type %s", 
            compPath, entityType);

    return chunkComp[it->second].comp;
}


void ComponentMgr::addComponent(const ComponentChunk& cc) {
    
    // make sure entities types are unique relative to other components
    const EntityTypeInfo * t = cc.typeInfo;
    for ( ; t && t->typeName; ++t) {
        if (mapTypename.find(t->typeName) != mapTypename.end()) {
            unloadComponent(cc);
            throw Exception("core: Component causes duplicate entity type %s", t->typeName);
        }
    }

    // store chunk 
    compid_t id;
    if (freeChunk.empty()) {
        id = (compid_t) chunkComp.size();
        chunkComp.push_back(cc);
    }
    else {
        id = freeChunk.top();
        freeChunk.pop();
        chunkComp[id] = cc;
    }

    // add types into map
    t = cc.typeInfo;
    for ( ; t && t->typeName; ++t) {
        // we may not copy t->typeName value into std::string because it exists 
        // all the time that mapTypename exists; so store it as is
        mapTypename.insert(MapTypename::value_type(t->typeName, id));
    }

}


void ComponentMgr::removeComponent(const ComponentChunk& cc) {

    // make sure all component's entities have been destroyed
    ensureNoEntityComponent(cc);

    // remove all types information
    const EntityTypeInfo * t = cc.typeInfo;
    for ( ; t && t->typeName; ++t) {
        mapTypename.erase(mapTypename.find(t->typeName));
    }
    
    // unload component
    IComponent * comp = cc.comp;
    for (unsigned int i=0; i < chunkComp.size(); ++i) {
        if (comp == chunkComp[i].comp) {
            chunkComp[i].comp->release();
            unloadComponent(chunkComp[i]);
            chunkComp[i].comp = NULL;
            freeChunk.push(i);
        }
    }

}


void ComponentMgr::ensureNoEntityComponent(const ComponentChunk& cc) {

    ICore * core = SingleCore::getInstance();
    if (NULL==core) return;
    const EntityTypeInfo * s = cc.typeInfo;
    for ( ; s && s->typeName; ++s) {
        while (true) {
            int numItems = core->findByType(s->typeName);
            if (numItems == 0) break;
            entid_t* ids = new entid_t[numItems];
            core->copyEntityResult( ids );
            core->logMessage("core: Automatically destroying entity '%s', id=0x%X", s->typeName, ids[0]);
            core->destroyEntity(ids[0]);
            //throw Exception("core: Component entity '%s' still exists", s->typeName);
			delete[] ids;
        }
    }
}


void ComponentMgr::refreshMapComp() {

    // Determine whether we're in debug mode
    // If yes, add '$debug' suffix to each component path
    bool inDebugMode = false;
    IParamPack * ppack = SingleCore::getInstance()->getCoreParamPack();
    paramid_t pid = ppack->find("com.debug");
    if (pid>=0) {
        ppack->get(pid,inDebugMode);
    }

    std::string path;
    std::string typeName;
    mapCompPath.clear();
    if (!ppack) return;
    int numItems = ppack->findParams("com.typeinfo*");
    paramid_t* items = new paramid_t[numItems];
    ppack->copyParamResult( items );
    for (int i=0; i < numItems; ++i) {
        const char * str = ppack->gets(items[i]);
        const char * name = ::strstr(str, ",");
        if(NULL!=name) {

            // try to find special config for this component 
            const char strRelease[] = "release:";
            const char strDebug[] = "debug:";
            bool debugComponent = inDebugMode;
            int numEqChars = ::strspn(str, strRelease);
            if (numEqChars>=sizeof(strRelease)-1) {
                str += sizeof(strRelease)-1;
                debugComponent = false;
            }
            numEqChars = ::strspn(str, strDebug);
            if (numEqChars>=sizeof(strDebug)-1) {
                str += sizeof(strDebug)-1;
                debugComponent = true;
            }

            // add suffix
            if (debugComponent)
            {
                char * s = ::strstr(str, ".");
                if (s && s < name) {
                    path.assign(str, s-str);
                    path.append("-d");
                    path.append(s, name-s);
                }
                else path.assign(str, name-str);
            }
            else path.assign(str, name-str);

            // find type names
            do {
                const char * nextDelim = ::strstr(name+1, ",");
                if (NULL!=nextDelim) {
                    typeName.assign(name+1, nextDelim-name-1);
                    name = nextDelim;
                }
                else {
                    typeName.assign(name+1);
                    name = NULL;
                }

                mapCompPath.insert(MapCstrStr::value_type(typeName,path));
            }
            while(NULL!=name);
        }
    }
	delete[] items;
}

}
