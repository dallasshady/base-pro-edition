
#include "headers.h"
#include "geoscape.h"
#include "database.h"
#include "gameplay.h"

Location::Location(Geoscape* geoscape, unsigned int databaseId)
{
    // initialize location
    _geoscape   = geoscape;
    _databaseId = databaseId;

    // make demo game data
    _demoGameData = new GameData( sizeof(LocationData) );
    LocationData* demoLocationData = reinterpret_cast<LocationData*>( _demoGameData->getData() );
    demoLocationData->player = true;
    // random weather
    unsigned int dice = unsigned int( getCore()->getRandToolkit()->getUniform( 0, 3 ) );
    switch( dice )
    {
    case 0:
        demoLocationData->weather = wtSunny; break;
    case 1:
        demoLocationData->weather = wtVariable; break;
    case 2:
        demoLocationData->weather = wtCloudy; break;    
    default:
        demoLocationData->weather = wtSunny;
    }
    demoLocationData->wind.set(
        getCore()->getRandToolkit()->getUniform( -1.0f, 1.0f ), 
        0.0f,
        getCore()->getRandToolkit()->getUniform( -1.0f, 1.0f )
    );
    demoLocationData->wind.normalize();
    demoLocationData->windAmbient = getCore()->getRandToolkit()->getUniform( 0.0f, 6.0f );
    demoLocationData->windBlast = getCore()->getRandToolkit()->getUniform( 0.0f, 6.0f );

    // location info
    database::LocationInfo* locationInfo = database::LocationInfo::getRecord( _databaseId );

    // create location window
    _window = Gameplay::iGui->createWindow( "CareerLocation" );

    // hide inaccessible location
    if( !locationInfo->accessible )
    {
        _window->getPanel()->setVisible( false );
    }

    // setup caption
    gui::IGuiPanel* caption       = _window->getPanel()->find( "Caption" );
    gui::IGuiPanel* captionShadow = _window->getPanel()->find( "CaptionShadow" );
    assert( caption && caption->getStaticText() );
    assert( captionShadow && captionShadow->getStaticText() );
    caption->getStaticText()->setText( Gameplay::iLanguage->getUnicodeString( locationInfo->nameId ) );
    captionShadow->getStaticText()->setText( Gameplay::iLanguage->getUnicodeString( locationInfo->nameId ) );

    // place location
    if( _geoscape )
    {
        gui::Rect oldRect = _window->getPanel()->getRect();
        gui::Rect newRect;
        if( _databaseId == 0 )
        {
            newRect.left = _geoscape->getCareer()->getHomeX() - oldRect.getWidth() / 2;
            newRect.top  = _geoscape->getCareer()->getHomeY() - oldRect.getHeight() / 2;
        }
        else
        {
            newRect.left   = locationInfo->worldX - oldRect.getWidth() / 2;
            newRect.top    = locationInfo->worldY - oldRect.getHeight() / 2;
        }
        newRect.right  = newRect.left + oldRect.getWidth();
        newRect.bottom = newRect.top + oldRect.getHeight();
        _window->getPanel()->setRect( newRect );
        _geoscape->getWindow()->getPanel()->insertPanel( _window->getPanel() );
    }

    // location serializable data
    GameData* gameData = NULL;
    if( geoscape )
    {
        gameData = geoscape->getCareer()->getGameData( locationInfo->gameData );
    }
    else
    {
        gameData = _demoGameData;
    }
    if( !gameData )
    {
        gameData = new GameData( sizeof( LocationData ) );
        _gameData = reinterpret_cast<LocationData*>( gameData->getData() );
        geoscape->getCareer()->addGameData( locationInfo->gameData, gameData );        
        setPlayer( false );
        // random weather
        unsigned int dice = unsigned int( getCore()->getRandToolkit()->getUniform( 0, 6 ) );
        switch( dice )
        {
        case 0:
            setWeather( wtSunny );
            break;
        case 1:
            setWeather( wtVariable );
            break;
        case 2:
            setWeather( wtCloudy );
            break;
        case 3:
            setWeather( wtLightRain );
            break;
        case 4:
            setWeather( wtHardRain );
            break;
        case 5:
            setWeather( wtThunder );
            break;
        default:
            setWeather( wtSunny );
            break;
        }
    }
    else
    {
        _gameData = reinterpret_cast<LocationData*>( gameData->getData() );
        setWeather( _gameData->weather );
        setPlayer( _gameData->player );
    }
}

Location::~Location()
{
    if( _geoscape ) _geoscape->getWindow()->getPanel()->removePanel( _window->getPanel() );
    _window->getPanel()->release();
    delete _demoGameData;
}

/** 
 * common behaviour
 */

