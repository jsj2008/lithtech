// FolderDisplay.cpp: implementation of the CFolderDisplay class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderDisplay.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
#include "GameSettings.h"
extern CGameClientShell* g_pGameClientShell;

namespace
{
	void AreYouSureCallBack(LTBOOL bReturn, void *pData)
	{
		CFolderDisplay *pThisFolder = (CFolderDisplay *)pData;
		if (pThisFolder)
		{
			pThisFolder->ConfirmHardwareCursor(bReturn);
		}
	}
}

//helper function used for sorting
int FolderDisplayCompare( const void *arg1, const void *arg2 )
{
	FolderDisplayResolution *pRes1=(FolderDisplayResolution *)arg1;
	FolderDisplayResolution *pRes2=(FolderDisplayResolution *)arg2;

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

CFolderDisplay::CFolderDisplay()
{

    m_bTexture32    =   LTFALSE;
	m_bEscape		=	LTFALSE;

    m_pRendererLabel    = LTNULL;
    m_pResolutionLabel  = LTNULL;
    m_pRendererCtrl     = LTNULL;
    m_pResolutionCtrl   = LTNULL;
	m_pHardwareCursor	= LTNULL;
}

CFolderDisplay::~CFolderDisplay()
{

}

// Build the folder
LTBOOL CFolderDisplay::Build()
{

	CreateTitle(IDS_TITLE_DISPLAYOPTIONS);

//	m_pRendererLabel = AddTextItem(IDS_DISPLAY_RENDERER,	FOLDER_CMD_DISPLAY, IDS_HELP_RENDERER, LTTRUE);
//	if (m_pRendererLabel)
//	{
//        m_pRendererLabel->Enable(LTFALSE);
//	}

	// Build the array of renderers
	BuildRendererArray();

	// Add the "renderer" option
    m_pRendererCtrl = AddCycleItem(IDS_DISPLAY_RENDERER,IDS_HELP_RENDERER,100,25,LTNULL);

	unsigned int i;
	for (i=0; i < m_rendererArray.GetSize(); i++)
	{
		// Load the renderer formating text.  This is "Renderer: [%s - %s]" in English
        HSTRING hRendererFormat=g_pLTClient->FormatString(IDS_DMODE_RENDERER, m_rendererArray[i].m_renderDll, m_rendererArray[i].m_description);
		m_pRendererCtrl->AddString(hRendererFormat);
        g_pLTClient->FreeString(hRendererFormat);
	}

//	m_pResolutionLabel = AddTextItem(IDS_DISPLAY_RESOLUTION,	FOLDER_CMD_RESOLUTION, IDS_HELP_RESOLUTION, LTTRUE);
//	if (m_pResolutionLabel)
//	{
//		m_pResolutionLabel->Enable(LTFALSE);
//	}

	// Add the "resolution" control
    m_pResolutionCtrl = AddCycleItem(IDS_DISPLAY_RESOLUTION,IDS_HELP_RESOLUTION,150,25,LTNULL, LTFALSE);

	// Setup the resolution control based on the currently selected renderer
	SetupResolutionCtrl();

	CToggleCtrl *pToggle = AddToggle(IDS_DISPLAY_TEXTURE,IDS_HELP_TEXTUREDEPTH,175,&m_bTexture32);
	pToggle->SetOnString(IDS_DISPLAY_32BIT);
	pToggle->SetOffString(IDS_DISPLAY_16BIT);

	m_pHardwareCursor = AddToggle(IDS_HARDWARE_CURSOR,IDS_HELP_HARDWARE_CURSOR,175,&m_bHardwareCursor);
	m_pHardwareCursor->SetOnString(IDS_ON);
	m_pHardwareCursor->SetOffString(IDS_OFF);
	


	CalculateLastDrawn();


 	// Make sure to call the base class
	if (!CBaseFolder::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);

	return LTTRUE;

}

void CFolderDisplay::Escape()
{
	m_bEscape = LTTRUE;
	CBaseFolder::Escape();
}

uint32 CFolderDisplay::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
};

