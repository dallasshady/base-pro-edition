
#include "headers.h"
#include "gui.h"
#include "sprite.h"

#ifndef WHEEL_DELTA
    #define WHEEL_DELTA 120
#endif

#ifndef GET_WHEEL_DELTA_WPARAM
    #define GET_WHEEL_DELTA_WPARAM(wParam) ((((int)wParam) >> 16)/WHEEL_DELTA)
#endif

Gui*                 Gui::instance = NULL;
language::ILanguage* Gui::iLanguage = NULL;

/**
 * class implementation
 */

Gui::Gui()
{
    // reset gui elements
    _guiDocument = NULL;
    _sprite      = NULL;
    _messageCallback = NULL;
    _messageCallbackUserData = NULL;
    _desktop = NULL;
    _panelUnderCursor  = NULL;
    _keyboardFocus = NULL;
    _blinkIsVisible = true;
    _mouseDX = _mouseDY = 0;
    _blinkTimeout = GUI_CARET_BLINK;
    _cursorIsVisible = true;

    // no hint mode by default
    _hintModeEnabled     = true;
    _hintTimeout         = 1.0f;
    _hintTimeAccumulator = 0.0f;

    // initialize useful interfaces
    queryInterface( "Language", &iLanguage ); assert( iLanguage );

    // initialize instance pointer
    instance = this;
}

Gui::~Gui()
{
    if( _desktop ) _desktop->release();
    if( _sprite ) _dxCR( _sprite->Release() );
    if( _guiDocument ) delete _guiDocument;

    // release fonts
    for( FontI fontI=_fontM.begin(); fontI!=_fontM.end(); fontI++ )
    {
        fontI->second->Release();
    }

    iLanguage = NULL;
    instance = NULL;
}

/**
 * component support
 */

EntityBase* Gui::creator()
{
    return new Gui;
}

void Gui::entityDestroy()
{
    delete this;
}

/** 
 * EntityBase
 */

void Gui::entityInit(Object * p)
{
    // register trigger listening
    getCore()->trigAddListener( mainwnd::TriggerMainwnd::tid, getid() );

    // load gui xml document
    // open main xml document, described gui sustem
    _guiDocument = new TiXmlDocument( "./res/gui/gui.xml" );
    _guiDocument->LoadFile();

    // load textures
    TiXmlNode* child = _guiDocument->FirstChild();
    do 
    {
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "texture" ) == 0 )
        {
            // read resource
            std::string path = static_cast<TiXmlElement*>( child )->Attribute( "resource" );
            path = strtrim( path, "\"" );
            engine::ITexture* texture = Engine::instance->createTexture( path.c_str() );
            texture->addReference();
        }
        child = child->NextSibling();
    }
    while( child != NULL );

    // initialize cursor
    bool cursorIsInitialized = false;
    child = _guiDocument->FirstChild();
    do 
    {
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "cursor" ) == 0 )
        {
            _cursorTexture     = xmlTexture( child, "texture" );
            _cursorColor       = xmlColor( child, "color" );
            _cursorRect        = xmlRect( child, "rect" );
            _cursorTextureRect = xmlRect( child, "uv" );
            cursorIsInitialized = true;
            break;
        }
        child = child->NextSibling();
    }
    while( child != NULL );
    assert( cursorIsInitialized );

    // create desktop
    Vector3f screenSize = Engine::instance->getScreenSize();
    unsigned int screenWidth = unsigned int( screenSize[0] );
    unsigned int screenHeight = unsigned int( screenSize[1] );
    _desktop = new GuiPanel( "GuiDesktop" ); assert( _desktop );
    _desktop->setRect( gui::Rect( 0, 0, screenWidth, screenHeight ) );
    _desktop->setTextureRect( gui::Rect( 0,0,0,0 ) );
    _desktop->setColor( Vector4f( 0,0,0,0 ) );
	
    // initialize desktop
    child = _guiDocument->FirstChild();
    do 
    {
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "desktop" ) == 0 )
        {
            _desktop->setTexture( xmlTexture( child, "texture" ) );
            RECT rect = xmlRect( child, "uv" );
            _desktop->setTextureRect( gui::Rect( rect.left, rect.top, rect.right, rect.bottom ) );
            _desktop->setColor( wrap( xmlColor( child, "color" ) ) );
        }
        child = child->NextSibling();
    }
    while( child != NULL );

    // create default font
    child = _guiDocument->FirstChild();
    do
    {
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "font" ) == 0 )
        {
            const char* name = static_cast<TiXmlElement*>( child )->Attribute( "name" ); assert( name );
            const char* face = static_cast<TiXmlElement*>( child )->Attribute( "face" ); assert( face );
            int height=10, width=10, weight=0, miplevels=1, italic=0, charset=0;
            static_cast<TiXmlElement*>( child )->Attribute( "height", &height );
            static_cast<TiXmlElement*>( child )->Attribute( "width", &width );
            static_cast<TiXmlElement*>( child )->Attribute( "weight", &weight );
            static_cast<TiXmlElement*>( child )->Attribute( "miplevels", &miplevels );
            static_cast<TiXmlElement*>( child )->Attribute( "italic", &italic );
            static_cast<TiXmlElement*>( child )->Attribute( "charset", &charset );
            ID3DXFont* font = NULL;
            _dxCR( D3DXCreateFont( 
                iDirect3DDevice,
                height,
                width,
                (weight!=0)?FW_BOLD:0,
                miplevels,
                (italic!=0),
                charset,
                OUT_DEFAULT_PRECIS,
                ANTIALIASED_QUALITY,
                0,
                face,
                &font
            ) );
            assert( font );
            _fontM.insert( FontT( name, font ) );
        }
        child = child->NextSibling();
    }
    while( child != NULL );

    // initialize render helpers
    D3DXCreateSprite( iDirect3DDevice, &_sprite );
}

