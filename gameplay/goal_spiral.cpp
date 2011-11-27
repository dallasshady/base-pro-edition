
#include "headers.h"
#include "goal.h"

/**
 * class implementation
 */

GoalSpiral::GoalSpiral(Jumper* player, Vector3f axisOffset) : Goal( player )
{
    _axisOffset = axisOffset;
    _axisOffset[1] = 0.0f;
    _captureIsActive = false;
    _angleAccumulator = 0.0f;
}

GoalSpiral::~GoalSpiral()
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

void GoalSpiral::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );

    // accumulate tracking data
    if( !_player->getFreefallActor()->isSleeping() )
    {
        if( _captureIsActive )
        {
            Vector3f currPos = _player->getClump()->getFrame()->getPos(); currPos[1] = 0.0f;
            Vector3f currDir = currPos - _axisOffset; currDir.normalize();
            Vector3f prevDir = _prevPos - _axisOffset; prevDir.normalize();
            _angleAccumulator += fabs( ::calcAngle( prevDir, currDir, Vector3f( 0,1,0 ) ) );
            _prevPos = currPos;
        }
        else
        {
            _captureIsActive = true;
            _prevPos = _player->getClump()->getFrame()->getPos();
            _prevPos[1] = 0.0f;
        }
    }
}

/**
 * Goal abstracts
 */

const wchar_t* GoalSpiral::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(541);
}

const wchar_t* GoalSpiral::getGoalValue(void)
{
    if( _captureIsActive )
    {
        _goalValue = wstrformat( Gameplay::iLanguage->getUnicodeString(542), _angleAccumulator );
        return _goalValue.c_str();
    }
    else
    {
        return L"";
    }
}

float GoalSpiral::getGoalScore(void)
{
    float score = 0;
    float angle = _angleAccumulator;
    float maxBonus = _player->getVirtues()->getMaximalBonusScore();
    
    float scoringAngle;

    // MB/10 for each angle more than 360"
    scoringAngle = angle - 360; scoringAngle = scoringAngle < 0 ? 0 : scoringAngle;
    score += float( int( scoringAngle * maxBonus / 10 ) );
    angle -= scoringAngle;

    // MB/25 for each angle more than 270"
    scoringAngle = angle - 270; scoringAngle = scoringAngle < 0 ? 0 : scoringAngle;
    score += float( int( scoringAngle * maxBonus / 25 ) );
    angle -= scoringAngle;

    // MB/50 for each angle more than 180"
    scoringAngle = angle - 180; scoringAngle = scoringAngle < 0 ? 0 : scoringAngle;
    score += float( int( scoringAngle * maxBonus / 50 ) );
    angle -= scoringAngle;

    // MB/75 for each angle more than 90"
    scoringAngle = angle - 90; scoringAngle = scoringAngle < 0 ? 0 : scoringAngle;
    score += float( int( scoringAngle * maxBonus / 75 ) );
    angle -= scoringAngle;

    // MB/100 for each angle less than 90"
    score += float( int( angle * maxBonus / 100 ) );

    return score;
}