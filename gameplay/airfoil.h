#ifndef AIRFOIL_INCLUDED
#define AIRFOIL_INCLUDED

#include "imath.h"

class Airfoil {
public:
	Airfoil();
	Airfoil(PxRigidDynamic *phActor);
	Airfoil(PxRigidDynamic *phActor, float Aw, float Cd, PxVec3 pos);
	~Airfoil();

private:
	float		_Aw;		// wing area (m^2)
	float		_Cd;		// drag coefficient (m^2)
	float		_Pa;		// kg/m^3
	PxVec3		_pos;		// airfoil position (m)
	PxRigidDynamic*	_phActor;	// physics actor
	float		_AOAadd;	// AOA addition to match airfoil leading edge direction
public:
	void setWingArea(float Aw);
	void setDragCoeff(float Cd);
	void setAirPressure(float Pa);	
	void setAOAadd(float AOAadd);	
	void setPosition(PxVec3 pos);

	// dir - body normal
	void calcForce(PxVec3 dir, float Pa = -1.0f);

private:
	float lift(float aoa);
	float drag(float aoa);
	float AOA(PxVec3 &airspeed);
	void calcAirPressure();
};

#endif