
#include "headers.h"
#include "createcareer.h"
#include "gameplay.h"
#include "messagebox.h"
#include "database.h"
#include "version.h"
#include "../common/istring.h"

CreateCareer::CreateCareer(Career* career)
{
    _endOfActivity = false;

    _career = career;

    // create window
    _window = Gameplay::iGui->createWindow( "CreateCareer" ); assert( _window );

    // retrieve hot controls
    _heightBox       = _window->getPanel()->find( "Height" )->getEdit(); assert( _heightBox );
    _weightBox       = _window->getPanel()->find( "Weight" )->getEdit(); assert( _weightBox );
    _facePicture     = _window->getPanel()->find( "FacePicture" )->getWindow(); assert( _facePicture );
    _faceDescription = _window->getPanel()->find( "FaceDescription" )->getStaticText(); assert( _faceDescription );

    // setup editboxes
    _heightBox->setText( strformat( "%4.2f", _career->getVirtues()->appearance.height ).c_str() );
    _weightBox->setText( strformat( "%4.2f", _career->getVirtues()->appearance.weight ).c_str() );

    // retrieve slider controls
    _predPool = _window->getPanel()->find( "PredPool" ); assert( _predPool && _predPool->getSlider() );
    _predPerception = _window->getPanel()->find( "PredPerception" ); assert( _predPerception && _predPerception->getSlider() );
    _predEndurance = _window->getPanel()->find( "PredEndurance" ); assert( _predEndurance && _predEndurance->getSlider() );
    _predTracking = _window->getPanel()->find( "PredTracking" ); assert( _predTracking && _predTracking->getSlider() );
    _predRigging = _window->getPanel()->find( "PredRigging" ); assert( _predRigging && _predRigging->getSlider() );
    _malfunctions = _window->getPanel()->find( "MalfunctionCheckBox" ); assert( _malfunctions && _malfunctions->getButton() );

    // setup predisposition
    _predPool->getSlider()->setLowerLimit( 0.0f );
    _predPool->getSlider()->setUpperLimit( 2.0f );
    _predPool->getSlider()->setPosition( 0.0f );

    _predPerception->getSlider()->setLowerLimit( 0.0f );
    _predPerception->getSlider()->setUpperLimit( 1.0f );
    _predPerception->getSlider()->setPosition( 0.5f );

    _predEndurance->getSlider()->setLowerLimit( 0.0f );
    _predEndurance->getSlider()->setUpperLimit( 1.0f );
    _predEndurance->getSlider()->setPosition( 0.5f );

    _predTracking->getSlider()->setLowerLimit( 0.0f );
    _predTracking->getSlider()->setUpperLimit( 1.0f );
    _predTracking->getSlider()->setPosition( 0.5f );

    _predRigging->getSlider()->setLowerLimit( 0.0f );
    _predRigging->getSlider()->setUpperLimit( 1.0f );
    _predRigging->getSlider()->setPosition( 0.5f );

    updateFaceGui();
}

CreateCareer::~CreateCareer()
{
    // destroy window
    _window->getPanel()->release();
}

void CreateCareer::updateActivity(float dt)
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

bool CreateCareer::endOfActivity(void)
{
    return _endOfActivity;
}

void CreateCareer::onBecomeActive(void)
{
    // insert window in Gui
    Gameplay::iGui->getDesktop()->insertPanel( _window->getPanel() );
    _window->align( gui::atCenter, 0, gui::atCenter, 0 );

    // register message callback
    Gameplay::iGui->setMessageCallback( messageCallback, this );
}

void CreateCareer::onBecomeInactive(void)
{
    // remove window from Gui
    Gameplay::iGui->getDesktop()->removePanel( _window->getPanel() );

    // unregister message callback
    Gameplay::iGui->setMessageCallback( NULL, NULL );
}

/**
 * gui message handling
 */

