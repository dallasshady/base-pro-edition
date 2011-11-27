
#include "headers.h"
#include "missionbrowser.h"
#include "imath.h"
#include "database.h"
#include "equip.h"
#include "mission.h"

/**
 * mission browser camera
 */
 
MissionBrowser::Camera::Camera(Scene* scene, Matrix4f pose, float fov) : Actor(scene)
{
    _name         = "MissionBrowserCamera";
    _cameraFOV    = fov;
    _cameraMatrix = pose;
}

MissionBrowser::Camera::~Camera()
{
}

void MissionBrowser::Camera::onUpdateActivity(float dt)
{
    // RT-RS pass
    Gameplay::iGameplay->getRenderTarget()->render( _scene, _cameraMatrix, _cameraFOV, true, false );

    // gui
    Gameplay::iEngine->getDefaultCamera()->beginScene( 0, Vector4f( 0,0,0,0 ) );
    Gameplay::iGui->render();
    Gameplay::iEngine->getDefaultCamera()->endScene();

    // present result
    Gameplay::iEngine->present();
}

/**
 * mission browser mode : abstraction layer
 */

void MissionBrowser::onUpdateActivity(float dt)
{
    // update footnote
    _footnoteTime -= dt;
    _footnoteTime = _footnoteTime < 0 ? 0 : _footnoteTime;
    _footnote->getPanel()->setVisible( _footnoteTime > 0 );
}

void MissionBrowser::onSuspend(void)
{
    _scene->setCamera( NULL );
    Gameplay::iGui->getDesktop()->setColor( Vector4f( 1,1,1,1 ) );
    Gameplay::iGui->getDesktop()->removePanel( _browser->getPanel() );
    Gameplay::iGui->getDesktop()->removePanel( _footnote->getPanel() );
    Gameplay::iGui->setMessageCallback( NULL, NULL );
    releaseThumbnails();
}

void MissionBrowser::onResume(void)
{
    Gameplay::iGui->setMessageCallback( messageCallback, this );
    Gameplay::iGui->getDesktop()->setColor( Vector4f( 0,0,0,0 ) );
    Gameplay::iGui->getDesktop()->insertPanel( _browser->getPanel() );
    Gameplay::iGui->getDesktop()->insertPanel( _footnote->getPanel() );

    _browser->align( gui::atCenter, 0, gui::atCenter, 0 );
    _footnote->align( gui::atBottom, 16, gui::atCenter, 0 );

    _scene->setCamera( _camera );

    // show missions
    updateGui();
}

bool MissionBrowser::endOfMode(void)
{
    return false;
}

/**
 * mission browser mode : class implementation
 */

MissionBrowser::MissionBrowser(Scene* scene, const Matrix4f& cameraPose, float cameraFOV) : 
    Mode(scene)
{
    assert( scene );

    _camera = new Camera( scene, cameraPose, cameraFOV );
    _browser = Gameplay::iGui->createWindow( "MissionBrowser" ); assert( _browser );

    _source.push( new TournamentSource( _scene->getLocation()->getDatabaseId(), scene->getCareer() ) );
    _topItem      = 0;
    _selectedItem = -1;

    _footnote = Gameplay::iGui->createWindow( "Footnote" ); assert( _footnote );
    _footnoteTime = 0.0f;
}

MissionBrowser::~MissionBrowser()
{
    while( _source.size() )
    {
        delete _source.top();
        _source.pop();
    }
    releaseThumbnails();
    _browser->getPanel()->release();
    delete _camera;
}

/**
 * mission browser mode : gui messaging
 */

