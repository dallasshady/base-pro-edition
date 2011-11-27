
#include "headers.h"
#include "equip.h"
#include "imath.h"
#include "database.h"
#include "mission.h"

/**
 * helpers
 */

// not complete
#define BEST_IS_LAST 1			// 0 - select best equipment (default behaviour); 1 - select last used equipment

static void equipBestBASERig(Career* career)
{
    assert( career );
    
    Virtues* virtues = career->getVirtues();
    assert( virtues->equipment.rig.type == gtRig );
    assert( virtues->equipment.canopy.type == gtCanopy );

    // build list of BASE rigs
    std::vector<unsigned int> rigs;
    for( unsigned int i=0; i<career->getNumGears(); i++ )
    {
        if( career->getGear( i ).type == gtRig ) 
        {
            if( !database::Rig::getRecord( career->getGear( i ).id )->skydiving )
            {
                rigs.push_back( i );
            }
        }
    }

    // select best rig
    if( rigs.size() )
    {
        unsigned int bestRig = 0;    
        float bestState = career->getGear( rigs[bestRig] ).state;

        for( i=1; i<rigs.size(); i++ )
        {
            if( career->getGear( rigs[i] ).state > bestState )
            {
                bestRig = i;
                bestState = career->getGear( rigs[i] ).state;
            }
        }

        // equip best rig
        career->equipGear( rigs[bestRig] );
    }
}

static void equipBestSkydivingRig(Career* career)
{
    assert( career );
    
    Virtues* virtues = career->getVirtues();
    assert( virtues->equipment.rig.type == gtRig );
    assert( virtues->equipment.canopy.type == gtCanopy );

    // build list of skydiving rigs
    std::vector<unsigned int> rigs;
    for( unsigned int i=0; i<career->getNumGears(); i++ )
    {
        if( career->getGear( i ).type == gtRig )
        {
            if( database::Rig::getRecord( career->getGear( i ).id )->skydiving )
            {
                rigs.push_back( i );
            }
        }
    }

    // select best rig
    if( rigs.size() )
    {
        unsigned int bestRig = 0;    
        float bestState = career->getGear( rigs[bestRig] ).state;

        for( i=1; i<rigs.size(); i++ )
        {
            if( career->getGear( rigs[i] ).state > bestState )
            {
                bestRig = i;
                bestState = career->getGear( rigs[i] ).state;
            }
        }

        // equip best rig
        career->equipGear( rigs[bestRig] );
    }
}

static void equipBestBASECanopy(Career* career, float windAmbient, float windBlast)
{
    assert( career );
    
    Virtues* virtues = career->getVirtues();
    assert( virtues->equipment.rig.type == gtRig );
    assert( virtues->equipment.canopy.type == gtCanopy );

    // determine optimal canopy square
    float optimalSquare = database::Canopy::getOptimalCanopySquare( 
        virtues->appearance.weight,
        0.5f * ( windAmbient + windBlast )
    );

    // build list of canopies
    std::vector<unsigned int> canopies;
    for( unsigned int i=0; i<career->getNumGears(); i++ )
    {
        if( career->getGear( i ).type == gtCanopy )
        {
            if( !database::Canopy::getRecord( career->getGear( i ).id )->skydiving )
            {
                canopies.push_back( i );
            }
        }
    }

    // select best canopy by optimal square and current gear state
    if( canopies.size() )
    {
        unsigned int bestIndex = 0;
        Gear canopy = career->getGear( canopies[bestIndex] );
        float bestEpsilon = fabs( database::Canopy::getRecord( canopy.id )->square - optimalSquare );

        for( i=1; i<canopies.size(); i++ )
        {
            canopy = career->getGear( canopies[i] );
            float extraEpsilon = fabs( database::Canopy::getRecord( canopy.id )->square - optimalSquare );
            // first, check the epsilon value
            if( BEST_IS_LAST || extraEpsilon <= bestEpsilon )
            {                
                bestIndex = i;
                bestEpsilon = extraEpsilon;
				break;
            }
        }

        // currently equipped canopy is a BASE canopy?
        if( !database::Canopy::getRecord( virtues->equipment.canopy.id )->skydiving )
        {
            // check current canopy epsilon and state
            float currentEpsilon = fabs( database::Canopy::getRecord( virtues->equipment.canopy.id )->square - optimalSquare );
            bool currentCanopyIsBestCanopy = false;
            if( currentEpsilon <= bestEpsilon )
            {
                currentCanopyIsBestCanopy = true;
            }
            if( !currentCanopyIsBestCanopy )
            {
                // equip best canopy
                career->equipGear( canopies[bestIndex] );
            }
        }
        else
        {
            // equip best canopy
            career->equipGear( canopies[bestIndex] );
        }
    }
}


