// FolderHost.cpp: implementation of the CFolderHost class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderHost.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "VarTrack.h"
#include "CommonUtilities.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;



VarTrack	g_vtNetGameType;
VarTrack	g_vtNetSessionName;
VarTrack	g_vtNetUsePassword;
VarTrack	g_vtNetPassword;
VarTrack	g_vtNetMaxPlayers;
VarTrack	g_vtNetPort;
VarTrack	g_vtNetBandwidthSelection;
VarTrack	g_vtNetBandwidthCustom;
extern VarTrack g_vtPlayerName;

#define DEFAULT_PORT	27888

namespace
{
	char	szOldSessionName[MAX_SESSION_NAME];
	char	szOldPassword[MAX_PASSWORD];
	char	szOldPort[MAX_PASSWORD];
	char	szOldBandwidth[MAX_SESSION_NAME];

	enum eLocalCommands
	{
		CMD_EDIT_NAME = FOLDER_CMD_CUSTOM+1,
		CMD_EDIT_PASS,
		CMD_EDIT_PORT,
		CMD_TOGGLE_PASS,
		CMD_SET_OPTIONS,
		CMD_SET_LEVELS,
		CMD_EDIT_BANDWIDTH,
		CMD_LAUNCH,
	};


}

void LaunchCallBack(LTBOOL bReturn, void *pData);


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderHost::CFolderHost()
{
	m_nGameType		= DEATHMATCH;
    m_pTypeCtrl     = LTNULL;

	m_pBandwidthCycle	= LTNULL;
	m_pBandwidthGroup	= LTNULL;
	m_pBandwidthEdit	= LTNULL;
	m_pBandwidthLabel	= LTNULL;
	m_nBandwidth		= 0;

	SAFE_STRCPY(m_szSessionName,"Multiplayer NOLF");
	SAFE_STRCPY(m_szPassword,"password");
	sprintf(m_szPort,"%d",DEFAULT_PORT);
	m_bUsePassword = LTFALSE;
	m_pPassToggle = LTNULL;
	m_bReadyToLaunch = LTFALSE;

	// Read the persistant host settings.
	ReadNetHostSettings( );
}


CFolderHost::~CFolderHost()
{
}

