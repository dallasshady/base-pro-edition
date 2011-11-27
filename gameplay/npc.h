
#ifndef NON_PLAYABLE_CHARACTER_INCLUDED
#define NON_PLAYABLE_CHARACTER_INCLUDED

#include "headers.h"
#include "jumper.h"

class NPC;

/**
 * NPC program 
 *  - generates control pattern (spinal cord) while certain condition
 *  - supports hierarchical program calls
 *  - supports results of program execution
 */

class NPCProgram
{
private:
    friend class NPC;
private:
    NPC*        _npc;        // points to NPC under control
    NPCProgram* _subProgram; // points to subprogram
protected:
    virtual void onUpdate(float dt) = 0;    
    virtual void onEndOfSubProgram(NPCProgram* subProgram) {}
    virtual bool isEndOfProgram(void) = 0;
public:
    // class implementation
    NPCProgram(NPC* npc);
    virtual ~NPCProgram();
public:
    // base behaviour
    void update(float dt);
    void call(NPCProgram* program);
public:
    // inlines
    inline NPC* getNPC(void) { return _npc; }
};

/**
 * NPC contains behaviour engine for non-playable jumpers
 */

class NPC : public Actor
{
private:
    Airplane*    _airplane;   // airplane for exiting
    Enclosure*   _enclosure;  // enclosure for roaming
    unsigned int _databaseId; // NPC database identifier
    Virtues      _virtues;    // NPC virtues
    SpinalCord   _spinalCord; // spinal cord for AI control
    Jumper*      _jumper;     // jumper for AI control
    CatToy*      _catToy;     // cat toy object for AI
    NPCProgram*  _npcProgram; // AI program
    bool         _ownsCattoy; // true, if NPC owns its cat toy object
private:
    void forceFBEquipment(void);
    void forceNonFBEquipment(void);
public:
    // class implementation
    NPC(Actor* parent, unsigned int databaseId, Airplane* airplane, Enclosure* enclosure, CatToy* catToy, bool dropzone = false);
    virtual ~NPC(void);
public:
    // actor abstracts
    virtual void onUpdateActivity(float dt);
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
    virtual Matrix4f getPose(void) { return _jumper->getPose(); }
    virtual Vector3f getVel(void) { return _jumper->getVel(); }
public:
    // class behaviour
    void setProgram(NPCProgram* program);
    const wchar_t* getNPCName(void);
public:
    // inlines    
    inline Enclosure* getEnclosure(void) { return _enclosure; }
    inline Jumper* getJumper(void) { return _jumper; }
    inline SpinalCord* getSpinalCord(void) { return &_spinalCord; }
    inline CatToy* getCatToy(void) { return _catToy; }
    inline void devoteCattoy(void) { _ownsCattoy = true; }
};

typedef std::list<NPC*> NPCL;
typedef NPCL::iterator NPCI;

#endif