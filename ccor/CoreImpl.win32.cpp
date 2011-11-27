#include "headers.h"
#include <windows.h>
#include "CoreImpl.h"
namespace ccor {

void CoreImpl::processSystemMessages() {

    MSG message;

    while( PeekMessage(&message, NULL, 0U, 0U, PM_REMOVE|PM_NOYIELD) )
    {
        if( message.message == WM_QUIT )
        {
            throw CoreTerminateException(0);
        }
        else
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }
#if 0
    else if( ForegroundApp )
    {
        if ( FALSE != RwInitialized )
        {
            RsEventHandler( rsIDLE, NULL );
        }
    }
    else
    {
        WaitMessage();
    }
#endif

}

}
