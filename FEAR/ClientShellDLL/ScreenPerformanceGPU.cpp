// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPerformanceGPU.cpp
//
// PURPOSE : Defines the CScreenPerformanceGPU class.  This class sets
//           the GPU performance settings.
//
// CREATED : 04/05/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenPerformanceGPU.h"
#include "PerformanceMgr.h"
#include "ScreenCommands.h"
#include "PerformanceDB.h"
#include "sys/win/mpstrconv.h"

CScreenPerformanceGPU::CScreenPerformanceGPU() :
	CScreenPerformanceAdvanced( "PerformanceGPU_Title", 1 ),
	m_dwFlags(0),
	m_pCtrlResolution(NULL)
{
	CScreenDisplay::GetRendererData( m_rendererData );
}

CScreenPerformanceGPU::~CScreenPerformanceGPU()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceGPU::Build()
//
//	PURPOSE:	build the controls used on the screen
//
// ----------------------------------------------------------------------- //
bool CScreenPerformanceGPU::Build()
{
	return CScreenPerformanceAdvanced::Build();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceGPU::OnFocus()
//
//	PURPOSE:	join/leave screen
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceGPU::OnFocus(bool bFocus)
{
	if( bFocus )
	{
		// clear all warning flags
		m_dwFlags &= ~eFlagWarningMask;
	}
	else
	{

	}

	CScreenPerformanceAdvanced::OnFocus(bFocus);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceGPU::Load()
//
//	PURPOSE:	read in the settings and apply them to the controls
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceGPU::Load()
{
	CScreenPerformanceAdvanced::Load();

	// this is a special case hardcoded function anyway so we hardcode the console commands as well
	// Performance_ScreenWidth
	float fWidth = 640.0f;
	if( !CPerformanceMgr::Instance().GetQueuedConsoleVariable("Performance_ScreenWidth", fWidth) )
		fWidth = GetConsoleFloat("Performance_ScreenWidth",640.0f);

	// Performance_ScreenHeight
	float fHeight = 480.0f;
	if( !CPerformanceMgr::Instance().GetQueuedConsoleVariable("Performance_ScreenHeight", fHeight) )
		fHeight = GetConsoleFloat("Performance_ScreenHeight",480.0f);

	wchar_t wszResolution[128];
	FormatString( "PerformanceOption_ResolutionFormat", wszResolution, LTARRAYSIZE(wszResolution), (int)fWidth, (int)fHeight );

	m_pCtrlResolution = FindColumnControl( LoadString("PerformanceOption_Resolution") );
	if( !m_pCtrlResolution )
		return;

	m_pCtrlResolution->SetString(1, wszResolution);

	UpdateResolutionColor();

	// check if we overflowed our video memory, if so change it's color
	CheckResolutionMemory();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceGPU::Save()
//
//	PURPOSE:	get settings from controls, and apply them
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceGPU::Save()
{
	CScreenPerformanceAdvanced::Save();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceGPU::UpdateResolutionColor()
//
//	PURPOSE:	update the color of the resolution based on memory usage
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceGPU::UpdateResolutionColor()
{
	if( m_pCtrlResolution && CPerformanceMgr::Instance().ArePerformanceStatsValid() )
	{
		float fMemoryUsed	= CPerformanceMgr::Instance().EstimateVideoMemoryUsage();
		float fMemoryTotal	= (float)CPerformanceMgr::Instance().GetPerformanceStats().m_nGPUMemory;
		if( fMemoryUsed > fMemoryTotal )
			m_pCtrlResolution->SetColor( m_nWarningColor );
		else
			m_pCtrlResolution->SetColor( m_NonSelectedColor );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceGPU::UpdateOption()
//
//	PURPOSE:	overridden to test for special cases
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceGPU::UpdateOption( CLTGUIColumnCtrl* pCtrl, int nGroup, int nIndex )
{
	LTASSERT( pCtrl, "Invalid control pointer" );
	if( !pCtrl )
		return;

    HRECORD hOptRec = CPerformanceMgr::Instance().GetOptionRecord(m_nGlobalType, nGroup, nIndex);
	if( !hOptRec )
		return;

	const char* pszRecordName = g_pLTDatabase->GetRecordName( hOptRec );
	if( LTStrIEquals(pszRecordName, "Resolution") )
	{
		UpdateOption_Resolution( pCtrl, nGroup, nIndex );
	}
	else if( LTStrIEquals(pszRecordName, "SoftShadows") )
	{
		UpdateOption_SoftShadows( pCtrl, nGroup, nIndex );
	}
	else if( LTStrIEquals(pszRecordName, "TextureResolution") )
	{
		UpdateOption_TextureResolution( pCtrl, nGroup, nIndex );
	}
	else if( LTStrIEquals(pszRecordName, "VolumeLights") )
	{
		UpdateOption_VolumeLights( pCtrl, nGroup, nIndex );
	}
	else
	{
		CScreenPerformanceAdvanced::UpdateOption( pCtrl, nGroup, nIndex );
	}

	UpdateResolutionColor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceGPU::UpdateOption_Resolution()
//
//	PURPOSE:	updates the resolution option.  special because resolutions 
//				have to be enumerated
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceGPU::UpdateOption_Resolution( CLTGUIColumnCtrl* pCtrl, int nGroup, int nIndex )
{
	CLTGUITextCtrl*	pColumn = pCtrl->GetColumn( 1 );
	if( !pColumn )
		return;

	const int32 nLevelCount = (int32)m_rendererData.m_resolutionArray.GetSize();

	const wchar_t* pwszCurrentDetailName = pColumn->GetString();

	wchar_t wszResolution[128];

	int32 nLevel = 0;
	for(;nLevel<nLevelCount;++nLevel)
	{
		FormatString( "PerformanceOption_ResolutionFormat", wszResolution, LTARRAYSIZE(wszResolution), m_rendererData.m_resolutionArray.Get(nLevel).m_dwWidth, m_rendererData.m_resolutionArray.Get(nLevel).m_dwHeight );
		if( LTStrIEquals(wszResolution, pwszCurrentDetailName) )
			break;
	}

	if( ++nLevel >= nLevelCount )
		nLevel = 0;

	FormatString( "PerformanceOption_ResolutionFormat", wszResolution, LTARRAYSIZE(wszResolution), m_rendererData.m_resolutionArray.Get(nLevel).m_dwWidth, m_rendererData.m_resolutionArray.Get(nLevel).m_dwHeight );
	pCtrl->SetString(1, wszResolution);

	// this is a special case hardcoded function anyway so we hardcode the console commands as well
	// Performance_ScreenWidth
	CPerformanceMgr::Instance().SetQueuedConsoleVariable( "Performance_ScreenWidth", (float)m_rendererData.m_resolutionArray.Get(nLevel).m_dwWidth, eAT_RestartRender );
	// Performance_ScreenHeight
	CPerformanceMgr::Instance().SetQueuedConsoleVariable( "Performance_ScreenHeight", (float)m_rendererData.m_resolutionArray.Get(nLevel).m_dwHeight, eAT_RestartRender );

	// warn about memory usage
	CheckResolutionMemory();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceGPU::UpdateOption_SoftShadows()
//
//	PURPOSE:	updates the soft shadows option.  overridden to display a 
//				message box to warn the user of the performance hit
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceGPU::UpdateOption_SoftShadows( CLTGUIColumnCtrl* pCtrl, int nGroup, int nIndex )
{
	CScreenPerformanceAdvanced::UpdateOption( pCtrl, nGroup, nIndex );

	CLTGUITextCtrl*	pColumn = pCtrl->GetColumn( 1 );
	if( !pColumn )
		return;

	const wchar_t* pwszCurrentDetailName = pColumn->GetString();

	if( !(m_dwFlags & eFlag_SoftShadowWarning) &&
		LTStrIEquals(pwszCurrentDetailName, LoadString("PerformanceOptionSetting_On")) )
	{
		MBCreate mb;
		mb.eType = LTMB_OK;
		g_pInterfaceMgr->ShowMessageBox("PerformanceMessage_SoftShadows",&mb);
		m_dwFlags |= eFlag_SoftShadowWarning;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceGPU::UpdateOption_TextureResolution()
//
//	PURPOSE:	updates the texture resolution option.  overridden to display a 
//				message box to warn the user of the performance hit
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceGPU::UpdateOption_TextureResolution( CLTGUIColumnCtrl* pCtrl, int nGroup, int nIndex )
{
	CScreenPerformanceAdvanced::UpdateOption( pCtrl, nGroup, nIndex );

	CLTGUITextCtrl*	pColumn = pCtrl->GetColumn( 1 );
	if( !pColumn )
		return;

	HRECORD hOptRec = CPerformanceMgr::Instance().GetOptionRecord(m_nGlobalType, nGroup, nIndex);
	if( !hOptRec )
		return;

	const wchar_t* pwszCurrentDetailName = pColumn->GetString();

	if( !(m_dwFlags & eFlag_TextureResolutionWarning) &&
		LTStrIEquals(pwszCurrentDetailName, LoadString("PerformanceOptionSetting_Maximum")) )
	{
		if( CPerformanceMgr::Instance().ArePerformanceStatsValid() &&
			CPerformanceMgr::Instance().GetPerformanceStats().m_nGPUMemory < DATABASE_CATEGORY( PerformanceOption ).GETRECORDATTRIBINDEX(hOptRec, AdditionalData, 0) )
		{
			MBCreate mb;
			mb.eType = LTMB_OK;
			g_pInterfaceMgr->ShowMessageBox("PerformanceMessage_TextureResolution",&mb);
			m_dwFlags |= eFlag_TextureResolutionWarning;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceGPU::UpdateOption_VolumeLights()
//
//	PURPOSE:	updates the volumetric light option.  overridden to display a 
//				message box to warn the user of the performance hit
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceGPU::UpdateOption_VolumeLights( CLTGUIColumnCtrl* pCtrl, int nGroup, int nIndex )
{
	CScreenPerformanceAdvanced::UpdateOption( pCtrl, nGroup, nIndex );

	float fVolumeLight;
	if( !CPerformanceMgr::Instance().GetQueuedConsoleVariable("EnableVolumetricLight", fVolumeLight) )
		fVolumeLight = GetConsoleFloat( "EnableVolumetricLight", 0.0f );

	if( !(m_dwFlags & eFlag_VolumeLightsWarning) &&	(fVolumeLight != 0.0f) )
	{
		MBCreate mb;
		mb.eType = LTMB_OK;
		g_pInterfaceMgr->ShowMessageBox("PerformanceMessage_VolumeLights",&mb);
		m_dwFlags |= eFlag_VolumeLightsWarning;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceGPU::Repair()
//
//	PURPOSE:	repairs values that may not be supported by system but 
//              were set by one of the defaults
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceGPU::Repair()
{
	// make sure the resolution is within a valid range
	float fScreenWidth = 640;
	CPerformanceMgr::Instance().GetQueuedConsoleVariable( "Performance_ScreenWidth", fScreenWidth );

	float fScreenHeight = 480;
	CPerformanceMgr::Instance().GetQueuedConsoleVariable( "Performance_ScreenHeight", fScreenHeight );

// for test purposes we can limit the resolution list to 640x480
#if 1
	CMoArray<ScreenDisplayResolution>& resolutionArray = m_rendererData.m_resolutionArray;
#else
	ScreenDisplayResolution pResolutionList[] = {
		{640, 480, 32}
	};
	CMoArray<ScreenDisplayResolution> resolutionArray;
	resolutionArray.Add( pResolutionList[0] );
#endif

	// the amount of available resolutions supported by this video card + monitor combo
	const int32 nLevelCount = (int32)resolutionArray.GetSize();

	// the requested screen real-estate in pixel area
	const int32 nRequestedArea = (int32)(fScreenWidth * fScreenHeight);

	int32 nLevel = 0;
	int32 nLevelBestFit = 0;
	int32 nBestAreaDelta = -1;
	for(;nLevel<nLevelCount;++nLevel)
	{
		if( resolutionArray.Get(nLevel).m_dwWidth == (uint32)fScreenWidth &&
			resolutionArray.Get(nLevel).m_dwHeight == (uint32)fScreenHeight )
		{
			break;
		}

		int32 nArea =	resolutionArray.Get(nLevel).m_dwWidth *
						resolutionArray.Get(nLevel).m_dwHeight;
		int32 nAreaDelta = nRequestedArea - nArea;
		if( nAreaDelta < 0 ) nAreaDelta = -nAreaDelta;
		if( nAreaDelta < nBestAreaDelta || nBestAreaDelta < 0 )
		{
			nBestAreaDelta = nAreaDelta;
			nLevelBestFit = nLevel;
		}
	}

	if( nLevel >= nLevelCount && nLevelCount )
	{
		// we didn't find one so lets use the closest fit
		CPerformanceMgr::Instance().SetQueuedConsoleVariable( "Performance_ScreenWidth", (float)resolutionArray.Get(nLevelBestFit).m_dwWidth, eAT_RestartRender );	
		CPerformanceMgr::Instance().SetQueuedConsoleVariable( "Performance_ScreenHeight", (float)resolutionArray.Get(nLevelBestFit).m_dwHeight, eAT_RestartRender );	
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceGPU::CheckResolutionMemory()
//
//	PURPOSE:	checks to see if the user has blown their video memory limit
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceGPU::CheckResolutionMemory()
{
	// check if we overflowed our video memory, if so change it's color
	if( CPerformanceMgr::Instance().ArePerformanceStatsValid() )
	{
		float fMemoryUsed	= CPerformanceMgr::Instance().EstimateVideoMemoryUsage();
		float fMemoryTotal	= (float)CPerformanceMgr::Instance().GetPerformanceStats().m_nGPUMemory;
		if( fMemoryUsed > fMemoryTotal )
		{
			if( !(m_dwFlags & eFlag_ScreenResolutionWarning) )
			{
				MBCreate mb;
				mb.eType = LTMB_OK;
				g_pInterfaceMgr->ShowMessageBox("PerformanceMessage_ScreenResolution",&mb);
				m_dwFlags |= eFlag_ScreenResolutionWarning;
			}
		}
	}
}