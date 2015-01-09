
#include "headers.h"
#include "mission.h"
#include "imath.h"
#include "database.h"
#include "../common/istring.h"
#include "version.h"

/**
 * class implementation
 */

Mission::FreeCamera::FreeCamera(Scene* scene) : Actor( scene )
{
    _cameraFOV  = 60.0f * CAMERA_FOV_MULTIPLIER;
    _cameraTilt = 0.0f;
    _cameraTurn = 0.0f;
    _cameraPos.set( 0,0,0 );
    _cameraMatrix.set( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );

    _writeWaypointMode = false;
    _writeWaypointModeTimeout = 0.25f;
    _lastWaypointPosition.set( 0,0,0 );
}

Mission::FreeCamera::~FreeCamera()
{
}

/**
 * actor abstracts
 */
   
void Mission::FreeCamera::onUpdateActivity(float dt)
{
    _writeWaypointModeTimeout += dt;

    Vector3f z = Vector3f( -_cameraMatrix[2][0], -_cameraMatrix[2][1], -_cameraMatrix[2][2] );
    Vector3f zFlat = z; zFlat[1] = 0; zFlat.normalize();
    Vector3f axis; axis.cross( z, zFlat ); axis.normalize();
    float angle = ::calcAngle( z, zFlat, axis );
    if( z[1] > 0 ) angle *= -1;

    float waypointVel = 30;
    if( angle > 0 )
    {
        float minAngle = 0;
        float minVel   = 30;
        float maxAngle = 89;
        float maxVel   = 10;
        float factor   = ( angle - minAngle ) / ( maxAngle - minAngle );
        factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );
        waypointVel = minVel * ( 1 - factor ) + maxVel * factor;
    }
    else if( angle < 0 )
    {
        float minAngle = -60;
        float minVel   = 60;
        float maxAngle = 0;
        float maxVel   = 30;
        float factor   = ( angle - minAngle ) / ( maxAngle - minAngle );
        factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );
        waypointVel = minVel * ( 1 - factor ) + maxVel * factor;
    }

    // switch waypoint mode
    if( Gameplay::iGameplay->getKeyboardState()->keyState[0x1C] & 0x80 )
    {
        if( _writeWaypointModeTimeout > 0.25f )
        {
            // reset timeout
            _writeWaypointModeTimeout = 0.0f;

            // switch trigger
            _writeWaypointMode = !_writeWaypointMode;

            // initialize output file
            if( _writeWaypointMode )
            {
                std::string text = strformat( 
                    "<waypoint pos=\"%3.1f,%3.1f,%3.1f\" dir=\"%3.1f,%3.1f,%3.1f\" vel=%3.1f />",
                    _cameraMatrix[3][0]*0.01f, _cameraMatrix[3][1]*0.01f, _cameraMatrix[3][2]*0.01f,
                    -_cameraMatrix[2][0], -_cameraMatrix[2][1], -_cameraMatrix[2][2],
                    waypointVel
                );

                FILE* f = fopen( "./usr/waypoints.xml", "w+t" );
                assert( f );
                fseek( f, 0, SEEK_END );
                fprintf( f, text.c_str() );
                fprintf( f, "\n" );
                fclose( f );

                _lastWaypointPosition.set( _cameraMatrix[3][0], _cameraMatrix[3][1], _cameraMatrix[3][2] );
            }
        }
    }

    // write waypoint
    if( _writeWaypointMode )
    {
        Vector3f offset = Vector3f( _cameraMatrix[3][0], _cameraMatrix[3][1], _cameraMatrix[3][2] ) - _lastWaypointPosition;
        if( offset.length() > 2500.0f )
        {            
            std::string text = strformat( 
                "<waypoint pos=\"%3.1f,%3.1f,%3.1f\" dir=\"%3.1f,%3.1f,%3.1f\" vel=%3.1f />",
                _cameraMatrix[3][0]*0.01f, _cameraMatrix[3][1]*0.01f, _cameraMatrix[3][2]*0.01f,
                -_cameraMatrix[2][0], -_cameraMatrix[2][1], -_cameraMatrix[2][2],
                waypointVel
            );

            FILE* f = fopen( "./usr/waypoints.xml", "r+t" );
            if( !f ) f = fopen( "./usr/waypoints.xml", "w+t" );
            assert( f );
            fseek( f, 0, SEEK_END );
            fprintf( f, text.c_str() );
            fprintf( f, "\n" );
            fclose( f );

            _lastWaypointPosition.set( _cameraMatrix[3][0], _cameraMatrix[3][1], _cameraMatrix[3][2] );
        }
    }

    // retrieve action channels
    ActionChannel* headLeft  = Gameplay::iGameplay->getActionChannel( iaHeadLeft );
    ActionChannel* headRight = Gameplay::iGameplay->getActionChannel( iaHeadRight );
    ActionChannel* headUp    = Gameplay::iGameplay->getActionChannel( iaHeadUp );
    ActionChannel* headDown  = Gameplay::iGameplay->getActionChannel( iaHeadDown );
    ActionChannel* zoomIn    = Gameplay::iGameplay->getActionChannel( iaZoomIn );
    ActionChannel* zoomOut   = Gameplay::iGameplay->getActionChannel( iaZoomOut );
    ActionChannel* phase     = Gameplay::iGameplay->getActionChannel( iaPhase );

    // rotate camera    
    _cameraTurn += 180 * dt * headLeft->getAmplitude();
    _cameraTurn -= 180 * dt * headRight->getAmplitude();
    _cameraTilt += 180 * dt * headUp->getAmplitude();
    _cameraTilt -= 180 * dt * headDown->getAmplitude();
    if( _cameraTilt < -89 ) _cameraTilt = -89;
    if( _cameraTilt > 89 ) _cameraTilt = 89;

    // make rotational component 
    _cameraMatrix.set( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );
    _cameraMatrix = Gameplay::iEngine->rotateMatrix( _cameraMatrix, Vector3f( 1,0,0 ), _cameraTilt );
    _cameraMatrix = Gameplay::iEngine->rotateMatrix( _cameraMatrix, Vector3f( 0,1,0 ), _cameraTurn );
    
    // move camera
    Vector3f at( _cameraMatrix[2][0], _cameraMatrix[2][1], _cameraMatrix[2][2] );
    Vector3f Vcontrol( 0,0,0 );
    Vcontrol += at * zoomOut->getAmplitude() * 10000.0f * dt;
    Vcontrol += at * zoomIn->getAmplitude() * -10000.0f * dt;

    // let camera follow jumpers in view range
    _cameraPos += Vcontrol;

    // make translational component
    _cameraMatrix = Gameplay::iEngine->translateMatrix(
        _cameraMatrix, 
        _cameraPos
    );

    // camera is actual now
    Gameplay::iEngine->getDefaultCamera()->setFOV( _cameraFOV );
    Gameplay::iEngine->getDefaultCamera()->getFrame()->setMatrix( _cameraMatrix );
    _scene->getScenery()->happen( this, EVENT_CAMERA_IS_ACTUAL );
    if( _scene->getTopMode() ) _scene->getTopMode()->happen( this, EVENT_CAMERA_IS_ACTUAL );

    // RT-RS pass
    bool flares = ( _scene->getLocation()->getWeather() == ::wtSunny ) || ( _scene->getLocation()->getWeather() == ::wtVariable );
    Gameplay::iGameplay->getRenderTarget()->render( _scene, _cameraMatrix, _cameraFOV, flares, false );
    // GUI
    Gameplay::iEngine->getDefaultCamera()->beginScene( 0, Vector4f( 0,0,0,0 ) );    
    if( _scene->isHUDEnabled() ) 
    {
        Gameplay::iGui->render();
        std::wstring text = wstrformat( 
            L"pos: %2.1f,%2.1f,%2.1f\ndir: %2.2f,%2.2f,%2.2f", 
            _cameraMatrix[3][0], _cameraMatrix[3][1], _cameraMatrix[3][2], 
            -_cameraMatrix[2][0], -_cameraMatrix[2][1], -_cameraMatrix[2][2] 
        );
        if( _writeWaypointMode )
        {
            text += L"\nWriting waypoints...";
        }
        Vector3f screenSize = Gameplay::iEngine->getScreenSize();
        Gameplay::iGui->renderUnicodeText( 
            gui::Rect( 1,1, int(screenSize[0])+1, int(screenSize[1])+1 ), 
            "caption", 
            Vector4f( 0,0,0,0.5f ), gui::atCenter, gui::atCenter, true, 
            text.c_str()    
        );
        Gameplay::iGui->renderUnicodeText( 
            gui::Rect( 0,0, int(screenSize[0]), int(screenSize[1]) ), 
            "caption", 
            Vector4f( 1,1,1,1 ), gui::atCenter, gui::atCenter, true, 
            text.c_str()    
        );
    }
    Gameplay::iEngine->getDefaultCamera()->endScene();
    // present result
    Gameplay::iEngine->present();

    #ifdef GAMEPLAY_DEVELOPER_EDITION
        if( phase->getTrigger() )
        {
            makeEnvironmentMap();
        }
    #endif
}

