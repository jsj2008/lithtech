// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIColumnCtrl.cpp
//
// PURPOSE : Control to display columns of text
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "ltguimgr.h"
#include "ltguiColumnCtrl.h"

const uint8	CLTGUIColumnCtrl::kMaxNumColumns = 64;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUIColumnCtrl::CLTGUIColumnCtrl()
{
}

CLTGUIColumnCtrl::~CLTGUIColumnCtrl()
{
	Destroy();
}

// Create the control
bool CLTGUIColumnCtrl::Create(const CFontInfo& Font, const CLTGUICtrl_create& cs)
{
	SetFont(Font,false);
	CLTGUICtrl::Create(cs);
	uint32 nHeight = m_rnBaseRect.GetHeight();
	CLTGUICtrl::SetSize( LTVector2n(0,nHeight) );
	m_bAllowColumnSelection = false;
	m_nCurrentIndex = kMaxNumColumns;
	m_nLBDownSel=kMaxNumColumns;
	m_nRBDownSel=kMaxNumColumns;
    return true;
}

// Destroys the control
void CLTGUIColumnCtrl::Destroy ( )
{
	// Remove the columns
	RemoveAllColumns();
}

// Adds a column to the control
// nWidth	  - Width of the column
// hString	  - The initial text for the column
uint8 CLTGUIColumnCtrl::AddColumn(const wchar_t *pString, uint32 nWidth, bool bClip, const CLTGUICtrl_create& cs)
{
	uint8 nIndex = AddColumn(pString,nWidth,bClip);
	CLTGUITextCtrl *pTxt = m_columnArray[nIndex];

	pTxt->SetCommandID(cs.nCommandID);
	pTxt->SetHelpID(cs.szHelpID);
	pTxt->SetCommandHandler(cs.pCommandHandler);
	pTxt->SetParam1(cs.nParam1);
	pTxt->SetParam2(cs.nParam1);

	return nIndex;

}


uint8 CLTGUIColumnCtrl::AddColumn(const wchar_t *pString, uint32 nWidth, bool bClip)
{
	if ( !pString )
	{
        assert(false);
        return kMaxNumColumns;
	}


	CLTGUICtrl_create cs;
	cs.rnBaseRect.Left() = m_rnBaseRect.Right();
	cs.rnBaseRect.Top() = m_rnBaseRect.Top();
	cs.rnBaseRect.Right() = m_rnBaseRect.Right() + nWidth;
	cs.rnBaseRect.Bottom() = m_rnBaseRect.Bottom();

	CLTGUITextCtrl *pCtrl = debug_new(CLTGUITextCtrl);
	if (!pCtrl->Create(pString, m_Font, cs))
	{
		debug_delete(pCtrl);
		return ( uint8 )-1;
	}
	m_columnArray.push_back(pCtrl);

	pCtrl->SetWordWrap(!bClip);
	pCtrl->SetClipping(bClip);
    pCtrl->SetColors(m_argbSelected, m_argbNormal, m_argbDisabled);
	pCtrl->SetScale(m_vfScale);


	//adjust the width of the control
	LTVector2n sz(m_rnBaseRect.GetWidth(), m_rnBaseRect.GetHeight() );
	sz.x += nWidth;
	CLTGUICtrl::SetSize(sz);

	ASSERT(m_columnArray.size() - 1 <= 255 );
    return ( uint8 )( m_columnArray.size() - 1 );
}


// Gets a string at a specific column index.  This returns a copy (new handle).
CLTGUITextCtrl*	 CLTGUIColumnCtrl::GetColumn(uint8 nColumnIndex) const
{
	if (nColumnIndex > kMaxNumColumns || nColumnIndex >= m_columnArray.size())
		return NULL;
	return m_columnArray[nColumnIndex];
}


// Sets a string for a column
void CLTGUIColumnCtrl::SetString(uint8 nColumnIndex, const wchar_t *pString)
{
	if (nColumnIndex >= m_columnArray.size() || !pString)
		return;

	// Allocate the new string
	m_columnArray[nColumnIndex]->SetString(pString);
}


