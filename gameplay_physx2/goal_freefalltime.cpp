
#include "headers.h"
#include "goal.h"

/**
 * class implementation
 */

GoalFreeFallTime::GoalFreeFallTime(Jumper* player) : Goal( player )
{
    _isAcquired = false;
    _prevFFTime = _newFFTime = _player->getVirtues()->statistics.freeFallTime;
}

GoalFreeFallTime::~GoalFreeFallTime()
{
}

/**
 * Actor abstracts
 */

void GoalFreeFallTime::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );

    // capture flight health
    if( !_isAcquired && _player->isOverActivity() )
    {
        _isAcquired = true;
        _newFFTime = _player->getVirtues()->statistics.freeFallTime;
    }
}

/**
 * Goal abstracts
 */

const wchar_t* GoalFreeFallTime::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(875);
}


const wchar_t* GoalFreeFallTime::getGoalValue(void)
{
	_newFFTime = _player->getVirtues()->statistics.freeFallTime;
    if( _newFFTime - _prevFFTime > 0.0f )
    {
		float time = _newFFTime;
		unsigned int hours, minutes, seconds;
		const unsigned int SECOND = 1;
		const unsigned int MINUTE = 60 * SECOND;
		const unsigned int HOUR = 60 * MINUTE;

		const float consts[] = {HOUR,MINUTE,SECOND};
		unsigned int *data[] = {&hours,&minutes,&seconds};
		for (int i = 0; i < 3; ++i) {
			if (data[i] != NULL) {
				*data[i] = (unsigned int)(time / consts[i]);
				time -= (float)*data[i] * consts[i];
			}
		}
		//getCore()->logMessage("%03d:%02d:%02d", hours, minutes, seconds);

        _goalValue = wstrformat( wstrformat(L"%03d:%02d:%02d", hours, minutes, seconds).c_str() );
    }
    else
    {
        _goalValue = L"";
    } 
    return _goalValue.c_str();
}

float GoalFreeFallTime::getGoalScore(void)
{
	_newFFTime = _player->getVirtues()->statistics.freeFallTime;
    if( _newFFTime - _prevFFTime > 0.0f )
    {
        return _newFFTime - _prevFFTime;
    }
    else
    {
        return 0;
    }
}