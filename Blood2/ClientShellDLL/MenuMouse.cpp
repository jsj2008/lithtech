// MenuMouse.cpp: implementation of the CMenuMouse class.
//
//////////////////////////////////////////////////////////////////////

#include "MenuBase.h"
#include "MainMenus.h"
#include "MenuCommands.h"
#include "MenuMouse.h"
#include "BloodClientShell.h"
#include "ClientRes.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuMouse::CMenuMouse()
{
	m_nMouseSensitivity=300;
	m_nInputRate=30;
	m_pInputRateCtrl=DNULL;
	m_bInvertYAxis=DFALSE;
	m_bMouseLook=DTRUE;
	m_bLookSpring=DFALSE;
	m_bUseWheel=DFALSE;
	m_bOrigUseWheel=m_bUseWheel;
}

CMenuMouse::~CMenuMouse()
{
	
}

// Build the menu
void CMenuMouse::Build()
{
	// Make sure to call the base class
	CMenuBase::Build();

	CreateTitle("interface\\mainmenus\\options.pcx", IDS_MENU_TITLE_OPTIONS, m_pMainMenus->GetTitlePos());		
	SetOptionPos(m_pMainMenus->GetOptionsPos());
	SetItemSpacing(5);
	SetScrollWrap(DFALSE);	

	// Load the mouse settings
	LoadMouseSettings();

	// Add the controls
	CLTGUISliderCtrl *pCtrl=AddSliderOption(IDS_MENU_MOUSE_SENSITIVITY, m_pMainMenus->GetSmallFont(), 100, m_pMainMenus->GetSurfaceSliderBar(), m_pMainMenus->GetSurfaceSliderTab(), &m_nMouseSensitivity);
	pCtrl->SetSliderRange(100, 1500);
	pCtrl->SetSliderIncrement(20);

	m_pInputRateCtrl=AddSliderOption(IDS_MENU_MOUSE_INPUTRATE, m_pMainMenus->GetSmallFont(), 100, m_pMainMenus->GetSurfaceSliderBar(), m_pMainMenus->GetSurfaceSliderTab(), &m_nInputRate);
	m_pInputRateCtrl->SetSliderIncrement(2);
	m_pInputRateCtrl->SetSliderRange(0, 40);

	AddOnOffOption(IDS_MENU_MOUSE_INVERT_YAXIS, m_pMainMenus->GetSmallFont(), 100, &m_bInvertYAxis);
	AddOnOffOption(IDS_MENU_MOUSE_MOUSELOOK, m_pMainMenus->GetSmallFont(), 100, &m_bMouseLook);
	AddOnOffOption(IDS_MENU_MOUSE_LOOKSPRING, m_pMainMenus->GetSmallFont(), 100, &m_bLookSpring);
	AddOnOffOption(IDS_MENU_MOUSE_USEWHEEL, m_pMainMenus->GetSmallFont(), 100, &m_bUseWheel);

	UpdateData(DFALSE);

	SetInputRateText();
}

// Load the mouse settings
void CMenuMouse::LoadMouseSettings()
{
	m_nMouseSensitivity=(int)(g_pBloodClientShell->GetMouseSensitivity()*100.0f);	
	m_bInvertYAxis=g_pBloodClientShell->IsMouseInvertYAxis();

	if (!m_pClientDE)
	{
		return;
	}

	// Load the InputRate
	HCONSOLEVAR hVar=m_pClientDE->GetConsoleVar("InputRate");
	if (hVar)
	{
		m_nInputRate=(int)m_pClientDE->GetVarValueFloat(hVar);
	}		
	//m_pClientDE->CPrint("Sensitivity = %d; inputrate = %d", m_nMouseSensitivity, m_nInputRate);

	// Load mouselook
	hVar=m_pClientDE->GetConsoleVar("MouseLook");
	if (hVar)
	{
		m_bMouseLook=((int)m_pClientDE->GetVarValueFloat(hVar)) ? DTRUE : DFALSE;
	}		
	//m_pClientDE->CPrint("MouseLook = %d", m_bMouseLook);

	// Load lookspring
	hVar=m_pClientDE->GetConsoleVar("LookSpring");
	if (hVar)
	{
		m_bLookSpring=((int)m_pClientDE->GetVarValueFloat(hVar)) ? DTRUE : DFALSE;
	}		
	//m_pClientDE->CPrint("LookSpring = %d", m_bLookSpring);

	// Load usewheel
	hVar=m_pClientDE->GetConsoleVar("UseWheel");
	if (hVar)
	{
		m_bUseWheel=((int)m_pClientDE->GetVarValueFloat(hVar)) ? DTRUE : DFALSE;
		m_bOrigUseWheel = m_bUseWheel;
	}		
	//m_pClientDE->CPrint("UseWheel = %d", m_bUseWheel);

	// Load mouse axis
	hVar=m_pClientDE->GetConsoleVar("MouseInvertYAxis");
	if (hVar)
	{
		m_bInvertYAxis=((int)m_pClientDE->GetVarValueFloat(hVar)) ? DTRUE : DFALSE;
	}		
	//m_pClientDE->CPrint("MouseInvertYAxis = %d", m_bInvertYAxis);

}

