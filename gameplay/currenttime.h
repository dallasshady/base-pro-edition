
#ifndef CURRENTTIME_ROUTINE
#define CURRENTTIME_ROUTINE

bool isGreaterTime(SYSTEMTIME* s1, SYSTEMTIME* s2);

typedef std::pair<SYSTEMTIME,bool> SYSTEMTIMEBOOL;

bool getLatestFileTimeA(std::string path, SYSTEMTIME* systemTime);
bool getLatestFileTimeB(SYSTEMTIME* systemTime);

#endif