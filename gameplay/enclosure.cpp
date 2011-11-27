
#include "headers.h"
#include "gameplay.h"
#include "sensor.h"
#include "../common/istring.h"

Enclosure::Enclosure(engine::IClump* clump, float delay)
{
    _clump = clump;
    _clump->getFrame()->translate( Vector3f( 0,0,0 ) );
    _clump->getFrame()->getLTM();

    _delay = delay;

    _ray = Gameplay::iEngine->createRayIntersection();
    _sphere = Gameplay::iEngine->createSphereIntersection();
    
    callback::Locator locator;
    locator.targetName = _clump->getName();
    locator.target = NULL;
    _clump->forAllAtomics( callback::locateAtomic, &locator );    
    _collisionAtomic = reinterpret_cast<engine::IAtomic*>( locator.target );
    assert( _collisionAtomic );

    engine::IGeometry* geometry = _collisionAtomic->getGeometry();
    engine::IShader* shader;
    unsigned int numFaces = geometry->getNumFaces();
    Vector3f vertex[3];
    Vector3f normal;
    bool flag;
    unsigned int i,j;

    // enumerate normals of wall faces 
    for( i=0; i<numFaces; i++ )
    {
        geometry->getFace( i, vertex[0], vertex[1], vertex[2], &shader );
        if( strcmp( shader->getName(), "EnclosureFloor" ) != 0 )
        {
            normal.cross( vertex[1]-vertex[0], vertex[2]-vertex[0] );
            normal.normalize();
            // check this normal was enumerated
            flag = false;
            for( j=0; j<_wallNormals.size(); j++ )
            {
                if( Vector3f::dot( _wallNormals[j], normal ) > 0.999f )
                {
                    flag = true;
                    break;
                }
            }
            if( !flag ) _wallNormals.push_back( normal );
        }
    }

    // enumerate position markers (all atomics under enclosure care)
    callback::AtomicL atomicL;
    _clump->forAllAtomics( callback::enumerateAtomics, &atomicL );    
    for( callback::AtomicI atomicI=atomicL.begin(); atomicI!=atomicL.end(); atomicI++ )
    {   
        if( (*atomicI) != _collisionAtomic )
        {
            _markers.push_back( (*atomicI)->getFrame() );
        }
    }

    /*
    // check number of path points (limit of 2)
    unsigned int numPathPoints = 0;
    for( unsigned int i=0; i<getNumMarkers(); i++ )
    {
        if( getMarkerFlags( i ) & mtPath ) numPathPoints++;
    }
    if( numPathPoints > 2 )
    {
        throw Exception( "Too many pathpoints in enclosure %s", clump->getName() );
    }
    */
}

Enclosure::~Enclosure()
{
    _ray->release();
    _sphere->release();
}

Vector3f Enclosure::place(void)
{
    // choose random trinagle upon floor of collision atomic
    int numFaces = _collisionAtomic->getGeometry()->getNumFaces();
    int faceId;
    Vector3f vertex[3];
    engine::IShader* shader;
    do
    {
        faceId = int( getCore()->getRandToolkit()->getUniform( 0, float( numFaces ) ) );
        if( faceId == numFaces ) faceId--;
        _collisionAtomic->getGeometry()->getFace( faceId, vertex[0], vertex[1], vertex[2], &shader );
    }
    while( strcmp( shader->getName(), "EnclosureFloor" ) != 0 );

    // choose random position upon floor of triangle
    Vector3f v01 = vertex[0] + ( vertex[1] - vertex[0] ) * getCore()->getRandToolkit()->getUniform( 0,1 );
    Vector3f v02 = vertex[0] + ( vertex[2] - vertex[0] ) * getCore()->getRandToolkit()->getUniform( 0,1 );
    Vector3f p = v01 + ( v02 - v01 ) * getCore()->getRandToolkit()->getUniform( 0,1 );
    p = Gameplay::iEngine->transformCoord( p, _collisionAtomic->getFrame()->getLTM() );
    return p;
}

engine::CollisionTriangle* Enclosure::onRayCollision(
    engine::CollisionTriangle* collTriangle,
    engine::IBSPSector* sector,
    engine::IAtomic* atomic,
    void* data
)
{
    Enclosure* __this = reinterpret_cast<Enclosure*>( data );
    if( __this->_numTriangles == 0 ||
        __this->_nearestTriangle.distance > collTriangle->distance )
    {
        __this->_nearestTriangle = *collTriangle;
    }
    __this->_numTriangles++;
    return collTriangle;
}

Vector3f Enclosure::move(const Vector3f& fromPos, const Vector3f& direction, float width, float height)
{
    // first, obtain motion distance
    float distance = direction.length();

    // we should determine maximum distance that we can move safely, without probability of 
    // leave enclosure boundary
    float safeDistance = 0.5f * ( width < height ? width : height );

    // now prepare to move
    Vector3f pos = fromPos;
    Vector3f dir = direction;
    dir.normalize();

    Vector3f penetration;
    float stepDistance;
    unsigned int i;

    // move until there is a distance left
    do
    {
        // determine step distance
        stepDistance = safeDistance < distance ? safeDistance : distance;

        // move along normalized direction
        pos += dir * stepDistance;

        // detect "foot" collision
        _numTriangles = 0;
        _ray->setRay( pos, Vector3f( 0,-1,0 ) * height );
        _ray->intersect( _collisionAtomic, onRayCollision, this );
        if( _numTriangles )
        {
            // determine penetration vector
            penetration = _nearestTriangle.normal * height * ( 1.0f - _nearestTriangle.distance );
            // disable penetration
            pos += penetration;
        }

        // detect "body" collision
        for( i=0; i<_wallNormals.size(); i++ )
        {
            _numTriangles = 0;
            _ray->setRay( pos, _wallNormals[i] * -width );
            _ray->intersect( _collisionAtomic, onRayCollision, this );
            if( _numTriangles )
            {
                // determine penetration vector
                penetration = _nearestTriangle.normal * width * ( 1.0f - _nearestTriangle.distance );
                // disable penetration
                pos += penetration;
            }
        }

        // decrease distance left
        distance -= stepDistance;
    }
    while( distance > 0 );
        
    return pos;
}

