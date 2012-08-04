// ----------------------------------------------------------------------- //
//
// MODULE  : ClientInfoMgr.cpp
//
// PURPOSE : Manages and displays player info for multiplayer games
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientInfoMgr.h"
#include "iltclient.h"
#include "GameClientShell.h"
#include "CharacterFX.h"
#include <stdio.h>
#include "GameModeMgr.h"
#include "iltfilemgr.h"

#define VERT_SPACING 3
extern CGameClientShell* g_pGameClientShell;

extern LTVector2n g_vNameSz;

void AddClientFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;
	CClientInfoMgr *pCIMgr = g_pGameClientShell->GetInterfaceMgr( )->GetClientInfoMgr();
	if (!pCIMgr) return;
	uint8 n = 1;
	while (pCIMgr->GetNumClients() < GameModeMgr::Instance( ).m_grnMaxPlayers)
	{
		wchar_t szName[32];
		LTSNPrintF(szName, LTARRAYSIZE(szName), L"fake%d", n );
		uint8 nTeam = 0;
		if (GameModeMgr::Instance( ).m_grbUseTeams)
		{
			nTeam = n % 2;
		}
		pCIMgr->AddClient(szName,"",false,n+1024,nTeam);
		n++;

	}
}


namespace
{
	wchar_t wszUnknownPlayer[32] = L"";
	const int kNumTeams = 2;
}

ClientDisplayData::ClientDisplayData()
{
	hNameTarget = NULL; 
	hNameMaterial = NULL;
	hInsigniaMaterial = NULL;
}

ClientDisplayData::~ClientDisplayData()
{
	if (hNameMaterial)
	{
		g_pLTClient->GetRenderer()->ReleaseMaterialInstance(hNameMaterial);
		hNameMaterial = NULL;
	}
	if (hInsigniaMaterial)
	{
		g_pLTClient->GetRenderer()->ReleaseMaterialInstance(hInsigniaMaterial);
		hInsigniaMaterial = NULL;
	}

	if (hNameTarget)
	{
		g_pLTClient->GetRenderer()->ReleaseRenderTarget(hNameTarget);
		hNameTarget = NULL;
	}
}


CLIENT_INFO::CLIENT_INFO()
{ 
	nPing = 0; 
	nID = 0; 
	pPrev = NULL; 
	pNext = NULL; 
	bIsAdmin = false; 
	nTeamID = INVALID_TEAM; 

}
CLIENT_INFO::~CLIENT_INFO()
{
}


CClientInfoMgr::CClientInfoMgr()
{
    m_pClients = NULL;
}

CClientInfoMgr::~CClientInfoMgr()
{
    CLIENT_INFO* ptr = NULL;
	while (m_pClients)
	{
		ptr = m_pClients->pNext;
		debug_delete(m_pClients);
		m_pClients = ptr;
	}

	for( uint32 nIndex = 0; nIndex < m_aNameTargets.size(); nIndex++ )
	{
		if (m_aNameTargets[nIndex])
		{
			g_pLTClient->GetRenderer()->ReleaseRenderTarget(m_aNameTargets[nIndex]);
			m_aNameTargets[nIndex] = NULL;
		}
	}
	m_aNameTargets.clear();

}

void CClientInfoMgr::Init()
{
	LTStrCpy( wszUnknownPlayer, LoadString("IDS_UNKNOWN_PLAYER"), LTARRAYSIZE(wszUnknownPlayer) );
	g_pLTClient->RegisterConsoleProgram( "AddClient", AddClientFn );
	m_nLocalID = 0;
}

void CClientInfoMgr::SetupMultiplayer()
{
	m_aNameTargets.reserve(MAX_MULTI_PLAYERS);
	while (m_aNameTargets.size() < MAX_MULTI_PLAYERS)
	{
		HRENDERTARGET hTarget = NULL;
		g_pLTClient->GetRenderer()->CreateRenderTarget(g_vNameSz.x, g_vNameSz.y, eRTO_AutoGenMipMaps, hTarget);
		m_aNameTargets.push_back(hTarget);

	}
}

void CClientInfoMgr::ClearMultiplayer()
{
	for( uint32 nIndex = 0; nIndex < m_aNameTargets.size(); nIndex++ )
	{
		if (m_aNameTargets[nIndex])
		{
			g_pLTClient->GetRenderer()->ReleaseRenderTarget(m_aNameTargets[nIndex]);
			m_aNameTargets[nIndex] = NULL;
		}
	}
	m_aNameTargets.clear();

}


