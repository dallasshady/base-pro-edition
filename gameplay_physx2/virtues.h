
#ifndef CHARACTER_VIRTUES_INCLUDED
#define CHARACTER_VIRTUES_INCLUDED

#include "gear.h"

/**
 * slider options
 */

enum SliderOption
{
    soRemoved,
    soDown,
    soUp
};

/**
 * character virtues
 */

// original Virtues structure for backward compatibility
struct VirtuesLegacy
{
public:
    // character statistics
    struct Statistics
    {
    public:        
        float        playingTime;  // playing time (in seconds)
        unsigned int numBaseJumps; // number of base jumps from all objects
        unsigned int numSkydives;  // number of skydiving jumps
    public:
        Statistics() : playingTime(0), numBaseJumps(0), numSkydives(0) {}
    } 
    statistics;
public:
    // character appearance
    struct Appearance
    {
    public:
        float        height; // character height (in cms)
        float        weight; // character weight (in kgs)
        unsigned int face;   // character face id
    public:
        Appearance() : height(180), weight(80), face(0) {}
    }
    appearance;
public:
    // character equipment
    struct Equipment
    {
    public:
        GearLegacy      helmet;       // helmet
        GearLegacy      suit;         // suit
        GearLegacy      rig;          // rig
        GearLegacy      canopy;       // canopy
        bool         malfunctions; // true, if going to simulate malfunctions
        bool         experience;   // true, if going to improve skills
        SliderOption sliderOption; // slider option
        unsigned int pilotchute;   // pilotchute
    public:
        Equipment() : experience(true), malfunctions(true), sliderOption(soRemoved), pilotchute(0) {}
    }
    equipment;
public:
    // character/career evolution
    struct Evolution
    {
    public:
        unsigned int credits;  // aka lives, it is used up for valuable game choices
        float        workTime; // time, spended up on regular work
        float        funds;    // earning funds
        float        time;     // virtual time (in minutes)
        float        health;   // character health ( 0...1)
        float        score;    // numerical score
    public:
        Evolution() : credits(3), workTime(0), funds(0), time(0), health(1), score(0) {}
    }
    evolution;
public:
    // character skills & predisposition to skills
    struct Skills
    {
    public:
        float perception;
        float tracking;
        float endurance;
        float rigging;
    public:
        Skills() : perception(0), tracking(0), endurance(0), rigging(0) {}
    }
    skills, predisp;
public:
    /**
     * procedural properties : skills & stats
     */
    float getPromotion(void);
    float getSalary(void);
    unsigned int getRank(void);
    float getMaximalBonusScore(void);
    float getPerceptionSkill(void);
    float getEnduranceSkill(void);
    float getTrackingSkill(void);
    float getRiggingSkill(void);
    float getTimeShift(float adrenaline);
    float getMaxTolerantOverburden(void);
    float getMaxEndurableOverburden(void);
    float getShock(float overburden);
    float getControlPenalty(float shock);
    float getLineoverProbability(float pcFactor);
    float getLinetwistsProbability(float jumperVelocity);
    float getSkillLevel(void);
    /**
     * procedural properties : setting the skills (for NPC usage only)
     */
    void setPerceptionSkill(float value);
    void setEnduranceSkill(float value);
    void setTrackingSkill(float value);
    void setRiggingSkill(float value);
    /**
     * procedural properties : model coefficients
     */
    float getFrogAirResistance(void);
    float getTrackingAirResistance(void);
    float getHeadfirstAirResistance(void);
    float getSteerPitch(void);
    float getSteerTurn(void);
    float getSteerSlide(void);
    float getSteerRoll(void);
    float getSteerAlign(void);
    float getFrogGlideRatio(void);
    float getTrackingGlideRatio(void);
    float getFrogGlideCoefficient(void);
    float getTrackingGlideCoefficient(void);
    float getHorizontalAligment(void);
    /**
     * damage, overburden & collision handlers
     */
    float onJumperDamage(float damage);
    float onJumperOverburden(float overburden, float dt);
};


