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
#include "ClientRes.h"

#include "GameClientShell.h"
#include "GameSettings.h"
extern CGameClientShell* g_pGameClientShell;

namespace
{
	int kGap = 0;
	int kWidth = 100;
	void AreYouSureCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenDisplay *pThisScreen = (CScreenDisplay *)pData;
		if (pThisScreen)
		{
			pThisScreen->ConfirmHardwareCursor(bReturn);
		}
	}
	LTBOOL bInitTex;
	LTBOOL bInitLM;

	const int	kNumSteps = 50;
	const float	kBreakpoint = (float)(kNumSteps/3);
	const float	kUpperSteps = (float)kNumSteps - kBreakpoint;
	const float fLowerRange = 1.0f - kMinGamma;
	const float fUpperRange = kMaxGamma - 1.0f;
	float ConvertToGamma(int slider)
	{
		float gamma = 1.0f;
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
			gamma = 1.0f + fRatio * fUpperRange;
		}

//		g_pLTClient->CPrint("Gamma: %0.2f",gamma);
//		g_pLTClient->CPrint("Slider: %d",slider);
		return gamma;

	}

	int	ConvertToSlider(float gamma)
	{
		int slider = 5;
		if (gamma < 1.0f)
		{
			float underVal = gamma - kMinGamma;
			float fRatio = underVal / fLowerRange;
			fRatio *= fRatio;
			slider = (int)(0.5f + kBreakpoint * fRatio);
			
		}
		else
		{
			float overVal = gamma - 1.0f;
			float fRatio = overVal / fUpperRange;
			fRatio = (float)sqrt(fRatio);
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

CScreenDisplay::CScreenDisplay()
{

	m_bEscape		=	LTFALSE;

    m_pResolutionCtrl   = LTNULL;
	m_pHardwareCursor	= LTNULL;
}

CScreenDisplay::~CScreenDisplay()
{

}

// Build the screen
LTBOOL CScreenDisplay::Build()
{

	CreateTitle(IDS_TITLE_DISPLAYOPTIONS);

	kGap = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_DISPLAY,"ColumnWidth");
	kWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_DISPLAY,"SliderWidth");

	//background frame
	LTRect frameRect = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_DISPLAY,"FrameRect");
	LTIntPt pos(frameRect.left,frameRect.top);
	int nHt = frameRect.bottom - frameRect.top;
	int nWd = frameRect.right - frameRect.left;

	char szFrame[128];
	g_pLayoutMgr->GetScreenCustomString(SCREEN_ID_DISPLAY,"FrameTexture",szFrame,sizeof(szFrame));
	HTEXTURE hFrame = g_pInterfaceResMgr->GetTexture(szFrame);
	CLTGUIFrame *pFrame = debug_new(CLTGUIFrame);
	pFrame->Create(hFrame,nWd,nHt+8,LTTRUE);
	pFrame->SetBasePos(pos);
	pFrame->SetBorder(2,m_SelectedColor);
	AddControl(pFrame);

	// Build the array of renderers
	GetRendererData();

	// Add the "resolution" control
    m_pResolutionCtrl = AddCycle(IDS_DISPLAY_RESOLUTION,IDS_HELP_RESOLUTION,kGap);

	// Setup the resolution control based on the current renderer
	SetupResolutionCtrl();

	m_pHardwareCursor = AddToggle(IDS_HARDWARE_CURSOR,IDS_HELP_HARDWARE_CURSOR,kGap,&m_bHardwareCursor);

	AddToggle(IDS_VSYNC,IDS_HELP_VSYNC,kGap,&m_bVSync);


	m_pGamma = AddSlider(IDS_GAMMA,IDS_HELP_GAMMA,kGap,kWidth,-1,&m_nGamma);
	m_pGamma->SetSliderRange(0,kNumSteps);
	m_pGamma->SetSliderIncrement(1);

	int nBaseGamma = ConvertToSlider(1.0f);
	float xoffset =  (m_pGamma->CalculateSliderOffset(nBaseGamma) / m_pGamma->GetScale());
	uint16 nHeight = m_pGamma->GetBarHeight();
	float yoffset = ((( (float)m_pGamma->GetBaseHeight()) - (float)nHeight)) / 2.0f;

	pos = m_pGamma->GetBasePos();
	pos.x += (int)(xoffset - 1.0f);
	pos.y += (int)(yoffset + 0.5f);

	CLTGUIFrame *pBar = debug_new(CLTGUIFrame);
	pBar->Create(0xBF000000,3,nHeight+1);
	pBar->SetBasePos(pos);
	pBar->SetScale(g_pInterfaceResMgr->GetXRatio());
	AddControl(pBar);





 	// Make sure to call the base class
	if (!CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);

	return LTTRUE;

}

void CScreenDisplay::Escape()
{
	m_bEscape = LTTRUE;
	CBaseScreen::Escape();
}

