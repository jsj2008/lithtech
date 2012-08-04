// MenuBase.cpp: implementation of the CMenuBase class.
//
//////////////////////////////////////////////////////////////////////

#include "VKDefs.h"
#include "LTGUIMgr.h"
#include "MenuBase.h"
#include "MainMenus.h"
#include <mbstring.h>

// #define MAX_FILE_LIST			512		// Max number of files in a file list

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuBase::CMenuBase()
{
	m_bInit=DFALSE;
	m_bBuilt=DFALSE;

	m_pClientDE=DNULL;
	m_hTitleSurf=DNULL;
	m_titlePos.x=0;
	m_titlePos.y=0;

	m_hTitleString=DNULL;

	m_bShowArrows=DFALSE;
	m_nArrowCenter=320;

#ifdef _ADDON
	m_szBackground = "interface_ao\\background.pcx";
#else
	m_szBackground = "interface\\mainmenus\\background.pcx";
#endif

	m_bBoxFormat = DTRUE;
}

CMenuBase::~CMenuBase()
{
	if ( m_bInit )
	{
		Term();
	}
}

/******************************************************************/

// Initialization
DBOOL CMenuBase::Init(CClientDE *pClientDE, CMainMenus *pMainMenus, CMenuBase *pParentMenu, DWORD dwMenuID, int nMenuHeight)
{
	m_pClientDE=pClientDE;	
	m_pMainMenus=pMainMenus;
	m_pParentMenu=pParentMenu;
	m_dwMenuID=dwMenuID;

	if ( !m_listOption.Create(nMenuHeight) )
	{
		return DFALSE;
	}

	m_bInit=TRUE;
	return DTRUE;
}

/******************************************************************/

// Termination
void CMenuBase::Term()
{
	// Remove all of the options
	RemoveAllOptions();

	// Note that the title surface is freed in CSharedResourceMgr

	// Free the title string
	if (m_hTitleString)
	{
		m_pClientDE->FreeString(m_hTitleString);
		m_hTitleString=DNULL;
	}

	m_bInit=FALSE;
}

/******************************************************************/

// Remove all of the options
void CMenuBase::RemoveAllOptions()
{
	// Terminate the options
	unsigned int i;
	for (i=0; i < m_controlArray.GetSize(); i++)
	{
		m_controlArray[i]->Destroy();
		delete m_controlArray[i];		
	}	
	m_controlArray.SetSize(0);
	m_largeFontItemArray.SetSize(0);
	m_listOption.RemoveAllControls();
}

/******************************************************************/

// Renders the menu to a surface
void CMenuBase::Render(HSURFACE hDestSurf)
{
	if (!hDestSurf)
	{
		return;
	}

	DDWORD dwDestinationWidth=0;
	DDWORD dwDestinationHeight=0;
			
	// Get the dims of the destination surface
	m_pClientDE->GetSurfaceDims (hDestSurf, &dwDestinationWidth, &dwDestinationHeight);	

	// Render the title
	if (m_pMainMenus->IsEnglish())
	{
		if ( m_hTitleSurf != DNULL )
		{
			m_pClientDE->DrawSurfaceToSurface(hDestSurf, m_hTitleSurf, DNULL, m_titlePos.x, m_titlePos.y);
		}
	}
	else
	{
		// The non-english version renders a text string instead of a title surface
		CLTGUIFont *pTitleFont=m_pMainMenus->GetTitleFont();

		if (pTitleFont && m_hTitleString)
		{
			pTitleFont->DrawSolid(m_hTitleString, hDestSurf, m_titlePos.x, m_titlePos.y, CF_JUSTIFY_LEFT, SETRGB(100,75,50));
		}
	}

	// Render the list of options
	m_listOption.EnableBoxFormat(m_bBoxFormat);
	m_listOption.Render(hDestSurf);
	
	// Render the arrows
	if (m_bShowArrows)
	{
		RenderArrows(hDestSurf);
	}		
}

/******************************************************************/