// Removes a column
void CLTGUIColumnCtrl::RemoveColumn(uint8 nIndex)
{
	if (nIndex >= m_columnArray.size())
		return;

	uint8 nOldIndex = m_nCurrentIndex;
	SelectColumn(kMaxNumColumns);

	TextControlArray::iterator iter = m_columnArray.begin();
	iter += nIndex;

	if (iter == m_columnArray.end())
		return;

	debug_delete(*iter);
	m_columnArray.erase(iter);
	ResetColumns();

	if (nOldIndex >= nIndex)
	{
		--nOldIndex;
	}
	SelectColumn(nOldIndex);
}

// hides/shows a column
void CLTGUIColumnCtrl::ShowColumn(uint8 nIndex, bool bShow)
{
	if (nIndex >= m_columnArray.size())
		return;

	if (bShow != m_columnArray[nIndex]->IsVisible())
	{
		m_columnArray[nIndex]->Show(bShow);
		ResetColumns();
	}

	if (nIndex == m_nCurrentIndex && !bShow)
	{
		SelectColumn(kMaxNumColumns);
	}
}



void CLTGUIColumnCtrl::ResetColumns()
{
	//adjust sizes and positions
	LTVector2n tmpPos = m_rnBaseRect.m_vMin;
	LTVector2n sz(0,m_rnBaseRect.GetHeight());
	for (int i=0; i < GetNumColumns(); i++)
	{
		m_columnArray[i]->SetBasePos(tmpPos);
		if (m_columnArray[i]->IsVisible()) 
		{
			tmpPos.x += m_columnArray[i]->GetBaseWidth();
			sz.x += m_columnArray[i]->GetBaseWidth();
		}
		
	}
	CLTGUICtrl::SetSize(sz);
	
}

// Removes all of the columns
void CLTGUIColumnCtrl::RemoveAllColumns()
{
	TextControlArray::iterator iter = m_columnArray.begin();

	while (iter != m_columnArray.end())
	{
		debug_delete(*iter);
		iter++;
	}
	m_columnArray.clear();

	uint32 nHeight = m_rnBaseRect.GetHeight();
	CLTGUICtrl::SetSize( LTVector2n(0,nHeight) );

	m_nCurrentIndex = kMaxNumColumns;

}

// Render the control
void CLTGUIColumnCtrl::Render ()
{
	if (!IsVisible()) return;

	for (int i=0; i < GetNumColumns(); i++)
	{
		if (m_columnArray[i]->IsVisible())
			m_columnArray[i]->Render();
	}
}

// Render the control
void CLTGUIColumnCtrl::RenderTransition(float fTrans )
{
	if (!IsVisible()) return;

	for (int i=0; i < GetNumColumns(); i++)
	{
		if (m_columnArray[i]->IsVisible())
			m_columnArray[i]->RenderTransition(fTrans);
	}
}


// Render the control
void CLTGUIColumnCtrl::OnSelChange()
{

	if (m_bAllowColumnSelection && IsSelected())
	{
		if (m_nCurrentIndex > m_columnArray.size()) 
		{
			SelectColumn(0);
		}
		else
		{
			SelectColumn(m_nCurrentIndex);
		}
		
	}
	else
	{
		for (int i=0; i < GetNumColumns(); i++)
		{
			m_columnArray[i]->Select(IsSelected());
		}
	}
}

// Set the font
void CLTGUIColumnCtrl::SetFont(const CFontInfo& Font, bool bSetForAll)
{
	if (!Font.m_nHeight)
	{
		return;
	}

	m_Font = Font;

	if (bSetForAll)
	{
		for (int i=0; i < GetNumColumns(); i++)
		{
			m_columnArray[i]->SetFont(Font);
		}
	}
}

// Set the font
void CLTGUIColumnCtrl::SetFontHeight(uint32 nFontHeight, bool bSetForAll)
{
	if (!nFontHeight)
	{
		return;
	}

	m_Font.m_nHeight = nFontHeight;

	if (bSetForAll)
	{
		for (int i=0; i < GetNumColumns(); i++)
		{
			m_columnArray[i]->SetFontHeight(nFontHeight);
		}
	}
}