static void equipBestSkydivingCanopy(Career* career, float windAmbient, float windBlast)
{
    assert( career );
    
    Virtues* virtues = career->getVirtues();
    assert( virtues->equipment.rig.type == gtRig );
    assert( virtues->equipment.canopy.type == gtCanopy );

    // determine optimal canopy square (skydiving case)
    float optimalSquare = 250.0f;
    if( virtues->appearance.weight < 75 ) optimalSquare = 220.0f;    

    // build list of canopies
    std::vector<unsigned int> canopies;
    for( unsigned int i=0; i<career->getNumGears(); i++ )
    {
        if( career->getGear( i ).type == gtCanopy )
        {
            if( database::Canopy::getRecord( career->getGear( i ).id )->skydiving )
            {
                canopies.push_back( i );
            }
        }
    }

    // select best canopy by optimal square and current gear state
    if( canopies.size() )
    {
        unsigned int bestIndex = 0;
        Gear canopy = career->getGear( canopies[bestIndex] );
        float bestEpsilon = fabs( database::Canopy::getRecord( canopy.id )->square - optimalSquare );

        for( i=1; i<canopies.size(); i++ )
        {
            canopy = career->getGear( canopies[i] );
            float extraState = canopy.state;
            float extraEpsilon = fabs( database::Canopy::getRecord( canopy.id )->square - optimalSquare );
            // first, check the epsilon value
            if( extraEpsilon <= bestEpsilon )
            {
                // second, check the state value
                bestIndex = i;
                bestEpsilon = extraEpsilon;                
            }
        }

        // currently equipped canopy is a skydiving canopy?
        if( database::Canopy::getRecord( virtues->equipment.canopy.id )->skydiving )
        {
            // check current canopy epsilon and state
            float currentState = virtues->equipment.canopy.state;
            float currentEpsilon = fabs( database::Canopy::getRecord( virtues->equipment.canopy.id )->square - optimalSquare );
            bool currentCanopyIsBestCanopy = false;
            if( currentEpsilon <= bestEpsilon )
            {
                currentCanopyIsBestCanopy = true;
            }
            if( !currentCanopyIsBestCanopy )
            {
                // equip best canopy
                career->equipGear( canopies[bestIndex] );
            }
        }
        else
        {
            // equip best canopy
            career->equipGear( canopies[bestIndex] );
        }
    }
}

/**
 * common callbacks
 */

bool equipBestBASEEquipment(Career* career, float windAmbient, float windBlast)
{
    assert( career );
    
    Virtues* virtues = career->getVirtues();

	if (!career->equipGear()) {
		return false;
	}

    // first, check if there is any BASE rig owned by player
    bool flag = false;
    if( !database::Rig::getRecord( virtues->equipment.rig.id )->skydiving )
    {
        flag = true;
    }
    else
    {
        Gear gear;
        for( unsigned int i=0; i<career->getNumGears(); i++ )
        {
            gear = career->getGear( i );
            if( gear.type == gtRig )
            {
                if( !database::Rig::getRecord( gear.id )->skydiving )
                {
                    flag = true;
                    break;
                }
            }
        }
    }
    // if player has no BASE rig - fail immediately
    if( !flag ) return false;

    // second, check if there is any BASE canopy owned by player
    flag = false;
    if( !database::Canopy::getRecord( virtues->equipment.canopy.id )->skydiving )
    {
        flag = true;
    }
    else
    {
        Gear gear;
        for( unsigned int i=0; i<career->getNumGears(); i++ )
        {
            gear = career->getGear( i );
            if( gear.type == gtCanopy )
            {
                if( !database::Canopy::getRecord( gear.id )->skydiving )
                {
                    flag = true;
                    break;
                }
            }
        }
    }
    // if player has no BASE canopy - fail immediately
    if( !flag ) return false;

    // if current rig is BASE rig
    if( !database::Rig::getRecord( virtues->equipment.rig.id )->skydiving )
    {
        // check rig state treshold
        if( virtues->equipment.rig.state < 0.75f )
        {
            // equip best rig
            equipBestBASERig( career );
        }
    }
    else
    {
        // equip best rig
        equipBestBASERig( career );
    }

    // equip best canopy for specifiend weather
    equipBestBASECanopy( career, windAmbient, windBlast );

    return true;
}

