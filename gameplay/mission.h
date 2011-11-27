
#ifndef MISSION_MODE_INCLUDED
#define MISSION_MODE_INCLUDED

#include "headers.h"
#include "scene.h"
#include "callback.h"
#include "sensor.h"
#include "jumper.h"

/**
 * mission mode
 */

class Mission : public Mode
{
private:
    /**
     * third person camera
     */
    class ThirdPersonCamera : public Actor
    {
    private:      
        Actor*   _target;
        bool     _positionMode;
        float    _positionModeTimeout;
        float    _cameraTilt;
        float    _cameraTurn;
        float    _cameraDistance;
        float    _cameraFOV;
        Matrix4f _cameraMatrix;
        Vector3f _cameraPos;
    public:
        // actor abstracts        
        virtual void onUpdateActivity(float dt);
        virtual Matrix4f getPose(void) { return _cameraMatrix; }
    public:
        // class implementation
        ThirdPersonCamera(Scene* scene, Actor* target);
        virtual ~ThirdPersonCamera();
    public:
        // class behaviour
        void switchMode(void);
        void resetSwitchTimer(void);
    };
    /**
     * first person camera
     */
    class FirstPersonCamera : public Actor
    {
    private:
        engine::IFrame* _headFrame;
        float           _cameraTilt;
        float           _cameraTurn;
        float           _cameraFOV;
        Matrix4f        _cameraMatrix;
    protected:
        // actor abstracts
        virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
        virtual void onUpdateActivity(float dt);
        virtual Matrix4f getPose(void) { return _cameraMatrix; }
    public:
        // class implementation
        FirstPersonCamera(Scene* scene, engine::IFrame* headFrame);
        virtual ~FirstPersonCamera();        
    };
    /**
     * flyway camera
     */
    class FlywayCamera : public Actor
    {
    private:
        Actor*   _target;
        float    _timeReserve;
        float    _lag;
        float    _cameraFOV;
        Vector3f _cameraPos;
        Matrix4f _cameraMatrix;
    protected:
        // actor abstracts
        virtual void onUpdateActivity(float dt);
        virtual Matrix4f getPose(void) { return _cameraMatrix; }
    public:
        // class implementation
        FlywayCamera(Scene* scene);
        virtual ~FlywayCamera();        
    public:
        // class behaviour
        void setupCamera(Actor* target, float timeReserse, float lag);
    };
    /**
     * free camera
     */
    class FreeCamera : public Actor
    {
    private:
        float    _cameraFOV;
        float    _cameraTilt;
        float    _cameraTurn;    
        Vector3f _cameraPos;
        Matrix4f _cameraMatrix;
        bool     _writeWaypointMode;
        float    _writeWaypointModeTimeout;
        Vector3f _lastWaypointPosition;
    private:
        // class internal behaviour
        void makeEnvironmentMap(void);
    protected:
        // actor abstracts
        virtual void onUpdateActivity(float dt);
        virtual Matrix4f getPose(void) { return _cameraMatrix; }
    public:
        // class implementation
        FreeCamera(Scene* scene);
        virtual ~FreeCamera();
    };
private:    
    database::MissionInfo* _missionInfo;
    unsigned int           _wttid;
    unsigned int           _wtmid;
    bool                   _endOfMission;
    bool                   _endOfPlayerActivity;
    Jumper*                _player;
    ThirdPersonCamera*     _tpcamera;
    FirstPersonCamera*     _fpcamera;
    FlywayCamera*          _fwcamera;
    FreeCamera*            _frcamera;
    gui::IGuiWindow*       _promptWindow;
    float                  _fadValue;       // flight mode acceleration value
    float                  _fadTargetValue; // flight mode target acceleration value
    float                  _fadTimeout;     // flight mode acceleration timeout
    float                  _phTimeLeft;     // it is needs for forced timestep simulation support
    float                  _interruptTimeout; // delays interrupt mode activation
public:
    // actor abstracts
    virtual void onUpdateActivity(float dt);
    // mode abstracts
    virtual void onSuspend(void);
    virtual void onResume(void);
    virtual bool endOfMode(void);
public:
    // class implementation
    Mission(Scene* scene, database::MissionInfo* missionInfo, unsigned int wttid, unsigned int wtmid);
    virtual ~Mission();
public:
    // complex behaviour
    void setFirstPersonCamera(void);
    void setThirdPersonCamera(void);
    bool cameraIsFirstPerson(void);
    bool cameraIsThirdPerson(void);
    void showGoals(bool flag);
    bool goalsIsVisible(void);
public:
    // inlines
    inline database::MissionInfo* getMissionInfo(void) { return _missionInfo; }
    inline Jumper* getPlayer(void) { return _player; }
    inline void setPlayer(Jumper* player) { _player = player; }    
};

#endif