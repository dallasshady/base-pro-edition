#ifndef BASE_JUMPER_INCLUDED
#define BASE_JUMPER_INCLUDED

//#include "headers.h"
#include "scene.h"
#include "callback.h"
#include "sensor.h"
#include "scene.h"
#include "imath.h"
#include "character.h"
#include "pilotchute.h"
#include "canopy.h"
#include "airplane.h"
#include "hud.h"
#include "pose.h"

class Jumper;
/**
 * base jumper rendering: contains textures & callback methods
 */

struct JumperRenderCallback
{
private:
    static engine::IAtomic* onRenderFace(engine::IAtomic* atomic, void* data);
    static engine::IAtomic* onRenderHand(engine::IAtomic* atomic, void* data);
    static engine::IAtomic* onRenderHelmet(engine::IAtomic* atomic, void* data);
    static engine::IAtomic* onRenderBody(engine::IAtomic* atomic, void* data);
    void restoreAtomic(engine::IAtomic* atomic);
public:
    engine::ITexture* faceTexture;
    engine::ITexture* helmetTexture;
    engine::ITexture* suitTexture;
    engine::ITexture* rigTexture;
public:
    JumperRenderCallback() : faceTexture(NULL), helmetTexture(NULL), suitTexture(NULL), rigTexture(NULL) {}
    virtual ~JumperRenderCallback();
public:
    void setFace(unsigned int faceId);
    void setFace(const char* textureName, const char* resourceName);
    void setHelmet(unsigned int helmetId);
    void setSuit(unsigned int suitId);
    void setRig(unsigned int rigId);
    void apply(engine::IClump* clump);
    void restore(engine::IClump* clump);
};

struct RiserRenderCallback
{
private:
    static engine::IAtomic* onRenderRiser(engine::IAtomic* atomic, void* data);
public:
    bool showSlider;
public:
    RiserRenderCallback() : showSlider(true) {}
public:
    void apply(engine::IClump* clump);
};

/**
 * base jumper states & constants
 */

enum JumperPhase
{
	jpCreating,		// jumper is still getting created, so leave it alone
    jpRoaming,     // jumper is roam the exit point enclosure
    jpFreeFalling, // jumper is fall
    jpFlight       // jumper is flight
};

const float jumperRoamingSphereSize = 25.0f;

/**
 * CatToy is abstract AI target object
 */

class CatToy
{
public:
    virtual ~CatToy() {}
public:
    // return virtues of CatToy 
    virtual Virtues* getVirtues(void) = 0;
    // return current CatToy phase
    virtual JumperPhase getPhase(void) = 0;
    // return current CatToy modifier
    virtual bool getModifier(void) = 0;
    // return current CatToy pose
    virtual Matrix4f getCurrentPose(void) = 0;
    // return jump pose of CatToy (equal to getCurrentPose in roaming phase)
    virtual Matrix4f getJumpPose(void) = 0;
    // updates CatToy state
    virtual void update(float dt) = 0;
    // disconnection method (invokes when connected jumper ends its existence)
    virtual void disconnect(void) = 0;
    // returns time to jump action (or negative value, if this value undetermined)
    virtual float getTimeToJump(void) = 0;
public:
    // creates CatToy that just wraps existing jumper
    static CatToy* wrap(Jumper* jumper);
    // creates CatToy that saves jumper (ghost) states each simulation step
    static CatToy* saveGhostCatToy(Jumper* jumper, const char* filename);
    // creates CatToy that replays jumper (ghost) states each simulation step
    static CatToy* loadGhostCatToy(const char* filename);
};

/**
 * base jumper classes
 */

struct SpinalCord
{
public:
    float left;      // context left channel
    float right;     // context right channel
    float up;        // fwd/up channel
    float down;      // back/down channel
    float leftWarp;  // left warp channel
    float rightWarp; // right warp channel
    float leftRearRiser;  // left rear riser channel
    float rightRearRiser; // right rear riser channel
	float rearBrake; // rear brake channel
    bool  phase;     // phase channel (jump/pull)
    float  modifier;  // context modifier channel (run/track)
    float  wlo;       // wlo toggles channel
    float  hook;      // hook knife channel
	bool  cutAway;	 // cut away channel
	bool  pullReserve; // pull reserve channel

