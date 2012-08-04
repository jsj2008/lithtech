// ----------------------------------------------------------------------- //
//
// MODULE  : ltguilistctrlex.h
//
// PURPOSE : Defines the CLTGUIListCtrlEx class.  This class creates a
//           list control that can use a CLTGUIHeaderCtrl and
//           CLTGUIScrollBar.
//
// CREATED : 06/30/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "ltguimgr.h"
#include "ltguilistctrl.h"
#if !defined(PLATFORM_XENON)
#include "vkdefs.h"
#endif // !PLATFORM_XENON

#define WHEEL_DELTA      120      // Default value for rolling one notch

const uint32 CLTGUIListCtrlEx::kNoSelection = 0xFFFFFFFF;
const uint32 CLTGUIListCtrlEx::kMaxNumControls = 0xFFFFFFFE;

CLTGUIListCtrlEx::CLTGUIListCtrlEx() :
	m_pScrollBar(NULL),
	m_pHeaderCtrl(NULL),
	m_nCurrentIndex(0),
	m_nFirstShown(0),
	m_nLastShown(kNoSelection),
	m_nItemSpacing(0),
	m_nLBDownSel(kNoSelection),
	m_nRBDownSel(kNoSelection),
	m_bNeedsRecalculation(true),
	m_bScrollWrap(true),
	m_bScrollByPage(false),
	m_nFrameWidth(0),
	m_indent(0, 0),
	m_bAutoSelect(true),
	m_iScrollOffset(0),
	m_iMaxScrollLine(0),
	m_nVisibleLineCount(0),
	m_nSelectedColumn(-1),
	m_nExpand(0),
	m_nBackgroundColumnColor(0),
	m_nSelectedColumnColor(0),
	m_nHighlightColor(0),
	m_nTextIdent(0)
{
}

CLTGUIListCtrlEx::~CLTGUIListCtrlEx()
{
	Destroy();
}

// create the control
bool CLTGUIListCtrlEx::Create(const CLTGUIListCtrlEx_create& cs)
{
	m_pScrollBar = cs.pScrollBar;
	if( m_pScrollBar )
	{
		m_pScrollBar->SetMessageControl( this );
		UpdateScrollBar();
	}

	m_pHeaderCtrl = cs.pHeaderCtrl;
	if( m_pHeaderCtrl )
	{
		m_pHeaderCtrl->SetMessageControl( this );
	}

	m_nBackgroundColumnColor = cs.nBackgroundColumnColor;
	m_nSelectedColumnColor = cs.nSelectedColumnColor;
	m_nHighlightColor = cs.nHighlightColor;
	m_bAutoSelect = cs.bAutoSelect;
	m_nTextIdent = cs.nTextIdent;

	CLTGUICtrl::Create( (CLTGUICtrl_create)cs );

	return true;
}

// destroys the control
void CLTGUIListCtrlEx::Destroy ( )
{
	if (m_bCreated && m_TexFrame.IsCreated())
	{
		m_TexFrame.Destroy();
	}

	RemoveAll();

	m_bCreated = false;
}