// Renders the up/down arrows
void CMenuBase::RenderArrows(HSURFACE hDestSurf)
{
	// Render the up arrow	
	if (m_listOption.GetStartIndex() != 0)
	{
		HSURFACE hUpSurf=m_pMainMenus->GetSurfaceUpArrow();

		if (hUpSurf)
		{
			DDWORD dwWidth=0;
			DDWORD dwHeight=0;
			m_pClientDE->GetSurfaceDims (hUpSurf, &dwWidth, &dwHeight);

			// Center the arrow on the screen and place it just above the list control
			int xPos=m_nArrowCenter-(dwWidth/2);
			int yPos=m_listOption.GetPos().y-dwHeight-8;

			HDECOLOR hTrans = m_pClientDE->SetupColor1 ( 1.0f, 0, 1.0f, DFALSE);			
			m_pClientDE->DrawSurfaceToSurfaceTransparent(hDestSurf, hUpSurf, DNULL, xPos, yPos, hTrans);	
		}
	}

	// Render the down arrow
	if (m_listOption.GetLastDisplayedIndex() != m_listOption.GetNum())
	{
		HSURFACE hDownSurf=m_pMainMenus->GetSurfaceDownArrow();

		if (hDownSurf)
		{
			DDWORD dwWidth=0;
			DDWORD dwHeight=0;
			m_pClientDE->GetSurfaceDims (hDownSurf, &dwWidth, &dwHeight);

			// Center the arrow on the screen and place it just above the list control
			int xPos=m_nArrowCenter-(dwWidth/2);
			int yPos=m_listOption.GetPos().y+m_listOption.GetHeight()-5;

			HDECOLOR hTrans = m_pClientDE->SetupColor1 ( 1.0f, 0, 1.0f, DFALSE);			
			m_pClientDE->DrawSurfaceToSurfaceTransparent(hDestSurf, hDownSurf, DNULL, xPos, yPos, hTrans);	
		}
	}
}

/******************************************************************/

// Creates the title for the menu from the path to the image.  The non-english version
// just sets up the string to draw to the screen.
DBOOL CMenuBase::CreateTitle(char *lpszTitleSurf, int nStringID, DIntPt titlePos)
{
	if (m_pMainMenus->IsEnglish())
	{
		// Remove the surface if one exists
		if ( m_hTitleSurf != DNULL )
		{
			m_pMainMenus->FreeSharedSurface(m_hTitleSurf);		
			m_hTitleSurf=DNULL;
		}
		
		m_hTitleSurf=m_pMainMenus->GetSharedSurface(lpszTitleSurf);
		if ( m_hTitleSurf == DNULL )
		{
			return DFALSE;
		}
	}
	else
	{
		m_hTitleString=m_pClientDE->FormatString(nStringID);
	}

	SetTitlePos(titlePos);
	return DTRUE;
}

/******************************************************************/

// Adds a menu option
CLTGUIFadeItemCtrl *CMenuBase::AddFadeItemOption(char *lpszOptionSurfPrefix, int nSurfaces, HSTRING hOptionText, DWORD dwCommandID, int xPos, int yPos)
{	
	char szTempString[256];

	assert(nSurfaces > 0);

	HSURFACE *pSurfArray=new HSURFACE[nSurfaces];

	// Load the option surfaces
	int i;
	for ( i=0; i < nSurfaces; i++ )
	{		
		// Add the extra zero if we are under 10 (index < 9)
		if ( i+1 < 10 )
		{
			sprintf(szTempString, "%s0%d.pcx", lpszOptionSurfPrefix, i+1);
		}
		else
		{
			sprintf(szTempString, "%s%d.pcx", lpszOptionSurfPrefix, i+1);
		}

		pSurfArray[i]=m_pClientDE->CreateSurfaceFromBitmap(szTempString);
	}

	// Load the disabled surface
	sprintf(szTempString, "%sdis.pcx", lpszOptionSurfPrefix);
	HSURFACE hDisabledSurf=m_pClientDE->CreateSurfaceFromBitmap(szTempString);

	// Create the new menu option
	CLTGUIFadeItemCtrl *pOption=new CLTGUIFadeItemCtrl;
	if ( !pOption->Create(m_pClientDE, dwCommandID, pSurfArray, nSurfaces, hDisabledSurf, TRUE, this) )
	{
		delete []pSurfArray;
		delete pOption;

		return DNULL;
	}

	// Set the position if it is specified
	if (xPos != -1 && yPos != -1)
	{
		pOption->SetPos(xPos, yPos);
	}

	// Add the option to the list
	m_listOption.AddControl(pOption);

	// Add the option to the list of controls to remove
	m_controlArray.Add(pOption);

	delete []pSurfArray;

	return pOption;
}

