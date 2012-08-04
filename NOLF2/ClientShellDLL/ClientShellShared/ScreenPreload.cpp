// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPreload.h
//
// PURPOSE : Interface screen to be displayed  before loading a level
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ScreenPreload.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "ClientMultiplayerMgr.h"
#include "MissionMgr.h"
#include "iserverdir.h"

extern bool g_bLAN;

namespace
{
}

static void FailCDKey( )
{
	MBCreate mb;
	g_pInterfaceMgr->ShowMessageBox(IDS_CDKEY_INVALID,&mb);
	g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_MAIN);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenPreload::CScreenPreload()
{
	m_bWaitingToExit = false;
	m_eValidatingCDKeyState = kValidatingCDKeyState_None;
	m_bFirstUpdate = false;
}

CScreenPreload::~CScreenPreload()
{

}


// Build the screen
LTBOOL CScreenPreload::Build()
{
	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTFALSE);
	return LTTRUE;
}


void CScreenPreload::OnFocus(LTBOOL bFocus)
{
	CBaseScreen::OnFocus(bFocus);
	m_bVisited = LTFALSE;
}


void CScreenPreload::Escape()
{
}

bool CScreenPreload::UpdateInterfaceSFX()
{
	if( m_bFirstUpdate )
	{
		m_bFirstUpdate = false;
		FirstUpdate( );
	}


	if (!CBaseScreen::UpdateInterfaceSFX())
		return false;

	ChainFXList::iterator iter = m_Chains.begin();
	bool bAnyActive = false;
	while (iter != m_Chains.end() && !bAnyActive)
	{
		CChainedFX *pChain = (*iter);
		pChain->Update();
		
		bAnyActive = !pChain->IsIntroDone();
		iter++;
	}

	bAnyActive = bAnyActive || !UpdateCDKeyValidation( );

	if (!bAnyActive )
	{
		bool bSuccess = true;
		if (m_bWaitingToExit)
			bSuccess = g_pMissionMgr->FinishExitLevel( );
		else
			bSuccess = g_pMissionMgr->FinishStartGame( );

		if (!bSuccess)
		{
			if (IsMultiplayerGame())
				g_pInterfaceMgr->ConnectionFailed(g_pClientMultiplayerMgr->GetLastConnectionResult());
			else
				g_pInterfaceMgr->LoadFailed();
		}
	}

	return bAnyActive;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPreload::UpdateCDKeyValidation
//
//	PURPOSE:	Updates the cdkey validation state.
//
//	RETURN:		true when validation complete.
//
// --------------------------------------------------------------------------- //

bool CScreenPreload::UpdateCDKeyValidation( )
{
	switch( m_eValidatingCDKeyState )
	{
		case kValidatingCDKeyState_None:
		{
			// Not validating cdkey, so we're done.
			return true;
			break;
		}
		case kValidatingCDKeyState_Start:
		{
			IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();
			if( !pServerDir )
			{
				pServerDir = g_pClientMultiplayerMgr->CreateServerDir( );
			}

			// Need to ask serverdir what the cdkey is from registry then set it.
			std::string sCDKey;
			pServerDir->GetCDKey( &sCDKey );
			if( !pServerDir->SetCDKey( sCDKey.c_str( )))
			{
				FailCDKey( );
				return false;
			}

			if( !pServerDir->QueueRequest(IServerDirectory::eRequest_Validate_CDKey))
			{
				FailCDKey( );
				return false;
			}

			m_eValidatingCDKeyState = kValidatingCDKeyState_Waiting;
			break;
		}
		case kValidatingCDKeyState_Waiting:
		{
			IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();

			// Are we still waiting?
			switch (pServerDir->GetCurStatus())
			{
				case IServerDirectory::eStatus_Processing : 
				{
					// Still waiting for response.
					break;
				}
				case IServerDirectory::eStatus_Waiting : 
				{
					// If we reach the waiting state, we're done.
					m_eValidatingCDKeyState = kValidatingCDKeyState_None;
					return true;
					break;
				}
				case IServerDirectory::eStatus_Error : 
				{
					FailCDKey( );
					break;
				}
				default :
				{
					ASSERT(!"Unknown directory status encountered");

					FailCDKey( );
					break;
				}
			}

			break;
		}
	}

	return false;
}


void CScreenPreload::FirstUpdate( )
{
	int n = 0;
	char szTagName[30];
	char szAttName[30];
	char szFXName[128];

	m_bHaveLights = LTFALSE;


	HOBJECT hCamera = g_pInterfaceMgr->GetInterfaceCamera();
	if (!hCamera) return;

	// Check if we have a mission entry.
	bool bGotMission = false;
	if( !g_pMissionMgr->IsCustomLevel( ))
	{
		int nNewMission = g_pMissionMgr->GetNewMission( );
		int nNewLevel = g_pMissionMgr->GetNewLevel( );
		MISSION* pMission = g_pMissionButeMgr->GetMission( nNewMission );
		if( pMission )
		{
			int nBriefingId = pMission->aLevels[nNewLevel].nBriefingId;
			if( nBriefingId >= 0 && !g_pMissionMgr->IsRestoringLevel( ))
				m_layout = pMission->szBriefLayout;
			else
				m_layout = pMission->szLayout;

			bGotMission = true;
		}
	}

	// If we were unsuccessful in getting info from the mission, then just
	// use defaults.
	if( !bGotMission )
	{
		m_layout = "LoadScreenDefault";
	}



    g_pLTClient->GetObjectPos(hCamera, &s_vPos);
    g_pLTClient->GetObjectRotation(hCamera, &s_rRot);
	s_vU = s_rRot.Up();
	s_vR = s_rRot.Right();
	s_vF = s_rRot.Forward();


	SAFE_STRCPY(szTagName,m_layout.c_str());
	if (!g_pLayoutMgr->Exist(szTagName))
		SAFE_STRCPY(szTagName,"LoadScreenDefault");

	sprintf(szAttName,"Light%d",n);
	while (g_pLayoutMgr->HasValue(szTagName,szAttName))
	{
		g_pLayoutMgr->GetString(szTagName,szAttName,szFXName,128);
		if (strlen(szFXName))
		{
			CreateLightFX(szFXName);
		}

		n++;
		sprintf(szAttName,"Light%d",n);

	}


	n = 0;
	sprintf(szAttName,"PreScale%d",n);
	while (g_pLayoutMgr->HasValue(szTagName,szAttName))
	{
		g_pLayoutMgr->GetString(szTagName,szAttName,szFXName,128);
		if (strlen(szFXName))
		{
			CBaseScaleFX *pSFX = CreateScaleFX(szFXName);
		}

		n++;
		sprintf(szAttName,"PreScale%d",n);

	}

	

	n = 0;
	sprintf(szAttName,"PreFX%d",n);
	while (g_pLayoutMgr->HasValue(szTagName,szAttName))
	{
		g_pLayoutMgr->GetString(szTagName,szAttName,szFXName,128);
		if (strlen(szFXName))
		{
			INT_FX* pFX = g_pLayoutMgr->GetFX(szFXName);
			if (pFX)
			{
				g_pInterfaceMgr->AddInterfaceFX(LTNULL, pFX->szFXName,pFX->vPos,pFX->bLoop);
			}
		}

		
		n++;
		sprintf(szAttName,"PreFX%d",n);

	}

	ChainFXList::iterator iter = m_Chains.begin();
	while (iter != m_Chains.end())
	{
		debug_delete(*iter);
		iter++;
	}
	m_Chains.clear();

	char szIntroFXName[128] = "";
	char szShortFXName[128] = "";
	char szLoopFXName[128] = "";
	int nFXNum = 0;
	bool bFound = false;
	do
	{
		szIntroFXName[0] = 0;
		szShortFXName[0] = 0;
		szLoopFXName[0] = 0;
		bFound = false;

		sprintf(szAttName,"PreIntroFX%d",nFXNum);
		if (g_pLayoutMgr->HasValue(szTagName,szAttName))
		{
			g_pLayoutMgr->GetString(szTagName,szAttName,szIntroFXName,128);
		}
		sprintf(szAttName,"PreLoopFX%d",nFXNum);
		if (g_pLayoutMgr->HasValue(szTagName,szAttName))
		{
			g_pLayoutMgr->GetString(szTagName,szAttName,szLoopFXName,128);
		}
		if (strlen(szIntroFXName) || strlen(szShortFXName) ) //don't start loop-only chains || strlen(szLoopFXName))
		{
			nFXNum++;
			bFound = true;

			CChainedFX *pChain = debug_new(CChainedFX);
			pChain->Init(szIntroFXName,szShortFXName,szLoopFXName);
			m_Chains.push_back(pChain);
		}

	} while (bFound);


	// Start all the chains.
	iter = m_Chains.begin();
	while (iter != m_Chains.end())
	{
		(*iter)->Start(!!m_bVisited);
		iter++;
	}
	
	// If we are not joining a lan game and the cdkey hasn't been validated,
	// we need to do it now.
	m_eValidatingCDKeyState = kValidatingCDKeyState_None;
	if( !g_bLAN && g_pClientMultiplayerMgr->GetStartGameRequest( ).m_Type == STARTGAME_CLIENTTCP )
	{
		IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();
		if( !pServerDir || !pServerDir->IsCDKeyValid( ))
		{
			m_eValidatingCDKeyState = kValidatingCDKeyState_Start;
		}
	}
}

void CScreenPreload::CreateInterfaceSFX()
{
	// Flag that this is will be the first update.
	m_bFirstUpdate = true;
}