
#include "headers.h"
#include "landingaccuracy.h"
#include "imath.h"

/**
 * class implementation
 */

GoalLandingAccuracy::GoalLandingAccuracy(Jumper* jumper, Vector3f pos, float scale) : Goal( jumper ) 
{
    _jumper = jumper;
    _radius = 0.5f * scale;
    _isFixed = false;
    _fixedAccuracy = 0.0f;

    // create 3d-model of landing circle
    engine::IClump* templateClump = Gameplay::iGameplay->findClump( "LandingCircle" ); assert( templateClump );
    _landingCircle = templateClump->clone( "LandingCircleClone" ); assert( _landingCircle );

    // place & scale landing circle
    Vector3f s = ::calcScale( _landingCircle->getFrame()->getMatrix() );
    s *= scale;
    _landingCircle->getFrame()->setMatrix( Matrix4f( 
        s[0], 0,0,0,
        0, s[1], 0,0,
        0,0, s[2], 0,
        pos[0],pos[1],pos[2],1
    ) );

    // insert landing circle in scene
    _scene->getStage()->add( _landingCircle );

    // create signature
    _signature = Gameplay::iGui->createWindow( "JumperSignature" ); assert( _signature );
    Gameplay::iGui->getDesktop()->insertPanel( _signature->getPanel() );
}

GoalLandingAccuracy::~GoalLandingAccuracy()
{
    // mission isn't aborted?
    if( _player->isOverActivity() )
    {
        Virtues* virtues = getScene()->getCareer()->getVirtues();
        assert( _finite( getGoalScore() ) );
        virtues->evolution.score += getGoalScore();
    }

    _signature->getPanel()->release();
    _landingCircle->release();
}

/**
 * actor abstracts
 */

void GoalLandingAccuracy::onUpdateActivity(float dt)
{
    // base behaviour
    Goal::onUpdateActivity( dt );
}

void GoalLandingAccuracy::onUpdatePhysics()
{
    // base behaviour
    Goal::onUpdatePhysics();
}

void GoalLandingAccuracy::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
    // base behaviour
    Goal::onEvent( initiator, eventId, eventData );

    if( eventId == EVENT_CAMERA_IS_ACTUAL )
    {
        // update signature window
        Vector3f screenPos = Gameplay::iEngine->getDefaultCamera()->projectPosition( _landingCircle->getFrame()->getPos() );
        if( screenPos[2] > 1 )
        {
            _signature->getPanel()->setVisible( false );
            return;
        }
        else
        {
            _signature->getPanel()->setVisible( true );
        }

        gui::Rect oldRect = _signature->getPanel()->getRect();
        gui::Rect newRect(
            int( screenPos[0] ), int( screenPos[1] ),
            int( screenPos[0] ) + oldRect.getWidth(),
            int( screenPos[1] ) + oldRect.getHeight()
        );
        _signature->getPanel()->setRect( newRect );

        // landing circle name
        gui::IGuiPanel* panel = _signature->getPanel()->find( "JumperName" ); assert( panel && panel->getStaticText() );
        panel->getStaticText()->setText( Gameplay::iLanguage->getUnicodeString(478) );

        // distance to landing circle
        Vector3f distance = Gameplay::iEngine->getDefaultCamera()->getFrame()->getPos() - _landingCircle->getFrame()->getPos();
        panel = _signature->getPanel()->find( "Distance" ); assert( panel && panel->getStaticText() );
        panel->getStaticText()->setText( 
            wstrformat( Gameplay::iLanguage->getUnicodeString(219), distance.length() * 0.01f ).c_str()
        );

        // no other panels is actual
        panel = _signature->getPanel()->find( "Canopy" ); assert( panel && panel->getStaticText() );
        panel->getStaticText()->setText( L"" );
        panel = _signature->getPanel()->find( "PC" ); assert( panel && panel->getStaticText() );
        panel->getStaticText()->setText( L"" );
    }
}

/**
 * Goal abstracts
 */

const wchar_t* GoalLandingAccuracy::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString(479);
}

const wchar_t* GoalLandingAccuracy::getGoalValue(void)
{
    float accuracy = getLandingAccuracy();
    
    if( accuracy == 0.0f )
    {
        return Gameplay::iLanguage->getUnicodeString(484);
    }
    else
    {
        _goalValue = wstrformat( Gameplay::iLanguage->getUnicodeString(480), ( 1 - accuracy ) * _radius );
        return _goalValue.c_str();
    }
}

float GoalLandingAccuracy::getGoalScore(void)
{
    float landingAccuracy = getLandingAccuracy();
    float scoreMultiplier = landingAccuracy * landingAccuracy * 2;
    return scoreMultiplier * _player->getVirtues()->getMaximalBonusScore();
}

/**
 * class behaviour
 */

float GoalLandingAccuracy::getLandingAccuracy(void)
{
    if( _isFixed ) return _fixedAccuracy;

    // principle of goal fixation
    if( !_isFixed && _jumper->isLanding() && _jumper->getVel().length() < 25.0f )
    {
        Vector3f jumperPos2D = _jumper->getClump()->getFrame()->getPos();
        jumperPos2D[1] = 0;
        Vector3f targetPos2D = _landingCircle->getFrame()->getPos();
        targetPos2D[1] = 0;
        float distance2D = ( jumperPos2D - targetPos2D ).length();
        distance2D *= 0.01f;

        // jumper is inside the circle?
        if( distance2D < _radius ) 
        {            
            _fixedAccuracy = 1 - distance2D / _radius;
        }
        else
        {
            _fixedAccuracy = 0;
        }
        _isFixed = true;
        return _fixedAccuracy;
    }

    // jumper is outside the circle or he was landed badly
    return 0;
}