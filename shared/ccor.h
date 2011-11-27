/**
 * This source code is a part of AqWARium game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description definitions for some game core interfaces
 *
 * @author asd
 */

#ifndef H76D3F6C4_EC9C_469A_8706_AC71F0CB2231
#define H76D3F6C4_EC9C_469A_8706_AC71F0CB2231
#include <ctime>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include "../shared/vector.h"
#include "../shared/matrix.h"
#include "../shared/product_version.h"
namespace ccor {

// Types that are declared in this file:
struct Object;
template <typename T> struct Holder;
struct ComponentInfo;
struct ManagedTriggerInfo;
class TriggerObject;
class EntityBase;
class Exception;
class ParamPackListener;
class IBase;
class ICore;
class IComponent;
class ITrigChecker;
class IParamPack;
class IParamPackFactory;
class IRandToolkit;
class ITime;
class SMLListener;

// Types for variant value 
enum VariantType
{
    vtBool,     // boolean
    vtInt,      // integer
    vtFloat,    // floating point
    vtVector2f, // 2d-array of floating points
    vtVector3f, // 3d-array of floating points
    vtVector4f, // 4d-array of floating points
    vtMatrix2f, // 2x2-array of floating points
    vtMatrix3f, // 3x3-array of floating points
    vtMatrix4f, // 4x4-array of floating points
    vtString    // const char*
};

// Variant value union 
union VariantValue
{
    bool        b;
    int         i;
    float       f;
    float       v[4];
    float       m[4][4];
    const char* str;
};

// Variant variable
struct Variant
{
public:
    VariantType  type;
    VariantValue value;
public:
    Variant(bool v) : type(vtBool) 
    { 
        value.b = v; 
    }
    Variant(int v) : type(vtInt) 
    { 
        value.i = v; 
    }
    Variant(float v) : type(vtFloat) 
    { 
        value.f = v; 
    }
    Variant(const Vector2f& v) : type(vtVector2f) 
    { 
        value.v[0] = v[0], value.v[1] = v[1]; 
        value.v[2] = value.v[3] = 0.0f;
    }
    Variant(const Vector3f& v) : type(vtVector3f)
    {
        value.v[0] = v[0], value.v[1] = v[1], value.v[2] = v[2];
        value.v[3] = 0.0f;
    }
    Variant(const Vector4f& v) : type(vtVector4f)
    {
        value.v[0] = v[0], value.v[1] = v[1], value.v[2] = v[2], value.v[3] = v[3];
    }
    Variant(const Matrix2f& v) : type(vtMatrix2f)
    {
        value.m[0][0] = v[0][0], value.m[0][1] = v[0][1], value.m[0][2] = 0.0f, value.m[0][3] = 0.0f;
        value.m[1][0] = v[1][0], value.m[1][1] = v[1][1], value.m[1][2] = 0.0f, value.m[1][3] = 0.0f;
        value.m[2][0] = 0.0f,    value.m[2][1] = 0.0f,    value.m[2][2] = 1.0f, value.m[2][3] = 0.0f;
        value.m[3][0] = 0.0f,    value.m[3][1] = 0.0f,    value.m[3][2] = 0.0f, value.m[3][3] = 1.0f;
    }
    Variant(const Matrix3f& v) : type(vtMatrix3f)
    {
        value.m[0][0] = v[0][0], value.m[0][1] = v[0][1], value.m[0][2] = v[0][2], value.m[0][3] = 0.0f;
        value.m[1][0] = v[1][0], value.m[1][1] = v[1][1], value.m[1][2] = v[1][2], value.m[1][3] = 0.0f;
        value.m[2][0] = v[2][0], value.m[2][1] = v[2][1], value.m[2][2] = v[2][2], value.m[2][3] = 0.0f;
        value.m[3][0] = 0.0f,    value.m[3][1] = 0.0f,    value.m[3][2] = 0.0f,    value.m[3][3] = 1.0f;
    }
    Variant(const Matrix4f& v) : type(vtMatrix4f)
    {
        value.m[0][0] = v[0][0], value.m[0][1] = v[0][1], value.m[0][2] = v[0][2], value.m[0][3] = v[0][3];
        value.m[1][0] = v[1][0], value.m[1][1] = v[1][1], value.m[1][2] = v[1][2], value.m[1][3] = v[1][3];
        value.m[2][0] = v[2][0], value.m[2][1] = v[2][1], value.m[2][2] = v[2][2], value.m[2][3] = v[2][3];
        value.m[3][0] = v[3][0], value.m[3][1] = v[3][1], value.m[3][2] = v[3][2], value.m[3][3] = v[3][3];
    }
    Variant(const char* v) : type(vtString)
    {
        value.str = v;
    }
public:
    operator bool() const { return value.b; }
    operator int() const { return value.i; }
    operator unsigned int() const { return value.i > 0 ? value.i : 0; }
    operator float() const { return value.f; }
    operator const char*() const { return value.str; }
    operator Vector2f() const 
    { 
        return Vector2f( value.v[0], value.v[1] ); 
    }
    operator Vector3f() const 
    { 
        return Vector3f( value.v[0], value.v[1], value.v[2] ); 
    }
    operator Vector4f() const 
    { 
        return Vector4f( value.v[0], value.v[1], value.v[2], value.v[3] ); 
    }
    operator Matrix2f() const 
    { 
        return Matrix2f( 
            value.m[0][0], value.m[0][1], 
            value.m[1][0], value.m[1][1]
        ); 
    }
    operator Matrix3f() const 
    { 
        return Matrix3f( 
            value.m[0][0], value.m[0][1], value.m[0][2], 
            value.m[1][0], value.m[1][1], value.m[1][2],
            value.m[2][0], value.m[2][1], value.m[2][2]
        ); 
    }
    operator Matrix4f() const 
    { 
        return Matrix4f( 
            value.m[0][0], value.m[0][1], value.m[0][2], value.m[0][3], 
            value.m[1][0], value.m[1][1], value.m[1][2], value.m[1][3], 
            value.m[2][0], value.m[2][1], value.m[2][2], value.m[2][3], 
            value.m[3][0], value.m[3][1], value.m[3][2], value.m[3][3]
        ); 
    }
};

// component identifier
typedef int compid_t;

// Entity id
typedef int entid_t;

// Interface id
typedef int iid_t;

// Entity event id
typedef int evtid_t;

// Parameter id
typedef int paramid_t;

// Trigger id
typedef int trigid_t;

// Global id (for interface, event or trigger)
typedef int globid_t;


// Time events types
enum TimeEventType {
    timeEvSimple,
    timeEvPulse,
    timeEvAccumulate
};


// Entity life time
enum EntityLifeTime {
    entityCreated,
    entityRelocated,
    entityPreDestroy,
    entityDestroyed
};


// Trigger creation flags
enum TriggerFlags {
    trigImmediate   = 0x1,  // immediate trigger
    trigValueable   = 0x2,  // trigger has an attached value
    trigManaged     = 0x4   // trigger automatically created/destroyed by core
};


// Id declare macros
#define DECLARE_INTERFACE_ID(x) enum _Id { iid=x };
#define DECLARE_TRIGGER_ID(x) enum _Id { tid=x };
#define DECLARE_EVENT_ID(x) enum _Id { eventId=x };

/**
 * Object is the base structure for all data objects
 */
struct Object { };

/**
 * IBase : Base class for all queriable interfaces.
 * Interface can be queried from an entity that supports it.
 */
class IBase {
protected:
    virtual void __stdcall _forcevirtual() { }
};


/**
 * Basic Holder object for types not derived from Object,
 * such as int, float etc.
 *
 * Example of use: 
 *
 *      Holder<const char*> obj = "Hello !";
 *      core->sendEventEntity(id, evtMyEvent, &obj);
 */
template <typename T> 
struct Holder : public Object {
    Holder() {}
    Holder(const T& v) : value(v) { } 
    Holder(const Holder<T>& obj) { value = obj.value; }
    operator T() const { return value; }
    T value;
};


typedef Holder<bool>           HolderBool;
typedef Holder<int>            HolderInt;
typedef Holder<float>          HolderFloat;
typedef Holder<const char*>    HolderString;


/**
 * EntityBase is the base class for all entities
 */
class EntityBase {
public:

