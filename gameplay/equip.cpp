
#include "headers.h"
#include "equip.h"
#include "imath.h"
#include "database.h"
#include "mission.h"
#include "version.h"

/**
 * basejumper animation sequences
 */

static engine::AnimSequence passiveFlightSequence = 
{ 
    FRAMETIME(592), 
    FRAMETIME(652), 
    engine::ltPeriodic, 
    FRAMETIME(592) 
};

/**
 * equip camera
 */
 
Equip::Camera::Camera(Scene* scene, Equip* equip) : Actor(scene)
{
    _name           = "EquipCamera";
    _equip          = equip;
    _cameraTilt     = 0;
    _cameraTurn     = 0;
    _cameraDistance = 650.0f;
    _cameraHeight   = 314.0f;
    _cameraFOV      = 60.0f * CAMERA_FOV_MULTIPLIER;
    _cameraMatrix.set( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );
}

Equip::Camera::~Camera()
{
}

void Equip::Camera::onUpdateActivity(float dt)
{
    // calculate camera matrix    
    _cameraMatrix.set( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );
    _cameraMatrix = Gameplay::iEngine->rotateMatrix( _cameraMatrix, Vector3f( 1,0,0 ), _cameraTilt );
    _cameraMatrix = Gameplay::iEngine->rotateMatrix( _cameraMatrix, Vector3f( 0,1,0 ), _cameraTurn );
    Vector3f at( _cameraMatrix[2][0], _cameraMatrix[2][1], _cameraMatrix[2][2] );
    _cameraMatrix = Gameplay::iEngine->translateMatrix( 
        _cameraMatrix, 
        Vector3f( 0, _cameraHeight, 0 ) + at * _cameraDistance
    );

    // camera is actual now
    Gameplay::iEngine->getDefaultCamera()->setFOV( _cameraFOV );
    Gameplay::iEngine->getDefaultCamera()->getFrame()->setMatrix( _cameraMatrix );
    _scene->getScenery()->happen( this, EVENT_CAMERA_IS_ACTUAL );
    if( _scene->getTopMode() ) _scene->getTopMode()->happen( this, EVENT_CAMERA_IS_ACTUAL );

    // RT-RS pass
    Gameplay::iGameplay->getRenderTarget()->render( _equip, _cameraMatrix, _cameraFOV, true, false );

    // gui
    Gameplay::iEngine->getDefaultCamera()->beginScene( 0, Vector4f( 0,0,0,0 ) );
    Gameplay::iGui->render();

    // promt
    Vector3f screenSize = Gameplay::iEngine->getScreenSize();
    gui::Rect rect = gui::Rect( 0, int(screenSize[1]) - 64, int(screenSize[0]), int(screenSize[1]) );
    Gameplay::iGui->renderUnicodeText( rect, "caption", Vector4f( 0, 0, 0, 0.5f ), gui::atCenter, gui::atCenter, true, Gameplay::iLanguage->getUnicodeString(383) );
    rect.left -= 1, rect.right -= 1, rect.top -= 1, rect.bottom -= 1;
    Gameplay::iGui->renderUnicodeText( rect, "caption", Vector4f( 0.25f, 1.0f, 0.25f, 1.0f ), gui::atCenter, gui::atCenter, true, Gameplay::iLanguage->getUnicodeString(383) );

    Gameplay::iEngine->getDefaultCamera()->endScene();

    // present result
    Gameplay::iEngine->present();
}

/**
 * equip : abstraction layer
 */

void Equip::onUpdateActivity(float dt)
{
    // action: ESC - end mission
    if( Gameplay::iGameplay->getKeyboardState()->keyState[0x01] & 0x80 )
    {
        // close equip
        _endOfMode = true;
        // start mission
        if( _missionInfo ) 
        {
            // update rigging skill
            Virtues* virtues = _scene->getCareer()->getVirtues();
            virtues->skills.rigging += virtues->predisp.rigging *
                                       5 * sqr( getCore()->getRandToolkit()->getUniform( 0.0f, 1.0f ) );
            // start mission
            new Mission( _scene, _missionInfo, _wttid, _wtmid );
        }
    }
}