void Gui::entityAct(float dt)
{
    // process blink
    _blinkTimeout -= dt;
    if( _blinkTimeout < 0 )
    {
        _blinkIsVisible = !_blinkIsVisible;
        _blinkTimeout = GUI_CARET_BLINK;
    }

    // process hint
    if( _hintModeEnabled ) _hintTimeAccumulator += dt;

    // process messages
    gui::Message message;
    while( _messageL.size() )
    {
        message = *_messageL.begin(); 
        _messageL.erase( _messageL.begin() );
        _desktop->dispatchMessage( &message );
        if( NULL != _messageCallback ) 
        {
            _messageCallback( &message, _messageCallbackUserData );
        }
    }
}

void Gui::entityHandleEvent(evtid_t id, trigid_t trigId, Object* param)
{
    if( trigId == mainwnd::TriggerMainwnd::tid )
    {
        // window message
        Holder<MSG>* p = static_cast<Holder<MSG>*>( param );
        // dispatch message
        switch( p->value.message )
        {
        case WM_MOUSEWHEEL:
            if( _panelUnderCursor ) pushMessage( &gui::Message( _panelUnderCursor, gui::onMouseWheel, -GET_WHEEL_DELTA_WPARAM(p->value.wParam), -GET_WHEEL_DELTA_WPARAM(p->value.wParam) ) );
            break;
        case WM_KEYUP:
            if( _panelUnderCursor ) pushMessage( &gui::Message( _panelUnderCursor, gui::onKeyUp, int( p->value.wParam ) ) );
            break;
        case WM_KEYDOWN:
            if( _panelUnderCursor ) pushMessage( &gui::Message( _panelUnderCursor, gui::onKeyDown, int( p->value.wParam ) ) );
            break;
        case WM_CHAR:
            if( _panelUnderCursor ) pushMessage( &gui::Message( _panelUnderCursor, gui::onKeyChar, char( p->value.wParam ) ) );
            break;
        case WM_LBUTTONDOWN:
            if( _panelUnderCursor ) pushMessage( &gui::Message( _panelUnderCursor, gui::onMouseDown, gui::mbLeft ) );
            _keyboardFocus = _panelUnderCursor;
            break;
        case WM_RBUTTONDOWN:
            if( _panelUnderCursor ) pushMessage( &gui::Message( _panelUnderCursor, gui::onMouseDown, gui::mbRight ) );
            break;
        case WM_MBUTTONDOWN:
            if( _panelUnderCursor ) pushMessage( &gui::Message( _panelUnderCursor, gui::onMouseDown, gui::mbMiddle ) );
            break;
        case WM_LBUTTONUP:
            if( _panelUnderCursor ) pushMessage( &gui::Message( _panelUnderCursor, gui::onMouseUp, gui::mbLeft ) );
            break;
        case WM_RBUTTONUP:
            if( _panelUnderCursor ) pushMessage( &gui::Message( _panelUnderCursor, gui::onMouseUp, gui::mbRight ) );
            break;
        case WM_MBUTTONUP:
            if( _panelUnderCursor ) pushMessage( &gui::Message( _panelUnderCursor, gui::onMouseUp, gui::mbMiddle ) );
            break;
        case WM_MOUSEMOVE:
            // reset hint accumulator
            _hintTimeAccumulator = 0;
            // update cursor position
            _mouseDX = -GET_X_LPARAM( p->value.lParam );
            _mouseDY = -GET_Y_LPARAM( p->value.lParam );
            _cursorRect.left   += _mouseDX;
            _cursorRect.right  += _mouseDX;
            _cursorRect.top    += _mouseDY;
            _cursorRect.bottom += _mouseDY;
            // crop position by desktop bounds
            gui::Rect desktopRect = _desktop->getRect();
            if( _cursorRect.left < desktopRect.left ) 
            {
                int width = _cursorRect.right - _cursorRect.left;
                _cursorRect.left = desktopRect.left;
                _cursorRect.right = _cursorRect.left + width;
            }
            if( _cursorRect.top < desktopRect.top ) 
            {
                int height = _cursorRect.bottom - _cursorRect.top;
                _cursorRect.top = desktopRect.top;
                _cursorRect.bottom = _cursorRect.top + height;
            }
            if( _cursorRect.left > desktopRect.right ) 
            {
                int width = _cursorRect.right - _cursorRect.left;
                _cursorRect.left = desktopRect.right;
                _cursorRect.right = _cursorRect.left + width;
            }
            if( _cursorRect.top > desktopRect.bottom ) 
            {
                int height = _cursorRect.bottom - _cursorRect.top;
                _cursorRect.top = desktopRect.bottom;
                _cursorRect.bottom = _cursorRect.top + height;
            }
            // detect panel under cursor
            GuiPanel* panel = _desktop->getPanelUnderScreenPosition( 
                _cursorRect.left, _cursorRect.top 
            );
            if( panel != _panelUnderCursor )
            {
                if( _panelUnderCursor != NULL )
                {
                    pushMessage( &gui::Message( _panelUnderCursor, gui::onLeaveCursor ) );
                }
                _panelUnderCursor = panel;
                if( _panelUnderCursor != NULL )
                {
                    pushMessage( &gui::Message( _panelUnderCursor, gui::onEnterCursor ) );
                }
            }
            // mousemove message
            if( _panelUnderCursor )
            {
                pushMessage( &gui::Message( _panelUnderCursor, gui::onMouseMove, _cursorRect.left, _cursorRect.top ) );
            }
            break;
        }
    }
}

