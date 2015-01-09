
#include "headers.h"
#include "geoscape.h"
#include "gameplay.h"
#include "selecthome.h"
#include "careercourse.h"
#include "scene.h"
#include "fatalitylist.h"
#include "night.h"
#include "travel.h"
#include "../common/istring.h"

Geoscape::Geoscape(Career* career)
{
    _career = career;
    _endOfActivity = false;
    _requestedActivity = NULL;
    _passedTime = 0.0f;
    _blinkingTime = 0.0f;

    // create geoscape window
    _geoscape = Gameplay::iGui->createWindow( "Geoscape" ); assert( _geoscape );
    _geoscape->getPanel()->setRenderCallback( panelRenderCallback, this );
    _moveMap = false;
    _lastPosIsValid = false;
    _lastX = _lastY = 0;
    _currX = _currY = 0;
    _geoscapeMode = NULL;

    // create locations
    if( _career->isHomeDefined() )
    {
        Location* homeLocation = new Location( this, 0 );
        _locations.push_back( homeLocation );
        if( _career->getHomePlacementFlag() )
        {
            homeLocation->setPlayer( true );
            _career->setHomePlacementFlag( false );
        }
    }
    for( unsigned int i=1; i<database::LocationInfo::getNumRecords(); i++ )
    {
        _locations.push_back( new Location( this, i ) );
    }

    // make market offers
    for( unsigned int i=0; i<125; i++ )
    {
        Gear marketOffer = generateGear( getCore()->getRandToolkit()->getUniform( 0, 1 ) > 0.75f );
        if( marketOffer.isTradeable() ) addGearToMarket( marketOffer );
    }

    // MULTIVERSION ISSUE ¹1
    // if player is in inaccessible location
    if( _career->isHomeDefined() )
    {
        Location* location = getPlayerLocation();
        if( location && !database::LocationInfo::getRecord( location->getDatabaseId() )->accessible )
        {
            // move player to the dropzone
            location->setPlayer( false );
            getLocation( unsigned int( 0 ) )->setPlayer( true );
        }
    }

    // MULTIVERSION ISSUE ¹2
    // determine player availability
    if( _career->isHomeDefined() )
    {
        bool playerIsAvailable = false;
        for( unsigned int i=0; i<getNumLocations(); i++ )
        {
            if( getLocation( i )->getPlayer() )
            {
                playerIsAvailable = true;
                break;
            }
        }
        if( !playerIsAvailable )
        {
            for( unsigned int i=0; i<_career->getNumEvents(); i++ )
            {
                if( strcmp( _career->getEvent(i)->getClassName(), TRAVEL_CLASS_NAME ) == 0 )
                {
                    playerIsAvailable = true;
                    break;
                }
            }
        }
        // if player is not avaibalble
        if( !playerIsAvailable )
        {
            // move player to the dropzone
            getLocation( unsigned int( 0 ) )->setPlayer( true );
        }
    }

    // MULTIVERSION ISSUE ¹3
    // demermine player is travelling
    if( _career->isHomeDefined() )
    {
        bool playerIsTravelling = false;
        for( unsigned int i=0; i<_career->getNumEvents(); i++ )
        {
            if( strcmp( _career->getEvent(i)->getClassName(), TRAVEL_CLASS_NAME ) == 0 )
            {
                playerIsTravelling = true;
                break;
            }
        }
        // if player is travelling
        if( playerIsTravelling )
        {
            // make sure that player isn't in a location
            for( unsigned int i=0; i<getNumLocations(); i++ )
            {
                getLocation( i )->setPlayer( false );            
            }
        }
        else
        {
            // make sure that player is in single location
            bool playerLocationIsFound = false;
            for( unsigned int i=0; i<getNumLocations(); i++ )
            {
                if( !playerLocationIsFound )
                {
                    if( getLocation( i )->getPlayer() )
                    {
                        playerLocationIsFound = true;
                    }
                }
                else
                {
                    getLocation( i )->setPlayer( false );
                }            
            }
        }
    }

    // prompt strings
    addHistory( Gameplay::iLanguage->getUnicodeString(288), Vector4f( 0.25,1,0.25,1 ) );
    addHistory( Gameplay::iLanguage->getUnicodeString(289), Vector4f( 0.25,1,0.25,1 ) );
}

