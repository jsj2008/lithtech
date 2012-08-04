// ListCtrl.cpp: implementation of the CListCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ListCtrl.h"
#include "InterfaceMgr.h"

namespace
{
	const int kUpSel = -1;
	const int kDownSel = -2;
}

const int CListCtrl::kNoSelection = 99999;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CListCtrl::CListCtrl()
{
	m_nCurrentIndex				= kNoSelection;
	m_nFirstDisplayedItem		= 0;
	m_nLastDisplayedItem		= -1;
	m_nHeight					= 100;
	m_nItemSpacing				= 10;
	m_nMouseDownItemSel			= kNoSelection;
    m_bEnableMouseMoveSelect    = LTFALSE;
    m_bEnableMouseClickSelect   = LTFALSE;
	m_nAlign					= LTF_JUSTIFY_LEFT;
    m_bArrows                   = LTFALSE;
	m_nArrowOffset				= 0;
	m_fScrollFirstDelay			= 1.0f;
	m_fScrollDelay				= 0.5f;
	m_fNextScrollTime			= -1.0f;
	m_pUp						= LTNULL;
	m_pDown						= LTNULL;

}

CListCtrl::~CListCtrl()
{
	Destroy();
}

// Creation
LTBOOL CListCtrl::Create ( int nHeight, LTBOOL bUseArrows, int nArrowOffset)
{
	m_nHeight=nHeight;
    m_bCreated=LTTRUE;

	if (bUseArrows)
	{
        m_bArrows       = LTTRUE;
		m_nArrowOffset	= nArrowOffset;
		//Add Server Arrows
		m_pUp = debug_new(CBitmapCtrl);
        if (!m_pUp->Create(g_pLTClient,"interface\\ListArrowUp.pcx","interface\\ListArrowUpH.pcx","interface\\ListArrowUpD.pcx"))
		{
			debug_delete(m_pUp);
            m_pUp = LTNULL;
		}

		m_pDown = debug_new(CBitmapCtrl);
        if (!m_pDown->Create(g_pLTClient,"interface\\ListArrowDn.pcx","interface\\ListArrowDnH.pcx","interface\\ListArrowDnD.pcx"))
		{
			debug_delete(m_pDown);
            m_pDown = LTNULL;
		}

	}

    return LTTRUE;
}

// Destroy the control
void CListCtrl::Destroy ( )
{
	m_controlArray.RemoveAll();

	if (m_pUp)
	{
		debug_delete(m_pUp);
        m_pUp = LTNULL;
	}
	if (m_pDown)
	{
		debug_delete(m_pDown);
        m_pDown = LTNULL;
	}
    m_bCreated=LTFALSE;
}

// Render the control
void CListCtrl::Render ( HSURFACE hDestSurf )
{
	if ((m_pUp && m_pUp->IsSelected()) || (m_pDown && m_pDown->IsSelected()))
	{
        LTIntPt cpos = g_pInterfaceMgr->GetCursorPos();
		cpos.x += g_pInterfaceResMgr->GetXOffset();
		cpos.y += g_pInterfaceResMgr->GetYOffset();
		int nLeft=m_pos.x;
		int nTop=m_pos.y;
		int nRight=nLeft+GetWidth();
		int nBottom=nTop+GetHeight();
		if ( cpos.x < nLeft || cpos.x > nRight || cpos.y < nTop || cpos.y > nBottom)
		{
            if (m_pUp && m_pUp->IsSelected()) 
				m_pUp->Select(LTFALSE);
            if (m_pDown && m_pDown->IsSelected()) 
				m_pDown->Select(LTFALSE);
		}

	}

    if (m_fNextScrollTime > 0.0f && m_fNextScrollTime < g_pLTClient->GetTime())
	{
		m_fNextScrollTime = -1.0f;
		if (m_nMouseDownItemSel == kUpSel && CanScrollUp())
		{
			ScrollUp();
			if (CanScrollUp() && IsKeyDown(VK_LBUTTON))
                m_fNextScrollTime = g_pLTClient->GetTime()+m_fScrollDelay;
		}
		if (m_nMouseDownItemSel == kDownSel && CanScrollDown())
		{
			ScrollDown();
			if (CanScrollDown() && IsKeyDown(VK_LBUTTON))
                m_fNextScrollTime = g_pLTClient->GetTime()+m_fScrollDelay;
		}
	}

	// Render the items unless they are off the control
    LTIntPt size;
	int y=m_pos.y;

	unsigned int i;
	for ( i = m_nFirstDisplayedItem; i < m_controlArray.GetSize(); i++ )
	{
		size.x = m_controlArray[i]->GetWidth();
		size.y = m_controlArray[i]->GetHeight();

		if ( y+size.y < m_pos.y+m_nHeight )
		{
			// The x position
			int x=m_pos.x;

			// Center the control if needed
			switch (m_nAlign)
			{
			case LTF_JUSTIFY_CENTER:
				x-=(size.x)/2;
				break;
			case LTF_JUSTIFY_RIGHT:
				x-=size.x;
				break;
			}

			x += m_controlArray[i]->GetBoxDrawAdjustX();

			// Set the position for the control
			m_controlArray[i]->SetPos(x, y);
			m_controlArray[i]->Render ( hDestSurf );

			y+=size.y+m_nItemSpacing;
			m_nLastDisplayedItem = i;
		}
		else
		{
			break;
		}
	}

	if (m_bArrows)
	{
		if (m_pUp) m_pUp->SetPos(m_pos.x+m_nArrowOffset,m_pos.y);
		if (m_pDown) m_pDown->SetPos(m_pos.x+m_nArrowOffset,(m_pos.y+m_nHeight)-m_pDown->GetHeight());
		if (CanScrollUp())
			m_pUp->Render(hDestSurf);
		if (CanScrollDown())
			m_pDown->Render(hDestSurf);
	}
}

