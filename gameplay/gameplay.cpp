
#include "headers.h"
#include "gameplay.h"
#include "preloaded.h"
#include "crypt.h"
#include "scene.h"
#include "version.h"
#include "unicode.h"
#include "currenttime.h"
#include "messagebox.h"
//#include "checkreg.h"

/**
 * interfacez
 */

Gameplay*            Gameplay::iGameplay = NULL;
engine::IEngine*     Gameplay::iEngine = NULL;
gui::IGui*           Gameplay::iGui = NULL;
language::ILanguage* Gameplay::iLanguage = NULL;
input::IInput*       Gameplay::iInput = NULL;
audio::IAudio*       Gameplay::iAudio = NULL;
PxCooking*			 Gameplay::pxCooking = NULL;
/**
 * class implementation
 */

Gameplay::Gameplay()
{
    iGameplay        = this;
    _globalTimeSpeed = 1.0f;
    _globalTimeIT    = 0.0f;

    _soundTrack = NULL;
    _fadedSoundTrack = NULL;
    _musicVolumeTune = 0.0f;

    _isAegisActive = false;
    _isUnsafeCleanup = false;
    _pitchShiftIsEnabled = false;

	_renderTarget = NULL;
	pxCooking = NULL;
   
	// zhulikotester
    //checkKey( "7LGQ-3F9H-C7LT-Q3W4-FR9F-CX9H", "WD-WMAJ94914315" );
}

Gameplay::~Gameplay()
{
    if( !_isUnsafeCleanup )
    {
        if( _soundTrack ) 
        {
            _soundTrack->stop();
            _soundTrack->release();
        }
        if( _fadedSoundTrack )
        {
            _fadedSoundTrack->stop();
            _fadedSoundTrack->release();
        }
    
        // delete activities
        while( _activities.size() )
        {
            Activity* activity = _activities.top();
            _activities.pop();
            activity->onBecomeInactive();
            if( _activities.size() ) 
            {
                _activities.top()->onReturnFromActivity( activity );
                delete activity;
                _activities.top()->onBecomeActive();
            }
            else
            {
                delete activity;
            }
        }
    
        // delete careers
        saveCareers();
        for( unsigned int i=0; i<_careers.size(); i++ ) delete _careers[i];

        // delete user events
        cleanupUserCommunityEvents();
    
        // delete render target
        if (_renderTarget) delete _renderTarget;
    
        // delete sdks
		PxGetPhysics().release();
		pxCooking->release();
		foundation->release();
    
        // cleanup action mapping
        destroyActionMap();

        // delete input device
        _inputDevice->release();

        delete _config;
    }

    // reset internal interface pointers
    iGameplay = NULL;
    iEngine   = NULL;
    iGui      = NULL;
    iLanguage = NULL;
    iInput    = NULL;
    iAudio    = NULL;
}

/**
 * private behaviour
 */

static TiXmlElement* findActionElement(TiXmlDocument* config, const char* actionName)
{
    TiXmlNode* child = config->FirstChild(); assert( child );
    if( child != NULL ) do 
    {
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "action" ) == 0 )
        {
            const char* name = static_cast<TiXmlElement*>( child )->Attribute( "name" );
            if( strcmp( name, actionName ) == 0 )
            {
                return static_cast<TiXmlElement*>( child );
            }
        }
        child = child->NextSibling();
    }
    while( child != NULL );
    return NULL;
}

static unsigned int getActionCode(TiXmlDocument* config, const char* actionName)
{
    TiXmlElement* configElement = findActionElement( config, actionName ); assert( configElement );
    int code;
    configElement->Attribute( "code", &code );
    assert( code > 0 && code < 255 );
    return unsigned int( code );
}

