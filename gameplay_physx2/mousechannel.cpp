
#include "headers.h"
#include "actionmap.h"
#include "gameplay.h"

float MouseChannel::mouseSensitivityX = 0.0025f;
float MouseChannel::mouseSensitivityY = 0.0025f;
float MouseChannel::mouseSensitivityZ = 0.0025f;

MouseChannel::MouseChannel(InputAction inputAction) : ActionChannel( inputAction )
{
    _amplitude   = 0.0f;
    setupDefault();
}

void MouseChannel::setupDefault(void)
{
    switch( _inputAction )
    {
    case iaLeft:
        _axis = ::xAxis;
        _direction = -1;
        break;
    case iaRight:
        _axis = ::xAxis;
        _direction = 1;
        break;
    case iaForward:
        _axis = ::yAxis;
        _direction = 1;
        break;
    case iaBackward:
        _axis = ::yAxis;
        _direction = -1;
        break;
    case iaLeftWarp:
        _axis = ::xAxis;
        _direction = -1;
        break;
    case iaRightWarp:
        _axis = ::xAxis;
        _direction = 1;
        break;
    case iaHeadLeft:
        _axis = ::xAxis;
        _direction = -1;
        break;
    case iaHeadRight:
        _axis = ::xAxis;
        _direction = 1;
        break;
    case iaHeadUp:
        _axis = ::yAxis;
        _direction = -1;
        break;
    case iaHeadDown:
        _axis = ::yAxis;
        _direction = 1;
        break;
    case iaZoomIn:
        _axis = ::zAxis;
        _direction = 1;
        break;
    case iaZoomOut:
        _axis = ::zAxis;
        _direction = -1;
        break;
    default:
        _axis = ::xAxis;
        _direction = 0;
    }
}

const wchar_t* MouseChannel::getInputActionHint(void)
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

void MouseChannel::setup(input::MouseState* mouseState, input::KeyboardState* keyboardState, input::JoyState* joystickState)
{
}

static bool samesgn(int value, int sgn)
{
    if( sgn < 0 ) return ( value <= 0 );
    if( sgn > 0 ) return ( value >= 0 );
    return false;
}

void MouseChannel::update(float dt, input::MouseState* mouseState, input::KeyboardState* keyboardState, input::JoyState* joystickState)
{
    _amplitude = 0.0f;
    if( dt == 0.0f ) return;
    switch( _axis )
    {
    case xAxis:
        if( samesgn( mouseState->dX, _direction ) ) 
        {
            _amplitude = fabs( float( mouseSensitivityX * mouseState->dX ) ) / dt;            
        }
        break;
    case yAxis:
        if( samesgn( mouseState->dY, _direction ) ) 
        {
            _amplitude = fabs( float( mouseSensitivityY * mouseState->dY ) ) / dt;
        }
        break;
    case zAxis:
        if( samesgn( mouseState->dZ, _direction ) )
        {
            _amplitude = fabs( float( mouseSensitivityZ * mouseState->dZ ) ) / dt;
        }
        break;
    }
    if( _amplitude > 1.0f ) _amplitude = 1.0f;
}

float MouseChannel::getAmplitude(void)
{
    return _amplitude;
}

void MouseChannel::setAmplitude(float amplitude)
{
	_amplitude = amplitude;
}
void MouseChannel::upAmplitude(float dt, float limit = 1.0f)
{
    _amplitude += dt;
	if (_amplitude > limit) _amplitude = limit;
}
void MouseChannel::downAmplitude(float dt, float limit = 0.0f)
{
    _amplitude -= dt;
	if (_amplitude < limit) _amplitude = limit;
}
bool MouseChannel::getTrigger(void)
{
    return ( _amplitude != 0 );
}

void MouseChannel::reset(void)
{
}