// Setup the resolution control based on the currently selected resolution
void CFolderDisplay::SetupResolutionCtrl()
{
	if (!m_pRendererCtrl || !m_pResolutionCtrl)
	{
		return;
	}


	// Get the selected renderer
	int nRendererIndex=m_pRendererCtrl->GetSelIndex();
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

	// Get the number of resolutions
	if (nRendererIndex < 0 || nRendererIndex > (int)m_rendererArray.GetSize()-1)
	{
		return;
	}

	unsigned int nResolutions=m_rendererArray[nRendererIndex].m_resolutionArray.GetSize();

	if ((unsigned int)nResolutionIndex < nResolutions)
	{
		dwOldWidth=m_rendererArray[nRendererIndex].m_resolutionArray[nResolutionIndex].m_dwWidth;
		dwOldHeight=m_rendererArray[nRendererIndex].m_resolutionArray[nResolutionIndex].m_dwHeight;
		dwOldBitDepth=m_rendererArray[nRendererIndex].m_resolutionArray[nResolutionIndex].m_dwBitDepth;
	}


	// Add each resolution
	unsigned int i;
	for (i=0; i < nResolutions; i++)
	{
        uint32 dwWidth=m_rendererArray[nRendererIndex].m_resolutionArray[i].m_dwWidth;
        uint32 dwHeight=m_rendererArray[nRendererIndex].m_resolutionArray[i].m_dwHeight;
        uint32 dwBitDepth=m_rendererArray[nRendererIndex].m_resolutionArray[i].m_dwBitDepth;

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
        HSTRING hResolutionFormat=g_pLTClient->FormatString(IDS_DMODE_RESOLUTION, dwWidth, dwHeight, dwBitDepth);
		m_pResolutionCtrl->AddString(hResolutionFormat);
        g_pLTClient->FreeString(hResolutionFormat);
	}

	m_pResolutionCtrl->SetSelIndex(nNewRes);

}

// Build the array of renderers
void CFolderDisplay::BuildRendererArray()
{
	// Clear the current renderer arrays
	unsigned int i;
	for (i=0; i < m_rendererArray.GetSize(); i++)
	{
		m_rendererArray[i].m_resolutionArray.SetSize(0);
	}
	m_rendererArray.SetSize(0);

	// Build the list of render modes
    RMode *pRenderModes=g_pLTClient->GetRenderModes();

	// Iterate through the list of render modes adding each one to the array
	RMode *pCurrentMode=pRenderModes;
    while (pCurrentMode != LTNULL)
	{
		if (pCurrentMode->m_Width >= 640 && pCurrentMode->m_Height >= 480)
		{
			// disallow non-standard aspect ratios
			uint32 testWidth = (pCurrentMode->m_Height * 4 / 3);
			if (pCurrentMode->m_Width != testWidth)
			{
			
				// Go to the next render mode
				pCurrentMode=pCurrentMode->m_pNext;
				continue;
			}

			// Get the index for this renderer
			int nRenderIndex=GetRendererIndex(pCurrentMode);

			// Check to see if we need to add this renderer
			if (nRenderIndex == -1)
			{
				FolderDisplayRenderer renderer;

				renderer.m_bHardware=pCurrentMode->m_bHardware;
				_mbscpy((unsigned char*)renderer.m_renderDll, (const unsigned char*)pCurrentMode->m_RenderDLL);
				_mbscpy((unsigned char*)renderer.m_description, (const unsigned char*)pCurrentMode->m_Description);
				_mbscpy((unsigned char*)renderer.m_internalName, (const unsigned char*)pCurrentMode->m_InternalName);

				m_rendererArray.Add(renderer);
				nRenderIndex=m_rendererArray.GetSize()-1;
			}

			// Add the display resolutions for this renderer
			FolderDisplayResolution resolution;
			resolution.m_dwWidth=pCurrentMode->m_Width;
			resolution.m_dwHeight=pCurrentMode->m_Height;
			resolution.m_dwBitDepth=pCurrentMode->m_BitDepth;

			m_rendererArray[nRenderIndex].m_resolutionArray.Add(resolution);
		}

		// Go to the next render mode
		pCurrentMode=pCurrentMode->m_pNext;
	}

	// Free the linked list of render modes
    g_pLTClient->RelinquishRenderModes(pRenderModes);

	// Sort the render resolution based on screen width and height
	for (i=0; i < m_rendererArray.GetSize(); i++)
	{
		SortRenderModes(i);
	}
}