void MissionBrowser::onClickItem(unsigned int itemId)
{
    // second click?
    if( _selectedItem == itemId )
    {
        if( _source.top()->browseBack( _selectedItem ) )
        {
            assert( _source.size() );
            delete _source.top();
            _source.pop();
            _selectedItem = -1;
        }
        else
        {
            // location wind info
            float windAmbient = _scene->getLocation()->getWindAmbient();
            float windBlast = _scene->getLocation()->getWindBlast();
            if( !database::LocationInfo::getRecord( _scene->getLocation()->getDatabaseId() )->wind )
            {
                // wind doesn't matters?
                windAmbient = 0;
                windBlast = 0;
            }

            // equip routine
            if( _source.top()->equip( _selectedItem, _scene->getCareer(), windAmbient, windBlast ) )
            {
                BrowserSource* newSource = _source.top()->browse( _selectedItem, _scene, this );
                if( newSource ) 
                {
                    _source.push( newSource );
                    _selectedItem = -1;
                }
            }
            else
            {
                setFootnote( Gameplay::iLanguage->getUnicodeString(594), 3.0f );
            }
        }
    }
    // first click?
    else
    {
        _selectedItem = itemId;
    }

    // update browser appearance
    updateGui();
}

void MissionBrowser::messageCallback(gui::Message* message, void* userData)
{
    MissionBrowser* __this = reinterpret_cast<MissionBrowser*>( userData );

    // mouse wheel event?
    if( message->event == gui::onMouseWheel )
    {
        gui::IGuiPanel* slider =__this->_browser->getPanel()->find( "Slider" );
        assert( slider && slider->getSlider() );
        __this->_topItem += int( message->mouseX );
        slider->getSlider()->setPosition( float( __this->_topItem ) );
        __this->updateGui();
    }

    // slider event?
    if( message->event == gui::onSlide )
    {
        gui::IGuiPanel* slider =__this->_browser->getPanel()->find( "Slider" );
        assert( slider && slider->getSlider() );
        __this->_topItem = unsigned int( slider->getSlider()->getPosition() );
        __this->updateGui();
    }

    // button event
    if( message->event == gui::onButtonClick )
    {
        // leave button?
        if( strcmp( message->origin->getName(), "Leave" ) == 0 )
        {
            __this->_scene->endOfScene();
        }
        // start button?
        else if( strcmp( message->origin->getName(), "Start" ) == 0 )
        {
            if( __this->_selectedItem != -1 ) __this->onClickItem( __this->_selectedItem );
        }
        // item button?
        else if( strcmp( message->origin->getName(), "Thumbnail" ) == 0 ||
                 strcmp( message->origin->getName(), "Name" ) == 0 )
        {
            if( strcmp( message->origin->getParent()->getName(), "Item01" ) == 0 )
            {
                __this->onClickItem( __this->_topItem + 0 );
            }
            else if( strcmp( message->origin->getParent()->getName(), "Item02" ) == 0 )
            {
                __this->onClickItem( __this->_topItem + 1 );
            }
            else if( strcmp( message->origin->getParent()->getName(), "Item03" ) == 0 )
            {
                __this->onClickItem( __this->_topItem + 2 );
            }
            else if( strcmp( message->origin->getParent()->getName(), "Item04" ) == 0 )
            {
                __this->onClickItem( __this->_topItem + 3 );
            }
            else if( strcmp( message->origin->getParent()->getName(), "Item05" ) == 0 )
            {
                __this->onClickItem( __this->_topItem + 4 );
            }
        }
        // scroll buttons?
        else if( strcmp( message->origin->getName(), "ScrollUp" ) == 0 )
        {
            __this->_topItem--;
            gui::IGuiPanel* slider =__this->_browser->getPanel()->find( "Slider" );
            assert( slider && slider->getSlider() );            
            slider->getSlider()->setPosition( float( __this->_topItem ) );
            __this->updateGui();
        }
        else if( strcmp( message->origin->getName(), "ScrollDown" ) == 0 )
        {
            __this->_topItem++;
            gui::IGuiPanel* slider =__this->_browser->getPanel()->find( "Slider" );
            assert( slider && slider->getSlider() );            
            slider->getSlider()->setPosition( float( __this->_topItem ) );
            __this->updateGui();
        }
    }
}

/**
 * private behaviour
 */

