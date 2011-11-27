
#include "headers.h"
#include "goal.h"

/**
 * class implementation
 */

Goal::Goal(Jumper* player) : Actor( player )
{
    _window = Gameplay::iGui->createWindow( "Goal" ); assert( _window );
    _player = player;
}

Goal::~Goal()
{
    _window->getPanel()->release();    
}

/**
 * Actor abstracts
 */

void Goal::onUpdateActivity(float dt)
{
    // update goal name
    gui::IGuiPanel* panel = _window->getPanel()->find( "GoalName" ); assert( panel && panel->getStaticText() );
    /**/panel->getStaticText()->setText( getGoalName() );

    // update goal value
    panel = _window->getPanel()->find( "GoalValue" ); assert( panel && panel->getStaticText() );
    /**/panel->getStaticText()->setText( getGoalValue() );

    gui::IGuiPanel* goalScoreShadow = _window->getPanel()->find( "GoalScoreShadow" ); assert( goalScoreShadow && goalScoreShadow->getStaticText() );
    gui::IGuiPanel* goalScorePanel  = _window->getPanel()->find( "GoalScore" ); assert( goalScoreShadow && goalScoreShadow->getStaticText() );
    float score = getGoalScore();
    if( score == 0 )
    {
        goalScoreShadow->setVisible( false );
        goalScorePanel->setVisible( false );
    }
    else
    {
        // show score
        goalScoreShadow->setVisible( true );
        goalScorePanel->setVisible( true );
        // build text
        std::wstring text = ( score < 0 ) ? L"" : L"+";
        text += wstrformat( L"%1.0f", score );
        // update panels
        goalScoreShadow->getStaticText()->setText( text.c_str() );
        goalScorePanel->getStaticText()->setText( text.c_str() );
        if( score > 0 )
        {
            goalScorePanel->getStaticText()->setTextColor( Vector4f( 0,1,0,1 ) );
        }
        else
        {
            goalScorePanel->getStaticText()->setTextColor( Vector4f( 1,0,0,1 ) );
        }
    }
}

void Goal::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
    if( eventId == EVENT_GOAL_ENUMERATE )
    {
        ActorV* goals = reinterpret_cast<ActorV*>( eventData );
        goals->push_back( this );
    }
}