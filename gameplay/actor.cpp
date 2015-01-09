
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
	_airfoils = NULL;
	_airfoilsC = 0;
	network_id = -1;
}

Actor::Actor(Actor* parent)
{
    _name = "Actor";
    _parent = parent;
    _parent->_children.push_back( this );
    _scene  = parent->getScene();
	_airfoils = NULL;
	_airfoilsC = 0;
	network_id = -1;
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

	clearAirfoils();
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

void Actor::clearAirfoils() {
	if (_airfoils != NULL) {
		for (int i = 0; i < _airfoilsC; ++i) {
			delete _airfoils[i];
		}
		delete[] _airfoils;
	}
	_airfoilsC = 0;
}
void Actor::addAirfoils(int c, Airfoil **airfoils) {
	Airfoil **new_airfoils;
	new_airfoils = new Airfoil*[_airfoilsC + c];
	// add old
	for (int i = 0; i < _airfoilsC; ++i) {
		new_airfoils[i] = _airfoils[i];
	}
	// add new
	for (int i = 0; i < c; ++i) {
		//new_airfoils[i+_airfoilsC] = new Airfoil();
		new_airfoils[i+_airfoilsC] = airfoils[i];
	}
	_airfoils = new_airfoils;
	_airfoilsC += c;
}