Geoscape::~Geoscape()
{
    for( unsigned int i=0; i<_locations.size(); i++ ) delete _locations[i];
    _geoscape->getPanel()->setRenderCallback( NULL, NULL );
    _geoscape->getPanel()->release();
}

void Geoscape::updateActivity(float dt)
{
    // update playing time
    getCareer()->getVirtues()->statistics.playingTime += dt;

    // map scrolling
    onUpdateMapScrolling( dt );

    // check player is alive
    if( _career->getVirtues()->evolution.health == 0 ) _endOfActivity = true;

    // update geoscape mode
    if( _geoscapeMode ) 
    {
        _geoscapeMode->onUpdateMode( dt );
    }
    if( _geoscapeMode && _geoscapeMode->endOfMode() )
    {
        // home selection is complete
        SelectHome* selectHome = dynamic_cast<SelectHome*>( _geoscapeMode );
        if( selectHome )
        {
            _career->setHome( selectHome->getHomeX(), selectHome->getHomeY() );
            Location* homeLocation = new Location( this, 0 );
            homeLocation->setPlayer( true );
            _locations.push_back( homeLocation );
            Gameplay::iGameplay->saveCareers();
            // remove mode
            delete _geoscapeMode;
            _geoscapeMode = new CareerCourse( this );
        }        
    }

    // color location
    gui::IGuiPanel* actionPanel;
    engine::ITexture* yellowLocation = Gameplay::iEngine->getTexture( "location1" ); assert( yellowLocation );
    engine::ITexture* greenLocation = Gameplay::iEngine->getTexture( "location2" ); assert( greenLocation );
    engine::ITexture* redLocation = Gameplay::iEngine->getTexture( "location3" ); assert( redLocation );
	engine::ITexture* nightLocation = Gameplay::iEngine->getTexture( "location_night" ); assert( nightLocation );

    bool nightEvent = false;
	unsigned int i;
    for( i=0; i<_career->getNumEvents(); i++ )
    {
        if( strcmp( _career->getEvent( i )->getClassName(), NIGHT_CLASS_NAME ) == 0 )            
        {
            if( _career->getEvent( i )->getFlags() & efActive )
            {
                nightEvent = true;
                break;
            }
        }
    }

	DateTime datetime = getDateTime();

    for( i=0; i<_locations.size(); i++ )
    {
        actionPanel = _locations[i]->getWindow()->getPanel()->find( "Action" ); assert( actionPanel );
        if( _locations[i]->getPlayer() )
        {
            if( ( _career->getVirtues()->evolution.health < 0.75f ) )
            {
                actionPanel->setTexture( redLocation );
            }
			else if ( datetime.hour < 6 || datetime.hour > 22 ) {
				actionPanel->setTexture( nightLocation );
			}
            else
            {
                actionPanel->setTexture( greenLocation );
            }
        }
        else
        {
            actionPanel->setTexture( yellowLocation );
        }
    }

    // blink location
    _blinkingTime += dt;
    if( _blinkingLocations.size() > 0 )
    {
        float alpha = 0.5f * ( 1 + sin( 16 * _blinkingTime ) );
        unsigned int i,j;
        bool isBlinking;
        for( i=0; i<_locations.size(); i++ )
        {
            isBlinking = false;
            for( j=0; j<_blinkingLocations.size(); j++ )
            {
                if( _locations[i]->getDatabaseId() == _blinkingLocations[j] )
                {
                    isBlinking = true;
                    break;
                }
            }
            gui::IGuiPanel* action = _locations[i]->getWindow()->getPanel()->find( "Action" ); assert( action && action->getButton() );
            Vector4f color = action->getColor();
            Vector4f inactiveColor = action->getButton()->getInactiveColor();
            color[3] = isBlinking ? alpha : inactiveColor[3];
            action->setColor( color );
        }
    }

    // update lifetime of history entries
    for( i=0; i<_history.size(); i++ ) _history[i].lifetime -= dt;
    bool found;
    do
    {
        found = false;
        for( i=0; i<_history.size(); i++ ) 
        {
            // fade alpha
            if( _history[i].lifetime < 1 )
            {
                _history[i].color[3] = _history[i].lifetime;
            }
            // remove dead history
            if( _history[i].lifetime < 0 ) 
            {
				_history.erase( _history.begin() + i );
                found = true;
                break;
            }
        }
    }
    while( found );

    // render if mode is active
    if( !_endOfActivity )
    {
        // begin render
        Gameplay::iEngine->getDefaultCamera()->beginScene( 
            engine::cmClearColor | engine::cmClearDepth,
    	    Vector4f( 0,0,0,0 )
        );

        // render gui
        Gameplay::iGui->render();

        // render lat/long
        //Vector3f latitude  = getCursorLatitude();
        //Vector3f longitude = getCursorLongitude();
        //std::string text = strformat( "lat: %1.0f %1.0f' %1.0f''\nlong: %1.0f %1.0f' %1.0f''", latitude[0], latitude[1], latitude[2], longitude[0], longitude[1], longitude[2] );
        //Gameplay::iGui->renderText( gui::Rect( 2,2,258,66 ), "tiny", Vector4f( 0,0,0,1 ), gui::atLeft, gui::atTop, false, text.c_str() );
        //Gameplay::iGui->renderText( gui::Rect( 0,0,256,64 ), "tiny", Vector4f( 1,1,1,1 ), gui::atLeft, gui::atTop, false, text.c_str() );

        // end render
        Gameplay::iEngine->getDefaultCamera()->endScene();
        Gameplay::iEngine->present();
    }

    // perform post-requests
    if( _requestedActivity )
    {
        Gameplay::iGameplay->pushActivity( _requestedActivity );
        _requestedActivity = NULL;
    }
}

