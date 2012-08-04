// GroupCtrl.cpp: implementation of the CGroupCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GroupCtrl.h"
#include "InterfaceMgr.h"

CGroupSubCtrl::CGroupSubCtrl(CLTGUICtrl* pCtrl, LTIntPt offset, LTBOOL bSelectable)
{
	_ASSERT(pCtrl);
	m_pCtrl = pCtrl;
	m_Offset = offset;
	m_bSelectable = bSelectable;
}

CGroupSubCtrl::~CGroupSubCtrl()
{
	debug_delete(m_pCtrl);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGroupCtrl::CGroupCtrl()
{
	m_nWidth = 0;
	m_nHeight = 0;
    m_bCreated = LTFALSE;
    m_bSubSelect = LTFALSE;
	m_nLastMouseDown = -1;
	m_nSelection = -1;
}

CGroupCtrl::~CGroupCtrl()
{
	Destroy();
}

LTBOOL CGroupCtrl::Create ( int nWidth , int nHeight, LTBOOL bSubSelect )
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;
    m_bCreated = LTTRUE;
	m_bSubSelect = bSubSelect;
	m_controlArray.SetSize(0);
    return LTTRUE;
}

// Destroy the control
void CGroupCtrl::Destroy ( )
{
	RemoveAllControls();
    m_bCreated=LTFALSE;
}

// Render the control
void CGroupCtrl::Render ( HSURFACE hDestSurf )
{
	unsigned int i;
	for ( i = 0; i < m_controlArray.GetSize(); i++ )
	{
		int x = m_pos.x + m_controlArray[i]->m_Offset.x;
		int y = m_pos.y + m_controlArray[i]->m_Offset.y;

		m_controlArray[i]->m_pCtrl->SetPos(x, y);
		m_controlArray[i]->m_pCtrl->Render ( hDestSurf );
	}
}

// Handle a keypress
LTBOOL CGroupCtrl::HandleKeyDown(int key, int rep)
{
    LTBOOL bHandled = LTFALSE;
	if (CLTGUICtrl::HandleKeyDown(key, rep))
	{
        bHandled = LTTRUE;
	}
	else
	{
		unsigned int i = 0;
		while (i < m_controlArray.GetSize() && !bHandled)
		{
			bHandled = m_controlArray[i]->m_pCtrl->HandleKeyDown(key, rep);
		}
	}

	return bHandled;
}

LTBOOL CGroupCtrl::OnLButtonDown(int x, int y)
{
	if (!m_bSubSelect)
        return LTFALSE;

	// Get the control that the click was on
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		m_nLastMouseDown = nControlIndex;
        return LTTRUE;
	}
    return LTFALSE;
}

LTBOOL CGroupCtrl::OnLButtonUp(int x, int y)
{
	if (!m_bSubSelect)
	{
        LTBOOL handled = LTFALSE;
		unsigned int i;
		unsigned int n = m_controlArray.GetSize();
		for ( i = 0;!handled && i < m_controlArray.GetSize(); i++ )
		{
			if (m_controlArray[i]->m_bSelectable)
				handled = m_controlArray[i]->m_pCtrl->OnLButtonUp(x,y);
		}
		return handled;

	}

	// Get the control that the click was on
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		if (m_nLastMouseDown == nControlIndex)
		{
			CLTGUICtrl* pCtrl = GetControl(nControlIndex);
			if (pCtrl->IsEnabled() )
			{
				if (pCtrl->OnLButtonUp(x,y))
                    return LTTRUE;
			}

		}
	}
	else
		m_nLastMouseDown = -1;

    return LTFALSE;
}

LTBOOL CGroupCtrl::OnEnter()
{
	if (!m_bSubSelect)
	{
        LTBOOL handled = LTFALSE;
		unsigned int i;
		for ( i = 0;!handled && i < m_controlArray.GetSize(); i++ )
		{
			if (m_controlArray[i]->m_bSelectable)
				handled = m_controlArray[i]->m_pCtrl->OnEnter();
		}
		return handled;

	}

	if (m_nSelection >= 0)
	{
		CLTGUICtrl* pCtrl = GetControl(m_nSelection);
		if (pCtrl->IsEnabled() )
		{
			if (pCtrl->OnEnter())
			{
                return LTTRUE;
			}
		}
	}

    return LTFALSE;
}

// Gets the index of the control that is under the specific screen point.
// Returns FALSE if there isn't one under the specified point.
LTBOOL CGroupCtrl::GetControlUnderPoint(int xPos, int yPos, int *pnIndex)
{
	_ASSERT(pnIndex);
	if (!m_bSubSelect)
        return LTFALSE;


	// See if the user clicked on any of the controls.
	for (unsigned int i=0; i < m_controlArray.GetSize(); i++)
	{
		CLTGUICtrl* pCtrl = GetControl(i);
		int nLeft = m_pos.x + m_controlArray[i]->m_Offset.x;
		int nTop = m_pos.y + m_controlArray[i]->m_Offset.y;

		int nRight=nLeft+pCtrl->GetWidth();
		int nBottom=nTop+pCtrl->GetHeight();

		// Check to see if the click is in the bounding box for the control
		if (xPos >= nLeft && xPos <= nRight && yPos >= nTop && yPos <= nBottom)
		{
			*pnIndex=i;

            return LTTRUE;
		}
	}
    return LTFALSE;
}


