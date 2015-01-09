
#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/product_version.h"
#include "../shared/input.h"
#include "../shared/mainwnd.h"
#include "dinput.h"

using namespace ccor;

/**
 * interface macro-helpers
 */

#define IMainWnd(V) mainwnd::IMainWnd* V; queryInterfaceT( "MainWnd", &V );

/**
 * DirectInput errors
 */



static void diReportResult(const char* file, int line, HRESULT result)
{
    const char* description = NULL;

    switch( result )
    {
    case DIERR_BETADIRECTINPUTVERSION:
        description = "The application was written for an unsupported prerelease version of DirectInput.";
        break;
    case DIERR_INVALIDPARAM:
        description = "An invalid parameter was passed to the returning function, or the object was not in a state that permitted the function to be called. This value is equal to the E_INVALIDARG standard COM return value.";
        break;
    case DIERR_OLDDIRECTINPUTVERSION:
        description = "The application requires a newer version of DirectInput.";
        break;
    case DIERR_OUTOFMEMORY:
        description = "The DirectInput subsystem couldn't allocate sufficient memory to complete the call. This value is equal to the E_OUTOFMEMORY standard COM return value.";
        break;
    case E_POINTER:
        description = "Invalid pointer";
        break;
    default: 
        description = "Unknown.";
        break;
    }

    throw ccor::Exception( "DirectInput interface method was failed with result %x\n\nDescription: \"%s\"\nFile: \"%s\" line %d", result, description, file, line );
}

static inline void diCheckResult(const char* file, int line, HRESULT result)
{
    if( result != DI_OK ) diReportResult( file, line, result );
}
BOOL CALLBACK
enumAxesCallback(const DIDEVICEOBJECTINSTANCE* instance, VOID* context);

#define _diCR(HRESULT) diCheckResult( __FILE__, __LINE__, HRESULT )

/**
 * Input implementation
 */

static HINSTANCE dllInstance = NULL;

class InputDevice : virtual public input::IInputDevice
{
private:
    IDirectInput8*       _iDirectInput;
    IDirectInputDevice8* _iKeyboard;
    IDirectInputDevice8* _iMouse;


    DIMOUSESTATE2        _mouseState;  
	DIJOYSTATE			 _joystickState;
public:
	IDirectInputDevice8* _iJoystick;
	static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE*     
                                       pdidInstance, VOID* pContext)
{
    HRESULT hr;

	// Get access to class variables
	InputDevice* p_this = (InputDevice*) pContext;

    // Obtain an interface to the enumerated joystick.
    hr = p_this->_iDirectInput->CreateDevice(pdidInstance->guidInstance,  
                                &p_this->_iJoystick, NULL);
    if(FAILED(hr)) 
        return DIENUM_CONTINUE;

	getCore()->logMessage("Joystick found and created");
    return DIENUM_STOP;
}

    InputDevice()
    {
        IMainWnd( iMainWnd );

        _diCR( DirectInput8Create( 
            dllInstance, 
            DIRECTINPUT_VERSION, 
            IID_IDirectInput8,
            (void**)(&_iDirectInput),
            NULL
        ) );

        // create & setup keyboard device
        _diCR( _iDirectInput->CreateDevice( GUID_SysKeyboard, &_iKeyboard, NULL ) );
        _diCR( _iKeyboard->SetDataFormat( &c_dfDIKeyboard ) );
        _diCR( _iKeyboard->SetCooperativeLevel( HWND( iMainWnd->getHandle() ), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE ) );
        _diCR( _iKeyboard->Acquire() );

        // create & setup mouse device
        _diCR( _iDirectInput->CreateDevice( GUID_SysMouse, &_iMouse, NULL ) );
        _diCR( _iMouse->SetDataFormat( &c_dfDIMouse2 ) );
        _diCR( _iMouse->SetCooperativeLevel( HWND( iMainWnd->getHandle() ), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE ) );
        _diCR( _iMouse->Acquire() );

        // create & setup joystick device
		if (_iDirectInput->CreateDevice( GUID_Joystick, &_iJoystick, NULL ) == DI_OK) {
			_iJoystick->SetDataFormat( &c_dfDIJoystick );
			_iJoystick->SetCooperativeLevel( HWND( iMainWnd->getHandle() ), DISCL_EXCLUSIVE | DISCL_FOREGROUND );
			if ( _iJoystick->Acquire() == DI_OK) {
				getCore()->logMessage("Joystick acquired");
			} else {
				getCore()->logMessage("Failed to aquire joystick");
			}
		} else {
			getCore()->logMessage("Joystick not found");
			_iJoystick = NULL;
		}

    } 
    virtual ~InputDevice() 
    {
        // unaquire input devices
        _iKeyboard->Unacquire();
        _iMouse->Unacquire();
        // release input devices
        _iKeyboard->Release();    
        _iMouse->Release();    
        _iDirectInput->Release();
		
		if (_iJoystick) {
			_iJoystick->Unacquire();
			_iJoystick->Release();
		}
    }
