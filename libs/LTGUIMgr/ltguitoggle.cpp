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
    m_bOn	= LTFALSE;
}

CLTGUIToggle::~CLTGUIToggle()
{
	Destroy();
}

// Create the control
// pText			- The initial text that is displayed for this control. Pass in 
//						LTNULL if you do not want initial text. A copy of this text
//						is made so the string may be discarded after making this call.
// pFont			- The font to use for this string.
// nFontSize		- The font size to use for this string.
// nHeaderWidth	- The width to use for the header string
// pbValue			  - Value to store the on/off status in when UpdateData is called
LTBOOL CLTGUIToggle::Create ( const char *pText, uint32 nHelpID, CUIFont *pFont, uint8 nFontSize,
                        uint16 nHeaderWidth, LTBOOL *pbValue)
{

	// Initialize the base class
    if (!CLTGUICycleCtrl::Create(pText, nHelpID, pFont, nFontSize, nHeaderWidth, LTNULL))
	{
        return LTFALSE;
	}

	AddString("OFF");
	AddString("ON");

	m_pbValue=pbValue;

    SetOn(LTFALSE);

    return LTTRUE;
}

// Update data
void CLTGUIToggle::UpdateData(LTBOOL bSaveAndValidate)
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
void CLTGUIToggle::SetOn(LTBOOL bOn)
{
	LTBOOL bWasOn = m_bOn;

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
void CLTGUIToggle::SetOnString(const char *pStrOn)
{
	m_stringArray[1]->SetText((char *)pStrOn);
}

// Set the string to use for "OFF"
void CLTGUIToggle::SetOffString(const char *pStrOff)
{
	m_stringArray[0]->SetText((char *)pStrOff);
}

