
#include "headers.h"
#include "canopy.h"

CanopySimulator::WLOToggles::WLOToggles(bool gui)
{
    _trigger = false;
    _weight  = 0.0f;
    _window  = NULL;
    if( gui )
    {
        // create window
        _window = Gameplay::iGui->createWindow( "RiggingWindow" );
        _window->getPanel()->setName( WLO_WINDOW_NAME );
        Gameplay::iGui->getDesktop()->insertPanel( _window->getPanel() );

        // determine left corner of window
        gui::Rect rect( 0,0,0,0 );        
        gui::IGuiPanel* panel = Gameplay::iGui->getDesktop()->find( "HealthStatus" );
        if( panel && rect.left < panel->getRect().right ) rect.left = panel->getRect().right;
        panel = Gameplay::iGui->getDesktop()->find( HOOK_KNIFE_WINDOW_NAME );
        if( panel && rect.left < panel->getRect().right ) rect.left = panel->getRect().right;
        rect.left += 8;
        // determine bottom corner of window
        Vector3f screenSize = Gameplay::iEngine->getScreenSize();        
        rect.bottom = int( screenSize[1] ) - 8;
        // determine whole rectangle
        rect.right = rect.left + _window->getPanel()->getRect().getWidth();
        rect.top   = rect.bottom - _window->getPanel()->getRect().getHeight();
        _window->getPanel()->setRect( rect );

        // setup icon texture
        engine::ITexture* icon = Gameplay::iEngine->getTexture( "rig_wlo" ); assert( icon );
        panel = _window->getPanel()->find( "IconShadow" ); assert( panel );
        panel->setTexture( icon );
        panel = _window->getPanel()->find( "Icon" ); assert( panel );
        panel->setTexture( icon );

        // setup hotkey text
        ActionChannel* wloChannel = Gameplay::iGameplay->getActionChannel( ::iaWLO ); assert( wloChannel );
        panel = _window->getPanel()->find( "HotkeyShadow" ); assert( panel && panel->getStaticText() );
        panel->getStaticText()->setText( wloChannel->getInputActionHint() );
        panel = _window->getPanel()->find( "Hotkey" ); assert( panel && panel->getStaticText() );
        panel->getStaticText()->setText( wloChannel->getInputActionHint() );

        // setup action text
        panel = _window->getPanel()->find( "TextShadow" ); assert( panel && panel->getStaticText() );
        panel->getStaticText()->setText( Gameplay::iLanguage->getUnicodeString(372) );
        panel = _window->getPanel()->find( "Text" ); assert( panel && panel->getStaticText() );
        panel->getStaticText()->setText( Gameplay::iLanguage->getUnicodeString(372) );
    }
   
    simulate( 0.0f, NULL );
}

CanopySimulator::WLOToggles::~WLOToggles()
{
    if( _window ) _window->getPanel()->release();
}

void CanopySimulator::WLOToggles::setTrigger(bool trigger)
{
    _trigger = trigger;
}

void CanopySimulator::WLOToggles::simulate(float dt, Virtues* virtues)
{
    if( virtues )
    {
        // determine action process dynamics
        float minRigging   = 0.0f;
        float minSpeed     = 0.25f;
        float maxRigging   = 1.0f;
        float maxSpeed     = 0.75f;
        float reverseSpeed = 2.0f;
    
        if( _trigger && _weight < 1 )
        {
            // simulate action
            float factor = ( virtues->getRiggingSkill() - minRigging ) / ( maxRigging - minRigging );
            factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );
            float speed = minSpeed * ( 1 - factor ) + maxSpeed * factor;
            _weight += speed * dt;
            if( _weight > 1 ) _weight = 1;
            // update rigging skill
            if( virtues->equipment.experience )
            {
                virtues->skills.rigging += dt * virtues->predisp.rigging;
            }
        }
        else
        {
            // simulate reverse action
            _weight -= reverseSpeed * dt;
            if( _weight < 0 ) _weight = 0;
        }
    }

    // update weight level
    if( _window )
    {
        gui::IGuiPanel* vessel = _window->getPanel()->find( "WeightVessel" ); assert( vessel );
        gui::IGuiPanel* level  = vessel->find( "WeightLevel" ); assert( level );
        level->setRect( gui::Rect( 0, int( 63.0f * ( 1.0f - _weight ) ), 7, 63 ) );
        level->setTextureRect( gui::Rect( 0, int( 128.0f * ( 1.0f - _weight ) ), 16, 128 ) );
    }
}

void CanopySimulator::WLOToggles::hideGui(void)
{
    if( _window ) _window->getPanel()->setVisible( false );
}