
#include "headers.h"
#include "landingaccuracy.h"
#include "imath.h"
#include "version.h"

/**
 * class implementation
 */

GoalHardLanding::GoalHardLanding(Jumper* jumper, Vector3f pos, float scale, Gear prize) : GoalLandingAccuracy( jumper, pos, scale ) 
{
    _prize = prize;
}

GoalHardLanding::~GoalHardLanding()
{
    if( getLandingAccuracy() > 0 )
    {
        #ifdef GAMEPLAY_EDITION_ATARI
            if( _scene->getCareer()->getLicensedFlag() )
            {
                // prise is money
                _scene->getCareer()->getVirtues()->evolution.funds += _prize.getCost();
            }
            else
            {
                // prise is gear
                _scene->getCareer()->addGear( _prize );
            }
        #else
            // prise is gear
            _scene->getCareer()->addGear( _prize );
        #endif
    }
}

/**
 * Goal abstracts
 */

const wchar_t* GoalHardLanding::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(549);
}

const wchar_t* GoalHardLanding::getGoalValue(void)
{
    float accuracy = getLandingAccuracy();
    
    if( accuracy == 0.0f )
    {
        return L"";
    }
    else
    {
        #ifdef GAMEPLAY_EDITION_ATARI
            if( _scene->getCareer()->getLicensedFlag() )
            {
                // money count
                _prizeCost = wstrformat( L"%2.1f$", _prize.getCost() );
                return _prizeCost.c_str();
            }
            else
            {
                // prise name
                return _prize.getName();
            }
        #else
            // prise name
            return _prize.getName();
        #endif
    }
}

float GoalHardLanding::getGoalScore(void)
{
    return 0.0f;
}