// Save the mouse settings
void CMenuMouse::SaveMouseSettings()
{
	char strConsole[256];
	//m_pClientDE->CPrint("Sensitivity = %d; inputrate = %d", m_nMouseSensitivity, m_nInputRate);
	sprintf(strConsole, "+MouseSensitivity %f", (float)m_nMouseSensitivity/100.0f);
	m_pClientDE->RunConsoleString(strConsole);

	sprintf(strConsole, "+InputRate %f", (float)m_nInputRate);
	m_pClientDE->RunConsoleString(strConsole);

	sprintf(strConsole, "+MouseInvertYAxis %f", (float)(m_bInvertYAxis ? 1 : 0));
	m_pClientDE->RunConsoleString(strConsole);

	sprintf(strConsole, "+MouseLook %f", (float)(m_bMouseLook ? 1 : 0));
	m_pClientDE->RunConsoleString(strConsole);

	sprintf(strConsole, "+LookSpring %f", (float)(m_bLookSpring ? 1 : 0));
	m_pClientDE->RunConsoleString(strConsole);

	sprintf(strConsole, "+UseWheel %f", (float)(m_bUseWheel ? 1 : 0));
	m_pClientDE->RunConsoleString(strConsole);

	// save bindings for the wheel setting
	if (m_bOrigUseWheel != m_bUseWheel)
	{
		if (m_bUseWheel)
		{
			sprintf(strConsole, "rangebind \"##Mouse\" \"Wheel\" 0.1 10000.0 \"NextWeapon\" -0.1 -10000.0 \"PrevWeapon\"");
			m_pClientDE->RunConsoleString(strConsole);
		}
		else
		{
			sprintf(strConsole, "rangebind \"##Mouse\" \"Wheel\" 0 0");
			m_pClientDE->RunConsoleString(strConsole);
		}
	}

}

void CMenuMouse::OnFocus(DBOOL bFocus)
{
	if (!bFocus)
	{
		UpdateData(DTRUE);
		g_pBloodClientShell->SetMouseSensitivity((float)m_nMouseSensitivity/100.0f);
		g_pBloodClientShell->SetMouseInvertYAxis(m_bInvertYAxis);
		g_pBloodClientShell->SetMouseLook(m_bMouseLook);
		g_pBloodClientShell->SetLookSpring(m_bLookSpring);

		SaveMouseSettings();

	}	
}

// Sets the inputrate text based on the current input rate
void CMenuMouse::SetInputRateText()
{
	UpdateData();

	if (!m_pInputRateCtrl)
	{
		return;
	}

	if (m_nInputRate <= 16)
	{
		m_pInputRateCtrl->SetText(IDS_MENU_MOUSE_INPUT_RESPONSIVE);
	}
	else if (m_nInputRate <= 26)
	{
		m_pInputRateCtrl->SetText(IDS_MENU_MOUSE_INPUT_NORMAL);
	}
	else if (m_nInputRate <= 40)
	{
		m_pInputRateCtrl->SetText(IDS_MENU_MOUSE_INPUT_SMOOTH);
	}
}

// Override left control
void CMenuMouse::OnLeft()
{
	CMenuBase::OnLeft();

	if (m_listOption.GetControl(m_listOption.GetSelectedItem()) == m_pInputRateCtrl)
	{
		SetInputRateText();
	}
}

// Override right control
void CMenuMouse::OnRight()
{
	CMenuBase::OnRight();

	if (m_listOption.GetControl(m_listOption.GetSelectedItem()) == m_pInputRateCtrl)
	{
		SetInputRateText();
	}
}

DDWORD CMenuMouse::OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
{
	return 0;
}