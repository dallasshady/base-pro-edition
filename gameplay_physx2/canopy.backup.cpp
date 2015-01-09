
#include "headers.h"
#include "canopy.h"
#include "jumper.h"
#include "sound.h"
#include "../common/istring.h"

/**
 * wind influence multiplier
 */

const float windfluence = 0.5f;

#define BRAKE_STOW_LEVEL 0.5f
/**
 * related animation sequences
 */

static float animStartTime = FRAMETIME(2);
static float animEndTime   = FRAMETIME(38);

static engine::AnimSequence openingSequence = 
{ 
    animStartTime, animEndTime, engine::ltNone, 0.0f 
};

/**
 * model management
 */

static engine::IFrame* findRootCB(engine::IFrame* frame, void* data)
{
    if( strcmp( frame->getName(), "Root" ) == 0 )
    {
        *((engine::IFrame**)(data)) = frame;
        return NULL;
    }
    return frame;
}

engine::IAtomic* CanopySimulator::getCollisionGeometry(engine::IClump* clump)
{
    engine::IFrame* frame = Gameplay::iEngine->findFrame( clump->getFrame(), "CollisionGeometry_child0" ); assert( frame );
    engine::IAtomic* collisionGeometryA = Gameplay::iEngine->getAtomic( clump, frame ); assert( collisionGeometryA );    
    return collisionGeometryA;
}

engine::IFrame* CanopySimulator::getPhysicsJointFrontLeft(engine::IClump* clump)
{
    engine::IFrame* physicsJointFrontLeft = Gameplay::iEngine->findFrame( clump->getFrame(), "PhysicsJointFrontLeft" );
    assert( physicsJointFrontLeft );    
    return physicsJointFrontLeft;
}

engine::IFrame* CanopySimulator::getPhysicsJointFrontRight(engine::IClump* clump)
{
    engine::IFrame* physicsJointFrontRight = Gameplay::iEngine->findFrame( clump->getFrame(), "PhysicsJointFrontRight" );
    assert( physicsJointFrontRight );    
    return physicsJointFrontRight;
}

engine::IFrame* CanopySimulator::getPhysicsJointRearLeft(engine::IClump* clump)
{
    engine::IFrame* physicsJointRearLeft = Gameplay::iEngine->findFrame( clump->getFrame(), "PhysicsJointRearLeft" );
    assert( physicsJointRearLeft );    
    return physicsJointRearLeft;
}

engine::IFrame* CanopySimulator::getPhysicsJointRearRight(engine::IClump* clump)
{
    engine::IFrame* physicsJointRearRight = Gameplay::iEngine->findFrame( clump->getFrame(), "PhysicsJointRearRight" );
    assert( physicsJointRearRight );    
    return physicsJointRearRight;
}

engine::IFrame* CanopySimulator::getPilotCordJoint(engine::IClump* clump)
{    
    engine::IFrame* pilotCordJoint = Gameplay::iEngine->findFrame( clump->getFrame(), "PilotCordJoint" );
    assert( pilotCordJoint );    
    return pilotCordJoint;
}

engine::IFrame* CanopySimulator::getSliderJointFrontLeft(engine::IClump* clump)
{
    engine::IFrame* sliderJointFrontLeft = Gameplay::iEngine->findFrame( clump->getFrame(), "FL" );
    assert( sliderJointFrontLeft );    
    return sliderJointFrontLeft;
}

engine::IFrame* CanopySimulator::getSliderJointFrontRight(engine::IClump* clump)
{
    engine::IFrame* sliderJointFrontRight = Gameplay::iEngine->findFrame( clump->getFrame(), "FR" );
    assert( sliderJointFrontRight );    
    return sliderJointFrontRight;
}

engine::IFrame* CanopySimulator::getSliderJointRearLeft(engine::IClump* clump)
{
    engine::IFrame* sliderJointRearLeft = Gameplay::iEngine->findFrame( clump->getFrame(), "RL" );
    assert( sliderJointRearLeft );    
    return sliderJointRearLeft;
}

engine::IFrame* CanopySimulator::getSliderJointRearRight(engine::IClump* clump)
{
    engine::IFrame* sliderJointRearRight = Gameplay::iEngine->findFrame( clump->getFrame(), "RR" );
    assert( sliderJointRearRight );    
    return sliderJointRearRight;
}

/**
 * class implementation
 */

static engine::IAtomic* setCanopyUpdateTresholdCB(engine::IAtomic* atomic, void* data)
{
    atomic->setUpdateTreshold( 0.0f, 0.0f );
    return atomic;
}

static engine::IAtomic* setCanopyShadowCastCB(engine::IAtomic* atomic, void* data)
{
    atomic->setFlags( engine::afCollision | engine::afRender | engine::afCastShadow );
    return atomic;
}

CanopySimulator::CanopySimulator(Actor* jumper, Gear* gear, bool sliderUp) :
    Actor( jumper )
{
    assert( _gear );
	Jumper* castedJumper = dynamic_cast<Jumper*>( jumper );

    _name = "CanopySimulator";
    _gear = gear; 
    _collideJumper = false;
	if (gear->type == gtCanopy) {
		_gearRecord = database::Canopy::getRecord( _gear->id );
	} else if (gear->type == gtReserve) {
		_gearRecord = database::Canopy::getReserveRecord( _gear->id );
	} else {
		assert("should not be here!!!");
	}

    _nxConnected = NULL;
    _nxCanopy = NULL;
    _frontLeftRope = _frontRightRope = _rearLeftRope = _rearRightRope = NULL;
    _roughJoints[0] = _roughJoints[1] = _roughJoints[2] = _roughJoints[3] = NULL;
    _frontLeftRiser = _frontRightRiser = _rearLeftRiser = _rearRightRiser= NULL;
    _leftDeep = _rightDeep = 0.0f;
	_leftForcedDeep = _rightForcedDeep = -1.0f;
	_leftStowed = _rightStowed = castedJumper->isPlayer();	// STUB: stow only for players
    _leftWarpDeep = _rightWarpDeep = 0.0f;
	_leftRearRiser = _rightRearRiser = 0.0f;
    _leftLOW = 0.0f;
    _rightLOW = 0.0f;
    _enableWind = true;
    _flightSoundEnabled = false;
    _wloIsEffective = true;
    _lineoverIsEliminated = true;
    _mBlinkTime = 0.0f;
    _cohesionState = false;
    _cohesionPoint = NxVec3( 0,0,0 );
    _cohesionJoint = NULL;
	isCutAway = false;
    // initialize burden calculation
    _bcStep = 0;
    _bcPrevVel.set( 0,0,0 );
    _bcBurden.set( 0,0,0 );

    // create 3d-models
    engine::IClump* canopyTemplate = Gameplay::iGameplay->findClump( _gearRecord->templateName ); assert( canopyTemplate );
    _canopyClump = canopyTemplate->clone( "Canopy" );    
    _canopyClump->forAllAtomics( setCanopyUpdateTresholdCB, NULL );

    // setup rendering
    _renderCallback = new CanopyRenderCallback;
    _renderCallback->setTexture( _gearRecord );
    _renderCallback->apply( _canopyClump );

    // initialize cords    
    _numCords = 4 * _gearRecord->riserScheme->getNumCords();
    _cords = new CordSimulator*[_numCords];
    for( unsigned int i=0; i<_numCords; i++ ) _cords[i] = NULL;
    _leftBrake = _rightBrake = NULL;
    _cordBatch = NULL;

    // scale model
    Vector3f scale = ::calcScale( _canopyClump->getFrame()->getMatrix() );
    scale *= _gearRecord->scale;

    _canopyClump->getFrame()->setMatrix( Matrix4f( 
        scale[0],        0,        0, 0,
               0, scale[1],        0, 0,
               0,        0, scale[2], 0,
               0,        0,        0, 1
    ) );

    // setup animation
    _canopyClump->getAnimationController()->setTrackAnimation( 0, &openingSequence );
    _canopyClump->getAnimationController()->setTrackSpeed( 0, 1.0f );
    _canopyClump->getAnimationController()->setTrackWeight( 0, 1.0f );
    _canopyClump->getAnimationController()->setTrackActivity( 0, true );
    _canopyClump->getAnimationController()->resetTrackTime( 0 );
    _canopyClump->getAnimationController()->advance( animEndTime - animStartTime );
    _canopyClump->getFrame()->getLTM();

    // geometry properties
    Vector3f aabbScale(
        getCollisionGeometry( _canopyClump )->getFrame()->getRight().length(),
        getCollisionGeometry( _canopyClump )->getFrame()->getUp().length(),
        getCollisionGeometry( _canopyClump )->getFrame()->getAt().length()
    );
    Vector3f aabbInf = getCollisionGeometry( _canopyClump )->getGeometry()->getAABBInf();
    Vector3f aabbSup = getCollisionGeometry( _canopyClump )->getGeometry()->getAABBSup();
    Vector3f aabbDim = ( aabbSup - aabbInf );
    aabbDim[0] *= aabbScale[0] * 0.5f,
    aabbDim[1] *= aabbScale[1] * 0.5f,
    aabbDim[2] *= aabbScale[2] * 0.5f;

    // calculate local anchors for joint connections
    Matrix4f childLTM  = CanopySimulator::getPhysicsJointFrontLeft( _canopyClump )->getLTM(); orthoNormalize( childLTM );
    childLTM[3][2] -= aabbDim[2];
    Matrix4f parentLTM = CanopySimulator::getCollisionGeometry( _canopyClump )->getFrame()->getLTM(); orthoNormalize( parentLTM );
    MatrixConversion mc;
    mc.setup( parentLTM, childLTM );
    Matrix4f childM = mc.getTransformation();
    _frontLeftAnchor[1] = wrap( Vector3f( childM[3][0], childM[3][1], childM[3][2] ) );
    childLTM  = CanopySimulator::getPhysicsJointFrontRight( _canopyClump )->getLTM(); orthoNormalize( childLTM );
    childLTM[3][2] -= aabbDim[2];
    parentLTM = CanopySimulator::getCollisionGeometry( _canopyClump )->getFrame()->getLTM(); orthoNormalize( parentLTM );
    mc.setup( parentLTM, childLTM );
    childM = mc.getTransformation();
    _frontRightAnchor[1] = wrap( Vector3f( childM[3][0], childM[3][1], childM[3][2] ) );
    childLTM  = CanopySimulator::getPhysicsJointRearLeft( _canopyClump )->getLTM(); orthoNormalize( childLTM );
    childLTM[3][2] -= aabbDim[2];
    parentLTM = CanopySimulator::getCollisionGeometry( _canopyClump )->getFrame()->getLTM(); orthoNormalize( parentLTM );
    mc.setup( parentLTM, childLTM );
    childM = mc.getTransformation();
    _rearLeftAnchor[1] = wrap( Vector3f( childM[3][0], childM[3][1], childM[3][2] ) );
    childLTM  = CanopySimulator::getPhysicsJointRearRight( _canopyClump )->getLTM(); orthoNormalize( childLTM );
    childLTM[3][2] -= aabbDim[2];
    parentLTM = CanopySimulator::getCollisionGeometry( _canopyClump )->getFrame()->getLTM(); orthoNormalize( parentLTM );
    mc.setup( parentLTM, childLTM );
    childM = mc.getTransformation();
    _rearRightAnchor[1] = wrap( Vector3f( childM[3][0], childM[3][1], childM[3][2] ) );

    // calculate local pilot anchor
    childLTM  = CanopySimulator::getPilotCordJoint( _canopyClump )->getLTM(); orthoNormalize( childLTM );
    childLTM[3][2] -= aabbDim[2];
    parentLTM = CanopySimulator::getCollisionGeometry( _canopyClump )->getFrame()->getLTM(); orthoNormalize( parentLTM );
    mc.setup( parentLTM, childLTM );
    childM = mc.getTransformation();
    _pilotAnchor = wrap( Vector3f( childM[3][0], childM[3][1], childM[3][2]+10.5f ) );

    // enable shadows
    if( true || castedJumper->isPlayer() )
    {
        int shadows = atoi( Gameplay::iGameplay->getConfigElement( "video" )->Attribute( "shadows" ) );
        if( shadows )
        {
            _canopyClump->forAllAtomics( setCanopyShadowCastCB, NULL );
        }
    }

    // hide auxially models    
    getCollisionGeometry( _canopyClump )->setFlags( 0 );

    // setup slider
    _sliderUp = sliderUp;
    _sliderClump = NULL;
    _sliderCordFL = _sliderCordFR = _sliderCordRL = _sliderCordRR = NULL;
    _sliderPosFL = _sliderPosFR = _gearRecord->frontCord;
    _sliderPosRL = _sliderPosRR = _gearRecord->rearCord;
    _slidingTime = 0.0f;
    if( _sliderUp )
    {
        engine::IClump* sliderTemplate = Gameplay::iGameplay->findClump( "Slider" ); assert( sliderTemplate );
        _sliderClump = sliderTemplate->clone( "Slider" ); assert( _sliderClump );       
		// scale model
		Vector3f scale = ::calcScale( _sliderClump->getFrame()->getMatrix() );
		scale *= 0.3f;

		_sliderClump->getFrame()->setMatrix( Matrix4f( 
			scale[0],        0,        0, 0,
				0, scale[1],        0, 0,
				0,        0, scale[2], 0,
				0,        0,        0, 1
		) );

        engine::IClump* cordTemplate = Gameplay::iGameplay->findClump( "Cord" ); assert( cordTemplate );
        _sliderCordFL = cordTemplate->clone( "SliderCordFL" );
        _sliderCordFR = cordTemplate->clone( "SliderCordFR" );
        _sliderCordRL = cordTemplate->clone( "SliderCordRL" );
        _sliderCordRR = cordTemplate->clone( "SliderCordRR" );

        // slider half-diagonal
        _sliderClump->getFrame()->setMatrix( Matrix4f( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 ) );
        _sliderClump->getFrame()->getLTM();
        _sliderHD = ( getSliderJointFrontLeft( _sliderClump )->getPos() - _sliderClump->getFrame()->getPos() ).length();


    }

    // reset animation
    _canopyClump->getAnimationController()->resetTrackTime( 0 );
    _canopyClump->getFrame()->getLTM();

    // setup inflation
    _opening = 0.0f;
    _inflation = 0.0f;

    // setup collapse simulation
    _collapseAreas = new CollapseArea[_gearRecord->Cnum];
    memset( _collapseAreas, 0, _gearRecord->Cnum * sizeof(CollapseArea) );

    // setup PABs
    _pabs = new PAB*[_gearRecord->riserScheme->getNumPABs()];
    memset( _pabs, 0, sizeof(PAB*) * _gearRecord->riserScheme->getNumPABs() );
    unsigned int j=0;
    for( i=0; i<_gearRecord->riserScheme->getNumPABs(); i++ )
    {
        if( _gearRecord->riserScheme->getPAB( i )->pabType == database::pabtLeftFlap )
        {
            _pabs[j] = new Flap( _canopyClump, _gearRecord->riserScheme->getPAB( i ), &_leftDeep );
            j++;
        }
        else if( _gearRecord->riserScheme->getPAB( i )->pabType == database::pabtRightFlap )
        {
            _pabs[j] = new Flap( _canopyClump, _gearRecord->riserScheme->getPAB( i ), &_rightDeep );
            j++;
        }
        else if( _gearRecord->riserScheme->getPAB( i )->pabType == database::pabtFrontSection ||
                 _gearRecord->riserScheme->getPAB( i )->pabType == database::pabtRearSection )
        {
            _pabs[j] = new Section( _canopyClump, _gearRecord->riserScheme->getPAB( i ), this );
            j++;
        }
    }
    for( i=0; i<_gearRecord->riserScheme->getNumPABs(); i++ )
    {
        _pabs[i]->setupConstraint( _gearRecord->riserScheme->getNumPABs(), _pabs );
    }

    // lineover joints
    _leftLineoverJoint = Gameplay::iEngine->findFrame( _canopyClump->getFrame(), _gearRecord->riserScheme->getLeftLineoverJointName() ); 
    _rightLineoverJoint = Gameplay::iEngine->findFrame( _canopyClump->getFrame(), _gearRecord->riserScheme->getRightLineoverJointName() );
    assert( _leftLineoverJoint );
    assert( _rightLineoverJoint );

    // signature windows
    _signature = Gameplay::iGui->createWindow( "CanopySignature" ); assert( _signature );
    _leftLineoverSignature = Gameplay::iGui->createWindow( "MalfunctionSignature" ); assert( _leftLineoverSignature );
    _rightLineoverSignature = Gameplay::iGui->createWindow( "MalfunctionSignature" ); assert( _rightLineoverSignature );
    _linetwistsSignature = Gameplay::iGui->createWindow( "MalfunctionSignature" ); assert( _linetwistsSignature );

    // jumper is player?    
    bool jumperIsPlayer = ( castedJumper != NULL ) && castedJumper->isPlayer();
    
    // dispatch player canopy
    if( jumperIsPlayer )
    {
        if( _gearRecord->skydiving || !castedJumper->getVirtues()->equipment.malfunctions )
        {            
            _wloToggles = NULL;
            _hookKnife = NULL;
        }
        else
        {
            _wloToggles = new WLOToggles( jumperIsPlayer );
            _hookKnife = new HookKnife( jumperIsPlayer );
        }
    }
    else
    {
        if( _gearRecord->skydiving )
        {
            _wloToggles = NULL;
            _hookKnife = NULL;
        }
        else
        {
            _wloToggles = new WLOToggles( jumperIsPlayer );
            _hookKnife = new HookKnife( jumperIsPlayer );
        }
    }

    // sound actor
    //if( jumperIsPlayer ) 
	new FlightSound( this );
}

