// Utils.cpp

#include "StdAfx.h"

CString TimeToString(DWORD dwTime, BOOL bNoHours)
{
	int nHours = dwTime / 3600000;
	dwTime -= nHours * 3600000;

	int nMinutes = dwTime / 60000;
	dwTime -= nMinutes * 60000;

	int nSeconds = dwTime / 1000;

	CString str;

	if (nHours == 0 && bNoHours)
	{
		str.Format("%02i:%02i", nMinutes, nSeconds);
	}
	else
	{
		str.Format("%i:%02i:%02i", nHours, nMinutes, nSeconds);
	}

	return(str);
}

void DissectTime(DWORD dwTime, int* pHour, int* pMin, int* pSec)
{
	*pHour = dwTime / 3600000;
	dwTime -= *pHour * 3600000;

	*pMin = dwTime / 60000;
	dwTime -= *pMin * 60000;

	*pSec = dwTime / 1000;
}

