// Utils.h

#ifndef _UTILS_H_
#define _UTILS_H_

CString	TimeToString(DWORD dwTime, BOOL bNoHours = FALSE);
void	DissectTime(DWORD dwTime, int* pHour, int* pMin, int* pSec);

#endif