uint32 CScreenDisplay::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
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
		m_pResolutionCtrl->AddString(FormatTempString(IDS_DMODE_RESOLUTION, dwWidth, dwHeight, dwBitDepth));
	}

	m_pResolutionCtrl->SetSelIndex(nNewRes);

}
// Build the array of renderers
void CScreenDisplay::GetRendererData()
{
	m_rendererData.m_description[0] = '\0';
	m_rendererData.m_internalName[0] = '\0';

	// Build the list of render modes
    RMode *pRenderModes=g_pLTClient->GetRenderModes();

	bool bHWTnL = true;

	RMode currentMode;
	if (g_pGameClientShell->IsRendererInitted() && g_pLTClient->GetRenderMode(&currentMode) == LT_OK)
	{
		bHWTnL = currentMode.m_bHWTnL;
	}


	// Iterate through the list of render modes adding each one to the array
	RMode *pCurrentMode=pRenderModes;
    while (pCurrentMode != LTNULL)
	{
		if (pCurrentMode->m_Width >= 640 && pCurrentMode->m_Height >= 480 && pCurrentMode->m_BitDepth == 32)
		{
			// disallow non-standard aspect ratios
			uint32 testWidth = (pCurrentMode->m_Height * 4 / 3);
			if (pCurrentMode->m_Width != testWidth)
			{
			
				// Go to the next render mode
				pCurrentMode=pCurrentMode->m_pNext;
				continue;
			}

			//disallow any that aren't hardware TnL
			if(bHWTnL && !pCurrentMode->m_bHWTnL)
			{
				pCurrentMode=pCurrentMode->m_pNext;
				continue;
			}

			// Check to see if we need to add this renderer
			if (!m_rendererData.m_description[0])
			{
				m_rendererData.m_bHWTnL = pCurrentMode->m_bHWTnL;
				SAFE_STRCPY(m_rendererData.m_description, pCurrentMode->m_Description);
				SAFE_STRCPY(m_rendererData.m_internalName, pCurrentMode->m_InternalName);

			}

			// Add the display resolutions for this renderer
			ScreenDisplayResolution resolution;
			resolution.m_dwWidth=pCurrentMode->m_Width;
			resolution.m_dwHeight=pCurrentMode->m_Height;
			resolution.m_dwBitDepth=pCurrentMode->m_BitDepth;

			LTBOOL bFound = LTFALSE;
			uint32 i = 0;
			while (!bFound && i < m_rendererData.m_resolutionArray.GetSize())
			{
				bFound = (ScreenDisplayCompare(&(m_rendererData.m_resolutionArray[i]), &resolution) == 0);
				++i;
			}
			if (!bFound)
				m_rendererData.m_resolutionArray.Add(resolution);
		}

		// Go to the next render mode
		pCurrentMode=pCurrentMode->m_pNext;
	}

	// Free the linked list of render modes
    g_pLTClient->RelinquishRenderModes(pRenderModes);

	// Sort the render resolution based on screen width and height
	SortRenderModes();
}

// Sort the render resolution based on screen width and height
void CScreenDisplay::SortRenderModes()
{
	// Build a temporary array of render modes
	int nResolutions=m_rendererData.m_resolutionArray.GetSize();
	if ( nResolutions < 1 )
	{
		return;
	}

	ScreenDisplayResolution *pResolutions = new ScreenDisplayResolution[nResolutions];

	int i;
	for (i=0; i < nResolutions; i++)
	{
		pResolutions[i]=m_rendererData.m_resolutionArray[i];
	}

	// Sort the array
	qsort(pResolutions, nResolutions, sizeof(ScreenDisplayResolution), ScreenDisplayCompare);

	// Clear the current renderer resolutions array
	m_rendererData.m_resolutionArray.SetSize(0);

	// Copy the sorted array back to the resolutions array
	for (i=0; i < nResolutions; i++)
	{
		m_rendererData.m_resolutionArray.Add(pResolutions[i]);
	}

	delete []pResolutions;
}


// Gets a RMode structure based on a renderer index and a resolution index
RMode CScreenDisplay::GetRendererModeStruct(int nResolutionIndex)
{
	// Copy the renderer information from the renderer structure to the temporary RMode structure
	RMode mode;
	mode.m_bHWTnL = m_rendererData.m_bHWTnL;

	SAFE_STRCPY(mode.m_InternalName, m_rendererData.m_internalName);
	SAFE_STRCPY(mode.m_Description, m_rendererData.m_description);

	ScreenDisplayResolution resolution=m_rendererData.m_resolutionArray[nResolutionIndex];
	mode.m_Width=resolution.m_dwWidth;
	mode.m_Height=resolution.m_dwHeight;
	mode.m_BitDepth=resolution.m_dwBitDepth;

    mode.m_pNext=LTNULL;

	return mode;
}

// Returns TRUE if two renderers are the same

