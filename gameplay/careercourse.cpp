
#include "headers.h"
#include "gameplay.h"
#include "careercourse.h"
#include "database.h"
#include "travel.h"
#include "night.h"
#include "../common/istring.h"
#include "version.h"

/**
 * class implementation
 */

void floatToUIntTime(float time, unsigned int *years, unsigned int *months, unsigned int *days, unsigned int *hours, unsigned int *minutes, unsigned int *seconds) {
	const float SECOND = 1;
	const float MINUTE = 60 * SECOND;
	const float HOUR = 60 * MINUTE;
	const float DAY = 24 * HOUR;
	const float MONTH = 30.41666666666667f * DAY;
	const float YEAR = 365 * MONTH;

	const float consts[] = {YEAR,MONTH,DAY,HOUR,MINUTE,SECOND};
	unsigned int *data[] = {years,months,days,hours,minutes,seconds};
	for (int i = 0; i < 6; ++i) {
		if (data[i] != NULL) {
			*data[i] = (unsigned int)(time / consts[i]);
			time -= (float)*data[i] * consts[i];
		}
	}
}

CareerCourse::CareerCourse(Geoscape* geoscape) : GeoscapeMode( geoscape )
{
    _timeSpeed     = tsPause;
    _timeBlink     = 0;
    _careerDialog = NULL;

    // setup career event callback
    geoscape->getCareer()->setEventCallback( CareerCourse::eventCallback, this );

    // create close career window
    _closeCareer = Gameplay::iGui->createWindow( "CloseCareer" );
    Gameplay::iGui->getDesktop()->insertPanel( _closeCareer->getPanel() );
    _closeCareer->align( gui::atTop, 0, gui::atRight, 0 );

    // create career course window
    _careerCourse = Gameplay::iGui->createWindow( "CareerCourse" );
    Gameplay::iGui->getDesktop()->insertPanel( _careerCourse->getPanel() );
    _careerCourse->align( gui::atTop, 0, gui::atLeft, 0 );
    gui::IGuiPanel* panel = _careerCourse->getPanel()->find( "Face" );
    assert( panel );
    assert( panel->getButton() );
    database::Face* face = database::Face::getRecord( _geoscape->getCareer()->getVirtues()->appearance.face );
    engine::ITexture* faceTexture = Gameplay::iEngine->getTexture( face->iconName );
    if( _geoscape->getCareer()->getLicensedFlag() ) faceTexture = Gameplay::iEngine->getTexture( "face00" );
    assert( faceTexture );
    panel->getButton()->getPanel()->setTexture( faceTexture );
    
    gui::Rect panelRect = panel->getRect();    
    float wAspect = float( faceTexture->getWidth() ) / float( panelRect.getWidth() );
    float hAspect = float( faceTexture->getHeight() ) / float( panelRect.getHeight() );
    
    unsigned int width, height;
    if( wAspect < hAspect )
    {
        width  = unsigned int( panelRect.getWidth() * wAspect );
        height = unsigned int( panelRect.getHeight() * wAspect );
    }
    else
    {
        width  = unsigned int( panelRect.getWidth() * hAspect );
        height = unsigned int( panelRect.getHeight() * hAspect );
    }

    gui::Rect textureRect;
    textureRect.left   = ( faceTexture->getWidth() - width ) / 2;
    textureRect.right  = textureRect.left + width;
    textureRect.top    = ( faceTexture->getHeight() - height ) / 2;
    textureRect.bottom = textureRect.top + height;
    panel->setTextureRect( textureRect );

    // create skills screen window
    _skillsScreen = Gameplay::iGui->createWindow( "SkillsScreen" ); assert( _skillsScreen );
    Gameplay::iGui->getDesktop()->insertPanel( _skillsScreen->getPanel() );
    gui::Rect anchorRect = _careerCourse->getPanel()->getRect();
    gui::Rect popupRect;
    popupRect.left   = anchorRect.right;
    popupRect.top    = anchorRect.top;
    popupRect.right  = popupRect.left + _skillsScreen->getPanel()->getRect().getWidth();
    popupRect.bottom = popupRect.top + _skillsScreen->getPanel()->getRect().getHeight();
    _skillsScreen->getPanel()->setRect( popupRect );
    _skillsScreen->getPanel()->setVisible( false );

    // create funds screen window
    _fundsScreen = Gameplay::iGui->createWindow( "FundsScreen" ); assert( _fundsScreen );
    Gameplay::iGui->getDesktop()->insertPanel( _fundsScreen->getPanel() );
    anchorRect = _careerCourse->getPanel()->getRect();
    popupRect.left   = anchorRect.right;
    popupRect.top    = anchorRect.top;
    popupRect.right  = popupRect.left + _fundsScreen->getPanel()->getRect().getWidth();
    popupRect.bottom = popupRect.top + _fundsScreen->getPanel()->getRect().getHeight();
    _fundsScreen->getPanel()->setRect( popupRect );
    _fundsScreen->getPanel()->setVisible( false );

    // create shop screen window
    _shopScreen = Gameplay::iGui->createWindow( "ShopScreen" ); assert( _shopScreen );
    Gameplay::iGui->getDesktop()->insertPanel( _shopScreen->getPanel() );
    anchorRect = _careerCourse->getPanel()->getRect();
    popupRect.left   = anchorRect.right;
    popupRect.top    = anchorRect.top;
    popupRect.right  = popupRect.left + _shopScreen->getPanel()->getRect().getWidth();
    popupRect.bottom = popupRect.top + _shopScreen->getPanel()->getRect().getHeight();
    _shopScreen->getPanel()->setRect( popupRect );
    _shopScreen->getPanel()->setVisible( false );
    _shopSlider = _shopScreen->getPanel()->find( "ShopSlider" ); 
    assert( _shopSlider && _shopSlider->getSlider() );
    _shopSlider->getSlider()->setLowerLimit( 0 );
    _shopSlider->getSlider()->setUpperLimit( 0 );
    _shopSlider->getSlider()->setPosition( 0 );
    _playerSlider = _shopScreen->getPanel()->find( "PlayerSlider" ); 
    assert( _playerSlider && _playerSlider->getSlider() );
    _playerSlider->getSlider()->setLowerLimit( 0 );
    _playerSlider->getSlider()->setUpperLimit( 0 );
    _playerSlider->getSlider()->setPosition( 0 );
    _gearFilter = ::gtCanopy;

    // create coming events texture
    _comingEventsTexture = Gameplay::iEngine->getTexture( "ev_calendar" );
    if( !_comingEventsTexture ) _comingEventsTexture = Gameplay::iEngine->createTexture( "./res/gui/textures/ev_calendar.dds" );
    assert( _comingEventsTexture );
    _comingEventsTexture->addReference();

    // create & initialize coming events icon
    _comingEventsIcon = Gameplay::iGui->createWindow( "CareerEvent" ); assert( _comingEventsIcon );
    Gameplay::iGui->getDesktop()->insertPanel( _comingEventsIcon->getPanel() );
    _comingEventsIcon->getPanel()->setName( "ComingEventsIcon" );
    gui::IGuiPanel* eventImage = _comingEventsIcon->getPanel()->find( "EventImage" );
    assert( eventImage && eventImage->getButton() );
    eventImage->setTexture( _comingEventsTexture );
    eventImage->setTextureRect( gui::Rect( 0,0,_comingEventsTexture->getWidth(),_comingEventsTexture->getHeight() ) );
    eventImage->setHint( Gameplay::iLanguage->getUnicodeString(320) );
    gui::IGuiPanel* eventImageShadow = _comingEventsIcon->getPanel()->find( "EventImageShadow" );
    assert( eventImageShadow && eventImageShadow->getWindow() );
    eventImageShadow->setTexture( _comingEventsTexture );
    eventImageShadow->setTextureRect( gui::Rect( 0,0,_comingEventsTexture->getWidth(),_comingEventsTexture->getHeight() ) );
    _comingEventsIcon->getPanel()->find( "Duration01" )->release();
    _comingEventsIcon->getPanel()->find( "Duration02" )->release();
    _comingEventsIcon->getPanel()->find( "Duration03" )->release();
    _comingEventsIcon->getPanel()->find( "Duration04" )->release();
    _comingEventsIcon->getPanel()->find( "Duration05" )->release();
    _comingEventsIcon->getPanel()->find( "Duration06" )->release();
    _comingEventsIcon->getPanel()->find( "Duration07" )->release();

    // create & initialize coming events icon
    _activeEventsIcon = Gameplay::iGui->createWindow( "CareerEvent" ); assert( _activeEventsIcon );
    Gameplay::iGui->getDesktop()->insertPanel( _activeEventsIcon->getPanel() );
    _activeEventsIcon->getPanel()->setName( "ActiveEventsIcon" );
    eventImage = _activeEventsIcon->getPanel()->find( "EventImage" );
    assert( eventImage && eventImage->getButton() );
    eventImage->setTexture( _comingEventsTexture );
    eventImage->setTextureRect( gui::Rect( 0,0,_comingEventsTexture->getWidth(),_comingEventsTexture->getHeight() ) );
    eventImage->setHint( Gameplay::iLanguage->getUnicodeString(321) );
    eventImageShadow = _activeEventsIcon->getPanel()->find( "EventImageShadow" );
    assert( eventImageShadow && eventImageShadow->getWindow() );
    eventImageShadow->setTexture( _comingEventsTexture );
    eventImageShadow->setTextureRect( gui::Rect( 0,0,_comingEventsTexture->getWidth(),_comingEventsTexture->getHeight() ) );
    _activeEventsIcon->getPanel()->find( "Duration01" )->release();
    _activeEventsIcon->getPanel()->find( "Duration02" )->release();
    _activeEventsIcon->getPanel()->find( "Duration03" )->release();
    _activeEventsIcon->getPanel()->find( "Duration04" )->release();
    _activeEventsIcon->getPanel()->find( "Duration05" )->release();
    _activeEventsIcon->getPanel()->find( "Duration06" )->release();
    _activeEventsIcon->getPanel()->find( "Duration07" )->release();

    // setup event display mode
    _displayMode = edmActive;

    // start simulation
    simulateCourse( 0.0f, false );

    // show gui
    updateGui();
}

