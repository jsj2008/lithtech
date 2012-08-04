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
#if !defined(PLATFORM_XENON)
#include "vkdefs.h"
#endif // !PLATFORM_XENON

const uint32 CLTGUIWindow::kMaxNumControls = 0xFFFFFFFE;
const uint32 CLTGUIWindow::kNoSelection = 0xFFFFFFFF;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUIWindow::CLTGUIWindow()
{
	m_nCurrentIndex		= 0;
	m_nMouseDownItemSel	= kNoSelection;
	m_nRMouseDownItemSel	= kNoSelection;

}

CLTGUIWindow::~CLTGUIWindow()
{
	Destroy();
}

// Creation
bool CLTGUIWindow::Create(HTEXTURE hFrame, const CLTGUICtrl_create& cs, bool bSimpleStretch /* = false */)
{


	m_Frame.Create(hFrame,cs,bSimpleStretch);

	CLTGUICtrl::Create(cs);

	return true;
}

// Destroy the control
void CLTGUIWindow::Destroy ( )
{
	if (m_bCreated)
	{
		m_Frame.Destroy();
		RemoveAll();
	}
}

// Render the control
void CLTGUIWindow::Render ( )
{
	if (!IsVisible()) return;

	m_Frame.Render();

	// Render the items 
	for (uint32 i = 0; i < m_controlArray.size(); i++ )
	{
		if (m_controlArray[i]->IsVisible())
			m_controlArray[i]->Render ();
	}
		
}

// Render the control
void CLTGUIWindow::RenderTransition(float fTrans )
{
	if (!IsVisible()) return;

	m_Frame.RenderTransition(fTrans);

	// Render the items 
	for (uint32 i = 0; i < m_controlArray.size(); i++ )
	{
		if (m_controlArray[i]->IsVisible())
			m_controlArray[i]->RenderTransition(fTrans);
	}

}



// Handles gamepad input
bool CLTGUIWindow::HandleInterfaceCommand(int command)
{
	if (CLTGUICtrl::HandleInterfaceCommand(command))
	{
        return true;
	}
    bool handled = false;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->HandleInterfaceCommand(command);
	return handled;
}

// Handle a keypress
bool CLTGUIWindow::HandleKeyDown(int key, int rep)
{
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

// Handle a keypress
bool CLTGUIWindow::HandleChar(wchar_t c)
{
    bool handled = false;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->HandleChar(c);
	return handled;

}

bool CLTGUIWindow::OnUp ( )
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->OnUp())
		return true;

	uint32 sel = m_nCurrentIndex;
	return (sel != PreviousSelection());
}

bool CLTGUIWindow::OnDown ( )
{
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->OnDown())
		return true;

	uint32 sel = m_nCurrentIndex;
	return (sel != NextSelection());
}


bool  CLTGUIWindow::OnLeft ( )
{
    bool handled = false;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnLeft();
	return handled;
}

bool  CLTGUIWindow::OnRight ( )
{
    bool handled = false;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnRight();
	return handled;
}

bool  CLTGUIWindow::OnEnter ( )
{
    bool handled = false;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnEnter();
	return handled;
}

// Handles the left button down message
bool CLTGUIWindow::OnLButtonDown(int x, int y)
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
		m_nMouseDownItemSel=nControlIndex;
		m_controlArray[nControlIndex]->OnLButtonDown(x,y);
        return true;
	}
	else
	{
		// This clears the index for what item was selected from a mouse down message
		m_nMouseDownItemSel=( uint32 )-1;

        return false;
	}
}



// Handles the left button up message
bool CLTGUIWindow::OnLButtonUp(int x, int y)
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
		if (nControlIndex == m_nMouseDownItemSel)
		{
			return m_controlArray[nControlIndex]->OnLButtonUp(x,y);
		}
	}
	else
	{
		m_nMouseDownItemSel=(uint32)-1;
	}
    return false;
}


// Handles the right button down message
bool CLTGUIWindow::OnRButtonDown(int x, int y)
{
	// Get the control that the click was on
	uint32 nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{
		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
			return false;

		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.
		m_nRMouseDownItemSel=nControlIndex;
		m_controlArray[nControlIndex]->OnRButtonDown(x,y);
		return true;
	}
	else
	{
		// This clears the index for what item was selected from a mouse down message
		m_nRMouseDownItemSel=( uint32 )-1;

		return false;
	}
}

// Handles the right button up message
bool CLTGUIWindow::OnRButtonUp(int x, int y)
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
		if (nControlIndex == m_nRMouseDownItemSel)
		{
			return m_controlArray[nControlIndex]->OnRButtonUp(x,y);
		}
	}
	else
	{
		m_nRMouseDownItemSel=(uint32)-1;
	}
	return false;
}



// Handles the mouse move message
bool CLTGUIWindow::OnMouseMove(int x, int y)
{
	uint32 nControlUnderPoint=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlUnderPoint);
	if(pCtrl)
	{
		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
            return false;

		if (pCtrl->OnMouseMove(x,y))
			return true;

		if (GetSelectedIndex() != nControlUnderPoint)
		{
			SetSelection(nControlUnderPoint);
            return true;
		}
	}

    return false;
}

// Add a control the the array of controls
uint32 CLTGUIWindow::AddControl ( CLTGUICtrl *pControl, LTVector2n offset )
{
	ASSERT(pControl);
	if (!pControl) return kNoSelection;

	LTVector2n pos = GetBasePos() + offset;
	pControl->SetBasePos(pos);
	pControl->SetScale(m_vfScale);

	m_controlArray.push_back(pControl);

	return m_controlArray.size()-1;
}

