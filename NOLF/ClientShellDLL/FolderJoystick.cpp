// FolderJoystick.cpp: implementation of the CFolderJoystick class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderJoystick.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "JoystickAxis.h"


#include "InterfaceMgr.h"
#include "GameSettings.h"
#include "keyfixes.h"


#define JOYSTICKAXISX 0
#define JOYSTICKAXISY 1

CCycleCtrl *pAxes[4];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderJoystick::CFolderJoystick()
{
	m_pAxisTurn=LTNULL;
	m_pAxisLook=LTNULL;
	m_pAxisStrafe=LTNULL;
	m_pAxisMove=LTNULL;

    m_bUseJoystick=LTFALSE;

}

CFolderJoystick::~CFolderJoystick()
{
	if (m_pAxisTurn)
	{
		debug_delete(m_pAxisTurn);
        m_pAxisTurn=LTNULL;
	}
	if (m_pAxisLook)
	{
		debug_delete(m_pAxisLook);
        m_pAxisLook=LTNULL;
	}
	if (m_pAxisStrafe)
	{
		debug_delete(m_pAxisStrafe);
        m_pAxisStrafe=LTNULL;
	}
	if (m_pAxisMove)
	{
		debug_delete(m_pAxisMove);
        m_pAxisMove=LTNULL;
	}

}

// Build the folder
LTBOOL CFolderJoystick::Build()
{


	CreateTitle(IDS_TITLE_JOYSTICK);

	m_pAxisTurn=debug_new(CJoystickAxisTurn);
	m_pAxisLook=debug_new(CJoystickAxisLook);
	m_pAxisStrafe=debug_new(CJoystickAxisStrafe);
	m_pAxisMove=debug_new(CJoystickAxisMove);

	BuildAxisFolders();

	// Update the enable/disable status of the controls
	UpdateEnable();


	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}

uint32 CFolderJoystick::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
};


// Build the axis folders
void CFolderJoystick::BuildAxisFolders()
{

    CStaticTextCtrl *pTitleCtrl=LTNULL;
	int nAxis = 0;

	// put the enabledevice command out to the console
	char tempStr[512];
	sprintf(tempStr, "enabledevice \"Joystick 1\"");
	g_pLTClient->RunConsoleString(tempStr);


	// Turn folder options
	pTitleCtrl= AddStaticTextItem(IDS_JOYSTICK_TURNLEFTRIGHTAXIS,LTNULL,IDS_HELP_JOYSTICK_LEFTRIGHT,200,0,LTTRUE,GetMediumFont());
	if (pTitleCtrl)
	{
        pTitleCtrl->Enable(LTFALSE);
	}
    m_pAxisTurn->Build(g_pLTClient, this);
	pAxes[nAxis] = m_pAxisTurn->GetAxisCtrl();
	pAxes[nAxis]->SetHelpID(IDS_HELP_JOYSTICK_LEFTRIGHT);
	nAxis++;

	AddPageBreak();

	// Look folder options
	pTitleCtrl= AddStaticTextItem(IDS_JOYSTICK_LOOKUPDOWNAXIS,LTNULL,IDS_HELP_JOYSTICK_UPDOWN,200,0,LTTRUE,GetMediumFont());
	if (pTitleCtrl)
	{
        pTitleCtrl->Enable(LTFALSE);
	}
    m_pAxisLook->Build(g_pLTClient, this);
	pAxes[nAxis] = m_pAxisLook->GetAxisCtrl();
	pAxes[nAxis]->SetHelpID(IDS_HELP_JOYSTICK_UPDOWN);
	nAxis++;
	AddPageBreak();

	// Move folder options
	pTitleCtrl= AddStaticTextItem(IDS_JOYSTICK_MOVEFORWARDBACKWARDAXIS,LTNULL,IDS_HELP_JOYSTICK_MOVE,200,0,LTTRUE,GetMediumFont());
	if (pTitleCtrl)
	{
        pTitleCtrl->Enable(LTFALSE);
	}
    m_pAxisMove->Build(g_pLTClient, this);
	pAxes[nAxis] = m_pAxisMove->GetAxisCtrl();
	pAxes[nAxis]->SetHelpID(IDS_HELP_JOYSTICK_MOVE);
	nAxis++;
	AddPageBreak();

	// Strafe folder options
	pTitleCtrl= AddStaticTextItem(IDS_JOYSTICK_STRAFELEFTRIGHTAXIS,LTNULL,IDS_HELP_JOYSTICK_STRAFE,200,0,LTTRUE,GetMediumFont());
	if (pTitleCtrl)
	{
        pTitleCtrl->Enable(LTFALSE);
	}
    m_pAxisStrafe->Build(g_pLTClient, this);
	pAxes[nAxis] = m_pAxisStrafe->GetAxisCtrl();
	pAxes[nAxis]->SetHelpID(IDS_HELP_JOYSTICK_STRAFE);
	nAxis++;



}

