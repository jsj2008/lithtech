// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIListCtrl.cpp
//
// PURPOSE : Control which maintains a scrolling list of other controls.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "ltguimgr.h"
#include "ltguilistctrl.h"
#if !defined(PLATFORM_XENON)
#include "vkdefs.h"
#endif // !PLATFORM_XENON

const uint32 CLTGUIListCtrl::kNoSelection = 0xFFFFFFFF;
const uint32 CLTGUIListCtrl::kMaxNumControls = 0xFFFFFFFE;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUIListCtrl::CLTGUIListCtrl()
{
	m_nCurrentIndex			= 0;
	m_nFirstShown			= 0;
	m_nLastShown			= kNoSelection;
	m_nItemSpacing			= 0;
	m_nLBDownSel			= kNoSelection;
	m_nRBDownSel			= kNoSelection;
	m_bNeedsRecalculation	= true;
	m_bScrollWrap			= true;
	m_bScrollByPage			= false;
	m_nFrameWidth			= 0;

	m_pUp = NULL;
	m_pDown = NULL;
	m_nArrowOffset = 0;

	m_indent.x = 0;
	m_indent.y = 0;

	m_bAutoSelect = true;
}

CLTGUIListCtrl::~CLTGUIListCtrl()
{
	Destroy();
}

// Creation
bool CLTGUIListCtrl::Create(const CLTGUIListCtrl_create& cs )
{
	if (m_pUp)
	{
		debug_delete(m_pUp);
		m_pUp = NULL;
	}
	if (m_pDown)
	{
		debug_delete(m_pDown);
		m_pDown = NULL;
	}

	if (cs.bArrows)
	{
		m_nArrowOffset = cs.nArrowOffset;

		
		CLTGUICtrl::Create( (CLTGUICtrl_create)cs );

		CLTGUITextureButton_create bcs;

		LTVector2n pos = GetBasePos();
		pos.x += m_nArrowOffset;

		bcs.rnBaseRect.m_vMin = pos;
		bcs.rnBaseRect.m_vMax = pos + cs.vnArrowSz;
		bcs.hNormal = cs.hUpNormal;
		bcs.hSelected = cs.hUpSelected;

		m_pUp = debug_new(CLTGUITextureButton);
		m_pUp->Create(bcs);

		pos.y = m_rnBaseRect.Bottom() - cs.vnArrowSz.y;
		bcs.rnBaseRect.m_vMin = pos;
		bcs.rnBaseRect.m_vMax = pos + cs.vnArrowSz;
		bcs.hNormal = cs.hDownNormal;
		bcs.hSelected = cs.hDownSelected;

		m_pDown = debug_new(CLTGUITextureButton);
		m_pDown->Create(bcs);
	}

	m_bAutoSelect = cs.bAutoSelect;

	CLTGUICtrl::Create((CLTGUICtrl_create)cs);

    return true;
}

// Destroy the control
void CLTGUIListCtrl::Destroy ( )
{
	if (m_bCreated && m_TexFrame.IsCreated())
	{
		m_TexFrame.Destroy();
	}

	RemoveAll();
	if (m_pUp)
	{
		debug_delete(m_pUp);
		m_pUp = NULL;
	}
	if (m_pDown)
	{
		debug_delete(m_pDown);
		m_pDown = NULL;
	}

    m_bCreated=false;
}

// Render the control
void CLTGUIListCtrl::Render ( )
{
	if (!IsVisible()) return;

	if (m_bNeedsRecalculation)
		CalculatePositions();

	if (m_TexFrame.IsCreated())
	{
		m_TexFrame.Render();
	}

	if (m_nFrameWidth)
	{
		// set up the render state	
		SetRenderState();

		for (int f = 0;f < 4; ++f)
			DrawPrimSetRGBA(m_Frame[f],GetCurrentColor());

		// draw our frames
		g_pDrawPrim->DrawPrim(m_Frame,4);
	}


	g_pTextureString->SetupTextRendering(g_pDrawPrim);
	if (m_controlArray.size() > 0)
	{
		// Render the items unless they are off the control
		for (uint32 i = m_nFirstShown; i <= m_nLastShown && i < m_controlArray.size( ); i++ )
		{
			if (m_controlArray[i]->IsVisible())
				m_controlArray[i]->Render ();
		}

		if (m_pUp && m_bEnabled && m_nFirstShown > 0)
			m_pUp->Render();
		if (m_pDown && m_bEnabled && m_nLastShown < (m_controlArray.size()-1) )
			m_pDown->Render();

	}
}

