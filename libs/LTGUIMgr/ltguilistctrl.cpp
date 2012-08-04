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
#include "vkdefs.h"

const uint16 CLTGUIListCtrl::kMaxNumControls = 0xFFFE;
const uint16 CLTGUIListCtrl::kNoSelection = 0xFFFF;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUIListCtrl::CLTGUIListCtrl()
{
	m_nCurrentIndex			= 0;
	m_nFirstShown			= 0;
	m_nLastShown			= kNoSelection;
	m_nHeight				= 100;
	m_nItemSpacing			= 0;
	m_nLBDownSel			= kNoSelection;
	m_nRBDownSel			= kNoSelection;
	m_bNeedsRecalculation	= LTTRUE;
	m_bScrollWrap			= LTTRUE;
	m_bScrollByPage			= LTFALSE;
	m_nFrameWidth			= 0;

	m_pUp = LTNULL;
	m_pDown = LTNULL;
	m_nArrowOffset = 0;

	m_indent.x = 0;
	m_indent.y = 0;

	memset(m_Frame,0,sizeof(m_Frame));

}

CLTGUIListCtrl::~CLTGUIListCtrl()
{
	Destroy();
}

// Creation
LTBOOL CLTGUIListCtrl::Create (uint16 nHeight )
{

	m_nHeight=nHeight;
    m_bCreated=LTTRUE;

    return LTTRUE;
}