IBase* Gui::entityAskInterface(iid_t id)
{
    if( id == gui::IGui::iid ) return this;
    return NULL;
}

/**
 * IGui
 */

void Gui::setCursorColor(const Vector4f& value)
{
    _cursorColor = wrap( value );
}

void Gui::setCursorVisible(bool flag)
{
    _cursorIsVisible = flag;
}

void Gui::setHintMode(bool showHints, float timeout)
{
    _hintModeEnabled     = showHints;
    _hintTimeout         = ( timeout < 0 ) ? 0 : timeout;
    _hintTimeAccumulator = 0.0f;
}

gui::IGuiPanel* Gui::getDesktop(void)
{
    return _desktop;
}

gui::IGuiPanel* Gui::getPanelUnderCursor(void)
{
    return _panelUnderCursor;
}

gui::Rect Gui::getCursorRect(void)
{
    return gui::Rect( _cursorRect.left, _cursorRect.top, _cursorRect.right, _cursorRect.bottom );
}

gui::IGuiPanel* Gui::createPanel(const char* panelName)
{
    return new GuiPanel( panelName );
}

gui::IGuiWindow* Gui::createWindow(const char* windowName)
{
    // search for node describing the window
    TiXmlNode* child = _guiDocument->FirstChild();
    do 
    {
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "window" ) == 0 )
        {
            if( strcmp( static_cast<TiXmlElement*>( child )->Attribute( "name" ), windowName ) == 0 )
            {
                return new GuiWindow( child );
            }
        }
        child = child->NextSibling();
    }
    while( child != NULL );
    return NULL;
}

