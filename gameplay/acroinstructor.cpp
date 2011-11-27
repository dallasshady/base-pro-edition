
#include "headers.h"
#include "acroinstructor.h"
#include "gameplay.h"
#include "../common/istring.h"

using namespace instructor;

/**
 * class implementation
 */

AcrobaticsInstructor::AcrobaticsInstructor(Jumper* player, Acrobatics skill) :
    Instructor( player )
{
    _skill = skill;
    _prevSkillValue = player->getScene()->getCareer()->getAcrobaticsSkill( skill );
    player->getScene()->getCareer()->setAcrobaticsSkill( skill, true );
}

AcrobaticsInstructor::~AcrobaticsInstructor()
{
    // mission is aborted?
    if( !_player->isOverActivity() )
    {
        _player->getScene()->getCareer()->setAcrobaticsSkill( _skill, _prevSkillValue );
    }

    // mission isn't aborted?
    if( _player->isOverActivity() )
    {
        Virtues* virtues = getScene()->getCareer()->getVirtues();
        assert( _finite( getGoalScore() ) );
        virtues->evolution.score += getGoalScore();
    }
}

/**
 * Goal abstracts
 */

const wchar_t* AcrobaticsInstructor::getGoalValue(void)
{
    return Gameplay::iLanguage->getUnicodeString(283);
}

float AcrobaticsInstructor::getGoalScore(void)
{
    return _player->getVirtues()->getMaximalBonusScore();
}

/**
 * Actor abstracts
 */

void AcrobaticsInstructor::onUpdateActivity(float dt)
{
    // base behaviour
    Instructor::onUpdateActivity( dt );
}