void Location::setWeather(WeatherType weather)
{
    _gameData->weather = weather;

    // setup icon
    gui::IGuiPanel* icon = _window->getPanel()->find( "Weather" );
    switch( _gameData->weather )
    {
    case wtSunny:
        icon->setTextureRect( gui::Rect( 0,0,31,31 ) );
        break;
    case wtVariable:
        icon->setTextureRect( gui::Rect( 32,0,63,31 ) );
        break;
    case wtCloudy:
        icon->setTextureRect( gui::Rect( 64,0,95,31 ) );
        break;
    case wtLightRain:
        icon->setTextureRect( gui::Rect( 96,0,127,31 ) );
        break;
    case wtHardRain:
        icon->setTextureRect( gui::Rect( 128,0, 159,31 ) );
        break;
    case wtThunder:
        icon->setTextureRect( gui::Rect( 160,0,191,31 ) );
        break;
    }
}

void Location::setWind(Vector3f dir, float ambient, float blast)
{
    dir.normalize();
    _gameData->wind        = dir;
    _gameData->windAmbient = ambient;
    _gameData->windBlast   = blast;
}

void Location::setPlayer(bool player)
{
    _gameData->player = player;

    // setup icon
    gui::IGuiPanel* icon = _window->getPanel()->find( "Player" );
    if( _gameData->player )
    {
        icon->setTextureRect( gui::Rect( 0,0,31,31 ) );
    }
    else
    {
        icon->setTextureRect( gui::Rect( 224,0,255,31 ) );
    }
}

Vector2f Location::getLocationCoords(void)
{
    if( _geoscape )
    {
        if( _databaseId == 0 )
        {
            Career* career = _geoscape->getCareer();
            return Vector2f( float( career->getHomeX() ), float( career->getHomeY() ) );
        }
        else
        {
            database::LocationInfo* locationInfo = database::LocationInfo::getRecord( _databaseId );
            return Vector2f( float( locationInfo->worldX ), float( locationInfo->worldY ) );
        }
    }
    else
    {
        return Vector2f( 0,0 );
    }
}

/**
 * weather data
 */

static struct WindMask
{
public:
    Vector3f       dir;
    const wchar_t* name;
} windMask[] =
{
    { Vector3f( 0.0f, 0.0f, 1.0f ), L"S" },
    { Vector3f( -0.5f, 0.0f, 1.0f ), L"SSW" },
    { Vector3f( -1.0f, 0.0f, 1.0f ), L"SW" },
    { Vector3f( -1.0f, 0.0f, 0.5f ), L"SWW" },
    { Vector3f( -1.0f, 0.0f, 0.0f ), L"W" },
    { Vector3f( -1.0f, 0.0f, -0.5f ), L"NWW" },
    { Vector3f( -1.0f, 0.0f, -1.0f ), L"NW" },
    { Vector3f( -0.5f, 0.0f, -1.0f ), L"NNW" },
    { Vector3f( -0.0f, 0.0f, -1.0f  ), L"N" },
    { Vector3f( 0.5f, 0.0f, -1.0f  ), L"NNE" },
    { Vector3f( 1.0f, 0.0f, -1.0f ), L"NE" },
    { Vector3f( 1.0f, 0.0f, -0.5f ), L"NEE" },
    { Vector3f( 1.0f, 0.0f, 0.0f ), L"E" },
    { Vector3f( 1.0f, 0.0f, 0.5f ), L"SEE" },
    { Vector3f( 1.0f, 0.0f, 1.0f  ), L"SE" },
    { Vector3f( 0.5f, 0.0f, 1.0f ), L"SSE" },
    { Vector3f( 0.0f, 0.0f, 0.0f ), NULL }
};

const wchar_t* Location::getWindForecast(void)
{
    unsigned int id = 0;
    unsigned int bestId = 0;
    Vector3f temp = windMask[id].dir;
    temp.normalize();
    float dotProduct = Vector3f::dot( getWindDirection(), temp );
    float bestDotProduct = dotProduct;
    id++;
    while( windMask[id].name )
    {
        temp = windMask[id].dir;
        temp.normalize();    
        dotProduct = Vector3f::dot( getWindDirection(), temp );
        if( dotProduct > bestDotProduct )
        {
            bestId = id;
            bestDotProduct = dotProduct;
        }
        id++;
    }
    return windMask[bestId].name;
}

const wchar_t* Location::getWeatherForecast(void)
{
    const wchar_t* weather = NULL;
    switch( getWeather() )
    {
    case wtSunny:
        weather = Gameplay::iLanguage->getUnicodeString(115);
        break;
    case wtVariable:
        weather = Gameplay::iLanguage->getUnicodeString(116);
        break;
    case wtCloudy:
        weather = Gameplay::iLanguage->getUnicodeString(117);
        break;
    case wtLightRain:
        weather = Gameplay::iLanguage->getUnicodeString(118);
        break;
    case wtHardRain:
        weather = Gameplay::iLanguage->getUnicodeString(119);
        break;
    case wtThunder:
        weather = Gameplay::iLanguage->getUnicodeString(120);
        break;
    }
    assert( weather );
    return weather;
}