gui::IGuiButton* Gui::createButton(const char* name, const char* fontName)
{
    return new GuiButton( name, fontName );
}

void Gui::render(void)
{
    // turn off z-test & z-write
    dxSetRenderState( D3DRS_ZENABLE, FALSE );
    dxSetRenderState( D3DRS_ZWRITEENABLE, FALSE );

    // begin render
    _dxCR( _sprite->Begin( D3DXSPRITE_DONOTSAVESTATE | D3DXSPRITE_ALPHABLEND ) );

    // render desktop & layered windows
    _desktop->render();

    // render cursor
    if( _cursorIsVisible )
    {
        renderRect( _cursorRect, _cursorTexture, _cursorTextureRect, _cursorColor );
    }

    // if hint can be showed
    if( _hintModeEnabled && ( _hintTimeAccumulator > _hintTimeout ) &&
        _panelUnderCursor && _panelUnderCursor->getHintString() )
    {
        // hint font
        ID3DXFont* hintFont = getFont( "hint" ); assert( hintFont );
        // calculate hint rect
        RECT hintRect;
        hintRect.left = 0;
        hintRect.right = 320;
        hintRect.top = 0;
        hintRect.bottom = 48;
        int result = hintFont->DrawTextW(
            _sprite,
            _panelUnderCursor->getHintString(),
            -1,
            &hintRect,
            DT_VCENTER | DT_CENTER | DT_WORDBREAK | DT_CALCRECT,
            ::black
        );
        if( result )
        {
            // rect properties
            int rectWidth  = hintRect.right - hintRect.left + 8;
            int rectHeight = hintRect.bottom - hintRect.top + 8;
            // finalize rect
            hintRect.left   = _cursorRect.left;
            hintRect.right  = hintRect.left + rectWidth;
            hintRect.top    = _cursorRect.bottom;
            hintRect.bottom = hintRect.top + rectHeight;
            // align rect
            Vector3f screenSize = Engine::instance->getScreenSize();
            if( hintRect.right > int( screenSize[0] ) )
            {
                int offset = hintRect.right - int( screenSize[0] );
                hintRect.right -= offset;
                hintRect.left -= offset;
            }
            if( hintRect.bottom > int( screenSize[1] ) )
            {
                int offset = hintRect.bottom - int( screenSize[1] );
                hintRect.bottom -= offset;
                hintRect.top -= offset;
            }
            // outline rect
            RECT outlineRect = hintRect;
            outlineRect.left -= 1, outlineRect.top -= 1,
            outlineRect.right += 1, outlineRect.bottom += 1;
            // texture rect
            RECT textureRect;
            textureRect.left = textureRect.top = 0;
            textureRect.right = textureRect.bottom = 7;
            // render rect
            TextureI whiteI = Texture::textures.find( "white" );
            assert( whiteI != Texture::textures.end() );
            renderRect( outlineRect, whiteI->second, textureRect, ::black );
            renderRect( hintRect, whiteI->second, textureRect, ::white );
            // render text
            renderUnicodeText( hintFont, &hintRect, DT_VCENTER | DT_CENTER | DT_WORDBREAK, ::black, _panelUnderCursor->getHintString() );
        }
    }

    // end render
    _dxCR( _sprite->End() );

    // turn on z-test & z-write
    dxSetRenderState( D3DRS_ZENABLE, TRUE );
    dxSetRenderState( D3DRS_ZWRITEENABLE, TRUE );
}

