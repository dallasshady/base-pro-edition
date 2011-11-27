
#include "headers.h"
#include "database.h"

static database::NPCInfo npcs[] =
{
    // LICENSED_CHAR (Valery Rosov, Felix Baumgartner etc.)
    { 265, 1.0, 179.0f, 76.0f, 0, 0.75f, 0.75f, 0.75f, 0.75f },
    // fresh fish (level = 0)
    { 257, 0, 185.0f, 80.0f, 2, 0.0f, 0.0f, 0.0f, 0.0f },
    { 258, 0, 176.0f, 69.0f, 3, 0.0f, 0.0f, 0.0f, 0.0f },
    // newbies (level = 0.25)
    { 259, 0.25, 179.0f, 76.0f, 4, 0.125f, 0.125f, 0.125f, 0.125f },
    { 260, 0.25, 187.0f, 90.0f, 3, 0.125f, 0.125f, 0.125f, 0.125f },
    // averages (level = 0.5)
    { 261, 0.5, 178.0f, 68.0f, 5, 0.25f, 0.25f, 0.25f, 0.25f },
    { 262, 0.5, 183.0f, 73.0f, 4, 0.25f, 0.25f, 0.25f, 0.25f },
    // masters (level = 0.75)
    { 263, 0.75, 177.0f, 68.0f, 6, 0.5f, 0.5f, 0.5f, 0.5f },
    { 264, 0.75, 183.0f, 73.0f, 7, 0.5f, 0.5f, 0.5f, 0.5f },
    // legends (level = 1.0)    
    { 266, 1.0, 182.0f, 81.0f, 3, 0.75f, 0.75f, 0.75f, 0.75f },
    { 267, 1.0, 182.0f, 81.0f, 2, 0.75f, 0.75f, 0.75f, 0.75f },
    // end of NPC list
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

/**
 * expandable list of NPCs
 */

static bool                           exNPCsIsActual = false;
static std::vector<database::NPCInfo> exNPCs;

static void buildExNPCs(void)
{
    if( !exNPCsIsActual )
    {
        assert( exNPCs.size() == 0 );

        unsigned int i = 0;
        while( npcs[i].nameId != 0 )
        {
            exNPCs.push_back( npcs[i] );
            i++;
        }

        exNPCsIsActual = true;
    }
}

/**
 * database access
 */

unsigned int database::NPCInfo::getNumRecords(void)
{
    buildExNPCs();
    return exNPCs.size();
}

database::NPCInfo* database::NPCInfo::getRecord(unsigned int id)
{
    assert( id >= 0 && id < getNumRecords() );
    buildExNPCs();
    return &exNPCs[id];
}

unsigned int database::NPCInfo::getLicensedCharacterId(void)
{
    // 0 is id of LICENSED_CHAR
    return 0;
}

void database::NPCInfo::select(float level, float epsilon, bool licensed, std::vector<unsigned int>& result)
{
    result.clear();

    for( unsigned int i=0; i<getNumRecords(); i++ )
    {
        if( i == getLicensedCharacterId() && !licensed ) continue;
        if( true || fabs( level - getRecord( i )->level ) <= fabs( epsilon ) )
        {
            result.push_back( i );
        }
    }
}

unsigned int database::NPCInfo::getRandomNonLicensedCharacter(float level, float epsilon)
{
    // select characters
    std::vector<unsigned int> selection;
    select( level, epsilon, false, selection );
    assert( selection.size() );

    // return random id
    return selection[ getCore()->getRandToolkit()->getUniformInt() % selection.size() ];
}

void database::NPCInfo::addRecord(database::NPCInfo* npc)
{
    buildExNPCs();
    exNPCs.push_back( *npc );
}