void Equip::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
    if( eventId == EVENT_CAMERA_IS_ACTUAL )
    {
        // place helmet slot
        Vector3f screenPos = Gameplay::iEngine->getDefaultCamera()->projectPosition( 
            Jumper::getHelmetEquipAnchor( _baseJumper )->getPos()
        );
        if( screenPos[2] > 1 )
        {
            _helmetSlot->getPanel()->setVisible( false );
        }
        else
        {
            _helmetSlot->getPanel()->setVisible( true );
            gui::Rect oldRect = _helmetSlot->getPanel()->getRect();
            gui::Rect newRect(
                int( screenPos[0] ), int( screenPos[1] ),
                int( screenPos[0] ) + oldRect.getWidth(),
                int( screenPos[1] ) + oldRect.getHeight()
            );
            _helmetSlot->getPanel()->setRect( newRect );

            // no helmet selection for LICENSED_CHAR
            #ifdef GAMEPLAY_EDITION_ATARI
                if( _scene->getCareer()->getLicensedFlag() )
                {
                    _helmetSlot->getPanel()->setVisible( false );
                }
            #endif
        }

        // place suit slot
        screenPos = Gameplay::iEngine->getDefaultCamera()->projectPosition( 
            Jumper::getSuitEquipAnchor( _baseJumper )->getPos()
        );
        if( screenPos[2] > 1 )
        {
            _suitSlot->getPanel()->setVisible( false );
        }
        else
        {
            _suitSlot->getPanel()->setVisible( true );
            gui::Rect oldRect = _suitSlot->getPanel()->getRect();
            gui::Rect newRect(
                int( screenPos[0] ), int( screenPos[1] ),
                int( screenPos[0] ) + oldRect.getWidth(),
                int( screenPos[1] ) + oldRect.getHeight()
            );
            _suitSlot->getPanel()->setRect( newRect );
        }

        // place rig slot
        screenPos = Gameplay::iEngine->getDefaultCamera()->projectPosition( 
            Jumper::getRigEquipAnchor( _baseJumper )->getPos()
        );
        if( screenPos[2] > 1 )
        {
            _rigSlot->getPanel()->setVisible( false );
        }
        else
        {
            _rigSlot->getPanel()->setVisible( true );
            gui::Rect oldRect = _rigSlot->getPanel()->getRect();
            gui::Rect newRect(
                int( screenPos[0] ), int( screenPos[1] ),
                int( screenPos[0] ) + oldRect.getWidth(),
                int( screenPos[1] ) + oldRect.getHeight()
            );
            _rigSlot->getPanel()->setRect( newRect );
        }

        // place reserve slot
        screenPos = Gameplay::iEngine->getDefaultCamera()->projectPosition( 
            Jumper::getRigEquipAnchor( _baseJumper )->getPos()
        );
        if( screenPos[2] > 1 )
        {
            _reserveSlot->getPanel()->setVisible( false );
        }
        else
        {
			if (database::Rig::getRecord(_virtues->equipment.rig.id)->skydiving) {
				_reserveSlot->getPanel()->setVisible( true );
			} else {
				_reserveSlot->getPanel()->setVisible( false );
			}

            gui::Rect oldRect = _reserveSlot->getPanel()->getRect();
			screenPos[0] -= oldRect.getWidth();
            gui::Rect newRect(
                int( screenPos[0] ), int( screenPos[1] ),
                int( screenPos[0] ) + oldRect.getWidth(),
                int( screenPos[1] ) + oldRect.getHeight()
            );
            _reserveSlot->getPanel()->setRect( newRect );
        }

        // place canopy slot
        screenPos = Gameplay::iEngine->getDefaultCamera()->projectPosition( 
            _canopy->getFrame()->getPos()
        );
        if( screenPos[2] > 1 )
        {
            _canopySlot->getPanel()->setVisible( false );
        }
        else
        {
            _canopySlot->getPanel()->setVisible( true );
            gui::Rect oldRect = _canopySlot->getPanel()->getRect();
            gui::Rect newRect(
                int( screenPos[0] ), int( screenPos[1] ),
                int( screenPos[0] ) + oldRect.getWidth(),
                int( screenPos[1] ) + oldRect.getHeight()
            );
            _canopySlot->getPanel()->setRect( newRect );
        }
    }
}

void Equip::onSuspend(void)
{
    _scene->setCamera( NULL );
    Gameplay::iGui->getDesktop()->setColor( Vector4f( 1,1,1,1 ) );
    Gameplay::iGui->setMessageCallback( NULL, NULL );
}

void Equip::onResume(void)
{
    Gameplay::iGui->setMessageCallback( messageCallback, this );
    Gameplay::iGui->getDesktop()->setColor( Vector4f( 0,0,0,0 ) );
    _scene->setCamera( _camera );
}

bool Equip::endOfMode(void)
{
    return _endOfMode;
}

/**
 * equip mode : RenderSource methods
 */

Vector4f Equip::getClearColor(void)
{
    return _scene->getClearColor();
}

float Equip::getBlur(void)
{
    return _scene->getBlur();
}

float Equip::getBrightPass(void)
{
    return _scene->getBrightPass();
}

float Equip::getBloom(void)
{
    return _scene->getBloom();
}

unsigned int Equip::getNumPasses(void)
{
    return 2;
}

float Equip::getPassNearClip(unsigned int passId)
{
    switch( passId )
    {
    case 0:
        return _scene->getPassNearClip( 0 );
    case 1:
        return 10.0f;
    default:
        return 0.0f;
    }
}

float Equip::getPassFarClip(unsigned int passId)
{
    switch( passId )
    {
    case 0:
        return _scene->getPassFarClip( 0 );
    case 1:
        return 10000.0f;
    default:
        return 0.0f;
    }
}

unsigned int Equip::getPassClearFlag(unsigned int passId)
{
    switch( passId )
    {
    case 0:
        return _scene->getPassClearFlag( 0 );
    case 1:
        return engine::cmClearDepth;
    default:
        return 0;
    }
}

void Equip::renderPass(unsigned int passId)
{
    switch( passId )
    {
    case 0:
        _scene->renderPass( 0 );
        break;
    case 1:
        renderEquipment();
        break;
    }
}

void Equip::renderLensFlares(void)
{
    _scene->renderLensFlares();
}

/**
 * equip mode : overrided rendering
 */

void Equip::renderEquipment(void)
{
    _bsp->render();
}

/**
 * cord builder
 */

