
#include "headers.h"
#include "crowd.h"
#include "imath.h"
#include "../common/istring.h"

/**
 * spectator idle sequence
 */

static engine::AnimSequence idleSequence = 
{ 
    FRAMETIME(1), 
    FRAMETIME(59), 
    engine::ltPeriodic, 
    FRAMETIME(1)
};

/**
 * spectator idle watching sequence
 */

static engine::AnimSequence watchSequence = 
{
    FRAMETIME(306), 
    FRAMETIME(358), 
    engine::ltMirror,
    FRAMETIME(329)
};

/**
 * properties of wishes
 */

const float activeRadius      = 5000.0f;
const float wishWatchDelayMin = 0.2f;
const float wishWatchDelayMax = 5.0f;
const float wishWatchTimeMin  = 3.0f;
const float wishWatchTimeMax  = 10.0f;
const float wishRelaxTimeMin  = 1.0f;
const float wishRelaxTimeMax  = 10.0f;

/**
 * actor abstracts
 */

void Spectator::onUpdateActivity(float dt)
{
    // spectator can become inactive when he is relaxing
    if( _active && _wish == wishRelax && _scene->getCamera() )
    {
        Matrix4f cameraPose = _scene->getCamera()->getPose();
        Vector3f cameraPos( cameraPose[3][0], cameraPose[3][1], cameraPose[3][2] );
        float distance = ( cameraPos - _clump->getFrame()->getPos() ).length();
        if( distance > activeRadius && _action->getActionTime() > _action->getBlendTime() )
        {
            _active = false;
        }
    }
    if( !_active && _scene->getCamera() )
    {
        Matrix4f cameraPose = _scene->getCamera()->getPose();
        Vector3f cameraPos( cameraPose[3][0], cameraPose[3][1], cameraPose[3][2] );
        float distance = ( cameraPos - _clump->getFrame()->getPos() ).length();
        if( distance < activeRadius ) _active = true;
    }

    if( !_active ) return;

    // fulfill wish
    switch( _wish )
    {
    case wishRelax: {
        // setup corresponding action
        if( !actionIs(Idle) || 
            _clump->getAnimationController()->getTrackAnimation(0) != &idleSequence )
        {
            delete _action;
            _action = new Character::Idle( _clump, &idleSequence, 0.2f, 1.0f );
        }
        // decrease relax time
        _relaxTime -= dt;
        if( _relaxTime < 0 ) _endOfWish = true;
        } break;
    case wishGoto: {
        // obtain distance vector to desired position
        Vector3f distance = _gotoPos - _clump->getFrame()->getPos();
        if( distance.length() > _gotoError )
        {
            // if i am did not moves yet
            if( !actionIs(Move) )
            {
                // obtain angle to desired pos
                Vector3f direction = distance;
                direction[1] = 0.0f;
                direction.normalize();
                float angle = calcAngle( direction, _clump->getFrame()->getAt(), Vector3f( 0,1,0 ) );
                // if angle too large, i will turns
                if( fabs( angle ) > 5.0f )
                {
                    if( !actionIs(Turn) )
                    {
                        delete _action;
                        _action = new Turn( _clump, direction );
                    }
                }
                // else i will move
                else if( !actionIs(Move) )
                {
                    delete _action;
                    _action = new Move( _clump, _enclosure, _gotoPos, false );
                }
            }
            // if wish is undesirable, reject it
            else if( _action->isEndOfAction() )
            {
                _endOfWish = true;
            }
        }
        else
        {
            _endOfWish = true;
        }
        } break;
    case wishWatch: {
        // simulate delay due to character orientation
        _watchDelay -= dt;
        if( _watchDelay > 0 ) break;
        // obtain distance vector to desired position
        Vector3f distance = _watchPos - _clump->getFrame()->getPos();
        // obtain angle to desired pos
        Vector3f direction = distance;
        direction[1] = 0.0f;
        direction.normalize();
        float angle = calcAngle( direction, _clump->getFrame()->getAt(), Vector3f( 0,1,0 ) );
        // if angle too large, i will turns
        if( fabs( angle ) > 5.0f )
        {
            if( !actionIs(Turn) )
            {
                delete _action;
                _action = new Turn( _clump, direction );
            }
        }
        else
        {
            if( !actionIs(Idle) ||
                _clump->getAnimationController()->getTrackAnimation( 0 ) != &watchSequence )
            {
                delete _action;
                _action = new Character::Idle( _clump, &watchSequence, 0.2f, 1.0f );
            }
        }
        // decrease watch time
        _watchTime -= dt;
        if( _watchTime <= 0 ) _endOfWish = true;
        } break;
    }

    // end of wish?
    if( _endOfWish )
    {
        // complete previous wish
        if( _wish == wishGoto )
        {
            Crowd* crowd = dynamic_cast<Crowd*>( _parent ); assert( crowd );
            crowd->endWalk();
        }

        // choose new wish
        if( _wish != wishRelax )
        {
            _wish = wishRelax;
            _relaxTime = getCore()->getRandToolkit()->getUniform( wishRelaxTimeMin ,wishRelaxTimeMax );
        }
        else
        {
            // request for a walk
            Crowd* crowd = dynamic_cast<Crowd*>( _parent ); assert( crowd );
            if( crowd->beginWalk() )
            {
                _wish = wishGoto;
                _gotoPos = _enclosure->place();
                _gotoError = 100.0f;
            }
            else
            {
                _wish = wishRelax;
                _relaxTime = getCore()->getRandToolkit()->getUniform( wishRelaxTimeMin ,wishRelaxTimeMax );
            }
        }
        _endOfWish = false;
    }

    // inherited behaviour
    Character::onUpdateActivity( dt );
}

