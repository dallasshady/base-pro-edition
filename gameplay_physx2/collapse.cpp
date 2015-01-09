
#include "headers.h"
#include "canopy.h"

void CanopySimulator::CollapseArea::reset(void)
{
    center.set( 0,0,0 );
    radius = 0;
}

void CanopySimulator::CollapseArea::setup(NxVec3 areaCenter, float areaRadius)
{
    center = areaCenter;
    radius = areaRadius;
}

void CanopySimulator::CollapseArea::unite(CollapseArea* anotherArea, database::Canopy* gear)
{
    unite( anotherArea->center, anotherArea->radius, gear );
}

void CanopySimulator::CollapseArea::unite(NxVec3 anotherAreaCenter, float anotherAreaRadius, database::Canopy* gear)
{
    // center offset interpolation
    float i1 = radius / ( radius + anotherAreaRadius );
    float i2 = anotherAreaRadius / ( radius + anotherAreaRadius );
    center = center * i1 + anotherAreaCenter * i2;
    radius += anotherAreaRadius;
    if( radius > gear->Cradius ) radius = gear->Cradius;
}

bool CanopySimulator::CollapseArea::canUnite(CollapseArea* anotherArea, database::Canopy* gear)
{
    return canUnite( anotherArea->center, anotherArea->radius, gear );
}

bool CanopySimulator::CollapseArea::canUnite(NxVec3 anotherAreaCenter, float anotherAreaRadius, database::Canopy* gear)
{
    // first rule : areas should intersects
    NxVec3 distance;
    distance = center - anotherAreaCenter;
    if( distance.magnitude() > radius + anotherAreaRadius ) return false;
    // second rule: united sphere should be in gear limits
    NxVec3 unitedCenter = ( center + anotherAreaCenter ) * 0.5f;
    float unitedRadius1 = ( unitedCenter - center ).magnitude();
    float unitedRadius2 = ( unitedCenter - anotherAreaCenter ).magnitude();
    float unitedRadius  = unitedRadius1 > unitedRadius2 ? unitedRadius1 : unitedRadius2;
    return unitedRadius <= gear->Cradius;
}