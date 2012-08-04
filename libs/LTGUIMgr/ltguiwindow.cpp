// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIWindow.h
//
// PURPOSE : Base class for window-type controls (e.g. message boxes)
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "ltguimgr.h"
#include "LTGUIWindow.h"
#include "vkdefs.h"

const uint16 CLTGUIWindow::kMaxNumControls = 0xFFFE;
const uint16 CLTGUIWindow::kNoSelection = 0xFFFF;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUIWindow::CLTGUIWindow()
{
	m_nCurrentIndex		= 0;
	m_nHeight			= 0;
	m_nWidth			= 0;
	m_nMouseDownItemSel	= -1;


}

CLTGUIWindow::~CLTGUIWindow()
{
	Destroy();
}

// Creation
LTBOOL CLTGUIWindow::Create (HTEXTURE hFrame, uint16 nWidth, uint16 nHeight, LTBOOL bSimpleStretch)
{

	m_nHeight=nHeight;
	m_nWidth=nWidth;

	m_Frame.Create(hFrame,nWidth,nHeight,bSimpleStretch);

    m_bCreated=LTTRUE;

    return LTTRUE;
}

// Destroy the control
void CLTGUIWindow::Destroy ( )
{
	if (m_bCreated)
	{
		m_Frame.Destroy();
		RemoveAll();
	}

    m_bCreated=LTFALSE;
}

// Render the control
void CLTGUIWindow::Render ( )
{
	if (!IsVisible()) return;

	m_Frame.Render();

	// Render the items 
	for (uint16 i = 0; i < m_controlArray.size(); i++ )
	{
		if (m_controlArray[i]->IsVisible())
			m_controlArray[i]->Render ();
	}
		
}


// Returns the height
uint16 CLTGUIWindow::GetHeight ( )
{
	return (uint16) ((float)m_nHeight * m_fScale);
}

// Returns the width
uint16 CLTGUIWindow::GetWidth ( )
{
	return (uint16) ((float)m_nWidth * m_fScale);
}

// Handles gamepad input
LTBOOL CLTGUIWindow::HandleInterfaceCommand(int command)
{
	if (CLTGUICtrl::HandleInterfaceCommand(command))
	{
        return LTTRUE;
	}
    LTBOOL handled = LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->HandleInterfaceCommand(command);
	return handled;
}

// Handle a keypress
LTBOOL CLTGUIWindow::HandleKeyDown(int key, int rep)
{
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

// Handle a keypress
LTBOOL CLTGUIWindow::HandleChar(unsigned char c)
{
    LTBOOL handled = LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->HandleChar(c);
	return handled;

}

LTBOOL CLTGUIWindow::OnUp ( )
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->OnUp())
		return LTTRUE;

	uint16 sel = m_nCurrentIndex;
	return (sel != PreviousSelection());
}

LTBOOL CLTGUIWindow::OnDown ( )
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->OnDown())
		return LTTRUE;

	uint16 sel = m_nCurrentIndex;
	return (sel != NextSelection());
}


LTBOOL  CLTGUIWindow::OnLeft ( )
{
    LTBOOL handled = LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnLeft();
	return handled;
}

LTBOOL  CLTGUIWindow::OnRight ( )
{
    LTBOOL handled = LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnRight();
	return handled;
}

LTBOOL  CLTGUIWindow::OnEnter ( )
{
    LTBOOL handled = LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnEnter();
	return handled;
}

// Handles the left button down message
LTBOOL CLTGUIWindow::OnLButtonDown(int x, int y)
{
	// Get the control that the click was on
	uint16 nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{
		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
            return LTFALSE;

		// Select the control
		SetSelection(nControlIndex);

		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.
		m_nMouseDownItemSel=nControlIndex;
		m_controlArray[nControlIndex]->OnLButtonDown(x,y);
        return LTTRUE;
	}
	else
	{
		// This clears the index for what item was selected from a mouse down message
		m_nMouseDownItemSel=-1;

        return LTFALSE;
	}
}

