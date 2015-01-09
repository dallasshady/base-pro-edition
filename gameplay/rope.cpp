
#include "headers.h"
#include "canopy.h"
#include "jumper.h"

CanopySimulator::Rope::Rope(unsigned int numJoints, float mass, float length, PxScene* scene)
{
    assert( numJoints >= 2 );
    _numJoints = numJoints;
    _mass      = mass;
    _length    = length;
    _nxScene   = scene;
    _nxSegmentBody  = new PxRigidDynamic*[_numJoints-1];
    _nxSegmentJoint = new PxDistanceJoint*[_numJoints];
    unsigned int i;
    for( i=0; i<_numJoints-1; i++ )
    {
        _nxSegmentBody[i] = NULL;
    }
    for( i=0; i<_numJoints; i++ )
    {
        _nxSegmentJoint[i] = NULL;
    }
}

CanopySimulator::Rope::~Rope()
{
    unsigned int i;
    for( i=0; i<_numJoints-1; i++ )
    {
		if( _nxSegmentBody[i] ) _nxSegmentBody[i]->release();
    }
    for( i=0; i<_numJoints; i++ )
    {
		if( _nxSegmentJoint[i] ) _nxSegmentJoint[i]->release();
    }
    delete[] _nxSegmentBody;
    delete[] _nxSegmentJoint;
}

void CanopySimulator::Rope::initialize(PxRigidDynamic* actor1, PxVec3 anchor1, PxRigidDynamic* actor2, PxVec3 anchor2)
{
    _nxActor1 = actor1; assert( _nxActor1 );
    _nxActor2 = actor2; assert( _nxActor2 );
    _anchor1 = anchor1;
    _anchor2 = anchor2;

    // calculate worldspace positions for connections points
    Matrix4f ltm1  = wrap( _nxActor1->getGlobalPose() );
    Matrix4f ltm2  = wrap( _nxActor2->getGlobalPose() );
    PxVec3 pos1 = wrap( Gameplay::iEngine->transformCoord( wrap( _anchor1 ), ltm1 ) );
    PxVec3 pos2 = wrap( Gameplay::iEngine->transformCoord( wrap( _anchor2 ), ltm2 ) );
    PxVec3 dir  = pos2 - pos1;

    // check distance btw connection points (correction just for stability purpose)
    float idist = dir.magnitude();
    dir.normalize();

    // create intermediate segment bodies
    PxVec3 pos = pos1;
    float step = idist / _numJoints;
	unsigned int i;
    for( i=0; i<_numJoints-1; i++ )
    {
        // segment position
        pos += dir * step;


        // create segment body
		_nxSegmentBody[i] = PxGetPhysics().createRigidDynamic(PxTransform(PxIDENTITY::PxIdentity));
		_nxSegmentBody[i]->userData = this;
		_nxSegmentBody[i]->setMass(_mass / ( _numJoints - 1 ));
		_nxSegmentBody[i]->setLinearDamping(2.0f);
		_nxSegmentBody[i]->setAngularDamping(0.0f);
		_nxSegmentBody[i]->setSolverIterationCounts(32);
		_nxSegmentBody[i]->setMassSpaceInertiaTensor(PxVec3(1, 1, 1) * (_mass / ( _numJoints - 1 )));
		_nxSegmentBody[i]->setActorFlag(PxActorFlag::eVISUALIZATION, true);
		_nxScene->addActor(*_nxSegmentBody[i]);

		PxTransform pose = _nxSegmentBody[i]->getGlobalPose();
		pose.p = pos;
		pose.q = PxQuat(PxIDENTITY::PxIdentity);

		_nxSegmentBody[i]->setGlobalPose(pose);

        // initialize velocity
		_nxSegmentBody[i]->addForce( _nxActor1->getLinearVelocity(), PxForceMode::eVELOCITY_CHANGE );
    }

    // create joints
    for( i=0; i<_numJoints; i++ )
    {
        // first joint (actor1-segment1)
        if( i == 0 )
        {
			_nxSegmentJoint[i] = :: PxDistanceJointCreate(PxGetPhysics(), _nxActor1, PxTransform(_anchor1), _nxSegmentBody[0], PxTransform(PxIDENTITY::PxIdentity));
			_nxSegmentJoint[i]->setMinDistance(0.0f);
			_nxSegmentJoint[i]->setMaxDistance(_length / _numJoints);
			_nxSegmentJoint[i]->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);
        }
        // last joint (segmentN-actor2)
        else if( i == _numJoints-1 )
        {
			_nxSegmentJoint[i] = :: PxDistanceJointCreate(PxGetPhysics(), _nxSegmentBody[_numJoints-2], PxTransform(PxIDENTITY::PxIdentity), _nxActor2, PxTransform(_anchor2));
			_nxSegmentJoint[i]->setMinDistance(0.0f);
			_nxSegmentJoint[i]->setMaxDistance(_length / _numJoints);
			_nxSegmentJoint[i]->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);
        }
        // intermediate joint
        else
        {
			_nxSegmentJoint[i] = :: PxDistanceJointCreate(PxGetPhysics(), _nxSegmentBody[i-1], PxTransform(PxIDENTITY::PxIdentity), _nxSegmentBody[i], PxTransform(PxIDENTITY::PxIdentity));
			_nxSegmentJoint[i]->setMinDistance(0.0f);
			_nxSegmentJoint[i]->setMaxDistance(_length / _numJoints);
			_nxSegmentJoint[i]->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);
        }
    }
}

void CanopySimulator::Rope::setAnchor1(PxVec3 anchor1)
{
    _anchor1 = anchor1;

    // update first joint
    if( _nxSegmentJoint[0] )
    {
		_nxSegmentJoint[0]->release();
		_nxSegmentJoint[0] = :: PxDistanceJointCreate(PxGetPhysics(), _nxActor1, PxTransform(_anchor1), _nxSegmentBody[0], PxTransform(PxIDENTITY::PxIdentity));
		_nxSegmentJoint[0]->setMinDistance(0.0f);
		_nxSegmentJoint[0]->setMaxDistance(_length / _numJoints);
		_nxSegmentJoint[0]->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);
    }
}