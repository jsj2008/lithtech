// ----------------------------------------------------------------------- //
//
// MODULE  : NavMarkerFX.cpp
//
// PURPOSE : NavMarkerFX - Implementation
//
// CREATED : 11/05/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "NavMarkerFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "GameClientShell.h"
#include "NavMarkerTypeDB.h"
#include "SFXMsgIds.h"
#include "HUDMgr.h"
#include "HUDNavMarker.h"
#include "HUDNavMarkerMgr.h"
#include "GameModeMgr.h"
#include "CharacterFX.h"

extern CGameClientShell* g_pGameClientShell;


CNavMarkerFX::CNavMarkerFX()
{
	m_pHUDItem = NULL;
	m_hTarget = NULL;
}

CNavMarkerFX::~CNavMarkerFX()
{
	if (m_pHUDItem)
	{
		if (g_pNavMarkerMgr)
		{
			g_pNavMarkerMgr->RemoveMarker(m_pHUDItem);
		}
		debug_delete(m_pHUDItem);
		m_pHUDItem = NULL;
	}

	// Setup the Looping ClientFX...
	if( m_fxLoop.IsValid( ))
	{
		CClientFXMgr *pClientFXMgr = &g_pGameClientShell->GetSimulationTimeClientFXMgr();
		pClientFXMgr->ShutdownClientFX( &m_fxLoop );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNavMarkerFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

bool CNavMarkerFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return false;

	NAVMARKERCREATESTRUCT* pNMCS = (NAVMARKERCREATESTRUCT*)psfxCreateStruct;
	if (!pNMCS)
	{
		return false;		
	}

	if (pNMCS->m_bBroadcast && !g_pProfileMgr->GetCurrentProfile()->m_bAllowBroadcast)
	{
		return false;		
	}

	m_pHUDItem = debug_new(CHUDNavMarker);

	m_cs = *pNMCS;

	// Setup our timer and pos if we're an instant navmarker.
	if( m_cs.m_bInstant )
	{
		//if we're a broadcast find and kill any existing broadcast message from this client
		if (m_cs.m_bBroadcast)
		{
			CSpecialFXList* const pNMList = g_pGameClientShell->GetSFXMgr()->GetFXList(SFX_NAVMARKER_ID);	
			int nNumSFX  = pNMList->GetSize();

			for (int nNM=0; nNM < nNumSFX; nNM++)
			{
				CNavMarkerFX* pNM = (CNavMarkerFX*)(*pNMList)[nNM];
				if (pNM && pNM->IsBroadcast() && pNM->GetClientID() == GetClientID())
				{
					pNM->WantRemove(true);
				}
			}

		}

		m_LifeTimeTimer.SetEngineTimer( SimulationTimer::Instance( ));
		m_LifeTimeTimer.Start( g_pNavMarkerTypeDB->GetLifetime(m_cs.m_hType));

		m_pHUDItem->SetPos( m_cs.m_vPos );
	}

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNavMarkerFX::CreateObject
//
//	PURPOSE:	Create object associated the fx
//
// ----------------------------------------------------------------------- //

bool CNavMarkerFX::CreateObject(ILTClient *pClientDE)
{
    bool bRet = CSpecialFX::CreateObject(pClientDE);
	if (!bRet) return bRet;

	UpdateData();
	if (m_pHUDItem)
	{
		g_pNavMarkerMgr->AddMarker(m_pHUDItem);
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNavMarkerFX::Update
//
//	PURPOSE:	Update the NavMarker
//
// ----------------------------------------------------------------------- //

bool CNavMarkerFX::Update()
{
	// Look for our target.  The target may have been null when we 
	// intialized due to the order it came down from the server.  If 
	// target came after navmarker, it would have been null at first
	// then later we would be able to read it.
	if( !m_cs.m_hTarget && (m_cs.m_nClientID != INVALID_CLIENT) )
	{
		CCharacterFX::CharFXList::iterator iter = CCharacterFX::GetCharFXList( ).begin( );
		while( iter != CCharacterFX::GetCharFXList( ).end( )) 
		{
			if( (*iter)->m_cs.nClientID == m_cs.m_nClientID )
			{
				m_cs.m_hTarget = (*iter)->GetServerObj( );
				UpdateData( );
			}

			++iter;
		}
	}

	if (m_bWantRemove)
	{
		if (m_pHUDItem)
		{
			g_pNavMarkerMgr->RemoveMarker(m_pHUDItem);
			debug_delete(m_pHUDItem);
			m_pHUDItem = NULL;
		}
		// kill the Looping ClientFX...
		CClientFXMgr *pClientFXMgr = &g_pGameClientShell->GetSimulationTimeClientFXMgr();
		if( m_fxLoop.IsValid( ))
			pClientFXMgr->ShutdownClientFX( &m_fxLoop );
	}
    if (!m_pClientDE || m_bWantRemove || ( !m_hServerObject && !m_cs.m_bInstant ))
		return false;

	// Have us removed if we're instant and our lifetime timed out.
	if( m_cs.m_bInstant && m_LifeTimeTimer.IsTimedOut())
		return false;

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNavMarkerFX::OnServerMessage
//
//	PURPOSE:	Handle recieving a message from the server...
//
// ----------------------------------------------------------------------- //

bool CNavMarkerFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg ))
		return false;

	m_cs.Read(pMsg);
	UpdateData();

	return true;
}

void CNavMarkerFX::UpdateData()
{
	HUDNavMarker_create HUDData;
	HUDData.m_bIsActive = m_cs.m_bIsActive;
	HUDData.m_hTarget = m_cs.m_hTarget;
	HUDData.m_hType = m_cs.m_hType;
	HUDData.m_nTeamId = m_cs.m_nTeamId;

	if	(!HUDData.m_hTarget)
		HUDData.m_hTarget = m_hServerObject;

	std::wstring sTmp;
	HUDData.m_pText = NULL;
	if( m_cs.m_nStringId != -1 )
	{
		sTmp = LoadString(m_cs.m_nStringId);
		HUDData.m_pText = sTmp.c_str();
	}
	else if( !m_cs.m_wsString.empty( ))
	{
		HUDData.m_pText = m_cs.m_wsString.c_str();
	}

	m_hTarget = HUDData.m_hTarget;

	if (m_pHUDItem)
	{
		m_pHUDItem->UpdateData(&HUDData);

		// Setup our pos if we're an instant navmarker.
		if( m_cs.m_bInstant )
		{
			m_pHUDItem->SetPos( m_cs.m_vPos );
		}
	}

	UpdateClientFX();
}

void CNavMarkerFX::UpdateClientFX()
{
	CClientFXMgr *pClientFXMgr = &g_pGameClientShell->GetSimulationTimeClientFXMgr();
	// Setup the Looping ClientFX...
	if( m_fxLoop.IsValid( ))
		pClientFXMgr->ShutdownClientFX( &m_fxLoop );

	if (GameModeMgr::Instance( ).m_grbUseTeams && m_cs.m_nTeamId != INVALID_TEAM && !g_pInterfaceMgr->GetClientInfoMgr()->IsLocalTeam(m_cs.m_nTeamId))
		return;

	const char *pszLoopFX = g_pNavMarkerTypeDB->GetClientFX(m_cs.m_hType);
	if( pszLoopFX && pszLoopFX[0] && m_hTarget )
	{
		CLIENTFX_CREATESTRUCT fxCS( pszLoopFX, FXFLAG_LOOP, m_hTarget );
		pClientFXMgr->CreateClientFX( &m_fxLoop, fxCS, true );
	}
}


void CNavMarkerFX::SetTarget(HLOCALOBJ hTarget)
{
	m_cs.m_hTarget = hTarget;
	m_hTarget = hTarget;
	UpdateData();
}

void CNavMarkerFX::SetTeamId( uint8 nTeamId )
{
	m_cs.m_nTeamId = nTeamId;
	UpdateData();
}