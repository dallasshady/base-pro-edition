
#include "headers.h"
#include "messagebox.h"
#include "gameplay.h"

/**
 * class implementation
 */

Messagebox::Messagebox(const wchar_t* message)
{
    _window = Gameplay::iGui->createWindow( "MessageBox" ); assert( _window );
    gui::IGuiStaticText* messageText = _window->getPanel()->find( "MessageText" )->getStaticText(); assert( messageText );
    messageText->setText( message );
    _endOfActivity = false;
}

Messagebox::~Messagebox()
{
    _window->getPanel()->release();
}

/**
 * gui message handling
 */

void Messagebox::messageCallback(gui::Message* message, void* userData)
{
    if( message->event == gui::onButtonClick && strcmp( message->origin->getName(), "OkButton" ) == 0 )
    {
        reinterpret_cast<Messagebox*>( userData )->_endOfActivity = true;
    }
}

/**
 * Activity
 */
    
void Messagebox::updateActivity(float dt)
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

bool Messagebox::endOfActivity(void)
{
    return _endOfActivity;
}

void Messagebox::onBecomeActive(void)
{
    // insert window in Gui
    Gameplay::iGui->getDesktop()->insertPanel( _window->getPanel() );
    _window->align( gui::atCenter, 0, gui::atCenter, 0 );

    // register message callback
    Gameplay::iGui->setMessageCallback( messageCallback, this );
}

void Messagebox::onBecomeInactive(void)
{
    // remove window from Gui
    Gameplay::iGui->getDesktop()->removePanel( _window->getPanel() );
    // unregister message callback
    Gameplay::iGui->setMessageCallback( NULL, NULL );
}