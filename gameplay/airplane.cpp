
#include "headers.h"
#include "airplane.h"
#include "gameplay.h"
#include "../common/istring.h"
#include "xpp.h"

/**
 * class implementation
 */

Airplane::Airplane(Actor* parent, AirplaneDesc* desc) : Actor( parent )
{
    _desc  = *desc;
    _roughMode = false;
    _landingMode = false;
    _restAltitude = _desc.initAltitude - _desc.lastAltitude;
    _waypointId = 0;
    _waypointFactor = 0;

    // obtain clump
    _clump = _desc.templateClump->clone( "AirplaneInstance" );    

    // obtain propeller frame
    _propellerFrame = Gameplay::iEngine->findFrame( 
        _clump->getFrame(), 
        _desc.propellerFrame.c_str() 
    );
    assert( _propellerFrame );

    // insert clump in to the scene
    getScene()->getStage()->add( _clump );

    // build initial transformation
    Vector3f z = _desc.initDirection;    
    Vector3f x; x.cross( Vector3f( 0,1,0 ), z ); x.normalize();
    Vector3f y; y.cross( z, x ); y.normalize();
    Vector3f p = _desc.initOffset;
    Matrix4f t( 
        x[0], x[1], x[2], 0.0f,
        y[0], y[1], y[2], 0.0f,
        z[0], z[1], z[2], 0.0f,
        p[0], p[1], p[2], 1.0f
    );

    // apply initial transformation
    _clump->getFrame()->setMatrix( Gameplay::iEngine->transformMatrix( _clump->getFrame()->getLTM(), t ) );

    // setup altitude
    _clump->getAnimationController()->advance( 0.0f );
    _clump->getFrame()->setPos( _clump->getFrame()->getPos() + Vector3f( 0.0f, _desc.initAltitude, 0.0f ) );
    _clump->getFrame()->getLTM();
    _velocity.set( 0,0,0 );
    _prevPoint = _propellerFrame->getPos();

    // create sound
    _propellerSound = Gameplay::iAudio->createStaticSound( _desc.propellerSound.c_str() );
    assert( _propellerSound );
    _propellerSound->setLoop( true );
    _propellerSound->setDistanceModel( 10000.0f, 100000.0f, 2.0f );    
    if( getScene()->getReverberation() )
    {
        _propellerSound->setReverberation(
            getScene()->getReverberation()->inGain,
            getScene()->getReverberation()->reverbMixDB,
            getScene()->getReverberation()->reverbTime,
            getScene()->getReverberation()->hfTimeRatio
        );
    }

    // build list of unoccupied exit points
    for( unsigned int i=0; i<_desc.exitPointFrames.size(); i++ )
    {
        engine::IFrame* frame = Gameplay::iEngine->findFrame(
            _clump->getFrame(), 
            _desc.exitPointFrames[i].c_str()
        );
        assert( frame );
        _unoccupiedExitPoints.push_back( frame );
    }

    // waypoint mode?
    if( _desc.waypoints.size() )
    {
        // setup airplane on to first waypoint
        Vector3f p;
        p = _desc.waypoints[0].pos;
        Vector3f z;
        z = _desc.waypoints[0].dir;
        Vector3f x;
        x.cross( Vector3f(0,1,0), z );
        x.normalize();
        Vector3f y;
        y.cross( z, x );
        y.normalize();
        _clump->getFrame()->setMatrix( Matrix4f( 
            x[0], x[1], x[2], 0.0f,
            y[0], y[1], y[2], 0.0f,
            z[0], z[1], z[2], 0.0f,
            p[0], p[1], p[2], 1.0f
        ) );
        _clump->getFrame()->getLTM();
    }
}

Airplane::~Airplane()
{
    if( _propellerSound ) _propellerSound->release();
    getScene()->getStage()->remove( _clump );
    _clump->release();
}

/**
 * actor abstracts
 */

void Airplane::onUpdatePhysics()
{
    if( !_roughMode )
    {
        float dt = ::simulationStepTime;

        _clump->getAnimationController()->advance( dt * _desc.animationSpeed );
        _clump->getFrame()->getLTM();
        
        // landing mode
        if( _landingMode && _clump->getFrame()->getPos()[1] > _desc.lastAltitude )
        {
            _clump->getFrame()->translate( Vector3f( 0.0f, -_desc.loweringSpeed * dt, 0.0f ) );
        }

        // calculate velocity
        Vector3f currPoint = _propellerFrame->getPos();
        _velocity = ( currPoint - _prevPoint ) / ( dt );
        _prevPoint = currPoint;
    }

    // fly by waypoints
    if( _desc.waypoints.size() && _waypointId < _desc.waypoints.size() - 1 )
    {
        // current waypoints
        AirplaneDesc::WayPoint waypoint1 = _desc.waypoints[_waypointId];
        AirplaneDesc::WayPoint waypoint2 = _desc.waypoints[_waypointId+1];

        // trajectory vector
        Vector3f trajectory = waypoint2.pos - waypoint1.pos;

        // current velocity
        float vel = waypoint1.vel * ( 1 - _waypointFactor ) + waypoint2.vel * _waypointFactor;

        // motion distance
        float distance = vel * ::simulationStepTime;
        
        // determine factor distance
        float fDistance = distance / trajectory.length();

        // move in factor space
        _waypointFactor += fDistance;

        // out of current waypoints?
        if( _waypointFactor > 1 )
        {
            // determine rest of distance
            fDistance = _waypointFactor - 1;
            distance = fDistance * trajectory.length();
            _waypointFactor = 0;
            _waypointId++;

            if( _waypointId < _desc.waypoints.size() - 1 )
            {
                // new waypoints
                waypoint1 = _desc.waypoints[_waypointId];
                waypoint2 = _desc.waypoints[_waypointId+1];

                // new trajectory vector
                trajectory = waypoint2.pos - waypoint1.pos;

                // new factor distance
                fDistance = distance / trajectory.length();

                // move in factor space
                _waypointFactor += fDistance;
            }
            else
            {
                _waypointFactor = 0;
            }
        }
    }
}

