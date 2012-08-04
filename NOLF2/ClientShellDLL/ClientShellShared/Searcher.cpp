// ----------------------------------------------------------------------- //
//
// MODULE  : Searcher.cpp
//
// PURPOSE : Searcher implementation
//
// CREATED : 12/20/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "MsgIDs.h"
#include "Searcher.h"
#include "PlayerMgr.h"
#include "TargetMgr.h"
#include "PlayerStats.h"
#include "GameClientShell.h"
#include "BodyFX.h"
#include "SearchItemMgr.h"

extern VarTrack g_vtProgressBarScaleToSkills;

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSearcher::Update
//
//  PURPOSE:	Update the searcher...
//
// ----------------------------------------------------------------------- //

void CSearcher::Update( )
{
	// Are we actually searching something
	if( !m_bSearching || !m_hTargetObj )
	{
		return;
	}

	HOBJECT hTarget = m_hTargetObj;
	if (m_hTargetHitBox) 
		hTarget = m_hTargetHitBox;
	HOBJECT hObj = g_pPlayerMgr->GetTargetMgr()->GetLockedTarget();

	bool bLookingAtTarget = (hObj == hTarget);
	bool bInRange = (g_pPlayerMgr->GetTargetMgr()->IsTargetInRange());

	float fSearchSkillEffect = g_pPlayerStats->GetSkillModifier(SKL_SEARCH,SearchModifiers::eSearchSpeed);

	m_fTimer -= g_pLTClient->GetFrameTime() * fSearchSkillEffect;
	if (m_fTimer < 0.0f)
		m_fTimer = 0.0f;


	bool bButtonDown = !!g_pLTClient->IsCommandOn( COMMAND_ID_ACTIVATE);

	// Do we still meet the requirements for searching?
	if( m_fTimer <= 0.0f || !bButtonDown || !bLookingAtTarget || !bInRange )
	{

		// Send message to target with the amount of time left...

		CAutoMessage cMsg;

		cMsg.Writeuint8( MID_SEARCH );
		cMsg.WriteObject( m_hTargetObj );
		cMsg.Writefloat( m_fTimer);
		g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

		// Stop searching...
		m_bSearching	= false;
		m_hTargetObj	= LTNULL;
		m_hTargetHitBox	= LTNULL;
		g_pPlayerMgr->GetTargetMgr()->LockTarget(NULL);

		// Let the progress bar hide it's self...

		if( !bButtonDown || !bLookingAtTarget || !bInRange  )
		{
			m_bShowTimeBar	= false;
			g_pPlayerStats->UpdateProgress( 0 );
			g_pHUDMgr->QueueUpdate( kHUDProgressBar );
		}
	}

	// Get the percentage of searching we have done
	
	uint8 nMaxProgress = GetMaxProgress();
	uint8 nVal = uint8( ((m_fRemainingTime+m_fTimer) / m_fTotalTime) * nMaxProgress );

	// Update the meter...
	if( m_bShowTimeBar )
	{
		// Show the progress bar...

		g_pPlayerStats->UpdateMaxProgress( g_vtProgressBarScaleToSkills.GetFloat() > 1.0f ? 100 : nMaxProgress );
		g_pPlayerStats->UpdateProgress( nVal );
		g_pHUDMgr->QueueUpdate( kHUDProgressBar );
	}

	if (!nVal)
	{
		g_pClientSoundMgr->PlaySoundLocal("interface\\snd\\SearchComplete.wav", SOUNDPRIORITY_MISC_MEDIUM);
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSearcher::OnSearchMessage
//
//  PURPOSE:	Read in the search message and start searching
//
// ----------------------------------------------------------------------- //

void CSearcher::OnSearchMessage( ILTMessage_Read *pMsg )
{
	if( !pMsg ) return;

	uint8 nMsgType = pMsg->Readuint8();

	switch (nMsgType)
	{
	case SEARCH_START:
		{
			// Read the data for searching...
			m_hTargetObj = pMsg->ReadObject();
			m_fTotalTime = pMsg->Readfloat();
			m_fRemainingTime = pMsg->Readfloat();
			m_fTimer = pMsg->Readfloat();

			if (m_fRemainingTime >  m_fTimer)
				m_fRemainingTime -= m_fTimer;
			else
				m_fRemainingTime = 0.0f;

			m_bShowTimeBar = (m_fTimer > 0.0f);

			// Start searching
			
			m_bSearching = true;

			//are we searching a body? if so, we have to jump through some hoops
			CBodyFX* pBody = g_pGameClientShell->GetSFXMgr()->GetBodyFX(m_hTargetObj);
			if (pBody)
			{
				m_hTargetHitBox = pBody->GetHitBox();
			}
			else
			{
				m_hTargetHitBox = LTNULL;
			}

			if (m_hTargetHitBox)
				g_pPlayerMgr->GetTargetMgr()->LockTarget(m_hTargetHitBox);
			else
				g_pPlayerMgr->GetTargetMgr()->LockTarget(m_hTargetObj);

		} break;

	case SEARCH_FOUND:
		{
			uint8 nFind = pMsg->Readuint8();
			uint8 nId = pMsg->Readuint8();

			SEARCH_ITEM *pItem = g_pSearchItemMgr->GetItem(nId);
			if (pItem)
			{

				g_pPickupMsgs->AddMessage(LoadTempString(pItem->nTextId),pItem->szIcon);
//				g_pClientSoundMgr->PlaySoundLocal("interface\\snd\\SearchFind.wav", SOUNDPRIORITY_MISC_MEDIUM);

			}

		} break;
	}

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSearcher::GetMaxProgress
//
//  PURPOSE:	Figure out the max allowable progress for the search bar...
//
// ----------------------------------------------------------------------- //

uint8 CSearcher::GetMaxProgress()
{
	float fSearchSkillEffect = g_pPlayerStats->GetSkillModifier(SKL_SEARCH,SearchModifiers::eSearchSpeed);

	uint8 nMaxProgress = 100;
	if( g_vtProgressBarScaleToSkills.GetFloat() > 0.0f )
	{
		nMaxProgress = (fSearchSkillEffect > 0.0f ? uint8(100 / fSearchSkillEffect) : 0);
	}
	
	return nMaxProgress;
}