// Returns the width
int CListCtrl::GetWidth ( )
{
	int x = 0;
	int y = 0;

    LTIntPt size;

	if (m_bArrows)
	{
		x = m_nArrowOffset;
		if (m_pUp)
			x += m_pUp->GetWidth();
	}
	unsigned int i;
	for ( i = 0; i < m_controlArray.GetSize(); i++ )
	{
		size.x = m_controlArray[i]->GetWidth();
		size.y = m_controlArray[i]->GetHeight();

		if ( y+size.y < m_nHeight )
		{
			if ( size.x > x )
			{
				x=size.x;
			}
			y+=size.y+m_nItemSpacing;
		}
		else
		{
			return x;
		}
	}
	return x;
}

// Returns the height
int	CListCtrl::GetHeight ( )
{
	return m_nHeight;
}

// Call this if you wish to enable highlighting the item that is under the mouse
// cursor and changing selection when the mouse is moved.
void CListCtrl::EnableMouseMoveSelect(LTBOOL bEnable)
{
	m_bEnableMouseMoveSelect = bEnable;
}
void CListCtrl::EnableMouseClickSelect(LTBOOL bEnable)
{
	m_bEnableMouseClickSelect = bEnable;
}

// Handle a keypress
LTBOOL CListCtrl::HandleKeyDown(int key, int rep)
{
	if (CLTGUICtrl::HandleKeyDown(key, rep))
	{
        return LTTRUE;
	}
	else
	{
		if ( m_nCurrentIndex < (int)m_controlArray.GetSize() )
		{
			return m_controlArray[m_nCurrentIndex]->HandleKeyDown(key, rep);
		}
	}

    return LTFALSE;
}

LTBOOL CListCtrl::OnUp ( )
{
	unsigned int nIndex=m_nCurrentIndex;

	if (nIndex == kNoSelection)
        return LTFALSE;

	// Find the next non-disabled index
	int i=m_nCurrentIndex-1;
	while ( i != m_nCurrentIndex )
	{
		if ( i < 0 )
		{
//            i = (int)m_controlArray-1;
			return LTFALSE;
		}

		if ( m_controlArray[i]->IsEnabled() )
		{
			nIndex=i;
			break;
		}
		else
		{
			i--;
		}
	}

	if ((int)nIndex != m_nCurrentIndex)
	{
		SelectItem ( nIndex );
        return LTTRUE;
	}
    return LTFALSE;
}

LTBOOL CListCtrl::OnDown ( )
{
	unsigned int nIndex=m_nCurrentIndex;

	if (nIndex == kNoSelection)
        return LTFALSE;

	// Find the next non-disabled index
	int i=m_nCurrentIndex+1;
	while ( i != m_nCurrentIndex )
	{
		if ( i >= (int)m_controlArray.GetSize() )
		{
//            i = 0;
			return LTFALSE;
		}

		if ( m_controlArray[i]->IsEnabled() )
		{
			nIndex=i;
			break;
		}
		else
		{
			i++;
		}
	}

	if ((int)nIndex != m_nCurrentIndex)
	{
		SelectItem ( nIndex );
        return LTTRUE;
	}
    return LTFALSE;
}