engine::CollisionTriangle* Enclosure::onSphereCollision(
    engine::CollisionTriangle* collTriangle,
    engine::IBSPSector* sector,
    engine::IAtomic* atomic,
    void* data
)
{
    Enclosure* __this = reinterpret_cast<Enclosure*>( data );

    for( unsigned int i=0; i<__this->_numTriangles; i++ )
    {
        // exclude triangle duplications
        if( __this->_intersections[i].triangleId == collTriangle->triangleId )
        {
            return collTriangle;
        }

        Vector3f crossTest;
        crossTest.cross( __this->_intersections[i].normal, collTriangle->normal );
        if( crossTest.length() == 0 )
        {
            // collision normal
            Vector3f collisionNormal;
            collisionNormal = __this->_pos - collTriangle->collisionPoint;
            collisionNormal.normalize();
            crossTest.cross( collisionNormal, collTriangle->normal );
            if( crossTest.length() == 0 )
            {
                __this->_intersections[i] = *collTriangle;
            }
            else
            {
                Vector3f abc = collTriangle->normal;
                Vector3f x   = collTriangle->vertices[0];
                float    d   = -abc[0]*x[0] -abc[1]*x[1] -abc[2]*x[2];
                __this->_intersections[i] = *collTriangle;
                __this->_intersections[i].distance = fabs( 
                    abc[0] * __this->_pos[0] + 
                    abc[1] * __this->_pos[1] + 
                    abc[2] * __this->_pos[2] + d 
                );
                Vector3f cn = abc * -__this->_intersections[i].distance;
                __this->_intersections[i].collisionPoint = __this->_pos + cn;
            }
            return collTriangle;
        } 
    }

    if( __this->_numTriangles == MAX_INTERSECTIONS ) return collTriangle;

    __this->_intersections[__this->_numTriangles] = *collTriangle;
    __this->_numTriangles++;

    return collTriangle;
}

Vector3f Enclosure::move(const Vector3f& fromPos, const Vector3f& direction, float radius)
{
    // first, obtain motion distance
    float distance = direction.length();

    // we should determine maximum distance that we can move safely, without probability of 
    // leave enclosure boundary
    float safeDistance = 0.9f * radius;

    // now prepare to move
    _pos = fromPos;
    Vector3f dir = direction; dir.normalize();
    Vector3f temp, temp2;
    float    pDot;
    unsigned int i;    

    Vector3f penetration;
    float stepDistance;

    // move until there is a distance left
    do
    {
        // determine step distance
        stepDistance = safeDistance < distance ? safeDistance : distance;

        // move along normalized direction
        _pos += dir * stepDistance;

        // detect collision
        _numTriangles = 0;
        _sphere->setSphere( _pos, radius );
        _sphere->intersect( _collisionAtomic, onSphereCollision, this );

        // solve penetration 
        penetration.set( 0,0,0 );
        for( i=0; i<_numTriangles; i++ )
        {
            temp = _pos - _intersections[i].collisionPoint;
            temp.normalize();
            temp *= radius - _intersections[i].distance;
            if( penetration.length() == 0 )
            {
                penetration = temp;
            }
            else
            {
                temp2 = penetration;
                temp2.normalize();
                pDot = Vector3f::dot( temp2, temp );
                temp2 = temp;
                temp2.normalize();
                temp -= temp2 * pDot;
                penetration += temp;
            }
        }
        _pos += penetration;
        distance -= stepDistance;
    }
    while( distance > 0 );

    // setup actual distance
    _actualDistance = _pos - fromPos;

    // return calculated position
    return _pos;
}

Vector3f Enclosure::getActualDistance(void)
{
    return _actualDistance;
}

unsigned int Enclosure::getNumMarkers(void)
{
    return _markers.size();
}

unsigned int Enclosure::getMarkerFlags(unsigned int id)
{
    assert( id < _markers.size() );
    
    unsigned int result = 0;
    if( strstr( _markers[id]->getName(), "R" ) ) result = result | mtRespawn;
    if( strstr( _markers[id]->getName(), "P" ) ) result = result | mtPath;
    if( strstr( _markers[id]->getName(), "J" ) ) result = result | mtJump;
    if( strstr( _markers[id]->getName(), "S" ) ) result = result | mtStart;
    return result;
}

engine::IFrame* Enclosure::getMarkerFrame(unsigned int id)
{
    assert( id < _markers.size() );
    return _markers[id];
}

unsigned int Enclosure::getRandomRespawnMarker(void)
{
    // build list of respawn markers
    std::vector<unsigned int> respawnIds;
    for( unsigned int i=0; i<getNumMarkers(); i++ )
    {
        if( getMarkerFlags(i) & mtRespawn ) respawnIds.push_back( i );
    }
    assert( respawnIds.size() );

    unsigned int result = unsigned int( getCore()->getRandToolkit()->getUniform( 0, float( respawnIds.size() ) ) );
    result = result > respawnIds.size()-1 ? respawnIds.size()-1 : result;
    return respawnIds[result];
}