
#ifndef CUTSCENE_MODE_INCLUDED
#define CUTSCENE_MODE_INCLUDED

#include "headers.h"
#include "scene.h"
#include "callback.h"
#include "sensor.h"

/**
 * cutscene mode
 */

struct CutsceneTrack
{
public:
    float begin;
    float end;
public:
    CutsceneTrack(float b, float e) : begin(b), end(e) {}
};

typedef std::vector<CutsceneTrack> CutsceneTrackV;
typedef CutsceneTrackV::iterator CutsceneTrackI;

struct CutsceneDesc
{
public:
    engine::IClump* source;
    CutsceneTrackV  tracks;
    float           speed;
};

class Cutscene : public Mode
{
private:
    /**
     * local camera
     */
    class Camera : public Actor
    {
    private:      
        float        _cameraFOV;
        Matrix4f     _cameraMatrix;
        std::wstring _titleText;
        float        _titleTimeout;
        float        _titleFadeTime;
    public:
        // actor abstracts
        virtual void onUpdateActivity(float dt);
        virtual Matrix4f getPose(void) { return _cameraMatrix; }
    public:
        // class implementation
        Camera(Scene* scene, float fov);
        virtual ~Camera();
        // class behaviour
        void setMatrix(const Matrix4f& matrix);
        void setTitle(const wchar_t* text, float timeout, float fadeTime);
    };
private:
    typedef std::list<engine::AnimSequence> AnimSequenceL;
    typedef AnimSequenceL::iterator AnimSequenceI;
private:
    CutsceneDesc    _desc;
    engine::IClump* _clump;
    AnimSequenceL   _animSequenceL;
    AnimSequenceI   _animSequenceI;
    Camera*         _camera;
public:
    // actor abstracts
    virtual void onUpdateActivity(float dt);
    // mode abstracts
    virtual void onSuspend(void);
    virtual void onResume(void);
    virtual bool endOfMode(void);
public:
    // class implementation
    Cutscene(Scene* scene, CutsceneDesc* desc);
    virtual ~Cutscene();
};

#endif