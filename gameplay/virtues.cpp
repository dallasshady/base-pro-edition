
#include "headers.h"
#include "gameplay.h"

static unsigned int Kentries = 10;

static float Kfrog[10][2] = 
{
    { 165.0f, 0.2184f },
    { 170.0f, 0.2352f },
    { 175.0f, 0.252f },
    { 180.0f, 0.2688f },
    { 185.0f, 0.2856f },
    { 190.0f, 0.3024f },
    { 195.0f, 0.31919f },
    { 200.0f, 0.336f },
    { 205.0f, 0.3528f },
    { 210.0f, 0.3696f }
};

static float Ktrack[10][2] = 
{
    { 165.0f, 0.1769f },
    { 170.0f, 0.1905f },
    { 175.0f, 0.20415f },
    { 180.0f, 0.21775f },
    { 185.0f, 0.23135f },
    { 190.0f, 0.245f },
   { 195.0f, 0.2585f },
    { 200.0f, 0.2722f },
    { 205.0f, 0.2857f },
    { 210.0f, 0.2994f }
};

static float Kheadfirst[10][2] = 
{
    { 165.0f, 0.13775f },
    { 170.0f, 0.14835f },
    { 175.0f, 0.15895f },
    { 180.0f, 0.16955f },
    { 185.0f, 0.18f },
    { 190.0f, 0.1907f },
    { 195.0f, 0.2013f },
    { 200.0f, 0.21159f },
    { 205.0f, 0.2225f },
    { 210.0f, 0.2331f }
};

static float salaryByPromotion[] =
{
    /* 0 */ 200.0f,
    /* 1 */ 250.0f,
    /* 2 */ 300.0f,
    /* 3 */ 350.0f,
    /* 4 */ 400.0f,
    /* 5 */ 500.0f,
    /* 6 */ 600.0f,
    /* 7 */ 700.0f,
    /* 8 */ 800.0f,
    /* 9 */ 1000.0f
};

const float Kperception = 0.0125f; // coefficient of evolution for perception skill
const float Kendurance  = 0.0125f; // coefficient of evolution for endurance skill
const float Ktracking   = 0.0125f; // coefficient of evolution for tracking skill
const float Krigging    = 0.0125f; // coefficient of evolution for rigging skill

/**
 * procedural properties: skills & relatives
 */


// load original Virtues structure
bool Virtues::loadLegacy (VirtuesLegacy &a) {
	// just hard copy unchanged structures
	memcpy(&this->appearance,	&a.appearance,	sizeof(Virtues::Appearance));
	memcpy(&this->evolution,	&a.evolution,	sizeof(Virtues::Evolution));
	memcpy(&this->predisp,		&a.predisp,		sizeof(Virtues::Skills));
	memcpy(&this->skills,		&a.skills,		sizeof(Virtues::Skills));

	// statistics
	this->statistics.numBaseJumps = a.statistics.numBaseJumps;
	this->statistics.numSkydives = a.statistics.numSkydives;
	this->statistics.playingTime = a.statistics.playingTime;
	this->statistics.canopyTime = 0.0f;
	this->statistics.wingsuitTime = 0.0f;
	this->statistics.freeFallTime = 0.0f;

	this->equipment.experience = a.equipment.experience;
	this->equipment.malfunctions = a.equipment.malfunctions;
	this->equipment.pilotchute = a.equipment.pilotchute;

	// copy equipment stuff
	this->equipment.canopy.loadLegacy(a.equipment.canopy);
	this->equipment.helmet.loadLegacy(a.equipment.helmet);
	this->equipment.rig.loadLegacy(a.equipment.rig);
	this->equipment.suit.loadLegacy(a.equipment.suit);
	this->equipment.sliderOption = a.equipment.sliderOption;

	// new elements
	this->equipment.reserve = Gear(gtReserve, 0);

	return true;
}


float Virtues::getPromotion(void)
{
    return 10.0f * ( 1.0f - 1.0f / ( 1.0f + 0.01f * evolution.workTime ) );
}

float Virtues::getSalary(void)
{
    unsigned int promotion = unsigned int( getPromotion() );
    promotion = promotion > 9 ? 9 : promotion;
    return salaryByPromotion[promotion];
}

const float rank0  = 0;
const float rank1  = rank0 + 100;
const float rank2  = rank1 + 250;
const float rank3  = rank2 + 500;
const float rank4  = rank3 + 1000;
const float rank5  = rank4 + 2500;
const float rank6  = rank5 + 5000;
const float rank7  = rank6 + 7500;
const float rank8  = rank7 + 10000;
const float rank9  = rank8 + 15000;
const float rank10 = rank9 + 25000;

