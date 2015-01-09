
#include "headers.h"
#include "goal.h"

/**
 * class implementation
 */

GoalWingsuitTime::GoalWingsuitTime(Jumper* player) : Goal( player )
{
    _isAcquired = false;
    _prevTime = _newTime = _player->getVirtues()->statistics.freeFallTime;
}

GoalWingsuitTime::~GoalWingsuitTime()
{
}

/**
 * Actor abstracts
 */

void GoalWingsuitTime::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );

    // capture flight health
    if( !_isAcquired && _player->isOverActivity() )
    {
        _isAcquired = true;
        _newTime = _player->getVirtues()->statistics.freeFallTime;
    }
}

/**
 * Goal abstracts
 */

const wchar_t* GoalWingsuitTime::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(875);
}


const wchar_t* GoalWingsuitTime::getGoalValue(void)
{
    if( _isAcquired )
    {
		float time = _newTime;
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

float GoalWingsuitTime::getGoalScore(void)
{
    if( _isAcquired )
    {
        return _newTime - _prevTime;
    }
    else
    {
        return 0;
    }
}