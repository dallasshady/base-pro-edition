
#ifndef INSTRUCTOR_ACTORS_INCLUDED
#define INSTRUCTOR_ACTORS_INCLUDED

#include "headers.h"
#include "mission.h"
#include "jumper.h"
#include "goal.h"

/**
 * abstract instructor (virtual actor, issuing an orders to the player)
 * contains base behavior for other instructors
 */

#define orderIs(A) ( NULL != dynamic_cast<A*>( _order ) )

class Instructor : public Goal
{
public:
    /**
     * abstract instructor order
     */
    class Order
    {
    private:
        Jumper*          _jumper;
        gui::IGuiWindow* _window;
    public:
        // class implementation
        Order(Jumper* jumper);
        virtual ~Order();
    public:
        // base behaviour
        void setMessage(const wchar_t* message);
    public:
        // inlines
        inline Jumper* getJumper(void) { return _jumper; }
    public:
        // abstract behaviour
        virtual float getMaximalScore(void) = 0;
        virtual float getAchievedScore(void) = 0;
        virtual void onUpdateOrder(float dt) = 0;
        virtual bool isExecuted(void) = 0;
    };
    /**
     * abstract notification order
     */
    class Notification : public Order
    {
    private:
        bool  _lock;
        float _timeLeft;
    public:
        // class implementation
        Notification(Jumper* jumper, const wchar_t* message, float delay);
    public:
        // Order implementation
        virtual float getMaximalScore(void);
        virtual float getAchievedScore(void);
        virtual void onUpdateOrder(float dt);
        virtual bool isExecuted(void);
    public:
        // this class behaviour
        inline bool isLocked(void) { return _lock; }
        inline void setLock(bool value) { _lock = value; }
    };
protected:
    Jumper* _jumper;
    Order*  _order;
public:
    // actor abstracts
    virtual void onUpdateActivity(float dt);
    virtual void onUpdatePhysics();
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
    // Goal abstracts
    virtual const wchar_t* getGoalName(void);
public:
    // class implementation
    Instructor(Jumper* jumper);
    virtual ~Instructor();
};

#endif