void CClientInfoMgr::UpdateClient ( const wchar_t* pszName, const char* pszInsignia,  bool bIsAdmin, uint32 nID, uint8 nTeamID )
{
	// if we have this client in the list, update their info
	CLIENT_INFO* pClient = GetClientByID(nID, false);
	if (pClient)
	{
		if ( LTStrEquals( pClient->sName.c_str(),pszName ) &&
			 LTStrEquals( pClient->sInsignia.c_str(),pszInsignia ) &&
			 (pClient->bIsAdmin == bIsAdmin) &&
			 (pClient->nTeamID == nTeamID)
			)
		{
			DebugCPrint(1,"%s - same data for client %ls",__FUNCTION__, pszName);
		}
		else
		{
			pClient->sName = pszName;
			pClient->sInsignia = VerifyInsignia(pszInsignia);
			pClient->bIsAdmin = bIsAdmin;
			bool bChanged = ( pClient->nTeamID != nTeamID );
			pClient->nTeamID = nTeamID;

			UpdateClientSort(pClient);

			CCharacterFX* pCFX = g_pGameClientShell->GetSFXMgr()->GetCharacterFromClientID(nID);
			if (pCFX)
				pCFX->UpdateHUDData();

			if( bChanged )
				PlayerChangedTeamsEvent.DoNotify( PlayerChangedTeamsNotifyParams( PlayerChangedTeamsEvent, pClient ));
		}

	}
	else
	{
		//hmmm... we should have found them...
		DebugCPrint(1,"CClientInfoMgr::UpdateClient () : Unknown Client %d (%s)", nID, pszName);
	}


}

void CClientInfoMgr::PlayerConnected( const wchar_t* pszName, const char* pszInsignia, uint32 nID )
{
	m_nLocalID = 0;
	g_pLTClient->GetLocalClientID (&m_nLocalID);

	// if we already have this client in the list, just return

	CLIENT_INFO* pDup = GetClientByID(nID, false);
	if (pDup)
	{
		pDup->sName = pszName;
		pDup->sInsignia = VerifyInsignia(pszInsignia);
	}
	else
	{
		// create the new object

		CLIENT_INFO* pNew = debug_new(CLIENT_INFO);
		if (!pNew) return;

		pNew->nID = nID;
		pNew->sName = pszName;
		pNew->sInsignia = VerifyInsignia(pszInsignia);
		pNew->bIsAdmin = false;
		pNew->nTeamID = INVALID_TEAM;
		pNew->sScore.Init(nID);
		if (m_aNameTargets.size())
		{
			pNew->sDisplayData.hNameTarget = m_aNameTargets[0];
			m_aNameTargets.erase(m_aNameTargets.begin());
		}
		else
		{
			LTERROR("shouldn't need to create render target");
			DebugCPrint(0,"%s: shoudn't need to create render target.",__FUNCTION__);
			g_pLTClient->GetRenderer()->CreateRenderTarget(g_vNameSz.x, g_vNameSz.y, eRTO_AutoGenMipMaps, pNew->sDisplayData.hNameTarget);
		}


		// if we don't have a list yet, set the list pointer to the new object
		if (!m_pClients)
		{
			m_pClients = pNew;
			return;
		}

		// we do have a list - insert the object
		CLIENT_INFO* ptr = m_pClients;

		//insert at head?
		if (ptr->sScore.GetScore() < pNew->sScore.GetScore())
		{
			pNew->pNext = m_pClients;
			m_pClients = pNew;
			return;
		}

		CLIENT_INFO* pNext = ptr->pNext;
		while (pNext && pNext->sScore.GetScore() >= pNew->sScore.GetScore())
		{
			ptr = pNext;
			pNext = ptr->pNext;
		}
		if (pNext)
		{
			pNext->pPrev = pNew;
		}
		ptr->pNext = pNew;
		pNew->pNext = pNext;
		pNew->pPrev = ptr;
	}
}

