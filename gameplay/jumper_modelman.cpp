
#include "headers.h"
#include "jumper.h"
#include "imath.h"
#include "gear.h"
#include "database.h"
#include "version.h"

/**
 * model management
 */

void Jumper::hideEffectors(engine::IClump* clump)
{
    // animation effectors    
    // Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "LhendEffector" ) )->setFlags( 0 );
    // Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "RhendEffector" ) )->setFlags( 0 );    
    // Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "RfootEffector" ) )->setFlags( 0 );
    // Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "LfootEffector" ) )->setFlags( 0 );
    Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "Walk_dummy" ) )->setFlags( 0 );
    Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "dummy00" ) )->setFlags( 0 );
    Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "Character01_root" ) )->setFlags( 0 );
    // physics effectors
    Jumper::getCollisionFF( clump )->setFlags( 0 );
    Jumper::getCollisionFC( clump )->setFlags( 0 );

    // also, hide all licensed stuff by default
    // hide head of Valery Rosov
    Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "Head05" ) )->setFlags( 0 );
    // hide head of Felix Baumgartner
    Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "Felix_mesh" ) )->setFlags( 0 );
}

void Jumper::placeCord(engine::IClump* cord, const Vector3f& p0, const Vector3f& p1, float width)
{
    Vector3f x,y,z;
    float    yScale;

    y = p1 - p0;
    yScale = y.length() * 0.01f;
    y.normalize();
    x.set( y[1], y[2], y[0] );
    z.cross( x,y );
    x.cross( y,z );
    y *= yScale;
    x *= width;
    z *= width;

    cord->getFrame()->setMatrix( Matrix4f(
        x[0], x[1], x[2], 0.0f,
        y[0], y[1], y[2], 0.0f,
        z[0], z[1], z[2], 0.0f,
        p0[0], p0[1], p0[2], 1.0f
    ) );
}

void Jumper::placeCord(Matrix4f& instanceMatrix, const Vector3f& p0, const Vector3f& p1, float width)
{
    Vector3f x,y,z;
    float    yScale;

    y = p1 - p0;
    yScale = y.length() * 0.01f;
    y.normalize();
    x.set( y[1], y[2], y[0] );
    z.cross( x,y );
    x.cross( y,z );
    y *= yScale;
    x *= width;
    z *= width;

    instanceMatrix.set(
        x[0], x[1], x[2], 0.0f,
        y[0], y[1], y[2], 0.0f,
        z[0], z[1], z[2], 0.0f,
        p0[0], p0[1], p0[2], 1.0f
    );
}

engine::IAtomic* Jumper::getRisers(engine::IClump* clump)
{
    engine::IAtomic* risers;
    risers = Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "Parachute_risers03_Parashute_risers_poly" ) ); assert( risers );
    return risers;
}

engine::IFrame* Jumper::getChestFrame(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "Grud_" );
}

engine::IFrame* Jumper::getPelvisFrame(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "Taz" );
}

engine::IFrame* Jumper::getHeadFrame(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "Head" );
}

unsigned int Jumper::getNumHeads(engine::IClump* clump)
{
    // common heads
    unsigned int result = 4;

    // head of Valery Rosov - for ND edition
    #ifdef GAMEPLAY_EDITION_ND
        result++;
    #endif

    // head of Felix Baumgartner - for ND edition
    #ifdef GAMEPLAY_EDITION_ATARI
        result++;
    #endif

    return result;
}

engine::IAtomic* Jumper::getHead(engine::IClump* clump, unsigned int headId)
{
    assert( headId >= 0 && headId < getNumHeads( clump ) );
    switch( headId )
    {
    case 0:
        return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "Head01" ) );
    case 1:
        return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "Head02" ) );
    case 2:
        return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "Head03" ) );
    case 3:
        return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "Head04" ) );
    case 4:
        // Valery Rosov
        #ifdef GAMEPLAY_EDITION_ND            
            return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "Head05" ) );
        #endif
        // Felix Baumgartner
        #ifdef GAMEPLAY_EDITION_ATARI
            return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "Felix_mesh" ) );
        #endif
        // no head?
        return NULL;
    default:
        assert( !"shouldn't be here!" );
        return NULL;
    }
}

void Jumper::setHead(engine::IClump* clump, unsigned int headId, Jumper* jumper)
{
    assert( headId < getNumHeads( clump ) );
    unsigned int flags;
    for( unsigned int i=0; i<getNumHeads(clump); i++ )
    {
        flags = 0;
        if( i == headId ) 
        {
            flags = engine::afRender | engine::afCollision;
            if( jumper && jumper->isPlayer() ) 
            {
                int shadows = atoi( Gameplay::iGameplay->getConfigElement( "video" )->Attribute( "shadows" ) );
                if( shadows ) flags = flags | engine::afCastShadow;
            }
        }        
        getHead( clump, i )->setFlags( flags );
    }
}

unsigned int Jumper::getNumHelmets(engine::IClump* clump)
{
    return 3;
}

engine::IAtomic* Jumper::getHelmet(engine::IClump* clump, unsigned int helmetId)
{
    switch( helmetId )
    {
    case 0:
        return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "Hemlet01_mod01" ) );
    case 1:
        return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "polySurface630" ) );
    case 2:
        return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "Helmet03" ) );
    default:
        assert( !"shouldn't be here!" );
        return NULL;
    }
}

