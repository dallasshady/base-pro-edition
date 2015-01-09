
#ifndef LANDING_ACCURACY_GOAL_INCLUDED
#define LANDING_ACCURACY_GOAL_INCLUDED

#include "headers.h"
#include "mission.h"
#include "jumper.h"
#include "goal.h"

/**
 * abstract landing accuracy goal
 */

class GoalLandingAccuracy : public Goal
{
protected:
    Jumper*          _jumper;
    engine::IClump*  _landingCircle;
    gui::IGuiWindow* _signature;
    float            _radius;
    std::wstring     _goalValue;
    bool             _isFixed;
    float            _fixedAccuracy;
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void);
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
public:
    // actor abstracts
    virtual void onUpdateActivity(float dt);
    virtual void onUpdatePhysics();
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
public:
    // class behaviour
    float getLandingAccuracy(void);
public:
    // class implementation
    GoalLandingAccuracy(Jumper* jumper, Vector3f pos, float scale);
    virtual ~GoalLandingAccuracy();
};

/**
 * hard landing goal
 */

class GoalHardLanding : public GoalLandingAccuracy
{
protected:
    Gear         _prize;
    std::wstring _prizeCost;
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void);
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
public:
    // class implementation
    GoalHardLanding(Jumper* jumper, Vector3f pos, float scale, Gear prize);
    virtual ~GoalHardLanding();
};

#endif