bool Geoscape::endOfActivity(void)
{
    return _endOfActivity;
}

void Geoscape::onBecomeActive(void)
{
    // insert window in Gui
    Gameplay::iGui->getDesktop()->insertPanel( _geoscape->getPanel() );
    _geoscape->align( gui::atCenter, 0, gui::atCenter, 0 );

    // register message callback
    Gameplay::iGui->setMessageCallback( messageCallback, this );

    // startup geoscape mode
    if( !_career->isHomeDefined() )
    {
        _geoscapeMode = new SelectHome( this );
    }
    else
    {
        CareerCourse* careerCourse = new CareerCourse( this );
        _geoscapeMode = careerCourse;

        // simulate course
        while( _passedTime > 0 )
        {
            _passedTime -= careerCourse->simulateCourse( _passedTime, false );
        }
        if( _passedTime < 0 ) _passedTime = 0;

        // update gui
        careerCourse->updateGui();
    }
}

void Geoscape::onBecomeInactive(void)
{
    // end mode
    if( _geoscapeMode ) 
    {
        delete _geoscapeMode;
        _geoscapeMode = NULL;
    }

    // remove window from Gui
    Gameplay::iGui->getDesktop()->removePanel( _geoscape->getPanel() );

    // unregister message callback
    Gameplay::iGui->setMessageCallback( NULL, NULL );
}

void Geoscape::onReturnFromActivity(Activity* activity)
{
    // return from scene mode
    Scene* scene = dynamic_cast<Scene*>( activity );
    if( scene ) 
    {
        // obtain time passed in scene
        _passedTime = scene->getPassedTime();
        // check player health
        if( _career->getVirtues()->evolution.health == 0.0f )
        {
            // add incident
            Location* playerLocation = getPlayerLocation(); assert( playerLocation );
            requestActivity( new FatalityList( _career, playerLocation->getDatabaseId() ) );
        }
    }
}

/**
 * private behaviour
 */

gui::IGuiWindow* Geoscape::getWindow(void)
{
    return _geoscape;
}

Career* Geoscape::getCareer(void)
{
    return _career;
}

int Geoscape::getCursorX(void)
{
    // obtain absolute coordinates in geoscape space
    gui::Rect georect = _geoscape->getPanel()->getRect();   
    return _currX - georect.left;
}