void Equip::initializeCords(void)
{
    database::Canopy* canopyInfo = database::Canopy::getRecord( _virtues->equipment.canopy.id );

    // calculate number of cord instances
    unsigned int numCordInstances;
    numCordInstances = canopyInfo->riserScheme->getNumCords() * 4 * 3 +
                       ( canopyInfo->riserScheme->getNumBrakes() + 1 ) * 2;

    // obtain cord template
    engine::IClump* cordTemplate = Gameplay::iGameplay->findClump( "Cord" ); assert( cordTemplate );
    callback::AtomicL atomics;
    cordTemplate->forAllAtomics( callback::enumerateAtomics, &atomics );
    assert( atomics.size() == 1 );

    // build batch scheme
    engine::BatchScheme cordBatchScheme;
    cordBatchScheme.numLods = 1;
    cordBatchScheme.lodGeometry[0] = (*atomics.begin())->getGeometry();
    cordBatchScheme.lodDistance[0] = 1000000;
    assert( cordBatchScheme.isValid() );

    // create cord batch
    _cordBatch = Gameplay::iEngine->createBatch( numCordInstances, &cordBatchScheme );
    assert( _cordBatch );
    _bsp->add( _cordBatch );

    if( _numCords && _cords )
    {
        for( unsigned int i=0; i<_numCords; i++ ) if( _cords[i] ) delete _cords[i];
        delete[] _cords;
        if ( _leftBrake ) delete _leftBrake;
        if ( _rightBrake ) delete _rightBrake;
    }

    _numCords = 4 * canopyInfo->riserScheme->getNumCords();
    _cords = new CanopySimulator::CordSimulator*[_numCords];
    for( unsigned int i=0; i<_numCords; i++ ) _cords[i] = NULL;
    _leftBrake = _rightBrake = NULL;

    // create cords
    float cascade = canopyInfo->cascade * 100.0f;
    unsigned int cordId = 0;
    unsigned int instanceId = 0;
    engine::IFrame* innerJoint;
    engine::IFrame* outerJoint;
    for( unsigned int i=0; i<canopyInfo->riserScheme->getNumCords(); i++ )
    {
        innerJoint = Gameplay::iEngine->findFrame( _canopy->getFrame(), canopyInfo->riserScheme->getJointName( database::RiserScheme::rtInnerFrontLeft, i ) );
        outerJoint = Gameplay::iEngine->findFrame( _canopy->getFrame(), canopyInfo->riserScheme->getJointName( database::RiserScheme::rtOuterFrontLeft, i ) );
        _cords[cordId] = new CanopySimulator::CordSimulator( cascade, Jumper::getFrontLeftRiser( _baseJumper ), innerJoint, outerJoint, _cordBatch, instanceId );
        cordId++;
        innerJoint = Gameplay::iEngine->findFrame( _canopy->getFrame(), canopyInfo->riserScheme->getJointName( database::RiserScheme::rtInnerFrontRight, i ) );
        outerJoint = Gameplay::iEngine->findFrame( _canopy->getFrame(), canopyInfo->riserScheme->getJointName( database::RiserScheme::rtOuterFrontRight, i ) );
        _cords[cordId] = new CanopySimulator::CordSimulator( cascade, Jumper::getFrontRightRiser( _baseJumper ), innerJoint, outerJoint, _cordBatch, instanceId );
        cordId++;
        innerJoint = Gameplay::iEngine->findFrame( _canopy->getFrame(), canopyInfo->riserScheme->getJointName( database::RiserScheme::rtInnerRearLeft, i ) );
        outerJoint = Gameplay::iEngine->findFrame( _canopy->getFrame(), canopyInfo->riserScheme->getJointName( database::RiserScheme::rtOuterRearLeft, i ) );
        _cords[cordId] = new CanopySimulator::CordSimulator( cascade, Jumper::getRearLeftRiser( _baseJumper ), innerJoint, outerJoint, _cordBatch, instanceId );
        cordId++;
        innerJoint = Gameplay::iEngine->findFrame( _canopy->getFrame(), canopyInfo->riserScheme->getJointName( database::RiserScheme::rtInnerRearRight, i ) );
        outerJoint = Gameplay::iEngine->findFrame( _canopy->getFrame(), canopyInfo->riserScheme->getJointName( database::RiserScheme::rtOuterRearRight, i ) );
        _cords[cordId] = new CanopySimulator::CordSimulator( cascade, Jumper::getRearRightRiser( _baseJumper ), innerJoint, outerJoint, _cordBatch, instanceId );
        cordId++;
    }

    // create brakes
    float brakeAspect = 0.85f;
    engine::IFrame** canopyJoints = new engine::IFrame*[canopyInfo->riserScheme->getNumBrakes()];
    for( i=0; i<canopyInfo->riserScheme->getNumBrakes(); i++ )
    {
        canopyJoints[i] = Gameplay::iEngine->findFrame( _canopy->getFrame(), canopyInfo->riserScheme->getJointName( database::RiserScheme::rtBrakeLeft, i ) );
        if( !canopyJoints[i] )
        {
            throw Exception( "canopy joint not found : %s", canopyInfo->riserScheme->getJointName( database::RiserScheme::rtBrakeLeft, i ) );
        }
    }
    _leftBrake = new CanopySimulator::BrakeSimulator( brakeAspect, canopyInfo->riserScheme->getNumBrakes(), Jumper::getRearLeftRiser( _baseJumper ), canopyJoints, _cordBatch, instanceId );
    for( i=0; i<canopyInfo->riserScheme->getNumBrakes(); i++ )
    {
        canopyJoints[i] = Gameplay::iEngine->findFrame( _canopy->getFrame(), canopyInfo->riserScheme->getJointName( database::RiserScheme::rtBrakeRight, i ) );
    }
    _rightBrake = new CanopySimulator::BrakeSimulator( brakeAspect, canopyInfo->riserScheme->getNumBrakes(), Jumper::getRearRightRiser( _baseJumper ), canopyJoints, _cordBatch, instanceId );
    delete[] canopyJoints;

    // place
    for( unsigned int i=0; i<_numCords; i++ ) _cords[i]->update( 0.0f );
    _leftBrake->update( 0.0f );
    _rightBrake->update( 0.0f );
}

/**
 * equip mode : class implementation
 */

static engine::ILight* cloneLightSourceCB(engine::ILight* light, void* data)
{
    engine::IBSP* bsp = reinterpret_cast<engine::IBSP*>( data );

    // clone light
    if( light->getLightset() == 0 )
    {
        engine::ILight* cloneLight = Gameplay::iEngine->createLight( light->getType() );
        cloneLight->setAttenuation( light->getAttenuation() );
        cloneLight->setDiffuseColor( light->getDiffuseColor() );
        cloneLight->setPhi( light->getPhi() );
        cloneLight->setRange( light->getRange() );
        cloneLight->setSpecularColor( light->getSpecularColor() ); 
        cloneLight->setTheta( light->getTheta() );

        engine::IFrame* frameClone = Gameplay::iEngine->createFrame( "LightFrame" );
        frameClone->setMatrix( light->getFrame()->getLTM() );
        cloneLight->setFrame( frameClone );

        bsp->add( cloneLight );
    }
    return light;
}

static engine::IClump* cloneClumpLightSourcesCB(engine::IClump* clump, void* data)
{
    clump->forAllLights( cloneLightSourceCB, data );
    return clump;
}

