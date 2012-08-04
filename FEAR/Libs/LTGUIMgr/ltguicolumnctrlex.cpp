// ----------------------------------------------------------------------- //
//
// MODULE  : ltguicolumnctrlex.cpp
//
// PURPOSE : Control to display columns of mixed types of controls
//
// CREATED : 12/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "ltguimgr.h"
#include "ltguicolumnctrlex.h"


const uint8	CLTGUIColumnCtrlEx::kMaxNumColumns = 64;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUIColumnCtrlEx::CLTGUIColumnCtrlEx() :
	m_nTextIndent(0)
{
}

CLTGUIColumnCtrlEx::~CLTGUIColumnCtrlEx()
{
	Destroy();
}

// Create the control
void CLTGUIColumnCtrlEx::Create(const CLTGUICtrl_create& cs)
{
	CLTGUICtrl::Create(cs);
	uint32 nHeight = m_rnBaseRect.GetHeight();
	CLTGUICtrl::SetSize( LTVector2n(0,nHeight) );
	m_bAllowColumnSelection = false;
	m_nCurrentIndex = kMaxNumColumns;
	m_nLBDownSel=kMaxNumColumns;
	m_nRBDownSel=kMaxNumColumns;

}

// Destroys the control
void CLTGUIColumnCtrlEx::Destroy ( )
{
	// Remove the columns
	RemoveAllColumns();
}

// Adds a column to the control
uint8 CLTGUIColumnCtrlEx::AddColumn(CLTGUICtrl* pCtrl)
{
	if ( !pCtrl )
	{
		LTERROR("CLTGUIColumnCtrlEx::AddColumn() - No control specified");
		return kMaxNumColumns;
	}
	
	if (m_columnArray.size() >= kMaxNumColumns)
	{
		LTERROR("CLTGUIColumnCtrlEx::AddColumn() - Control full");
		return kMaxNumColumns;
	}

	m_columnArray.push_back(pCtrl);

	LTVector2n vPos( m_rnBaseRect.Right(), m_rnBaseRect.Top() );
	pCtrl->SetBasePos( vPos );


	//adjust the width of the control
	LTVector2n sz(m_rnBaseRect.GetWidth(), m_rnBaseRect.GetHeight() );
	sz.x += pCtrl->GetBaseWidth();
	CLTGUICtrl::SetSize(sz);

    return ( uint8 )( m_columnArray.size() - 1 );
}

// Gets a string at a specific column index.  This returns a copy (new handle).
CLTGUICtrl*	 CLTGUIColumnCtrlEx::GetColumn(uint8 nColumnIndex) const
{
	if (nColumnIndex > kMaxNumColumns || nColumnIndex >= m_columnArray.size())
		return NULL;
	return m_columnArray[nColumnIndex];
}



