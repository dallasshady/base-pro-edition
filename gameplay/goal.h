
#ifndef GOAL_ACTORS_INCLUDED
#define GOAL_ACTORS_INCLUDED

#include "headers.h"
#include "scene.h"
#include "jumper.h"
#include "npc.h"
#include "smokeball.h"
#include "smokejet.h"

/**
 * abstract goal
 */

class Goal : public Actor
{
protected:
    gui::IGuiWindow* _window;
    Jumper*          _player;
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void) = 0;
    virtual const wchar_t* getGoalValue(void) = 0;
    virtual float getGoalScore(void) = 0;
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
public:
    // class implementation
    Goal(Jumper* player);
    virtual ~Goal();
public:
    // inlines
    inline gui::IGuiWindow* getWindow(void) { return _window; }
};

/**
 * health goal
 */

class GoalStateOfHealth : public Goal
{
private:
    float _freefallHealth; // health level before free fall
    float _flightHealth;   // health level after canopy opening
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void);
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    GoalStateOfHealth(Jumper* player);
    virtual ~GoalStateOfHealth();
};

/**
 * gear goal
 */

class GoalStateOfGear : public Goal
{
private:
    float        _helmetState; // state of gear before jump
    float        _suitState;   // ...
    float        _rigState;    // ...
    float        _canopyState; // ...
    std::wstring _goalValue;   // generated goal value
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void);
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    GoalStateOfGear(Jumper* player);
    virtual ~GoalStateOfGear();
};

/**
 * landing goal
 */

class GoalLanding : public Goal
{
private:
    float _score;
    bool  _isAcquired;
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void);
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    GoalLanding(Jumper* player);
    virtual ~GoalLanding();
};

/**
 * dropzone goal : canopy deployment altitude
 */

class GoalDropzone : public Goal
{
private:
    bool         _isAcquired;
    float        _deploymentAltitude;
    std::wstring _goalValue;
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void);
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    GoalDropzone(Jumper* player);
    virtual ~GoalDropzone();
};

/**
 * experience goal
 */

class GoalExperience : public Goal
{
private:
    Virtues::Skills  _prevSkills;
    Virtues::Skills  _currSkills;
    bool             _isAcquired;
    std::wstring     _goalValue;
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void);
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    GoalExperience(Jumper* player);
    virtual ~GoalExperience();
public:
    // class behaviour
    float getTotalSkillEvolution(void);
};

/**
 * bonus goal
 */

enum BonusType
{
    btAbsolute,    // grants score, passed as parameter
    btProgressive, // grants score, depended of player rank
    btUnderground  // progressive, but grants negative score if player hurt itself during jump
};

class GoalBonus : public Goal
{
private:
    std::wstring _bonusName;
    BonusType    _bonusType;
    float        _bonusParameter;
    float        _startHealth;
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void);
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    GoalBonus(Jumper* player, const wchar_t* bonusName, BonusType bonusType, float bonusParameter);
    virtual ~GoalBonus();
};

/**
 * tracking performance goal
 */

class GoalTrackingPerformance : public Goal
{
private:
    bool         _captureIsActive;
    Vector3f     _prevPos;
    float        _horizontalDistance;
    float        _verticalDistance;
    std::wstring _goalValue;
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void);
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    GoalTrackingPerformance(Jumper* player);
    virtual ~GoalTrackingPerformance();
};

/**
 * spiral freefall goal
 */

class GoalSpiral : public Goal
{
private:
    Vector3f     _axisOffset;
    bool         _captureIsActive;
    Vector3f     _prevPos;
    float        _angleAccumulator;
    std::wstring _goalValue;
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void);
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    GoalSpiral(Jumper* player, Vector3f axisOffset);
    virtual ~GoalSpiral();
};

/**
 * cameraman footage goal
 */

