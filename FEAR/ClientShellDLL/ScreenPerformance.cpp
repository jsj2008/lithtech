// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPerformance.cpp
//
// PURPOSE : screen to set performance options
//
// CREATED : 09/23/04
//
// (c) 2004-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenPerformance.h"
#include "ScreenPerformanceAdvanced.h"
#include "ScreenPerformanceGPU.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "MissionMgr.h"
#include "PerformanceDB.h"
#include "PerformanceTest.h"
#include "ClientConnectionMgr.h"
#include "GameClientShell.h"
#include "PerformanceStats.h"
#include "PerformanceMgr.h"
#include "ltgamecfg.h"

VarTrack g_vtPerformanceScreenDebug;

namespace
{
	enum ScreenMainCmds
	{
		CMD_SWITCH_TO_SINGLE = CMD_CUSTOM+1,
	};

	void SwitchToSinglePlayerCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenPerformance *pThisScreen = (CScreenPerformance *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_SWITCH_TO_SINGLE,0,0);
	};
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenPerformance::CScreenPerformance() :
	m_bTrueExit(false),
	m_nWarningColor(0)
{
}

CScreenPerformance::~CScreenPerformance()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformance::Build()
//
//	PURPOSE:	build the controls used on the screen
//
// ----------------------------------------------------------------------- //