// Handle the Enter key being pressed
bool CLTGUIColumnCtrl::OnEnter ( )
{
	if (m_bAllowColumnSelection )
	{
		if (m_nCurrentIndex < m_columnArray.size())
		{
			return m_columnArray[m_nCurrentIndex]->OnEnter();
		}
	}

	// Send the command
	if ( m_pCommandHandler )
	{
		if	(m_pCommandHandler->SendCommand(m_nCommandID, m_nParam1, m_nParam2))
			return true;
	}
    return false;
}


void CLTGUIColumnCtrl::SetBasePos(const LTVector2n& pos )
{ 
	CLTGUICtrl::SetBasePos(pos);
	ResetColumns();
}

void CLTGUIColumnCtrl::SetSize(const LTVector2n& sz )
{ 
	LTUNREFERENCED_PARAMETER(sz);
	LTASSERT( (false), "SetSize() not a valid function for CLTGUIColumnCtrl");
}

void CLTGUIColumnCtrl::SetScale(const LTVector2& vfScale )
{
	CLTGUICtrl::SetScale(vfScale);
	for (int i=0; i < GetNumColumns(); i++)
	{
		m_columnArray[i]->SetScale(vfScale);
	}
}

void CLTGUIColumnCtrl::SetColors(uint32 argbSelected, uint32 argbNormal, uint32 argbDisabled)
{	
	CLTGUICtrl::SetColors(argbSelected, argbNormal, argbDisabled);
	for (int i=0; i < GetNumColumns(); i++)
	{
		m_columnArray[i]->SetColors(m_argbSelected, m_argbNormal, m_argbDisabled);
	}
}

void CLTGUIColumnCtrl::Enable ( bool bEnabled )
{ 
	CLTGUICtrl::Enable (bEnabled);
	for (uint8 i = 0; i < m_columnArray.size(); i++ )
	{
		m_columnArray[i]->Enable (bEnabled);
	}

}

// free texture memory by flushing any texture strings owned by the control
void CLTGUIColumnCtrl::FlushTextureStrings()
{
	for (uint8 i = 0; i < m_columnArray.size(); i++ )
	{
		m_columnArray[i]->FlushTextureStrings();
	}
}

// rebuild any texture strings owned by the control
void CLTGUIColumnCtrl::RecreateTextureStrings()
{
	for (uint8 i = 0; i < m_columnArray.size(); i++ )
	{
		m_columnArray[i]->RecreateTextureStrings();
	}

}

bool CLTGUIColumnCtrl::OnLeft()
{
	if (!m_bAllowColumnSelection || m_nCurrentIndex >= m_columnArray.size())
		return false;

	if (m_nCurrentIndex == 0)
		SelectColumn(m_columnArray.size()-1);
	else
		SelectColumn(m_nCurrentIndex-1);
	return true;
}
bool CLTGUIColumnCtrl::OnRight() 
{
	if (!m_bAllowColumnSelection || m_nCurrentIndex >= m_columnArray.size())
		return false;

	uint8 n = (m_nCurrentIndex + 1) % m_columnArray.size();
	SelectColumn(n);
	return true;
}



// Handles the left button down message
bool CLTGUIColumnCtrl::OnLButtonDown(int x, int y)
{
	if (!m_bAllowColumnSelection)
		return false;

	// Get the control that the click was on
	uint8 nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{

		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
			return false;

		// Select the control
		SelectColumn(nControlIndex);

		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.
		m_nLBDownSel=nControlIndex;
		m_columnArray[nControlIndex]->OnLButtonDown(x,y);
		return true;
	}
	else
	{
		// This clears the index for what item was selected from a mouse down message
		m_nLBDownSel=kMaxNumColumns;

		return false;
	}
}

