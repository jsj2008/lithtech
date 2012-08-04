// ----------------------------------------------------------------------- //
//
// MODULE  : HUDMgr.cpp
//
// PURPOSE : Implementation of CHUDMgr class
//
// CREATED : 07/17/01
//
// (c) 2001-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDMgr.h"
#include "GameClientShell.h"
#include "HUDMessageQueue.h"
#include "HUDChatInput.h"
#include "HUDDebugInput.h"
#include "HUDTransmission.h"
#include "HUDSubtitles.h"
#include "HUDDecision.h"
#include "HUDScores.h"
#include "HUDScoreDiff.h"
#include "HUDTeamScores.h"
#include "HUDAmmo.h"
#include "HUDCrosshair.h"
#include "HUDHealth.h"
#include "HUDHealth2.h"
#include "HUDArmor.h"
#include "HUDDamageDir.h"
#include "HUDDistance.h"
#include "HUDRadio.h"
#include "HUDRespawn.h"
#include "HUDSlowMo.h"
#include "HUDFlashlight.h"
#include "HUDTimer.h"
#include "HUDOverlay.h"
#include "HUDPlayerList.h"
#include "HUDMgr.h"
#include "HUDDebug.h"
#include "HUDDialogue.h"
#include "HUDEvidence.h"
#include "HUDGrenade.h"
#include "HUDGearList.h"
#include "HUDWeaponList.h"
#include "HUDSwap.h"
#include "HUDNavMarkerMgr.h"
#include "HUDFocus.h"
#include "HUDActivate.h"
#include "HUDToolSelect.h"
#include "HUDActivateObject.h"
#include "HUDSpectator.h"
#include "HUDAmmoStack.h"
#include "HUDBuildVersion.h"
#include "HUDTitleSafeAreaNTSC.h"
#include "HUDEndRoundMessage.h"
#include "HUDWeaponBreak.h"
#include "HUDCTFFlag.h"
#include "HUDCTFBase.h"
#include "HUDVote.h"
#include "HUDControlPoint.h"
#include "HUDControlPointBar.h"
#ifndef _FINAL 
// For debugging only in non-final builds.
#include "HUDInstinct.h"
#endif _FINAL 

#include "PlayerMgr.h"
#include "PlayerCamera.h"
#include "TurretFX.h"

CHUDMgr*			g_pHUDMgr = NULL;
CHUDChatMsgQueue*	g_pChatMsgs = NULL;
CHUDGameMsgQueue*	g_pGameMsgs = NULL;
CHUDChatInput*		g_pChatInput = NULL;
CHUDDebugInput*		g_pDebugInput = NULL;
CHUDTransmission*	g_pTransmission = NULL;
CHUDEndRoundMessage*g_pEndRoundMessage = NULL;
CHUDSubtitles*		g_pSubtitles = NULL;
CHUDDecision*		g_pDecision = NULL;
CHUDPaused*			g_pPaused = NULL;
CHUDScores*			g_pScores = NULL;
CHUDCrosshair*		g_pCrosshair = NULL;
CHUDRadio*			g_pRadio = NULL;
CHUDTimerMain*		g_pMainTimer = NULL;
CHUDTimerTeam0*		g_pTeam0Timer = NULL;
CHUDTimerTeam1*		g_pTeam1Timer = NULL;
CHUDOverlayMgr*		g_pOverlay = NULL;
CHUDDebug*			g_pHUDDebug = NULL;
CHUDDialogue*		g_pHUDDialogue = NULL;
CHUDEvidence*		g_pHUDEvidence = NULL;
CHUDNavMarkerMgr*	g_pNavMarkerMgr = NULL;
CHUDDistance*		g_pDistance = NULL;
CHUDToolSelect*		g_pHUDToolSelect = NULL;
CHUDActivateObject*	g_pHUDActivateObject = NULL;
CHUDSpectator*		g_pHUDSpectator = NULL;
CHUDSlowMo*			g_pHUDSlowMo = NULL;
CHUDFlashlight*		g_pHUDFlashlight = NULL;
CHUDAmmoStack*		g_pHUDAmmoStack = NULL;
CHUDWeaponList*		g_pHUDWeaponList = NULL;
CHUDGrenadeList*		g_pHUDGrenadeList = NULL;
CHUDCTFFlag*		g_pHUDCTFFlag = NULL;
CHUDCTFBaseFriendly*	g_pHUDCTFBaseFriendly = NULL;
CHUDCTFBaseEnemy*		g_pHUDCTFBaseEnemy = NULL;
CHUDVote*			g_pHUDVote = NULL;
CHUDControlPointList*	g_pHUDControlPointList = NULL;
CHUDControlPointBar*	g_pHUDControlPointBar = NULL;

