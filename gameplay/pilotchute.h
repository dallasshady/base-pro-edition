
#ifndef PILOTCHUTE_SIMULATOR_INCLUDED
#define PILOTCHUTE_SIMULATOR_INCLUDED

#include "headers.h"
#include "scene.h"
#include "callback.h"
#include "sensor.h"
#include "scene.h"
#include "imath.h"
#include "gear.h"

enum PilotchuteState
{
    pcsStowed, // stowed (hidden, simulation is off)
    pcsPulled, // pulled (in hand, simulation is off)
    pcsDropped // dropped (simulation is on)
};

struct PilotchuteRenderCallback
{
private:
    static engine::IAtomic* onRenderPilotchute(engine::IAtomic* atomic, void* data);
    void restoreAtomic(engine::IAtomic* atomic);
public:
    engine::ITexture* canopyTexture;
public:
    PilotchuteRenderCallback() : canopyTexture(NULL) {}
    virtual ~PilotchuteRenderCallback();
public:
    void setTexture(const char* textureName, const char* resourceName);
    void setTexture(database::Canopy* gearRecord);
    void setTexture(Gear* gear);    
    void apply(engine::IClump* clump);
    void restore(engine::IClump* clump);
};

class PilotchuteSimulator : public Actor
{
private:
    database::Canopy*         _canopyInfo; // canopy info
    database::Pilotchute*     _gearRecord; // database record of gear
    engine::IClump*           _pilotClump; // pilotchute model
    engine::IClump*           _cordClump;  // strand cord model
    float                     _cordLength; // lenght of strand cord
    PilotchuteState           _state;
    engine::IFrame*           _poseModeFrame;
	PxRigidDynamic*           _phConnected;      // pilot connected to this actor
    engine::IFrame*           _phConnectedFrame; // pilot connected to actor by this frame
    PxVec3                    _phLocalAnchor;    // local anchor of connected actor
    MatrixConversion          _mcPilotchute;     // pilot matrix conversion
    PxRigidDynamic*            _phPilotchute;     // pilot physics simulator
    engine::IFrame*           _pullFrame;        // pulled pilot will be holded at this frame
	PxDistanceJoint*          _phJoint;          // connection joint
    PxVec3                    _phPilotAnchor;    // pilot local anchor
    PilotchuteRenderCallback* _renderCallback;   // render callback
	
	PxRigidDynamic*			  _nx;				 // dead weight actor
	bool					  _freebag;			 // is pilotchute a freebag (disconnects after full inflation)
	float					  _inflation;		 // inflation level (0..1)
	bool					  _collapsed;		 // if true, the pilotchute will never inflate obove 20%
protected:
    // Actor
    virtual void onUpdateActivity(float dt);
    virtual void onUpdatePhysics(void);
public:
    // class implementation
    PilotchuteSimulator(Actor* jumper, database::Canopy* canopyInfo, database::Pilotchute* gearRecord);
    virtual ~PilotchuteSimulator();
    // class behaviour
    void connect(PxRigidDynamic* actor, engine::IFrame* frame, const PxVec3& localAnchor);
	void disconnect();
    void pull(engine::IFrame* frame);
    void pull(Matrix4f pose);
    void drop(const PxVec3& velocity);
    void setInflation(float value);
	float getInflation(void);
	void setFreebag(bool value);
	bool isFreebag(void);
public:
    inline PilotchuteState getState(void) { return _state; }
    inline database::Pilotchute* getGearRecord(void) { return _gearRecord; }
    inline PxRigidDynamic* getPhActor(void) { return _phPilotchute; }

	inline bool isConnected(void) { return ( _phConnected != NULL ); }
    inline bool isPulled(void) { return ( _pullFrame != NULL ); }
    inline bool isDropped(void) { return (_phPilotchute != NULL); }
    inline bool isOpened(void) { return _collapsed || _pilotClump->getAnimationController()->isEndOfAnimation( 0 ); }
public:
    // model management
    static engine::IAtomic* getCollision(engine::IClump* clump);
};

#endif