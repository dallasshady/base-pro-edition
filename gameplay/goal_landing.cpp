
#include "headers.h"
#include "goal.h"

const wchar_t* GoalLanding::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(422);
}

const wchar_t* GoalLanding::getGoalValue(void)
{
    Virtues* virtues = _scene->getCareer()->getVirtues();

    if( virtues->evolution.health == 0 )
    {
        return Gameplay::iLanguage->getUnicodeString(454);
    }
    else if( _player->isLanding() )
    {
        return Gameplay::iLanguage->getUnicodeString(423);
    }
    else if( _player->isBadLanding() )
    {
        return Gameplay::iLanguage->getUnicodeString(424);
    }
    else
    {
        return Gameplay::iLanguage->getUnicodeString(425);
    }
}

float GoalLanding::getGoalScore(void)
{
    return _score;
}

void GoalLanding::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );

    // this class behaviour
    if( !_isAcquired )
    {
        if( _player->getVirtues()->evolution.health == 0 ) _score = 0;
        else if( _player->isLanding() ) _score = 5.0f;
        else if( _player->isBadLanding() ) _score = -1.0f;
        else _score = 0.0f;
    }
}

GoalLanding::GoalLanding(Jumper* player) : Goal( player )
{
    _score      = 0.0f;
    _isAcquired = false;
}

GoalLanding::~GoalLanding()
{
    Virtues* virtues = _scene->getCareer()->getVirtues();
    virtues->evolution.score += getGoalScore();
    assert( _finite( getGoalScore() ) );
    if( virtues->evolution.score < 0 )
    {
        virtues->evolution.score = 0;
    }
}