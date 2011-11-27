
#include "headers.h"
#include "goal.h"

/**
 * class implementation
 */

GoalExperience::GoalExperience(Jumper* player) : Goal( player )
{
    _isAcquired = false;
    _prevSkills = _player->getVirtues()->skills;
}

GoalExperience::~GoalExperience()
{
    Virtues* virtues = getScene()->getCareer()->getVirtues();
    assert( _finite( getGoalScore() ) );
    virtues->evolution.score += getGoalScore();
}

/**
 * Actor abstracts
 */

void GoalExperience::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );

    // capture flight health
    if( !_isAcquired && _player->isOverActivity() )
    {
        _isAcquired = true;
        _currSkills = _player->getVirtues()->skills;
    }
}

/**
 * Goal abstracts
 */

const wchar_t* GoalExperience::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(493);
}

const wchar_t* GoalExperience::getGoalValue(void)
{
    if( _isAcquired )
    {
        _goalValue = wstrformat( Gameplay::iLanguage->getUnicodeString(494), getTotalSkillEvolution() );
    }
    else
    {
        _goalValue = L"";
    } 
    return _goalValue.c_str();
}

float GoalExperience::getGoalScore(void)
{
    if( _isAcquired )
    {
        return getTotalSkillEvolution();
    }
    else
    {
        return 0;
    }
}

float GoalExperience::getTotalSkillEvolution(void)
{
    return _currSkills.perception - _prevSkills.perception +
           _currSkills.endurance - _prevSkills.endurance +
           _currSkills.tracking - _prevSkills.tracking +
           _currSkills.rigging - _prevSkills.rigging;
}