bool equipBestSkydivingEquipment(Career* career, float windAmbient, float windBlast)
{
    assert( career );
    
    Virtues* virtues = career->getVirtues();
	if (!career->equipGear()) {
		return false;
	}

    assert( virtues->equipment.suit.type == gtSuit );

    // first, check if there is any skydiving rig owned by player
    bool flag = false;
    if( database::Rig::getRecord( virtues->equipment.rig.id )->skydiving )
    {
        flag = true;
    }
    else
    {
        Gear gear;
        for( unsigned int i=0; i<career->getNumGears(); i++ )
        {
            gear = career->getGear( i );
            if( gear.type == gtRig )
            {
                if( database::Rig::getRecord( gear.id )->skydiving )
                {
                    flag = true;
                    break;
                }
            }
        }
    }
    // if player has no skydiving rig - fail immediately
    if( !flag ) return false;

    // second, check if there is any skydiving canopy owned by player
    flag = false;
    if( database::Canopy::getRecord( virtues->equipment.canopy.id )->skydiving )
    {
        flag = true;
    }
    else
    {
        Gear gear;
        for( unsigned int i=0; i<career->getNumGears(); i++ )
        {
            gear = career->getGear( i );
            if( gear.type == gtCanopy )
            {
                if( database::Canopy::getRecord( gear.id )->skydiving )
                {
                    flag = true;
                    break;
                }
            }
        }
    }
    // if player has no skydiving canopy - fail immediately
    if( !flag ) return false;

    // if current rig is skydiving rig
    if( database::Rig::getRecord( virtues->equipment.rig.id )->skydiving )
    {
        // check rig state treshold
        if( virtues->equipment.rig.state < 0.75f )
        {
            // equip best rig
            equipBestSkydivingRig( career );
        }
    }
    else
    {
        // equip best rig
        equipBestSkydivingRig( career );
    }

    // equip best canopy for specifiend weather
    equipBestSkydivingCanopy( career, windAmbient, windBlast );

    return true;
}

bool equipBestSuit(Career* career, float windAmbient, float windBlast)
{
    assert( career );
    
    Virtues* virtues = career->getVirtues();

    // build list of suits
    std::vector<unsigned int> suits;
    for( unsigned int i=0; i<career->getNumGears(); i++ )
    {
        if( career->getGear( i ).type == gtSuit )
        {
            if( !database::Suit::getRecord( career->getGear( i ).id )->wingsuit )
            {
                suits.push_back( i );
            }
        }
    }

    // no wingsuits?
    if( suits.size() == 0 )
    {
        // check the suit is equipped
        return !database::Suit::getRecord( virtues->equipment.suit.id )->wingsuit;
    }
    else
    {
        // select suit with best state value
        unsigned int bestIndex = 0;
        float bestState = career->getGear( suits[bestIndex] ).state;
        for( i=1; i<suits.size(); i++ )
        {
            float extraState = career->getGear( suits[i] ).state;
            if( extraState > bestState )
            {
                bestState = extraState;
                bestIndex = i;
            }
        }

        // check the suit is currently equipped
        if( !database::Suit::getRecord( virtues->equipment.suit.id )->wingsuit )
        {
            if( virtues->equipment.suit.state < bestState )
            {
                career->equipGear( suits[bestIndex] );
            }
        }
        else
        {
            career->equipGear( suits[bestIndex] );
        }

        return true;
    }
}

bool equipBestWingsuit(Career* career, float windAmbient, float windBlast)
{
    assert( career );
    
    Virtues* virtues = career->getVirtues();

    // build list of wing suits
    std::vector<unsigned int> wingsuits;
    for( unsigned int i=0; i<career->getNumGears(); i++ )
    {
        if( career->getGear( i ).type == gtSuit )
        {
            if( database::Suit::getRecord( career->getGear( i ).id )->wingsuit )
            {
                wingsuits.push_back( i );
            }
        }
    }

    // no wingsuits?
    if( wingsuits.size() == 0 )
    {
        // check the wing suit is equipped
        return database::Suit::getRecord( virtues->equipment.suit.id )->wingsuit;
    }
    else
    {
        // select wing suit with best state value
        unsigned int bestIndex = 0;
        float bestState = career->getGear( wingsuits[bestIndex] ).state;
        for( i=1; i<wingsuits.size(); i++ )
        {
            float extraState = career->getGear( wingsuits[i] ).state;
            if( extraState > bestState )
            {
                bestState = extraState;
                bestIndex = i;
            }
        }

        // check the wing suit is currently equipped
        if( database::Suit::getRecord( virtues->equipment.suit.id )->wingsuit )
        {
            if( virtues->equipment.suit.state < bestState )
            {
                career->equipGear( wingsuits[bestIndex] );
            }
        }
        else
        {
            career->equipGear( wingsuits[bestIndex] );
        }

        return true;
    }
}