// Build the folder
LTBOOL CFolderHost::Build()
{
	if (!g_vtNetGameType.IsInitted())
	{
        g_vtNetGameType.Init(g_pLTClient,"NetGameType",LTNULL,(float)m_nGameType);
        g_vtNetSessionName.Init(g_pLTClient,"NetSessionName",m_szSessionName,0.0f);
        g_vtNetPassword.Init(g_pLTClient,"NetPassword",m_szPassword,0.0f);
        g_vtNetUsePassword.Init(g_pLTClient,"NetUsePassword",LTNULL,0.0f);
        g_vtNetPort.Init(g_pLTClient,"NetPort",LTNULL,(LTFLOAT)DEFAULT_PORT);
        g_vtNetBandwidthSelection.Init(g_pLTClient,"NetBandwidthSelection",LTNULL,0);
        g_vtNetBandwidthCustom.Init(g_pLTClient,"NetBandwidthCustom",LTNULL,2000);
	}


	CreateTitle(IDS_TITLE_HOST);

	m_pLabel = CreateTextItem(IDS_HOST_NAME, CMD_EDIT_NAME, IDS_HELP_SESSION_NAME);

	m_pEdit = CreateEditCtrl(" ", CMD_EDIT_NAME, IDS_HELP_SESSION_NAME, m_szSessionName, sizeof(m_szSessionName), 25, LTTRUE);
	m_pEdit->EnableCursor();
    m_pEdit->Enable(LTFALSE);
	m_pEdit->SetAlignment(LTF_JUSTIFY_CENTER);

	m_pNameGroup = AddGroup(640,m_pLabel->GetHeight(),IDS_HELP_SESSION_NAME);

    LTIntPt offset(0,0);
    m_pNameGroup->AddControl(m_pLabel,offset,LTTRUE);
	offset.x = 200;
    m_pNameGroup->AddControl(m_pEdit,offset,LTFALSE);


	m_pTypeCtrl = AddCycleItem(IDS_GAME_TYPE,IDS_HELP_GAME_TYPE,200,25,&m_nGameType);
	m_pTypeCtrl->AddString(IDS_SPACER);
	m_pTypeCtrl->AddString(IDS_COOPERATIVE_ASSAULT);
	m_pTypeCtrl->AddString(IDS_DEATHMATCH);

	AddTextItem(IDS_HOST_OPTIONS, CMD_SET_OPTIONS, IDS_HELP_HOST_OPTIONS);
	AddTextItem(IDS_HOST_LEVELS, CMD_SET_LEVELS, IDS_HELP_HOST_LEVELS);

	m_pPassToggle = AddToggle(IDS_USE_PASSWORD,IDS_HELP_PASSWORD,225,&m_bUsePassword);
	m_pPassToggle->SetOnString(IDS_ON);
	m_pPassToggle->SetOffString(IDS_OFF);
	m_pPassToggle->NotifyOnChange(CMD_TOGGLE_PASS,this);

	m_pPassLabel = CreateTextItem(IDS_PASSWORD, CMD_EDIT_PASS, IDS_HELP_ENTER_PASSWORD);

	m_pPassEdit = CreateEditCtrl(" ", CMD_EDIT_PASS, IDS_HELP_ENTER_PASSWORD, m_szPassword, sizeof(m_szPassword), 25, LTTRUE);
	m_pPassEdit->EnableCursor();
    m_pPassEdit->Enable(LTFALSE);
	m_pPassEdit->SetAlignment(LTF_JUSTIFY_CENTER);

	m_pPassGroup = AddGroup(640,m_pPassLabel->GetHeight(),IDS_HELP_ENTER_PASSWORD);

    offset = LTIntPt(0,0);
    m_pPassGroup->AddControl(m_pPassLabel,offset,LTTRUE);
	offset.x = 200;
    m_pPassGroup->AddControl(m_pPassEdit,offset,LTFALSE);

	m_pPortLabel = CreateTextItem(IDS_PORT, CMD_EDIT_PORT, IDS_HELP_ENTER_PORT);

	m_pPortEdit = CreateEditCtrl(" ", CMD_EDIT_PORT, IDS_HELP_ENTER_PORT, m_szPort, sizeof(m_szPort), 25, LTTRUE);
	m_pPortEdit->EnableCursor();
    m_pPortEdit->SetAlignment(LTF_JUSTIFY_CENTER);

	m_pPortGroup = AddGroup(640,m_pPortLabel->GetHeight(),IDS_HELP_ENTER_PORT);

    offset = LTIntPt(0,0);
    m_pPortGroup->AddControl(m_pPortLabel,offset,LTTRUE);
	offset.x = 200;
    m_pPortGroup->AddControl(m_pPortEdit,offset,LTFALSE);


	
	CLTGUITextItemCtrl *pCtrl = CreateTextItem(IDS_HOST_LAUNCH, CMD_LAUNCH, IDS_HELP_LAUNCH,LTFALSE,GetLargeFont());
	LTIntPt pos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_HOST,"LaunchPos");
	AddFixedControl(pCtrl,pos);

	m_pBandwidthCycle = AddCycleItem(IDS_BANDWIDTH_CYCLE,IDS_HELP_BANDWIDTH_CYCLE,200,25,&m_nBandwidth);
	m_pBandwidthCycle->AddString(IDS_56K);
	m_pBandwidthCycle->AddString(IDS_CABLE);
	m_pBandwidthCycle->AddString(IDS_DSL);
	m_pBandwidthCycle->AddString(IDS_T1);
	m_pBandwidthCycle->AddString(IDS_T3);
	m_pBandwidthCycle->AddString(IDS_CUSTOM);

	m_pBandwidthLabel = CreateTextItem(IDS_BANDWIDTH_EDIT, CMD_EDIT_BANDWIDTH, IDS_HELP_BANDWIDTH_EDIT);

	m_pBandwidthEdit = CreateEditCtrl(" ", CMD_EDIT_BANDWIDTH, IDS_HELP_BANDWIDTH_EDIT, m_szBandwidth, sizeof(m_szBandwidth), 25, LTTRUE);
	m_pBandwidthEdit->EnableCursor();
    m_pBandwidthEdit->SetAlignment(LTF_JUSTIFY_CENTER);

	m_pBandwidthGroup = AddGroup(640,m_pBandwidthLabel->GetHeight(),IDS_HELP_BANDWIDTH_EDIT);

    offset = LTIntPt(0,0);
    m_pBandwidthGroup->AddControl(m_pBandwidthLabel,offset,LTTRUE);
	offset.x = 200;
    m_pBandwidthGroup->AddControl(m_pBandwidthEdit,offset,LTFALSE);

	UpdateBandwidth();

 	// Make sure to call the base class
	return CBaseFolder::Build();
}