void Mission::FreeCamera::makeEnvironmentMap(void)
{
    unsigned int size = 128;
    const char* resourceName = "./res/environment.dds";

    engine::ITexture* result = Gameplay::iEngine->createCubeRenderTarget( size, 32, "cubeRenderTarget" );
    
    // pass cubemap faces
    for( unsigned int i=0; i<6; i++ )
    {
        engine::CubemapFace face;
        Matrix4f cameraMatrix;
        switch( i )
        {
        case 0:
            face = engine::cfPositiveX;
            cameraMatrix.set( 0,0,1,0,
                              0,1,0,0,
                              -1,0,0,0,
                              0,0,0,1 );
            cameraMatrix = Gameplay::iEngine->translateMatrix( cameraMatrix, _cameraPos );
            break;
        case 1:
            face = engine::cfNegativeX;
            cameraMatrix.set( 0,0,-1,0,
                              0,1,0,0,
                              1,0,0,0,
                              0,0,0,1 );
            cameraMatrix = Gameplay::iEngine->translateMatrix( cameraMatrix, _cameraPos );
            break;
        case 2:
            face = engine::cfPositiveY;
            cameraMatrix.set( 1,0,0,0,
                              0,0,1,0,
                              0,-1,0,0,
                              0,0,0,1 );            
            cameraMatrix = Gameplay::iEngine->translateMatrix( cameraMatrix, _cameraPos );
            break;
        case 3:
            face = engine::cfNegativeY;
            cameraMatrix.set( 1,0,0,0,
                              0,0,-1,0,
                              0,1,0,0,
                              0,0,0,1 );
            cameraMatrix = Gameplay::iEngine->translateMatrix( cameraMatrix, _cameraPos );
            break;
        case 4:
            face = engine::cfPositiveZ;
            cameraMatrix.set( -1,0,0,0,
                              0,1,0,0,
                              0,0,-1,0,
                              0,0,0,1 );            
            cameraMatrix = Gameplay::iEngine->translateMatrix( cameraMatrix, _cameraPos );
            break;
        case 5:
            face = engine::cfNegativeZ;
            cameraMatrix.set( 1,0,0,0,
                              0,1,0,0,
                              0,0,1,0,
                              0,0,0,1 );
            cameraMatrix = Gameplay::iEngine->translateMatrix( cameraMatrix, _cameraPos );
            break;
        }

        // render
        Gameplay::iEngine->getDefaultCamera()->setFarClipPlane( _scene->getPassFarClip(0) );
        Gameplay::iEngine->getDefaultCamera()->setNearClipPlane( _scene->getPassNearClip(0) );
        Gameplay::iEngine->beginEnvironmentMap( result, face, cameraMatrix, engine::cmClearColor | engine::cmClearDepth, Vector4f( 0,0,1,1 ) );
        _scene->getPanorama()->render();
        Gameplay::iEngine->endEnvironmentMap();
        Gameplay::iEngine->getDefaultCamera()->setFarClipPlane( _scene->getPassFarClip(1) );
        Gameplay::iEngine->getDefaultCamera()->setNearClipPlane( _scene->getPassNearClip(1) );
        Gameplay::iEngine->beginEnvironmentMap( result, face, cameraMatrix, engine::cmClearDepth, Vector4f( 0,0,0,1 ) );
        _scene->getStage()->render();
        Gameplay::iEngine->endEnvironmentMap();
    }

    result->save( resourceName );
    result->release();
}