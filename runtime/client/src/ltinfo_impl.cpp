/****************************************************************************
;
;	MODULE:		LTInfo_Impl (.CPP)
;
;	PURPOSE:	General purpose functions for system info
;
;	HISTORY:	4-18-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Monolith Productions, Inc.
;
***************************************************************************/


// Includes...
#include "bdefs.h"
#include "ltinfo_impl.h"
#include "renderinfostruct.h"
#include "clientde_impl.h"

#include "clientmgr.h"
#include "render.h"

// Externs...

extern int32 g_ScreenWidth;
extern int32 g_ScreenHeight;


// Functions...

//----------------------------------------------------------------------------------------------
//	in_GetVersionInfoExt()
//		Provides extended information about the executable version
//----------------------------------------------------------------------------------------------
LTRESULT in_GetVersionInfoExt(LTVERSIONINFOEXT* pVersionInfo)
{
	// Make sure structure pointer is not null
	if (!pVersionInfo)
		RETURN_ERROR(1, ClientDE::GetVersionInfoExt, LT_INVALIDPARAMS);

	// Make sure structure size is what we're expecting
	if (sizeof(LTVERSIONINFOEXT) != pVersionInfo->m_dwSize)
		RETURN_ERROR(1, ClientDE::GetVersionInfoExt, LT_INVALIDPARAMS);

	// Fill out structure info
	pVersionInfo->m_dwFlags = 0;
	pVersionInfo->m_dwMajorVersion = LT_VI_VER_MAJOR;
	pVersionInfo->m_dwMinorVersion = LT_VI_VER_MINOR;
	pVersionInfo->m_dwBuildNumber = LT_VI_BUILD_NUM;
	pVersionInfo->m_dwBuildMonth = LT_VI_BUILD_MONTH;
	pVersionInfo->m_dwBuildDay = LT_VI_BUILD_DAY;
	pVersionInfo->m_dwBuildYear = LT_VI_BUILD_YEAR;
	lstrcpy(pVersionInfo->m_sBuildName, LT_VI_BUILD_NAME);
	pVersionInfo->m_dwLanguage = LT_VI_LANG;
	pVersionInfo->m_dwPlatform = LT_VI_PLAT;
	pVersionInfo->m_dwReserved1 = 0;
	pVersionInfo->m_dwReserved2 = 0;

	return LT_OK;
}


//----------------------------------------------------------------------------------------------
//	in_GetPerformanceInfo()
//		Provides information about the rendering performance
//----------------------------------------------------------------------------------------------
LTRESULT in_GetPerformanceInfo(LTPERFORMANCEINFO* pPerformanceInfo)
{
	// Make sure structure pointer is not null
	if (!pPerformanceInfo)
		RETURN_ERROR(1, ClientDE::GetPerformanceInfo, LT_INVALIDPARAMS);

	// Make sure structure size is what we're expecting
	if (sizeof(LTPERFORMANCEINFO) != pPerformanceInfo->m_dwSize)
		RETURN_ERROR(1, ClientDE::GetVersionInfoExt, LT_INVALIDPARAMS);

	// For some information, we ask the renderer directly
	RenderInfoStruct renderInfoStruct;
	memset(&renderInfoStruct, 0, sizeof(RenderInfoStruct));
	r_GetRenderStruct()->GetRenderInfo(&renderInfoStruct);

	// Fill out structure info
	pPerformanceInfo->m_dwFlags = 0;
	pPerformanceInfo->m_dwScreenWidth = g_ScreenWidth;
	pPerformanceInfo->m_dwScreenHeight = g_ScreenHeight;
	pPerformanceInfo->m_dwFps = (DWORD)(g_pClientMgr->m_FramerateTracker.GetRate() + 0.5f);	// round up
	pPerformanceInfo->m_dwNumWorldPolys = renderInfoStruct.m_dwWorldPolysDrawn;
	pPerformanceInfo->m_dwNumWorldPolysProcessed = renderInfoStruct.m_dwWorldPolysProcessed;
	pPerformanceInfo->m_dwNumModelPolys = renderInfoStruct.m_dwModelPolysDrawn;
	pPerformanceInfo->m_dwReserved1 = 0;

	return LT_OK;
}

