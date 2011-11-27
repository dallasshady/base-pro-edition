
#include "headers.h"
#include "gameplay.h"
#include "selecthome.h"

/**
 * class implementation
 */

SelectHome::SelectHome(Geoscape* geoscape) : GeoscapeMode(geoscape)
{
    // create message box
    _messageBox = Gameplay::iGui->createWindow( "MessageBox" ); assert( _messageBox );
    Gameplay::iGui->getDesktop()->insertPanel( _messageBox->getPanel() );
    _messageBox->align( gui::atCenter, 0, gui::atCenter, 0 );
    _messageBox->setTopWindow();

    // setup text
    gui::IGuiPanel* panel = _messageBox->getPanel()->find( "MessageText" ); 
    assert( panel );
    gui::IGuiStaticText* staticText = panel->getStaticText();
    assert( staticText );
    staticText->setText( Gameplay::iLanguage->getUnicodeString(73) );

    // load world map
    _worldMap = new BMP;
    bool result = _worldMap->ReadFromFile( "./res/worldmap.bmp" );
    assert( result );

    // setup class behaviour
    _endOfMode = false;
    _homeX = 0;
    _homeY = 0;
}

SelectHome::~SelectHome()
{
    delete _worldMap;
    if( _messageBox ) _messageBox->getPanel()->release();
}

/**
 * GeoscapeMode
 */

bool SelectHome::endOfMode(void)
{
    return _endOfMode;
}

void SelectHome::onGuiMessage(gui::Message* message)
{
    if( strcmp( message->origin->getName(), "OkButton" ) == 0 && 
        message->event == gui::onButtonClick )
    {
        _messageBox->getPanel()->release();
        _messageBox = NULL;
        return;
    }

    if( strcmp( message->origin->getName(), "Geoscape" ) == 0 && 
        message->event == gui::onMouseDown &&
        message->mouseButton == gui::mbLeft &&
        _messageBox == NULL )
    {
        // check location color
        bool homeApproved = true;
        int x = _geoscape->getCursorX();
        int y = _geoscape->getCursorY();
        RGBApixel pixel = _worldMap->GetPixel( x / 2, y / 2 );
        if( pixel.Blue > pixel.Red && pixel.Blue > pixel.Green ) homeApproved = false;
        if( pixel.Red > 200 && pixel.Green > 200 && pixel.Blue > 200 ) homeApproved = false;
        if( homeApproved )
        {
            getCore()->logMessage( "Home approved, %d %d %d", pixel.Red, pixel.Green, pixel.Blue );
        }
        else
        {
            getCore()->logMessage( "Home rejected, %d %d %d", pixel.Red, pixel.Green, pixel.Blue );
        }

        // setup home location
        if( homeApproved )
        {
            _endOfMode = true;
            _homeX = _geoscape->getCursorX();
            _homeY = _geoscape->getCursorY();
        }
    }
}

void SelectHome::onUpdateMode(float dt)
{
}