int Geoscape::getCursorY(void)
{
    // obtain absolute coordinates in geoscape space
    gui::Rect georect = _geoscape->getPanel()->getRect();
    return _currY - georect.top;
}

Vector3f Geoscape::getCursorLatitude(void)
{
    // obtain absolute coordinates in geoscape space
    gui::Rect georect = _geoscape->getPanel()->getRect();    
    float cursorY = float( _currY - georect.top );

    // obtain relative coordinates    
    cursorY = 0.5f * ( georect.bottom - georect.top ) - cursorY;

    // obtain latitude ratio
    float ratio = 180.0f / float( georect.bottom - georect.top );

    // obtain latitude angle, minute & second
    float angle = cursorY * ratio;
    float minute = ( angle - int( angle ) ) * 60.0f;
    float second = ( minute - int( minute ) ) * 60.0f;
    angle = float(int(angle));

    // result
    return Vector3f( angle, fabs(minute), fabs(second) );
}

Vector3f Geoscape::getCursorLongitude(void)
{
    // obtain absolute coordinates in geoscape space
    gui::Rect georect = _geoscape->getPanel()->getRect();   
    float cursorX = float( _currX - georect.left);

    // obtain relative coordinates    
    cursorX = 0.5f * ( georect.right - georect.left ) - cursorX;

    // obtain longitude ratio
    float ratio = 360.0f / float( georect.right - georect.left );

    // obtain longitude angle, minute & second
    float angle = -cursorX * ratio;
    float minute = ( angle - int( angle ) ) * 60.0f;
    float second = ( minute - int( minute ) ) * 60.0f;
    angle = float(int(angle));

    // result
    return Vector3f( angle, fabs(minute), fabs(second) );
}

unsigned int Geoscape::getNumLocations(void)
{
    return _locations.size();
}

Location* Geoscape::getLocation(unsigned int id)
{
    assert( id>=0 && id<_locations.size() );
    return _locations[id];
}

Location* Geoscape::getLocation(gui::IGuiPanel* panel)
{
    for( unsigned int i=0; i<_locations.size(); i++ )
    {
        gui::IGuiWindow* thisWindow = _locations[i]->getWindow();
        gui::IGuiPanel* thisPanel = thisWindow->getPanel()->find( panel->getName() );
        if( thisPanel == panel ) return _locations[i];
    }
    return NULL;
}

Location* Geoscape::getPlayerLocation(void)
{
    for( unsigned int i=0; i<_locations.size(); i++ )
    {
        if( _locations[i]->getPlayer() ) return _locations[i];
    }
    return NULL;
}

Location* Geoscape::getHomeLocation(void)
{
    for( unsigned int i=0; i<_locations.size(); i++ )
    {
        if( _locations[i]->getDatabaseId() == 0 ) return _locations[i];
    }
    assert( !"Logic error - home location not found" );
    return NULL;
}

DateTime Geoscape::getDateTime(void)
{
    return DateTime( _career->getVirtues()->evolution.time );
}

void Geoscape::requestActivity(Activity* activity)
{
    assert( _requestedActivity == NULL );
    _requestedActivity = activity;
}

void Geoscape::setBlinkingLocations(Event* event)
{
    assert( event );
    event->enumerateLocationIds( _blinkingLocations );
}

void Geoscape::setNoBlinkingLocations(void)
{
    _blinkingLocations.clear();

    for( unsigned int i=0; i<_locations.size(); i++ )
    {
        gui::IGuiPanel* action = _locations[i]->getWindow()->getPanel()->find( "Action" ); assert( action && action->getButton() );
        Vector4f color = action->getColor();
        Vector4f inactiveColor = action->getButton()->getInactiveColor();
        color[3] = inactiveColor[3];
        action->setColor( color );
    }
}

unsigned int Geoscape::getHistorySize(void)
{
    return _history.size();
}

void Geoscape::addHistory(const wchar_t* message, const Vector4f color)
{
    _history.push_back( HistoryEntry( message, color, 10.0f ) );
}

void Geoscape::addGearToMarket(Gear gear)
{
    _market.push_back( gear );
}

