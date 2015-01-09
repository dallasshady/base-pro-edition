/**
 * This file contains version information and general compiler orders
 * WARNING: do not include this file in to other header files!
 */

#ifndef VERSION_CONTROL_INCLUDED
#define VERSION_CONTROL_INCLUDED

// uncomment this macro to compile developer's version
//#define GAMEPLAY_DEVELOPER_EDITION

// uncomment and designate this macro to compile evaluation version
// actually its value is a SYSTEMTIME initializer
//#define GAMEPLAY_EVALUATION_TIME { 2007, 5, 0, 1, 0, 0, 0, 0 }

// uncomment this macro to compile demo version
//#define GAMEPLAY_DEMOVERSION

// uncomment one of these macros to compile specific retail edition
// or comment each of these macros to compile web-edition
//#define GAMEPLAY_EDITION_ND
//#define GAMEPLAY_EDITION_ATARI
//#define GAMEPLAY_EDITION_POLISH

// uncomment these macros to compile add-ons
// ADDON_WEB_KK contains Kjerag and KVLY-TV mast
// ADDON_WEB_ELCAP contains El Capitan scene
#define ADDON_WEB_KK
#define ADDON_WEB_EL_CAPITAN

/**
 * presets
 */

#define CAMERA_FOV_MULTIPLIER 1.0f

/**
 * version structure and current version info
 */

struct Version
{
public:
    enum Status { alpha, beta, release };
    enum Edition { oem, retail, web, pro };
private:
    Status          _status;
    Edition         _edition;
    const wchar_t*  _localization;
    unsigned int    _version;
    unsigned int    _subversion;
    unsigned int    _patch;
    wchar_t         _versionString[128];
public:
    Version(Status s, Edition e, const wchar_t* l, unsigned int v, unsigned int sv, unsigned int patch) :
        _status(s), _edition(e), _localization(l),
        _version(v), _subversion(sv), _patch(patch)
    {
        // build status string
        const wchar_t* statusString = L"";
        switch( _status )
        {
        case alpha: statusString = L"alpha"; break; 
        case beta: statusString = L"beta"; break; 
        case release: statusString = L"release"; break;
        }

        // build edition string
        const wchar_t* editionString = L"";
        switch( _edition )
        {
        case oem: editionString = L"oem"; break; 
        case retail: editionString = L"retail"; break; 
        case web: editionString = L"web"; break; 
        case pro: editionString = L"pro"; break; 
        }
        
        #ifdef GAMEPLAY_EVALUATION_TIME
            editionString = L"evaluation";
        #endif

        // build result string
			//patch %d
			//_patch, 
        swprintf( _versionString, L"v%d.%d  (%s,%s,%s) update 2.6",
                  _version, _subversion, 
                  statusString, editionString, _localization 
        );
    }
public:
    inline const wchar_t* getLocalization(void) 
    { 
        return _localization; 
    }
    inline const wchar_t* getVersionString(void) 
    { 
        return _versionString; 
    }
};

// version info
static Version version( Version::beta, Version::pro, L"en", 0, 5, 0);

/**
 * version history
 */

// v1.3.25 (en)
// - official boost update
// - changes:
//   - improved trees, collision with trees
//   - pitch shift option
//   - gear state goal
//   - mission show goals at any time by TAB key

// v1.2.24 (en)
// - official free-for-charge add-on for web-edition
// - changes:
//   - new location "Angel Falls"
//   - blacklist support (users with refunded orders can't play game)
//   - new ability to create a plane flying by route (in custom mission)
// - bugfixes:
//   - occasional screen blinking when using afterFx; fixed by render state 
//     corrector in BSP::render()
//   - IRendering and IParticleSystem were disappearanced after previous bugfix,
//     their rendering are inside of BSP::render() now

// v1.1.23 (pl)
// - final poland OEM release
// - changes:
//   - localisation corrections
//   - GUI corrections

// v1.1.22 (de)
// - patch for german edition

// v1.1.22 (ru)
// - patch for russian edition

// v1.1.21 (pl)
// - poland OEM release

// v1.1.20 (en)
// - web edition update
// - gameplay features:
//   - user missions can be played at the each time, 
//     even while other events (and while no events)

// v1.1.19 (en)
// - web edition update
// - gameplay features:
//   - user community events and missions (completed)

// v1.1.18 (pl)
// - poland release
// - gameplay features:
//   - user community events and missions (under development)
// - bugfixes:
//   - night event time correction

// v1.1.17 (en)
// - web edition changes:
//   - licensing engine supports MAC address as pre-registration code
//   - wingsuit acrobatics

// v1.1.16 (en)
// - web edition
// - improving licensing and protection for web edition

// v1.1.15 (ru&de)
// - licensing and protection for web edition
// - fixed launcher (controls are disappearing in some kases)
// - patch for russian version
// - patch for german version

// v1.1.14
// - patch for Demo and Atari edition
// - bugfixes
//   - can't win untradeable item in Ostankino Hard Landing challenge
//   - Audio mosule based on DirectSound

// v1.1.13
// - release candidate for ND
// - bugfixes
//   - alt+tab
//   - language resources protected by control sum checkout

// v1.1.12
// - public demo

// v1.1.11
// - publisher's evaluation beta version
// - protected, expires 1.05.2007
// - bugfixes:
//   - home selection on water and ice
//   - "character is perished" messagebox for all career slots

// v1.1.10
// - web edition demo

// v1.1.9
// - new version build
// - bugfixes:
//   - credits spend for reanimation after dead
//   - no locked equipment for sale

// v1.0.8
// - retail, release-candidate for ND
// - several insignificant bugfixes according to beta-testing registry

// v1.0.7
// - retail, release-candidate for Atari
// - several bugfixes 
//   - history text over geoscape is under cursor
//   - instructor goals now scoring correctly

// v1.0.6
// - web alpha build
// - several bugfixes
//   - camera collides with world (solution ¹2)

// v1.0.5
// - english demo build

// v1.0.4
// - beta build
// - Felix Baumgartner as playable character (Atari edition)
// - several changes in malfunction simulation
// - original GUI icons
// - several bugfixes
//   - skinning vertex shader forced from 3.0 to 1.1
//   - camera collides with world (solution ¹1)

// v1.0.3
// - beta build
// - unicode format for local resources

// v1.0.2
// - beta build
// - several bug fixes
//   - footage goal in Ostankino Open Air Mission
// - equip activity will improve rigging skill
// - offheadings at opening, depended on rigging skill

// v1.0.1 
// - beta build 
// - supports version check of string resources around language and gameplay modules

// v1.0.0 
// - first beta build

#endif