Equip::Equip(Scene* scene, database::MissionInfo* missionInfo, unsigned int wttid, unsigned int wtmid) : Mode(scene)
{
    _endOfMode = false;
    _missionInfo = missionInfo;
    _camera = new Camera( scene, this );    
    _wttid = wttid;
    _wtmid = wtmid;
    _cordBatch = NULL;

    _virtues = _scene->getCareer()->getVirtues();

    // create test BSP
    Vector3f inf = _scene->getStage()->getAABBInf();
    inf[1] = inf[1] > 0 ? 0 : inf[1];
    Vector3f sup = _scene->getStage()->getAABBSup();
    _bsp = Gameplay::iEngine->createBSP( "EquipmentStage", inf, sup );

    // clone scene light sources
    _scene->getStage()->forAllClumps( cloneClumpLightSourcesCB, _bsp );

    // reset model members
    _baseJumper = NULL;
    _jumperRenderCallback = NULL;
    _canopy = NULL;
    _canopyRenderCallback = NULL;
    _numCords = 0;
    _cords = NULL, _leftBrake = NULL, _rightBrake = NULL;;

    // build models
    buildEquipment();

    // reset navigation mode
    _selectionMode = _turnMode = _moveMode = false;
    _lastMouseX = _lastMouseY = 0;

    // create gui
    _helmetSlot = Gameplay::iGui->createWindow( "HSREquipSlot" ); assert( _helmetSlot );
    Gameplay::iGui->getDesktop()->insertPanel( _helmetSlot->getPanel() );
    _suitSlot = Gameplay::iGui->createWindow( "HSREquipSlot" ); assert( _suitSlot );
    Gameplay::iGui->getDesktop()->insertPanel( _suitSlot->getPanel() );
    _rigSlot = Gameplay::iGui->createWindow( "HSREquipSlot" ); assert( _rigSlot );
    Gameplay::iGui->getDesktop()->insertPanel( _rigSlot->getPanel() );
    _canopySlot = Gameplay::iGui->createWindow( "CanopyEquipSlot" ); assert( _canopySlot );
    Gameplay::iGui->getDesktop()->insertPanel( _canopySlot->getPanel() );
	
	_reserveSlot = Gameplay::iGui->createWindow( "ReserveEquipSlot" ); assert( _reserveSlot );
    Gameplay::iGui->getDesktop()->insertPanel( _reserveSlot->getPanel() );

    _dropList = Gameplay::iGui->createWindow( "EquipDropList" ); assert( _dropList );
    _dropList->getPanel()->setVisible( false );
    Gameplay::iGui->getDesktop()->insertPanel( _dropList->getPanel() );

    // update gui
    showEquipment();
}

Equip::~Equip()
{
    if( _cordBatch )
    {
        _bsp->remove( _cordBatch );
        _cordBatch->release();
    }

    _dropList->getPanel()->release();
    _helmetSlot->getPanel()->release();
    _suitSlot->getPanel()->release();
    _rigSlot->getPanel()->release();
    _canopySlot->getPanel()->release();
	_reserveSlot->getPanel()->release();

    for( unsigned int i=0; i<_numCords; i++ ) if( _cords[i] ) delete _cords[i];
    delete[] _cords;
    if( _leftBrake ) delete _leftBrake;
    if( _rightBrake ) delete _rightBrake;

    _canopyRenderCallback->restore( _canopy );
    delete _canopyRenderCallback;
    _bsp->remove( _canopy );
    _canopy->release();

    _jumperRenderCallback->restore( _baseJumper );
    delete _jumperRenderCallback;
    _bsp->remove( _baseJumper );
    _baseJumper->release();
    
    _bsp->release();

    delete _camera;
}

/**
 * equip mode : gui messaging
 */

