/****************************************************************************
;
;   MODULE:     LTInfo_Impl (.H)
;
;   PURPOSE:    General purpose functions for system info
;
;   HISTORY:    4-14-2000 [mds] File created.
;
;   NOTICE:     Copyright (c) 2000 Monolith Productions, Inc.
;
***************************************************************************/

#ifndef __LTINFO_IMPL_H__
#define __LTINFO_IMPL_H__

#ifndef __ILTINFO_H__
#include "iltinfo.h"
#endif

LTRESULT in_GetVersionInfoExt(LTVERSIONINFOEXT* pVersionInfo);
LTRESULT in_GetPerformanceInfo(LTPERFORMANCEINFO* pPerformanceInfo);

#endif 
