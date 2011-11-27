#if !defined(M_MAIWND_INCLUDED)
#define M_MAIWND_INCLUDED

#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/product_version.h"
#include "../shared/mainwnd.h"

using namespace ccor;

/**
 * Mainwnd implementation
 */
class MainWnd : public EntityBase,
                virtual public mainwnd::IMainWnd
{
private:
    int        _instance;      // application instance 
    WNDCLASSEX _windowClass;   // main window class
    HWND       _windowHandle;  // main window handle
    POINT      _windowCenterClient;
    POINT      _windowCenterScreen;
    bool       _windowed;
private:
    static MainWnd* _mainwnd; // Main window instance
private:
    void saveWindowCenter(void);
    void centerCursor(void);
public:
    ~MainWnd();
    virtual void __stdcall entityInit(Object * p);    
    virtual void __stdcall entityHandleEvent(evtid_t id, trigid_t trigId, Object * param);
    virtual void __stdcall entityAct(float dt);
    virtual IBase* __stdcall entityAskInterface(iid_t id);
    // IMainWnd
    virtual int __stdcall getHandle(void) {
        return (int)(_windowHandle);
    }
    virtual void __stdcall setWindowText(const char* text) {
        SetWindowText( _windowHandle, text );
    }
    // component interface
    virtual void __stdcall entityDestroy();
    static EntityBase* creator();
private:
    static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif
