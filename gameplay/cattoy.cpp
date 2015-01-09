
#include "headers.h"
#include "jumper.h"

/**
 * cat toy that just  wrapping existed jumper
 */

class CatToyWrap : public CatToy
{
private: 
    Jumper*     _jumper;
    JumperPhase _phase;
    bool        _modifier;
    Matrix4f    _currentPose;
    Matrix4f    _jumpPose;
public:
    CatToyWrap(Jumper* jumper)
    {
        assert( jumper );
        _jumper = jumper;
        _jumper->registerCatToy( this );
        update( 0.0f );
    }
    virtual ~CatToyWrap()
    {
        if( _jumper ) _jumper->unregisterCatToy( this );
    }
public:
    virtual Virtues* getVirtues(void)
    {
        return _jumper->getVirtues();
    }
    virtual JumperPhase getPhase(void)
    {
        return _phase;
    }
    virtual bool getModifier(void)
    {
        return _modifier;
    }
    virtual Matrix4f getCurrentPose(void)
    {
        return _currentPose;
    }
    virtual Matrix4f getJumpPose(void)
    {
        return _jumpPose;
    }
    virtual void update(float dt)
    {
        if( _jumper )
        {
            _phase       = _jumper->getPhase();
            _modifier    = _jumper->getSpinalCord()->modifier > 0.0f;
            _currentPose = _jumper->getPose();
            if( _phase == ::jpRoaming ) _currentPose[3][1] += ::jumperRoamingSphereSize;
            _jumpPose    = ( _phase == ::jpRoaming ) ? _currentPose : _jumper->getJumpPose();
        }
    }
    virtual void disconnect(void)
    {
        _jumper = NULL;
    }
    virtual float getTimeToJump(void)
    {
        return -1.0f;
    }
};

/**
 * save ghost cat toy (telemetry builder)
 */

const float ghostFrameRate = 0.1f;

struct GhostHeader
{
public:
    unsigned int numFrames; // number of ghost state frames    
    Matrix4f     jumpPose;  // jump pose of ghost (it is unique for entire telemetry)
    Virtues      virtues;   // ghost virtues 
};
struct GhostHeaderLegacy	// (backward compatible)
{
public:
    unsigned int numFrames; // number of ghost state frames    
    Matrix4f     jumpPose;  // jump pose of ghost (it is unique for entire telemetry)
    VirtuesLegacy      virtues;   // ghost virtues (original)
};


struct GhostState
{
public:
    float       time;        // state frame time
    JumperPhase phase;       // basejumper phase state (@see SpinalCord::phase)
    bool        modifier;    // basejumper modifier state (@see SpinalCord::modifier)
    Matrix4f    currentPose; // current basejumper position
};

class CatToySaveGhost : public CatToy
{
private:
    Jumper*     _ghost;
    GhostHeader _ghostHeader;
    GhostState  _ghostState;
    float       _ghostTime;
    float       _ghostFrameTime;
    FILE*       _file;    
public:
    CatToySaveGhost(Jumper* jumper, const char* filename)
    {
        // create jumper connection
        assert( jumper );
        _ghost = jumper;
        _ghost->registerCatToy( this );        
        // open file for writting
        _file = fopen( filename, "wb" ); assert( _file );
        // initialize ghost header
        _ghostHeader.numFrames = 0;
        _ghostHeader.virtues = *_ghost->getVirtues();
        _ghostHeader.jumpPose.set( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );
        // save ghost header
        fwrite( &_ghostHeader, sizeof( GhostHeader ), 1, _file );
        // save initial ghost state
        _ghostFrameTime = _ghostTime = 0.0f;        
        update( 0.0f );
    }
    virtual ~CatToySaveGhost()
    {
        // save actual ghost header 
        fseek( _file, 0, SEEK_SET );
        fwrite( &_ghostHeader, sizeof( GhostHeader ), 1, _file );
        // close file
        fclose( _file );

        // break jumper connection
        if( _ghost ) _ghost->unregisterCatToy( this );
    }
public:
    virtual Virtues* getVirtues(void)
    {
        return &_ghostHeader.virtues;
    }
    virtual JumperPhase getPhase(void)
    {
        return _ghostState.phase;
    }
    virtual bool getModifier(void)
    {
        return _ghostState.modifier;
    }
    virtual Matrix4f getCurrentPose(void)
    {
        return _ghostState.currentPose;
    }
    virtual Matrix4f getJumpPose(void)
    {
        return _ghostHeader.jumpPose;
    }
    virtual void update(float dt)
    {
        if( _ghost )
        {
            // increase timers
            _ghostTime += dt;
            _ghostFrameTime += dt;

            // update cat toy states
            _ghostState.time        = _ghostTime;
            _ghostState.phase       = _ghost->getPhase();
            _ghostState.modifier    = _ghost->getSpinalCord()->modifier > 0.0f;
            _ghostState.currentPose = _ghost->getPose();
            if( _ghostState.phase == ::jpRoaming ) 
            {
                _ghostState.currentPose[3][1] += ::jumperRoamingSphereSize;
            }
            _ghostHeader.jumpPose = ( _ghostState.phase == ::jpRoaming ) ? _ghostState.currentPose : _ghost->getJumpPose();

            // if frame time treshold is overcomed
            if( _ghostTime == 0.0f || _ghostFrameTime > ghostFrameRate )
            {                                
                // save states
                _ghostFrameTime = 0.0f;
                _ghostHeader.numFrames++;
                fwrite( &_ghostState, sizeof( GhostState ), 1, _file );
            }
        }
    }
    virtual void disconnect(void)
    {
        _ghost = NULL;
    }
    virtual float getTimeToJump(void)
    {
        return -1.0f;
    }
};

/**
 * load ghost cat toy (telemetry player)
 */

class CatToyLoadGhost : public CatToy
{
private:
    GhostHeader  _ghostHeader;  // heading infos
    GhostState*  _ghostStates;  // state frames
    unsigned int _startFrameId; // seaching helper
    float        _ghostTime;    // process time
    float        _timeToJump;   // time left to jump action
    float        _inhibitor;    // damping multiplier
    GhostState   _currentState; // interpolated state frame
public:
    CatToyLoadGhost(const char* filename)
    {        
        // open file for reading
        FILE* file = fopen( filename, "rb" ); assert( file );

		// reset file
		fseek (file , 0 , SEEK_SET);

		/// Try reading new ghost header
        // load ghost header
        int readsize = fread( &_ghostHeader, sizeof( GhostHeader ), 1, file );
		if (ferror(file)) {
			getCore()->logMessage("read error");
			return;
		}
        // load ghost states
        _ghostStates = new GhostState[_ghostHeader.numFrames];
        unsigned int fileResult = fread( _ghostStates, sizeof( GhostState ), _ghostHeader.numFrames, file );
		//getCore()->logMessage("Loading cattoy: frames (file, numFrames) = %d ; %d", fileResult, _ghostHeader.numFrames);

		// cattoy failed, try compatibility mode
		if (fileResult != _ghostHeader.numFrames) {
			GhostHeaderLegacy _ghostHeaderLegacy;

			// reset file
			fseek (file , 0 , SEEK_SET);
			//getCore()->logMessage("reset file");

			// load ghost header
			fread( &_ghostHeaderLegacy, sizeof( GhostHeaderLegacy ), 1, file );
			//getCore()->logMessage("read header");

			// load ghost states
			_ghostStates = new GhostState[_ghostHeaderLegacy.numFrames];
			fileResult = fread( _ghostStates, sizeof( GhostState ), _ghostHeaderLegacy.numFrames, file );
			//getCore()->logMessage("Loading compat. cattoy: frames (file, numFrames) = %d ; %d", fileResult, _ghostHeaderLegacy.numFrames);

			// put old header into new header
			_ghostHeader.jumpPose = _ghostHeaderLegacy.jumpPose;

			assert(_ghostHeader.virtues.loadLegacy(_ghostHeaderLegacy.virtues));
		}

        assert( fileResult == _ghostHeader.numFrames );
        _ghostHeader.numFrames = fileResult;
        fclose( file );
        // initialize ghost timer & time inhibitor
        _ghostTime    = 0.0f;
        _inhibitor    = 0.0f;
        _startFrameId = 0;
        // calculate time to jump
        _timeToJump = -1.0f;
        for( unsigned int i=0; i<_ghostHeader.numFrames; i++ )
        {
            if( _ghostStates[i].phase != ::jpRoaming )
            {
                _timeToJump = _ghostStates[i].time;
                break;
            }
        }
    }
    virtual ~CatToyLoadGhost()
    {
        delete[] _ghostStates;
    }
public:
    virtual Virtues* getVirtues(void)
    {
        return &_ghostHeader.virtues;
    }
    virtual JumperPhase getPhase(void)
    {
        return _currentState.phase;
    }
    virtual bool getModifier(void)
    {
        return _currentState.modifier;
    }
    virtual Matrix4f getCurrentPose(void)
    {
        return _currentState.currentPose;
    }
    virtual Matrix4f getJumpPose(void)
    {
        if( getPhase() == ::jpRoaming )
        {
            return _currentState.currentPose;
        }
        else
        {
            return _ghostHeader.jumpPose;
        }
    }
    virtual void update(float dt)
    {
        // update time
        _ghostTime += dt * _inhibitor;

        // update jumping time prediction value
        if( _timeToJump >= 0 )
        {
            _timeToJump -= dt * _inhibitor;
            _timeToJump = _timeToJump < 0 ? 0 : _timeToJump;
        }
        
        // search for couple of interpolation frames
        bool found = false;
        unsigned int frameId = _startFrameId;
        while( frameId < ( _ghostHeader.numFrames - 1 ) )
        {
            if( _ghostStates[frameId].time <= _ghostTime && _ghostStates[frameId+1].time > _ghostTime )
            {                
                found = true;
                break;
            }
            frameId++;
        }

        if( found )
        {
            // save helper state
            _startFrameId = frameId;

            // determine interpolation factor
            float factor = ( _ghostTime - _ghostStates[frameId].time ) / ( _ghostStates[frameId+1].time - _ghostStates[frameId].time );
            assert( factor >= 0 && factor <= 1 );

            // interpolate states
            _currentState.time        = _ghostTime;
            _currentState.phase       = factor < 0.5f ? _ghostStates[frameId].phase : _ghostStates[frameId+1].phase;
            _currentState.modifier    = factor < 0.5f ? _ghostStates[frameId].modifier : _ghostStates[frameId+1].modifier;        
            _currentState.currentPose = Gameplay::iEngine->interpolate( 
                _ghostStates[frameId].currentPose,
                _ghostStates[frameId+1].currentPose,
                factor
            );
        }
        else
        {
            _currentState = _ghostStates[_ghostHeader.numFrames-1];
        }

        // weaken process inhibition
        _inhibitor += dt * 0.2f;
        if( _inhibitor > 1.0f ) _inhibitor = 1.0f;
    }
    virtual void disconnect(void)
    {
    }
    virtual float getTimeToJump(void)
    {
        return _timeToJump;
    }
};

/**
 * CatToy methods
 */

CatToy* CatToy::wrap(Jumper* jumper)
{
    return new CatToyWrap( jumper );
}

CatToy* CatToy::saveGhostCatToy(Jumper* jumper, const char* filename)
{
    return new CatToySaveGhost( jumper, filename );
}

CatToy* CatToy::loadGhostCatToy(const char* filename)
{
    return new CatToyLoadGhost( filename );
}