
#include "headers.h"
#include "goal.h"

/**
 * class implementation
 */

GoalFootage::GoalFootage(Jumper* player, NPC* npc, GoalFootageDescriptor* descriptor) : Goal( player )
{
    _npc = npc;
    _step = 0;
    _averageDistance = 0;
    _footageTime = 0.0f;
    _descriptor = *descriptor;

    // check descriptor
    assert( _descriptor.bad.distance > _descriptor.poor.distance );
    assert( _descriptor.poor.distance > _descriptor.tolerable.distance );
    assert( _descriptor.tolerable.distance > _descriptor.good.distance );
}

GoalFootage::~GoalFootage()
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

void GoalFootage::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );

    // accumulate tracking data
    if( !_player->getFreefallActor()->isSleeping() &&
        !_npc->getJumper()->getFreefallActor()->isSleeping() )
    {
        // determine distance between player and NPC
        float distance = 0.01f * ( _player->getClump()->getFrame()->getPos() - _npc->getJumper()->getClump()->getFrame()->getPos() ).length();

        // do not consider large distances into account
        if( distance < _descriptor.bad.distance )
        {
            _footageTime += dt;

            // accumulate average distance
            _step++;
            if( _step == 1 )
            {
                _averageDistance = distance;
            }
            else
            {
                _averageDistance = ( ( _step - 1 ) * _averageDistance + distance ) / _step;
            }
        }
    }
}

/**
 * Goal abstracts
 */

const wchar_t* GoalFootage::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(553);
}

const wchar_t* GoalFootage::getGoalValue(void)
{
    if( _step )
    {
        if( _averageDistance > _descriptor.bad.distance )
        {
            _goalValue = wstrformat( Gameplay::iLanguage->getUnicodeString(558), int( _footageTime ) );
        }
        else if( _averageDistance > _descriptor.poor.distance )
        {
            _goalValue = wstrformat( Gameplay::iLanguage->getUnicodeString(557), int( _footageTime ) );
        }
        else if( _averageDistance > _descriptor.tolerable.distance )
        {
            _goalValue = wstrformat( Gameplay::iLanguage->getUnicodeString(556), int( _footageTime ) );
        }
        else if( _averageDistance > _descriptor.good.distance )
        {
            _goalValue = wstrformat( Gameplay::iLanguage->getUnicodeString(555), int( _footageTime ) );
        }
        else
        {
            _goalValue = wstrformat( Gameplay::iLanguage->getUnicodeString(554), int( _footageTime ) );
        }
        return _goalValue.c_str();
    }
    else
    {
        return L"";
    }
}

float GoalFootage::getGoalScore(void)
{
    if( !_step ) return 0;

    float maxBonus = _player->getVirtues()->getMaximalBonusScore();

    float score;
    if( _averageDistance > _descriptor.bad.distance )
    {
        score = maxBonus * _descriptor.bad.score;
    }
    else if( _averageDistance > _descriptor.poor.distance )
    {
        score = maxBonus * _descriptor.poor.score;
    }
    else if( _averageDistance > _descriptor.tolerable.distance )
    {
        score = maxBonus * _descriptor.tolerable.score;
    }
    else if( _averageDistance > _descriptor.good.distance )
    {
        score = maxBonus * _descriptor.good.score;
    }
    else
    {
        score = maxBonus * _descriptor.fine.score;
    }

    score *= _footageTime * _descriptor.timeFactor;
    return score;
}