// Destroy the control
void CLTGUIListCtrl::Destroy ( )
{
	RemoveAll();
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
void CLTGUIListCtrl::Render ( )
{
	if (!IsVisible()) return;

	if (m_bNeedsRecalculation)
		CalculatePositions();

	if (m_nFrameWidth)
	{
		// set up the render state	
		SetRenderState();

		for (int f = 0;f < 4; ++f)
			g_pDrawPrim->SetRGBA(&m_Frame[f],GetCurrentColor());

		// draw our frames
		g_pDrawPrim->DrawPrim(m_Frame,4);
	}


	if (m_controlArray.size() > 0)
	{
		// Render the items unless they are off the control
		for (uint16 i = m_nFirstShown; i <= m_nLastShown; i++ )
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


// Returns the height
uint16	CLTGUIListCtrl::GetHeight ( )
{
	return (uint16) ((float)m_nHeight * m_fScale);
}

// Returns the width
uint16	CLTGUIListCtrl::GetWidth ( )
{
	if (m_bNeedsRecalculation)
		CalculatePositions();
	return m_nWidth;
}


// Handle a keypress
LTBOOL CLTGUIListCtrl::HandleKeyDown(int key, int rep)
{
	if (key == VK_PRIOR)
		return OnPageUp();
	if (key == VK_NEXT)
		return OnPageDown();

	if (CLTGUICtrl::HandleKeyDown(key, rep))
	{
        return LTTRUE;
	}
    LTBOOL handled = LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->HandleKeyDown(key,rep);
	return handled;

}

LTBOOL CLTGUIListCtrl::OnUp ( )
{
	if (m_bScrollByPage) return OnPageUp();
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->OnUp())
		return LTTRUE;

	uint32 sel = m_nCurrentIndex;
	return (sel != PreviousSelection() || m_bScrollWrap);
}

LTBOOL CLTGUIListCtrl::OnDown ( )
{
	if (m_bScrollByPage) return OnPageDown();
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->OnDown())
		return LTTRUE;

	uint32 sel = m_nCurrentIndex;
	return (sel != NextSelection() || m_bScrollWrap);
}


LTBOOL  CLTGUIListCtrl::OnLeft ( )
{
    LTBOOL handled = LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnLeft();
	return handled;
}

LTBOOL  CLTGUIListCtrl::OnRight ( )
{
    LTBOOL handled = LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnRight();
	return handled;
}

LTBOOL  CLTGUIListCtrl::OnEnter ( )
{
    LTBOOL handled = LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnEnter();
	return handled;
}

LTBOOL CLTGUIListCtrl::OnPageUp ( )
{
	if (m_nFirstShown == 0) return LTFALSE;
	uint16 nSel = m_nFirstShown;

	int nHeight = m_nHeight - (2 * m_indent.y);

	int h = (int) ( (float)m_controlArray[nSel]->GetHeight()/m_fScale );
	while (nHeight >= h && nSel > 0)
	{
		//get the control's height (but unscale it)
		nHeight -= h;
		nSel--;
		h = (int) ( (float)m_controlArray[nSel]->GetHeight()/m_fScale );

		if (nHeight >= h)
			nHeight -= m_nItemSpacing;

	}


	m_nFirstShown = nSel;
	SetSelection(nSel);
	m_bNeedsRecalculation = LTTRUE;

	return LTTRUE;
}

LTBOOL CLTGUIListCtrl::OnPageDown ( )
{
	if (m_controlArray.size() == 0) return LTFALSE;
	if (m_nLastShown >= (m_controlArray.size()-1)) return LTFALSE;

	SetSelection(m_nLastShown+1);
	m_nFirstShown = m_nCurrentIndex;
	m_bNeedsRecalculation = LTTRUE;

	return LTTRUE;
	
}

// Handles the left button down message
LTBOOL CLTGUIListCtrl::OnLButtonDown(int x, int y)
{
	// Get the control that the click was on
	uint16 nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{
		if (pCtrl == m_pUp)
		{
			m_nLBDownSel=kNoSelection;
			m_pUp->Select(LTTRUE);
			return LTTRUE;
		}
		if (pCtrl == m_pDown)
		{
			m_nLBDownSel=kNoSelection;
			m_pDown->Select(LTTRUE);
			return LTTRUE;
		}

		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
            return LTFALSE;

		// Select the control
		SetSelection(nControlIndex);

		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.
		m_nLBDownSel=nControlIndex;
		m_controlArray[nControlIndex]->OnLButtonDown(x,y);
        return LTTRUE;
	}
	else
	{
		// This clears the index for what item was selected from a mouse down message
		m_nLBDownSel=kNoSelection;

        return LTFALSE;
	}
}

// Handles the left button up message
LTBOOL CLTGUIListCtrl::OnLButtonUp(int x, int y)
{
	// Get the control that the click was on
	uint16 nControlIndex=0;
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
            return LTFALSE;

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
    return LTFALSE;
}

// Handles the right button down message
LTBOOL CLTGUIListCtrl::OnRButtonDown(int x, int y)
{
	// Get the control that the click was on
	uint16 nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{
		if (pCtrl == m_pUp)
		{
			return LTFALSE;
		}
		if (pCtrl == m_pDown)
		{
			return LTFALSE;
		}

		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
            return LTFALSE;

		// Select the control
		SetSelection(nControlIndex);

		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.
		m_nRBDownSel=nControlIndex;
		m_controlArray[nControlIndex]->OnRButtonDown(x,y);
        return LTTRUE;
	}
	else
	{
		// This clears the index for what item was selected from a mouse down message
		m_nLBDownSel=kNoSelection;

        return LTFALSE;
	}
}

// Handles the right button up message
LTBOOL CLTGUIListCtrl::OnRButtonUp(int x, int y)
{
	// Get the control that the click was on
	uint16 nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{
		if (pCtrl == m_pUp)
		{
			return LTFALSE;
		}
		if (pCtrl == m_pDown)
		{
			return LTFALSE;
		}

		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
            return LTFALSE;

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
    return LTFALSE;
}

// Handles the mouse move message
LTBOOL CLTGUIListCtrl::OnMouseMove(int x, int y)
{
	uint16 nControlUnderPoint=0;

	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlUnderPoint);

	if(pCtrl)
	{
		if(m_pUp && m_pDown)
		{
			if (pCtrl == m_pUp)
			{
				bool bRV = !m_pUp->IsSelected();
				m_pUp->Select(LTTRUE);
				m_pDown->Select(LTFALSE);
				return bRV;
			}
			else if (pCtrl == m_pDown)
			{
				bool bRV = !m_pDown->IsSelected();
				m_pUp->Select(LTFALSE);
				m_pDown->Select(LTTRUE);
				return bRV;
			}
			else
			{
				m_pUp->Select(LTFALSE);
				m_pDown->Select(LTFALSE);
			}
		}

		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
            return LTFALSE;

		pCtrl->OnMouseMove(x,y);

		if (GetSelectedIndex() != nControlUnderPoint)
		{
			SetSelection(nControlUnderPoint);

            return LTTRUE;
		}
	}

    return LTFALSE;
}

// Add a control the the array of controls
uint16 CLTGUIListCtrl::AddControl ( CLTGUICtrl *pControl )
{
	ASSERT(pControl);
	m_controlArray.push_back(pControl);

	if (!m_bEnabled)
		pControl->Enable(LTFALSE);

	if ( m_controlArray.size() > 1 && (m_controlArray.size()-1) == m_nCurrentIndex )
	{
		m_controlArray[m_nCurrentIndex]->Select(LTTRUE);
	}

	m_bNeedsRecalculation = LTTRUE;
	return m_controlArray.size()-1;
}

// Remove a control
void CLTGUIListCtrl::RemoveControl ( CLTGUICtrl *pControl, LTBOOL bDelete)
{
	ControlArray::iterator iter = m_controlArray.begin();
	while (iter != m_controlArray.end() && (*iter) != pControl)
		iter++;

	if (iter != m_controlArray.end())
	{
		if (bDelete)
			debug_delete(*iter);
		m_controlArray.erase(iter);
		m_bNeedsRecalculation = LTTRUE;
	}
}

// Remove a control
void CLTGUIListCtrl::RemoveControl ( uint16 nIndex, LTBOOL bDelete )
{
	if (nIndex >= m_controlArray.size())
		return;

	ControlArray::iterator iter = m_controlArray.begin() + nIndex;
	if (bDelete)
		debug_delete(*iter);
	m_controlArray.erase(iter);
	m_bNeedsRecalculation = LTTRUE;
}

// Removes all of the controls
void CLTGUIListCtrl::RemoveAll ( LTBOOL bDelete )
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
	m_bNeedsRecalculation = LTTRUE;
}