/******************************************************************/

// Add a text item option
CLTGUITextItemCtrl *CMenuBase::AddTextItemOption(HSTRING hText, DWORD dwCommandID, CLTGUIFont *pFontArray, int nNumFonts, DBOOL bDrawSolid, int *pnValue)
{	
	// Create the new menu option
	CLTGUITextItemCtrl *pOption=new CLTGUITextItemCtrl;
	if ( !pOption->Create(m_pClientDE, dwCommandID, hText, pFontArray, nNumFonts, bDrawSolid, this, pnValue) )
	{		
		delete pOption;

		return DNULL;
	}

	// Set the color
	pOption->SetColor(SETRGB(220,190,170), SETRGB(125,30,0));

	// Add the option to the list
	m_listOption.AddControl(pOption);

	// Add the option to the list of controls to remove
	m_controlArray.Add(pOption);	

	return pOption;
}

/******************************************************************/

// Just a wrapper for adding fading text options with large fonts
CLTGUITextItemCtrl *CMenuBase::AddLargeTextItemOption(HSTRING hText, DWORD dwCommandID)
{
	CLTGUITextItemCtrl *pCtrl=DNULL;
	if (m_pMainMenus->IsLowResolution())
	{
		pCtrl=AddTextItemOption(hText, dwCommandID, m_pMainMenus->GetSmallFont());
	}
	else
	{
		// If we are using engine fonts (instead of bitmap fonts) then use
		// the large font instead of the font array.		
		if (!m_pMainMenus->IsEnglish())
		{
			pCtrl=AddTextItemOption(hText, dwCommandID, m_pMainMenus->GetLargeFont());
		}
		else
		{
			pCtrl=AddTextItemOption(hText, dwCommandID, m_pMainMenus->GetLargeFadeFonts(), m_pMainMenus->GetNumLargeFadeFonts(), DFALSE);
		}
	}
	m_largeFontItemArray.Add(pCtrl);

	return pCtrl;
}

/******************************************************************/

// Add a column text option
CLTGUIColumnTextCtrl *CMenuBase::AddColumnTextOption(DWORD dwCommandID, CLTGUIFont *pFont)
{
	// Create the new menu option
	CLTGUIColumnTextCtrl *pOption=new CLTGUIColumnTextCtrl;
	if ( !pOption->Create(m_pClientDE, dwCommandID, pFont, this) )
	{		
		delete pOption;

		return DNULL;
	}

	// Set the color
	pOption->SetColor(SETRGB(220,190,170), SETRGB(125,30,0), SETRGB(96, 96, 96));

	// Add the option to the list
	m_listOption.AddControl(pOption);

	// Add the option to the list of controls to remove
	m_controlArray.Add(pOption);	

	return pOption;
}

/******************************************************************/

// Add a slider control
CLTGUISliderCtrl *CMenuBase::AddSliderOption(HSTRING hText, CLTGUIFont *pFont, int nSliderOffset, HSURFACE hBarSurf, HSURFACE hTabSurf, int *pnValue)
{
	// Create the new menu option
	CLTGUISliderCtrl *pOption=new CLTGUISliderCtrl;
	if ( !pOption->Create(m_pClientDE, hText, pFont, nSliderOffset, hBarSurf, hTabSurf, pnValue) )
	{		
		delete pOption;

		return DNULL;
	}

	// Set the color
	pOption->SetColor(SETRGB(220,190,170), SETRGB(125,30,0), SETRGB(96, 96, 96));

	// Set the text alignment
	pOption->SetTextAlignment(CF_JUSTIFY_RIGHT);

	// Add the option to the list
	m_listOption.AddControl(pOption);

	// Add the option to the list of controls to remove
	m_controlArray.Add(pOption);	

	return pOption;
}