// render the control
void CLTGUIListCtrlEx::Render()
{
	if( !IsVisible() )  
		return;

	if( m_bNeedsRecalculation )
		RecalcLayout();

	//if( m_TexFrame.IsCreated() )
	//{
	//	m_TexFrame.Render();
	//}

	// render the selected column
	if( m_nSelectedColumn != -1 && m_pHeaderCtrl )
	{
		LTRect2n rcColumn = m_pHeaderCtrl->GetItemRect( m_nSelectedColumn );
		LT_POLYG4 polySelect;
		DrawPrimSetRGBA( polySelect, m_nSelectedColumnColor );
		DrawPrimSetXYWH( polySelect, (float)rcColumn.Left() + 1, (float)(GetPos().y - m_nExpand), (float)rcColumn.GetWidth() - 1, GetHeight() + m_nExpand + m_nExpand );
		g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
		g_pDrawPrim->DrawPrim( &polySelect );

		LT_POLYG4 polyBackground;
		DrawPrimSetRGBA( polyBackground, m_nBackgroundColumnColor );

		if( m_nSelectedColumn != 0 )
		{
			DrawPrimSetXYWH( polyBackground, GetPos().x - m_nExpand, GetPos().y - m_nExpand, (float)rcColumn.Left() + 1 - (GetPos().x - m_nExpand), GetHeight() + m_nExpand + m_nExpand );
			g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
			g_pDrawPrim->DrawPrim( &polyBackground );
		}
		if( m_nSelectedColumn + 1 != m_pHeaderCtrl->GetItemCount() )
		{
			DrawPrimSetXYWH( polyBackground, (float)rcColumn.Right(), GetPos().y - m_nExpand, GetWidth() + m_nExpand + m_nExpand - (rcColumn.Right() + 1 - (GetPos().x - m_nExpand)), GetHeight() + m_nExpand + m_nExpand );
			g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
			g_pDrawPrim->DrawPrim( &polyBackground );
		}
	}
	else
	{
		LT_POLYG4 polyBackground;
		DrawPrimSetRGBA( polyBackground, m_nBackgroundColumnColor );
		DrawPrimSetXYWH( polyBackground, GetPos().x - m_nExpand, GetPos().y - m_nExpand, GetWidth() + m_nExpand + m_nExpand, GetHeight() + m_nExpand + m_nExpand );
		g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
		g_pDrawPrim->DrawPrim( &polyBackground );
	}

	// highlight the selected row
	if( m_nCurrentIndex != kNoSelection && 
		m_nCurrentIndex < m_controlArray.size() && 
		m_nCurrentIndex >= m_nFirstShown && 
		m_nCurrentIndex <= m_nLastShown &&
		m_controlArray[m_nCurrentIndex]->IsVisible() )
	{
		LTRect2f rcLine = m_controlArray[m_nCurrentIndex]->GetRect();

		LT_POLYG4 polyBackground;
		DrawPrimSetRGBA( polyBackground, m_nHighlightColor );
		DrawPrimSetXYWH( polyBackground, rcLine.Left() + 1, rcLine.Top(), rcLine.GetWidth() - 2, rcLine.GetHeight() );
		g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
		g_pDrawPrim->DrawPrim( &polyBackground );
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
	}

	if( m_nFrameWidth )
	{
		// set up the render state	
		SetRenderState();

		for (int f = 0;f < 4; ++f)
			DrawPrimSetRGBA(m_Frame[f],GetCurrentColor());

		// draw our frames
		if( m_pScrollBar )
			g_pDrawPrim->DrawPrim(m_Frame,m_pScrollBar->IsVisible()?3:4);
		else
			g_pDrawPrim->DrawPrim(m_Frame,4);
	}
}

// Render the control
void CLTGUIListCtrlEx::RenderTransition(float fTrans)
{
	if (!IsVisible()) return;

	if (m_bNeedsRecalculation)
		RecalcLayout();

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
		g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);

		// Render the items unless they are off the control
		for (uint32 i = m_nFirstShown; i <= m_nLastShown && i < m_controlArray.size( ); i++ )
		{
			if (m_controlArray[i]->IsVisible())
				m_controlArray[i]->RenderTransition(fTrans);
		}
	}
}

// Handle a keypress
bool CLTGUIListCtrlEx::HandleKeyDown(int key, int rep)
{
	if( key == VK_PRIOR )
		return OnPageUp();
	if( key == VK_NEXT )
		return OnPageDown();

	if( CLTGUICtrl::HandleKeyDown(key, rep) )
		return true;

	bool handled = false;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if( pCtrl )
		handled = pCtrl->HandleKeyDown( key, rep );
	
	return handled;
}

bool CLTGUIListCtrlEx::OnUp()
{
	if( m_bScrollByPage ) 
		return OnPageUp();

	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->OnUp())
		return true;

	uint32 sel = m_nCurrentIndex;
	return (sel != PreviousSelection() || m_bScrollWrap);
}

bool CLTGUIListCtrlEx::OnDown()
{
	if( m_bScrollByPage ) 
		return OnPageDown();

	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->OnDown())
		return true;

	uint32 sel = m_nCurrentIndex;
	return (sel != NextSelection() || m_bScrollWrap);
}

bool CLTGUIListCtrlEx::OnLeft ( )
{
	bool handled = false;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnLeft();
	return handled;
}

bool CLTGUIListCtrlEx::OnRight ( )
{
	bool handled = false;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnRight();
	return handled;
}

bool CLTGUIListCtrlEx::OnEnter()
{
	bool handled = false;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnEnter();
	return handled;
}

