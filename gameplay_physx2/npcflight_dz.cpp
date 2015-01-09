
#include "headers.h"
#include "npcflight_dz.h"
#include "npcmove.h"


/**
 * class implementation
 */

NPCFlight_DZ::NPCFlight_DZ(NPC* npc, float steeringAltitude, float landingAltitude) : 
    NPCProgram( npc )
{
    _steeringAltitude = steeringAltitude;
    _landingAltitude = landingAltitude;
	_groundTarget = Vector3f(30000.0f,0,0);
	_groundTargetEnabled = true;
	
    // determine steering range (multiplier) depended on wing loading
    // this is actual only for skydiving canopies
    _steeringRange = 1.0f;
    if( getNPC()->getJumper()->getCanopySimulator()->getGearRecord()->skydiving )
    {
        // determine wing loading
        float wingLoading = getNPC()->getJumper()->getCanopySimulator()->getWingLoading( 
            getNPC()->getJumper()->getVirtues()->appearance.weight
        );

        float minWingLoading = 0.5f;
        float minImpulse     = 1.0f;
        float maxWingLoading = 1.5f;
        float maxImpulse     = 0.33f;
        float factor         = ( wingLoading - minWingLoading ) / ( maxWingLoading - minWingLoading );
        factor = factor < 0 ? 0 : ( factor > 1 ? 1 : factor );
        _steeringRange = minImpulse * ( 1 - factor ) + maxImpulse * factor;
    }
}

NPCFlight_DZ::~NPCFlight_DZ()
{
}

/**
 * abstraction layer
 */