void Geoscape::removeGearFromMarket(unsigned int id)
{
    assert( id < _market.size() );
    _market.erase( _market.begin() + id );
}

unsigned int Geoscape::getMarketSize(void)
{
    return _market.size();
}

Gear Geoscape::getGearFromMarket(unsigned int id)
{
    assert( id < _market.size() );
    return _market[id];
}

Gear Geoscape::generateGear(bool generateUsedGear)
{
    Gear result;
        
    // generate gear type
    switch( getCore()->getRandToolkit()->getUniformInt() % 5 )
	//switch( 4 )
    {
    case 0: 
        result.type = gtHelmet;
        result.id   = getCore()->getRandToolkit()->getUniformInt() % database::Helmet::getNumRecords();
        assert( result.id < database::Helmet::getNumRecords() );
        break;
    case 1: 
        result.type = gtSuit; 
        result.id   = getCore()->getRandToolkit()->getUniformInt() % database::Suit::getNumRecords();
        assert( result.id < database::Suit::getNumRecords() );
        break;
    case 2: 
        result.type = gtRig; 
        result.id   = getCore()->getRandToolkit()->getUniformInt() % database::Rig::getNumRecords();
        assert( result.id < database::Rig::getNumRecords() );
        break;
    case 3: 
        result.type = gtCanopy; 
        result.id   = getCore()->getRandToolkit()->getUniformInt() % database::Canopy::getNumRecords();
        assert( result.id < database::Canopy::getNumRecords() );
        break;
    case 4: 
        result.type = gtReserve; 
        result.id   = getCore()->getRandToolkit()->getUniformInt() % database::Canopy::getReserveNumRecords();
        assert( result.id < database::Canopy::getReserveNumRecords() );
        break;
    default:
        assert( !"shouldn't be here!" );
    }

    if( generateUsedGear )
    {
        result.state = getCore()->getRandToolkit()->getUniform( 0.5f, 1.0f );
        result.age   = unsigned int( 1000.0f * ( 1.0f - result.state ) * getCore()->getRandToolkit()->getUniform( 0.75f, 1.25f ) );
    }
    else
    {
        result.state = 1.0f;
        result.age   = 0;
    }

    return result;
}

/**
 * gui message handling
 */

