// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenDisplay.cpp
//
// PURPOSE : Interface screen for setting display options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenDisplay.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "iltrenderer.h"

#include "GameClientShell.h"
#include "GameSettings.h"
extern CGameClientShell* g_pGameClientShell;

#define CMD_DISPLAY_CYCLE (CMD_CUSTOM + 1)

namespace
{
	int32 kGap = 0;
	int32 kWidth = 100;
	int32 kTotalWidth = 250;

	void AreYouSureCallBack(bool bReturn, void *pData)
	{
		CScreenDisplay *pThisScreen = (CScreenDisplay *)pData;
		if (pThisScreen)
		{
			pThisScreen->ConfirmHardwareCursor(bReturn);
		}
	}
	bool bInitTex;
	bool bInitLM;

	const int	kNumSteps = 50;
	const float	kBreakpoint = (float)(kNumSteps/3);
	const float	kUpperSteps = (float)kNumSteps - kBreakpoint;
	const float fLowerRange = kDefaultGamma - kMinGamma;
	const float fUpperRange = kMaxGamma - kDefaultGamma;
	float ConvertToGamma(int slider)
	{
		float gamma = kDefaultGamma;
		if (slider < (int)kBreakpoint)
		{
			float fRatio = (float)slider / kBreakpoint;
			fRatio = (float)sqrt(fRatio);
			gamma = kMinGamma + fRatio * fLowerRange;
		}
		else
		{
			float fRatio = ((float)slider - kBreakpoint) / kUpperSteps;
			fRatio *= fRatio;
			gamma = kDefaultGamma + fRatio * fUpperRange;
		}

//		g_pLTClient->CPrint("Gamma: %0.2f",gamma);
//		g_pLTClient->CPrint("Slider: %d",slider);
		return gamma;

	}

	int	ConvertToSlider(float gamma)
	{
		int slider = 5;
		if (gamma < kDefaultGamma)
		{
			float underVal = gamma - kMinGamma;
			float fRatio = underVal / fLowerRange;
			fRatio *= fRatio;
			slider = (int)(0.5f + kBreakpoint * fRatio);
			
		}
		else
		{
			float overVal = gamma - kDefaultGamma;
			float fRatio = overVal / fUpperRange;
			fRatio = sqrtf(fRatio);
			slider = (int)(0.5f + kBreakpoint + kUpperSteps * fRatio);
		}
//		g_pLTClient->CPrint("Gamma: %0.2f",gamma);
//		g_pLTClient->CPrint("Slider: %d",slider);
		return slider;
	}
}