struct GoalFootageDescriptor
{
public:
    struct RangeMark
    {
    public:
        float distance;
        float score;
    public:
        RangeMark() : distance(0), score(0) {}
        RangeMark(float d, float s) : distance(d), score(s) {}
    };
public:
    RangeMark bad;
    RangeMark poor;
    RangeMark tolerable;
    RangeMark good;
    RangeMark fine;
    float     timeFactor;
public:
    GoalFootageDescriptor() : timeFactor( 1.0f ) {}
};

class GoalFootage : public Goal
{
private:
    GoalFootageDescriptor _descriptor;
    NPC*                  _npc;
    unsigned int          _step;
    float                 _averageDistance;
    float                 _footageTime;
    std::wstring          _goalValue;
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void);
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    GoalFootage(Jumper* player, NPC* npc, GoalFootageDescriptor* descriptor);
    virtual ~GoalFootage();
};

/**
 * tracking proximity goal
 */

struct GoalProximityDescriptor 
{
public:
    struct RangeMark
    {
    public:
        float distance;       // distance
        float scorePerSecond; // this is progressive multiplier!
    public:
        RangeMark() : distance(0), scorePerSecond(0) {}
        RangeMark(float d, float s) : distance(d), scorePerSecond(s) {}
    };
public:
    RangeMark range0;
    RangeMark range1;
    RangeMark range2;
    RangeMark range3;
};

class GoalProximity : public Goal
{
private:
    GoalProximityDescriptor _descriptor;
    float                   _score;
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void);
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    GoalProximity(Jumper* player, GoalProximityDescriptor* descriptor);
    virtual ~GoalProximity();
};

/**
 * delay accuracy goal
 */

struct GoalDelayAccuracyDescriptor
{
public:
    float maxDelay;
    float maxScore;    
    float minDelay;
    float minScore;
    float interpolationSmoothing;
public:
    GoalDelayAccuracyDescriptor() : 
        maxDelay(0.0f), maxScore(0.0f),
        minDelay(0.0f), minScore(0.0f),
        interpolationSmoothing(1.0f)
    {}
};

class GoalDelayAccuracy : public Goal
{
private:
    GoalDelayAccuracyDescriptor _descriptor;
    bool                        _freefall;
    float                       _delay;
    std::wstring                _goalValue;
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void);
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
public:
    // class implementation
    GoalDelayAccuracy(Jumper* player, GoalDelayAccuracyDescriptor* descriptor);
    virtual ~GoalDelayAccuracy();
};

/**
 * smokeball mission goal
 */

class GoalSmokeball : public Goal
{
private:
    std::vector<SmokeBall*> _smokeballs;
    SmokeJet*               _smokeJet;
    std::wstring            _goalValue;
    Vector4f                _currentJetColor;
    Vector4f                _targetJetColor;
    unsigned int            _numPiercedBalls;
    float                   _ballScore;
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void);
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
    virtual void onUpdatePhysics(void);
public:
    // class implementation
    GoalSmokeball(Jumper* player, std::vector<SmokeBallDesc>& smokeballs, float ballScore);
    virtual ~GoalSmokeball();
};

/**
 * flip count goal
 */

class GoalFlipCount : public Goal
{
private:
    float        _score;
    bool         _captureIsActive;
    Vector3f     _prevAt;
    Vector3f     _prevRight;
    float        _angleAccumulator;
    std::wstring _goalValue;
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void);
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    GoalFlipCount(Jumper* player, float score);
    virtual ~GoalFlipCount();
};

/**
 * jump funds
 */
/*

struct GoalRewardDescriptor
{
public:
    float rate[11]; // rate per RANK of player
public:
    GoalRewardDescriptor()
    {
        memset( rate, 0, sizeof(float)*11 );
    }
    GoalRewardDescriptor(float* r)
    {
        for( unsigned int i=0; i<11; i++ ) rate[i] = r[i];
    }
};

class GoalReward : public Goal
{
private:
    GoalRewardDescriptor _descriptor;
    std::wstring         _goalValue;
    float                _funds;
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void);
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
public:
    // class implementation
    GoalReward(Jumper* player, GoalRewardDescriptor* descriptor);
    virtual ~GoalReward();
};*/

#endif