void Jumper::setHelmet(engine::IClump* clump, unsigned int helmetId, Jumper* jumper)
{
    assert( helmetId < getNumHelmets( clump ) );
    unsigned int flags;
    for( unsigned int i=0; i<getNumHelmets(clump); i++ )
    {
        flags = 0;
        if( i == helmetId ) 
        {
            flags = engine::afRender | engine::afCollision;
            if( jumper && jumper->isPlayer() ) 
            {
                int shadows = atoi( Gameplay::iGameplay->getConfigElement( "video" )->Attribute( "shadows" ) );
                if( shadows ) flags = flags | engine::afCastShadow;
            }
        }
        getHelmet( clump, i )->setFlags( flags );
    }
}

void Jumper::setNoHelmet(engine::IClump* clump)
{
    for( unsigned int i=0; i<Jumper::getNumHelmets( clump ); i++ )
    {
        Jumper::getHelmet( clump, i )->setFlags( 0 );
    }
}

engine::IAtomic* Jumper::getLeftHand(engine::IClump* clump)
{
    return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "polySurface619" ) );
}

engine::IAtomic* Jumper::getRightHand(engine::IClump* clump)
{
    return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "polySurface573" ) );
}

engine::IAtomic* Jumper::getLeftEye(engine::IClump* clump)
{
    return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "polySurface582" ) );
}

engine::IAtomic* Jumper::getRightEye(engine::IClump* clump)
{
    return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "polySurface620" ) );
}

engine::IAtomic* Jumper::getLeftRing(engine::IClump* clump)
{
    return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "LD_poly" ) );
}

engine::IAtomic* Jumper::getRightRing(engine::IClump* clump)
{
    return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "RD_poly" ) );
}

unsigned int Jumper::getNumBodies(engine::IClump* clump)
{
    return 2;
}

engine::IAtomic* Jumper::getBody(engine::IClump* clump, unsigned int bodyId)
{
    switch( bodyId )
    {
    case 0 :
        return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "Body01" ) );
    case 1:
        return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "polySurface627" ) );
    //case 2:
      //  return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "Body02" ) );
    default:
        assert( !"shouldn't be here!" );
        return NULL;
    }
}

void Jumper::setBody(engine::IClump* clump, unsigned int bodyId, Jumper* jumper)
{
    assert( bodyId < getNumBodies( clump ) );
    unsigned int flags;
    for( unsigned int i=0; i<getNumBodies(clump); i++ )
    {
        flags = 0;
        if( i == bodyId ) 
        {
            flags = engine::afRender | engine::afCollision;
            if( jumper && jumper->isPlayer() ) 
            {
                int shadows = atoi( Gameplay::iGameplay->getConfigElement( "video" )->Attribute( "shadows" ) );
                if( shadows ) flags = flags | engine::afCastShadow;
            }
        }
        getBody( clump, i )->setFlags( flags );
    }
}

engine::IAtomic* Jumper::getCollisionFF(engine::IClump* clump)
{
    return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "CollisionFF" ) );
}

engine::IAtomic* Jumper::getCollisionFC(engine::IClump* clump)
{
    return Gameplay::iEngine->getAtomic( clump, Gameplay::iEngine->findFrame( clump->getFrame(), "CollisionFC" ) );
}

engine::IFrame* Jumper::getBackBone(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "Poyasnitsa" );
}

engine::IFrame* Jumper::getLeftLegBone(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "Hip_Joint" );
}

engine::IFrame* Jumper::getRightLegBone(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "Hip_Joint1" );
}

engine::IFrame* Jumper::getLineHandJoint(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "LineHandJoint" );
}

engine::IFrame* Jumper::getLineRigJoint(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "LineRigJoint" );
}

engine::IFrame* Jumper::getFrontLeftRiser(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "Parachute_risers03_LRisers_back_End1" );
}

engine::IFrame* Jumper::getFrontRightRiser(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "Parachute_risers03_RRisers_front_End" );
}

engine::IFrame* Jumper::getRearLeftRiser(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "Parachute_risers03_LRisers_back_End" );
}

engine::IFrame* Jumper::getRearRightRiser(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "Parachute_risers03_RRisers_back_End" );
}

engine::IFrame* Jumper::getPhysicsJointFrontLeft(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "PhysicsJointFrontLeft" );
}

engine::IFrame* Jumper::getPhysicsJointFrontRight(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "PhysicsJointFrontRight" );
}

engine::IFrame* Jumper::getPhysicsJointRearLeft(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "PhysicsJointRearLeft" );
}

engine::IFrame* Jumper::getPhysicsJointRearRight(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "PhysicsJointRearRight" );
}

engine::IFrame* Jumper::getFirstPersonFrame(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "FirstPerson" );
}

engine::IFrame* Jumper::getHelmetEquipAnchor(engine::IClump* clump)
{
    engine::IFrame* helmetEquipAnchor = Gameplay::iEngine->findFrame( clump->getFrame(), "joint6" );
    assert( helmetEquipAnchor );
    return helmetEquipAnchor;
}

engine::IFrame* Jumper::getSuitEquipAnchor(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "Poyasnitsa" );
}

engine::IFrame* Jumper::getRigEquipAnchor(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "RKluchitsa1" );
}

engine::IFrame* Jumper::getLeftSmokeJetAnchor(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "Ankle_Joint" );
}

engine::IFrame* Jumper::getRightSmokeJetAnchor(engine::IClump* clump)
{
    return Gameplay::iEngine->findFrame( clump->getFrame(), "Ankle_Joint1" );
}