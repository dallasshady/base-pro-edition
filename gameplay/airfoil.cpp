#include "airfoil.h"

Airfoil::Airfoil() {
	_Aw = 1.0f;
	_Cd = 1.0f;
	_Pa = -1.0f;
	_pos = PxVec3(0,0,0);
	_AOAadd = 0.0f;
	_phActor = NULL;
}
Airfoil::Airfoil(PxRigidDynamic *phActor) {
	Airfoil();
	_phActor = phActor;
}
Airfoil::Airfoil(PxRigidDynamic *phActor, float Aw, float Cd, PxVec3 pos) {
	_Aw = Aw;
	_Cd = Cd;
	_Pa = -1.0f;
	_pos = pos;
	_AOAadd = 0.0f;
	_phActor = phActor;
}
Airfoil::~Airfoil() {

}

void Airfoil::setWingArea(float Aw) {
	_Aw = Aw;
}
void Airfoil::setDragCoeff(float Cd) {
	_Cd = Cd;
}
void Airfoil::setAirPressure(float Pa) {
	_Pa = Pa;
}
void Airfoil::setPosition(PxVec3 pos) {
	_pos = pos;
}
void Airfoil::setAOAadd(float AOAadd) {
	_AOAadd = AOAadd;
}
void Airfoil::calcAirPressure() {
	PxVec3 pos = _phActor->getGlobalPose().p;
	_Pa = pos.y <= 10000.0f ? (1.196f - 0.0000826f * pos.y) : (0.27f);
}

void Airfoil::calcForce(PxVec3 dir, float Pa) {
	return;
	if (!_phActor) return;
	if (_phActor->isSleeping()) return;

	float Pressure = Pa;
	if (Pressure == -1.0f) {
		if (_Pa == -1.0f) {
			calcAirPressure();
		}
		Pressure = _Pa;
	}

	// get base drag (dynamic air pressure)
	PxVec3 pointVel = _phActor->getLinearVelocity();
	//pointVel = _phActor->getLocalPointVelocity(_pos);
	float Vsq = pointVel.magnitudeSquared();
	float Dyn = 0.5f * Pressure * Vsq * _Cd * _Aw;
	//getCore()->logMessage("pointVel: %2.5f, %2.5f, %2.5f", pointVel.x, pointVel.y, pointVel.z);
	//getCore()->logMessage("Press: %2.5f; Vsq: %2.5f; Cd: %2.5f; Aw: %2.5f", Pressure, Vsq, _Cd, _Aw);

	// get angle of attack
	float aoa = AOA(pointVel);

	const float piover180 = 0.0174532925f;

	const float aoa_reduce = 180.0f;
	while(aoa >= aoa_reduce) aoa -= aoa_reduce;
	while(aoa <= -aoa_reduce) aoa += aoa_reduce;

	// DRAG
	// Fd = cosf(aoa*piover180)^2*(1-Fdmax)+Fdmin
	// Fd = cos(aoa*(pi/180))^2*0.6+0.2
	const float Fdmax = 1.0f;
	const float Fdmin = 0.6f;
	float Fd = Dyn * (powf(cosf(aoa*piover180),2)*(Fdmax-Fdmin)+Fdmin);

	// LIFT
	const float Flmax = 0.7f;
	const float Flmin = 0.0f;
	float Fl = Dyn * (powf(sinf(aoa*piover180*4.0f),2)*(Flmax-Flmin)+Flmin);
	
	//getCore()->logMessage("Press: %2.5f; LiftCoeff: %2.5f, DragCoeff: %2.5f", Pressure, (powf(sinf(aoa*piover180*4.0f),2)*(Flmax-Flmin)+Flmin), powf(cosf(aoa*piover180),2)*(Fdmax-Fdmin)+Fdmin);

	// add force multiplied by normalized velocity vector
	pointVel.normalize();
	PxVec3 Drag = Fd * -pointVel;
	
	PxVec3 pointVel90CCWx = PxVec3(pointVel.x, pointVel.z, -pointVel.y);
	pointVel90CCWx.normalize();

	PxVec3 Lift = Drag;
	Lift = Lift.cross(dir);
	Lift = Lift.cross(Drag);
	Lift.normalize();
	Lift = PxVec3(0.0f, 1, 1);
	//Lift.normalize();
	Lift *= Fl;

	//getCore()->logMessage("AOA: %2.1f; Drag: %2.5f, %2.5f, %2.5f", aoa, Drag.x, Drag.y, Drag.z);
	//getCore()->logMessage("AOA: %2.1f; Lift: %2.5f, %2.5f, %2.5f (%2.5f)", aoa, Lift.x, Lift.y, Lift.z, Lift.magnitude());
	//getCore()->logMessage("");
	PxRigidBodyExt::addForceAtLocalPos(*_phActor, Drag, _pos);
	PxRigidBodyExt::addLocalForceAtLocalPos(*_phActor, Lift, _pos);
}


float Airfoil::lift(float aoa) {
	if (aoa > 90.0f) aoa -= 90.0f;

	if (aoa <= -8) return 0;
	if (aoa <= 17) {		// -8:17
		// x * 0.0682 + 0.34
		return aoa*0.0682f + 0.34f;

	} else if (aoa <= 19) { // 17:19
		//-((((-x+38)*0.117)-2.213)^2.213) + 1.55
		return -powf(((-aoa+38)*0.117f)-2.213f, 2.213f) + 1.55f;
	
	} else if (aoa <= 22) { // 19:22
		//-(((x*0.117)-2.213)^2.213) + 1.55
		return -powf((aoa*0.117f)-2.213f, 2.213f) + 1.55f;
	//} else if (aoa <= 50) { // 22:50
	//	//(x-22.0)*-0.05 + 1.419679
	//	return (aoa-22.0f)*-0.05f + 1.419679f;
	} else if (aoa <= 90) {	// 22:90
		//(x^0.87-22.0)*-0.05 + 1.419679
		return (powf(aoa,0.87f)-22.0f)*-0.05f + 1.419679f;
	} else {
		return 0;
	}
}
float Airfoil::drag(float aoa) {
	aoa = fabs(aoa);
	
	//((x+5)*0.032)^3.04 * 0.43 + 0.01
	float dragCoeff = powf(((aoa+5)*0.032f), 3.04f) * 0.43f + 0.01f;
	if (dragCoeff > 1.0f) dragCoeff = 1.0f;
	return dragCoeff;
}
float Airfoil::AOA(PxVec3 &airspeed) {
	PxVec3 motionDir = _phActor->getLinearVelocity(); motionDir.normalize();
	
	//PHYSX3
	PxVec3 canopyDown  = _phActor->getGlobalPose().q.getBasisVector2(); canopyDown.normalize();
	float relativity = canopyDown.dot( motionDir );
	return relativity * 90.0f;
	
	PxTransform freeFallPose = _phActor->getGlobalPose();
	PxVec3 x = freeFallPose.q.getBasisVector0();
    PxVec3 y = freeFallPose.q.getBasisVector2();
    PxVec3 z = -freeFallPose.q.getBasisVector1();

    //PxVec3 hz = z; hz.y = 0; hz.normalize();
	PxVec3 velocity = airspeed;
	velocity.normalize();
    float aoa = ::calcAngle( z, velocity, x );

	//return aoa;


}
