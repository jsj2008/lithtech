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
    m_bSelected     = false;
    m_bCreated      = false;
    m_bEnabled      = true;
    m_bVisible      = true;
	m_nCommandID	= 0;
	m_szHelpID		= "";
	m_nParam1		= 0;
	m_nParam2		= 0;
	m_vfScale.Init(1.0f,1.0f);

	m_argbSelected	= 0xFFFFFFFF;
    m_argbNormal	= 0xFF000000;
    m_argbDisabled	= 0xFF808080;
}

CLTGUICtrl::~CLTGUICtrl()
{
}


// Select a control
void CLTGUICtrl::Select(bool bSelected)
{
	if (IsSelected() != bSelected)
	{
		m_bSelected=bSelected;
		OnSelChange();
	}
}

void CLTGUICtrl::Enable ( bool bEnabled )
{ 
	m_bEnabled=bEnabled; 
	if (!m_bEnabled)
	{
		Select(false);
	}
}

void CLTGUICtrl::SetScale(const LTVector2& vfScale)
{
	m_rfRect.m_vMin.x = (vfScale.x * (float)m_rnBaseRect.m_vMin.x);
	m_rfRect.m_vMin.y = (vfScale.y * (float)m_rnBaseRect.m_vMin.y);
	m_rfRect.m_vMax.x = (vfScale.x * (float)m_rnBaseRect.m_vMax.x);
	m_rfRect.m_vMax.y = (vfScale.y * (float)m_rnBaseRect.m_vMax.y);
	m_vfScale = vfScale;
}


void CLTGUICtrl::SetBasePos(const LTVector2n& pos )
{
	LTVector2n sz = m_rnBaseRect.m_vMax - m_rnBaseRect.m_vMin;
	m_rnBaseRect.m_vMin = pos;
	m_rnBaseRect.m_vMax = pos + sz;

	m_rfRect.m_vMin.x = (m_vfScale.x * (float)m_rnBaseRect.m_vMin.x);
	m_rfRect.m_vMin.y = (m_vfScale.y * (float)m_rnBaseRect.m_vMin.y);
	m_rfRect.m_vMax.x = (m_vfScale.x * (float)m_rnBaseRect.m_vMax.x);
	m_rfRect.m_vMax.y = (m_vfScale.y * (float)m_rnBaseRect.m_vMax.y);
}

void CLTGUICtrl::SetBaseWidth( uint32 nWidth ) 
{ 
	m_rnBaseRect.Right() = m_rnBaseRect.Left() + nWidth; 

	m_rfRect.m_vMin.x = (m_vfScale.x * (float)m_rnBaseRect.m_vMin.x);
	m_rfRect.m_vMin.y = (m_vfScale.y * (float)m_rnBaseRect.m_vMin.y);
	m_rfRect.m_vMax.x = (m_vfScale.x * (float)m_rnBaseRect.m_vMax.x);
	m_rfRect.m_vMax.y = (m_vfScale.y * (float)m_rnBaseRect.m_vMax.y);
}

void CLTGUICtrl::SetSize( const LTVector2n& sz )
{
	m_rnBaseRect.m_vMax = m_rnBaseRect.m_vMin + sz;

	m_rfRect.m_vMin.x = (m_vfScale.x * (float)m_rnBaseRect.m_vMin.x);
	m_rfRect.m_vMin.y = (m_vfScale.y * (float)m_rnBaseRect.m_vMin.y);
	m_rfRect.m_vMax.x = (m_vfScale.x * (float)m_rnBaseRect.m_vMax.x);
	m_rfRect.m_vMax.y = (m_vfScale.y * (float)m_rnBaseRect.m_vMax.y);
}
