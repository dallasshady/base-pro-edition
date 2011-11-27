
#include "headers.h"
#include "activity.h"
#include "gameplay.h"

void Activity::updateCooperativeActivity(float dt)
{
    // cooperative work
    engine::DeviceState deviceState = Gameplay::iEngine->handleCooperativeLevel();
    switch( deviceState )
    {
    case engine::dsLost:
    case engine::dsNotReset:
        Sleep( 50 );
        return;
    case engine::dsDriverInternalError:
        getCore()->logMessage( "Driver internal error occured during cooperative work!" );
        PostQuitMessage( 0 );
        return;
    }
    if( !Gameplay::iEngine->canRestoreDevice() )
    {
        getCore()->logMessage( "Error during full screen cooperative work!" );
        Gameplay::iGameplay->setUnsafeCleanup( true );
        PostQuitMessage( 0 );
    }

    // abstract activity
    updateActivity( dt );
}