// MessageBoxCtrl.cpp: implementation of the CMessageBoxCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MessageBoxCtrl.h"
#include "vkdefs.h"

#define MB_EDGESPACE	20	// The space between the side edges of the message box and the text
#define MB_NOSELECT	-1

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMessageBoxCtrl::CMessageBoxCtrl()
{
    m_pFont                     = LTNULL;
    m_hText                     = LTNULL;
    m_pClientDE                 = LTNULL;
    m_pCommandHandler           = LTNULL;
    m_hBackgroundSurf           = LTNULL;
	m_hTransColor				= kBlack;
	m_textColor					= kBlack;
	m_textButtonColor			= SETRGB(255,255,0);
	m_textButtonHighlightColor	= SETRGB(255,255,145);
	m_nTextButtonHorzSpace		= 5;
	m_nTextButtonVertSpace		= 5;
	m_nMouseDownItemSel			= MB_NOSELECT;
	m_screenPos.x				= 0;
	m_screenPos.y				= 0;
	m_nSelection				= MB_NOSELECT;
	m_fScale					= 1.0f;
}

CMessageBoxCtrl::~CMessageBoxCtrl()
{
	Destroy();
}

/*************************************************************************************/
// Create the message box
// pClientDE	   - Pointer to the client interface
// lpszBackground  - Pathname to the pcx file for the background
// pFont		   - Font which is used to display text in the message box.
// hText		   - Text to be displayed in the message box
// pCommandHandler - Command handler which recieves a message when a key (call AddKey first) is pressed
LTBOOL CMessageBoxCtrl::Create(ILTClient *pClientDE, char *lpszBackground, CLTGUIFont *pFont,
                               HSTRING hText, CLTGUICommandHandler *pCommandHandler)
{
	//Sanity check...
	if (!pClientDE || !lpszBackground || !pFont)
        return LTFALSE;

	// Load the background
	HSURFACE hSurf = pClientDE->CreateSurfaceFromBitmap(lpszBackground);
	if (!hSurf)
        return LTFALSE;

	return Create(pClientDE,hSurf,pFont,hText,pCommandHandler);
}

/*************************************************************************************/
// Create the message box
// pClientDE	   - Pointer to the client interface
// hBack		   - Surface to use for the background
// pFont		   - Font which is used to display text in the message box.
// hText		   - Text to be displayed in the message box
// pCommandHandler - Command handler which recieves a message when a key (call AddKey first) is pressed
LTBOOL CMessageBoxCtrl::Create(ILTClient *pClientDE, HSURFACE hBack, CLTGUIFont *pFont,
                               HSTRING hText, CLTGUICommandHandler *pCommandHandler)
{
	//Sanity check...
	if (!pClientDE || !hBack || !pFont)
        return LTFALSE;

	//Copy engine pointer
	m_pClientDE = pClientDE;

	// Load the background
	m_hBackgroundSurf = hBack;

	// Set the font
	m_pFont = pFont;

	// Set the text
	SetText(hText);

	// Set the command handler
	m_pCommandHandler = pCommandHandler;

	m_nSelection				= MB_NOSELECT;

    return LTTRUE;
}

/*************************************************************************************/
// Destroys the message box
void CMessageBoxCtrl::Destroy()
{
	if (m_pClientDE && m_hBackgroundSurf)
	{
		m_pClientDE->DeleteSurface(m_hBackgroundSurf);
        m_hBackgroundSurf = LTNULL;
	}

	if (m_hText && m_pClientDE)
	{
		m_pClientDE->FreeString(m_hText);
        m_hText=LTNULL;
	}

    m_pCommandHandler   = LTNULL;
    m_pFont             = LTNULL;

	RemoveKeys();
	RemoveMessageButtons();

    m_pClientDE = LTNULL;
}