	// reserve control channels
	float leftReserve; 
	float rightReserve;
	float leftReserveWarp;
	float rightReserveWarp;
	float leftReserveRearRiser;
	float rightReserveRearRiser;

	// TRIGGERS (bool values that are true when button is pressed, false otherwise)
    bool trigger_left;      // context left channel
    bool trigger_right;     // context right channel
    bool trigger_up;        // fwd/up channel
    bool trigger_down;      // back/down channel
    bool trigger_leftWarp;  // left warp channel
    bool trigger_rightWarp; // right warp channel
    bool trigger_leftRearRiser;  // left rear riser channel
    bool trigger_rightRearRiser; // right rear riser channel
	bool trigger_rearBrake; // rear brake channel
    bool  trigger_phase;     // phase channel (jump/pull)
    bool  trigger_modifier;  // context modifier channel (run/track)
    bool  trigger_wlo;       // wlo toggles channel
    bool  trigger_hook;      // hook knife channel
	bool  trigger_cutAway;	 // cut away channel
	bool  trigger_pullReserve; // pull reserve channel

	// reserve control channels
	bool trigger_leftReserve; 
	bool trigger_rightReserve;
	bool trigger_leftReserveWarp;
	bool trigger_rightReserveWarp;
	bool trigger_leftReserveRearRiser;
	bool trigger_rightReserveRearRiser;

public:
    SpinalCord() : left(0), right(0), up(0), down(0), leftWarp(0), rightWarp(0), leftRearRiser(0), rightRearRiser(0),
                   phase(0), modifier(0), wlo(0), hook(0), cutAway(0), pullReserve(0),
				   leftReserve(0), rightReserve(0), leftReserveWarp(0), rightReserveWarp(0), leftReserveRearRiser(0), rightReserveRearRiser(0), rearBrake(0),
					
				   trigger_left(false), trigger_right(false), trigger_up(false), trigger_down(false), trigger_leftWarp(false), trigger_rightWarp(false), trigger_leftRearRiser(false), trigger_rightRearRiser(false),
                   trigger_phase(false), trigger_modifier(false), trigger_wlo(false), trigger_hook(false), trigger_cutAway(false), trigger_pullReserve(false),
				   trigger_leftReserve(false), trigger_rightReserve(false), trigger_leftReserveWarp(false), trigger_rightReserveWarp(false), trigger_leftReserveRearRiser(false), trigger_rightReserveRearRiser(false), trigger_rearBrake(false)
	{}
public:
    void mapActionChannels(void);
    void reset(void);
};

enum SignatureType
{
    stNone,
    stBrief,
    stFull
};

