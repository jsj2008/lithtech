// ----------------------------------------------------------------------- //
//
// MODULE  : MultiplayerMenu.cpp
//
// PURPOSE : In-game Multiplayer Menu
//
// CREATED : 3/20/00
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "MultiplayerMenu.h"
#include "FolderCommands.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "ClientRes.h"
#include "MsgIDs.h"
extern CGameClientShell* g_pGameClientShell;

namespace
{
	enum eLocalCommands
	{
		CMD_MAIN = 1,
		CMD_PLAYER,
		CMD_TEAM,
		CMD_DISCONNECT,
		CMD_OPTIONS,
		CMD_VIEW,

	};
}

CMultiplayerMenu::CMultiplayerMenu()
{
}

void CMultiplayerMenu::Build()
{
	//add items
	m_pMenuWnd->AddItem(IDS_MAIN,CMD_MAIN);
	m_pMenuWnd->AddItem(IDS_PLAYER,CMD_PLAYER);
	if (g_pGameClientShell->GetGameType() == COOPERATIVE_ASSAULT)
		m_pMenuWnd->AddItem(IDS_CHOOSETEAM,CMD_TEAM);
	m_pMenuWnd->AddItem(IDS_HOST_OPTIONS,CMD_OPTIONS);
	if (g_pGameClientShell->IsHosting())
	{
		m_pMenuWnd->AddItem(IDS_SHUTDOWN,CMD_DISCONNECT);
	}
	else
		m_pMenuWnd->AddItem(IDS_DISCONNECT,CMD_DISCONNECT);
}
void CMultiplayerMenu::Select(uint8 byItem)
{
	m_pMenuWnd->Close();
	//act on selection
	switch (byItem)
	{
	case CMD_MAIN:
		g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_MAIN);
		break;
	case CMD_PLAYER:
		g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_PLAYER);
		break;
	case CMD_TEAM:
		g_pInterfaceMgr->ChooseTeam();
		break;
	case CMD_OPTIONS:
		if (g_pGameClientShell->IsHosting())
		{
			g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_HOST_OPTIONS);
		}
		else
		{
			g_pInterfaceMgr->ViewOptions();
		}
		break;
	case CMD_DISCONNECT:
		if (g_pGameClientShell->IsHosting())
		{
			g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_MAIN);
			g_pLTClient->Disconnect();
		}
		else
		{
			g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_MAIN);
			g_pLTClient->Disconnect();
		}
		
		break;
	}
}


CTeamMenu::CTeamMenu()
{
}

void CTeamMenu::Build()
{
	//add items
	CClientInfoMgr *pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
	if (pCIMgr)
	{
		m_pMenuWnd->AddItem(pCIMgr->GetTeam(1)->szName,1);
		m_pMenuWnd->AddItem(pCIMgr->GetTeam(2)->szName,2);
	}
	else
	{
		m_pMenuWnd->AddItem("Team 1",1);
		m_pMenuWnd->AddItem("Team 2",2);
	}
}

void CTeamMenu::Select(uint8 byItem)
{
	//act on selection
	m_pMenuWnd->Close();


	if (byItem == 1 || byItem == 2)
	{
        HMESSAGEWRITE hWrite = g_pLTClient->StartMessage(MID_PLAYER_CHANGETEAM);
        g_pLTClient->WriteToMessageByte(hWrite, byItem);
        g_pLTClient->EndMessage(hWrite);
	}

}

COptionMenu::COptionMenu()
{
}

void COptionMenu::Build()
{
	char szItem[256];
	const char *szAddr		= g_pGameClientShell->GetServerAddress();
	const char *szName		= g_pGameClientShell->GetServerName();
	const LTFLOAT *fOpt	= g_pGameClientShell->GetServerOptions();

	HSTRING hTemp1 = g_pLTClient->FormatString(IDS_SERVER_NAME);
	sprintf(szItem,"%s: %s",g_pLTClient->GetStringData(hTemp1),szName);
	g_pLTClient->FreeString(hTemp1);
	m_pMenuWnd->AddItem(szItem,0);

	HSTRING hTemp2 = g_pLTClient->FormatString(IDS_SERVER_ADDRESS);
	sprintf(szItem, "%s: %s",g_pLTClient->GetStringData(hTemp2),szAddr);
	g_pLTClient->FreeString(hTemp2);
	m_pMenuWnd->AddItem(szItem,0);


	for (int i = 0; i < g_pServerOptionMgr->GetNumOptions(); i++)
	{
		
		OPTION* pOpt = g_pServerOptionMgr->GetOption(i);
		if (pOpt->eGameType != SINGLE && pOpt->eGameType != g_pGameClientShell->GetGameType()) continue;
		// Add the options/value info...

		char sTemp[32] ="";
		int nVal = 0;
		int nStringID = 0;

		switch (pOpt->eType)
		{
		case SO_TOGGLE:
		case SO_CYCLE:
			nVal = (int)fOpt[i];
			nStringID = pOpt->nStringId[nVal];
			hTemp1 = g_pLTClient->FormatString(pOpt->nNameId);
			hTemp2 = g_pLTClient->FormatString(nStringID);
			sprintf(szItem, "%s: %s",g_pLTClient->GetStringData(hTemp1),g_pLTClient->GetStringData(hTemp2));
			g_pLTClient->FreeString(hTemp1);
			g_pLTClient->FreeString(hTemp2);
			m_pMenuWnd->AddItem(szItem,0);
			break;
		case SO_SLIDER:
		case SO_SLIDER_NUM:
			hTemp1 = g_pLTClient->FormatString(pOpt->nNameId);
			if (pOpt->fSliderScale < 1.0f || pOpt->fSliderScale > 1.0f)
				sprintf(szItem, "%s: %0.2f",g_pLTClient->GetStringData(hTemp1),fOpt[i]);
			else
				sprintf(szItem, "%s: %d",g_pLTClient->GetStringData(hTemp1),(int)fOpt[i]);
			g_pLTClient->FreeString(hTemp1);
			m_pMenuWnd->AddItem(szItem,0);
			break;
		case SO_SPECIAL:
			{
				if (stricmp(pOpt->szVariable,"NetDefaultWeapon") == 0)
				{
					nVal = (int)fOpt[i];
					if (nVal)
					{
						int nWeaponId = g_pWeaponMgr->GetWeaponId(nVal);
						if (nWeaponId != WMGR_INVALID_ID)
						{
							WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
							if (pWeapon)
							{
								nStringID = pWeapon->nNameId;
								hTemp1 = g_pLTClient->FormatString(pOpt->nNameId);
								hTemp2 = g_pLTClient->FormatString(nStringID);
								sprintf(szItem, "%s: %s",g_pLTClient->GetStringData(hTemp1),g_pLTClient->GetStringData(hTemp2));
								g_pLTClient->FreeString(hTemp1);
								g_pLTClient->FreeString(hTemp2);
								m_pMenuWnd->AddItem(szItem,0);
							}
						}
					}
				}
			}
		}		
	}

}

void COptionMenu::Select(uint8 byItem)
{
	//act on selection
	m_pMenuWnd->Close();

}


