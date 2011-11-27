/**
 * very simple profiling
 */

#pragma once

#include "windows.h"

inline __int64 getPerformanceCounter(void)
{
    __int64 result;
    ::QueryPerformanceCounter( (LARGE_INTEGER*)(&result) );
    return result;
}

inline float convertCounterToSeconds(__int64 value)
{
    __int64 frequency;
    ::QueryPerformanceFrequency( (LARGE_INTEGER*)(&frequency) );
    return float( (double)value / (double)frequency );
}