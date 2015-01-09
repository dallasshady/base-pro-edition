
#ifndef INTERRUPT_MODE_INCLUDED
#define INTERRUPT_MODE_INCLUDED

#include "headers.h"
#include "scene.h"

/**
 * interrupt mode
 */

class Interrupt : public Mode
{
private:    
    /**
     * interrupt mode camera
     */
    class Camera : public Actor
    {
    private:
        float    _cameraFOV;
        Matrix4f _cameraMatrix;
    protected:
        // actor abstracts
        virtual void onUpdateActivity(float dt);
        virtual Matrix4f getPose(void) { return _cameraMatrix; }
    public:
        // class implementation
        Camera(Scene* scene, const Matrix4f& cameraMatrix);
        virtual ~Camera();
    };
private:
    float            _modeStartupTimeout;
    bool             _endOfMode;
    bool*            _interruptResult;
    bool             _payCredits;
    Camera*          _camera;
    gui::IGuiWindow* _window;
private:
    // gui messaging
    static void messageCallback(gui::Message* message, void* userData);
public:
    // actor abstracts
    virtual void onUpdateActivity(float dt);
    // mode abstracts
    virtual void onSuspend(void);
    virtual void onResume(void);
    virtual bool endOfMode(void);
public:
    // class implementation
    Interrupt(Scene* scene, const Matrix4f& cameraMatrix, bool* interruptResult, bool payCredits);
    virtual ~Interrupt();
};

#endif