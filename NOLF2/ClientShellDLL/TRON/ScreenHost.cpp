// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHost.cpp
//
// PURPOSE : Interface screen for hosting multi player games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenHost.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "CommonUtilities.h"
#include "ClientRes.h"
#include "MissionMgr.h"
#include "clientmultiplayermgr.h"
#include "WinUtil.h"
#include "ClientSaveLoadMgr.h"
#include "GameClientShell.h"


static const int kMaxPortStrLen = 5;
static const int kMaxBandwidthStrLen = 7;


namespace
{
	void EditNameCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenHost *pThisScreen = (CScreenHost *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_HOST);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_NAME);
	};
	void EditPassCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenHost *pThisScreen = (CScreenHost *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_HOST);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_PASS);
	};
	void EditPortCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenHost *pThisScreen = (CScreenHost *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_HOST);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_PORT);
	};
	void EditBandwidthCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenHost *pThisScreen = (CScreenHost *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_HOST);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_BANDWIDTH);
	};

}

void LaunchCallBack(LTBOOL bReturn, void *pData);


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenHost::CScreenHost()
{
	m_pName			= LTNULL;
	m_pCampaign		= LTNULL;

//    m_pLoadCtrl = LTNULL;

	m_pPassword		= LTNULL;
	m_pPassToggle	= LTNULL;

	m_pPort			= LTNULL;

	m_pBandwidthCycle	= LTNULL;
	m_pBandwidth		= LTNULL;
	m_nBandwidth		= 0;

	LoadString( IDS_HOST_NAME_DEFAULT, m_sSessionName.GetBuffer( MAX_SESSION_NAME ), MAX_SESSION_NAME );
	m_sSessionName.ReleaseBuffer( );
	LoadString( IDS_PASSWORD_DEFAULT, m_sPassword.GetBuffer( MAX_PASSWORD ), MAX_PASSWORD );
	m_sPassword.ReleaseBuffer( );

	m_sCampaignName = DEFAULT_CAMPAIGN;

	m_sPort.Format( "%d", DEFAULT_PORT );
	m_bUsePassword = LTFALSE;
	m_bReadyToLaunch = LTFALSE;

	// Read the persistant host settings.
	ReadNetHostSettings( );
}


CScreenHost::~CScreenHost()
{
}

// Build the screen
LTBOOL CScreenHost::Build()
{
	int kColumn0 = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_HOST,"ColumnWidth");
	int kColumn1 = (640 - GetPageLeft()) - kColumn0;

	CreateTitle(IDS_TITLE_HOST);

	m_pName = AddColumnCtrl(CMD_EDIT_NAME, IDS_HELP_SESSION_NAME);
	m_pName->AddColumn(LoadTempString(IDS_HOST_NAME), kColumn0);
	m_pName->AddColumn("<host name>", kColumn1, LTTRUE);

//	m_pLoadCtrl = AddTextItem(IDS_LOADGAME,	CMD_LOAD_GAME, IDS_HELP_LOAD);

	m_pCampaign = AddColumnCtrl(CMD_CHOOSE_CAMPAIGN, IDS_HELP_CAMPAIGN_NAME);
	m_pCampaign->AddColumn(LoadTempString(IDS_HOST_CAMPAIGN), kColumn0);
	m_pCampaign->AddColumn("<mission name>", kColumn1, LTTRUE);