// Render the control
void CLTGUIListCtrl::RenderTransition(float fTrans)
{
	if (!IsVisible()) return;

	if (m_bNeedsRecalculation)
		CalculatePositions();

	if (m_TexFrame.IsCreated())
	{
		m_TexFrame.RenderTransition(fTrans);
	}

	if (m_nFrameWidth)
	{
		// set up the render state	
		SetRenderState();

		for (int f = 0;f < 4; ++f)
			DrawPrimSetRGBA(m_Frame[f],FadeARGB(GetCurrentColor(),fTrans));

		// draw our frames
		g_pDrawPrim->DrawPrim(m_Frame,4);
	}

	if (m_controlArray.size() > 0)
	{
		// Render the items unless they are off the control
		for (uint32 i = m_nFirstShown; i <= m_nLastShown && i < m_controlArray.size( ); i++ )
		{
			if (m_controlArray[i]->IsVisible())
				m_controlArray[i]->RenderTransition(fTrans);
		}
	}
}



// Handle a keypress
bool CLTGUIListCtrl::HandleKeyDown(int key, int rep)
{
	if (key == VK_PRIOR)
		return OnPageUp();
	if (key == VK_NEXT)
		return OnPageDown();

	if (CLTGUICtrl::HandleKeyDown(key, rep))
	{
        return true;
	}
    bool handled = false;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->HandleKeyDown(key,rep);
	return handled;

}

bool CLTGUIListCtrl::OnUp ( )
{
	if (m_bScrollByPage) return OnPageUp();
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->OnUp())
		return true;

	uint32 sel = m_nCurrentIndex;
	return (sel != PreviousSelection() || m_bScrollWrap);
}

bool CLTGUIListCtrl::OnDown ( )
{
	if (m_bScrollByPage) return OnPageDown();
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->OnDown())
		return true;

	uint32 sel = m_nCurrentIndex;
	return (sel != NextSelection() || m_bScrollWrap);
}


bool  CLTGUIListCtrl::OnLeft ( )
{
    bool handled = false;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnLeft();
	return handled;
}

bool  CLTGUIListCtrl::OnRight ( )
{
    bool handled = false;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnRight();
	return handled;
}

bool  CLTGUIListCtrl::OnEnter ( )
{
    bool handled = false;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnEnter();
	return handled;
}

bool CLTGUIListCtrl::OnPageUp ( )
{
	if (m_nFirstShown == 0) return false;
	uint32 nSel = m_nFirstShown;

	int nHeight = m_rnBaseRect.GetHeight() - (2 * m_indent.y);

	int h = m_controlArray[nSel]->GetBaseHeight();
	while (nHeight >= h && nSel > 0)
	{
		//get the control's height (but unscale it)
		nHeight -= h;
		nSel--;
		h = m_controlArray[nSel]->GetBaseHeight();

		if (nHeight >= h)
			nHeight -= m_nItemSpacing;

	}


	m_nFirstShown = nSel;
	SetSelection(nSel);
	m_bNeedsRecalculation = true;

	return true;
}

bool CLTGUIListCtrl::OnPageDown ( )
{
	if (m_controlArray.size() == 0) return false;
	if (m_nLastShown >= (m_controlArray.size()-1)) return false;

	SetSelection(m_nLastShown+1);
	m_nFirstShown = m_nCurrentIndex;
	m_bNeedsRecalculation = true;

	return true;
	
}

// Handles the left button down message
bool CLTGUIListCtrl::OnLButtonDown(int x, int y)
{
	// Get the control that the click was on
	uint32 nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{
		if (pCtrl == m_pUp)
		{
			m_nLBDownSel=kNoSelection;
			m_pUp->Select(true);
			return true;
		}
		if (pCtrl == m_pDown)
		{
			m_nLBDownSel=kNoSelection;
			m_pDown->Select(true);
			return true;
		}

		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
            return false;

		// Select the control
		SetSelection(nControlIndex);

		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.
		m_nLBDownSel=nControlIndex;
		m_controlArray[nControlIndex]->OnLButtonDown(x,y);
        return true;
	}
	else
	{
		// This clears the index for what item was selected from a mouse down message
		m_nLBDownSel=kNoSelection;

        return false;
	}
}

