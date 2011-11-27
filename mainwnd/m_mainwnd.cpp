
#include "headers.h"
#include "m_mainwnd.h"
#include "resource.h"


SINGLE_ENTITY_COMPONENT(MainWnd);


MainWnd* MainWnd::_mainwnd = NULL;

HINSTANCE g_DLLInstance = NULL;

extern "C" int __declspec(dllexport) WINAPI DllMain(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved) 
{
    if (dwReason==DLL_PROCESS_ATTACH) {
        // store this DLL instance
        g_DLLInstance = (HINSTANCE)hinstDLL;
    }
    return TRUE;
}


/**
 * Private stuff
 */

void MainWnd::saveWindowCenter(void)
{
    RECT r;
    GetClientRect(_windowHandle, &r);        
    _windowCenterClient.x = r.right / 2;
    _windowCenterClient.y = r.bottom / 2;
    _windowCenterScreen = _windowCenterClient;
    ClientToScreen(_windowHandle, &_windowCenterScreen);
}

void MainWnd::centerCursor(void)
{
    SetCursorPos(_windowCenterScreen.x, _windowCenterScreen.y);        
}

/**
 * Interface methods
 */

MainWnd::~MainWnd() 
{
    // Show cursor
    ShowCursor(true);
}

void MainWnd::entityInit(Object * p) 
{ 
    // Get the application instance
    IParamPack* ppack = icore->getCoreParamPack();
    ppack->get(
        "startup.arguments.instance",
        _instance
    );    
    
    // open configuration file
    TiXmlDocument* generalConfig = new TiXmlDocument( "./cfg/config.xml" );
    generalConfig->LoadFile();
    // locate video element
    TiXmlNode* child = generalConfig->FirstChild(); assert( child );
    if( child != NULL ) do 
    {
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "video" ) == 0 ) break;
        child = child->NextSibling();
    }
    while( child != NULL );
    TiXmlElement* video = static_cast<TiXmlElement*>( child );
    if( !video ) throw Exception( "Invalid configuration file!" );    

    // determine windowed mode
    int windowed;
    video->Attribute( "windowed", &windowed );   
    _windowed = ( windowed != 0 );
    
    // determine Width & Height of the window
    int width, height;
    video->Attribute( "width", &width );
    video->Attribute( "height", &height );
    POINT ptWindow = {0,0};
    SIZE szWindow = { width, height };

    // Specify the "window class" structure
    _windowClass.cbSize          = sizeof(WNDCLASSEX);
    _windowClass.style           = CS_HREDRAW | CS_VREDRAW;
    _windowClass.lpfnWndProc     = (WNDPROC)MainWndProc;
    _windowClass.cbClsExtra      = 0;
    _windowClass.cbWndExtra      = 0;
    _windowClass.hbrBackground   = (HBRUSH)(COLOR_BACKGROUND+1);     
    _windowClass.lpszClassName   = "metathrone.main.window.class";
    _windowClass.hInstance       = g_DLLInstance;//(HINSTANCE)_instance;
    _windowClass.hIcon           = LoadIcon(g_DLLInstance, MAKEINTRESOURCE(IDI_APPICON));
    _windowClass.hCursor         = LoadCursor(NULL, IDC_ARROW);
    _windowClass.lpszMenuName    = NULL;
    _windowClass.hIconSm         = _windowClass.hIcon;
    // Register window class
    RegisterClassEx(&_windowClass);
    // Create window
    _windowHandle = CreateWindow(
        "metathrone.main.window.class",
        "D3 Engine",
        WS_OVERLAPPEDWINDOW,
        ptWindow.x, ptWindow.y, szWindow.cx, szWindow.cy,
        NULL, NULL, 
        g_DLLInstance,
        NULL
    );
    // Store window handle in startup arguments
    ppack->set(
        "mainwnd.handle",
        reinterpret_cast<int>(_windowHandle)
    );
    // let main window be visible to static WndProc
    _mainwnd = this;        
    // change window flags for fullscreen
    if( !windowed )
    {
        SetWindowLong( _windowHandle, GWL_STYLE, WS_POPUP );
        SetWindowPos(
            _windowHandle, 0, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
        );
    }
    // show & refresh window (do this after _mainwnd becomes valid!)
    ShowWindow(_windowHandle, SW_SHOW);
    UpdateWindow(_windowHandle);
    // Hide cursor
    ShowCursor(false);
    // Set viewport width and height.
    RECT rect;
    GetClientRect(_windowHandle,&rect);
    if( !windowed ) 
    {
        rect.left = 0, 
        rect.right = szWindow.cx, 
        rect.top = 0, 
        rect.bottom = szWindow.cy;
    }
    // $SPECIFIC(CROSSMODULE:engine,mainwnd): create viewport parameters
    ppack->set("engine.viewport.width", rect.right);
    ppack->set("engine.viewport.height", rect.bottom);
    // Store window center to automatically positioning mouse cursor on it
    // after mouse moving
    saveWindowCenter();

    // release config
    delete generalConfig;
}

