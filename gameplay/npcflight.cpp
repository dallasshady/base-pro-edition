
#include "headers.h"
#include "npcflight.h"
#include "npcmove.h"


/**
 * class implementation
 */

NPCFlight::NPCFlight(NPC* npc, float steeringAltitude, float landingAltitude) : 
    NPCProgram( npc )
{
    _steeringAltitude = steeringAltitude;
    _landingAltitude = landingAltitude;

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

NPCFlight::~NPCFlight()
{
}

/**
 * abstraction layer
 */

void NPCFlight::onUpdate(float dt)
{
    // obtain spinal cord
    SpinalCord* spinalCord = getNPC()->getSpinalCord();
    spinalCord->reset();

    // ||                    ||
    // \/ AUTOMATE CONDITION \/

    // update target position
    Matrix4f catToyPose = getNPC()->getCatToy()->getCurrentPose();
    Vector3f targetPos( catToyPose[3][0], catToyPose[3][1], catToyPose[3][2] );

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
    }
    else
    {
        spinalCord->left = impulse * factorSteering * _steeringRange;
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

bool NPCFlight::isEndOfProgram(void)
{
    return getNPC()->getJumper()->isOverActivity();
}

void NPCFlight::onEndOfSubProgram(NPCProgram* subProgram)
{
}