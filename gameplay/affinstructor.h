
#ifndef AFF_INSTRUCTORS_INCLUDED
#define AFF_INSTRUCTORS_INCLUDED

#include "headers.h"
#include "instructor.h"

namespace instructor
{

/**
 * abstract static line instructor
 */

class StaticLineInstructor : public Instructor
{
private:
    bool  _isAcquired;
    float _score;
public:
    class Jump : public Notification
    {
    public:
        Jump(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
    class DontBeAfraid : public Notification
    {
    public:
        DontBeAfraid(Jumper* jumper, float duration);
    };
    class AgainstTheWind : public Notification
    {
    private:
        Vector3f     _wind;
        std::wstring _orderText;
        float        _orderAltitude;
    public:
        AgainstTheWind(Jumper* jumper, Vector3f wind, const wchar_t* orderText, float orderAltitude);
    public:
        virtual void onUpdateOrder(float dt);
    };
    class Brake : public Notification
    {
    public:
        Brake(Jumper* jumper);
    };
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
    // Goal abstracts
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
    // class implementation
    StaticLineInstructor(Jumper* player);
    virtual ~StaticLineInstructor();
};

/**
 * static line instructor ¹1
 */

class StaticLineInstructor01 : public StaticLineInstructor
{
public:
    /**
     * orders
     */
    class Invitation : public Notification
    {
    public:
        Invitation(Jumper* jumper);
    };
    class CameraControl : public Notification
    {
    public:
        CameraControl(Jumper* jumper);
    };        
    class DiscoverTheWind : public Notification
    {
    public:
        DiscoverTheWind(Jumper* jumper);
    };
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
    // class implementation
    StaticLineInstructor01(Jumper* player);
};

/**
 * static line instructor ¹2
 */

class StaticLineInstructor02 : public StaticLineInstructor
{
public:
    /**
     * orders
     */
    class Invitation : public Notification
    {
    public:
        Invitation(Jumper* jumper);
    };
    class FirstPersonMode : public Notification
    {
    public:
        FirstPersonMode(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
    class ThirdPersonMode : public Notification
    {
    public:
        ThirdPersonMode(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
    class CanopyControl : public Notification
    {
    public:
        CanopyControl(Jumper* jumper);
    };
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
    // class implementation
    StaticLineInstructor02(Jumper* player);
};

/**
 * static line instructor ¹3
 */

class StaticLineInstructor03 : public StaticLineInstructor
{
public:
    /**
     * orders
     */
    class QuickLeft : public Notification
    {
    public:
        QuickLeft(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
    class QuickRight : public Notification
    {
    public:
        QuickRight(Jumper* jumper);
    };
    class OtherCombos : public Notification
    {
    public:
        OtherCombos(Jumper* jumper);
    };
    class BowCanopy : public Notification
    {
    public:
        BowCanopy(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
    // class implementation
    StaticLineInstructor03(Jumper* player);
};

/**
 * hop and pop instructor
 */

class HopAndPopInstructor : public StaticLineInstructor
{
public:
    /**
     * orders
     */
    class Invitation : public Notification
    {
    public:
        Invitation(Jumper* jumper);
    };
    /**
     * pull
     */
    class Pull : public Notification
    {
    private:
        float _timeSpeed;
    public:
        Pull(Jumper* jumper);
        virtual ~Pull();
    public:
        virtual void onUpdateOrder(float dt);
        virtual bool isExecuted(void);
    };
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
    // class implementation
    HopAndPopInstructor(Jumper* player);
};

/**
 * abstract freefall instructor
 */

class FreefallInstructor : public StaticLineInstructor
{
public:
    class Pull : public Notification
    {
    public:
        Pull(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
    // class implementation
    FreefallInstructor(Jumper* player);
};

/**
 * freefall instructor ¹1
 */

class FreefallInstructor01 : public FreefallInstructor
{
public:
    /**
     * orders
     */
    class Invitation : public Notification
    {
    public:
        Invitation(Jumper* jumper);
    };
    class FixCamera : public Notification
    {
    public:
        FixCamera(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
    class ZoomCamera : public Notification
    {
    public:
        ZoomCamera(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
    class ThirdPersonMode : public Notification
    {
    public:
        ThirdPersonMode(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
    class TurnLeft : public Notification
    {
    public:
        TurnLeft(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
    class TurnRight : public Notification
    {
    public:
        TurnRight(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
    class TurnHeadUpAndHeadDown : public Notification
    {
    public:
        TurnHeadUpAndHeadDown(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
    class AlignAndStabilize : public Notification
    {
    public:
        AlignAndStabilize(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };    
    class TimeCompression : public Notification
    {
    public:
        TimeCompression(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
    class LoseHeight : public Notification
    {
    public:
        LoseHeight(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
    // class implementation
    FreefallInstructor01(Jumper* player);
};

/**
 * freefall instructor ¹2
 */

class FreefallInstructor02 : public FreefallInstructor
{
public:
    /**
     * orders
     */
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
    class AssumeTrackingPose : public Notification
    {
    public:
        AssumeTrackingPose(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
    class InclinePose : public Notification
    {
    public:
        InclinePose(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
    class SteerPose : public Notification
    {
    public:
        SteerPose(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
    // class implementation
    FreefallInstructor02(Jumper* player);
};

/**
 * freefall instructor ¹3
 */

class FreefallInstructor03 : public FreefallInstructor
{
public:
    /**
     * orders
     */
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
    class SetupAltimeter : public Notification
    {
    public:
        SetupAltimeter(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
    class TurnOnAltimeter : public Notification
    {
    public:
        TurnOnAltimeter(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
    class TurnOffAltimeter : public Notification
    {
    public:
        TurnOffAltimeter(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
    class ExamineVariometer : public Notification
    {
    public:
        ExamineVariometer(Jumper* jumper);
    };
    class ExamineHUD : public Notification
    {
    public:
        ExamineHUD(Jumper* jumper);
    };
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
    // class implementation
    FreefallInstructor03(Jumper* player);
};

/**
 * linetwists instructor
 */

class LinetwistsInstructor : public StaticLineInstructor
{
private:
    bool _lineoverIsApplied;
public:
    /**
     * orders
     */
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
    class FightLinetwists : public Notification
    {
    public:
        FightLinetwists(Jumper* jumper);
    public:
        virtual void onUpdateOrder(float dt);
    };
    class OtherLinetwists : public Notification
    {
    public:
        OtherLinetwists(Jumper* jumper);
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
    LinetwistsInstructor(Jumper* player);
};

}

#endif