bool CScreenPerformance::Build()
{

	g_vtPerformanceScreenDebug.Init(g_pLTClient,"PerformanceScreenDebug",NULL,1.0f);

	CreateTitle("IDS_TITLE_PERFORMANCE");

	//get layout info
	int32 kColumn0 = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,0);
	int32 kColumn1 = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,1);
	uint32 nFontHeight = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize);
	uint32 nListFontHeight = g_pLayoutDB->GetListSize(m_hLayout,0);

	m_nWarningColor	= g_pLayoutDB->GetInt32(m_hLayout, LDB_ScreenAddColor, 0);

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(kColumn0+kColumn1,nFontHeight);
	cs.nCommandID = CMD_AUTO;
	cs.szHelpID = "Performance_Auto_Help";
	AddTextItem("Performance_Auto", cs );

	//add a cycle control to handle setting the group value
	CLTGUICycleCtrl_create ccs;
	ccs.rnBaseRect.m_vMin.Init();
	ccs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),nFontHeight);
	ccs.nHeaderWidth = kColumn0;
	ccs.pCommandHandler = this;

	ccs.nCommandID = CMD_CPU;
	ccs.szHelpID = "PerformanceLevel_CPU_Help";
	m_pOverallCPU = AddCycle("PerformanceLevel_CPU", ccs);
	m_pOverallCPU->AddString(LoadString("Performance_Detail_Minimum"));
	m_pOverallCPU->AddString(LoadString("IDS_LOW"));
	m_pOverallCPU->AddString(LoadString("IDS_MEDIUM"));
	m_pOverallCPU->AddString(LoadString("IDS_HIGH"));
	m_pOverallCPU->AddString(LoadString("Performance_Detail_Maximum"));
	m_pOverallCPU->AddString(LoadString("IDS_CUSTOM"));
	m_pOverallCPU->SetCycleCallback(CycleCallback);

	ccs.nCommandID = CMD_GPU;
	ccs.szHelpID = "PerformanceLevel_GPU_Help";
	ccs.rnBaseRect.m_vMin.Init();
	ccs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),nFontHeight);
	m_pOverallGPU = AddCycle("PerformanceLevel_GPU", ccs);
	m_pOverallGPU->AddString(LoadString("Performance_Detail_Minimum"));
	m_pOverallGPU->AddString(LoadString("IDS_LOW"));
	m_pOverallGPU->AddString(LoadString("IDS_MEDIUM"));
	m_pOverallGPU->AddString(LoadString("IDS_HIGH"));
	m_pOverallGPU->AddString(LoadString("Performance_Detail_Maximum"));
	m_pOverallGPU->AddString(LoadString("IDS_CUSTOM"));
	m_pOverallGPU->SetCycleCallback(CycleCallback);

	LTVector2n vPosAdvanced = g_pLayoutDB->GetPosition(m_hLayout,LDB_ScreenAdditionalPos,1);
	cs.rnBaseRect.m_vMin.x = m_ScreenRect.Left() + vPosAdvanced.x;
	cs.rnBaseRect.m_vMin.y = m_ScreenRect.Top() + vPosAdvanced.y;
	cs.rnBaseRect.m_vMax.x = m_ScreenRect.Right();
	cs.rnBaseRect.m_vMax.y = cs.rnBaseRect.m_vMin.y + nFontHeight;
	cs.nCommandID = CMD_ADVANCED_CPU;
	cs.szHelpID = "Performance_Advanced_CPU_Help";
	CLTGUITextCtrl *pAdvancedCPU = AddTextItem("Performance_Advanced_CPU", cs );

	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(kColumn0+kColumn1,nFontHeight);
	cs.nCommandID = CMD_ADVANCED_GPU;
	cs.szHelpID = "Performance_Advanced_GPU_Help";
	CLTGUITextCtrl *pAdvancedGPU = AddTextItem("Performance_Advanced_GPU", cs );

	// Don't show the test performance option if this is the mpfree product.  The
	// test performance level is a sp level and mpfree can only run mp levels.
	if( !LTGameCfg::IsMPFreeProduct())
	{
		LTVector2n vPos = g_pLayoutDB->GetPosition(m_hLayout,LDB_ScreenAdditionalPos,0);
		cs.rnBaseRect.m_vMin.x = vPos.x - kColumn0;
		cs.rnBaseRect.m_vMin.y = vPos.y;
		cs.rnBaseRect.m_vMax.x = vPos.x;
		cs.rnBaseRect.m_vMax.y = vPos.y + nFontHeight;
		cs.nCommandID = CMD_PERFORMANCE;
		cs.szHelpID = "IDS_HELP_PERFORMANCE_TEST";
		CLTGUITextCtrl* pTxt = AddTextItem("IDS_PERFORMANCE_TEST", cs );
		pTxt->SetAlignment(kRight);
	}

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return false;

	UseBack(true,true);
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformance::OnCommand()
//
//	PURPOSE:	handle user input
//
// ----------------------------------------------------------------------- //
uint32 CScreenPerformance::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_ADVANCED_CPU:
		m_pScreenMgr->SetCurrentScreen(SCREEN_ID_PERFORMANCE_CPU);
		break;

	case CMD_ADVANCED_GPU:
		m_pScreenMgr->SetCurrentScreen(SCREEN_ID_PERFORMANCE_GPU);
		break;

	case CMD_AUTO:
	{
		if( !CPerformanceMgr::Instance().ArePerformanceStatsValid() )
		{
			// KLS 2/1/05 - Explain to user what's happening...
			g_pInterfaceResMgr->DrawMessage("PerformanceMessage_Detect");
			CPerformanceMgr::Instance().DetectPerformanceStats();
		}

		SetBasedOnPerformanceStats();
		UpdateGPUColor();
	}break;

	case CMD_PERFORMANCE:
	{
		// KLS 2/1/05 - Can only start a performance test if not currently in a level...

		if (g_pLTClient->IsConnected())
		{
			MBCreate mb;
			mb.eType = LTMB_OK;
			g_pInterfaceMgr->ShowMessageBox("IDS_PERFORMANCE_ERROR_IN_LEVEL", &mb);
		}
		else
		{

			// If running MP, then ask user to switch to SP exe to run the performance test level.
			if( IsMultiplayerGameClient( ))
			{
				MBCreate mb;
				mb.eType = LTMB_YESNO;
				mb.pFn = SwitchToSinglePlayerCallBack;
				mb.pUserData = this;
				g_pInterfaceMgr->ShowMessageBox("ScreenMain_SwitchToSinglePlayer",&mb);
			}
			else
			{
				LaunchPerformanceTest();
			}
			break;

		}
	} break;

	case CMD_CPU:
		{
			DetailLevel eLevel = (DetailLevel)m_pOverallCPU->GetSelIndex();
			SetOverall(ePT_CPU,eLevel);
		} break;
	case CMD_GPU:
		{
			DetailLevel eLevel = (DetailLevel)m_pOverallGPU->GetSelIndex();
			SetOverall(ePT_GPU,eLevel);
			CheckResolutionMemory();
			UpdateGPUColor();
		} break;

	case CMD_SWITCH_TO_SINGLE:
		{
			// Switch to the SP executable.
			if( !LaunchApplication::LaunchSinglePlayerExe( LaunchApplication::kSwitchToScreen_Performance ))
				return false;
		}

	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformance::OnFocus()