class Jumper : public Character
{
private:
    friend class WalkForward;
    friend class WalkBackward;
	float _dt;
	PxDistanceJoint		*_phJoint;			// joint with other Jumpers
	Jumper				*_tandemStudent;	// pointer to the student in a tandem ride
	PxDistanceJoint		*_tandemJoint[4];	// joint with tandem students
	Altimeter			*_altimeter;
	Variometer			*_variometer;
	bool _inDropzone;
	// pose id
	int _default_pose;
	
public:
	void consumePacket(NetworkData *packet);
    typedef std::list<Jumper*> JumperL;
    typedef JumperL::iterator JumperI;
    typedef std::list<CatToy*> CatToyL;
    typedef CatToyL::iterator CatToyI;
private:
    /**
     * general jumper action
     */
    class JumperAction : public Character::Action
    {
    protected:
        Jumper* _jumper;
    public:
        JumperAction(Jumper* jumper);
    };
    /**
     * airplane idle action
     */
    class AirplaneIdle : public JumperAction
    {
    public:
        // class implementation
        AirplaneIdle(Jumper* jumper);
        virtual ~AirplaneIdle();
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void) {}
    };
    /**
     * airplane jump action
     */
    class AirplaneJump : public JumperAction
    {
    private:
        MatrixConversion* _matrixConversion;
		PxRigidDynamic*          _phActor;
    public:
        // class implementation
        AirplaneJump(Jumper* jumper, PxRigidDynamic* actor, MatrixConversion* mc);
        virtual ~AirplaneJump();
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void) {}
    };
    /**
     * turn action
     */
    class Turn : public JumperAction
    {
    private:
        float _side;
    public:
        // class implementation
        Turn(Jumper* jumper, float side);
        virtual ~Turn();
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void) {}
    };
    /**
     * move actions
     */
    class WalkForward : public JumperAction
    {
    private:
        Enclosure* _enclosure;
        Vector3f   _velocity;
        bool       _running;
    public:
        // class implementation
        WalkForward(Jumper* jumper);
        virtual ~WalkForward();
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void) {}
    public:
        float getVelocity(void);
    };
    class WalkBackward : public JumperAction
    {
    private:
        Enclosure* _enclosure;
    public:
        // class implementation
        WalkBackward(Jumper* jumper);
        virtual ~WalkBackward();
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void) {}
    };
    /**
     * freefall actions
     */
    class StandingJump : public JumperAction
    {
    private:
        MatrixConversion* _matrixConversion;
        PxRigidDynamic*          _phActor;
    public:
        // class implementation
        StandingJump(Jumper* jumper, PxRigidDynamic* actor, MatrixConversion* mc);
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void) {}
    };
    class RunningJump : public JumperAction
    {
    private:
        MatrixConversion* _matrixConversion;
        PxRigidDynamic*          _phActor;
        float             _vel;
        Vector3f             _prevPelvisPos; // position tracker
        Vector3f             _prevPelvisUp;  // angle tracker
        Vector3f             _prevPelvisAt;  // axis tracker
    public:
        // class implementation
        RunningJump(Jumper* jumper, PxRigidDynamic* actor, MatrixConversion* mc, float vel);
		~RunningJump();
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void) {}
    };    
    class OutOfControl : public JumperAction
    {
    private:
        MatrixConversion* _matrixConversion;
        PxRigidDynamic*          _phActor;
        float             _time;
    public:
        // class implementation
        OutOfControl(Jumper* jumper, PxRigidDynamic* actor, MatrixConversion* mc, float time);
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void);
    };
    class Tracking : public JumperAction
    {
    protected:
        PxRigidDynamic*          _phActor;
        MatrixConversion* _matrixConversion;        
        float             _wing_area;   
        float             _tracking;
        float             _leg_pitch;
		float             _carving_forward;
		float             _carving_sideways;
		float			  _leveling;	// flight levels (fall rate)
		float			  _leveling_higher;	// flight levels (fall rate) slower
		float			  _leveling_lower;	// flight levels (fall rate) faster

	// poses
	public:
		Pose currentPose;
		// regular poses
		Pose ArchPose, HeaddownPose, BackflyPose, SitflyPose;
		// transfer poses
		Pose FrontFlipPose, BackFlipPose, LeftRollPose, RightRollPose;
		bool _transfer_pose;
		int _default_pose;
		int _current_pose;
		

		bool useWingsuit;
		int _machine_state;	// current machine state
		int _machine_state_previous;
		int _machine_state_pre_previous;
		bool _fresh_state;	// true if it's state's first frame

		void changePose(int pose, float rate, bool force = false);
    protected:
        void updateBlending(float dt);
        void updateProceduralAnimation(float dt);
		void initPoses();
		void initPosesWings();
		void machine();
		void machineActions(bool actions[], SpinalCord *spine);

    public:
		void machine_state(int state);
		int machine_state();
		int machine_previuos_state();
		int machine_pre_previuos_state();

        // class implementation        
        Tracking(Jumper* jumper, PxRigidDynamic* phActor, MatrixConversion* mc);
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void);
    public:
        inline float getTrackingModifier(void) { return _tracking; }
    };
    class Pull : public Tracking
    {
    private:
        PilotchuteSimulator* _pilotchute;
		bool				 _pcInHand;
    public:
        // class implementation
        Pull(Jumper* jumper, PxRigidDynamic* phActor, MatrixConversion* mc,  PilotchuteSimulator* pc, PxVec3 localAnchor);
        // Action
        virtual void update(float dt);
    };
    class PullReserve : public Tracking
    {
    private:
        PilotchuteSimulator* _pilotchute;
    public:
        // class implementation
        PullReserve(Jumper* jumper, PxRigidDynamic* phActor, MatrixConversion* mc,  PilotchuteSimulator* pc, PxVec3 localAnchor);
        // Action
        virtual void update(float dt);
    };
    /**
     * flight actions
     */
    class CanopyOpening : public JumperAction
    {
    private:
        MatrixConversion*    _matrixConversion;
        PxRigidDynamic*      _phActor;
        PilotchuteSimulator* _pilotchute;
        CanopySimulator*     _canopy;
        PxVec3               _frontLeftAnchor;
        PxVec3               _frontRightAnchor;
        PxVec3               _rearLeftAnchor;
        PxVec3               _rearRightAnchor;
        float                _initialLD;
    public:
        // class implementation
        CanopyOpening(Jumper* jumper, PxRigidDynamic* phFreeFall, PxRigidDynamic* phFlight, MatrixConversion* mcFlight, PilotchuteSimulator* pc, CanopySimulator* c, PxVec3 fla, PxVec3 fra, PxVec3 rla, PxVec3 rra);
        virtual ~CanopyOpening();
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void);
    public:
        // class behaviour
        bool isCriticalAnimationRange(void);
    };
    class Linetwists : public JumperAction
    {
    private:
        MatrixConversion* _matrixConversion;
        PxRigidDynamic*   _phActor;
    public:
        // class implementation
        Linetwists(Jumper* jumper, PxRigidDynamic* phFlight, MatrixConversion* mcFlight);
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void);
    };
    class Flight : public JumperAction
    {
    private:
        MatrixConversion*     _matrixConversion;
        PxRigidDynamic*       _phActor;
        engine::AnimSequence* _targetSequence;
    private:
        void setAnimation(engine::AnimSequence* sequence);
    public:
        // class implementation
        Flight(Jumper* jumper, PxRigidDynamic* phFlight, MatrixConversion* mcFlight);
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void);
    };
    class Landing : public JumperAction
    {
    private:
        MatrixConversion* _matrixConversion;
        PxRigidDynamic*   _phActor;
        float             _Kfr;
    private:
        void startCanopySuppression(void);
    public:
        // class implementation
        Landing(Jumper* jumper, PxRigidDynamic* phFlight, MatrixConversion* mcFlight);
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void);
        virtual float getDamageFactor(void);
    };
    class BadLanding : public JumperAction
    {
    private:
        MatrixConversion*  _matrixConversion;
        PxRigidDynamic*           _phActor;        
        float              _Kfr;
    public:
        // class implementation
        BadLanding(Jumper* jumper, PxRigidDynamic* phFlight, MatrixConversion* mcFlight);
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void);
    };
    /**
     * acrobatics
     */
    class Flip : public JumperAction
    {
    private:        
        PxRigidDynamic*          _phActor;
        MatrixConversion* _matrixConversion;
    public:
        // class implementation
        Flip(Jumper* jumper, PxRigidDynamic* phActor, MatrixConversion* mc, float blendTime);
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void);
    public:
        static engine::AnimSequence* getFlipSequence(void);
    };
    class FlipJump : public JumperAction
    {
    private:        
        PxRigidDynamic*             _phActor;
        MatrixConversion*    _matrixConversion;
        float                _criticalPeriod;
        Vector3f             _prevPelvisPos; // position tracker
        Vector3f             _prevPelvisUp;  // angle tracker
        Vector3f             _prevPelvisAt;  // axis tracker
		bool				 _allowModifier; // becomes true when modifier is released after the jump
    public:
        // class implementation
        FlipJump(Jumper* jumper, PxRigidDynamic* phActor, MatrixConversion* mc, engine::AnimSequence* sequence, float animSpeed, float criticalPeriod);
        virtual ~FlipJump();
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void);
    };
    class SideStepJump : public JumperAction
    {
    private:        
        PxRigidDynamic*             _phActor;
        MatrixConversion*    _matrixConversion;
        float                _criticalPeriod;
        Vector3f             _prevPelvisPos; // position tracker
        Vector3f             _prevPelvisUp;  // angle tracker
        Vector3f             _prevPelvisAt;  // axis tracker
    public:
        // class implementation
        SideStepJump(Jumper* jumper, PxRigidDynamic* phActor, MatrixConversion* mc, engine::AnimSequence* sequence, float animSpeed, float criticalPeriod);
        virtual ~SideStepJump();
        // Action
        virtual void update(float dt);
        virtual void updatePhysics(void);
    };
