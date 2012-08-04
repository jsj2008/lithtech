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
#include "ClientConnectionMgr.h"
#include "ClientSaveLoadMgr.h"
#include "sys/win/mpstrconv.h"
#include "GameModeMgr.h"
#include "ClientVoteMgr.h"

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
		MC_LOADOUT,
		MC_MAIN,
		MC_STORY,
		MC_RESUME,
		MC_ABORT,
		MC_QUIT,
		MC_EXIT,
		MC_VOTE,
		MC_MUTE,
	};

	void SkipCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CMenuSystem *pThisMenu = (CMenuSystem *)pUserData;
		if (bReturn && pThisMenu)
			pThisMenu->SendCommand(MC_STORY,1,0);
	};

	void QuitCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CMenuSystem *pThisMenu = (CMenuSystem *)pUserData;
		if (bReturn && pThisMenu)
			pThisMenu->SendCommand(MC_EXIT,0,0);
	};

	void AbortCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CMenuSystem* pThisMenu = (CMenuSystem *)pUserData;
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
	m_pWpnCtrl = NULL;
	m_pStoryCtrl = NULL;
	m_pHostCtrl = NULL;
	m_pVoteCtrl = NULL;
	m_pMuteCtrl = NULL;
	m_pServerCtrl = NULL;
	m_pServerIPCtrl = NULL;
}

bool CMenuSystem::Init( CMenuMgr& menuMgr )
{
	m_MenuID = MENU_ID_SYSTEM;

	if (!CBaseMenu::Init( menuMgr )) 
		return false;

	uint16 nLoadCtrlId = AddControl("IDS_LOADGAME",MC_LOAD);
	m_pLoadCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nLoadCtrlId );
	uint16 nSaveCtrlId = AddControl("IDS_SAVEGAME",MC_SAVE);
	m_pSaveCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nSaveCtrlId );

	AddControl("IDS_OPTIONS",MC_OPTIONS);

	uint16 nHostCtrlId = AddControl("IDS_HOST_OPTIONS",MC_HOST_OPTIONS);
	m_pHostCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nHostCtrlId );
	
	uint16 nPlayerCtrlId = AddControl("SCREEN_CLIENT_SETTINGS",MC_PLAYER);
	m_pPlayerCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nPlayerCtrlId );
	
	uint16 nTeamCtrlId = AddControl("IDS_CHOOSE_TEAM",MC_TEAM);
	m_pTeamCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nTeamCtrlId );

	uint16 nWpnCtrlId = AddControl("IDS_CHOOSE_LOADOUT",MC_LOADOUT);
	m_pWpnCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nWpnCtrlId );
	
	uint16 nStoryCtrlId = AddControl("StoryMode_Cancel",MC_STORY);
	m_pStoryCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nStoryCtrlId );

	uint16 nVoteCtrlId = AddControl("Menu_CallVote",MC_VOTE);
	m_pVoteCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nVoteCtrlId );

	uint16 nMuteCtrlId = AddControl("Menu_Mute",MC_MUTE);
	m_pMuteCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nMuteCtrlId );


	AddControl("IDS_ABORT",MC_ABORT);
	AddControl("IDS_RESUME", MC_RESUME);
	AddControl("IDS_EXIT",MC_QUIT);

	AddControl(L" ",0,true);
	uint16 nServerCtrlId = AddControl(L"",0,true);
	m_pServerCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nServerCtrlId );

	nServerCtrlId = AddControl(L"",0,LTTRUE);
	m_pServerIPCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nServerCtrlId );

// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)
	m_pMenuMgr->RegisterHotKey(VK_ESCAPE,MENU_ID_SYSTEM);
#endif // !PLATFORM_XENON

	return true;
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
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_OPTIONS);
		break;
	case MC_VOTE:
		if (ClientVoteMgr::Instance().IsVoteInProgress())
		{
			MBCreate mb;
			mb.eType = LTMB_OK;
			g_pInterfaceMgr->ShowMessageBox("ScreenVote_VoteInProgress",&mb);
		}
		else if (!GameModeMgr::Instance( ).m_grbAllowDeadVoting && !g_pPlayerMgr->IsPlayerAlive())
		{
			MBCreate mb;
			mb.eType = LTMB_OK;
			g_pInterfaceMgr->ShowMessageBox("ScreenVote_NoVoteWhileDead",&mb);
		}
		else
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_VOTE);
		}
		
		break;
	case MC_MUTE:
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_MUTE);
		break;
	case MC_PLAYER:
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PLAYER);
		break;
	case MC_TEAM:
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PLAYER_TEAM);
		break;
	case MC_LOADOUT:
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PLAYER_LOADOUT);
		break;
	case MC_MAIN:
		{		
			// Make sure we're disconnected from server.
			if(g_pLTClient->IsConnected())
			{
				g_pInterfaceMgr->SetIntentionalDisconnect( true );
				g_pClientConnectionMgr->ForceDisconnect();
			}

			if (IsMultiplayerGameClient())
			{
				g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_MULTI);
			}
			else
			{
				g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_MAIN);
			}
		}
		break;
	case MC_STORY:
		{
			if (nParam1) 
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8( MID_PLAYER_CLIENTMSG );
				cMsg.Writeuint8( CP_STORY_CANCEL );
				g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
				
				g_pInterfaceMgr->ChangeState(GS_PLAYING);

			}
			else
			{
				MBCreate mb;
				mb.eType = LTMB_YESNO;
				mb.pFn = SkipCallBack;
				mb.pUserData = this;
				g_pInterfaceMgr->ShowMessageBox("StoryMode_ConfirmCancel",&mb);
			}
		} break;

	case MC_RESUME:
		{
			m_pMenuMgr->Close();
		} break;

	case MC_QUIT:
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = QuitCallBack;
			mb.pUserData = this;
			g_pInterfaceMgr->ShowMessageBox("IDS_SUREWANTQUIT",&mb);
		} break;
	case MC_ABORT:
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = AbortCallBack;
			mb.pUserData = this;

			const char* szMsgID = "IDS_SUREWANTABORT";
			if (IsMultiplayerGameClient())
			{
				// Since we know we're starting a new server, ignore any disconnect messages.
				bool bIsLocalClient = false;
				g_pLTClient->IsLocalToServer(&bIsLocalClient);
				if( bIsLocalClient)
					szMsgID = "IDS_SHUTDOWNSERVER";
				
			}
			g_pInterfaceMgr->ShowMessageBox(szMsgID,&mb);

			break;
		}
	case MC_EXIT:
		{
#ifdef _DEMO
			g_pInterfaceMgr->ShowDemoScreens(LTTRUE);
#else
            g_pLTClient->Shutdown();
#endif
			break;
		}
	default:
		return CBaseMenu::OnCommand(nCommand,nParam1,nParam2);
	}
	return 1;
}