// Sort the render resolution based on screen width and height
void CFolderDisplay::SortRenderModes(int nRendererIndex)
{
	// Build a temporary array of render modes
	int nResolutions=m_rendererArray[nRendererIndex].m_resolutionArray.GetSize();
	if ( nResolutions < 1 )
	{
		return;
	}

	FolderDisplayResolution *pResolutions = debug_newa(FolderDisplayResolution, nResolutions);

	int i;
	for (i=0; i < nResolutions; i++)
	{
		pResolutions[i]=m_rendererArray[nRendererIndex].m_resolutionArray[i];
	}

	// Sort the array
	qsort(pResolutions, nResolutions, sizeof(FolderDisplayResolution), FolderDisplayCompare);

	// Clear the current renderer resolutions array
	m_rendererArray[nRendererIndex].m_resolutionArray.SetSize(0);

	// Copy the sorted array back to the resolutions array
	for (i=0; i < nResolutions; i++)
	{
		m_rendererArray[nRendererIndex].m_resolutionArray.Add(pResolutions[i]);
	}

	debug_deletea(pResolutions);
}

// Returns an index into m_rendererArray for this renderer.
// -1 is returned if it cannot be found
int CFolderDisplay::GetRendererIndex(RMode *pMode)
{
	_ASSERT(pMode);
	if (!pMode)
	{
		return -1;
	}

	// Find out if this renderer already exists in the array
	int nRenderIndex=-1;

	unsigned int i;
	for (i=0; i < m_rendererArray.GetSize(); i++)
	{
		if (_mbsicmp((const unsigned char*)m_rendererArray[i].m_description, (const unsigned char*)pMode->m_Description) == 0 &&
			_mbsicmp((const unsigned char*)m_rendererArray[i].m_renderDll, (const unsigned char*)pMode->m_RenderDLL) == 0)
		{
			nRenderIndex=i;
			break;
		}
	}

	return nRenderIndex;
}

// Sets the renderer
LTBOOL CFolderDisplay::SetRenderer(int nRendererIndex, int nResolutionIndex)
{
	// Get the new renderer structure
	RMode newMode=GetRendererModeStruct(nRendererIndex, nResolutionIndex);

	// If the current renderer is the same as the one we are changing to, then don't set the renderer!
	RMode currentMode;
    if (g_pLTClient->GetRenderMode(&currentMode) == LT_OK)
	{
		if (IsRendererEqual(&newMode, &currentMode))
		{
            return LTFALSE;
		}
	}

	// Set the renderer mode
    g_pInterfaceMgr->SetSwitchingRenderModes(LTTRUE);
    g_pLTClient->SetRenderMode(&newMode);
    g_pInterfaceMgr->SetSwitchingRenderModes(LTFALSE);



	// Write the renderer information to the autoexec.cfg file
	char szTemp[256];
	sprintf(szTemp, "+RenderDll %s\0", newMode.m_RenderDLL);
    g_pLTClient->RunConsoleString(szTemp);

	sprintf(szTemp, "+CardDesc %s\0", newMode.m_InternalName);
    g_pLTClient->RunConsoleString(szTemp);

	sprintf(szTemp, "+screenwidth %d\0", newMode.m_Width);
    g_pLTClient->RunConsoleString(szTemp);

	sprintf(szTemp, "+screenheight %d\0", newMode.m_Height);
    g_pLTClient->RunConsoleString(szTemp);

	sprintf(szTemp, "+bitdepth %d\0", newMode.m_BitDepth);
    g_pLTClient->RunConsoleString(szTemp);


	g_pInterfaceMgr->ScreenDimsChanged();
	g_pInterfaceMgr->InitCursor();


    return LTTRUE;
}