void MissionBrowser::updateGui(void)
{
    // check bounds
    if( _topItem < 0 ) _topItem = 0;
    if( _source.top()->getNumItems() <= MBGUI_NUMITEMS ) _topItem = 0;
    if( _source.top()->getNumItems() > MBGUI_NUMITEMS && 
        _topItem + MBGUI_NUMITEMS > int( _source.top()->getNumItems() ) )
    {
        _topItem = _source.top()->getNumItems() - MBGUI_NUMITEMS;
    }

    // retrieve slider
    gui::IGuiPanel* slider = _browser->getPanel()->find( "Slider" );
    assert( slider && slider->getSlider() );

    // update slider
    float lowerLimit = 0.0f;
    float upperLimit = float( _source.top()->getNumItems() - MBGUI_NUMITEMS );
    if( upperLimit < 0 ) upperLimit = 0;
    slider->getSlider()->setLowerLimit( lowerLimit );
    slider->getSlider()->setUpperLimit( upperLimit );

    // enable gui items    
    for( unsigned int i=0; i<MBGUI_NUMITEMS; i++ )
    {        
        // slot item
        gui::IGuiPanel* item = NULL;        
        switch( i )
        {
        case 0: 
            item = _browser->getPanel()->find( "Item01" );
            break;
        case 1:
            item = _browser->getPanel()->find( "Item02" );
            break;
        case 2:
            item = _browser->getPanel()->find( "Item03" );
            break;
        case 3:
            item = _browser->getPanel()->find( "Item04" );
            break;
        case 4:
            item = _browser->getPanel()->find( "Item05" );
            break;
        }
        assert( item );

        // item controls
        gui::IGuiPanel* thumbnail = item->find( "Thumbnail" ); assert( thumbnail );
        gui::IGuiPanel* name      = item->find( "Name" ); assert( name && name->getButton() );
        gui::IGuiPanel* wtf       = item->find( "WTF" ); assert( wtf );

        // vessel controls
        gui::IGuiPanel* durationVessel = item->find( "DurationVessel" ); assert( durationVessel );
        gui::IGuiPanel* durationLevel  = durationVessel->find( "DurationLevel" ); assert( durationLevel );

        bool itemEnabled = ( i < _source.top()->getNumItems() );
        if( itemEnabled )
        {
            // show item controls
            thumbnail->setVisible( true );
            name->setVisible( true );            
            durationVessel->setVisible( true );

            // show thumbnail
            engine::ITexture* thumbnailTexture = getThumbnail( _source.top()->getThumbnail( i + _topItem ) );
            assert( thumbnailTexture );
            thumbnail->setTexture( thumbnailTexture );
            thumbnail->setTextureRect( gui::Rect( 0,0,thumbnailTexture->getWidth(),thumbnailTexture->getHeight() ) );

            // show name
            /**/name->getButton()->setCaption( _source.top()->getName( i + _topItem ) );

            // show walkthroug flag
            wtf->setVisible( _source.top()->getWalkthroughFlag( _scene->getCareer(), ( i + _topItem ) ) );

            // show duration
            gui::Rect vesselRect = durationVessel->getRect();
            gui::Rect levelRect(
                0, 0,
                int( vesselRect.getWidth() * _source.top()->getDuration( i + _topItem ) / ( 12 * 60 ) ),
                vesselRect.getHeight()
            );

            durationLevel->setRect( levelRect );
        }
        else
        {
            // hide item controls
            thumbnail->setVisible( false );
            name->setVisible( false );
            wtf->setVisible( false );
            durationVessel->setVisible( false );
        }
    }

    float missionTime = 0;

    // update weather icon
    gui::IGuiPanel* weatherIcon = _browser->getPanel()->find( "WeatherIcon" ); assert( weatherIcon );
    switch( _scene->getLocation()->getWeather() )
    {
    case wtSunny:
        weatherIcon->setTextureRect( gui::Rect( 0,0,31,31 ) );
        break;
    case wtVariable:
        weatherIcon->setTextureRect( gui::Rect( 32,0,63,31 ) );
        break;
    case wtCloudy:
        weatherIcon->setTextureRect( gui::Rect( 64,0,95,31 ) );
        break;
    case wtLightRain:
        weatherIcon->setTextureRect( gui::Rect( 96,0,127,31 ) );
        break;
    case wtHardRain:
        weatherIcon->setTextureRect( gui::Rect( 128,0, 159,31 ) );
        break;
    case wtThunder:
        weatherIcon->setTextureRect( gui::Rect( 160,0,191,31 ) );
        break;
    }
    weatherIcon->setVisible( false );

    // hide weather ban
    gui::IGuiPanel* weatherBan = _browser->getPanel()->find( "WeatherBan" ); assert( weatherBan );
    weatherBan->setVisible( false );

    // hide windsock icon
    gui::IGuiPanel* windsockIcon = _browser->getPanel()->find( "WindsockIcon" ); assert( windsockIcon );
    windsockIcon->setVisible( false );

    // hide windsock ban
    gui::IGuiPanel* windsockBan = _browser->getPanel()->find( "WindsockBan" ); assert( windsockBan );
    windsockBan->setVisible( false );

    // item is selected?
    if( _selectedItem != -1 )
    {
        missionTime = _source.top()->getDuration( _selectedItem );

        // controls
        gui::IGuiPanel* description = _browser->getPanel()->find( "Description" ); assert( description && description->getStaticText() );

        // show description
        /**/description->getStaticText()->setText( _source.top()->getBriefing( _selectedItem ) );

        // show icons
        weatherIcon->setVisible( true );
        windsockIcon->setVisible( true );

        // check weather clearance        
        if( !_source.top()->getWeatherClearance( _selectedItem, _scene->getLocation()->getWeather() ) )
        {
            weatherBan->setVisible( true );
        }

        // check wind clearance
        if( !_source.top()->getWindClearance( _selectedItem, _scene->getLocation()->getWindDirection(), _scene->getLocation()->getWindAmbient(), _scene->getLocation()->getWindBlast() ) )
        {
            windsockBan->setVisible( true );
        }
    }

    // show weather
    gui::IGuiPanel* weather = _browser->getPanel()->find( "Weather" ); assert( weather && weather->getStaticText() );

    weather->getStaticText()->setText( wstrformat( 
        Gameplay::iLanguage->getUnicodeString(108), 
        _scene->getLocation()->getWeatherForecast(),
        _scene->getLocation()->getWindForecast(), 
        int( _scene->getLocation()->getWindAmbient() ), 
        int( _scene->getLocation()->getWindBlast() ) 
    ).c_str() );

    // update sandglass

    // localization resources (for sandglass)
    const wchar_t* hhmm = Gameplay::iLanguage->getUnicodeString(154);
    const wchar_t* hh   = Gameplay::iLanguage->getUnicodeString(211);
    const wchar_t* mm   = Gameplay::iLanguage->getUnicodeString(155);

    // sandglass resources
    gui::IGuiPanel* sandglass  = _browser->getPanel()->find( "Sandglass" ); 
    assert( sandglass && sandglass->getWindow() );
    gui::IGuiPanel* greenlevel = sandglass->find( "GreenLevel" ); 
    assert( greenlevel && greenlevel->getWindow() );
    gui::IGuiPanel* redlevel = sandglass->find( "RedLevel" ); 
    assert( greenlevel && greenlevel->getWindow() );    
    gui::IGuiPanel* greenIndicator = sandglass->find( "GreenIndicator" ); 
    assert( greenIndicator && greenIndicator->getStaticText() );
    gui::IGuiPanel* redIndicator = sandglass->find( "RedIndicator" ); 
    assert( redIndicator && redIndicator->getStaticText() );

    // determine time ranges
    float greenTime = _scene->getHoldingTime() - missionTime;
    float redTime   = missionTime;
    if( greenTime < 0 )
    {
        greenTime = 0;
        redTime   = _scene->getHoldingTime();
    }

    // update sandglass
    gui::Rect sandglassRect = sandglass->getRect();
    gui::Rect greenLevelRect;
    greenLevelRect.left   = 0;
    greenLevelRect.top    = 0;
    greenLevelRect.bottom = 15;
    greenLevelRect.right  = int( sandglassRect.getWidth() * greenTime / maxHoldingTime );
    gui::Rect redLevelRect;
    redLevelRect.left = greenLevelRect.right;
    redLevelRect.top  = 0;
    redLevelRect.bottom = 15;
    redLevelRect.right = redLevelRect.left + int( sandglassRect.getWidth() * redTime / maxHoldingTime );
    greenlevel->setRect( greenLevelRect );
    redlevel->setRect( redLevelRect );

    // update dial-plates
    unsigned int minWidth = 52;
    gui::Rect greenIndicatorRect = greenLevelRect;
    gui::Rect redIndicatorRect = redLevelRect;
    if( greenLevelRect.getWidth() < minWidth )
    {
        int lack = ( minWidth - greenLevelRect.getWidth() ) / 2;
        greenLevelRect.left -= lack;
        greenLevelRect.right += lack;
    }
    if( redLevelRect.getWidth() < minWidth )
    {
        int lack = ( minWidth - redLevelRect.getWidth() ) / 2;
        redLevelRect.left -= lack;
        redLevelRect.right += lack;
    }
    DateTime greenDT( greenTime );
    DateTime redDT( redTime );
    greenIndicator->setRect( greenLevelRect );
    if( greenDT.hour == 0 )
    {
        greenIndicator->getStaticText()->setText( wstrformat(
            mm, greenDT.minute
        ).c_str() );
    }
    else if( greenDT.minute == 0 )
    {
        greenIndicator->getStaticText()->setText( wstrformat(
            hh, greenDT.hour
        ).c_str() );
    }
    else
    {
        greenIndicator->getStaticText()->setText( wstrformat(
            hhmm, greenDT.hour, greenDT.minute
        ).c_str() );
    }
    redIndicator->setRect( redLevelRect );
    if( redDT.hour == 0 )
    {
        redIndicator->getStaticText()->setText( wstrformat(
            mm, redDT.minute
        ).c_str() );
    }
    else if( redDT.minute == 0 )
    {
        redIndicator->getStaticText()->setText( wstrformat(
            hh, redDT.hour
        ).c_str() );
    }
    else
    {
        redIndicator->getStaticText()->setText( wstrformat(
            hhmm, redDT.hour, redDT.minute
        ).c_str() );
    }
    if( greenTime == 0 ) greenIndicator->getStaticText()->setText( L"" );
    if( redTime == 0 ) redIndicator->getStaticText()->setText( L"" );

    // update thumbnail
    gui::IGuiPanel* image = _browser->getPanel()->find( "Image" ); assert( image );
    engine::ITexture* thumbnail = NULL;
    if( _selectedItem == -1 )
    {
        thumbnail = getThumbnail( _source.top()->getDefaultThumbnail() ); assert( thumbnail );
    }
    else
    {
        thumbnail = getThumbnail( _source.top()->getThumbnail( _selectedItem ) ); assert( thumbnail );
    }
    image->setTexture( thumbnail );
    image->setTextureRect( gui::Rect( 0,0,thumbnail->getWidth(),thumbnail->getHeight() ) );
}

engine::ITexture* MissionBrowser::getThumbnail(const char* resource)
{
    ThumbnailI thumbnailI = _thumbnails.find( resource );
    if( thumbnailI == _thumbnails.end() )
    {
        engine::ITexture* texture = Gameplay::iEngine->createTexture( resource ); assert( texture );
        _thumbnails.insert( Thumbnail( resource, texture ) );
        return texture;
    }
    else
    {
        return thumbnailI->second;
    }
}

void MissionBrowser::releaseThumbnails(void)
{
    for( ThumbnailI thumbnailI = _thumbnails.begin();
                    thumbnailI != _thumbnails.end();
                    thumbnailI++ )
    {
        thumbnailI->second->release();
    }
    _thumbnails.clear();
}

void MissionBrowser::setFootnote(const wchar_t* text, float time)
{
    gui::IGuiPanel* panel = _footnote->getPanel()->find( "Text" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( text );
    _footnoteTime = time;
    _footnote->getPanel()->setVisible( _footnoteTime > 0 );
}