
#include "headers.h"
#include "demo.h"
#include "gameplay.h"
#include "scene.h"
#include "fatalitylist.h"

/**
 * class implementation
 */

Demo::Demo(Career* career)
{
    _endOfActivity = false;
    _career = career;
    _location = new Location( NULL, 1 );
    
    // if player is still alive
    if( _career->getVirtues()->evolution.health > 0.0f )
    {
        // heal him
        _career->getVirtues()->evolution.health = 1.0f;
    }
}

Demo::~Demo()
{
    delete _location;
}

/**
 * gui message handling
 */

void Demo::messageCallback(gui::Message* message, void* userData)
{    
}

/**
 * Activity
 */

void Demo::updateActivity(float dt)
{
    // check player health
    if( _career->getVirtues()->evolution.health == 0.0f )
    {
        _endOfActivity = true;
    }
    else if( !_endOfActivity )
    {
        Gameplay::iGameplay->pushActivity( new Scene( _career, _location, HOURS_TO_MINUTES( 12 ) ) );
        _endOfActivity = true;
    }
}

bool Demo::endOfActivity(void)
{
    return _endOfActivity;
}

void Demo::onBecomeActive(void)
{
    // register message callback
    Gameplay::iGui->setMessageCallback( messageCallback, this );
}

void Demo::onBecomeInactive(void)
{
    // unregister message callback
    Gameplay::iGui->setMessageCallback( NULL, NULL );
}

void Demo::onReturnFromActivity(Activity* activity)
{
    // return from scene mode
    Scene* scene = dynamic_cast<Scene*>( activity );
    if( scene ) 
    {
        // check player health
        if( _career->getVirtues()->evolution.health == 0.0f )
        {
            // add incident
            Gameplay::iGameplay->pushActivity( new FatalityList( _career, _location->getDatabaseId() ) );
        }
    }
}