
#include "headers.h"
#include "goal.h"

/**
 * class implementation
 */

GoalFlipCount::GoalFlipCount(Jumper* player, float score) : Goal( player )
{
    _score            = score;
    _captureIsActive  = false;
    _prevAt           = player->getClump()->getFrame()->getAt();
    _prevRight        = player->getClump()->getFrame()->getRight();
    _angleAccumulator = 0.0f;
    _goalValue        = L"";
}

GoalFlipCount::~GoalFlipCount()
{
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

void GoalFlipCount::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );

    // accumulate tracking data
    if( !_player->getFreefallActor()->isSleeping() )
    {
        if( _captureIsActive )
        {
            Vector3f currAt    = _player->getClump()->getFrame()->getAt();
            Vector3f currRight = _player->getClump()->getFrame()->getRight();
            currAt.normalize();
            currRight.normalize();;
            _angleAccumulator += fabs( ::calcAngle( _prevAt, currAt, currRight ) );
            _prevAt    = currAt;
            _prevRight = currRight;
        }
        else
        {
            _captureIsActive  = true;
            _prevAt           = _player->getClump()->getFrame()->getAt();
            _prevRight        = _player->getClump()->getFrame()->getRight();
            _prevAt.normalize();
            _prevRight.normalize();
        }
    }
}

/**
 * Goal abstracts
 */

const wchar_t* GoalFlipCount::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(687);
}

const wchar_t* GoalFlipCount::getGoalValue(void)
{
    if( _captureIsActive )
    {        
        float angle = _angleAccumulator;
        int numFullFlips = int( _angleAccumulator / 360.0f );
        angle -= numFullFlips * 360.0f;
        
        _goalValue = wstrformat( L"%d ", numFullFlips );
        if( angle >= 270 )
        {
            _goalValue += L"+3/4";
        }
        else if( angle >= 180 )
        {
            _goalValue += L"+1/2";
        }
        else if( angle >= 90 )
        {
            _goalValue += L"+1/4";
        }
        else if( angle >= 45 )
        {
            _goalValue += L"+1/8";
        }

        return _goalValue.c_str();
    }
    else
    {
        return L"";
    }
}

float GoalFlipCount::getGoalScore(void)
{
    return _score * _angleAccumulator / 360.0f;
}