uint32 CFolderHost::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_EDIT_NAME:
		{
			if (GetCapture())
			{
                SetCapture(LTNULL);
				m_pEdit->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
                m_pEdit->Select(LTFALSE);
                m_pLabel->Select(LTTRUE);
				ForceMouseUpdate();
			}
			else
			{
				strcpy(szOldSessionName,m_szSessionName);
				SetCapture(m_pEdit);
				m_pEdit->SetColor(m_hSelectedColor,m_hSelectedColor,m_hSelectedColor);
                m_pEdit->Select(LTTRUE);
                m_pLabel->Select(LTFALSE);
			}
		} break;
	case CMD_EDIT_PASS:
		{
			if (GetCapture())
			{
                SetCapture(LTNULL);
				m_pPassEdit->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
                m_pPassEdit->Select(LTFALSE);
                m_pPassLabel->Select(LTTRUE);
				ForceMouseUpdate();
			}
			else
			{
				strcpy(szOldPassword,m_szPassword);
				SetCapture(m_pPassEdit);
				m_pPassEdit->SetColor(m_hSelectedColor,m_hSelectedColor,m_hSelectedColor);
                m_pPassEdit->Select(LTTRUE);
                m_pPassLabel->Select(LTFALSE);
			}
		} break;
	case CMD_EDIT_PORT:
		{
			if (GetCapture())
			{
                SetCapture(LTNULL);
				m_pPortEdit->UpdateData();
				uint16 nPort = (uint16)atoi(m_szPort);
				if (nPort == 0 || !IsValidPort(nPort))
				{
					strcpy(m_szPort,szOldPort);
					m_pPortEdit->UpdateData(LTFALSE);
				}
				
				m_pPortEdit->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
                m_pPortEdit->Select(LTFALSE);
                m_pPortLabel->Select(LTTRUE);
				ForceMouseUpdate();
			}
			else
			{
				strcpy(szOldPort,m_szPort);
				SetCapture(m_pPortEdit);
				m_pPortEdit->SetColor(m_hSelectedColor,m_hSelectedColor,m_hSelectedColor);
                m_pPortEdit->Select(LTTRUE);
                m_pPortLabel->Select(LTFALSE);
			}
		} break;
	case CMD_EDIT_BANDWIDTH:
		{
			if (GetCapture())
			{
                SetCapture(LTNULL);
				m_pBandwidthEdit->UpdateData();
				uint32 nBandwidth = (uint32)atoi(m_szBandwidth);
				if ( !IsValidBandwidth(nBandwidth) )
				{
					strcpy(m_szBandwidth,szOldBandwidth);
					m_pBandwidthEdit->UpdateData(LTFALSE);
				}
				
				m_pBandwidthEdit->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
                m_pBandwidthEdit->Select(LTFALSE);
                m_pBandwidthLabel->Select(LTTRUE);
				g_vtNetBandwidthCustom.SetStr(m_szBandwidth);
				UpdateBandwidth();
				ForceMouseUpdate();
			}
			else
			{
				strcpy(szOldBandwidth,m_szBandwidth);
				SetCapture(m_pBandwidthEdit);
				m_pBandwidthEdit->SetColor(m_hSelectedColor,m_hSelectedColor,m_hSelectedColor);
                m_pBandwidthEdit->Select(LTTRUE);
                m_pBandwidthLabel->Select(LTFALSE);
			}
		} break;
	case CMD_TOGGLE_PASS:
		{
			m_pPassToggle->UpdateData();
			m_pPassGroup->Enable(m_bUsePassword);
		} break;
	case CMD_SET_OPTIONS:
		{
			g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_HOST_OPTIONS);
		} break;
	case CMD_SET_LEVELS:
		{
			g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_HOST_LEVELS);
		} break;
	case CMD_LAUNCH:
		{
		    if (g_pGameClientShell->IsInWorld())
		    {
                HSTRING hString = g_pLTClient->FormatString(IDS_ENDCURRENTGAME);
			    g_pInterfaceMgr->ShowMessageBox(hString,LTMB_YESNO,LaunchCallBack,this);
				g_pLTClient->FreeString(hString);
		    }
		    else
		    {
				HandleLaunch();
		    }

		} break;
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};

