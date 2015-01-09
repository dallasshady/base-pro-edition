
#ifndef BASE_INSTRUCTORS_INCLUDED
#define BASE_INSTRUCTORS_INCLUDED

#include "headers.h"
#include "instructor.h"
#include "landingaccuracy.h"

namespace instructor
{

/**
 * abstract BASE jumping instructor
 */

class BASEInstructor : public Instructor
{
private:
    bool  _isAcquired;
    float _score;
public:
    class JumpBeforeTheWind : public Notification
    {
    private:
        Vector3f _wind;
    public:
        JumpBeforeTheWind(Jumper* jumper, Vector3f wind);
    public:
        virtual void onUpdateOrder(float dt);
    };
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
    // Goal abstracts
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
    // class implementation
    BASEInstructor(Jumper* player);
    virtual ~BASEInstructor();
};

/**
 * BASE jumping instructor ¹1
 */

class BASEInstructor01 : public BASEInstructor
{
public:
    class Invitation01 : public Notification
    {
    public:
        Invitation01(Jumper* jumper);
    };
    class Invitation02 : public Notification
    {
    public:
        Invitation02(Jumper* jumper);
    };
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
    // class implementation
    BASEInstructor01(Jumper* player);
};

/**
 * BASE jumping instructor ¹2
 */

class BASEInstructor02 : public BASEInstructor
{
public:
    class Invitation : public Notification
    {
    public:
        Invitation(Jumper* jumper);
    };
    class Pull : public Notification
    {
    private:
        float _timeSpeed;
    public:
        // class implementation
        Pull(Jumper* jumper);
        virtual ~Pull();
    public:        
        // order abstracts
        virtual void onUpdateOrder(float dt);
        virtual bool isExecuted(void);
    };
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
    // class implementation
    BASEInstructor02(Jumper* player);
};

/**
 * BASE jumping instructor ¹3
 */

class BASEInstructor03 : public BASEInstructor
{
protected:
    GoalLandingAccuracy* _goalLandingAccuracy;
public:
    class Invitation : public Notification
    {
    public:
        Invitation(Jumper* jumper);
    };
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
    // Goal abstracts
    virtual const wchar_t* getGoalValue(void);
    // class implementation
    BASEInstructor03(Jumper* player);
};

/**
 * BASE jumping instructor ¹4
 */

class BASEInstructor04 : public BASEInstructor
{
private:
    bool _lineoverIsApplied;
public:
    class Invitation01 : public Notification
    {
    public:
        Invitation01(Jumper* jumper);
    };
    class Invitation02 : public Notification
    {
    public:
        Invitation02(Jumper* jumper);
    };
    class Jump : public Notification
    {
    public:
        Jump(Jumper* jumper);
    public:
        // order abstracts
        virtual void onUpdateOrder(float dt);
    };
    class Pull : public Notification
    {
    public:
        Pull(Jumper* jumper);
    public:
        // order abstracts
        virtual void onUpdateOrder(float dt);
    };
    class FightLineover : public Notification
    {
    public:
        FightLineover(Jumper* jumper);
    public:
        // order abstracts
        virtual void onUpdateOrder(float dt);
    };
    class AboutHookKnife : public Notification
    {
    public:
        AboutHookKnife(Jumper* jumper);
    };
    class Farewell : public Notification
    {
    public:
        Farewell(Jumper* jumper);
    };
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
    // class implementation
    BASEInstructor04(Jumper* player);
};

}

#endif