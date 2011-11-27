
#include "headers.h"
#include "gameplay.h"
#include "preloaded.h"
#include "credits.h"
#include "callback.h"
#include "xpp.h"
#include "../common/istring.h"
#include "version.h"
#include "currenttime.h"

/**
 * class implementation
 */

Preloaded::Preloaded()
{
    // create window
    _loadingWindow = Gameplay::iGui->createWindow( "Loading" );
    gui::IGuiPanel* panel = _loadingWindow->getPanel()->find( "LoadingMessage" ); assert( panel );
    _loadingMessage = panel->getStaticText(); assert( _loadingMessage );

    // build container of preloaded assets     
    _preloadedAssets.push_back( PreloadedAsset( "Slider", engine::atXFile, "./res/x/slider.x", NULL ) );
    _preloadedAssets.push_back( PreloadedAsset( "Cord", engine::atXFile, "./res/x/cord.x", NULL ) );
    _preloadedAssets.push_back( PreloadedAsset( "Pilot02", engine::atXFile, "./res/x/pilot02.x", NULL ) );
    _preloadedAssets.push_back( PreloadedAsset( "Canopy01", engine::atXFile, "./res/x/canopy01.x", NULL ) );
    _preloadedAssets.push_back( PreloadedAsset( "Canopy02", engine::atXFile, "./res/x/canopy02.x", NULL ) );
    _preloadedAssets.push_back( PreloadedAsset( "Canopy03", engine::atXFile, "./res/x/canopy03.x", NULL ) );

		_preloadedAssets.push_back( PreloadedAsset( "Canopy04", engine::atXFile, "./res/x/canopy04.x", NULL ) );
        _preloadedAssets.push_back( PreloadedAsset( "Canopy05", engine::atXFile, "./res/x/canopy05.x", NULL ) );
        _preloadedAssets.push_back( PreloadedAsset( "Canopy06", engine::atXFile, "./res/x/canopy06.x", NULL ) );

    _preloadedAssets.push_back( PreloadedAsset( "LandingCircle", engine::atBinary, "./res/others/landingzone.ba", NULL ) );
    _preloadedAssets.push_back( PreloadedAsset( "BaseJumper01", engine::atXFile, "./res/x/character05.x", NULL ) );
   
    // additional evaluation protection
    #ifdef GAMEPLAY_EVALUATION_TIME
        SYSTEMTIME evaluationTime = GAMEPLAY_EVALUATION_TIME;
        SYSTEMTIME latestFileTime;
        if( getLatestFileTimeB( &latestFileTime ) )
        {
            if( isGreaterTime( &latestFileTime, &evaluationTime ) )
            {
                Gameplay::iGameplay->setAegisActive( true );
            }
        }
    #endif
}

Preloaded::~Preloaded()
{
    // release preloaded assets
    for( unsigned int i=0; i<_preloadedAssets.size(); i++ )
    {
        if( _preloadedAssets[i].asset ) _preloadedAssets[i].asset->release();
    }
    // release window
    _loadingWindow->getPanel()->release();
}

/**
 * Activity
 */

void Preloaded::updateActivity(float dt)
{
    // act preloading
    for( unsigned int i=0; i<_preloadedAssets.size(); i++ )
    {
        if( _preloadedAssets[i].asset ) continue;        
        // load asset
        _preloadedAssets[i].asset = Gameplay::iEngine->createAsset(
            _preloadedAssets[i].assetType,
            _preloadedAssets[i].resourceName
        );
        // rename content clumps
        callback::ClumpL clumpL;
        callback::ClumpI clumpI;
        _preloadedAssets[i].asset->forAllClumps( callback::enumerateClumps, &clumpL );
        for( clumpI = clumpL.begin(); clumpI != clumpL.end(); clumpI++ )
        {
            (*clumpI)->setName( _preloadedAssets[i].forcedName );
        }
        // preprocess asset
        xpp::preprocessXAsset( _preloadedAssets[i].asset );
    }

    // start credits
    Gameplay::iGameplay->pushActivity( new Credits() );
}

void Preloaded::progressCallback(const wchar_t* description, float progress, void* userData)
{
    Preloaded* __this = reinterpret_cast<Preloaded*>( userData );

    // update & render Gui
    __this->_loadingMessage->setText( wstrformat( L"%s...%2.1f%%", description, progress*100 ).c_str() );
    Gameplay::iEngine->getDefaultCamera()->beginScene( 
        engine::cmClearColor | engine::cmClearDepth,
    	  Vector4f( 0,0,0,0 )
    );
    Gameplay::iGui->render();
    Gameplay::iEngine->getDefaultCamera()->endScene();
    Gameplay::iEngine->present();

    // update streaming audio
    Gameplay::iAudio->updateStreamSounds();
}

bool Preloaded::endOfActivity(void)
{
    return false;
}

void Preloaded::onBecomeActive(void)
{
    // insert window in Gui
    Gameplay::iGui->getDesktop()->insertPanel( _loadingWindow->getPanel() );
    _loadingWindow->align( gui::atBottom, 4, gui::atCenter, 0 );

    // register progress callback
    Gameplay::iEngine->setProgressCallback( progressCallback, this );
}

void Preloaded::onBecomeInactive(void)
{
    // remove window from Gui
    Gameplay::iGui->getDesktop()->removePanel( _loadingWindow->getPanel() );

    // unregister progress callback
    Gameplay::iEngine->setProgressCallback( NULL, NULL );
}

engine::IClump* Preloaded::findClump(const char* name)
{
    engine::IClump* result;
    callback::Locator locator;
    for( unsigned int i=0; i<_preloadedAssets.size(); i++ )
    {
        result = locator.locate( _preloadedAssets[i].asset, name );
        if( result ) return result;
    }
    return NULL;
}