CanopySimulator::~CanopySimulator()
{
    // release PABs
    for( unsigned int i=0; i<_gearRecord->riserScheme->getNumPABs(); i++ ) if( _pabs[i] ) delete _pabs[i];
    delete[] _pabs;

    // release collapse simulation structs
    delete[] _collapseAreas;

    // release rigging simulators
    if( _wloToggles ) delete _wloToggles;
    if( _hookKnife ) delete _hookKnife;

    // release signature window
    _signature->getPanel()->release();
    _leftLineoverSignature->getPanel()->release();
    _rightLineoverSignature->getPanel()->release();
    _linetwistsSignature->getPanel()->release();

    if( isOpened() )
    {
        if( _frontLeftRope ) delete _frontLeftRope;
        if( _frontRightRope ) delete _frontRightRope;
        if( _rearLeftRope ) delete _rearLeftRope;
        if( _rearRightRope ) delete _rearRightRope;
		if (!isCutAway) {
			if( _roughJoints[0] ) _scene->getPhScene()->releaseJoint( *(_roughJoints[0]) );
			if( _roughJoints[1] ) _scene->getPhScene()->releaseJoint( *(_roughJoints[1]) );
			if( _roughJoints[2] ) _scene->getPhScene()->releaseJoint( *(_roughJoints[2]) );
			if( _roughJoints[3] ) _scene->getPhScene()->releaseJoint( *(_roughJoints[3]) );
			if( _cohesionJoint ) _scene->getPhScene()->releaseJoint( *_cohesionJoint );
		}
        if( _nxCanopy ) _scene->getPhScene()->releaseActor( *_nxCanopy );
    }
    if (_cords) for( unsigned int i=0; i<_numCords; i++ ) if( _cords[i] ) delete _cords[i];
    delete[] _cords;
    if( _leftBrake ) delete _leftBrake;
    if( _rightBrake ) delete _rightBrake;
    if( isOpened() ) _scene->getStage()->remove( _canopyClump );
    _renderCallback->restore( _canopyClump );
    delete _renderCallback;
    _canopyClump->release();
    if( _sliderUp )
    {
        if( isOpened() ) 
        {
            _scene->getStage()->remove( _sliderClump );
            _scene->getStage()->remove( _sliderCordFL );
            _scene->getStage()->remove( _sliderCordFR );
            _scene->getStage()->remove( _sliderCordRL );
            _scene->getStage()->remove( _sliderCordRR );
        }
        _sliderClump->release();
        _sliderCordFL->release();
        _sliderCordFR->release();
        _sliderCordRL->release();
        _sliderCordRR->release();
    }
    if( _cordBatch ) 
    {
        getScene()->getStage()->remove( _cordBatch );
        _cordBatch->release();
    }
}

/**
 * class behaviour
 */

