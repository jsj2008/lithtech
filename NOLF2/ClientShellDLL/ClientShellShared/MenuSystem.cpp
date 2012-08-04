// ----------------------------------------------------------------------- //
//
// MODULE  : MenuSystem.cpp
//
// PURPOSE : In-game system menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MenuSystem.h"
#include "InterfaceMgr.h"
#include "ClientResShared.h"
#include "ClientMultiplayerMgr.h"
#include "ClientSaveLoadMgr.h"
namespace
{
	enum eMenuCmds
	{
		MC_LOAD = MC_CUSTOM,
		MC_SAVE,
		MC_OPTIONS,
		MC_HOST_OPTIONS,
		MC_PLAYER,
		MC_TEAM,
		MC_MAIN,
		MC_ABORT,
		MC_QUIT,
		MC_EXIT,
#ifdef _TRON_E3_DEMO
		MC_RESUME,
#endif
	};

	void QuitCallBack(LTBOOL bReturn, void *pData)
	{
		CMenuSystem *pThisMenu = (CMenuSystem *)pData;
		if (bReturn && pThisMenu)
			pThisMenu->SendCommand(MC_EXIT,0,0);
	};

	void AbortCallBack(LTBOOL bReturn, void *pData)
	{
		CMenuSystem* pThisMenu = (CMenuSystem *)pData;
		if (bReturn && pThisMenu)
			pThisMenu->SendCommand(MC_MAIN,0,0);
	}


}

CMenuSystem::CMenuSystem( )
{
	m_pSaveCtrl = NULL;
	m_pLoadCtrl = NULL;
	m_pPlayerCtrl = NULL;
	m_pTeamCtrl = NULL;
	m_pHostCtrl = NULL;
	m_pServerCtrl = NULL;
}

LTBOOL CMenuSystem::Init()
{
	m_MenuID = MENU_ID_SYSTEM;

	if (!CBaseMenu::Init()) return LTFALSE;

	SetTitle(IDS_TITLE_SYSTEM);

	uint16 nLoadCtrlId = AddControl(IDS_LOADGAME,MC_LOAD);
	m_pLoadCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nLoadCtrlId );
	uint16 nSaveCtrlId = AddControl(IDS_SAVEGAME,MC_SAVE);
	m_pSaveCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nSaveCtrlId );

	AddControl(IDS_OPTIONS,MC_OPTIONS);

	uint16 nHostCtrlId = AddControl(IDS_HOST_OPTIONS,MC_HOST_OPTIONS);
	m_pHostCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nHostCtrlId );

	uint16 nPlayerCtrlId = AddControl(IDS_PLAYER,MC_PLAYER);
	m_pPlayerCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nPlayerCtrlId );

	uint16 nTeamCtrlId = AddControl(IDS_CHOOSE_TEAM,MC_TEAM);
	m_pTeamCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nTeamCtrlId );
	
	AddControl(IDS_ABORT,MC_ABORT);
	AddControl(IDS_EXIT,MC_QUIT);

	AddControl(" ",0,LTTRUE);
	uint16 nServerCtrlId = AddControl("",0,LTTRUE);
	m_pServerCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nServerCtrlId );


	g_pInterfaceMgr->GetMenuMgr()->RegisterHotKey(VK_ESCAPE,MENU_ID_SYSTEM);

	return LTTRUE;
}

uint32 CMenuSystem::OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2)
{
	switch (nCommand)
	{
	case MC_LOAD:
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_LOAD);
		break;
	case MC_SAVE:
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_SAVE);
		break;
	case MC_OPTIONS:
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_OPTIONS);
		break;
	case MC_HOST_OPTIONS:
		switch (g_pGameClientShell->GetGameType())
		{
		case eGameTypeCooperative:
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_OPTIONS);
			break;
		case eGameTypeDeathmatch:
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_DM_OPTIONS);
			break;
		case eGameTypeTeamDeathmatch:
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_TDM_OPTIONS);
			break;
		case eGameTypeDoomsDay:
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_DD_OPTIONS);
			break;
		};
		break;
	case MC_PLAYER:
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PLAYER);
		break;
	case MC_TEAM:
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PLAYER_TEAM);
		break;
	case MC_MAIN:
		{
			
			// Make sure we're disconnected from server.
			if(g_pLTClient->IsConnected())
			{
				g_pInterfaceMgr->SetIntentionalDisconnect( true );
				g_pClientMultiplayerMgr->ForceDisconnect();
			}
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_MAIN);

		}
		break;
	case MC_QUIT:
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = QuitCallBack;
			mb.pData = this;
			g_pInterfaceMgr->ShowMessageBox(IDS_SUREWANTQUIT,&mb);
			break;
		}
	case MC_ABORT:
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = AbortCallBack;
			mb.pData = this;

			int nMsgID = IDS_SUREWANTABORT;
			if (IsMultiplayerGame())
			{
				// Since we know we're starting a new server, ignore any disconnect messages.
				bool bIsLocalClient = false;
				g_pLTClient->IsLocalToServer(&bIsLocalClient);
				if( bIsLocalClient)
					nMsgID = IDS_SHUTDOWNSERVER;
				
			}
			g_pInterfaceMgr->ShowMessageBox(nMsgID,&mb);

			break;
		}
	case MC_EXIT:
		{
            g_pLTClient->Shutdown();
			break;
		}
#ifdef _TRON_E3_DEMO
	case MC_RESUME:
		{
			g_pInterfaceMgr->ChangeState(GS_PLAYING);
			break;
		}
#endif
	default:
		return CBaseMenu::OnCommand(nCommand,nParam1,nParam2);
	}
	return 1;
}

void CMenuSystem::OnFocus(LTBOOL bFocus)
{
	ClearSelection();
	m_List.ClearSelection();
	if( bFocus )
	{
		// Hide the save/load controls if we are a remote client.
		if	( !g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ) && 
			  ( g_pGameClientShell->GetGameType() == eGameTypeSingle ||
				g_pGameClientShell->GetGameType() == eGameTypeCooperative
			  )
			)
		{
			m_pSaveCtrl->Show( LTTRUE );
			m_pLoadCtrl->Show( LTTRUE );
		}
		else
		{
			m_pSaveCtrl->Show( LTFALSE );
			m_pLoadCtrl->Show( LTFALSE );
		}

		m_pSaveCtrl->Enable(g_pClientSaveLoadMgr->CanSaveGame());

		if (IsMultiplayerGame())
		{
			m_pPlayerCtrl->Show( LTTRUE );
			m_pTeamCtrl->Show( IsTeamGameType() );
			m_pHostCtrl->Show( !g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ) );
			m_pServerCtrl->Show(g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ));

			if (g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
			{
				std::string str = g_pClientMultiplayerMgr->GetStartGameRequest( ).m_HostInfo.m_sName;
				str += " : ";
				str += g_pClientMultiplayerMgr->GetStartGameRequest( ).m_TCPAddress;
				m_pServerCtrl->SetString(str.c_str());
			}


		}
		else
		{
			m_pPlayerCtrl->Show( LTFALSE );
			m_pTeamCtrl->Show( LTFALSE );
			m_pHostCtrl->Show( LTFALSE  );
			m_pServerCtrl->Show( LTFALSE  );
		}

		if (m_fScale != g_pInterfaceResMgr->GetXRatio())
		{
			SetScale(g_pInterfaceResMgr->GetXRatio());
		}

		SetSelection(GetIndex(&m_List));
		m_List.ClearSelection();
		m_List.NextSelection();
	}
}