void Geoscape::messageCallback(gui::Message* message, void* userData)
{
    Geoscape* __this = reinterpret_cast<Geoscape*>( userData );
    
    // move geoscape
    if( message->origin == __this->_geoscape->getPanel() )
    {
        __this->setNoBlinkingLocations();

        gui::Rect desktop = Gameplay::iGui->getDesktop()->getRect();

        if( message->event == gui::onMouseMove )
        {
            __this->_currX = message->mouseX;
            __this->_currY = message->mouseY;
        }

        if( message->event == gui::onMouseDown && message->mouseButton == gui::mbRight )
        {
            __this->_moveMap = true;
        }
        else if( message->event == gui::onMouseUp && message->mouseButton == gui::mbRight )
        {
            __this->_moveMap = false;
            __this->_lastPosIsValid = false;
        }
        else if( message->event == gui::onMouseMove && __this->_moveMap )
        {            
            if( __this->_lastPosIsValid )
            {
                gui::Rect rect = __this->_geoscape->getPanel()->getRect();
                rect.left   += message->mouseX - __this->_lastX;
                rect.right  += message->mouseX - __this->_lastX;
                rect.top    += message->mouseY - __this->_lastY;
                rect.bottom += message->mouseY - __this->_lastY;
                if( rect.left > 0 )
                {
                    rect.right -= rect.left;
                    rect.left = 0;
                }
                if( rect.right < desktop.right )
                {
                    rect.left += ( desktop.right - rect.right );
                    rect.right = desktop.right;
                }
                if( rect.top > 0 )
                {
                    rect.bottom -= rect.top;
                    rect.top = 0;
                }
                if( rect.bottom < desktop.bottom )
                {
                    rect.top += ( desktop.bottom - rect.bottom );
                    rect.bottom = desktop.bottom;
                }
                __this->_geoscape->getPanel()->setRect( rect );
                __this->_lastX = message->mouseX;
                __this->_lastY = message->mouseY;
            }
            else
            {
                __this->_lastPosIsValid = true;
                __this->_lastX = message->mouseX;
                __this->_lastY = message->mouseY;
            }
        }
    }
    // cursor is above of event button
    else if( message->event == gui::onEnterCursor &&
             strcmp( message->origin->getName(), "EventImage" ) == 0 )
    {
        gui::IGuiPanel* eventPanel = message->origin->getParent(); assert( eventPanel );
        Event* event;
        for( unsigned int i=0; i<__this->getCareer()->getNumEvents(); i++ )
        {
            event = __this->getCareer()->getEvent( i );
            if( event->getWindow()->getPanel() == eventPanel )
            {
                __this->setBlinkingLocations( event );
                break;
            }
        }
    }
    // cursor is leave area of event button
    else if( message->event == gui::onLeaveCursor &&
             strcmp( message->origin->getName(), "EventImage" ) == 0 )
    {
        __this->setNoBlinkingLocations();
    }
    // click on event button
    else if( message->event == gui::onButtonClick &&
             strcmp( message->origin->getName(), "EventImage" ) == 0 &&
             __this->_blinkingLocations.size() > 0 )
    {
        // determine center of event area
        int x = 0, y = 0;
        for( unsigned int i=0; i<__this->_blinkingLocations.size(); i++ )
        {
            if( __this->_blinkingLocations[i] == 0 )
            {
                x += __this->getCareer()->getHomeX();
                y += __this->getCareer()->getHomeY();
            }
            else
            {
                x += database::LocationInfo::getRecord( __this->_blinkingLocations[i] )->worldX;
                y += database::LocationInfo::getRecord( __this->_blinkingLocations[i] )->worldY;
            }
        }
        x /= __this->_blinkingLocations.size();
        y /= __this->_blinkingLocations.size();

        __this->centerGeoscape( x,y );
    }

    // let mode to handle this message
    if( __this->_geoscapeMode ) __this->_geoscapeMode->onGuiMessage( message );
}

void Geoscape::panelRenderCallback(gui::IGuiPanel* panel, void* data)
{
    Geoscape* __this = reinterpret_cast<Geoscape*>( data );

    // prepare to render history
    Vector3f screenSize = Gameplay::iEngine->getScreenSize();
    int   maxWidth  = 640;
    int   maxHeight = 128;
    int   left      = int( screenSize[0] ) - maxWidth;
    int   bottom    = int( screenSize[1] );
    int   width, height;
    int   centeredLeft;

    // render history
    gui::Rect textRect;
    for( unsigned int i=0; i<__this->_history.size(); i++ )
    {
        // calculate rect
        textRect = Gameplay::iGui->calcUnicodeTextRect( 
            gui::Rect( 0,0,maxWidth, maxHeight ), 
            "caption", 
            gui::atTop, gui::atCenter, true, 
            __this->_history[ __this->_history.size()-i-1 ].message.c_str() 
        );
        height = textRect.getHeight(); 
        width  = textRect.getWidth();
        textRect.left += left;
        textRect.right += left;
        textRect.bottom = bottom;
        textRect.top = textRect.bottom - height;
        // center text
        centeredLeft = ( int( screenSize[0] ) - textRect.getWidth() ) / 2;
        textRect.left = centeredLeft;
        textRect.right = textRect.left + width;
        // render shadow
        Gameplay::iGui->renderUnicodeText(
            textRect,
            "caption", 
            Vector4f( 0, 0, 0, 0.5f * __this->_history[ __this->_history.size()-i-1 ].color[3]),
            gui::atTop, gui::atCenter, true, 
            __this->_history[ __this->_history.size()-i-1 ].message.c_str(),
            false
        );
        // modify anchor
        bottom = textRect.top - 8;
        // render text
        textRect.left -= 1, textRect.right -= 1,
        textRect.top  -= 1, textRect.bottom -= 1;
        Gameplay::iGui->renderUnicodeText(
            textRect,
            "caption", 
            __this->_history[ __this->_history.size()-i-1 ].color,
            gui::atTop, gui::atCenter, true, 
            __this->_history[ __this->_history.size()-i-1 ].message.c_str(),
            false
        );
    }
}