void CClientInfoMgr::AddClient ( const wchar_t* pszName, const char* pszInsignia, bool bIsAdmin, uint32 nID, uint8 nTeamID)
{
	m_nLocalID = 0;
	g_pLTClient->GetLocalClientID (&m_nLocalID);

	// if we already have this client in the list, then it's an update, not an add.
	CLIENT_INFO* pDup = GetClientByID(nID, false);
	if (pDup)
	{
		UpdateClient( pszName, pszInsignia, bIsAdmin, nID, nTeamID );
		return;
	}


	// create the new object

	CLIENT_INFO* pNew = debug_new(CLIENT_INFO);
	if (!pNew) return;

	pNew->nID = nID;
	pNew->sName = pszName;
	pNew->sInsignia = VerifyInsignia(pszInsignia);
	pNew->bIsAdmin = bIsAdmin;
	pNew->nTeamID = nTeamID;
	pNew->sScore.Init(nID);

	if (m_aNameTargets.size())
	{
		pNew->sDisplayData.hNameTarget = m_aNameTargets[0];
		m_aNameTargets.erase(m_aNameTargets.begin());
	}
	else
	{
		LTERROR("shouldn't need to create render target");
		DebugCPrint(0,"%s: shoudn't need to create render target.",__FUNCTION__);
		g_pLTClient->GetRenderer()->CreateRenderTarget(g_vNameSz.x, g_vNameSz.y, eRTO_AutoGenMipMaps, pNew->sDisplayData.hNameTarget);
	}

	// if we don't have a list yet, set the list pointer to the new object
	if (!m_pClients)
	{
		m_pClients = pNew;
	}
	else
	{
		// we do have a list - insert the object
		CLIENT_INFO* ptr = m_pClients;

		//insert at head?
		if (ptr->sScore.GetScore() < pNew->sScore.GetScore())
		{
			pNew->pNext = m_pClients;
			m_pClients = pNew;
		}
		else
		{
			CLIENT_INFO* pNext = ptr->pNext;
			while (pNext && pNext->sScore.GetScore() >= pNew->sScore.GetScore())
			{
				ptr = pNext;
				pNext = ptr->pNext;
			}
			if (pNext)
			{
				pNext->pPrev = pNew;
			}
			ptr->pNext = pNew;
			pNew->pNext = pNext;
			pNew->pPrev = ptr;
		}

		g_pHUDMgr->QueueUpdate(kHUDScores);	
		ScoresChangedEvent.DoNotify( );
	}

	CCharacterFX* pCFX = g_pGameClientShell->GetSFXMgr()->GetCharacterFromClientID(nID);
	if (pCFX)
	{
		pCFX->UpdateHUDData();
	}
}

void CClientInfoMgr::RemoveClient (uint32 nID)
{
	if (!m_pClients) return;

	m_nLocalID = 0;
	g_pLTClient->GetLocalClientID (&m_nLocalID);

	// find the client

	CLIENT_INFO* ptr = GetClientByID(nID, false);
	if (!ptr) return;

	if (ptr->sDisplayData.hNameTarget)
	{
		m_aNameTargets.push_back(ptr->sDisplayData.hNameTarget);
		ptr->sDisplayData.hNameTarget = NULL;
	}

	// remove the client from the list

	if (ptr->pNext) 
		ptr->pNext->pPrev = ptr->pPrev;
	if (ptr->pPrev) 
		ptr->pPrev->pNext = ptr->pNext;
	if (m_pClients == ptr) 
		m_pClients = ptr->pNext;

	debug_delete(ptr);

	g_pHUDMgr->QueueUpdate(kHUDScores);	
	ScoresChangedEvent.DoNotify( );
}

void CClientInfoMgr::RemoveAllClients()
{
    CLIENT_INFO* ptr = NULL;
	while (m_pClients)
	{
		if (m_pClients->sDisplayData.hNameTarget)
		{
			m_aNameTargets.push_back(m_pClients->sDisplayData.hNameTarget);
			m_pClients->sDisplayData.hNameTarget = NULL;
		}
		ptr = m_pClients->pNext;
		debug_delete(m_pClients);
		m_pClients = ptr;
	}

	g_pHUDMgr->QueueUpdate(kHUDScores);	

	ScoresChangedEvent.DoNotify( );
}

void CClientInfoMgr::UpdateClientSort(CLIENT_INFO* pCur)
{
	if (!pCur) return;

	g_pHUDMgr->QueueUpdate(kHUDScores);
	ScoresChangedEvent.DoNotify();

	// put this client in the correct position in the list (most frags to least frags)
	CLIENT_INFO* pTmp = pCur;

	bool bBack = false;
	//try going forward
	while (pTmp->pPrev && IsScoreBetter(pCur,pTmp->pPrev))	
		pTmp = pTmp->pPrev;
	if (pTmp == pCur)
	{
		//try going backward
		while (pTmp->pNext && IsScoreBetter(pTmp->pNext,pCur))	
			pTmp = pTmp->pNext;
		if (pTmp == pCur) return;
		bBack = true;

	}

	// we found a new position - remove current from the list
	if (pCur->pPrev)
		pCur->pPrev->pNext = pCur->pNext;
	else
		m_pClients = pCur->pNext;
	if (pCur->pNext) 
		pCur->pNext->pPrev = pCur->pPrev;

	// put us back in in the correct position
	if (bBack)
	{
		pCur->pPrev = pTmp;
		pCur->pNext = pTmp->pNext;
		pTmp->pNext = pCur;
		if (pCur->pNext)
			pCur->pNext->pPrev = pCur;
	}
	else
	{
		pCur->pPrev = pTmp->pPrev;
		pCur->pNext = pTmp;
		if (pTmp->pPrev) 
			pTmp->pPrev->pNext = pCur;
		else
			m_pClients = pCur;
		pTmp->pPrev = pCur;
	}
}