// Return a control at a specific index
CLTGUICtrl *CLTGUIListCtrl::GetControl ( uint16 nIndex )
{
	if (nIndex >= m_controlArray.size())
		return LTNULL;

	return m_controlArray[nIndex];
}

// Select a control
void CLTGUIListCtrl::ClearSelection()
{
	if (m_nCurrentIndex < m_controlArray.size() )
	{
		m_controlArray[m_nCurrentIndex]->Select(LTFALSE);
	}
	m_nCurrentIndex = kNoSelection;
}


// Select a control
uint16 CLTGUIListCtrl::SetSelection( uint16 nIndex )
{

	if (nIndex >= m_controlArray.size() && nIndex != kNoSelection)
		return m_nCurrentIndex;

	if (m_nCurrentIndex==nIndex)
	{
		if (m_nCurrentIndex < m_controlArray.size() )
		{
			m_controlArray[m_nCurrentIndex]->Select(LTTRUE);
		}
		return m_nCurrentIndex;
	}

	int nOldIndex=m_nCurrentIndex;

	if (m_nCurrentIndex < m_controlArray.size() )
	{
		m_controlArray[m_nCurrentIndex]->Select(LTFALSE);
	}


	m_nCurrentIndex=nIndex;

	if (nIndex == kNoSelection)
		return nIndex;

	m_controlArray[m_nCurrentIndex]->Select(LTTRUE);

	if (m_bNeedsRecalculation)
		CalculatePositions();


	// Figure out if we should move the up or down
	if ( m_nCurrentIndex < m_nFirstShown )
	{
		m_nFirstShown = m_nCurrentIndex;
		m_bNeedsRecalculation = LTTRUE;
	}
	else if ( m_nCurrentIndex > m_nLastShown )
	{
		int nTop = m_basePos.y + m_indent.y;
		int nBottom = (nTop + m_nHeight) - m_indent.y;

		int i = m_nCurrentIndex;
		do
		{
			//get the control's height (but unscale it)
			int h = (uint16) ( (float)m_controlArray[i]->GetHeight()/m_fScale );

			nBottom -= h;

			if (nBottom > nTop)
			{
				i--;
				nBottom -= m_nItemSpacing;
			}

			

		} while (nBottom > nTop);

		m_nFirstShown = (i+1);

		m_bNeedsRecalculation = LTTRUE;
	}

	return m_nCurrentIndex;

}