void Airplane::onUpdateActivity(float dt)
{
    if( _roughMode )
    {
        _clump->getAnimationController()->advance( dt * _desc.animationSpeed );
        _clump->getFrame()->getLTM();

        // landing mode
        if( _landingMode && _clump->getFrame()->getPos()[1] > _desc.lastAltitude )
        {
            // damp landing
            float damp = 1.0f;
            float dampAltitude = 1000.0f;            
            if( _restAltitude < dampAltitude )
            {
                damp = _restAltitude / dampAltitude;
                if( damp < 0 ) damp = 0;
            }

            _restAltitude -= _desc.loweringSpeed * dt * damp;
            _clump->getFrame()->translate( Vector3f( 0.0f, -_desc.loweringSpeed * dt * damp, 0.0f ) );
        }

        // calculate velocity
        Vector3f currPoint = _propellerFrame->getPos();
        _velocity = ( currPoint - _prevPoint ) / ( dt );
        _prevPoint = currPoint;
    }

    // place plane on to waypoint
    if( _desc.waypoints.size() )
    {
        //getCore()->logMessage( "%3.3f", _waypointFactor );

        while( _waypointFactor > 1 )
        {
            _waypointFactor -= 1;
        }

        if( _waypointId < _desc.waypoints.size() - 1 )
        {
            // setup airplane between waypoints
            Vector3f p;
            p = _desc.waypoints[_waypointId].pos * ( 1 - _waypointFactor ) + 
                _desc.waypoints[_waypointId+1].pos * _waypointFactor;
            Vector3f z;
            z = _desc.waypoints[_waypointId].dir * ( 1 - _waypointFactor ) + 
                _desc.waypoints[_waypointId+1].dir * _waypointFactor;
            z.normalize();
            Vector3f x;
            x.cross( Vector3f(0,1,0), z );
            x.normalize();
            Vector3f y;
            y.cross( z, x );
            y.normalize();
            _clump->getFrame()->setMatrix( Matrix4f( 
                x[0], x[1], x[2], 0.0f,
                y[0], y[1], y[2], 0.0f,
                z[0], z[1], z[2], 0.0f,
                p[0], p[1], p[2], 1.0f
            ) );
            _clump->getFrame()->getLTM();
        }
        else
        {
            // setup airplane on to last waypoint
            Vector3f p;
            p = _desc.waypoints[_waypointId].pos;
            Vector3f z;
            z = _desc.waypoints[_waypointId].dir;
            Vector3f x;
            x.cross( Vector3f(0,1,0), z );
            x.normalize();
            Vector3f y;
            y.cross( z, x );
            y.normalize();
            _clump->getFrame()->setMatrix( Matrix4f( 
                x[0], x[1], x[2], 0.0f,
                y[0], y[1], y[2], 0.0f,
                z[0], z[1], z[2], 0.0f,
                p[0], p[1], p[2], 1.0f
            ) );
            _clump->getFrame()->getLTM();
        }

        // calculate velocity
        Vector3f currPoint = _propellerFrame->getPos();
        _velocity = ( currPoint - _prevPoint ) / ( dt );
        _prevPoint = currPoint;
    }

    // update sound
    if( !_propellerSound->isPlaying() ) 
    {
        _propellerSound->play();
        _propellerSound->setGainLimits( 0.0f, 1.0f );        
    }
    _propellerSound->place( _propellerFrame->getPos(), _velocity );
    if( Gameplay::iGameplay->pitchShiftIsEnabled() )
    {
        _propellerSound->setPitchShift( _scene->getTimeSpeed() );
    }
}

void Airplane::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
}

/**
 * class behaviour
 */

unsigned int Airplane::getNumUnoccupiedExitPoints(void)
{
    return _unoccupiedExitPoints.size();
}

engine::IFrame* Airplane::occupyRandomExitPoint(void)
{
    assert( _unoccupiedExitPoints.size() > 0 );
    if( _unoccupiedExitPoints.size() )
    {
        unsigned int id = getCore()->getRandToolkit()->getUniformInt() % _unoccupiedExitPoints.size();
        assert( id < _unoccupiedExitPoints.size() );
        engine::IFrame* result = _unoccupiedExitPoints[id];
        _unoccupiedExitPoints.erase( &_unoccupiedExitPoints[id] );
        return result;
    }
    else
    {
        return NULL;
    }
}

void Airplane::setRoughMode(bool mode)
{
    _roughMode = mode;
}

void Airplane::setLandingMode(bool mode)
{
    _landingMode = mode;
}