void	CFolderHost::Escape()
{
	if (GetCapture() == m_pEdit)
	{
        SetCapture(LTNULL);
		strcpy(m_szSessionName,szOldSessionName);
        m_pEdit->UpdateData(LTFALSE);
		m_pEdit->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
        m_pEdit->Select(LTFALSE);
        m_pLabel->Select(LTTRUE);
		ForceMouseUpdate();
	}
	else if (GetCapture() == m_pPassEdit)
	{
        SetCapture(LTNULL);
		strcpy(m_szPassword,szOldPassword);
        m_pPassEdit->UpdateData(LTFALSE);
		m_pPassEdit->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
        m_pPassEdit->Select(LTFALSE);
        m_pPassLabel->Select(LTTRUE);
		ForceMouseUpdate();
	}
	else if (GetCapture() == m_pPortEdit)
	{
        SetCapture(LTNULL);
		strcpy(m_szPort,szOldPort);
        m_pPortEdit->UpdateData(LTFALSE);
		m_pPortEdit->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
        m_pPortEdit->Select(LTFALSE);
        m_pPortLabel->Select(LTTRUE);
		ForceMouseUpdate();
	}
	else if (GetCapture() == m_pBandwidthEdit)
	{
        SetCapture(LTNULL);
		strcpy(m_szBandwidth,szOldBandwidth);
        m_pBandwidthEdit->UpdateData(LTFALSE);
		m_pBandwidthEdit->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
        m_pBandwidthEdit->Select(LTFALSE);
        m_pBandwidthLabel->Select(LTTRUE);
		ForceMouseUpdate();
	}
	else

		CBaseFolder::Escape();

}

// Change in focus
void    CFolderHost::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		SAFE_STRCPY(m_szSessionName,g_vtNetSessionName.GetStr() );
		SAFE_STRCPY(m_szPassword,g_vtNetPassword.GetStr() );
		uint16 nPort = (uint16)g_vtNetPort.GetFloat();
		sprintf(m_szPort,"%d",nPort);
		m_nBandwidth = (uint32)g_vtNetBandwidthSelection.GetFloat();
		m_bUsePassword = (g_vtNetUsePassword.GetFloat() > 0.0f);
		m_nGameType = (int)g_vtNetGameType.GetFloat();
		if (m_nGameType != DEATHMATCH && m_nGameType != COOPERATIVE_ASSAULT)
			m_nGameType = DEATHMATCH;

		m_pPassGroup->Enable(m_bUsePassword);

		UpdateBandwidth();
        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();
		g_vtNetPassword.SetStr(m_szPassword);
		g_vtNetUsePassword.WriteFloat((float)m_bUsePassword);
		g_vtNetSessionName.SetStr(m_szSessionName);
		g_vtNetGameType.WriteFloat((float)m_nGameType);

		uint16 nPort = (uint16)atoi(m_szPort);
		g_vtNetPort.WriteFloat((LTFLOAT)nPort);

		g_vtNetBandwidthSelection.WriteFloat((LTFLOAT)m_nBandwidth);

        g_pLTClient->WriteConfigFile("autoexec.cfg");

		// Write the persistant host settings.
		WriteNetHostSettings( );
	}
	CBaseFolder::OnFocus(bFocus);
}