CareerCourse::~CareerCourse()
{
    // reset event callback
    _geoscape->getCareer()->setEventCallback( NULL, NULL );

    // remove dialog
    setCareerDialog( NULL );

    // career
    Career* career = _geoscape->getCareer();

    // pass all of events
    Event* event;
    for( unsigned int i=0; i<career->getNumEvents(); i++ )
    {
        event = career->getEvent( i );
        // remove event control from timeline window
        if( event->getWindow()->getPanel()->getParent() )
        {
            event->getWindow()->getPanel()->getParent()->removePanel( 
                event->getWindow()->getPanel()
            );
        } 
    }

    _closeCareer->getPanel()->release();
    _careerCourse->getPanel()->release();
    _skillsScreen->getPanel()->release();
    _fundsScreen->getPanel()->release();
    _shopScreen->getPanel()->release();
    _comingEventsIcon->getPanel()->release();
    _activeEventsIcon->getPanel()->release();
    _comingEventsTexture->release();    
}

/**
 * private behaviour
 */

static bool eventSort(Event*& event1, Event*& event2)
{
    float value1 = event1->getTimeTo();
    float value2 = event2->getTimeTo();
    return ( value1 < value2 );
}

void CareerCourse::updateGui(void)
{
    // career
    Career* career = _geoscape->getCareer();
	career->getVirtues()->evolution.credits = 3;

	//mycareer
	//career->getVirtues()->evolution.score = 290564.0f;
	//career->getVirtues()->evolution.funds = 15600.0f;
	//career->getVirtues()->statistics.numBaseJumps = 1653;
	//career->getVirtues()->statistics.numSkydives = 621;
	//career->getVirtues()->evolution.workTime = 900.0f;
	//career->setAcrobaticsSkill(acroJumpFromRun, true);
	//career->setAcrobaticsSkill(acroFreeflyFlip, true);
	//career->setAcrobaticsSkill(acroFreeflySitfly, true);
	//career->setAcrobaticsSkill(acroFrontFlip, true);
	//career->setAcrobaticsSkill(acroFrontBackFlip, true);
	//career->setAcrobaticsSkill(acroBackFlip, true);
	//career->setAcrobaticsSkill(acroBackFrontFlip, true);

    // update health
    gui::IGuiPanel* vessel = _careerCourse->getPanel()->find( "HealthVessel" ); assert( vessel );
    gui::IGuiPanel* level  = vessel->find( "HealthLevel" ); assert( level );
    gui::Rect vesselRect = vessel->getRect();
    gui::Rect levelRect;
    levelRect.left   = 0;
    levelRect.right  = int( vesselRect.getWidth() * career->getVirtues()->evolution.health );
    levelRect.top    = 0;
    levelRect.bottom = vesselRect.getHeight();
    level->setRect( levelRect );

    // update credits
    unsigned int maxCredits = 3;
    for( unsigned int i=0; i<maxCredits; i++ )
    {
        gui::IGuiPanel* credit = NULL;
        switch( i )
        {
        case 0:
            credit = _careerCourse->getPanel()->find( "Soul0" ); assert( credit ); break;
        case 1:
            credit = _careerCourse->getPanel()->find( "Soul1" ); assert( credit ); break;
        case 2:
            credit = _careerCourse->getPanel()->find( "Soul2" ); assert( credit ); break;
        default:
            assert( !"shouldn't be here!" );
        }
        if( i <= career->getVirtues()->evolution.credits - 1 )
        {
            credit->setVisible( true );
            credit->setHint( wstrformat( 
                Gameplay::iLanguage->getUnicodeString( 446 ), 
                career->getVirtues()->evolution.credits 
            ).c_str() );
        }
        else
        {
            credit->setVisible( false );
            credit->setHint( L"" );
        }
    }


    // treatment state
    gui::IGuiPanel* treatment = _careerCourse->getPanel()->find( "Treatment" ); assert( treatment );
    if( career->getVirtues()->evolution.health < 0.75f )
    {
        treatment->setColor( Vector4f( 1,1,1,1 ) );
    }
    else
    {
        treatment->setColor( Vector4f( 0,0,0,0 ) );
    }

    // update funds
    gui::IGuiPanel* funds = _careerCourse->getPanel()->find( "Funds" );
    assert( funds && funds->getStaticText() );
    funds->getStaticText()->setText( wstrformat( L"%3.2f$", career->getVirtues()->evolution.funds ).c_str() );

    // update time
    DateTime dateTime = _geoscape->getDateTime();
    gui::IGuiPanel* time = _careerCourse->getPanel()->find( "Time" );
    assert( time && time->getStaticText() );
    time->getStaticText()->setText( wstrformat( 
        L"%d/%d/%d/%d %02d:%02d", dateTime.year, dateTime.month, dateTime.week, dateTime.day, dateTime.hour, dateTime.minute
    ).c_str() );

    // calculate number of events under group icon
    unsigned int numIconedEvents = 0;    
    for( unsigned int i=0; i<career->getNumEvents(); i++ )
    {
        if( _displayMode == ::edmActive )
        {
            if( !( career->getEvent( i )->getFlags() & ::efSystem ) &&
                !( career->getEvent( i )->getFlags() & ::efActive ) ) 
            {
                numIconedEvents++;
            }
        }
        else 
        {
            if( !( career->getEvent( i )->getFlags() & ::efSystem ) &&
                ( career->getEvent( i )->getFlags() & ::efActive ) ) 
            {
                numIconedEvents++;
            }
        }
    }

    // update coming events icon
    if( numIconedEvents )
    {
        std::wstring text;
        gui::IGuiPanel* description;
        gui::IGuiPanel* descriptionShadow;
        if( _displayMode == ::edmActive )
        {
            text = wstrformat( Gameplay::iLanguage->getUnicodeString(250), numIconedEvents );
            _comingEventsIcon->getPanel()->setVisible( true );
            _activeEventsIcon->getPanel()->setVisible( false );
            // text & shadow
            description = _comingEventsIcon->getPanel()->find( "EventDescription" );
            assert( description && description->getStaticText() );
            descriptionShadow = _comingEventsIcon->getPanel()->find( "EventDescriptionShadow" );
            assert( descriptionShadow && descriptionShadow->getStaticText() );
        }
        else
        {
            text = wstrformat( Gameplay::iLanguage->getUnicodeString(255), numIconedEvents );
            _comingEventsIcon->getPanel()->setVisible( false );
            _activeEventsIcon->getPanel()->setVisible( true );
            // text & shadow
            description = _activeEventsIcon->getPanel()->find( "EventDescription" );
            assert( description && description->getStaticText() );
            descriptionShadow = _activeEventsIcon->getPanel()->find( "EventDescriptionShadow" );
            assert( descriptionShadow && descriptionShadow->getStaticText() );
        }
        /**/description->getStaticText()->setText( text.c_str() );
        /**/descriptionShadow->getStaticText()->setText( text.c_str()  );
    }
    else
    {
        _comingEventsIcon->getPanel()->setVisible( false );
        _activeEventsIcon->getPanel()->setVisible( false );
    }

    // pass all events, remove them from desktop
    gui::IGuiPanel* parent;
    for( unsigned int i=0; i<career->getNumEvents(); i++ )
    {
        parent = career->getEvent( i )->getWindow()->getPanel()->getParent();
        if( parent ) parent->removePanel( career->getEvent( i )->getWindow()->getPanel() );
    }

    // build list of visible events
    Event* event;
    std::vector<Event*> visibleEvents;
    if( _displayMode == ::edmActive )
    {
        for( i=0; i<career->getNumEvents(); i++ )
        {
            event = career->getEvent( i );
            if( ( event->getFlags() & ::efActive ) &&
                !( event->getFlags() & efFinished ) &&
                !( event->getFlags() & ::efSystem ) )
            {            
                visibleEvents.push_back( event );
            }
        }
    }
    else
    {
        for( i=0; i<career->getNumEvents(); i++ )
        {
            event = career->getEvent( i );
            if( !( event->getFlags() & ::efActive ) &&               
                !( event->getFlags() & ::efSystem ) )
            {            
                visibleEvents.push_back( event );
            }
        }
    }

    // prepare events & icons placement
    int x = _careerCourse->getPanel()->getRect().left + 8;
    int y = _careerCourse->getPanel()->getRect().bottom + 8;
    gui::Rect desktopRect = Gameplay::iGui->getDesktop()->getRect();
    gui::Rect rect;

    // is active events are present
    if( visibleEvents.size() > 0 )
    {
        // update progress bars        
        for( i=0; i<visibleEvents.size(); i++ )
        {
            updateEventProgress( visibleEvents[i] );            
        }

        // sort active events by duration
        std::sort( visibleEvents.begin(), visibleEvents.end(), eventSort );

        // pass active events, check each event and insert it in to desktop
        for( i=0; i<visibleEvents.size(); i++ )
        {
            if( visibleEvents[i]->getWindow()->getPanel()->getParent() == NULL )
            {
                Gameplay::iGui->getDesktop()->insertPanel(
                    visibleEvents[i]->getWindow()->getPanel()
                );
            }
        }

        // place events upon desktop        
        for( i=0; i<visibleEvents.size(); i++ )
        {
            // build rect
            rect.left   = x;
            rect.right  = x + visibleEvents[i]->getWindow()->getPanel()->getRect().getWidth();
            rect.top    = y;
            rect.bottom = y + visibleEvents[i]->getWindow()->getPanel()->getRect().getHeight();
            // check rect is not overlaps the desktop
            if( rect.bottom > desktopRect.bottom )
            {
                x = rect.right + 8;
                y = _careerCourse->getPanel()->getRect().bottom + 8;
                rect.left   = x;
                rect.right  = x + visibleEvents[i]->getWindow()->getPanel()->getRect().getWidth();
                rect.top    = y;
                rect.bottom = y + visibleEvents[i]->getWindow()->getPanel()->getRect().getHeight();
            }
            // place event
            visibleEvents[i]->getWindow()->getPanel()->setRect( rect );
            // prepare next event placement
            y = rect.bottom + 8;
        }
    }

    // place coming events icon
    if( _comingEventsIcon->getPanel()->getVisible() )
    {
        // build rect
        rect.left   = x;
        rect.right  = x + _comingEventsIcon->getPanel()->getRect().getWidth();
        rect.top    = y;
        rect.bottom = y + _comingEventsIcon->getPanel()->getRect().getHeight();
        // check rect is not overlaps the desktop
        if( rect.bottom > desktopRect.bottom )
        {
            x = rect.right + 8;
            y = _careerCourse->getPanel()->getRect().bottom + 8;
            rect.left   = x;
            rect.right  = x + _comingEventsIcon->getPanel()->getRect().getWidth();
            rect.top    = y;
            rect.bottom = y + _comingEventsIcon->getPanel()->getRect().getHeight();
        }
        // place event
        _comingEventsIcon->getPanel()->setRect( rect );
        // prepare next icon placement
        y = rect.bottom + 8;
    }

    // place active events icon
    if( _activeEventsIcon->getPanel()->getVisible() )
    {
        // build rect
        rect.left   = x;
        rect.right  = x + _activeEventsIcon->getPanel()->getRect().getWidth();
        rect.top    = y;
        rect.bottom = y + _activeEventsIcon->getPanel()->getRect().getHeight();
        // check rect is not overlaps the desktop
        if( rect.bottom > desktopRect.bottom )
        {
            x = rect.right + 8;
            y = _careerCourse->getPanel()->getRect().bottom + 8;
            rect.left   = x;
            rect.right  = x + _activeEventsIcon->getPanel()->getRect().getWidth();
            rect.top    = y;
            rect.bottom = y + _activeEventsIcon->getPanel()->getRect().getHeight();
        }
        // place event
        _activeEventsIcon->getPanel()->setRect( rect );
        // prepare next icon placement
        y = rect.bottom + 8;
    }

    updateSkillsScreen();
    updateFundsScreen();
    updateShopScreen();
}