    entid_t getid() const { return entityId; }

    // Interface to core (set up by core). Don't modify this field!
    static ICore * icore;

protected:
    friend class EntityMgr;
    friend class TriggerMgr;

    /**
     * Init entity (is called automatically by core).
     * This function should be reimplemented in derived class.
     * Note! This function should be called by core only.
     *
     * @param p The same paramter that was passed to component's createEntity().
     * @throw Exception if initialization fails.
     */
    virtual void __stdcall entityInit(Object * p) { }

    /**
     * Execute entity actions.
     * This function should be reimplemented in derived class.
     * Note! This function should be called by core only.
     *
     * @param dt Seconds passed between the two consistent calls
     */
    virtual void __stdcall entityAct(float dt) { }

    /**
     * Handle core event. This function should be reimplemented in derived class.
     * Note! This function should be called by core only.
     *
     * @param id Event id
     * @param evtData Object with event data
     */
    virtual void __stdcall entityHandleEvent(evtid_t id, trigid_t trigId, Object * param) { }

    /**
     * Ask entity for interface with specified id.
     * Note! This function should be called by core only.
     *
     * @param id Interface id
     * @return Pointer to interface or NULL if the one is not supported
     */
    virtual IBase * __stdcall entityAskInterface(iid_t id) { return 0; }

    /**
     * Destroy entity (just implement as 'delete this')
     */
    virtual void __stdcall entityDestroy() = 0;

    // Check whether entity has ever acted
    bool entityEverActed() const                { return entityNumActs > 0; }

    // Get number of acts
    unsigned entityGetNumActs() const                { return entityNumActs; }

    // Core entity identifier (set up by core).
    const entid_t entityId;

    // Parent entity identifier (set up by core).
    const entid_t parentId;

    // Children entities ids (set up by core). This array ends with -1
    const entid_t * const childId;

    EntityBase() : entityId(-1), parentId(-1), childId(0), entityNumActs(0) { }

private:

    // Number of completed calls to entityAct() (managed up by core).
    const unsigned entityNumActs;

};



#ifdef _MSC_VER
#pragma warning ( disable : 4096 )
#endif

/**
 * Exception class is used to store and format exception message 
 * in order to pass it to exception handler
 *
 * Example of use: throw Exception("core: something bad happened");
 */
class Exception {
public:
    enum {msgMaxLen = 512};
    Exception()                               { ::strcpy(msg, "<no exception string>"); }
    Exception(const char * fmt, ...)          { va_list vl; va_start(vl, fmt); ::vsprintf(msg, fmt, vl); va_end(vl); }
    operator const char * () const            { return msg; }
    const char * getMsg() const               { return msg; }
    void setMsg(const char * fmt, va_list vl) { ::vsprintf(msg, fmt, vl); }

private:
    char msg[msgMaxLen];
};

#ifdef _MSC_VER
#pragma warning ( default : 4096 )
#endif


// Type-pointer to function that creates entity object
typedef EntityBase * (*entity_creator_t)();


// Pair <entity_type_name, entity_creator>
struct EntityTypeInfo {
    const char * typeName;
    entity_creator_t creator;
};


// Type-pointer to component initialization function 
typedef IComponent * (__cdecl *CoreInitComponentProc)(ICore * core);


/**
 * Interface for component (must be implemented by each component).
 *
 * Component is a DLL that holds one or more entities implementations. 
 * Each entity has it's type name, which should be unique for the whole
 * system (taking type name case insensibility into account). Each entity
 * can implement one or more interfaces.
 *
 * Component's DLL must implement the folowing function:
 *     extern "C" __declspec(dllexport) IComponent * coreInitComponent(ICore * core);
 * The one is called by core to initialize component and get it's IComponent interface.
 *
 */
class IComponent {
public:
    DECLARE_INTERFACE_ID(0x10000);
    /**
     * Get component label string
     * @return Label string
     */
    virtual const char * __stdcall getComponentLabel() = 0;

    virtual const EntityTypeInfo * __stdcall getTypeInfo() = 0;