// Handles the left button up message
LTBOOL CLTGUIWindow::OnLButtonUp(int x, int y)
{
	// Get the control that the click was on
	uint16 nControlIndex=0;
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
			return m_controlArray[nControlIndex]->OnLButtonUp(x,y);
		}
	}
	else
	{
		m_nMouseDownItemSel=-1;
	}
    return LTFALSE;
}

// Handles the mouse move message
LTBOOL CLTGUIWindow::OnMouseMove(int x, int y)
{
	uint16 nControlUnderPoint=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlUnderPoint);
	if(pCtrl)
	{
		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
            return LTFALSE;

		if (pCtrl->OnMouseMove(x,y))
			return LTTRUE;

		if (GetSelectedIndex() != nControlUnderPoint)
		{
			SetSelection(nControlUnderPoint);
            return LTTRUE;
		}
	}

    return LTFALSE;
}

// Add a control the the array of controls
uint16 CLTGUIWindow::AddControl ( CLTGUICtrl *pControl, LTIntPt offset )
{
	ASSERT(pControl);
	if (!pControl) return kNoSelection;

	LTIntPt pos = m_basePos;
	pos.x += offset.x;
	pos.y += offset.y;
	pControl->SetBasePos(pos);
	pControl->SetScale(m_fScale);

	m_controlArray.push_back(pControl);

	return m_controlArray.size()-1;
}

// Remove a control
void CLTGUIWindow::RemoveControl ( CLTGUICtrl *pControl, LTBOOL bDelete )
{
	ControlArray::iterator iter = m_controlArray.begin();
	while (iter != m_controlArray.end() && (*iter) != pControl)
		iter++;

	if (iter != m_controlArray.end())
	{
		if (bDelete)
			debug_delete(*iter);
		m_controlArray.erase(iter);
	}
}

// Remove a control
void CLTGUIWindow::RemoveControl ( uint16 nIndex, LTBOOL bDelete )
{
	if (nIndex >= m_controlArray.size())
		return;

	ControlArray::iterator iter = m_controlArray.begin() + nIndex;
	if (bDelete)
		debug_delete(*iter);
	m_controlArray.erase(iter);
}

// Removes all of the controls
void CLTGUIWindow::RemoveAll ( LTBOOL bDelete )
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
}

// Return a control at a specific index
CLTGUICtrl *CLTGUIWindow::GetControl ( uint16 nIndex )
{
	if (nIndex >= m_controlArray.size())
		return LTNULL;

	return m_controlArray[nIndex];
}

int CLTGUIWindow::GetIndex(CLTGUICtrl* pControl)
{
	int index = 0;
	ControlArray::iterator iter = m_controlArray.begin();
	while (iter != m_controlArray.end() && (*iter) != pControl)
	{	
		iter++;
		index++;
	}

	if (iter == m_controlArray.end())
	{
		index = -1;
	}

	return index;

}

void CLTGUIWindow::SetControlOffset(CLTGUICtrl *pControl, LTIntPt offset)
{
	ControlArray::iterator iter = m_controlArray.begin();
	while (iter != m_controlArray.end() && (*iter) != pControl)
		iter++;

	if (iter != m_controlArray.end())
	{
		LTIntPt pos = m_basePos;
		pos.x += offset.x;
		pos.y += offset.y;
		(*iter)->SetBasePos(pos);
	}
}

// Select a control
void CLTGUIWindow::ClearSelection()
{
	if (m_nCurrentIndex < m_controlArray.size() )
	{
		m_controlArray[m_nCurrentIndex]->Select(LTFALSE);
	}
	m_nCurrentIndex = kNoSelection;
}


// Select a control
uint16 CLTGUIWindow::SetSelection( uint16 nIndex )
{

	if (nIndex >= m_controlArray.size())
		return m_nCurrentIndex;

	if (!m_controlArray[nIndex]->IsEnabled())
		return m_nCurrentIndex;

	if (m_nCurrentIndex==nIndex)
		return m_nCurrentIndex;

	if (m_nCurrentIndex < m_controlArray.size() )
	{
		m_controlArray[m_nCurrentIndex]->Select(LTFALSE);
	}


	m_nCurrentIndex=nIndex;
	m_controlArray[m_nCurrentIndex]->Select(LTTRUE);

	return m_nCurrentIndex;

}

