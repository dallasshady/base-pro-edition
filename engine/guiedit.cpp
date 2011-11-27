
#include "headers.h"
#include "gui.h"
#include "usp10.h"

/**
 * class implementation
 */

GuiEdit::GuiEdit(TiXmlNode* node)
{
    assert( node->Type() == TiXmlNode::ELEMENT && strcmp( node->Value(), "edit" ) == 0 );

    // initialize control
    _name = static_cast<TiXmlElement*>( node )->Attribute( "name" );
    _font = static_cast<TiXmlElement*>( node )->Attribute( "font" );
    _fontColor = ::xmlColor( node, "fontColor" );
    _caretTexture = ::xmlTexture( node, "caretTexture" ); assert( _caretTexture );   
    _maxLength = 128;
    static_cast<TiXmlElement*>( node )->Attribute( "maxLength", &_maxLength );
    if( _maxLength < 0 ) _maxLength = 0;
    _isNumerical = false;
    static_cast<TiXmlElement*>( node )->Attribute( "isNumerical", &_isNumerical );
    
    _text = "";
    _caretPos = 0;
    _strAnalysis = NULL; 

    // initialize panel
    initializePanel( node );
    
    analyseString();
}

GuiEdit::~GuiEdit()
{
    if( _strAnalysis ) ScriptStringFree( &_strAnalysis );
}

/**
 * IGuiEdit
 */

void GuiEdit::release(void)
{
    delete this;
}

gui::IGuiPanel* GuiEdit::getPanel(void)
{
    return this;
}

const char* GuiEdit::getText(void)
{
    return _text.c_str();
}

void GuiEdit::setText(const char* text)
{
    _text = text;
    analyseString();
    _caretPos = _text.length();
}

/**
 * abstraction & specification
 */

void GuiEdit::analyseString()
{
    if( _strAnalysis ) ScriptStringFree( &_strAnalysis );

    SCRIPT_CONTROL scriptControl;
    SCRIPT_STATE scriptState;
    ::memset( &scriptControl, 0, sizeof(scriptControl) );
    ::memset( &scriptState, 0, sizeof(scriptState) );
    ScriptApplyDigitSubstitution( NULL, &scriptControl, &scriptState );

    ID3DXFont* font = Gui::instance->getFont( _font.c_str() ); assert( font );

    ScriptStringAnalyse( font->GetDC(),
                         _text.c_str(),
                         (int) _text.length() + 1,
                         (int) _text.length() * 3 / 2 + 1,
                         DEFAULT_CHARSET,
                         SSA_GLYPHS | SSA_BREAK,
                         0,
                         &scriptControl,
                         &scriptState,
                         NULL,
                         NULL,
                         NULL,
                         &_strAnalysis 
    );

    const int* length = ScriptString_pcOutChars( _strAnalysis );
    assert( length );
    _glyphWidths.resize( (size_t) *length );
    ScriptStringGetLogicalWidths( _strAnalysis, &_glyphWidths[0] );
}

#pragma warning(disable:4018)
void GuiEdit::onRender(void)
{
    // render text
    RECT screenRect = clientToScreen( _rect );
    ID3DXFont* font = Gui::instance->getFont( _font.c_str() ); assert( font );
    Gui::instance->renderASCIIText( font, &screenRect, DT_LEFT | DT_TOP, _fontColor, _text.c_str() );
    
    // render caret
    if( this == Gui::instance->getKeyboardFocus() && Gui::instance->blinkIsVisible() )
    {
        // determine caret rectangle
        D3DXFONT_DESC fontDesc;
        font->GetDesc( &fontDesc );
        for( int i=0; i<_caretPos; i++ ) screenRect.left += _glyphWidths[i];        
        screenRect.bottom = screenRect.top + fontDesc.Height;
        if( _caretPos < int( _text.size() ) ) screenRect.top = screenRect.bottom - 2;
        screenRect.right = screenRect.left;
        if( i<_text.size() ) screenRect.right += _glyphWidths[i]; else screenRect.right += 2;

        RECT textureRect = { 0,0,8,8 };
        Color color = wrap( Vector4f( 1,1,1,1 ) );
        Gui::instance->renderRect( screenRect, _caretTexture, textureRect, color );
    }
}
#pragma warning(default:4018)

void GuiEdit::onMessage(gui::Message* message)
{
    // filter control by keyboard focus to begin KB message handling
    if( this != Gui::instance->getKeyboardFocus() ) return;   
    switch( message->event )
    {
    case gui::onKeyDown:
        switch( message->keyCode )
        {
        case VK_LEFT:
        case VK_NUMPAD4:
            _caretPos--, _caretPos = _caretPos > 0 ? _caretPos : 0;
            break;
        case VK_RIGHT:
        case VK_NUMPAD6:
            _caretPos++, _caretPos = _caretPos <= int(_text.size()) ? _caretPos : _text.size();
            break;
        case VK_UP:
        case VK_NUMPAD8:
        case VK_HOME:
            _caretPos = 0;
            break;
        case VK_DOWN:
        case VK_NUMPAD2:
        case VK_END:
            _caretPos = _text.size();            
            break;
        case VK_BACK:
            // delete character before caret position
            if( _caretPos > 0 )
            {
                std::string temp;
                for( int i=0; i<_caretPos-1; i++ ) temp += _text[i];
                for( i=_caretPos; i<int(_text.size()); i++ ) temp += _text[i];
                _caretPos--;
                if( _caretPos < 0 ) _caretPos = 0;
                _text = temp;
                analyseString();
            }
            break;
        case VK_DELETE:
        case VK_DECIMAL:
            // delete character at caret position
            if( _caretPos < int( _text.size() ) )
            {
                std::string temp;
                for( int i=0; i<_caretPos; i++ ) temp += _text[i];
                for( i=_caretPos+1; i<int(_text.size()); i++ ) temp += _text[i];
                if( _caretPos > int(_text.size()) ) _caretPos = _text.size();
                _text = temp;
                analyseString();
            }
            break;
        }
        break;
    case gui::onKeyChar:        
        // insert character at caret position
        if( ( _text.length() < unsigned int( _maxLength ) ) && unsigned char( message->keyChar ) >= 0x20 )
        {
            bool characterIsApproved = true;
            if( _isNumerical )
            {
                characterIsApproved = ( message->keyChar == '-' ||
                                        message->keyChar == '.' ||
                                        message->keyChar == '0' || 
                                        message->keyChar == '1' ||
                                        message->keyChar == '2' ||
                                        message->keyChar == '3' ||
                                        message->keyChar == '4' ||
                                        message->keyChar == '5' ||
                                        message->keyChar == '6' ||
                                        message->keyChar == '7' ||
                                        message->keyChar == '8' ||
                                        message->keyChar == '9' );
            }
            if( characterIsApproved )
            {
                std::string temp;
                for( int i=0; i<_caretPos; i++ ) temp += _text[i];
                temp += char( message->keyChar );
                for( i=_caretPos; i<int(_text.size()); i++ ) temp += _text[i];
                _caretPos++;
                _text = temp;
                analyseString();
            }
        }
        break;
    }
}