private:    
    gui::IGuiWindow*     _signature;       // jumper HUD
    Virtues*             _virtues;         // character virtues
    Airplane*            _airplane;        // roaming airplane
    engine::IFrame*      _airplaneExit;    // exit point frame in airplane mode
    Enclosure*           _enclosure;       // roaming enclosure
    JumperPhase          _phase;           // current jumper phase
    Sensor*              _sensor;          // personal sensor for miscellaneous usage
    float                _headIncidence;   // procanim: head vertical incidence
    float                _angleLR;         // procanim: curr. head turn angle in LR plane
    float                _angleUD;         // procanim: curr. head turn angle in UD plane
    MatrixConversion     _mcFreeFall;      // free fall matrix conversion
    PxRigidDynamic*             _phFreeFall;      // free fall physics simulator
    MatrixConversion     _mcFlight;        // flight matrix conversion
    PxRigidDynamic*             _phFlight;        // flight physics simulator
    MatrixConversion     _mcPelvisToClump; // pelvis-to-clump matrix conversion
    MatrixConversion     _mcChestToClump;  // chest-to-clump matrix conversion
    bool                 _isContacted;
    PxVec3               _frontLeftAnchor;
    PxVec3               _frontRightAnchor;
    PxVec3               _rearLeftAnchor;
    PxVec3               _rearRightAnchor;
    PxVec3               _pilotAnchor;
    bool                 _player;
    SpinalCord*          _spinalCord;
    PilotchuteSimulator* _pilotchuteSimulator;
    CanopySimulator*     _canopySimulator;
    PilotchuteSimulator* _pilotchuteReserveSimulator;
    CanopySimulator*     _canopyReserveSimulator;
    unsigned int         _bcStep;     // burden calculator: counter
    PxVec3               _bcPrevVel;  // burden calculator: previous velocity
    PxVec3               _bcBurden;   // burden calculator: overburden in mts/sq.sec.
    float                _distanceToAbyss; // roaming register
    float                _jerkLimit;       // limit of adrenaline rush by jerking on EP
    float                _fallLimit;       // limit of adrenaline rush by fall
    float                _adrenaline;      // adrenaline level
    float                _pulse;           // pulse
    bool                 _hazardState;     // jumper is in hazard state
    float                _shock;           // pain shock level
    bool                 _isStuck;         // true if jumper is stuck during roaming phase
    Matrix4f             _jumpPose;        // contains last roaming pose
    bool                 _isOverActivity;  // jumper activion is over
    float                _jumpTime;        // total jump time including frefall and flight
    CatToyL              _catToys;         // cattoy objects wrapping this jumper
    CatToy*              _saveGhost;       // internal cattoy (ghost builder)
    bool                 _expIsOverrided;  // true, if experience flag was overrided
    bool                 _expFlag;         // previous value of experience flag 
    SignatureType        _signatureType;   // signature is full, brief or disabled
    bool                 _phaseIsEnabled;  // jump/pull is enabled (player only!)
