#ifndef __TIMEMGR_H__
#define __TIMEMGR_H__

#ifndef __LTINTEGER_H__
#include "ltinteger.h"
#endif

// Note: we're assuming the timer doesn't need to be initialized .. if it does,
// it should be initialized in system-dependent code.
float	time_GetTime(); // returns time in seconds as a floating point number
uint32  timeGetTime(); // returns time in milliseconds
#define time_GetMSTime timeGetTime

#endif  // __TIMEMGR_H__