/*
Version history:
0 Original
0 Reserve canopies
0 Rig_add
1 Version, freefall time, wingsuit time, canopy time
*/
struct Virtues
{
private:
	static const int version = 1;  // virtues structure version for backward compat.
public:
    // character statistics
    struct Statistics
    {
    public:        
        float        playingTime;  // playing time (in seconds)
        unsigned int numBaseJumps; // number of base jumps from all objects
        unsigned int numSkydives;  // number of skydiving jumps
		float		 freeFallTime; // seconds spent in freefall (wingsuit included)
		float		 wingsuitTime; // seconds spent flying a wingsuit
		float		 canopyTime;   // seconds spent flying a canopy
    public:
        Statistics() : playingTime(0), numBaseJumps(0), numSkydives(0), freeFallTime(0), wingsuitTime(0), canopyTime(0) {}
    } 
    statistics;
public:
    // character appearance
    struct Appearance
    {
    public:
        float        height; // character height (in cms)
        float        weight; // character weight (in kgs)
        unsigned int face;   // character face id
    public:
        Appearance() : height(180), weight(80), face(0) {}
    }
    appearance;
public:
    // character equipment
    struct Equipment
    {
    public:
        Gear         helmet;       // helmet
        Gear         suit;         // suit
        Gear         rig;          // rig
        Gear         canopy;       // canopy
		Gear         reserve;      // reserve canopy
        bool         malfunctions; // true, if going to simulate malfunctions
        bool         experience;   // true, if going to improve skills
        SliderOption sliderOption; // slider option
        unsigned int pilotchute;   // pilotchute
    public:
        Equipment() : experience(true), malfunctions(true), sliderOption(soRemoved), pilotchute(0) {}
    }
    equipment;
public:
    // character/career evolution
    struct Evolution
    {
    public:
        unsigned int credits;  // aka lives, it is used up for valuable game choices
        float        workTime; // time, spended up on regular work
        float        funds;    // earning funds
        float        time;     // virtual time (in minutes)
        float        health;   // character health ( 0...1)
        float        score;    // numerical score
    public:
        Evolution() : credits(3), workTime(0), funds(0), time(0), health(1), score(0) {}
    }
    evolution;
public:
    // character skills & predisposition to skills
    struct Skills
    {
    public:
        float perception;
        float tracking;
        float endurance;
        float rigging;
    public:
        Skills() : perception(0), tracking(0), endurance(0), rigging(0) {}
    }
    skills, predisp;
public:
    /**
     * procedural properties : skills & stats
     */

	bool loadLegacy (VirtuesLegacy &a);

    float getPromotion(void);
    float getSalary(void);
    unsigned int getRank(void);
    float getMaximalBonusScore(void);
    float getPerceptionSkill(void);
    float getEnduranceSkill(void);
    float getTrackingSkill(void);
    float getRiggingSkill(void);
    float getTimeShift(float adrenaline);
    float getMaxTolerantOverburden(void);
    float getMaxEndurableOverburden(void);
    float getShock(float overburden);
    float getControlPenalty(float shock);
    float getLineoverProbability(float pcFactor);
    float getLinetwistsProbability(float jumperVelocity);
    float getSkillLevel(void);
    /**
     * procedural properties : setting the skills (for NPC usage only)
     */
    void setPerceptionSkill(float value);
    void setEnduranceSkill(float value);
    void setTrackingSkill(float value);
    void setRiggingSkill(float value);
    /**
     * procedural properties : model coefficients
     */
    float getFrogAirResistance(void);
    float getTrackingAirResistance(void);
    float getHeadfirstAirResistance(void);
    float getSteerPitch(void);
    float getSteerTurn(void);
    float getSteerSlide(void);
    float getSteerRoll(void);
    float getSteerAlign(void);
    float getFrogGlideRatio(void);
    float getTrackingGlideRatio(void);
    float getFrogGlideCoefficient(void);
    float getTrackingGlideCoefficient(void);
    float getHorizontalAligment(void);
    /**
     * damage, overburden & collision handlers
     */
    float onJumperDamage(float damage);
    float onJumperOverburden(float overburden, float dt);
};



#endif