public:
    /**
     * IInputDevice
     */
    virtual bool __stdcall getKeyboardState(input::KeyboardState* state)
    {
        ZeroMemory( state, sizeof( input::KeyboardState ) );

        // retrieve kb status
        HRESULT result = _iKeyboard->GetDeviceState( 
            sizeof(state->keyState), 
            state->keyState 
        );
    
        // device is unacquired?
        if( result < 0 )
        {
            // re-acquire kb
            result = _iKeyboard->Acquire();
            if( result != DI_OK ) return false;

            // re-read kb state
            _diCR( _iKeyboard->GetDeviceState( 
                sizeof(state->keyState), 
                state->keyState 
            ) );
        }
        return true;
    }
    virtual bool __stdcall getMouseState(input::MouseState* state)
    {
        ZeroMemory( &_mouseState, sizeof(_mouseState) );
    
        HRESULT result = _iMouse->GetDeviceState( 
            sizeof(DIMOUSESTATE2), 
            &_mouseState 
        );

        // device is unacquired?
        if( result < 0 )
        {
            // re-acquire mouse
            result = _iMouse->Acquire();
            if( result != DI_OK ) return false;

            // re-read kb state
            _diCR( _iMouse->GetDeviceState( 
                sizeof(DIMOUSESTATE2), 
                &_mouseState 
            ) );
        }

        state->dX = _mouseState.lX;
        state->dY = _mouseState.lY;
        state->dZ = _mouseState.lZ;
        memcpy( state->buttonState, _mouseState.rgbButtons, sizeof(unsigned char) * 8 );
        return true;
    }

    virtual bool __stdcall getJoystickState(input::JoyState* state)
    {
		if (_iJoystick == NULL) {
			state->usable = false;
			return false;
		}

        ZeroMemory( &_joystickState, sizeof(_joystickState) );
		HRESULT result = _iJoystick->Poll();

		result = _iJoystick->GetDeviceState( 
			sizeof(DIJOYSTATE), 
            &_joystickState
        );

        // device is unacquired?
        if( result < 0 )
        {

            // re-acquire mouse
            result = _iJoystick->Acquire();
            if( result != DI_OK ) return false;

            // re-read kb state
            _diCR( _iJoystick->GetDeviceState( 
                sizeof(DIJOYSTATE), 
                &_joystickState 
            ) );
        }

		state->axis[0] = (float)_joystickState.lX / 32767.0f - 1.0f;
		state->axis[1] = (float)_joystickState.lY / 32767.0f - 1.0f;
		state->axis[2] = (float)_joystickState.lZ / 32767.0f - 1.0f;
		state->axis[3] = (float)_joystickState.rglSlider[0] / 32767.0f - 1.0f;
		state->axis[4] = (float)_joystickState.rglSlider[1] / 32767.0f - 1.0f;
		for (int i = 0; i < 32; ++i) {
			state->buttonState[i] = _joystickState.rgbButtons[i] != 0;
		}
		state->usable = true;
		return true;
	}
    virtual void __stdcall release(void)
    {
        delete this;
    }
};
BOOL CALLBACK
enumAxesCallback(const DIDEVICEOBJECTINSTANCE* instance, VOID* context)
{
    HWND hDlg = (HWND)context;

    DIPROPRANGE propRange; 
    propRange.diph.dwSize       = sizeof(DIPROPRANGE); 
    propRange.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
    propRange.diph.dwHow        = DIPH_BYID; 
    propRange.diph.dwObj        = instance->dwType;
    propRange.lMin              = -1000; 
    propRange.lMax              = +1000; 
    
    // Set the range for the axis
	InputDevice *input = (InputDevice*)context;
    if (FAILED(input->_iJoystick->SetProperty(DIPROP_RANGE, &propRange.diph))) {
        return DIENUM_STOP;
    }

    return DIENUM_CONTINUE;
}