/******************************************************************/

// Adds an on/off control
CLTGUIOnOffCtrl	*CMenuBase::AddOnOffOption(HSTRING hText, CLTGUIFont *pFont, int nRightColumnOffset, DBOOL *pnValue)
{
	// Create the new menu option
	CLTGUIOnOffCtrl *pOption=new CLTGUIOnOffCtrl;
	if ( !pOption->Create(m_pClientDE, hText, pFont, nRightColumnOffset, pnValue) )
	{		
		delete pOption;

		return DNULL;
	}

	// Set the color
	pOption->SetColor(SETRGB(220,190,170), SETRGB(125,30,0), SETRGB(96, 96, 96));

	// Add the option to the list
	m_listOption.AddControl(pOption);

	// Add the option to the list of controls to remove
	m_controlArray.Add(pOption);	

	return pOption;
}

/******************************************************************/

// Adds an edit control
CLTGUIEditCtrl *CMenuBase::AddEditOption(HSTRING hDescription, DWORD dwCommandID, CLTGUIFont *pFont, int nEditStringOffset, int nBufferSize, char *lpszValue)
{
	// Create the new menu option
	CLTGUIEditCtrl *pOption=new CLTGUIEditCtrl;

	if ( !pOption->Create(m_pClientDE, dwCommandID, hDescription, pFont, nEditStringOffset, nBufferSize, this, lpszValue) )
	{		
		delete pOption;

		return DNULL;
	}

	// Set the color
	pOption->SetColor(SETRGB(220,190,170), SETRGB(125,30,0), SETRGB(96, 96, 96));

	// Add the option to the list
	m_listOption.AddControl(pOption);

	// Add the option to the list of controls to remove
	m_controlArray.Add(pOption);	

	return pOption;
}

/******************************************************************/

// Wrapper for adding a control with a string ID instead of an HSTRING
CLTGUIFadeItemCtrl *CMenuBase::AddFadeItemOption(char *lpszOptionSurfPrefix, int nSurfaces, int messageCode, DWORD dwCommandID, int xPos, int yPos)
{
	// Load the string
	HSTRING hString=DNULL;
	if (messageCode)
	{
		hString=m_pClientDE->FormatString(messageCode);
	}

	// Create the control
	CLTGUIFadeItemCtrl *pCtrl=AddFadeItemOption(lpszOptionSurfPrefix, nSurfaces, hString, dwCommandID, xPos, yPos);
	
	// Free the string
	if (hString)
	{
		m_pClientDE->FreeString(hString);
	}

	return pCtrl;
}

/******************************************************************/

// Wrapper for adding a control with a string ID instead of an HSTRING
CLTGUITextItemCtrl *CMenuBase::AddTextItemOption(int messageCode, DWORD dwCommandID, CLTGUIFont *pFontArray, int nNumFonts, DBOOL bDrawSolid, int *pnValue)
{
	// Load the string
	HSTRING hString=DNULL;
	if (messageCode)
	{
		hString=m_pClientDE->FormatString(messageCode);
	}

	// Create the control
	CLTGUITextItemCtrl *pCtrl=AddTextItemOption(hString, dwCommandID, pFontArray, nNumFonts, bDrawSolid, pnValue);
	
	// Free the string
	if (hString)
	{
		m_pClientDE->FreeString(hString);
	}

	return pCtrl;
}

/******************************************************************/

// Wrapper for adding a control with a string ID instead of an HSTRING
CLTGUISliderCtrl *CMenuBase::AddSliderOption(int messageCode, CLTGUIFont *pFont, int nSliderOffset, HSURFACE hBarSurf, HSURFACE hTabSurf, int *pnValue)
{
	// Load the string
	HSTRING hString=DNULL;
	if (messageCode)
	{
		hString=m_pClientDE->FormatString(messageCode);
	}

	// Create the control
	CLTGUISliderCtrl *pCtrl=AddSliderOption(hString, pFont, nSliderOffset, hBarSurf, hTabSurf, pnValue);
	
	// Free the string
	if (hString)
	{
		m_pClientDE->FreeString(hString);
	}

	return pCtrl;
}

/******************************************************************/