    /**
     * Free up component
     * This function is called by core when the component is to be unloaded.
     * You may be sure all component's entities have been destroyed before the call.
     */
    virtual void __stdcall release() = 0;

};



/**
 *  Here are macroses to simplify component's entities declaration
 */

inline ICore * getCore() { return EntityBase::icore; }

#define SIMPLE_COMPONENT_BEGIN(name) \
    extern EntityTypeInfo name##TypeInfo[]; \
    class name##Component : public ccor::IComponent { \
    public: \
        virtual void __stdcall release() {} \
        virtual const char * __stdcall getComponentLabel() { return #name " v" PRODUCT_VERSION_LABEL " " __DATE__ " " __TIME__; } \
        virtual const EntityTypeInfo * __stdcall getTypeInfo() { return name##TypeInfo; } \
    }; \
    name##Component name##_component; \
    ICore * ccor::EntityBase::icore = 0; \
    extern "C" __declspec(dllexport) IComponent * __cdecl coreInitComponent(ICore * core) \
        { ccor::EntityBase::icore = core; return &name##_component; } \
    EntityTypeInfo name##TypeInfo[] = {

#define DECLARE_COMPONENT_ENTITY(name)    { #name, name::creator },

#define SIMPLE_COMPONENT_END     { 0, 0 } }

#define SINGLE_ENTITY_COMPONENT(name)  \
    SIMPLE_COMPONENT_BEGIN(name) \
        DECLARE_COMPONENT_ENTITY(name) \
    SIMPLE_COMPONENT_END


/**
 * Resource provides access to files on disk, compressed files and files located in memory
 */
class IResource {
public:

    DECLARE_INTERFACE_ID(0x10001);

    virtual FILE * __stdcall getFile() = 0;

    virtual const char * __stdcall getName() = 0;

    virtual void __stdcall release() = 0;
};




/**
 * Game core provides major functions for application via ICore interface.
 *
 * There are:
 *
 * 1. Entity management. 
 *    Ability to create, destroy, find entities and set their activity.
 *
 * 2. Global ids management. 
 *    Global id is just identifier for interface, trigger or event.
 *    Each id has its unique name.
 *
 * 3. Event management. 
 *    Ability to send events to entities.
 *
 * 4. Support for interfaces. 
 *    Interface is C++ class with all functions are pure virtual 
 *    and with a unique global id. Entity can support none, one or more interfaces.
 *    Likewise ActiveX/COM technology, interface can be queried (asked) 
 *    from entity that supports it.
 *
 * 5. Support for triggers.
 *    Trigger is object wich can be 'activated'. When trigger has been activated,
 *    all it's listener entities will receive a special message about this.
 *    Entity can subscribe/unsubscribe to listen the event about trigger activation.
 *    More, trigger can have value assotiated with it. The type of the value
 *    is defined in interface part of component that is trigger's owner.
 *    
 * 6. Support for resources.
 *    Resource is a named file likewise oridnary file on disk, .zip-archive or
 *    in-memory file. You should access such resources via IResource interface 
 *    functionality and via generic FILE structure, calling generic i/o routines 
 *    such as fread, fscanf, fprintf and so on.
 *    Don't use fopen() and fclose() on such files!!! Use core functions instead.
 *    @see IResource
 *
 * 7. Support for parameters.
 *    You can store packs of named parameters in external resources (like old .ini
 *    files or more progressive .config files). IParamPack is used to operate 
 *    with such packs of parameters. Core also has it's own Core Param Pack.
 *    @see IParamPack
 *    @see IParamPackFactory
 *
 * 8. Debugging & logging
 *
 * 9. System functions.
 *
 */
class ICore {
public:

    DECLARE_INTERFACE_ID(0x10002);

//
// Entity management
// 

    /**
     * Create entity by type name.
     * @param type Entity type name
     * @param idParent Parent entity id
     * @param param This argument will be passed to entity constructor
     * @return Created entity's id
     * @throws Exception when error creating entity
     */
    virtual entid_t __stdcall createEntity(const char * type, entid_t idParent, Object * param) = 0;

    /** 
     * Destroy entity.
     * @param id Entity id
     * @throws Exception when error
     */
    virtual void __stdcall destroyEntity(entid_t id) = 0;

    /**
     * Find entities by type name.
     * @param type Entity type name
     * @return Number of entities of entities found
     */
    virtual int __stdcall findByType(const char * type) = 0;

    /**
     * Get first entity by type name.
     * @param type Entity type name
     * @return Entity id or -1 if not found
     */
    virtual entid_t __stdcall getByType(const char * type) = 0;

    /**
     * Copy findByType() result into specified memory location.
     * You must ensure that size of array is wide enough to store the result.
     * @param array memory to copy result in
     */
    virtual void __stdcall copyEntityResult(entid_t * array) = 0;

    /**
     * Get entity type name
     * @return Entity type name
     * @throws Exception if invalid id
     */
    virtual const char * __stdcall getEntityType(entid_t id) = 0;

    /**
     * Set entity's activity (rate at which act() function will be called)
     * @param id Entity id
     * @param activityRate Float value from 0.0(fully non-active) to 1.0(highest activity)
     */
    virtual void __stdcall setActivity(entid_t id, float activityRate) = 0;

//
// Global ids management
//

    /**
     * Find global id by its name
     *
     * @param name Id name (must start with 'trigger.', 'interface.' or 'event.')
     * @return Global id (which is trigger id, interface id or event id) or -1 if not found
     */
    virtual globid_t __stdcall findId(const char * name) = 0;

    /**
     * Get global id name
     *
     * @param id Global id
     * @return Immediate pointer to string buffer
     * @throw Exception if id is invalid
     */
    virtual const char * __stdcall getIdName(globid_t id) = 0;

//
// Event management
//

    /**
     * Send event to specified entity
     * @param entId Entity id
     * @param evtId Event id
     * @param param Event data
     * @throws Exception when error
     */
    virtual void __stdcall sendEventEntity(entid_t entId, evtid_t evtId, Object * param) = 0;

    /**
     * Send event to all entities of specified type. If there is no entity 
     * with such a type, no action will be performed.
     * This function invalidates findByType/findNext state.
     * @param type Entities type name wich to send event to. NULL will cause send event to all entities of all types.
     * @param evtId Event id
     * @param param Event data 
     */
    virtual void __stdcall sendEventType(const char * type, evtid_t evtId, Object * param) = 0;

//
// Interface support
//

    /**
     * Ask entity for interface with specified id
     * @sa template <typename T> askInterface
     *
     * @param eid Entity id
     * @param iid Interface id
     * @return _Immediate_ pointer to interface or NULL if the one is not supported
     */
    virtual IBase * __stdcall askInterface(entid_t eid, iid_t iid) = 0;

    /**
     * Ask entity for interface with specified id
     * @sa template <typename T> askInterface
     *
     * @param entityType Entity type name
     * @param iid Interface id
     * @return _Immediate_ pointer to interface or NULL if the one is not supported
     */
    virtual IBase * __stdcall askInterface(const char * entityType, iid_t iid) = 0;

    /**
     * Query entity for interface with specified id
     * This function is identical to askInterface(), but it throws Exception 
     * in the case interface is not supported by entity
     * @sa template <typename T> queryInterface
     *
     * @param eid Entity id
     * @param iid Interface id
     * @return _Immediate_ pointer to interface
     * @throws Exception when interface is not supported     
     */
    virtual IBase * __stdcall queryInterface(entid_t eid, iid_t iid) = 0;

    /**
     * Query entity for interface with specified id
     * This function is identical to askInterface(), but it throws Exception 
     * in the case interface is not supported by entity
     * @sa template <typename T> queryInterface
     *
     * @param entityType Entity type name
     * @param iid Interface id
     * @return _Immediate_ pointer to interface
     * @throws Exception when interface is not supported     
     */
    virtual IBase * __stdcall queryInterface(const char * entityType, iid_t iid) = 0;

    /**
     * Find all entities types that support specified interface
     * Note! This function asks each first instanced entity of each type,
     * which can take quite long time.
     * @param iid Interface to check support
     * @return Number of type names wich support specified interface
     */
    virtual int __stdcall findTypesForInterface(iid_t iid) = 0;

    /**
     * Copy findTypesForInterface() result into specified memory location.
     * You must ensure that size of array is wide enough to store the result.
     * @param array memory to copy result in
     */
    virtual void __stdcall copyStrResult(const char** array) = 0;

//
// Time support
// 

    /**
     * Create named time
     *
     * @param name Name of the time, case-insensitive
     * @param eid Parent entity id (-1 if none). When entity is deleted, all times
     *        which are child for this entity are also deleted.
     * @param evType Type of time events
     * @return Interface to created time object
     */
    virtual ITime * __stdcall createTime(const char * name, entid_t eid, TimeEventType evType) = 0;

    /**
     * Get interface for time by name
     *
     * @param name Name of the time, case-insensitive ("system" for system time)
     * @return Interface for time or NULL if name not found
     */
    virtual ITime * __stdcall getTime(const char * name) = 0;

    /**
     * Get interface for system time
     *
     * @return Interface for system time (always not NULL)
     */
    virtual ITime * __stdcall getSystemTime() = 0;

//
// Trigger support
//

    /**
     * Create new trigger
     *
     * @param name Trigger id (wich is global id with name started with 'trigger.')
     * @param flags A combination of TriggerFlags (@see TriggerFlags)
     * @param value Trigger value for valuable trigger
     * @throws Exception if trigger with such id already exists
     */
    virtual void __stdcall createTrigger(trigid_t id, int flags, Object * value) = 0;

    /**
     * Destroy trigger
     *
     * @param id Trigger id
     * @throw Exception if invalid id
     */
    virtual void __stdcall destroyTrigger(trigid_t id) = 0;

    /**
     * Set trigger value (for valuable trigger)
     *
     * @param id Trigger id
     * @param value Value object (may be NULL)
     * @throw Exception if the value cannot be assigned (for non-valuable trigger)
     */
    virtual void __stdcall setTriggerValue(trigid_t id, Object * value) = 0;

    /**
     * Get trigger value (for valuable trigger)
     *
     * @param id Trigger id
     * @return Value object (may be NULL)
     * @throw Exception if the value cannot be assigned (for non-valuable trigger)
     */
    virtual Object * __stdcall getTriggerValue(trigid_t id) = 0;

    /**
     * Add listener for trigger object(s)
     * Custom-checker is an entity with function entityHandleEvent() overriden
     * which handles triggering events.
     *
     * @param templateName Name for newly created trigger or template for existing triggers
     * @param id Listener entity for this trigger
     * @param immediate Flag shows whether the immediate notification is required
     */
    virtual void __stdcall trigAddListener(trigid_t trigId, entid_t id) = 0;

    /**
     * Remove listener for trigger object(s)
     * 
     * @param templateName Name for existing triggers
     * @param id Listener entity for this trigger
     */
    virtual void __stdcall trigRemoveListener(trigid_t trigId, entid_t id) = 0;

    /**
     * Activate trigger object(s)
     * 
     * @param templateName Name for existing triggers
     * @param data Notification data that will be passed directly to handlers
     */
    virtual void __stdcall activate(trigid_t trigId, Object * param) = 0;

//
// Resource support
//

    /**
     * Get resource by name
     * @param resourceName Name of resource
     * @param mode Type of access to resource - combination of the following characters: \
     *     'r' - opens for reading, \
     *     'w' - opens for writing, \
     *     't' - opens in text mode (the default mode), \
     *     'b' - opens in binary mode.
     * @param type Type of resource
     * @throws Exception if cannot open resource or unallowed type of access is specified
     */
    virtual IResource * __stdcall getResource(const char * resourceName, const char * mode, const char * type = 0) = 0;

    /**
     * Get time of last modification of resource.
     * @param resourceName Name of resource
     * @return Time of last modification of resource or 0 if resource with such a name doesn't exist.
     */
    virtual time_t __stdcall getResourceTime(const char * resourceName) = 0;

//
// Support for parameters
//

    /**
     * Get parameter pack factory
     * @return Param pack factory interface
     */
    virtual IParamPackFactory * __stdcall getParamPackFactory() = 0;

    /**
     * Get core paremeter pack
     * @return Continual pointer to core parameter pack object
     */
    virtual IParamPack * __stdcall getCoreParamPack() = 0;

//
// Debugging & logging
//

    /**
     * Write message to the log
     * @param fmt Message format string
     */
    inline void __cdecl logMessage(const char * fmt, ...);

    /**
     * Write message to the log
     * @param fmt Message format string
     * @param vl Message parameters (volatile argument list)
     */
    virtual void __stdcall logMessageV(const char * fmt, va_list vl) = 0;

//
// System functions & other
//

    /**
     * Exit process
     * @param code Exit code
     */
    virtual void __stdcall exit(int code) = 0;

    /**
     * Process text in SML language
     *
     * @param smlText simply marked-up text 
     * @param np Node listener 
     */
    virtual void __stdcall processSML(const char * smlText, SMLListener * listener) = 0;

    /** 
     * Get random numbers generator toolkit
     */
    virtual IRandToolkit * __stdcall getRandToolkit() = 0;

    /**
     * Get core extended data
     * @return Pointer to data object
     */
    virtual Object * __stdcall getXData() = 0;

    /**
     * Set core extended data
     * @param p Pointer to data object
     */
    virtual void __stdcall setXData(Object * p) = 0;

};



/**
 * Interface for object that checks triggers itself
 */
class ITrigChecker {
public:

    DECLARE_INTERFACE_ID(0x10003);

    /*
     * Check the state of the custom-checking trigger.
     *
     * @param id Id of the trigger
     * @param data Reference to store data object that will be passed to handler
     * @return True to throw trigger event, false otherwise
     */
    virtual bool __stdcall onCheckTrigger(trigid_t id, Object*& data) = 0;
};


/**
 * Interface to manage pack of parameters.
 * You can create new empty instance or load it from text resource.
 * Also you can store the existing pack into text resource.
 */
class IParamPackFactory {
public:

    DECLARE_INTERFACE_ID(0x10004);

    /**
     * Create empty parameter pack
     * @return Interface to a newly created parameter pack 
     */
    virtual IParamPack * __stdcall createInstance() = 0;

    /**
     * Load parameter pack resource. 
     * @param ppack Interface to pack object
     * @param type Pack resource type
     * @return Interface to a newly created parameter pack 
     * @throws Exception when i/o error occurs
     */
    virtual IParamPack * __stdcall load(const char * name, const char * type = NULL) = 0;

    /**
     * Load parameter pack resource. 
     * @param ppack Pack resource type
     * @param name Pack resource name
     * @param append Indicates if new params should be appended to existing ones
     * @param type Pack resource type
     * @return Interface to a newly created parameter pack 
     * @throws Exception when i/o error occurs
     */
    virtual void __stdcall load(IParamPack * ppack, const char * name, bool append = false, const char * type = NULL) = 0;

    /**
     * Save parameter pack
     * @param ppack Interface to pack object
     * @param name Pack resource name. If the one is NULL, name of resource \
     *    is considered the same it had been loaded from.
     * @throws Exception when i/o error occurs
     */
    virtual void __stdcall save(IParamPack * ppack, const char * name = 0) = 0;

    /**
     * Update parameter pack 
     * (reload data from resource or save it, depends on what is newer).
     * @param ppack Interface to pack object
     * @throws Exception when i/o error occurs
     */
    virtual void __stdcall update(IParamPack * ppack) = 0;
};



/**
 * Pack of parameters
 *
 *  Example of use:
 *
 *  IParamPack * p = g_comCore->getParamPackFactory()->createInstance();
 *  p->set("vfs.enable.cache"), true);
 *  p->set("vfs.enable.cache.input"), true);
 *  p->set("vfs.cache.maxrecordsize", 10000);
 *  p->release();
 *
 */
class IParamPack {
public:

    DECLARE_INTERFACE_ID(0x10005);

    /**
     * Get number of parameters in this pack.
     * @return Number of paramteres
     */
    virtual int __stdcall getNumParams() = 0;

    /**
     * Get parameter name by it's id
     * @return Immediate pointer to parameter name
     */
    virtual const char * __stdcall getName(paramid_t id) = 0;

    /**
     * Check whether this pack is readonly. Any attempt
     * to modify a paramter in readonly pack will cause Exception.
     * @return Readonly flag
     * (not yet implemented)
     */
    virtual bool __stdcall isReadonly() = 0;

    /**
     * Protect/unprotect this object from being modified
     * @param readOnly Readonly flag
     * (not yet implemented)
     */
    virtual void __stdcall setReadonly(bool readOnly) = 0;

    /**
     * Get text value 
     * @param id Parameter id
     * @return Immediate pointer to string
     * @throw Exception when id is invalid
     */
    virtual const char * __stdcall gets(paramid_t id) = 0;

    /**
     * Get text value 
     * @param name Parameter name
     * @return Immediate pointer to string
     * @throw Exception when no such parameter
     */
    virtual const char * __stdcall gets(const char * name) = 0;

    /**
     * Get text value 
     * @param name Parameter name
     * @param defaultValue Return it when no such parameter
     * @return Immediate pointer to string
     */
    virtual const char * __stdcall getv(const char * name, const char * defaultValue) = 0;

    /**
     * Get text value 
     * @param id Parameter id
     * @param defaultValue Return it when id is invalid
     * @return Immediate pointer to string
     */
    virtual const char * __stdcall getv(paramid_t id, const char * defaultValue) = 0;

    /**
     * Get int value 
     * @param id Parameter id
     * @param value Reference to variable that receives paramer value
     * @throw Exception when id is invalid
     */
    virtual void __stdcall get(paramid_t id, int& value) = 0;

    /**
     * Get int value 
     * @param name Parameter name
     * @param value Reference to variable that receives paramer value
     * @throw Exception when no such parameter
     */
    virtual void __stdcall get(const char * name, int& value) = 0;

    /**
     * Get int value 
     * @param id Parameter id
     * @param defaultValue Return it when id is invalid
     * @return Parameter value
     */
    virtual int __stdcall getv(paramid_t id, int defaultValue) = 0;

    /**
     * Get int value 
     * @param name Parameter name
     * @param defaultValue Return it when no such parameter
     * @return Parameter value
     */
    virtual int __stdcall getv(const char * name, int defaultValue) = 0;

    /**
     * Get bool value
     * @param id Parameter id
     * @param value Reference to variable that receives paramer value
     * @throw Exception when id is invalid
     */
    virtual void __stdcall get(paramid_t id, bool& value) = 0;

    /**
     * Get bool value
     * @param name Parameter name
     * @param value Reference to variable that receives paramer value
     * @throw Exception when no such parameter
     */
    virtual void __stdcall get(const char * name, bool& value) = 0;

    /**
     * Get bool value 
     * @param id Parameter id
     * @param defaultValue Return it when id is invalid
     * @return Parameter value
     */
    virtual bool __stdcall getv(paramid_t id, bool defaultValue) = 0;

    /**
     * Get bool value 
     * @param name Parameter name
     * @param defaultValue Return it when no such parameter
     * @return Parameter value
     */
    virtual bool __stdcall getv(const char * name, bool defaultValue) = 0;

    /**
     * Get float value 
     * @param id Parameter id
     * @param value Reference to variable that receives paramer value
     * @throw Exception when id is invalid
     */
    virtual void __stdcall get(paramid_t id, float& value) = 0;

    /**
     * Get float value 
     * @param name Parameter name
     * @param value Reference to variable that receives paramer value
     * @throw Exception when no such parameter
     */
    virtual void __stdcall get(const char * name, float& value) = 0;

    /**
     * Get float value 
     * @param id Parameter id
     * @param defaultValue Return it when id is invalid
     * @return Parameter value
     */
    virtual float __stdcall getv(paramid_t id, float defaultValue) = 0;

    /**
     * Get float value 
     * @param name Parameter name
     * @param defaultValue Return it when no such parameter
     * @return Parameter value
     */
    virtual float __stdcall getv(const char * name, float defaultValue) = 0;

    /**
     * Get Object pointer value 
     * @param id Parameter id
     * @param value Reference to variable that receives paramer value
     * @throw Exception when id is invalid
     */
    virtual void __stdcall get(paramid_t id, Object *& value) = 0;

    /**
     * Get Object pointer value 
     * @param name Parameter name
     * @param value Reference to variable that receives paramer value
     * @throw Exception when no such parameter
     */
    virtual void __stdcall get(const char * name, Object *& value) = 0;

    /**
     * Get Object pointer value 
     * @param id Parameter id
     * @param defaultValue Return it when id is invalid
     * @return Parameter value
     */
    virtual Object * __stdcall getv(paramid_t id, Object * defaultValue) = 0;

    /**
     * Get Object pointer value 
     * @param name Parameter name
     * @param defaultValue Return it when no such parameter
     * @return Parameter value
     */
    virtual Object * __stdcall getv(const char * name, Object * defaultValue) = 0;

    /**
     * Get Vector2<float> value
     * @param id Parameter id
     * @param value Reference to variable that receives paramer value
     * @throw Exception when id is invalid
     */
    virtual void __stdcall get(paramid_t id, Vector2f& value) = 0;

    /**
     * Get Vector2<float> value
     * @param name Parameter name
     * @param value Reference to variable that receives paramer value
     * @throw Exception when no such parameter
     */
    virtual void __stdcall get(const char * name, Vector2f& value) = 0;

    /**
     * Get Vector2<float> value 
     * @param id Parameter id
     * @param defaultValue Return it when id is invalid
     * @return Parameter value
     */
    virtual Vector2f __stdcall getv(paramid_t id, Vector2f defaultValue) = 0;

    /**
     * Get Vector2<float> value 
     * @param name Parameter name
     * @param defaultValue Return it when no such parameter
     * @return Parameter value
     */
    virtual Vector2f __stdcall getv(const char * name, Vector2f defaultValue) = 0;

    /**
     * Get Vector3<float> value
     * @param id Parameter id
     * @param value Reference to variable that receives paramer value
     * @throw Exception when id is invalid
     */
    virtual void __stdcall get(paramid_t id, Vector3f& value) = 0;

    /**
     * Get Vector3<float> value
     * @param name Parameter name
     * @param value Reference to variable that receives paramer value
     * @throw Exception when no such parameter
     */
    virtual void __stdcall get(const char * name, Vector3f& value) = 0;

    /**
     * Get Vector3<float> value 
     * @param id Parameter id
     * @param defaultValue Return it when id is invalid
     * @return Parameter value
     */
    virtual Vector3f __stdcall getv(paramid_t id, const Vector3f& defaultValue) = 0;

    /**
     * Get Vector3<float> value 
     * @param name Parameter name
     * @param defaultValue Return it when no such parameter
     * @return Parameter value
     */
    virtual Vector3f __stdcall getv(const char * name, const Vector3f& defaultValue) = 0;

    /**
     * Get Vector4<float> value
     * @param id Parameter id
     * @param value Reference to variable that receives paramer value
     * @throw Exception when id is invalid
     */
    virtual void __stdcall get(paramid_t id, Vector4f& value) = 0;

    /**
     * Get Vector4<float> value
     * @param name Parameter name
     * @param value Reference to variable that receives paramer value
     * @throw Exception when no such parameter
     */
    virtual void __stdcall get(const char * name, Vector4f& value) = 0;

    /**
     * Get Vector4<float> value 
     * @param id Parameter id
     * @param defaultValue Return it when id is invalid
     * @return Parameter value
     */
    virtual Vector4f __stdcall getv(paramid_t id, const Vector4f& defaultValue) = 0;

    /**
     * Get Vector4<float> value 
     * @param name Parameter name
     * @param defaultValue Return it when no such parameter
     * @return Parameter value
     */
    virtual Vector4f __stdcall getv(const char * name, const Vector4f& defaultValue) = 0;

    /**
     * Set text value 
     * @param id Parameter id
     * @param text String to copy value from
     * @throw Exception when id is invalid or pack id read-only
     */
    virtual void __stdcall set(paramid_t id, const char * text) = 0;

    /**
     * Set text value 
     * @param name Parameter name
     * @param text String to copy value from
     * @throw Exception when no such parameter or pack id read-only
     */
    virtual void __stdcall set(const char * name, const char * text) = 0;

    /**
     * Set bool value 
     * @param id Parameter id
     * @param value Value to be set
     * @throw Exception when id is invalid or pack id read-only
     */
    virtual void __stdcall set(paramid_t id, bool value) = 0;

    /**
     * Set bool value 
     * @param name Parameter name
     * @param value Value to be set
     * @throw Exception when no such parameter or pack id read-only
     */
    virtual void __stdcall set(const char * name, bool value) = 0;

    /**
     * Set int value 
     * @param id Parameter id
     * @param value Value to be set
     * @throw Exception when id is invalid or pack id read-only
     */
    virtual void __stdcall set(paramid_t id, int value) = 0;

    /**
     * Set int value 
     * @param name Parameter name
     * @param value Value to be set
     * @throw Exception when no such parameter or pack id read-only
     */
    virtual void __stdcall set(const char * name, int value) = 0;

    /**
     * Set float value 
     * @param id Parameter id
     * @param value Value to be set
     * @throw Exception when id is invalid or pack id read-only
     */
    virtual void __stdcall set(paramid_t id, float value) = 0;

    /**
     * Set float value 
     * @param name Parameter name
     * @param value Value to be set
     * @throw Exception when no such parameter or pack id read-only
     */
    virtual void __stdcall set(const char * name, float value) = 0;

    /**
     * Set Object pointer value
     * @param id Parameter id
     * @param value Value to be set
     * @throw Exception when id is invalid or pack id read-only
     */
    virtual void __stdcall set(paramid_t id, Object * value) = 0;

    /**
     * Set Object pointer value
     * @param name Parameter name
     * @param value Value to be set
     * @throw Exception when no such parameter or pack id read-only
     */
    virtual void __stdcall set(const char * name, Object * value) = 0;

    /**
     * Set Vector2<float> value
     * @param id Parameter id
     * @param value Value to be set
     * @throw Exception when id is invalid or pack id read-only
     */
    virtual void __stdcall set(paramid_t id, const Vector2f& vector) = 0;

    /**
     * Set Vector2<float> value
     * @param name Parameter name
     * @param value Value to be set
     * @throw Exception when no such parameter or pack id read-only
     */
    virtual void __stdcall set(const char * name, const Vector2f& vector) = 0;
    
    /**
     * Set Vector3<float> value
     * @param id Parameter id
     * @param value Value to be set
     * @throw Exception when id is invalid or pack id read-only
     */
    virtual void __stdcall set(paramid_t id, const Vector3f& vector) = 0;
    
    /**
     * Set Vector3<float> value
     * @param name Parameter name
     * @param value Value to be set
     * @throw Exception when no such parameter or pack id read-only
     */
    virtual void __stdcall set(const char * name, const Vector3f& vector) = 0;

    /**
     * Set Vector4<float> value
     * @param id Parameter id
     * @param value Value to be set
     * @throw Exception when id is invalid or pack id read-only
     */
    virtual void __stdcall set(paramid_t id, const Vector4f& vector) = 0;

    /**
     * Set Vector4<float> value
     * @param name Parameter name
     * @param value Value to be set
     * @throw Exception when no such parameter or pack id read-only
     */
    virtual void __stdcall set(const char * name, const Vector4f& vector) = 0;

    /**
     * Find parameter by name
     * @param qualifiedName Qualified name for parameter
     * @return Parameter id or -1 if no such parameter
     */
    virtual paramid_t __stdcall find(const char * qualifiedName) = 0;

    /**
     * Find parameter by name
     * @param templateName Parameter template (may contain '*')
     * @return Pair<number of params, array of their ids>
     */
    virtual int __stdcall findParams(const char * templateName) = 0;

    /**
     * Copy findParams() result into specified memory location.
     * You must ensure that size of array is wide enough to store the result.
     * @param array memory to copy result in
     */
    virtual void __stdcall copyParamResult(paramid_t * array) = 0;

    /**
     * Add new parameter or find the existing one.
     * @param qualifiedName Qualified name for parameter
     * @return New parameter id or existing parameter id.
     */
    virtual paramid_t __stdcall add(const char * qualifiedName) = 0;

    virtual int __stdcall getMaxIndex(paramid_t id) = 0;

    virtual int __stdcall getMaxIndex(const char * name) = 0;

    /**
     * Optimize parameter value by minimizing the size of allocated memory
     * @param id Parameter id
     * @throw Exception when id is invalid or pack is read-only
     */
    virtual void __stdcall optimizeValue(paramid_t id) = 0;

    /**
     * Optimize parameter value by minimizing the size of allocated memory
     * @param name Parameter name
     * @throw Exception when no such parameter or pack is read-only
     */
    virtual void __stdcall optimizeValue(const char * name) = 0;

    /**
     * Optimize all parameter values
     */
    virtual void __stdcall optimizeValues() = 0;

    /**
     * Release this pack.
     * No more calls through this interface should be done after release() executed.
     */
    virtual void __stdcall release() = 0;

    /**
     * Add parameter listener. @see IParamPackListener
     * @param listener Listener to add
     */
    virtual void __stdcall addListener(ParamPackListener * listener) = 0;

    /**
     * Remove parameter listener. @see IParamPackListener
     * @param listener Listener to remove
     */
    virtual void __stdcall removeListener(ParamPackListener * listener) = 0;

    /**
     * Copy subset of parameters from another ParamPack
     *
     * @param pack Pack with source parameters
     * @param templateName Template name for parameters to copy
     * @param subst String for template substitution
     */
    virtual void __stdcall copySubsetFrom(IParamPack * pack, const char * templateName, const char * subst = 0) = 0;

    /**
     * @link
     * @shapeType PatternLink
     * @pattern Class Factory
     * @supplierRole Factory */
    /*# ParamPackFactory _ParamPackFactory; */
};


/**
 * Listener for pack of parameters.
 * It handles events about changing parameter values.
 */
class ParamPackListener {
public:

    /**
     * Parameter change event handler. Is called by pack when its value 
     * is to be modified. 
     *
     * @param ppack Paramter pack that contains parameter (the caller)
     * @param id Paramter id that changes it's value
     * @param idNew Parameter id that contains new value (not assigned yet)
     * @return True to allow changing, false to block changing
     */
    virtual bool __stdcall onParamPreChange(IParamPack * ppack, paramid_t id, paramid_t idNew) { return true; }

    /**
     * Parameter change event handler. Is called by pack after its value
     * has been modified. 
     *
     * @param ppack Paramter pack that contains parameter (the caller)
     * @param id Paramter id that changes it's value
     * @param idOld Parameter id that contains old value
     */
    virtual void __stdcall onParamChanged(IParamPack * ppack, paramid_t id, paramid_t idOld) { }

    /**
     * Parampack destroy event handler. Is called by pack when it is to be destroyed.
     *
     * @param ppack Paramter pack (the caller)
     */
    virtual void __stdcall onParamPackDestroy(IParamPack * ppack) { }

};


/**
 * Toolkit for random numbers generation
 */
class IRandToolkit {
public:

    DECLARE_INTERFACE_ID(0x10007);

    /**
     * Initialize generator with new seed 
     *
     * @param seed Index of numeric table that is used to generate random numbers
     */
    virtual void __stdcall setSeed(long seed) = 0;

    /**
     * Initialize generator with new seed get from current timer
     */
    virtual void __stdcall resetSeed() = 0;

    /**
     * Get current seed
     *
     * @return Current seed index
     */
    virtual long __stdcall getSeed() = 0;

    /**
     * Generate uniformly-distributed boolean value
     *
     * @return True or false
     */
    virtual bool __stdcall isReshka() = 0;

    /**
     * Generate uniformly-distributed float value
     *
     * @return Float value in range [0..1)
     */
    virtual float __stdcall getUniform() = 0;

    /**
     * Generate uniformly-distributed float value
     *
     * @return Float value in range [a, b)
     */
    virtual float __stdcall getUniform(float a, float b) = 0;

    /**
     * Generate uniformly-distributed integer value
     *
     * @return Integer value in range [0..INT_MAX)
     */
    virtual int __stdcall getUniformInt() = 0;

    /**
     * Generate normally-distributed float value
     *
     * @return Float value in range [-1, 1]
     */
    virtual float __stdcall getNorm() = 0;

};



/**
 * Simple markup language listener base class
 */
class SMLListener {
public:

    /**
     * Called before beginning processing SML document
     */     
    virtual void __stdcall onSmlBegin() { }

    /**
     * Called after finalizing processing SML document
     *
     * @param finalText Final text document which is result of processing
     */
    virtual void __stdcall onSmlEnd(const char * finalText) { }

    /**
     * Called for each SML node is processed.
     * Sample document: <author id=1>Markov</author>
     *
     * @param nodeType Type of node (is "author" for above sample)
     * @param attrib Pack of node attributes (is <name="id", value="1" for above sample)
     * @param nodeText Inner text of node (is "Markov" for above sample)
     */
    virtual const char * __stdcall onSmlNode(const char * nodeType, IParamPack * attrib, const char * nodeText) { return nodeText; }
};



/**
 * Interface for time 
 */
class ITime {
public:

    DECLARE_INTERFACE_ID(0x10008);
    
    /**
     * Set timeout interval for events calculation
     *
     * @param value Timeout value
     */
    virtual void __stdcall setTimeout(float value) = 0;
    
    /**
     * Get current time value
     *
     * @return Current time value
     */
    virtual float __stdcall getValue() = 0;

    /**
     * Set current time value
     *
     * @param value Time value to replace
     */
    virtual void __stdcall setValue(float value) = 0;
    
    /**
     * Enable/disable automatic advancement of time due to system time
     *
     * @param enable True to enable, false to disable
     */
    virtual void __stdcall enable(bool enable) = 0;

    /**
     * Advance time 
     *
     * @param dt Advance value
     */
    virtual void __stdcall advance(float dt) = 0;
    
    /**
     * Release this time (destroys time object)
     */
    virtual void __stdcall release() = 0;

};


class SerializeStream {
public:

    virtual bool __stdcall isSaving() = 0;

    virtual void __stdcall setRules(long rules) = 0;

    virtual void __stdcall assertVersion(long versionId) = 0;

    virtual void __stdcall setLabel(const char * label) = 0;

    virtual int __stdcall getNumElements() = 0;

    virtual void __stdcall serialize(int * value, int maxElements = 1) = 0;

    virtual void __stdcall serialize(float * value, int maxElements = 1) = 0;

    virtual void __stdcall serialize(bool * value, int maxElements = 1) = 0;

    virtual void __stdcall serialize(char * value, int maxElements = 1) = 0;

    virtual void __stdcall serialize(Vector2f * value, int maxElements = 1) = 0;

    virtual void __stdcall serialize(Vector3f * value, int maxElements = 1) = 0;

    virtual void __stdcall serialize(Vector4f * value, int maxElements = 1) = 0;

    virtual void __stdcall serializeIds(globid_t * value, int maxElements = 1) = 0;

};


class ISerializable : public IBase {
public:
    DECLARE_INTERFACE_ID(0x10009);

    virtual void __stdcall entitySerialize(SerializeStream * stream) = 0;

};



// Event for activated trigger
struct EvtTrigger {
    DECLARE_EVENT_ID(0x10006);
};


// Trigger for entity life cycle
struct TrigEntityLife {
    DECLARE_TRIGGER_ID(0x1000a);
    struct Param : public Object {
        entid_t eid;
        EntityLifeTime lifeTime;
        Object * param;
        const char * entityType;
    };
};


// Inline implementation


// Templated version of ICore::askInterface()
template <typename T> inline bool askInterfaceT(entid_t eid, T** pInt) {
    *pInt = static_cast<T*>(getCore()->askInterface(eid, T::iid));
    return *pInt!=0;
}

// Templated version of ICore::askInterface()
template <typename T> inline bool askInterfaceT(const char * entityType, T** pInt) {
    *pInt = static_cast<T*>(getCore()->askInterface(entityType, T::iid));
    return *pInt!=0;
}

// Templated version of ICore::queryInterface()
template <typename T> inline void queryInterfaceT(entid_t eid, T** pInt) {
    *pInt = static_cast<T*>(getCore()->queryInterface(eid, T::iid));
}

// Templated version of ICore::queryInterface()
template <typename T> inline void queryInterfaceT(const char * entityType, T** pInt) {
    *pInt = static_cast<T*>(getCore()->queryInterface(entityType, T::iid));
}


// Templated version of ICore::askInterface()
template <typename T> inline bool askInterface(entid_t eid, T** pInt) {
    *pInt = static_cast<T*>(getCore()->askInterface(eid, T::iid));
    return *pInt!=0;
}

// Templated version of ICore::askInterface()
template <typename T> inline bool askInterface(const char * entityType, T** pInt) {
    *pInt = static_cast<T*>(getCore()->askInterface(entityType, T::iid));
    return *pInt!=0;
}

// Templated version of ICore::queryInterface()
template <typename T> inline void queryInterface(entid_t eid, T** pInt) {
    *pInt = static_cast<T*>(getCore()->queryInterface(eid, T::iid));
}

// Templated version of ICore::queryInterface()
template <typename T> inline void queryInterface(const char * entityType, T** pInt) {
    *pInt = static_cast<T*>(getCore()->queryInterface(entityType, T::iid));
}


inline void __cdecl ICore::logMessage(const char * fmt, ...) {
    va_list args;
    va_start(args,fmt);
    logMessageV(fmt,args);
    va_end(args);
}

}
#endif