// Handles the left button up message
bool CLTGUIListCtrl::OnLButtonUp(int x, int y)
{
	// Get the control that the click was on
	uint32 nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{
		if (pCtrl == m_pUp)
		{
			return OnUp();
		}
		if (pCtrl == m_pDown)
		{
			return OnDown();
		}

		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
            return false;

		// If the mouse is over the same control now as it was when the down message was called
		// then send the "enter" message to the control.
		if (nControlIndex == m_nLBDownSel)
		{
			return m_controlArray[nControlIndex]->OnLButtonUp(x,y);
		}
	}
	else
	{
		m_nLBDownSel=kNoSelection;
	}
    return false;
}

// Handles the right button down message
bool CLTGUIListCtrl::OnRButtonDown(int x, int y)
{
	// Get the control that the click was on
	uint32 nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{
		if (pCtrl == m_pUp)
		{
			return false;
		}
		if (pCtrl == m_pDown)
		{
			return false;
		}

		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
            return false;

		// Select the control
		SetSelection(nControlIndex);

		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.
		m_nRBDownSel=nControlIndex;
		m_controlArray[nControlIndex]->OnRButtonDown(x,y);
        return true;
	}
	else
	{
		// This clears the index for what item was selected from a mouse down message
		m_nLBDownSel=kNoSelection;

        return false;
	}
}

// Handles the right button up message
bool CLTGUIListCtrl::OnRButtonUp(int x, int y)
{
	// Get the control that the click was on
	uint32 nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{
		if (pCtrl == m_pUp)
		{
			return false;
		}
		if (pCtrl == m_pDown)
		{
			return false;
		}

		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
            return false;

		// If the mouse is over the same control now as it was when the down message was called
		// then send the "enter" message to the control.
		if (nControlIndex == m_nRBDownSel)
		{
			return m_controlArray[nControlIndex]->OnRButtonUp(x,y);
		}
	}
	else
	{
		m_nRBDownSel=kNoSelection;
	}
    return false;
}

// Handles the mouse move message
bool CLTGUIListCtrl::OnMouseMove(int x, int y)
{
	uint32 nControlUnderPoint=0;

	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlUnderPoint);

	if(pCtrl)
	{
		if(m_pUp && m_pDown)
		{
			if (pCtrl == m_pUp)
			{
				bool bRV = !m_pUp->IsSelected();
				m_pUp->Select(true);
				m_pDown->Select(false);
				return bRV;
			}
			else if (pCtrl == m_pDown)
			{
				bool bRV = !m_pDown->IsSelected();
				m_pUp->Select(false);
				m_pDown->Select(true);
				return bRV;
			}
			else
			{
				m_pUp->Select(false);
				m_pDown->Select(false);
			}
		}

		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
            return false;

		pCtrl->OnMouseMove(x,y);

		if ( m_bAutoSelect && GetSelectedIndex() != nControlUnderPoint)
		{
			SetSelection(nControlUnderPoint);

            return true;
		}
	}

    return false;
}

// Add a control the the array of controls
uint32 CLTGUIListCtrl::AddControl ( CLTGUICtrl *pControl )
{
	ASSERT(pControl);
	m_controlArray.push_back(pControl);

	if (!m_bEnabled)
		pControl->Enable(false);

	if ( m_controlArray.size() > 1 && (m_controlArray.size()-1) == m_nCurrentIndex )
	{
		m_controlArray[m_nCurrentIndex]->Select(true);
	}

	m_bNeedsRecalculation = true;
	return m_controlArray.size()-1;
}

// Add a control the the array of controls
bool CLTGUIListCtrl::InsertControl( uint32 nIndex, CLTGUICtrl* pControl )
{
	if( !pControl || m_controlArray.size( ) < nIndex )
	{
		LTERROR( "Invalid parameters." );
		return false;
	}

	m_controlArray.insert( m_controlArray.begin( ) + nIndex, pControl );

	if (!m_bEnabled)
		pControl->Enable(false);

	// If we inserted before the current selection, we need to advance our selection index.
	if( nIndex <= m_nCurrentIndex )
	{
		m_nCurrentIndex++;
	}

	m_bNeedsRecalculation = true;
	return true;
}

// Remove a control
void CLTGUIListCtrl::RemoveControl ( CLTGUICtrl *pControl, bool bDelete)
{
	ControlArray::iterator iter = m_controlArray.begin();
	while (iter != m_controlArray.end() && (*iter) != pControl)
		iter++;

	if (iter != m_controlArray.end())
	{
		if (bDelete)
			debug_delete(*iter);
		m_controlArray.erase(iter);
		m_bNeedsRecalculation = true;
	}
}

