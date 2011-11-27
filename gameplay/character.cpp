
#include "headers.h"
#include "character.h"

/**
 * character action
 */

void Character::Action::updateAnimation(float dt)
{
    // blend phase?
    if( _actionTime < _blendTime )
    {
        if( _actionTime + dt < _blendTime )
        {
            _actionTime += dt;
            _clump->getAnimationController()->blend( _actionTime/_blendTime );
        }
        else
        {
            // pass blend, move to animation
            dt -= ( _blendTime - _actionTime );
            _actionTime = _blendTime;
            // advance animation
            _actionTime += dt;
            _clump->getAnimationController()->advance( dt );
        }
    }
    else
    {              
        // advance animation
        _actionTime += dt;
        _clump->getAnimationController()->advance( dt );
    }
}

/**
 * actor abstracts
 */

void Character::onUpdateActivity(float dt)
{    
    // update action
    if( _active ) _action->update( dt );
}

void Character::onUpdatePhysics(void)
{
    // update physics
    _action->updatePhysics();
}

/**
 * class implementation
 */

Character::Character(Actor* parent, engine::IClump* clump) : Actor( parent )
{
    assert( clump );

    _clump  = clump;
    _action = NULL;
    _active = true;

    // add character model to scene stage
    _scene->getStage()->add( _clump );
}

Character::~Character()
{
    if( _action ) delete _action;
    _scene->getStage()->remove( _clump );
    _clump->release();
}