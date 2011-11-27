
#include "headers.h"
#include "canopy.h"
#include "jumper.h"

/**
 * cord simulator
 */

CanopySimulator::CordSimulator::CordSimulator(
    float cascade, 
    engine::IFrame* riserJoint, 
    engine::IFrame* innerJoint, 
    engine::IFrame* outerJoint, 
    engine::IBatch* cordBatch,
    unsigned int&   inOutFirstInstanceId
)
{
    _cascade = cascade;
    _riserJoint = riserJoint;
    _innerJoint = innerJoint;
    _outerJoint = outerJoint;
    _cordBatch  = cordBatch;
    _firstInstaceId = inOutFirstInstanceId;
    inOutFirstInstanceId += 3;
}

CanopySimulator::CordSimulator::~CordSimulator()
{    
}

void CanopySimulator::CordSimulator::update(float dt)
{
    Vector3f riserPos = _riserJoint->getPos();
    Vector3f innerPos = _innerJoint->getPos();
    Vector3f outerPos = _outerJoint->getPos();

    // now calculate cascade position
    Vector3f cascadePos = innerPos + ( outerPos - innerPos ) * 0.5f ;
    Vector3f riserToCascade = cascadePos - riserPos;
    Vector3f riserToCascadeN = riserToCascade;
    riserToCascadeN.normalize();
    if( riserToCascade.length() < _cascade )
    {
        riserToCascadeN = riserToCascade;
    }
    else
    {
        riserToCascadeN *= _cascade;
    }
    cascadePos = riserPos + riserToCascadeN;

    // place cords
    Matrix4f matrix;
    unsigned int instanceId = _firstInstaceId;
    Jumper::placeCord( matrix, riserPos, cascadePos );
    _cordBatch->setMatrix( instanceId, matrix );
    instanceId++;
    Jumper::placeCord( matrix, cascadePos, innerPos );
    _cordBatch->setMatrix( instanceId, matrix );
    instanceId++;
    Jumper::placeCord( matrix, cascadePos, outerPos );
    _cordBatch->setMatrix( instanceId, matrix );
}

/**
 * brake cord simulator
 */


CanopySimulator::BrakeSimulator::BrakeSimulator(
    float aspect, 
    unsigned int numCanopyJoints, 
    engine::IFrame*  riserJoint, 
    engine::IFrame** canopyJoints,
    engine::IBatch*  cordBatch,
    unsigned int&    inOutFirstInstanceId
)
{
    _aspect = aspect;
    _numCanopyJoints = numCanopyJoints;
    _riserJoint = riserJoint;
    _canopyJoints = new engine::IFrame*[_numCanopyJoints];
    for( unsigned int i=0; i<_numCanopyJoints; i++ )
    {        
        _canopyJoints[i] = canopyJoints[i]; assert( _canopyJoints[i] );        
    }
    _cordBatch = cordBatch;
    _firstInstaceId = inOutFirstInstanceId;
    inOutFirstInstanceId += ( 1 + _numCanopyJoints );
}

CanopySimulator::BrakeSimulator::~BrakeSimulator()
{
}

void CanopySimulator::BrakeSimulator::update(float dt)
{
    Vector3f riserPos = _riserJoint->getPos();
    Vector3f branchingPos = _canopyJoints[0]->getPos();
    for( unsigned int i=1; i<_numCanopyJoints; i++ ) branchingPos += _canopyJoints[i]->getPos();
    branchingPos *= ( 1.0f / _numCanopyJoints );
    branchingPos = branchingPos - riserPos;
    branchingPos *= _aspect;
    branchingPos = riserPos + branchingPos;

    // place cords
    Matrix4f matrix;
    unsigned int instanceId = _firstInstaceId;
    Jumper::placeCord( matrix, riserPos, branchingPos );
    _cordBatch->setMatrix( instanceId, matrix );
    instanceId++;
    for( i=0; i<_numCanopyJoints; i++ )
    {
        Jumper::placeCord( matrix, branchingPos, _canopyJoints[i]->getPos() );
        _cordBatch->setMatrix( instanceId, matrix );
        instanceId++;
    }
}