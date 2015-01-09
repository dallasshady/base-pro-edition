
#include "headers.h"
#include "goal.h"

/**
 * class implementation
 */

GoalBonus::GoalBonus(Jumper* player, const wchar_t* bonusName, BonusType bonusType, float bonusParameter) : Goal( player )
{
    _bonusName      = bonusName;
    _bonusType      = bonusType;
    _bonusParameter = bonusParameter;
    _startHealth    = player->getVirtues()->evolution.health;
}

GoalBonus::~GoalBonus()
{
    // mission isn't aborted?
    if( _player->isOverActivity() )
    {
        Virtues* virtues = getScene()->getCareer()->getVirtues();
        assert( _finite( getGoalScore() ) );
        virtues->evolution.score += getGoalScore();
    }
}

/**
 * Actor abstracts
 */

void GoalBonus::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );
}

/**
 * Goal abstracts
 */

const wchar_t* GoalBonus::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(532);
}

const wchar_t* GoalBonus::getGoalValue(void)
{
    return _bonusName.c_str();
}

float GoalBonus::getGoalScore(void)
{
    if( !_player->isOverActivity() ) return 0;

    float damage = _startHealth - _player->getVirtues()->evolution.health;
    switch( _bonusType )
    {
    case btAbsolute:
        return _bonusParameter;
        break;
    case btProgressive:
        return _bonusParameter * _player->getVirtues()->getMaximalBonusScore();
        break;
    case btUnderground:
        if( damage > 0.125f && !_player->isLanding() )
        {
            return -1 * _player->getVirtues()->getMaximalBonusScore();
        }
        else
        {
            return _bonusParameter * _player->getVirtues()->getMaximalBonusScore();
        }
        break;
    default:
        return 0;
    }
}