// Remove a control
void CLTGUIWindow::RemoveControl ( CLTGUICtrl *pControl, bool bDelete )
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
void CLTGUIWindow::RemoveControl ( uint32 nIndex, bool bDelete )
{
	if (nIndex >= m_controlArray.size())
		return;

	ControlArray::iterator iter = m_controlArray.begin() + nIndex;
	if (bDelete)
		debug_delete(*iter);
	m_controlArray.erase(iter);
}

// Removes all of the controls
void CLTGUIWindow::RemoveAll ( bool bDelete )
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
CLTGUICtrl *CLTGUIWindow::GetControl ( uint32 nIndex )
{
	if (nIndex >= m_controlArray.size())
		return NULL;

	return m_controlArray[nIndex];
}

uint32 CLTGUIWindow::GetIndex(CLTGUICtrl* pControl)
{
	uint32 index = 0;
	ControlArray::iterator iter = m_controlArray.begin();
	while (iter != m_controlArray.end() && (*iter) != pControl)
	{	
		iter++;
		index++;
	}

	if (iter == m_controlArray.end())
	{
		index = kNoSelection;
	}

	return index;

}

void CLTGUIWindow::SetControlOffset(CLTGUICtrl *pControl, LTVector2n offset)
{
	ControlArray::iterator iter = m_controlArray.begin();
	while (iter != m_controlArray.end() && (*iter) != pControl)
		iter++;

	if (iter != m_controlArray.end())
	{
		LTVector2n pos = GetBasePos() + offset;
		(*iter)->SetBasePos(pos);
	}
}

// Select a control
void CLTGUIWindow::ClearSelection()
{
	if (m_nCurrentIndex < m_controlArray.size() )
	{
		m_controlArray[m_nCurrentIndex]->Select(false);
	}
	m_nCurrentIndex = kNoSelection;
}


// Select a control
uint32 CLTGUIWindow::SetSelection( uint32 nIndex )
{

	if (nIndex >= m_controlArray.size())
		return m_nCurrentIndex;

	if (!m_controlArray[nIndex]->IsEnabled())
		return m_nCurrentIndex;

	if (m_nCurrentIndex==nIndex)
		return m_nCurrentIndex;

	if (m_nCurrentIndex < m_controlArray.size() )
	{
		m_controlArray[m_nCurrentIndex]->Select(false);
	}


	m_nCurrentIndex=nIndex;
	m_controlArray[m_nCurrentIndex]->Select(true);

	return m_nCurrentIndex;

}

// Gets the index of the control that is under the specific screen point.
// Returns FALSE if there isn't one under the specified point.
CLTGUICtrl *CLTGUIWindow::GetControlUnderPoint(int xPos, int yPos, uint32 *pnIndex)
{
	ASSERT(pnIndex);
	if (!pnIndex) return NULL;

	// See if the user clicked on any of the controls.
	for (uint32 i = 0; i < m_controlArray.size(); i++)
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

uint32 CLTGUIWindow::NextSelection()
{
	uint32 select = m_nCurrentIndex;
	if (select == kNoSelection)
		select = m_controlArray.size()-1;
	uint32 oldSelect = select;
	
	CLTGUICtrl* pCtrl = NULL;	
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

uint32 CLTGUIWindow::PreviousSelection()
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

void CLTGUIWindow::SetBasePos(const LTVector2n& pos)
{ 
	LTVector2n offset = pos - GetBasePos();

	CLTGUICtrl::SetBasePos(pos);
	m_Frame.SetBasePos(pos);

	for (uint32 i = 0; i < m_controlArray.size(); i++)
	{
		LTVector2n pos = m_controlArray[i]->GetBasePos();
		pos += offset;
		m_controlArray[i]->SetBasePos(pos);
	}

}


void CLTGUIWindow::SetScale(const LTVector2& vfScale)
{
	CLTGUICtrl::SetScale(vfScale);
	m_Frame.SetScale(m_vfScale);
	for (uint32 i = 0; i < m_controlArray.size(); i++ )
	{
		m_controlArray[i]->SetScale(m_vfScale);
	}
}

void CLTGUIWindow::SetSize(const LTVector2n& sz)
{
	CLTGUICtrl::SetSize(sz);
	m_Frame.SetSize(sz);
}

void CLTGUIWindow::Enable ( bool bEnabled )
{ 
	CLTGUICtrl::Enable (bEnabled);
	for (uint32 i = 0; i < m_controlArray.size(); i++ )
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
		uint32 nTry = 0;
		while (nTry < m_controlArray.size() && m_nCurrentIndex != kNoSelection)
		{
			SetSelection(nTry);
			nTry++;
		}
	}
}


const char* CLTGUIWindow::GetHelpID()
{ 
	if (m_szHelpID && m_szHelpID[0])
		return m_szHelpID; 
	
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		return pCtrl->GetHelpID();

	return "";
}

void CLTGUIWindow::FlushTextureStrings()
{
	for (uint32 i = 0; i < m_controlArray.size(); i++)
	{
		m_controlArray[i]->FlushTextureStrings();
	}
}

void CLTGUIWindow::RecreateTextureStrings()
{
	for (uint32 i = 0; i < m_controlArray.size(); i++)
	{
		m_controlArray[i]->RecreateTextureStrings();
	}
}

bool CLTGUIWindow::OnMouseWheel(int x, int y, int zDelta)
{
	// Get the control that the click was on
	uint32 nControlIndex=0;
	CLTGUICtrl *pCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if(pCtrl)
	{
		// Make sure we're enabled
		if(!pCtrl->IsEnabled())
			return false;

		m_controlArray[nControlIndex]->OnMouseWheel(x,y,zDelta);
		return true;
	}

	return false;
}