unsigned int Virtues::getRank(void)
{
    if( evolution.score < rank1 ) return 0;
    else if( evolution.score < rank2 ) return 1;
    else if( evolution.score < rank3 ) return 2;
    else if( evolution.score < rank4 ) return 3;
    else if( evolution.score < rank5 ) return 4;
    else if( evolution.score < rank6 ) return 5;
    else if( evolution.score < rank7 ) return 6;
    else if( evolution.score < rank8 ) return 7;
    else if( evolution.score < rank9 ) return 8;
    else if( evolution.score < rank10 ) return 9;
    else return 10;
}

float Virtues::getMaximalBonusScore(void)
{
    if( evolution.score < rank1 )       return 100.0f / 10.0f;
    else if( evolution.score < rank2 )  return 250.0f / 15.0f;
    else if( evolution.score < rank3 )  return 500.0f / 20.0f;
    else if( evolution.score < rank4 )  return 1000.0f / 25.0f;
    else if( evolution.score < rank5 )  return 2500.0f / 30.0f;
    else if( evolution.score < rank6 )  return 5000.0f / 35.0f;
    else if( evolution.score < rank7 )  return 7500.0f / 40.0f;
    else if( evolution.score < rank8 )  return 10000.0f / 45.0f;
    else if( evolution.score < rank9 )  return 15000.0f / 50.0f;
    else                                return 25000.0f / 55.0f;
}

float Virtues::getPerceptionSkill(void)
{
    return 1 - 1 / ( 1 + Kperception * skills.perception );
}

float Virtues::getEnduranceSkill(void)
{
	return 1.0f;
    return 1 - 1 / ( 1 + Kendurance * skills.endurance );
}

float Virtues::getTrackingSkill(void)
{
    return 1 - 1 / ( 1 + Ktracking * skills.tracking );
}

float Virtues::getRiggingSkill(void)
{
    return 1 - 1 / ( 1 + Krigging * skills.rigging );
}

void Virtues::setPerceptionSkill(float value)
{
	//todo
    //assert( value < 1.0f );
    skills.perception = 1 / Kperception * ( 1 / ( 1 - value ) - 1 );
}

void Virtues::setEnduranceSkill(float value)
{
	//todo
    //assert( value < 1.0f );
    skills.endurance = 1 / Kendurance * ( 1 / ( 1 - value ) - 1 );
}

void Virtues::setTrackingSkill(float value)
{
	// TODO
    //assert( value < 1.0f );
    skills.tracking = 1 / Ktracking * ( 1 / ( 1 - value ) - 1 );
}

void Virtues::setRiggingSkill(float value)
{
	//todo
    //assert( value < 1.0f );
    skills.rigging = 1 / Krigging * ( 1 / ( 1 - value ) - 1 );
}

float Virtues::getTimeShift(float adrenaline)
{
    // effect interpolation points
    float ideal   = 0.125f + 0.875f * ( 1.0f / 64.0f * pow( adrenaline, -6 ) );
    float minimal = 1.0f / 1.189f * pow( adrenaline, -0.25f );

    // skill factor
    float factor = getPerceptionSkill();

    // effect value
    return minimal * ( 1 - factor ) + ideal * factor;
}

float Virtues::getMaxTolerantOverburden(void)
{
    return 1.0f + 1.5f * pow( getEnduranceSkill(), 0.55f );
}

float Virtues::getMaxEndurableOverburden(void)
{
    return 3.0f + 9.0f * pow( getEnduranceSkill(), 0.65f );
}

float Virtues::getShock(float overburden)
{
    // effect interpolation points
    float min = getMaxTolerantOverburden();
    float max = getMaxEndurableOverburden();

    // shock value
    float factor = ( overburden - min ) / ( max - min );
    factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );
    return factor;
}

float Virtues::getControlPenalty(float shock)
{
    shock = ( shock < 0 ) ? 0 : ( ( shock > 1 ) ? 1 : shock );
    return pow( shock, 3 );
}

float Virtues::getLineoverProbability(float pcFactor)
{
    float result = 0.03125f * ( 1 - pcFactor ) * ( 1 - pcFactor );

    getCore()->logMessage( "lineover prob: %3.2f", result );

    return result;
}