bool CLTGUIListCtrlEx::OnPageUp ( )
{
	if( m_nFirstShown == 0 ) 
		return false;

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

bool CLTGUIListCtrlEx::OnPageDown()
{
	if (m_controlArray.size() == 0) 
		return false;

	if (m_nLastShown >= (m_controlArray.size()-1)) 
		return false;

	uint32 nSel = m_nLastShown;

	int nHeight = m_rnBaseRect.GetHeight() - (2 * m_indent.y);

	int h = m_controlArray[nSel]->GetBaseHeight();
	while (nHeight >= h && nSel < (m_controlArray.size()-1))
	{
		//get the control's height (but unscale it)
		nHeight -= h;
		nSel++;
		h = m_controlArray[nSel]->GetBaseHeight();

		if (nHeight >= h)
			nHeight -= m_nItemSpacing;
	}

	SetSelection(nSel);
	m_bNeedsRecalculation = true;

	return true;
}

// Handles the left button down message
bool CLTGUIListCtrlEx::OnLButtonDown(int x, int y)
{
	// Get the control that the click was on
	uint32 nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{
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
bool CLTGUIListCtrlEx::OnLButtonUp(int x, int y)
{
	// Get the control that the click was on
	uint32 nControlIndex=0;
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
bool CLTGUIListCtrlEx::OnRButtonDown(int x, int y)
{
	// Get the control that the click was on
	uint32 nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{
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
bool CLTGUIListCtrlEx::OnRButtonUp(int x, int y)
{
	// Get the control that the click was on
	uint32 nControlIndex=0;
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
bool CLTGUIListCtrlEx::OnMouseMove(int x, int y)
{
	uint32 nControlUnderPoint=0;

	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlUnderPoint);

	if(pCtrl)
	{
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

bool CLTGUIListCtrlEx::OnMouseWheel(int x, int y, int zDelta)
{
	m_iScrollOffset += (zDelta/WHEEL_DELTA);
	m_iScrollOffset = LTMIN( 0, LTMAX( -m_iMaxScrollLine, m_iScrollOffset ));
	UpdateScrollBar();

	m_bNeedsRecalculation = true;

	return false;
}

// Add a control the the array of controls
uint32 CLTGUIListCtrlEx::AddControl ( CLTGUICtrl *pControl )
{
	ASSERT(pControl);
	m_controlArray.push_back(pControl);

	CLTGUIColumnCtrlEx* pColumnCtrl = dynamic_cast<CLTGUIColumnCtrlEx*>(pControl);
	if( pColumnCtrl )
	{
		pColumnCtrl->SetTextIndent( m_nTextIdent );

		if( m_pHeaderCtrl )
		{
			uint32 nHeaderCount = m_pHeaderCtrl->GetItemCount();
			for(uint32 nHeader=0;nHeader<nHeaderCount;++nHeader)
			{
				uint32 nWidth = m_pHeaderCtrl->GetItemBaseWidth(nHeader);
				pColumnCtrl->SetColumnWidth( (uint8)nHeader, nWidth );
			}
		}
		else if( pColumnCtrl->GetNumColumns() == 1 )
		{
			pColumnCtrl->SetColumnWidth( 0, pColumnCtrl->GetBaseWidth() );
		}

		pColumnCtrl->ResetColumns();
	}
	else
	{
		uint32 nWidth = pControl->GetBaseWidth();
	//	pControl->SetBaseWidth( nWidth - m_nTextIdent*2 );

		LTVector2n vPos = pControl->GetBasePos();
		vPos.x += m_nTextIdent;
	//	pControl->SetBasePos( vPos );
	}

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
bool CLTGUIListCtrlEx::InsertControl( uint32 nIndex, CLTGUICtrl* pControl )
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
void CLTGUIListCtrlEx::RemoveControl ( CLTGUICtrl *pControl, bool bDelete)
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
void CLTGUIListCtrlEx::RemoveControl ( uint32 nIndex, bool bDelete )
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
void CLTGUIListCtrlEx::RemoveAll ( bool bDelete )
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
CLTGUICtrl *CLTGUIListCtrlEx::GetControl ( uint32 nIndex ) const
{
	if (nIndex >= m_controlArray.size())
		return NULL;

	return m_controlArray[nIndex];
}

// Return index of a specific control
uint32 CLTGUIListCtrlEx::GetIndex(CLTGUICtrl *pCtrl) const
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
void CLTGUIListCtrlEx::ClearSelection()
{
	if (m_nCurrentIndex < m_controlArray.size() )
	{
		m_controlArray[m_nCurrentIndex]->Select(false);
	}
	m_nCurrentIndex = kNoSelection;
	SelectionChanged();

}


// Select a control
uint32 CLTGUIListCtrlEx::SetSelection( uint32 nIndex )
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
		RecalcLayout();


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

	int32 nFirstShownOfVisible = 0;
	for(uint32 nControl=0;nControl<m_nFirstShown;++nControl)
	{
		if( !m_controlArray[nControl]->IsVisible() ) continue;
		++nFirstShownOfVisible;
	}

	m_iScrollOffset = -nFirstShownOfVisible;
	UpdateScrollBar();

	SelectionChanged();
	return m_nCurrentIndex;

}

void CLTGUIListCtrlEx::RecalcLayout()
{
	int32 y  = 0;
	uint32 h  = 0;
	uint32 w  = 0;

	m_nLastShown = kNoSelection;

	uint32 nBaseWidth = 0;

	uint32 nSpace = (uint32)m_indent.x;

	m_nFirstShown = -m_iScrollOffset;
	int32 nScrollItem = m_iScrollOffset;
	for (uint32 i = 0; i < m_controlArray.size(); i++ )
	{
		if (!m_controlArray[i]->IsVisible()) continue;
		if( nScrollItem++ == 0 )
		{
			m_nFirstShown = i;
			break;
		}
	}

	//step through the controls
	for (uint32 i = m_nFirstShown; i < m_controlArray.size(); i++ )
	{
		if (!m_controlArray[i]->IsVisible()) continue;

		LTVector2n pos = m_rnBaseRect.m_vMin;
		pos.x -= m_nExpand;
		pos.y += (y+m_indent.y);

		CLTGUIColumnCtrlEx* pColumnCtrl = dynamic_cast<CLTGUIColumnCtrlEx*>(m_controlArray[i]);
		if( pColumnCtrl )
		{
			pColumnCtrl->SetTextIndent( m_nTextIdent );
			m_controlArray[i]->SetBasePos(pos);
		}
		else
		{
			//m_controlArray[i]->SetBaseWidth( m_rnBaseRect.GetWidth() - m_nTextIdent*8 );
			m_controlArray[i]->SetBasePos(pos + LTVector2n(m_nTextIdent, 0));
			//m_controlArray[i]->SetBasePos(pos);
		}

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
		//SetSize(sz);
	}

	if (m_nLastShown > m_controlArray.size())
	{
		m_nLastShown = m_controlArray.size()-1;
	}

	float frameW = ((float)m_nFrameWidth/* * m_vfScale.x*/);
	float frameH = ((float)m_nFrameWidth/* * m_vfScale.y*/);

	//top
	float fx = m_rfRect.Left();
	float fy = m_rfRect.Top();
	float fw = m_rfRect.GetWidth();
	float fh = frameH;
	DrawPrimSetXYWH(m_Frame[0],fx,fy,fw,fh);

	//bottom
	fx = m_rfRect.Left();
	fy = m_rfRect.Bottom() - frameH;
	fw = m_rfRect.GetWidth();
	fh = frameH;
	DrawPrimSetXYWH(m_Frame[1],fx,fy,fw,fh);

	//left
	fx = m_rfRect.Left();
	fy = m_rfRect.Top();
	fw = frameW;
	fh = m_rfRect.GetHeight();
	DrawPrimSetXYWH(m_Frame[2],fx,fy,fw,fh);

	//right
	fx = m_rfRect.Right() - frameW;
	fy = m_rfRect.Top();
	fw = frameW;
	fh = m_rfRect.GetHeight();
	DrawPrimSetXYWH(m_Frame[3],fx,fy,fw,fh);

	UpdateScrollBar();

	m_bNeedsRecalculation = false;
}

// Gets the index of the control that is under the specific screen point.
// Returns FALSE if there isn't one under the specified point.
CLTGUICtrl *CLTGUIListCtrlEx::GetControlUnderPoint(int xPos, int yPos, uint32 *pnIndex)
{
	ASSERT(pnIndex);

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

uint32 CLTGUIListCtrlEx::NextSelection()
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

uint32 CLTGUIListCtrlEx::PreviousSelection()
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

void CLTGUIListCtrlEx::SetBasePos(const LTVector2n& pos)
{ 
	CLTGUICtrl::SetBasePos(pos);
	if (m_TexFrame.IsCreated())
	{
		m_TexFrame.SetBasePos(pos);
	}

	m_bNeedsRecalculation	= true;
}

void CLTGUIListCtrlEx::SetSize(const LTVector2n& sz)
{ 
	CLTGUICtrl::SetSize(sz);
	if (m_TexFrame.IsCreated())
	{
		m_TexFrame.SetSize(sz);
	}
	m_bNeedsRecalculation	= true;
}


void CLTGUIListCtrlEx::SetScale(const LTVector2& vfScale)
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
	m_bNeedsRecalculation = true;
}

//when the list is selected/deselected
void CLTGUIListCtrlEx::OnSelChange()
{
	if (IsSelected())
	{
		//if( m_bAutoSelect )
		//	SetSelection(0);

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

		if( m_bAutoSelect )
			ClearSelection();
	}
}


// Sets the width of the list's frame, set to 0 to not show the frame
void CLTGUIListCtrlEx::SetFrameWidth(uint8 nFrameWidth)
{
	m_nFrameWidth = nFrameWidth;
	m_bNeedsRecalculation = true;
}

void CLTGUIListCtrlEx::SetFrame(HTEXTURE hNormalFrame,HTEXTURE hSelectedFrame, uint8 nExpand)
{
	m_hFrame[0] = hNormalFrame;
	m_hFrame[1] = hSelectedFrame;

	m_nExpand = nExpand;

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



void CLTGUIListCtrlEx::SetRenderState()
{
	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_NoBlend);
}


void CLTGUIListCtrlEx::Enable ( bool bEnabled )
{ 
	CLTGUICtrl::Enable (bEnabled);
	for (uint32 i = 0; i < m_controlArray.size(); i++ )
	{
		m_controlArray[i]->Enable (bEnabled);
	}
}


// Update data
void CLTGUIListCtrlEx::UpdateData (bool bSaveAndValidate)
{
	for (uint32 i = 0; i < m_controlArray.size(); i++ )
	{
		m_controlArray[i]->UpdateData(bSaveAndValidate);
	}

}

const char* CLTGUIListCtrlEx::GetHelpID()
{ 
	if (m_szHelpID && m_szHelpID[0])
		return m_szHelpID; 

	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		return pCtrl->GetHelpID();

	return "";
}

void CLTGUIListCtrlEx::FlushTextureStrings()
{
	for (uint32 i = 0; i < m_controlArray.size(); i++)
	{
		m_controlArray[i]->FlushTextureStrings();
	}
}

void CLTGUIListCtrlEx::RecreateTextureStrings()
{
	for (uint32 i = 0; i < m_controlArray.size(); i++)
	{
		m_controlArray[i]->RecreateTextureStrings();
	}
}

void CLTGUIListCtrlEx::SelectionChanged()
{
	if (!m_pCommandHandler || !m_nCommandID)
		return;
	m_pCommandHandler->SendCommand(m_nCommandID, m_nCurrentIndex, 0);
}

//swap the positions of two items
void CLTGUIListCtrlEx::SwapItems(uint32 nIndex1,uint32 nIndex2)
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

// called when a command is received
uint32 CLTGUIListCtrlEx::OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2)
{
	switch( nCommand )
	{
	case eScrollBarCmd_Top:
	case eScrollBarCmd_Bottom:
	case eScrollBarCmd_LineUp:
	case eScrollBarCmd_LineDown:
	case eScrollBarCmd_ThumbTrack:
	case eScrollBarCmd_ThumbPosition:
	case eScrollBarCmd_PageUp:
	case eScrollBarCmd_PageDown:
		return OnScrollBarCommand( nCommand, nParam1, nParam2 );

	case eHeaderCtrlCmd_Size:
		{
			uint32 nItemCount = (uint32)m_controlArray.size();
			for(uint32 nItem=0;nItem<nItemCount;++nItem)
			{
				CLTGUIColumnCtrlEx* pCtrl = dynamic_cast<CLTGUIColumnCtrlEx*>(m_controlArray[nItem]);
				if( !pCtrl )
					continue;

				uint32 nHeaderCount = m_pHeaderCtrl->GetItemCount();
				for(uint32 nHeader=nParam1;nHeader<nHeaderCount;++nHeader)
				{
					uint32 nWidth = m_pHeaderCtrl->GetItemBaseWidth(nHeader);
					pCtrl->SetColumnWidth( (uint8)nHeader, nWidth );
				}

				pCtrl->ResetColumns();
			}
		}
		break;

	case eHeaderCtrlCmd_Click:
		if( m_pCommandHandler )
		{
			m_pCommandHandler->SendCommand( nParam1, nParam2, 0 );
		}
		break;
	}

	return CLTGUICommandHandler::OnCommand(nCommand, nParam1, nParam2);
}

// called when we get a scrollbar command
uint32 CLTGUIListCtrlEx::OnScrollBarCommand( uint32 nCommand, uint32 nParam1, uint32 nParam2 )
{
	switch( nCommand )
	{
	case eScrollBarCmd_Top:
		// Scroll to top
		m_iScrollOffset = 0;
		break;
	case eScrollBarCmd_Bottom:
		// Scroll to bottom
		m_iScrollOffset = -m_iMaxScrollLine;
		break;
	case eScrollBarCmd_LineUp:
		// Scroll one line up
		m_iScrollOffset += 1;
		break;
	case eScrollBarCmd_LineDown:
		// Scroll one line down
		m_iScrollOffset -= 1;
		break;
	case eScrollBarCmd_ThumbTrack:
		// Drag scroll box to specified position. The current position is provided in nParam1
		m_iScrollOffset = -(int32)nParam1;
		break;
	case eScrollBarCmd_ThumbPosition:
		// Scroll to the absolute position. The current position is provided in nParam1
		m_iScrollOffset = -(int32)nParam1;
		break;
	case eScrollBarCmd_PageUp:
		// Scroll one page up
		m_iScrollOffset += 5;
		break;
	case eScrollBarCmd_PageDown:
		// Scroll one page down
		m_iScrollOffset -= 5;
		break;
	}

	m_iScrollOffset = LTMIN( 0, LTMAX( -m_iMaxScrollLine, m_iScrollOffset ));

	if( m_pScrollBar )
		m_pScrollBar->SetScrollPos( -m_iScrollOffset );

	m_bNeedsRecalculation = true;

	return CLTGUICommandHandler::OnCommand(nCommand, nParam1, nParam2);
}

// update the scroll bar state
void CLTGUIListCtrlEx::UpdateScrollBar()
{
	if( !m_pScrollBar )
		return;

	m_nVisibleLineCount = ComputeVisibleLineCount();

	uint32 nLineCount = ComputeAllLineCount();
	m_iMaxScrollLine = nLineCount - m_nVisibleLineCount;
	if( m_iMaxScrollLine <= 0 )
		m_iScrollOffset = 0;

	m_pScrollBar->SetScrollPage( m_nVisibleLineCount );
	m_pScrollBar->SetScrollMin( 0 );
	m_pScrollBar->SetScrollMax( m_iMaxScrollLine );

	m_iScrollOffset = LTMAX( -m_iMaxScrollLine, m_iScrollOffset );
	m_pScrollBar->SetScrollPos( -m_iScrollOffset );
}

// determine how many lines are visible
uint32 CLTGUIListCtrlEx::ComputeVisibleLineCount()
{
	uint32 dwHeight = 0;
	uint32 nControl = 0;
	uint32 nLineCount = 0;
	for(;nControl<(uint32)m_controlArray.size();nControl++)
	{
		if( !m_controlArray[nControl]->IsVisible() ) 
			continue;

		//get the control's height (but unscale it)
		dwHeight += m_controlArray[nControl]->GetBaseHeight();
		dwHeight += m_nItemSpacing;

		if( dwHeight > GetBaseHeight() )
			return nLineCount;

		++nLineCount;
	}

	return nLineCount;
}

// determines how many lines really exist in the list
uint32 CLTGUIListCtrlEx::ComputeAllLineCount()
{
	uint32 nLineCount = 0;
	for(uint32 nControl=0;nControl<(uint32)m_controlArray.size();nControl++)
	{
		if( !m_controlArray[nControl]->IsVisible() ) 
			continue;

		++nLineCount;
	}

	return nLineCount;
}

void CLTGUIListCtrlEx::Show( bool bShow )
{
	if( m_pScrollBar )
		m_pScrollBar->Show( bShow );
	if( m_pHeaderCtrl )
		m_pHeaderCtrl->Show( bShow );

	CLTGUICtrl::Show( bShow );
}