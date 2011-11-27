
#include "headers.h"
#include "career.h"
#include "gameplay.h"
#include "crypt.h"
#include "salary.h"
#include "weatherforecast.h"
#include "divine.h"
#include "night.h"
#include "database.h"
#include "bitfield.h"
#include "../shared/ccor.h"

/**
 * class implementation
 */

Career::Career(const char* name)
{
    _name = name;
    _isHomeDefined = false;
    _homeX = _homeY = 0;
    _eventCallback = NULL;
    _eventCallbackData = NULL;

    // initialize game walk-through meter
    initializeWalkthroughMeter();

    // initialize virtues
    _virtues.evolution.funds = _virtues.getSalary();

    // generate beginning helmet, suit and rig
    _virtues.equipment.helmet = Gear( gtHelmet, 0 );
    _virtues.equipment.helmet.state = 1;
    _virtues.equipment.helmet.age   = 0;
    _virtues.equipment.suit   = Gear( gtSuit, 0 );
    _virtues.equipment.suit.state = 1;
    _virtues.equipment.suit.age   = 0;
    _virtues.equipment.rig    = Gear( gtRig, 0 );
    _virtues.equipment.rig.state = 1;
    _virtues.equipment.rig.age   = 0;
	_virtues.equipment.rig.rig_aad = 0;

    // generate skydiving canopy
    _virtues.equipment.canopy = Gear( gtCanopy, 19 );
    _virtues.equipment.canopy.state = 1;
    _virtues.equipment.canopy.age   = 0;
    _virtues.equipment.sliderOption = ::soUp;
    
    // generate skydiving reserve
    _virtues.equipment.reserve = Gear( gtReserve, 0 );
    _virtues.equipment.reserve.state = 1;
    _virtues.equipment.reserve.age   = 0;

    // initialize events
    _events.push_back( new RegularWork( this, true ) );
    _events.push_back( new WeatherForecast( this ) );
    _events.push_back( new Divine( this ) );
    _events.push_back( new Night( this, true ) );
}

