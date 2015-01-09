
#include "headers.h"
#include "hud.h"
#include "jumper.h"
#include "../common/istring.h"

/**
 * actor abstracts
 */
 
void HealthStatus::onUpdateActivity(float dt)
{
    Jumper* jumper = dynamic_cast<Jumper*>( _parent ); assert( jumper );

    // update pulse
    std::wstring text = wstrformat( L"%2.0f", jumper->getPulse() );
    gui::IGuiPanel* panel = _healthWindow->getPanel()->find( "TonometerShadow" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( text.c_str() );
    panel = _healthWindow->getPanel()->find( "Tonometer" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( text.c_str() );

    // update adrenaline level
    float adrenaline = jumper->getAdrenaline();
    gui::IGuiPanel* vessel = _healthWindow->getPanel()->find( "AdrenalineVessel" ); assert( vessel );
    gui::IGuiPanel* level  = vessel->find( "AdrenalineLevel" ); assert( level );
    gui::Rect vesselRect = vessel->getRect();
    gui::Rect levelRect;
    levelRect.left   = 0;
    levelRect.right  = vesselRect.getWidth();
    levelRect.bottom = vesselRect.getHeight();
    levelRect.top    = int( ( 1.0f - adrenaline ) * vesselRect.getHeight() );
    level->setRect( levelRect );

    // update shock level
    float shock = jumper->getShock();
    vessel = _healthWindow->getPanel()->find( "ShockVessel" ); assert( vessel );
    level  = vessel->find( "ShockLevel" ); assert( level );
    vesselRect = vessel->getRect();    
    levelRect.left   = 0;
    levelRect.right  = vesselRect.getWidth();
    levelRect.bottom = vesselRect.getHeight();
    levelRect.top    = int( ( 1.0f - shock ) * vesselRect.getHeight() );
    level->setRect( levelRect );

    // update health level
    float health = jumper->getVirtues()->evolution.health;
    vessel = _healthWindow->getPanel()->find( "HealthVessel" ); assert( vessel );
    level  = vessel->find( "HealthLevel" ); assert( level );
    vesselRect = vessel->getRect();
    levelRect.left   = 0;
    levelRect.right  = vesselRect.getWidth();
    levelRect.bottom = vesselRect.getHeight();
    levelRect.top    = int( ( 1.0f - health ) * vesselRect.getHeight() );
    level->setRect( levelRect );

    // update perception vessel
    vessel = _skillsWindow->getPanel()->find( "PerceptionVessel" ); assert( vessel );
    level  = vessel->find( "PerceptionLevel" ); assert( level );
    vesselRect = vessel->getRect();
    levelRect.left   = 0;
    levelRect.right  = int( vesselRect.getWidth() * jumper->getVirtues()->getPerceptionSkill() );
    levelRect.top    = 0;
    levelRect.bottom = 15;    
    level->setRect( levelRect );

    // update endurance vessel
    vessel = _skillsWindow->getPanel()->find( "EnduranceVessel" ); assert( vessel );
    level  = vessel->find( "EnduranceLevel" ); assert( level );
    vesselRect = vessel->getRect();
    levelRect.left   = 0;
    levelRect.right  = int( vesselRect.getWidth() * jumper->getVirtues()->getEnduranceSkill() );
    levelRect.top    = 0;
    levelRect.bottom = 15;    
    level->setRect( levelRect );

    // update tracking vessel
    vessel = _skillsWindow->getPanel()->find( "TrackingVessel" ); assert( vessel );
    level  = vessel->find( "TrackingLevel" ); assert( level );
    vesselRect = vessel->getRect();
    levelRect.left   = 0;
    levelRect.right  = int( vesselRect.getWidth() * jumper->getVirtues()->getTrackingSkill() );
    levelRect.top    = 0;
    levelRect.bottom = 15;    
    level->setRect( levelRect );

    // update rigging vessel
    vessel = _skillsWindow->getPanel()->find( "RiggingVessel" ); assert( vessel );
    level  = vessel->find( "RiggingLevel" ); assert( level );
    vesselRect = vessel->getRect();
    levelRect.left   = 0;
    levelRect.right  = int( vesselRect.getWidth() * jumper->getVirtues()->getRiggingSkill() );
    levelRect.top    = 0;
    levelRect.bottom = 15;    
    level->setRect( levelRect );

    // update skill evolution values
    text = wstrformat( L"%2.0f", jumper->getVirtues()->skills.perception );
    panel = _skillsWindow->getPanel()->find( "PerceptionValue" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( text.c_str() );

    text = wstrformat( L"%2.0f", jumper->getVirtues()->skills.endurance );
    panel = _skillsWindow->getPanel()->find( "EnduranceValue" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( text.c_str() );

    text = wstrformat( L"%2.0f", jumper->getVirtues()->skills.tracking );
    panel = _skillsWindow->getPanel()->find( "TrackingValue" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( text.c_str() );

    text = wstrformat( L"%2.0f", jumper->getVirtues()->skills.rigging );
    panel = _skillsWindow->getPanel()->find( "RiggingValue" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( text.c_str() );
}

void HealthStatus::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
    if( eventId == EVENT_HUD_HIDE )
    {
        if( _healthWindow->getPanel()->getParent() )
        {
            _healthWindow->getPanel()->getParent()->removePanel( _healthWindow->getPanel() );
        }
    }
}

/**
 * class implemetation
 */

HealthStatus::HealthStatus(Actor* parent) : Actor( parent )
{
    Jumper* jumper = dynamic_cast<Jumper*>( parent ); assert( jumper );

    _healthWindow = Gameplay::iGui->createWindow( "HealthStatus" ); assert( _healthWindow );
    Gameplay::iGui->getDesktop()->insertPanel( _healthWindow->getPanel() );
    _healthWindow->align( gui::atBottom, 0, gui::atLeft, 0 );

    _skillsWindow = Gameplay::iGui->createWindow( "SkillStatus" ); assert( _skillsWindow );
    //Gameplay::iGui->getDesktop()->insertPanel( _skillsWindow->getPanel() );    
    gui::Rect oldRect = _skillsWindow->getPanel()->getRect();		
    gui::Rect newRect;
    newRect.left   = _healthWindow->getPanel()->getRect().right;
    newRect.top    = _healthWindow->getPanel()->getRect().bottom - oldRect.getHeight();
    newRect.right  = newRect.left + oldRect.getWidth();
    newRect.bottom = newRect.top + oldRect.getHeight();
    _skillsWindow->getPanel()->setRect( newRect );
}

HealthStatus::~HealthStatus()
{
    Gameplay::iGui->getDesktop()->removePanel( _healthWindow->getPanel() );
    Gameplay::iGui->getDesktop()->removePanel( _skillsWindow->getPanel() );
    _healthWindow->getPanel()->release();
    _skillsWindow->getPanel()->release();
}