
#ifndef GUI_IMPLEMENTATION_INCLUDED
#define GUI_IMPLEMENTATION_INCLUDED

#include "headers.h"
#include "../shared/gui.h"
#include "../shared/language.h"
#include "tinyxml.h"
#include "fundamentals.h"
#include "texture.h"
#include "../common/istring.h"
#include "engine.h"

/**
 * compiler constants
 */

#define GUI_DEFAULT_INDENT 2
#define GUI_CARET_BLINK    0.25f

/**
 * panel implementation
 */

class GuiPanel : public gui::IGuiPanel
{
protected:
    typedef std::list<GuiPanel*> GuiPanelL;
    typedef GuiPanelL::iterator GuiPanelI;
protected:
    std::string                 _name;
    bool                        _visible;
    RECT                        _rect;
    RECT                        _textureRect;
    Texture*                    _texture;
    Color                       _color;
    GuiPanel*                   _parent;
    GuiPanelL                   _children;
    std::wstring                _hint;
    gui::GuiPanelRenderCallback _renderCallback;
    void*                       _renderCallbackData;
protected:
    virtual void onRender(void) {}
    virtual void onMessage(gui::Message* message) {}
public:
    // class implementation
    GuiPanel();
    GuiPanel(const char* panelName);
    virtual ~GuiPanel();
    // IGuiPanel
    virtual void __stdcall release(void);
    virtual const char* __stdcall getName(void);
    virtual void __stdcall setName(const char* name);
    virtual const wchar_t* __stdcall getHint(void);
    virtual void __stdcall setHint(const wchar_t* hint);
    virtual gui::IGuiPanel* __stdcall getParent(void);
    virtual void __stdcall insertPanel(gui::IGuiPanel* panel);
    virtual void __stdcall removePanel(gui::IGuiPanel* panel);
    virtual gui::IGuiPanel* __stdcall find(const char* name);
    virtual bool __stdcall getVisible(void);
    virtual void __stdcall setVisible(bool visible);
    virtual gui::Rect __stdcall getRect(void);
    virtual gui::Rect __stdcall getScreenRect(void);
    virtual void __stdcall setRect(const gui::Rect& rect);
    virtual engine::ITexture* __stdcall getTexture(void);
    virtual void __stdcall setTexture(engine::ITexture* texture);
    virtual gui::Rect __stdcall getTextureRect(void);
    virtual void __stdcall setTextureRect(const gui::Rect& rect);
    virtual Vector4f __stdcall getColor(void);
    virtual void __stdcall setColor(const Vector4f& color);
    virtual void __stdcall setRenderCallback(gui::GuiPanelRenderCallback renderCallback, void* data);
public:
    // typecasting
    virtual gui::IGuiWindow* __stdcall getWindow(void);
    virtual gui::IGuiStaticText* __stdcall getStaticText(void);
    virtual gui::IGuiButton* __stdcall getButton(void);
    virtual gui::IGuiEdit* __stdcall getEdit(void);
    virtual gui::IGuiSlider* __stdcall getSlider(void);
public:
    // internal inlines
    inline RECT rect(void) { return _rect; }
    inline RECT textureRect(void) { return _textureRect; }
    inline const wchar_t* getHintString(void) { if( _hint.length() ) return _hint.c_str(); else return NULL; }
public:
    // module internals
    void render(void);
    RECT clientToScreen(const RECT& rect);
    RECT screenToClient(const RECT& rect);
    RECT childToScreen(const RECT& rect);
    void initializePanel(TiXmlNode* node);
    GuiPanel* getPanelUnderScreenPosition(int x, int y);
    void dispatchMessage(gui::Message* message);
    void setTopPanel(GuiPanel* panel);
};

/**
 * static text implementation
 */

class GuiStaticText : public GuiPanel,
                      virtual public gui::IGuiStaticText
{
protected:
    std::wstring       _text;
    std::string        _font;
    Color              _fontColor;
    gui::AlignmentType _verticalAlignment;
    gui::AlignmentType _horizontalAlignment;
protected:
    virtual void onRender(void);
public:
    // class implementatiocn
    GuiStaticText(TiXmlNode* node);
    virtual ~GuiStaticText(void);
    // IGuiStaticText
    virtual gui::IGuiPanel* __stdcall getPanel(void);
    virtual const wchar_t* __stdcall getText(void);
    virtual void __stdcall setText(const wchar_t* value);
    virtual Vector4f __stdcall getTextColor(void);
    virtual void __stdcall setTextColor(Vector4f value);
};

