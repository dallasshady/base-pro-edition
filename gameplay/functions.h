#ifndef GLOBAL_FUNCTIONS_INCLUDED
#define GLOBAL_FUNCTIONS_INCLUDED

#pragma warning(disable:4786)

void floatToUIntTime(float time, unsigned int &years, unsigned int &months, unsigned int &days, unsigned int &hours, unsigned int &minutes, unsigned int &seconds) {
	const unsigned int SECOND = 1;
	const unsigned int MINUTE = 60 * SECOND;
	const unsigned int HOUR = 60 * MINUTE;
	const unsigned int DAY = 24 * HOUR;
	const unsigned int MONTH = 30 * DAY;
	const unsigned int YEAR = 365 * DAY;

	const float consts[] = {YEAR,MONTH,DAY,HOUR,MINUTE,SECOND};
	unsigned int *data[] = {&years,&months,&days,&hours,&minutes,&seconds};
	for (int i = 0; i < 6; ++i) {
		if (data[i] != NULL) {
			*data[i] = (unsigned int)(time / consts[i]);
			time -= (float)*data[i] * consts[i];
		}
	}
}

#endif