
#include "headers.h"
#include "gui.h"

/**
 * class implementation
 */

GuiSlider::GuiSlider(TiXmlNode* node)
{
    // retrieve name
    _name = static_cast<TiXmlElement*>( node )->Attribute( "name" );

    // initialize control panel
    initializePanel( node );

    // determine type of slider
    const char* orientation = static_cast<TiXmlElement*>( node )->Attribute( "orientation" );                                                                              
    assert( orientation );
    if( strstr( orientation, "h" ) || strstr( orientation, "H" ) )
    {
        _horizontal = true;
    }
    else if( strstr( orientation, "v" ) || strstr( orientation, "V" ) )
    {
        _horizontal = false;
    }
    else
    {
        throw Exception( "Unknown slider type: \"%s\"", _name.c_str() );
    }
    if( strstr( orientation, "r" ) || strstr( orientation, "R" ) )
    {
        _reverse = true;
    }
    else
    {
        _reverse = false;
    }

    // search for scroll button
    _scrollButton = NULL;
    for( GuiPanelI panelI = _children.begin(); panelI != _children.end(); panelI++ )
    {
        if( strcmp( (*panelI)->getName(), "ScrollButton" ) == 0 )
        {
            _scrollButton = dynamic_cast<GuiButton*>( (*panelI)->getButton() );            
        }
    }
    assert( _scrollButton );

    // initialize autos
    if( _horizontal )
    {
        _lowerRect.left   = 0;
        _lowerRect.top    = 0;
        _lowerRect.right  = _rect.bottom - _rect.top;
        _lowerRect.bottom = _lowerRect.right;
        _upperRect.left   = ( _rect.right - _rect.left ) - _lowerRect.right;
        _upperRect.top    = 0;
        _upperRect.right  = ( _rect.right - _rect.left );
        _upperRect.bottom = _lowerRect.right;
    }
    else
    {
        _lowerRect.left   = 0;
        _lowerRect.top    = 0;
        _lowerRect.right  = _rect.right - _rect.left;    
        _lowerRect.bottom = _lowerRect.right;
        _upperRect.left   = 0;
        _upperRect.top    = ( _rect.bottom - _rect.top ) - _lowerRect.bottom;
        _upperRect.right  = _rect.right - _rect.left;
        _upperRect.bottom = ( _rect.bottom - _rect.top );
    }

    if( _reverse )
    {
        RECT temp = _lowerRect;
        _lowerRect = _upperRect;
        _upperRect = temp;
    }

    // update slider appearance
    _lowerLimit = 0;
    _upperLimit = 10;
    _pos        = 0;
    _scrolling  = false;
    updateAppearance();
}

GuiSlider::~GuiSlider()
{
}

/**
 * private behaviour
 */

void GuiSlider::updateAppearance(void)
{
    // lerp on scroll button rectangle
    float factor = float( _pos - _lowerLimit ) / float( _upperLimit - _lowerLimit );
    RECT rect;
    rect.left = int( float( _lowerRect.left ) * ( 1 - factor ) + float( _upperRect.left ) * factor );
    rect.top = int( float( _lowerRect.top ) * ( 1 - factor ) + float( _upperRect.top ) * factor );
    rect.right = int( float( _lowerRect.right ) * ( 1 - factor ) + float( _upperRect.right ) * factor );
    rect.bottom = int( float( _lowerRect.bottom ) * ( 1 - factor ) + float( _upperRect.bottom ) * factor );
    if( _scrollButton )
    {
        _scrollButton->setRect( gui::Rect( rect.left, rect.top, rect.right, rect.bottom ) );
    }
}

/**
 * abstractions
 */

void GuiSlider::onRender(void)
{
}

void GuiSlider::onMessage(gui::Message* message)
{
    // anywhere onMouseUp
    if( message->event == gui::onMouseUp )
    {
        if( _scrolling ) _scrolling = false;
    }

    // anywhere onMouseMove if scrolling is enabled
    if( _scrolling && message->event == gui::onMouseMove )
    {
        // slider vector
        Flector sliding( float(_upperRect.left - _lowerRect.left), float(_upperRect.top - _lowerRect.top) );
        Flector slidingN;
        D3DXVec2Normalize( &slidingN, &sliding );
        // motion offset
        Flector offset( float( Gui::instance->getMouseDX() ), float( Gui::instance->getMouseDY() ) );
        // motion offset in slider space
        Flector slidingOffset = slidingN * D3DXVec2Dot( &slidingN, &offset );
        // parametric motion offset (0..1)
        float pSlidingOffset = D3DXVec2Length( &slidingOffset ) / D3DXVec2Length( &sliding );
        if( D3DXVec2Dot( &slidingN, &offset ) < 0 ) pSlidingOffset *= -1;
        // final update
        setPosition( getPosition() + pSlidingOffset * getUpperLimit() - getLowerLimit() );

        // messaging
        gui::Message message;
        message.origin = this;
        message.event = gui::onSlide;
        Gui::instance->pushMessage( &message );
    }

    // mousedown from scroll button
    if( _scrollButton && 
        message->event == gui::onMouseDown && 
        message->origin == _scrollButton->getPanel() )
    {    
        _scrolling = true;
    }
}

/**
 * IGuiSlider
 */

gui::IGuiPanel* GuiSlider::getPanel(void)
{
    return this;
}

float GuiSlider::getLowerLimit(void)
{
    return _lowerLimit;
}

void GuiSlider::setLowerLimit(float value)
{
    _lowerLimit = value;
    if( _lowerLimit > _upperLimit ) _lowerLimit = _upperLimit;
    if( _pos < _lowerLimit ) _pos = _lowerLimit;
    updateAppearance();
}

float GuiSlider::getUpperLimit(void)
{
    return _upperLimit;
}

void GuiSlider::setUpperLimit(float value)
{
    _upperLimit = value;
    if( _upperLimit < _lowerLimit ) _upperLimit = _lowerLimit;
    if( _pos > _upperLimit ) _pos = _upperLimit;
    updateAppearance();
}

float GuiSlider::getPosition(void)
{
    return _pos;
}

void GuiSlider::setPosition(float value)
{
    _pos = value;
    if( _pos < _lowerLimit ) _pos = _lowerLimit;
    if( _pos > _upperLimit ) _pos = _upperLimit;
    updateAppearance();
}