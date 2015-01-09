
#include "headers.h"
#include "goal.h"

/**
 * class implementation
 */

GoalJumps::GoalJumps(Jumper* player) : Goal( player )
{
    _isAcquired = false;
    _prevTime = _newTime = _player->getVirtues()->statistics.numSkydives + _player->getVirtues()->statistics.numBaseJumps;
}

GoalJumps::~GoalJumps()
{
}

/**
 * Actor abstracts
 */

void GoalJumps::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );

    // capture flight health
    if( !_isAcquired && _player->isOverActivity() )
    {
        _isAcquired = true;
        _newTime = _player->getVirtues()->statistics.numSkydives + _player->getVirtues()->statistics.numBaseJumps;
    }
}

/**
 * Goal abstracts
 */

const wchar_t* GoalJumps::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(459);
}


const wchar_t* GoalJumps::getGoalValue(void)
{
	//if (!_isAcquired) {
		_newTime = _player->getVirtues()->statistics.numSkydives + _player->getVirtues()->statistics.numBaseJumps;
	//}

    if( _newTime - _prevTime > 0 )
    {
		const unsigned int jumps = _newTime - _prevTime;
		if (jumps == 100 ||
			jumps == 200 ||
			jumps == 500 ||
			jumps % 1000 == 0) {
			_goalValue = wstrformat( wstrformat(L"%d (BEER!!)", _newTime).c_str() );
		} else {
			_goalValue = wstrformat( wstrformat(L"%d", _newTime).c_str() );
		}
    }
    else
    {
        _goalValue = L"";
    } 
    return _goalValue.c_str();
}

float GoalJumps::getGoalScore(void)
{
	_newTime = _player->getVirtues()->statistics.numSkydives + _player->getVirtues()->statistics.numBaseJumps;
    if( _newTime - _prevTime > 0 )
    {
        return 1.0f;
    }
    else
    {
        return 0.0f;
    }
}