void Equip::messageCallback(gui::Message* message, void* userData)
{
    Equip* __this = reinterpret_cast<Equip*>( userData );

    // left mouse button down on desktop?
    if( message->event == gui::onMouseDown && 
        message->mouseButton == gui::mbLeft &&
        message->origin == Gameplay::iGui->getDesktop() )
    {
        if( __this->_selectionMode )
        {
            __this->_selectionMode = false;
            __this->_dropList->getPanel()->setVisible( false );
        }
        else
        {
            __this->_turnMode = true;
        }
    }

    // left mouse button up?
    if( message->event == gui::onMouseUp && message->mouseButton == gui::mbLeft )
    {
        if( __this->_turnMode ) __this->_turnMode = false;
    }

    // right mouse button down on desktop?
    if( message->event == gui::onMouseDown && 
        message->mouseButton == gui::mbRight &&
        message->origin == Gameplay::iGui->getDesktop() )
    {
        if( __this->_selectionMode )
        {
            __this->_selectionMode = false;
            __this->_dropList->getPanel()->setVisible( false );
        }
        else
        {
            __this->_moveMode = true;
        }
    }

    // right mouse button up?
    if( message->event == gui::onMouseUp && message->mouseButton == gui::mbRight )
    {
        if( __this->_moveMode ) __this->_moveMode = false;
    }

    // mouse wheel?
    if( message->event == gui::onMouseWheel )
    {
        if( __this->_selectionMode )
        {
            gui::IGuiPanel* slider =__this->_dropList->getPanel()->find( "Slider" );
            assert( slider && slider->getSlider() );
            __this->_selectionTop += int( message->mouseX );
            slider->getSlider()->setPosition( float( __this->_selectionTop ) );
            __this->showListbox();
        }
        else
        {
            float distance = __this->_camera->getDistance();
            distance += 25.0f * message->mouseX;
            if( distance < 50.0f ) distance = 50.0f;
            if( distance > 1000.0f ) distance = 1000.0f;
            __this->_camera->setDistance( distance );
        }
    }

    // mouse move?
    if( message->event == gui::onMouseMove )
    {
        // turn mode?
        if( __this->_turnMode )
        {
            float turn = -0.5f * ( message->mouseX - __this->_lastMouseX );
            float tilt = 0.5f * ( message->mouseY - __this->_lastMouseY );
            turn += __this->_camera->getTurn();
            tilt += __this->_camera->getTilt();
            if( tilt < -89 ) tilt = -89;
            if( tilt > 89 ) tilt = 89;
            __this->_camera->setTurn( turn );
            __this->_camera->setTilt( tilt );            
        }
        // move mode?
        if( __this->_moveMode )
        {
            float height = 0.5f * ( message->mouseY - __this->_lastMouseY );
            height += __this->_camera->getHeight();
            if( height < 0 ) height = 0;
            if( height > 500 ) height = 500;            
            __this->_camera->setHeight( height );
        }
        __this->_lastMouseX = message->mouseX;
        __this->_lastMouseY = message->mouseY;
    }

    // listbox slider?
    else if( message->event == gui::onSlide )
    {
        if( __this->_dropList->getPanel()->getVisible() )
        {
            gui::IGuiPanel* slider =__this->_dropList->getPanel()->find( "Slider" );
            assert( slider && slider->getSlider() );
            __this->_selectionTop = unsigned int( slider->getSlider()->getPosition() );
            __this->showListbox();
        }
    }

    // button click?
    if( message->event == gui::onButtonClick )
    {
        // AAD option?
        if( strcmp( message->origin->getName(), "NextAAD" ) == 0 )
        {
			unsigned int count = database::AAD::getNumRecords();

			if (__this->_virtues->equipment.rig.rig_aad + 1 >= count) {
				__this->_virtues->equipment.rig.rig_aad = 0;
			} else {
				++__this->_virtues->equipment.rig.rig_aad;
			}

            __this->showEquipment();

		} else if( strcmp( message->origin->getName(), "PrevAAD" ) == 0 )
        {
			if (__this->_virtues->equipment.rig.rig_aad - 1 < 0) {
				__this->_virtues->equipment.rig.rig_aad = database::AAD::getNumRecords()-1;
			} else {
				--__this->_virtues->equipment.rig.rig_aad;
			}
            __this->showEquipment();

        // slider option?
		} else if( strcmp( message->origin->getName(), "PrevSlider" ) == 0 )
        {
            database::Canopy* canopyInfo = database::Canopy::getRecord( __this->_virtues->equipment.canopy.id );
            if( canopyInfo->skydiving )
            {
                __this->_virtues->equipment.sliderOption = ::soUp;
            }
            else
            {
                switch( __this->_virtues->equipment.sliderOption )
                {
                case ::soUp:
                    __this->_virtues->equipment.sliderOption = ::soRemoved;
                    break;
                case ::soDown:
                    __this->_virtues->equipment.sliderOption = ::soUp;
                    break;
                case ::soRemoved:
                    __this->_virtues->equipment.sliderOption = ::soDown;
                    break;
                }
            }            
            __this->showEquipment();
        }
        else if( strcmp( message->origin->getName(), "NextSlider" ) == 0 )
        {
            database::Canopy* canopyInfo = database::Canopy::getRecord( __this->_virtues->equipment.canopy.id );
            if( canopyInfo->skydiving )
            {
                __this->_virtues->equipment.sliderOption = ::soUp;
            }
            else
            {
                switch( __this->_virtues->equipment.sliderOption )
                {
                case ::soUp:
                    __this->_virtues->equipment.sliderOption = ::soDown;
                    break;
                case ::soDown:
                    __this->_virtues->equipment.sliderOption = ::soRemoved;
                    break;
                case ::soRemoved:
                    __this->_virtues->equipment.sliderOption = ::soUp;
                    break;
                }
            }            
            __this->showEquipment();
        }

        // PC option?
        if( strcmp( message->origin->getName(), "PrevPC" ) == 0 )
        {
            database::Canopy* canopyInfo = database::Canopy::getRecord( __this->_virtues->equipment.canopy.id );
            if( __this->_virtues->equipment.pilotchute == 0 )
            {
                __this->_virtues->equipment.pilotchute = canopyInfo->numPilots - 1;
            }
            else
            {
                __this->_virtues->equipment.pilotchute--;
            }
            __this->showEquipment();
        }
        else if( strcmp( message->origin->getName(), "NextPC" ) == 0 )
        {
            database::Canopy* canopyInfo = database::Canopy::getRecord( __this->_virtues->equipment.canopy.id );
            if( __this->_virtues->equipment.pilotchute == canopyInfo->numPilots - 1 )
            {
                __this->_virtues->equipment.pilotchute = 0;
            }
            else
            {
                __this->_virtues->equipment.pilotchute++;
            }
            __this->showEquipment();
        }

        // scroll buttons?
        if( strcmp( message->origin->getName(), "ScrollUp" ) == 0 )
        {
            if( __this->_dropList->getPanel()->getVisible() )
            {
                gui::IGuiPanel* slider =__this->_dropList->getPanel()->find( "Slider" );
                assert( slider && slider->getSlider() );
                __this->_selectionTop--;            
                slider->getSlider()->setPosition( float( __this->_selectionTop ) );
                __this->showListbox();
            }
        }
        else if( strcmp( message->origin->getName(), "ScrollDown" ) == 0 )
        {
            if( __this->_dropList->getPanel()->getVisible() )
            {
                gui::IGuiPanel* slider =__this->_dropList->getPanel()->find( "Slider" );
                assert( slider && slider->getSlider() );
                __this->_selectionTop++;
                slider->getSlider()->setPosition( float( __this->_selectionTop ) );
                __this->showListbox();
            }
        }

        // equipment item?
        if( strcmp( message->origin->getName(), "Name" ) == 0 )
        {
            // reserve slot
            if( message->origin->getParent() == __this->_reserveSlot->getPanel() )
            {
                __this->selectEquipment(
                    message->origin,
                    gtReserve,
                    &__this->_virtues->equipment.reserve
                );
            }
            // helmet slot
            else if( message->origin->getParent() == __this->_helmetSlot->getPanel() )
            {
                __this->selectEquipment(
                    message->origin,
                    gtHelmet,
                    &__this->_virtues->equipment.helmet
                );
            }
            // suit slot
            else if( message->origin->getParent() == __this->_suitSlot->getPanel() )
            {
                __this->selectEquipment(
                    message->origin,
                    gtSuit,
                    &__this->_virtues->equipment.suit
                );
            }
            // rig slot
            else if( message->origin->getParent() == __this->_rigSlot->getPanel() )
            {
                __this->selectEquipment(
                    message->origin,
                    gtRig,
                    &__this->_virtues->equipment.rig
                );
            }
            // canopy slot
            else if( message->origin->getParent() == __this->_canopySlot->getPanel() )
            {
                __this->selectEquipment(
                    message->origin,
                    gtCanopy,
                    &__this->_virtues->equipment.canopy
                );
            }
            // droplist slot?
            else if( strcmp( message->origin->getParent()->getName(), "Item01" ) == 0 )
            {
                __this->equipItem( 0 );
            }
            else if( strcmp( message->origin->getParent()->getName(), "Item02" ) == 0 )
            {
                __this->equipItem( 1 );
            }
            else if( strcmp( message->origin->getParent()->getName(), "Item03" ) == 0 )
            {
                __this->equipItem( 2 );
            }
            else if( strcmp( message->origin->getParent()->getName(), "Item04" ) == 0 )
            {
                __this->equipItem( 3 );
            }
            else if( strcmp( message->origin->getParent()->getName(), "Item05" ) == 0 )
            {
                __this->equipItem( 4 );
            }
        }
    }
}

