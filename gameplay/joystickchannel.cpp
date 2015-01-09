#include "headers.h"
#include "actionmap.h"
#include "gameplay.h"

JoystickChannel::JoystickChannel(InputAction inputAction) : ActionChannel( inputAction )
{
    _amplitude   = 0.0f;
    setupDefault();
}

void JoystickChannel::setupDefault(void)
{
    switch( _inputAction )
    {
    case iaLeftJoy:
        _axis = ::xAxisJoy;
        _direction = -1;
        break;
    case iaRightJoy:
        _axis = ::xAxisJoy;
        _direction = 1;
        break;
    case iaForward:
        _axis = ::yAxisJoy;
        _direction = -1;
        break;
    case iaBackwardJoy:
        _axis = ::yAxisJoy;
        _direction = 1;
        break;
    case iaLeftWarp:
        _axis = ::xAxisJoy;
        _direction = -1;
        break;
    case iaRightWarp:
        _axis = ::xAxisJoy;
        _direction = 1;
        break;
    case iaHeadLeft:
        _axis = ::xAxisJoy;
        _direction = -1;
        break;
    case iaHeadRight:
        _axis = ::xAxisJoy;
        _direction = 1;
        break;
    case iaHeadUp:
        _axis = ::yAxisJoy;
        _direction = -1;
        break;
    case iaHeadDown:
        _axis = ::yAxisJoy;
        _direction = 1;
        break;
    case iaZoomIn:
        _axis = ::zAxisJoy;
        _direction = 1;
        break;
    case iaZoomOut:
        _axis = ::zAxisJoy;
        _direction = -1;
        break;

	// buttons
	case iaCutAwayJoy:
		_axis = ::joyButton1;
        _direction = 1;
        break;

    default:
        _axis = ::xAxisJoy;
        _direction = 0;
    }
}

const wchar_t* JoystickChannel::getInputActionHint(void)
{
    assert( _direction != 0 );
    switch( _axis )
    {
    case xAxis:
        return Gameplay::iLanguage->getUnicodeString(150);
    case yAxis:
        return Gameplay::iLanguage->getUnicodeString(151);
    case zAxis:
        return Gameplay::iLanguage->getUnicodeString(152);
    }
    assert( "shouldn't be here!" );
    return NULL;
}

void JoystickChannel::setup(input::MouseState* mouseState, input::KeyboardState* keyboardState, input::JoyState* joystickState)
{
}

static bool samesgn(float value, float sgn)
{
    if( sgn < 0 ) return ( value <= 0 );
    if( sgn > 0 ) return ( value >= 0 );
    return false;
}

void JoystickChannel::update(float dt, input::MouseState* mouseState, input::KeyboardState* keyboardState, input::JoyState* joystickState)
{
	_amplitude = 0.0f;
	if (!joystickState->usable) return;

	// button update
	if ( _axis > zAxisJoy ) {
		if ( joystickState->buttonState[_axis - joyButton1] ) {
			_amplitude = 1.0f;
		}
	}
	// axis update
	else if( fabs(joystickState->axis[_axis]) > 0.0001f && samesgn( joystickState->axis[_axis], (float)_direction ) ) {
		_amplitude = fabs(joystickState->axis[_axis]);
	}
		


    if( _amplitude > 1.0f ) _amplitude = 1.0f;
	return;


    switch( _axis )
    {
    case xAxisJoy:
		if( samesgn( joystickState->axis[xAxisJoy], (float)_direction ) ) 
        {
            _amplitude = joystickState->axis[xAxisJoy];            
        }
        break;
    case yAxisJoy:
        if( samesgn( joystickState->axis[yAxisJoy], (float)_direction ) ) 
        {
            _amplitude = joystickState->axis[yAxisJoy];
				
        }
        break;
    case zAxisJoy:
        if( samesgn( joystickState->axis[zAxisJoy], (float)_direction ) )
        {
            _amplitude = joystickState->axis[zAxisJoy];
        }
        break;
    }

    if( _amplitude > 1.0f ) _amplitude = 1.0f;
}

float JoystickChannel::getAmplitude(void)
{
    return _amplitude;
}

void JoystickChannel::setAmplitude(float amplitude)
{
	_amplitude = amplitude;
}
void JoystickChannel::upAmplitude(float dt, float limit = 1.0f)
{
    _amplitude += dt;
	if (_amplitude > limit) _amplitude = limit;
}
void JoystickChannel::downAmplitude(float dt, float limit = 0.0f)
{
    _amplitude -= dt;
	if (_amplitude < limit) _amplitude = limit;
}
bool JoystickChannel::getTrigger(void)
{
    return ( _amplitude != 0 );
}

void JoystickChannel::reset(void)
{
}