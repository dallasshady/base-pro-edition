/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description engine entity
 *
 * @author bad3p
 */

#pragma once

#include "headers.h"
#include "engine.h"
#include "bsp.h"

/**
 * implementation of IRayIntersection
 */

class RayIntersection : public engine::IRayIntersection
{
private:
    Line                      _ray;
    Line                      _asRay;
    engine::CollisionCallBack _callBack;
    void*                     _callBackData;
    BSP*                      _bsp;
    BSPSector*                _bspSector;
    Atomic*                   _atomic;
    engine::CollisionTriangle _collisionTriangle;
    Geometry*                 _geometry;
    Vector*                   _vertices;
    Triangle*                 _triangles;
private:
    BSPSector* collideBSPSector(BSPSector* sector);
    OcTreeSector* collideBSPOcTreeSector(OcTreeSector* ocTreeSector);
    OcTreeSector* collideAtomicOcTreeSector(OcTreeSector* ocTreeSector);
    OcTreeSector* collideGeometryOcTreeSector(OcTreeSector* ocTreeSector);
public:
    // class implementation
    RayIntersection(void);
    virtual ~RayIntersection(void);
public:
    // IRayIntersection implementation
    virtual void __stdcall release(void);
    virtual void __stdcall setRay(const Vector3f& start, const Vector3f& direction);
    virtual void __stdcall intersect(engine::IBSP* bsp, engine::CollisionCallBack callBack, void* data);
    virtual void __stdcall intersect(engine::IAtomic* atomic, engine::CollisionCallBack callBack, void* data);
public:
    void intersect(Geometry* geometry, engine::CollisionCallBack callBack, void* data);
};

/**
 * implementation of ISphereIntersection
 */

class SphereIntersection : public engine::ISphereIntersection
{
private:
    Sphere                    _sphere;
    Sphere                    _asSphere;
    engine::CollisionCallBack _callBack;
    void*                     _callBackData;
    BSP*                      _bsp;
    BSPSector*                _bspSector;
    Atomic*                   _atomic;
    engine::CollisionTriangle _collisionTriangle;
    Geometry*                 _geometry;
    Vector*                   _vertices;
    Triangle*                 _triangles;
private:
    OcTreeSector* collideAtomicOcTreeSector(OcTreeSector* ocTreeSector);
public:
    // class implementation
    SphereIntersection(void);
    virtual ~SphereIntersection(void);
public:
    virtual void __stdcall release(void);
    virtual void __stdcall setSphere(const Vector3f& center, float radius);
    virtual void __stdcall intersect(engine::IAtomic* atomic, engine::CollisionCallBack callBack, void* data);
    virtual bool __stdcall intersect(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, engine::CollisionTriangle* collisionTriangle);
};