void CanopySimulator::disconnect(
        NxActor* actor, // connected weight simulator
        NxVec3 fla, NxVec3 fra, NxVec3 rla, NxVec3 rra, // quartet of local anchors for physics simulation
        engine::IFrame* flr, engine::IFrame* frr, engine::IFrame* rlr, engine::IFrame* rrr  // quartet of joints to place cord connections
) {


	//// create new actor
	//NxBodyDesc nxBodyDesc;
	//nxBodyDesc.massSpaceInertia.set( 0,0,0 ); // tensor will be computed automatically
	//nxBodyDesc.mass = 0.37f;
	//nxBodyDesc.linearDamping = 1.0f;
	//nxBodyDesc.angularDamping = 0.0f;
	//nxBodyDesc.flags = NX_BF_VISUALIZATION;    
	//nxBodyDesc.solverIterationCount = 16;

	//NxCapsuleShapeDesc nxFreefallDesc;
	//nxFreefallDesc.setToDefault();
	//nxFreefallDesc.radius = _gearRecord->scale * 0.65f;
	//nxFreefallDesc.height = _gearRecord->scale * 0.65f;
	//
	//Matrix4f localPose( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );
	//localPose = Gameplay::iEngine->rotateMatrix( localPose, Vector3f( 1,0,0 ), 90.0f );
	//nxFreefallDesc.localPose = wrap( localPose );
	//nxFreefallDesc.materialIndex = _scene->getPhFleshMaterial()->getMaterialIndex();
	//
	//NxActorDesc nxActorDesc;
	//nxActorDesc.userData = this;
	//nxActorDesc.setToDefault();
	//nxActorDesc.shapes.pushBack( &nxFreefallDesc );
	//nxActorDesc.body = &nxBodyDesc;

	//NxActor *_nx = _scene->getPhScene()->createActor( nxActorDesc );
	//
	//_nx->wakeUp();
	//
	//_nx->setGlobalPose(_nxCanopy->getGlobalPose());
	//_nx->setLinearVelocity(_nxCanopy->getLinearVelocity());
	//engine::IClump* pilotTemplate = Gameplay::iGameplay->findClump( "Cord" );




 //   if( _sliderUp )
 //   {
 //       if( isOpened() ) 
 //       {
 //           _scene->getStage()->remove( _sliderClump );
 //           _scene->getStage()->remove( _sliderCordFL );
 //           _scene->getStage()->remove( _sliderCordFR );
 //           _scene->getStage()->remove( _sliderCordRL );
 //           _scene->getStage()->remove( _sliderCordRR );
 //       }
 //       _sliderClump->release();
 //       _sliderCordFL->release();
 //       _sliderCordFR->release();
 //       _sliderCordRL->release();
 //       _sliderCordRR->release();
	//	_sliderUp = false;
 //   }
 //   if( _cordBatch ) 
 //   {
 //       getScene()->getStage()->remove( _cordBatch );
 //       _cordBatch->release();
 //   }


 //   NxJoint* _phJoint;          // connection joint

 //   //_scene->getPhScene()->releaseJoint( *_phJoint );
 //   NxDistanceJointDesc jointDesc;
 //   jointDesc.actor[0] = _nx;
	//jointDesc.actor[1] = _nxCanopy;
 //   jointDesc.maxDistance = 1.0f;
 //   jointDesc.minDistance = 1.0f;
 //   jointDesc.flags = NX_DJF_MAX_DISTANCE_ENABLED | NX_DJF_MIN_DISTANCE_ENABLED;      
 //   jointDesc.localAnchor[0] = NxVec3(0,0,0);
 //   jointDesc.localAnchor[1] = NxVec3(0,0,0);
 //   jointDesc.jointFlags = NX_JF_VISUALIZATION;
	//jointDesc.spring = false;
 //   _phJoint = _scene->getPhScene()->createJoint( jointDesc ); 


	//this->connect(_nx, 
	//	NxVec3(0,0,0),
	//	NxVec3(0,0,0),
	//	NxVec3(0,0,0),
	//	NxVec3(0,0,0),
	//	NULL,
	//	NULL,
	//	NULL,
	//	NULL);

//	_scene->getStage()->remove( _cordClump );


	if( _leftBrake ) {
		// release
		delete _leftBrake; _leftBrake = NULL;
		if( _rightBrake ) { delete _rightBrake; _rightBrake = NULL; }

		for( unsigned int i=0; i<_numCords; i++ ) if( _cords[i] ) { delete _cords[i]; _cords[i] = NULL; }
		delete[] _cords;
		_cords = NULL;
		this->isCutAway = true;
	}
//return;
//
//		// create new actor
//		NxBodyDesc nxBodyDesc;
//		nxBodyDesc.massSpaceInertia.set( 0,0,0 ); // tensor will be computed automatically
//		nxBodyDesc.mass = 10.00f;
//		nxBodyDesc.linearDamping = 1.0f;
//		nxBodyDesc.angularDamping = 0.0f;
//		nxBodyDesc.flags = NX_BF_VISUALIZATION;    
//		nxBodyDesc.solverIterationCount = 16;
//
//		NxCapsuleShapeDesc nxFreefallDesc;
//		nxFreefallDesc.setToDefault();
//		nxFreefallDesc.radius = _gearRecord->square * 0.65f;
//		nxFreefallDesc.height = _gearRecord->square * 0.65f;
//		
//		Matrix4f localPose( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );
//		localPose = Gameplay::iEngine->rotateMatrix( localPose, Vector3f( 1,0,0 ), 90.0f );
//		nxFreefallDesc.localPose = wrap( localPose );
//		nxFreefallDesc.materialIndex = _scene->getPhFleshMaterial()->getMaterialIndex();
//		
//		NxActorDesc nxActorDesc;
//		nxActorDesc.userData = this;
//		//nxActorDesc.setToDefault();
//		nxActorDesc.shapes.pushBack( &nxFreefallDesc );
//		nxActorDesc.body = &nxBodyDesc;
//		if (nxActorDesc.isValid()) {
//			getCore()->logMessage( "VALID" );
//		} else {
//			getCore()->logMessage( "INVALID" );
//		}
//
//		NxActor *_nx = _scene->getPhScene()->createActor( nxActorDesc );
//		_nx->setGlobalPose(_nxCanopy->getGlobalPose());
//		_nx->setLinearVelocity(_nxCanopy->getLinearVelocity());
//		_nx->wakeUp();
//
//
//		this->connect(_nx, NxVec3(0,0,0), NxVec3(0,0,0), NxVec3(0,0,0), NxVec3(0,0,0),
//			flr,
//			frr,
//			rlr,
//			rrr
//		);
///		_scene->getStage()->remove( _cordClump );
//	}
}



void CanopySimulator::connect(
    NxActor* actor,
    NxVec3 fla, NxVec3 fra, NxVec3 rla, NxVec3 rra,
    engine::IFrame* flr, engine::IFrame* frr, engine::IFrame* rlr, engine::IFrame* rrr
)
{
    assert( actor );
    //assert( flr ); assert( frr );
    //assert( rlr ); assert( rrr );
    _nxConnected = actor;
    _frontLeftAnchor[0] = fla;
    _frontRightAnchor[0] = fra;
    _rearLeftAnchor[0] = rla;
    _rearRightAnchor[0] = rra;
    _frontLeftRiser = flr;
    _frontRightRiser = frr;
    _rearLeftRiser = rlr;
    _rearRightRiser = rrr;

    Jumper* jumper = dynamic_cast<Jumper*>( _parent ); assert( jumper );

    // calculate number of cord instances
    unsigned int numCordInstances;
    numCordInstances = _gearRecord->riserScheme->getNumCords() * 4 * 3 +
                       ( _gearRecord->riserScheme->getNumBrakes() + 1 ) * 2;

    // obtain cord template
    engine::IClump* cordTemplate = Gameplay::iGameplay->findClump( "Cord" ); assert( cordTemplate );
    callback::AtomicL atomics;
    cordTemplate->forAllAtomics( callback::enumerateAtomics, &atomics );
    assert( atomics.size() == 1 );

    // build batch scheme
    engine::BatchScheme cordBatchScheme;
    cordBatchScheme.numLods = 1;
    cordBatchScheme.lodGeometry[0] = (*atomics.begin())->getGeometry();
    cordBatchScheme.lodDistance[0] = 1000000;
    assert( cordBatchScheme.isValid() );

    // create cord batch
    _cordBatch = Gameplay::iEngine->createBatch( numCordInstances, &cordBatchScheme );
    assert( _cordBatch );
    getScene()->getStage()->add( _cordBatch );

    // create cords
    float cascade = _gearRecord->cascade * 100.0f;
    unsigned int cordId = 0;
    unsigned int instanceId = 0;
    engine::IFrame* innerJoint;
    engine::IFrame* outerJoint;

	// create cords if needed
	if (_cords == NULL) {
		_cords = new CordSimulator*[_numCords];
		for( unsigned int i=0; i<_numCords; i++ ) _cords[i] = NULL;
		_inflation = 0.2f;
	}

	unsigned int i;
	if (_frontLeftRiser) {
		for( i=0; i<_gearRecord->riserScheme->getNumCords(); i++ )
		{
			innerJoint = Gameplay::iEngine->findFrame( _canopyClump->getFrame(), _gearRecord->riserScheme->getJointName( database::RiserScheme::rtInnerFrontLeft, i ) );
			outerJoint = Gameplay::iEngine->findFrame( _canopyClump->getFrame(), _gearRecord->riserScheme->getJointName( database::RiserScheme::rtOuterFrontLeft, i ) );
			_cords[cordId] = new CordSimulator( cascade, _sliderUp ? getSliderJointFrontLeft( _sliderClump ) : _frontLeftRiser, innerJoint, outerJoint, _cordBatch, instanceId );
			cordId++;
			innerJoint = Gameplay::iEngine->findFrame( _canopyClump->getFrame(), _gearRecord->riserScheme->getJointName( database::RiserScheme::rtInnerFrontRight, i ) );
			outerJoint = Gameplay::iEngine->findFrame( _canopyClump->getFrame(), _gearRecord->riserScheme->getJointName( database::RiserScheme::rtOuterFrontRight, i ) );
			_cords[cordId] = new CordSimulator( cascade, _sliderUp ? getSliderJointFrontRight( _sliderClump ) : _frontRightRiser, innerJoint, outerJoint, _cordBatch, instanceId );
			cordId++;
			innerJoint = Gameplay::iEngine->findFrame( _canopyClump->getFrame(), _gearRecord->riserScheme->getJointName( database::RiserScheme::rtInnerRearLeft, i ) );
			outerJoint = Gameplay::iEngine->findFrame( _canopyClump->getFrame(), _gearRecord->riserScheme->getJointName( database::RiserScheme::rtOuterRearLeft, i ) );
			_cords[cordId] = new CordSimulator( cascade, _sliderUp ? getSliderJointRearLeft( _sliderClump ) : _rearLeftRiser, innerJoint, outerJoint, _cordBatch, instanceId );
			cordId++;
			innerJoint = Gameplay::iEngine->findFrame( _canopyClump->getFrame(), _gearRecord->riserScheme->getJointName( database::RiserScheme::rtInnerRearRight, i ) );
			outerJoint = Gameplay::iEngine->findFrame( _canopyClump->getFrame(), _gearRecord->riserScheme->getJointName( database::RiserScheme::rtOuterRearRight, i ) );
			_cords[cordId] = new CordSimulator( cascade, _sliderUp ? getSliderJointRearRight( _sliderClump ) : _rearRightRiser, innerJoint, outerJoint, _cordBatch, instanceId );
			cordId++;
		}
	}

    // create brakes
    float brakeAspect = 0.85f;
    engine::IFrame** canopyJoints = new engine::IFrame*[_gearRecord->riserScheme->getNumBrakes()];
    for( i=0; i<_gearRecord->riserScheme->getNumBrakes(); i++ )
    {
        canopyJoints[i] = Gameplay::iEngine->findFrame( _canopyClump->getFrame(), _gearRecord->riserScheme->getJointName( database::RiserScheme::rtBrakeLeft, i ) );
    }
    _leftBrake = new BrakeSimulator( brakeAspect, _gearRecord->riserScheme->getNumBrakes(), _sliderUp ? getSliderJointRearLeft( _sliderClump ) : _rearLeftRiser, canopyJoints, _cordBatch, instanceId );
    for( i=0; i<_gearRecord->riserScheme->getNumBrakes(); i++ )
    {
        canopyJoints[i] = Gameplay::iEngine->findFrame( _canopyClump->getFrame(), _gearRecord->riserScheme->getJointName( database::RiserScheme::rtBrakeRight, i ) );
    }
    _rightBrake = new BrakeSimulator( brakeAspect, _gearRecord->riserScheme->getNumBrakes(), _sliderUp ? getSliderJointRearRight( _sliderClump ) : _rearRightRiser, canopyJoints, _cordBatch, instanceId );
    delete[] canopyJoints;

	
}

#define MAX(X,Y) ( X > Y ? X : Y )