LTBOOL CFolderHost::OnLeft()
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pTypeCtrl)
	{
		int nGameType = m_pTypeCtrl->GetSelIndex();
		--nGameType;
		if (nGameType < COOPERATIVE_ASSAULT)
			nGameType = DEATHMATCH;
		m_pTypeCtrl->SetSelIndex(nGameType);
        return LTTRUE;
	}
	else if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		--m_nBandwidth;
		if (m_nBandwidth < 0)
			m_nBandwidth = 5;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
        return LTTRUE;
	}
	return CBaseFolder::OnLeft();
}

LTBOOL CFolderHost::OnRight()
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pTypeCtrl)
	{
		int nGameType = m_pTypeCtrl->GetSelIndex();
		++nGameType;
		if (nGameType > DEATHMATCH)
			nGameType = COOPERATIVE_ASSAULT;
		m_pTypeCtrl->SetSelIndex(nGameType);
        return LTTRUE;
	}
	else if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		++m_nBandwidth;
		if (m_nBandwidth > 5)
			m_nBandwidth = 0;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
        return LTTRUE;
	}
	return CBaseFolder::OnRight();
}

/******************************************************************/
LTBOOL CFolderHost::OnLButtonUp(int x, int y)
{
	CLTGUICtrl *pCapture = GetCapture();
	if (pCapture)
	{
		return pCapture->OnEnter();
	}

	// Get the control that the click was on
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
            return LTFALSE;
		// If the mouse is over the same control now as it was when the down message was called
		// then send the "enter" message to the control.
		if (nControlIndex == m_nLMouseDownItemSel)
		{
			if (pCtrl->IsEnabled() )
			{
				if (pCtrl == m_pTypeCtrl)
				{
					int nGameType = m_pTypeCtrl->GetSelIndex();
					++nGameType;
					if (nGameType > DEATHMATCH)
						nGameType = COOPERATIVE_ASSAULT;
					m_pTypeCtrl->SetSelIndex(nGameType);
                    return LTTRUE;
				}
				else if (pCtrl == m_pBandwidthCycle)
				{
					m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
					++m_nBandwidth;
					if (m_nBandwidth > 5)
						m_nBandwidth = 0;
					m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
					UpdateBandwidth();
                    return LTTRUE;
				}
				else
				{
					SetSelection(nControlIndex);
					return CBaseFolder::OnLButtonUp(x,y);
				}
			}

		}
	}
	else
	{
		m_nLMouseDownItemSel= kNoSelection;
	}
    return LTFALSE;
}


/******************************************************************/
LTBOOL CFolderHost::OnRButtonUp(int x, int y)
{
	if (GetCapture())
	{
		Escape();
		return LTTRUE;
	}

	// Get the control that the click was on
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (m_pCaptureCtrl && pCtrl != m_pCaptureCtrl)
            return LTFALSE;

		// If the mouse is over the same control now as it was when the down message was called
		// then send the "left" message to the control.
		if (nControlIndex == m_nRMouseDownItemSel)
		{
			if (pCtrl->IsEnabled())
			{
				if (GetSelectedControl() == m_pTypeCtrl)
				{
					int nGameType = m_pTypeCtrl->GetSelIndex();
					--nGameType;
					if (nGameType < COOPERATIVE_ASSAULT)
						nGameType = DEATHMATCH;
					m_pTypeCtrl->SetSelIndex(nGameType);
                    return LTTRUE;
				}
				else if (GetSelectedControl() == m_pBandwidthCycle)
				{
					m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
					--m_nBandwidth;
					if (m_nBandwidth < 0)
						m_nBandwidth = 5;
					m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
					UpdateBandwidth();
                    return LTTRUE;
				}
				else
				{
					SetSelection(nControlIndex);
					return CBaseFolder::OnRButtonUp(x,y);
				}
			}
		}
	}
	else
	{
		m_nRMouseDownItemSel= kNoSelection;
	}
    return LTFALSE;
}