// Remove a control
void CLTGUIListCtrl::RemoveControl ( uint32 nIndex, bool bDelete )
{
	if (nIndex >= m_controlArray.size())
		return;

	ControlArray::iterator iter = m_controlArray.begin() + nIndex;
	if (bDelete)
		debug_delete(*iter);
	m_controlArray.erase(iter);
	m_bNeedsRecalculation = true;
}

// Removes all of the controls
void CLTGUIListCtrl::RemoveAll ( bool bDelete )
{
	if (bDelete)
	{
		ControlArray::iterator iter = m_controlArray.begin();
		while (iter != m_controlArray.end())
		{
			debug_delete(*iter);
			iter++;
		}
	}

	m_controlArray.clear();
	m_bNeedsRecalculation = true;
}

// Return a control at a specific index
CLTGUICtrl *CLTGUIListCtrl::GetControl ( uint32 nIndex ) const
{
	if (nIndex >= m_controlArray.size())
		return NULL;

	return m_controlArray[nIndex];
}

// Return index of a specific control
uint32 CLTGUIListCtrl::GetIndex(CLTGUICtrl *pCtrl) const
{
	if (pCtrl)
	{
		uint32 nIndex = 0;
		while (nIndex < m_controlArray.size())
		{
			//found it?
			if (m_controlArray[nIndex] == pCtrl)
				return nIndex;
			nIndex++;
		}
	}

	//didn't find it
	return kNoSelection;
}


// Select a control
void CLTGUIListCtrl::ClearSelection()
{
	if (m_nCurrentIndex < m_controlArray.size() )
	{
		m_controlArray[m_nCurrentIndex]->Select(false);
	}
	m_nCurrentIndex = kNoSelection;
	SelectionChanged();

}


// Select a control
uint32 CLTGUIListCtrl::SetSelection( uint32 nIndex )
{

	if (nIndex >= m_controlArray.size() && nIndex != kNoSelection)
		return m_nCurrentIndex;

	if (m_nCurrentIndex==nIndex)
	{
		if (m_nCurrentIndex < m_controlArray.size() )
		{
			m_controlArray[m_nCurrentIndex]->Select(true);
		}
		return m_nCurrentIndex;
	}

	if (m_nCurrentIndex < m_controlArray.size() )
	{
		m_controlArray[m_nCurrentIndex]->Select(false);
	}

	//find next selectable item
	while (nIndex < m_controlArray.size() && !m_controlArray[nIndex]->IsEnabled() )
	{
		nIndex++;
	}
	if (nIndex >= m_controlArray.size())
	{
		nIndex = kNoSelection;
	}

	m_nCurrentIndex=nIndex;


	if (nIndex == kNoSelection)
		return nIndex;


	m_controlArray[m_nCurrentIndex]->Select(true);

	if (m_bNeedsRecalculation)
		CalculatePositions();


	// Figure out if we should move the up or down
	if ( m_nCurrentIndex < m_nFirstShown )
	{
		m_nFirstShown = m_nCurrentIndex;
		m_bNeedsRecalculation = true;
	}
	else if ( m_nCurrentIndex > m_nLastShown )
	{
		int nTop = m_rnBaseRect.Top() + m_indent.y;
		int nBottom = (nTop + m_rnBaseRect.GetHeight()) - m_indent.y;

		int i = m_nCurrentIndex;
		do
		{
			//get the control's height (but unscale it)
			int h = m_controlArray[i]->GetBaseHeight();

			nBottom -= h;

			if (nBottom > nTop)
			{
				i--;
				nBottom -= m_nItemSpacing;
			}

			

		} while (nBottom > nTop);

		m_nFirstShown = (i+1);

		m_bNeedsRecalculation = true;
	}
	
	SelectionChanged();
	return m_nCurrentIndex;

}

