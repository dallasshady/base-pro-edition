
#include "headers.h"
#include "windpointer.h"
#include "../common/istring.h"

/**
 * actor abstracts
 */

void WindPointer::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
    if( eventId == EVENT_CAMERA_IS_ACTUAL )
    {
        // current wind
        NxVec3 wind = _scene->getWindAtPoint( NxVec3( 0,0,0 ) );

        // make signature worldspace position
        Matrix4f cameraPose = _scene->getCamera()->getPose();
        Vector3f cameraPos( cameraPose[3][0], cameraPose[3][1], cameraPose[3][2] );
        Vector3f pos( wind.x, wind.y, wind.z );
        pos.normalize();
        pos *= 10000.0f;
        pos += cameraPos;

        // update signature window
        Vector3f screenPos = Gameplay::iEngine->getDefaultCamera()->projectPosition( pos );
        if( screenPos[2] > 1 )
        {
            _signature->getPanel()->setVisible( false );
            return;
        }
        else
        {
            _signature->getPanel()->setVisible( true );
            gui::Rect oldRect = _signature->getPanel()->getRect();
            gui::Rect newRect(
                int( screenPos[0] ), int( screenPos[1] ),
                int( screenPos[0] ) + oldRect.getWidth(),
                int( screenPos[1] ) + oldRect.getHeight()
            );
            _signature->getPanel()->setRect( newRect );
            _windSpeed->getStaticText()->setText( wstrformat( Gameplay::iLanguage->getUnicodeString(295), wind.magnitude() ).c_str() );
        }        
    }
}

void WindPointer::onUpdateActivity(float dt)
{
}

/**
 * class implementation
 */

WindPointer::WindPointer(Actor* parent) : Actor( parent )
{
    _signature = Gameplay::iGui->createWindow( "WindSignature" ); assert( _signature );
    _windSpeed = _signature->getPanel()->find( "WindSpeed" ); assert( _windSpeed && _windSpeed->getStaticText() );
    Gameplay::iGui->getDesktop()->insertPanel( _signature->getPanel() );
}

WindPointer::~WindPointer()
{
    _signature->getPanel()->release();
}