//helper function used for sorting
int ScreenDisplayCompare( const void *arg1, const void *arg2 )
{
	ScreenDisplayResolution *pRes1=(ScreenDisplayResolution *)arg1;
	ScreenDisplayResolution *pRes2=(ScreenDisplayResolution *)arg2;

	if ( pRes1->m_dwHeight < pRes2->m_dwHeight )
	{
		return -1;
	}
	else if ( pRes1->m_dwHeight > pRes2->m_dwHeight )
	{
		return 1;
	}
	else
	{
		if (pRes1->m_dwWidth < pRes2->m_dwWidth)
		{
			return -1;
		}
		else if (pRes1->m_dwWidth > pRes2->m_dwWidth)
		{
			return 1;
		}
		else
		{
			if ( pRes1->m_dwBitDepth < pRes2->m_dwBitDepth )
			{
				return -1;
			}
			else if ( pRes1->m_dwBitDepth > pRes2->m_dwBitDepth )
			{
				return 1;
			}

		}
	}

	return 0;
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenDisplay::CScreenDisplay() :
	m_dwFlags(0),
	m_nWarningColor(0)
{

	m_bEscape		=	false;

    m_pResolutionCtrl   = NULL;
	m_pHardwareCursor	= NULL;
	m_pGamma			= NULL;
}

CScreenDisplay::~CScreenDisplay()
{

}

// Build the screen
bool CScreenDisplay::Build()
{

	CreateTitle("IDS_TITLE_DISPLAYOPTIONS");

	kGap = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,0);
	kWidth = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,1);
	kTotalWidth = kGap + kWidth;

	m_nWarningColor	= g_pLayoutDB->GetInt32(m_hLayout, LDB_ScreenAddColor, 0);

	//background frame
	CLTGUICtrl_create frameCs;
	TextureReference hFrame(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenFrameTexture));

	frameCs.rnBaseRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect,0);

	CLTGUIFrame *pFrame = debug_new(CLTGUIFrame);
	pFrame->Create(hFrame, frameCs);
	AddControl(pFrame);

	// Build the array of renderers
	GetRendererData( m_rendererData );

	// Add the "resolution" control
	CLTGUICycleCtrl_create ccs;
	ccs.nHeaderWidth = kGap;
	ccs.rnBaseRect.m_vMin = m_ScreenRect.m_vMin;
	ccs.rnBaseRect.m_vMax = m_ScreenRect.m_vMin + LTVector2n(kTotalWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));


	ccs.szHelpID = "IDS_HELP_RESOLUTION";
	ccs.pCommandHandler = this;
	ccs.nCommandID = CMD_DISPLAY_CYCLE;

    m_pResolutionCtrl = AddCycle("IDS_DISPLAY_RESOLUTION",ccs);

	// Setup the resolution control based on the current renderer
	SetupResolutionCtrl();


	CLTGUIToggle_create tcs;
	tcs.rnBaseRect.m_vMin.Init();
	tcs.rnBaseRect.m_vMax = LTVector2n(kGap+kWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));

	tcs.szHelpID = "IDS_HELP_VSYNC";
	tcs.pbValue = &m_bVSync;
	tcs.nHeaderWidth = kGap;
	AddToggle("IDS_VSYNC",tcs);

	tcs.szHelpID = "ScreenDisplay_RestartRender_Help";
	tcs.pbValue = &m_bRestartRenderBetweenMaps;
	tcs.nHeaderWidth = kGap;
	AddToggle("ScreenDisplay_RestartRender",tcs);


	tcs.szHelpID = "ScreenDisplay_TextScaling_Help";
	tcs.pbValue = &m_bUseTextScaling;
	tcs.nHeaderWidth = kGap;
	CLTGUIToggle* pTog = AddToggle("ScreenDisplay_TextScaling",tcs);
	pTog->SetOnString(LoadString("ScreenDisplay_TextScaling_Large"));
	pTog->SetOffString(LoadString("ScreenDisplay_TextScaling_Small"));


	CLTGUISlider_create scs;
	scs.rnBaseRect.m_vMin.Init();
	scs.rnBaseRect.m_vMax = LTVector2n(kGap+kWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	scs.nBarOffset = kGap;
	scs.szHelpID = "IDS_HELP_GAMMA";
	scs.pnValue = &m_nGamma;
	scs.nMin = 0;
	scs.nMax = kNumSteps;
	scs.nIncrement = 1;
	m_pGamma= AddSlider("IDS_GAMMA", scs );


	int nBaseGamma = ConvertToSlider(kDefaultGamma);
	float xoffset =  (m_pGamma->CalculateSliderOffset(nBaseGamma) / m_pGamma->GetScale().x);
	uint16 nHeight = m_pGamma->GetBarHeight();
	float yoffset = ((( (float)m_pGamma->GetBaseHeight()) - (float)nHeight)) / 2.0f;

	LTVector2n pos = m_pGamma->GetBasePos();
	pos.x += int(xoffset - 1.0f);
	pos.y += int(yoffset - 0.5f);

	CLTGUIFrame *pBar = debug_new(CLTGUIFrame);
	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(3,nHeight+2);

	pBar->Create(0xBF000000,cs);
	pBar->SetBasePos(pos);
	pBar->SetScale(g_pInterfaceResMgr->GetScreenScale());
	AddControl(pBar);

	CLTGUICtrl_create imageCs;
	hFrame.Load(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenAddTex));
	imageCs.rnBaseRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect,1);


	pFrame = debug_new(CLTGUIFrame);
	pFrame->Create(hFrame, imageCs, true);
	AddControl(pFrame);

	uint32 nFontHeight = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenAdditionalInt,0);
	cs.rnBaseRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect,2);
	m_pWarning = AddTextItem("ScreenDisplay_ResolutionWarning",cs,true,NULL,nFontHeight);
	m_pWarning->SetWordWrap(true);
	m_pWarning->Show(false);



 	// Make sure to call the base class
	if (!CBaseScreen::Build()) return false;

	UseBack(true,true);

	return true;

}