/*************************************************************************************/
// Render the message box.  Default coordinates center the box along that axis
void CMessageBoxCtrl::Render(HSURFACE hDestSurf)
{
	// Make sure we have a client pointer and a background surface
	if (!m_pClientDE || !m_hBackgroundSurf)
		return;

	if (!m_pFont || !m_hText) return;

	int nMBWidth = GetWidth();
	int nMBHeight = GetHeight();

	if (m_fScale > 1.0f)
	{
		LTRect dstRect(m_screenPos.x, m_screenPos.y,m_screenPos.x+nMBWidth, m_screenPos.y+nMBHeight);
		m_pClientDE->ScaleSurfaceToSurface(hDestSurf, m_hBackgroundSurf,&dstRect,LTNULL);
	}
	else
	{
		m_pClientDE->DrawSurfaceToSurfaceTransparent(hDestSurf, m_hBackgroundSurf, LTNULL, m_screenPos.x, m_screenPos.y, m_hTransColor);
	}

	// Render the text string
	LITHFONTDRAWDATA lfDD;

	lfDD.dwFlags = LTF_DRAW_NORMAL;

	// Center the text in the box
	int nWidth=nMBWidth-(MB_EDGESPACE*2);
    LTIntPt size=m_pFont->GetTextExtentsFormat(m_hText, nWidth);

	int x,y;
	x=(m_screenPos.x+(nMBWidth/2))-(size.x/2);
	y=(m_screenPos.y+MB_EDGESPACE);

	m_pFont->DrawFormat(m_hText, hDestSurf, x, y, nWidth, m_textColor);


	// Check if we need to render any text buttons
	if (m_textButtonArray.GetSize() > 0)
	{
		// Render the text buttons
		unsigned int i;
		for (i=0; i < m_textButtonArray.GetSize(); i++)
		{
			// The bounding rectangle for the text relative to the left side of the message box
            LTRect rcText=GetTextButtonRect(i);
			int x = m_screenPos.x+rcText.left;
			int y = m_screenPos.y+rcText.top;
			// Render the text
            HLTCOLOR hColor = m_textButtonColor;;
			if ((int)i == m_nSelection)
				hColor = m_textButtonHighlightColor;
			m_pFont->Draw(m_textButtonArray[i].m_hString, hDestSurf, x, y, LTF_JUSTIFY_LEFT, hColor);
		}
	}
}

/*************************************************************************************/
// Sets the message box position
void CMessageBoxCtrl::SetPos(int xPos, int yPos, HSURFACE hDestSurf)
{
	// If there is a destination surface, then allow the message box to be centered.
	if (hDestSurf)
	{
		if (!m_pClientDE)
		{
            ASSERT(LTFALSE);
			return;
		}

		// Get the size of the destination surface
		uint32 dwScreenWidth=0;
		uint32 dwScreenHeight=0;
		m_pClientDE->GetSurfaceDims (hDestSurf, &dwScreenWidth, &dwScreenHeight);

		// Center the message box if specified
		if (xPos == LTGUI_MB_CENTER)
		{
			m_screenPos.x=(dwScreenWidth/2)-(GetWidth()/2);
		}
		else
		{
			m_screenPos.x=xPos;
		}

		if (yPos == LTGUI_MB_CENTER)
		{
			m_screenPos.y=(dwScreenHeight/2)-(GetHeight()/2);
		}
		else
		{
			m_screenPos.y=yPos;
		}
	}
	else
	{
		// Make sure that the didn't want to center the message box
		// since there isn't a destination surface.
		if (xPos == LTGUI_MB_CENTER || yPos == LTGUI_MB_CENTER)
		{
			ASSERT(FALSE);
			return;
		}

		m_screenPos.x=xPos;
		m_screenPos.y=yPos;
	}
}

/*************************************************************************************/
// Returns the bounding rectangle for a text button at a specific index relative to the
// message box.
LTRect CMessageBoxCtrl::GetTextButtonRect(int nIndex)
{
	ASSERT(nIndex < (int)m_textButtonArray.GetSize());

	// The top of the bounding box
	int nTop=GetHeight()-m_nTextButtonVertSpace-m_pFont->GetHeight();

	// Get the total width of all of the buttons to be used for centering
	int nTotalWidth=0;
	int i;
	for (i=0; i < (int)m_textButtonArray.GetSize(); i++)
	{
        LTIntPt size=m_pFont->GetTextExtents(m_textButtonArray[i].m_hString);
		nTotalWidth+=size.x;

		// If there are more buttons, add the spacing
		if (i < (int)m_textButtonArray.GetSize()-1)
		{
			nTotalWidth+=m_nTextButtonHorzSpace;
		}
	}

	// Set the starting position for the left edge of the bounding box
	int nLeft=(GetWidth()/2)-(nTotalWidth/2);

	// Add each button's width to the left edge until we get to the index that we are looking for
	for (i=0; i < nIndex; i++)
	{
		// Get the text dimentions for this index
        LTIntPt size=m_pFont->GetTextExtents(m_textButtonArray[i].m_hString);

		// Add the text width
		nLeft+=size.x;

		// Add the spacing
		nLeft+=m_nTextButtonHorzSpace;
	}

	// Get the text dimentions for the specified index
    LTIntPt size=m_pFont->GetTextExtents(m_textButtonArray[nIndex].m_hString);

	// Build the bounding box
    LTRect rcBounding;
	rcBounding.top=nTop;
	rcBounding.left=nLeft;
	rcBounding.right=rcBounding.left+size.x;
	rcBounding.bottom=rcBounding.top+size.y;

	return rcBounding;
}

