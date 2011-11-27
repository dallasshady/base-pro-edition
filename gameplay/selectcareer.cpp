
#include "headers.h"
#include "selectcareer.h"
#include "gameplay.h"
#include "messagebox.h"
#include "createcareer.h"
#include "deletecareer.h"
#include "geoscape.h"
#include "demo.h"
#include "fatalitylist.h"
#include "version.h"
#include "../common/istring.h"
#include "unicode.h"
#include "version.h"

/**
 * aggregate : Item
 */

SelectCareer::Item::Item()
{
    itemWindow       = NULL;
    itemPicture      = NULL;
    itemCareer       = NULL;
    itemSelectButton = NULL;
    itemDeleteButton = NULL;
    career           = NULL;
}

SelectCareer::Item::Item(
    gui::IGuiWindow* root, 
    const char* windowName,
    const char* pictureName,
    const char* careerName,
    const char* selectButtonName,
    const char* deleteButtonName
)
{
    itemWindow       = root->getPanel()->find( windowName )->getWindow(); assert( itemWindow );
    itemPicture      = itemWindow->getPanel()->find( pictureName )->getWindow(); assert( itemPicture );
    itemCareer       = itemWindow->getPanel()->find( careerName )->getStaticText(); assert( itemCareer );
    itemSelectButton = itemWindow->getPanel()->find( selectButtonName )->getButton(); assert( itemSelectButton );
    itemDeleteButton = itemWindow->getPanel()->find( deleteButtonName )->getButton(); assert( itemDeleteButton );
    career           = NULL;
}

void SelectCareer::Item::enable(bool flag)
{
    itemPicture->getPanel()->setVisible( flag );
    itemCareer->getPanel()->setVisible( flag );
    itemSelectButton->getPanel()->setVisible( flag );
    itemDeleteButton->getPanel()->setVisible( flag );
}

/**
 * class implementation
 */

SelectCareer::SelectCareer()
{
    _endOfActivity = false;

    // create window
    _window = Gameplay::iGui->createWindow( "SelectCareer" ); assert( _window );

    // retrieve hot controls
    _slider = _window->getPanel()->find( "Slider" )->getSlider(); assert( _slider );    

    // initialize item helpers
    _items[0] = Item( _window, "Item01", "Picture01", "Name01", "Select01", "Delete01" );
    _items[1] = Item( _window, "Item02", "Picture02", "Name02", "Select02", "Delete02" );
    _items[2] = Item( _window, "Item03", "Picture03", "Name03", "Select03", "Delete03" );
    _items[3] = Item( _window, "Item04", "Picture04", "Name04", "Select04", "Delete04" );

    // initialize rest stuff
    _topCareer = 0;
}

SelectCareer::~SelectCareer()
{
    // destroy window
    _window->getPanel()->release();
}

/**
 * private behaviour
 */

void SelectCareer::updateGUI(void)
{
    // check bounds
    if( _topCareer < 0 ) _topCareer = 0;
    if( Gameplay::iGameplay->getNumCareers() <= SCGUI_NUM_ITEMS ) _topCareer = 0;
    if( Gameplay::iGameplay->getNumCareers() > SCGUI_NUM_ITEMS && 
        _topCareer + SCGUI_NUM_ITEMS > int( Gameplay::iGameplay->getNumCareers() ) )
    {
        _topCareer = Gameplay::iGameplay->getNumCareers() - SCGUI_NUM_ITEMS;
    }

    // update slider
    float lowerLimit = 0.0f;
    float upperLimit = float( Gameplay::iGameplay->getNumCareers() - SCGUI_NUM_ITEMS );
    if( upperLimit < 0 ) upperLimit = 0;
    _slider->setLowerLimit( lowerLimit );
    _slider->setUpperLimit( upperLimit );

    // enable gui items
    for( unsigned int i=0; i<SCGUI_NUM_ITEMS; i++ )
    {
        bool itemEnabled = ( i < Gameplay::iGameplay->getNumCareers() );
        _items[i].enable( itemEnabled );
        if( itemEnabled )
        {        
            _items[i].career = Gameplay::iGameplay->getCareer( i + _topCareer );
            _items[i].itemCareer->setText( asciizToUnicode( _items[i].career->getName() ).c_str() );
            unsigned int faceId = _items[i].career->getVirtues()->appearance.face;
            // character face icon
            engine::ITexture* faceTexture = Gameplay::iEngine->getTexture( database::Face::getRecord( faceId )->iconName );
            // LICENSED_CHAR?
            if( _items[i].career->getLicensedFlag() ) faceTexture = Gameplay::iEngine->getTexture( "face00" );
            assert( faceTexture );
            _items[i].itemPicture->getPanel()->setTexture( faceTexture );
            _items[i].itemPicture->getPanel()->setTextureRect( gui::Rect(
                0, 0, faceTexture->getWidth(), faceTexture->getHeight()
            ) );
        }
        else
        {
            _items[i].career = NULL;
            _items[i].itemCareer->setText( L"" );
            engine::ITexture* faceTexture = Gameplay::iEngine->getTexture( "black" );
            assert( faceTexture );
            _items[i].itemPicture->getPanel()->setTexture( faceTexture );
            _items[i].itemPicture->getPanel()->setTextureRect( gui::Rect( 0,0,7,7 ) );            
        }
    }
}

