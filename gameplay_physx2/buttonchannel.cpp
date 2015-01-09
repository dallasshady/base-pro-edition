
#include "headers.h"
#include "actionmap.h"
#include "gameplay.h"

ButtonChannel::ButtonChannel(InputAction inputAction) : ActionChannel( inputAction )
{
    _smoothMode    = false;
    _amplitude     = 0.0f;
    _ascendingVel  = 0.0f;
    _descendingVel = 0.0f;    
    setupDefault();
}

ButtonChannel::ButtonChannel(InputAction inputAction, float ascendingVel, float descendingVel) : ActionChannel( inputAction )
{
    _inputAction   = inputAction;
    _smoothMode    = true;
    _amplitude     = 0.0f;
    _ascendingVel  = ascendingVel;
    _descendingVel = descendingVel;
    setupDefault();
}

void ButtonChannel::setupDefault(void)
{
    _deviceCode = 0;
    switch( _inputAction )
    {
    case iaLeft:
        _keyCode = 0x1E; // A
        break;
    case iaRight:
        _keyCode = 0x20; // D
        break;
    case iaForward:
        _keyCode = 0x11; // W
        break;
    case iaBackward:
        _keyCode = 0x1F; // S
        break;
    case iaLeftWarp:
        _keyCode = 0x10; // Q
        break;
    case iaRightWarp:
        _keyCode = 0x12; // E
        break;
    case iaLeftRearRiser:
        _keyCode = 0x2C; // Z
        break;
    case iaRightRearRiser:
        _keyCode = 0x2E; // C
        break;
    case iaRearBrake:
        _keyCode = 0x2D; // X
        break;
    case iaHeadLeft:
        _keyCode = 0x4B; // Num 4
        break;
    case iaHeadRight:
        _keyCode = 0x4D; // Num 6
        break;
    case iaHeadUp:
        _keyCode = 0x48; // Num 8
        break;
    case iaHeadDown:
        _keyCode = 0x50; // Num 2        
        break;
    case iaZoomIn:
        _keyCode = 0x4E; // Num +
        break;
    case iaZoomOut:
        _keyCode = 0x4A; // Num -
        break;
    case iaCameraMode0:
        _keyCode = 0x3B; // F1
        break;
    case iaCameraMode1:
        _keyCode = 0x3C; // F2
        break;
    case iaCameraMode2:
        _keyCode = 0x3D; // `
        break;
    case iaCameraMode3:
        _keyCode = 0x58; // F12
        break;
    case iaPhase:
        _keyCode = 0x39; // Space
        break;
    case iaModifier:
        _keyCode = 0x2A; // Left shift
        break;    
    case iaGlobalDeceleration:
        _keyCode = 0x4A; // NUM -
        break;
    case iaGlobalAcceleration:
        _keyCode = 0x4E; // NUM +
        break;
    case ::iaAltimeterMode:
        _keyCode = 0x19; // P
        break;
    case ::iaIncreaseWarningAltitude:
        _keyCode = 0x1B; // ]
        break;
    case ::iaDecreaseWarningAltitude:
        _keyCode = 0x1A; // [
        break;
    case ::iaSwitchHUDMode:
        _keyCode = 0x23; // H
        break;
    case ::iaWLO:
        _keyCode = 0x02; // 1
        break;
    case ::iaHook:
        _keyCode = 0x03; // 2
        break;
    case ::iaAccelerateFlightTime:
        _keyCode = 0x0D; // =
        break;
    case ::iaDecelerateFlightTime:
        _keyCode = 0x0C; // -
        break;
    case ::iaIncreaseMusicVolume:
        _keyCode = 0x49; // Num 9
        break;
    case ::iaDecreaseMusicVolume:
        _keyCode = 0x51; // Num 3
        break;
    case ::iaCutAway:
        _keyCode = 0x0E; // Backspace
        break;
	case ::iaPullReserve:
        _keyCode = 0x1C; // Enter
        break;

    case iaReserveLeft:
        _keyCode = 0x24; // J
        break;
    case iaReserveRight:
        _keyCode = 0x26; // L
        break;
    case iaReserveLeftWarp:
        _keyCode = 0x16; // U
        break;
    case iaReserveRightWarp:
        _keyCode = 0x18; // O
		break;
    case iaReserveLeftRearRiser:
        _keyCode = 0x32; // M
        break;
    case iaReserveRightRearRiser:
        _keyCode = 0x34; // >
        break;
    }
}