void CFolderHost::FillGameStruct(NetGame *pNG)
{
	SAFE_STRCPY(pNG->m_sSession,m_szSessionName);	

    pNG->m_byType = (uint8)m_nGameType;

	pNG->m_byNumLevels = 0;
	if (m_nGameType == COOPERATIVE_ASSAULT)
	{
        pNG->m_byNumLevels = (uint8)GetConsoleInt("NetCANumLevels", 0);
	}
	else
	{
        pNG->m_byNumLevels = (uint8)GetConsoleInt("NetNumLevels", 0);
	}
	if (pNG->m_byNumLevels <= 0) return;


	// Get each level...

    for (uint8 i = 0; i < pNG->m_byNumLevels; i++)
	{
		char sLevel[256] = { "" };
		char sLabel[32];

		if (m_nGameType == COOPERATIVE_ASSAULT)
		{
			wsprintf(sLabel, "NetCALevel%i", i);
		}
		else
		{
			wsprintf(sLabel, "NetLevel%i", i);
		}

		GetConsoleString(sLabel, pNG->m_sLevels[i], "");
	}

    pNG->m_byNumOptions = (uint8)g_pServerOptionMgr->GetNumOptions();
	if (pNG->m_byNumOptions > MAX_GAME_OPTIONS)
		pNG->m_byNumOptions = MAX_GAME_OPTIONS;


	for (i = 0; i < pNG->m_byNumOptions; i++)
	{
		OPTION* pOpt = g_pServerOptionMgr->GetOption(i);
		if (m_nGameType == pOpt->eGameType || pOpt->eGameType == SINGLE)
		{
			pNG->m_fOptions[i] = pOpt->GetValue();
		}
	}

	pNG->m_bUsePassword = m_bUsePassword;
	SAFE_STRCPY(pNG->m_sPassword,m_szPassword);	

}

void CFolderHost::HandleLaunch()
{
	m_bReadyToLaunch = LTFALSE;
	UpdateData();
	g_vtNetSessionName.SetStr(m_szSessionName);
	g_vtNetUsePassword.WriteFloat((float)m_bUsePassword);
	g_vtNetSessionName.SetStr(m_szSessionName);
	g_vtNetGameType.WriteFloat((float)m_nGameType);
	g_vtNetBandwidthSelection.WriteFloat((LTFLOAT)m_nBandwidth);
	uint16 nPort = (uint16)atoi(m_szPort);
	g_vtNetPort.WriteFloat((LTFLOAT)nPort);
	if (!IsValidPort(nPort))
		return;

	int cLevels = 0;
	if (m_nGameType == COOPERATIVE_ASSAULT)
	{
		cLevels = GetConsoleInt("NetCANumLevels", 0);
	}
	else
	{
		cLevels = GetConsoleInt("NetNumLevels", 0);
	}
	if (cLevels == 0)
	{
        HSTRING hString = g_pLTClient->FormatString(IDS_NOLEVELS);
        g_pInterfaceMgr->ShowMessageBox(hString,LTMB_OK,LTNULL,LTNULL);
		g_pLTClient->FreeString(hString);
	}
	else
	{
		if (!LaunchGame())
		{
			g_pInterfaceMgr->LoadFailed();
		}
	}
}



LTBOOL CFolderHost::LaunchGame()
{
    LTRESULT dr = g_pLTClient->InitNetworking(NULL, 0);
	if (dr != LT_OK)
	{
//		s_nErrorString = IDS_NETERR_INIT;
//		NetStart_DisplayError(hInst);
        return(LTFALSE);
	}

	if (!SetService())
        return LTFALSE;

	StartGameRequest req;
	NetGame gameInfo;
	NetClientData clientData;

	memset( &req, 0, sizeof( req ));
	memset( &gameInfo, 0, sizeof( gameInfo ));
	FillGameStruct(&gameInfo);
	// Setup our client.
	clientData.m_dwTeam = (uint32)GetConsoleInt("NetPlayerTeam",0);
	SAFE_STRCPY(clientData.m_sName,g_vtPlayerName.GetStr());

	req.m_Type = STARTGAME_HOST;
	req.m_pGameInfo = &gameInfo;
	req.m_GameInfoLen = sizeof( gameInfo );
	req.m_pClientData = &clientData;
	req.m_ClientDataLen = sizeof( clientData );

	strcpy(req.m_WorldName,gameInfo.m_sLevels[0]);

	// Setup the host info.
	uint16 nPort = (uint16)atoi(m_szPort);

	req.m_HostInfo.m_Port = nPort;
    req.m_HostInfo.m_dwMaxPlayers = (uint32)GetConsoleInt("NetMaxPlayers",16)-1;

	strcpy(req.m_HostInfo.m_sName,g_vtNetSessionName.GetStr());

	g_pGameClientShell->SetGameType( (GameType)m_nGameType );
	g_pInterfaceMgr->DrawFragCount(LTFALSE);
	char *pWorld = strrchr(req.m_WorldName,'\\');
	pWorld++;
	HSTRING hWorld = g_pLTClient->CreateString(pWorld);
	g_pInterfaceMgr->SetLoadLevelString(hWorld);
	g_pLTClient->FreeString(hWorld);

	char szPhoto[512];
	SAFE_STRCPY(szPhoto,req.m_WorldName);
	strtok(szPhoto,".");
	strcat(szPhoto,".pcx");
	g_pInterfaceMgr->SetLoadLevelPhoto(szPhoto);

	g_pInterfaceMgr->ChangeState(GS_LOADINGLEVEL);

	// Host a new game.
    if( g_pLTClient->StartGame( &req ) != LT_OK )
        return LTFALSE;

    return LTTRUE;
}