Career::Career(TiXmlElement* node)
{
    assert( strcmp( node->Value(), "career" ) == 0 );

    _name = node->Attribute( "name" );

    _isHomeDefined = false;
    _homeX = _homeY = 0;
    _eventCallback = NULL;
    _eventCallbackData = NULL;

    TiXmlNode* child = node->FirstChild(); assert( child );
    if( child != NULL ) do 
    {
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "virtues" ) == 0 )
        {
            int cs;
            static_cast<TiXmlElement*>( child )->Attribute( "checksum", &cs );
            std::string data = static_cast<TiXmlElement*>( child )->Attribute( "data" );
			if (!decrypt( &_virtues, sizeof(Virtues), data, _name.c_str() )) {

				VirtuesLegacy virtuesL;
				decrypt( &virtuesL, sizeof(VirtuesLegacy), data, _name.c_str() );
				
				_virtues.loadLegacy(virtuesL);
				//Virtues *a = NULL;a->appearance.face = 1;
			}
            if( cs != checksum( &_virtues, sizeof(Virtues) ) )
            {
                throw ccor::Exception( "User database entry corrupted: \"%s\"! Cheating not allowed!", _name.c_str() );
            }
        }
        else if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "gears" ) == 0 )
        {
            TiXmlNode* gearNode = child->FirstChild();
            if( gearNode ) do
            {
                assert( gearNode->Type() == TiXmlNode::ELEMENT && strcmp( gearNode->Value(), "gear" ) == 0 );
                int cs;
                static_cast<TiXmlElement*>( gearNode )->Attribute( "checksum", &cs );
                std::string data = static_cast<TiXmlElement*>( gearNode )->Attribute( "data" );
                Gear gear;
                decrypt( &gear, sizeof(Gear), data, _name.c_str() );
                if( cs != checksum( &gear, sizeof(Gear) ) )
                {
                    throw ccor::Exception( "User database entry corrupted: \"%s\"! Cheating not allowed!", _name.c_str() );
                }
                addGear( gear );
                gearNode = gearNode->NextSibling();
            }
            while( gearNode != NULL );
        }
        else if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "home" ) == 0 )
        {
            _isHomeDefined = true;
            static_cast<TiXmlElement*>( child )->Attribute( "x", &_homeX );
            static_cast<TiXmlElement*>( child )->Attribute( "y", &_homeY );
        }
        else if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "events" ) == 0 )
        {
            TiXmlNode* eventNode = child->FirstChild();
            if( eventNode ) do
            {
                if( eventNode->Type() == TiXmlNode::ELEMENT && strcmp( eventNode->Value(), "event" ) == 0 )
                {
                    Event* event = Event::createFromXml( this, static_cast<TiXmlElement*>( eventNode ) );
                    assert( event );
                    _events.push_back( event );
                }
                eventNode = eventNode->NextSibling();
            }
            while( eventNode != NULL );
        }        
        else if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "gamedata" ) == 0 )
        {
            TiXmlNode* entryNode = child->FirstChild();
            if( entryNode ) do
            {
                if( entryNode->Type() == TiXmlNode::ELEMENT && strcmp( entryNode->Value(), "entry" ) == 0 )
                {
                    std::string name = static_cast<TiXmlElement*>( entryNode )->Attribute( "name" );
                    std::string data = static_cast<TiXmlElement*>( entryNode )->Attribute( "data" );
                    int cs;
                    static_cast<TiXmlElement*>( entryNode )->Attribute( "checksum", &cs );
                    GameData* gameData = new GameData( data.length() / 2 );
                    ::decrypt( gameData->getData(), gameData->getSize(), data, _name.c_str() );
                    if( ::checksum( gameData->getData(), gameData->getSize() ) != cs )
                    {
                        throw ccor::Exception( "User database entry corrupted: \"%s\"! Cheating not allowed!", _name.c_str() );
                    }
                    _gameDataM.insert( GameDataT( name, gameData ) );
                }
                entryNode = entryNode->NextSibling();
            }
            while( entryNode != NULL );
        }
        child = child->NextSibling();
    }
    while( child != NULL );

    // TEST_MODE( enable acrobatics )
    
    //setAcrobaticsSkill( acroJumpFromRun, true );
    //setAcrobaticsSkill( acroFreeflyFlip, true );
    //setAcrobaticsSkill( acroFreeflySitfly, true );
    //setAcrobaticsSkill( acroFrontFlip, true );
    //setAcrobaticsSkill( acroFrontBackFlip, true );
    //setAcrobaticsSkill( acroBackFlip, true );
    //setAcrobaticsSkill( acroBackFrontFlip, true );
    

    // initialize game walk-through meter
    initializeWalkthroughMeter();
}

Career::~Career()
{
    // remove game data
    for( GameDataI gameDataI = _gameDataM.begin();
                   gameDataI != _gameDataM.end();
                   gameDataI++ )
    {
        delete gameDataI->second;
    }
    // remove events
    for( unsigned int i=0; i<_events.size(); i++ ) delete _events[i];
}

/**
 * gear management
 */

void Career::addGear(Gear gear)
{
    _gears.push_back( gear );
}

void Career::removeGear(unsigned int id)
{
    assert( id>=0 && id<_gears.size() );
    _gears.erase( &_gears[id] );
}

