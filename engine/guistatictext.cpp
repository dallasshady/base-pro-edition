
#include "headers.h"
#include "gui.h"

/**
 * class implementatiocn
 */
   
GuiStaticText::GuiStaticText(TiXmlNode* node)
{
    assert( node->Type() == TiXmlNode::ELEMENT && strcmp( node->Value(), "statictext" ) == 0 );

    // initialize control name
    _name = static_cast<TiXmlElement*>( node )->Attribute( "name" );

    // initialize contol properties
    _font = static_cast<TiXmlElement*>( node )->Attribute( "font" );
    _fontColor = ::xmlColor( node, "fontColor" );
    _verticalAlignment = ::xmlVerticalAlignment( node, "align" );
    _horizontalAlignment = ::xmlHorizontalAlignment( node, "align" );

    // initialize text from localization resource
    int textId;
    static_cast<TiXmlElement*>( node )->Attribute( "textId", &textId );
    if( textId < 0 ) textId = 0;
    assert( Gui::iLanguage->getNumStrings() > unsigned int( textId ) );
    _text = Gui::iLanguage->getUnicodeString( textId );

    // initialize control panel
    initializePanel( node );
}

GuiStaticText::~GuiStaticText(void)
{
}

/**
 * abstraction
 */

void GuiStaticText::onRender(void)
{
    RECT captionTextRect = clientToScreen( _rect );

    if( ( _rect.right - _rect.left > GUI_DEFAULT_INDENT * 3 ) &&
        ( _rect.bottom - _rect.top > GUI_DEFAULT_INDENT * 3 ) )
    {
        captionTextRect.left    += GUI_DEFAULT_INDENT,
        captionTextRect.top     += GUI_DEFAULT_INDENT,
        captionTextRect.right   -= GUI_DEFAULT_INDENT,
        captionTextRect.bottom  -= GUI_DEFAULT_INDENT;
    }

    DWORD format = 0 | DT_WORDBREAK;
    switch( _verticalAlignment )
    {
    case gui::atTop:    format = format | DT_TOP; break;
    case gui::atCenter: format = format | DT_VCENTER; break;
    case gui::atBottom: format = format | DT_BOTTOM; break;
    }
    switch( _horizontalAlignment )
    {
    case gui::atLeft:   format = format | DT_LEFT; break;
    case gui::atCenter: format = format | DT_CENTER; break;
    case gui::atRight:  format = format | DT_RIGHT; break;
    }

    Gui::instance->renderUnicodeText(
        Gui::instance->getFont( _font.c_str() ),
        &captionTextRect, format,
        _fontColor,
        _text.c_str() 
    );
}

/**
 * IGuiStaticText
 */
 
gui::IGuiPanel* GuiStaticText::getPanel(void)
{
    return this;
}

const wchar_t* GuiStaticText::getText(void)
{
    return _text.c_str();
}

void GuiStaticText::setText(const wchar_t* value)
{
    _text = value;
}

Vector4f GuiStaticText::getTextColor(void)
{
    return wrap( _fontColor );
}

void GuiStaticText::setTextColor(Vector4f value)
{
    _fontColor = wrap( value );
}