#include "goal.h"

/**
 * class implementation
 */

GoalOpening::GoalOpening(Jumper* player) : Goal( player )
{
    _isAcquired      = false;
    _maxDecelaration = 0.0f;
	_prevSpeed		 = 0.0f;
}

/**
 * Actor abstracts
 */
GoalOpening::~GoalOpening()
{
}
float GoalOpening::getGoalScore(void)
{
	return 0;
}
void GoalOpening::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );

    // capture flight health
	if( !_isAcquired && _player->getPhase() == ::jpFlight && _player->getOverburden().magnitude() > _maxDecelaration)
    {
		_maxDecelaration = _player->getOverburden().magnitude();
    }

	if (_player->getDominantCanopy() && _player->getDominantCanopy()->getInflation() > 0.6f) {
		_isAcquired = true;
	}
}

/**
 * Goal abstracts
 */

const wchar_t* GoalOpening::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(916);
}

const wchar_t* GoalOpening::getGoalValue(void)
{
    _goalValue = wstrformat( Gameplay::iLanguage->getUnicodeString(917), _maxDecelaration );
    return _goalValue.c_str();
}