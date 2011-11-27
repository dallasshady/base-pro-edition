/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description Engine interfaces
 *
 * @author bad3p
 */

#ifndef GUI_INTERFACES_INCLUDED
#define GUI_INTERFACES_INCLUDED

#include "engine.h"
#include "mainwnd.h"

namespace gui {

/**
 * forward declaration
 */

class IGuiPanel;
class IGuiWindow;
class IGuiStaticText;
class IGuiButton;
class IGuiSlider;
class IGuiEdit;

/**
 * alignment types
 */

enum AlignmentType
{
    atLeft,
    atRight,
    atTop,
    atBottom,
    atCenter
};

/**
 * rectangular area
 */

struct Rect
{
public:
    int left;
    int top;
    int right;
    int bottom;
public:
    Rect() : left(0), top(0), right(0), bottom(0) {}
    Rect(int l, int t, int r, int b) : left(l), top(t), right(r), bottom(b) {}
public:
    inline unsigned int getWidth(void) { return right-left; }
    inline unsigned int getHeight(void) { return bottom-top; }
};

/**
 * messaging
 */

enum MouseButton
{
    mbUnknown,
    mbLeft,
    mbRight,
    mbMiddle
};

enum Event
{
    onNothing,
    onEnterCursor, // origin, event
    onLeaveCursor, // origin, event
    onMouseMove,   // origin, event, mouseX, mouseY
    onMouseDown,   // origin, event, mouseButtonId
    onMouseUp,     // origin, event, mouseButtonId
    onMouseWheel,  // origin, event, ( mouseX = mouseY = wheel offset)
    onButtonClick, // origin, event
    onSlide,       // origin, event
    onKeyChar,     // origin, event, keyChar
    onKeyDown,     // origin, event, keyCode
    onKeyUp        // origin, event, keyCode
};

struct Message
{   
public:
    IGuiPanel*  origin;      // event origin
    Event       event;       // event type
    MouseButton mouseButton; // mouse button concerned with event
    int         mouseX;      // mouse cursor X-coord 
    int         mouseY;      // mouse cursor Y-coord 
    char        keyChar;     // key character
    int         keyCode;     // key code
public:
    Message() : 
      origin(NULL), event(onNothing), mouseButton(mbUnknown), mouseX(0), mouseY(0), keyChar(0), keyCode(0) {}
    Message(IGuiPanel* o, Event e) : 
      origin(o), event(e), mouseButton(mbUnknown), mouseX(0), mouseY(0), keyChar(0), keyCode(0) {}
    Message(IGuiPanel* o, Event e, char c) : 
      origin(o), event(e), mouseButton(mbUnknown), mouseX(0), mouseY(0), keyChar(c), keyCode(0) {} 
    Message(IGuiPanel* o, Event e, int c) : 
      origin(o), event(e), mouseButton(mbUnknown), mouseX(0), mouseY(0), keyChar(0), keyCode(c) {} 
    Message(IGuiPanel* o, Event e, MouseButton mb) :
      origin(o), event(e), mouseButton(mb), mouseX(0), mouseY(0), keyChar(0), keyCode(0) {}
    Message(IGuiPanel* o, Event e, int mx, int my) :
      origin(o), event(e), mouseButton(mbUnknown), mouseX(mx), mouseY(my), keyChar(0), keyCode(0) {}
};

typedef void (*MessageCallback)(Message* message, void* userData);

/**
 * panel
 */

typedef void (*GuiPanelRenderCallback)(IGuiPanel* panel, void* data);

class IGuiPanel
{
public:
    DECLARE_INTERFACE_ID(0x7000b);
public:
	/**
	 * management & hierarchy
     */
    virtual void __stdcall release(void) = 0;
    virtual const char* __stdcall getName(void) = 0;
    virtual void __stdcall setName(const char* name) = 0;
    virtual const wchar_t* __stdcall getHint(void) = 0;
    virtual void __stdcall setHint(const wchar_t* hint) = 0;
    virtual IGuiPanel* __stdcall getParent(void) = 0;        
    virtual void __stdcall insertPanel(IGuiPanel* panel) = 0;
    virtual void __stdcall removePanel(IGuiPanel* panel) = 0;
    virtual IGuiPanel* __stdcall find(const char* name) = 0;
    /**
     * properties
     */
    virtual bool __stdcall getVisible(void) = 0;
    virtual void __stdcall setVisible(bool visible) = 0;
    virtual Rect __stdcall getRect(void) = 0;
    virtual Rect __stdcall getScreenRect(void) = 0;
    virtual void __stdcall setRect(const Rect& rect) = 0;
    virtual engine::ITexture* __stdcall getTexture(void) = 0;
    virtual void __stdcall setTexture(engine::ITexture* texture) = 0;
    virtual Rect __stdcall getTextureRect(void) = 0;
    virtual void __stdcall setTextureRect(const Rect& rect) = 0;
    virtual Vector4f __stdcall getColor(void) = 0;
    virtual void __stdcall setColor(const Vector4f& color) = 0;    
    /**
     * rendering & customization
     */
    virtual void __stdcall setRenderCallback(GuiPanelRenderCallback renderCallback, void* data) = 0;
    /**
     * typecasting
     */
    virtual IGuiWindow* __stdcall getWindow(void) = 0;
    virtual IGuiStaticText* __stdcall getStaticText(void) = 0;
    virtual IGuiButton* __stdcall getButton(void) = 0;
    virtual IGuiEdit* __stdcall getEdit(void) = 0;
    virtual IGuiSlider* __stdcall getSlider(void) = 0;
};

/**
 * static text
 */

class IGuiStaticText
{
public:
    DECLARE_INTERFACE_ID(0x70005);
public:
    /**
     * inherited
     */	
    virtual IGuiPanel* __stdcall getPanel(void) = 0;
    /**
     * properties
     */	
    virtual const wchar_t* __stdcall getText(void) = 0;
    virtual void __stdcall setText(const wchar_t* value) = 0;
    virtual Vector4f __stdcall getTextColor(void) = 0;
    virtual void __stdcall setTextColor(Vector4f value) = 0;
};

/**
 * window
 */

class IGuiWindow
{
public:
    DECLARE_INTERFACE_ID(0x70003);
public:
    /**
     * inherited
     */	
    virtual IGuiPanel* __stdcall getPanel(void) = 0;
    /**
     * behaviour
     */
    virtual void __stdcall align(AlignmentType verticalAlignment, int verticalIndent, AlignmentType horizontalAlignment, int horizontalIndent) = 0;
    virtual void __stdcall setTopWindow(void) = 0;
};

/**
 * button is a active panel with caption
 */

class IGuiButton
{
public:
    DECLARE_INTERFACE_ID(0x70002);
public: 
    /**
     * inherited
     */
    virtual IGuiPanel* __stdcall getPanel(void) = 0;
    /**
     * properties
     */
    virtual Vector4f __stdcall getActiveColor(void) = 0;
    virtual void __stdcall setActiveColor(Vector4f value) = 0;
    virtual Vector4f __stdcall getInactiveColor(void) = 0;
    virtual void __stdcall setInactiveColor(Vector4f value) = 0;
    virtual const wchar_t* __stdcall getCaption(void) = 0;
    virtual void __stdcall setCaption(const wchar_t* value) = 0;
    virtual Vector4f __stdcall getTextColor(void) = 0;
    virtual void __stdcall setTextColor(Vector4f value) = 0;
};

/**
 * slider (compound control)
 */

class IGuiSlider
{
public:
    DECLARE_INTERFACE_ID(0x7000c);
public:
    /**
     * inherited
     */
    virtual IGuiPanel* __stdcall getPanel(void) = 0;
    /**
     * properties
     */
    virtual float __stdcall getLowerLimit(void) = 0;
    virtual void __stdcall setLowerLimit(float value) = 0;
    virtual float __stdcall getUpperLimit(void) = 0;
    virtual void __stdcall setUpperLimit(float value) = 0;
    virtual float __stdcall getPosition(void) = 0;
    virtual void __stdcall setPosition(float value) = 0;
};

/**
 * editbox
 */

class IGuiEdit
{
public:
    DECLARE_INTERFACE_ID(0x70007);
public:
    /**
     * inherited
     */
    virtual IGuiPanel* __stdcall getPanel(void) = 0;
    /**
     * properties
     */
    virtual const char* __stdcall getText(void) = 0;
    virtual void __stdcall setText(const char* text) = 0;
};

/**
 * GUI manager
 */

class IGui : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x70004);
public:
    /**
     * gui cursor
     */
    virtual void __stdcall setCursorColor(const Vector4f& value) = 0;
    virtual void __stdcall setCursorVisible(bool flag) = 0;    
    virtual void __stdcall setHintMode(bool showHints, float timeout) = 0;
    /**
     * global gui resources
     */
    virtual IGuiPanel* __stdcall getDesktop(void) = 0;
    virtual IGuiPanel* __stdcall getPanelUnderCursor(void) = 0;
    virtual Rect __stdcall getCursorRect(void) = 0;
    /**
     * gui elements
     */
    virtual IGuiPanel* __stdcall createPanel(const char* panelName) = 0;
    virtual IGuiWindow* __stdcall createWindow(const char* windowName) = 0;
    virtual IGuiButton* __stdcall createButton(const char* name, const char* fontName) = 0;
    /**
     * rendering
     */
    virtual void __stdcall render(void) = 0;    
    virtual void __stdcall renderRect(const Rect& rect, engine::ITexture* texture, const Rect& textureRect, const Vector4f& color, bool spriteBeginEnd=true) = 0;
    virtual void __stdcall renderUnicodeText(const Rect& rect, const char* fontName, const Vector4f& color, AlignmentType vAlign, AlignmentType hAlign, bool wordWrap, const wchar_t* text, bool spriteBeginEnd=true) = 0;
    virtual Rect __stdcall calcUnicodeTextRect(const Rect& origin, const char* fontName, AlignmentType vAlign, AlignmentType hAlign, bool wordWrap, const wchar_t* text) = 0;    
    /**
     * events
     */
    virtual void __stdcall setMessageCallback(MessageCallback callback, void* userData) = 0;
};

}

#endif