void CMenuSystem::OnFocus(bool bFocus)
{
	ClearSelection();
	m_List.ClearSelection();
	if( bFocus )
	{
		// Hide the save/load controls if we are a remote client.
		if( !IsMultiplayerGameClient())
		{
			m_pSaveCtrl->Show( true );
			m_pLoadCtrl->Show( true );
		}
		else
		{
			m_pSaveCtrl->Show( false );
			m_pLoadCtrl->Show( false );
		}

		m_pSaveCtrl->Enable(g_pClientSaveLoadMgr->CanSaveGame());

		if (IsMultiplayerGameClient())
		{
			m_pPlayerCtrl->Show( true );
			m_pTeamCtrl->Show( GameModeMgr::Instance( ).m_grbUseTeams );
			m_pWpnCtrl->Show( GameModeMgr::Instance( ).m_grbUseLoadout );
			m_pStoryCtrl->Show( false );
			m_pHostCtrl->Show( true );
			m_pVoteCtrl->Show( true );
			m_pMuteCtrl->Show( true );
			m_pVoteCtrl->Enable(!ClientVoteMgr::Instance().IsVoteInProgress());
			m_pServerCtrl->Show(g_pClientConnectionMgr->IsConnectedToRemoteServer( ));
			m_pServerIPCtrl->Show(g_pClientConnectionMgr->IsConnectedToRemoteServer( ));

			if (!GameModeMgr::Instance( ).m_grbAllowDeadVoting && !g_pPlayerMgr->IsPlayerAlive())
			{
				m_pVoteCtrl->Enable(false);
			}

			if (g_pClientConnectionMgr->IsConnectedToRemoteServer( ))
			{
				m_pServerCtrl->SetString( GameModeMgr::Instance( ).m_grwsSessionName );
				m_pServerIPCtrl->SetString(MPA2W(g_pClientConnectionMgr->GetStartGameRequest( ).m_TCPAddress).c_str());
			}
		}
		else
		{
			m_pPlayerCtrl->Show( false );
			m_pTeamCtrl->Show( false );
			m_pWpnCtrl->Show( false );
			m_pStoryCtrl->Show( g_pPlayerMgr->InStoryMode() && g_pPlayerMgr->CanSkipStory() );
			m_pHostCtrl->Show( false  );
			m_pVoteCtrl->Show( false );
			m_pMuteCtrl->Show( false );
			m_pServerCtrl->Show( false  );
			m_pServerIPCtrl->Show( false );
		}

		if (m_vfScale != g_pInterfaceResMgr->GetScreenScale())
		{
			SetScale(g_pInterfaceResMgr->GetScreenScale());
		}

		SetSelection(GetIndex(&m_List));
		m_List.ClearSelection();
		m_List.NextSelection();

		m_List.CalculatePositions();

		LTVector2n listpos = m_List.GetBasePos();
		uint16 i = m_List.GetNumControls() - 1;
		CLTGUICtrl *pCtrl = m_List.GetControl(i);
		while (pCtrl && !pCtrl->IsVisible() && i > 0)
		{
			--i;
			pCtrl = m_List.GetControl(i);
		}

		if (pCtrl)
		{
			LTVector2n pos = pCtrl->GetBasePos();
			LTVector2n sz( m_List.GetBaseWidth(), (pos.y - listpos.y) + pCtrl->GetBaseHeight() + m_Indent.y);

			m_List.SetSize(sz);
			sz.y += m_Indent.y;
			sz.x = m_vDefaultSize.x;
			SetSize(sz);
		}

	}
}

// Render the control
void CMenuSystem::Render ( )
{
	if (!IsVisible()) return;

	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);

	LTPoly_G4 back;
	DrawPrimSetRGBA(back, SET_ARGB(0xB0,0,0,0));
	DrawPrimSetXYWH(back,-0.5f,-0.5f,(float)g_pInterfaceResMgr->GetScreenWidth(),(float)g_pInterfaceResMgr->GetScreenHeight());
	g_pDrawPrim->DrawPrim(&back,1);

	CBaseMenu::Render();
}

