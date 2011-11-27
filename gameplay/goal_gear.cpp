
#include "headers.h"
#include "goal.h"
#include "version.h"

/**
 * class implementation
 */

GoalStateOfGear::GoalStateOfGear(Jumper* player) : Goal( player )
{
    _helmetState = player->getVirtues()->equipment.helmet.state;
    _suitState   = player->getVirtues()->equipment.canopy.state;
    _rigState    = player->getVirtues()->equipment.rig.state;
    _canopyState = player->getVirtues()->equipment.canopy.state;
}

GoalStateOfGear::~GoalStateOfGear()
{
    if( _player->isOverActivity() )
    {	
        Virtues* virtues = getScene()->getCareer()->getVirtues();
        assert( _finite( getGoalScore() ) );
        virtues->evolution.score += getGoalScore();
        if( virtues->evolution.score < 0 )
        {
            virtues->evolution.score = 0;
        }        
    }
}

/**
 * Actor abstracts
 */

void GoalStateOfGear::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );
}

/**
 * Goal abstracts
 */

const wchar_t* GoalStateOfGear::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(843);
}

const wchar_t* GoalStateOfGear::getGoalValue(void)
{
    Virtues* virtues = getScene()->getCareer()->getVirtues();

    // verbalize gear state
    _goalValue = L"";
    float damage;
    float totalDamage = 0;
    float maxDamage = 0;

    #ifndef GAMEPLAY_EDITION_ATARI
        damage = _helmetState - virtues->equipment.helmet.state;
        if( damage < 0 ) damage = 0;
        totalDamage += damage;
        if( damage > 0.01f && damage > maxDamage )
        {
            maxDamage = damage;
            _goalValue = Gameplay::iLanguage->getUnicodeString(184);
            _goalValue += L": ";
            _goalValue += wstrformat( L"-%d%%", int( damage * 100 ) );
            _goalValue += L"; ";
        }
    #endif

    damage = _suitState - virtues->equipment.suit.state;
    if( damage < 0 ) damage = 0;
    totalDamage += damage;
    if( damage > 0.01f && damage > maxDamage )
    {
        maxDamage = damage;
        _goalValue = Gameplay::iLanguage->getUnicodeString(185);
        _goalValue += L": ";
        _goalValue += wstrformat( L"-%d%%", int( damage * 100 ) );
        _goalValue += L"; ";
    }

    damage = _rigState - virtues->equipment.rig.state;
    if( damage < 0 ) damage = 0;
    totalDamage += damage;
    if( damage > 0.01f && damage > maxDamage )
    {
        maxDamage = damage;
        _goalValue = Gameplay::iLanguage->getUnicodeString(186);
        _goalValue += L": ";
        _goalValue += wstrformat( L"-%d%%", int( damage * 100 ) );
        _goalValue += L"; ";
    }

    damage = _canopyState - virtues->equipment.canopy.state;
    if( damage < 0 ) damage = 0;
    totalDamage += damage;
    if( damage > 0.01f && damage > maxDamage )
    {
        maxDamage = damage;
        _goalValue = Gameplay::iLanguage->getUnicodeString(188);
        _goalValue += L": ";
        _goalValue += wstrformat( L"-%d%%", int( damage * 100 ) );
        _goalValue += L"; ";
    }

    _goalValue += Gameplay::iLanguage->getUnicodeString(101);
    _goalValue += L": ";
    _goalValue += wstrformat( L"-%d%%", int( totalDamage * 100 ) );

    return _goalValue.c_str();
}

float GoalStateOfGear::getGoalScore(void)
{
    Virtues* virtues = getScene()->getCareer()->getVirtues();

    float totalDamage = 0;

    #ifndef GAMEPLAY_EDITION_ATARI
        totalDamage += ( _helmetState - virtues->equipment.helmet.state );
    #endif
    totalDamage += ( _suitState - virtues->equipment.suit.state );
    totalDamage += ( _rigState - virtues->equipment.rig.state );
    totalDamage += ( _canopyState - virtues->equipment.canopy.state );

    if( totalDamage < 0.01f )
    {
        return 5.0f;
    }
    else
    {
        return -( totalDamage * 100.0f );
    }
}