gui::IGuiPanel* CareerCourse::getRankProgressBox(unsigned int id)
{
    gui::IGuiPanel* result = NULL;

    switch( id )
    {
    case 0:
        result = _skillsScreen->getPanel()->find( "Rank01" );
        break;
    case 1:
        result = _skillsScreen->getPanel()->find( "Rank02" );
        break;
    case 2:
        result = _skillsScreen->getPanel()->find( "Rank03" );
        break;
    case 3:
        result = _skillsScreen->getPanel()->find( "Rank04" );
        break;
    case 4:
        result = _skillsScreen->getPanel()->find( "Rank05" );
        break;
    case 5:
        result = _skillsScreen->getPanel()->find( "Rank06" );
        break;
    case 6:
        result = _skillsScreen->getPanel()->find( "Rank07" );
        break;
    case 7:
        result = _skillsScreen->getPanel()->find( "Rank08" );
        break;
    case 8:
        result = _skillsScreen->getPanel()->find( "Rank09" );
        break;
    case 9:
        result = _skillsScreen->getPanel()->find( "Rank10" );
        break;
    }
    
    assert( result );
    return result;
}

gui::IGuiPanel* CareerCourse::getPromoProgressBox(unsigned int id)
{
    gui::IGuiPanel* result = NULL;

    switch( id )
    {
    case 0:
        result = _fundsScreen->getPanel()->find( "Promotion01" );
        break;
    case 1:
        result = _fundsScreen->getPanel()->find( "Promotion02" );
        break;
    case 2:
        result = _fundsScreen->getPanel()->find( "Promotion03" );
        break;
    case 3:
        result = _fundsScreen->getPanel()->find( "Promotion04" );
        break;
    case 4:
        result = _fundsScreen->getPanel()->find( "Promotion05" );
        break;
    case 5:
        result = _fundsScreen->getPanel()->find( "Promotion06" );
        break;
    case 6:
        result = _fundsScreen->getPanel()->find( "Promotion07" );
        break;
    case 7:
        result = _fundsScreen->getPanel()->find( "Promotion08" );
        break;
    case 8:
        result = _fundsScreen->getPanel()->find( "Promotion09" );
        break;
    case 9:
        result = _fundsScreen->getPanel()->find( "Promotion10" );
        break;
    }
    
    assert( result );
    return result;
}