void SelectCareer::createCareer(void)
{
    // can't create user career until licensed career is not passed
    #ifdef GAMEPLAY_EDITION_ATARI
        // search for licensed career
        Career* licensedCareer = NULL;
        for( unsigned int i=0; i<Gameplay::iGameplay->getNumCareers(); i++ )
        {
            if( Gameplay::iGameplay->getCareer(i)->getLicensedFlag() )
            {
                licensedCareer = Gameplay::iGameplay->getCareer(i);
                break;
            }
        }
        if( licensedCareer )
        {
            if( licensedCareer->getWalkthroughPercent() < 0.75f )
            {
                #ifndef GAMEPLAY_DEVELOPER_EDITION
                    Gameplay::iGameplay->pushActivity( new Messagebox( Gameplay::iLanguage->getUnicodeString(760) ) );
                    return;
                #endif
            }
        }
    #endif

    // retrieve career name
    gui::IGuiEdit* edit = _window->getPanel()->find( "NewName" )->getEdit(); assert( edit );

    // name is empty?
    if( strcmp( edit->getText(), "" ) == 0 )
    {
        // message box
        Gameplay::iGameplay->pushActivity( new Messagebox( Gameplay::iLanguage->getUnicodeString(56) ) );
    }
    else
    {
        // check same career name
        if( NULL != Gameplay::iGameplay->findCareer( edit->getText() ) )
        {
            // message box
            Gameplay::iGameplay->pushActivity( new Messagebox( Gameplay::iLanguage->getUnicodeString(71) ) );
        }
        else
        {
            // career creation activity
            Career* career = new Career( edit->getText() );
            Gameplay::iGameplay->pushActivity( new CreateCareer( career ) );
        }
    }
}

/**
 * Activity
 */
    
void SelectCareer::updateActivity(float dt)
{
    // begin render
    Gameplay::iEngine->getDefaultCamera()->beginScene( 
        engine::cmClearColor | engine::cmClearDepth,
    	  Vector4f( 0,0,0,0 )
    );

    // render gui
    Gameplay::iGui->render();

    // render version info
    Vector3f screenSize = Gameplay::iEngine->getScreenSize();
    gui::Rect tdrect( 0, int( screenSize[1] ) - 16, 320, int( screenSize[1] ) );
    Gameplay::iGui->renderUnicodeText( tdrect, "hint",  Vector4f(0,0,0,0.5f), gui::atBottom, gui::atLeft, true, ::version.getVersionString() );
    tdrect.left -= 1, tdrect.right -=1,
    tdrect.top -= 1, tdrect.bottom -=1;
    Gameplay::iGui->renderUnicodeText( tdrect, "hint", Vector4f(1,1,1,1),gui::atBottom, gui::atLeft, true, ::version.getVersionString() );

    // present
    Gameplay::iEngine->getDefaultCamera()->endScene();
    Gameplay::iEngine->present();
}

bool SelectCareer::endOfActivity(void)
{
    return _endOfActivity;
}

void SelectCareer::onBecomeActive(void)
{
    // insert window in Gui
    Gameplay::iGui->getDesktop()->insertPanel( _window->getPanel() );
    _window->align( gui::atCenter, 0, gui::atCenter, 0 );

    // register message callback
    Gameplay::iGui->setMessageCallback( messageCallback, this );

    // cleanup
    gui::IGuiEdit* edit = _window->getPanel()->find( "NewName" )->getEdit(); assert( edit );    
    edit->setText( "" );

    // update
    updateGUI();
}

void SelectCareer::onBecomeInactive(void)
{
    // remove window from Gui
    Gameplay::iGui->getDesktop()->removePanel( _window->getPanel() );

    // unregister message callback
    Gameplay::iGui->setMessageCallback( NULL, NULL );
}

/**
 * gui messaging
 */