void CanopySimulator::open(const NxMat34& pose, const NxVec3& velocity, float leftLOW, float rightLOW, float linetwists)
{
    assert( isConnected() );

    Jumper* jumper = dynamic_cast<Jumper*>( _parent );

    // setup burden calculation
    _bcPrevVel = velocity;

    // setup lineovers
    _leftLOW = leftLOW;
	
    _rightLOW = rightLOW;
    // setup lineover flag
    if( _leftLOW > 0 || _rightLOW > 0 ) _lineoverIsEliminated = false;
    // setup WLO effectiviness flag
    if( jumper )
    {        
        float dice = getCore()->getRandToolkit()->getUniform( 0, 1 );
        _wloIsEffective = ( dice < jumper->getVirtues()->getRiggingSkill() );
        // second chance! ( +50% of base probability )
        if( !_wloIsEffective )
        {
            dice = getCore()->getRandToolkit()->getUniform( 0, 1 );
            _wloIsEffective = ( dice < jumper->getVirtues()->getRiggingSkill() );
        }
    }

    // disable lineover if malfunctions disabled
    if( jumper && !jumper->getVirtues()->equipment.malfunctions )
    {
        _leftLOW = 0.0f;
        _rightLOW = 0.0f;
    }

    // disable lineover is canopy is skydiving
    if( _gearRecord->skydiving )
    {
        _leftLOW = 0.0f;
        _rightLOW = 0.0f;
    }

    // setup linetwists
    _linetwists = linetwists;
    // disable linetwists if malfunctions disabled
    if( jumper && !jumper->getVirtues()->equipment.malfunctions )
    {
        _linetwists = 0.0f;
    }

    if( jumper && jumper->isPlayer() )
    {
        Gameplay::iGui->getDesktop()->insertPanel( _signature->getPanel() );
    }

    _scene->getStage()->add( _canopyClump );
    if( _sliderUp )
    {
				// scale model
		Vector3f scale = ::calcScale( _sliderClump->getFrame()->getMatrix() );
		scale *= 0.3f;

		_sliderClump->getFrame()->setMatrix( Matrix4f( 
			scale[0],        0,        0, 0,
				0, scale[1],        0, 0,
				0,        0, scale[2], 0,
				0,        0,        0, 1
		) );
        _scene->getStage()->add( _sliderClump );
        _scene->getStage()->add( _sliderCordFL );
        _scene->getStage()->add( _sliderCordFR );
        _scene->getStage()->add( _sliderCordRL );
        _scene->getStage()->add( _sliderCordRR );
    }
    
    // initialize canopy physics simulator
    NxBodyDesc nxBodyDesc;
    nxBodyDesc.massSpaceInertia.set( 0,0,0 ); // tensor will be computed automatically
    nxBodyDesc.mass = _gearRecord->mass;
    nxBodyDesc.linearDamping = 0.0f;
    nxBodyDesc.angularDamping = 0.0f;
    nxBodyDesc.flags = NX_BF_VISUALIZATION;
    nxBodyDesc.solverIterationCount = 32;
    Vector3f aabbScale(
        getCollisionGeometry( _canopyClump )->getFrame()->getRight().length(),
        getCollisionGeometry( _canopyClump )->getFrame()->getUp().length(),
        getCollisionGeometry( _canopyClump )->getFrame()->getAt().length()
    );
    Vector3f aabbInf = getCollisionGeometry( _canopyClump )->getGeometry()->getAABBInf();
    Vector3f aabbSup = getCollisionGeometry( _canopyClump )->getGeometry()->getAABBSup();
    Vector3f aabbDim = ( aabbSup - aabbInf );
    aabbDim[0] *= aabbScale[0] * 0.5f,
    aabbDim[1] *= aabbScale[1] * 0.5f,
    aabbDim[2] *= aabbScale[2] * 0.5f;
    NxBoxShapeDesc nxBoxDesc;
    nxBoxDesc.dimensions = wrap( aabbDim );
    nxBoxDesc.materialIndex = _scene->getPhClothMaterial()->getMaterialIndex();
    NxActorDesc nxActorDesc;
    nxActorDesc.userData = this;
    nxActorDesc.shapes.pushBack( &nxBoxDesc );
    nxActorDesc.body = &nxBodyDesc;        
    nxActorDesc.globalPose = pose;
    _nxCanopy = _scene->getPhScene()->createActor( nxActorDesc );
    assert( _nxCanopy );
    unsigned int flags = _scene->getPhScene()->getActorPairFlags( *_scene->getPhTerrain(), *_nxCanopy );
    flags = flags | NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH;
    _scene->getPhScene()->setActorPairFlags( *_scene->getPhTerrain(), *_nxCanopy, flags );

    // disable collision btw. canopy and base jumper
    flags = _scene->getPhScene()->getActorPairFlags( *_nxCanopy, *jumper->getFlightActor() );
    flags = flags | NX_IGNORE_PAIR;
    _scene->getPhScene()->setActorPairFlags( *_nxCanopy, *jumper->getFlightActor(), flags );

    // initialize velocity
    _nxCanopy->addForce( velocity, NX_VELOCITY_CHANGE );
    
    // initialize PTV transformation
    Matrix4f viewLTM = _canopyClump->getFrame()->getLTM();
    Matrix4f canopyLTM = getCollisionGeometry( _canopyClump )->getFrame()->getLTM();
    // this is due to max pivot placement specific
    canopyLTM[3][2] += aabbDim[2];
    _mcCanopy.setup( canopyLTM, viewLTM );

    // synchronize physics & rendering structures to achieve valid data for joint
    _canopyClump->getFrame()->setMatrix( _mcCanopy.convert( wrap( _nxCanopy->getGlobalPose() ) ) );
    _canopyClump->getFrame()->getLTM();

    // initialize rough joints
    float rjMultiplier = 1.125f;
    NxDistanceJointDesc jointDesc;
    jointDesc.actor[0]   = _nxConnected;
    jointDesc.actor[1]   = _nxCanopy;    
    jointDesc.flags      = NX_DJF_MAX_DISTANCE_ENABLED;
    jointDesc.jointFlags = NX_JF_VISUALIZATION | NX_JF_COLLISION_ENABLED;

    jointDesc.localAnchor[0] = _frontLeftAnchor[0];
    jointDesc.localAnchor[1] = _frontLeftAnchor[1];
    jointDesc.maxDistance    = _gearRecord->frontCord * rjMultiplier;
    jointDesc.minDistance    = 0.0f;
    _roughJoints[0] = _scene->getPhScene()->createJoint( jointDesc );
    assert( _roughJoints[0] );

    jointDesc.localAnchor[0] = _frontRightAnchor[0];
    jointDesc.localAnchor[1] = _frontRightAnchor[1];
    jointDesc.maxDistance    = _gearRecord->frontCord * rjMultiplier;
    jointDesc.minDistance    = 0.0f;
    _roughJoints[1] = _scene->getPhScene()->createJoint( jointDesc );
    assert( _roughJoints[1] );

    jointDesc.localAnchor[0] = _rearLeftAnchor[0];
    jointDesc.localAnchor[1] = _rearLeftAnchor[1];
    jointDesc.maxDistance    = _gearRecord->rearCord * rjMultiplier;
    jointDesc.minDistance    = 0.0f;
    _roughJoints[2] = _scene->getPhScene()->createJoint( jointDesc );
    assert( _roughJoints[2] );

    jointDesc.localAnchor[0] = _rearRightAnchor[0];
    jointDesc.localAnchor[1] = _rearRightAnchor[1];
    jointDesc.maxDistance    = _gearRecord->rearCord * rjMultiplier;
    jointDesc.minDistance    = 0.0f;
    _roughJoints[3] = _scene->getPhScene()->createJoint( jointDesc );
    assert( _roughJoints[3] );

    // initialize ropes
    unsigned int ropeJoints = jumper->isPlayer() ? 5 : 2;
    float        ropeMass = 0.5f;
    _frontLeftRope  = new Rope( ropeJoints, ropeMass, _gearRecord->frontCord, _scene->getPhScene() );
    _frontRightRope = new Rope( ropeJoints, ropeMass, _gearRecord->frontCord, _scene->getPhScene() );
    _rearLeftRope  = new Rope( ropeJoints, ropeMass, _gearRecord->rearCord, _scene->getPhScene() );
    _rearRightRope = new Rope( ropeJoints, ropeMass, _gearRecord->rearCord, _scene->getPhScene() );
    _frontLeftRope->initialize( _nxConnected, _frontLeftAnchor[0], _nxCanopy, _frontLeftAnchor[1] );
    _frontRightRope->initialize( _nxConnected, _frontRightAnchor[0], _nxCanopy, _frontRightAnchor[1] );
    _rearLeftRope->initialize( _nxConnected, _rearLeftAnchor[0], _nxCanopy, _rearLeftAnchor[1] );
    _rearRightRope->initialize( _nxConnected, _rearRightAnchor[0], _nxCanopy, _rearRightAnchor[1] );
}

void CanopySimulator::reset(void)
{
    _leftDeep = _rightDeep = 0.0f;
    _leftWarpDeep = _rightWarpDeep = 0.0f;
}

void CanopySimulator::setLeftDeep(float value)
{
	if (getLeftForcedDeep() != -1.0f) {
		_leftDeep = getLeftForcedDeep();
	} else {
		_leftDeep = value;
	}
}

void CanopySimulator::setRightDeep(float value)
{
	if (getRightForcedDeep() != -1.0f) {
		_rightDeep = getRightForcedDeep();
	} else {
		_rightDeep = value;
	}
}
float CanopySimulator::getLeftForcedDeep(void) {
	if (_leftStowed && _leftForcedDeep < BRAKE_STOW_LEVEL) return BRAKE_STOW_LEVEL;
	return _leftForcedDeep;
}
float CanopySimulator::getRightForcedDeep(void) {
	if (_rightStowed && _rightForcedDeep < BRAKE_STOW_LEVEL) return BRAKE_STOW_LEVEL;
	return _rightForcedDeep;
}
void CanopySimulator::setLeftForcedDeep(float value) {
	_leftForcedDeep = value;
}
void CanopySimulator::setRightForcedDeep(float value) {
	_rightForcedDeep = value;
}


// brake stowing
void CanopySimulator::setLeftStowed(bool trigger) {
	_leftStowed = trigger;
}
void CanopySimulator::setRightStowed(bool trigger) {
	_rightStowed = trigger;
}
bool CanopySimulator::getLeftStowed(void) {
	return _leftStowed;
}
bool CanopySimulator::getRightStowed(void) {
	return _rightStowed;
}

// riser control
void CanopySimulator::setLeftWarpDeep(float value)
{
    _leftWarpDeep = value;
}

void CanopySimulator::setRightWarpDeep(float value)
{
    _rightWarpDeep = value;
}
void CanopySimulator::setLeftRearRiser(float value)
{
    _leftRearRiser = value;
}

void CanopySimulator::setRightRearRiser(float value)
{
    _rightRearRiser = value;
}
void CanopySimulator::setWLOToggles(bool trigger)
{
    if( _wloToggles ) _wloToggles->setTrigger( trigger );
}

void CanopySimulator::setHookKnife(bool trigger)
{
    if( _hookKnife ) _hookKnife->setTrigger( trigger );
}

void CanopySimulator::enableWind(bool flag)
{
    _enableWind = flag;
}

void CanopySimulator::renderCollapseAreas(void)
{
    if( !_nxCanopy ) return;
    Vector3f center;
    Matrix4f canopyLTM = wrap( _nxCanopy->getGlobalPose() );
    for( unsigned int i=0; i<_gearRecord->Cnum; i++ )
    {
        if( _collapseAreas[i].radius > 0.0f )
        {
            center = Gameplay::iEngine->transformCoord( wrap( _collapseAreas[i].center ), canopyLTM );
            Gameplay::iEngine->renderSphere( center, 100*_collapseAreas[i].radius, Vector4f( 1,1,1,1 ) );
        }
    }
}
  
/**
 * Actor
 */

float getBrakingFactor(float i)
{
    i = i < 0 ? 0 : ( i > 1 ? 1 : i );
    return pow( 1 - i, 3 );
}

void CanopySimulator::onUpdateActivity(float dt)
{
	if (this->isCutAway) {
		//this->disconnect();
	}

    _mBlinkTime += dt;

    if( isOpened() )
    {
        // sound processing
        if( !_flightSoundEnabled && _inflation > 0.025f )
        {
            happen( this, EVENT_CANOPY_OPEN );
            _flightSoundEnabled = true;
        }
        float vel = _nxCanopy->getLinearVelocity().magnitude();
        happen( this, EVENT_CANOPY_VELOCITY, &vel );

        // animate canopy, and synchronize canopy simulator & canopy model
        float animTime = animStartTime * ( 1.0f - _inflation ) + animEndTime * _inflation;
		//if (_inflation <= 0.26f) {
			//animTime = powf(animTime, 1.6487212707f);
		//}
        _canopyClump->getAnimationController()->resetTrackTime( 0 );
        _canopyClump->getAnimationController()->advance( animTime );
        _canopyClump->getFrame()->setMatrix( _mcCanopy.convert( wrap( _nxCanopy->getGlobalPose() ) ) );
        _canopyClump->getFrame()->getLTM();
        
        // specific behaviour
        updateWarp( dt );
        updateSlider( dt );
        updateProceduralAnimation( dt );
        _canopyClump->getFrame()->getLTM();

        // place cords
		if (!isCutAway) {
			for( unsigned int i=0; i<_numCords; i++ ) _cords[i]->update( dt );
			if (_leftBrake) _leftBrake->update( dt );
			if (_rightBrake) _rightBrake->update( dt );
		}

        if( _sliderUp )
        {
            Jumper::placeCord( _sliderCordFL, _frontLeftRiser->getPos(), getSliderJointFrontLeft( _sliderClump )->getPos(), 2.0f );
            Jumper::placeCord( _sliderCordFR, _frontRightRiser->getPos(), getSliderJointFrontRight( _sliderClump )->getPos(), 2.0f );
            Jumper::placeCord( _sliderCordRL, _rearLeftRiser->getPos(), getSliderJointRearLeft( _sliderClump )->getPos(), 2.0f );
            Jumper::placeCord( _sliderCordRR, _rearRightRiser->getPos(), getSliderJointRearRight( _sliderClump )->getPos(), 2.0f );
        }
    }
}

