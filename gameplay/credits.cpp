
#include "headers.h"
#include "credits.h"
#include "gameplay.h"
#include "version.h"
#include "selectcareer.h"
#include "../common/istring.h"

/**
 * credits section
 */

Credits::Section::Section(TiXmlElement* element, const Vector3f& screenSize)
{
    // load text fields
    int left = 0;
    int right = 0;
    element->Attribute( "left", &left );
    element->Attribute( "right", &right );
    if( left < 0 ) left = 0;
    if( unsigned int( left ) > Gameplay::iLanguage->getNumStrings() ) left = 0;
    if( right < 0 ) right = 0;
    if( unsigned int( right ) > Gameplay::iLanguage->getNumStrings() ) right = 0;
    _leftText  = Gameplay::iLanguage->getUnicodeString( left );
    _rightText = Gameplay::iLanguage->getUnicodeString( right );

    // determine size of text fields
    gui::Rect maxLeftRect( 0, 0, int( screenSize[0] / 2  - 16 ), int( screenSize[1] ) );
    gui::Rect maxRightRect( int( screenSize[0] / 2  ), 0, int( screenSize[0] ), int( screenSize[1] ) );
    
    _leftRect = Gameplay::iGui->calcUnicodeTextRect( 
        maxLeftRect,
        "caption",
        gui::atTop, gui::atRight, true, 
        _leftText
    );

    _rightRect = Gameplay::iGui->calcUnicodeTextRect( 
        maxRightRect,
        "caption",
        gui::atTop, gui::atLeft, true, 
        _rightText
    );

    int rightWidth = _rightRect.getWidth();
    _rightRect.left = int( screenSize[0] / 2 );
    _rightRect.right = _rightRect.left + rightWidth;

    int maxHeight = _leftRect.bottom;
    if( maxHeight < _rightRect.bottom ) maxHeight = _rightRect.bottom;

    _leftRect.bottom  = maxHeight;
    _rightRect.bottom = maxHeight;
}

Credits::Section::~Section()
{
}

void Credits::Section::render(void)
{
    Gameplay::iGui->renderUnicodeText(
        _leftRect, 
        "caption", 
        Vector4f( 1,1,1,1 ),
        gui::atTop, gui::atRight, true, 
        _leftText
    );

    Gameplay::iGui->renderUnicodeText(
        _rightRect, 
        "caption", 
        Vector4f( 1,1,1,1 ),
        gui::atTop, gui::atLeft, true, 
        _rightText
    );
}

/**
 * credits activity
 */

Credits::Credits()
{
    _activityMode = false;
    _scrollTimer  = 0.0f;

    // determine screen size
    Vector3f screenSize = Gameplay::iEngine->getScreenSize();

    // open credits
    TiXmlDocument* credits = new TiXmlDocument( "./res/gui/credits.xml" );
    credits->LoadFile();

    // load sections
    TiXmlNode* child = credits->FirstChild();
    if( child ) do 
    {
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "section" ) == 0 )
        {
            _sections.push_back( new Section( static_cast<TiXmlElement*>( child ), screenSize ) );
        }
        child = child->NextSibling();
    }
    while( child != NULL );

    arrangeSections();
}

Credits::~Credits()
{
    for( unsigned int i=0; i<_sections.size(); i++ ) delete _sections[i];
}

void Credits::updateActivity(float dt)
{
    if( !_activityMode )
    {
        // start career selection
        Gameplay::iGameplay->pushActivity( new SelectCareer() );
        // switch mode
        _activityMode = true;
        return;
    }

    // ESC - interrupt activity
    if( Gameplay::iGameplay->getKeyboardState()->keyState[0x01] & 0x80 )
    {
        getCore()->exit( 0 );
    }

    // scroll sections
    if( dt > 0 )
    {
        _scrollTimer += dt;
    }
    else
    {
        Sleep( 1 );
        _scrollTimer += 0.001f;
    }
    if( _scrollTimer > 0.05f )
    {
        Vector3f screenSize = Gameplay::iEngine->getScreenSize();
        int scroll = int( screenSize[1] / 20 * _scrollTimer );
        for( unsigned int i=0; i<_sections.size(); i++ )
        {
            _sections[i]->getLeftRect()->top     -= scroll;
            _sections[i]->getLeftRect()->bottom  -= scroll;
            _sections[i]->getRightRect()->top    -= scroll;
            _sections[i]->getRightRect()->bottom -= scroll;
        }
        _scrollTimer = 0.0f;
    }

    // render gui
    Gameplay::iEngine->getDefaultCamera()->beginScene( 
        engine::cmClearColor | engine::cmClearDepth,
    	  Vector4f( 0,0,0,0 )
    );
    // Gameplay::iGui->render();
    for( unsigned int i=0; i<_sections.size(); i++ ) _sections[i]->render();
    Gameplay::iEngine->getDefaultCamera()->endScene();
    Gameplay::iEngine->present();
}

bool Credits::endOfActivity(void)
{
    return false;
}

void Credits::onBecomeActive(void)
{
}

void Credits::onBecomeInactive(void)
{
}

void Credits::arrangeSections(void)
{
    // determine screen size
    Vector3f screenSize = Gameplay::iEngine->getScreenSize();

    // stepping
    int stepping = 16;

    // start arrange
    int currPos = int( screenSize[1] );
    for( unsigned int i=0; i<_sections.size(); i++ )
    {
        int height = _sections[i]->getLeftRect()->getHeight();
        _sections[i]->getLeftRect()->top     = currPos;
        _sections[i]->getLeftRect()->bottom  = currPos + height;
        _sections[i]->getRightRect()->top    = currPos;
        _sections[i]->getRightRect()->bottom = currPos + height;
        currPos += height + stepping;
    }
}