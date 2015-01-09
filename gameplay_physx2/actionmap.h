#ifndef ACTION_MAPPING_INCLUDED
#define ACTION_MAPPING_INCLUDED

#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/input.h"
#include "tinyxml.h"

using namespace ccor;

/**
 * input actions
 */

enum InputAction
{
    iaLeft,
    iaRight,
    iaForward,
    iaBackward,
    iaLeftWarp,
    iaRightWarp,
    iaLeftRearRiser,
    iaRightRearRiser,
	iaRearBrake,
    iaHeadLeft,
    iaHeadRight,
    iaHeadUp,
    iaHeadDown,
    iaZoomIn,
    iaZoomOut,
    iaCameraMode0,
    iaCameraMode1,
    iaCameraMode2,
    iaCameraMode3,
    iaPhase,
    iaModifier,  
    iaGlobalDeceleration,
    iaGlobalAcceleration,
    iaAltimeterMode,
    iaIncreaseWarningAltitude,
    iaDecreaseWarningAltitude,
    iaSwitchHUDMode,
    iaWLO,
    iaHook,
    iaAccelerateFlightTime,
    iaDecelerateFlightTime,
    iaIncreaseMusicVolume,
    iaDecreaseMusicVolume,
	iaCutAway,
	iaPullReserve,

	// joystick actions
	iaCutAwayJoy,
    iaLeftJoy,
    iaRightJoy,
    iaForwardJoy,
    iaBackwardJoy,
    iaLeftWarpJoy,
    iaRightWarpJoy,

	// reserve canopy controls in case of double trouble
    iaReserveLeft,
    iaReserveRight,
    iaReserveLeftWarp,
    iaReserveRightWarp,
	iaReserveLeftRearRiser,
	iaReserveRightRearRiser
};

/**
 * abstract action channel
 */

class ActionChannel
{
protected:
    InputAction _inputAction;
public:
    ActionChannel(InputAction inputAction) : _inputAction(inputAction) {}
    virtual ~ActionChannel() {}
public:
    InputAction getInputAction(void);
    const wchar_t* getInputActionDescription(void);
public:
    virtual const wchar_t* getInputActionHint(void) = 0;
    virtual void setup(input::MouseState* mouseState, input::KeyboardState* keyboardState, input::JoyState* joystickState) = 0;
    virtual void update(float dt, input::MouseState* mouseState, input::KeyboardState* keyboardState, input::JoyState* joystickState) = 0;
    virtual float getAmplitude(void) = 0;
    virtual void setAmplitude(float amplitude) = 0;
	virtual void upAmplitude(float dt, float limit) = 0;
	virtual void downAmplitude(float dt, float limit) = 0;
    virtual bool getTrigger(void) = 0;
    virtual void reset(void) = 0;
};

/**
 * action channel container
 */

typedef std::pair<InputAction,ActionChannel*> ActionChannelT;
typedef std::map<InputAction,ActionChannel*> ActionChannelM;
typedef ActionChannelM::iterator ActionChannelI;

/**
 * button channel
 */

class ButtonChannel : public ActionChannel
{
private:
    unsigned int _deviceCode; // 0 is keyboard 1 is mouse 2 is joystick
    unsigned int _keyCode;
    bool         _smoothMode;
    float        _amplitude;
    float        _ascendingVel;
    float        _descendingVel;
	bool		 _trigger;		// button pressed
private:
    void setupDefault(void);
public:
    // class implementation
    ButtonChannel(InputAction inputAction);
    ButtonChannel(InputAction inputAction, float ascendingVel, float descendingVel);
    // ActionChannel
    virtual const wchar_t* getInputActionHint(void);
    virtual void setup(input::MouseState* mouseState, input::KeyboardState* keyboardState, input::JoyState* joystickState);
    virtual void update(float dt, input::MouseState* mouseState, input::KeyboardState* keyboardState, input::JoyState* joystickState);
    virtual float getAmplitude(void);
    virtual void setAmplitude(float amplitude);
	virtual void upAmplitude(float dt, float limit);
	virtual void downAmplitude(float dt, float limit);
    virtual bool getTrigger(void);
    virtual void reset(void);
    // ButtonChannel specifics
    void setup(unsigned int deviceCode, unsigned int keyCode);
};