void CLTGUIListCtrl::CalculatePositions()
{
	int32 y  = 0;
	uint32 h  = 0;
	uint32 w  = 0;

	m_nLastShown = kNoSelection;

	uint32 nBaseWidth = 0;
	if (m_pUp)
	{
		nBaseWidth = m_nArrowOffset + (m_pUp->GetBaseWidth() + 2);
	}

	uint32 nSpace = (uint32)m_indent.x;

	//step through the controls
	for (uint32 i = m_nFirstShown; i < m_controlArray.size(); i++ )
	{
		if (!m_controlArray[i]->IsVisible()) continue;

		LTVector2n pos = m_rnBaseRect.m_vMin;
		pos.x += m_indent.x;
		pos.y += (y+m_indent.y);
		m_controlArray[i]->SetBasePos(pos);

		//check the control's width
		w = m_controlArray[i]->GetBaseWidth() + nSpace;
		if (w > nBaseWidth)
			nBaseWidth = w;

		//get the control's height (but unscale it)
		h = m_controlArray[i]->GetBaseHeight();


		//if we haven't identified the last shown control
		//and the bottom of the control is beyond the height of the list
		// the previous control is the last one that fit
		uint32 ctrlBottom = y+h;
		uint32 rectBottom = m_rnBaseRect.GetHeight()-m_indent.y;
		
		if (m_nLastShown > m_controlArray.size() && ctrlBottom > rectBottom)
		{
			m_nLastShown = i-1;
		}

		y += (h + m_nItemSpacing);

	}

	if (GetBaseWidth() <  nBaseWidth)
	{
		LTVector2n sz(nBaseWidth, GetBaseHeight());
		SetSize(sz);
	}

	if (m_pUp && nBaseWidth < GetBaseWidth())
	{
		m_nArrowOffset = GetBaseWidth() - (m_pUp->GetBaseWidth() + 2);
		LTVector2n pos = m_rnBaseRect.m_vMin;
		pos.x += m_nArrowOffset;
		pos.y += 2;
		m_pUp->SetBasePos(pos);

		pos.y = (m_rnBaseRect.Bottom() - (m_pDown->GetBaseHeight() + 2 ));
		m_pDown->SetBasePos(pos);
	
	}


	if (m_nLastShown > m_controlArray.size())
	{
		m_nLastShown = m_controlArray.size()-1;
	}

	float frameW = ((float)m_nFrameWidth * m_vfScale.x);
	float frameH = ((float)m_nFrameWidth * m_vfScale.y);

	//top
	float fx = m_rfRect.Left() - frameW;
	float fy = m_rfRect.Top() - frameH;
	float fw = m_rfRect.GetWidth() + frameW;
	float fh = frameH;
	DrawPrimSetXYWH(m_Frame[0],fx,fy,fw,fh);

	//right
	fx = m_rfRect.Right();
	fy = m_rfRect.Top() - frameH;
	fw = frameW;
	fh = m_rfRect.GetHeight() + frameH;
	DrawPrimSetXYWH(m_Frame[1],fx,fy,fw,fh);

	//bottom
	fx = m_rfRect.Left();
	fy = m_rfRect.Bottom();
	fw = m_rfRect.GetWidth() + frameW;
	fh = frameH;
	DrawPrimSetXYWH(m_Frame[2],fx,fy,fw,fh);

	//left
	fx = m_rfRect.Left() - frameW;
	fy = m_rfRect.Top();
	fw = frameW;
	fh = m_rfRect.GetHeight() + frameH;
	DrawPrimSetXYWH(m_Frame[3],fx,fy,fw,fh);

	m_bNeedsRecalculation = false;
}

// Gets the index of the control that is under the specific screen point.
// Returns FALSE if there isn't one under the specified point.
CLTGUICtrl *CLTGUIListCtrl::GetControlUnderPoint(int xPos, int yPos, uint32 *pnIndex)
{
	ASSERT(pnIndex);

	if (m_pUp && (m_nFirstShown > 0) && m_pUp->IsOnMe(xPos,yPos))
	{
		*pnIndex=kNoSelection;
		return m_pUp;
	}

	if (m_pDown && (m_nLastShown < (m_controlArray.size()-1)) && m_pDown->IsOnMe(xPos,yPos))
	{
		*pnIndex=kNoSelection;
		return m_pDown;
	}

	//alright, so the user didn't click on either the up or down arrows, but lets see if they
	//are in the scrollbar gutter, because if they are, we shouldn't allow them to
	if(m_pUp && m_pDown)
	{
		//see if the point is in the rectangle that is left of the arrows
		int nLeft	= (int)LTMIN(m_pUp->GetPos().x, m_pDown->GetPos().x);

		if(xPos >= nLeft)
		{
			//we are in the scrollbar gutter, don't allow the user to select a list item
			*pnIndex=kNoSelection;
			return NULL;
		}
	}

	//no items
	if (m_nLastShown >= m_controlArray.size()) return NULL;

	// See if the user clicked on any of the controls.
	uint32 i;
	for (i=m_nFirstShown; i <= m_nLastShown; i++)
	{
		// Check to see if the click is in the bounding box for the control
		if (m_controlArray[i]->IsOnMe(xPos,yPos))
		{
			*pnIndex=i;

			return m_controlArray[i];
		}
	}

    return NULL;
}