void Career::equipGear(unsigned int id)
{
    assert( id>=0 && id<_gears.size() );
    Gear temp;
    switch( _gears[id].type )
    {
    case gtHelmet:
        temp = _virtues.equipment.helmet;
        _virtues.equipment.helmet = _gears[id];
        break;
    case gtSuit:
        temp = _virtues.equipment.suit;
        _virtues.equipment.suit = _gears[id];
        break;
    case gtRig:
        temp = _virtues.equipment.rig;
        _virtues.equipment.rig = _gears[id];
        break;
    case gtCanopy:
        temp = _virtues.equipment.canopy;
        _virtues.equipment.canopy = _gears[id];
        break;
    case gtReserve:
        temp = _virtues.equipment.reserve;
        _virtues.equipment.reserve = _gears[id];
        break;
    default:
        assert( !"shouldn't be here!" );
    }
    removeGear( id );
    if( temp.type != gtUnequipped ) addGear( temp );
}
bool Career::equipGear() {
	for (unsigned int i = _gears.size()-1; (signed int)i >= 0 ; --i) {
		// helmet
		if (_virtues.equipment.helmet.type == gtUnequipped && _gears[i].type == gtHelmet) {
			equipGear(i);
		}
		// suit
		else if (_virtues.equipment.suit.type == gtUnequipped && _gears[i].type == gtSuit) {
			equipGear(i);
		}
		// rig
		else if (_virtues.equipment.rig.type == gtUnequipped && _gears[i].type == gtRig) {
			equipGear(i);
		}
		// canopy
		else if (_virtues.equipment.canopy.type == gtUnequipped && _gears[i].type == gtCanopy) {
			equipGear(i);
		}
		// reserve
		else if (_virtues.equipment.reserve.type == gtUnequipped && _gears[i].type == gtReserve) {
			equipGear(i);
		}

		if (i == 0) break;
	}

	return  _virtues.equipment.helmet.type != gtUnequipped &&
			_virtues.equipment.suit.type != gtUnequipped &&
			_virtues.equipment.rig.type != gtUnequipped &&
			_virtues.equipment.canopy.type != gtUnequipped &&
			_virtues.equipment.reserve.type != gtUnequipped;
}

bool Career::unequipGear(GearType type) {
	Gear temp;
    switch( type )
    {
    case gtHelmet:
        temp = _virtues.equipment.helmet;
        _virtues.equipment.helmet.id = 0;
		_virtues.equipment.helmet.type = gtUnequipped;
        break;
    case gtSuit:
        temp = _virtues.equipment.suit;
        _virtues.equipment.suit.id = 0;
		_virtues.equipment.suit.type = gtUnequipped;
        break;
    case gtRig:
        temp = _virtues.equipment.rig;
        _virtues.equipment.rig.id = 0;
		_virtues.equipment.rig.type = gtUnequipped;
        break;
    case gtCanopy:
        temp = _virtues.equipment.canopy;
        _virtues.equipment.canopy.id = 0;
		_virtues.equipment.canopy.type = gtUnequipped;
        break;
    case gtReserve:
        temp = _virtues.equipment.reserve;
        _virtues.equipment.reserve.id = 0;
		_virtues.equipment.reserve.type = gtUnequipped;
        break;
    default:
        assert( !"shouldn't be here!" );
    }

	if (temp.type != gtUnequipped) {
		this->addGear(temp);
		return true;
	} else {
		return false;
	}
}

bool Career::unequipGear() {
	return 
		this->unequipGear(gtHelmet) ||
		this->unequipGear(gtSuit)	||
		this->unequipGear(gtRig)	||
		this->unequipGear(gtCanopy) ||
		this->unequipGear(gtReserve);
}
/**
 * event management
 */

void Career::addEvent(Event* event)
{
    _events.push_back( event );
    if( _eventCallback ) _eventCallback( event, _eventCallbackData );
}

void Career::removeEvent(Event* event)
{
    for( unsigned int i=0; i<_events.size(); i++ )
    {
        if( _events[i] == event )
        {
            _events.erase( &_events[i] );
            delete event;
            return;
        }
    }
}

void Career::setEventCallback(EventCallback eventCallback, void* data)
{
    _eventCallback = eventCallback;
    _eventCallbackData = data;
}

/**
 * game data management
 */

void Career::addGameData(const char* name, GameData* gameData)
{
    _gameDataM.insert( GameDataT( name, gameData ) );
}

GameData* Career::getGameData(const char* name)
{
    GameDataI gameDataI = _gameDataM.find( name );
    if( gameDataI == _gameDataM.end() ) return NULL;
    return gameDataI->second;
}

/**
 * career management
 */

void Career::setHome(int homeX, int homeY)
{
    _isHomeDefined = true;
    _homeX = homeX;
    _homeY = homeY;
}
    