//
//	PURPOSE:	join/leave screen
//
// ----------------------------------------------------------------------- //
void CScreenPerformance::OnFocus(bool bFocus)
{
	if (bFocus)
	{
#ifndef _FINAL
		CPerformanceMgr::Instance().PrintCurrentValues();
#endif // _FINAL

		if (g_pGameClientShell->IsRunningPerformanceTest())
		{
			// Make sure we're disconnected from server.
			if(g_pLTClient->IsConnected())
			{
				g_pInterfaceMgr->SetIntentionalDisconnect( true );
				g_pClientConnectionMgr->ForceDisconnect();
			}

			g_pGameClientShell->StopPerformanceTest();

			//display results...
			CPerformanceTest* pTest = g_pGameClientShell->GetLastPerformanceTest();
			if (pTest)
			{
				wchar_t wszMsgBuffer[512];
				FormatString("IDS_PERFORMANCE_RESULTS", wszMsgBuffer, LTARRAYSIZE(wszMsgBuffer), 
					pTest->GetAveFPS(), pTest->GetMaxFPS(), pTest->GetMinFPS(),
					pTest->GetPercentFPSBelowMin(), pTest->GetMinTestFPS(),
					pTest->GetPercentFPSMintoMax(), pTest->GetMinTestFPS(), pTest->GetMaxTestFPS(),
					pTest->GetPercentFPSAboveMax(), pTest->GetMaxTestFPS() );

				MBCreate mb;
				mb.eType = LTMB_OK;
				g_pInterfaceMgr->ShowMessageBox(wszMsgBuffer, &mb);
			}
		}

		Load();
		UpdateData(false);

		// Rebuild our history
		if (m_pScreenMgr->GetLastScreenID() == SCREEN_ID_NONE)
		{
			m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MAIN);
			m_pScreenMgr->AddScreenToHistory(SCREEN_ID_OPTIONS);
		}

		if( !CPerformanceMgr::Instance().ArePerformanceStatsValid() )
		{
			g_pInterfaceResMgr->DrawMessage("PerformanceMessage_Detect");
			CPerformanceMgr::Instance().DetectPerformanceStats();
		}

		UpdateGPUColor();

		// Check if we're coming here from a multiplayer switch to run the performance test.
		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVariable("screen");
		if( hVar )
		{
			// Look through the list of avaiable menus that can be switched to.
			const char* pszScreen = g_pLTClient->GetConsoleVariableString( hVar );
			if( LTStrIEquals( pszScreen, "performance" ))
			{
				// Clear out the console variable since we're processing it now.
				g_pLTClient->SetConsoleVariableString( "screen", "" );

				// Launch the performance test level now.
				LaunchPerformanceTest();
			}
		}
	}
	else
	{
		UpdateData();
		Save();
	}
	CBaseScreen::OnFocus(bFocus);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformance::Load()