/**
 * window implementation
 */

class GuiWindow : public GuiPanel,
                  virtual public gui::IGuiWindow
{
public:
    // class implementation
    GuiWindow(TiXmlNode* node);
    virtual ~GuiWindow();
    // IGuiWindow
    virtual gui::IGuiPanel* __stdcall getPanel(void);
    virtual void __stdcall align(gui::AlignmentType verticalAlignment, int verticalIndent, gui::AlignmentType horizontalAlignment, int horizontalIndent);
    virtual void __stdcall setTopWindow(void);
};

/**
 * button implementation
 */

class GuiButton : public GuiPanel,
                  virtual public gui::IGuiButton
{
protected:
    std::wstring _caption;
    std::string  _font;
    Color        _fontColor;
    Color        _activeColor;
    Color        _inactiveColor;
protected:
    virtual void onRender(void);
    virtual void onMessage(gui::Message* message);
public:
    // class implementatiocn
    GuiButton(const char* name, const char* fontName);
    GuiButton(TiXmlNode* node);
    virtual ~GuiButton(void);
    // IGuiButton
    virtual gui::IGuiPanel* __stdcall getPanel(void);
    virtual Vector4f __stdcall getActiveColor(void);
    virtual void __stdcall setActiveColor(Vector4f value);
    virtual Vector4f __stdcall getInactiveColor(void);
    virtual void __stdcall setInactiveColor(Vector4f value);
    virtual const wchar_t* __stdcall getCaption(void);
    virtual void __stdcall setCaption(const wchar_t* value);
    virtual Vector4f __stdcall getTextColor(void);
    virtual void __stdcall setTextColor(Vector4f value);
};

/**
 * slider implementation
 */

class GuiSlider : public GuiPanel,
                  virtual public gui::IGuiSlider
{
private:
    bool       _horizontal; // slider type: horizontal (true) or vertical (false)
    bool       _reverse;    // slider is reversed
    float      _lowerLimit; // slider state
    float      _upperLimit; // slider state
    float      _pos;        // slider state 
    RECT       _lowerRect;  // lower limit rect (automatic, affects scroll button)
    RECT       _upperRect;  // upper limit rect (automatic, affects scroll button)
    bool       _scrolling;  // true if scroll button is pressed and manual scrolling is performed
    GuiButton* _scrollButton;
private:
    void updateAppearance(void);
protected:
    // gui abstractions
    virtual void onRender(void);
    virtual void onMessage(gui::Message* message);    
public:
    // class implementation
    GuiSlider(TiXmlNode* node);
    virtual ~GuiSlider();
    // IGuiSlider
    virtual gui::IGuiPanel* __stdcall getPanel(void);
    virtual float __stdcall getLowerLimit(void);
    virtual void __stdcall setLowerLimit(float value);
    virtual float __stdcall getUpperLimit(void);
    virtual void __stdcall setUpperLimit(float value);
    virtual float __stdcall getPosition(void);
    virtual void __stdcall setPosition(float value);
};

/**
 * editbox implementation
 */

class GuiEdit : public GuiPanel,
                virtual public gui::IGuiEdit
{
private:
    std::string      _text;
    std::string      _font;
    Color            _fontColor;
    Texture*         _caretTexture;
    int              _caretPos;
    int              _maxLength;
    int              _isNumerical;
    void*            _strAnalysis;
    std::vector<int> _glyphWidths;
private:
    void analyseString();
protected:
    // gui abstractions
    virtual void onRender(void);
    virtual void onMessage(gui::Message* message);    
public:
    // class implementation
    GuiEdit(TiXmlNode* node);
    virtual ~GuiEdit();
    // IGuiEdit
    virtual void __stdcall release(void);
    virtual gui::IGuiPanel* __stdcall getPanel(void);
    virtual const char* __stdcall getText(void);
    virtual void __stdcall setText(const char* text);
};

/**
 * gui manager implementation
 */

