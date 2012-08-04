/****************************************************************************
;
;	MODULE:		LTRConOut_Impl (.CPP)
;
;	PURPOSE:	Console output functionality for RealAudio and RealVideo
;
;	HISTORY:	4-18-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#include "ltrconout.h"
#include "bdefs.h"
#include "console.h"

extern int32 g_CV_LTRConsoleOutput;

void LTRConsoleOutput(int nLevel, char *pMsg, ...)
{
	if (g_CV_LTRConsoleOutput >= nLevel)
	{
		char msg[500] = "";
		va_list marker;

		va_start(marker, pMsg);
		_vsnprintf(msg, 499, pMsg, marker);
		va_end(marker);
		msg[499] = '\0';

		char msgAndType[550] = "";
		switch(nLevel)
		{
			case LTRA_CONOUT_ERROR:
				lstrcpy(msgAndType, "ERROR: ");
				break;
			case LTRA_CONOUT_WARNING:
				lstrcpy(msgAndType, "WARNING: ");
				break;
			case LTRA_CONOUT_INFO:
				lstrcpy(msgAndType, "INFO: ");
				break;
		}
		lstrcat(msgAndType, msg);

//		con_PrintString(CONRGB(255, 100, 255), 0, msgAndType);
	}
}

#endif // LITHTECH_ESD