void Career::save(TiXmlDocument* xmlDocument)
{
    unsigned int i;
    std::string data;

    // create career node
    TiXmlElement* careerElement = new TiXmlElement( "career" );
    careerElement->SetAttribute( "name", _name.c_str() );

    // create virtues node        
    TiXmlElement* virtuesElement = new TiXmlElement( "virtues" );
    virtuesElement->SetAttribute( "checksum", checksum( &_virtues, sizeof(Virtues) ) );
    encrypt( data, &_virtues, sizeof(Virtues), _name.c_str() );
    virtuesElement->SetAttribute( "data", data.c_str() );
    careerElement->InsertEndChild( *virtuesElement );
    delete virtuesElement;

    // save gears
    TiXmlElement* gearsGroupElement = new TiXmlElement( "gears" );
    for( i=0; i<_gears.size(); i++ )
    {
        Gear gear = _gears[i];
        TiXmlElement* gearElement = new TiXmlElement( "gear" );
        gearElement->SetAttribute( "checksum", checksum( &gear, sizeof(Gear) ) );
        encrypt( data, &gear, sizeof(Gear), _name.c_str() );
        gearElement->SetAttribute( "data", data.c_str() );
        gearsGroupElement->InsertEndChild( *gearElement );
        delete gearElement;
    }
    careerElement->InsertEndChild( *gearsGroupElement );
    delete gearsGroupElement;

    // save home
    if( _isHomeDefined )
    {
        TiXmlElement* homeElement = new TiXmlElement( "home" );
        homeElement->SetAttribute( "x", _homeX );
        homeElement->SetAttribute( "y", _homeY );
        careerElement->InsertEndChild( *homeElement );
        delete homeElement;
    }

    // save events
    if( _events.size() )
    {
        TiXmlElement eventsGroup( "events" );
        for( unsigned int i=0; i<_events.size(); i++ )
        {
            _events[i]->saveToXml( &eventsGroup );
        }
        careerElement->InsertEndChild( eventsGroup );
    }    

    // save game data
    if( _gameDataM.size() )
    {
        TiXmlElement gameDataGroup( "gamedata" );
        for( GameDataI gameDataI = _gameDataM.begin();
                       gameDataI != _gameDataM.end();
                       gameDataI++ )
        {
            int cs = ::checksum( gameDataI->second->getData(), gameDataI->second->getSize() );
            std::string data;
            ::encrypt( data, gameDataI->second->getData(), gameDataI->second->getSize(), _name.c_str() );
            TiXmlElement entryElement( "entry" );
            entryElement.SetAttribute( "name", gameDataI->first.c_str() );
            entryElement.SetAttribute( "checksum", cs );
            entryElement.SetAttribute( "data", data.c_str() );
            gameDataGroup.InsertEndChild( entryElement );
        }
        careerElement->InsertEndChild( gameDataGroup );
    }

    xmlDocument->InsertEndChild( *careerElement );
    delete careerElement;
}

/**
 * walkthrought meter
 */

void Career::initializeWalkthroughMeter(void)
{
    // for all tournaments...
    for( unsigned int i=0; i<database::TournamentInfo::getNumRecords(); i++ )
    {
        // except for demo tournament
        if( i == database::TournamentInfo::getDemoTournament() ) continue;

        // retrieve tournament record
        database::TournamentInfo* tournamentInfo = database::TournamentInfo::getRecord( i );
        assert( tournamentInfo->gameData );

        // check there is gamedata record for "tournament walkthrough"
        GameData* gameData = getGameData( tournamentInfo->gameData );
        if( !gameData )
        {
            // create gamedata for tournament & reset its content
            gameData = new GameData( sizeof( Bitfield32 ) );
            memset( gameData->getData(), 0, gameData->getSize() );
            addGameData( tournamentInfo->gameData, gameData );
        }
    }
}

float Career::getWalkthroughPercent(void)
{
    unsigned int numMissions = 0;
    unsigned int numCompletedMissions = 0;

    unsigned int i,j;

    // for all tournaments...
    for( i=0; i<database::TournamentInfo::getNumRecords(); i++ )
    {
        // except for demo tournament
        if( i == database::TournamentInfo::getDemoTournament() ) continue;

        // retrieve tournament record
        database::TournamentInfo* tournamentInfo = database::TournamentInfo::getRecord( i );

        // for all tournament missions
        for( j=0; j<tournamentInfo->getNumMissions(); j++ )
        {
            numMissions++;
            if( getMissionWalkthroughFlag( i, j ) ) numCompletedMissions++;
        }
    }

    return ( float( numCompletedMissions ) / float( numMissions ) );
}