void CScreenDisplay::Escape()
{
	m_bEscape = true;
	CBaseScreen::Escape();
}

uint32 CScreenDisplay::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch( dwCommand )
	{
	case CMD_DISPLAY_CYCLE:
		CheckResolutionMemory();
		UpdateResolutionColor();
		break;
	}

	return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
};

// Setup the resolution control based on the currently selected resolution
void CScreenDisplay::SetupResolutionCtrl()
{
	if (!m_pResolutionCtrl)
	{
		return;
	}


	// Get the selected renderer
	int nResolutionIndex=m_pResolutionCtrl->GetSelIndex();

	// Clear the current resolutions
	m_pResolutionCtrl->RemoveAll();
    uint32 dwOldWidth=0;
    uint32 dwOldHeight=0;
    uint32 dwOldBitDepth=0;
    uint32 dwCurWidth = 0;
    uint32 dwCurHeight = 0;
    uint32 dwCurBitDepth = 0;
	int	 nNewRes = 0;

	unsigned int nResolutions=m_rendererData.m_resolutionArray.GetSize();

	if ((unsigned int)nResolutionIndex < nResolutions)
	{
		dwOldWidth=m_rendererData.m_resolutionArray[nResolutionIndex].m_dwWidth;
		dwOldHeight=m_rendererData.m_resolutionArray[nResolutionIndex].m_dwHeight;
		dwOldBitDepth=m_rendererData.m_resolutionArray[nResolutionIndex].m_dwBitDepth;
	}


	// Add each resolution
	unsigned int i;
	for (i=0; i < nResolutions; i++)
	{
        uint32 dwWidth=m_rendererData.m_resolutionArray[i].m_dwWidth;
        uint32 dwHeight=m_rendererData.m_resolutionArray[i].m_dwHeight;
        uint32 dwBitDepth=m_rendererData.m_resolutionArray[i].m_dwBitDepth;

        if (    LTDIFF(dwWidth,dwOldWidth) <= LTDIFF(dwCurWidth,dwOldWidth) &&
                LTDIFF(dwHeight,dwOldHeight) <= LTDIFF(dwCurHeight,dwOldHeight) &&
                LTDIFF(dwBitDepth,dwOldBitDepth) < LTDIFF(dwCurBitDepth,dwOldBitDepth)
				)
		{
			nNewRes = i;
			dwCurWidth = dwWidth;
			dwCurHeight = dwHeight;
			dwCurBitDepth = dwBitDepth;
		}
		// Load the resolution format string.  This is "Resolution: [%dx%dx%d]" in English
		wchar_t wszBuffer[92];
		FormatString( "IDS_DMODE_RESOLUTION", wszBuffer, LTARRAYSIZE(wszBuffer), dwWidth, dwHeight, dwBitDepth );

		uint32 testWidth = (dwHeight * 4 / 3);
		if (dwWidth != testWidth)
		{
			LTStrCat(wszBuffer,L" *",LTARRAYSIZE(wszBuffer));
		}


		m_pResolutionCtrl->AddString( wszBuffer );
	}

	m_pResolutionCtrl->SetSelIndex(nNewRes);

}
// Build the array of renderers
void CScreenDisplay::GetRendererData( ScreenDisplayRenderer& rendererData )
{
	rendererData.m_DeviceName[0] = '\0';

	// Build the list of render modes
    RMode *pRenderModes = g_pLTClient->GetRenderer()->GetRenderModes();

	bool bHWTnL = true;

	RMode currentMode;
	if (g_pGameClientShell->IsRendererInitted() && g_pLTClient->GetRenderMode(&currentMode) == LT_OK)
	{
		bHWTnL = currentMode.m_bHWTnL;
	}


	// Iterate through the list of render modes adding each one to the array
	RMode *pCurrentMode=pRenderModes;
    while (pCurrentMode != NULL)
	{
		if (pCurrentMode->m_Width >= 640 && pCurrentMode->m_Height >= 480 && pCurrentMode->m_BitDepth == 32)
		{
/*
			// disallow non-standard aspect ratios

			uint32 testWidth = (pCurrentMode->m_Height * 4 / 3);
			if (pCurrentMode->m_Width != testWidth)
			{
			
				// Go to the next render mode
				pCurrentMode=pCurrentMode->m_pNext;
				continue;
			}
*/
			//disallow any that aren't hardware TnL
			if(bHWTnL && !pCurrentMode->m_bHWTnL)
			{
				pCurrentMode=pCurrentMode->m_pNext;
				continue;
			}

			// Check to see if we need to add this renderer
			rendererData.m_bHWTnL = pCurrentMode->m_bHWTnL;
			LTStrCpy(rendererData.m_DeviceName, pCurrentMode->m_DeviceName, LTARRAYSIZE(rendererData.m_DeviceName));

			// Add the display resolutions for this renderer
			ScreenDisplayResolution resolution;
			resolution.m_dwWidth=pCurrentMode->m_Width;
			resolution.m_dwHeight=pCurrentMode->m_Height;
			resolution.m_dwBitDepth=pCurrentMode->m_BitDepth;

			bool bFound = false;
			uint32 i = 0;
			while (!bFound && i < rendererData.m_resolutionArray.GetSize())
			{
				bFound = (ScreenDisplayCompare(&(rendererData.m_resolutionArray[i]), &resolution) == 0);
				++i;
			}
			if (!bFound)
				rendererData.m_resolutionArray.Add(resolution);
		}

		// Go to the next render mode
		pCurrentMode=pCurrentMode->m_pNext;
	}

	// Free the linked list of render modes
    g_pLTClient->GetRenderer()->RelinquishRenderModes(pRenderModes);

	// Sort the render resolution based on screen width and height
	SortRenderModes( rendererData );
}