public:
	gui::IGuiWindow*	_debug_window;
private:
    bool               _useForcedEquipment;
    Virtues::Equipment _layoffEquipment;
private:
    JumperRenderCallback _renderCallback;
    RiserRenderCallback  _riserCallback;
private:    
    static JumperL _jumperL;
private:
    // private behaviour    
    void lookWhereYouFly(Vector3f direction, float dt);
    void lookAtPoint(Vector3f point, float dt);
    void onRoaming(float dt);
    void onFreeFalling(float dt);
    void onFlight(float dt);
    void onRoamingProcAnim(float dt);
    void onFreeFallingProcAnim(float dt);
    void onFlightProcAnim(float dt);
    void onUpdateHealthStatus(void);
    void onUpdateSkills(void);
    void onDamage(float normalForce, float frictionForce, float velocity);
    void onCameraIsActual(void);

	void cutAwayMainCanopy(void);
	void fireReserveCanopy(void);
public:
    // actor abstracts
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
    //PHYSX3
	//virtual void onContact(NxContactPair &pair, PxU32 events);
    virtual void onUpdateActivity(float dt);
    virtual void onUpdatePhysics(void);
    virtual Matrix4f getPose(void) { return _clump->getFrame()->getLTM(); }
    virtual Vector3f getVel(void);

	virtual bool animBodyPart(const char *frame_title, Vector3f axis, float angle);
