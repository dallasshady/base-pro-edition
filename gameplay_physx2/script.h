
#ifndef SCRIPT_ACTORS_INCLUDED
#define SCRIPT_ACTORS_INCLUDED

#include "headers.h"
#include "mission.h"
#include "jumper.h"
#include "npc.h"

/**
 * abstract script is like to abstract instructor, but has no goal behaviour
 * this class contains base behavior for other scripts
 */

#define routineIs(A) ( NULL != dynamic_cast<A*>( _routine ) )

class Script : public Actor
{
public:
    /**
     * abstract script routine
     */
    class Routine
    {
    private:
        Jumper*          _jumper;
        gui::IGuiWindow* _window;
    public:
        // class implementation
        Routine(Jumper* jumper);
        virtual ~Routine();
    public:
        // base behaviour
        void setMessage(const wchar_t* message);
    public:
        // inlines
        inline Jumper* getJumper(void) { return _jumper; }
    public:
        // abstract behaviour
        virtual void onUpdateRoutine(float dt) = 0;
        virtual bool isExecuted(void) = 0;
    };
    /**
     * message routine
     */
    class Message : public Routine
    {
    private:
        bool  _lock;
        float _timeLeft;
    public:
        // class implementation
        Message(Jumper* jumper, const wchar_t* message, float delay);
    public:
        // Order implementation
        virtual void onUpdateRoutine(float dt);
        virtual bool isExecuted(void);
    public:
        // this class behaviour
        inline bool isLocked(void) { return _lock; }
        inline void setLock(bool value) { _lock = value; }
    };
    /**
     * assist routine
     */
    class Assist : public Message
    {
    private:
        NPC*     _npc;
        Vector3f _wind;
        float    _timeout;
    public:
        Assist(Jumper* jumper, NPC* npc, Vector3f wind);
    public:
        virtual void onUpdateRoutine(float dt);
    };
    /**
     * countdowns
     */
    class Countdown : public Message
    {
    private:
        CatToy* _catToy;
    public:
        Countdown(Jumper* jumper, CatToy* catToy);
    public:
        virtual void onUpdateRoutine(float dt);
    };
    /**
     * timer routine
     */
    class Timer : public Message
    {
    private:
        float _time;
    public:
        Timer(Jumper* jumper);
    public:
        virtual void onUpdateRoutine(float dt);
    };
    /**
     * "see you" routine
     */
    class SeeYou : public Message
    {
    private:
        NPCL _npcs;
        NPC* _dubbingNPC;
    public:
        SeeYou(Jumper* jumper, NPCL& npcs);
    public:
        virtual void onUpdateRoutine(float dt);
    };
protected:
    Jumper*  _jumper;
    Routine* _routine;
public:
    // actor abstracts
    virtual void onUpdateActivity(float dt);
    virtual void onUpdatePhysics();
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
public:
    // class implementation
    Script(Jumper* jumper);
    virtual ~Script();
};

#endif