const wchar_t* ButtonChannel::getInputActionHint(void)
{
    if( _deviceCode == 0 ) switch( _keyCode )
    {
    case 0x01: return L"Escape";
    case 0x02: return L"1";
    case 0x03: return L"2";
    case 0x04: return L"3";
    case 0x05: return L"4";
    case 0x06: return L"5";
    case 0x07: return L"6";
    case 0x08: return L"7";
    case 0x09: return L"8";
    case 0x0A: return L"9";
    case 0x0B: return L"0";
    case 0x0C: return L"-";
    case 0x0D: return L"=";
    case 0x0E: return L"Back";
    case 0x0F: return L"Tab";
    case 0x10: return L"Q";
    case 0x11: return L"W";
    case 0x12: return L"E";
    case 0x13: return L"R";
    case 0x14: return L"T";
    case 0x15: return L"Y";
    case 0x16: return L"U";
    case 0x17: return L"I";
    case 0x18: return L"O";
    case 0x19: return L"P";
    case 0x1A: return L"[";
    case 0x1B: return L"]";
    case 0x1C: return L"Return";
    case 0x1D: return L"Left Ctrl";
    case 0x1E: return L"A";
    case 0x1F: return L"S";
    case 0x20: return L"D";
    case 0x21: return L"F";
    case 0x22: return L"G";
    case 0x23: return L"H";
    case 0x24: return L"J";
    case 0x25: return L"K";
    case 0x26: return L"L";
    case 0x27: return L";";
    case 0x28: return L"\"";
    case 0x29: return L"|";
    case 0x2A: return L"Left shift";
    case 0x2B: return L"\\";
    case 0x2C: return L"Z";
    case 0x2D: return L"X";
    case 0x2E: return L"C";
    case 0x2F: return L"V";
    case 0x30: return L"B";
    case 0x31: return L"N";
    case 0x32: return L"M";
    case 0x33: return L"<";
    case 0x34: return L">";
    case 0x35: return L"/";
    case 0x36: return L"Right shift";
    case 0x37: return L"Num *";
    case 0x38: return L"Left Alt";
    case 0x39: return L"Space";
    case 0x3B: return L"F1";
    case 0x3C: return L"F2";
    case 0x3D: return L"F3";
    case 0x3E: return L"F4";
    case 0x3F: return L"F5";
    case 0x40: return L"F6";
    case 0x41: return L"F7";
    case 0x42: return L"F8";
    case 0x43: return L"F9";
    case 0x44: return L"F10";
    case 0x45: return L"Num lock";
    case 0x46: return L"Scroll lock";
    case 0x47: return L"Num 7";
    case 0x48: return L"Num 8";
    case 0x49: return L"Num 9";
    case 0x4A: return L"Num -";
    case 0x4B: return L"Num 4";
    case 0x4C: return L"Num 5";
    case 0x4D: return L"Num 6";
    case 0x4E: return L"Num +";
    case 0x4F: return L"Num 1";
    case 0x50: return L"Num 2";
    case 0x51: return L"Num 3";
    case 0x52: return L"Num 0";
    case 0x53: return L"Num .";
    case 0x57: return L"F11";
    case 0x58: return L"F12";
    case 0xB8: return L"Right alt";
    case 0xC5: return L"Pause";
    case 0xC7: return L"Home";
    case 0xC8: return L"Up";
    case 0xC9: return L"Page up";
    case 0xCB: return L"Left";
    case 0xCD: return L"Right";
    case 0xCF: return L"End";
    case 0xD0: return L"Down";
    case 0xD1: return L"Page down";
    case 0xD2: return L"Insert";
    case 0xD3: return L"Delete";
    }
    else if( _deviceCode == 1) switch( _keyCode )
    {
    case 0: return L"Mouse 1";
    case 1: return L"Mouse 2";
    case 2: return L"Mouse 3";
	}
	else if( _deviceCode == 2) switch( _keyCode )
	{
		case 0: return L"Joystick button 1";
		case 1: return L"Joystick button 2";
		case 2: return L"Joystick button 3";
	}
    return NULL;
}

