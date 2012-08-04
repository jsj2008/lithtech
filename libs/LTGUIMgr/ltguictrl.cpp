// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUICtrl.cpp
//
// PURPOSE : Base clase for controls
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "ltguimgr.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUICtrl::CLTGUICtrl()
{
    m_bSelected     = LTFALSE;
    m_bCreated      = LTFALSE;
    m_bEnabled      = LTTRUE;
    m_bVisible      = LTTRUE;
	m_nCommandID	= 0;
	m_nHelpID		= 0;
	m_nParam1		= 0;
	m_nParam2		= 0;
	m_pos.x			= 0;
	m_pos.y			= 0;
	m_basePos.x		= 0;
	m_basePos.y		= 0;
	m_fScale		= 1.0f;

    m_argbSelected	= 0xFFFFFFFF;
    m_argbNormal	= 0xFF000000;
    m_argbDisabled	= 0xFF808080;

}

CLTGUICtrl::~CLTGUICtrl()
{
}


// Select a control
void CLTGUICtrl::Select(LTBOOL bSelected)
{
	if (IsSelected() != bSelected)
	{
		m_bSelected=bSelected;
		OnSelChange();
	}
}

LTBOOL CLTGUICtrl::IsOnMe(int x, int y)
{
	if (!m_bVisible) return LTFALSE;

	return (x >= m_pos.x && y >= m_pos.y &&
			x <= (m_pos.x + GetWidth()) &&
			y <= (m_pos.y + GetHeight()) );
}


void CLTGUICtrl::SetScale(float fScale)
{
	m_pos.x = (int)(fScale * (float)m_basePos.x);
	m_pos.y = (int)(fScale * (float)m_basePos.y);
	m_fScale = fScale;
}