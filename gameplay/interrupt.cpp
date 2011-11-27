
#include "headers.h"
#include "interrupt.h"
#include "version.h"

/**
 * camera of interrupt mode
 */

Interrupt::Camera::Camera(Scene* scene, const Matrix4f& cameraMatrix) : Actor( scene )
{
    _cameraFOV    = 60.0f * CAMERA_FOV_MULTIPLIER;
    _cameraMatrix = cameraMatrix;
}

Interrupt::Camera::~Camera()
{
}

void Interrupt::Camera::onUpdateActivity(float dt)
{
    // camera is actual
    Gameplay::iEngine->getDefaultCamera()->setFOV( _cameraFOV );
    Gameplay::iEngine->getDefaultCamera()->getFrame()->setMatrix( _cameraMatrix );
    _scene->getScenery()->happen( this, EVENT_CAMERA_IS_ACTUAL );
    if( _scene->getTopMode() ) _scene->getTopMode()->happen( this, EVENT_CAMERA_IS_ACTUAL );

    // RT-RS pass
    bool flares = ( _scene->getLocation()->getWeather() == ::wtSunny ) || ( _scene->getLocation()->getWeather() == ::wtVariable );
    Gameplay::iGameplay->getRenderTarget()->render( _scene, _cameraMatrix, _cameraFOV, flares, false );
    // GUI
    Gameplay::iEngine->getDefaultCamera()->beginScene( 0, Vector4f( 0,0,0,0 ) );    
    Gameplay::iGui->render();    
    Gameplay::iEngine->getDefaultCamera()->endScene();
    // present result
    Gameplay::iEngine->present();
}

/**
 * interrupt mode : class implementation
 */

Interrupt::Interrupt(Scene* scene, const Matrix4f& cameraMatrix, bool* interruptResult, bool payCredits) : Mode( scene )
{
    _endOfMode          = false;
    _payCredits         = payCredits;
    _camera             = new Camera( scene, cameraMatrix );
    _interruptResult    = interruptResult;
    _modeStartupTimeout = 0.25f;
    
    _window = Gameplay::iGui->createWindow( "Interrupt" ); assert( _window );

    if( _payCredits )
    {
        gui::IGuiPanel* panel = _window->getPanel()->find( "ActionInterrupt" ); assert( panel && panel->getButton() );
        panel->getButton()->setCaption( wstrformat( Gameplay::iLanguage->getUnicodeString(635), scene->getCareer()->getVirtues()->evolution.credits - 1 ).c_str() );
    }
}

Interrupt::~Interrupt()
{
    if( _window ) _window->getPanel()->release();
}

/**
 * interrupt mode : actor abstracts
 */

void Interrupt::onUpdateActivity(float dt)
{
    // startup timeot
    _modeStartupTimeout -= dt;
    if( _modeStartupTimeout > 0 ) return;

    // action: ESC - continue mission
    if( Gameplay::iGameplay->getKeyboardState()->keyState[0x01] & 0x80 )
    {        
        _endOfMode = true;
    }
}

/**
 * interrupt mode : mode abstracts
 */
    
void Interrupt::onSuspend(void)
{    
    Gameplay::iGui->setMessageCallback( NULL, NULL );
    Gameplay::iGui->getDesktop()->removePanel( _window->getPanel() );    
    if( _scene->getCamera() )
    {
        if( _scene->getCamera() == _camera ) _scene->setCamera( NULL );        
    }
}

void Interrupt::onResume(void)
{
    Gameplay::iGui->setCursorVisible( true );    
    Gameplay::iGui->getDesktop()->setColor( Vector4f( 0,0,0,0 ) );
    Gameplay::iGui->getDesktop()->insertPanel( _window->getPanel() );
    Gameplay::iGui->setMessageCallback( messageCallback, this );
    _window->align( gui::atCenter, 0, gui::atCenter, 0 );
    _scene->setCamera( _camera );
}

bool Interrupt::endOfMode(void)
{
    return _endOfMode;
}

/**
 * interrupt mode : gui messaging
 */

void Interrupt::messageCallback(gui::Message* message, void* userData)
{
    Interrupt* __this = reinterpret_cast<Interrupt*>( userData );

    // button event?
    if( message->event == gui::onButtonClick )
    {
        // continue mission?
        if( strcmp( message->origin->getName(), "ActionContinue" ) == 0 )
        {
            __this->_endOfMode = true;
        }
        // interrupt mission? 
		// This is used to abort a mission once started
        else if( strcmp( message->origin->getName(), "ActionInterrupt" ) == 0 )
        {
            if( __this->_payCredits ) 
            {
                if( __this->_scene->getCareer()->getVirtues()->evolution.credits > 1 )
                {
                    bool creditsIsAffected = true;                    
                    // credits is not affected for licensed character
                    creditsIsAffected = !__this->_scene->getCareer()->getLicensedFlag();
                    // credits is not affected in developer edition
                    #ifdef GAMEPLAY_DEVELOPER_EDITION
                        creditsIsAffected = false;
                    #endif

                    if( creditsIsAffected ) __this->_scene->getCareer()->getVirtues()->evolution.credits--;
                    (*__this->_interruptResult) = true;
                    __this->_endOfMode = true;
                }
            }
            else
            {
                (*__this->_interruptResult) = true;
                __this->_endOfMode = true;
            }
        }
    }
}