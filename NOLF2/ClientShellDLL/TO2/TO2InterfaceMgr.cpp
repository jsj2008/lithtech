// ----------------------------------------------------------------------- //
//
// MODULE  : TronInterfaceMgr.cpp
//
// PURPOSE : Manage all interface related functionality
//
// CREATED : 4/6/99
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TO2InterfaceMgr.h"
#include "GameClientShell.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2InterfaceMgr::CTO2InterfaceMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTO2InterfaceMgr::CTO2InterfaceMgr() : CInterfaceMgr()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2InterfaceMgr::~CTO2InterfaceMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CTO2InterfaceMgr::~CTO2InterfaceMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2InterfaceMgr::OnExitWorld()
//
//	PURPOSE:	Handle exiting a world
//
// ----------------------------------------------------------------------- //

void CTO2InterfaceMgr::OnExitWorld()
{
	g_pMissionText->Clear();
	g_pChatInput->OnExitWorld();
	g_pSubtitles->Clear();

	// Now handle the base class
	CInterfaceMgr::OnExitWorld();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2InterfaceMgr::OnCommandOn()
//
//	PURPOSE:	Handle command on
//
// ----------------------------------------------------------------------- //
LTBOOL CTO2InterfaceMgr::OnCommandOn(int command)
{
    if (g_pChatInput->IsVisible()) return LTTRUE;

	// Take appropriate action

	if (g_pRadio->IsVisible())
	{
		if (command >= COMMAND_ID_CHOOSE_1 &&
			command <= COMMAND_ID_CHOOSE_6 )
		{
			uint8 nChoice = command - COMMAND_ID_CHOOSE_1;
			g_pRadio->Choose(nChoice);
			return LTTRUE;
		} 
		else if (command >= COMMAND_ID_NEXT_WEAPON_1 &&
			command <= COMMAND_ID_NEXT_WEAPON_6 )
		{
			return LTTRUE;
		}
	}

	switch (command)
	{
		case COMMAND_ID_MESSAGE :
		case COMMAND_ID_TEAM_MESSAGE :
		{
			if (!IsTeamGameType() && (command == COMMAND_ID_TEAM_MESSAGE))
				break;

			if (m_eGameState == GS_PLAYING && !g_pPlayerMgr->IsSpectatorMode() && !FadingScreen())
			{
				if (m_AmmoChooser.IsOpen())
				{
					m_AmmoChooser.Close();
				}
				if (m_WeaponChooser.IsOpen())
				{
					m_WeaponChooser.Close();
				}

                g_pChatInput->Show(true, (command == COMMAND_ID_TEAM_MESSAGE) );
				g_pScores->Show(false);
			}

            return LTTRUE;
		}
		break;

		case COMMAND_ID_RADIO :
		{
			if (m_eGameState == GS_PLAYING && (g_pGameClientShell->GetGameType() == eGameTypeCooperative || IsTeamGameType() )  )
			{
				// [KLS 9/9/02] Made toggle and added sounds...
				g_pRadio->Show(!g_pRadio->IsVisible());
				
				if (g_pRadio->IsVisible())
				{
					g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\RadioOn.wav");
				}
				else
				{
					g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\RadioOff.wav");
				}
			}

            return LTTRUE;
		}
		break;

		case COMMAND_ID_COMPASS :
		{
			if (m_eGameState == GS_PLAYING && !g_pPlayerMgr->IsSpectatorMode())
			{
                g_pCompass->Toggle();

				if (g_pCompass->GetDraw())
				{
					g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\CompassOn.wav");
				}
				else
				{
					g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\CompassOff.wav");
				}

				if( g_pGameClientShell->ShouldUseRadar() )
				{
					g_pRadar->Toggle();
					CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
				
					if( IsMultiplayerGame() )
					{
						if (g_pGameClientShell->GetGameType() != eGameTypeDeathmatch)
						{
							pProfile->m_bMPRadar = g_pRadar->GetDraw( );
						}
					}
					else
					{
						pProfile->m_bSPRadar = g_pRadar->GetDraw( );
					}

					pProfile->Save();
				}
			}

            return LTTRUE;
		}
		break;

		default :
		{
			return CInterfaceMgr::OnCommandOn(command);
		}
		break;
	}
    return LTFALSE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnKeyDown()
//
//	PURPOSE:	Handle OnKeyDown messages
//
// ----------------------------------------------------------------------- //

LTBOOL CTO2InterfaceMgr::OnKeyDown(int key, int rep)
{
	//handle stuff before default handling
	if (g_pRadio->IsVisible() && m_eGameState == GS_PLAYING && key == VK_ESCAPE)
	{
		g_pRadio->Show(false);
		g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\RadioOff.wav");
		return LTTRUE;
	}

	//default handling
	if (CInterfaceMgr::OnKeyDown(key, rep))
		return LTTRUE;

	//handle stuff after default handling
	// (nothing needed here yet)
	


	return LTFALSE;
}