//	AddTextItem(IDS_HOST_OPTIONS, CMD_SET_OPTIONS, IDS_HELP_HOST_OPTIONS);

	m_pPassToggle = AddToggle(IDS_USE_PASSWORD,IDS_HELP_PASSWORD,kColumn0,&m_bUsePassword);
	m_pPassToggle->NotifyOnChange(CMD_TOGGLE_PASS,this);

	m_pPassword = AddColumnCtrl(CMD_EDIT_PASS, IDS_HELP_ENTER_PASSWORD);
	m_pPassword->AddColumn(LoadTempString(IDS_PASSWORD), kColumn0);
	m_pPassword->AddColumn("<password>", kColumn1, LTTRUE);

	m_pPort = AddColumnCtrl(CMD_EDIT_PORT, IDS_HELP_ENTER_PORT);
	m_pPort->AddColumn(LoadTempString(IDS_PORT), kColumn0);
	m_pPort->AddColumn("<port>", kColumn1, LTTRUE);

	m_pBandwidthCycle = AddCycle(IDS_BANDWIDTH_CYCLE,IDS_HELP_BANDWIDTH_CYCLE,200,&m_nBandwidth);
	m_pBandwidthCycle->AddString(LoadTempString(IDS_56K));
	m_pBandwidthCycle->AddString(LoadTempString(IDS_CABLE));
	m_pBandwidthCycle->AddString(LoadTempString(IDS_DSL));
	m_pBandwidthCycle->AddString(LoadTempString(IDS_T1));
	m_pBandwidthCycle->AddString(LoadTempString(IDS_T3));
	m_pBandwidthCycle->AddString(LoadTempString(IDS_CUSTOM));

	m_pBandwidth = AddColumnCtrl(CMD_EDIT_BANDWIDTH, IDS_HELP_BANDWIDTH_EDIT);
	m_pBandwidth->AddColumn(LoadTempString(IDS_BANDWIDTH_EDIT), kColumn0);
	m_pBandwidth->AddColumn("<bandwidth>", kColumn1, LTTRUE);

	LTIntPt pos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_HOST,"LaunchPos");
	uint8 nFont = g_pLayoutMgr->GetBackFont();
	uint8 nFontSize = g_pLayoutMgr->GetBackSize();
	CLTGUITextCtrl* pCtrl = AddTextItem(IDS_HOST_LAUNCH, CMD_LAUNCH, IDS_HELP_LAUNCH,pos,LTFALSE,nFont);
	pCtrl->SetFont(LTNULL,nFontSize);

 	// Make sure to call the base class
	return CBaseScreen::Build();
}

uint32 CScreenHost::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == CMD_OK)
	{
		HandleCallback(dwParam1,dwParam2);
		return 1;
	}
	switch(dwCommand)
	{
	case CMD_EDIT_NAME:
		{
			//show edit box here	
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditNameCallBack;
			mb.pString = m_sSessionName;
			mb.nMaxChars = MAX_SESSION_NAME-1;
			g_pInterfaceMgr->ShowMessageBox(IDS_HOST_NAME,&mb);
		} break;
	case CMD_LOAD_GAME:
		{
			// Setup the host info.
			uint16 nPort = (uint16)atoi(m_sPort);
			if( g_pClientMultiplayerMgr->SetupServerHost( nPort ))
				g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_LOAD);
			break;
		}
	case CMD_EDIT_PASS:
		{
			//show edit box here	
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditPassCallBack;
			mb.pString = m_sPassword;
			mb.nMaxChars = MAX_PASSWORD-1;
			g_pInterfaceMgr->ShowMessageBox(IDS_PASSWORD,&mb);
		} break;
	case CMD_EDIT_PORT:
		{
			//show edit box here	
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditPortCallBack;
			mb.pString = m_sPort;
			mb.nMaxChars = kMaxPortStrLen;
			mb.eInput = CLTGUIEditCtrl::kInputNumberOnly;
			g_pInterfaceMgr->ShowMessageBox(IDS_PORT,&mb);
		} break;
	case CMD_EDIT_BANDWIDTH:
		{
			//show edit box here	
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditBandwidthCallBack;
			mb.pString = m_sBandwidth;
			mb.nMaxChars = kMaxBandwidthStrLen;
			mb.eInput = CLTGUIEditCtrl::kInputNumberOnly;
			g_pInterfaceMgr->ShowMessageBox(IDS_BANDWIDTH_EDIT,&mb);
		} break;
	case CMD_TOGGLE_PASS:
		{
			m_pPassToggle->UpdateData();
			m_pPassword->Enable(m_bUsePassword);
		} break;
	case CMD_SET_OPTIONS:
		{
//			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_OPTIONS);
		} break;
	case CMD_CHOOSE_CAMPAIGN:
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_MISSION);
		} break;
	case CMD_LAUNCH:
		{

		    if (g_pGameClientShell->IsWorldLoaded())
		    {
				MBCreate mb;
				mb.eType = LTMB_YESNO;
				mb.pFn = LaunchCallBack;
				mb.pData = this;
			    g_pInterfaceMgr->ShowMessageBox(IDS_ENDCURRENTGAME,&mb);
		    }
		    else
		    {
				HandleLaunch();
		    }

		} break;
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};