void MainWnd::entityHandleEvent(evtid_t id, trigid_t trigId, Object * param)
{
    // Messages
    if(trigId==-1)
    {
        switch(id)
        {
        // Store window center after engine set fullscreen videomode
        case mainwnd::evScreenSize::eventId:
            {
                mainwnd::evScreenSize* evObj = static_cast<mainwnd::evScreenSize*>(param);
                _windowCenterClient.x = evObj->width / 2;
                _windowCenterClient.y = evObj->height / 2;
                _windowCenterScreen = _windowCenterClient;
                //SetWindowPos(_windowHandle, 0, 0, 0, evObj->width-1, evObj->height-1,0);
                RECT rcClient;
                GetWindowRect(_windowHandle,&rcClient);
                icore->logMessage(" %d %d %d %d ",rcClient.left,rcClient.top,rcClient.right,rcClient.bottom);
                return;
            }
        }
    }
    // Triggers
    else
    {
    }
}    

void MainWnd::entityAct(float dt) 
{        
    return;
}

IBase * MainWnd::entityAskInterface(iid_t id) 
{
    if (id==mainwnd::IMainWnd::iid) return (mainwnd::IMainWnd*) this;
    return 0; 
}

EntityBase* MainWnd::creator() 
{ 
    return new MainWnd; 
}

void MainWnd::entityDestroy() 
{ 
    delete this; 
}


// Main window callback
LRESULT CALLBACK MainWnd::MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   
    if(_mainwnd && (hWnd==_mainwnd->_windowHandle) )
    {
        // build MSG structure
        MSG msg;
        msg.hwnd = hWnd, msg.lParam = lParam, 
        msg.wParam = wParam, msg.message = message;
        // translate keyboard messages, ignore repeat events
        if( ( msg.message == WM_KEYDOWN && !(msg.lParam & 0x40000000) )
            || msg.message == WM_KEYUP )
        {
            _mainwnd->icore->activate(
                mainwnd::TriggerMainwnd::tid,
                &Holder<MSG>(msg)
            );
        }
        // translate application activation message
        else if( msg.message == WM_ACTIVATEAPP )
        {
            if (wParam)
            {
                _mainwnd->centerCursor();
            }
        }
        else if( msg.message == WM_ACTIVATE )
        {
            _mainwnd->icore->activate(
                mainwnd::TriggerMainwnd::tid,
                &Holder<MSG>(msg)
            );
        }
        // translate show window message
        else if( msg.message == WM_SHOWWINDOW )
        {
            if (wParam)
            {
                assert(_mainwnd!=NULL);
                _mainwnd->saveWindowCenter();
                _mainwnd->centerCursor();
            }
        }
        // translate character code to users
        else if( msg.message == WM_CHAR )
        {
            _mainwnd->icore->activate(
                mainwnd::TriggerMainwnd::tid,
                &Holder<MSG>(msg)
            );
        }
        // translate mousemove messages
        else if( msg.message == WM_MOUSEMOVE )
        {
            if (GetActiveWindow()==hWnd)
            {
                // If cursor position does not correspond to window center
                if( GET_X_LPARAM(msg.lParam) != _mainwnd->_windowCenterClient.x ||
                    GET_Y_LPARAM(msg.lParam) != _mainwnd->_windowCenterClient.y )
                {
                    msg.lParam = MAKELPARAM(
                        _mainwnd->_windowCenterClient.x - GET_X_LPARAM(msg.lParam),
                        _mainwnd->_windowCenterClient.y - GET_Y_LPARAM(msg.lParam)
                    );
                    _mainwnd->icore->activate(
                        mainwnd::TriggerMainwnd::tid,
                        &Holder<MSG>(msg)
                    );
                    _mainwnd->centerCursor();
                }
            }
        }
        // translate other mouse messages
        else if( msg.message == WM_MOUSEMOVE || msg.message == WM_LBUTTONDOWN ||
                 msg.message == WM_LBUTTONUP || msg.message == WM_RBUTTONDOWN ||
                 msg.message == WM_RBUTTONUP || msg.message == WM_MBUTTONDOWN ||
                 msg.message == WM_MBUTTONUP || msg.message == WM_MOUSEWHEEL 
               )
        {
            if (GetActiveWindow()==hWnd)
            {
                msg.lParam = MAKELPARAM(0,0);
                _mainwnd->icore->activate(
                    mainwnd::TriggerMainwnd::tid,
                    &Holder<MSG>(msg)
                );
            }
        }
        // translate window size-move messages
        else if( msg.message == WM_SIZE || msg.message == WM_MOVE ||
                 msg.message == WM_SYSCOMMAND 
               )
        {
            _mainwnd->saveWindowCenter();
            _mainwnd->icore->activate(
                mainwnd::TriggerMainwnd::tid,
                &Holder<MSG>(msg)
            );                
        }
        // translate killfocus & setfocus
        else if( msg.message == WM_KILLFOCUS || msg.message == WM_SETFOCUS )
        {
            _mainwnd->icore->activate(
                mainwnd::TriggerMainwnd::tid,
                &Holder<MSG>(msg)
            );
        }
    }
    switch (message) 
    {
        case WM_CLOSE:
        case WM_DESTROY:
            // quit
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
            break;
    }
    return 0;
}