// Wrapper for adding a control with a string ID instead of an HSTRING
CLTGUIOnOffCtrl *CMenuBase::AddOnOffOption(int messageCode, CLTGUIFont *pFont, int nRightColumnOffset, DBOOL *pbValue)
{
	// Load the string
	HSTRING hString=DNULL;
	if (messageCode)
	{
		hString=m_pClientDE->FormatString(messageCode);
	}

	// Create the control
	CLTGUIOnOffCtrl *pCtrl=AddOnOffOption(hString, pFont, nRightColumnOffset, pbValue);
	
	// Free the string
	if (hString)
	{
		m_pClientDE->FreeString(hString);
	}

	return pCtrl;
}

/******************************************************************/

// Wrapper for adding a control with a string ID instead of an HSTRING
CLTGUIEditCtrl *CMenuBase::AddEditOption(int messageCode, DWORD dwCommandID, CLTGUIFont *pFont, int nEditStringOffset, int nBufferSize, char *lpszValue)
{
	// Load the string
	HSTRING hString=DNULL;
	if (messageCode)
	{
		hString=m_pClientDE->FormatString(messageCode);
	}

	// Create the control
	CLTGUIEditCtrl *pCtrl=AddEditOption(hString, dwCommandID, pFont, nEditStringOffset, nBufferSize, lpszValue);
	
	// Free the string
	if (hString)
	{
		m_pClientDE->FreeString(hString);
	}

	return pCtrl;	
}

/******************************************************************/

// Wrapper for adding a control with a string ID instead of an HSTRING
CLTGUITextItemCtrl *CMenuBase::AddLargeTextItemOption(int messageCode, DWORD dwCommandID)
{
	// Load the string
	HSTRING hString=DNULL;
	if (messageCode)
	{
		hString=m_pClientDE->FormatString(messageCode);
	}

	// Create the control
	CLTGUITextItemCtrl *pCtrl=AddLargeTextItemOption(hString, dwCommandID);
	
	// Free the string
	if (hString)
	{
		m_pClientDE->FreeString(hString);
	}

	return pCtrl;	
}

/******************************************************************/

// Turns on and off low resolution fonts for the menus
void CMenuBase::SetLowResolutionFonts(DBOOL bLowRes)
{
	// Go through each control added by AddLargeTextItemOption and sets its font accordingly.
	unsigned int i;
	for (i=0; i < m_largeFontItemArray.GetSize(); i++)
	{
		if (m_largeFontItemArray[i])
		{
			if (bLowRes)
			{
				m_largeFontItemArray[i]->SetFont(m_pMainMenus->GetSmallFont());				
			}
			else
			{
				// If we are using engine fonts (instead of bitmap fonts) then use
				// the large font instead of the font array.
				if (!m_pMainMenus->IsEnglish())
				{
					m_largeFontItemArray[i]->SetFont(m_pMainMenus->GetLargeFont());
				}
				else
				{
					m_largeFontItemArray[i]->SetFont(m_pMainMenus->GetLargeFadeFonts(), m_pMainMenus->GetNumLargeFadeFonts(), DFALSE);
				}
			}
		}
	}
}

/******************************************************************/

// Calls UpdateData on each control in the menu
void CMenuBase::UpdateData(DBOOL bSaveAndValidate)
{
	unsigned int i;
	for (i=0; i < m_controlArray.GetSize(); i++)
	{
		m_controlArray[i]->UpdateData(bSaveAndValidate);
	}
}

/******************************************************************/

// Turns the up/down arrows on/off and sets the X position that they should be pointing
void CMenuBase::UseArrows(DBOOL bUse, int xCenter)
{
	m_bShowArrows=bUse;
	m_nArrowCenter=xCenter;
}

/******************************************************************/

// Set the current menu item selection.  Note that this will reset any animations.
void CMenuBase::SetCurrentItem(int nItemIndex)
{
	if (nItemIndex >= (int)m_controlArray.GetSize())
	{
		// Just set it to zero if we are out of bounds
		if (m_controlArray.GetSize() > 0)
		{
			m_listOption.SelectItem(0);
		}
	}	
	else
	{
		// Set the current item
		m_listOption.SelectItem(nItemIndex);
	}

	// Reset the animations
	int i;
	for (i=0; i < (int)m_controlArray.GetSize(); i++)
	{
		m_controlArray[i]->ResetAnimation();
	}
}