// reference velocity is a velocity by which the probability of
// linetwists is equals to one half of maximal probability

const float maxiLTProbability            = 0.125f;
const float weakReferenceVelosity        = 30.0f;
const float irregularReferenceVelosity   = 40.0f;

float Virtues::getLinetwistsProbability(float jumperVelocity)
{    
    // probability of linetwists because of weak inflation (slider up)
    float factor = jumperVelocity / weakReferenceVelosity;
    float weakProb = maxiLTProbability / ( 1.0f + pow( factor, 5 ) );
    if( equipment.sliderOption != soUp ) weakProb = 0.0f;

    // probability of linetwists because of irregular inflation (slider down/removed)
    factor = jumperVelocity / irregularReferenceVelosity;
    float irregularProb = maxiLTProbability - ( maxiLTProbability / ( 1.0f + pow( factor, 3 ) ) );
    if( equipment.sliderOption == soUp ) irregularProb = 0.0f;

    // probability of linetwists because of canopy state
    float canopyStateProb = pow( ( 1 - equipment.canopy.state ), 15 );

    // probability of linetwists because of rig state
    float rigStateProb = pow( ( 1 - equipment.rig.state ), 30 );

    // result 
    float result = weakProb + irregularProb + canopyStateProb + rigStateProb;
    result = result > 1 ? 1 : result;

    return result;
}

float Virtues::getSkillLevel(void)
{
    return 0.25f * ( getPerceptionSkill() + 
                     getEnduranceSkill() + 
                     getTrackingSkill() + 
                     getRiggingSkill() );
}

/**
 * procedural properties: model coefficients
 */

static inline float getSuitStateFactor(float state)
{
    return 0.25f + 0.75f * pow( state, 0.5f );
}

static inline float getBodyFrogAirResistance(float height)
{
    // search for correspondign pair of database entries
    for( unsigned int i=0; i<(Kentries-1); i++ )
    {
        if( Kfrog[i][0] <= height && Kfrog[i+1][0] >= height )
        {
            // determine interpolation 
            float interpolation = ( height - Kfrog[i][0] ) / ( Kfrog[i+1][0] - Kfrog[i][0] );
            // return interpolated resistance coefficient
            return Kfrog[i][1] * ( 1.0f - interpolation ) + Kfrog[i+1][1] * interpolation;
        }
    }
    assert( !"Shouldn't be here!" );
    return 0.28f;
}

float Virtues::getFrogAirResistance(void)
{
    float Ksuit = getSuitStateFactor( equipment.suit.state ) * 
                  database::Suit::getRecord( equipment.suit.id )->mFrog;
    if( Ksuit < 1.0f ) Ksuit = 1.0f;
    return getBodyFrogAirResistance( appearance.height ) * Ksuit;
}

static inline float getBodyTrackingAirResistance(float height)
{
    // search for correspondign pair of database entries
    for( unsigned int i=0; i<(Kentries-1); i++ )
    {
        if( Kheadfirst[i][0] <= height && Kheadfirst[i+1][0] >= height )
        {
            // determine interpolation 
            float interpolation = ( height - Ktrack[i][0] ) / ( Ktrack[i+1][0] - Ktrack[i][0] );
            // return interpolated resistance coefficient
            return Ktrack[i][1] * ( 1.0f - interpolation ) + Ktrack[i+1][1] * interpolation;
        }
    }
    assert( !"Shouldn't be here!" );
    return 0.22f;
}

float Virtues::getTrackingAirResistance(void)
{
    float Ksuit = getSuitStateFactor( equipment.suit.state ) * 
                  database::Suit::getRecord( equipment.suit.id )->mTracking;
    if( Ksuit < 1.0f ) Ksuit = 1.0f;
    return getBodyTrackingAirResistance( appearance.height ) * Ksuit;
}

static inline float getBodyHeadfirstAirResistance(float height)
{
    // search for correspondign pair of database entries
    for( unsigned int i=0; i<(Kentries-1); i++ )
    {
        if( Kheadfirst[i][0] <= height && Kheadfirst[i+1][0] >= height )
        {
            // determine interpolation 
            float interpolation = ( height - Kheadfirst[i][0] ) / ( Kheadfirst[i+1][0] - Kheadfirst[i][0] );
            // return interpolated resistance coefficient
            return Kheadfirst[i][1] * ( 1.0f - interpolation ) + Kheadfirst[i+1][1] * interpolation;
        }
    }
    assert( !"Shouldn't be here!" );
    return 0.18f;
}