// Sort the render resolution based on screen width and height
void CScreenDisplay::SortRenderModes( ScreenDisplayRenderer& rendererData )
{
	// Build a temporary array of render modes
	int nResolutions=rendererData.m_resolutionArray.GetSize();
	if ( nResolutions < 1 )
	{
		return;
	}

	ScreenDisplayResolution *pResolutions = debug_newa(ScreenDisplayResolution, nResolutions);

	int i;
	for (i=0; i < nResolutions; i++)
	{
		pResolutions[i]=rendererData.m_resolutionArray[i];
	}

	// Sort the array
	qsort(pResolutions, nResolutions, sizeof(ScreenDisplayResolution), ScreenDisplayCompare);

	// Clear the current renderer resolutions array
	rendererData.m_resolutionArray.SetSize(0);

	// Copy the sorted array back to the resolutions array
	for (i=0; i < nResolutions; i++)
	{
		rendererData.m_resolutionArray.Add(pResolutions[i]);
	}

	delete []pResolutions;
}


// Gets a RMode structure based on a renderer index and a resolution index
RMode CScreenDisplay::GetRendererModeStruct(int nResolutionIndex)
{
	// Copy the renderer information from the renderer structure to the temporary RMode structure
	RMode mode;
	mode.m_bHWTnL = m_rendererData.m_bHWTnL;

	LTStrCpy(mode.m_DeviceName, m_rendererData.m_DeviceName, LTARRAYSIZE(mode.m_DeviceName));

	ScreenDisplayResolution resolution=m_rendererData.m_resolutionArray[nResolutionIndex];
	mode.m_Width=resolution.m_dwWidth;
	mode.m_Height=resolution.m_dwHeight;
	mode.m_BitDepth=resolution.m_dwBitDepth;

    mode.m_pNext=NULL;

	return mode;
}


