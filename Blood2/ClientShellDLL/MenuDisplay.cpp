// MenuDisplay.cpp: implementation of the CMenuDisplay class.
//
//////////////////////////////////////////////////////////////////////

#include "MenuBase.h"
#include "MainMenus.h"
#include "MenuDisplay.h"
#include "MenuCommands.h"
#include "BloodClientShell.h"
#include "ClientRes.h"

int MenuDisplayCompare( const void *arg1, const void *arg2 )
{
	MenuDisplayResolution *pRes1=(MenuDisplayResolution *)arg1;
	MenuDisplayResolution *pRes2=(MenuDisplayResolution *)arg2;

	if (pRes1->m_dwWidth < pRes2->m_dwWidth)
	{
		return -1;
	}
	if (pRes1->m_dwWidth > pRes2->m_dwWidth)
	{
		return 1;
	}
	else
	{
		if ( pRes1->m_dwHeight < pRes2->m_dwHeight )
		{
			return -1;
		}
		if ( pRes1->m_dwHeight > pRes2->m_dwHeight )
		{
			return 1;
		}		
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuDisplay::CMenuDisplay()
{
	m_pRendererCtrl=DNULL;
	m_pResolutionCtrl=DNULL;
	m_nDetailLevel=2;
}

CMenuDisplay::~CMenuDisplay()
{	
}

// Build the menu
void CMenuDisplay::Build()
{
	// Make sure to call the base class
	CMenuBase::Build();

	CreateTitle("interface\\mainmenus\\options.pcx", IDS_MENU_TITLE_OPTIONS, m_pMainMenus->GetTitlePos());		
	SetOptionPos(m_pMainMenus->GetOptionsPos());
	SetItemSpacing(0);
	SetScrollWrap(DFALSE);	

	// Show the please wait message
	m_pMainMenus->ShowSyncMessage(IDS_MENU_MESSAGE_PLEASEWAIT);

	// Build the array of renderers
	BuildRendererArray();

	// Add the "renderer" option
	m_pRendererCtrl=AddTextItemOption(DNULL, 0, m_pMainMenus->GetSmallFont());
	if (!m_pRendererCtrl)
	{
		return;
	}

	unsigned int i;
	for (i=0; i < m_rendererArray.GetSize(); i++)
	{
		// Load the renderer formating text.  This is "Renderer: [%s - %s]" in English
		HSTRING hRendererFormat=m_pClientDE->FormatString(IDS_MENU_DISPLAY_RENDERER, m_rendererArray[i].m_renderDll, m_rendererArray[i].m_description);		
		m_pRendererCtrl->AddString(hRendererFormat);		
		m_pClientDE->FreeString(hRendererFormat);
	}	

	// Add the "resolution" control
	m_pResolutionCtrl=AddTextItemOption(DNULL, 0, m_pMainMenus->GetSmallFont());
	if (!m_pResolutionCtrl)
	{
		return;
	}

	// Setup the resolution control based on the currently selected renderer
	SetupResolutionCtrl();

	// Add the detail option
	CLTGUITextItemCtrl *pCtrl=AddTextItemOption(DNULL, 0, m_pMainMenus->GetSmallFont(), 1, DTRUE, &m_nDetailLevel);
	pCtrl->AddString(IDS_MENU_DISPLAY_DETAIL_LOW);
	pCtrl->AddString(IDS_MENU_DISPLAY_DETAIL_MEDIUM);
	pCtrl->AddString(IDS_MENU_DISPLAY_DETAIL_HIGH);

	// Load the detail setting
	HCONSOLEVAR hVar=m_pClientDE->GetConsoleVar("GlobalDetail");
	if (hVar)
	{
		switch ((int)m_pClientDE->GetVarValueFloat(hVar))
		{
		case DETAIL_LOW:
			{
				m_nDetailLevel=0;
				break;
			}
		case DETAIL_MEDIUM:
			{
				m_nDetailLevel=1;
				break;
			}
		case DETAIL_HIGH:
			{
				m_nDetailLevel=2;
				break;
			}
		}
	}
	else
	{
		m_nDetailLevel=2;
	}

	UpdateData(DFALSE);
}

// Setup the resolution control based on the currently selected resolution
void CMenuDisplay::SetupResolutionCtrl()
{
	if (!m_pRendererCtrl || !m_pResolutionCtrl)
	{
		return;
	}

	// Clear the current resolutions
	m_pResolutionCtrl->RemoveAll();

	// Get the selected renderer
	int nRendererIndex=m_pRendererCtrl->GetSelIndex();

	// Get the number of resolutions
	if (nRendererIndex > (int)m_rendererArray.GetSize()-1)
	{
		return;
	}

	unsigned int nResolutions=m_rendererArray[nRendererIndex].m_resolutionArray.GetSize();
	
	// Add each resolution
	unsigned int i;
	for (i=0; i < nResolutions; i++)
	{
		DDWORD dwWidth=m_rendererArray[nRendererIndex].m_resolutionArray[i].m_dwWidth;
		DDWORD dwHeight=m_rendererArray[nRendererIndex].m_resolutionArray[i].m_dwHeight;
		DDWORD dwBitDepth=m_rendererArray[nRendererIndex].m_resolutionArray[i].m_dwBitDepth;

		// Load the resolution format string.  This is "Resolution: [%dx%dx%d]" in English
		HSTRING hResolutionFormat=m_pClientDE->FormatString(IDS_MENU_DISPLAY_RESOLUTION, dwWidth, dwHeight, dwBitDepth);		
		m_pResolutionCtrl->AddString(hResolutionFormat);
		m_pClientDE->FreeString(hResolutionFormat);
	}	

	m_pResolutionCtrl->SetSelIndex(0);
}

// Build the array of renderers
void CMenuDisplay::BuildRendererArray()
{
	// Clear the current renderer arrays
	unsigned int i;
	for (i=0; i < m_rendererArray.GetSize(); i++)
	{
		m_rendererArray[i].m_resolutionArray.SetSize(0);
	}
	m_rendererArray.SetSize(0);

	// Build the list of render modes
	RMode *pRenderModes=m_pClientDE->GetRenderModes();

	// Iterate through the list of render modes adding each one to the array
	RMode *pCurrentMode=pRenderModes;
	while (pCurrentMode != DNULL)
	{		
		// Get the index for this renderer
		int nRenderIndex=GetRendererIndex(pCurrentMode);

		// Check to see if we need to add this renderer
		if (nRenderIndex == -1)
		{
			MenuDisplayRenderer renderer;

			renderer.m_bHardware=pCurrentMode->m_bHardware;
			_mbscpy((unsigned char*)renderer.m_renderDll, (const unsigned char*)pCurrentMode->m_RenderDLL);
			_mbscpy((unsigned char*)renderer.m_description, (const unsigned char*)pCurrentMode->m_Description);
			_mbscpy((unsigned char*)renderer.m_internalName, (const unsigned char*)pCurrentMode->m_InternalName);

			m_rendererArray.Add(renderer);
			nRenderIndex=m_rendererArray.GetSize()-1;
		}

		// Add the display resolutions for this renderer
		MenuDisplayResolution resolution;
		resolution.m_dwWidth=pCurrentMode->m_Width;
		resolution.m_dwHeight=pCurrentMode->m_Height;
		resolution.m_dwBitDepth=pCurrentMode->m_BitDepth;

		m_rendererArray[nRenderIndex].m_resolutionArray.Add(resolution);

		// Go to the next render mode
		pCurrentMode=pCurrentMode->m_pNext;
	}

	// Free the linked list of render modes
	m_pClientDE->RelinquishRenderModes(pRenderModes);

	// Sort the render resolution based on screen width and height
	for (i=0; i < m_rendererArray.GetSize(); i++)
	{
		SortRenderModes(i);
	}
}

// Sort the render resolution based on screen width and height
void CMenuDisplay::SortRenderModes(int nRendererIndex)
{	
	// Build a temporary array of render modes
	int nResolutions=m_rendererArray[nRendererIndex].m_resolutionArray.GetSize();
	if ( nResolutions < 1 )
	{
		return;
	}

	MenuDisplayResolution *pResolutions = new MenuDisplayResolution[nResolutions];

	int i;
	for (i=0; i < nResolutions; i++)
	{
		pResolutions[i]=m_rendererArray[nRendererIndex].m_resolutionArray[i];
	}

	// Sort the array
	qsort(pResolutions, nResolutions, sizeof(MenuDisplayResolution), MenuDisplayCompare);
	
	// Clear the current renderer resolutions array
	m_rendererArray[nRendererIndex].m_resolutionArray.SetSize(0);

	// Copy the sorted array back to the resolutions array	
	for (i=0; i < nResolutions; i++)
	{
		m_rendererArray[nRendererIndex].m_resolutionArray.Add(pResolutions[i]);
	}
}

// Returns an index into m_rendererArray for this renderer.
// -1 is returned if it cannot be found
int CMenuDisplay::GetRendererIndex(RMode *pMode)
{
	assert(pMode);
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
DBOOL CMenuDisplay::SetRenderer(int nRendererIndex, int nResolutionIndex)
{
	// Get the new renderer structure
	RMode newMode=GetRendererModeStruct(nRendererIndex, nResolutionIndex);

	// If the current renderer is the same as the one we are changing to, then don't set the renderer!
	RMode currentMode;
	if (m_pClientDE->GetRenderMode(&currentMode) == LT_OK)
	{	
		if (IsRendererEqual(&newMode, &currentMode))
		{
			return DFALSE;
		}
	}

	// Set the renderer mode
	m_pClientDE->SetRenderMode(&newMode);

	// Set the menu resolution size
	if (newMode.m_Width < 512 || newMode.m_Height < 384)
	{
		m_pMainMenus->SetLowResolution(DTRUE);
	}
	else
	{
		m_pMainMenus->SetLowResolution(DFALSE);
	}

	// Write the renderer information to the autoexec.cfg file
	char szTemp[256];
	sprintf(szTemp, "++RenderDll %s\0", newMode.m_RenderDLL);
	m_pClientDE->RunConsoleString(szTemp);

	sprintf(szTemp, "++CardDesc %s\0", newMode.m_InternalName);
	m_pClientDE->RunConsoleString(szTemp);

	sprintf(szTemp, "++screenwidth %d\0", newMode.m_Width);
	m_pClientDE->RunConsoleString(szTemp);

	sprintf(szTemp, "++screenheight %d\0", newMode.m_Height);
	m_pClientDE->RunConsoleString(szTemp);

	sprintf(szTemp, "++screendepth %d\0", newMode.m_BitDepth);
	m_pClientDE->RunConsoleString(szTemp);

	return DTRUE;
}

// Gets a RMode structure based on a renderer index and a resolution index
RMode CMenuDisplay::GetRendererModeStruct(int nRendererIndex, int nResolutionIndex)
{
	// Copy the renderer information from the renderer structure to the temporary RMode structure
	RMode mode;
	mode.m_bHardware=m_rendererArray[nRendererIndex].m_bHardware;	
	
	_mbscpy((unsigned char*)mode.m_RenderDLL, (const unsigned char*)m_rendererArray[nRendererIndex].m_renderDll);
	_mbscpy((unsigned char*)mode.m_InternalName, (const unsigned char*)m_rendererArray[nRendererIndex].m_internalName);
	_mbscpy((unsigned char*)mode.m_Description, (const unsigned char*)m_rendererArray[nRendererIndex].m_description);

	MenuDisplayResolution resolution=m_rendererArray[nRendererIndex].m_resolutionArray[nResolutionIndex];
	mode.m_Width=resolution.m_dwWidth;
	mode.m_Height=resolution.m_dwHeight;
	mode.m_BitDepth=resolution.m_dwBitDepth;

	mode.m_pNext=DNULL;

	return mode;
}

// Returns TRUE if two renderers are the same
DBOOL CMenuDisplay::IsRendererEqual(RMode *pRenderer1, RMode *pRenderer2)
{
	assert(pRenderer1);
	assert(pRenderer2);

	if (_mbsicmp((const unsigned char*)pRenderer1->m_RenderDLL, (const unsigned char*)pRenderer2->m_RenderDLL) != 0)
	{
		return DFALSE;
	}
	
	if (_mbsicmp((const unsigned char*)pRenderer1->m_InternalName, (const unsigned char*)pRenderer2->m_InternalName) != 0)
	{
		return DFALSE;
	}
	
	if (_mbsicmp((const unsigned char*)pRenderer1->m_Description, (const unsigned char*)pRenderer2->m_Description) != 0)
	{
		return DFALSE;
	}
	
	if (pRenderer1->m_Width != pRenderer2->m_Width)
	{
		return DFALSE;
	}

	if (pRenderer1->m_Height != pRenderer2->m_Height)
	{
		return DFALSE;
	}

	if (pRenderer1->m_BitDepth != pRenderer2->m_BitDepth)
	{
		return DFALSE;
	}

	return DTRUE;
}

// Called when the menu loses focus
void CMenuDisplay::OnFocus(DBOOL bFocus)
{
	HCONSOLEVAR	hVar;
	int curMO, oldMO;
	DBOOL bSet;

	// Set the render mode if we are losing focus
	if (!bFocus && m_pRendererCtrl && m_pResolutionCtrl)
	{
		hVar = m_pClientDE->GetConsoleVar("MipmapOffset");
		oldMO = (int)m_pClientDE->GetVarValueFloat (hVar);

		UpdateData();

		// Save the global detail setting
		SaveDetailSetting();		

		bSet = SetRenderer(m_pRendererCtrl->GetSelIndex(), m_pResolutionCtrl->GetSelIndex());

		// If we didn't switch resolutions and the mipmap offset changed, rebind textures.
		if(!bSet)
		{
			hVar = m_pClientDE->GetConsoleVar("MipmapOffset");
			curMO = (int)m_pClientDE->GetVarValueFloat (hVar);
			if(curMO != oldMO)
			{
				m_pClientDE->RunConsoleString("RebindTextures");
			}
		}
	}
	else
	{
		// The current render mode
		RMode currentMode;
		m_pClientDE->GetRenderMode(&currentMode);		

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
	}
}

// Returns the currently selected resolution
MenuDisplayResolution CMenuDisplay::GetCurrentSelectedResolution()
{
	int nRendererIndex=m_pRendererCtrl->GetSelIndex();
	int nResolutionIndex=m_pResolutionCtrl->GetSelIndex();

	return m_rendererArray[nRendererIndex].m_resolutionArray[nResolutionIndex];
}

// Set the resolution for the resolution control.  If it cannot be found the
// next highest resolution is selected.
void CMenuDisplay::SetCurrentCtrlResolution(MenuDisplayResolution resolution)
{
	int nCurrentRenderer=m_pRendererCtrl->GetSelIndex();

	// Go through the resolutions searching for a match
	unsigned int i;
	for (i=0; i < m_rendererArray[nCurrentRenderer].m_resolutionArray.GetSize(); i++)
	{
		MenuDisplayResolution searchRes=m_rendererArray[nCurrentRenderer].m_resolutionArray[i];

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
		MenuDisplayResolution searchRes=m_rendererArray[nCurrentRenderer].m_resolutionArray[i];

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

// Saves the global detail level
void CMenuDisplay::SaveDetailSetting()
{
	char szConsole[256];
	switch (m_nDetailLevel)
	{
	case 0:
		{
			sprintf(szConsole, "+GlobalDetail %d", DETAIL_LOW);
			break;
		}
	case 1:
		{
			sprintf(szConsole, "+GlobalDetail %d", DETAIL_MEDIUM);
			break;
		}
	case 2:
		{
			sprintf(szConsole, "+GlobalDetail %d", DETAIL_HIGH);
			break;
		}
	}

	m_pClientDE->RunConsoleString(szConsole);
	g_pBloodClientShell->MenuSetDetail();
}

// Override the left control
void CMenuDisplay::OnLeft()
{
	// Get the current resolution
	MenuDisplayResolution currentResolution=GetCurrentSelectedResolution();

	CMenuBase::OnLeft();

	// If the renderer control is selected, rebuild the resolutions control
	if (GetCurrentItem() == m_pRendererCtrl)
	{
		SetupResolutionCtrl();

		// Set the resolution for the control
		SetCurrentCtrlResolution(currentResolution);
	}
}

// Override the right control
void CMenuDisplay::OnRight()
{
	// Get the current resolution
	MenuDisplayResolution currentResolution=GetCurrentSelectedResolution();

	CMenuBase::OnRight();

	// If the renderer control is selected, rebuild the resolutions control
	if (GetCurrentItem() == m_pRendererCtrl)
	{
		SetupResolutionCtrl();

		// Set the resolution for the control
		SetCurrentCtrlResolution(currentResolution);
	}
}

DDWORD CMenuDisplay::OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
{
	return 0;
}
