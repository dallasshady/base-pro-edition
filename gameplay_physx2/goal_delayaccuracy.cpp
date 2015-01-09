
#include "headers.h"
#include "goal.h"

/**
 * class implementation
 */

GoalDelayAccuracy::GoalDelayAccuracy(Jumper* player, GoalDelayAccuracyDescriptor* descriptor) : Goal( player )
{
    assert( descriptor->maxDelay >= descriptor->minDelay );
    assert( descriptor->interpolationSmoothing > 0 );
    _descriptor = *descriptor;
    _delay = 0.0f;
    _freefall = false;
}

GoalDelayAccuracy::~GoalDelayAccuracy()
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

void GoalDelayAccuracy::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );

    if( _freefall ) _delay += dt;
}

void GoalDelayAccuracy::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
    // base behaviour
    Goal::onEvent( initiator, eventId, eventData );

    // this class behaviour
    if( initiator == static_cast<Actor*>( _player ) )
    {
        if( eventId == EVENT_DELAY_START )
        {
            _freefall = true;
        }
        else if( eventId == EVENT_DELAY_STOP )
        {
            _freefall = false;
        }
    }
}

/**
 * Goal abstracts
 */

const wchar_t* GoalDelayAccuracy::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(607);
}

const wchar_t* GoalDelayAccuracy::getGoalValue(void)
{
    _goalValue = wstrformat( Gameplay::iLanguage->getUnicodeString(608), _delay );
    return _goalValue.c_str();
}

float GoalDelayAccuracy::getGoalScore(void)
{
    if( _freefall ) return 0;

    float maxBonus = _player->getVirtues()->getMaximalBonusScore();

    float factor = ( _delay - _descriptor.minDelay ) / ( _descriptor.maxDelay - _descriptor.minDelay);
    factor = factor < 0 ? 0 : ( factor > 1 ? 1 : factor );
    factor = pow( factor, _descriptor.interpolationSmoothing );

    return maxBonus * ( _descriptor.minScore * ( 1 - factor ) + _descriptor.maxScore * factor );
}