/*************************************************************************************/
// Set the text for the message box
void CMessageBoxCtrl::SetText(HSTRING hText)
{
	if (!m_pClientDE)
	{
        assert(LTFALSE);
		return;
	}
	m_fScale = 1.0f;

	if (m_hText)
	{
		m_pClientDE->FreeString(m_hText);
        m_hText=LTNULL;

	}

	if (hText)
	{
		m_hText=m_pClientDE->CopyString(hText);

		int nWidth =  GetWidth()-(MB_EDGESPACE*2);
		int nHeight = GetHeight()-(MB_EDGESPACE*2); 
		LTIntPt size=m_pFont->GetTextExtentsFormat(m_hText, nWidth);
		if (size.y > nHeight)
		{
			m_fScale = (LTFLOAT)size.y / (LTFLOAT)nHeight;
		}

	}
}

/*************************************************************************************/
// Add a key and tie it to a message.  The message is sent when the key is passed into HandleKeyDown
void CMessageBoxCtrl::AddKey(int nVKeyCode, uint32 dwMessageID)
{
	LTGUIKeyMessage key;
	key.m_nVKeyCode=nVKeyCode;
	key.m_dwMessageID=dwMessageID;

	m_keyBindings.Add(key);
}

/*************************************************************************************/
// Adds a "text button" to the bottom row of the message box.  The message is sent when the user clicks
// on the button with the mouse.
void CMessageBoxCtrl::AddMessageButton(HSTRING hString, uint32 dwMessageID)
{
	ASSERT(m_pClientDE);

	LTGUITextButton button;
	button.m_hString=m_pClientDE->CopyString(hString);
	button.m_dwMessageID=dwMessageID;

	if (m_textButtonArray.Add(button) && m_nSelection == MB_NOSELECT)
	{
		m_nSelection = (int)m_textButtonArray.GetSize() -1;
	}



}

/*************************************************************************************/
// Removes all text buttons
void CMessageBoxCtrl::RemoveMessageButtons()
{
	// Free each string
	if (m_pClientDE)
	{
		unsigned int i;
		for (i=0; i < m_textButtonArray.GetSize(); i++)
		{
			m_pClientDE->FreeString(m_textButtonArray[i].m_hString);
		}
	}

	m_textButtonArray.SetSize(0);
}

/*************************************************************************************/
// Sets the "text buttons" color and highlight color
void CMessageBoxCtrl::SetTextButtonColor(HLTCOLOR color, HLTCOLOR highlightColor)
{
	m_textButtonColor=color;
	m_textButtonHighlightColor=highlightColor;
}

/*************************************************************************************/
// Returns the width of the message box
int	CMessageBoxCtrl::GetWidth()
{
	uint32 dwWidth=0;
	uint32 dwHeight=0;

	if (m_hBackgroundSurf)
	{
		m_pClientDE->GetSurfaceDims (m_hBackgroundSurf, &dwWidth, &dwHeight);

		if (m_fScale > 1.0f)
		{
			dwWidth = (uint32)(m_fScale * (LTFLOAT)dwWidth);
		}
		return dwWidth;
	}
	else
	{
		return 0;
	}
}

/*************************************************************************************/
// Returns the height of the message box
int CMessageBoxCtrl::GetHeight()
{
	uint32 dwWidth=0;
	uint32 dwHeight=0;

	if (m_hBackgroundSurf)
	{
		m_pClientDE->GetSurfaceDims (m_hBackgroundSurf, &dwWidth, &dwHeight);
		if (m_fScale > 1.0f)
		{
			dwHeight = (uint32)(m_fScale * (LTFLOAT)dwHeight);
		}
		return dwHeight;
	}
	else
	{
		return 0;
	}
}

