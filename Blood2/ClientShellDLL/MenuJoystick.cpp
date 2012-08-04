// MenuJoystick.cpp: implementation of the CMenuJoystick class.
//
//////////////////////////////////////////////////////////////////////

#include "MainMenus.h"
#include "MenuJoystick.h"
#include "MenuJoystickAxis.h"
#include "BloodClientShell.h"
#include "ClientRes.h"

#define JOYSTICKAXISX 0
#define JOYSTICKAXISY 1


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuJoystick::CMenuJoystick()
{
	m_bUseJoystick=DFALSE;	
	m_bUsePovHat=DFALSE;

	m_pUsePovHat=NULL;

	m_pAxisTurn=new CMenuJoystickAxisTurn;
	m_pAxisLook=new CMenuJoystickAxisLook;	
	m_pAxisStrafe=new CMenuJoystickAxisStrafe;
	m_pAxisMove=new CMenuJoystickAxisMove;	
}

CMenuJoystick::~CMenuJoystick()
{
	if (m_pAxisTurn)
	{
		delete m_pAxisTurn;
		m_pAxisTurn=DNULL;
	}
	if (m_pAxisLook)
	{
		delete m_pAxisLook;
		m_pAxisLook=DNULL;
	}
	if (m_pAxisStrafe)
	{
		delete m_pAxisStrafe;
		m_pAxisStrafe=DNULL;
	}
	if (m_pAxisMove)
	{
		delete m_pAxisMove;
		m_pAxisMove=DNULL;
	}	
}

// Build the menu
void CMenuJoystick::Build()
{
	// Make sure to call the base class
	CMenuBase::Build();	

//	m_pClientDE->CPrint("CMenuJoystick::Build called!"); // BLB TEMP
	
	CreateTitle("interface\\mainmenus\\options.pcx", IDS_MENU_TITLE_OPTIONS, m_pMainMenus->GetTitlePos());		
	SetOptionPos(m_pMainMenus->GetOptionsPos());
	SetItemSpacing(5);
	SetScrollWrap(DFALSE);	
	UseArrows(DTRUE, 300);

	m_bUseJoystick=g_pBloodClientShell->IsUseJoystick();

	// check if the joystick has been disabled by the advanced menu console variables
	HCONSOLEVAR hVar = m_pClientDE->GetConsoleVar( "joystickdisable");
	if (hVar != NULL) 
	{
		if (m_pClientDE->GetVarValueFloat(hVar) == 1) 
		{
			m_bUseJoystick = DFALSE;
			g_pBloodClientShell->SetUseJoystick(FALSE);
		}
	}

	// read in the JOYSTICK.CFG file
	m_pClientDE->ReadConfigFile ("joystick.cfg");

	UpdateData(DFALSE);
	BuildAxisMenus();	

	// Update the enable/disable status of the controls
	UpdateEnable();
}

// Build the axis menus
void CMenuJoystick::BuildAxisMenus()
{
	// Add the menu options
	AddOnOffOption(IDS_MENU_JOYSTICK_USE, m_pMainMenus->GetSmallFont(), 100, &m_bUseJoystick);

	CLTGUITextItemCtrl *pTitleCtrl=DNULL;

	// Turn menu options
	pTitleCtrl=AddTextItemOption(IDS_MENU_JOYSTICK_TURNLEFTRIGHTAXIS, 0, m_pMainMenus->GetSmallFont());
	pTitleCtrl->SetColor(SETRGB(220,190,170), SETRGB(125,30,0), SETRGB(0,255,255));
	if (pTitleCtrl) pTitleCtrl->Enable(DFALSE);

	m_pAxisTurn->Build(m_pClientDE, this);	

	// Look menu options
	pTitleCtrl=AddTextItemOption(IDS_MENU_JOYSTICK_LOOKUPDOWNAXIS, 0, m_pMainMenus->GetSmallFont());
	pTitleCtrl->SetColor(SETRGB(220,190,170), SETRGB(125,30,0), SETRGB(0,255,255));
	if (pTitleCtrl) pTitleCtrl->Enable(DFALSE);

	m_pAxisLook->Build(m_pClientDE, this);

	// Move menu options
	pTitleCtrl=AddTextItemOption(IDS_MENU_JOYSTICK_MOVEFORWARDBACKWARDAXIS, 0, m_pMainMenus->GetSmallFont());
	pTitleCtrl->SetColor(SETRGB(220,190,170), SETRGB(125,30,0), SETRGB(0,255,255));
	if (pTitleCtrl) pTitleCtrl->Enable(DFALSE);

	m_pAxisMove->Build(m_pClientDE, this);

	// Strafe menu options
	pTitleCtrl=AddTextItemOption(IDS_MENU_JOYSTICK_STRAFELEFTRIGHTAXIS, 0, m_pMainMenus->GetSmallFont());
	pTitleCtrl->SetColor(SETRGB(220,190,170), SETRGB(125,30,0), SETRGB(0,255,255));
	if (pTitleCtrl) pTitleCtrl->Enable(DFALSE);

	m_pAxisStrafe->Build(m_pClientDE, this);

	m_pUsePovHat = AddOnOffOption(IDS_MENU_JOYSTICK_HATONOFF, m_pMainMenus->GetSmallFont(), 100, &m_bUsePovHat);
}

