
#include "headers.h"
#include "goal.h"

/**
 * class implementation
 */

GoalDropzone::GoalDropzone(Jumper* player) : Goal( player )
{
    _isAcquired         = false;
    _deploymentAltitude = -1.0f;
}

GoalDropzone::~GoalDropzone()
{
    Virtues* virtues = getScene()->getCareer()->getVirtues();
    assert( _finite( getGoalScore() ) );
    virtues->evolution.score += getGoalScore();
    if( virtues->evolution.score < 0 )
    {
        virtues->evolution.score = 0;
    }
}

/**
 * Actor abstracts
 */

void GoalDropzone::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );

    // capture flight health
    if( !_isAcquired && _player->getPhase() == ::jpFlight )
    {
        _isAcquired = true;
        _deploymentAltitude = _player->getClump()->getFrame()->getPos()[1];
    }
}

/**
 * Goal abstracts
 */

const wchar_t* GoalDropzone::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(489);
}

const wchar_t* GoalDropzone::getGoalValue(void)
{
    if( _isAcquired )
    {
        _goalValue = wstrformat( Gameplay::iLanguage->getUnicodeString(490), int( _deploymentAltitude * 0.01f ) );
    }
    else
    {
        _goalValue = L"";
    } 
    return _goalValue.c_str();
}

float GoalDropzone::getGoalScore(void)
{
    if( _isAcquired )
    {
        float maxBonus = _player->getVirtues()->getMaximalBonusScore();
        if( _deploymentAltitude < 30000 ) return maxBonus * -3.0f;
        else if( _deploymentAltitude < 40000 ) return maxBonus * -2.0f;
        else if( _deploymentAltitude < 50000 ) return maxBonus * -1.0f;
        else if( _deploymentAltitude < 60000 ) return maxBonus * -0.5f;
        else return 0;
    }
    else
    {
        return 0;
    }
}