/**
 * momental update
 */

void Equip::showEquipment(void)
{
	// aad selection
	gui::IGuiPanel* aad = _reserveSlot->getPanel()->find("AAD"); assert(aad);
	if (database::Rig::getRecord(_virtues->equipment.rig.id)->skydiving) {
		if (_virtues->equipment.rig.rig_aad < 0 || _virtues->equipment.rig.rig_aad >= database::AAD::getNumRecords()) {
			_virtues->equipment.rig.rig_aad = 0;
		}
		_reserveSlot->getPanel()->setVisible(true);
		// take back the reserve canopy (choose last from stack)
		if (_virtues->equipment.reserve.type == gtUnequipped) {
			Career *career = _scene->getCareer();
			int numGears = career->getNumGears();
			int lastReserveId = -1;
			for (int i = 0; i < numGears; ++i) {
				if (career->getGear(i).type == gtReserve) {
					lastReserveId = i;
				}
			}
			if (lastReserveId != -1) {
				career->equipGear(lastReserveId);
			}
		}
	} else {
		_virtues->equipment.rig.rig_aad = 0;
		_reserveSlot->getPanel()->setVisible(false);
		// put away reserve
		_scene->getCareer()->addGear(_virtues->equipment.reserve);
		_virtues->equipment.reserve.id = 0;
		_virtues->equipment.reserve.type = gtUnequipped;
	}
	unsigned int descid = database::AAD::getRecord(_virtues->equipment.rig.rig_aad)->descriptionId;
	aad->getStaticText()->setText( Gameplay::iLanguage->getUnicodeString(descid) );


    // show HRS equipment
    showHRS( &_virtues->equipment.helmet, _helmetSlot );
    showHRS( &_virtues->equipment.suit, _suitSlot );
    showHRS( &_virtues->equipment.rig, _rigSlot );
	showHRS( &_virtues->equipment.reserve, _reserveSlot );


    // canopy gear
    Gear* gear = &_virtues->equipment.canopy;
    database::Canopy* gearRecord = database::Canopy::getRecord( _virtues->equipment.canopy.id );

    // show canopy
    gui::IGuiPanel* name   = _canopySlot->getPanel()->find( "Name" ); assert( name && name->getButton() );
    gui::IGuiPanel* status = _canopySlot->getPanel()->find( "Status" ); assert( status && status->getStaticText() );
    gui::IGuiPanel* age    = _canopySlot->getPanel()->find( "Age" ); assert( age && age->getStaticText() );
    gui::IGuiPanel* color  = _canopySlot->getPanel()->find( "Color" ); assert( color );
    gui::IGuiPanel* pc     = _canopySlot->getPanel()->find( "PC" ); assert( color );
    gui::IGuiPanel* slider = _canopySlot->getPanel()->find( "Slider" ); assert( slider );

    // update controls
    name->getButton()->setCaption( gear->getName() );
    name->setHint( gear->getDescription() );
    color->setColor( gear->getGearColor() );
    color->setTextureRect( gear->getGearPreview() );
    status->getStaticText()->setText( gear->getStateText() );
    status->getStaticText()->setTextColor( gear->getStateColor() );
    status->setHint( gear->getStateDescription() );
    age->getStaticText()->setText( wstrformat( L"%d", gear->age ).c_str() );
    pc->getStaticText()->setText( wstrformat( L"%d'", int( gearRecord->pilots[_virtues->equipment.pilotchute].size ) ).c_str() );
    switch( _virtues->equipment.sliderOption )
    {
    case ::soUp:
        slider->getStaticText()->setText( Gameplay::iLanguage->getUnicodeString(199) );
        break;
    case ::soDown:
        slider->getStaticText()->setText( Gameplay::iLanguage->getUnicodeString(200) );
        break;
    case ::soRemoved:
        slider->getStaticText()->setText( Gameplay::iLanguage->getUnicodeString(201) );
        break;
    default:
        slider->getStaticText()->setText( L"" );
        break;
    }
}

/**
 * HRS (helmet/rig/suit) slot update
 */

void Equip::showHRS(Gear* gear, gui::IGuiWindow* window)
{
    // slot controls
    gui::IGuiPanel* name   = window->getPanel()->find( "Name" ); assert( name && name->getButton() );
    gui::IGuiPanel* status = window->getPanel()->find( "Status" ); assert( status && status->getStaticText() );
    gui::IGuiPanel* age    = window->getPanel()->find( "Age" ); assert( age && age->getStaticText() );
    gui::IGuiPanel* color  = window->getPanel()->find( "Color" ); assert( color );

    // update controls
    name->getButton()->setCaption( gear->getName() );
    name->setHint( gear->getDescription() );
    color->setColor( gear->getGearColor() );
    color->setTextureRect( gear->getGearPreview() );
    status->getStaticText()->setText( gear->getStateText() );
    status->getStaticText()->setTextColor( gear->getStateColor() );
    status->setHint( gear->getStateDescription() );
    age->getStaticText()->setText( wstrformat( L"%d", gear->age ).c_str() );
}

/**
 * equipment selection
 */

