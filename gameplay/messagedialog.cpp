
#include "headers.h"
#include "careercourse.h"
#include "gameplay.h"

/**
 * class implementation
 */

MessageDialog::MessageDialog(const wchar_t* message)
{
    _window = Gameplay::iGui->createWindow( "MessageBox" ); assert( _window );
    gui::IGuiStaticText* messageText = _window->getPanel()->find( "MessageText" )->getStaticText(); assert( messageText );    
    messageText->setText( message );
    _isComplete = false;

    Gameplay::iGui->getDesktop()->insertPanel( _window->getPanel() );
    _window->align( gui::atCenter, 0, gui::atCenter, 0 );
}

MessageDialog::~MessageDialog()
{
    _window->getPanel()->release();
}

/**
 * CareerDialog implementation
 */

bool MessageDialog::isComplete(void)
{
    return _isComplete;
}

void MessageDialog::onGuiMessage(gui::Message* message)
{
    if( message->event == gui::onButtonClick && strcmp( message->origin->getName(), "OkButton" ) == 0 )
    {
        _isComplete = true;
    }
}