float Virtues::getHeadfirstAirResistance(void)
{
    float Ksuit = getSuitStateFactor( equipment.suit.state ) * 
                  database::Suit::getRecord( equipment.suit.id )->mHeadDown;
    if( Ksuit < 1.0f ) Ksuit = 1.0f;
    return getBodyHeadfirstAirResistance( appearance.height ) * Ksuit;
}

float Virtues::getSteerPitch(void)
{
    return appearance.weight;
}

float Virtues::getSteerTurn(void)
{
    return appearance.weight * 0.5f;
}

float Virtues::getSteerSlide(void)
{
    return appearance.weight * 0.0001f;
}

float Virtues::getSteerRoll(void)
{
    return appearance.weight * 0.0002f;
}

float Virtues::getSteerAlign(void)
{
    return appearance.weight * 0.0005f;
}

float Virtues::getFrogGlideRatio(void)
{
    float skill = getTrackingSkill();
    float Kbody = 0.25f + 0.5f * pow( skill, 0.75f );
    float Ksuit = getSuitStateFactor( equipment.suit.state ) * 
                  database::Suit::getRecord( equipment.suit.id )->mFrogGlideRatio;
    if( Ksuit < 1.0f ) Ksuit = 1.0f;
    return Kbody * Ksuit;
}

float Virtues::getTrackingGlideRatio(void)
{
    float skill = getTrackingSkill();
    float Kbody = 0.75f + 0.75f * pow( skill, 0.65f );
    float Ksuit = getSuitStateFactor( equipment.suit.state ) * 
                  database::Suit::getRecord( equipment.suit.id )->mTrackingGlideRatio;
    if( Ksuit < 1.0f ) Ksuit = 1.0f;
    return Kbody * Ksuit;
}

float Virtues::getFrogGlideCoefficient(void)
{
    float Ksuit = getSuitStateFactor( equipment.suit.state ) * 
                  database::Suit::getRecord( equipment.suit.id )->mFrogGlideForce;
    if( Ksuit < 1.0f ) Ksuit = 1.0f;
    return 75.0f * Ksuit;
}

float Virtues::getTrackingGlideCoefficient(void)
{
    float Ksuit = getSuitStateFactor( equipment.suit.state ) * 
                  database::Suit::getRecord( equipment.suit.id )->mTrackingGlideForce;
    if( Ksuit < 1.0f ) Ksuit = 1.0f;
    return 500.00f * Ksuit;
}

float Virtues::getHorizontalAligment(void)
{
    return getSuitStateFactor( equipment.suit.state ) * 
           database::Suit::getRecord( equipment.suit.id )->kAlign;
}

/**
 * damage, overburden & collision handlers
 */

float Virtues::onJumperDamage(float damage)
{
    // first, absorb damage by helmet
    // note that fatal damage won't be absorbed!
    database::Helmet* helmetInfo = database::Helmet::getRecord( equipment.helmet.id );
    float absorbedDamage = damage * ( helmetInfo->Kda * equipment.helmet.state );
    float helmetDamage = damage * helmetInfo->Kage;
    equipment.helmet.state -= helmetDamage;
    if( equipment.helmet.state < 0 ) equipment.helmet.state = 0;
    if( damage < 1.0f ) damage -= absorbedDamage;

    // damage suit
    database::Suit* suitInfo = database::Suit::getRecord( equipment.suit.id );
    float suitDamage = damage * suitInfo->Kage;
    equipment.suit.state -= suitDamage;
    if( equipment.suit.state < 0 ) equipment.suit.state = 0;

    // result
    return damage;
}

float Virtues::onJumperOverburden(float overburden, float dt)
{
    // absorb overburden
    database::Rig* rigInfo = database::Rig::getRecord( equipment.rig.id );
    float absorbedOverburden = overburden * ( rigInfo->Kab * equipment.rig.state );

    // damage rig
    float factor = ( overburden - rigInfo->Kminoverb ) / ( rigInfo->Kmaxoverb - rigInfo->Kminoverb );
    factor = factor < 0 ? 0 : ( factor > 1 ? 1 : factor );
    float damage = rigInfo->Kminage * ( 1 - factor ) + rigInfo->Kmaxage * factor;
    damage *= dt;

    equipment.rig.state -= damage;
    if( equipment.rig.state < 0 ) equipment.rig.state = 0;

    overburden -= overburden;
    return overburden;
}