uint32 CLTGUIListCtrl::NextSelection()
{
	uint32 select = m_nCurrentIndex;
	if (select == kNoSelection)
		select = m_controlArray.size();
	uint32 oldSelect = select;
	
	CLTGUICtrl* pCtrl = NULL;	
	do
	{
		select++;
		if (select >= m_controlArray.size())
		{
			if (m_bScrollWrap)
				select = 0;
			else
			{
				select = m_controlArray.size() - 1;
				oldSelect = select;
			}

		}
	
		pCtrl = GetControl(select);	

	} while (select != oldSelect && pCtrl && !pCtrl->IsEnabled() );


	if (!pCtrl || !pCtrl->IsEnabled() )
		select = m_nCurrentIndex;

	return SetSelection(select);

}

uint32 CLTGUIListCtrl::PreviousSelection()
{
	uint32 select = m_nCurrentIndex;
	if (select == kNoSelection)
		select = 0;
	uint32 oldSelect = select;
	
	CLTGUICtrl* pCtrl = NULL;	
	do
	{
		if (select == 0)
		{
			if (m_bScrollWrap)
				select = m_controlArray.size()-1;
			else
			{
				select = 0;
				oldSelect = select;
			}
		}
		else
			select--;
	
		pCtrl = GetControl(select);	

	} while (select != oldSelect && pCtrl && !pCtrl->IsEnabled() );


	if (!pCtrl || !pCtrl->IsEnabled() )
		select = m_nCurrentIndex;

	return SetSelection(select);

}

void CLTGUIListCtrl::SetBasePos(const LTVector2n& pos)
{ 
	CLTGUICtrl::SetBasePos(pos);
	if (m_TexFrame.IsCreated())
	{
		m_TexFrame.SetBasePos(pos);
	}

	m_bNeedsRecalculation	= true;

	if (m_pUp)
	{
		LTVector2n pos = m_rnBaseRect.m_vMin;
		pos.x += m_nArrowOffset;
		m_pUp->SetBasePos(pos);
	}

	if (m_pDown)
	{
		LTVector2n pos = m_rnBaseRect.m_vMin;
		pos.x += m_nArrowOffset;
		pos.y = m_rnBaseRect.Bottom() - m_pDown->GetBaseHeight();
		m_pDown->SetBasePos(pos);
	}
}

void CLTGUIListCtrl::SetSize(const LTVector2n& sz)
{ 
	CLTGUICtrl::SetSize(sz);
	if (m_TexFrame.IsCreated())
	{
		m_TexFrame.SetSize(sz);
	}
	m_bNeedsRecalculation	= true;

	if (m_pUp)
	{
		LTVector2n pos = m_rnBaseRect.m_vMin;
		pos.x += m_nArrowOffset;
		m_pUp->SetBasePos(pos);
	}

	if (m_pDown)
	{
		LTVector2n pos = m_rnBaseRect.m_vMin;
		pos.x += m_nArrowOffset;
		pos.y = m_rnBaseRect.Bottom() - m_pDown->GetBaseHeight();
		m_pDown->SetBasePos(pos);
	}
}


void CLTGUIListCtrl::SetScale(const LTVector2& vfScale)
{
	CLTGUICtrl::SetScale(vfScale);
	if (m_TexFrame.IsCreated())
	{
		m_TexFrame.SetScale(m_vfScale);
	}

	for (uint32 i = 0; i < m_controlArray.size(); i++ )
	{
		m_controlArray[i]->SetScale(m_vfScale);
	}
	if (m_pUp)
	{
		m_pUp->SetScale(m_vfScale);
	}
	if (m_pDown)
	{
		m_pDown->SetScale(m_vfScale);
	}
	m_bNeedsRecalculation = true;
}