void SelectCareer::messageCallback(gui::Message* message, void* userData)
{
    SelectCareer* __this = reinterpret_cast<SelectCareer*>( userData );

    if( message->event == gui::onButtonClick )
    {
        // quit button
        if( strcmp( message->origin->getName(), "Quit" ) == 0 )
        {
            __this->_endOfActivity = true;            
        }
        // fatality list button
        else if( strcmp( message->origin->getName(), "FatalityList" ) == 0 )
        {
            Gameplay::iGameplay->pushActivity( new FatalityList() );
        }
        // create button
        else if( strcmp( message->origin->getName(), "Create" ) == 0 )
        {
            __this->createCareer();
        }
        // scroll up
        else if( strcmp( message->origin->getName(), "ScrollUp" ) == 0 )
        {
            __this->_topCareer--;
            __this->_slider->setPosition( float( __this->_topCareer ) );
            __this->updateGUI();
        }
        // scroll down
        else if( strcmp( message->origin->getName(), "ScrollDown" ) == 0 )
        {
            __this->_topCareer++;
            __this->_slider->setPosition( float( __this->_topCareer ) );
            __this->updateGUI();
        }
        // delete items
        else if( strcmp( message->origin->getName(), "Delete01" ) == 0 )
        {
            if( __this->_items[0].career )
            {
                Gameplay::iGameplay->pushActivity( new DeleteCareer( __this->_items[0].career ) );
            }
        }
        else if( strcmp( message->origin->getName(), "Delete02" ) == 0 )
        {
            if( __this->_items[1].career )
            {
                Gameplay::iGameplay->pushActivity( new DeleteCareer( __this->_items[1].career ) );
            }
        }
        else if( strcmp( message->origin->getName(), "Delete03" ) == 0 )
        {
            if( __this->_items[2].career )
            {
                Gameplay::iGameplay->pushActivity( new DeleteCareer( __this->_items[2].career ) );
            }
        }
        else if( strcmp( message->origin->getName(), "Delete04" ) == 0 )
        {
            if( __this->_items[3].career )
            {
                Gameplay::iGameplay->pushActivity( new DeleteCareer( __this->_items[3].career ) );
            }
        }
        // select items
        else if( strcmp( message->origin->getName(), "Select01" ) == 0 )
        {
            if( __this->_items[0].career )
            {
                if( __this->_items[0].career->getVirtues()->evolution.health == 0 )
                {
                    Gameplay::iGameplay->pushActivity( new Messagebox( Gameplay::iLanguage->getUnicodeString( 764 ) ) );
                }
                else
                {
                    #ifdef GAMEPLAY_DEMOVERSION
                        Gameplay::iGameplay->pushActivity( new Demo( __this->_items[0].career ) );
                    #else
                        Gameplay::iGameplay->pushActivity( new Geoscape( __this->_items[0].career ) );
                    #endif
                }
            }
        }
        else if( strcmp( message->origin->getName(), "Select02" ) == 0 )
        {
            if( __this->_items[1].career )
            {
                if( __this->_items[1].career->getVirtues()->evolution.health == 0 )
                {
                    Gameplay::iGameplay->pushActivity( new Messagebox( Gameplay::iLanguage->getUnicodeString( 764 ) ) );
                }
                else
                {
                    #ifdef GAMEPLAY_DEMOVERSION
                        Gameplay::iGameplay->pushActivity( new Demo( __this->_items[1].career ) );
                    #else
                        Gameplay::iGameplay->pushActivity( new Geoscape( __this->_items[1].career ) );
                    #endif
                }
            }
        }
        else if( strcmp( message->origin->getName(), "Select03" ) == 0 )
        {
            if( __this->_items[2].career )
            {
                if( __this->_items[2].career->getVirtues()->evolution.health == 0 )
                {
                    Gameplay::iGameplay->pushActivity( new Messagebox( Gameplay::iLanguage->getUnicodeString( 764 ) ) );
                }
                else
                {
                    #ifdef GAMEPLAY_DEMOVERSION
                        Gameplay::iGameplay->pushActivity( new Demo( __this->_items[2].career ) );
                    #else
                        Gameplay::iGameplay->pushActivity( new Geoscape( __this->_items[2].career ) );
                    #endif
                }
            }
        }
        else if( strcmp( message->origin->getName(), "Select04" ) == 0 )
        {
            if( __this->_items[3].career )
            {
                if( __this->_items[3].career->getVirtues()->evolution.health == 0 )
                {
                    Gameplay::iGameplay->pushActivity( new Messagebox( Gameplay::iLanguage->getUnicodeString( 764 ) ) );
                }
                else
                {
                    #ifdef GAMEPLAY_DEMOVERSION
                        Gameplay::iGameplay->pushActivity( new Demo( __this->_items[3].career ) );
                    #else
                        Gameplay::iGameplay->pushActivity( new Geoscape( __this->_items[3].career ) );
                    #endif
                }
            }
        }
    }
    else if( message->event == gui::onSlide )
    {
        __this->_topCareer = unsigned int( __this->_slider->getPosition() );
        __this->updateGUI();
    }
    else if( message->event == gui::onMouseWheel )
    {
        __this->_topCareer += int( message->mouseX );
        __this->_slider->setPosition( float( __this->_topCareer ) );
        __this->updateGUI();
    }
}