LTBOOL CListCtrl::ScrollUp ()
{
	if (!CanScrollUp())
        return LTFALSE;
	m_nFirstDisplayedItem--;
	m_nLastDisplayedItem = CalculateLastDisplayedIndex(m_nFirstDisplayedItem);
    return LTTRUE;
}

LTBOOL CListCtrl::ScrollDown ()
{
	if (!CanScrollDown())
        return LTFALSE;
	m_nFirstDisplayedItem++;
	m_nLastDisplayedItem = CalculateLastDisplayedIndex(m_nFirstDisplayedItem);
    return LTTRUE;
}

LTBOOL  CListCtrl::OnLeft ( )
{
	if ( m_nCurrentIndex < (int)m_controlArray.GetSize() )
	{
		return m_controlArray[m_nCurrentIndex]->OnLeft();
	}
    return LTFALSE;
}

LTBOOL  CListCtrl::OnRight ( )
{
	if ( m_nCurrentIndex < (int)m_controlArray.GetSize() )
	{
		return m_controlArray[m_nCurrentIndex]->OnRight();
	}
    return LTFALSE;
}

LTBOOL  CListCtrl::OnEnter ( )
{
	if ( m_nCurrentIndex < (int)m_controlArray.GetSize() )
	{
		return m_controlArray[m_nCurrentIndex]->OnEnter();
	}
    return LTFALSE;
}

// Handles the left button down message
LTBOOL CListCtrl::OnLButtonDown(int x, int y)
{
	x += g_pInterfaceResMgr->GetXOffset();
	y += g_pInterfaceResMgr->GetYOffset();

	// Get the control that the click was on
	int nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{
		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
            return LTFALSE;

		if (nControlIndex >= 0 && m_bEnableMouseClickSelect)
		{
			// Select the control
			SelectItem(nControlIndex);
		}

		if ((nControlIndex == kUpSel && CanScrollUp())|| (nControlIndex == kDownSel && CanScrollDown()))
            m_fNextScrollTime = g_pLTClient->GetTime()+m_fScrollFirstDelay;

		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.
		m_nMouseDownItemSel=nControlIndex;

        return LTTRUE;
	}
	else
	{
		// This clears the index for what item was selected from a mouse down message
		m_nMouseDownItemSel=kNoSelection;

        return LTFALSE;
	}
}

// Handles the left button up message
LTBOOL CListCtrl::OnLButtonUp(int x, int y)
{
	x += g_pInterfaceResMgr->GetXOffset();
	y += g_pInterfaceResMgr->GetYOffset();
	m_fNextScrollTime = -1.0f;
	// Get the control that the click was on
	int nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{
		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
            return LTFALSE;


		// If the mouse is over the same control now as it was when the down message was called
		// then send the "enter" message to the control.
		if (nControlIndex == m_nMouseDownItemSel)
		{
			if (nControlIndex == kUpSel)
			{
				return ScrollUp();
			}
			else if (nControlIndex == kDownSel)
			{
				return ScrollDown();
			}
			else
			{
				return m_controlArray[nControlIndex]->OnLButtonUp( x,  y);
			}
		}
	}
	else
	{
		m_nMouseDownItemSel=kNoSelection;
	}
    return LTFALSE;
}

// Handles the mouse move message
LTBOOL CListCtrl::OnMouseMove(int x, int y)
{
	x += g_pInterfaceResMgr->GetXOffset();
	y += g_pInterfaceResMgr->GetYOffset();
	int nControlUnderPoint=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlUnderPoint);
	if (m_pUp && CanScrollUp()) 
		m_pUp->Select(pCtrl == m_pUp);
	if (m_pDown && CanScrollDown()) 
		m_pDown->Select(pCtrl == m_pDown);

	// If m_bEnableMouseMoveSelect is TRUE then select the item that the mouse is over
	if (m_bEnableMouseMoveSelect)
	{
		if(pCtrl && nControlUnderPoint >= 0)
		{
			// Make sure we're enabled
			if(!pCtrl->IsEnabled())
                return LTFALSE;

			if (GetSelectedItem() != nControlUnderPoint)
			{
				SelectItem(nControlUnderPoint);

                return LTTRUE;
			}
		}
		else
		{
/*
			//if not on anything select nothing
			if ( m_nCurrentIndex < (int)m_controlArray.GetSize() )
			{
				m_controlArray[m_nCurrentIndex]->Select(FALSE);
			}
			m_nCurrentIndex = kNoSelection;
*/
		}
	}

    return LTFALSE;

}

