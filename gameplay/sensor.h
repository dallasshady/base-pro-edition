
#ifndef RAY_COLLISION_INTELLIGENCE_INCLUDED
#define RAY_COLLISION_INTELLIGENCE_INCLUDED

#include "headers.h"
#include "../shared/engine.h"
#include "callback.h"

/**
 * ray sensor
 */

class Sensor
{
private:
    typedef std::vector<engine::CollisionTriangle> IntersectionV;
    typedef IntersectionV::iterator IntersectionI;
private:
    engine::IRayIntersection* _rayIntersection;
    IntersectionV             _intersections;
    callback::ClumpL          _clumpL;
private:
    static engine::CollisionTriangle* onIntersection(
        engine::CollisionTriangle* collTriangle,
        engine::IBSPSector* sector,
        engine::IAtomic* atomic,
        void* data
    );
public:
    Sensor();
    virtual ~Sensor();
public:
    void sense(const Vector3f& pos, const Vector3f& dir, engine::IBSP* bsp);
    void sense(const Vector3f& pos, const Vector3f& dir, engine::IAtomic* atomic);
public:
    inline unsigned int getNumIntersections(void) 
    { 
        return (unsigned int)_intersections.size(); 
    }
    inline engine::CollisionTriangle* getIntersection(unsigned int iid)
    {
        assert( iid >= 0 && iid < _intersections.size() );
        return &_intersections[iid];
    }
};

/**
 * enclosure engine
 */

#define MAX_INTERSECTIONS 128

enum MarkerType
{
    mtRespawn = 0x01,
    mtPath    = 0x02,
    mtJump    = 0x04,
    mtStart   = 0x08
};

class Enclosure
{
private:
    typedef std::vector<engine::IFrame*> MarkerV;
    typedef MarkerV::iterator MarkerI;
private:
    // enclosure properties
    float                        _delay;
    engine::IClump*              _clump;
    engine::IAtomic*             _collisionAtomic;
    engine::IRayIntersection*    _ray;
    engine::ISphereIntersection* _sphere;
    std::vector<Vector3f>        _wallNormals;
    unsigned int                 _numMarkers;
    MarkerV                      _markers;
private:
    // collision detection : infos
    unsigned int              _numTriangles;
    engine::CollisionTriangle _nearestTriangle;
    engine::CollisionTriangle _intersections[MAX_INTERSECTIONS];
    Vector3f                  _pos;    
    Vector3f                  _actualDistance;
private:
    // collision detection : callbacks
    static engine::CollisionTriangle* onRayCollision(
        engine::CollisionTriangle* collTriangle,
        engine::IBSPSector* sector,
        engine::IAtomic* atomic,
        void* data
    );
    static engine::CollisionTriangle* onSphereCollision(
        engine::CollisionTriangle* collTriangle,
        engine::IBSPSector* sector,
        engine::IAtomic* atomic,
        void* data
    );
public:
    Enclosure(engine::IClump* clump, float delay);
    ~Enclosure();
public:
    inline engine::IAtomic* getCollisionAtomic(void) { return _collisionAtomic; }
    inline unsigned int getLastNumIntersectedTriangles(void) { return _numTriangles; }
    inline float getDelay(void) { return _delay; }
public:
    Vector3f place(void);
    Vector3f move(const Vector3f& fromPos, const Vector3f& direction, float width, float height);
    Vector3f move(const Vector3f& fromPos, const Vector3f& direction, float radius);
    Vector3f getActualDistance(void);    
public:
    // marker access
    unsigned int getNumMarkers(void);
    unsigned int getMarkerFlags(unsigned int id);
    engine::IFrame* getMarkerFrame(unsigned int id);
    unsigned int getRandomRespawnMarker(void);
};

#endif