uint32 CClientInfoMgr::GetNumClients()
{
	if (!m_pClients) return 0;

	CLIENT_INFO* ptr = m_pClients;

    uint32 nCount = 0;
	while (ptr)
	{
		nCount++;
		ptr = ptr->pNext;
	}

	return nCount;
}

const wchar_t* CClientInfoMgr::GetPlayerName (uint32 nID)
{
    if (!m_pClients) return NULL;

	CLIENT_INFO* ptr = GetClientByID(nID);
	if (ptr)
		return ptr->sName.c_str( );

	return wszUnknownPlayer;
}

const char* CClientInfoMgr::GetPlayerInsignia (uint32 nID)
{
	if (!m_pClients) return NULL;

	CLIENT_INFO* ptr = GetClientByID(nID);
	if (ptr)
		return ptr->sInsignia.c_str( );

	return "";
}


uint8 CClientInfoMgr::GetPlayerTeam(uint32 nID)
{
	if (!m_pClients) return INVALID_TEAM;
	if (!GameModeMgr::Instance( ).m_grbUseTeams) return INVALID_TEAM;

	CLIENT_INFO* ptr = GetClientByID(nID);
	if (ptr)
		return ptr->nTeamID;

	return INVALID_TEAM;
}




CLIENT_INFO* CClientInfoMgr::GetLocalClient()
{
	return GetClientByID(m_nLocalID);
}

CLIENT_INFO* CClientInfoMgr::GetClientByID(uint32 nID, bool bUpdateOnFailure)
{
	if( nID == INVALID_CLIENT )
		return NULL;

	CLIENT_INFO* ptr = m_pClients;
	while (ptr)
	{
		if (ptr->nID == nID)
			return ptr;
		
		ptr = ptr->pNext;
	}
	if (bUpdateOnFailure)
		g_pInterfaceMgr->UpdateClientList();
    return NULL;
}
			
uint8 CClientInfoMgr::GetNumPlayersOnTeam(uint8 nTeam)
{
	if (nTeam > kNumTeams)
	{
		CLIENT_INFO* pLocal = GetLocalClient();
		if (pLocal)
			nTeam = pLocal->nTeamID;
	}

	uint8 nCount = 0;

	CLIENT_INFO* ptr = m_pClients;
	while (ptr)
	{
		if (ptr->nTeamID == nTeam)
			nCount++;

		ptr = ptr->pNext;
	}

	return nCount;

}

uint8 CClientInfoMgr::GetLocalTeam()
{
	uint8 nTeam = INVALID_TEAM;
	const CLIENT_INFO* pLocal = GetLocalClient();
	if (pLocal)
		nTeam = pLocal->nTeamID;
	return nTeam;
}

bool CClientInfoMgr::IsLocalTeam( uint8 nTeamID )
{
	if (!GameModeMgr::Instance( ).m_grbUseTeams) return false;
	uint8 nLocalTeam = INVALID_TEAM;
	const CLIENT_INFO* pLocal = GetLocalClient();
	if (pLocal)
		nLocalTeam = pLocal->nTeamID;
	return (nLocalTeam == nTeamID);
}



//returns true if Client A has a better Score than Client B
bool CClientInfoMgr::IsScoreBetter( CLIENT_INFO* pA, CLIENT_INFO* pB) 
{
	//in elimination scoring, whether the player is alive is more important than their actual score
	if (GameModeMgr::Instance( ).m_grbEliminationWin)
	{
		CCharacterFX* pCharA = g_pGameClientShell->GetSFXMgr()->GetCharacterFromClientID(pA->nID);
		CCharacterFX* pCharB = g_pGameClientShell->GetSFXMgr()->GetCharacterFromClientID(pB->nID);

		bool bAliveA = (pCharA &&  !pCharA->IsPlayerDead() && !pCharA->m_cs.bIsSpectating );
		bool bAliveB = (pCharB &&  !pCharB->IsPlayerDead() && !pCharB->m_cs.bIsSpectating );

		//if A is alive and B isn't, A is better
		if (bAliveA && !bAliveB)
		{
			return true;
		}
		//if B is alive and A isn't, B is better
		if (bAliveB && !bAliveA)
		{
			return false;
		}

		//if both are alive or both are dead, do normal comparison

	}
	return pA->sScore.GetScore() > pB->sScore.GetScore();
}


const char* CClientInfoMgr::VerifyInsignia(const char* pszInsignia)
{
	// if the file does not exist then use the default
	ILTInStream* pStream = g_pLTBase->FileMgr()->OpenFile( pszInsignia );
	if( !pStream )
		return g_pLTDatabase->GetString( g_pModelsDB->GetInsigniaAttribute(), 0, "" );;

	LTSafeRelease(pStream);

	//we found the file, so we can use it
	return pszInsignia;

}