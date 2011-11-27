/**
 * This source code is a part of Metathrone game project. 
 * (c) Perfect Play 2003.
 *
 * @description Component manager implementation.
 *
 * @author Sergey Alekhin
 */

#ifndef H82DDCCF0_7B5A_44DF_9B27_AAD7D2C6BD0F
#define H82DDCCF0_7B5A_44DF_9B27_AAD7D2C6BD0F
#include "../shared/ccor.h"
namespace ccor {

struct ComponentChunk {
    std::string name;
    void * instance;
    IComponent * comp;
    const EntityTypeInfo * typeInfo;
};


typedef int typeid_t;

class ComponentMgr;


class SingleComponentMgr {
public:
    static ComponentMgr * getInstance();
    static void releaseInstance();

private:
    static ComponentMgr * instance;
};



class CaseInsensitiveString {
public:
    const char* s;
    CaseInsensitiveString() {}
    CaseInsensitiveString(const char* s) { this->s = s; }
    CaseInsensitiveString(const std::string& s) { this->s = s.c_str(); }
    CaseInsensitiveString& operator=(const CaseInsensitiveString& b) {
        s = b.s;
        return *this;
    }
    bool operator==(const CaseInsensitiveString& b) const {
        return ::stricmp(s, b.s)==0;
    }
    operator const char* () const {
        return s;
    }
};


struct CaseInsensitiveStringComparator {
    bool operator()(const CaseInsensitiveString& a, const CaseInsensitiveString& b) const {
        return ::stricmp(a.s, b.s) < 0;
    }
};

typedef std::map<CaseInsensitiveString, compid_t, CaseInsensitiveStringComparator> MapTypename;

typedef std::map<std::string, std::string, CaseInsensitiveStringComparator> MapCstrStr;


class ComponentMgr {
public:

    ComponentMgr();
    ~ComponentMgr();

    IComponent * getComponentForType(const char * entityType);

    /**
     * @link
     * @shapeType PatternLink
     * @pattern Singleton
     * @supplierRole Singleton factory 
     */
    /*# SingleComponentMgr _singleComponentMgr; */

private:

    std::vector<ComponentChunk> chunkComp;
    std::stack<compid_t,std::vector<compid_t> > freeChunk;
    MapTypename mapTypename;
    MapCstrStr mapCompPath;

    void addComponent(const ComponentChunk& cc);
    void removeComponent(const ComponentChunk& cc);
    void loadStaticComponents();
    bool loadComponent(const char * name, Object * param);
    void unloadComponent(const ComponentChunk& cc);
    void ensureNoEntityComponent(const ComponentChunk& cc);
    void refreshMapComp();

};


}


#endif
