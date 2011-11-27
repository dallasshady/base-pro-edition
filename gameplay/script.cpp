
#include "headers.h"
#include "script.h"
#include "imath.h"

/**
 * abstract routine
 */

Script::Routine::Routine(Jumper* jumper)
{
    // memorize jumper
    _jumper = jumper;

    // create window
    _window = Gameplay::iGui->createWindow( "Instructor" ); assert( _window );
    Gameplay::iGui->getDesktop()->insertPanel( _window->getPanel() );
    _window->align( gui::atCenter, 0, gui::atCenter, 0 );
}

Script::Routine::~Routine()
{
    _window->getPanel()->release();
}

void Script::Routine::setMessage(const wchar_t* message)
{
    // setup message
    gui::IGuiPanel* panel = _window->getPanel()->find( "Message" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( message );
    panel = _window->getPanel()->find( "MessageShadow" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( message );
}

/**
 * message routine
 */

Script::Message::Message(Jumper* jumper, const wchar_t* message, float delay) :
    Routine( jumper )
{
    _lock = false;
    _timeLeft = delay;
    setMessage( message );
}

void Script::Message::onUpdateRoutine(float dt)
{
    if( !_lock ) _timeLeft -= dt;
}

bool Script::Message::isExecuted(void)
{
    return _timeLeft <= 0;
}

/**
 * assist routine
 */

const float assistTimeout = 0.25f;

Script::Assist::Assist(Jumper* jumper, NPC* npc, Vector3f wind) : Script::Message( jumper, L"", 0.5f )
{
    _npc  = npc;
    _wind = wind;
    _timeout = assistTimeout;
    setLock( true );
}

void Script::Assist::onUpdateRoutine(float dt)
{
    // base behaviour
    Message::onUpdateRoutine( dt );

    // timing
    _timeout -= dt;
    if( _timeout > 0 ) return;
    _timeout = assistTimeout;

    // check jumper is already jump
    if( isLocked() && getJumper()->getPhase() != ::jpRoaming )
    {
        setLock( false );
    }

    if( isLocked() )
    {
        // check distance between jumper and npc
        Vector3f distance = getJumper()->getClump()->getFrame()->getPos() - _npc->getJumper()->getClump()->getFrame()->getPos();
        if( distance.length() < 100.0f )
        {            
            // distance from jumper to abyss
            float distanceToAbyss = getJumper()->getDistanceToAbyss();
            if( distanceToAbyss > 0 && distanceToAbyss < 50.0f )
            {
                // determine condition: "jumper is turned before the wind" 
                Vector3f jumperAt = getJumper()->getClump()->getFrame()->getAt();
                jumperAt[1] = 0;
                jumperAt.normalize();
                float dotProduct = Vector3f::dot( jumperAt, _wind );
                if( dotProduct > 0 )
                {
                    setMessage( wstrformat( Gameplay::iLanguage->getUnicodeString( 518 ), _npc->getNPCName() ).c_str() );
                    getJumper()->enablePhase( false );
                }
                else if( dotProduct < 0 && dotProduct > -0.77 )
                {
                    setMessage( L"" );
                    getJumper()->enablePhase( true );
                }
                else
                {
                    setMessage( L"" );
                    getJumper()->enablePhase( true );
                }
            }
            else
            {
                setMessage( L"" );
            }
        }
        else
        {
            if( distance.length() > 500.0f )
            {
                setMessage( wstrformat( Gameplay::iLanguage->getUnicodeString( 519 ), _npc->getNPCName() ).c_str() );
            }
            else
            {
                setMessage( L"" );
            }
            getJumper()->enablePhase( false );
        }
    }
}

/**
 * countdown routine
 */

Script::Countdown::Countdown(Jumper* jumper, CatToy* catToy) :
    Script::Message( jumper, L"", 1.0f )
{
    assert( catToy );
    assert( catToy->getTimeToJump() > 0 );
    _catToy = catToy;
    setLock( true );
}

void Script::Countdown::onUpdateRoutine(float dt)
{
    // base behaviour
    Message::onUpdateRoutine( dt );
        
    float timeToJump = _catToy->getTimeToJump();

    if( timeToJump < 10.0f )
    {
        if( timeToJump == 0.0f )
        {
            setMessage( Gameplay::iLanguage->getUnicodeString(531) );
            setLock( false );
        }
        else if( timeToJump <= 1.0f )
        {
            setMessage( Gameplay::iLanguage->getUnicodeString(530) );
        }
        else if( timeToJump <= 2.0f )
        {
            setMessage( Gameplay::iLanguage->getUnicodeString(529) );
        }        
        else
        {
            setMessage( Gameplay::iLanguage->getUnicodeString(528) );
        }
    }
    else
    {
        setMessage( L"" );
    }
}

/**
 * timer routine
 */

Script::Timer::Timer(Jumper* jumper) : Script::Message( jumper, L"", 1.0f )
{
    _time = 0.0f;
    setLock( true );
}
    
void Script::Timer::onUpdateRoutine(float dt)
{
    // base behaviour
    Message::onUpdateRoutine( dt );

    _time += dt;
    setMessage( wstrformat( L"%2.1fss", _time ).c_str() );
}

/**
 * "see you" routine
 */

Script::SeeYou::SeeYou(Jumper* jumper, NPCL& npcs) : Message( jumper, L"", 1.0f )
{
    _npcs = npcs;
    _dubbingNPC = NULL;
    setLock( true );
}

void Script::SeeYou::onUpdateRoutine(float dt)
{
    // base behaviour
    Message::onUpdateRoutine( dt );

    if( isLocked() )
    {
        // first pass : search for NPCs, prepared to jump and dubbing it
        for( NPCI npcI = _npcs.begin(); npcI != _npcs.end(); npcI++ )
        {
            float timeToJump = (*npcI)->getCatToy()->getTimeToJump();
            if( ( timeToJump > 0.0f ) && ( timeToJump < 1.0f ) )
            {
                _dubbingNPC = (*npcI);
                setMessage( wstrformat( Gameplay::iLanguage->getUnicodeString(591), _dubbingNPC->getNPCName() ).c_str() );
                _npcs.erase( npcI );
                break;
            }
        }

        // second pass : check if debbing NPC is jumped
        if( _dubbingNPC && _dubbingNPC->getJumper()->getPhase() != ::jpRoaming )
        {
            _dubbingNPC = NULL;
            setMessage( L"" );
        }

        // third pass, complete routine
        if( _npcs.size() == 0 && _dubbingNPC == NULL )
        {
            setLock( false );
        }
    }
}

/**
 * Script
 */

Script::Script(Jumper* jumper) : Actor( jumper )
{
    _jumper  = jumper;
    _routine = NULL;
}

Script::~Script()
{
    if( _routine ) delete _routine;
}

void Script::onUpdatePhysics()
{
}

void Script::onUpdateActivity(float dt)
{    
    // update current order
    if( _routine ) _routine->onUpdateRoutine( dt );   
}

void Script::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
}