/*************************************************************************************/
// Process a key message
LTBOOL CMessageBoxCtrl::HandleKeyDown(int key, int rep)
{
	if (!m_pCommandHandler) return LTFALSE;

	switch (key)
	{
	case VK_RETURN:
	{
		if (m_nSelection != MB_NOSELECT)
		{
			g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
			return (m_pCommandHandler->SendCommand(m_textButtonArray[m_nSelection].m_dwMessageID, 0, 0));

		}
	}	break;
	case VK_LEFT:
	{
		if (m_nSelection != MB_NOSELECT)
		{
			m_nSelection--;
			if (m_nSelection < 0)
				m_nSelection = m_textButtonArray.GetSize() - 1;
			g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
			return LTTRUE;

		}
	}	break;
	case VK_RIGHT:
	{
		if (m_nSelection != MB_NOSELECT)
		{
			m_nSelection++;
			if (m_nSelection >= (int)m_textButtonArray.GetSize())
				m_nSelection = 0;
			g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
			return LTTRUE;

		}
	}	break;
	default:
	{
		// Search the array of keys for a key binding matching this key
		unsigned int i;
		for (i=0; i < m_keyBindings.GetSize(); i++)
		{
			if (m_keyBindings[i].m_nVKeyCode == key)
			{
				if (m_pCommandHandler->SendCommand(m_keyBindings[i].m_dwMessageID, 0, 0))
				{
					g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
					return LTTRUE;
				}
			}
		}
	} break;
	}
    return LTFALSE;
}

/*************************************************************************************/
// Mouse down message
LTBOOL CMessageBoxCtrl::OnLButtonDown(int x, int y)
{
	// Check to see if we have text buttons
	if (m_textButtonArray.GetSize() <= 0)
	{
        return LTFALSE;
	}


	m_nMouseDownItemSel = GetButtonFromPoint(x, y);


	return (m_nMouseDownItemSel != MB_NOSELECT);

}


/*************************************************************************************/
// Mouse up message
LTBOOL CMessageBoxCtrl::OnLButtonUp(int x, int y)
{
	// Check to see if we have text buttons
	if (m_textButtonArray.GetSize() <= 0)
	{
        return LTFALSE;
	}

	// Check to see if the mouse is over the same button as it was clicked on
	int i = GetButtonFromPoint(x, y);
	if (i != MB_NOSELECT && m_nMouseDownItemSel == i)
	{
		// Send the message
		if (m_pCommandHandler->SendCommand(m_textButtonArray[i].m_dwMessageID, 0, 0))
		{
			// Reset the selected item
			m_nMouseDownItemSel=MB_NOSELECT;
			g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);

            return LTTRUE;
		}
	}

	// Reset the selected item
	m_nMouseDownItemSel=MB_NOSELECT;

    return LTFALSE;
}

/*************************************************************************************/
// Mouse move message
LTBOOL CMessageBoxCtrl::OnMouseMove(int x, int y)
{
	int ndx = GetButtonFromPoint(x,y);
	if (ndx != MB_NOSELECT && ndx != m_nSelection)
	{
		g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
		m_nSelection = ndx;
	}
    return LTFALSE;
}

int	CMessageBoxCtrl::GetButtonFromPoint(int x, int y)
{
	// Check each button to see if the point is on any buttons
	int i;
	for (i=0; i < (int)m_textButtonArray.GetSize(); i++)
	{
		// The bounding rectangle for the text relative to the left side of the message box
        LTRect rcText=GetTextButtonRect(i);

		// Check to see if the mouse click is in the button
		if (x >= m_screenPos.x+rcText.left && y >= m_screenPos.y+rcText.top &&
			x <= m_screenPos.x+rcText.right && y <= m_screenPos.y+rcText.bottom)
		{
			return i;
		}
	}
	return MB_NOSELECT;
}

void CMessageBoxCtrl::SetSelection(int nSelect)
{
	if (nSelect == MB_NOSELECT || (nSelect >= 0 && nSelect < (int)m_textButtonArray.GetSize()))
		m_nSelection = nSelect;
}