void Gameplay::createActionMap(void)
{
    // read configuration...
    ButtonChannel* buttonChannel;

	float ascLvl = 2.0f;
	float descLvl = 5.0f;
    // create left channel
    buttonChannel = new ButtonChannel( iaLeft, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaLeft" ) );
    _actionChannels.insert( ActionChannelT( iaLeft, buttonChannel ) );

    // create right channel
    buttonChannel = new ButtonChannel( iaRight, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaRight" ) );
    _actionChannels.insert( ActionChannelT( iaRight, buttonChannel ) );

    // create forward channel
    buttonChannel = new ButtonChannel( iaForward, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaForward" ) );
    _actionChannels.insert( ActionChannelT( iaForward, buttonChannel ) );

    // create backward channel
    buttonChannel = new ButtonChannel( iaBackward, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaBackward" ) );
    _actionChannels.insert( ActionChannelT( iaBackward, buttonChannel ) );
    
    // create left-warp channel
    buttonChannel = new ButtonChannel( iaLeftWarp, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaLeftWarp" ) );
    _actionChannels.insert( ActionChannelT( iaLeftWarp, buttonChannel ) );

    // create right-warp channel
    buttonChannel = new ButtonChannel( iaRightWarp, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaRightWarp" ) );
    _actionChannels.insert( ActionChannelT( iaRightWarp, buttonChannel ) );

    // create left-rear riser channel
    buttonChannel = new ButtonChannel( iaLeftRearRiser, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaLeftRearRiser" ) );
    _actionChannels.insert( ActionChannelT( iaLeftRearRiser, buttonChannel ) );

    // create right-rear riser channel
    buttonChannel = new ButtonChannel( iaRightRearRiser, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaRightRearRiser" ) );
    _actionChannels.insert( ActionChannelT( iaRightRearRiser, buttonChannel ) );

    // create phase channel
    buttonChannel = new ButtonChannel( iaPhase, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaPhase" ) );
    _actionChannels.insert( ActionChannelT( iaPhase, buttonChannel ) );

    // create modifier channel
    buttonChannel = new ButtonChannel( iaModifier, ascLvl*0.7f, descLvl*0.7f );
    buttonChannel->setup( 0, getActionCode( _config, "iaModifier" ) );
    _actionChannels.insert( ActionChannelT( iaModifier, buttonChannel ) );
    
    // create camera mode 0 channel
    buttonChannel = new ButtonChannel( iaCameraMode0, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaCameraMode0" ) );
    _actionChannels.insert( ActionChannelT( iaCameraMode0, buttonChannel ) );

    // create camera mode 1 channel
    buttonChannel = new ButtonChannel( iaCameraMode1, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaCameraMode1" ) );
    _actionChannels.insert( ActionChannelT( iaCameraMode1, buttonChannel ) );

    // create camera mode 2 channel
    buttonChannel = new ButtonChannel( iaCameraMode2, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaCameraMode2" ) );
    _actionChannels.insert( ActionChannelT( iaCameraMode2, buttonChannel ) );

    // create WLO channel
    buttonChannel = new ButtonChannel( iaWLO, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaWLO" ) );
    _actionChannels.insert( ActionChannelT( iaWLO, buttonChannel ) );

    // create hook knife channel
    buttonChannel = new ButtonChannel( iaHook, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaHook" ) );
    _actionChannels.insert( ActionChannelT( iaHook, buttonChannel ) );

    // create flight time (+) channel
    buttonChannel = new ButtonChannel( iaAccelerateFlightTime, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaAccelerateFlightTime" ) );
    _actionChannels.insert( ActionChannelT( iaAccelerateFlightTime, buttonChannel ) );

    // create flight time (-) channel
    buttonChannel = new ButtonChannel( iaDecelerateFlightTime, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaDecelerateFlightTime" ) );
    _actionChannels.insert( ActionChannelT( iaDecelerateFlightTime, buttonChannel ) );

    // create altimeter mode channel
    buttonChannel = new ButtonChannel( iaAltimeterMode, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaAltimeterMode" ) );
    _actionChannels.insert( ActionChannelT( iaAltimeterMode, buttonChannel ) );

    // create warn. alt. (+) channel
    buttonChannel = new ButtonChannel( iaIncreaseWarningAltitude, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaIncreaseWarningAltitude" ) );
    _actionChannels.insert( ActionChannelT( iaIncreaseWarningAltitude, buttonChannel ) );

    // create warn. alt. (-) channel
    buttonChannel = new ButtonChannel( iaDecreaseWarningAltitude, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaDecreaseWarningAltitude" ) );
    _actionChannels.insert( ActionChannelT( iaDecreaseWarningAltitude, buttonChannel ) );

    // create HUD mode channel
    buttonChannel = new ButtonChannel( iaSwitchHUDMode, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaSwitchHUDMode" ) );
    _actionChannels.insert( ActionChannelT( iaSwitchHUDMode, buttonChannel ) );

    // create music volume (+) channel
    buttonChannel = new ButtonChannel( iaIncreaseMusicVolume, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaIncreaseMusicVolume" ) );
    _actionChannels.insert( ActionChannelT( iaIncreaseMusicVolume, buttonChannel ) );

    // create music volume (-) channel
    buttonChannel = new ButtonChannel( iaDecreaseMusicVolume, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaDecreaseMusicVolume" ) );
    _actionChannels.insert( ActionChannelT( iaDecreaseMusicVolume, buttonChannel ) );

    // cutaway
    buttonChannel = new ButtonChannel( iaCutAway, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaCutAway" ) );
    _actionChannels.insert( ActionChannelT( iaCutAway, buttonChannel ) );
	
    // pull reserve
    buttonChannel = new ButtonChannel( iaPullReserve, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaPullReserve" ) );
    _actionChannels.insert( ActionChannelT( iaPullReserve, buttonChannel ) );

    // create left reserve channel
    buttonChannel = new ButtonChannel( iaReserveLeft, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaReserveLeft" ) );
    _actionChannels.insert( ActionChannelT( iaReserveLeft, buttonChannel ) );

    // create right reserve channel
    buttonChannel = new ButtonChannel( iaReserveRight, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaReserveRight" ) );
    _actionChannels.insert( ActionChannelT( iaReserveRight, buttonChannel ) );

    // create left-warp reserve channel
    buttonChannel = new ButtonChannel( iaReserveLeftWarp, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaReserveLeftWarp" ) );
    _actionChannels.insert( ActionChannelT( iaReserveLeftWarp, buttonChannel ) );

    // create right-warp reserve channel
	buttonChannel = new ButtonChannel( iaReserveRightWarp, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaReserveRightWarp" ) );
    _actionChannels.insert( ActionChannelT( iaReserveRightWarp, buttonChannel ) );

    // create left rear riser reserve channel
    buttonChannel = new ButtonChannel( iaReserveLeftRearRiser, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaReserveLeftRearRiser" ) );
    _actionChannels.insert( ActionChannelT( iaReserveLeftRearRiser, buttonChannel ) );

    // create right rear riser reserve channel
	buttonChannel = new ButtonChannel( iaReserveRightRearRiser, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaReserveRightRearRiser" ) );
    _actionChannels.insert( ActionChannelT( iaReserveRightRearRiser, buttonChannel ) );

    // create rear break channel
    buttonChannel = new ButtonChannel( iaRearBrake, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaRearBrake" ) );
    _actionChannels.insert( ActionChannelT( iaRearBrake, buttonChannel ) );

    // create right channel
    buttonChannel = new ButtonChannel( iaRight, ascLvl, descLvl );
    buttonChannel->setup( 0, getActionCode( _config, "iaRight" ) );
    _actionChannels.insert( ActionChannelT( iaRight, buttonChannel ) );

	/// JOYSTICK CHANNELS

		// create left channel (joystick)
		_actionChannels.insert( ActionChannelT( iaLeftJoy, new JoystickChannel( iaLeftJoy ) ) );
		// create left channel (joystick)
		_actionChannels.insert( ActionChannelT( iaRightJoy, new JoystickChannel( iaRightJoy ) ) );
		// create backward channel (joystick)
		_actionChannels.insert( ActionChannelT( iaBackwardJoy, new JoystickChannel( iaBackwardJoy ) ) );
		// create cutaway channel (joystick)
		_actionChannels.insert( ActionChannelT( iaCutAwayJoy, new JoystickChannel( iaCutAwayJoy ) ) );

		//// cutaway (joystick)
		//buttonChannel = new ButtonChannel( iaCutAwayJoy, 1, 4 );
		//unsigned int JoyBt1 = 0;
		//buttonChannel->setup( 2, JoyBt1 );
		//_actionChannels.insert( ActionChannelT( iaCutAwayJoy, buttonChannel ) );
	

    // default channels : head left/right/up/down
    _actionChannels.insert( ActionChannelT( iaHeadLeft, new MouseChannel( iaHeadLeft ) ) );
    _actionChannels.insert( ActionChannelT( iaHeadRight, new MouseChannel( iaHeadRight ) ) );
    _actionChannels.insert( ActionChannelT( iaHeadUp, new MouseChannel( iaHeadUp ) ) );
    _actionChannels.insert( ActionChannelT( iaHeadDown, new MouseChannel( iaHeadDown ) ) );

    // default channels : zoom in and zoom out
    ButtonChannel* channel = new ButtonChannel( iaZoomIn, 2, 2 );
    channel->setup( 1, 0 );
    _actionChannels.insert( ActionChannelT( iaZoomIn, channel ) );
    channel = new ButtonChannel( iaZoomOut, 2, 2 );
    channel->setup( 1, 1 );
    _actionChannels.insert( ActionChannelT( iaZoomOut, channel ) );

    // default channels : camera mode 3
    _actionChannels.insert( ActionChannelT( iaCameraMode3, new ButtonChannel( iaCameraMode3 ) ) );

    // developer's channels
    #ifdef GAMEPLAY_DEVELOPER_EDITION
        _actionChannels.insert( ActionChannelT( iaGlobalDeceleration, new ButtonChannel( iaGlobalDeceleration ) ) );
        _actionChannels.insert( ActionChannelT( iaGlobalAcceleration, new ButtonChannel( iaGlobalAcceleration ) ) );
    #endif
}

void Gameplay::destroyActionMap(void)
{
    for( ActionChannelI actionChannelI = _actionChannels.begin();
                        actionChannelI != _actionChannels.end();
                        actionChannelI++ )
    {
        delete actionChannelI->second;
    }
    _actionChannels.clear();
}

/**
 * component support
 */

EntityBase* Gameplay::creator()
{
    assert( iGameplay == NULL );
    return new Gameplay;
}

void Gameplay::entityDestroy()
{
    delete this;
}

/**
 * EntityBase 
 */

void Gameplay::entityInit(Object * p)
{
    // load config
    _config = new TiXmlDocument( "./cfg/config.xml" );
    _config->LoadFile();

    // read pitch shift option    
    TiXmlElement* xmlSound = Gameplay::iGameplay->getConfigElement( "sound" ); assert( xmlSound );
    int pitchShift;
    xmlSound->Attribute( "pitchShift", &pitchShift );
    _pitchShiftIsEnabled = ( pitchShift != 0 );   

	// read cheats option    
    TiXmlElement* details = Gameplay::iGameplay->getConfigElement( "details" ); assert( details );
    int cheats;
    details->Attribute( "cheats", &cheats );
    _cheatsEnabled = ( cheats != 0 );  

	// read free jumping mode  
    int freemode;
    details->Attribute( "freemode", &freemode );
    _freeModeIsEnabled = ( freemode != 0 ); 

	// read meters / feet mode  
    int units;
    details->Attribute( "units", &units );
    _feetModeIsEnabled = ( units != 0 ); 

    // setup random number generation
    getCore()->getRandToolkit()->setSeed( GetTickCount() );


    // retrieve interfaces
    queryInterface( "Engine", &iEngine ); assert( iEngine );
    queryInterface( "Gui", &iGui ); assert( iGui );
    queryInterface( "Language", &iLanguage ); assert( iLanguage );
    queryInterface( "Input", &iInput ); assert( iInput );
    queryInterface( "Audio", &iAudio ); assert( iAudio );
    if( !iAudio || !iInput || !iLanguage || !iGui || !iEngine )
    {
        throw Exception( "One or more core modules are not found, so gameplay will Crash Right Now!" );
    }

    // check language module
    if( wcscmp( iLanguage->getVersionString(), ::version.getVersionString() ) != 0 )
    {
        // incompatible module? - show no localization data
        iLanguage->reset();
    }

	getCore()->logMessage("Version: %ls (Clean)", ::version.getVersionString());

    // create input device
    _inputDevice = iInput->createInputDevice();
    createActionMap();

    // create physics resources
	foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
    gPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale() );
	pxCooking = PxCreateCooking(PX_PHYSICS_VERSION, *foundation, PxCookingParams(PxTolerancesScale()));
	PxInitExtensions(*gPhysicsSDK);

	//PHYSX3
    //NxGetPhysicsSDK()->setParameter( NX_VISUALIZATION_SCALE, 100.0f );
    //NxGetPhysicsSDK()->setParameter( NX_VISUALIZE_ACTOR_AXES, 1 );
    //NxGetPhysicsSDK()->setParameter( NX_VISUALIZE_COLLISION_SHAPES, 1 );
    //NxGetPhysicsSDK()->setParameter( NX_VISUALIZE_COLLISION_STATIC, 1 );
    //NxGetPhysicsSDK()->setParameter( NX_VISUALIZE_COLLISION_DYNAMIC,1 );

    // generate user community events from XML documents
    generateUserCommunityEvents();

    // open index
    TiXmlDocument* index = new TiXmlDocument( "./usr/index.xml" );
    index->LoadFile();

    // enumerate career nodes
    TiXmlNode* child = index->FirstChild();
    if( child ) do 
    {
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "career" ) == 0 )
        {
            _careers.push_back( new Career( static_cast<TiXmlElement*>( child ) ) );
        }
        child = child->NextSibling();
    }
    while( child != NULL );

    // close index document
    delete index;

    // create career for LICENSED_CHAR
    #ifdef GAMEPLAY_EDITION_ATARI
        createLicensedCareer();
    #endif

    // determine afterfx configuration
    TiXmlElement* video = getConfigElement( "video" ); assert( video );
    int afterfx = 0;
    video->Attribute( "afterfx", &afterfx );

    // create render target
    if( afterfx &&
        iEngine->isPfxSupported( engine::pfxBloom ) &&
        iEngine->isPfxSupported( engine::pfxMotionBlur ) )
    {
        _renderTarget = new AfterFxRT();
    }
    else
    {
        _renderTarget = new SimpleRT();
    }

    // play menu music
    playSoundtrack( "./res/sounds/music/dirty_moleculas_execution.ogg" );

    // evaluation protection
    #ifdef GAMEPLAY_EVALUATION_TIME
        SYSTEMTIME evaluationTime = GAMEPLAY_EVALUATION_TIME;
        SYSTEMTIME latestFileTime;
        if( getLatestFileTimeB( &latestFileTime ) )
        {
            if( isGreaterTime( &latestFileTime, &evaluationTime ) )
            {
                pushActivity( new Messagebox( Gameplay::iLanguage->getUnicodeString( 765 ) ) );
            }
            else
            {
                // startup
                _preloaded = new Preloaded();
                pushActivity( _preloaded );
            }
        }
    #else
        // determine if licence is required to play game
        bool licenceIsRequired = false;
        #ifndef GAMEPLAY_EDITION_ND
            #ifndef GAMEPLAY_EDITION_ATARI
                #ifndef GAMEPLAY_EDITION_POLISH
                    licenceIsRequired = false;
                #endif
            #endif
        #endif

        // startup        
        _preloaded = new Preloaded();
        pushActivity( _preloaded );
    #endif   
}

void Gameplay::entityAct(float dt)
{
    _globalTimeIT -= dt;
    dt *= _globalTimeSpeed;

    // fade soundtracks
    if( _fadedSoundTrack )
    {
        if( !_fadedSoundTrack->isPlaying() )
        {
            _fadedSoundTrack->release();
            _fadedSoundTrack = NULL;
        }
        else
        {
            float fadeVelocity = 0.125f;
            _fadedGain -= fadeVelocity * dt;
            if( _fadedGain <= 0 )
            {
                _fadedSoundTrack->release();
                _fadedSoundTrack = NULL;
            }
            else
            {
                _fadedSoundTrack->setGain( _fadedGain );
            }
        }
    }

    // update soundtrack
    if( _soundTrack && !_soundTrack->isPlaying() )
    {
        _soundTrack->release();
        _soundTrack = NULL;
    }

    // update music volume
    float tuneVelocity = 0.2f;
    // increase volume?
    if( Gameplay::iGameplay->getActionChannel( ::iaIncreaseMusicVolume )->getTrigger() )
    {
        _musicVolumeTune += tuneVelocity * dt;
    }
    // decrease volume?
    if( Gameplay::iGameplay->getActionChannel( ::iaDecreaseMusicVolume )->getTrigger() )
    {
        _musicVolumeTune -= tuneVelocity * dt;
    }
    // finalize
    if( fabs( _musicVolumeTune ) > 0.01f )
    {
        float volume = Gameplay::iGameplay->getMusicVolume();
        volume += _musicVolumeTune, _musicVolumeTune = 0;
        if( volume < 0.0f ) volume = 0.0f;
        if( volume > 1.0f ) volume = 1.0f;
        Gameplay::iGameplay->setMusicVolume( volume );
    }

    // retrieve input device state
    _inputDevice->getKeyboardState( &_keyboardState );
    _inputDevice->getMouseState( &_mouseState );
	_inputDevice->getJoystickState( &_joystickState );

    // map input actions
    for( ActionChannelI actionChannelI = _actionChannels.begin();
                        actionChannelI != _actionChannels.end();
                        actionChannelI++ )
    {
        actionChannelI->second->update( dt, &_mouseState, &_keyboardState, &_joystickState );
    }

    // global controls
    #ifdef GAMEPLAY_DEVELOPER_EDITION
        if( getActionChannel( ::iaGlobalAcceleration )->getTrigger() )
        {
            if( _globalTimeIT < 0 )
            {
                _globalTimeSpeed *= 2.0f;
                if( _globalTimeSpeed > 2.0f ) _globalTimeSpeed = 2.0f;
                _globalTimeIT = 0.25f;
            }
        }
        if( getActionChannel( ::iaGlobalDeceleration )->getTrigger() )
        {
            if( _globalTimeIT < 0 )
            {
                _globalTimeSpeed *= 0.5f;
                if( _globalTimeSpeed < 0.125f ) _globalTimeSpeed = 0.125f;
                _globalTimeIT = 0.25f;
            }
        }
    #endif

    // schedule activities
    if( _activities.size() ) 
    {
        _activities.top()->updateCooperativeActivity( dt );
    }
    else
    {
        getCore()->exit( 0 );
    }
    if( _activities.size() && _activities.top()->endOfActivity() )
    {
        Activity* activity = _activities.top();
        _activities.pop();
        activity->onBecomeInactive();

        if( _activities.size() ) 
        {
            _activities.top()->onReturnFromActivity( activity );
            delete activity;
            _activities.top()->onBecomeActive();
        }
        else
        {
            delete activity;
        }
    }
}

void Gameplay::entityHandleEvent(evtid_t id, trigid_t trigId, Object* param)
{
}

IBase* Gameplay::entityAskInterface(iid_t id)
{
    return NULL;
}

/**
 * module local : activities
 */

void Gameplay::pushActivity(Activity* activity)
{
    // current top activity become inactive
    if( _activities.size() ) _activities.top()->onBecomeInactive();

    // push new activity
    _activities.push( activity );

    // current top activity become active
    activity->onBecomeActive();
}

/**
 * module local : careers
 */

unsigned int Gameplay::getNumCareers(void)
{
    return _careers.size();
}

Career* Gameplay::getCareer(unsigned int id)
{
    assert( id>=0 && id<_careers.size() );
    return _careers[id];
}

Career* Gameplay::findCareer(const char* careerName)
{
    for( unsigned int i=0; i<_careers.size(); i++ )
    {
        if( strcmp( _careers[i]->getName(), careerName ) == 0 )
        {
            return _careers[i];
        }
    }
    return NULL;
}

void Gameplay::addCareer(Career* career)
{
    _careers.push_back( career );
}

void Gameplay::deleteCareer(Career* career)
{
    for( unsigned int i=0; i<_careers.size(); i++ )
    {
        if( _careers[i] == career )
        {
            delete _careers[i];
			_careers.erase( _careers.begin() + i );
        }
    }
}

void Gameplay::saveCareers(void)
{
	// backup
	TiXmlDocument* backup = new TiXmlDocument( "./usr/index.xml" );
	backup->LoadFile();
	backup->SaveFile("./usr/index.xml.bak");
	delete backup;

	// save
    TiXmlDocument* index = new TiXmlDocument( "./usr/index.xml" );
    for( unsigned int i=0; i<_careers.size(); i++ )
    {
        _careers[i]->save( index );
    }
    index->SaveFile();
    delete index;
}

/**
 * module local : preloaded resources
 */

engine::IClump* Gameplay::findClump(const char* name)
{
    return dynamic_cast<Preloaded*>( _preloaded )->findClump( name );
}

/**
 * module local : music & sound
 */

void Gameplay::playSoundtrack(const char* resource)
{
    // stop playing
    stopSoundtrack();

    // obtain desired sound volume
    TiXmlElement* xmlSound = Gameplay::iGameplay->getConfigElement( "sound" ); assert( xmlSound );
    double musicVolume = 0.0f;
    xmlSound->Attribute( "musicVolume", &musicVolume );
    assert( musicVolume >= 0 );
    assert( musicVolume <= 1 );

    // play new sound
    if( musicVolume > 0 )
    {
        _soundTrack = iAudio->createStreamSound( resource, 4, 256*1024 ); assert( _soundTrack );
        _soundTrack->play();
        _soundTrack->setGainLimits( 0.0f, 1.0f );
        _soundTrack->setGain( float( musicVolume ) );
    }
}

void Gameplay::stopSoundtrack(void)
{
    if( _soundTrack )
    {
        // initialize fade gain
        TiXmlElement* xmlSound = Gameplay::iGameplay->getConfigElement( "sound" ); assert( xmlSound );
        double musicVolume = 0.0f;
        xmlSound->Attribute( "musicVolume", &musicVolume );
        assert( musicVolume >= 0 );
        assert( musicVolume <= 1 );
        _fadedGain = float( musicVolume );
        
        // move pointer to fade controller
        if( _fadedSoundTrack )
        {
            if( _fadedSoundTrack->isPlaying() ) _fadedSoundTrack->stop();
            _fadedSoundTrack->release();
        }
        _fadedSoundTrack = _soundTrack;
        _soundTrack = NULL;
    }
}

float Gameplay::getMusicVolume(void)
{
    // initialize fade gain
    TiXmlElement* xmlSound = Gameplay::iGameplay->getConfigElement( "sound" ); assert( xmlSound );
    double musicVolume = 0.0f;
    xmlSound->Attribute( "musicVolume", &musicVolume );
    assert( musicVolume >= 0 );
    assert( musicVolume <= 1 );
    return float( musicVolume );
}

void Gameplay::setMusicVolume(float volume)
{
    assert( volume >= 0 );
    assert( volume <= 1 );
    volume = volume < 0 ? 0 : ( volume > 1 ? 1 : volume ); 

    // initialize fade gain
    TiXmlElement* xmlSound = Gameplay::iGameplay->getConfigElement( "sound" ); assert( xmlSound );
    xmlSound->SetAttribute( "musicVolume", strformat( "%4.2f", volume ).c_str() );
    _config->SaveFile();

    // update sound
    if( _soundTrack ) _soundTrack->setGain( volume );
}

/**
 * module local : action mapping
 */

input::KeyboardState* Gameplay::getKeyboardState(void)
{
    return &_keyboardState;
}

input::MouseState* Gameplay::getMouseState(void)
{
    return &_mouseState;
}

ActionChannel* Gameplay::getActionChannel(InputAction inputAction)
{
    ActionChannelI actionChannelI = _actionChannels.find( inputAction );
    assert( actionChannelI != _actionChannels.end() );
    return actionChannelI->second;
}

void Gameplay::resetActionChannels(void)
{
    for( ActionChannelI actionChannelI = _actionChannels.begin();
                        actionChannelI != _actionChannels.end();
                        actionChannelI++ )
    {
        actionChannelI->second->reset();
    }
}

void Gameplay::resetActionChannels(InputAction exceptForAction)
{
    for( ActionChannelI actionChannelI = _actionChannels.begin();
                        actionChannelI != _actionChannels.end();
                        actionChannelI++ )
    {
        if( actionChannelI->first != exceptForAction )
        {
            actionChannelI->second->reset();
        }
    }
}

TiXmlElement* Gameplay::getConfigElement(const char* name)
{
    TiXmlNode* child = _config->FirstChild(); assert( child );
    if( child != NULL ) do 
    {
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), name ) == 0 )
        {
            return static_cast<TiXmlElement*>( child );
        }
        child = child->NextSibling();
    }
    while( child != NULL );
    return NULL;
}

void Gameplay::createLicensedCareer(void)
{
    std::string licensedCareerName = unicodeToAsciiz( iLanguage->getUnicodeString( 265 ) );
    bool licensedCareerFound = false;        
    for( unsigned int i=0; i<getNumCareers(); i++ )
    {
        if( strcmp( getCareer( i )->getName(), licensedCareerName.c_str() ) == 0 )
        {
            licensedCareerFound = true;
            break;
        }
    }
    if( !licensedCareerFound )
    {
        Career* licensedCareer = new Career( licensedCareerName.c_str() );
        Virtues* virtues = licensedCareer->getVirtues();

        // home
        licensedCareer->setHome( 1149, 209 );
        
        // appearance
        virtues->appearance.height = 183;
        virtues->appearance.weight = 77;
        virtues->appearance.face = 0;
        // predisposition
        virtues->predisp.perception = 0.25f;
        virtues->predisp.endurance  = 0.75f;
        virtues->predisp.tracking   = 0.5f;
        virtues->predisp.rigging    = 0.5f;
        // evolution
        virtues->evolution.workTime = 30;
        // enable malfunctions
        virtues->equipment.malfunctions = true;

        // raise "licensed career" flag
        licensedCareer->setLicensedFlag( true );

        // raise "home placement" flag,
        // it is cause character to be in home when this career begins
        licensedCareer->setHomePlacementFlag( true );

        // generate gear
        generateLicensedCareerGear( licensedCareer );

        // finalize
        addCareer( licensedCareer );
        saveCareers();
    }
}

void Gameplay::generateLicensedCareerGear(Career* career)
{
    // generate equipment

    // equip personal tracking suit
    career->getVirtues()->equipment.suit = Gear( gtSuit, 28 );
    // add wingsuit (white "Falco") in to gear stock
    career->addGear( Gear( gtSuit, 26 ) );

    // equip personal BASE rig
    career->getVirtues()->equipment.rig = Gear( gtRig, 22 );
    // add personal skydiving rig in to gear stock
    career->addGear( Gear( gtRig, 23 ) );

    // equip first personal canopy
    career->getVirtues()->equipment.canopy = Gear( gtCanopy, 105 );
    // add second personal canopy in to gear stock
    career->addGear( Gear( gtCanopy, 106 ) );
    // add third personal canopy in to gear stock
    career->addGear( Gear( gtCanopy, 124 ) );
}