// Gets the index of the control that is under the specific screen point.
// Returns FALSE if there isn't one under the specified point.
CLTGUICtrl *CLTGUIWindow::GetControlUnderPoint(int xPos, int yPos, uint16 *pnIndex)
{
	ASSERT(pnIndex);
	if (!pnIndex) return LTNULL;

	// See if the user clicked on any of the controls.
	for (uint16 i = 0; i < m_controlArray.size(); i++)
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

uint16 CLTGUIWindow::NextSelection()
{
	uint16 select = m_nCurrentIndex;
	if (select == kNoSelection)
		select = m_controlArray.size()-1;
	uint16 oldSelect = select;
	
	CLTGUICtrl* pCtrl = LTNULL;	
	do
	{
		select++;
		if (select >= m_controlArray.size())
		{
			select = 0;
		}
	
		pCtrl = GetControl(select);	

	} while (select != oldSelect && pCtrl && !pCtrl->IsEnabled() );


	if (!pCtrl || !pCtrl->IsEnabled() )
		select = m_nCurrentIndex;

	return SetSelection(select);

}

uint16 CLTGUIWindow::PreviousSelection()
{
	uint16 select = m_nCurrentIndex;
	if (select == kNoSelection)
		select = 0;
	int oldSelect = select;
	
	CLTGUICtrl* pCtrl = LTNULL;	
	do
	{
		if (select == 0)
		{
			select = m_controlArray.size()-1;
		}
		else
			select--;
	
		pCtrl = GetControl(select);	

	} while (select != oldSelect && pCtrl && !pCtrl->IsEnabled() );


	if (!pCtrl || !pCtrl->IsEnabled() )
		select = m_nCurrentIndex;

	return SetSelection(select);

}

void CLTGUIWindow::SetBasePos ( LTIntPt pos )
{ 
	LTIntPt offset;
	offset.x = pos.x - m_basePos.x;
	offset.y = pos.y - m_basePos.y;
	CLTGUICtrl::SetBasePos(pos);
	m_Frame.SetBasePos(pos);

	for (uint16 i = 0; i < m_controlArray.size(); i++)
	{
		LTIntPt pos = m_controlArray[i]->GetBasePos();
		pos.x += offset.x;
		pos.y += offset.y;
		m_controlArray[i]->SetBasePos(pos);
	}

}


void CLTGUIWindow::SetScale(float fScale)
{
	CLTGUICtrl::SetScale(fScale);
	m_Frame.SetScale(m_fScale);
	for (uint16 i = 0; i < m_controlArray.size(); i++ )
	{
		m_controlArray[i]->SetScale(m_fScale);
	}
}


void CLTGUIWindow::Enable ( LTBOOL bEnabled )
{ 
	CLTGUICtrl::Enable (bEnabled);
	for (uint16 i = 0; i < m_controlArray.size(); i++ )
	{
		m_controlArray[i]->Enable (bEnabled);
	}

}


//when the list is selected/deselected
void CLTGUIWindow::OnSelChange()
{
	ClearSelection();
	if (IsSelected())
	{
		uint16 nTry = 0;
		while (nTry < m_controlArray.size() && m_nCurrentIndex != kNoSelection)
		{
			SetSelection(nTry);
			nTry++;
		}
	}
}

void CLTGUIWindow::SetSize(uint16 nWidth, uint16 nHeight)
{
	m_nHeight = nHeight;
	m_nWidth = nWidth;
	m_Frame.SetSize(nWidth,nHeight);
}


uint32 CLTGUIWindow::GetHelpID()
{ 
	if (m_nHelpID)
		return m_nHelpID; 
	
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		return pCtrl->GetHelpID();

	return 0;
}