const float scrollSpeed = 384.0f;

void Geoscape::onUpdateMapScrolling(float dt)
{
    if( Gameplay::iGui->getPanelUnderCursor() == NULL || 
        Gameplay::iGui->getPanelUnderCursor() != _geoscape->getPanel() )
    {
        return;
    }

    Vector3f screenSize = Gameplay::iEngine->getScreenSize();

    gui::Rect cursor = Gameplay::iGui->getCursorRect();
    gui::Rect desktop = Gameplay::iGui->getDesktop()->getRect();
    gui::Rect rect = _geoscape->getPanel()->getRect();

    bool isScrolled = false;

    if( cursor.left < 8 )
    {
        isScrolled = true;
        rect.left  += int( scrollSpeed * dt );
        rect.right += int( scrollSpeed * dt );
    }
    if( cursor.left > screenSize[0] - 8 )
    {
        isScrolled = true;
        rect.left  -= int( scrollSpeed * dt );
        rect.right -= int( scrollSpeed * dt );
    }
    if( cursor.top < 8 )
    {
        isScrolled = true;
        rect.top    += int( scrollSpeed * dt );
        rect.bottom += int( scrollSpeed * dt );
    }
    if( cursor.top > screenSize[1] - 8 )
    {
        isScrolled   = true;
        rect.top    -= int( scrollSpeed * dt );
        rect.bottom -= int( scrollSpeed * dt );
    }

    if( isScrolled )
    {
        // align rectangle
        if( rect.left > 0 )
        {
            rect.right -= rect.left;
            rect.left = 0;
        }
        if( rect.right < desktop.right )
        {
            rect.left += ( desktop.right - rect.right );
            rect.right = desktop.right;
        }
        if( rect.top > 0 )
        {
            rect.bottom -= rect.top;
            rect.top = 0;
        }
        if( rect.bottom < desktop.bottom )
        {
            rect.top += ( desktop.bottom - rect.bottom );
            rect.bottom = desktop.bottom;
        }
        _geoscape->getPanel()->setRect( rect );
    }
}

void Geoscape::centerGeoscape(int x, int y)
{
    // determine current center of visible geoscape
    gui::Rect desktopRect = Gameplay::iGui->getDesktop()->getRect();
    gui::Rect oldGeoscapeRect = _geoscape->getPanel()->getRect();
    int Gx = -oldGeoscapeRect.left + desktopRect.getWidth() / 2;
    int Gy = -oldGeoscapeRect.top + desktopRect.getHeight() / 2;
    // determine offset
    int Ox = Gx - x;
    int Oy = Gy - y;       
    // superpose geoscape
    gui::Rect newGeoscapeRect;
    newGeoscapeRect.left   = oldGeoscapeRect.left   + Ox;
    newGeoscapeRect.right  = oldGeoscapeRect.right  + Ox;
    newGeoscapeRect.top    = oldGeoscapeRect.top    + Oy;
    newGeoscapeRect.bottom = oldGeoscapeRect.bottom + Oy;
    // align geoscape
    if( newGeoscapeRect.left > 0 )
    {
        newGeoscapeRect.right -= newGeoscapeRect.left;
        newGeoscapeRect.left = 0;
    }
    if( newGeoscapeRect.right < desktopRect.right )
    {
        newGeoscapeRect.left += ( desktopRect.right - newGeoscapeRect.right );
        newGeoscapeRect.right = desktopRect.right;
    }
    if( newGeoscapeRect.top > 0 )
    {
        newGeoscapeRect.bottom -= newGeoscapeRect.top;
        newGeoscapeRect.top = 0;
    }
    if( newGeoscapeRect.bottom < desktopRect.bottom )
    {
        newGeoscapeRect.top += ( desktopRect.bottom - newGeoscapeRect.bottom );
        newGeoscapeRect.bottom = desktopRect.bottom;
    }
    _geoscape->getPanel()->setRect( newGeoscapeRect );
}

void Geoscape::closeGeoscapeMode(void)
{
    _endOfActivity = true;
}