void ButtonChannel::setup(input::MouseState* mouseState, input::KeyboardState* keyboardState, input::JoyState* joystickState)
{
	unsigned int i;
    for( i=0; i<255; i++ )
    {
        if( keyboardState->keyState[i] & 0x80 )
        {
            // remember previous code
            unsigned int temp1 = _deviceCode;
            unsigned int temp2 = _keyCode;
            // validate key
            _deviceCode = 0;
            _keyCode = i;
            if( getInputActionHint() ) return;            
            // restore key
            _deviceCode = temp1;
            _keyCode = temp2;
        }
    }
    for( i=0; i<3; i++ )
    {
        if( mouseState->buttonState[i] & 0x80 )
        {
            _keyCode = i;
            _deviceCode = 1;
            return;
        }
    }
}

void ButtonChannel::setup(unsigned int deviceCode, unsigned int keyCode)
{
    unsigned int temp1 = _deviceCode;
    unsigned int temp2 = _keyCode;

    _deviceCode = deviceCode;
    _keyCode = keyCode;
    if( getInputActionHint() ) return;
    
    _deviceCode = temp1;
    _keyCode = temp2;
	_trigger = false;
}

void ButtonChannel::update(float dt, input::MouseState* mouseState, input::KeyboardState* keyboardState, input::JoyState* joystickState)
{
	// joystick
	if (_deviceCode == 2) {
		if (joystickState->buttonState[_keyCode]) {
			_amplitude = 1.0f;
		} else {
			_amplitude = 0.0f;
		}
		return;
	}


    if( _smoothMode )
    {
        if( ( _deviceCode == 0 && keyboardState->keyState[_keyCode] & 0x80 ) ||
            ( _deviceCode == 1 && mouseState->buttonState[_keyCode] & 0x80 ) )
        {
            _amplitude += _ascendingVel * dt;
            if( _amplitude > 1.0f ) _amplitude = 1.0f;
			_trigger = true;
        }
        else
        {
            _amplitude -= _descendingVel * dt;
            if( _amplitude < 0.0f ) _amplitude = 0.0f;
			_trigger = false;
        }
    } 
    else
    {
        if( ( _deviceCode == 0 && keyboardState->keyState[_keyCode] & 0x80 ) ||
            ( _deviceCode == 1 && mouseState->buttonState[_keyCode] & 0x80 ) )
        {
            _amplitude = 1.0f;
			_trigger = true;
        }
        else
        {
            _amplitude = 0.0f;
			_trigger = false;
        }
    }    
}

float ButtonChannel::getAmplitude(void)
{
    return _amplitude;
}

void ButtonChannel::setAmplitude(float amplitude)
{
    _amplitude = amplitude;
}
void ButtonChannel::upAmplitude(float dt, float limit = 1.0f)
{
    _amplitude += _ascendingVel * dt;
	if (_amplitude > limit) _amplitude = limit;
}
void ButtonChannel::downAmplitude(float dt, float limit = 0.0f)
{
    _amplitude -= _ascendingVel * dt;
	if (_amplitude < limit) _amplitude = limit;
}
bool ButtonChannel::getTrigger(void)
{
    //return ( _amplitude != 0.0f );
	return _trigger;
}

void ButtonChannel::reset(void)
{
	_trigger = false;
    _amplitude = 0.0f;    
}