// Called when the screen gains or focus
void CScreenDisplay::OnFocus(bool bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{
		// clear all warning flags
		m_dwFlags &= ~eFlagWarningMask;

		m_bEscape = false;

		pProfile->SetDisplay();

		m_bHardwareCursor = pProfile->m_bHardwareCursor;
		m_bVSync = pProfile->m_bVSync;
		m_bRestartRenderBetweenMaps = pProfile->m_bRestartRenderBetweenMaps;
		m_bUseTextScaling = pProfile->m_bUseTextScaling;

		float gamma = pProfile->m_fGamma;
		m_nGamma = ConvertToSlider(gamma);

		

//		m_pHardwareCursor->Enable(GetConsoleInt("DisableHardwareCursor",0) == 0);

		// The current render mode
		RMode currentMode;
		g_pLTClient->GetRenderMode(&currentMode);

		// Set the renderer controls so that they match the currently selected renderer
		unsigned int i;
		for (i=0; i < m_rendererData.m_resolutionArray.GetSize(); i++)
		{
			RMode mode=GetRendererModeStruct(i);

			if (IsRendererEqual(&currentMode, &mode))
			{
				// Setup the resolution control
				SetupResolutionCtrl();

				// Set the resolution index
				m_pResolutionCtrl->SetSelIndex(i);
			}
		}
			

        UpdateData(false);

		if( !CPerformanceMgr::Instance().ArePerformanceCapsValid() )
		{
			g_pInterfaceResMgr->DrawMessage("PerformanceMessage_DetectVideoMemory");
			CPerformanceMgr::Instance().DetectPerformanceStats( true );
		}

		UpdateResolutionColor();
		CheckResolutionMemory();
	}
	else
	{
		UpdateData();


		if (m_bEscape)
		{

			pProfile->m_bHardwareCursor = m_bHardwareCursor;
			pProfile->m_bVSync = m_bVSync;
			pProfile->m_bRestartRenderBetweenMaps = m_bRestartRenderBetweenMaps;
			pProfile->m_bUseTextScaling = m_bUseTextScaling;
			
			pProfile->m_fGamma = ConvertToGamma(m_nGamma);
			// Set the render mode if we are losing focus
			if (m_pResolutionCtrl)
			{
				// Get the new renderer structure
				RMode newMode=GetRendererModeStruct(m_pResolutionCtrl->GetSelIndex());

				pProfile->m_nScreenWidth = newMode.m_Width;
				pProfile->m_nScreenHeight = newMode.m_Height;
				pProfile->m_nScreenDepth = newMode.m_BitDepth;
			}
	
			pProfile->ApplyDisplay();

			pProfile->Save();
				
		}




	}
	CBaseScreen::OnFocus(bFocus);
}


// Returns the currently selected resolution
ScreenDisplayResolution CScreenDisplay::GetCurrentSelectedResolution()
{
	int nResolutionIndex=m_pResolutionCtrl->GetSelIndex();

	LTASSERT(nResolutionIndex >= 0, "");
	if (nResolutionIndex >= 0)
		return m_rendererData.m_resolutionArray[nResolutionIndex];
	else
		return m_rendererData.m_resolutionArray[0];
}

// Set the resolution for the resolution control.  If it cannot be found the
// next highest resolution is selected.
void CScreenDisplay::SetCurrentCtrlResolution(ScreenDisplayResolution resolution)
{

	// Go through the resolutions searching for a match
	unsigned int i;
	for (i=0; i < m_rendererData.m_resolutionArray.GetSize(); i++)
	{
		ScreenDisplayResolution searchRes=m_rendererData.m_resolutionArray[i];

		if (resolution.m_dwWidth == searchRes.m_dwWidth &&
			resolution.m_dwHeight == searchRes.m_dwHeight &&
			resolution.m_dwBitDepth == searchRes.m_dwBitDepth)
		{
			m_pResolutionCtrl->SetSelIndex(i);
			return;
		}
	}

	// Since an exact match wasn't found, set it to the next highest resolution
	for (i=0; i < m_rendererData.m_resolutionArray.GetSize(); i++)
	{
		ScreenDisplayResolution searchRes=m_rendererData.m_resolutionArray[i];

		if (resolution.m_dwWidth > searchRes.m_dwWidth ||
			resolution.m_dwHeight > searchRes.m_dwHeight &&
			resolution.m_dwBitDepth == searchRes.m_dwBitDepth)
		{
			if (i > 0)
			{
				m_pResolutionCtrl->SetSelIndex(i-1);
			}
			else
			{
				m_pResolutionCtrl->SetSelIndex(0);
			}
			return;
		}
	}
}