void CareerCourse::updateSkillsScreen(void)
{
    // playing time
    float playingTime  = _geoscape->getCareer()->getVirtues()->statistics.playingTime;
    int hh = int( playingTime / 3600.0f );
    playingTime -= hh * 3600;
    int mm = int( playingTime / 60.0f );
    playingTime -= mm * 60;
    int ss = int( playingTime );
    
    // walk through value
    float walkthrough = _geoscape->getCareer()->getWalkthroughPercent();
    assert( walkthrough >= 0 && walkthrough <= 1 );
    walkthrough *= 100.0f;

    // update control
    gui::IGuiPanel* panel = this->_skillsScreen->getPanel()->find( "PlayingTime" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( wstrformat( Gameplay::iLanguage->getUnicodeString(242), hh, mm, ss, walkthrough ).c_str() );

    // update score
    float score = _geoscape->getCareer()->getVirtues()->evolution.score;
    panel = _skillsScreen->getPanel()->find( "Score" ); assert( panel && panel->getStaticText() );
	panel->getStaticText()->setText( wstrformat( L"%d", int(score) ).c_str() );

	// skydives
	panel = _skillsScreen->getPanel()->find( "Skydives" );
	panel->getStaticText()->setText( wstrformat(L"%d", _geoscape->getCareer()->getVirtues()->statistics.numSkydives).c_str() );
	// basejumps
	panel = _skillsScreen->getPanel()->find( "Basejumps" );
	panel->getStaticText()->setText( wstrformat(L"%d", _geoscape->getCareer()->getVirtues()->statistics.numBaseJumps).c_str() );
	
	// times
	const unsigned int JUST_TIME_LIMIT = 3600 * 1000 - 1;		// if over the limit (999:59:59) show date too

	// freefall time
	unsigned int y = 0, M = 0, d = 0, h = 0, m = 0, s = 0;
	float time = _geoscape->getCareer()->getVirtues()->statistics.freeFallTime;
	if (time < JUST_TIME_LIMIT) {
		floatToUIntTime(time, NULL, NULL, NULL, &h, &m, &s);
		panel = _skillsScreen->getPanel()->find( "FreefallTime" );
		panel->getStaticText()->setText( wstrformat(L"%03d:%02d:%02d", h, m, s).c_str() );
	} else {
		floatToUIntTime(time, &y, &M, &d, &h, &m, &s);
		panel = _skillsScreen->getPanel()->find( "FreefallTime" );
		panel->getStaticText()->setText( wstrformat(L"%02d-%02d-%02d %03d:%02d:%02d", y, M, d, h, m, s).c_str() );
	}

	// wingsuit time
	time = _geoscape->getCareer()->getVirtues()->statistics.wingsuitTime;
	if (time < JUST_TIME_LIMIT) {
		floatToUIntTime(time, NULL, NULL, NULL, &h, &m, &s);
		panel = _skillsScreen->getPanel()->find( "WingsuitTime" );
		panel->getStaticText()->setText( wstrformat(L"%03d:%02d:%02d", h, m, s).c_str() );
	} else {
		floatToUIntTime(time, &y, &M, &d, &h, &m, &s);
		panel = _skillsScreen->getPanel()->find( "WingsuitTime" );
		panel->getStaticText()->setText( wstrformat(L"%02d-%02d-%02d %03d:%02d:%02d", y, M, d, h, m, s).c_str() );
	}

	// canopy time
	time = _geoscape->getCareer()->getVirtues()->statistics.canopyTime;
	if (time < JUST_TIME_LIMIT) {
		floatToUIntTime(time, NULL, NULL, NULL, &h, &m, &s);
		panel = _skillsScreen->getPanel()->find( "CanopyTime" );
		panel->getStaticText()->setText( wstrformat(L"%03d:%02d:%02d", h, m, s).c_str() );
	} else {
		floatToUIntTime(time, &y, &M, &d, &h, &m, &s);
		panel = _skillsScreen->getPanel()->find( "CanopyTime" );
		panel->getStaticText()->setText( wstrformat(L"%02d-%02d-%02d %03d:%02d:%02d", y, M, d, h, m, s).c_str() );
	}

    // update rank
    int u;
    gui::IGuiPanel* rankBox;
    float rank = float( _geoscape->getCareer()->getVirtues()->getRank() );
    for( unsigned int i=0; i<10; i++ )
    {
        rankBox = getRankProgressBox( i );

        // modify mapping of trap texture         
        if( rank >= 1 )
        {
            rankBox->setTextureRect( gui::Rect( 0,0,7,7 ) );
            rank -= 1.0f;
        }
        else if( rank == 0 )
        {
            rankBox->setTextureRect( gui::Rect( 8,0,15,7 ) );
        }
        else
        {
            u = int( 8.0f - 8.0f * ( rank - int( rank ) ) );
            rankBox->setTextureRect( gui::Rect( u,0,u+7,7 ) );
            rank -= rank;
        }
    }

    Virtues* virtues = _geoscape->getCareer()->getVirtues();

    // update perception skill
    gui::IGuiPanel* vessel = _skillsScreen->getPanel()->find( "PerceptionVessel" ); assert( vessel );
    gui::IGuiPanel* level  = vessel->find( "PerceptionLevel" ); assert( level );
    gui::Rect vesselRect = vessel->getRect();
    gui::Rect levelRect;
    levelRect.left   = 0;
    levelRect.right  = int( vesselRect.getWidth() * virtues->getPerceptionSkill() );
    levelRect.top    = 0;
    levelRect.bottom = 15;    
    level->setRect( levelRect );    
    panel = _skillsScreen->getPanel()->find( "Perception" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( wstrformat( L"%2.1f", virtues->skills.perception ).c_str() );

    // update endurance skill
    vessel = _skillsScreen->getPanel()->find( "EnduranceVessel" ); assert( vessel );
    level  = vessel->find( "EnduranceLevel" ); assert( level );
    vesselRect = vessel->getRect();
    levelRect.left   = 0;
    levelRect.right  = int( vesselRect.getWidth() * virtues->getEnduranceSkill() );
    levelRect.top    = 0;
    levelRect.bottom = 15;    
    level->setRect( levelRect );    
    panel = _skillsScreen->getPanel()->find( "Endurance" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( wstrformat( L"%2.1f", virtues->skills.endurance ).c_str() );

    // update tracking skill
    vessel = _skillsScreen->getPanel()->find( "TrackingVessel" ); assert( vessel );
    level  = vessel->find( "TrackingLevel" ); assert( level );
    vesselRect = vessel->getRect();
    levelRect.left   = 0;
    levelRect.right  = int( vesselRect.getWidth() * virtues->getTrackingSkill() );
    levelRect.top    = 0;
    levelRect.bottom = 15;    
    level->setRect( levelRect );    
    panel = _skillsScreen->getPanel()->find( "Tracking" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( wstrformat( L"%2.1f", virtues->skills.tracking ).c_str() );

    // update rigging skill
    vessel = _skillsScreen->getPanel()->find( "RiggingVessel" ); assert( vessel );
    level  = vessel->find( "RiggingLevel" ); assert( level );
    vesselRect = vessel->getRect();
    levelRect.left   = 0;
    levelRect.right  = int( vesselRect.getWidth() * virtues->getRiggingSkill() );
    levelRect.top    = 0;
    levelRect.bottom = 15;    
    level->setRect( levelRect );    
    panel = _skillsScreen->getPanel()->find( "Rigging" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( wstrformat( L"%2.1f", virtues->skills.rigging ).c_str() );
}

void CareerCourse::updateFundsScreen(void)
{
    // update cash
    float cash = _geoscape->getCareer()->getVirtues()->evolution.funds;
    gui::IGuiPanel* panel = _fundsScreen->getPanel()->find( "Cash" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( wstrformat( L"%2.2f$", cash ).c_str() );

    // update treatment
    float treatmentFee = getTreatmentFee() * WEEKS_TO_MINUTES(1);
    panel = _fundsScreen->getPanel()->find( "Treatment" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( wstrformat( Gameplay::iLanguage->getUnicodeString(285), treatmentFee ).c_str() );

    // update hotel
    panel = _fundsScreen->getPanel()->find( "Hotel" ); assert( panel && panel->getStaticText() );
    if( _geoscape->getPlayerLocation() )
    {
        float stayFee = database::LocationInfo::getRecord( _geoscape->getPlayerLocation()->getDatabaseId() )->stayFee;
        panel->getStaticText()->setText( wstrformat( Gameplay::iLanguage->getUnicodeString(287), stayFee ).c_str() );
    }
    else
    {
        panel->getStaticText()->setText( L"" );
    }

    // update salary
    float salary = _geoscape->getCareer()->getVirtues()->getSalary();
    panel = _fundsScreen->getPanel()->find( "Salary" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( wstrformat( L"%2.2f$", salary ).c_str() );

    // update promotion
    int u;
    gui::IGuiPanel* promoBox;
    float promotion = _geoscape->getCareer()->getVirtues()->getPromotion();
    for( unsigned int i=0; i<10; i++ )
    {
        promoBox = getPromoProgressBox( i );

        // modify mapping of trap texture         
        if( promotion >= 1 )
        {
            promoBox->setTextureRect( gui::Rect( 0,0,7,7 ) );
            promotion -= 1.0f;
        }
        else if( promotion == 0 )
        {
            promoBox->setTextureRect( gui::Rect( 8,0,15,7 ) );
        }
        else
        {
            u = int( 8.0f - 8.0f * ( promotion - int( promotion ) ) );
            promoBox->setTextureRect( gui::Rect( u,0,u+7,7 ) );
            promotion -= promotion;
        }
    }
}

bool CareerCourse::isBestOffer(Gear* candidate, Gear* nominant)
{
    // compare them by rules
    if( candidate->state < nominant->state ) return false;
    if( wcsncmp( candidate->getName(), nominant->getName(), 2 ) > 0 ) return false;
    if( candidate->getCost() < nominant->getCost() ) return false;
    return true;
}

void CareerCourse::updateShopScreen(void)
{
    Gear gear;
    Gear candidate, nominant;
    unsigned int i,j,bestId;

    gui::IGuiPanel* item;
    gui::IGuiPanel* cost;
    gui::IGuiPanel* name;
    gui::IGuiPanel* status;
    gui::IGuiPanel* age;
    gui::IGuiPanel* color;

    _marketOffer.clear();
    _playerOffer.clear();

    // update market offer
    _unsorted.clear();
    for( i=0; i<_geoscape->getMarketSize(); i++ )
    {
        gear = _geoscape->getGearFromMarket( i );
        if( gear.type == _gearFilter ) _unsorted.push_back( i );
    }

    // sort market offer
    while( _unsorted.size() )
    {
        bestId = 0;
        for( i=1; i<_unsorted.size(); i++ )
        {
            candidate = _geoscape->getGearFromMarket( _unsorted[i] );
            nominant  = _geoscape->getGearFromMarket( _unsorted[bestId] );
            if( isBestOffer( &candidate, &nominant ) ) bestId = i;
        }       
        _marketOffer.push_back( _unsorted[bestId] );
        _unsorted.erase( &_unsorted[bestId] );
    }

    // update player offer
    _unsorted.clear();
	Career *career = _geoscape->getCareer();
	// add equipped (unequip them also)
	career->unequipGear();

    for( i=0; i<career->getNumGears(); i++ )
    {
        gear = _geoscape->getCareer()->getGear( i );
        if( gear.type == _gearFilter ) _unsorted.push_back( i );
    }	

    // sort player offer
    while( _unsorted.size() )
    {
        bestId = 0;
        for( i=1; i<_unsorted.size(); i++ )
        {
            candidate = _geoscape->getCareer()->getGear( _unsorted[i] );
            nominant  = _geoscape->getCareer()->getGear( _unsorted[bestId] );
            if( isBestOffer( &candidate, &nominant ) ) bestId = i;
        }
        _playerOffer.push_back( _unsorted[bestId] );
        _unsorted.erase( &_unsorted[bestId] );
    }

    // update shop slider
    float lowerLimit = 0;
    float upperLimit = float( _marketOffer.size() );
    upperLimit -= NUM_SHOP_ITEMS;
    if( upperLimit < lowerLimit ) upperLimit = lowerLimit;
    if( _shopSlider->getSlider()->getPosition() > upperLimit ) _shopSlider->getSlider()->setPosition( upperLimit );
    _shopSlider->getSlider()->setLowerLimit( lowerLimit );
    _shopSlider->getSlider()->setUpperLimit( upperLimit );

    // update items
    for( i=0; i<NUM_SHOP_ITEMS; i++ )
    {
        // item window
        item = NULL;
        switch( i )
        {
        case 0: item = _shopScreen->getPanel()->find( "ShopItem01" ); assert( item ); break;
        case 1: item = _shopScreen->getPanel()->find( "ShopItem02" ); assert( item ); break;
        case 2: item = _shopScreen->getPanel()->find( "ShopItem03" ); assert( item ); break;
        case 3: item = _shopScreen->getPanel()->find( "ShopItem04" ); assert( item ); break;
        case 4: item = _shopScreen->getPanel()->find( "ShopItem05" ); assert( item ); break;
		case 5: item = _shopScreen->getPanel()->find( "ShopItem06" ); assert( item ); break;
		case 6: item = _shopScreen->getPanel()->find( "ShopItem07" ); assert( item ); break;
		case 7: item = _shopScreen->getPanel()->find( "ShopItem08" ); assert( item ); break;
		case 8: item = _shopScreen->getPanel()->find( "ShopItem09" ); assert( item ); break;
		case 9: item = _shopScreen->getPanel()->find( "ShopItem10" ); assert( item ); break;
        }
        assert( item );
        // gear index
        j = i + unsigned int( _shopSlider->getSlider()->getPosition() );
        // item is enabled
        if( j < _marketOffer.size() )
        {
            gear = _geoscape->getGearFromMarket( _marketOffer[j] );
            // show item
            item->setVisible( true );
            // item controls
            cost   = item->find( "Cost" ); assert( cost && cost->getStaticText() );
            name   = item->find( "Name" ); assert( name && name->getButton() );
            status = item->find( "Status" ); assert( status && status->getStaticText() );
            age    = item->find( "Age" ); assert( age && age->getStaticText() );
            color  = item->find( "Color" ); assert( color );
            // update item info
            cost->getStaticText()->setText( wstrformat( Gameplay::iLanguage->getUnicodeString(104), gear.getCost() ).c_str() );
            name->getButton()->setCaption( gear.getName() );
            name->setHint( gear.getDescription() );
            color->setColor( gear.getGearColor() );
            color->setTextureRect( gear.getGearPreview() );
            status->getStaticText()->setText( gear.getStateText() );
            status->getStaticText()->setTextColor( gear.getStateColor() );
            status->setHint( gear.getStateDescription() );
            age->getStaticText()->setText( wstrformat( L"%d", gear.age ).c_str() );
        }
        else
        {
            // hide item
            item->setVisible( false );
        }
    }

    // update player slider
    lowerLimit = 0;
    upperLimit = float( _playerOffer.size() );
    upperLimit -= NUM_SHOP_ITEMS;
    if( upperLimit < lowerLimit ) upperLimit = lowerLimit;
    if( _playerSlider->getSlider()->getPosition() > upperLimit ) _playerSlider->getSlider()->setPosition( upperLimit );
    _playerSlider->getSlider()->setLowerLimit( lowerLimit );
    _playerSlider->getSlider()->setUpperLimit( upperLimit );

    // update items
    for( i=0; i<NUM_SHOP_ITEMS; i++ )
    {
        // item window
        item = NULL;
        switch( i )
        {
        case 0: item = _shopScreen->getPanel()->find( "PlayerItem01" ); assert( item ); break;
        case 1: item = _shopScreen->getPanel()->find( "PlayerItem02" ); assert( item ); break;
        case 2: item = _shopScreen->getPanel()->find( "PlayerItem03" ); assert( item ); break;
        case 3: item = _shopScreen->getPanel()->find( "PlayerItem04" ); assert( item ); break;
        case 4: item = _shopScreen->getPanel()->find( "PlayerItem05" ); assert( item ); break;
		case 5: item = _shopScreen->getPanel()->find( "PlayerItem06" ); assert( item ); break;
		case 6: item = _shopScreen->getPanel()->find( "PlayerItem07" ); assert( item ); break;
		case 7: item = _shopScreen->getPanel()->find( "PlayerItem08" ); assert( item ); break;
		case 8: item = _shopScreen->getPanel()->find( "PlayerItem09" ); assert( item ); break;
		case 9: item = _shopScreen->getPanel()->find( "PlayerItem10" ); assert( item ); break;
        }
        // gear index
        j = i + unsigned int( _playerSlider->getSlider()->getPosition() );
        // item is enabled
        if( j < _playerOffer.size() )
        {
            gear = _geoscape->getCareer()->getGear( _playerOffer[j] );
            // show item
            item->setVisible( true );
            // item controls
            cost   = item->find( "Cost" ); assert( cost && cost->getStaticText() );
            name   = item->find( "Name" ); assert( name && name->getButton() );
            status = item->find( "Status" ); assert( status && status->getStaticText() );
            age    = item->find( "Age" ); assert( age && age->getStaticText() );
            color  = item->find( "Color" ); assert( color );
            // update item info
            cost->getStaticText()->setText( wstrformat( Gameplay::iLanguage->getUnicodeString(104), gear.getCost() ).c_str() );
            name->getButton()->setCaption( gear.getName() );
            name->setHint( gear.getDescription() );
            name->getButton()->getPanel()->setHint( gear.getDescription() );
            color->setColor( gear.getGearColor() );
            color->setTextureRect( gear.getGearPreview() );
            status->getStaticText()->setText( gear.getStateText() );
            status->getStaticText()->setTextColor( gear.getStateColor() );
            status->setHint( gear.getStateDescription() );
            age->getStaticText()->setText( wstrformat( L"%d", gear.age ).c_str() );
        }
        else
        {
            // hide item
            item->setVisible( false );
        }
    }
}

void CareerCourse::updateEventProgress(Event* event)
{
    DateTime timeLeft;
    if( _displayMode == ::edmActive )
    {
        timeLeft = DateTime( event->getTimeTo() );
    }
    else
    {
        timeLeft = DateTime( event->getTimeTo() - event->getDuration() );
    }

    gui::IGuiPanel* progressBox;
    int u;

    for( int i=0; i<7; i++ )
    {
        switch( i )
        {
        case 0:
            progressBox = event->getWindow()->getPanel()->find( "Duration01" );
            assert( progressBox );
            break;
        case 1:
            progressBox = event->getWindow()->getPanel()->find( "Duration02" );
            assert( progressBox );
            break;
        case 2:
            progressBox = event->getWindow()->getPanel()->find( "Duration03" );
            assert( progressBox );
            break;
        case 3:
            progressBox = event->getWindow()->getPanel()->find( "Duration04" );
            assert( progressBox );
            break;
        case 4:
            progressBox = event->getWindow()->getPanel()->find( "Duration05" );
            assert( progressBox );
            break;
        case 5:
            progressBox = event->getWindow()->getPanel()->find( "Duration06" );
            assert( progressBox );
            break;
        case 6:
            progressBox = event->getWindow()->getPanel()->find( "Duration07" );
            assert( progressBox );
            break;
        default:
            assert( !"shouldn't be here!" );
        }

        // modify mapping of trap texture         
        if( i+1 < timeLeft.day )
        {
            progressBox->setTextureRect( gui::Rect( 0,0,7,7 ) );
        }
        else if( i+1 > timeLeft.day )
        {
            progressBox->setTextureRect( gui::Rect( 8,0,15,7 ) );
        }
        else
        {
            u = int( 8.0f - timeLeft.hour * 8 / 24.0f );
            progressBox->setTextureRect( gui::Rect( u,0,u+7,7 ) );
        }
    }
}

/**
 * shopping
 */

void CareerCourse::sellItem(unsigned int itemId)
{
    // sell item            
    if( itemId < _playerOffer.size() )
    {
        Gear gear = _geoscape->getCareer()->getGear( _playerOffer[itemId] );
        _geoscape->getCareer()->removeGear( _playerOffer[itemId] );
        _geoscape->addGearToMarket( gear );
        _geoscape->getCareer()->getVirtues()->evolution.funds += gear.getCost();
    }
}

void CareerCourse::buyItem(unsigned int itemId)
{
    // buy item            
    if( itemId < _marketOffer.size() )
    {
        Gear gear = _geoscape->getGearFromMarket( _marketOffer[itemId] );
        if( _geoscape->getCareer()->getVirtues()->evolution.funds >= gear.getCost() )
        {
            _geoscape->getCareer()->addGear( gear );
            _geoscape->removeGearFromMarket( _marketOffer[itemId] );
            _geoscape->getCareer()->getVirtues()->evolution.funds -= gear.getCost();
        }
    }
}

/**
 * GeoscapeMode
 */
   
bool CareerCourse::endOfMode(void)
{
    return false;
}

void CareerCourse::onGuiMessage(gui::Message* message)
{
    if( message->event == gui::onButtonClick )
    {
        if( strcmp( message->origin->getName(), "Face" ) == 0 )
        {
            // aim on to player location
            if( _geoscape->getPlayerLocation() )
            {
                database::LocationInfo* locationInfo = database::LocationInfo::getRecord( _geoscape->getPlayerLocation()->getDatabaseId() );
                if( _geoscape->getPlayerLocation()->getDatabaseId() == 0 )
                {
                    _geoscape->centerGeoscape( 
                        _geoscape->getCareer()->getHomeX(),
                        _geoscape->getCareer()->getHomeY()
                    );
                }
                else
                {
                    _geoscape->centerGeoscape( locationInfo->worldX, locationInfo->worldY );
                }
            }
			// Cheats to enable full health, increase score and add $100 when clicking on character icon
			if ( Gameplay::iGameplay->_cheatsEnabled )
			{
				_geoscape->getCareer()->getVirtues()->evolution.funds += 100.0f;
                _geoscape->getCareer()->getVirtues()->evolution.score += 100.0f;
                _geoscape->getCareer()->getVirtues()->evolution.health = 1.0f;
				_geoscape->getCareer()->getVirtues()->evolution.credits = 3;
                _geoscape->addHistory( Gameplay::iLanguage->getUnicodeString(631), Vector4f( 1, 0.25f, 0.25f, 1 ) );
			}
            #ifdef GAMEPLAY_DEVELOPER_EDITION
                _geoscape->getCareer()->getVirtues()->evolution.funds += 100.0f;
                _geoscape->getCareer()->getVirtues()->evolution.score += 100.0f;
                _geoscape->getCareer()->getVirtues()->evolution.health = 1.0f;
                _geoscape->addHistory( Gameplay::iLanguage->getUnicodeString(631), Vector4f( 1, 0.25f, 0.25f, 1 ) );
            #endif
        }
        else if( strcmp( message->origin->getName(), "Skills" ) == 0 )
        {
            _fundsScreen->getPanel()->setVisible( false );
            _skillsScreen->getPanel()->setVisible( !_skillsScreen->getPanel()->getVisible() );
            _shopScreen->getPanel()->setVisible( false );
        }
        else if( strcmp( message->origin->getName(), "Funds" ) == 0 )
        {
            _fundsScreen->getPanel()->setVisible( !_fundsScreen->getPanel()->getVisible() );
            _skillsScreen->getPanel()->setVisible( false );
            _shopScreen->getPanel()->setVisible( false );
        }
        else if( strcmp( message->origin->getName(), "Shop" ) == 0 )
        {
            // trade feature is avaiable by default
            bool shopScreenIsAvaiable = true;
            
            // trade feature is not avaiable for licensed character
            shopScreenIsAvaiable = !_geoscape->getCareer()->getLicensedFlag();                

            if( shopScreenIsAvaiable )
            {
                _fundsScreen->getPanel()->setVisible( false );
                _skillsScreen->getPanel()->setVisible( false );
                _shopScreen->getPanel()->setVisible( !_shopScreen->getPanel()->getVisible() );
            }
            else
            {
                _geoscape->addHistory( Gameplay::iLanguage->getUnicodeString(758), Vector4f( 1,0,0,1 ) );
            }
        }
        else if( strcmp( message->origin->getName(), "Close" ) == 0 )
        {
            _geoscape->closeGeoscapeMode();
        }
        else if( strcmp( message->origin->getName(), "Pause" ) == 0 )
        {
            setTimeSpeed( ::tsPause );
        }        
        else if( strcmp( message->origin->getName(), "FastTime" ) == 0 )
        {
            setTimeSpeed( ::tsFast );
            setCareerDialog( NULL );
        }
        else if( strcmp( message->origin->getName(), "FastestTime" ) == 0 )
        {
            setTimeSpeed( ::tsFastest );
            setCareerDialog( NULL );
        }
        else if( message->origin->getParent() &&
                 strcmp( message->origin->getParent()->getName(), "ComingEventsIcon" ) == 0 )
        {
            _displayMode = ::edmComing;
        }
        else if( message->origin->getParent() &&
                 strcmp( message->origin->getParent()->getName(), "ActiveEventsIcon" ) == 0 )
        {
            _displayMode = ::edmActive;
        }
        else if( strcmp( message->origin->getName(), "ShopSliderUp" ) == 0 )
        {
            float pos = _shopSlider->getSlider()->getPosition() - 1;
            if( pos < _shopSlider->getSlider()->getLowerLimit() ) pos = _shopSlider->getSlider()->getLowerLimit();
            _shopSlider->getSlider()->setPosition( pos );
        }
        else if( strcmp( message->origin->getName(), "ShopSliderDown" ) == 0 )
        {
            float pos = _shopSlider->getSlider()->getPosition() + 1;
            if( pos > _shopSlider->getSlider()->getUpperLimit() ) pos = _shopSlider->getSlider()->getUpperLimit();
            _shopSlider->getSlider()->setPosition( pos );
        }
        else if( strcmp( message->origin->getName(), "PlayerSliderUp" ) == 0 )
        {
            float pos = _playerSlider->getSlider()->getPosition() - 1;
            if( pos < _playerSlider->getSlider()->getLowerLimit() ) pos = _playerSlider->getSlider()->getLowerLimit();
            _playerSlider->getSlider()->setPosition( pos );
        }
        else if( strcmp( message->origin->getName(), "PlayerSliderDown" ) == 0 )
        {
            float pos = _playerSlider->getSlider()->getPosition() + 1;
            if( pos > _playerSlider->getSlider()->getUpperLimit() ) pos = _playerSlider->getSlider()->getUpperLimit();
            _playerSlider->getSlider()->setPosition( pos );
        }
        else if( strcmp( message->origin->getName(), "HelmetFilter" ) == 0 )
        {
            _gearFilter = ::gtHelmet;
        }
        else if( strcmp( message->origin->getName(), "SuitFilter" ) == 0 )
        {
            _gearFilter = ::gtSuit;
        }
        else if( strcmp( message->origin->getName(), "RigFilter" ) == 0 )
        {
            _gearFilter = ::gtRig;
        }
        else if( strcmp( message->origin->getName(), "CanopyFilter" ) == 0 )
        {
            _gearFilter = ::gtCanopy;
        }
        else if( strcmp( message->origin->getName(), "ReserveFilter" ) == 0 )
        {
            _gearFilter = ::gtReserve;
        }
        else if( message->origin->getParent() && 
                 strcmp( message->origin->getName(), "Name" ) == 0 )
        {
            // player item?            
            if( strcmp( message->origin->getParent()->getName(), "PlayerItem01" ) == 0 )
            {
                sellItem( unsigned int( _playerSlider->getSlider()->getPosition() ) );
            }
            else if( strcmp( message->origin->getParent()->getName(), "PlayerItem02" ) == 0 )
            {
                sellItem( 1 + unsigned int( _playerSlider->getSlider()->getPosition() ) );
            }
            else if( strcmp( message->origin->getParent()->getName(), "PlayerItem03" ) == 0 )
            {
                sellItem( 2 + unsigned int( _playerSlider->getSlider()->getPosition() ) );
            }
            else if( strcmp( message->origin->getParent()->getName(), "PlayerItem04" ) == 0 )
            {
                sellItem( 3 + unsigned int( _playerSlider->getSlider()->getPosition() ) );
            }
            else if( strcmp( message->origin->getParent()->getName(), "PlayerItem05" ) == 0 )
            {
                sellItem( 4 + unsigned int( _playerSlider->getSlider()->getPosition() ) );
            }
            else if( strcmp( message->origin->getParent()->getName(), "PlayerItem06" ) == 0 )
            {
                sellItem( 5 + unsigned int( _playerSlider->getSlider()->getPosition() ) );
            }
            else if( strcmp( message->origin->getParent()->getName(), "PlayerItem07" ) == 0 )
            {
                sellItem( 6 + unsigned int( _playerSlider->getSlider()->getPosition() ) );
            }
            else if( strcmp( message->origin->getParent()->getName(), "PlayerItem08" ) == 0 )
            {
                sellItem( 7 + unsigned int( _playerSlider->getSlider()->getPosition() ) );
            }
            else if( strcmp( message->origin->getParent()->getName(), "PlayerItem09" ) == 0 )
            {
                sellItem( 8 + unsigned int( _playerSlider->getSlider()->getPosition() ) );
            }
            else if( strcmp( message->origin->getParent()->getName(), "PlayerItem10" ) == 0 )
            {
                sellItem( 9 + unsigned int( _playerSlider->getSlider()->getPosition() ) );
            }
            // market item?            
            if( strcmp( message->origin->getParent()->getName(), "ShopItem01" ) == 0 )
            {
                buyItem( unsigned int( _shopSlider->getSlider()->getPosition() ) );
            }
            else if( strcmp( message->origin->getParent()->getName(), "ShopItem02" ) == 0 )
            {
                buyItem( 1 + unsigned int( _shopSlider->getSlider()->getPosition() ) );
            }
            else if( strcmp( message->origin->getParent()->getName(), "ShopItem03" ) == 0 )
            {
                buyItem( 2 + unsigned int( _shopSlider->getSlider()->getPosition() ) );
            }
            else if( strcmp( message->origin->getParent()->getName(), "ShopItem04" ) == 0 )
            {
                buyItem( 3 + unsigned int( _shopSlider->getSlider()->getPosition() ) );
            }
            else if( strcmp( message->origin->getParent()->getName(), "ShopItem05" ) == 0 )
            {
                buyItem( 4 + unsigned int( _shopSlider->getSlider()->getPosition() ) );
            }
            else if( strcmp( message->origin->getParent()->getName(), "ShopItem06" ) == 0 )
            {
                buyItem( 5 + unsigned int( _shopSlider->getSlider()->getPosition() ) );
            }
            else if( strcmp( message->origin->getParent()->getName(), "ShopItem07" ) == 0 )
            {
                buyItem( 6 + unsigned int( _shopSlider->getSlider()->getPosition() ) );
            }
            else if( strcmp( message->origin->getParent()->getName(), "ShopItem08" ) == 0 )
            {
                buyItem( 7 + unsigned int( _shopSlider->getSlider()->getPosition() ) );
            }
            else if( strcmp( message->origin->getParent()->getName(), "ShopItem09" ) == 0 )
            {
                buyItem( 8 + unsigned int( _shopSlider->getSlider()->getPosition() ) );
            }
            else if( strcmp( message->origin->getParent()->getName(), "ShopItem10" ) == 0 )
            {
                buyItem( 9 + unsigned int( _shopSlider->getSlider()->getPosition() ) );
            }
        }
        else if( strcmp( message->origin->getName(), "Action" ) == 0 )
        {
            Location* location = _geoscape->getLocation( message->origin );
            assert( location );
			if ( Gameplay::iGameplay->_freeModeIsEnabled )
			{
				// enter to location
                setTimeSpeed( ::tsPause );
                setCareerDialog( new EnterLocationDialog( _geoscape, location ) );
			}
            // click on current location?
            else if( !Gameplay::iGameplay->_freeModeIsEnabled && location->getPlayer() )
            {
                // check health
                if( _geoscape->getCareer()->getVirtues()->evolution.health < 0.75f )
                {
                    setTimeSpeed( ::tsPause );
                    _geoscape->addHistory( Gameplay::iLanguage->getUnicodeString(296), Vector4f( 1.0f, 0.25f, 0.25f, 1.0f ) );
                }
                else
                {
                    // obtain current time
                    DateTime currentTime( _geoscape->getCareer()->getVirtues()->evolution.time );
                    if( currentTime.hour < 6 || currentTime.hour >= 18 )
                    {
                        _geoscape->addHistory( Gameplay::iLanguage->getUnicodeString(121), Vector4f( 1.0f, 0.25f, 0.25f, 1.0f ) );
                    }
                    else
                    {
                        // enter to location
                        setTimeSpeed( ::tsPause );
                        setCareerDialog( new EnterLocationDialog( _geoscape, location ) );
                    }
                }
            }

            else
            {
                // travel to location
                travelToLocation( location );
                setTimeSpeed( ::tsPause );
            }
        }
    }    

    // proceed dialog
    if( _careerDialog ) 
    {
        _careerDialog->onGuiMessage( message );
        if( _careerDialog->isComplete() )
        {
            delete _careerDialog;
            _careerDialog = NULL;
        }
    }
}

// treatment effectivity & treatment fee
const float treatmentMaxHealth   = 1.0f;
const float treatmentMaxRecovery = 1.0f / WEEKS_TO_MINUTES(1);
const float treatmentMaxFee      = 0.0f;
const float treatmentMinHealth   = 0.25f;
const float treatmentMinRecovery = 1.0f / MONTHS_TO_MINUTES(3);
const float treatmentMinFee      = 250.0f / WEEKS_TO_MINUTES(1);

float CareerCourse::getTreatmentEffectivity(void)
{
    Career* career = _geoscape->getCareer();
    float factor = ( career->getVirtues()->evolution.health - treatmentMinHealth ) / ( treatmentMaxHealth - treatmentMinHealth );
    factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );
    return treatmentMinRecovery * ( 1.0f - factor ) + treatmentMaxRecovery * factor;
}

float CareerCourse::getTreatmentFee(void)
{
    Career* career = _geoscape->getCareer();
    float factor = ( career->getVirtues()->evolution.health - treatmentMinHealth) / ( treatmentMaxHealth - treatmentMinHealth );
    factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );
    return ( treatmentMinFee * ( 1.0f - factor ) + treatmentMaxFee * factor );
}

void CareerCourse::onUpdateMode(float dt)
{
    updateBlinking( dt );

    // career
    Career* career = _geoscape->getCareer();

    // time speed value
    float tsValue = -1;
    switch( _timeSpeed )
    {
    case ::tsPause: tsValue = 0.0f; break;
    case ::tsFast: tsValue = 60.0f * 60.0f; break;
    case ::tsFastest: tsValue = 60.0f * 60.0f * 24.0f; break;
    default:
        assert( !"shouldn't be here!" );
    }

    // time in minutes
    float minutes = dt / 60.0f * tsValue;

    if( minutes == 0 )
    {
        updateGui();
        return;
    }

    // simulate course
    simulateCourse( minutes, true );

    updateGui();

    // treatment 
    if( career->getVirtues()->evolution.health < 1.0f )
    {
        // (only in home location)
        if( _geoscape->getPlayerLocation() &&
            _geoscape->getPlayerLocation()->getDatabaseId() == 0 )
        {             
            // calculate current treatment effectivity & treatment fee
            float recovery = getTreatmentEffectivity();
            float fee      = getTreatmentFee();

            // restore health if player have enough of enough of funds
            if( career->getVirtues()->evolution.funds > fee * minutes )
            {
                career->getVirtues()->evolution.funds -= fee * minutes;
                career->getVirtues()->evolution.health += recovery * minutes;
                if( career->getVirtues()->evolution.health > 1 )
                {
                    career->getVirtues()->evolution.health = 1;
                }
            }
        }
        // notify no healing
        else
        {
            if( _geoscape->getHistorySize() == 0 )
            {
                _geoscape->addHistory( Gameplay::iLanguage->getUnicodeString(347), Vector4f( 1.0f,0.25f,0.25f,1.0f ) );
            }
        }
    }
}

void CareerCourse::updateBlinkingButton(gui::IGuiPanel* buttonPanel, float alpha)
{
    Vector4f color;
    color = buttonPanel->getColor();
    color[3] = alpha;
    buttonPanel->setColor( color );
}

void CareerCourse::updateBlinking(float dt)
{
    // update blinking
    _timeBlink += dt;
    float alpha = 0.5f * ( 1 + sin( _timeBlink * 10 ) );
    
    gui::IGuiPanel* buttonPanel = _careerCourse->getPanel()->find( "Pause" );    
    assert( buttonPanel && buttonPanel->getButton() );
    if( _timeSpeed == ::tsPause ) updateBlinkingButton( buttonPanel, alpha );

    buttonPanel = _careerCourse->getPanel()->find( "FastTime" );    
    assert( buttonPanel && buttonPanel->getButton() );
    if( _timeSpeed == ::tsFast ) updateBlinkingButton( buttonPanel, alpha );

    buttonPanel = _careerCourse->getPanel()->find( "FastestTime" );    
    assert( buttonPanel && buttonPanel->getButton() );
    if( _timeSpeed == ::tsFastest ) updateBlinkingButton( buttonPanel, alpha );
}

void CareerCourse::resetBlinking(void)
{
    gui::IGuiPanel* buttonPanel = _careerCourse->getPanel()->find( "Pause" );    
    assert( buttonPanel && buttonPanel->getButton() );
    buttonPanel->setColor( buttonPanel->getButton()->getInactiveColor() );

    buttonPanel = _careerCourse->getPanel()->find( "FastTime" );    
    assert( buttonPanel && buttonPanel->getButton() );
    buttonPanel->setColor( buttonPanel->getButton()->getInactiveColor() );

    buttonPanel = _careerCourse->getPanel()->find( "FastestTime" );    
    assert( buttonPanel && buttonPanel->getButton() );
    buttonPanel->setColor( buttonPanel->getButton()->getInactiveColor() );
}

void CareerCourse::setCareerDialog(CareerDialog* dialog)
{
    if( _careerDialog ) delete _careerDialog;
    _careerDialog = dialog;
}

void CareerCourse::setTimeSpeed(TimeSpeed timeSpeed)
{
    _timeSpeed = timeSpeed;
    resetBlinking();
}

void CareerCourse::travelToLocation(Location* location)
{
    // retrieve player location
    Location* playerLocation = _geoscape->getPlayerLocation();
    
    // in there are no player location, then player is travels
    if( !playerLocation )
    {
        // show corresponding message
        _geoscape->addHistory( Gameplay::iLanguage->getUnicodeString(112), Vector4f( 1.0f, 0.25f, 0.25f, 1.0f ) );
    }
    else
    {
        // calculate distance btw. locations
        Vector2f currentCoords = playerLocation->getLocationCoords();
        Vector2f targetCoords = location->getLocationCoords();
        Vector2f homeCoords = _geoscape->getHomeLocation()->getLocationCoords();
        float targetDistance = ( targetCoords - currentCoords ).length();
        float homeDistance = ( targetCoords - homeCoords ).length();
        // scale distance to real values
        targetDistance = targetDistance * 40075.696f / _geoscape->getWindow()->getPanel()->getRect().getWidth();
        homeDistance = homeDistance * 40075.696f / _geoscape->getWindow()->getPanel()->getRect().getWidth();
        // calculate traveling cost
        float referenceCost = 0.05f;
        float targetCost = targetDistance * referenceCost;
        float homeCost = homeDistance * referenceCost;
        // calculate traveling time
        float referenceTime = 0.1f;
        float targetTime = targetDistance * referenceTime;
        float homeTime = homeDistance * referenceTime;
        // show dialog
        setCareerDialog( new TravelDialog( _geoscape, location, targetCost, targetTime, homeCost, homeTime ) );
    }
}

float CareerCourse::simulateCourse(float minutes, bool guiFeedback)
{
    // career
    Career* career = _geoscape->getCareer();

    // SHIFT press
    bool shift = ( Gameplay::iGameplay->getKeyboardState()->keyState[0x2A] & 0x80 ) || 
                 ( Gameplay::iGameplay->getKeyboardState()->keyState[0x36] & 0x80 );

    // pass all of events, determine step time and event to handle
    float  stepTime = minutes;
    Event* eventToHandle = NULL;
    Event* event;
    for( unsigned int i=0; i<career->getNumEvents(); i++ )
    {
        event = career->getEvent( i );
        // if event is not finished
        if( !( event->getFlags() & efFinished ) )
        {
            // if event is inactive, and event activation is coming
            if( !( event->getFlags() & efActive ) && 
                 ( event->getTimeTo() - event->getDuration() <= stepTime ) )
            {
                stepTime = event->getTimeTo() - event->getDuration();
                eventToHandle = event;
            }
            // if event ending is coming
            else if( event->getTimeTo() <= stepTime )
            {
                stepTime = event->getTimeTo();
                eventToHandle = event;                
            }
        }
    }

    // update career    
    career->getVirtues()->evolution.time += stepTime;

    // pass all of events, course them by step time
    for( i=0; i<career->getNumEvents(); i++ )
    {
        event = career->getEvent( i );
        event->getTimeTo() -= stepTime;
        // update event
        event->onUpdateEvent( _geoscape, stepTime );
    }

    // handle event
    if( eventToHandle )
    {
        if( !(eventToHandle->getFlags() & efSystem ) )
        {
            if( guiFeedback && !shift ) setTimeSpeed( ::tsPause );
        }
        if( eventToHandle->getTimeTo() - eventToHandle->getDuration() == 0 )
        {            
            eventToHandle->getFlags() = eventToHandle->getFlags() | efActive;
            eventToHandle->onBeginEvent( _geoscape );
        }
        if( eventToHandle->getTimeTo() == 0 )
        {
            eventToHandle->getFlags() = eventToHandle->getFlags() | efFinished;
            eventToHandle->onEndEvent( _geoscape );
            if( eventToHandle->getWindow()->getPanel()->getParent() )
            {
                eventToHandle->getWindow()->getPanel()->getParent()->removePanel( 
                    eventToHandle->getWindow()->getPanel()
                );
            }
            career->removeEvent( eventToHandle );
        }        
    }

    // search for hot events (that are ready to just happen)
    bool hotEventsFound;
    do
    {
        hotEventsFound = false;
        for( i=0; i<career->getNumEvents(); i++ )
        {
            if( ( career->getEvent(i)->getTimeTo() == 0.0f ||
                  career->getEvent(i)->getTimeTo() - career->getEvent(i)->getDuration() == 0.0f ) &&
                !( career->getEvent(i)->getFlags() & efActive ) )
            {
                hotEventsFound = true;
                break;
            }
        }
        if( hotEventsFound )
        {
            simulateCourse( 0.0f, guiFeedback );
        }
    }
    while( hotEventsFound );

    // correction of time errors due to float type of time
    // search for nigth event
    Event* night = NULL;
    for( i=0; i<career->getNumEvents(); i++ )
    {
        if( strcmp( career->getEvent(i)->getClassName(), NIGHT_CLASS_NAME ) == 0 )
        {
            night = career->getEvent(i);
            break;
        }
    }
    assert( night );
    // night is inactive?
    if( night->getTimeTo() > night->getDuration() )
    {
        // check current time
        DateTime currentDT = _geoscape->getDateTime();
        if( currentDT.hour < 6 )
        {
            float error = float( HOURS_TO_MINUTES(6) - ( HOURS_TO_MINUTES( currentDT.hour ) + currentDT.minute ) );
            assert( error > 0 );
            // getCore()->logMessage( "Time correction error: %2.1f", error );
            simulateCourse( error, false );
        }
    }

    return stepTime;
}

void CareerCourse::eventCallback(Event* event, void* data)
{
    CareerCourse* __this = reinterpret_cast<CareerCourse*>( data );
    __this->simulateCourse( 0.0f, true );
    __this->updateGui();
}