void Equip::selectEquipment(gui::IGuiPanel* panel, GearType type, Gear* slot)
{
    _selectionMode = true;

    Career* career = _scene->getCareer();

    // save slot & type
    _selectionType = type;
    _selectionDst  = slot;

    // show window
    _dropList->getPanel()->setVisible( true );

    // setup window `
    gui::Rect panelRect = panel->getScreenRect();
    gui::Rect oldRect = _dropList->getPanel()->getRect();
    gui::Rect newRect(
        panelRect.left,
        panelRect.top,
        panelRect.left + oldRect.getWidth(),
        panelRect.top + oldRect.getHeight()
    );
    _dropList->getPanel()->setRect( newRect );

    // build list of gears
    _selectionTop = 0;
    _selectionSrc.clear();
    for( unsigned int i=0; i<career->getNumGears(); i++ )
    {
        if( career->getGear(i).type == _selectionType ) _selectionSrc.push_back( i );
    }

    // filter canopies
    if( _selectionType == ::gtCanopy )
    {
        bool rigIsSkydiving = database::Rig::getRecord( _virtues->equipment.rig.id )->skydiving;
        std::vector<unsigned int> filteredSrc;
        for( i=0; i<_selectionSrc.size(); i++ )
        {
            if( rigIsSkydiving == database::Canopy::getRecord( career->getGear(_selectionSrc[i]).id )->skydiving )
            {
                filteredSrc.push_back( _selectionSrc[i] );
            }
        }
        _selectionSrc = filteredSrc;
    }

    gui::IGuiPanel* slider = _dropList->getPanel()->find( "Slider" );
    assert( slider && slider->getSlider() );
    slider->getSlider()->setPosition( 0 );

    // update listbox
    showListbox();
}

void Equip::showListbox(void)
{
    Career* career = _scene->getCareer();

    // check bounds
    if( _selectionTop < 0 ) _selectionTop = 0;
    if( _selectionSrc.size() <= EQGUI_NUMITEMS ) _selectionTop = 0;
    if( _selectionSrc.size() > EQGUI_NUMITEMS && 
        _selectionTop + EQGUI_NUMITEMS > int( _selectionSrc.size() ) )
    {
        _selectionTop = _selectionSrc.size() - EQGUI_NUMITEMS;
    }

    // retrieve slider
    gui::IGuiPanel* slider = _dropList->getPanel()->find( "Slider" );
    assert( slider && slider->getSlider() );

    // update slider
    float lowerLimit = 0.0f;
    float upperLimit = float( _selectionSrc.size() - EQGUI_NUMITEMS );
    if( upperLimit < 0 ) upperLimit = 0;
    slider->getSlider()->setLowerLimit( lowerLimit );
    slider->getSlider()->setUpperLimit( upperLimit );    

    // enable gui items
    for( unsigned int i=0; i<EQGUI_NUMITEMS; i++ )
    {        
        // slot item
        gui::IGuiPanel* item = NULL;
        switch( i )
        {
        case 0: 
            item = _dropList->getPanel()->find( "Item01" );
            break;
        case 1:
            item = _dropList->getPanel()->find( "Item02" );
            break;
        case 2:
            item = _dropList->getPanel()->find( "Item03" );
            break;
        case 3:
            item = _dropList->getPanel()->find( "Item04" );
            break;
        case 4:
            item = _dropList->getPanel()->find( "Item05" );
            break;
        }
        assert( item );

        // item controls
        gui::IGuiPanel* name   = item->find( "Name" ); assert( name && name->getButton() );
        gui::IGuiPanel* status = item->find( "Status" ); assert( status && status->getStaticText() );
        gui::IGuiPanel* age    = item->find( "Age" ); assert( age && age->getStaticText() );
        gui::IGuiPanel* color  = item->find( "Color" ); assert( color );

        bool itemEnabled = ( i < _selectionSrc.size() );
        item->setVisible( itemEnabled );

        if( itemEnabled )
        {
            Gear gear = career->getGear( _selectionSrc[_selectionTop+i] );
            name->getButton()->setCaption( gear.getName() );
            name->setHint( gear.getDescription() );
            color->setColor( gear.getGearColor() );
            color->setTextureRect( gear.getGearPreview() );
            status->getStaticText()->setText( gear.getStateText() );
            status->getStaticText()->setTextColor( gear.getStateColor() );
            status->setHint( gear.getStateDescription() );
            age->getStaticText()->setText( wstrformat( L"%d", gear.age ).c_str() );
         }
    }
}

void Equip::equipItem(unsigned int dropListId)
{
    // reset stow if canopy was equipped
    /*
    if( _selectionType == ::gtCanopy )
    {
        _virtues->equipment.sliderOption = ::soUp;
        _virtues->equipment.pilotchute   = 0;
    }
    */

    // switch canopy for rig
    if( _selectionType == ::gtRig )
    {
        Gear rig = _scene->getCareer()->getGear( _selectionSrc[_selectionTop + dropListId] );
        assert( rig.type == ::gtRig );
        database::Rig* rigInfo = database::Rig::getRecord( rig.id );
        // check canopy type is appropriate
        database::Canopy* canopyInfo = database::Canopy::getRecord( _virtues->equipment.canopy.id );
        if( canopyInfo->skydiving != rigInfo->skydiving )
        {
            // search for appropriate canopy
            bool found = false;
            for( unsigned int i=0; i<_scene->getCareer()->getNumGears(); i++ )
            {
                Gear gear = _scene->getCareer()->getGear( i );
                if( gear.type == ::gtCanopy )
                {
                    canopyInfo = database::Canopy::getRecord( gear.id );
                    if( canopyInfo->skydiving == rigInfo->skydiving )
                    {
                        found = true;
                        break;
                    }
                }
            }
            if( found )
            {
                // equip rig
                _scene->getCareer()->equipGear( _selectionSrc[_selectionTop + dropListId] );
                // equip appropriate canopy
                for( i=0; i<_scene->getCareer()->getNumGears(); i++ )
                {
                    Gear gear = _scene->getCareer()->getGear( i );
                    if( gear.type == ::gtCanopy )
                    {
                        canopyInfo = database::Canopy::getRecord( gear.id );
                        if( canopyInfo->skydiving == rigInfo->skydiving )                            
                        {
                            _scene->getCareer()->equipGear( i );
                            _virtues->equipment.sliderOption = ::soUp;
                            _virtues->equipment.pilotchute   = 0;
                            break;
                        }
                    }
                }
                // update GUI
                _dropList->getPanel()->setVisible( false );
                buildEquipment();
                showEquipment();
                _selectionMode = false;
            }
            else
            {
                // notify user
                // ...
            }
            return;
        }
    }

    // equip gear
    _scene->getCareer()->equipGear( _selectionSrc[_selectionTop + dropListId] );

    // update GUI
    _dropList->getPanel()->setVisible( false );
    buildEquipment();
    showEquipment();
    _selectionMode = false;
}

