// FolderEffects.cpp: implementation of the CFolderEffects class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderEffects.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
#include "GameSettings.h"
extern CGameClientShell* g_pGameClientShell;

namespace
{
	int kHeaderWidth = 300;
	int kSliderWidth = 200;
	int kSpacerWidth = 25;
	int kTotalWidth  = kHeaderWidth + kSpacerWidth;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderEffects::CFolderEffects()
{
	m_bTracers = LTFALSE;
	m_bShellCasings = LTFALSE;
	m_bMuzzleLight = LTFALSE;
	m_bWeather = LTFALSE;
	m_nImpact = 2;
	m_nDebris = 2;
}

CFolderEffects::~CFolderEffects()
{
	Term();
}

// Build the folder
LTBOOL CFolderEffects::Build()
{

	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_EFFECTS,"ColumnWidth"))
	{
		kTotalWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_EFFECTS,"ColumnWidth");
		kHeaderWidth = kTotalWidth - kSpacerWidth;
	}
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_EFFECTS,"SliderWidth"))
		kSliderWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_EFFECTS,"SliderWidth");

	CreateTitle(IDS_TITLE_EFFECTS);

	CToggleCtrl* pToggle = AddToggle(IDS_TRACERS, IDS_HELP_TRACERS, kTotalWidth, &m_bTracers );
	pToggle->SetOnString(IDS_ON);
	pToggle->SetOffString(IDS_OFF);

	pToggle = AddToggle(IDS_SHELLCASINGS, IDS_HELP_SHELLCASINGS, kTotalWidth, &m_bShellCasings );
	pToggle->SetOnString(IDS_ON);
	pToggle->SetOffString(IDS_OFF);

	pToggle = AddToggle(IDS_MUZZLELIGHT, IDS_HELP_MUZZLELIGHT, kTotalWidth, &m_bMuzzleLight );
	pToggle->SetOnString(IDS_ON);
	pToggle->SetOffString(IDS_OFF);

	pToggle = AddToggle(IDS_WEATHER, IDS_HELP_WEATHER, kTotalWidth, &m_bWeather );
	pToggle->SetOnString(IDS_ON);
	pToggle->SetOffString(IDS_OFF);

	CCycleCtrl *pCycle = AddCycleItem(IDS_IMPACT,IDS_HELP_IMPACTS,kHeaderWidth,kSpacerWidth,&m_nImpact);
	pCycle->AddString(IDS_LOW);
	pCycle->AddString(IDS_MEDIUM);
	pCycle->AddString(IDS_HIGH);

	pCycle = AddCycleItem(IDS_DEBRIS,IDS_HELP_DEBRIS,kHeaderWidth,kSpacerWidth,&m_nDebris);
	pCycle->AddString(IDS_LOW);
	pCycle->AddString(IDS_MEDIUM);
	pCycle->AddString(IDS_HIGH);



 	// Make sure to call the base class
	if (!CBaseFolder::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);

	return LTTRUE;

}

void CFolderEffects::Term()
{
	// Make sure to call the base class
	CBaseFolder::Term();
}

uint32 CFolderEffects::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
};


void CFolderEffects::OnFocus(LTBOOL bFocus)
{

	
	if (bFocus)
	{
		m_bTracers = (LTBOOL)GetConsoleInt("Tracers",1);
		m_bShellCasings = (LTBOOL)GetConsoleInt("ShellCasings",1);
		m_bMuzzleLight = (LTBOOL)GetConsoleInt("MuzzleLight",1);
		m_bWeather = (LTBOOL)GetConsoleInt("EnableWeatherFX",1);
		m_nImpact = GetConsoleInt("ImpactFXLevel",2);
		m_nDebris = GetConsoleInt("DebrisFXLevel",2);
		UpdateData(LTFALSE);
	}
	else
	{
        UpdateData(LTTRUE);
		WriteConsoleInt("Tracers",(int)m_bTracers);
		WriteConsoleInt("ShellCasings",(int)m_bShellCasings);
		WriteConsoleInt("MuzzleLight",(int)m_bMuzzleLight);
		WriteConsoleInt("EnableWeatherFX",(int)m_bWeather);
		WriteConsoleInt("ImpactFXLevel",m_nImpact);
		WriteConsoleInt("DebrisFXLevel",m_nDebris);

	}
	CBaseFolder::OnFocus(bFocus);
}


