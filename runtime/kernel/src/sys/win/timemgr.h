#ifndef __TIMEMGR_H__
#define __TIMEMGR_H__



// Note: we're assuming the timer doesn't need to be initialized .. if it does,
// it should be initialized in system-dependent code.
float	time_GetTime();
uint32	time_GetMSTime();


#endif  // __TIMEMGR_H__

