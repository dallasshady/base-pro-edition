
#include "headers.h"
#include "goal.h"
#include "imath.h"

/**
 * class implementation
 */

GoalSmokeball::GoalSmokeball(Jumper* player, std::vector<SmokeBallDesc>& smokeballs, float ballScore) : Goal( player )
{
    // create smokeballs
    for( unsigned int i=0; i<smokeballs.size(); i++ )
    {
        _smokeballs.push_back( new SmokeBall( this, &smokeballs[i] ) );
    }

    // create & disable smoke jet 
    _currentJetColor = _targetJetColor = Vector4f( 1,1,1,1 );
    _smokeJet = new SmokeJet( player, _currentJetColor, ::sjmLeft );
    _smokeJet->enable( false );

    // reset score calculator
    _numPiercedBalls = 0;
    _ballScore = ballScore;
}

GoalSmokeball::~GoalSmokeball()
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

const float colorSpeed = 0.5f;

void GoalSmokeball::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );

    // animate jet color
    if( _smokeJet )
    {
        float c,t;
        for( unsigned int i=0; i<3; i++ )
        {
            c = _currentJetColor[i];
            t = _targetJetColor[i];
            if( sgn( t - c ) > 0 )
            {
                c += colorSpeed * dt;
                c = c > t ? t : c;
            }
            else if( sgn( t - c ) < 0 )
            {
                c -= colorSpeed * dt;
                c = c < t ? t : c;
            }
            _currentJetColor[i] = c;
        }
        _smokeJet->setColor( _currentJetColor );
    }
}

void GoalSmokeball::onUpdatePhysics(void)
{
    // goal logic
    if( _player->getPhase() == ::jpFreeFalling ||
        _player->getCanopySimulator()->getInflation() < 0.25f )
    {
        Vector3f playerPos = wrap( _player->getFreefallActor()->getGlobalPosition() );

        for( unsigned int i=0; i<_smokeballs.size(); i++ )
        {
            if( !_smokeballs[i]->getUserFlag() && _smokeballs[i]->isInside( playerPos ) )
            {
                _smokeballs[i]->setUserFlag( true );
                _numPiercedBalls++;
                _targetJetColor = _smokeballs[i]->getDescriptor()->color;
                if( !_smokeJet->isEnabled() ) _currentJetColor = _targetJetColor;
                _smokeJet->enable( true );
            }
        }
    }
}

/**
 * Goal abstracts
 */

const wchar_t* GoalSmokeball::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(637);
}

const wchar_t* GoalSmokeball::getGoalValue(void)
{
    _goalValue = wstrformat( L"%d", _numPiercedBalls );
    return _goalValue.c_str();
}

float GoalSmokeball::getGoalScore(void)
{
    return _numPiercedBalls * _ballScore;
}