void Gui::renderRect(const gui::Rect& rect, engine::ITexture* texture, const gui::Rect& textureRect, const Vector4f& color, bool spriteBeginEnd)
{
    RECT r = { rect.left, rect.top, rect.right, rect.bottom };
    RECT tr = { textureRect.left, textureRect.top, textureRect.right, textureRect.bottom };
    Color c = wrap( color );
    Texture* t = dynamic_cast<Texture*>( texture );
    assert( t );
	
    // begin sprite render
    if( spriteBeginEnd ) _dxCR( _sprite->Begin( D3DXSPRITE_DONOTSAVESTATE | D3DXSPRITE_ALPHABLEND ) );

    // turn off z-test & z-write
    dxSetRenderState( D3DRS_ZENABLE, FALSE );
    dxSetRenderState( D3DRS_ZWRITEENABLE, FALSE );
    
    renderRect( r, t, tr, c );

    // end sprite render
    if( spriteBeginEnd ) _dxCR( _sprite->End() );

    // turn on z-test & z-write
    dxSetRenderState( D3DRS_ZENABLE, TRUE );
    dxSetRenderState( D3DRS_ZWRITEENABLE, TRUE );
}

void Gui::renderUnicodeText(const gui::Rect& rect, const char* fontName, const Vector4f& color, gui::AlignmentType vAlign, gui::AlignmentType hAlign, bool wordWrap, const wchar_t* text, bool spriteBeginEnd)
{
    // obtain font
    FontI fontI = _fontM.find( fontName );
    assert( fontI != _fontM.end() );

    RECT r = { rect.left, rect.top, rect.right, rect.bottom };
	
    DWORD format = 0;
    switch( vAlign )
    {
    case gui::atTop:    format = format | DT_TOP; break;
    case gui::atCenter: format = format | DT_VCENTER; break;
    case gui::atBottom: format = format | DT_BOTTOM; break;
    }
    switch( hAlign )
    {
    case gui::atLeft:   format = format | DT_LEFT; break;
    case gui::atCenter: format = format | DT_CENTER; break;
    case gui::atRight:  format = format | DT_RIGHT; break;
    }
    if( wordWrap ) format = format | DT_WORDBREAK;

    // begin sprite render
    if( spriteBeginEnd ) _dxCR( _sprite->Begin( D3DXSPRITE_DONOTSAVESTATE | D3DXSPRITE_ALPHABLEND ) );

    // turn off z-test & z-write
    dxSetRenderState( D3DRS_ZENABLE, FALSE );
    dxSetRenderState( D3DRS_ZWRITEENABLE, FALSE );

    renderUnicodeText( fontI->second, &r, format, wrap( color ), text );

    // end sprite render
    if( spriteBeginEnd ) _dxCR( _sprite->End() );

    // turn on z-test & z-write
    dxSetRenderState( D3DRS_ZENABLE, TRUE );
    dxSetRenderState( D3DRS_ZWRITEENABLE, TRUE );
}