void NPCFlight_DZ::onUpdate(float dt)
{
    // obtain spinal cord
    SpinalCord* spinalCord = getNPC()->getSpinalCord();
    spinalCord->reset();

	const NxVec3 pos = this->getNPC()->getJumper()->getFlightActor()->getGlobalPosition();
	const NxVec3 vel = getNPC()->getJumper()->getFreefallActor()->getLinearVelocity();

	// handle linetwist
	if (getNPC()->getJumper()->getDominantCanopy()->getLinetwists() > 0.0f) {
		if (getNPC()->getJumper()->getDominantCanopy()->isOpened()) {
			spinalCord->leftWarp = 1.0f;
		}
		if (pos.y <= 600.0f && pos.y > 0.0f) {
			spinalCord->cutAway = true;
			spinalCord->pullReserve = true;
		}
		return;
	} else if (getNPC()->getJumper()->getDominantCanopy()->getLinetwists() < 0.0f) {
		if (getNPC()->getJumper()->getDominantCanopy()->isOpened()) {
			spinalCord->rightWarp = 1.0f;
		}
		if (pos.y <= 600.0f && pos.y > 0.0f) {
			spinalCord->cutAway = true;
			spinalCord->pullReserve = true;
		}
		return;
	}

	// is canopy inflated enough and is flying slow enough below 600 m.?
	if (pos.y <= 600.0f && pos.y > 0.0f &&
		(getNPC()->getJumper()->getDominantCanopy()->getInflation() < 0.6f ||
		fabs(vel.y) > 27.0f
		)) {
		
		//spinalCord->cutAway = true;
		//spinalCord->pullReserve = true;
	}


	/// landing vars
	float turnIntoFinalAlt;
	const float square = getNPC()->getJumper()->getDominantCanopy()->getGearRecord()->square;

	/*
	220		200
	200		 90
	180		110
	160		130
	140		 60
	120		 80
	100		100
	80		130
	60		130
	*/

	if (square > 200.0f) {
		turnIntoFinalAlt = 80.0f;
	} else if (square <= 60) {
		turnIntoFinalAlt = 100.0f;
	} else {
		turnIntoFinalAlt = 80.0f + (200.0f / square) / 4.0f;
	}
	const Vector3f wind = wrap(getNPC()->getScene()->getWindAtPoint( pos ));
	const Vector3f landingoffset = wind*(12-wind.length())*8;

    // ||                    ||
    // \/ AUTOMATE CONDITION \/

    // update target position
    Matrix4f catToyPose = getNPC()->getCatToy()->getCurrentPose();
    Vector3f targetPos( catToyPose[3][0]*0, catToyPose[3][1]*0, catToyPose[3][2]*0 );
	
	if (_groundTargetEnabled) {
		if (pos.y > 300.0f) {
			/*
			if (pos.x >= 300.0f) {
				_groundTarget[0] = getCore()->getRandToolkit()->getUniform(-33000.0f, -35000.0f);
				_groundTarget[2] = getCore()->getRandToolkit()->getUniform(33000.0f, 35000.0f);
				//getCore()->logMessage("going left");
			} else if (pos.x <= -300.0f) {
				_groundTarget[0] = getCore()->getRandToolkit()->getUniform(33000.0f, 35000.0f);
				_groundTarget[2] = getCore()->getRandToolkit()->getUniform(33000.0f, 35000.0f);
				//getCore()->logMessage("going right");
			}*/
		} else {
			_groundTarget = Vector3f(0,0,0);
		}

		_groundTarget = Vector3f(-8000.0,0,-9860.0f) + (wind/wind.length())*turnIntoFinalAlt;
		targetPos = _groundTarget;
	}

    // jumper absolute orientation
    Vector3f jumperAt = getNPC()->getJumper()->getClump()->getFrame()->getAt();
    jumperAt.normalize();

    Vector3f jumperRight = getNPC()->getJumper()->getClump()->getFrame()->getRight();
    jumperRight.normalize();

    // retrieve current jumper position    
    Vector3f jumperPos = getNPC()->getJumper()->getClump()->getFrame()->getPos();
    jumperPos += Vector3f( 0, jumperRoamingSphereSize, 0 );

    // direction to target
    Vector3f targetDir = targetPos - jumperPos;

	// turn upwind
	if (pos.y <= turnIntoFinalAlt) {
		targetDir = wrap(getNPC()->getScene()->getWindAtPoint( pos ));
	}
	targetDir.normalize();

    // angle to target
    Vector3f atH = jumperAt; atH[1] = 0; atH.normalize();
    Vector3f dirH = targetDir; dirH[1] = 0; dirH.normalize();
    float targetAngle = ::calcAngle( dirH, atH, Vector3f( 0,1,0 ) );

    // determine bank angle
    Vector3f jumperUp = getNPC()->getJumper()->getClump()->getFrame()->getUp();
    jumperUp.normalize();
    float bankAngle = ::calcAngle( jumperUp, Vector3f(0,1,0), jumperAt );

    // ||                ||
    // \/ AUTOMATE MODES \/

    // determine steering factor
    float factorSteering = 1.0f;
    float altitude = _steeringAltitude;
    if( getNPC()->getJumper()->getDistanceToSurface( altitude ) )
    {
        factorSteering = altitude / _steeringAltitude;
        factorSteering = factorSteering < 0.0f ? 0.0f : factorSteering;
        factorSteering = factorSteering > 1.0f ? 1.0f : factorSteering;
		factorSteering = 1.0f;
    }

    // horizontal steering (steering mode)
    float minAngle = 0.0f;
    float minValue = 0.0f;
    float maxAngle = 110.0f;
    float maxValue = 1.0f;
    float factor = ( fabs( targetAngle ) - minAngle ) / ( maxAngle - minAngle );
    factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );
    float impulse = minValue * ( 1 - factor ) + maxValue * factor;
    // smooth impulse
    impulse = pow( impulse, 1.25f );
    // apply impulse
    if( targetAngle < 0 )
    {            
        spinalCord->right = impulse * factorSteering * _steeringRange;
		if (pos.y < turnIntoFinalAlt && pos.y > turnIntoFinalAlt - 20.0f) spinalCord->right = 0.8f;
    }
    else
    {
        spinalCord->left = impulse * factorSteering * _steeringRange;
		if (pos.y < turnIntoFinalAlt && pos.y > turnIntoFinalAlt - 20.0f) spinalCord->left = 0.8f;
    }

    // detect obstacles
    NxRay worldRay;
    worldRay.orig = wrap( getNPC()->getJumper()->getClump()->getFrame()->getPos() );
    worldRay.dir  = wrap( getNPC()->getJumper()->getClump()->getFrame()->getAt() );
    worldRay.dir.y = 0.0f;
    worldRay.dir.normalize();
    float maxDist = 25.0f;
    float dist = maxDist;
    NxRaycastHit raycastHit;
    if( getNPC()->getScene()->getPhTerrainShape()->raycast( worldRay, maxDist, NX_RAYCAST_DISTANCE | NX_RAYCAST_NORMAL, raycastHit, true ) )
    {
        // solve steering
        Vector3f N = wrap( raycastHit.worldNormal ); N[1] = 0; N.normalize();
        Vector3f Xp = getNPC()->getJumper()->getClump()->getFrame()->getRight(); Xp[1] = 0; Xp.normalize();
        Vector3f Xn = Xp * -1;         
        Vector3f oY = Vector3f( 0,1,0 );
        float Ap = ::calcAngle( Xp, N, oY );
        float An = ::calcAngle( Xn, N, oY );
        if( Ap < An )
        {
            spinalCord->right += ( 1.0f - dist / maxDist ) * factorSteering * _steeringRange;
        }
        else
        {
            spinalCord->left += ( 1.0f - dist / maxDist ) * factorSteering * _steeringRange;
        }
    }

    // oscullation reduction (steering mode)
    // works if NPC is precisely oriented on its "cat toy"
    if( fabs( targetAngle ) < 30.0f )
    {
        // reduce oscillations
        float minAngle = 0.0f;
        float minValue = 0.0f;
        float maxAngle = 10.0f;
        float maxValue = 0.125f;
        float factor = ( fabs( bankAngle ) - minAngle ) / ( maxAngle - minAngle );
        factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );
        float impulse = minValue * ( 1 - factor ) + maxValue * factor;
        // smooth impulse
        impulse = pow( impulse, 1.25f );
        // apply impulse
        if( bankAngle > 0 )
        {            
            spinalCord->right += impulse * factorSteering * _steeringRange;
        }
        else
        {
            spinalCord->left += impulse * factorSteering * _steeringRange;
        }
    }
    
    // determine landing factor
    float factorLanding = 0.0f;
    altitude = _landingAltitude;
    if( getNPC()->getJumper()->getDistanceToSurface( altitude ) )
    {       
        factorLanding = altitude / _landingAltitude;
        factorLanding = factorLanding < 0.0f ? 0.0f : factorLanding;
        factorLanding = factorLanding > 1.0f ? 1.0f : factorLanding;
    }
    
    if( factorLanding > 0.0f )
    {
        // landing mode
        float minAltitude = 1.0f;
        float minImpulse  = 1.0f;
        float maxAltitude = _landingAltitude;
        float maxImpulse  = 0.0f;
        float factor = ( altitude - minAltitude ) / ( maxAltitude - minAltitude );
        factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );        
        float impulse = minImpulse * ( 1 - factor ) + maxImpulse * factor;
        spinalCord->right += impulse;
        spinalCord->left += impulse;

        // alignment mode
        float minAngle = 0.0f;
        float minValue = 0.0f;
        float maxAngle = 45.0f;
        float maxValue = 0.25f;
        factor = ( fabs( bankAngle ) - minAngle ) / ( maxAngle - minAngle );
        factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );
        impulse = minValue * ( 1 - factor ) + maxValue * factor;
        // smooth impulse
        impulse = pow( impulse, 1.25f );
        // apply impulse
        if( bankAngle < 0 )
        {            
            spinalCord->right += impulse * factorLanding;
        }
        else
        {
            spinalCord->left += impulse * factorLanding;
        }
    }
}

bool NPCFlight_DZ::isEndOfProgram(void)
{
    return getNPC()->getJumper()->isOverActivity();
}

void NPCFlight_DZ::onEndOfSubProgram(NPCProgram* subProgram)
{
}