engine::IAtomic* Spectator::onRenderAtomic(engine::IAtomic* atomic, void* data)
{
    Spectator* __this = reinterpret_cast<Spectator*>( data );

    engine::IShader* shader;
    int numShaders = atomic->getGeometry()->getNumShaders();
    for( int i=0; i<numShaders; i++ )
    {
        shader = atomic->getGeometry()->getShader( i );
        if( shader->getLayerTexture( 0 ) != __this->_texture )
        {
            shader->setLayerTexture( 0, __this->_texture );
        }
    }

    return NULL;
}

/**
 * class implementation
 */

Spectator::Spectator(Crowd* crowd, engine::IClump* cloneSource, Enclosure* enclosure) :
    Character( crowd, cloneSource->clone( "Spectator" ) )
{
    assert( crowd );
    _name = "Spectator";
    _enclosure = enclosure;

    // choose texture
    unsigned int id = unsigned int( getCore()->getRandToolkit()->getUniform( 0, 7 ) );
    id = id < 1 ? 1 : ( id > 6 ? 6 : id );
    _texture = Gameplay::iEngine->getTexture( strformat( "crowdmale0%d", id ).c_str() );
    assert( _texture );

    // setup LODs
    float LOD0min = 0.0f;
    float LOD0max = 500.0f;    
    float LOD1min = 500.0f;
    float LOD1max = 1000.0f;    
    float LOD2min = 1000.0f;
    float LOD2max = 3000.0f;    
    float LOD3min = 3000.0f;
    float LOD3max = 100000.0f;

    callback::AtomicL atomicL;
    _clump->forAllAtomics( callback::enumerateAtomics, &atomicL );
    for( callback::AtomicI atomicI = atomicL.begin(); atomicI != atomicL.end(); atomicI++ )
    {
        // LOD0
        if( strcmp( "character01_head01_04_Secondery_character02", (*atomicI)->getFrame()->getName() ) == 0 )
        {
            _clump->setLOD( *atomicI, LOD0max, LOD0min );
            (*atomicI)->setRenderCallback( onRenderAtomic, this );
        }
        // LOD1
        if( strcmp( "character01_head01_04_Secondery_character01_LOD1", (*atomicI)->getFrame()->getName() ) == 0 )
        {
            _clump->setLOD( *atomicI, LOD1max, LOD1min );
            (*atomicI)->setRenderCallback( onRenderAtomic, this );
        }
        // LOD2
        if( strcmp( "character01_head01_04_Secondery_character01_LOD2", (*atomicI)->getFrame()->getName() ) == 0 )
        {
            _clump->setLOD( *atomicI, LOD2max, LOD2min );
            (*atomicI)->setRenderCallback( onRenderAtomic, this );
        }
        // LOD2
        if( strcmp( "character01_head01_04_Secondery_character01_LOD3", (*atomicI)->getFrame()->getName() ) == 0 )
        {
            _clump->setLOD( *atomicI, LOD3max, LOD3min );
            (*atomicI)->setRenderCallback( onRenderAtomic, this );
        }
    }

    // place actor
    _clump->getFrame()->setPos( enclosure->place() );

    // choose random direction for an actor
    _clump->getFrame()->getLTM();
    _clump->getFrame()->rotateRelative( 
        Vector3f(0,1,0), 
        getCore()->getRandToolkit()->getUniform(-180,180)
    );
    _clump->getFrame()->getLTM();

    // setup idle action
    _action = new Character::Idle( _clump, &idleSequence, 0.2f, 1.0f );

    // setup wish
    _wish = wishRelax;
    _relaxTime = getCore()->getRandToolkit()->getUniform( wishRelaxTimeMin, wishRelaxTimeMax );
    _endOfWish = false;
}

Spectator::~Spectator()
{
}