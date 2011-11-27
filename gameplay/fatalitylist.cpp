
#include "headers.h"
#include "fatalitylist.h"
#include "gameplay.h"
#include "../common/istring.h"
#include "unicode.h"

#define FL_NUM_ITEMS 10

/**
 * class implementation
 */

FatalityList::FatalityList()
{
    // common initialization
    initialize();

    // update list
    updateGui();
}

FatalityList::FatalityList(Career* career, unsigned int locationId)
{
    // common initialization
    initialize();

    // add new incident
    _freshIncident = new Incident( career, locationId );
    _incidents.push_back( _freshIncident );

    // update list
    updateGui();
}

FatalityList::~FatalityList()
{
    // save incidents
    TiXmlDocument* index = new TiXmlDocument( "./usr/fatality.xml" );
    for( unsigned int i=0; i<_incidents.size(); i++ )
    {
        _incidents[i]->save( index );
    }
    index->SaveFile();
    delete index;

    // destroy window
    _window->getPanel()->release();
}

/**
 * Activity abstracts
 */
    
void FatalityList::updateActivity(float dt)
{
    // render gui
    Gameplay::iEngine->getDefaultCamera()->beginScene( 
        engine::cmClearColor | engine::cmClearDepth,
    	  Vector4f( 0,0,0,0 )
    );
    Gameplay::iGui->render();
    Gameplay::iEngine->getDefaultCamera()->endScene();
    Gameplay::iEngine->present();
}

bool FatalityList::endOfActivity(void)
{
    return _endOfActivity;
}

void FatalityList::onBecomeActive(void)
{
    // insert window in Gui
    Gameplay::iGui->getDesktop()->insertPanel( _window->getPanel() );
    _window->align( gui::atCenter, 0, gui::atCenter, 0 );

    // register message callback
    Gameplay::iGui->setMessageCallback( messageCallback, this );
}

void FatalityList::onBecomeInactive(void)
{
    // remove window from Gui
    Gameplay::iGui->getDesktop()->removePanel( _window->getPanel() );

    // unregister message callback
    Gameplay::iGui->setMessageCallback( NULL, NULL );
}

/**
 * gui messaging
 */

void FatalityList::messageCallback(gui::Message* message, void* userData)
{
    FatalityList* __this = reinterpret_cast<FatalityList*>( userData );

    // slider event?
    if( message->event == gui::onSlide )
    {
        gui::IGuiPanel* slider =__this->_window->getPanel()->find( "Slider" );
        assert( slider && slider->getSlider() );
        __this->_topIndex = unsigned int( slider->getSlider()->getPosition() );
        __this->updateGui();
    }

    // button is pressed?
    if( message->event == gui::onButtonClick )
    {
        // ok?
        if( strcmp( message->origin->getName(), "Ok" ) == 0 )
        {
            __this->_endOfActivity = true;
        }
        // scroll buttons?
        else if( strcmp( message->origin->getName(), "ScrollUp" ) == 0 )
        {
            __this->_topIndex--;
            gui::IGuiPanel* slider =__this->_window->getPanel()->find( "Slider" );
            assert( slider && slider->getSlider() );            
            slider->getSlider()->setPosition( float( __this->_topIndex ) );
            __this->updateGui();
        }
        else if( strcmp( message->origin->getName(), "ScrollDown" ) == 0 )
        {
            __this->_topIndex++;
            gui::IGuiPanel* slider =__this->_window->getPanel()->find( "Slider" );
            assert( slider && slider->getSlider() );            
            slider->getSlider()->setPosition( float( __this->_topIndex ) );
            __this->updateGui();
        }
    }

    // mouse wheel
    if( message->event == gui::onMouseWheel )
    {
        __this->_topIndex += int( message->mouseX );
        gui::IGuiPanel* slider =__this->_window->getPanel()->find( "Slider" );
        assert( slider && slider->getSlider() );            
        slider->getSlider()->setPosition( float( __this->_topIndex ) );
        __this->updateGui();
    }
}

/**
 * private behaviour
 */

void FatalityList::initialize(void)
{
    _endOfActivity = false;
    _topIndex = 0;
    _freshIncident = NULL;

    // create window
    _window = Gameplay::iGui->createWindow( "FatalityList" ); assert( _window );

    // open fatality list database
    TiXmlDocument* index = new TiXmlDocument( "./usr/fatality.xml" );
    index->LoadFile();

    // enumerate incident nodes
    TiXmlNode* child = index->FirstChild();
    if( child ) do 
    {
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "incident" ) == 0 )
        {
            _incidents.push_back( new Incident( static_cast<TiXmlElement*>( child ) ) );
        }
        child = child->NextSibling();
    }
    while( child != NULL );

    // close index document
    delete index;
}