/******************************************************************/

// Returns the currently selected item
CLTGUICtrl *CMenuBase::GetCurrentItem()
{
	int nIndex=GetCurrentItemIndex();

	if (nIndex >= 0 && nIndex < m_listOption.GetNum())
	{
		return m_listOption.GetControl(nIndex);
	}
	else
	{
		return DNULL;
	}	
}

/******************************************************************/

// Sorts a file list.  This was taken from the old menu code.
FileEntry* CMenuBase::SortFileList(FileEntry *pfe)
{

	FileEntry	*pfindex;
	FileEntry	*pfList[MAX_FILE_LIST];
	int nCount=0;


	// Build an array of FileEntries.
	pfindex = pfe;
	while (pfindex && nCount < MAX_FILE_LIST)
	{
		pfList[nCount++] = pfindex;
		pfindex = pfindex->m_pNext;
	}
	if (pfindex) // Free any remaining items
	{
		m_pClientDE->FreeFileList(pfindex);
	}

	for (int i = nCount / 2; i > 0; i = (i == 2) ? 1 : (int) (i / 2.2))
	{
		for (int j = i; j < nCount; j++)
		{
			FileEntry *pfTemp = pfList[j];
			
			for (int k = j; k >= i && _mbsicmp((const unsigned char*)pfTemp->m_pBaseFilename, (const unsigned char*)pfList[k - i]->m_pBaseFilename) < 0; k -= i)
			{
				pfList[k] = pfList[k - i];
			}

			pfList[k] = pfTemp;
		}
	}

	pfindex = pfList[0];
	for (i=1; i < nCount-1; i++)
	{
		pfindex->m_pNext = pfList[i];
		pfindex = pfindex->m_pNext;
	}
	pfindex->m_pNext = DNULL;

	return pfList[0];
}

/******************************************************************/

// Handles a key press.  Returns FALSE if the key was not processed through this method.
// Left, Up, Down, Right, and Enter are automatically passed through OnUp(), OnDown(), etc.
DBOOL CMenuBase::HandleKeyDown(int key, int rep)
{
	switch (key)
	{
	case VK_LEFT:
		{
			OnLeft();
			break;
		}
	case VK_RIGHT:
		{
			OnRight();
			break;
		}
	case VK_UP:
		{
			OnUp();
			break;
		}
	case VK_DOWN:
		{
			OnDown();
			break;
		}
	case VK_RETURN:
		{
			OnEnter();
			break;
		}
	default:
		{
			return m_listOption.HandleKeyDown(key, rep);
			break;
		}
	}

	// Handled the key
	return DTRUE;
}

/******************************************************************/

void CMenuBase::OnUp()
{
	if ( m_pMainMenus )
	{
		m_pMainMenus->PlaySelectSound();
	}
	m_listOption.OnUp();
}

/******************************************************************/

void CMenuBase::OnDown()
{
	if ( m_pMainMenus )
	{
		m_pMainMenus->PlaySelectSound();
	}
	m_listOption.OnDown();
}

/******************************************************************/

void CMenuBase::OnLeft()
{
	if ( m_pMainMenus )
	{
		m_pMainMenus->PlaySelectSound();
	}
	m_listOption.OnLeft();
}

/******************************************************************/

void CMenuBase::OnRight()
{
	if ( m_pMainMenus )
	{
		m_pMainMenus->PlaySelectSound();
	}
	m_listOption.OnRight();
}

/******************************************************************/

void CMenuBase::OnEnter()
{
	if ( m_pMainMenus )
	{
		m_pMainMenus->PlayEnterSound();
	}
	m_listOption.OnEnter();
}


/******************************************************************/

// Device tracking
DBOOL CMenuBase::HandleDeviceTrack(DeviceInput *pInput, DDWORD dwNumDevices)
{
	return DFALSE;
}

/******************************************************************/

DDWORD CMenuBase::OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
{
	return 0;
}