// Gets a RMode structure based on a renderer index and a resolution index
RMode CFolderDisplay::GetRendererModeStruct(int nRendererIndex, int nResolutionIndex)
{
	// Copy the renderer information from the renderer structure to the temporary RMode structure
	RMode mode;
	mode.m_bHardware=m_rendererArray[nRendererIndex].m_bHardware;

	_mbscpy((unsigned char*)mode.m_RenderDLL, (const unsigned char*)m_rendererArray[nRendererIndex].m_renderDll);
	_mbscpy((unsigned char*)mode.m_InternalName, (const unsigned char*)m_rendererArray[nRendererIndex].m_internalName);
	_mbscpy((unsigned char*)mode.m_Description, (const unsigned char*)m_rendererArray[nRendererIndex].m_description);

	FolderDisplayResolution resolution=m_rendererArray[nRendererIndex].m_resolutionArray[nResolutionIndex];
	mode.m_Width=resolution.m_dwWidth;
	mode.m_Height=resolution.m_dwHeight;
	mode.m_BitDepth=resolution.m_dwBitDepth;

    mode.m_pNext=LTNULL;

	return mode;
}

// Returns TRUE if two renderers are the same
LTBOOL CFolderDisplay::IsRendererEqual(RMode *pRenderer1, RMode *pRenderer2)
{
	_ASSERT(pRenderer1);
	_ASSERT(pRenderer2);

	if (_mbsicmp((const unsigned char*)pRenderer1->m_RenderDLL, (const unsigned char*)pRenderer2->m_RenderDLL) != 0)
	{
        return LTFALSE;
	}

	if (_mbsicmp((const unsigned char*)pRenderer1->m_InternalName, (const unsigned char*)pRenderer2->m_InternalName) != 0)
	{
        return LTFALSE;
	}

	if (_mbsicmp((const unsigned char*)pRenderer1->m_Description, (const unsigned char*)pRenderer2->m_Description) != 0)
	{
        return LTFALSE;
	}

	if (pRenderer1->m_Width != pRenderer2->m_Width)
	{
        return LTFALSE;
	}

	if (pRenderer1->m_Height != pRenderer2->m_Height)
	{
        return LTFALSE;
	}

	if (pRenderer1->m_BitDepth != pRenderer2->m_BitDepth)
	{
        return LTFALSE;
	}

    return LTTRUE;
}