// Handles the left button up message
bool CLTGUIColumnCtrl::OnLButtonUp(int x, int y)
{
	if (!m_bAllowColumnSelection)
		return OnEnter();

	// Get the control that the click was on
	uint8 nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{
		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
			return false;

		// If the mouse is over the same control now as it was when the down message was called
		// then send the "enter" message to the control.
		if (nControlIndex == m_nLBDownSel)
		{
			return m_columnArray[nControlIndex]->OnLButtonUp(x,y);
		}
	}
	else
	{
		m_nLBDownSel=kMaxNumColumns;
	}
	return false;
}

// Handles the right button down message
bool CLTGUIColumnCtrl::OnRButtonDown(int x, int y)
{
	if (!m_bAllowColumnSelection)
		return false;

	// Get the control that the click was on
	uint8 nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{
		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
			return false;

		// Select the control
		SelectColumn(nControlIndex);

		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.
		m_nRBDownSel=nControlIndex;
		m_columnArray[nControlIndex]->OnRButtonDown(x,y);
		return true;
	}
	else
	{
		// This clears the index for what item was selected from a mouse down message
		m_nLBDownSel=kMaxNumColumns;

		return false;
	}
}

// Handles the right button up message
bool CLTGUIColumnCtrl::OnRButtonUp(int x, int y)
{
	if (!m_bAllowColumnSelection)
		return false;

	// Get the control that the click was on
	uint8 nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{
		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
			return false;

		// If the mouse is over the same control now as it was when the down message was called
		// then send the "enter" message to the control.
		if (nControlIndex == m_nRBDownSel)
		{
			return m_columnArray[nControlIndex]->OnRButtonUp(x,y);
		}
	}
	else
	{
		m_nRBDownSel=kMaxNumColumns;
	}
	return false;
}

// Handles the mouse move message
bool CLTGUIColumnCtrl::OnMouseMove(int x, int y)
{

	if (!m_bAllowColumnSelection)
		return false;

	uint8 nControlUnderPoint=0;

	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlUnderPoint);

	if(pCtrl)
	{
		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
			return false;

		pCtrl->OnMouseMove(x,y);

		if ( m_nCurrentIndex != nControlUnderPoint)
		{
			SelectColumn(nControlUnderPoint);
			return true;
		}
	}

	return false;
}
// Gets the index of the control that is under the specific screen point.
// Returns FALSE if there isn't one under the specified point.
CLTGUICtrl *CLTGUIColumnCtrl::GetControlUnderPoint(int xPos, int yPos, uint8 *pnIndex)
{
	ASSERT(pnIndex);

	// See if the user clicked on any of the controls.
	uint8 i;
	for (i=0; i <= m_columnArray.size(); i++)
	{
		// Check to see if the click is in the bounding box for the control
		if (m_columnArray[i]->IsOnMe(xPos,yPos))
		{
			*pnIndex=i;

			return m_columnArray[i];
		}
	}

	return NULL;
}


// Select a control
uint8 CLTGUIColumnCtrl::SelectColumn( uint8 nIndex )
{
	if (!m_bAllowColumnSelection)
		return kMaxNumColumns;

	if (nIndex >= m_columnArray.size() && nIndex != kMaxNumColumns)
		return m_nCurrentIndex;

	if (m_nCurrentIndex==nIndex)
	{
		if (m_nCurrentIndex < m_columnArray.size() )
		{
			m_columnArray[m_nCurrentIndex]->Select(true);
		}
		return m_nCurrentIndex;
	}

	if (m_nCurrentIndex < m_columnArray.size() )
	{
		m_columnArray[m_nCurrentIndex]->Select(false);
	}


	m_nCurrentIndex=nIndex;


	if (nIndex == kMaxNumColumns)
		return nIndex;

	m_columnArray[m_nCurrentIndex]->Select(true);

	return m_nCurrentIndex;
}

const char* CLTGUIColumnCtrl::GetHelpID()
{ 

	if (m_bAllowColumnSelection && m_nCurrentIndex < m_columnArray.size())
	{
		return m_columnArray[m_nCurrentIndex]->GetHelpID();
	}

	return m_szHelpID; 
}
