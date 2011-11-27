
#include "headers.h"
#include "goal.h"
#include "version.h"

/**
 * class implementation
 */

GoalStateOfHealth::GoalStateOfHealth(Jumper* player) : Goal( player )
{
    _freefallHealth = player->getVirtues()->evolution.health;
    _flightHealth   = -1;
}

GoalStateOfHealth::~GoalStateOfHealth()
{
    if( _player->isOverActivity() )
    {	
        Virtues* virtues = getScene()->getCareer()->getVirtues();
        assert( _finite( getGoalScore() ) );
        virtues->evolution.score += getGoalScore();
        if( virtues->evolution.score < 0 )
        {
            virtues->evolution.score = 0;
        }
        if( virtues->evolution.health == 0 )
        {
            bool creditsIsAffected = true;            
            // credits is not affected for licensed character
            creditsIsAffected = !_scene->getCareer()->getLicensedFlag();
            // credits is not affected in developer edition
			if ( Gameplay::iGameplay->_cheatsEnabled )
			{
				creditsIsAffected = false;
			}
            #ifdef GAMEPLAY_DEVELOPER_EDITION
                creditsIsAffected = false;
            #endif
            getCore()->logMessage( "creditsIsAffected=%d", creditsIsAffected );

            if( creditsIsAffected ) virtues->evolution.credits--;
            if( virtues->evolution.credits > 0 )
            {
                virtues->evolution.health = 0.125f;
            }
        }
    }
}

/**
 * Actor abstracts
 */

void GoalStateOfHealth::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );

    // capture flight health
    if( _flightHealth < 0 &&
        _player->getPhase() == ::jpFlight &&
        _player->getCanopySimulator()->getInflation() > 0.75 )
    {
        _flightHealth = getScene()->getCareer()->getVirtues()->evolution.health;
    }
}

/**
 * Goal abstracts
 */

const wchar_t* GoalStateOfHealth::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(447);
}

const wchar_t* GoalStateOfHealth::getGoalValue(void)
{
    Virtues* virtues = getScene()->getCareer()->getVirtues();

    float injuries = _freefallHealth - virtues->evolution.health;

    // verbalize injuries
    if( virtues->evolution.health == 0 )
    {
        return Gameplay::iLanguage->getUnicodeString(453);
    }
    else if( injuries > 0.75f )
    {
        return Gameplay::iLanguage->getUnicodeString(452);
    }
    else if( injuries > 0.5f )
    {
        return Gameplay::iLanguage->getUnicodeString(451);
    }
    else if( injuries > 0.25 )
    {
        return Gameplay::iLanguage->getUnicodeString(450);
    }
    else if( injuries > 0.125 )
    {
        return Gameplay::iLanguage->getUnicodeString(449);
    }
    else
    {
        return Gameplay::iLanguage->getUnicodeString(448);
    }
}

float GoalStateOfHealth::getGoalScore(void)
{
    Virtues* virtues = getScene()->getCareer()->getVirtues();

    float injuries = _freefallHealth - virtues->evolution.health;

    // score injuries
    if( virtues->evolution.health == 0 )
    {
        return -100;
    }
    else if( injuries > 0.75f )
    {
        return -20;
    }
    else if( injuries > 0.5f )
    {
        return -10;
    }
    else if( injuries > 0.25 )
    {
        return -5;
    }
    else if( injuries > 0.125 )
    {
        return -1;
    }
    else
    {
        return 5;
    }
}