
#include "headers.h"
#include "deletecareer.h"
#include "gameplay.h"
#include "../common/istring.h"
#include "unicode.h"
#include "messagebox.h"
#include "version.h"

/**
 * class implementation
 */ 
    
DeleteCareer::DeleteCareer(Career* career)
{
    assert( career );
    _career = career;
    _endOfActivity = false;
    _window = Gameplay::iGui->createWindow( "DialogBox" ); assert( _window );

    // setup dialog message
    gui::IGuiStaticText* messageText = _window->getPanel()->find( "MessageText" )->getStaticText(); 
    assert( messageText );
    messageText->setText( wstrformat( 
        Gameplay::iLanguage->getUnicodeString(72), 
        asciizToUnicode( _career->getName() ).c_str()
    ).c_str() );
}

DeleteCareer::~DeleteCareer()
{
}

/**
 * Activity
 */ 

void DeleteCareer::updateActivity(float dt)
{
    // can't delete licensed career
    if( !_endOfActivity && _career->getLicensedFlag() ) 
    {
        Gameplay::iGameplay->pushActivity( new Messagebox( Gameplay::iLanguage->getUnicodeString(759) ) );
        _endOfActivity = true;
    }

    // render gui
    Gameplay::iEngine->getDefaultCamera()->beginScene( 
        engine::cmClearColor | engine::cmClearDepth,
    	  Vector4f( 0,0,0,0 )
    );
    Gameplay::iGui->render();
    Gameplay::iEngine->getDefaultCamera()->endScene();
    Gameplay::iEngine->present();
}

bool DeleteCareer::endOfActivity(void)
{
    return _endOfActivity;
}

void DeleteCareer::onBecomeActive(void)
{
    // insert window in Gui
    Gameplay::iGui->getDesktop()->insertPanel( _window->getPanel() );
    _window->align( gui::atCenter, 0, gui::atCenter, 0 );

    // register message callback
    Gameplay::iGui->setMessageCallback( messageCallback, this );
}

void DeleteCareer::onBecomeInactive(void)
{
    // remove window from Gui
    Gameplay::iGui->getDesktop()->removePanel( _window->getPanel() );
    // unregister message callback
    Gameplay::iGui->setMessageCallback( NULL, NULL );
}

/**
 * Gui messgae handler
 */

void DeleteCareer::messageCallback(gui::Message* message, void* userData)
{
    DeleteCareer* __this = reinterpret_cast<DeleteCareer*>( userData );

    if( message->event == gui::onButtonClick )
    {
        if( strcmp( message->origin->getName(), "OkButton" ) == 0 )
        {
            Gameplay::iGameplay->deleteCareer( __this->_career );
            Gameplay::iGameplay->saveCareers();
            __this->_endOfActivity = true;
        }
        else if( strcmp( message->origin->getName(), "CancelButton" ) == 0 )
        {
            __this->_endOfActivity = true;
        }        
    }
}