
#include "headers.h"
#include "goal.h"

/**
 * class implementation
 */

GoalProximity::GoalProximity(Jumper* player, GoalProximityDescriptor* descriptor) : Goal( player )
{
    _descriptor = *descriptor;
    _score = 0.0f;

    // check descriptor
    assert( _descriptor.range0.distance > _descriptor.range1.distance );
    assert( _descriptor.range1.distance > _descriptor.range2.distance );
}

GoalProximity::~GoalProximity()
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

void GoalProximity::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );

    // goal works only in freefall phase, and if player is alive
    if( _player->getPhase() == ::jpFreeFalling && _player->getVirtues()->evolution.health > 0 )
    {
        // maximal bonus
        float maxBonus = _player->getVirtues()->getMaximalBonusScore();

		PxSphereGeometry worldSphere;
		PxOverlapBuffer hit;
        //PHYSX3
		//worldSphere.center = _player->getFreefallActor()->getGlobalPose().p;

        // detect first range proximity        
        worldSphere.radius = _descriptor.range0.distance;
        if(_scene->getPhScene()->overlap(worldSphere, _player->getFreefallActor()->getGlobalPose(), hit))
        {
            // detect second range proximity
            worldSphere.radius = _descriptor.range1.distance;
            if(_scene->getPhScene()->overlap(worldSphere, _player->getFreefallActor()->getGlobalPose(), hit))
            {
                // detect third range proximity
                worldSphere.radius = _descriptor.range2.distance;
                if(_scene->getPhScene()->overlap(worldSphere, _player->getFreefallActor()->getGlobalPose(), hit))
                {
                    // detect fourth range proximity
                    worldSphere.radius = _descriptor.range3.distance;
                    if(_scene->getPhScene()->overlap(worldSphere, _player->getFreefallActor()->getGlobalPose(), hit))
                    {
                        // scoring by fourth range
                        _score += maxBonus * _descriptor.range3.scorePerSecond * dt;
                    }
                    else
                    {
                        // scoring by third range
                        _score += maxBonus * _descriptor.range2.scorePerSecond * dt;
                    }
                }
                else
                {
                    // scoring by second range
                    _score += maxBonus * _descriptor.range1.scorePerSecond * dt;
                }
            }
            else
            {
                // scoring by first range
                _score += maxBonus * _descriptor.range0.scorePerSecond * dt;
            }
        }
    }
}

/**
 * Goal abstracts
 */

const wchar_t* GoalProximity::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(578);
}

const wchar_t* GoalProximity::getGoalValue(void)
{
    return L"";
}

float GoalProximity::getGoalScore(void)
{
    if( _player->getVirtues()->evolution.health > 0 )
    {
        return _score;
    }
    else
    {
        return 0;
    }
}