// Called when the folder gains or focus
void CFolderDisplay::OnFocus(LTBOOL bFocus)
{
	CGameSettings *pSettings = g_pInterfaceMgr->GetSettings();

	if (bFocus)
	{
		m_bEscape = LTFALSE;

		m_bHardwareCursor = (GetConsoleInt("HardwareCursor",0) > 0 && GetConsoleInt("DisableHardwareCursor",0) == 0);
		m_pHardwareCursor->Enable(GetConsoleInt("DisableHardwareCursor",0) == 0);

		// The current render mode
		RMode currentMode;
		g_pLTClient->GetRenderMode(&currentMode);

		// Set the renderer controls so that they match the currently selected renderer
		unsigned int n;
		for (n=0; n < m_rendererArray.GetSize(); n++)
		{
			unsigned int i;
			for (i=0; i < m_rendererArray[n].m_resolutionArray.GetSize(); i++)
			{
				RMode mode=GetRendererModeStruct(n, i);

				if (IsRendererEqual(&currentMode, &mode))
				{
					// Set the renderer index
					m_pRendererCtrl->SetSelIndex(n);

					// Setup the resolution control
					SetupResolutionCtrl();

					// Set the resolution index
					m_pResolutionCtrl->SetSelIndex(i);
				}
			}
		}
		m_bTexture32 = pSettings->GetBoolVar("32BitTextures");
	

        UpdateData(LTFALSE);
		SetSelection(1);

	}
	else
	{
		UpdateData();


		if (m_bEscape)
		{
			LTBOOL bTexture32 = pSettings->GetBoolVar("32BitTextures");

			WriteConsoleInt("HardwareCursor",(int)m_bHardwareCursor);

			pSettings->SetBoolVar("32BitTextures",m_bTexture32);


			LTBOOL bRebind = (bTexture32 != m_bTexture32);

			// Set the render mode if we are losing focus
			if (m_pRendererCtrl && m_pResolutionCtrl)
			{

				int oldMO = (int)pSettings->GetFloatVar("MipmapOffset");

				LTBOOL bSet = SetRenderer(m_pRendererCtrl->GetSelIndex(), m_pResolutionCtrl->GetSelIndex());

				// If we didn't switch resolutions and the mipmap offset changed, rebind textures.
				if(!bSet)
				{
					int curMO = (int)pSettings->GetFloatVar("MipmapOffset");
					if(curMO != oldMO)
					{
						bRebind = LTTRUE;
					}
				}
			}
			if (bRebind)
			{

                g_pLTClient->Start3D();
                g_pLTClient->StartOptimized2D();

				g_pInterfaceResMgr->DrawMessage(GetSmallFont(),IDS_REBINDING_TEXTURES);

                g_pLTClient->EndOptimized2D();
                g_pLTClient->End3D();
                g_pLTClient->FlipScreen(0);
				g_pLTClient->RunConsoleString("RebindTextures");

			}
				
		}

		if (GetConsoleInt("BitDepth",16) == 16)
			WriteConsoleInt("DrawPortals",0);
		else if ((GetConsoleInt("BitDepth",16) == 32) && (GetConsoleInt("PerformanceLevel",1) == 2))
			WriteConsoleInt("DrawPortals",1);

        g_pLTClient->WriteConfigFile("autoexec.cfg");


	}
	CBaseFolder::OnFocus(bFocus);
}

// Returns the currently selected resolution
FolderDisplayResolution CFolderDisplay::GetCurrentSelectedResolution()
{
	int nRendererIndex=m_pRendererCtrl->GetSelIndex();
	int nResolutionIndex=m_pResolutionCtrl->GetSelIndex();

	_ASSERT(nRendererIndex >= 0 && nResolutionIndex >= 0);
	if (nRendererIndex >= 0 && nResolutionIndex >= 0)
		return m_rendererArray[nRendererIndex].m_resolutionArray[nResolutionIndex];
	else
		return m_rendererArray[0].m_resolutionArray[0];
}

