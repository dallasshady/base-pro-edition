
#include "headers.h"
#include "database.h"
#include "career.h"

using namespace database;

static float _salary_unemployed[] = { 100.0f };
static float _progress_unemployed[] = { 0.0f };

static float _salary_service[] = 
{ 
    150.0f, 
    300.0f, 
    500.0f 
};

static float _progress_service[] = 
{  
    1 / HOURS_TO_MINUTES(30),
    1 / HOURS_TO_MINUTES(60),
    0
};

static EmploymentInfo employments[] = 
{
    /* 00 */ { 97, HOURS_TO_MINUTES(0), 0, _salary_unemployed, _progress_unemployed },
    /* 01 */ { 98, HOURS_TO_MINUTES(30), 3, _salary_service, _progress_service },
    { 0, 0, NULL, NULL, }
};

unsigned int EmploymentInfo::getNumRecords(void)
{
    unsigned int result = 0;
    unsigned int i = 0;
    while( employments[i].descriptionId != 0 ) i++, result++;
    return result;
}

EmploymentInfo* EmploymentInfo::getRecord(unsigned int id)
{
    assert( id >= 0 && id < getNumRecords() );
    return employments + id;
}