//when the list is selected/deselected
void CLTGUIListCtrl::OnSelChange()
{
	if (IsSelected())
	{
		if( m_bAutoSelect )
			SetSelection(0);

		if (m_hFrame[1])
		{
			m_TexFrame.SetFrame(m_hFrame[1]);
		}

	}
	else
	{
		if (m_hFrame[0])
		{
			m_TexFrame.SetFrame(m_hFrame[0]);
		}

		if (m_pUp)
			m_pUp->Select(false);
		if (m_pDown)
			m_pDown->Select(false);
		if( m_bAutoSelect )
			ClearSelection();
	}
}


// Sets the width of the list's frame, set to 0 to not show the frame
void CLTGUIListCtrl::SetFrameWidth(uint8 nFrameWidth)
{
	m_nFrameWidth = nFrameWidth;
	m_bNeedsRecalculation = true;
}

void CLTGUIListCtrl::SetFrame(HTEXTURE hNormalFrame,HTEXTURE hSelectedFrame, uint8 nExpand)
{
	m_hFrame[0] = hNormalFrame;
	m_hFrame[1] = hSelectedFrame;

	if (m_TexFrame.IsCreated())
	{
		m_TexFrame.SetFrame(hNormalFrame);
		LTVector2n vPos = m_rnBaseRect.m_vMin;
		LTVector2n vSz = m_rnBaseRect.m_vMax - m_rnBaseRect.m_vMin;
		vPos -= nExpand;
		vSz += 2 * nExpand;
		m_TexFrame.SetBasePos(vPos);
		m_TexFrame.SetSize(vSz);
	}
	else
	{
		CLTGUICtrl_create cs;
		cs.rnBaseRect = m_rnBaseRect;
		cs.rnBaseRect.Left() -= nExpand;
		cs.rnBaseRect.Top() -= nExpand;
		cs.rnBaseRect.Right() += nExpand;
		cs.rnBaseRect.Bottom() += nExpand;
		m_TexFrame.Create(hNormalFrame,cs);
	}

	if (IsSelected() && hSelectedFrame)
	{
		m_TexFrame.SetFrame(hSelectedFrame);
	}
}



void CLTGUIListCtrl::SetRenderState()
{
	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_NoBlend);
}


void CLTGUIListCtrl::Enable ( bool bEnabled )
{ 
	CLTGUICtrl::Enable (bEnabled);
	for (uint32 i = 0; i < m_controlArray.size(); i++ )
	{
		m_controlArray[i]->Enable (bEnabled);
	}
	if (m_pUp)
	{
		m_pUp->Enable (bEnabled);
	}
	if (m_pDown)
	{
		m_pDown->Enable (bEnabled);
	}

}


// Update data
void CLTGUIListCtrl::UpdateData (bool bSaveAndValidate)
{
	for (uint32 i = 0; i < m_controlArray.size(); i++ )
	{
		m_controlArray[i]->UpdateData(bSaveAndValidate);
	}

}

const char* CLTGUIListCtrl::GetHelpID()
{ 
	if (m_szHelpID && m_szHelpID[0])
		return m_szHelpID; 
	
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		return pCtrl->GetHelpID();

	return "";
}

void CLTGUIListCtrl::FlushTextureStrings()
{
	for (uint32 i = 0; i < m_controlArray.size(); i++)
	{
		m_controlArray[i]->FlushTextureStrings();
	}
}

void CLTGUIListCtrl::RecreateTextureStrings()
{
	for (uint32 i = 0; i < m_controlArray.size(); i++)
	{
		m_controlArray[i]->RecreateTextureStrings();
	}
}

void CLTGUIListCtrl::SelectionChanged()
{
	if (!m_pCommandHandler || !m_nCommandID)
		return;
	m_pCommandHandler->SendCommand(m_nCommandID, m_nCurrentIndex, 0);
}

//swap the positions of two items
void CLTGUIListCtrl::SwapItems(uint32 nIndex1,uint32 nIndex2)
{
	if (nIndex1 >= m_controlArray.size() || nIndex2 >= m_controlArray.size())
		return;

	CLTGUICtrl *pCtrl = m_controlArray[nIndex1];
	m_controlArray[nIndex1] = m_controlArray[nIndex2];
	m_controlArray[nIndex2] = pCtrl;

	if (nIndex1 == m_nCurrentIndex || nIndex1 == m_nCurrentIndex)
	{
		uint32 nOldSel = m_nCurrentIndex;
		m_nCurrentIndex = kNoSelection;
		SetSelection(nOldSel);
	}
	else
	{
		m_bNeedsRecalculation = true;
	}
}