// Change in focus
void    CScreenHost::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
		
	if (bFocus)
	{
//		m_pLoadCtrl->Enable( g_pClientSaveLoadMgr->ReloadSaveExists() );

		m_sSessionName = pProfile->m_sSessionName.c_str();
		m_sSessionName = m_sSessionName.Left( MAX_SESSION_NAME );

		
		bool isDefault = (pProfile->m_sCampaignName.compare(DEFAULT_CAMPAIGN) == 0);
		if (!isDefault)
		{
			std::string sCampaignFile = g_pProfileMgr->GetCurrentCampaignFile();

			if (!CWinUtil::FileExist(sCampaignFile.c_str()))
			{
				pProfile->m_sCampaignName = DEFAULT_CAMPAIGN;
				isDefault = true;
			}
		}
		
		m_sCampaignName = pProfile->m_sCampaignName.c_str();
		m_sCampaignName = m_sCampaignName.Left( MAX_SESSION_NAME );



		m_sPassword = pProfile->m_sPassword.c_str();
		m_sPassword = m_sPassword.Left( MAX_PASSWORD );
		uint16 nPort = pProfile->m_nPort;
		m_sPort.Format( "%d", nPort );
		m_nBandwidth = pProfile->m_nBandwidthSelection;
		m_bUsePassword = !!(pProfile->m_bUsePassword);

		m_pPassword->Enable(m_bUsePassword);

		m_pName->SetString(1,m_sSessionName);

		m_pPassword->SetString(1,m_sPassword);
		m_pPort->SetString(1,m_sPort);


		UpdateBandwidth();
        UpdateData(LTFALSE);
/*
		// Initialize to the coop mission bute.
		if( !g_pMissionButeMgr->Init( MISSION_COOP_FILE ))
		{
			g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.", MISSION_COOP_FILE );
			return;
  		}


		if (isDefault)
			m_pCampaign->SetString(1,LoadTempString(IDS_HOST_CAMPAIGN_DEFAULT));
		else
			m_pCampaign->SetString(1,m_sCampaignName);
*/

		g_pGameClientShell->SetGameType( eGameTypeCooperative );
	}
	else
	{
		UpdateData();

		pProfile->m_sSessionName = m_sSessionName;
//		pProfile->m_sCampaignName = m_sCampaignName;
		pProfile->m_sPassword = m_sPassword;

		pProfile->m_nPort = (uint16)atoi(m_sPort);
		pProfile->m_nBandwidthSelection = m_nBandwidth;
		pProfile->m_bUsePassword = !!m_bUsePassword;

		pProfile->Save();

		// Write the persistant host settings.
		WriteNetHostSettings( );
	}
	CBaseScreen::OnFocus(bFocus);
}

LTBOOL CScreenHost::OnLeft()
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		--m_nBandwidth;
		if (m_nBandwidth < 0)
			m_nBandwidth = 5;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
        return LTTRUE;
	}
	return CBaseScreen::OnLeft();
}

LTBOOL CScreenHost::OnRight()
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		++m_nBandwidth;
		if (m_nBandwidth > 5)
			m_nBandwidth = 0;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
        return LTTRUE;
	}
	return CBaseScreen::OnRight();
}

/******************************************************************/
LTBOOL CScreenHost::OnLButtonUp(int x, int y)
{
	if (!CBaseScreen::OnLButtonUp(x,y))
		return LTFALSE;
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		UpdateBandwidth();
	}
	return LTTRUE;
}


/******************************************************************/
LTBOOL CScreenHost::OnRButtonUp(int x, int y)
{
	if (!CBaseScreen::OnRButtonUp(x,y))
		return LTFALSE;
	CLTGUICtrl *pCtrl = GetSelectedControl();

	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		UpdateBandwidth();
	}
	return LTTRUE;
}

