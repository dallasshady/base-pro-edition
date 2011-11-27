
#include "headers.h"
#include "goal.h"

/**
 * class implementation
 */

GoalTrackingPerformance::GoalTrackingPerformance(Jumper* player) : Goal( player )
{
    _horizontalDistance = 0.0f;
    _verticalDistance = 0.0f;
    _captureIsActive = false;
}

GoalTrackingPerformance::~GoalTrackingPerformance()
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

void GoalTrackingPerformance::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );

    // accumulate tracking data
    if( !_player->getFreefallActor()->isSleeping() )
    {
        if( _captureIsActive )
        {
            Vector3f distance = _player->getClump()->getFrame()->getPos() - _prevPos;
            _prevPos = _player->getClump()->getFrame()->getPos();
            _verticalDistance += fabs( distance[1] );
            distance[1] = 0;
            _horizontalDistance += distance.length();
        }
        else
        {
            _captureIsActive = true;
            _prevPos = _player->getClump()->getFrame()->getPos();
        }
    }
}

/**
 * Goal abstracts
 */

const wchar_t* GoalTrackingPerformance::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(539);
}

const wchar_t* GoalTrackingPerformance::getGoalValue(void)
{
    if( _captureIsActive )
    {
        _goalValue = wstrformat( Gameplay::iLanguage->getUnicodeString(540), _horizontalDistance * 0.01f, _horizontalDistance/_verticalDistance );
        return _goalValue.c_str();
    }
    else
    {
        return L"";
    }
}

float GoalTrackingPerformance::getGoalScore(void)
{
    float score = 0;
    float distance = _horizontalDistance * 0.01f;
    float maxBonus = _player->getVirtues()->getMaximalBonusScore();
    
    float scoringDistance;

    // MB/25 for each meter more than 250m
    scoringDistance = distance - 250; scoringDistance = scoringDistance < 0 ? 0 : scoringDistance;
    score += float( int( scoringDistance * maxBonus / 25.0f ) );
    distance -= scoringDistance;

    // MB/50 for each meter more than 200m
    scoringDistance = distance - 200; scoringDistance = scoringDistance < 0 ? 0 : scoringDistance;
    score += float( int( scoringDistance * maxBonus / 50.0f ) );
    distance -= scoringDistance;

    // MB/75 for each meter more than 150m
    scoringDistance = distance - 150; scoringDistance = scoringDistance < 0 ? 0 : scoringDistance;
    score += float( int( scoringDistance * maxBonus / 75.0f ) );
    distance -= scoringDistance;

    // MB/100 for each meter less than 150m
    score += float( int( distance * maxBonus / 100.0f ) );

    return score;
}