static NxVec3 getResistanceForce(const NxVec3& normal, const NxVec3& vel, float K)
{
    float normalVel = normal.dot( vel );
    normalVel = ( normalVel < 0 ) ? 0 : normalVel;
    return -normal * K * sqr( normalVel );
}

static float getLiftPower(float angle)
{
    // 30'
    return pow( ( ( 5000.0f - sqr( 30.0f - angle ) ) / 5000.0f ), 9 );
}

static float getDragPower(float angle)
{
    return 0.1f + 1.0f / 1000.0f * pow( angle, 1.9f );
}

static float getWingPower(float inflation)
{
	if (inflation <= 0.3f) return 0.1f;
	return inflation * inflation;

	return pow( inflation, 3 );
}

void CanopySimulator::entangle(const NxVec3& cohesionPoint)
{
    _cohesionState = true;
    _cohesionPoint = cohesionPoint;
}

//float max(float a, float b) {
//	return a > b ? a : b;
//}
void CanopySimulator::onUpdatePhysics(void)
{
    if( !isOpened() ) return;
	if( !isConnected() ) return;


	// control override
	// set forced brake settings
	if (getLeftForcedDeep() != -1.0f) {
		setLeftDeep(getLeftForcedDeep());
	}
	if (getRightForcedDeep() != -1.0f) {
		setRightDeep(getRightForcedDeep());
	}

    // determine brake deep taking into account canopy mode
    float modeLeftDeep  = _leftDeep;
    float modeRightDeep = _rightDeep;
    float modeLeftFrontRiser  = _leftWarpDeep;
    float modeRightFrontRiser = _rightWarpDeep;
    float modeLeftRearRiser  = _leftRearRiser;
    float modeRightRearRiser = _rightRearRiser;

	// don't control the canopy while in a linetwist
	if (_linetwists != 0.0f) {
		modeLeftDeep = 0.0f;
		modeRightDeep = 0.0f;
		modeLeftFrontRiser = 0.0f;
		modeRightFrontRiser = 0.0f;
		modeLeftRearRiser = 0.0f;
		modeRightRearRiser = 0.0f;
	}

    // collapse
    updateCollapse( ::simulationStepTime );

    // simulate canopy inflation
    updateInflation();

    // simulate canopy cohesion
    if( !isCutAway && _cohesionState && !_cohesionJoint )
    {
        NxDistanceJointDesc cohesionDesc;
        cohesionDesc.actor[0] = NULL;
        cohesionDesc.actor[1] = _nxCanopy;
        cohesionDesc.setGlobalAnchor( _cohesionPoint );
        cohesionDesc.flags = NX_DJF_MAX_DISTANCE_ENABLED;
        cohesionDesc.maxDistance = 0.25f;
        assert( cohesionDesc.isValid() );
        _cohesionJoint = _scene->getPhScene()->createJoint( cohesionDesc );
        assert( _cohesionJoint );
    }

    // simulate rigging gear
    Jumper* jumper = dynamic_cast<Jumper*>( _parent );
    if( jumper && _wloToggles ) _wloToggles->simulate( ::simulationStepTime, jumper->getVirtues() );
    if( jumper && _hookKnife ) _hookKnife->simulate( ::simulationStepTime, jumper->getVirtues() );

    // simulate lineover removal
    if( !_lineoverIsEliminated )
    {
        // remove by WLO
        if( _wloIsEffective && _wloToggles && _wloToggles->getWeight() == 1 )
        {
			// remove brake toggles after WLO use
			if (_leftLOW > _rightLOW) {
				delete _leftBrake;
				_leftBrake = NULL;
				setLeftForcedDeep(0.0f);
				_leftDeep = 0.0f;
			} else {
				delete _rightBrake;
				_rightBrake = NULL;
				setRightForcedDeep(0.0f);
				_rightDeep = 0.0f;
			}
            _lineoverIsEliminated = true;
			
        }
        if( _hookKnife && _hookKnife->getWeight() == 1 )
        {
            _lineoverIsEliminated = true;
        }
    }
    else if( _leftLOW > 0 || _rightLOW > 0 )
    {
        float removalSpeed = 0.25f;
        _leftLOW -= removalSpeed * ::simulationStepTime;
        if( _leftLOW < 0 ) _leftLOW = 0;
        _rightLOW -= removalSpeed * ::simulationStepTime;
        if( _rightLOW < 0 ) _rightLOW = 0;
    }
    
    // velocity of canopy
    NxVec3 velocity = _nxCanopy->getLinearVelocity();       
    if( _enableWind )
    {
        // wind velocity
        NxVec3 wind = _scene->getWindAtPoint( _nxCanopy->getGlobalPose().t );
        // final canopy velocity
        velocity += wind * windfluence;
    }
    // normalized velocity
    NxVec3 velocityN = velocity; velocityN.normalize();

    // calculate overburden
    _bcStep++;
    if( _bcStep >= 10 )
    {
        _bcBurden.set( 0, -9.8f, 0 );
        _bcBurden -= ( velocity - _bcPrevVel ) / ( float( _bcStep ) * ::simulationStepTime );
        _bcPrevVel = velocity;
        _bcStep = 0;
    }

    // simulate canopy ageing    
    float overburden = _bcBurden.magnitude() / 9.8f;
    // damage canopy
    float factor = ( overburden - _gearRecord->Kminoverb ) / ( _gearRecord->Kmaxoverb - _gearRecord->Kminoverb );
    factor = factor < 0 ? 0 : ( factor > 1 ? 1 : factor );
    float damage = _gearRecord->Kmindamage * ( 1 - factor ) + _gearRecord->Kmaxdamage * factor;
    damage *= ::simulationStepTime;
    _gear->state -= damage;
    if( _gear->state < 0.1f ) _gear->state = 0.1f;

    // local coordinate system of canopy
    NxMat34 pose = _nxCanopy->getGlobalPose();
    NxVec3 cx = pose.M.getColumn(0);
    NxVec3 cy = pose.M.getColumn(1);
    NxVec3 cz = pose.M.getColumn(2);
	
    // 3dmax conversion
    NxVec3 z = -cy;
    NxVec3 y = cz;
    NxVec3 x = cx;

    // dynamic perfomance, as a function of gear state
    float perfomance = sqrt( sqrt( _gear->state ) );

    // air resistance force
    float Kzaird = _gearRecord->Kzair * _inflation * perfomance + _gearRecord->Kyair * ( 1.0f - _inflation ) * perfomance;
    NxVec3 Fair = _inflation * getResistanceForce( -y, velocity, _gearRecord->Kyair * perfomance ) +
                  _inflation * getResistanceForce( y, velocity, _gearRecord->Kyair * perfomance ) +
                  _inflation * getResistanceForce( z, velocity, Kzaird * perfomance ) +
                  _inflation * getResistanceForce( -z, velocity, Kzaird * perfomance ) +
                  _inflation * getResistanceForce( x, velocity, _gearRecord->Kxair * perfomance ) +
                  _inflation * getResistanceForce( -x, velocity, _gearRecord->Kxair * perfomance );

    // attack angle
    float attackAngle = -calcAngle( z, velocityN, x );
    if( velocityN.magnitude() == 0.0f ) attackAngle = 0.0f;
	
	bool stall = false;
	if( attackAngle < -5.0f) {
		stall = true;
		//_inflation -= 0.8f * jumper->getDeltaTime(); 
		//if (_inflation <= 0.1f) _inflation = 0.1f;
	}
	if( attackAngle < 0 ) { 
		attackAngle = 0; 
	}
    if( attackAngle > 90 ) attackAngle = 90;

    // average deep of brakes affects the lift & drag force and also attack angle
	float avgDeep = NxMath::max(modeLeftDeep * modeRightDeep, modeLeftRearRiser * modeRightRearRiser * 0.8f);
	avgDeep -= (modeLeftFrontRiser * modeRightFrontRiser * 0.5f);
    attackAngle += _gearRecord->AAdeep * avgDeep;

	if( attackAngle <= 0.0f ) {
        attackAngle = 1.0f;
    }

    // wing function
    float WF = getWingPower( _inflation );
    WF = _inflation;

    // linetwists will reduces wing function
    float wfLoss = fabs( _linetwists ) / 90.0f;
    if( wfLoss > 1 ) wfLoss = 1.0f;
    WF *= ( 1 - wfLoss );

    // lineover will also reduces wing function
    wfLoss = 0.5f * ( _leftLOW + _rightLOW );
    if( wfLoss > 1 ) wfLoss = 1.0f;
    WF *= ( 1 - wfLoss );    

    // lift force
    float Klift = _gearRecord->Klifts * perfomance * ( 1.0f - avgDeep ) + _gearRecord->Kliftd * avgDeep * perfomance;
    NxVec3 Nlift;
    Nlift.cross( velocityN, x );
    Nlift.normalize();
    NxVec3 Flift = Nlift * WF * Klift * getLiftPower( attackAngle ) * sqr( velocity.magnitude() );

    // drag force
    float Kdrag = _gearRecord->Kdrags * perfomance * ( 1.0f - avgDeep ) + _gearRecord->Kdragd * avgDeep * perfomance;
    NxVec3 Ndrag = -velocityN;
    NxVec3 Fdrag = Ndrag * WF * Kdrag * getDragPower( attackAngle ) * sqr( velocity.magnitude() );

    // control force
    NxVec3 leftPoint = wrap( CanopySimulator::getPhysicsJointRearLeft( _canopyClump )->getPos() );
    NxVec3 rightPoint = wrap( CanopySimulator::getPhysicsJointRearRight( _canopyClump )->getPos() );
    NxVec3 leftPointVel = _nxCanopy->getPointVelocity( leftPoint );
    NxVec3 rightPointVel = _nxCanopy->getPointVelocity( rightPoint );

    // wind simulation
    if( _enableWind )
    {
        // include wind velocity
        leftPointVel  += _scene->getWindAtPoint( leftPoint ) * windfluence;
        rightPointVel += _scene->getWindAtPoint( rightPointVel ) * windfluence;
    }   

    float leftPointNormalVel = z.dot( leftPointVel );
    float rightPointNormalVel = z.dot( rightPointVel );
    leftPointNormalVel = leftPointNormalVel < 0 ? 0 : leftPointNormalVel;
    rightPointNormalVel = rightPointNormalVel < 0 ? 0 : rightPointNormalVel;

	// toggle input
    NxVec3 Fcl = -z * modeLeftDeep * WF * sqr(leftPointNormalVel) * _gearRecord->Kbraking * perfomance * 1.3f;
    NxVec3 Fcr = -z * modeRightDeep * WF * sqr(rightPointNormalVel) * _gearRecord->Kbraking * perfomance * 1.3f;
    //Fcl += -y * _gearRecord->Kturn * _nxConnected->getMass() * modeLeftDeep * perfomance;
    //Fcr += -y * _gearRecord->Kturn * _nxConnected->getMass() * modeRightDeep * perfomance;

    // angular damping is a function of canopy velocity
    float Idamp = velocity.magnitude() / _gearRecord->Vdampmax;
    Idamp = Idamp > 1.0f ? 1.0f : Idamp;
    float Kdamp = _gearRecord->Kdampmin * perfomance * ( 1.0f - Idamp ) + _gearRecord->Kdampmax * Idamp * perfomance;
    _nxCanopy->setAngularDamping( Kdamp*1.6f );

    // total unit force
    NxVec3 Funit = Fair + Flift + Fdrag;

    // finalize motion equation
    _nxCanopy->addForceAtPos( Fcl, leftPoint );
    _nxCanopy->addForceAtPos( Fcr, rightPoint );
    _nxCanopy->addForce( Funit );

	// Helicopter wake turbulance
	if (jumper->getAirplane() != NULL && strcmp(jumper->getAirplane()->getDesc()->templateClump->getName(), "Helicopter01") == 0) {

		NxVec3 pos = this->getActor()->getGlobalPosition();				// canopy position
		NxVec3 helipos = wrap( jumper->getAirplane()->getPosition() );	// heli position
		if (helipos.y >= pos.y) {	// if below heli
			float dstV = helipos.y - pos.y;	  // vertical distance
			helipos.y = pos.y = 0;			  
			if (dstV <= 1.0f) dstV = 1.0f;	 

			// wind moves turbulence
			NxVec3 wind = _scene->getWindAtPoint( _nxCanopy->getGlobalPose().t ) * windfluence;
			helipos -= wind * (dstV/50.0f);

			if (pos.distance(helipos) <= (10.0f + 6.0f/dstV)) {		// area of influence: diameter of 10.0m from the center of the craft + no more than 6m. depending on vertical distance (close to zero at 50m distance)
				// calculate force
				float F = -100000000.0f / (dstV*dstV);				
				F *= getCore()->getRandToolkit()->getUniform( 0.8f, 1.4f );

				// apply force to push the canopy down
				_nxCanopy->addForceAtPos( wrap(Vector3f(0, F, 0)), helipos);

				// give a little spin
				_nxCanopy->addForceAtLocalPos( wrap(Vector3f(0, 0, F*0.2f)), wrap(Vector3f(2.5f,0,0)));
				_nxCanopy->addForceAtLocalPos( wrap(Vector3f(0, 0, -F*0.2f)), wrap(Vector3f(-2.5f,0,0)));

				//getCore()->logMessage("dstV: %2.5f; dstH: %2.5f; infl: %2.2f F: %2.16f", dstV, pos.distance(helipos), _inflation, F);
				//getCore()->logMessage("rand: %2.5f", getCore()->getRandToolkit()->getUniform( 0.99999f, 1.0f ));

				// adjust inflation
				//_inflation *= getCore()->getRandToolkit()->getUniform( 0.9999f, 1.0001f );
			}
		}
	}

    // finalize collapse simulation
    float collapsePenalty = 0.0f;
    for( unsigned int i=0; i<_gearRecord->Cnum; i++ )
    {
        if( _collapseAreas[i].radius > 0 )
        {
            collapsePenalty += _collapseAreas[i].radius / _gearRecord->Cradius * _gearRecord->Cpower;
        }
    }
    if( collapsePenalty > 1.0f ) collapsePenalty = 1.0f;
    _nxCanopy->addForce( Funit * -collapsePenalty );

    // lineover or linetwist bracking force
	float malfunctionLeftForce = _leftLOW;
	float malfunctionRightForce = _rightLOW;

	if (_linetwists < 0.0f) {
		malfunctionLeftForce -= _linetwists / 6000.0f;
	} else if (_linetwists > 0.0f) {
		malfunctionRightForce += _linetwists / 6000.0f;
	}

    NxVec3 Fllob = -z * malfunctionLeftForce * sqr(leftPointNormalVel) * _gearRecord->Kbraking * getCore()->getRandToolkit()->getUniform( 5,10 );
    NxVec3 Frlob = -z * malfunctionRightForce * sqr(rightPointNormalVel) * _gearRecord->Kbraking * getCore()->getRandToolkit()->getUniform( 5,10 );
    _nxCanopy->addForceAtPos( Fllob, leftPoint );
    _nxCanopy->addForceAtPos( Frlob, rightPoint );

    // simulate risers
    //*
    {
        NxVec3 backLeftPoint = wrap( CanopySimulator::getPhysicsJointRearLeft( _canopyClump )->getPos() );
        NxVec3 backRightPoint = wrap( CanopySimulator::getPhysicsJointRearRight( _canopyClump )->getPos() );
        NxVec3 backLeftPointVel = _nxCanopy->getPointVelocity( backLeftPoint );
        NxVec3 backRightPointVel = _nxCanopy->getPointVelocity( backRightPoint );
        NxVec3 frontLeftPoint = wrap( CanopySimulator::getPhysicsJointFrontLeft( _canopyClump )->getPos() );
        NxVec3 frontRightPoint = wrap( CanopySimulator::getPhysicsJointFrontRight( _canopyClump )->getPos() );
        NxVec3 frontLeftPointVel = _nxCanopy->getPointVelocity( frontLeftPoint );
        NxVec3 frontRightPointVel = _nxCanopy->getPointVelocity( frontRightPoint );

        //if( _enableWind )
        //{
        //    // include wind velocity
        //    frontLeftPointVel  += _scene->getWindAtPoint( frontLeftPointVel ) * windfluence;
        //    frontRightPointVel += _scene->getWindAtPoint( frontRightPointVel ) * windfluence;
        //    backLeftPointVel  += _scene->getWindAtPoint( backLeftPointVel ) * windfluence;
        //    backRightPointVel += _scene->getWindAtPoint( backRightPointVel ) * windfluence;
        //}

        float backLeftPointNormalVel = z.dot( backLeftPointVel );
        float backRightPointNormalVel = z.dot( backRightPointVel );
        backLeftPointNormalVel = backLeftPointVel.magnitude();
        backRightPointNormalVel = backRightPointVel.magnitude();
        backLeftPointNormalVel = backLeftPointNormalVel < 0 ? 0 : backLeftPointNormalVel;
        backRightPointNormalVel = backRightPointNormalVel < 0 ? 0 : backRightPointNormalVel;

        float frontLeftPointNormalVel = z.dot( frontLeftPointVel );
        float frontRightPointNormalVel = z.dot( frontRightPointVel );
        frontLeftPointNormalVel = frontLeftPointNormalVel < 0 ? 0 : frontLeftPointNormalVel;
        frontRightPointNormalVel = frontRightPointNormalVel < 0 ? 0 : frontRightPointNormalVel;

        if (modeLeftRearRiser > 0.0f) {
            NxVec3 force = -y * modeLeftRearRiser * 50.0f * backLeftPointNormalVel * _gearRecord->Kbraking;
            _nxCanopy->addForceAtPos( force, backLeftPoint );
        }
        if (modeRightRearRiser > 0.0f) {
            NxVec3 force = -y * modeRightRearRiser * 50.0f * backRightPointNormalVel * _gearRecord->Kbraking;
            _nxCanopy->addForceAtPos( force, backRightPoint );
        }

		if (modeLeftFrontRiser > 0.0f && !stall) {
            NxVec3 force = -y * modeLeftFrontRiser * 7.0f * sqr(frontLeftPointNormalVel) * _gearRecord->Kbraking;
            _nxCanopy->addForceAtPos( force, frontLeftPoint );
        }
		if (modeRightFrontRiser > 0.0f && !stall) {
            NxVec3 force = -y * modeRightFrontRiser * 7.0f * sqr(frontRightPointNormalVel) * _gearRecord->Kbraking;
			//force += 0.4f * z * modeRightFrontRiser * 7.0f * sqr(frontRightPointNormalVel) * _gearRecord->Kbraking;
            _nxCanopy->addForceAtPos( force, frontRightPoint );
        }
    }
    //*/

    // down canopy nose at low speed
    /*float minSpeed  = 1.0f;
    float minTorque = 2 * jumper->getVirtues()->appearance.weight;
    float maxSpeed  = 5.5f;
    float maxTorque = 0.0f;
    factor = ( velocity.magnitude() - minSpeed ) / ( maxSpeed - minSpeed );
    factor = factor < 0 ? 0 : ( factor > 1 ? 1 : factor );    
    factor = factor * factor;
    float torque = minTorque * ( 1 - factor ) + maxTorque * factor;
    _nxCanopy->addLocalTorque( x * torque );*/
}