void CLTGUIListCtrl::CalculatePositions()
{
	int y  = 0;
	uint16 h  = 0;
	uint16 w  = 0;

	m_nWidth = 0;
	m_nLastShown = kNoSelection;

	uint16 nBaseWidth = 0;
	if (m_pUp)
	{
		nBaseWidth = (uint16)((float)m_nArrowOffset * m_fScale) + m_pUp->GetWidth();
		m_nWidth = nBaseWidth;
	}

	int nSpace = (int)(2.0f * m_fScale * (float)m_indent.x);

	//step through the controls
	for (uint16 i = m_nFirstShown; i < m_controlArray.size(); i++ )
	{
		if (!m_controlArray[i]->IsVisible()) continue;

		LTIntPt pos = m_basePos;
		pos.x += m_indent.x;
		pos.y += (y+m_indent.y);
		m_controlArray[i]->SetBasePos(pos);

		//check the control's width
		w = m_controlArray[i]->GetWidth() + nSpace;
		if (w > m_nWidth)
			m_nWidth = w;

		//get the control's height (but unscale it)
		h = (uint16) ( (float)m_controlArray[i]->GetHeight()/m_fScale );


		//if we haven't identified the last shown control
		//and the bottom of the control is beyond the height of the list
		// the previous control is the last one that fit
		if (m_nLastShown > m_controlArray.size() && (y+h) > (m_nHeight-m_indent.y))
		{
			m_nLastShown = i-1;
		}

		y += (h + m_nItemSpacing);

	}

	if (m_pUp && nBaseWidth < m_nWidth)
	{
		m_nArrowOffset = (uint16)((float)(m_nWidth - m_pUp->GetWidth()) / m_fScale);
		LTIntPt pos = m_basePos;
		pos.x += m_nArrowOffset;
		m_pUp->SetBasePos(pos);

		pos.y += (m_nHeight - m_pDown->GetBaseHeight() );
		m_pDown->SetBasePos(pos);
	
	}


	if (m_nLastShown > m_controlArray.size())
	{
		m_nLastShown = m_controlArray.size()-1;
	}

	float frameW = ((float)m_nFrameWidth * m_fScale);

	//top
	float fx = (float)m_pos.x - frameW;
	float fy = (float)m_pos.y - frameW;
	float fw = (float)m_nWidth + frameW;
	float fh = frameW;
	g_pDrawPrim->SetXYWH(&m_Frame[0],fx,fy,fw,fh);

	//right
	fx = (float)(m_pos.x + m_nWidth);
	fy = (float)m_pos.y - frameW;
	fw = (float)frameW;
	fh = (float)GetHeight() + frameW;
	g_pDrawPrim->SetXYWH(&m_Frame[1],fx,fy,fw,fh);

	//bottom
	fx = (float)m_pos.x;
	fy = (float)(m_pos.y + GetHeight());
	fw = (float)m_nWidth + frameW;
	fh = frameW;
	g_pDrawPrim->SetXYWH(&m_Frame[2],fx,fy,fw,fh);

	//left
	fx = (float)m_pos.x - frameW;
	fy = (float)m_pos.y;
	fw = (float)frameW;
	fh = (float)GetHeight() + frameW;
	g_pDrawPrim->SetXYWH(&m_Frame[3],fx,fy,fw,fh);

	m_bNeedsRecalculation = LTFALSE;
}

// Gets the index of the control that is under the specific screen point.
// Returns FALSE if there isn't one under the specified point.
CLTGUICtrl *CLTGUIListCtrl::GetControlUnderPoint(int xPos, int yPos, uint16 *pnIndex)
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
		int nLeft	= LTMIN(m_pUp->GetPos().x, m_pDown->GetPos().x);

		if(xPos >= nLeft)
		{
			//we are in the scrollbar gutter, don't allow the user to select a list item
			*pnIndex=kNoSelection;
			return NULL;
		}
	}

	//no items
	if (m_nLastShown >= m_controlArray.size()) return LTNULL;

	// See if the user clicked on any of the controls.
	int i;
	for (i=m_nFirstShown; i <= m_nLastShown; i++)
	{
		// Check to see if the click is in the bounding box for the control
		if (m_controlArray[i]->IsOnMe(xPos,yPos))
		{
			*pnIndex=i;

			return m_controlArray[i];
		}
	}

    return LTNULL;
}