LTBOOL CFolderHost::SetService( )
{
	NetService *pCur, *pListHead;
	HNETSERVICE hNetService;

	pCur      = NULL;
	pListHead = NULL;
	hNetService = NULL;

    if( g_pLTClient->GetServiceList( pListHead ) != LT_OK || !pListHead )
        return LTFALSE;

	// Find the service specified.
	pCur = pListHead;
	while( pCur )
	{
		if( pCur->m_dwFlags & NETSERVICE_TCPIP )
		{
			hNetService = pCur->m_handle;
			break;
		}

		pCur = pCur->m_pNext;
	}

	// Free the service list.
    g_pLTClient->FreeServiceList( pListHead );

	// Check if tcp not found.
	if( !hNetService )
        return LTFALSE;

	// Select it.
    if( g_pLTClient->SelectService( hNetService ) != LT_OK )
        return LTFALSE;

    return LTTRUE;
}


void LaunchCallBack(LTBOOL bReturn, void *pData)
{
	CFolderHost *pThisFolder = (CFolderHost *)pData;
	if (bReturn && pThisFolder)
	{
		pThisFolder->ReadyLaunch(LTTRUE);
    }
}


void CFolderHost::UpdateInterfaceSFX()
{
	if (m_bReadyToLaunch)
		HandleLaunch();
}


void CFolderHost::ReadyLaunch(LTBOOL bReady)
{
	if (g_pGameClientShell->IsInWorld() && g_pGameClientShell->GetGameType() != SINGLE)
		g_pInterfaceMgr->StartingNewGame();
	m_bReadyToLaunch = bReady;
}

void CFolderHost::UpdateBandwidth()
{
	if ( m_nBandwidth == 5 )
	{
		m_pBandwidthGroup->Enable(LTTRUE);
		m_pBandwidthEdit->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
        m_pBandwidthEdit->Select(LTFALSE);

		const char* szCustom = g_vtNetBandwidthCustom.GetStr();
		char szBandwidth[128];
		sprintf(szBandwidth, "%d", (int)atof(szCustom));
        m_pBandwidthEdit->SetText(szBandwidth);

		WriteConsoleString("SendBandwidth", szBandwidth);
		strcpy(m_szBandwidth, szBandwidth);
	}
	else
	{
		m_pBandwidthGroup->Enable(LTFALSE);
		m_pBandwidthEdit->SetColor(m_hDisabledColor,m_hDisabledColor,m_hDisabledColor);
        m_pBandwidthEdit->Select(LTFALSE);

	    static char* s_aszBandwidths[5] = { "4000", "16000", "32000", "1000000", "10000000" };
		WriteConsoleString("SendBandwidth", s_aszBandwidths[m_nBandwidth]);
        m_pBandwidthEdit->SetText(s_aszBandwidths[m_nBandwidth]);
		strcpy(m_szBandwidth, s_aszBandwidths[m_nBandwidth]);
	}
}
