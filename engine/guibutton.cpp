
#include "headers.h"
#include "gui.h"

/**
 * class implementatiocn
 */

GuiButton::GuiButton(const char* name, const char* fontName)
{
    // initialize control name
    _name = name;

    // initialize contol properties
    assert( Gui::instance->getFont( fontName ) );
    _font = fontName;    
    _fontColor = wrap( Vector4f( 0,0,0,1 ) );
    _activeColor = wrap( Vector4f( 1,1,1,1 ) );
    _inactiveColor = wrap( Vector4f( 0.5f,0.5f,0.5f,1 ) );

    // reset text
    _caption = L"";
}
   
GuiButton::GuiButton(TiXmlNode* node)
{
    assert( node->Type() == TiXmlNode::ELEMENT && strcmp( node->Value(), "button" ) == 0 );

    // initialize control name
    _name = static_cast<TiXmlElement*>( node )->Attribute( "name" );

    // initialize contol properties
    _font = static_cast<TiXmlElement*>( node )->Attribute( "font" );
    _fontColor = ::xmlColor( node, "fontColor" );
    _activeColor = ::xmlColor( node, "activeColor" );
    _inactiveColor = ::xmlColor( node, "inactiveColor" );

    // initialize text from localization resource
    int textId;
    static_cast<TiXmlElement*>( node )->Attribute( "textId", &textId );
    if( textId < 0 ) textId = 0;
    assert( Gui::iLanguage->getNumStrings() > unsigned int( textId ) );
    _caption = Gui::iLanguage->getUnicodeString( textId );

    // initialize control panel
    initializePanel( node );
}

GuiButton::~GuiButton(void)
{
}

/**
 * abstraction
 */

void GuiButton::onRender(void)
{
    RECT captionTextRect = clientToScreen( _rect );
    captionTextRect.left    += GUI_DEFAULT_INDENT,
    captionTextRect.top     += GUI_DEFAULT_INDENT,
    captionTextRect.right   -= GUI_DEFAULT_INDENT,
    captionTextRect.bottom  -= GUI_DEFAULT_INDENT;

    DWORD format = DT_WORDBREAK | DT_VCENTER | DT_CENTER;

    Gui::instance->renderUnicodeText(
        Gui::instance->getFont( _font.c_str() ),
        &captionTextRect, format,
        _fontColor,
        _caption.c_str() 
    );
}

void GuiButton::onMessage(gui::Message* message)
{
    if( message->origin == this ) switch( message->event )
    {
    case gui::onEnterCursor:
        _color = _activeColor;
        break;
    case gui::onLeaveCursor:
        _color = _inactiveColor;
        break;
    case gui::onMouseDown:
        if( message->mouseButton == gui::mbLeft ) Gui::instance->pushMessage( &gui::Message( this, gui::onButtonClick ) );
        break;
    }
}

/** 
 * IGuiButton
 */

gui::IGuiPanel* GuiButton::getPanel(void)
{
    return this;
}

Vector4f GuiButton::getActiveColor(void)
{
    return wrap( _activeColor );
}

void GuiButton::setActiveColor(Vector4f value)
{
    _activeColor = wrap( value );
}

Vector4f GuiButton::getInactiveColor(void)
{
    return wrap( _inactiveColor );
}

void GuiButton::setInactiveColor(Vector4f value)
{
    _inactiveColor = wrap( value );
}

const wchar_t* GuiButton::getCaption(void)
{
    return _caption.c_str();
}

void GuiButton::setCaption(const wchar_t* value)
{
    _caption = value;
}

Vector4f GuiButton::getTextColor(void)
{
    return wrap( _fontColor );
}

void GuiButton::setTextColor(Vector4f value)
{
    _fontColor = wrap( value );
}