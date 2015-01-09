
#include "headers.h"
#include "hud.h"

/**
 * common HUD methods
 */

void HUD::resetIndicator(gui::IGuiPanel* panel)
{
    assert( panel );
    panel->setTextureRect( ::hud_digit_reset );
}

void HUD::setIndicator(gui::IGuiPanel* panel, unsigned int digit)
{
    assert( panel );
    switch( digit )
    {
    case 0:
        panel->setTextureRect( ::hud_digit_0 );
        break;
    case 1:
        panel->setTextureRect( ::hud_digit_1 );
        break;
    case 2:
        panel->setTextureRect( ::hud_digit_2 );
        break;
    case 3:
        panel->setTextureRect( ::hud_digit_3 );
        break;
    case 4:
        panel->setTextureRect( ::hud_digit_4 );
        break;
    case 5:
        panel->setTextureRect( ::hud_digit_5 );
        break;
    case 6:
        panel->setTextureRect( ::hud_digit_6 );
        break;
    case 7:
        panel->setTextureRect( ::hud_digit_7 );
        break;
    case 8:
        panel->setTextureRect( ::hud_digit_8 );
        break;
    case 9:
        panel->setTextureRect( ::hud_digit_9 );
        break;
    default:
        resetIndicator( panel );       
    }
}

void HUD::setSignumIndicator(gui::IGuiPanel* panel, float value)
{
    if( value < 0 )
    {
        panel->setTextureRect( ::hud_digit_minus );
    }
    else
    {
        panel->setTextureRect( ::hud_digit_null );
    }
}

/**
 * class implementation
 */

HUD::HUD(Actor* parent) : Actor( parent )
{
}

HUD::~HUD()
{
}