// Add a control the the array of controls
int	CListCtrl::AddControl ( CLTGUICtrl *pControl )
{
	ASSERT(pControl);
	m_controlArray.Add(pControl);

	int nIndex = m_controlArray.GetSize()-1;
	if ( nIndex == m_nCurrentIndex )
	{
		m_controlArray[m_nCurrentIndex]->Select(TRUE);
	}
	if (m_nFirstDisplayedItem < 0)
		m_nFirstDisplayedItem = 0;
	m_nLastDisplayedItem = CalculateLastDisplayedIndex(m_nFirstDisplayedItem);
	return nIndex;
}

// Remove a control
void CListCtrl::RemoveControl ( CLTGUICtrl *pControl )
{
	unsigned int i;
	for ( i = 0; i < m_controlArray.GetSize(); i++ )
	{
		if ( m_controlArray[i] == pControl )
		{
			RemoveControl(i);
			return;
		}
	}
}

// Remove a control
void CListCtrl::RemoveControl ( int nIndex )
{
	if (nIndex >= GetNum()) return;
	debug_delete(m_controlArray[nIndex]);
	m_controlArray.Remove(nIndex);
	if (m_nFirstDisplayedItem >= GetNum())
		m_nFirstDisplayedItem = GetNum()-1;
	m_nLastDisplayedItem = CalculateLastDisplayedIndex(m_nFirstDisplayedItem);
	ClearSelection();
	if (nIndex < GetNum())
		SelectItem (nIndex);
}

// Removes all of the controls
void CListCtrl::RemoveAllControls ( )
{
	while ( m_controlArray.GetSize() > 0 )
	{
		debug_delete(m_controlArray[0]);
		m_controlArray.Remove(0);
	}
	m_nFirstDisplayedItem	= 0;
	m_nLastDisplayedItem	= -1;
	m_nCurrentIndex			= kNoSelection;

}

// Return a control at a specific index
CLTGUICtrl *CListCtrl::GetControl ( int nIndex )
{
    if (nIndex >= (int)m_controlArray.GetSize()) return LTNULL;
	return m_controlArray[nIndex];
}

// Select a control
void CListCtrl::SelectItem ( int nIndex )
{
	int nOldIndex=m_nCurrentIndex;

	if ( m_nCurrentIndex < (int)m_controlArray.GetSize() )
	{
		m_controlArray[m_nCurrentIndex]->Select(FALSE);
	}

	if ( m_controlArray.GetSize() == 0 )
	{
		m_nFirstDisplayedItem=0;
		m_nCurrentIndex=kNoSelection;
		return;
	}

	m_nCurrentIndex=nIndex;

	if (nIndex == kNoSelection)
		return;

	m_controlArray[m_nCurrentIndex]->Select(TRUE);

	// Figure out if we should move the up or down
	if ( m_nCurrentIndex < m_nFirstDisplayedItem )
	{
		while ( m_nCurrentIndex < m_nFirstDisplayedItem )
		{
			m_nFirstDisplayedItem--;
		}
		m_nLastDisplayedItem = CalculateLastDisplayedIndex(m_nFirstDisplayedItem);
	}
	else if ( m_nLastDisplayedItem < m_nCurrentIndex )
	{
		while ( m_nLastDisplayedItem < m_nCurrentIndex &&
				m_nFirstDisplayedItem < (int)m_controlArray.GetSize() )
		{
			m_nFirstDisplayedItem++;
			m_nLastDisplayedItem = CalculateLastDisplayedIndex(m_nFirstDisplayedItem);
		}
	}

}