class Input : public EntityBase, 
              virtual public input::IInput 
{
public:
    static Input* instance;    
public:
    Input()
    {
        instance = this;
    } 
    virtual ~Input() 
    {
        instance = NULL;
    }
public:
    virtual void __stdcall entityInit(Object * p) 
    { 
    }
    virtual void __stdcall entityAct(float dt) 
    {
    }
    virtual IBase * __stdcall entityAskInterface(iid_t id) 
    { 
        if (id==input::IInput::iid) return (input::IInput*) this;
        return 0; 
    }
    static EntityBase * creator() 
    { 
        return new Input; 
    }
    virtual void __stdcall entityDestroy() 
    { 
        delete this;
    }
public:
    /**
     * IInput
     */
    virtual input::IInputDevice* __stdcall createInputDevice(void)
    {
        return new InputDevice();
    }
    virtual char __stdcall getChar(int keyCode, bool shift)
    {
        switch( keyCode )
        {
        case DIK_1:          return !shift ? '1' : '!';
        case DIK_2:          return !shift ? '2' : '@';
        case DIK_3:          return !shift ? '3' : '#';
        case DIK_4:          return !shift ? '4' : '$';
        case DIK_5:          return !shift ? '5' : '%';
        case DIK_6:          return !shift ? '6' : '^';
        case DIK_7:          return !shift ? '7' : '&';
        case DIK_8:          return !shift ? '8' : '*';
        case DIK_9:          return !shift ? '9' : '(';
        case DIK_0:          return !shift ? '0' : ')';
        case DIK_MINUS:      return !shift ? '-' : '_';
        case DIK_EQUALS:     return !shift ? '=' : '+';  
        case DIK_Q:          return !shift ? 'q' : 'Q';
        case DIK_W:          return !shift ? 'w' : 'W';
        case DIK_E:          return !shift ? 'e' : 'E';
        case DIK_R:          return !shift ? 'r' : 'R';
        case DIK_T:          return !shift ? 't' : 'T';
        case DIK_Y:          return !shift ? 'y' : 'Y';
        case DIK_U:          return !shift ? 'u' : 'U';
        case DIK_I:          return !shift ? 'i' : 'I';
        case DIK_O:          return !shift ? 'o' : 'O';
        case DIK_P:          return !shift ? 'p' : 'P';
        case DIK_LBRACKET:   return !shift ? '[' : '{';
        case DIK_RBRACKET:   return !shift ? ']' : '}';
        case DIK_A:          return !shift ? 'a' : 'A';
        case DIK_S:          return !shift ? 's' : 'S';
        case DIK_D:          return !shift ? 'd' : 'D';
        case DIK_F:          return !shift ? 'f' : 'F';
        case DIK_G:          return !shift ? 'g' : 'G';
        case DIK_H:          return !shift ? 'h' : 'H';
        case DIK_J:          return !shift ? 'j' : 'J';
        case DIK_K:          return !shift ? 'k' : 'K';
        case DIK_L:          return !shift ? 'l' : 'L';
        case DIK_SEMICOLON:  return !shift ? ';' : ':';
        case DIK_APOSTROPHE: return !shift ? '\'' : '\"';
        case DIK_BACKSLASH:  return !shift ? '/' : '?';
        case DIK_Z:          return !shift ? 'z' : 'Z';
        case DIK_X:          return !shift ? 'x' : 'X';
        case DIK_C:          return !shift ? 'c' : 'C';
        case DIK_V:          return !shift ? 'v' : 'V';
        case DIK_B:          return !shift ? 'b' : 'B';
        case DIK_N:          return !shift ? 'n' : 'N';
        case DIK_M:          return !shift ? 'm' : 'M';
        case DIK_SLASH:      return !shift ? '\\' : '|';
        case DIK_SPACE:      return ' ';
        default: return 0;
        }
    }
};

Input* Input::instance = NULL;

SINGLE_ENTITY_COMPONENT(Input);


/**
 * windows DLL instance
 */

extern "C" int __declspec(dllexport) WINAPI DllMain(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved) 
{
    if( dwReason==DLL_PROCESS_ATTACH ) dllInstance = (HINSTANCE)hinstDLL;
    return TRUE;
}