void FatalityList::updateGui(void)
{    
    // sort indices
    sortByScore();

    // check bounds
    if( _topIndex < 0 ) _topIndex = 0;
    if( _incidents.size() <= FL_NUM_ITEMS ) _topIndex = 0;
    if( _incidents.size() > FL_NUM_ITEMS && 
        _topIndex + FL_NUM_ITEMS > int( _incidents.size() ) )
    {
        _topIndex = _incidents.size() - FL_NUM_ITEMS;
    }

    // retrieve slider
    gui::IGuiPanel* slider = _window->getPanel()->find( "Slider" );
    assert( slider && slider->getSlider() );

    // update slider
    float lowerLimit = 0.0f;
    float upperLimit = float( _incidents.size() - FL_NUM_ITEMS );
    if( upperLimit < 0 ) upperLimit = 0;
    slider->getSlider()->setLowerLimit( lowerLimit );
    slider->getSlider()->setUpperLimit( upperLimit );

    // update items
    for( unsigned int i=0; i<FL_NUM_ITEMS; i++ )
    {
        // obtain slot item
        gui::IGuiPanel* item = NULL;
        switch( i )
        {
        case 0: item = _window->getPanel()->find( "Item01" ); break;
        case 1: item = _window->getPanel()->find( "Item02" ); break;
        case 2: item = _window->getPanel()->find( "Item03" ); break;
        case 3: item = _window->getPanel()->find( "Item04" ); break;
        case 4: item = _window->getPanel()->find( "Item05" ); break;
        case 5: item = _window->getPanel()->find( "Item06" ); break;    
        case 6: item = _window->getPanel()->find( "Item07" ); break;
        case 7: item = _window->getPanel()->find( "Item08" ); break;
        case 8: item = _window->getPanel()->find( "Item09" ); break;
        case 9: item = _window->getPanel()->find( "Item10" ); break;
        }
        assert( item );

        // obtain item controls
        gui::IGuiPanel* number = item->find( "Place" ); assert( number && number->getStaticText() );
        gui::IGuiPanel* name   = item->find( "Name" ); assert( name && name->getStaticText() );
        gui::IGuiPanel* score  = item->find( "Score" ); assert( score && score->getStaticText() );
        gui::IGuiPanel* jumps  = item->find( "Jumps" ); assert( jumps && jumps->getStaticText() );
        gui::IGuiPanel* site   = item->find( "Site" ); assert( site && site->getStaticText() );

        // item is enabled?
        if( _topIndex + i < _incidents.size() )
        {
            Incident* incident = _incidents[_sortedIndices[_topIndex + i]];            
            database::LocationInfo* incidentSite = database::LocationInfo::getRecord( incident->getLocationId() );            

            // update text
            number->getStaticText()->setText( wstrformat( L"%d", _topIndex + i ).c_str() );
            name->getStaticText()->setText( asciizToUnicode( incident->getName() ).c_str() );
            score->getStaticText()->setText( wstrformat( L"%d", int(incident->getVirtues()->evolution.score) ).c_str() );
            jumps->getStaticText()->setText( wstrformat( L"%d/%d", incident->getVirtues()->statistics.numSkydives, incident->getVirtues()->statistics.numBaseJumps ).c_str() );
            site->getStaticText()->setText( Gameplay::iLanguage->getUnicodeString( incidentSite->nameId ) );

            // update color
            Vector4f textColor = ( incident == _freshIncident ) ? 
                                 ( Vector4f( 0.25f, 1.0f, 0.25f, 1.0f ) ) :
                                 ( Vector4f( 1.0f, 1.0f, 0.25f, 1.0f ) );
            number->getStaticText()->setTextColor( textColor );
            name->getStaticText()->setTextColor( textColor );
            score->getStaticText()->setTextColor( textColor );
            jumps->getStaticText()->setTextColor( textColor );
            site->getStaticText()->setTextColor( textColor );
        }
        else
        {
            number->getStaticText()->setText( L"" );
            number->setHint( L"" );
            name->getStaticText()->setText( L"" );
            name->setHint( L"" );
            score->getStaticText()->setText( L"" );
            score->setHint( L"" );
            jumps->getStaticText()->setText( L"" );
            jumps->setHint( L"" );
            site->getStaticText()->setText( L"" );
            site->setHint( L"" );
        }
    }
}

void FatalityList::sortByScore(void)
{
    // build unsorted indices
    Indices unsorted;
    for( unsigned int i=0; i<_incidents.size(); i++ ) unsorted.push_back( i );

    _sortedIndices.clear();

    unsigned int bestId;
    float bestScore;
    while( unsorted.size() )
    {
        bestId = 0;
        bestScore = _incidents[unsorted[bestId]]->getVirtues()->evolution.score;

        for( unsigned int i=1; i<unsorted.size(); i++ )
        {
            if( _incidents[unsorted[i]]->getVirtues()->evolution.score > bestScore )
            {
                bestId = i;
                bestScore = _incidents[unsorted[bestId]]->getVirtues()->evolution.score;
            }
        }
        _sortedIndices.push_back( unsorted[bestId] );
        unsorted.erase( &unsorted[bestId] );
    }
}