extern bool	g_bScreenShotMode;
extern VarTrack			g_vtAlwaysHUD;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AllocateHUDItem()
//
//	PURPOSE:	Helper function which allocates a class of the template 
//				specified type, adds it to the specified list, and optionally 
//				sets the specified global pointer .
//
// ----------------------------------------------------------------------- //
template <typename T>
static void AllocateHUDItem(CHUDMgr::ItemArray* poutItemArray, T** m_ppoutGlobalPtr = NULL)
{
	ASSERT(poutItemArray);
	if (!poutItemArray)
	{
		return;
	}

	T* pT = debug_new(T);

	// Set the global pointer, if one was specified.
	if (m_ppoutGlobalPtr)
	{
		ASSERT(*m_ppoutGlobalPtr == NULL);
		*m_ppoutGlobalPtr = pT;
	}

	// add the item to the item array.
	poutItemArray->push_back(pT);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDMgr::CHUDMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CHUDMgr::CHUDMgr() :
	m_eLevel(kHUDRenderNone),
	m_fFlicker(0.0f),
	m_fFlickerDuration(0.0f)
{
	m_itemArray.reserve(20);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDMgr::~CHUDMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CHUDMgr::~CHUDMgr()
{
	Term();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

bool CHUDMgr::Init()
{
	// NOTE: Crosshair-type elements should be first so they are rendered first (potential overlap)

#if defined(PROJECT_FEAR)

	AllocateHUDItem<CHUDNavMarkerMgr>(&m_itemArray, &g_pNavMarkerMgr);
	AllocateHUDItem<CHUDCrosshair>(&m_itemArray);
	AllocateHUDItem<CHUDOverlayMgr>(&m_itemArray, &g_pOverlay);
	AllocateHUDItem<CHUDAmmo>(&m_itemArray);
	AllocateHUDItem<CHUDGrenade>(&m_itemArray);
	AllocateHUDItem<CHUDDialogue>(&m_itemArray, &g_pHUDDialogue);
	AllocateHUDItem<CHUDGearList>(&m_itemArray);
	AllocateHUDItem<CHUDWeaponList>(&m_itemArray,&g_pHUDWeaponList);
	AllocateHUDItem<CHUDGrenadeList>(&m_itemArray,&g_pHUDGrenadeList);
	AllocateHUDItem<CHUDSwap>(&m_itemArray);
	AllocateHUDItem<CHUDHealth>(&m_itemArray);
	AllocateHUDItem<CHUDArmor>(&m_itemArray);
	AllocateHUDItem<CHUDDamageDir>(&m_itemArray);
	AllocateHUDItem<CHUDDistance>(&m_itemArray, &g_pDistance);
	AllocateHUDItem<CHUDRadio>(&m_itemArray, &g_pRadio);
	AllocateHUDItem<CHUDRespawn>(&m_itemArray);
	AllocateHUDItem<CHUDSlowMo>(&m_itemArray,&g_pHUDSlowMo);
	AllocateHUDItem<CHUDFlashlight>(&m_itemArray,&g_pHUDFlashlight);
	AllocateHUDItem<CHUDDecision>(&m_itemArray, &g_pDecision);
	AllocateHUDItem<CHUDSubtitles>(&m_itemArray, &g_pSubtitles);
	AllocateHUDItem<CHUDChatMsgQueue>(&m_itemArray, &g_pChatMsgs);
	AllocateHUDItem<CHUDGameMsgQueue>(&m_itemArray, &g_pGameMsgs);
	AllocateHUDItem<CHUDChatInput>(&m_itemArray, &g_pChatInput);
	AllocateHUDItem<CHUDDebugInput>(&m_itemArray, &g_pDebugInput);
	AllocateHUDItem<CHUDTransmission>(&m_itemArray, &g_pTransmission);
	AllocateHUDItem<CHUDEndRoundMessage>(&m_itemArray, &g_pEndRoundMessage);
	AllocateHUDItem<CHUDTimerMain>(&m_itemArray, &g_pMainTimer);
	AllocateHUDItem<CHUDTimerTeam0>(&m_itemArray, &g_pTeam0Timer);
	AllocateHUDItem<CHUDTimerTeam1>(&m_itemArray, &g_pTeam1Timer);
	AllocateHUDItem<CHUDPlayerList>(&m_itemArray);
	AllocateHUDItem<CHUDDebug>(&m_itemArray, &g_pHUDDebug);
	AllocateHUDItem<CHUDActivateObject>(&m_itemArray, &g_pHUDActivateObject);
	AllocateHUDItem<CHUDSpectator>(&m_itemArray, &g_pHUDSpectator);
	AllocateHUDItem<CHUDScores>(&m_itemArray, &g_pScores);
	AllocateHUDItem<CHUDScoreDiff>(&m_itemArray);
	AllocateHUDItem<CHUDCTFFlag>(&m_itemArray,&g_pHUDCTFFlag);
	AllocateHUDItem<CHUDCTFBaseFriendly>(&m_itemArray,&g_pHUDCTFBaseFriendly);
	AllocateHUDItem<CHUDCTFBaseEnemy>(&m_itemArray,&g_pHUDCTFBaseEnemy);
	AllocateHUDItem<CHUDVote>(&m_itemArray, &g_pHUDVote);
	AllocateHUDItem<CHUDControlPointList>(&m_itemArray, &g_pHUDControlPointList);
	AllocateHUDItem<CHUDControlPointBar>(&m_itemArray, &g_pHUDControlPointBar);
	AllocateHUDItem<CHUDTeamScores>(&m_itemArray);


#elif defined(PROJECT_DARK)

	AllocateHUDItem<CHUDFocus>(&m_itemArray);
	AllocateHUDItem<CHUDOverlayMgr>(&m_itemArray, &g_pOverlay);
	AllocateHUDItem<CHUDHealth2>(&m_itemArray);
	AllocateHUDItem<CHUDEvidence>(&m_itemArray, &g_pHUDEvidence);
	AllocateHUDItem<CHUDDialogue>(&m_itemArray, &g_pHUDDialogue);
	AllocateHUDItem<CHUDSubtitles>(&m_itemArray, &g_pSubtitles);
	AllocateHUDItem<CHUDDebug>(&m_itemArray, &g_pHUDDebug);
	AllocateHUDItem<CHUDGameMsgQueue>(&m_itemArray, &g_pGameMsgs);
	AllocateHUDItem<CHUDToolSelect>(&m_itemArray, &g_pHUDToolSelect);
	AllocateHUDItem<CHUDActivateObject>(&m_itemArray, &g_pHUDActivateObject);
	AllocateHUDItem<CHUDAmmoStack>(&m_itemArray, &g_pHUDAmmoStack);
	AllocateHUDItem<CHUDBuildVersion>(&m_itemArray);
	AllocateHUDItem<CHUDWeaponBreak>(&m_itemArray);

#if defined (PLATFORM_XENON)
	AllocateHUDItem<CHUDTitleSafeAreaNTSC>(&m_itemArray);
#endif
#ifndef _FINAL 
	// For debugging only in non-final builds.
	AllocateHUDItem<CHUDInstinct>(&m_itemArray);
#endif _FINAL 


#else
#pragma message( "Invalid PROJECT_ define" )
#endif


	g_pPaused = &m_Paused;


	ItemArray::iterator iter = m_itemArray.begin();
	while (iter != m_itemArray.end())
	{
		if (!(*iter)->Init())
			return false;
		iter++;
	}

	m_UpdateFlags = ( uint32 )kHUDAll;

	g_pHUDMgr = this;

	m_bVisible = true;
    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDMgr::Term()
//
//	PURPOSE:	Terminate the HUD
//
// ----------------------------------------------------------------------- //

void CHUDMgr::Term()
{
	ItemArray::iterator iter = m_itemArray.begin();

	while (iter != m_itemArray.end())
	{
		(*iter)->Term();
		debug_delete(*iter);
		iter++;
	}

	m_itemArray.clear();

	g_pHUDMgr = NULL;

	// Invalidate all of the hud pointer.
	g_pChatMsgs = NULL;
	g_pChatInput = NULL;
	g_pDebugInput = NULL;
	g_pTransmission = NULL;
	g_pEndRoundMessage = NULL;
	g_pSubtitles = NULL;
	g_pDecision = NULL;
	g_pPaused = NULL;
	g_pScores = NULL;
	g_pRadio = NULL;
	g_pMainTimer = NULL;
	g_pTeam0Timer = NULL;
	g_pTeam1Timer = NULL;
	g_pOverlay = NULL;
	g_pHUDDebug = NULL;
	g_pHUDDialogue = NULL;
	g_pHUDEvidence = NULL;
	g_pHUDToolSelect = NULL;
	g_pHUDActivateObject = NULL;
	g_pHUDAmmoStack = NULL;
	g_pHUDWeaponList = NULL;
	g_pHUDGrenadeList = NULL;
	g_pHUDCTFFlag = NULL;
	g_pHUDCTFBaseFriendly = NULL;
	g_pHUDCTFBaseEnemy = NULL;
	g_pHUDVote = NULL;
	g_pHUDControlPointList = NULL;
	g_pHUDControlPointBar = NULL;
}


void CHUDMgr::Update()
{
	m_UpdateFlags |= kHUDFrame;

	if (m_fFlicker > 0.0f)
	{
		float fDecay = RealTimeTimer::Instance().GetTimerElapsedS( );
		if (m_fFlickerDuration > 0.0f)
		{
			fDecay /= m_fFlickerDuration;
		}
		if (m_fFlicker < fDecay)
		{
			EndFlicker();			
		}
		else
		{
			m_fFlicker -= fDecay;
			if (m_fFlicker < 0.5f)
			{
				g_pOverlay->Hide( g_pOverlay->GetHUDOverlay( "SignalStatic" ));
			}
		}
	}

	ItemArray::iterator iter = m_itemArray.begin();

	while (iter != m_itemArray.end())
	{
		if (m_UpdateFlags & (*iter)->GetUpdateFlags())
			(*iter)->Update();
		(*iter)->UpdateFade();
		if (m_fFlicker > 0.0f)
			(*iter)->UpdateFlicker();
		(*iter)->UpdateFlash();

		iter++;
	}

	m_UpdateFlags = kHUDNone;


}

void CHUDMgr::Reset()
{
	ItemArray::iterator iter = m_itemArray.begin();

	while (iter != m_itemArray.end())
	{
		(*iter)->Reset();
		iter++;
	}
	m_UpdateFlags = kHUDAll;

}

void CHUDMgr::Render( EHUDRenderLayer eHUDRenderLayer )
{
	if (!m_bVisible) return;

	//we only perform draw primitive calls within here, so what we want to do is start a draw
	//primitive block so that it only needs to be initialized once
	g_pLTClient->GetDrawPrim()->BeginDrawPrimBlock();

	ItemArray::iterator iter = m_itemArray.begin();

	for( ; iter != m_itemArray.end(); iter++ )
	{
		CHUDItem* pHudItem = *iter;
		if( pHudItem->GetHUDRenderLayer() != eHUDRenderLayer )
			continue;

		//we need to make sure that this hud item is up to date though
		if (m_UpdateFlags & (*iter)->GetUpdateFlags())
			pHudItem->Update();

		if (pHudItem->GetRenderLevel() <= m_eLevel)
			pHudItem->Render();
	}

	//and we no longer need draw primitive
	g_pLTClient->GetDrawPrim()->EndDrawPrimBlock();	
}


void CHUDMgr::QueueUpdate(uint32 nUpdateFlag)
{
	m_UpdateFlags |= nUpdateFlag;
}

void CHUDMgr::ScreenDimsChanged()
{
	ItemArray::iterator iter = m_itemArray.begin();
	while (iter != m_itemArray.end())
	{

		(*iter)->ScaleChanged();
		iter++;
	}
}

void CHUDMgr::OnExitWorld()
{
	EndFlicker();

	ItemArray::iterator iter = m_itemArray.begin();

	while (iter != m_itemArray.end())
	{
		(*iter)->OnExitWorld();
		iter++;
	}

	if (g_pHUDDialogue)
		g_pHUDDialogue->HideAll();

	if (g_pHUDEvidence)
		g_pHUDEvidence->HideAll();
}
void CHUDMgr::UpdateLayout()
{
	ItemArray::iterator iter = m_itemArray.begin();

	while (iter != m_itemArray.end())
	{
		(*iter)->UpdateLayout();
		iter++;
	}


	m_UpdateFlags = ( uint32 )kHUDAll;
}



//reset the fade for all items (i.e. show everything)
void CHUDMgr::ResetAllFades()
{
	ItemArray::iterator iter = m_itemArray.begin();
	while (iter != m_itemArray.end())
	{
		(*iter)->ResetFade();
		iter++;
	}

}


void CHUDMgr::StartFlicker(float fDuration)
{
	g_pOverlay->Show( g_pOverlay->GetHUDOverlay( "SignalStatic" ));
	g_pHUDDialogue->Show("Signal",INVALID_CLIENT);
	m_fFlickerDuration = fDuration;
	m_fFlicker = 1.0;
	ItemArray::iterator iter = m_itemArray.begin();
	while (iter != m_itemArray.end())
	{
		(*iter)->StartFlicker();
		iter++;
	}

}

void CHUDMgr::EndFlicker()
{
	g_pOverlay->Hide( g_pOverlay->GetHUDOverlay( "SignalStatic" ));
	g_pHUDDialogue->Hide("Signal");

	m_fFlicker = 0.0f;
	ItemArray::iterator iter = m_itemArray.begin();
	while (iter != m_itemArray.end())
	{
		(*iter)->EndFlicker();
		iter++;
	}
}

float CHUDMgr::GetFlickerLevel()
{
	return m_fFlicker;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDMgr::SetRenderLevel
//
//  PURPOSE:	Update the HUD render level based on the current game state
//
// ----------------------------------------------------------------------- //

void CHUDMgr::UpdateRenderLevel()
{
	eHUDRenderLevel eLevel = kHUDRenderFull;

	if (g_bScreenShotMode)
	{
		SetRenderLevel(kHUDRenderNone);
		return;
	}

	if(!g_pPlayerMgr->IsPlayerAlive())
	{
		SetRenderLevel(kHUDRenderDead);
		return;
	}

	CPlayerCamera::CameraMode cm = g_pPlayerMgr->GetPlayerCamera()->GetCameraMode();
	if (cm==CPlayerCamera::kCM_Cinematic)
	{
		SetRenderLevel( kHUDRenderText );
		return;
	}

	// Don't show stats in spectator mode...
	// Unless "alwaysHUD" console variable is set.
	if (g_pPlayerMgr->IsSpectating( ) && ( g_vtAlwaysHUD.GetFloat() == 0.0f ) )
	{
		SetRenderLevel(kHUDRenderDead);
		return;
	}

	switch (g_pPlayerMgr->GetPlayerState())
	{
		case ePlayerState_Dying_Stage1:
		case ePlayerState_Dying_Stage2:
		case ePlayerState_Dead:
		case ePlayerState_None:
		{
			if( IsMultiplayerGameClient() )
			{
				SetRenderLevel(kHUDRenderDead);
			}
			else
			{
				SetRenderLevel(kHUDRenderNone);
			}
			return;
		}
		break;
	};


	if (g_pPlayerMgr->IsOperatingTurret())
	{
		CTurretFX* pTurretFX = g_pPlayerMgr->GetTurret();
		if (pTurretFX)
		{
			HTURRET hTurret = pTurretFX->GetTurretRecord();
			if (hTurret && g_pWeaponDB->GetBool( hTurret, WDB_TURRET_bHideHUD ))
			{
				SetRenderLevel(kHUDRenderText);
				return;
			}
		}
	}

	if (g_pPlayerMgr->InStoryMode())
	{
		SetRenderLevel(kHUDRenderText);
		return;
	}

	SetRenderLevel(kHUDRenderFull);
}