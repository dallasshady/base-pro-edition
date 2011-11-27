
#ifndef COMPABILITY_ISSUES_INCLUDED
#define COMPABILITY_ISSUES_INCLUDED

#include "headers.h"
#include "math.h"

inline LONG convertToDecibels(float volume)
{
    LONG result = -10000;
    volume = volume < 0 ? 0 : ( volume > 1 ? 1 : volume );
    if( volume > 0 ) result = LONG( -600.0f / log(0.5f) * log(volume) );
    result = result < -10000 ? -10000 : ( result > 0 ? 0 : result );
    return result;
}

#endif