class Gui : public EntityBase,
            virtual public Lostable,
	        virtual public gui::IGui
{
private:
    // font collection container
    typedef std::pair<std::string,ID3DXFont*> FontT;
    typedef std::map<std::string,ID3DXFont*> FontM;
    typedef FontM::iterator FontI;
    // message queue
    typedef std::list<gui::Message> MessageL;
    typedef MessageL::iterator MessageI;
private:
    TiXmlDocument* _guiDocument;
    ID3DXSprite*   _sprite;
    GuiPanel*      _desktop;
    GuiPanel*      _panelUnderCursor;
    GuiPanel*      _keyboardFocus;
    bool           _blinkIsVisible;
    float          _blinkTimeout;
    FontM          _fontM;
    MessageL       _messageL;
private:
    bool                 _cursorIsVisible;
    Texture*             _cursorTexture;
    Color                _cursorColor;
    RECT                 _cursorRect;
    RECT                 _cursorTextureRect;
    gui::MessageCallback _messageCallback;
    void*                _messageCallbackUserData;
    int                  _mouseDX;
    int                  _mouseDY;
    bool                 _hintModeEnabled;
    float                _hintTimeout;
    float                _hintTimeAccumulator;
public:
    // class implementation
    Gui();
    virtual ~Gui();
    // component support
    static EntityBase* creator();
    virtual void __stdcall entityDestroy();
    // EntityBase
    virtual void __stdcall entityInit(Object * p);
    virtual void __stdcall entityAct(float dt);
    virtual void __stdcall entityHandleEvent(evtid_t id, trigid_t trigId, Object* param);
    virtual IBase* __stdcall entityAskInterface(iid_t id);
    // Lostable
    virtual void onLostDevice(void);
    virtual void onResetDevice(void);
    // IGui
    virtual void __stdcall setCursorColor(const Vector4f& value);
    virtual void __stdcall setCursorVisible(bool flag);
    virtual void __stdcall setHintMode(bool showHints, float timeout);
    virtual gui::IGuiPanel* __stdcall getDesktop(void);
    virtual gui::IGuiPanel* __stdcall getPanelUnderCursor(void);
    virtual gui::Rect __stdcall getCursorRect(void);
    virtual gui::IGuiPanel* __stdcall createPanel(const char* panelName);
    virtual gui::IGuiWindow* __stdcall createWindow(const char* windowName);
    virtual gui::IGuiButton* __stdcall createButton(const char* name, const char* fontName);
    virtual void __stdcall render(void);
    virtual void __stdcall renderRect(const gui::Rect& rect, engine::ITexture* texture, const gui::Rect& textureRect, const Vector4f& color, bool spriteBeginEnd=true);
    virtual void __stdcall renderUnicodeText(const gui::Rect& rect, const char* fontName, const Vector4f& color, gui::AlignmentType vAlign, gui::AlignmentType hAlign, bool wordWrap, const wchar_t* text, bool spriteBeginEnd=true);
    virtual gui::Rect __stdcall calcUnicodeTextRect(const gui::Rect& origin, const char* fontName, gui::AlignmentType vAlign, gui::AlignmentType hAlign, bool wordWrap, const wchar_t* text);
    virtual void __stdcall setMessageCallback(gui::MessageCallback callback, void* userData);
public:
    // inlines
    inline int getMouseDX(void) { return _mouseDX; }
    inline int getMouseDY(void) { return _mouseDY; }
    inline GuiPanel* getKeyboardFocus(void) { return _keyboardFocus; }
    inline bool blinkIsVisible(void) { return _blinkIsVisible; }
public:
    // module locals
    ID3DXFont* getFont(const char* fontName);
    void renderRect(const RECT& rect, Texture* texture, const RECT& textureRect, Color color);
    void renderUnicodeText(ID3DXFont* font, RECT* screenRect, DWORD format, Color color, const wchar_t* str);
    void renderASCIIText(ID3DXFont* font, RECT* screenRect, DWORD format, Color color, const char* str);
    void pushMessage(gui::Message* message);
    void notifyPanelDestroy(GuiPanel* panel);
public:
    static Gui* instance;
    static language::ILanguage* iLanguage;
};

/**
 * xml helpers
 */

Color xmlColor(TiXmlNode* node, const char* attributeName);
RECT xmlRect(TiXmlNode* node, const char* attributeName);
Texture* xmlTexture(TiXmlNode* node, const char* attributeName);
gui::AlignmentType xmlVerticalAlignment(TiXmlNode* node, const char* attributeName);
gui::AlignmentType xmlHorizontalAlignment(TiXmlNode* node, const char* attributeName);

#endif