void CanopySimulator::updateInflation(void)
{
    // poses
    NxMat34 canopyPose = _nxCanopy->getGlobalPose();
    NxMat34 connectedPose = _nxConnected->getGlobalPose();

    // global connection points
    NxVec3 frontLeftAnchor[2];
    NxVec3 frontRightAnchor[2];
    NxVec3 rearLeftAnchor[2];
    NxVec3 rearRightAnchor[2];    
    frontLeftAnchor[0] = connectedPose * _frontLeftAnchor[0];
    frontLeftAnchor[1] = canopyPose * _frontLeftAnchor[1];
    frontRightAnchor[0] = connectedPose * _frontRightAnchor[0];
    frontRightAnchor[1] = canopyPose * _frontRightAnchor[1];
    rearLeftAnchor[0] = connectedPose * _rearLeftAnchor[0];
    rearLeftAnchor[1] = canopyPose * _rearLeftAnchor[1];
    rearRightAnchor[0] = connectedPose * _rearRightAnchor[0];
    rearRightAnchor[1] = canopyPose * _rearRightAnchor[1];    

    // determine average cord tension
    float averageTension = 0.0f;    
    float cordTension = ( frontLeftAnchor[0] - frontLeftAnchor[1] ).magnitude();
    averageTension += cordTension / _gearRecord->frontCord;
    cordTension = ( frontRightAnchor[0] - frontRightAnchor[1] ).magnitude();
    averageTension += cordTension / _gearRecord->frontCord;
    cordTension = ( rearLeftAnchor[0] - rearLeftAnchor[1] ).magnitude();
    averageTension += cordTension / _gearRecord->rearCord;
    cordTension = ( rearRightAnchor[0] - rearRightAnchor[1] ).magnitude();
    averageTension += cordTension / _gearRecord->rearCord;
    averageTension *= 0.25f;
    averageTension = ( averageTension > 1.0f ) ? 1.0f : averageTension;
    averageTension = sqr( sqr( averageTension ) );

	// left/right tension difference induces torque
	float leftTension, rightTension, LRtension;
	leftTension = ( frontLeftAnchor[0] - frontLeftAnchor[1] ).magnitude() / _gearRecord->frontCord;
	leftTension += ( rearLeftAnchor[0] - rearLeftAnchor[1] ).magnitude() / _gearRecord->rearCord;
	rightTension = ( frontRightAnchor[0] - frontRightAnchor[1] ).magnitude() / _gearRecord->frontCord;
	rightTension += ( rearRightAnchor[0] - rearRightAnchor[1] ).magnitude() / _gearRecord->rearCord;
	
	LRtension = leftTension - rightTension;
	float tensionDiff = (leftTension - rightTension);
	if (tensionDiff > 3.0f) tensionDiff = 3.0f;
	if (tensionDiff < -3.0f) tensionDiff = -3.0f;
	NxVec3 tensionTorque = NxVec3(0,1,0) * (leftTension - rightTension);
	
	Jumper* jumper = dynamic_cast<Jumper*>( _parent ); assert( jumper );
	if (!jumper->isLanding() && (leftTension - rightTension) > 0.0f && tensionTorque.magnitude() > 0.001f) {
		//_nxCanopy->addLocalTorque( tensionTorque * -30000.0f * (this->_gearRecord->square / 150.0f));
	}
	
	
	float sliderPos = (_sliderPosFL + _sliderPosRL + _sliderPosFR + _sliderPosRR) / 4.0f;
	NxVec3 canopySpeed =_nxCanopy->getLocalPointVelocity(NxVec3(0,0,0));
//	if( jumper->isPlayer() ) {
//		getCore()->logMessage( "tension (L/R):%2.10f", (leftTension - rightTension) );
//	}
	// debug
	if (jumper->_debug_window) {
		gui::IGuiPanel* panel = jumper->_debug_window->getPanel()->find( "Message" );
		assert( panel && panel->getStaticText() );
		panel->getStaticText()->setText( wstrformat(L"tension (L/R):%2.10f\nslider: %2.3f\nCanopy speed: %2.3f, %2.3f, %2.3f", (leftTension - rightTension), sliderPos, canopySpeed.x, canopySpeed.y, canopySpeed.z).c_str() );
	}

    // determine canopy orientation relative to motion direction
    NxVec3 motionDir = _nxCanopy->getLinearVelocity(); motionDir.normalize();
    NxVec3 canopyDown  = _nxCanopy->getGlobalPose().M.getColumn(2); canopyDown.normalize();
    canopyDown *= -1;
    float relativity = canopyDown.dot( motionDir );
    if( relativity < 0 ) averageTension = 0.0f;

    // determine opening factor
    NxVec3 canopyVel = _nxCanopy->getLinearVelocity();
    canopyVel += _scene->getWindAtPoint( _nxCanopy->getGlobalPose().t ) * windfluence;
    float factor;
    float openingK;
    if( _sliderUp )
    {        
        factor = ( canopyVel.magnitude() - _gearRecord->SUminvel ) / ( _gearRecord->SUmaxvel - _gearRecord->SUminvel );
        //factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );
        factor =  factor < 0 ? 0 : factor;
        openingK  = _gearRecord->SUmink * ( 1.0f - factor ) + _gearRecord->SUmaxk * factor;
    }
    else
    {
        factor = ( canopyVel.magnitude() - _gearRecord->SRDminvel ) / ( _gearRecord->SRDmaxvel - _gearRecord->SRDminvel );
        factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );
        openingK  = _gearRecord->SRDmink * ( 1.0f - factor ) + _gearRecord->SRDmaxk * factor;
    }

    // inflation by velocity
	if (_inflation >= 0.8f) {
		openingK *= 3.3333330f;
	} else if (_inflation >= 0.25f) {
		openingK *= 1.2121212f;
	} else if (_inflation >= 0.1f) {
		//openingK *= getCore()->getRandToolkit()->getUniform( 0.3f,0.9f );
		openingK *= 4.000000f;
	} else {
		openingK *= 0.6666660f;
	}



