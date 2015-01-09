
#include "headers.h"
#include "gameplay.h"
#include "sensor.h"

Sensor::Sensor()
{
    _rayIntersection = Gameplay::iEngine->createRayIntersection();
}

Sensor::~Sensor()
{
    _rayIntersection->release();
}

engine::CollisionTriangle* Sensor::onIntersection(
    engine::CollisionTriangle* collTriangle,
    engine::IBSPSector* sector,
    engine::IAtomic* atomic,
    void* data
)
{
    Sensor* sensor = (Sensor*)( data );
    sensor->_intersections.push_back( *collTriangle );
    return collTriangle;
}

void Sensor::sense(const Vector3f& pos, const Vector3f& dir, engine::IBSP* bsp)
{
    _intersections.clear();
    _rayIntersection->setRay( pos, dir );
    _rayIntersection->intersect( bsp, onIntersection, this );
}

void Sensor::sense(const Vector3f& pos, const Vector3f& dir, engine::IAtomic* atomic)
{
    _intersections.clear();
    _rayIntersection->setRay( pos, dir );
    _rayIntersection->intersect( atomic, onIntersection, this );
}