/**
 * scene builder
 */

void Equip::buildEquipment(void)
{
    if( _baseJumper )
    {        
        _bsp->remove( _baseJumper );
        _jumperRenderCallback->restore( _baseJumper );
        delete _jumperRenderCallback;
        _baseJumper->release();
    }

    if( _canopy )
    {
		for( unsigned int i=0; i<_numCords; i++ ) {
			if( _cords[i] ) {
				delete _cords[i];
				_cords[i] = NULL;
			}
		}
		delete[] _cords;
		_cords = NULL;

		if( _leftBrake ) {
			delete _leftBrake;
			_leftBrake = NULL;
		}
		if( _rightBrake ) {
			delete _rightBrake;
			_rightBrake = NULL;
		}

		if( _cordBatch )
		{
			_bsp->remove( _cordBatch );
			_cordBatch->release();
			//delete _cordBatch;		// must be a bug!!
			_cordBatch = NULL;
		}

        _bsp->remove( _canopy );
        _canopyRenderCallback->restore( _canopy );
        delete _canopyRenderCallback;
		_canopyRenderCallback = NULL;
        _canopy->release();
    }

    // create basejumper
    engine::IClump* templateClump = Gameplay::iGameplay->findClump( "BaseJumper01" );
    assert( templateClump );
    _baseJumper = templateClump->clone( "Maneken" );
    assert( _baseJumper );

    // scale model to obtain desired height
    Matrix4f m = _baseJumper->getFrame()->getMatrix();
    m[0][0] *= 0.01f * _virtues->appearance.height, 
    m[0][1] *= 0.01f * _virtues->appearance.height, 
    m[0][2] *= 0.01f * _virtues->appearance.height,
    m[1][0] *= 0.01f * _virtues->appearance.height, 
    m[1][1] *= 0.01f * _virtues->appearance.height, 
    m[1][2] *= 0.01f * _virtues->appearance.height,
    m[2][0] *= 0.01f * _virtues->appearance.height, 
    m[2][1] *= 0.01f * _virtues->appearance.height, 
    m[2][2] *= 0.01f * _virtues->appearance.height;
    _baseJumper->getFrame()->setMatrix( m );
    _baseJumper->getFrame()->getLTM();

    // hude basejumper effectors
    Jumper::hideEffectors( _baseJumper );

    // setup appearance
    Jumper::setHead( _baseJumper, database::Face::getRecord( _virtues->appearance.face )->modelId, NULL );
    Jumper::setHelmet( _baseJumper, database::Helmet::getRecord( _virtues->equipment.helmet.id )->modelId, NULL );
    Jumper::setBody( _baseJumper, database::Suit::getRecord( _virtues->equipment.suit.id )->modelId, NULL );

    // setup render callback
    _jumperRenderCallback = new JumperRenderCallback;
    _jumperRenderCallback->setFace( _virtues->appearance.face );
    _jumperRenderCallback->setHelmet( _virtues->equipment.helmet.id );
    _jumperRenderCallback->setSuit( _virtues->equipment.suit.id );
    _jumperRenderCallback->setRig( _virtues->equipment.rig.id );
    _jumperRenderCallback->apply( _baseJumper );

    // force LICENSED_CHAR appearance
    if( _scene->getCareer()->getLicensedFlag() )
    {
        Jumper::setHead( _baseJumper, 4, NULL );
        _jumperRenderCallback->setFace( "Felix_Head01_02", "./res/x/textures/Felix_Head01_02.dds" );
        Jumper::getLeftEye( _baseJumper )->setFlags( 0 );
        Jumper::getRightEye( _baseJumper )->setFlags( 0 );
        Jumper::setNoHelmet( _baseJumper );
    }

    // setup animation
    engine::IAnimationController* animCtrl = _baseJumper->getAnimationController();
    animCtrl->setTrackAnimation( 0, &passiveFlightSequence );
    animCtrl->setTrackActivity( 0, true );
    animCtrl->setTrackSpeed( 0, 0.25f );
    animCtrl->setTrackWeight( 0, 1.0f );
    animCtrl->resetTrackTime( 0 );
    animCtrl->advance( 0.0f );
    _baseJumper->getFrame()->getLTM();

    // add base jumper to BSP
    _bsp->add( _baseJumper );

    // create canopy clump
    database::Canopy* canopyInfo = database::Canopy::getRecord( _virtues->equipment.canopy.id );
    engine::IClump* canopyTemplate = Gameplay::iGameplay->findClump( canopyInfo->templateName ); assert( canopyTemplate );
    _canopy = canopyTemplate->clone( "DummyCanopy" );
    _canopy->getAnimationController()->advance( FRAMETIME(38) );

    // scale model
    Vector3f scale = ::calcScale( _canopy->getFrame()->getMatrix() );
    scale *= canopyInfo->scale;
    _canopy->getFrame()->setMatrix( Matrix4f( 
        scale[0], 0,0,0,
        0, scale[1], 0,0,
        0,0, scale[2], 0,
        0,0,0,1
    ) );

    // hide canopy collision
    CanopySimulator::getCollisionGeometry( _canopy )->setFlags( 0 );

    // move canopy up
    _canopy->getFrame()->rotate( _canopy->getFrame()->getRight(), -90.0f );
    _canopy->getFrame()->translate( Vector3f(
        0,
        _virtues->appearance.height + canopyInfo->centralCord * 100.0f,
        0
    ) );
    _canopy->getFrame()->getLTM();

    // setup canopy rendering
    _canopyRenderCallback = new CanopyRenderCallback;
    _canopyRenderCallback->setTexture( canopyInfo );
    _canopyRenderCallback->apply( _canopy );

    // add canopy to BSP
    _bsp->add( _canopy );

    // initialize cords
    if( _numCords && _cords )
    {
        for( unsigned int i=0; i<_numCords; i++ ) if( _cords[i] ) delete _cords[i];
        delete[] _cords;
        if( _leftBrake ) delete _leftBrake;
        if( _rightBrake ) delete _rightBrake;
        _numCords = 0;
        _cords = NULL, _leftBrake = NULL, _rightBrake = NULL;;
    }
    
    initializeCords();
}
