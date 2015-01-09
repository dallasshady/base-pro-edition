
#include "headers.h"
#include "goal.h"

/**
 * class implementation
 */

GoalCanopyTime::GoalCanopyTime(Jumper* player) : Goal( player )
{
    _isAcquired = false;
    _prevTime = _newTime = _player->getVirtues()->statistics.canopyTime;
}

GoalCanopyTime::~GoalCanopyTime()
{
}

/**
 * Actor abstracts
 */

void GoalCanopyTime::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );

    // capture flight health
    if( !_isAcquired && _player->isOverActivity() )
    {
        _isAcquired = true;
        _newTime = _player->getVirtues()->statistics.canopyTime;
    }
}

/**
 * Goal abstracts
 */

const wchar_t* GoalCanopyTime::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(877);
}


const wchar_t* GoalCanopyTime::getGoalValue(void)
{
	//if (!_isAcquired) {
		_newTime = _player->getVirtues()->statistics.canopyTime;
	//}

    if( _newTime - _prevTime > 0.0f )
    {
		//_isAcquired = true;
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

float GoalCanopyTime::getGoalScore(void)
{
	_newTime = _player->getVirtues()->statistics.canopyTime;
    if( _newTime - _prevTime > 0.0f )
    {
        return _newTime - _prevTime;
    }
    else
    {
        return 0;
    }
}