void CScreenHost::HandleLaunch()
{
	m_bReadyToLaunch = LTFALSE;
	UpdateData();
	uint16 nPort = (uint16)atoi(m_sPort);
	if (!IsValidPort(nPort))
		return;
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	pProfile->m_sSessionName = m_sSessionName;
	pProfile->m_sCampaignName = m_sCampaignName;
	pProfile->m_sPassword = m_sPassword;
	pProfile->m_nPort = nPort;
	pProfile->m_nBandwidthSelection = m_nBandwidth;
	pProfile->m_nBandwidthTarget = (uint32)atol(m_sBandwidth);
	pProfile->m_bUsePassword = !!m_bUsePassword;

	pProfile->Save();

	if (!LaunchGame())
	{
		g_pInterfaceMgr->LoadFailed();
	}
}



LTBOOL CScreenHost::LaunchGame()
{
	// Setup the host info.
	uint16 nPort = (uint16)atoi(m_sPort);

	if( !g_pClientMultiplayerMgr->SetupServerHost( nPort ))
        return LTFALSE;
	if( !g_pMissionMgr->StartGameNew( ))
        return LTFALSE;

    return LTTRUE;
}


void LaunchCallBack(LTBOOL bReturn, void *pData)
{
	CScreenHost *pThisScreen = (CScreenHost *)pData;
	if (bReturn && pThisScreen)
	{
		pThisScreen->ReadyLaunch(LTTRUE);
    }
}


bool CScreenHost::UpdateInterfaceSFX()
{
	if (m_bReadyToLaunch)
		HandleLaunch();

	return true;
}


void CScreenHost::ReadyLaunch(LTBOOL bReady)
{
	m_bReadyToLaunch = bReady;
}

void CScreenHost::UpdateBandwidth()
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if ( m_nBandwidth == 5 )
	{
		m_sBandwidth.Format( "%d", pProfile->m_nBandwidthTarget);
		m_pBandwidth->SetString(1,m_sBandwidth);
	}
	else
	{
	    static char* s_aszBandwidths[5] = { "56000", "256000", "512000", "1500000", "10000000" };
		m_sBandwidth = s_aszBandwidths[m_nBandwidth];
		m_pBandwidth->SetString(1,m_sBandwidth);
	}
}


void CScreenHost::HandleCallback(uint32 dwParam1, uint32 dwParam2)
{
	switch(dwParam2)
	{
	case CMD_EDIT_NAME:
		m_sSessionName = ((char *)dwParam1);
		m_sSessionName = m_sSessionName.Left( MAX_SESSION_NAME );
		m_pName->SetString(1,m_sSessionName);
		break;
	case CMD_EDIT_PASS:
		m_sPassword = ((char *)dwParam1);
		m_sPassword = m_sPassword.Left( MAX_PASSWORD );
		m_pPassword->SetString(1,m_sPassword);
		break;
	case CMD_EDIT_PORT:
		{
			char *pszPort = (char *)dwParam1;
			uint16 nPort = (uint16)atoi(pszPort);
			if (IsValidPort(nPort))
			{
				m_sPort = pszPort;
				m_sPort = m_sPort.Left( kMaxPortStrLen );
				m_pPort->SetString(1,m_sPort);
			}
		}
		break;
	case CMD_EDIT_BANDWIDTH:
		{
			char *pszBandwidth = (char *)dwParam1;
			uint32 nBandwidth = (uint32)atoi(pszBandwidth);
			if ( IsValidBandwidth(nBandwidth) )
			{
				m_sBandwidth = pszBandwidth;
				m_sBandwidth = m_sBandwidth.Left( kMaxBandwidthStrLen );
				m_nBandwidth = 5;
				CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
				pProfile->m_nBandwidthTarget = (uint32)atol(m_sBandwidth);
				m_pBandwidthCycle->UpdateData(LTFALSE);
				UpdateBandwidth();
			}
		}
		break;
	}
	UpdateData();
}