// Return the last displayed index
int	CListCtrl::CalculateLastDisplayedIndex ( int nStartIndex )
{
    LTIntPt size;
	int y=m_pos.y;
	if (nStartIndex < 0)
		nStartIndex = 0;

	unsigned int i;
	for ( i = nStartIndex; i < m_controlArray.GetSize(); i++ )
	{
		size.x=m_controlArray[i]->GetWidth();
		size.y=m_controlArray[i]->GetHeight();

		if ( y+size.y < m_pos.y+m_nHeight )
		{
			y+=size.y+m_nItemSpacing;;
		}
		else
		{
			return i-1;
		}
	}
	return m_controlArray.GetSize()-1;
}

// Gets the index of the control that is under the specific screen point.
// Returns FALSE if there isn't one under the specified point.
CLTGUICtrl *CListCtrl::GetControlUnderPoint(int xPos, int yPos, int *pnIndex)
{
	ASSERT(pnIndex);

	if (m_bArrows)
	{
		if (m_pUp)
		{
			int nLeft=m_pUp->GetPos().x;
			int nTop=m_pUp->GetPos().y;
			int nRight=nLeft+m_pUp->GetWidth();
			int nBottom=nTop+m_pUp->GetHeight();

			// Check to see if the click is in the bounding box for the control
			if (xPos >= nLeft && xPos <= nRight && yPos >= nTop && yPos <= nBottom)
			{
				*pnIndex= kUpSel;
				return m_pUp;
			}
		}
		if (m_pDown)
		{
			int nLeft=m_pDown->GetPos().x;
			int nTop=m_pDown->GetPos().y;
			int nRight=nLeft+m_pDown->GetWidth();
			int nBottom=nTop+m_pDown->GetHeight();

			// Check to see if the click is in the bounding box for the control
			if (xPos >= nLeft && xPos <= nRight && yPos >= nTop && yPos <= nBottom)
			{
				*pnIndex= kDownSel;
				return m_pDown;
			}
		}
	}

	// The first and last item to be checked
	int nFirstItem=0;
	int nLastItem=m_controlArray.GetSize()-1;

	nFirstItem=m_nFirstDisplayedItem;
	nLastItem=m_nLastDisplayedItem;

	if (nFirstItem >= 0)
	{
		// See if the user clicked on any of the controls.
		int i;
		for (i=nFirstItem;i <= nLastItem; i++)
		{
			int nLeft=m_controlArray[i]->GetPos().x;
			int nTop=m_controlArray[i]->GetPos().y;
			int nRight=nLeft+m_controlArray[i]->GetWidth();
			int nBottom=nTop+m_controlArray[i]->GetHeight();

			// Check to see if the click is in the bounding box for the control
			if (xPos >= nLeft && xPos <= nRight && yPos >= nTop && yPos <= nBottom)
			{
				*pnIndex=i;

				return m_controlArray[i];
			}
		}
	}

    return LTNULL;
}


// Set the number of pixels between items
void CListCtrl::SetItemSpacing ( int nSpacing )
{
	m_nItemSpacing=nSpacing;
	m_nLastDisplayedItem = CalculateLastDisplayedIndex(m_nFirstDisplayedItem);
}

// Sets the height of the listbox
void CListCtrl::SetHeight(int nHeight)
{
	m_nHeight=nHeight;
	m_nLastDisplayedItem = CalculateLastDisplayedIndex(m_nFirstDisplayedItem);
}


// The selection for this control has changed
void CListCtrl::OnSelChange()
{
	if (IsSelected())
	{
		if (m_controlArray.GetSize() > 0 && m_nCurrentIndex == kNoSelection)
			SelectItem(0);
	}
//	else
//		SelectItem(kNoSelection);
}

/******************************************************************/
LTBOOL CListCtrl::OnLButtonDblClick(int x, int y)
{
	x += g_pInterfaceResMgr->GetXOffset();
	y += g_pInterfaceResMgr->GetYOffset();
	m_fNextScrollTime = -1.0f;
	// Get the control that the click was on
	int nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{
		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
            return LTFALSE;


		// If the mouse is over the same control now as it was when the down message was called
		// then send the "enter" message to the control.
		if (nControlIndex == m_nMouseDownItemSel)
		{
			if (nControlIndex == kUpSel)
			{
				return (!CanScrollUp() || ScrollUp());
			}
			else if (nControlIndex == kDownSel)
			{
				return (!CanScrollDown() || ScrollDown());
			}
			else
			{
				return m_controlArray[nControlIndex]->OnLButtonDblClick( x,  y);
			}
		}
	}
	else
	{
		m_nMouseDownItemSel=kNoSelection;
	}
    return LTFALSE;
}