// Called when the screen gains or focus
void CScreenDisplay::OnFocus(LTBOOL bFocus)
{
		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	if (bFocus)
	{
		m_bEscape = LTFALSE;

		pProfile->SetDisplay();

		m_bHardwareCursor = pProfile->m_bHardwareCursor;
		m_bVSync = pProfile->m_bVSync;


		float gamma = pProfile->m_fGamma;

		m_nGamma = ConvertToSlider(gamma);

		

		m_pHardwareCursor->Enable(GetConsoleInt("DisableHardwareCursor",0) == 0);

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
			

        UpdateData(LTFALSE);

	}
	else
	{
		UpdateData();


		if (m_bEscape)
		{

			pProfile->m_bHardwareCursor = m_bHardwareCursor;
			pProfile->m_bVSync = m_bVSync;
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
					
			//display settings can affect performance sttings so update them here...
			g_pPerformanceMgr->GetPerformanceOptions(&pProfile->m_sPerformance);

			pProfile->Save();
				
		}




	}
	CBaseScreen::OnFocus(bFocus);
}


// Returns the currently selected resolution
ScreenDisplayResolution CScreenDisplay::GetCurrentSelectedResolution()
{
	int nResolutionIndex=m_pResolutionCtrl->GetSelIndex();

	_ASSERT(nResolutionIndex >= 0);
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





LTBOOL CScreenDisplay::OnLButtonUp(int x, int y)
{
	uint16 nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl == m_pHardwareCursor)
		{
			return OnRight();
		}
		if (pCtrl == m_pGamma)
		{
			if (!pCtrl->OnLButtonUp(x,y)) return LTFALSE;
			pCtrl->UpdateData();
			float fGamma = ConvertToGamma(m_nGamma);
			WriteConsoleFloat("GammaR",fGamma);
			WriteConsoleFloat("GammaG",fGamma);
			WriteConsoleFloat("GammaB",fGamma);
			return LTTRUE;
		}
	}
	return CBaseScreen::OnLButtonUp(x, y);
}

LTBOOL CScreenDisplay::OnRButtonUp(int x, int y)
{
	uint16 nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl == m_pHardwareCursor)
		{
			return OnLeft();
		}
		if (pCtrl == m_pGamma)
		{
			if (!pCtrl->OnRButtonUp(x,y)) return LTFALSE;
			pCtrl->UpdateData();
			float fGamma = ConvertToGamma(m_nGamma);
			WriteConsoleFloat("GammaR",fGamma);
			WriteConsoleFloat("GammaG",fGamma);
			WriteConsoleFloat("GammaB",fGamma);
			return LTTRUE;
		}
	}
	return CBaseScreen::OnRButtonUp(x, y);
}


LTBOOL CScreenDisplay::OnLeft()
{
	if (!CBaseScreen::OnLeft()) return LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl == m_pHardwareCursor)
	{
		m_pHardwareCursor->UpdateData(LTTRUE);
		if (m_bHardwareCursor)
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = AreYouSureCallBack;
			mb.pData = this;
			g_pInterfaceMgr->ShowMessageBox(IDS_CONFIRM_CURSOR,&mb,0,LTFALSE);
		}
		else
		{
			g_pCursorMgr->UseHardwareCursor(LTFALSE);
			WriteConsoleInt("HardwareCursor",0);
		}
	}
	if (GetSelectedControl() == m_pGamma)
	{
		pCtrl->UpdateData();
		float fGamma = ConvertToGamma(m_nGamma);
		WriteConsoleFloat("GammaR",fGamma);
		WriteConsoleFloat("GammaG",fGamma);
		WriteConsoleFloat("GammaB",fGamma);
	}

	return LTTRUE;
}
LTBOOL CScreenDisplay::OnRight()
{
	if (!CBaseScreen::OnRight()) return LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl == m_pHardwareCursor)
	{
		m_pHardwareCursor->UpdateData(LTTRUE);
		if (m_bHardwareCursor)
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = AreYouSureCallBack;
			mb.pData = this;
			g_pInterfaceMgr->ShowMessageBox(IDS_CONFIRM_CURSOR,&mb,0,LTFALSE);
		}
		else
		{
			g_pCursorMgr->UseHardwareCursor(LTFALSE);
			WriteConsoleInt("HardwareCursor",0);
		}
	}
	if (pCtrl == m_pGamma)
	{
		pCtrl->UpdateData();
		float fGamma = ConvertToGamma(m_nGamma);
		WriteConsoleFloat("GammaR",fGamma);
		WriteConsoleFloat("GammaG",fGamma);
		WriteConsoleFloat("GammaB",fGamma);
	}
	return LTTRUE;
}
LTBOOL CScreenDisplay::OnEnter()
{
	return CBaseScreen::OnEnter();
}


void CScreenDisplay::ConfirmHardwareCursor(LTBOOL bReturn)
{
	UpdateData(LTTRUE);
	m_bHardwareCursor = bReturn;
	g_pCursorMgr->UseHardwareCursor(m_bHardwareCursor);
	WriteConsoleInt("HardwareCursor",(int)m_bHardwareCursor);
	UpdateData(LTFALSE);
}