// Update the enable/disabled status of the controls
void CMenuJoystick::UpdateEnable()
{
	// Update the enable/disable status of the controls
	m_pAxisTurn->UpdateEnable(m_bUseJoystick);
	m_pAxisMove->UpdateEnable(m_bUseJoystick);
	m_pAxisLook->UpdateEnable(m_bUseJoystick);
	m_pAxisStrafe->UpdateEnable(m_bUseJoystick);
	m_pAxisStrafe->UpdateEnable(m_bUseJoystick);
	if (m_pUsePovHat != NULL) m_pUsePovHat->Enable(m_bUseJoystick);
}


// The left key was pressed
void CMenuJoystick::OnLeft()
{
	// Call the base class
	CMenuBase::OnLeft();

	UpdateData();

	// Update the enable/disable status of the controls
	UpdateEnable();
}

// The right key was pressed
void CMenuJoystick::OnRight()
{
	// Call the base class
	CMenuBase::OnRight();

	UpdateData();

	// Update the enable/disable status of the controls
	UpdateEnable();	
}

// Change in focus
void CMenuJoystick::OnFocus(DBOOL bFocus)
{
	if (bFocus)
	{
		// get the hat variable 
		HCONSOLEVAR hVar = m_pClientDE->GetConsoleVar( "UsePovHat");
		if (hVar != NULL) 
		{
			if (m_pClientDE->GetVarValueFloat(hVar) == 1) m_bUsePovHat = DTRUE;
			else m_bUsePovHat = DFALSE;
		}

		// Load the menu options from the console	
		m_pAxisTurn->LoadFromConsole(m_pClientDE);
		m_pAxisMove->LoadFromConsole(m_pClientDE);
		m_pAxisLook->LoadFromConsole(m_pClientDE);
		m_pAxisStrafe->LoadFromConsole(m_pClientDE);		

		UpdateData(DFALSE);

		// Update the enable/disable status of the controls
		UpdateEnable();	
	}
	else		
	{
		char strConsole[512];

		UpdateData();

		// Save the menu options to the console
		m_pAxisTurn->SaveToConsole(m_pClientDE);
		m_pAxisMove->SaveToConsole(m_pClientDE);
		m_pAxisLook->SaveToConsole(m_pClientDE);
		m_pAxisStrafe->SaveToConsole(m_pClientDE);

		// get the name of the joystic device
		char* sDeviceName = m_pAxisTurn->GetDeviceName();

		// save the POV console variable
		{
			int nVal;
			if (m_bUsePovHat) nVal = 1;
			else nVal = 0;
			sprintf(strConsole, "+UsePovHat %f", (float)nVal);
			m_pClientDE->RunConsoleString(strConsole);
		}

		// write out the POV binding
		if (sDeviceName != NULL)
		{
			if (m_bUsePovHat)
			{
				sprintf(strConsole, "rangebind \"%s\" \"##POV 0\" 0.0 1000.0 \"LookUp\"  35000.0 36000.0 \"LookUp\" 8000.0 10000.0 \"NextWeapon\" 17000.0 19000.0 \"LookDown\" 26000 28000 \"PrevWeapon\"", sDeviceName);
				m_pClientDE->RunConsoleString(strConsole);
			}
			else
			{
				sprintf(strConsole, "rangebind \"%s\" \"##POV 0\" 0 0", sDeviceName);
				m_pClientDE->RunConsoleString(strConsole);
			}
		}

		g_pBloodClientShell->SetUseJoystick(m_bUseJoystick);
/* this code is no longer needed
		if (m_bUseJoystick)
		{
			char strJoystick[128];
			memset (strJoystick, 0, 128);
			DRESULT result = m_pClientDE->GetDeviceName (DEVICETYPE_JOYSTICK, strJoystick, 127);
			if (result == LT_OK) 
			{
				char strConsole[256];
				sprintf (strConsole, "EnableDevice \"%s\"", strJoystick);
				m_pClientDE->RunConsoleString (strConsole);
			}
			else
			{
				g_pBloodClientShell->SetUseJoystick(FALSE);
			}
		}	
*/
	}
}

