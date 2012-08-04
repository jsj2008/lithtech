// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIToggle.cpp
//
// PURPOSE : Text control which toggles between two values.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "ltguimgr.h"
#include "LTGUIToggle.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUIToggle::CLTGUIToggle()
{
    m_bOn	= false;
}

CLTGUIToggle::~CLTGUIToggle()
{
	Destroy();
}

// Create the control
bool CLTGUIToggle::Create(const wchar_t *pText, const CFontInfo& Font, const CLTGUIToggle_create& cs )
{

	
	// Initialize the base class
    if (!CLTGUICycleCtrl::Create(pText, Font, (CLTGUICycleCtrl_create)cs ))
	{
        return false;
	}

	//shouldn't try to create as both cycle and toggle
	ASSERT(!cs.pnValue);
	m_pnValue = NULL;

	AddString(L"On");
	AddString(L"Off");

	m_pbValue=cs.pbValue;

    SetOn(false);

    return true;
}

// Update data
void CLTGUIToggle::UpdateData(bool bSaveAndValidate)
{
	if (!m_pbValue)
		return;

	if (bSaveAndValidate)
	{
		*m_pbValue = m_bOn;
	}
	else
	{
		SetOn(*m_pbValue);
	}
}

// Sets the control on/off state
void CLTGUIToggle::SetOn(bool bOn)
{
	bool bWasOn = m_bOn;

	m_bOn = bOn;

	if ( m_bOn )
	{
		SetSelIndex(1);
	}
	else
	{
		SetSelIndex(0);
	}

	if (m_pCommandHandler && m_bOn != bWasOn)
	{
		m_pCommandHandler->SendCommand(m_nCommandID, m_nParam1, m_nParam2);
	}
	

}

// Set the string to use for "ON"
void CLTGUIToggle::SetOnString(const wchar_t *pStrOn)
{
	m_StringArray[1]->SetText(pStrOn);
}

// Set the string to use for "OFF"
void CLTGUIToggle::SetOffString(const wchar_t *pStrOff)
{
	m_StringArray[0]->SetText(pStrOff);
}

