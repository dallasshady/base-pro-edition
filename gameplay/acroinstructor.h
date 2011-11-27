
#ifndef ACROBATICS_INSTRUCTORS_INCLUDED
#define ACROBATICS_INSTRUCTORS_INCLUDED

#include "headers.h"
#include "instructor.h"
#include "landingaccuracy.h"

namespace instructor
{

/**
 * abstract acrobatics instructor
 */

class AcrobaticsInstructor : public Instructor
{
private:
    Acrobatics _skill;
    bool       _prevSkillValue;
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt);
    // Goal abstracts
    virtual const wchar_t* getGoalValue(void);
    virtual float getGoalScore(void);
    // class implementation
    AcrobaticsInstructor(Jumper* player, Acrobatics skill);
    virtual ~AcrobaticsInstructor();
};

}

#endif