gui::Rect Gui::calcUnicodeTextRect(const gui::Rect& origin, const char* fontName, gui::AlignmentType vAlign, gui::AlignmentType hAlign, bool wordWrap, const wchar_t* text)
{
    // obtain font
    FontI fontI = _fontM.find( fontName );
    assert( fontI != _fontM.end() );

    // wrap rect
    RECT rect = { origin.left, origin.top, origin.right, origin.bottom };

    // build format
    DWORD format = 0;
    switch( vAlign )
    {
    case gui::atTop:    format = format | DT_TOP; break;
    case gui::atCenter: format = format | DT_VCENTER; break;
    case gui::atBottom: format = format | DT_BOTTOM; break;
    }
    switch( hAlign )
    {
    case gui::atLeft:   format = format | DT_LEFT; break;
    case gui::atCenter: format = format | DT_CENTER; break;
    case gui::atRight:  format = format | DT_RIGHT; break;
    }
    if( wordWrap ) format = format | DT_WORDBREAK;
    format = format | DT_CALCRECT;

    int result = fontI->second->DrawTextW(
        _sprite,
        text,
        -1,
        &rect,
        format,
        ::black
    );
    if( !result )
    {
        return gui::Rect( 0, 0, 0, 0 );
    }
    else
    {
        return gui::Rect( rect.left, rect.top, rect.right, rect.bottom );
    }
}

void Gui::setMessageCallback(gui::MessageCallback callback, void* userData)
{
    _messageCallback         = callback;
    _messageCallbackUserData = userData;
}

/**
 * module locals
 */

ID3DXFont* Gui::getFont(const char* fontName)
{
    // obtain font
    FontI fontI = _fontM.find( fontName );
    assert( fontI != _fontM.end() );
    if( fontI == _fontM.end() ) return NULL;
    return fontI->second;
}

void Gui::renderRect(const RECT& rect, Texture* texture, const RECT& textureRect, Color color)
{
    // filter transparent rectangles
    if( wrap( color )[3] == 0 ) return;

    float scaleX = (float) ( rect.right - rect.left ) / ( textureRect.right - textureRect.left );
    float scaleY = (float) ( rect.bottom - rect.top ) / ( textureRect.bottom - textureRect.top );

    Matrix transform;
    D3DXMatrixScaling( &transform, scaleX, scaleY, 1.0f );
    _dxCR( _sprite->SetTransform( &transform ) );
    
    D3DXVECTOR3 pos( (float)rect.left, (float)rect.top, 0.0f );
    pos.x /= scaleX;
    pos.y /= scaleY;

    if( texture )
    {
        _dxCR( _sprite->Draw( texture->iDirect3DTexture(), &textureRect, NULL, &pos, color ) );
    }
    else
    {
        _dxCR( _sprite->Draw( NULL, &textureRect, NULL, &pos, color ) );
    }
}

void Gui::renderUnicodeText(ID3DXFont* font, RECT* screenRect, DWORD format, Color color, const wchar_t* str)
{
    _dxCR( _sprite->SetTransform( &identity ) );
	font->DrawTextW( _sprite, str, -1, screenRect, format, color );
}

void Gui::renderASCIIText(ID3DXFont* font, RECT* screenRect, DWORD format, Color color, const char* str)
{
    _dxCR( _sprite->SetTransform( &identity ) );
	font->DrawTextA( _sprite, str, -1, screenRect, format, color );
}

void Gui::pushMessage(gui::Message* message)
{
    _messageL.push_back( *message );
}

void Gui::notifyPanelDestroy(GuiPanel* panel)
{
    if( _panelUnderCursor == panel )
    {
        _panelUnderCursor = dynamic_cast<GuiPanel*>( panel->getParent() );
    }

    bool     foundAndRemoved;
    MessageI messageI;    

    do
    {
        foundAndRemoved = false;
        for( messageI = _messageL.begin(); messageI != _messageL.end(); messageI++ )
        {
            if( (*messageI).origin == panel )
            {
                foundAndRemoved = true;
                _messageL.erase( messageI );
                break;
            }
        }
    }
    while( foundAndRemoved );
}

/**
 * Lostable
 */

void Gui::onLostDevice(void)
{
    _sprite->OnLostDevice();

    for( FontI fontI = _fontM.begin(); fontI != _fontM.end(); fontI++ )
    {
        fontI->second->OnLostDevice();
    }
}

void Gui::onResetDevice(void)
{
    _sprite->OnResetDevice();

    for( FontI fontI = _fontM.begin(); fontI != _fontM.end(); fontI++ )
    {
        fontI->second->OnResetDevice();
    }
}