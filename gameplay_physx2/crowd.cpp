
#include "headers.h"
#include "crowd.h"

/**
 * actor abstracts
 */

void Crowd::onUpdateActivity(float dt)
{
}

/**
 * class implementation
 */

Crowd::Crowd(Actor* parent, CrowdDesc* desc) : Actor( parent )
{
    assert( parent );
    assert( desc );
    assert( desc->extras );

    _name = "Crowd";
    _desc = *desc;
    _numWalkingActors = 0;

    // actualize extras frame hierarchy
    // this operation should be forced because extras are not in world BSP and 
    // can not be actualized automactically
    _desc.extras->getFrame()->translate( Vector3f( 0,0,0 ) );
    _desc.extras->getFrame()->getLTM();

    // create enclosure for a crowd
    _enclosure = new Enclosure( _desc.extras, 0.0f );

    // generate crowd actors
    engine::IClump* cloneSource = _scene->findClump( "CrowdMale01" );
    assert( cloneSource );
    for( unsigned int i=0; i<_desc.numActors; i++ )
    {
        new Spectator( this, cloneSource, _enclosure );
    }
}

Crowd::~Crowd()
{
    if( _enclosure ) delete _enclosure;
}

/**
 * class behaviour
 */

bool Crowd::beginWalk(void)
{
    if( _numWalkingActors == _desc.numWalkingActors ) return false;
    _numWalkingActors++;
    return true;
}

void Crowd::endWalk(void)
{
    _numWalkingActors--;
    assert( _numWalkingActors >= 0 );
}