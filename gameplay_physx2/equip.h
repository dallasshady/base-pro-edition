
#ifndef EQUIP_MODE_INCLUDED
#define EQUIP_MODE_INCLUDED

#include "headers.h"
#include "scene.h"
#include "callback.h"
#include "sensor.h"
#include "jumper.h"

/**
 * equip mode
 */

#define EQGUI_NUMITEMS 5

#define NO_MISSION 0xFFFFFFFF

class Equip : public Mode,
              virtual public RenderSource
{
private:
    /**
     * local camera
     */
    class Camera : public Actor
    {
    private:
        Equip*   _equip;
        float    _cameraTilt;
        float    _cameraTurn;
        float    _cameraDistance;
        float    _cameraFOV;
        float    _cameraHeight;
        Matrix4f _cameraMatrix;        
    public:
        // actor abstracts
        virtual void onUpdateActivity(float dt);
        virtual Matrix4f getPose(void) { return _cameraMatrix; }
    public:
        // class implementation
        Camera(Scene* scene, Equip* equip);
        virtual ~Camera();
        // class behaviour
        void setMatrix(const Matrix4f& matrix);
    public:
        // inlines
        inline float getTilt(void) { return _cameraTilt; }
        inline float getTurn(void) { return _cameraTurn; }
        inline float getDistance(void) { return _cameraDistance; }
        inline float getHeight(void) { return _cameraHeight; }
        inline void setTilt(float value) { _cameraTilt = value; }
        inline void setTurn(float value) { _cameraTurn = value; }
        inline void setDistance(float value) { _cameraDistance = value; }
        inline void setHeight(float value) { _cameraHeight = value; }
    };
private:
    database::MissionInfo* _missionInfo;
    Camera*                _camera;
    bool                   _endOfMode;
    unsigned int           _wttid;
    unsigned int           _wtmid;
    Virtues*               _virtues;
    engine::IBSP*          _bsp;
    engine::IClump*        _baseJumper;
    engine::IClump*        _canopy;
    JumperRenderCallback*  _jumperRenderCallback;
    CanopyRenderCallback*  _canopyRenderCallback;
private:
    engine::IBatch*                  _cordBatch;
    unsigned int                     _numCords;
    CanopySimulator::CordSimulator** _cords;
    CanopySimulator::BrakeSimulator* _leftBrake;
    CanopySimulator::BrakeSimulator* _rightBrake;
private:
    bool _selectionMode;
    bool _turnMode;
    bool _moveMode;
    int  _lastMouseX;
    int  _lastMouseY;
private:
    gui::IGuiWindow* _helmetSlot;
    gui::IGuiWindow* _suitSlot;
    gui::IGuiWindow* _rigSlot;
    gui::IGuiWindow* _canopySlot;
	gui::IGuiWindow* _reserveSlot;
    gui::IGuiWindow* _dropList;
private:
    GearType                  _selectionType;
    Gear*                     _selectionDst;
    int                       _selectionTop;
    std::vector<unsigned int> _selectionSrc;
private:
    // initialization
    void initializeCords(void);
    // gui messaging
    static void messageCallback(gui::Message* message, void* userData);
    // rendering
    void buildEquipment(void);
    void renderEquipment(void);
    // momental update
    void showEquipment(void);
    void showHRS(Gear* gear, gui::IGuiWindow* window);
    // equipment selection
    void selectEquipment(gui::IGuiPanel* panel, GearType type, Gear* slot);
    void showListbox(void);
    void equipItem(unsigned int dropListId);
public:
    // actor abstracts
    virtual void onUpdateActivity(float dt);
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
    // mode abstracts
    virtual void onSuspend(void);
    virtual void onResume(void);
    virtual bool endOfMode(void);
    // RenderSource abstracts
    virtual Vector4f getClearColor(void);
    virtual float getBlur(void);
    virtual float getBrightPass(void);
    virtual float getBloom(void);
    virtual unsigned int getNumPasses(void);
    virtual float getPassNearClip(unsigned int passId);
    virtual float getPassFarClip(unsigned int passId);
    virtual unsigned int getPassClearFlag(unsigned int passId);
    virtual void renderPass(unsigned int passId);
    virtual void renderLensFlares(void);
public:
    // class implementation
    Equip(Scene* scene, database::MissionInfo* missionInfo, unsigned int wttid, unsigned int wtmid);
    virtual ~Equip();
};

/**
 * common equip callbacks & helpers
 */

bool equipBestBASEEquipment(Career* career, float windAmbient, float windBlast);
bool equipBestSkydivingEquipment(Career* career, float windAmbient, float windBlast);
bool equipBestSuit(Career* career, float windAmbient, float windBlast);
bool equipBestWingsuit(Career* career, float windAmbient, float windBlast);

#endif