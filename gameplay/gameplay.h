#ifndef GAMEPLAY_IMPLEMENTATION_INCLUDED
#define GAMEPLAY_IMPLEMENTATION_INCLUDED

#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/engine.h"
#include "../shared/gui.h"
#include "../shared/mainwnd.h"
#include "../shared/input.h"
#include "../shared/language.h"
#include "../shared/audio.h"

#include "activity.h"
#include "career.h"
#include "render.h"
#include "memstream.h"
#include "actionmap.h"

using namespace ccor;






/**
 * gameplay implementation
 */

class Gameplay : public EntityBase
{
private:
    typedef std::vector<TiXmlDocument*> TiXmlDocuments;    
    typedef std::vector<std::string*> StringPointers;
    typedef std::vector<database::MissionInfo*> Missions;

private:
    PxDefaultAllocator		_allocator;       // PX memory allocator
    PxDefaultErrorCallback	_outputStream;    // PX output stream
    std::stack<Activity*>	_activities;      // callstack of activities
    std::vector<Career*>	_careers;         // container of careers
    RenderTarget*			_renderTarget;    // current render target
    ActionChannelM			_actionChannels;  // input action mapping
    input::IInputDevice*	_inputDevice;     // current input device
    input::KeyboardState	_keyboardState;   // internal keyboard buffer
    input::MouseState		_mouseState;      // internal mouse buffer
	input::JoyState			_joystickState;   // internal mouse buffer
    Activity*				_preloaded;       // preloaded activity
    //PHYSX3
	//NxPhysicsSDK*         _physicsSDK; // made static
    TiXmlDocument*        _config;
    audio::ISound*        _soundTrack;
    audio::ISound*        _fadedSoundTrack;
    float                 _fadedGain;
    float                 _musicVolumeTune;
    float                 _globalTimeIT;        // input treshold
    float                 _globalTimeSpeed;     // subj
    bool                  _isAegisActive;       // evaluation protection
    bool                  _isUnsafeCleanup;     // unsafe to perform gameplay cleanup
    bool                  _pitchShiftIsEnabled; // pitch shift is enabled
    TiXmlDocuments        _userTournaments;     // container for user tournament files
    StringPointers        _userStrings;         // strings cache (random usage)
    Missions              _userMissions;        // container for user missions
public:
	bool                  _freeModeIsEnabled; // allow all events at all locations
	bool                  _cheatsEnabled; // jumping in all weathers is enabled
	bool                  _feetModeIsEnabled; // allow all events at all locations

private:
    void createActionMap(void);
    void destroyActionMap(void);
    void createLicensedCareer(void);
    void generateLicensedCareerGear(Career* career);
    void generateUserCommunityEvents(void);
    void cleanupUserCommunityEvents(void);
    void generateMissions(TiXmlElement* node, database::TournamentInfo* tournamentInfo);
public:
    // class implementation
    Gameplay();
    virtual ~Gameplay();
    // component support
    static EntityBase* creator();
    virtual void __stdcall entityDestroy();
    // EntityBase 
    virtual void __stdcall entityInit(Object * p);
    virtual void __stdcall entityAct(float dt);
    virtual void __stdcall entityHandleEvent(evtid_t id, trigid_t trigId, Object* param);
    virtual IBase* __stdcall entityAskInterface(iid_t id);
public:
    // module local : activities
    void pushActivity(Activity* activity);
    // module local : careers
    unsigned int getNumCareers(void);
    Career* getCareer(unsigned int id);
    Career* findCareer(const char* careerName);
    void addCareer(Career* career);
    void deleteCareer(Career* career);
    void saveCareers(void);
    // module local : preloaded resources
    engine::IClump* findClump(const char* name);
    // module local : music
    void playSoundtrack(const char* resource);
    void stopSoundtrack(void);
    float getMusicVolume(void);
    void setMusicVolume(float volume);    
public:
    // module local : action mapping
    input::KeyboardState* getKeyboardState(void);
    input::MouseState* getMouseState(void);
    ActionChannel* getActionChannel(InputAction inputAction);
    void resetActionChannels();
    void resetActionChannels(InputAction exceptForAction);
    TiXmlElement* getConfigElement(const char* name);

public:
    // module local : gameplay inlines
    inline RenderTarget* getRenderTarget(void) { return _renderTarget; }
    inline bool getAegisActive(void) { return _isAegisActive; }
    inline void setAegisActive(bool isAegisActive) { _isAegisActive = isAegisActive; }
    inline bool getUnsafeCleanup(void) { return _isUnsafeCleanup; }
    inline void setUnsafeCleanup(bool isUnsafeCleanup) { _isUnsafeCleanup = isUnsafeCleanup; }
    inline bool pitchShiftIsEnabled(void) { return _pitchShiftIsEnabled; }
    inline bool getcheatsEnabled(void) { return _cheatsEnabled; }
    inline void setcheatsEnabled(bool cheatsEnabled) { _cheatsEnabled = cheatsEnabled; }
    inline bool getfreeModeIsEnabled(void) { return _freeModeIsEnabled; }
    inline void setfreeModeIsEnabled(bool freeModeIsEnabled) { _freeModeIsEnabled = freeModeIsEnabled; }
    inline bool getfeetModeIsEnabled(void) { return _feetModeIsEnabled; }
    inline void setfeetModeIsEnabled(bool feetModeIsEnabled) { _feetModeIsEnabled = feetModeIsEnabled; }

	inline float getGlobalTime(void) { return _globalTimeIT; }
public:
    static Gameplay*            iGameplay;
    static engine::IEngine*     iEngine;
    static gui::IGui*           iGui;
    static language::ILanguage* iLanguage;
    static input::IInput*       iInput;
    static audio::IAudio*       iAudio;
	static PxCooking*			pxCooking;
};

#endif