public:
    // class implementation
    Jumper(Actor* parent, Airplane* airplane, Enclosure* enclosure, Virtues* virtues, SpinalCord* spinalCord, Virtues::Equipment* forcedEquipment);
    virtual ~Jumper();
    // class behaviour
	Jumper *getNextJumper(void);
    float getDistanceToAbyss(void);
    bool getDistanceToImpact(float& inOutDistance);
    bool getDistanceToSurface(float& inOutDistance);
    void setLicensedCharacterAppearance(void);
    void initOverburdenCalculator(PxVec3& velocity);
    void damage(float normalForce, float frictionForce, float velocity);
    // cat toy support
    void registerCatToy(CatToy* catToy);
    void unregisterCatToy(CatToy* catToy);
public:
	void InDropzone(bool dropzone);
	bool InDropzone();
	void setTandemStudent(Jumper *student);
	Jumper *getTandemStudent(void);

	void beginFreefall(void);

	void setDefaultPose(int pose);
	int getDefaultPose(void);

    inline JumperPhase getPhase(void) { return _phase; }
    inline Airplane* getAirplane(void) { return _airplane; }
    inline engine::IFrame* getAirplaneExit(void) { return _airplaneExit; }
    inline Enclosure* getEnclosure(void) { return _enclosure; }
    inline SpinalCord* getSpinalCord(void) { return _spinalCord; }
    inline Virtues* getVirtues(void) { return _virtues; }
    inline PilotchuteSimulator* getPilotchuteSimulator(void) { return _pilotchuteSimulator; }
	inline CanopySimulator* getCanopySimulator(void) { return getDominantCanopy(); }
	CanopySimulator *getDominantCanopy(void);
	PilotchuteSimulator *getDominantPilotChute(void);
	inline PilotchuteSimulator* getPilotchuteReserveSimulator(void) { return _pilotchuteReserveSimulator; }
    inline CanopySimulator* getCanopyReserveSimulator(void) { return _canopyReserveSimulator; }
    inline PxVec3 getOverburden(void) { return _bcBurden; }
    inline PxRigidDynamic* getFreefallActor(void) { return _phFreeFall; }
    inline PxRigidDynamic* getFlightActor(void) { return _phFlight; }
    inline MatrixConversion* getPelvisToClumpConversion(void) { return &_mcPelvisToClump; }
    inline MatrixConversion* getChestToClumpConversion(void) { return &_mcChestToClump; }
    inline float getAdrenaline(void) { return _adrenaline; }
    inline float getPulse(void) { return _pulse; }
    inline float getShock(void) { return _shock; }
    inline bool isPlayer(void) { return _player; }
    inline bool isOverActivity(void) { return _isOverActivity; }
    inline bool isStuck(void) { return _isStuck; }
    inline bool isFlight(void) { if( _phase == jpFlight ) return actionIs(Flight); else return false; }
    inline bool isLanding(void) { if( _phase == jpFlight ) return actionIs(Landing); else return false; }
    inline bool isBadLanding(void) { if( _phase == jpFlight ) return actionIs(BadLanding); else return false; }
    inline const Matrix4f& getJumpPose(void) { return _jumpPose; }
    inline PxVec3 getLocalPilotAnchor(void) { return _pilotAnchor; }
    inline SignatureType getSignatureType(void) { return _signatureType; }
    inline void setSignatureType(SignatureType value) { _signatureType = value; }
    inline bool phaseIsEnabled(void) { return _phaseIsEnabled; }
    inline void enablePhase(bool flag) { _phaseIsEnabled = flag; }
	inline float getDeltaTime() { return _dt; };
