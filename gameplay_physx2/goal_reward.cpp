
#include "headers.h"
#include "goal.h"

/**
 * class implementation
 */
/*
GoalReward::GoalReward(Jumper* player, GoalRewardDescriptor* descriptor) : Goal( player )
{
    _descriptor = *descriptor;
    _funds = 0.0f;
    
    // assert them
    for( unsigned int i=0; i<11; i++ )
    {
        assert( _descriptor.rate[i] >= 0.0f && _descriptor.rate[i] < 100000.0f );
    }
}

GoalReward::~GoalReward()
{
    // mission isn't aborted?
    if( _player->isOverActivity() )
    {
        Virtues* virtues = getScene()->getCareer()->getVirtues();
        assert( _finite( getGoalScore() ) );
        virtues->evolution.score += getGoalScore();
        virtues->evolution.funds += _funds;
    }
}
*/
/**
 * Actor abstracts
 */
/*
void GoalReward::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );

    // update goal
    _funds = 0.0f;
    if( _player->isLanding() )
    {
        // determine reward
        Virtues* virtues = getScene()->getCareer()->getVirtues();
        unsigned int rank = virtues->getRank();
        if( rank > 10 ) rank = 10;
        _funds = _descriptor.rate[rank];
    }
    _goalValue = wstrformat( L"%3.2f$", _funds );
}
*/
/**
 * Goal abstracts
 */

/*
const wchar_t* GoalReward::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(845);
}

const wchar_t* GoalReward::getGoalValue(void)
{
    return _goalValue.c_str();
}

float GoalReward::getGoalScore(void)
{
    return 0;
}
*/