void CreateCareer::messageCallback(gui::Message* message, void* userData)
{
    CreateCareer* __this = reinterpret_cast<CreateCareer*>( userData );
        
    if( message->event == gui::onButtonClick )
    {
        if( strcmp( message->origin->getName(), "ContinueButton" ) == 0 )
        {
            bool correctionApplied = false;
            // read height
            float value = float( atof( __this->_heightBox->getText() ) );
            if( value < 170 ) value = 170, correctionApplied = true;
            if( value > 190 ) value = 190, correctionApplied = true;
            __this->_career->getVirtues()->appearance.height = value;
            if( correctionApplied )
            {
                __this->_heightBox->setText( strformat( "%4.2f", __this->_career->getVirtues()->appearance.height ).c_str() );
                Gameplay::iGameplay->pushActivity( new Messagebox( 
                    Gameplay::iLanguage->getUnicodeString(69)
                ) );
                return;
            }
            // read weight
            value = float( atof( __this->_weightBox->getText() ) );
            if( value < 50 ) value = 50, correctionApplied = true;
            if( value > 110 ) value = 110, correctionApplied = true;
            __this->_career->getVirtues()->appearance.weight = value;
            if( correctionApplied )
            {
                __this->_weightBox->setText( strformat( "%4.2f", __this->_career->getVirtues()->appearance.weight ).c_str() );
                Gameplay::iGameplay->pushActivity( new Messagebox( 
                    Gameplay::iLanguage->getUnicodeString(70)
                ) );
                return;
            }
            // read predispositions
            __this->_career->getVirtues()->predisp.perception = __this->_predPerception->getSlider()->getPosition();
            __this->_career->getVirtues()->predisp.endurance = __this->_predEndurance->getSlider()->getPosition();
            __this->_career->getVirtues()->predisp.tracking = __this->_predTracking->getSlider()->getPosition();
            __this->_career->getVirtues()->predisp.rigging = __this->_predRigging->getSlider()->getPosition();
            // startup skills            
            #ifdef GAMEPLAY_DEMOVERSION
                // rande-up predispositions (due to restrictions of skill nature)
                if( __this->_career->getVirtues()->predisp.perception > 0.99f ) __this->_career->getVirtues()->predisp.perception = 0.99f;
                if( __this->_career->getVirtues()->predisp.endurance > 0.99f ) __this->_career->getVirtues()->predisp.endurance = 0.99f;
                if( __this->_career->getVirtues()->predisp.tracking > 0.99f ) __this->_career->getVirtues()->predisp.tracking = 0.99f;
                if( __this->_career->getVirtues()->predisp.rigging > 0.99f ) __this->_career->getVirtues()->predisp.rigging = 0.99f;
                // setup skills for demo mode
                __this->_career->getVirtues()->setPerceptionSkill( __this->_career->getVirtues()->predisp.perception );
                __this->_career->getVirtues()->setEnduranceSkill( __this->_career->getVirtues()->predisp.endurance );
                __this->_career->getVirtues()->setTrackingSkill( __this->_career->getVirtues()->predisp.tracking );
                __this->_career->getVirtues()->setRiggingSkill( __this->_career->getVirtues()->predisp.rigging );
            #else
                __this->_career->getVirtues()->skills.perception = 0;
                __this->_career->getVirtues()->skills.endurance = 0;
                __this->_career->getVirtues()->skills.tracking = 0;
                __this->_career->getVirtues()->skills.rigging = 0;
            #endif
            // read malfunctions
            __this->_career->getVirtues()->equipment.malfunctions = ( __this->_malfunctions->getTextureRect().left > 0 );
            // generate startup gears
            __this->generateStartupGearStock();
            // decision to finalize
            Gameplay::iGameplay->addCareer( __this->_career );
            Gameplay::iGameplay->saveCareers();
            __this->_endOfActivity = true;
        }
        else if( strcmp( message->origin->getName(), "CancelButton" ) == 0 )
        {
            __this->_endOfActivity = true;
        }
        else if( strcmp( message->origin->getName(), "MalfunctionCheckBox" ) == 0 )
        {
            if( message->origin->getTextureRect().left > 0 )
            {
                message->origin->setTextureRect( gui::Rect( 0,0,32,32 ) );
            }
            else
            {
                message->origin->setTextureRect( gui::Rect( 32,0,64,32 ) );
            }
        }
        else if( strcmp( message->origin->getName(), "PrevFace" ) == 0 )
        {
            if( __this->_career->getVirtues()->appearance.face == 0 )
            {
                __this->_career->getVirtues()->appearance.face = ( database::Face::getNumRecords() - 1 );
            }
            else
            {
                __this->_career->getVirtues()->appearance.face--;
            }
            __this->updateFaceGui();
        }
        else if( strcmp( message->origin->getName(), "NextFace" ) == 0 )
        {
            if( __this->_career->getVirtues()->appearance.face == database::Face::getNumRecords() - 1 )
            {
                __this->_career->getVirtues()->appearance.face = 0;
            }
            else
            {
                __this->_career->getVirtues()->appearance.face++;
            }
            __this->updateFaceGui();
        }
    }
    else if( message->event == gui::onSlide )
    {
        if( message->origin == __this->_predPool )
        {
            float distributed = __this->_predPerception->getSlider()->getPosition() +
                                __this->_predEndurance->getSlider()->getPosition() +
                                __this->_predTracking->getSlider()->getPosition() +
                                __this->_predRigging->getSlider()->getPosition();
            __this->_predPool->getSlider()->setPosition( 2.0f - distributed );
        }
        else
        {
            __this->updatePredisposition( message->origin );
        }
    }
}

/**
 * private behaviour
 */

void CreateCareer::updateFaceGui(void)
{
    // retrieve face db record
    database::Face* face = database::Face::getRecord( _career->getVirtues()->appearance.face );
    
    // setup face texture
    engine::ITexture* faceTexture = Gameplay::iEngine->getTexture( face->iconName ); assert( faceTexture );
    _facePicture->getPanel()->setTexture( faceTexture );
    _facePicture->getPanel()->setTextureRect( gui::Rect( 0,0, faceTexture->getWidth()-1, faceTexture->getHeight()-1 ) );

    // setup face description
    assert( face->descriptionId < Gameplay::iLanguage->getNumStrings() );
    _faceDescription->setText( Gameplay::iLanguage->getUnicodeString(face->descriptionId) );
}