// Add/Remove controls to the array
int CGroupCtrl::AddControl ( CLTGUICtrl *pControl, LTIntPt offset, LTBOOL bSelectable)
{
	_ASSERT(pControl);
	CGroupSubCtrl *pNewCtrl = debug_new3(CGroupSubCtrl, pControl,offset,bSelectable);
	m_controlArray.Add(pNewCtrl);

	int nIndex = m_controlArray.GetSize()-1;
	if ( IsSelected() && bSelectable )
	{
		m_controlArray[nIndex]->m_pCtrl->Select(TRUE);
	}

	return nIndex;
}


void CGroupCtrl::RemoveControl ( CLTGUICtrl *pControl )
{
	unsigned int i;
	for ( i = 0; i < m_controlArray.GetSize(); i++ )
	{
		if ( m_controlArray[i]->m_pCtrl == pControl )
		{
			debug_delete(m_controlArray[i]);
			m_controlArray.Remove(i);
			return;
		}
	}
}

void CGroupCtrl::RemoveControl ( int nIndex )
{
	debug_delete(m_controlArray[nIndex]);
	m_controlArray.Remove(nIndex);
}

void CGroupCtrl::RemoveAllControls ( )
{
	while ( GetNumControls() > 0 )
	{
		RemoveControl(0);
	}
}


CLTGUICtrl	*CGroupCtrl::GetControl ( int nIndex )
{
	if (nIndex < 0 || nIndex >= (int)m_controlArray.GetSize()) return LTNULL;
	return m_controlArray[nIndex]->m_pCtrl;
}

LTIntPt CGroupCtrl::GetControlOffset( int nIndex )
{
	if (nIndex < 0 || nIndex >= (int)m_controlArray.GetSize()) return LTIntPt(0,0);
	return m_controlArray[nIndex]->m_Offset;
}


void CGroupCtrl::OnSelChange()
{
    LTBOOL bSelect = IsSelected() && !m_bSubSelect;
	unsigned int i;
	for ( i = 0; i < m_controlArray.GetSize(); i++ )
	{
		if ( m_controlArray[i]->m_bSelectable )
		{
			m_controlArray[i]->m_pCtrl->Select(bSelect);
		}
	}
	m_nSelection = -1;
	if (IsSelected())
	{
		if (m_bSubSelect)
		{
            LTIntPt mPos = g_pInterfaceMgr->GetCursorPos();
			OnMouseMove(mPos.x,mPos.y);
		}
	}
}

// Disable the control
void    CGroupCtrl::Enable( LTBOOL bEnabled )
{
	m_bEnabled=bEnabled;
	unsigned int i;
	for ( i = 0; i < m_controlArray.GetSize(); i++ )
	{
		m_controlArray[i]->m_pCtrl->Enable(bEnabled);
	}
}

LTBOOL CGroupCtrl::OnMouseMove(int x, int y)
{
	if (!m_bSubSelect)
        return LTFALSE;
	// Get the control that the click was on
	int nControlIndex=0;
    LTBOOL bOnCtrl = GetControlUnderPoint(x, y, &nControlIndex);
	if (bOnCtrl)
	{
		SelectControl(nControlIndex);
	}
	else
	{
		SelectControl(-1);
	}

	return bOnCtrl;

}

void CGroupCtrl::SelectControl(int nNewSelection)
{
	if (!m_bSubSelect)
		return;

	if (nNewSelection == m_nSelection)
		return;
	if (m_nSelection >= 0)
	{
		if ( m_controlArray[m_nSelection]->m_bSelectable)
		{
            m_controlArray[m_nSelection]->m_pCtrl->Select(LTFALSE);
		}
		m_nSelection = -1;
	}
	if (nNewSelection >= 0)
	{
		if ( m_controlArray[nNewSelection]->m_bSelectable )
		{
            m_controlArray[nNewSelection]->m_pCtrl->Select(LTTRUE);
			m_nSelection = nNewSelection;
		}
	}

}

uint32 CGroupCtrl::GetHelpID()
{
	if (m_bSubSelect)
	{
		if (m_nSelection >= 0 && m_controlArray[m_nSelection]->m_bSelectable)
		{
			if (m_controlArray[m_nSelection]->m_pCtrl->GetHelpID())
				return m_controlArray[m_nSelection]->m_pCtrl->GetHelpID();
		}
	}
	return m_dwHelpID;
}

// Calls UpdateData on each control in the group
void CGroupCtrl::UpdateData(LTBOOL bSaveAndValidate)
{
	unsigned int i;
	for (i=0; i < m_controlArray.GetSize(); i++)
	{
		m_controlArray[i]->m_pCtrl->UpdateData(bSaveAndValidate);
	}
}