// Update the enable/disabled status of the controls
void CFolderJoystick::UpdateEnable()
{
	// Update the enable/disable status of the controls
	m_pAxisTurn->UpdateEnable(m_bUseJoystick);
	m_pAxisMove->UpdateEnable(m_bUseJoystick);
	m_pAxisLook->UpdateEnable(m_bUseJoystick);
	m_pAxisStrafe->UpdateEnable(m_bUseJoystick);
}


// The left key was pressed
LTBOOL CFolderJoystick::OnLeft()
{
    LTBOOL handled = LTFALSE;
	// Call the base class
	handled = CBaseFolder::OnLeft();
	if (handled)
	{
		UpdateData();

		// Update the enable/disable status of the controls
		UpdateEnable();
	}

	return handled;
}

// The right key was pressed
LTBOOL CFolderJoystick::OnRight()
{
    LTBOOL handled = LTFALSE;
	// Call the base class
	handled = CBaseFolder::OnRight();

	if (handled)
	{
		UpdateData();

		// Update the enable/disable status of the controls
		UpdateEnable();
	}

	return handled;
}

// The enter key was pressed
LTBOOL CFolderJoystick::OnEnter()
{
    LTBOOL handled = LTFALSE;
	// Call the base class
	handled = CBaseFolder::OnEnter();

	if (handled)
	{

		UpdateData();

		// Update the enable/disable status of the controls
		UpdateEnable();
	}

	return handled;
}

LTBOOL CFolderJoystick::OnLButtonUp(int x, int y)
{

    LTBOOL handled = LTFALSE;
	// Call the base class
	handled = CBaseFolder::OnLButtonUp(x,y);

	if (handled)
	{
		UpdateData();

		// Update the enable/disable status of the controls
		UpdateEnable();
	}

	return handled;
}

LTBOOL CFolderJoystick::OnRButtonUp(int x, int y)
{
    LTBOOL handled = LTFALSE;
	// Call the base class
	handled = CBaseFolder::OnRButtonUp(x,y);

	if (handled)
	{
		UpdateData();

		// Update the enable/disable status of the controls
		UpdateEnable();
	}

	return handled;
}


// Change in focus
void CFolderJoystick::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_nCurrPage = 0;

	// Load the folder options from the console
		CGameSettings *pSettings = g_pInterfaceMgr->GetSettings();
		m_bUseJoystick=pSettings->UseJoystick();

        m_pAxisTurn->LoadFromConsole(g_pLTClient);
        m_pAxisMove->LoadFromConsole(g_pLTClient);
        m_pAxisLook->LoadFromConsole(g_pLTClient);
        m_pAxisStrafe->LoadFromConsole(g_pLTClient);


        UpdateData(LTFALSE);

		// Update the enable/disable status of the controls
		UpdateEnable();
	}
	else
	{
		UpdateAxes();
		UpdateData();

		// Save the folder options to the console
        m_pAxisTurn->SaveToConsole(g_pLTClient);
        m_pAxisMove->SaveToConsole(g_pLTClient);
        m_pAxisLook->SaveToConsole(g_pLTClient);
        m_pAxisStrafe->SaveToConsole(g_pLTClient);

        g_pLTClient->WriteConfigFile("autoexec.cfg");


	}
	CBaseFolder::OnFocus(bFocus);
}


LTBOOL	CFolderJoystick::IsJoystickConfigured()
{
	if (m_pAxisTurn->IsAxisBound()) return LTTRUE;

	if (m_pAxisMove->IsAxisBound()) return LTTRUE;

	if (m_pAxisLook->IsAxisBound()) return LTTRUE;

	if (m_pAxisStrafe->IsAxisBound()) return LTTRUE;


	return LTFALSE;
}


void CFolderJoystick::UpdateAxes()
{
	int nAxis = m_nCurrPage;
	int nAxisVal = pAxes[nAxis]->GetSelIndex();
	if (nAxisVal > 0)
	{
		for (int i = 0 ; i < 4; i++)
		{
			if (nAxis != i && nAxisVal == pAxes[i]->GetSelIndex())
			{
				pAxes[i]->SetSelIndex(0);
			}
		}
	}
	UpdateData();
	UpdateEnable();

}

LTBOOL CFolderJoystick::NextPage(LTBOOL bChangeSelection)
{
	if (!CBaseFolder::NextPage(bChangeSelection)) return LTFALSE;
	UpdateAxes();
	m_nCurrPage++;

	return LTTRUE;
}

LTBOOL CFolderJoystick::PreviousPage(LTBOOL bChangeSelection)
{
	if (!CBaseFolder::PreviousPage(bChangeSelection)) return LTFALSE;
	UpdateAxes();
	m_nCurrPage--;

	return LTTRUE;
}