/*
3.3333330000
1.2121212000
4.0000000000
0.6666660000
*/

	// [0.80 1.00] = 2.0	0.20	= 0.400
	// [0.25 0.80) = 1.5	0.55	= 0.825
	// [0.10 0.25) = 0.9	0.15	= 0.135
	// [0.00 0.10) = 1.0	0.10	= 0.100

	if (sliderPos > 3.0f) openingK = 0.0f;
	//float canopySpeedScalar = canopySpeed.magnitude();
	//openingK = (0.5f + (canopySpeedScalar - 10.0f) / 10.0f) * _inflation;

    _inflation += averageTension * ::simulationStepTime * openingK;
	
    if( _inflation > 1 ) _inflation = 1;
	if( _inflation < 0.05f) _inflation = 0.05f;

    // enable collision generation for canopy and base jumper
    if( false && !_collideJumper && _inflation > 0.25f )
    {
        _collideJumper = true;
        Jumper* jumper = dynamic_cast<Jumper*>( _parent ); assert( jumper );        
        unsigned int flags = _scene->getPhScene()->getActorPairFlags( *_nxCanopy, *jumper->getFlightActor() );
        flags = flags & ~NX_IGNORE_PAIR;
        _scene->getPhScene()->setActorPairFlags( *_nxCanopy, *jumper->getFlightActor(), flags );
    }

    // remove rough joints
    //
    //if( _roughJoints[0] && _inflation > 0.99f )
    //{
    //    _scene->getPhScene()->releaseJoint( *_roughJoints[0] );
    //    _scene->getPhScene()->releaseJoint( *_roughJoints[1] );
    //    _scene->getPhScene()->releaseJoint( *_roughJoints[2] );
    //    _scene->getPhScene()->releaseJoint( *_roughJoints[3] );
    //    _roughJoints[0] = NULL;
    //    _roughJoints[1] = NULL;
    //    _roughJoints[2] = NULL;
    //    _roughJoints[3] = NULL;
    //}
   
}

void CanopySimulator::updateWarp(float dt)
{
	Jumper* jumper = dynamic_cast<Jumper*>( _parent ); assert( jumper );

    // harshness coefficient
    // prevents warping while inflation is incomplete
    float Kharsh = _inflation == 1.0f ? _inflation : pow( _inflation, 4 );

    // warp offsets
    float Kfront = 0.25f * Kharsh;
    float Krear  = 0.0625f * Kharsh;
    float Kangle = 45.0f * Kharsh;

    float leftDynamicWarp = _leftWarpDeep * 0;
    leftDynamicWarp = leftDynamicWarp > 1 ? 1 : leftDynamicWarp;
    float rightDynamicWarp = _rightWarpDeep * 0;
    rightDynamicWarp = rightDynamicWarp > 1 ? 1 : rightDynamicWarp;

    float twistFactor = fabs( _linetwists ) / 1440;
    twistFactor = twistFactor < 0 ? 0 : ( twistFactor > 1 ? 1 : twistFactor );
    float leftDynamicTwist = _linetwists < 0 ? twistFactor : 0;
    float rightDynamicTwist = _linetwists > 0 ? twistFactor : 0;

    NxVec3 frontLeftOffset( 0, -Kfront * leftDynamicWarp + Kfront * rightDynamicWarp, 0 );
    NxVec3 frontRightOffset( 0, -Kfront * rightDynamicWarp + Kfront * leftDynamicWarp, 0 );
    NxVec3 rearLeftOffset( 0, -Krear * leftDynamicWarp + Krear * rightDynamicWarp, 0 );
    NxVec3 rearRightOffset( 0, -Krear * rightDynamicWarp + Krear * leftDynamicWarp, 0 );

    Matrix4f lt( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0 );
    Matrix4f rt( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0 );

    lt = Gameplay::iEngine->rotateMatrix( lt, Vector3f( 0,1,0 ), -Kangle * leftDynamicWarp + _linetwists );
    rt = Gameplay::iEngine->rotateMatrix( rt, Vector3f( 0,1,0 ), Kangle * rightDynamicWarp + _linetwists );

	// pull cords with riser input
	frontLeftOffset.y -= 0.3f * _leftWarpDeep;
	frontRightOffset.y -= 0.3f * _rightWarpDeep;
	rearLeftOffset.y -= 0.3f * _leftRearRiser;
	rearRightOffset.y -= 0.3f * _rightRearRiser;

    NxVec3 frontLeftAnchor = wrap( Gameplay::iEngine->transformCoord( wrap( _frontLeftAnchor[0] + frontLeftOffset ), lt ) );
    NxVec3 rearLeftAnchor = wrap( Gameplay::iEngine->transformCoord( wrap( _rearLeftAnchor[0] + rearLeftOffset ), lt ) );
    NxVec3 frontRightAnchor = wrap( Gameplay::iEngine->transformCoord( wrap( _frontRightAnchor[0] + frontRightOffset ), rt ) );
    NxVec3 rearRightAnchor = wrap( Gameplay::iEngine->transformCoord( wrap( _rearRightAnchor[0] + rearRightOffset ), rt ) );

    // re-initialize ropes   
    _frontLeftRope->setAnchor1( frontLeftAnchor );
    _frontRightRope->setAnchor1( frontRightAnchor );
    _rearLeftRope->setAnchor1( rearLeftAnchor );
    _rearRightRope->setAnchor1( rearRightAnchor );

    // remove twists
    if( _linetwists != 0 && ( _leftWarpDeep != 0 || _rightWarpDeep != 0 ) ) 
    {
        Jumper* jumper = dynamic_cast<Jumper*>( _parent ); assert( jumper );
        
        // twists removal speed
        float minSkill = 0.0f;
        float minSpeed = 30.0f;
        float maxSkill = 1.0f;
        float maxSpeed = 180.0f;
        float factor = ( jumper->getVirtues()->getRiggingSkill() - minSkill ) / ( maxSkill - minSkill );
        factor = factor < 0 ? 0 : ( factor > 1 ? 1 : factor );
        float speed = minSpeed * ( 1 - factor ) + maxSpeed * factor;
        
        // simulate removal
        if( _linetwists > 0 )
        {
			// heavy linetwist
			if (_linetwists > 1000.0f) {
				_linetwists -= speed * _leftWarpDeep * dt * 0.01f;
			} else {
				_linetwists -= speed * _leftWarpDeep * dt;
			}
            if( _linetwists < 0 ) _linetwists = 0;
        }
        else if( _linetwists < 0 )
        {
            _linetwists += speed * _rightWarpDeep * dt;
            if( _linetwists > 0 ) _linetwists = 0;
        }

        // update skill
        if( jumper->getVirtues()->equipment.experience )
        {
            jumper->getVirtues()->skills.rigging += dt * jumper->getVirtues()->predisp.rigging;
        }
    }
}

void CanopySimulator::updateSlider(float dt)
{
    if( !_sliderUp ) return;

    // simulation properties
    float slidingVel = 4.0f;
    float slidingInflation = 0.25f;

    _slidingTime += dt;
	float canopySpeed =_nxCanopy->getLinearVelocity().y;

    // simulate sliding
	if ( fabs(_linetwists) > 0.0f) {
		float maxInflation = 1.0f - fabs(_linetwists) / 1440.0f * 0.5f; // max inflation due to linetwist
		float sliderMax = (fabs(_linetwists) / 1440.0f);
		slidingVel = 0.1f;
		if (_sliderPosFL > _gearRecord->frontCord * sliderMax) {
			_sliderPosFL -= slidingVel * dt;
			_sliderPosFR -= slidingVel * dt;
			_sliderPosRL -= slidingVel * dt;
			_sliderPosRR -= slidingVel * dt;
		}

	} else if( abs(canopySpeed) <= 20.0f )
    {
        _sliderPosFL -= slidingVel * dt;
        _sliderPosFR -= slidingVel * dt;
        _sliderPosRL -= slidingVel * dt;
        _sliderPosRR -= slidingVel * dt;
        if( _sliderPosFL < 0 ) _sliderPosFL = 0;
        if( _sliderPosFR < 0 ) _sliderPosFR = 0;
        if( _sliderPosRL < 0 ) _sliderPosRL = 0;
        if( _sliderPosRR < 0 ) _sliderPosRR = 0;
	 }

	// parametric positions
	float pSliderPosFL = _sliderPosFL / _gearRecord->frontCord;
	float pSliderPosFR = _sliderPosFR / _gearRecord->frontCord;
	float pSliderPosRL = _sliderPosRL / _gearRecord->rearCord;
	float pSliderPosRR = _sliderPosRR / _gearRecord->rearCord;

    // place slider
    Vector3f at = _canopyClump->getFrame()->getUp() * -1; at.normalize();
    Vector3f up = _canopyClump->getFrame()->getAt(); up.normalize();
    Vector3f right = _canopyClump->getFrame()->getRight(); right.normalize();

    Vector3f canopyFL = CanopySimulator::getPhysicsJointFrontLeft( _canopyClump )->getPos();    
    Vector3f canopyFR = CanopySimulator::getPhysicsJointFrontRight( _canopyClump )->getPos();
    Vector3f canopyRL = CanopySimulator::getPhysicsJointRearLeft( _canopyClump )->getPos();    
    Vector3f canopyRR = CanopySimulator::getPhysicsJointRearRight( _canopyClump )->getPos();    
    Vector3f jumperFL = _frontLeftRiser->getPos();
    Vector3f jumperFR = _frontRightRiser->getPos();
    Vector3f jumperRL = _rearLeftRiser->getPos();
    Vector3f jumperRR = _rearRightRiser->getPos();
    Vector3f posFL    = (canopyFL - jumperFL); posFL *= pSliderPosFL; posFL = jumperFL + posFL;
    Vector3f posFR    = (canopyFR - jumperFR); posFR *= pSliderPosFR; posFR = jumperFR + posFR;
    Vector3f posRL    = (canopyRL - jumperRL); posRL *= pSliderPosRL; posRL = jumperRL + posRL;
    Vector3f posRR    = (canopyRR - jumperRR); posRR *= pSliderPosRR; posRR = jumperRR + posRR;
    Vector3f clumpPos = (posFL + posFR + posRL + posRR) * 0.25f;

    MATRIX( C, right, up, at, clumpPos );   
    MATRIX( FL, right, up, at, posFL );
    MATRIX( FR, right, up, at, posFR );
    MATRIX( RL, right, up, at, posRL );
    MATRIX( RR, right, up, at, posRR );

    _sliderClump->getFrame()->setMatrix( C );

    _sliderClump->getFrame()->getLTM();
}