bool CScreenDisplay::OnLButtonUp(int x, int y)
{

	uint16 nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl == m_pGamma)
		{
			if (!pCtrl->OnLButtonUp(x,y)) return false;
			pCtrl->UpdateData();
			float fGamma = ConvertToGamma(m_nGamma);
			WriteConsoleFloat("GammaR",fGamma);
			WriteConsoleFloat("GammaG",fGamma);
			WriteConsoleFloat("GammaB",fGamma);
			return true;
		}
	}

	return CBaseScreen::OnLButtonUp(x, y);
}

bool CScreenDisplay::OnRButtonUp(int x, int y)
{
	uint16 nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl == m_pGamma)
		{
			if (!pCtrl->OnRButtonUp(x,y)) return false;
			pCtrl->UpdateData();
			float fGamma = ConvertToGamma(m_nGamma);
			WriteConsoleFloat("GammaR",fGamma);
			WriteConsoleFloat("GammaG",fGamma);
			WriteConsoleFloat("GammaB",fGamma);
			return true;
		}
	}
	return CBaseScreen::OnRButtonUp(x, y);
}


bool CScreenDisplay::OnLeft()
{
	if (!CBaseScreen::OnLeft()) return false;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl == m_pGamma)
	{
		pCtrl->UpdateData();
		float fGamma = ConvertToGamma(m_nGamma);
		WriteConsoleFloat("GammaR",fGamma);
		WriteConsoleFloat("GammaG",fGamma);
		WriteConsoleFloat("GammaB",fGamma);
	}


	return true;
}
bool CScreenDisplay::OnRight()
{
	if (!CBaseScreen::OnRight()) return false;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl == m_pGamma)
	{
		pCtrl->UpdateData();
		float fGamma = ConvertToGamma(m_nGamma);
		WriteConsoleFloat("GammaR",fGamma);
		WriteConsoleFloat("GammaG",fGamma);
		WriteConsoleFloat("GammaB",fGamma);
	}

	return true;
}
bool CScreenDisplay::OnEnter()
{
	return CBaseScreen::OnEnter();
}


void CScreenDisplay::ConfirmHardwareCursor(bool bReturn)
{
	UpdateData(true);
	m_bHardwareCursor = bReturn;
	g_pCursorMgr->UseHardwareCursor(m_bHardwareCursor);
	WriteConsoleInt("HardwareCursor",(int)m_bHardwareCursor);
	UpdateData(false);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenDisplay::CheckResolutionMemory()
//
//	PURPOSE:	checks to see if the user has blown their video memory limit
//
// ----------------------------------------------------------------------- //
void CScreenDisplay::CheckResolutionMemory()
{
	if( !(m_dwFlags & eFlag_ScreenResolutionWarning) )
	{
		float fMemoryUsed	= CPerformanceMgr::Instance().EstimateVideoMemoryUsage();
		float fMemoryTotal	= (float)CPerformanceMgr::Instance().GetPerformanceStats().m_nGPUMemory;
		if( fMemoryUsed > fMemoryTotal )
		{
			MBCreate mb;
			mb.eType = LTMB_OK;
			g_pInterfaceMgr->ShowMessageBox("PerformanceMessage_ScreenResolution",&mb);
			m_dwFlags |= eFlag_ScreenResolutionWarning;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenDisplay::UpdateResolutionColor()
//
//	PURPOSE:	update the color of the resolution based on memory usage
//
// ----------------------------------------------------------------------- //
void CScreenDisplay::UpdateResolutionColor()
{
	if( !m_pResolutionCtrl) return;

	if( CPerformanceMgr::Instance().ArePerformanceCapsValid() )
	{
		float fMemoryUsed	= CPerformanceMgr::Instance().EstimateVideoMemoryUsage();
		float fMemoryTotal	= (float)CPerformanceMgr::Instance().GetPerformanceStats().m_nGPUMemory;
		if ( fMemoryUsed > fMemoryTotal )
		{
			m_pResolutionCtrl->SetColor( m_nWarningColor );
		}
		else
		{
			m_pResolutionCtrl->SetColor( m_NonSelectedColor );
		}
	}

	ScreenDisplayResolution res = GetCurrentSelectedResolution();
	uint32 testWidth = (res.m_dwHeight * 4 / 3);
	if (res.m_dwWidth != testWidth)
	{
		m_pWarning->Show(true);
	}
	else
	{
		m_pWarning->Show(false);

	}


}