//
//	PURPOSE:	read in the settings and apply them to the controls
//
// ----------------------------------------------------------------------- //
void CScreenPerformance::Load()
{
	uint32 nWidth = GetConsoleInt("ScreenWidth",640);
	uint32 nHeight = GetConsoleInt("ScreenHeight",480);
	WriteConsoleInt("Performance_ScreenWidth",nWidth);
	WriteConsoleInt("Performance_ScreenHeight",nHeight);

	for (uint32 nType = 0; nType < kNumPerformanceTypes; nType++)
	{
		UpdateOverall(nType);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformance::Save()
//
//	PURPOSE:	get settings from controls, and apply them
//
// ----------------------------------------------------------------------- //
void CScreenPerformance::Save()
{
	if( m_bTrueExit )
	{
		CPerformanceMgr::Instance().ApplyQueuedConsoleChanges(true);
		g_pProfileMgr->GetCurrentProfile()->Save();
	}

	m_bTrueExit = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformance::SetBasedOnPerformanceStats()
//
//	PURPOSE:	sets the controls based on the users performance stats
//
// ----------------------------------------------------------------------- //
void CScreenPerformance::SetBasedOnPerformanceStats()
{
	CPerformanceMgr::Instance().SetBasedOnPerformanceStats();
	Repair();
	UpdateOverall( ePT_CPU );	
	UpdateOverall( ePT_GPU );	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformance::SetOverall()
//
//	PURPOSE:	set all controls based on an overall level
//
// ----------------------------------------------------------------------- //
void CScreenPerformance::SetOverall(uint32 nType, DetailLevel eLevel)
{
	if (nType >= kNumPerformanceTypes) return;

	if (nType == ePT_CPU)
	{
		m_pOverallCPU->SetSelIndex( (uint8)eLevel );
	}
	else
	{
		m_pOverallGPU->SetSelIndex( (uint8)eLevel );
	}
	
	CPerformanceMgr::Instance().SetDetailLevel(nType, eLevel);

	Repair();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformance::UpdateOverall()
//
//	PURPOSE:	update the overall control based on the values of the settings/groups
//
// ----------------------------------------------------------------------- //
void CScreenPerformance::UpdateOverall(uint32 nType)
{
	if (nType >= kNumPerformanceTypes) return;

	const DetailLevel eOvrLevel = CPerformanceMgr::Instance().GetDetailLevel(nType);

	//	DebugCPrint(0,"CScreenPerformance::UpdateOverall to %d",eOvrLevel);
	if (nType == ePT_CPU)
	{
		m_pOverallCPU->SetSelIndex((uint8)eOvrLevel);
	}
	else
	{
		m_pOverallGPU->SetSelIndex((uint8)eOvrLevel);
	}
	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformance::CycleCallback
//
//	PURPOSE:	prevent cycle controls from selecting "Custom"
//
// ----------------------------------------------------------------------- //
uint8 CScreenPerformance::CycleCallback(CLTGUICycleCtrl* pCycle, uint8 nCurIndex, bool bIncrement )
{

	uint8 nCustomIndex = (pCycle->GetNumStrings() - 1);
	if (bIncrement) 
	{
		nCurIndex++;
		if (nCurIndex >= nCustomIndex)
			nCurIndex = 0;
	}
	else
	{
		if (nCurIndex == 0)
		{
			nCurIndex = (nCustomIndex - 1);
		}
		else
			nCurIndex--;
	}

	return nCurIndex;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::Escape
//
//	PURPOSE:	called when the user escapes out of the screen
//
// ----------------------------------------------------------------------- //
void CScreenPerformance::Escape()
{
	m_bTrueExit = true;
	CBaseScreen::Escape();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::Repair
//
//	PURPOSE:	repairs values if they are invalid
//
// ----------------------------------------------------------------------- //
void CScreenPerformance::Repair()
{
	CScreenPerformanceAdvanced* pCPU = (CScreenPerformanceAdvanced*)m_pScreenMgr->GetScreenFromID(SCREEN_ID_PERFORMANCE_CPU);
	if( pCPU )
		pCPU->Repair();

	CScreenPerformanceAdvanced* pGPU = (CScreenPerformanceAdvanced*)m_pScreenMgr->GetScreenFromID(SCREEN_ID_PERFORMANCE_GPU);
	if( pGPU )
		pGPU->Repair();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformance::CheckResolutionMemory()
//
//	PURPOSE:	checks to see if the user has blown their video memory limit
//
// ----------------------------------------------------------------------- //
void CScreenPerformance::CheckResolutionMemory()
{
	CScreenPerformanceGPU* pGPU = (CScreenPerformanceGPU*)m_pScreenMgr->GetScreenFromID(SCREEN_ID_PERFORMANCE_GPU);
	if( pGPU )
		pGPU->CheckResolutionMemory();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformance::UpdateGPUColor()
//
//	PURPOSE:	update the color of the global GPU option based on memory usage
//
// ----------------------------------------------------------------------- //
void CScreenPerformance::UpdateGPUColor()
{
	if( m_pOverallGPU && CPerformanceMgr::Instance().ArePerformanceCapsValid() )
	{
		float fMemoryUsed	= CPerformanceMgr::Instance().EstimateVideoMemoryUsage();
		float fMemoryTotal	= (float)CPerformanceMgr::Instance().GetPerformanceStats().m_nGPUMemory;
		if( fMemoryUsed > fMemoryTotal )
			m_pOverallGPU->SetColor( m_nWarningColor );
		else
			m_pOverallGPU->SetColor( m_NonSelectedColor );
	}
}

// Launches the performance test level.
void CScreenPerformance::LaunchPerformanceTest( )
{
	UpdateData();
	CPerformanceMgr::Instance().ApplyQueuedConsoleChanges(true);
	g_pProfileMgr->GetCurrentProfile()->Save();

	g_pGameClientShell->StartPerformanceTest();
	if (!g_pMissionMgr->StartPerformanceLevel())
	{
		LTERROR("Performace Test Level failed!");
		g_pGameClientShell->StopPerformanceTest();
	}
}