uint16 CLTGUIListCtrl::NextSelection()
{
	uint16 select = m_nCurrentIndex;
	if (select == kNoSelection)
		select = m_controlArray.size();
	uint16 oldSelect = select;
	
	CLTGUICtrl* pCtrl = LTNULL;	
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

uint16 CLTGUIListCtrl::PreviousSelection()
{
	uint16 select = m_nCurrentIndex;
	if (select == kNoSelection)
		select = 0;
	uint16 oldSelect = select;
	
	CLTGUICtrl* pCtrl = LTNULL;	
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

void CLTGUIListCtrl::SetBasePos ( LTIntPt pos )
{ 
	CLTGUICtrl::SetBasePos(pos);
	m_bNeedsRecalculation	= LTTRUE;

	if (m_pUp)
	{
		LTIntPt pos = m_basePos;
		pos.x += m_nArrowOffset;
		m_pUp->SetBasePos(pos);
	}

	if (m_pDown)
	{
		LTIntPt pos = m_basePos;
		pos.x += m_nArrowOffset;
		pos.y += (m_nHeight - m_pDown->GetBaseHeight() );
		m_pDown->SetBasePos(pos);
	}
}


void CLTGUIListCtrl::SetScale(float fScale)
{
	m_pos.x = (int)(fScale * (float)m_basePos.x);
	m_pos.y = (int)(fScale * (float)m_basePos.y);
	m_fScale = fScale;
	for (uint16 i = 0; i < m_controlArray.size(); i++ )
	{
		m_controlArray[i]->SetScale(m_fScale);
	}
	if (m_pUp)
	{
		m_pUp->SetScale(m_fScale);
	}
	if (m_pDown)
	{
		m_pDown->SetScale(m_fScale);
	}
	m_bNeedsRecalculation = LTTRUE;
}

//when the list is selected/deselected
void CLTGUIListCtrl::OnSelChange()
{
	if (IsSelected())
		SetSelection(0);
	else
	{
		if (m_pUp)
			m_pUp->Select(LTFALSE);
		if (m_pDown)
			m_pDown->Select(LTFALSE);
		ClearSelection();
	}
}


LTBOOL	CLTGUIListCtrl::UseArrows(uint16 xOffset, LTFLOAT fTextureScale, HTEXTURE hUpNormal,  
								  HTEXTURE hUpSelected, HTEXTURE hDownNormal,  HTEXTURE hDownSelected)
{
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

	m_nArrowOffset = xOffset;

	m_pUp = debug_new(CLTGUIButton);
	m_pUp->Create(LTNULL,LTNULL,hUpNormal,hUpSelected);
	m_pUp->SetTextureScale(fTextureScale);
	LTIntPt pos = m_basePos;
	pos.x += m_nArrowOffset;
	m_pUp->SetBasePos(pos);
	m_pUp->SetScale(m_fScale);

	m_pDown = debug_new(CLTGUIButton);
	m_pDown->Create(LTNULL,LTNULL,hDownNormal,hDownSelected);
	m_pDown->SetTextureScale(fTextureScale);
	m_pDown->SetScale(m_fScale);
	pos.y += (m_nHeight - m_pDown->GetBaseHeight() );
	m_pDown->SetBasePos(pos);
	
	

	return LTTRUE;
}


// Sets the width of the list's frame, set to 0 to not show the frame
void CLTGUIListCtrl::SetFrameWidth(uint8 nFrameWidth)
{
	m_nFrameWidth = nFrameWidth;
	if (nFrameWidth)
	{
		m_bNeedsRecalculation = LTTRUE;
	}
}



void CLTGUIListCtrl::SetRenderState()
{
	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_NOCOLOROP);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_NOBLEND);
		
}


void CLTGUIListCtrl::Enable ( LTBOOL bEnabled )
{ 
	CLTGUICtrl::Enable (bEnabled);
	for (uint16 i = 0; i < m_controlArray.size(); i++ )
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
void CLTGUIListCtrl::UpdateData (LTBOOL bSaveAndValidate)
{
	for (uint16 i = 0; i < m_controlArray.size(); i++ )
	{
		m_controlArray[i]->UpdateData(bSaveAndValidate);
	}

}

uint32 CLTGUIListCtrl::GetHelpID()
{ 
	if (m_nHelpID)
		return m_nHelpID; 
	
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		return pCtrl->GetHelpID();

	return 0;
}