// Removes a column
void CLTGUIColumnCtrlEx::RemoveColumn(uint8 nIndex)
{
	if (nIndex >= m_columnArray.size())
		return;

	uint8 nOldIndex = m_nCurrentIndex;
	SelectColumn(kMaxNumColumns);

	ControlArray::iterator iter = m_columnArray.begin();
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
void CLTGUIColumnCtrlEx::ShowColumn(uint8 nIndex, bool bShow)
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



void CLTGUIColumnCtrlEx::ResetColumns()
{
	//adjust sizes and positions
	LTVector2n tmpPos = m_rnBaseRect.m_vMin;
	LTVector2n sz(0,m_rnBaseRect.GetHeight());
	for (int i=0; i < GetNumColumns(); i++)
	{
		if( m_columnArray[i]->ShouldIndent() )
		{
			tmpPos.x += m_nTextIndent;
			sz.x += m_nTextIndent;
		}

		m_columnArray[i]->SetBasePos(tmpPos);

		if (m_columnArray[i]->IsVisible()) 
		{
			uint32 nWidth = m_columnArray[i]->GetBaseWidth();

			if( m_columnArray[i]->ShouldIndent() )
				nWidth += m_nTextIndent;

			tmpPos.x += nWidth;
			sz.x += nWidth;
		}
		
	}
	CLTGUICtrl::SetSize(sz);
	
}

// Removes all of the columns
void CLTGUIColumnCtrlEx::RemoveAllColumns()
{
	ControlArray::iterator iter = m_columnArray.begin();

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
void CLTGUIColumnCtrlEx::Render ()
{
	if (!IsVisible()) return;

	for (int i=0; i < GetNumColumns(); i++)
	{
		if (m_columnArray[i]->IsVisible())
			m_columnArray[i]->Render();
	}
}

// Render the control
void CLTGUIColumnCtrlEx::RenderTransition(float fTrans )
{
	if (!IsVisible()) return;

	for (int i=0; i < GetNumColumns(); i++)
	{
		if (m_columnArray[i]->IsVisible())
			m_columnArray[i]->RenderTransition(fTrans);
	}
}


// Render the control
void CLTGUIColumnCtrlEx::OnSelChange()
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


// Handle the Enter key being pressed
bool CLTGUIColumnCtrlEx::OnEnter ( )
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


void CLTGUIColumnCtrlEx::SetBasePos(const LTVector2n& pos )
{ 
	CLTGUICtrl::SetBasePos(pos);
	ResetColumns();
}

void CLTGUIColumnCtrlEx::SetSize(const LTVector2n& sz )
{ 
	LTUNREFERENCED_PARAMETER(sz);
	LTASSERT( (false), "SetSize() not a valid function for CLTGUIColumnCtrlEx");
}

void CLTGUIColumnCtrlEx::SetScale(const LTVector2& vfScale )
{
	CLTGUICtrl::SetScale(vfScale);
	for (int i=0; i < GetNumColumns(); i++)
	{
		m_columnArray[i]->SetScale(vfScale);
	}
}

void CLTGUIColumnCtrlEx::SetColors(uint32 argbSelected, uint32 argbNormal, uint32 argbDisabled)
{	
	CLTGUICtrl::SetColors(argbSelected, argbNormal, argbDisabled);
	for (int i=0; i < GetNumColumns(); i++)
	{
		m_columnArray[i]->SetColors(m_argbSelected, m_argbNormal, m_argbDisabled);
	}
}

void CLTGUIColumnCtrlEx::Enable ( bool bEnabled )
{ 
	CLTGUICtrl::Enable (bEnabled);
	for (uint8 i = 0; i < m_columnArray.size(); i++ )
	{
		m_columnArray[i]->Enable (bEnabled);
	}

}

// free texture memory by flushing any texture strings owned by the control
void CLTGUIColumnCtrlEx::FlushTextureStrings()
{
	for (uint8 i = 0; i < m_columnArray.size(); i++ )
	{
		m_columnArray[i]->FlushTextureStrings();
	}
}

// rebuild any texture strings owned by the control
void CLTGUIColumnCtrlEx::RecreateTextureStrings()
{
	for (uint8 i = 0; i < m_columnArray.size(); i++ )
	{
		m_columnArray[i]->RecreateTextureStrings();
	}

}

bool CLTGUIColumnCtrlEx::OnLeft()
{
	if (!m_bAllowColumnSelection || m_nCurrentIndex >= m_columnArray.size())
		return false;

	if (m_nCurrentIndex == 0)
		SelectColumn(m_columnArray.size()-1);
	else
		SelectColumn(m_nCurrentIndex-1);
	return true;
}
bool CLTGUIColumnCtrlEx::OnRight() 
{
	if (!m_bAllowColumnSelection || m_nCurrentIndex >= m_columnArray.size())
		return false;

	uint8 n = (m_nCurrentIndex + 1) % m_columnArray.size();
	SelectColumn(n);
	return true;
}



// Handles the left button down message
bool CLTGUIColumnCtrlEx::OnLButtonDown(int x, int y)
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
bool CLTGUIColumnCtrlEx::OnLButtonUp(int x, int y)
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
bool CLTGUIColumnCtrlEx::OnRButtonDown(int x, int y)
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
bool CLTGUIColumnCtrlEx::OnRButtonUp(int x, int y)
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
bool CLTGUIColumnCtrlEx::OnMouseMove(int x, int y)
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
CLTGUICtrl *CLTGUIColumnCtrlEx::GetControlUnderPoint(int xPos, int yPos, uint8 *pnIndex)
{
	ASSERT(pnIndex);

	// See if the user clicked on any of the controls.
	uint8 i;
	for (i=0; i < m_columnArray.size(); i++)
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
uint8 CLTGUIColumnCtrlEx::SelectColumn( uint8 nIndex )
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

const char* CLTGUIColumnCtrlEx::GetHelpID()
{ 

	if (m_bAllowColumnSelection && m_nCurrentIndex < m_columnArray.size())
	{
		return m_columnArray[m_nCurrentIndex]->GetHelpID();
	}

	return m_szHelpID; 
}



// Set the font
void CLTGUIColumnCtrlEx::SetFont(const CFontInfo& Font)
{
	if (!Font.m_nHeight)
	{
		return;
	}

	m_Font = Font;

}


// Adds a column to the control
// nWidth	  - Width of the column
// hString	  - The initial text for the column
uint8 CLTGUIColumnCtrlEx::AddTextColumn(const wchar_t *pString, uint32 nWidth, bool bClip, const CLTGUICtrl_create& cs)
{
	uint8 nIndex = AddTextColumn(pString,nWidth,bClip);
	CLTGUITextCtrl *pTxt = (CLTGUITextCtrl *)m_columnArray[nIndex];

	pTxt->SetCommandID(cs.nCommandID);
	pTxt->SetHelpID(cs.szHelpID);
	pTxt->SetCommandHandler(cs.pCommandHandler);
	pTxt->SetParam1(cs.nParam1);
	pTxt->SetParam2(cs.nParam1);

	return nIndex;

}


uint8 CLTGUIColumnCtrlEx::AddTextColumn(const wchar_t *pString, uint32 nWidth, bool bClip /*= false*/, eTextAlign align /*= kLeft*/)
{
	if ( !pString )
	{
		assert(false);
		return kMaxNumColumns;
	}

	nWidth -= (m_nTextIndent + m_nTextIndent);

	CLTGUICtrl_create cs;
	cs.rnBaseRect.Left() = m_rnBaseRect.Right() + m_nTextIndent;
	cs.rnBaseRect.Top() = m_rnBaseRect.Top();
	cs.rnBaseRect.Right() = m_rnBaseRect.Right() + nWidth;
	cs.rnBaseRect.Bottom() = m_rnBaseRect.Bottom();

	CLTGUITextCtrl *pCtrl = debug_new(CLTGUITextCtrl);
	if (!pCtrl->Create(pString, m_Font, cs, bClip))
	{
		debug_delete(pCtrl);
		return ( uint8 )-1;
	}
	m_columnArray.push_back(pCtrl);

	pCtrl->SetWordWrap(!bClip);
	pCtrl->SetClipping(bClip);
	pCtrl->SetColors(m_argbSelected, m_argbNormal, m_argbDisabled);
	pCtrl->SetScale(m_vfScale);
	pCtrl->SetAlignment( align );


	//adjust the width of the control
	LTVector2n sz(m_rnBaseRect.GetWidth(), m_rnBaseRect.GetHeight() );
	sz.x += nWidth + m_nTextIndent + m_nTextIndent;
	CLTGUICtrl::SetSize(sz);

	ASSERT(m_columnArray.size() - 1 <= 255 );
	return ( uint8 )( m_columnArray.size() - 1 );
}

uint8 CLTGUIColumnCtrlEx::AddIconColumn(const char *pTex, uint32 nWidth, const LTVector2n& vSize )
{
	if ( !pTex )
	{
		assert(false);
		return kMaxNumColumns;
	}


	CLTGUITextureButton_create cs;

	cs.rnImageRect.m_vMin.x = (nWidth - vSize.x )/ 2;
	cs.rnImageRect.m_vMax = cs.rnImageRect.m_vMin + vSize;
	cs.bCenterImage = true;

	cs.rnBaseRect.Left() = m_rnBaseRect.Right();
	cs.rnBaseRect.Top() = m_rnBaseRect.Top();
	cs.rnBaseRect.Right() = m_rnBaseRect.Right() + nWidth;
	cs.rnBaseRect.Bottom() = m_rnBaseRect.Top() + vSize.y;
	
	cs.hNormal.Load(pTex);

	CLTGUITextureButton *pCtrl = debug_new(CLTGUITextureButton);
	if (!pCtrl->Create(cs))
	{
		debug_delete(pCtrl);
		return ( uint8 )-1;
	}
	m_columnArray.push_back(pCtrl);

	pCtrl->SetColors(m_argbSelected, m_argbNormal, m_argbDisabled);
	pCtrl->SetScale(m_vfScale);

	//adjust the width of the control
	LTVector2n sz(m_rnBaseRect.GetWidth(), m_rnBaseRect.GetHeight() );
	sz.x += nWidth;
	CLTGUICtrl::SetSize(sz);

	ASSERT(m_columnArray.size() - 1 <= 255 );
	return ( uint8 )( m_columnArray.size() - 1 );
}