bool Career::getMissionWalkthroughFlag(unsigned int tournamentId, unsigned int missionId)
{
    // this feature is not available for demo tournament
    if( tournamentId == database::TournamentInfo::getDemoTournament() ) return false;

    // retrieve tournament record
    database::TournamentInfo* tournamentInfo = database::TournamentInfo::getRecord( tournamentId );

    // check mission index is valid
    assert( missionId < tournamentInfo->getNumMissions() );

    // retrieve correspoding gamedata
    GameData* gameData = getGameData( tournamentInfo->gameData ); assert( gameData );
    Bitfield32* bitfield = reinterpret_cast<Bitfield32*>( gameData->getData() );

    // return result
    return ( bitfield->getBit( missionId ) != 0 );
}

void Career::setMissionWalkthroughFlag(unsigned int tournamentId, unsigned int missionId, bool flag)
{
    // this feature is not available for demo tournament
    if( tournamentId == database::TournamentInfo::getDemoTournament() ) return;

    // retrieve tournament record
    database::TournamentInfo* tournamentInfo = database::TournamentInfo::getRecord( tournamentId );

    // check mission index is valid
    assert( missionId < tournamentInfo->getNumMissions() );

    // retrieve correspoding gamedata
    GameData* gameData = getGameData( tournamentInfo->gameData ); assert( gameData );
    Bitfield32* bitfield = reinterpret_cast<Bitfield32*>( gameData->getData() );

    // save result
    bitfield->setBit( missionId, flag );
}

/**
 * baggage of acrobatics 
 */

const char* actobaticsGameData = "ACRO";

bool Career::getAcrobaticsSkill(Acrobatics skill)
{
    GameData* gameData = getGameData( actobaticsGameData );
    if( !gameData )
    {
        // create gamedata for acrobatics and reset its contents
        gameData = new GameData( sizeof(unsigned int) );
        memset( gameData->getData(), 0, gameData->getSize() );
        addGameData( actobaticsGameData, gameData );
    }
    unsigned int bitset = *reinterpret_cast<unsigned int*>( gameData->getData() );
    return ( bitset & skill ) != 0;
}

void Career::setAcrobaticsSkill(Acrobatics skill, bool isLearned)
{
    GameData* gameData = getGameData( actobaticsGameData );
    if( !gameData )
    {
        // create gamedata for acrobatics and reset its contents
        gameData = new GameData( sizeof(unsigned int) );
        memset( gameData->getData(), 0, gameData->getSize() );
        addGameData( actobaticsGameData, gameData );
    }

    // modify flag
    unsigned int bitset = *reinterpret_cast<unsigned int*>( gameData->getData() );
    if( isLearned ) bitset = bitset | skill; else bitset = bitset & ~skill;
    memcpy( gameData->getData(), &bitset, sizeof(unsigned int) );
}

/**
 * other properties
 */

bool Career::getLicensedFlag(void)
{
    GameData* gameData = getGameData( "LCF" );
    if( !gameData )
    {
        gameData = new GameData( sizeof(bool) );
        *reinterpret_cast<bool*>(gameData->getData()) = false;
        addGameData( "LCF", gameData );
    }
    return *reinterpret_cast<bool*>(gameData->getData());
}

void Career::setLicensedFlag(bool flag)
{
    getLicensedFlag();
    GameData* gameData = getGameData( "LCF" );
    *reinterpret_cast<bool*>(gameData->getData()) = flag;
}

bool Career::getHomePlacementFlag(void)
{
    GameData* gameData = getGameData( "HPF" );
    if( !gameData )
    {
        gameData = new GameData( sizeof(bool) );
        *reinterpret_cast<bool*>(gameData->getData()) = false;
        addGameData( "HPF", gameData );
    }
    return *reinterpret_cast<bool*>(gameData->getData());
}

void Career::setHomePlacementFlag(bool flag)
{
    getHomePlacementFlag();
    GameData* gameData = getGameData( "HPF" );
    *reinterpret_cast<bool*>(gameData->getData()) = flag;
}