public:
    // jumper model management stuff
    static void hideEffectors(engine::IClump* clump);
    static void placeCord(engine::IClump* cord, const Vector3f& p0, const Vector3f& p1, float width = 1.0f);
    static void placeCord(Matrix4f& instanceMatrix, const Vector3f& p0, const Vector3f& p1, float width = 1.0f);
    static unsigned int getNumHeads(engine::IClump* clump);
    static engine::IFrame* getChestFrame(engine::IClump* clump);
    static engine::IFrame* getPelvisFrame(engine::IClump* clump);
    static engine::IFrame* getHeadFrame(engine::IClump* clump);
    static engine::IAtomic* getHead(engine::IClump* clump, unsigned int headId);
    static engine::IAtomic* getRisers(engine::IClump* clump);
    static void setHead(engine::IClump* clump, unsigned int headId, Jumper* jumper);
    static unsigned int getNumHelmets(engine::IClump* clump);
    static engine::IAtomic* getHelmet(engine::IClump* clump, unsigned int helmetId);
    static void setHelmet(engine::IClump* clump, unsigned int helmetId, Jumper* jumper);
    static void setNoHelmet(engine::IClump* clump);
    static engine::IAtomic* getLeftHand(engine::IClump* clump);
    static engine::IAtomic* getRightHand(engine::IClump* clump);
    static engine::IAtomic* getLeftEye(engine::IClump* clump);
    static engine::IAtomic* getRightEye(engine::IClump* clump);
    static engine::IAtomic* getLeftRing(engine::IClump* clump);
    static engine::IAtomic* getRightRing(engine::IClump* clump);
    static unsigned int getNumBodies(engine::IClump* clump);
    static engine::IAtomic* getBody(engine::IClump* clump, unsigned int bodyId);
    static void setBody(engine::IClump* clump, unsigned int bodyId, Jumper* jumper);
    static engine::IAtomic* getCollisionFF(engine::IClump* clump);
    static engine::IAtomic* getCollisionFC(engine::IClump* clump);
    static engine::IFrame* getBackBone(engine::IClump* clump);
    static engine::IFrame* getLeftLegBone(engine::IClump* clump);
    static engine::IFrame* getRightLegBone(engine::IClump* clump);
    static engine::IFrame* getLineHandJoint(engine::IClump* clump);
    static engine::IFrame* getLineRigJoint(engine::IClump* clump);
    static engine::IFrame* getFrontLeftRiser(engine::IClump* clump);
    static engine::IFrame* getFrontRightRiser(engine::IClump* clump);
    static engine::IFrame* getRearLeftRiser(engine::IClump* clump);
    static engine::IFrame* getRearRightRiser(engine::IClump* clump);
    static engine::IFrame* getPhysicsJointFrontLeft(engine::IClump* clump);
    static engine::IFrame* getPhysicsJointFrontRight(engine::IClump* clump);
    static engine::IFrame* getPhysicsJointRearLeft(engine::IClump* clump);
    static engine::IFrame* getPhysicsJointRearRight(engine::IClump* clump);
    static engine::IFrame* getFirstPersonFrame(engine::IClump* clump);
    static engine::IFrame* getHelmetEquipAnchor(engine::IClump* clump);
    static engine::IFrame* getSuitEquipAnchor(engine::IClump* clump);
    static engine::IFrame* getRigEquipAnchor(engine::IClump* clump);
    static engine::IFrame* getLeftSmokeJetAnchor(engine::IClump* clump);
    static engine::IFrame* getRightSmokeJetAnchor(engine::IClump* clump);
public:
    // tricklist methods
    static Jumper* getPositionConflict(Vector3f position, float* distance = NULL);
    static Jumper* getPositionConflict(engine::IFrame* airplaneExit);
};

#endif