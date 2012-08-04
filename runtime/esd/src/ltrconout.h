/****************************************************************************
;
;	MODULE:		LTRConOut_Impl (.H)
;
;	PURPOSE:	Console output functionality for RealAudio and RealVideo
;
;	HISTORY:	4-18-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTRConOut_Impl_H
#define LTRConOut_Impl_H

#define LTRA_CONOUT_NONE		0	// No output
#define LTRA_CONOUT_ERROR		1	// Output errors
#define LTRA_CONOUT_WARNING		2	// Output warnings
#define LTRA_CONOUT_INFO		3	// Output general info (verbose)

void LTRConsoleOutput(int nLevel, char *pMsg, ...);

#endif // LTRConOut_Impl_H
#endif // LITHTECH_ESD