void CreateCareer::updatePredisposition(gui::IGuiPanel* origin)
{
    assert( origin->getSlider() );

    float distributed = _predPerception->getSlider()->getPosition() +
                        _predEndurance->getSlider()->getPosition() +
                        _predTracking->getSlider()->getPosition() +
                        _predRigging->getSlider()->getPosition();
    float undistributed = _predPool->getSlider()->getPosition();

    if( distributed + undistributed > 2.0f )
    {
        float taken = distributed + undistributed - 2.0f;
        _predPool->getSlider()->setPosition( 
            _predPool->getSlider()->getPosition() - taken
        );
    }
    else if( distributed + undistributed < 2.0f )
    {
        float given = 2.0f - ( distributed + undistributed );
        _predPool->getSlider()->setPosition( 
            _predPool->getSlider()->getPosition() + given
        );
    }

    // finally, check predispositions are distributed correctly
    distributed = _predPerception->getSlider()->getPosition() +
                  _predEndurance->getSlider()->getPosition() +
                  _predTracking->getSlider()->getPosition() +
                  _predRigging->getSlider()->getPosition();   
    undistributed = _predPool->getSlider()->getPosition();
    if( distributed + undistributed > 2.0f )
    {
        float overflow = distributed + undistributed - 2.0f;
        origin->getSlider()->setPosition( origin->getSlider()->getPosition() - overflow );
    }
}

void CreateCareer::generateStartupGearStock(void)
{
    // generate BASE canopies

    #ifdef GAMEPLAY_EDITION_ATARI
        // define selection source (all u-turn morpheus)
        unsigned int numVariants = 9;
        unsigned int variants[] = { 107, 108, 109, 110, 111, 112, 113, 114, 115 };
    #else
        // define selection source (all purple psychonauts)
        unsigned int numVariants = 10;
        unsigned int variants[] = { 0,1,2,3,4,5,6,7,8,9 };
    #endif

    // determine optimal canopy square (size)
    float optimalSquare = database::Canopy::getOptimalCanopySquare( 
        _career->getVirtues()->appearance.weight, 0.0f
    );

    // choose optimal canopy
    unsigned int variantId = 0;
    float epsilon = fabs( database::Canopy::getRecord( variants[variantId] )->square - optimalSquare );
    for( unsigned int i=1; i<numVariants; i++ )
    {
        float extraEpsilon = fabs( database::Canopy::getRecord( variants[i] )->square - optimalSquare );
        if( extraEpsilon < epsilon )
        {
            epsilon = extraEpsilon;
            variantId = i;
        }
    }

    // equip this canopy
    _career->getVirtues()->equipment.canopy = Gear( gtCanopy, variants[variantId] );

    // determine optimal canopy square (size) for windy weather
    optimalSquare = database::Canopy::getOptimalCanopySquare( 
        _career->getVirtues()->appearance.weight, 9.0f
    );

    // choose optimal canopy
    variantId = 0;
    epsilon = fabs( database::Canopy::getRecord( variants[variantId] )->square - optimalSquare );
    for( i=1; i<numVariants; i++ )
    {
        float extraEpsilon = fabs( database::Canopy::getRecord( variants[i] )->square - optimalSquare );
        if( extraEpsilon < epsilon )
        {
            epsilon = extraEpsilon;
            variantId = i;
        }
    }

    // add this canopy in to gear stock
    _career->addGear( Gear( gtCanopy, variants[variantId] ) );

    // select skydiving canopy and add to gear stock
    if( _career->getVirtues()->appearance.weight > 75.0f )
    {
        #ifdef GAMEPLAY_EDITION_ATARI
            // G-Force 250
            _career->addGear( Gear( ::gtCanopy, 121 ) );
        #else
            // Haibane 250
            _career->addGear( Gear( ::gtCanopy, 19 ) );
        #endif
    }
    else
    {
        #ifdef GAMEPLAY_EDITION_ATARI
            // G-Force 220
            _career->addGear( Gear( ::gtCanopy, 120 ) );
        #else
            // Haibane 220
            _career->addGear( Gear( ::gtCanopy, 18 ) );
        #endif
    }

    // select skydiving rig (Harpy) and add to gear stock
    _career->addGear( Gear( ::gtRig, 17 ) );

    // in demo mode - include wingsuit 
    #ifdef GAMEPLAY_DEMOVERSION
        _career->addGear( Gear( ::gtSuit, 21 ) );
    #endif

    // in demo mode - enable acrobatics
    #ifdef GAMEPLAY_DEMOVERSION
        _career->setAcrobaticsSkill( acroJumpFromRun, true );
        _career->setAcrobaticsSkill( acroFrontFlip, true );
        _career->setAcrobaticsSkill( acroFrontBackFlip, true );
        _career->setAcrobaticsSkill( acroBackFlip, true );
    #endif
}