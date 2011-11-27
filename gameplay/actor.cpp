
#include "headers.h"
#include "scene.h"

/**
 * class implementation
 */

Actor::Actor(Scene* scene)
{
    _name  = "Actor";
    _scene = scene;
    _parent = NULL;
}

Actor::Actor(Actor* parent)
{
    _name = "Actor";
    _parent = parent;
    _parent->_children.push_back( this );
    _scene  = parent->getScene();
}

Actor::~Actor()
{
    // remove children
    while( _children.size() ) delete *_children.begin();

    // unregister if it have a parent actor
    if( _parent ) 
    {
        for( ActorI actorI = _parent->_children.begin(); 
                    actorI != _parent->_children.end(); 
                    actorI++ )
        {
            if( *actorI == this )
            {
                _parent->_children.erase( actorI );
                break;
            }
        }
    }
}

/**
 * class behaviour
 */

void Actor::happen(Actor* initiator, unsigned int eventId, void* eventData)
{
    onEvent( initiator, eventId, eventData );
    for( ActorI actorI = _children.begin(); actorI != _children.end(); actorI++ ) 
    {
        (*actorI)->happen( initiator, eventId, eventData );
    }
}

void Actor::updateActivity(float dt)
{
    onUpdateActivity( dt );
    for( ActorI actorI = _children.begin(); actorI != _children.end(); actorI++ ) 
    {
        (*actorI)->updateActivity( dt );
    }
}

void Actor::updatePhysics(void)
{
    onUpdatePhysics();
    for( ActorI actorI = _children.begin(); actorI != _children.end(); actorI++ ) 
    {
        (*actorI)->updatePhysics();
    }
}