void CanopySimulator::updateProceduralAnimation(float dt)
{    
    for( unsigned int i=0; i<_gearRecord->riserScheme->getNumPABs(); i++ )
    {
        if( _pabs[i] ) _pabs[i]->update( dt );
    }
}

void CanopySimulator::updateCollapse(float dt)
{
    float vel = _nxCanopy->getLinearVelocity().magnitude();
    float restore = _gearRecord->Crestore * vel / _gearRecord->Cresvel;    

    // update collapse areas    
    unsigned int i,j;
    for( i=0; i<_gearRecord->Cnum; i++ )
    {        
        _collapseAreas[i].radius -= restore * dt;
        if( _collapseAreas[i].radius < 0 ) _collapseAreas[i].radius = 0;
    }

    // unite collapse areas
    bool isUnited;
    do
    {
        isUnited = false;
        // pass each pair        
        for( i=0; i<_gearRecord->Cnum; i++ )
        {
            for( j=0; j<_gearRecord->Cnum; j++ )
            {
                if( i != j && _collapseAreas[i].radius > 0 && _collapseAreas[j].radius > 0 )
                {
                    if( _collapseAreas[i].canUnite( _collapseAreas+j, _gearRecord ) )
                    {
                        // unite the collapse areas
                        _collapseAreas[i].unite( _collapseAreas+j, _gearRecord );
                        _collapseAreas[j].reset();
                        isUnited = true;
                        break;
                    }
                }
                if( isUnited ) break;
            }
        }
    }
    while( isUnited );
}

const NxVec3 worldUp( 0,-1,0 );

void CanopySimulator::rip(float force)
{
    if( _finite( force ) )
    {
        float factor = ( force - _gearRecord->Kminfrict ) / ( _gearRecord->Kmaxfrict - _gearRecord->Kminfrict );
        factor = factor < 0 ? 0 : factor;
        float damage = _gearRecord->Kminrip * ( 1 - factor ) + _gearRecord->Kmaxrip * factor;
        damage *= ::simulationStepTime;
        _gear->state -= damage;
        _gear->state = _gear->state < 0.1f ? 0.1f : _gear->state;
    }
}

void CanopySimulator::onContact(NxContactPair &pair, NxU32 events)
{
    // iterate contact points
    NxContactStreamIterator contactI( pair.stream );
    while( contactI.goNextPair() ) while( contactI.goNextPatch() ) while( contactI.goNextPoint() )
    {
        NxVec3 point = contactI.getPoint();
        float  pointVel = _nxCanopy->getPointVelocity( point ).magnitude();
        // filter slight contacts
        if( pointVel > _gearRecord->Cminvel )
        {
            // simulate cohesion
            if( _cohesionJoint == NULL )
            {
                // detemine cohesion instantaneous probability
                float dot = contactI.getPatchNormal().dot( worldUp );
                if( dot > 0 )
                {
                    float probPerSecond = pow( dot, 3.0f ) * pow( cos( dot ), 2.0f );
                    float probPerStep = probPerSecond * ::simulationStepTime;
                    float dice = getCore()->getRandToolkit()->getUniform( 0,1 );                    
                    if( dice < probPerStep )
                    {
                        _cohesionState = true;
                        _cohesionPoint = point;
                    }
                }
            }

            // ripping damage canopy
            float friction = pair.sumFrictionForce.magnitude();
            rip( friction );            

            // calculate contact point in local space of canopy shape
            NxMat34 pose = _nxCanopy->getGlobalPose();
            NxMat34 ipose;
            bool result = pose.getInverse( ipose ); assert( result );            
            NxVec3 localPoint = wrap( Gameplay::iEngine->transformCoord( wrap( point ), wrap( ipose ) ) );
            // calculate power & radius of collapse
            float interpolator = ( pointVel - _gearRecord->Cminvel ) / ( _gearRecord->Cmaxvel - _gearRecord->Cminvel );
            interpolator = interpolator < 0 ? 0 : interpolator;
            interpolator = interpolator > 1 ? 1 : interpolator;
            float power  = _gearRecord->Cpower * interpolator;
            float radius = _gearRecord->Cradius * interpolator;
            // integrate collapse in to simulation
            // first, search for crossover collapse area
            bool isUnited = false;
            for( unsigned int i=0; i<_gearRecord->Cnum; i++ )
            {
                if( _collapseAreas[i].radius > 0 )
                {                    
                    if( _collapseAreas[i].canUnite( localPoint, radius, _gearRecord ) )
                    {
                        // unite the collapse areas
                        _collapseAreas[i].unite( localPoint, radius, _gearRecord );
                        isUnited = true;
                        break;
                    }
                }
            }
            if( !isUnited )
            {
                for( unsigned int i=0; i<_gearRecord->Cnum; i++ )
                {
                    if( _collapseAreas[i].radius == 0 )
                    {
                        _collapseAreas[i].setup( localPoint, radius );
                        break;
                    }
                }
            }
            else
            {
                updateCollapse( 0.0f );
            }
            // event (for parent actor)
            _parent->happen( this, EVENT_CANOPY_COLLAPSED );
        }
    }
}

void CanopySimulator::visualizeForce(NxVec3& pos, NxVec3& force)
{
}

void CanopySimulator::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
    if( eventId == EVENT_CAMERA_IS_ACTUAL && isOpened() && !isCutAway )
    {
        if( _signature->getPanel()->getParent() )
        {
            // update signature window
            Vector3f screenPos = Gameplay::iEngine->getDefaultCamera()->projectPosition( _canopyClump->getFrame()->getPos() );
            if( screenPos[2] > 1 )
            {
                _signature->getPanel()->setVisible( false );
            }
            else
            {   
                _signature->getPanel()->setVisible( true );
                gui::Rect oldRect = _signature->getPanel()->getRect();
                gui::Rect newRect(
                    int( screenPos[0] ), int( screenPos[1] ),
                    int( screenPos[0] ) + oldRect.getWidth(),
                    int( screenPos[1] ) + oldRect.getHeight()
                );
                _signature->getPanel()->setRect( newRect );

                // canopy name
                gui::IGuiPanel* panel = _signature->getPanel()->find( "CanopyName" ); assert( panel && panel->getStaticText() );
				
				database::Canopy* canopyInfo;
				if (_gear->type == gtCanopy) {
					canopyInfo = database::Canopy::getRecord( this->_gear->id );
				} else if (gtReserve)  {
					canopyInfo = database::Canopy::getReserveRecord( this->_gear->id );
				} else {
					assert("Should not be here!!!");
				}
                panel->getStaticText()->setText( Gameplay::iLanguage->getUnicodeString( canopyInfo->nameId ) );
    
                // canopy inflation
                panel = _signature->getPanel()->find( "CanopyInflation" ); assert( panel && panel->getStaticText() );
                panel->getStaticText()->setText( wstrformat( Gameplay::iLanguage->getUnicodeString(234), 100.0f * _inflation ).c_str() );
            }
        }
    
        // jumper
        Jumper* jumper = dynamic_cast<Jumper*>( _parent );
        if( jumper && jumper->isPlayer() )
        {
            // left lineover
            updateMalfunctionSignature(
                _leftLineoverSignature,
                Gameplay::iLanguage->getUnicodeString(256),
                _leftLOW,
                _leftLineoverJoint->getPos()
            );

            // right lineover
            updateMalfunctionSignature(
                _rightLineoverSignature,
                Gameplay::iLanguage->getUnicodeString(256),
                _rightLOW,
                _rightLineoverJoint->getPos()
            );

            // linetwists text
            unsigned int textId = _linetwists > 0 ? 374 : 375;
            const wchar_t* text = Gameplay::iLanguage->getUnicodeString(textId);

            // linetwists weight
            float minTwists = 0.0f;
            float minWeight = 0.0f;
            float maxTwists = 1440.0f;
            float maxWeight = 1.0f;
            float factor = ( fabs( _linetwists ) - minTwists ) / ( maxTwists - minTwists );
            factor = factor < 0 ? 0 : ( factor > 1 ? 1 : factor );
            float weight = minWeight * ( 1 - factor ) + maxWeight * factor;

            // linetwists position
            Vector3f pos = ( Jumper::getFrontLeftRiser( jumper->getClump() )->getPos() + 
                             Jumper::getFrontRightRiser( jumper->getClump() )->getPos() + 
                             Jumper::getRearLeftRiser( jumper->getClump() )->getPos() + 
                             Jumper::getRearRightRiser( jumper->getClump() )->getPos() ) * 0.25f;
            
            // update signature
            updateMalfunctionSignature( _linetwistsSignature, text, weight, pos );
        }        
    }
    // HUD hide event
    if( eventId == EVENT_HUD_HIDE )
    {
        if( _wloToggles ) _wloToggles->hideGui();
        if( _hookKnife ) _hookKnife->hideGui();
    }
}

void CanopySimulator::updateMalfunctionSignature(gui::IGuiWindow* signature, const wchar_t* description, float weight, const Vector3f& pos)
{
    // saturate weight
    weight = weight > 1 ? 1 : ( weight < 0 ? 0 : weight );

    if( weight > 0 )
    {
        // show signature panel
        if( !signature->getPanel()->getParent() )
        {
            Gameplay::iGui->getDesktop()->insertPanel( signature->getPanel() );
        }
        
        // blink dot pointer
        gui::IGuiPanel* panel = signature->getPanel()->find( "DotPoint" );
        assert( panel );
        Vector4f color = panel->getColor();
        color[3] = float( sin( 0.5f * ( 1 + sin( _mBlinkTime * 31.4 ) ) ) );
        panel->setColor( color );

        // update description & shadow
        panel = signature->getPanel()->find( "Description" );
        assert( panel && panel->getStaticText() );
        panel->getStaticText()->setText( description );
        panel = signature->getPanel()->find( "DescriptionShadow" );
        assert( panel && panel->getStaticText() );
        panel->getStaticText()->setText( description );

        // update weight level
        gui::IGuiPanel* vessel = signature->getPanel()->find( "WeightVessel" ); assert( vessel );
        gui::IGuiPanel* level  = vessel->find( "WeightLevel" ); assert( level );
        level->setRect( gui::Rect( 0, 0, int( 64 * weight ), 8 ) );
        level->setTextureRect( gui::Rect( 0, 0, int( 128 * weight ), 16 ) );

        // place signature window
        Vector3f screenPos = Gameplay::iEngine->getDefaultCamera()->projectPosition( pos );
        if( screenPos[2] > 1 )
        {
            signature->getPanel()->setVisible( false );
        }
        else
        {
            signature->getPanel()->setVisible( true );
            gui::Rect oldRect = signature->getPanel()->getRect();
            gui::Rect newRect(
                int( screenPos[0] ), int( screenPos[1] ),
                int( screenPos[0] ) + oldRect.getWidth(),
                int( screenPos[1] ) + oldRect.getHeight()
            );
            signature->getPanel()->setRect( newRect );
        }
    }
    // hide signature panel
    else if( signature->getPanel()->getParent() )
    {        
        signature->getPanel()->getParent()->removePanel( signature->getPanel() );
    }
}