// Set the resolution for the resolution control.  If it cannot be found the
// next highest resolution is selected.
void CFolderDisplay::SetCurrentCtrlResolution(FolderDisplayResolution resolution)
{
	int nCurrentRenderer=m_pRendererCtrl->GetSelIndex();

	// Go through the resolutions searching for a match
	unsigned int i;
	for (i=0; i < m_rendererArray[nCurrentRenderer].m_resolutionArray.GetSize(); i++)
	{
		FolderDisplayResolution searchRes=m_rendererArray[nCurrentRenderer].m_resolutionArray[i];

		if (resolution.m_dwWidth == searchRes.m_dwWidth &&
			resolution.m_dwHeight == searchRes.m_dwHeight &&
			resolution.m_dwBitDepth == searchRes.m_dwBitDepth)
		{
			m_pResolutionCtrl->SetSelIndex(i);
			return;
		}
	}

	// Since an exact match wasn't found, set it to the next highest resolution
	for (i=0; i < m_rendererArray[nCurrentRenderer].m_resolutionArray.GetSize(); i++)
	{
		FolderDisplayResolution searchRes=m_rendererArray[nCurrentRenderer].m_resolutionArray[i];

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


/******************************************************************/
LTBOOL CFolderDisplay::OnLButtonUp(int x, int y)
{
	// Get the control that the click was on
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
            return LTFALSE;
		// If the mouse is over the same control now as it was when the down message was called
		// then send the "enter" message to the control.
		if (nControlIndex == m_nLMouseDownItemSel)
		{
			if (pCtrl->IsEnabled() )
			{
				if (pCtrl == m_pRendererCtrl)
				{
					// Get the current resolution
					FolderDisplayResolution currentResolution=GetCurrentSelectedResolution();
                    LTBOOL handled = pCtrl->OnLButtonUp(x, y);
					if (handled)
					{
						SetupResolutionCtrl();

						// Set the resolution for the control
						SetCurrentCtrlResolution(currentResolution);
					}
					return handled;
				}
				else if (pCtrl == m_pHardwareCursor)
				{
					return OnRight();
				}
				else
				{
					SetSelection(nControlIndex);
					return CBaseFolder::OnLButtonUp(x,y);
				}
			}

		}
	}
	else
	{
		m_nLMouseDownItemSel= kNoSelection;
	}
    return LTFALSE;
}


/******************************************************************/
LTBOOL CFolderDisplay::OnRButtonUp(int x, int y)
{
	// Get the control that the click was on
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
            return LTFALSE;

		// If the mouse is over the same control now as it was when the down message was called
		// then send the "left" message to the control.
		if (nControlIndex == m_nRMouseDownItemSel)
		{
			if (pCtrl->IsEnabled())
			{
				if (GetSelectedControl() == m_pRendererCtrl)
				{

					// Get the current resolution
					FolderDisplayResolution currentResolution=GetCurrentSelectedResolution();
                    LTBOOL handled = pCtrl->OnRButtonUp(x,y);
					if (handled)
					{
						SetupResolutionCtrl();
						// Set the resolution for the control
						SetCurrentCtrlResolution(currentResolution);
					}
					return handled;
				}
				else if (pCtrl == m_pHardwareCursor)
				{
					return OnLeft();
				}
				else
				{
					SetSelection(nControlIndex);
					return CBaseFolder::OnRButtonUp(x,y);
				}
			}
		}
	}
	else
	{
		m_nRMouseDownItemSel= kNoSelection;
	}
    return LTFALSE;
}


LTBOOL CFolderDisplay::OnLeft()
{
	if (!CBaseFolder::OnLeft()) return LTFALSE;
	if (GetSelectedControl() == m_pHardwareCursor)
	{
		m_pHardwareCursor->UpdateData(LTTRUE);
		if (m_bHardwareCursor)
		{
			HSTRING hString = g_pLTClient->FormatString(IDS_CONFIRM_CURSOR);
			g_pInterfaceMgr->ShowMessageBox(hString,LTMB_YESNO,AreYouSureCallBack,this,LTFALSE,LTFALSE);
			g_pLTClient->FreeString(hString);
		}
		else
		{
			g_pInterfaceMgr->UseHardwareCursor(LTFALSE);
			WriteConsoleInt("HardwareCursor",0);
		}
	}
	return LTTRUE;
}
LTBOOL CFolderDisplay::OnRight()
{
	if (!CBaseFolder::OnRight()) return LTFALSE;
	if (GetSelectedControl() == m_pHardwareCursor)
	{
		m_pHardwareCursor->UpdateData(LTTRUE);
		if (m_bHardwareCursor)
		{
			HSTRING hString = g_pLTClient->FormatString(IDS_CONFIRM_CURSOR);
			g_pInterfaceMgr->ShowMessageBox(hString,LTMB_YESNO,AreYouSureCallBack,this,LTFALSE,LTFALSE);
			g_pLTClient->FreeString(hString);
		}
		else
		{
			g_pInterfaceMgr->UseHardwareCursor(LTFALSE);
			WriteConsoleInt("HardwareCursor",0);
		}
	}
	return LTTRUE;
}
LTBOOL CFolderDisplay::OnEnter()
{
	return CBaseFolder::OnEnter();
}


void CFolderDisplay::ConfirmHardwareCursor(LTBOOL bReturn)
{
	UpdateData(LTTRUE);
	m_bHardwareCursor = bReturn;
	g_pInterfaceMgr->UseHardwareCursor(m_bHardwareCursor);
	WriteConsoleInt("HardwareCursor",(int)m_bHardwareCursor);
	UpdateData(LTFALSE);
}