/**
 * mouse channel
 */

enum MouseAxis
{
    xAxis,
    yAxis,
    zAxis
};

class MouseChannel : public ActionChannel
{
private:
    MouseAxis _axis;
    int       _direction;
    float     _amplitude;
public:
    static float mouseSensitivityX;
    static float mouseSensitivityY;
    static float mouseSensitivityZ;
private:
    void setupDefault(void);
public:
    // class implementation
    MouseChannel(InputAction inputAction);
    // ActionChannel
    virtual const wchar_t* getInputActionHint(void);
    virtual void setup(input::MouseState* mouseState, input::KeyboardState* keyboardState, input::JoyState* joystickState);
    virtual void update(float dt, input::MouseState* mouseState, input::KeyboardState* keyboardState, input::JoyState* joystickState);
    virtual float getAmplitude(void);
    virtual void setAmplitude(float amplitude);
	virtual void upAmplitude(float dt, float limit);
	virtual void downAmplitude(float dt, float limit);
    virtual bool getTrigger(void);
    virtual void reset(void);
};

enum JoystickAxis
{
	// axis
    xAxisJoy,
    yAxisJoy,
    zAxisJoy,
	rAxisJoy,
	uAxisJoy,
	vAxisJoy,

	// buttons
	joyButton1,
	joyButton2,
	joyButton3,
	joyButton4,
	joyButton5,
	joyButton6,
	joyButton7,
	joyButton8,
	joyButton9,
	joyButton10,
	joyButton11,
	joyButton12,
	joyButton13,
	joyButton14,
	joyButton15,
	joyButton16,
	joyButton17,
	joyButton18,
	joyButton19,
	joyButton20,
	joyButton22,
	joyButton21,
	joyButton23,
	joyButton24,
	joyButton25,
	joyButton26,
	joyButton27,
	joyButton28,
	joyButton29,
	joyButton30,
	joyButton31,
	joyButton32,

	// POV
	joyPOV1Center,
	joyPOV1N,
	joyPOV1NW,
	joyPOV1W,
	joyPOV1SW,
	joyPOV1S,
	joyPOV1SE,
	joyPOV1E,
	joyPOV1NE,

	joyPOV2Center,
	joyPOV2N,
	joyPOV2NW,
	joyPOV2W,
	joyPOV2SW,
	joyPOV2S,
	joyPOV2SE,
	joyPOV2E,
	joyPOV2NE,

	joyPOV3Center,
	joyPOV3N,
	joyPOV3NW,
	joyPOV3W,
	joyPOV3SW,
	joyPOV3S,
	joyPOV3SE,
	joyPOV3E,
	joyPOV3NE,

	joyPOV4Center,
	joyPOV4N,
	joyPOV4NW,
	joyPOV4W,
	joyPOV4SW,
	joyPOV4S,
	joyPOV4SE,
	joyPOV4E,
	joyPOV4NE
};

class JoystickChannel : public ActionChannel
{
private:
    JoystickAxis _axis;
    int       _direction;
    float     _amplitude;
public:
private:
    void setupDefault(void);
public:
    // class implementation
    JoystickChannel(InputAction inputAction);
    // ActionChannel
    virtual const wchar_t* getInputActionHint(void);
    virtual void setup(input::MouseState* mouseState, input::KeyboardState* keyboardState, input::JoyState* joystickState);
    virtual void update(float dt, input::MouseState* mouseState, input::KeyboardState* keyboardState, input::JoyState* joystickState);
    virtual float getAmplitude(void);
    virtual void setAmplitude(float amplitude);
	virtual void upAmplitude(float dt, float limit);
	virtual void downAmplitude(float dt, float limit);
    virtual bool getTrigger(void);
    virtual void reset(void);
};
#endif