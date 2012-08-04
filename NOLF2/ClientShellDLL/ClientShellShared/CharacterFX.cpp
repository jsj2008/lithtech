// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterFX.cpp
//
// PURPOSE : Character special FX - Implementation
//
// CREATED : 8/24/98
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "CharacterFX.h"
#include "GameClientShell.h"
#include "ParticleTrailFX.h"
#include "SmokeFX.h"
#include "SFXMsgIds.h"
#include "ClientUtilities.h"
#include "SoundMgr.h"
#include "iltphysics.h"
#include "SurfaceFunctions.h"
#include "BaseScaleFX.h"
#include "VarTrack.h"
#include "ClientButeMgr.h"
#include "MsgIDs.h"
#include "PlayerShared.h"
#include "VolumeBrushFX.h"
#include "CMoveMgr.h"
#include "VehicleMgr.h"
#include "PlayerCamera.h"
#include "ClientWeaponBase.h"
#include "ClientWeaponMgr.h"


extern CGameClientShell* g_pGameClientShell;
extern CClientButeMgr* g_pClientButeMgr;

extern LTVector g_vPlayerCameraOffset;

#define INTERSECT_Y_OFFSET		50.0f
#define FOOTSTEP_SOUND_RADIUS	1000.0f
#define KEY_FOOTSTEP_SOUND		"FOOTSTEP_KEY"
#define KEY_BUTE_SOUND			"BUTE_SOUND_KEY"
#define KEY_MOVEMENT_LOUD		"MOVE_LOUD_KEY"
#define KEY_MOVEMENT_QUIET		"MOVE_QUIET_KEY"
#define KEY_SHOW_ATTACHFX		"SHOW_ATTACHFX" // SHOW_ATTACHFX n
#define KEY_HIDE_ATTACHFX		"HIDE_ATTACHFX" // HIDE_ATTACHFX n
#define KEY_FX					"FX"			// HIDE_ATTACHFX n


#define SUBTITLE_STRINGID_OFFSET	0
#define DEFAULT_TAUNT_RADIUS		1500.0f
#define DEFAULT_VEHICLE_RADIUS		2500.0f

#define DEFAULT_CIGARETTE_TEXTURE	"SFX\\Impact\\Spr\\Smoke.spr"
#define DEFAULT_ZZZ_TEXTURE			"Interface\\hud\\icon_sleep.dtx"
#define DEFAULT_HEART_TEXTURE		"SFX\\Particle\\Heart.dtx"
#define DEFAULT_SMOKEPUFF_TEXTURE	""

#define RADAR_AI_TRACKING			"AITracking"
#define RADAR_PLAYER_TRACKING		"PlayerTracking"

SurfaceType g_eClientLastSurfaceType = ST_UNKNOWN;


VarTrack g_vtBreathTime;
VarTrack g_vtModelKey;
VarTrack g_vtFootPrintBlend;
VarTrack g_vtMinTrailSegment;
VarTrack g_vtTrailSegmentLifetime;
VarTrack g_vtDialogueCinematicSoundRadius;
VarTrack g_vtVehicleTrials;
VarTrack g_vtDingDelay;

VarTrack g_vtOrientOnAnim;
VarTrack g_vtPlayerPitchTracking;
VarTrack g_vtUpperTorsoPitchMax;
VarTrack g_vtLowerTorsoPitchMax;
VarTrack g_vtPitchScale;
VarTrack g_vtPitchBias;

VarTrack g_vtQuietMovementVolumeFactor;

extern VarTrack g_vtVehicleContourExtraDimsZ;
extern VarTrack g_vtVehicleContourExtraDimsX;
extern VarTrack g_vtVehicleContourMaxRotation;

///////////////

const LTVector g_kvPlayerScubaCamOffset(0.0, 41.0, 0.0);

CCharacterFX::CharFXList CCharacterFX::m_lstPlayersInGame;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::Init
//
//	PURPOSE:	Init the character fx
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
    if (!CSpecialFX::Init(hServObj, pMsg)) return LTFALSE;
    if (!pMsg) return LTFALSE;

	CHARCREATESTRUCT ch;

	ch.hServerObj = hServObj;
    ch.Read(pMsg);

	return Init(&ch);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::Init
//
//	PURPOSE:	Init the character fx
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	m_cs = *((CHARCREATESTRUCT*)psfxCreateStruct);

	// Init the node controller

	if ( !m_NodeController.Init(this) )
	{
        return LTFALSE;
	}

	// Init our 3rd person flashlight fx (i.e., what other players see)...

	m_3rdPersonFlashlight.Init(m_hServerObject);

	HOBJECT hLocalObj = g_pLTClient->GetClientObject();
	
	// Only do pitch node tracking if it's a non-local player in a multiplayer game...
   
	if( m_cs.bIsPlayer && IsMultiplayerGame() )
   	{
		if (m_hServerObject != hLocalObj)
		{

			// create the chat icon
			static char szChatFX[128] = "";
			if (!szChatFX[0])
				g_pClientButeMgr->GetSpecialFXAttributeString("ChatFX",szChatFX,sizeof(szChatFX));

			CLIENTFX_CREATESTRUCT fxInit( szChatFX, FXFLAG_LOOP, m_hServerObject );

			g_pClientFXMgr->CreateClientFX( &m_linkChatFX, fxInit, true );
			if ( m_linkChatFX.IsValid() ) 
			{
				// start out hidden
				m_linkChatFX.GetInstance()->Hide();
			}
		}
	}

	ResetPitchTracking();

	// Size the damageFX vector to the number of damagefx we have...

	//make sure that we don't have any old arrays lying around
	debug_deletea(m_p3rdPersonDamageFX);
	m_p3rdPersonDamageFX = NULL;
	m_nNum3rdPersonDamageFX = 0;

	if( g_pDamageFXMgr )
	{
		m_p3rdPersonDamageFX = debug_newa(CLIENTFX_LINK, g_pDamageFXMgr->GetNumDamageFX() );
		if(m_p3rdPersonDamageFX)
			m_nNum3rdPersonDamageFX = g_pDamageFXMgr->GetNumDamageFX();
	}

	// Create the hitbox object

	m_HitBox.Init( m_hServerObject, m_cs.vHitBoxDims, m_cs.vHitBoxOffset );

	// Add to radar if tracking...
	if( m_cs.bRadarVisible )
	{
		if( m_cs.bIsPlayer )
		{
			if( IsMultiplayerGame( ))
			{
				g_pRadar->AddPlayer( m_hServerObject, m_cs.nClientID );
			}
		}
		else
		{
			g_pRadar->AddObject( m_hServerObject, RADAR_AI_TRACKING, INVALID_TEAM );
		}
	}

	if( m_cs.bTracking )
	{
		if( m_cs.bIsPlayer )
		{
			g_pRadar->AddObject( m_hServerObject, RADAR_PLAYER_TRACKING, INVALID_TEAM );
		}
		else
		{
			g_pRadar->AddObject( m_hServerObject, RADAR_AI_TRACKING, INVALID_TEAM );
		}
	}

	UpdateAttachments();

	m_nUniqueDialogueId = 0;
	m_bSubtitlePriority	= false;

	if( m_cs.bIsPlayer )
	{
		// Add this player to the list of players in game...
		m_lstPlayersInGame.push_back( this );
	}

	UpdateCarrying();


	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::~CCharacterFX()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CCharacterFX::~CCharacterFX()
{
	RemoveUnderwaterFX();
	RemoveCigaretteFX();
	RemoveZzzFX();
	RemoveFlashLightFX();
	RemoveAttachClientFX();
	
	if (m_hDialogueSnd)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hDialogueSnd);
	}

	if (m_hVehicleSound)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hVehicleSound);
	}

	if( m_hUpperTorsoNode != INVALID_TRACKEDNODE )
	{
		g_pClientTrackedNodeMgr->DestroyTrackingNode( m_hUpperTorsoNode );
		m_hUpperTorsoNode = INVALID_TRACKEDNODE;
	}

	if( m_hLowerTorsoNode != INVALID_TRACKEDNODE )
	{
		g_pClientTrackedNodeMgr->DestroyTrackingNode( m_hLowerTorsoNode );
		m_hLowerTorsoNode = INVALID_TRACKEDNODE;
	}

	if( m_hHeadNode != INVALID_TRACKEDNODE )
	{
		g_pClientTrackedNodeMgr->DestroyTrackingNode( m_hHeadNode );
		m_hHeadNode = INVALID_TRACKEDNODE;
	}

	KillWeaponLoopSound();

 	if (m_pStr)
	{
		g_pFontManager->DestroyPolyString(m_pStr);
        m_pStr=LTNULL;
	}

	// Shutdown any damageFX...
	
	ShutdownDamageFX();

	//free all the 3rd person damage effects
	debug_deletea(m_p3rdPersonDamageFX);
	m_p3rdPersonDamageFX = NULL;
	
	if( m_linkChatFX.IsValid() )
	{
		g_pClientFXMgr->ShutdownClientFX( &m_linkChatFX );
	}

	CharFXList::iterator iter = m_lstPlayersInGame.begin();
	while( iter != m_lstPlayersInGame.end() )
	{
		if( *iter == this )
		{
			m_lstPlayersInGame.erase( iter );
			break;
		}

		++iter;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateObject
//
//	PURPOSE:	Create the various fx
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterFX::CreateObject(ILTClient* pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return LTFALSE;

	if (!g_vtBreathTime.IsInitted())
	{
        g_vtBreathTime.Init(pClientDE, "BreathTime", LTNULL, 5.0f);
	}

	if (!g_vtModelKey.IsInitted())
	{
        g_vtModelKey.Init(pClientDE, "ModelKey", LTNULL, 0.0f);
	}

	if (!g_vtFootPrintBlend.IsInitted())
	{
        g_vtFootPrintBlend.Init(pClientDE, "FootPrintBlendMode", LTNULL, 2.0f);
	}

	if (!g_vtMinTrailSegment.IsInitted())
	{
        g_vtMinTrailSegment.Init(pClientDE, "MinTrailSegment", LTNULL, 25.0f);
	}

	if (!g_vtTrailSegmentLifetime.IsInitted())
	{
        g_vtTrailSegmentLifetime.Init(pClientDE, "TrailSegmentLifetime", LTNULL, 15.0f);
	}

	if (!g_vtDialogueCinematicSoundRadius.IsInitted())
	{
        g_vtDialogueCinematicSoundRadius.Init(pClientDE, "DialogueCinematicSndRadius", LTNULL, 10000.0f);
	}

	if (!g_vtVehicleTrials.IsInitted())
	{
        g_vtVehicleTrials.Init(pClientDE, "VehicleTrails", LTNULL, 0.0f);
	}

	if (!g_vtDingDelay.IsInitted())
	{
        g_vtDingDelay.Init(pClientDE, "DingDelay", LTNULL, 1.0f);
	}

	if( !g_vtOrientOnAnim.IsInitted() )
	{
		g_vtOrientOnAnim.Init( g_pLTClient, "OrientOnAnim", LTNULL, 1.0f );
	}

	if( !g_vtPlayerPitchTracking.IsInitted() )
	{
		g_vtPlayerPitchTracking.Init(g_pLTClient, "PlayerPitchTracking", LTNULL, 1.0f);
	}

	if( !g_vtUpperTorsoPitchMax.IsInitted() )
	{
		g_vtUpperTorsoPitchMax.Init( g_pLTClient, "UpperTorsoPitchMax", LTNULL, 25.0f );
	}

	if( !g_vtLowerTorsoPitchMax.IsInitted() )
	{
		g_vtLowerTorsoPitchMax.Init( g_pLTClient, "LowerTorsoPitchMax", LTNULL, 45.0f );
	}

	if( !g_vtPitchScale.IsInitted() )
	{
		g_vtPitchScale.Init( g_pLTClient, "PitchScale", LTNULL, 0.5f );
	}

	if( !g_vtPitchBias.IsInitted() )
	{
		g_vtPitchBias.Init( g_pLTClient, "PitchBias", LTNULL, -3.0f );
	}

	if( !g_vtQuietMovementVolumeFactor.IsInitted() )
	{
		g_vtQuietMovementVolumeFactor.Init( g_pLTClient, "QuietMovementVolumeFactor", LTNULL, 0.65f );
	}

	//initialize our breath information
	m_fBreathElapsedTime = 0.0f;
	m_fBreathEndTime = g_vtBreathTime.GetFloat();
	
	// NOTE: Since we only use node control for the mouth now, we can safely use CF_INSIDERADIUS
	g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Client, CF_NOTIFYMODELKEYS, CF_NOTIFYMODELKEYS);

	// Set up MoveMgr's point to us, if applicable...

    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();

	if( hPlayerObj == m_hServerObject)
	{
		InitLocalPlayer();
	}
	else // AI
	{
		uint32 dwFlags = FLAG_SOLID | ((IsMultiplayerGame()) ? (FLAG_STAIRSTEP | FLAG_GRAVITY) : 0);

		g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags, dwFlags, dwFlags);
	}

	uint8 nFont = g_pLayoutMgr->GetHUDFont();
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);

	m_pStr = g_pFontManager->CreateFormattedPolyString(pFont,"",0.0f, 0.0f);
	m_pStr->SetColor(argbWhite);
	m_pStr->SetAlignmentH(CUI_HALIGN_CENTER);

	// create and show all client effects associated with this model
	CreateAttachClientFX();
	//ShowAttachClientFX();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::Update
//
//	PURPOSE:	Update the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterFX::Update()
{
    if (!m_pClientDE || !m_hServerObject || m_bWantRemove) return LTFALSE;

	// Update our hitbox...
	// We need to do this here incase of the early out when the server object is inactive. 
	// The hitbox should always be in the correct position.

	m_HitBox.Update();


	// See if our server side object is active

	uint32 dwUserFlags(0);
	g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_User, dwUserFlags);

	if ( !(dwUserFlags & USRFLG_GAMEBASE_ACTIVE) )
	{
		return LTTRUE;
	}

	// Make us solid if our ai usrflg solid is set

    uint32 dwFlags(0);

	if ( dwUserFlags & USRFLG_AI_CLIENT_SOLID )
	{
		dwFlags |= FLAG_SOLID;
	}
	else if ( m_cs.bIsPlayer )
	{
		g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_Flags, dwFlags);
	}

	g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags, dwFlags, FLAG_SOLID);


	// Update

    g_pLTClient->ProcessAttachments(m_hServerObject);

    uint32 dwUsrFlags;
	g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_User, dwUsrFlags);

    LTVector vPos;
	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

	m_vStrPos = vPos;
	m_vStrPos.y += 64.0f;


	if (dwUsrFlags & USRFLG_PLAYER_UNDERWATER)
	{
		UpdateUnderwaterFX(vPos);
	}
	else
	{
		RemoveUnderwaterFX();
	}

	// Update the alpha cycling.
	if( dwUsrFlags & USRFLG_PLAYER_ALPHACYCLE )
	{
		UpdatePlayerAlphaCycle( );
	}
	else if( m_PlayerAlphaCycleTimer.On( ))
	{
		RemovePlayerAlphaCycle( );
	}

	// Update the character damage FX.  1st and 3rd person.
   
	UpdateDamageFX();

	// Update the pitch of this character
	UpdatePitch();

	// Update various FX

	if ( m_cs.byFXFlags & CHARCREATESTRUCT::eZzz )
	{
		UpdateZzzFX();
	}
	else
	{
		RemoveZzzFX();
	}

	if ( m_cs.byFXFlags & CHARCREATESTRUCT::eCigarette )
	{
		UpdateCigaretteFX();
	}
	else
	{
		RemoveCigaretteFX();
	}

	if ( m_cs.byFXFlags & CHARCREATESTRUCT::eFlashLight )
	{
		UpdateFlashLightFX();
	}
	else
	{
		RemoveFlashLightFX();
	}

	// Update node controller...

	m_NodeController.Update();


	// Update being on vehicle if necessary...

    UpdateOnVehicle();


	// Update breath fx...

	UpdateBreathFX();


	// Update our sounds...

	UpdateSounds();



	if (m_pStr && m_pStr->GetLength())
	{
		LTVector pos = g_pInterfaceMgr->GetScreenFromWorldPos(m_vStrPos, g_pPlayerMgr->GetCamera());

		if (pos.z > 0.0f && pos.z < 900.0f && pos.x > 0 && pos.y > 0)
		{
			
			uint8 h = (uint8)(g_pInterfaceResMgr->GetXRatio() * 5000.0f / (100.0f + pos.z));

			if (h != m_pStr->GetCharScreenHeight())
			{
				m_pStr->SetCharScreenHeight(h);
			}
			
			m_pStr->SetPosition( pos.x, pos.y);
		}
		else
		{
			m_pStr->SetPosition( 2000.0f, -1000.0f);
		}
	}


	if (m_bUpdateAttachments)
		UpdateAttachments();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateDamageFX
//
//	PURPOSE:	Update our damage fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateDamageFX()
{
	if( !m_hServerObject )
		return;

	//hack to make instant choke damage act like progressive damage
	static DamageFlags chokeFlag = DamageTypeToFlag(DT_CHOKE);
	if (m_nInstantDamageFlags & chokeFlag)
	{
		m_cs.nDamageFlags |= chokeFlag;
	}
	else
	{
		m_cs.nDamageFlags &= ~chokeFlag;
	}

	HLOCALOBJ	hPlayerObj = g_pLTClient->GetClientObject();
	bool		bLocal = (hPlayerObj == m_hServerObject);
	bool		bPlayingAni = false;

	CMoveMgr* pMoveMgr = g_pPlayerMgr->GetMoveMgr();
	if (!pMoveMgr) return;

	// Check our main tracker to see we if we are done playing any special damage anis...

	if (m_cs.bIsPlayer)
	{
		HMODELANIM	hBaseAnim, hCurAnim;
		g_pModelLT->GetAnimIndex( m_hServerObject, "base", hBaseAnim );
		g_pModelLT->GetCurAnim( m_hServerObject, MAIN_TRACKER, hCurAnim );

		if( hBaseAnim != INVALID_MODEL_ANIM && hCurAnim != INVALID_MODEL_ANIM )
		{
			if ( hCurAnim == hBaseAnim )
			{
				bPlayingAni = false;
			}
			else if ( bLocal && (pMoveMgr->IsBodyOnLadder() || m_bOnVehicle || m_bPlayerDead) )
			{
				// If we're on a ladder, vehicle, or dead we're not playing the damage ani...
				bPlayingAni = false;
			}
			else if ( !bLocal && (m_bOnVehicle || m_bPlayerDead) )
			{
				// Note: currently no way to determine if we're on a ladder or not...
				bPlayingAni = false;
			}
			else
			{
				bPlayingAni = true;
			}

			m_bDamageFxTrackingOverride = bPlayingAni;
		}
	}

	// No need to go further if our damageflags haven't changed...

	if( (m_nLastDamageFlags == m_cs.nDamageFlags) && (m_nInstantDamageFlags == 0) && !m_bWasPlayingSpecialDamageAni )
	{
		return;
	}

	m_nLastDamageFlags = m_cs.nDamageFlags;
   	
	DAMAGEFX *pDamageFX = g_pDamageFXMgr->GetFirstDamageFX();
	while( pDamageFX )
	{
		// Test the damage flags against the DamageFX...

		if( m_nLastDamageFlags & pDamageFX->m_nDamageFlag || pDamageFX->m_vtTestFX.GetFloat() > 0.0f )
		{
			// Start this DamageFX if necessary

			if( bLocal )
			{
				// First person DamageFX for the local object...

				pDamageFX->Start();
			}
			else
			{
				// Play the 3rd person FX if this is not the local clients characterfx...

				if( pDamageFX->m_sz3rdPersonFXName[0] && 
					(pDamageFX->m_nID < m_nNum3rdPersonDamageFX) &&
					!m_p3rdPersonDamageFX[pDamageFX->m_nID].IsValid() )
				{
					CLIENTFX_CREATESTRUCT	fxInit( pDamageFX->m_sz3rdPersonFXName, FXFLAG_LOOP, m_hServerObject ); 
					g_pClientFXMgr->CreateClientFX(&m_p3rdPersonDamageFX[pDamageFX->m_nID], fxInit, LTTRUE );
				}
			}
		}
		else
		{
			// Stop any current DamageFX
			
			if( !bPlayingAni )
			{
				if( bLocal )
				{
					pDamageFX->Stop();
				}
				else
				{
					if( (pDamageFX->m_nID < m_nNum3rdPersonDamageFX) && m_p3rdPersonDamageFX[pDamageFX->m_nID].IsValid() )
					{
						g_pClientFXMgr->ShutdownClientFX( &m_p3rdPersonDamageFX[pDamageFX->m_nID] );
					}
				}
			}
		}	

		// Check if we are taking any instant damage...

		if( m_nInstantDamageFlags & pDamageFX->m_nDamageFlag )
		{
			if( pDamageFX->m_sz3rdPersonInstFXName[0] && !m_link3rdPersonInstFX.IsValid() )
			{
				CLIENTFX_CREATESTRUCT fxInit( pDamageFX->m_sz3rdPersonInstFXName, 0, m_hServerObject );
				g_pClientFXMgr->CreateClientFX( &m_link3rdPersonInstFX, fxInit, LTTRUE );
			}
		}

		pDamageFX = g_pDamageFXMgr->GetNextDamageFX();
	}

	// Clear the instant damage...
	
	m_nInstantDamageFlags = 0;
	m_bWasPlayingSpecialDamageAni = bPlayingAni;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::HandleDialogueMsg
//
//	PURPOSE:	Start/Stop a dialogue sound
//
// ----------------------------------------------------------------------- //

void CCharacterFX::HandleDialogueMsg(ILTMessage_Read *pMsg)
{
    HSTRING hSound = pMsg->ReadHString();
    LTFLOAT fRadius = pMsg->Readfloat();
	m_nUniqueDialogueId = pMsg->Readuint8( );
	CharacterSoundType cst = (CharacterSoundType)pMsg->Readuint8( );
	m_bSubtitlePriority = (cst == CST_DIALOG);
	bool bCensor = (cst == CST_DEATH) || (cst == CST_DAMAGE);

	if (bCensor && g_pVersionMgr->IsLowViolence())
	{
		//don't play the sound, but if it is a pain sound, do notify the server that we're done with it
		// if it's a death sound, don't bother because it's not waiting for notification
		if (cst == CST_DAMAGE)
			KillLipSyncSound( true );
		return;
	}

	char szSound[128];
	*szSound = 0;

	if (hSound)
	{
		strcpy(szSound, g_pLTClient->GetStringData(hSound));
	    g_pLTClient->FreeString(hSound);
	}

	if (m_hDialogueSnd)
	{
		KillLipSyncSound( false );
	}

	if (*szSound && fRadius > 0.0f)
	{
		m_bSubtitle = LTFALSE;

		//don't bother getting the handle if it's a death sound, because we don't track them
		bool bGetHandle = (cst != CST_DEATH);
		m_hDialogueSnd = PlayLipSyncSound(szSound, fRadius, m_bSubtitle, m_bSubtitlePriority, bGetHandle);

		//if we didn't get a handle (and we did ask for one) then tell the server we're done with the sound
		if( !m_hDialogueSnd && bGetHandle)
		{
			KillLipSyncSound( true );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::HandleTauntMsg
//
//	PURPOSE:	Start a taunt sound
//
// ----------------------------------------------------------------------- //

void CCharacterFX::HandleTauntMsg(ILTMessage_Read *pMsg)
{
    uint32 nTauntID = pMsg->Readuint32();
	uint32 nTeam = pMsg->Readuint8();

	if (IsTeamGameType() && nTeam != INVALID_TEAM)
	{
		CLIENT_INFO *pLocalCI = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
		if( !pLocalCI )
			return;

		if( pLocalCI->nTeamID != nTeam )
			return;
	}
	PlayTaunt(nTauntID, LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateSounds
//
//	PURPOSE:	Update our sounds
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateSounds()
{
	if (m_hDialogueSnd)
	{
		if (g_pLTClient->IsDone(m_hDialogueSnd))
		{
			KillLipSyncSound( true );
		}
	}

	// See if we should play a ding sound...

	if (IsMultiplayerGame())
	{
		LTFLOAT fTime = g_pLTClient->GetTime();
		for (int i=0; i < MAX_DINGS; i++)
		{
			if (m_fNextDingTime[i] >= fTime)
			{
//				g_pLTClient->CPrint("Playing Ding Sound at Time %.2f", m_fNextDingTime[i]);
				m_fNextDingTime[i] = -1.0f;
				char* pSound = "Guns\\snd\\Impacts\\MultiDing.wav";
				g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateUnderwaterFX
//
//	PURPOSE:	Update the underwater fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateUnderwaterFX(LTVector & vPos)
{
	if (!m_pClientDE || !m_hServerObject) return;

	HOBJECT hCamera = g_pPlayerMgr->GetCamera();
    LTRotation rRot;

    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
    LTBOOL bIsLocalClient = (m_cs.bIsPlayer && hPlayerObj == m_hServerObject);

	if ( !bIsLocalClient || !(g_pPlayerMgr->IsFirstPerson() && hCamera) )
	{
        LTVector vDims;

		g_pPhysicsLT->GetObjectDims(m_hServerObject, &vDims);
		g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);

		vPos += rRot.Forward() * 20.0f;
		vPos.y += vDims.y * 0.4f;
	}
	else
	{
		g_pLTClient->GetObjectPos(hCamera, &vPos);
		g_pLTClient->GetObjectRotation(hCamera, &rRot);
		vPos += rRot.Forward() * 20.0f;
		vPos.y -= 20.0f;

		vPos += rRot.Up() * -20.0f;
	}

	if (!m_pBubbles)
	{
		CreateUnderwaterFX(vPos);
	}

	if (m_pBubbles)
	{
		g_pLTClient->SetObjectPos(m_pBubbles->GetObject(), &vPos);

		m_pBubbles->Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateUnderwaterFX
//
//	PURPOSE:	Create underwater special fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateUnderwaterFX(const LTVector & vPos)
{
	if (!m_pClientDE || !g_pGameClientShell || !m_hServerObject) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

    LTFLOAT fTime = m_pClientDE->GetTime();

	if (m_fNextBubbleTime > 0.0f && fTime < m_fNextBubbleTime)
	{
		return;
	}

	m_fNextBubbleTime = fTime + GetRandom(2.0f, 5.0f);

	SMCREATESTRUCT sm;

	sm.vPos = vPos;
	sm.vPos.y += 25.0f;

	sm.vColor1.Init(100.0f, 100.0f, 100.0f);
	sm.vColor2.Init(150.0f, 150.0f, 150.0f);
	sm.vMinDriftVel.Init(-7.5f, 20.0f, -7.5f);
	sm.vMaxDriftVel.Init(7.5f, 40.0f, 7.5f);

	GetLiquidColorRange(CC_WATER, &sm.vColor1, &sm.vColor2);

	sm.fVolumeRadius		= 10.0f;
	sm.fLifeTime			= 0.2f;
	sm.fRadius				= 1000;
	sm.fParticleCreateDelta	= 0.1f;
	sm.fMinParticleLife		= 1.0f;
	sm.fMaxParticleLife		= 3.0f;
	sm.nNumParticles		= GetRandom(2, 5);
    sm.bIgnoreWind          = LTTRUE;
	sm.hstrTexture			= m_pClientDE->CreateString(DEFAULT_BUBBLE_TEXTURE);

	//m_pBubbles =
		psfxMgr->CreateSFX(SFX_SMOKE_ID, &sm);

	m_pClientDE->FreeString(sm.hstrTexture);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::RemoveUnderwaterFX
//
//	PURPOSE:	Remove the underwater fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::RemoveUnderwaterFX()
{
	m_fNextBubbleTime = -1.0f;

	if (m_pBubbles)
	{
		debug_delete(m_pBubbles);
        m_pBubbles = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateZzzFX
//
//	PURPOSE:	Update the Zzz fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateZzzFX()
{
	if (!m_pClientDE || !m_hServerObject) return;

    LTVector vPos(0,0,0);

	HMODELSOCKET hSocket;
	if ( LT_OK == g_pModelLT->GetSocket(m_hServerObject, "head", hSocket) )
	{
		LTransform transform;
        if ( LT_OK == g_pModelLT->GetSocketTransform(m_hServerObject, hSocket, transform, LTTRUE) )
		{
			vPos = transform.m_Pos;
		}
	}

	if (!m_pZzz)
	{
		CreateZzzFX();
	}

	if (m_pZzz)
	{
		g_pLTClient->SetObjectPos(m_pZzz->GetObject(), &vPos);

		m_pZzz->Update();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateZzzFX
//
//	PURPOSE:	Create Zzz special fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateZzzFX()
{
	if (!m_pClientDE || !g_pGameClientShell || !m_hServerObject) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

    LTFLOAT fTime = m_pClientDE->GetTime();

	if (m_fNextZzzTime > 0.0f && fTime < m_fNextZzzTime)
	{
		return;
	}

	m_fNextZzzTime = fTime + GetRandom(0.75f, 1.0f);

	SMCREATESTRUCT sm;

    LTVector vPos(0,0,0);

	HMODELSOCKET hSocket;
	if ( LT_OK == g_pModelLT->GetSocket(m_hServerObject, "head", hSocket) )
	{
		LTransform transform;
        if ( LT_OK == g_pModelLT->GetSocketTransform(m_hServerObject, hSocket, transform, LTTRUE) )
		{
			vPos = transform.m_Pos;
		}
	}

    sm.vPos = vPos;
	sm.vPos.y += 15.0f;

	sm.vColor1.Init(100.0f, 100.0f, 100.0f);
	sm.vColor2.Init(150.0f, 150.0f, 150.0f);
	sm.vMinDriftVel.Init(-1, 5, -1);
	sm.vMaxDriftVel.Init(1, 10, 1);

	GetLiquidColorRange(CC_WATER, &sm.vColor1, &sm.vColor2);

	sm.fVolumeRadius		= 10.0f;
	sm.fLifeTime			= 0.2f;
	sm.fRadius				= 1000;
	sm.fParticleCreateDelta	= 0.1f;
	sm.fMinParticleLife		= 1.0f;
	sm.fMaxParticleLife		= 3.0f;
	sm.nNumParticles		= GetRandom(1, 1);
    sm.bIgnoreWind          = LTFALSE;
	sm.hstrTexture			= m_pClientDE->CreateString(DEFAULT_ZZZ_TEXTURE);

	psfxMgr->CreateSFX(SFX_SMOKE_ID, &sm);

	m_pClientDE->FreeString(sm.hstrTexture);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::RemoveZzzFX
//
//	PURPOSE:	Remove the Zzz fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::RemoveZzzFX()
{
	m_fNextZzzTime = -1.0f;

	if (m_pZzz)
	{
		debug_delete(m_pZzz);
        m_pZzz = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateCigaretteFX
//
//	PURPOSE:	Update the Cigarette fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateCigaretteFX()
{
	if (!m_pClientDE || !m_hServerObject) return;

    LTVector vPos(0,0,0);

	HMODELSOCKET hSocket;
	if ( LT_OK == g_pModelLT->GetSocket(m_hServerObject, "LeftHand", hSocket) )
	{
		LTransform transform;
        if ( LT_OK == g_pModelLT->GetSocketTransform(m_hServerObject, hSocket, transform, LTTRUE) )
		{
			vPos = transform.m_Pos;
		}
	}

	if (!m_pCigarette)
	{
		CreateCigaretteFX();
	}

	if (m_pCigarette)
	{
		g_pLTClient->SetObjectPos(m_pCigarette->GetObject(), &vPos);
		m_pCigarette->Update();
	}

	HOBJECT hObj = m_CigaretteModel.GetObject();
	if (!hObj)
	{
		// Create fx...

		m_scalecs.dwFlags = FLAG_VISIBLE | FLAG_NOLIGHT;

		// Set up fx flags...

		m_scalecs.pFilename			= "Props\\Models\\CigButt_01.ltb";

		CButeListReader blrSkinReader;
		blrSkinReader.SetItem(0, "Props\\Skins\\CigButt.dtx", MAX_CS_FILENAME_LEN+1);
		m_scalecs.pSkinReader = &blrSkinReader;

		m_scalecs.vPos				= vPos;
		m_scalecs.vVel.Init();
		m_scalecs.vInitialScale.Init(1, 1, 1);
		m_scalecs.vFinalScale.Init(1, 1, 1);
		m_scalecs.vInitialColor.Init(1, 1, 1);
		m_scalecs.vFinalColor.Init(1, 1, 1);
        m_scalecs.bUseUserColors    = LTFALSE;
		m_scalecs.fLifeTime			= 1.0f;
		m_scalecs.fInitialAlpha		= 1.0f;
		m_scalecs.fFinalAlpha		= 1.0f;
        m_scalecs.bLoop             = LTFALSE;
		m_scalecs.fDelayTime		= 0.0f;
        m_scalecs.bAdditive         = LTFALSE;
		m_scalecs.nType				= OT_MODEL;

		m_CigaretteModel.Init(&m_scalecs);
        m_CigaretteModel.CreateObject(g_pLTClient);
	}
	else
	{
		// Update fx...

		g_pCommonLT->SetObjectFlags(hObj, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
		g_pLTClient->SetObjectPos(hObj, &vPos);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateCigaretteFX
//
//	PURPOSE:	Create Cigarette special fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateCigaretteFX()
{
	if ( !(m_cs.byFXFlags & CHARCREATESTRUCT::eCigaretteSmoke) ) return;
	if (!m_pClientDE || !g_pGameClientShell || !m_hServerObject) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

    LTFLOAT fTime = m_pClientDE->GetTime();

	if (m_fNextCigaretteTime > 0.0f && fTime < m_fNextCigaretteTime)
	{
		return;
	}

	m_fNextCigaretteTime = fTime + GetRandom(0.1f, 1.0f);

	SMCREATESTRUCT sm;

    LTVector vPos(0,0,0);

	HMODELSOCKET hSocket;
	if ( LT_OK == g_pModelLT->GetSocket(m_hServerObject, "LeftHand", hSocket) )
	{
		LTransform transform;
        if ( LT_OK == g_pModelLT->GetSocketTransform(m_hServerObject, hSocket, transform, LTTRUE) )
		{
			g_pTransLT->GetPos(transform, vPos);
		}
	}

    sm.vPos = vPos;

	sm.vColor1.Init(100.0f, 100.0f, 100.0f);
	sm.vColor2.Init(150.0f, 150.0f, 150.0f);
	sm.vMinDriftVel.Init(-1, 5, -1);
	sm.vMaxDriftVel.Init(1, 10, 1);

	GetLiquidColorRange(CC_WATER, &sm.vColor1, &sm.vColor2);

	sm.fVolumeRadius		= 0.0f;
	sm.fLifeTime			= 0.2f;
	sm.fRadius				= 200;
	sm.fParticleCreateDelta	= 0.1f;
	sm.fMinParticleLife		= 1.0f;
	sm.fMaxParticleLife		= 3.0f;
	sm.nNumParticles		= GetRandom(1, 2);
    sm.bIgnoreWind          = LTFALSE;
	sm.hstrTexture			= m_pClientDE->CreateString(DEFAULT_CIGARETTE_TEXTURE);

	psfxMgr->CreateSFX(SFX_SMOKE_ID, &sm);

	m_pClientDE->FreeString(sm.hstrTexture);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::RemoveCigaretteFX
//
//	PURPOSE:	Remove the Cigarette fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::RemoveCigaretteFX()
{
	m_fNextCigaretteTime = -1.0f;

	if (m_pCigarette)
	{
		debug_delete(m_pCigarette);
        m_pCigarette = LTNULL;
	}

	HOBJECT hObj = m_CigaretteModel.GetObject();
	if (hObj)
	{
		g_pCommonLT->SetObjectFlags(hObj, OFT_Flags, 0, FLAG_VISIBLE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateFlashLightFX
//
//	PURPOSE:	Update the flash light fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateFlashLightFX()
{
	// See if this is the local player...

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (m_cs.bIsPlayer && hPlayerObj == m_hServerObject)
	{
		// If we're the local player don't show the 3rd person fx

		m_3rdPersonFlashlight.TurnOff();
		return;
	}
	
	m_3rdPersonFlashlight.TurnOn();
	m_3rdPersonFlashlight.Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::RemoveFlashLightFX
//
//	PURPOSE:	Remove the flash light fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::RemoveFlashLightFX()
{
	m_3rdPersonFlashlight.TurnOff();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::OnModelKey
//
//	PURPOSE:	Handle model key
//
// ----------------------------------------------------------------------- //

void CCharacterFX::OnModelKey(HLOCALOBJ hObj, ArgList *pArgs)
{
	if (!hObj || !pArgs || !pArgs->argv || pArgs->argc == 0) return;

	char* pKey = pArgs->argv[0];
	if (!pKey) return;

	if (g_vtModelKey.GetFloat() > 0.0f)
	{
        g_pLTClient->CPrint("%s ModelKey: '%s'", (m_cs.bIsPlayer ? "Player" : "AI"), pKey);
	}

	// See if the damagefx take care of it...

	if( g_pDamageFXMgr->OnModelKey( hObj, pArgs ) )
		return;

	if (stricmp(pKey, KEY_FOOTSTEP_SOUND) == 0)
	{
		
		ModelId eModelId = GetModelId();

		if (pArgs->argc > 1)
		{
			// See if this is the left (2) or right (1) foot...

			if (stricmp(pArgs->argv[1], "1") == 0)
			{
                m_bLeftFoot = LTFALSE;
			}
			else
			{
                m_bLeftFoot = LTTRUE;
			}
		}
		else
		{
			// Alternate feet...
			m_bLeftFoot = !m_bLeftFoot;
		}

		DoFootStepKey(hObj);
	}
	else if( stricmp( pKey, RENDERSTYLE_MODEL_KEY ) == 0 )
	{
		// start the index at the first argument
		int i = 0;
		int nRS;

		// while there are arguments
		while((i < pArgs->argc) &&
				('\0' != pArgs->argv[i][0]))
		{
			// Check for renderstyle model key
			if(stricmp(pArgs->argv[i],RENDERSTYLE_MODEL_KEY) == 0)
			{
				// Check params
				if(pArgs->argc >= i+3)
				{
					nRS = (atoi)(pArgs->argv[i+1]);
					SetObjectRenderStyle(m_hServerObject,nRS,pArgs->argv[i+2]);
				}
				else
				{
					// Not enough params
					g_pLTClient->CPrint("CCharacterFX::OnModelKey - ERROR - Not enough RS arguments! Syntax: RS <RSNum> <RSName>\n");
				}

				// Move past all arguments of this key
				i += 3;
			}
			else
			{
				// Go to the next string
				i++;
			}
		}
	}
	else if( stricmp( pKey, KEY_MOVEMENT_LOUD ) == 0 )
	{
		const char* pSnd = g_pModelButeMgr->GetModelLoudMovementSnd( m_cs.eModelId );
		if( pSnd )
		{
			g_pClientSoundMgr->PlaySoundFromObject( m_hServerObject, pSnd );
		}
	}
	else if( stricmp( pKey, KEY_MOVEMENT_QUIET ) == 0 )
	{
		const char* pSnd = g_pModelButeMgr->GetModelQuietMovementSnd( m_cs.eModelId );
		if( pSnd )
		{
			g_pClientSoundMgr->PlaySoundFromObject( m_hServerObject, pSnd );
		}
	}
	else if( stricmp( pKey, KEY_SHOW_ATTACHFX ) == 0 )
	{
		if( pArgs->argc > 1 )
		{
			uint32 i = atoi( pArgs->argv[1] );

			//note that the +1 is to compensate for our dummy head
			CLIENTFX_LINK* pLink = m_AttachClientFX.GetElement(i + 1);

			if (pLink && pLink->IsValid())
			{
				pLink->GetInstance()->Show();
			}
		}
		else
		{
			ShowAttachClientFX();
		}
	}
	else if( stricmp( pKey, KEY_HIDE_ATTACHFX ) == 0)
	{
		if( pArgs->argc > 1 )
		{
			uint32 i = atoi( pArgs->argv[1] );

			//note that the +1 is to compensate for our dummy head
			CLIENTFX_LINK* pLink = m_AttachClientFX.GetElement(i + 1);

			if (pLink && pLink->IsValid())
			{
				pLink->GetInstance()->Hide();
			}
		}
		else
		{
			HideAttachClientFX();
		}
	}
	else if( stricmp( pKey, KEY_FX ) == 0)
	{
		if( pArgs->argc > 1 && pArgs->argv[1] )
		{
			CLIENTFX_CREATESTRUCT fxInit( pArgs->argv[1], 0, m_hServerObject );
			g_pClientFXMgr->CreateClientFX( LTNULL, fxInit, LTTRUE );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::DoFootStepKey
//
//	PURPOSE:	Handle model foot step key
//
// ----------------------------------------------------------------------- //

void CCharacterFX::DoFootStepKey(HLOCALOBJ hObj, LTBOOL bForceSound)
{
	if (!hObj) return;

    // Play one-off sounds on vehicles...

	PlayerPhysicsModel eModel = PPM_NORMAL;
    if (m_cs.bIsPlayer)
    {
        uint32 dwFlags;
	    g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_User, dwFlags);

        if (dwFlags & USRFLG_PLAYER_SNOWMOBILE)
		{
			eModel = PPM_SNOWMOBILE;
		}
    }


	CMoveMgr* pMoveMgr = g_pPlayerMgr->GetMoveMgr();
	if (!pMoveMgr) return;

    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
    LTBOOL bIsLocalClient = (m_cs.bIsPlayer && hPlayerObj == hObj);

	m_eLastSurface = ST_UNKNOWN;

	if (bIsLocalClient)
	{
		if (g_pPlayerMgr->IsSpectatorMode() || !pMoveMgr->CanDoFootstep())
		{
			return;
		}

		// Use our current standing on surface if we still don't know what
		// we're standing on...

		m_eLastSurface = pMoveMgr->GetStandingOnSurface();
	}


    LTVector vPos;
	g_pLTClient->GetObjectPos(hObj, &vPos);

	ClientIntersectQuery iQuery;
	ClientIntersectInfo  iInfo;


	// Do an intersect segment to determine where to put the footprint
	// sprites...

	iQuery.m_Flags = IGNORE_NONSOLID | INTERSECT_OBJECTS | INTERSECT_HPOLY;
	iQuery.m_From  = vPos;

	// If the object has Left/RightFoot sockets, use them to determine
	// the location for the InteresectSegment...

    ILTModel* pModelLT   = g_pLTClient->GetModelLT();

    char* pSocketName = (char *)(m_bLeftFoot ? "LeftFoot" : "RightFoot");
	HMODELSOCKET hSocket;

	if (pModelLT->GetSocket(hObj, pSocketName, hSocket) == LT_OK)
	{
		LTransform transform;
        if (pModelLT->GetSocketTransform(hObj, hSocket, transform, LTTRUE) == LT_OK)
		{
            ILTTransform* pTransLT = g_pLTClient->GetTransformLT();
			pTransLT->GetPos(transform, iQuery.m_From);

			// Testing...
			iQuery.m_From.y += INTERSECT_Y_OFFSET;
		}
	}

	iQuery.m_To	= iQuery.m_From;
	iQuery.m_To.y -= (INTERSECT_Y_OFFSET * 2.0f);

	// Don't hit ourself...

    HOBJECT hFilterList[] = {g_pLTClient->GetClientObject(), g_pPlayerMgr->GetMoveMgr()->GetObject(), LTNULL};

	if (bIsLocalClient)
	{
		iQuery.m_FilterFn  = WorldOnlyFilterFn;
		iQuery.m_pUserData = hFilterList;
	}

	if (m_pClientDE->IntersectSegment(&iQuery, &iInfo))
	{
		if (IsMainWorld(iInfo.m_hObject) ||
			GetObjectType(iInfo.m_hObject) == OT_WORLDMODEL)
		{
			if (m_eLastSurface == ST_UNKNOWN)
			{
				m_eLastSurface = GetSurfaceType(iInfo);
			}
		}
	}


	// Play a footstep sound if this isn't the local client (or we're in
	// 3rd person).  The local client's footsteps are tied to the head bob
	// in 1st person...

    LTBOOL bPlaySound = LTTRUE;
	
	if (!bForceSound && bIsLocalClient)
	{
		bPlaySound = (!g_pPlayerMgr->IsFirstPerson() || m_bOnVehicle);
	}

	if (bPlaySound)
	{
		PlayMovementSound(vPos, m_eLastSurface, m_bLeftFoot, eModel);
	}


	// Leave footprints on the appropriate surfaces...

	if (ShowsTracks(m_eLastSurface))
	{
		// Use intersect position for footprint sprite...

		CreateFootprint(m_eLastSurface, iInfo);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::PlayMovementSound
//
//	PURPOSE:	Play movement sound
//
// ----------------------------------------------------------------------- //

void CCharacterFX::PlayMovementSound(LTVector vPos, SurfaceType eSurface,
                                     LTBOOL bLeftFoot, PlayerPhysicsModel eModel)
{
    char* pSound = LTNULL;

	CMoveMgr* pMoveMgr = g_pPlayerMgr->GetMoveMgr();
	if (!pMoveMgr) return;

	// Don't do movement sounds if in the menu...

	if ( !g_pInterfaceMgr->IsInGame( )) return;

    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
    LTBOOL bIsLocalClient = (m_cs.bIsPlayer && hPlayerObj == m_hServerObject);

	// Dead men don't make movement sounds...

	if (bIsLocalClient && g_pPlayerMgr->IsPlayerDead()) return;

	// Dead men don't make movement sounds in multiplayer either...

	if (m_cs.bIsPlayer && IsMultiplayerGame() && m_bPlayerDead) return;


	// If we're on a ladder, make sure it plays sounds and see if there is a surface override...

	if (eSurface == ST_LADDER)
	{
		// Find the ladder we're in...

		CVolumeBrushFX *pVolumeFX = LTNULL;

		// Use the Ladder container from the server if we are the local client...

		if( bIsLocalClient )
		{
			if( pMoveMgr->IsBodyOnLadder() && pMoveMgr->GetLadderObject() )
			{
				pVolumeFX = (CVolumeBrushFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_VOLUMEBRUSH_ID, pMoveMgr->GetLadderObject() );
			}
		}
		
		// If we aren't the local client or failed to get the ladder object use the server objects position...

		if( !pVolumeFX )
		{
			LTVector vMyPos;

			LTVector vDims;
			g_pPhysicsLT->GetObjectDims( m_hServerObject, &vDims );

			HLOCALOBJ	objList[16];
			uint32		nFound;
			uint32		dwNum;

			g_pLTClient->FindObjectsInBox( &vPos, vDims.Mag(), objList, ARRAY_LEN( objList ), &dwNum, &nFound );
			
			for (uint32 i=0; i < dwNum; i++)
			{
				uint16 code;
				if (g_pLTClient->GetContainerCode(objList[i], &code))
				{
					if (CC_LADDER == (ContainerCode)code)
					{
						pVolumeFX = (CVolumeBrushFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_VOLUMEBRUSH_ID, objList[i]);
						
						break;
					}
				}
			}
		}

		if( pVolumeFX )
		{
			if( !pVolumeFX->CanPlayMovementSounds() ) return;

			// If there is a valid override surface use it for the footstep sounds...

			if( pVolumeFX->GetSurfaceOverride() != ST_UNKNOWN )
			{
				eSurface = pVolumeFX->GetSurfaceOverride();
			}
		}

	}



	pSound = GetMovementSound(eSurface, bLeftFoot, eModel);

	if (pSound && *pSound)
	{
        uint32 dwFlags = bIsLocalClient ? PLAYSOUND_CLIENTLOCAL : 0;
		SoundPriority ePriority = m_cs.bIsPlayer ? SOUNDPRIORITY_PLAYER_HIGH : SOUNDPRIORITY_AI_HIGH;

		LTFLOAT fStealth = (eModel == PPM_NORMAL ? m_cs.fStealthPercent : 1.0f);
		int nVolume = (int) (100.0f * fStealth);

		// Adjust the volume if we are walking or ducking...

		if (bIsLocalClient && pMoveMgr->IsMovingQuietly())
		{
			nVolume = ( int )((( float )nVolume * g_vtQuietMovementVolumeFactor.GetFloat( )) + 0.5f );
		}

		g_pClientSoundMgr->PlaySoundFromPos(vPos, pSound, FOOTSTEP_SOUND_RADIUS, ePriority, dwFlags, nVolume);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateFootprint
//
//	PURPOSE:	Create a footprint sprite at the specified location
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateFootprint(SurfaceType eType, IntersectInfo & iInfo)
{
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eType);
	if (!pSurf || !pSurf->szLtFootPrintSpr[0] || !pSurf->szRtFootPrintSpr[0]) return;

	// Don't do footprints if we are on a vehicle...

    if (m_cs.bIsPlayer && m_bOnVehicle)
	{
		return;
	}


	// Dead men don't make footprints...

	if (m_cs.bIsPlayer && IsMultiplayerGame() && m_bPlayerDead) return;


    LTVector vDir = iInfo.m_Plane.m_Normal;

    LTRotation rRot;
	g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);

	// Create a sprite...

	BSCREATESTRUCT scale;
	scale.dwFlags = FLAG_VISIBLE | FLAG_ROTATEABLESPRITE | FLAG_NOLIGHT;
	scale.rRot = LTRotation(vDir, rRot.Forward());

	// Nope, this isn't a typo, artists just have a problem telling
	// left from right...
	scale.pFilename			= m_bLeftFoot ? pSurf->szRtFootPrintSpr : pSurf->szLtFootPrintSpr;

	scale.vPos				= iInfo.m_Point + vDir;
	scale.vInitialScale		= pSurf->vFootPrintScale;
	scale.vFinalScale		= pSurf->vFootPrintScale;
	scale.fLifeTime			= pSurf->fFootPrintLifetime;
	scale.fInitialAlpha		= 1.0f;
	scale.fFinalAlpha		= 0.0f;
	scale.vInitialColor.Init(0.5f, 0.5f, 0.5f);
	scale.vFinalColor.Init(0.5f, 0.5f, 0.5f);
    scale.bUseUserColors    = LTFALSE;
	scale.nType				= OT_SPRITE;
	scale.bPausable			= LTTRUE;

	// Special hack so we get cute little bunny footprints...
	const char* pModelName = g_pModelButeMgr->GetModelName( GetModelId() );
	if (pModelName && pModelName[0] && stricmp(pModelName, "Rabbit") == 0)
	{
		scale.pFilename = m_bLeftFoot ? "FX\\Test\\Snow\\Spr\\SnowRabbitFS_L.spr" : "FX\\Test\\Snow\\Spr\\SnowRabbitFS_R.spr";
	}

	if (g_vtFootPrintBlend.GetFloat() == 1.0f)
	{
		scale.bAdditive	= LTTRUE;
		scale.vInitialColor.Init(0.5f, 0.5f, 0.5f);
		scale.vFinalColor.Init(0.0f, 0.0f, 0.0f);
	}
	else if (g_vtFootPrintBlend.GetFloat() == 2.0f)
	{
		scale.bMultiply = LTTRUE;
		scale.vInitialColor.Init(0.5f, 0.5f, 0.5f);
		scale.vFinalColor.Init(0.0f, 0.0f, 0.0f);
	}

    CSpecialFX* pFX = LTNULL;
	pFX = g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_SCALE_ID, &scale);

	if (pFX)
	{
		pFX->Update();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateVehicleTrail
//
//	PURPOSE:	Create a vehicle trail segment at the specified location
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateVehicleTrail(CPolyLineFX* pTrail, LTVector vDir,
                                      LTVector vStartPoint, LTVector vEndPoint, LTBOOL bNewTrail)
{
    if (!pTrail) return;

	if (!pTrail->HasBeenDrawn())
	{
		PLFXCREATESTRUCT pls;
		pls.pTexture			= "sfx\\test\\fxtest44.dtx";
		pls.vStartPos			= vStartPoint + (vDir * 2.0f);
		pls.vEndPos				= vEndPoint + (vDir * 2.0f);
        pls.vInnerColorStart    = LTVector(255, 255, 255);
        pls.vInnerColorEnd      = LTVector(255, 255, 255);
        pls.vOuterColorStart    = LTVector(255, 255, 255);
        pls.vOuterColorEnd      = LTVector(255, 255, 255);
		pls.fAlphaStart			= 0.9f;
		pls.fAlphaEnd			= 0.0f;
		pls.fMinWidth			= 0.0f;
		pls.fMaxWidth			= 128.0f;
		pls.fLifeTime			= g_vtTrailSegmentLifetime.GetFloat();
		pls.fAlphaLifeTime		= g_vtTrailSegmentLifetime.GetFloat();
        pls.bAdditive           = LTFALSE;
        pls.bMultiply           = LTTRUE;
		pls.nWidthStyle			= PLWS_CONSTANT;
        pls.bUseObjectRotation  = LTFALSE;
		pls.nNumSegments		= 1;
		pls.fMinDistMult		= 1.0f;
		pls.fMaxDistMult		= 1.0f;
		pls.fPerturb			= 0.0f;
        pls.bAlignFlat          = LTTRUE;
        pls.bAlignUsingRot      = LTTRUE;
        pls.bNoZ                = LTTRUE;

		pTrail->Init(&pls);
		pTrail->CreateObject(m_pClientDE);

		LTRotation rRot;
		g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);

        LTRotation rTrailRot;
		rTrailRot = LTRotation(rRot.Forward(), vDir);
		pTrail->SetRot(rTrailRot);
	}
	else
	{
		PLFXLINESTRUCT ls;
		ls.vStartPos = vStartPoint + (vDir * 2.0f);

        if (!bNewTrail)
        {
		    // Get the last vert position...

		    PolyLineList* pLines = pTrail->GetLines();
		    if (pLines->GetLength() > 0)
		    {
			    PolyLine** pLine = pLines->GetItem(TLIT_LAST);
			    if (pLine && *pLine)
			    {
				    PolyVertStruct** pVert = (*pLine)->list.GetItem(TLIT_LAST);
				    if (pVert && *pVert)
				    {
					    ls.vStartPos = pTrail->GetVertPos((*pVert));
				    }
			    }
		    }
        }

		ls.vEndPos = vEndPoint + (vDir * 2.0f);

		LTVector vDist = ls.vStartPos - ls.vEndPos;

		// Only create a segment if we've moved far enough...
		if (vDist.Mag() >= g_vtMinTrailSegment.GetFloat())
		{
			ls.vInnerColorStart     = LTVector(255, 255, 255);
			ls.vInnerColorEnd       = LTVector(255, 255, 255);
			ls.vOuterColorStart     = LTVector(255, 255, 255);
			ls.vOuterColorEnd       = LTVector(255, 255, 255);
			ls.fAlphaStart			= 0.9f;
			ls.fAlphaEnd			= 0.0f;
			ls.fLifeTime			= g_vtTrailSegmentLifetime.GetFloat();
			ls.fAlphaLifeTime		= g_vtTrailSegmentLifetime.GetFloat();

			pTrail->AddLine(ls);
		}
    }
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateOnVehicle
//
//	PURPOSE:	Update being on a vehicle if appropriate
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateOnVehicle()
{
    // Only players can be on vehicles...

    if (!m_cs.bIsPlayer) return;

    uint32 dwFlags;
    g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_User, dwFlags);

    m_bOnVehicle = (dwFlags & USRFLG_PLAYER_SNOWMOBILE);

	// Play multiplayer sounds if appropriate...

	if (IsMultiplayerGame())
	{
		// Don't play sounds for local client...they get their own

        if (g_pLTClient->GetClientObject() != m_hServerObject)
		{
			UpdateMultiVehicleSounds();
		}
	}
	else if (m_hVehicleSound)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hVehicleSound);
		m_hVehicleSound = LTNULL;
	}

	// See if we should continue the trail...

    if (m_bOnVehicle && g_vtVehicleTrials.GetFloat())
    {
        // Cast a ray down to see if we are on a surface that leaves
        // a trail...

        LTVector vPos, vDims;
		g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
	    g_pPhysicsLT->GetObjectDims(m_hServerObject, &vDims);

	    ClientIntersectQuery iQuery;
	    ClientIntersectInfo  iInfo;

	    iQuery.m_Flags = IGNORE_NONSOLID | INTERSECT_OBJECTS | INTERSECT_HPOLY;
	    iQuery.m_From  = vPos;

    	iQuery.m_To	= iQuery.m_From;
	    iQuery.m_To.y -= (vDims.y + 50.0f);

	    // Don't hit ourself...

        HOBJECT hFilterList[] = {g_pLTClient->GetClientObject(), g_pPlayerMgr->GetMoveMgr()->GetObject(), LTNULL};

        LTBOOL bIsLocalClient = (m_cs.bIsPlayer && g_pLTClient->GetClientObject() == m_hServerObject);

        if (bIsLocalClient)
	    {
		    iQuery.m_FilterFn  = ObjListFilterFn;
		    iQuery.m_pUserData = hFilterList;
	    }

        // We're starting a new trail if the last surface didn't show
        // footprints (and this one does)...

        LTBOOL bNewTrail = !ShowsTracks(m_eLastSurface);

        m_eLastSurface = ST_UNKNOWN;

        if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
	    {
		    m_eLastSurface = GetSurfaceType(iInfo);
	    }

        // Only create trails on surfaces that show footprints...

    	if (ShowsTracks(m_eLastSurface))
        {
            LTRotation rRot;
			g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);

            LTVector vF = rRot.Forward();

            LTVector vStartPoint = iInfo.m_Point;
            LTVector vEndPoint   = iInfo.m_Point + (vF * 25.0f);

			// Create trail for the front tire...

			//CreateVehicleTrail(&m_VehicleTrail1, iInfo.m_Plane.m_Normal,
            //    vStartPoint, vEndPoint, bNewTrail);

			// Create the trail for the back tire...

 			vStartPoint	= iInfo.m_Point - (vF * 50.0f);
			vEndPoint	= iInfo.m_Point - (vF * 25.0f);

			CreateVehicleTrail(&m_VehicleTrail2, iInfo.m_Plane.m_Normal,
				vStartPoint, vEndPoint, bNewTrail);
        }
    }

    // Always update the trail, so it can fade away....

	if (m_VehicleTrail1.HasBeenDrawn())
	{
		m_VehicleTrail1.Update();
	}

    if (m_VehicleTrail2.HasBeenDrawn())
	{
		m_VehicleTrail2.Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateMultiVehicleSounds
//
//	PURPOSE:	Update multiplayer vehicle sounds
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateMultiVehicleSounds()
{
	if (m_bOnVehicle)
	{
		// See if we just got on the vehicle...

		if (!m_hVehicleSound)
		{
			// Play startup sound...

            const char* pSnd = "snd\\vehicle\\snowmobile\\startup.wav";
            g_pClientSoundMgr->PlaySoundFromObject(m_hServerObject, (char *)pSnd,
				DEFAULT_VEHICLE_RADIUS, SOUNDPRIORITY_PLAYER_HIGH);


			// Play running sound...

			uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP;

			pSnd = "snd\\vehicle\\snowmobile\\MP_Loop.wav";
			m_hVehicleSound = g_pClientSoundMgr->PlaySoundFromObject(m_hServerObject,
                (char *)pSnd, DEFAULT_VEHICLE_RADIUS, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
		}
		else
		{
			// Make sure the sound is playing from the correct position

			LTVector vPos;
			g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
			((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->SetSoundPosition(m_hVehicleSound, &vPos);
		}
	}
	else if (m_hVehicleSound)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hVehicleSound);
		m_hVehicleSound = LTNULL;

		// Play turn-off sound

        const char* pSnd = "snd\\vehicle\\snowmobile\\turnoff.wav";
        g_pClientSoundMgr->PlaySoundFromObject(m_hServerObject, (char *)pSnd,
			DEFAULT_VEHICLE_RADIUS, SOUNDPRIORITY_PLAYER_HIGH);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateVehicleContour
//
//	PURPOSE:	Update the contouring to terain on the vehicle...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateVehicleContour( LTRotation &rCharacterRot )
{
	if( !m_bOnVehicle || !m_cs.bIsPlayer )
		return;

	// Get the normals under the 4 corners of the vehicle and average them toghether 
	// to get the normal we should use.  This helps smooth out the transition between 
	// two or more planes.

	LTVector	vNormal, vModelPYR;
	LTVector	vDims, vPos;

	LTRotation	rRot = rCharacterRot;

	LTVector	vPlayerF = rRot.Forward();
	LTVector	vPlayerR = rRot.Right();
	
	// Keep the yaw from the original rotation...

	float		fYaw = (float)atan2( vPlayerF.x, vPlayerF.z );
	
	HOBJECT		hFilter[] = { m_hServerObject, LTNULL };
	
	g_pLTClient->GetObjectPos( m_hServerObject, &vPos );
	g_pPhysicsLT->GetObjectDims( m_hServerObject, &vDims );

	LTVector	vForward = vPlayerF * (vDims.z + g_vtVehicleContourExtraDimsZ.GetFloat());
	LTVector	vRight = vPlayerR * (vDims.x + g_vtVehicleContourExtraDimsX.GetFloat());
	vNormal = GetContouringNormal( vPos, vDims, vForward, vRight, hFilter );

	// Calculate how much pitch and roll we should apply...

	float fPitchPercent, fRollPercent, fAmount;
	GetContouringInfo( vPlayerF, vNormal, fAmount, fPitchPercent, fRollPercent );

	float fClamp = g_vtVehicleContourMaxRotation.GetFloat();

	vModelPYR.x = fAmount * fPitchPercent;
	vModelPYR.z	= fAmount * fRollPercent;

	vModelPYR.x = Clamp( vModelPYR.x, MATH_DEGREES_TO_RADIANS( -fClamp ), MATH_DEGREES_TO_RADIANS( fClamp ));
	vModelPYR.y	= fYaw;
	vModelPYR.z	= Clamp( vModelPYR.z, MATH_DEGREES_TO_RADIANS( -fClamp ), MATH_DEGREES_TO_RADIANS( fClamp ));

	LTRotation rModelRot( vModelPYR.x, vModelPYR.y, vModelPYR.z );

	// Set the new modified contour rotation...
	
	rCharacterRot = rModelRot;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::OnServerMessage
//
//	PURPOSE:	Handle any messages from our server object...
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterFX::OnServerMessage(ILTMessage_Read *pMsg)
{
    if (!CSpecialFX::OnServerMessage(pMsg)) return LTFALSE;

    uint8 nMsgId = pMsg->Readuint8();

	switch(nMsgId)
	{
		case CFX_CROSSHAIR_MSG:
		{
			m_cs.eCrosshairCharacterClass = (CharacterClass)pMsg->Readuint8();
		}
		break;

		case CFX_CARRY:
		{
			m_cs.nCarrying = pMsg->Readuint8();
			UpdateCarrying();
		}
		break;

		//sent down when a new spear is attached so we can hide them as necessary
		case CFX_UPDATE_ATTACHMENTS:
		{
			m_bUpdateAttachments = true;
		}
		break;


		case CFX_NODECONTROL_LIP_SYNC:
		case CFX_NODECONTROL_HEAD_FOLLOW_OBJ:
		case CFX_NODECONTROL_SCRIPT:
		{
			m_NodeController.HandleNodeControlMessage(nMsgId, pMsg);
		}
		break;

		case CFX_DIALOGUE_MSG:
		{
			HandleDialogueMsg(pMsg);
		}
		break;

		case CFX_TAUNT_MSG:
		{
			HandleTauntMsg(pMsg);
		}
		break;

		case CFX_RESET_TRACKER:
		{
            uint8 iTracker = pMsg->Readuint8();
			if ( iTracker == 0 )
			{
				g_pLTClient->ResetModelAnimation(m_hServerObject);
			}
			else
			{
				g_pModelLT->ResetAnim(m_hServerObject, iTracker);
			}
		}
		break;

		case CFX_DMGFLAGS_MSG:
		{
			m_cs.nDamageFlags = pMsg->Readuint64();
		}
		break;

		case CFX_INSTANTDMGFLAGS_MSG:
		{
			m_nInstantDamageFlags = pMsg->Readuint64();
   		}
   		break;

		case CFX_STEALTH_MSG:
		{
            m_cs.fStealthPercent = pMsg->Readfloat();
		}
		break;

		case CFX_CLIENTID_MSG:
		{
            m_cs.nClientID = pMsg->Readuint8();
			if( g_pGameClientShell->ShouldUseRadar() )
			{
				g_pRadar->UpdatePlayerID( m_hServerObject, m_cs.nClientID );
			}


		}
		break;

		case CFX_CHAT_MSG:
		{
            m_cs.SetChatting((LTBOOL)pMsg->Readuint8());
			if ( m_linkChatFX.IsValid() ) 
			{
				if (m_cs.IsChatting())
					m_linkChatFX.GetInstance()->Show();
				else
					m_linkChatFX.GetInstance()->Hide();
			}

		}
		break;

		case CFX_ZZZ_CREATE_MSG:
		{
			_ASSERT(!(m_cs.byFXFlags & CHARCREATESTRUCT::eZzz));
			m_cs.byFXFlags |= CHARCREATESTRUCT::eZzz;
		}
		break;

		case CFX_ZZZ_DESTROY_MSG:
		{
			_ASSERT(m_cs.byFXFlags & CHARCREATESTRUCT::eZzz);
			m_cs.byFXFlags &= ~CHARCREATESTRUCT::eZzz;
		}
		break;

		case CFX_CIGARETTESMOKE_CREATE_MSG:
		{
			_ASSERT(!(m_cs.byFXFlags & CHARCREATESTRUCT::eCigaretteSmoke));
			m_cs.byFXFlags |= CHARCREATESTRUCT::eCigaretteSmoke;
		}
		break;

		case CFX_CIGARETTESMOKE_DESTROY_MSG:
		{
			_ASSERT(m_cs.byFXFlags & CHARCREATESTRUCT::eCigaretteSmoke);
			m_cs.byFXFlags &= ~CHARCREATESTRUCT::eCigaretteSmoke;
		}
		break;

		case CFX_CIGARETTE_CREATE_MSG:
		{		
			_ASSERT(!(m_cs.byFXFlags & CHARCREATESTRUCT::eCigarette));
			m_cs.byFXFlags |= CHARCREATESTRUCT::eCigarette;
		}
		break;

		case CFX_CIGARETTE_DESTROY_MSG:
		{
			_ASSERT(m_cs.byFXFlags & CHARCREATESTRUCT::eCigarette);
			m_cs.byFXFlags &= ~CHARCREATESTRUCT::eCigarette;
			m_cs.byFXFlags &= ~CHARCREATESTRUCT::eCigaretteSmoke;
		}
		break;

		case CFX_FLASHLIGHT_CREATE_MSG:
		{
			_ASSERT(!(m_cs.byFXFlags & CHARCREATESTRUCT::eFlashLight));
			m_cs.byFXFlags |= CHARCREATESTRUCT::eFlashLight;
		}
		break;

		case CFX_FLASHLIGHT_DESTROY_MSG:
		{
			_ASSERT(m_cs.byFXFlags & CHARCREATESTRUCT::eFlashLight);
			m_cs.byFXFlags &= ~CHARCREATESTRUCT::eFlashLight;
		}
		break;

		case CFX_ALLFX_MSG:
		{
			// Re-init our data...

            m_cs.Read(pMsg);

			InitLocalPlayer();
		}
		break;

		case CFX_INFO_STRING:
		{
			pMsg->ReadString(m_szInfoString,kMaxInfoStringLength);
			char szTmp[kMaxInfoStringLength];
			SAFE_STRCPY(szTmp,m_szInfoString);
			char *pTok = strtok(szTmp,"\n");
			if (pTok)
				m_pStr->SetText(pTok);
			else
				m_pStr->SetText("");

		}
		break;
		
		case CFX_WEAPON_SOUND_LOOP_MSG :
		{
			// Only play sounds that did not originate from us...

			if( g_pLTClient->GetClientObject() != m_hServerObject )
			{
				HandleWeaponSoundLoopMsg( pMsg );				
			}
		}
		break;

		case CFX_TRACK_TARGET_MSG:
		{			
			m_TrackedNodeContext.HandleServerMessage( pMsg );
		}
		break;
		
		case CFX_PITCH:
		{
			HandlePitchMsg( pMsg );
		}
		break;

		case CFX_HITBOX_MSG:
		{
			LTVector vDims, vOffset;
			bool bCanBeSearched;
			
			vDims = pMsg->ReadCompLTVector();
			vOffset = pMsg->ReadCompLTVector();
			bCanBeSearched = pMsg->Readbool();

			// Set the dims and offset on the hitbox
			
			m_HitBox.SetDims( vDims );
			m_HitBox.SetOffset( vOffset );
			m_HitBox.SetCanBeSearched( bCanBeSearched );

		}
		break;

		case CFX_CAN_CARRY:
		{
			m_cs.bCanCarry = !!(pMsg->Readuint8());
		}
		break;

		case CFX_CAN_WAKE:
		{
			m_cs.bCanWake = !!(pMsg->Readuint8());
		}
		break;

		case CFX_PLAYER_DEAD:
		{
			if( m_cs.bIsPlayer && IsMultiplayerGame() && g_pGameClientShell->ShouldUseRadar() )
			{
				m_bPlayerDead = true;
				if (m_cs.bRadarVisible)
				{
					g_pRadar->SetPlayerDead(m_hServerObject,true);
				}
				else if (m_cs.bTracking)
				{
					g_pRadar->RemoveObject(m_hServerObject);
				}
			}

			ShutdownDamageFX();
		}
		break;

		case CFX_PLAYER_REVIVED:
		{
			if( m_cs.bIsPlayer && IsMultiplayerGame() && g_pGameClientShell->ShouldUseRadar() )
			{
				m_bPlayerDead = false;
				if (m_cs.bRadarVisible)
				{
					g_pRadar->SetPlayerDead(m_hServerObject,false);
				}
			}
		}
		break;
   
		case CFX_PLAYER_RESPAWN:
		{
			if( m_cs.bIsPlayer && IsMultiplayerGame() && g_pGameClientShell->ShouldUseRadar())
			{
				m_bPlayerDead = false;
				if (m_cs.bRadarVisible)
				{
					g_pRadar->SetPlayerDead(m_hServerObject,false);
				}
			}
		}
		break;
		
		case CFX_CHARACTER_RADAR:
		{
			m_cs.bRadarVisible = pMsg->Readbool();

			if( m_cs.bRadarVisible )
			{
				if( m_cs.bIsPlayer )
					g_pRadar->AddPlayer( m_hServerObject, m_cs.nClientID );
				else
					g_pRadar->AddObject( m_hServerObject, RADAR_AI_TRACKING, INVALID_TEAM );
			}
			else
			{
				g_pRadar->RemoveObject( m_hServerObject );
			}
		}
		break;

		case CFX_CHARACTER_TRACKING:
		{
			m_cs.bTracking = pMsg->Readbool();

			if( m_cs.bTracking )
			{
				if( m_cs.bIsPlayer )
				{
					g_pRadar->AddObject( m_hServerObject, RADAR_PLAYER_TRACKING, INVALID_TEAM );
				}
				else
				{
					g_pRadar->AddObject( m_hServerObject, RADAR_AI_TRACKING, INVALID_TEAM );
				}
			}
			else
			{
				// If we're not intended to be visible, then remove us.
				if( !m_cs.bRadarVisible)
				{
					g_pRadar->RemoveObject( m_hServerObject );
				}
				// We need to re-add ourselves to get our tracking/team color right.
				else
				{
					g_pRadar->RemoveObject( GetServerObj( ));
					g_pRadar->AddPlayer( GetServerObj(), m_cs.nClientID );
				}
			}
		}
		break;

		case CFX_CINEMATICAI_MSG:
		{
			m_cs.bIsCinematicAI = pMsg->Readbool();
		}
		break;

		default : break;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::PlayLipSyncSound
//
//	PURPOSE:	Play a lip synced sound.
//
// ----------------------------------------------------------------------- //

HLTSOUND CCharacterFX::PlayLipSyncSound(char* szSound, LTFLOAT fRadius, LTBOOL & bSubtitle, bool bSubtitlePriority, bool bGetHandle /*= true */)
{

	bSubtitle = LTFALSE;

    if (!szSound || !szSound[0] || fRadius <= 0.0f) return LTNULL;

    uint32 dwFlags = 0;
	if (bGetHandle)
	{
		dwFlags = PLAYSOUND_GETHANDLE;
	}

	LTBOOL bIsLocalClient = LTFALSE;
	SoundPriority ePriority = SOUNDPRIORITY_AI_HIGH;
	if (m_cs.bIsPlayer)
	{
		ePriority = SOUNDPRIORITY_PLAYER_HIGH;

        bIsLocalClient = (g_pLTClient->GetClientObject() == m_hServerObject);
		dwFlags |= bIsLocalClient ? PLAYSOUND_CLIENTLOCAL : 0;
	}


	// Show subtitles? (Dialogue sounds only)...

	LTVector vPos;
	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

	uint32 nStringId = g_pClientSoundMgr->GetSoundIdFromFilename(szSound);

	if (nStringId)
	{
		char szStr[128] = "";	
		g_pClientSoundMgr->GetSoundFilenameFromId("Dialogue", nStringId, szStr, sizeof(szStr));

		// Okay, make sure that id we got really is a dialogue sound...

		if (strlen(szStr) && (stricmp(szSound, szStr) == 0))
		{
			// If we're in a cinematic use the cinematic radius, else
			// use the conversation radius...

			if (g_pPlayerMgr->IsUsingExternalCamera())
			{
				// Okay this dialogue is being played during a cinematic so make sure
				// we are either the player or a cinematic ai.  If not, don't play the
				// sound.
				
				if (bIsLocalClient || m_cs.bIsCinematicAI)
				{
					fRadius = g_vtDialogueCinematicSoundRadius.GetFloat();

					// Since we're in a cinematic, force the dialogue to played
					// in the player's head (so it feels like a movie)
					dwFlags |= PLAYSOUND_CLIENTLOCAL;

					// Unload cinematic dialogue sounds after they are played...
					// NOTE: This will only unload dialogue sounds in cinematics,
					// to unload the conversation dialogue sounds we need to come
					// up with another approach...
					dwFlags |= PLAYSOUND_ONCE;
				}
				else
				{
					return LTNULL;  // Don't play this sound
				}
			}

			if (dwFlags & PLAYSOUND_CLIENTLOCAL)
			{
				// Force subtitle to be shown...
				vPos.Init();
			}

			nStringId += SUBTITLE_STRINGID_OFFSET;
			bSubtitle = LTTRUE;
		}
	}

	HLTSOUND hSound = LTNULL;

	if (bGetHandle)
	{
		hSound = g_pClientSoundMgr->PlaySoundFromObject(m_hServerObject,
			szSound, fRadius, ePriority, dwFlags, SMGR_DEFAULT_VOLUME, 1.0f, -1.0f, SPEECH_SOUND_CLASS );
	}
	else
	{

		g_pClientSoundMgr->PlaySoundFromPos(vPos, szSound, fRadius, ePriority);
	}
	
	if (bSubtitle && hSound)
	{
		LTFLOAT fDuration = -1.0f;
		g_pLTClient->SoundMgr()->GetSoundDuration(hSound, fDuration);
		bSubtitle = g_pSubtitles->Show(nStringId, vPos, fRadius, fDuration, bSubtitlePriority);
	}

	return hSound;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::PlayTaunt
//
//	PURPOSE:	Play taunt sound
//
// ----------------------------------------------------------------------- //

void CCharacterFX::PlayTaunt(uint32 nTauntId, LTBOOL bForce)
{
	if (!m_cs.bIsPlayer) return;
	if (GetConsoleInt("IgnoreTaunts", 0)) return;

	LTBOOL bIsLocalClient = (g_pLTClient->GetClientObject() == m_hServerObject);
	if (bIsLocalClient && !bForce) return;

	char szStr[128] = "";
	g_pClientSoundMgr->GetSoundFilenameFromId("Dialogue", nTauntId, szStr, sizeof(szStr));
	if (!szStr[0]) return;

	m_NodeController.HandleNodeControlLipSync(szStr, DEFAULT_TAUNT_RADIUS);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateBreathFX
//
//	PURPOSE:	Update breath fx if appropriate
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateBreathFX()
{
	//don't bother if we are paused
	if(g_pGameClientShell->IsServerPaused())
		return;

	if (m_eLastSurface == ST_UNKNOWN) 
		return;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eLastSurface);

	if (!pSurf || !pSurf->bShowBreath) 
		return;

	//update our breath timer
	m_fBreathElapsedTime += g_pLTClient->GetFrameTime();

	// If it is time to do a breath, do it...

	if (m_fBreathElapsedTime > m_fBreathEndTime)
	{
		HMODELSOCKET hSocket;
		if (g_pModelLT->GetSocket(m_hServerObject, "Chin", hSocket) == LT_OK)
		{
			LTransform transform;
            if (g_pModelLT->GetSocketTransform(m_hServerObject, hSocket, transform, LTTRUE) == LT_OK)
			{
                LTVector vPos;
                LTRotation rRot;
				vPos = transform.m_Pos;

				g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);
                LTVector vF = rRot.Forward();

				SMCREATESTRUCT sm;

				sm.vPos					= vPos;
                sm.bAdjustParticleScale = LTTRUE;
				sm.fStartParticleScale  = GetRandom(g_pClientButeMgr->GetBreathFXAttributeFloat("MinPStartScale"),
					g_pClientButeMgr->GetBreathFXAttributeFloat("MaxPStartScale"));
				sm.fEndParticleScale  = GetRandom(g_pClientButeMgr->GetBreathFXAttributeFloat("MinPEndScale"),
					g_pClientButeMgr->GetBreathFXAttributeFloat("MaxPEndScale"));

                sm.bAdjustParticleAlpha = LTTRUE;
				sm.fStartParticleAlpha  = g_pClientButeMgr->GetBreathFXAttributeFloat("PStartAlpha");
				sm.fEndParticleAlpha    = g_pClientButeMgr->GetBreathFXAttributeFloat("PEndAlpha");

				char szSpr[128] = "";
				g_pClientButeMgr->GetBreathFXAttributeString("Sprite",szSpr,sizeof(szSpr));
                sm.hstrTexture = g_pLTClient->CreateString(szSpr);

				sm.vColor1.Init(255.0, 255.0, 255.0);
				sm.vColor2.Init(255.0, 255.0, 255.0);

				sm.vMinDriftVel = g_pClientButeMgr->GetBreathFXAttributeVector("MinVel");
				sm.vMaxDriftVel = g_pClientButeMgr->GetBreathFXAttributeVector("MaxVel");

                LTFLOAT fVel = g_pClientButeMgr->GetBreathFXAttributeFloat("ForwardVel");
				sm.vMinDriftVel			+= (vF * fVel);
				sm.vMaxDriftVel			+= (vF * fVel * 1.25);
				sm.fVolumeRadius		= g_pClientButeMgr->GetBreathFXAttributeFloat("Volume");
				sm.fRadius				= g_pClientButeMgr->GetBreathFXAttributeFloat("Radius");
				sm.fMinParticleLife		= g_pClientButeMgr->GetBreathFXAttributeFloat("MinPLife");
				sm.fMaxParticleLife		= g_pClientButeMgr->GetBreathFXAttributeFloat("MaxPLife");
				sm.nNumParticles		= g_pClientButeMgr->GetBreathFXAttributeInt("NumParticles");

                sm.bIgnoreWind          = LTFALSE;
				sm.fLifeTime			= sm.fMaxParticleLife;
				sm.fParticleCreateDelta	= (sm.fLifeTime * 4.0f);  // Only create once

				CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
				if (psfxMgr)
				{
					psfxMgr->CreateSFX(SFX_SMOKE_ID, &sm);
				}

                g_pLTClient->FreeString(sm.hstrTexture);
			}
		}

		m_fBreathElapsedTime = 0.0f;
		m_fBreathEndTime = g_vtBreathTime.GetFloat() * GetRandom(0.75f, 1.25f);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::ShowAttachClientFX
//
//	PURPOSE:	Show all the player view attach client fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::ShowAttachClientFX()
{
	for ( CLIENTFX_LINK_NODE* pCurr = m_AttachClientFX.m_pNext; pCurr; pCurr = pCurr->m_pNext )
	{
		if ( pCurr->m_Link.IsValid() )
		{
			pCurr->m_Link.GetInstance()->Show();
		}
		else
		{
			// when we hit 0, there are no more
			return;
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::HideAttachClientFX
//
//	PURPOSE:	Hide all the player view attach client fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::HideAttachClientFX()
{
	for ( CLIENTFX_LINK_NODE* pCurr = m_AttachClientFX.m_pNext; pCurr; pCurr = pCurr->m_pNext )
	{
		if ( pCurr->m_Link.IsValid() )
		{
			pCurr->m_Link.GetInstance()->Hide();
		}
		else
		{
			// when we hit 0, there are no more
			return;
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateAttachClientFX
//
//	PURPOSE:	Create all the player view attach client fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateAttachClientFX()
{
	ASSERT( 0 != g_pClientFXMgr );
	ASSERT( 0 != g_pModelButeMgr );

	RemoveAttachClientFX();
	
	ModelId mModelId = GetModelId();

	// Don't do attachments if the modelid is bad.  The Speaker object
	// can have an invalid modelid.
	if( mModelId == eModelIdInvalid )
		return;

	int nNumClientFX = g_pModelButeMgr->GetNumClientFX( mModelId );
	for ( int i = 0; i < nNumClientFX ; ++i )
	{
		char const *pClientFXName = g_pModelButeMgr->GetClientFX( mModelId, i );
		if ( pClientFXName && ( '\0' != pClientFXName[ 0 ] ) )
		{
			CLIENTFX_CREATESTRUCT fxInit( pClientFXName, FXFLAG_LOOP, m_hServerObject );

			CLIENTFX_LINK_NODE* pNewNode = debug_new(CLIENTFX_LINK_NODE);

			if(pNewNode)
			{
				g_pClientFXMgr->CreateClientFX( &pNewNode->m_Link, fxInit, true );
				if ( pNewNode->m_Link.IsValid() ) 
				{
					// start out hidden
					pNewNode->m_Link.GetInstance()->Hide();
					
					// add the link to the list
					m_AttachClientFX.AddToEnd(pNewNode);
				}
			}
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::RemoveAttachClientFX
//
//	PURPOSE:	Destroys all the player view attach client fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::RemoveAttachClientFX()
{
	ASSERT( 0 != g_pClientFXMgr );

	for ( CLIENTFX_LINK_NODE* pCurr = m_AttachClientFX.m_pNext; pCurr; pCurr = pCurr->m_pNext )
	{
		if ( pCurr->m_Link.IsValid() )
		{
			g_pClientFXMgr->ShutdownClientFX( &pCurr->m_Link );
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::PlayDingSound
//
//	PURPOSE:	Play an impact ding sound
//
// ----------------------------------------------------------------------- //

void CCharacterFX::PlayDingSound()
{
	if (IsMultiplayerGame())
	{
		for (int i=0; i < MAX_DINGS; i++)
		{
			if (m_fNextDingTime[i] == -1.0f)
			{
				m_fNextDingTime[i] = g_pLTClient->GetTime();
//				g_pLTClient->CPrint("Adding Ding Sound at Time %.2f", m_fNextDingTime[i]);
				m_fNextDingTime[i] += g_vtDingDelay.GetFloat();
				break;
			}
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::InitLocalPlayer
//
//	PURPOSE:	Initialize the local player
//
// ----------------------------------------------------------------------- //

void CCharacterFX::InitLocalPlayer()
{
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (m_cs.bIsPlayer && hPlayerObj == m_hServerObject)
	{
		// Set up our move mgr...

		g_pPlayerMgr->GetMoveMgr()->SetCharacterFX(this); 

		// Set the first-person camera offset...

		g_pPlayerMgr->GetPlayerCamera()->SetFirstPersonOffset(GetPlayerHeadOffset( ));

		// Update the player-view weapon model so that it uses the correct
		// textures based on the model style...

		// this is called to update the skins on the playerview
		// model (to reflect the costume of the character),
		// figure out how to do this differently
		IClientWeaponBase *pClientWeapon = g_pPlayerMgr->GetClientWeaponMgr()->GetCurrentClientWeapon();
		if ( pClientWeapon )
		{
			pClientWeapon->ResetWeaponFilenames();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCharacterFX::HandleWeaponSoundLoopMsg
//
//  PURPOSE:	Play or stop a looping weapon sound...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::HandleWeaponSoundLoopMsg( ILTMessage_Read *pMsg )
{
	uint8 nType		= pMsg->Readuint8();
	uint8 nWeaponID	= pMsg->Readuint8();

	const WEAPON *pWeapon = g_pWeaponMgr->GetWeapon( nWeaponID );
	if( !pWeapon ) return;

	char* pBuf = LTNULL;

	switch( nType )
	{
		case PSI_RELOAD:														// 1
		case PSI_RELOAD2:														// 2
		case PSI_RELOAD3:														// 3
		{
								pBuf = pWeapon->szReloadSounds[ ( nType - PSI_RELOAD ) ];
		}
		break;

		case PSI_SELECT:		pBuf = pWeapon->szSelectSound;			break;	// 4
		case PSI_DESELECT:		pBuf = pWeapon->szDeselectSound;		break;	// 5
		case PSI_FIRE:			pBuf = pWeapon->szFireSound;			break;	// 6
		case PSI_DRY_FIRE:		pBuf = pWeapon->szDryFireSound;			break;	// 7
		case PSI_ALT_FIRE:		pBuf = pWeapon->szAltFireSound;			break;	// 8
		case PSI_SILENCED_FIRE:	pBuf = pWeapon->szSilencedFireSound;	break;	// 9
			
		case PSI_WEAPON_MISC1:													// 10
		case PSI_WEAPON_MISC2:													// 11
		case PSI_WEAPON_MISC3:													// 12
		case PSI_WEAPON_MISC4:													// 13
		case PSI_WEAPON_MISC5:													// 14
		{
								pBuf = pWeapon->szMiscSounds[nType - PSI_WEAPON_MISC1];
		}
		break; 
		
		case PSI_INVALID:
		default:
		{
			KillWeaponLoopSound();
		}
		break;
	}

	if( pBuf && pBuf[0] )
	{
		// Stop any previous looping sound...

		KillWeaponLoopSound();

		LTVector	vPos;
		g_pLTClient->GetObjectPos( m_hServerObject, &vPos );

		// Play the sound from the character

		m_hWeaponLoopSound = g_pClientSoundMgr->PlaySoundFromObject( m_hServerObject, pBuf, (float)pWeapon->nFireSoundRadius,
																	 SOUNDPRIORITY_PLAYER_MEDIUM, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE, 
																	 SMGR_DEFAULT_VOLUME, 1.0f, -1.0f, WEAPONS_SOUND_CLASS );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCharacterFX::KillWeaponLoopSound
//
//  PURPOSE:	Kill any looping sound
//
// ----------------------------------------------------------------------- //

void CCharacterFX::KillWeaponLoopSound()
{
	if( m_hWeaponLoopSound )
	{
		g_pLTClient->SoundMgr()->KillSound( m_hWeaponLoopSound );
		m_hWeaponLoopSound = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCharacterFX::HandlePitchMsg
//
//  PURPOSE:	Update the tracking nodes for pitching
//
// ----------------------------------------------------------------------- //

void CCharacterFX::HandlePitchMsg( ILTMessage_Read *pMsg )
{
	// Unpack the pitch.  It comes from the server in a byte.
	uint8 ucPitch = pMsg->Readuint8( );

	// remap it to a float
	UncompressAngleFromByte( ucPitch, &m_cs.fPitch );

	// Only players can pitch...
	if( !m_cs.bIsPlayer ) 
		return;
	
	float fPitch = m_cs.fPitch;


	LTRotation	rUpperTorsoRot = LTRotation(LTVector(1.0f, 0.0f, 0.0f), fPitch * g_vtPitchScale.GetFloat() + DEG2RAD(g_vtPitchBias.GetFloat()));

	//we first take our player point and extend it along our forward a few thousand units
	LTVector vLookAt = rUpperTorsoRot.Forward() * 1000.0f;

	//we now need to add the pitch onto this....

	
	bool bOrient = false;
	if( g_vtOrientOnAnim.GetFloat() > 0.0f )
	{
		bOrient = true;
	}

	g_pClientTrackedNodeMgr->SetOrientOnAnim( m_hUpperTorsoNode, bOrient );
	g_pClientTrackedNodeMgr->SetOrientOnAnim( m_hLowerTorsoNode, bOrient );

	g_pClientTrackedNodeMgr->SetTargetObject( m_hUpperTorsoNode, vLookAt );
	g_pClientTrackedNodeMgr->SetTargetObject( m_hLowerTorsoNode, vLookAt );	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCharacterFX::UpdatePitch
//
//  PURPOSE:	Handles updating the pitch for this character
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdatePitch( )
{
	// Only players can pitch...
	if( !m_cs.bIsPlayer ) 
		return;

	// Don't allow tracking when playing special damage fx anim or riding vehicle.
	bool bEnable = !m_bDamageFxTrackingOverride && !m_bOnVehicle && g_vtPlayerPitchTracking.GetFloat( );
	if( bEnable )
	{
		// Don't allow tracking when leaning.
		uint32 nFlags = 0;
		g_pCommonLT->GetObjectFlags( m_hServerObject, OFT_User, nFlags );
		bEnable = !( nFlags & USRFLG_PLAYER_LEANING );
	}

	EnablePitch( bEnable );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::ResetPitchTracking
//
//	PURPOSE:	Reset the pitch tracking stuff.  (For resetting the model and things of that ilk.)
//
// ----------------------------------------------------------------------- //
void CCharacterFX::ResetPitchTracking()
{
	// Re-initialize the node tracking
	HOBJECT hLocalObj = g_pLTClient->GetClientObject();
	if( m_cs.bIsPlayer && IsMultiplayerGame() && (m_hServerObject != hLocalObj))
	{
		if( m_hUpperTorsoNode != INVALID_TRACKEDNODE )
		{
			g_pClientTrackedNodeMgr->DestroyTrackingNode( m_hUpperTorsoNode );
			m_hUpperTorsoNode = INVALID_TRACKEDNODE;
		}

		if( m_hLowerTorsoNode != INVALID_TRACKEDNODE )
		{
			g_pClientTrackedNodeMgr->DestroyTrackingNode( m_hLowerTorsoNode );
			m_hLowerTorsoNode = INVALID_TRACKEDNODE;
		}

		m_hUpperTorsoNode = g_pClientTrackedNodeMgr->CreateTrackingNode( m_hServerObject, "Upper_torso" );
		if( m_hUpperTorsoNode != INVALID_TRACKEDNODE )
		{
			float fMax = g_vtUpperTorsoPitchMax.GetFloat();

			g_pClientTrackedNodeMgr->SetNodeConstraints( m_hUpperTorsoNode, 0.0f, 0.0f, DEG2RAD(fMax), DEG2RAD(fMax), DEG2RAD(180.0f) );
			g_pClientTrackedNodeMgr->EnableTracking( m_hUpperTorsoNode, true );
			g_pClientTrackedNodeMgr->SetOrientOnAnim( m_hUpperTorsoNode, true );
		}

		m_hLowerTorsoNode = g_pClientTrackedNodeMgr->CreateTrackingNode( m_hServerObject, "Torso" );
		if( m_hLowerTorsoNode != INVALID_TRACKEDNODE )
		{
			float fMax = g_vtLowerTorsoPitchMax.GetFloat();
			
			g_pClientTrackedNodeMgr->SetNodeConstraints( m_hLowerTorsoNode, 0.0f, 0.0f, DEG2RAD(fMax), DEG2RAD(fMax), DEG2RAD(180.0f) );
			g_pClientTrackedNodeMgr->EnableTracking( m_hLowerTorsoNode, true );
			g_pClientTrackedNodeMgr->SetOrientOnAnim( m_hLowerTorsoNode, true );
		}

		EnablePitch( true );
	}

	// Don't do tracking if the skeleton is bad.  The Speaker object
	// can have an invalid skeleton.
	if( m_cs.eModelSkeleton != eModelSkeletonInvalid )
	{
		m_TrackedNodeContext.Init( m_hServerObject, m_cs.eModelSkeleton, g_pClientTrackedNodeMgr );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::EnablePitch
//
//	PURPOSE:	Enable\Disable the Pitch tracking...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::EnablePitch( bool bEnable )
{
	if( m_bPitchEnabled == bEnable )
		return;

	m_bPitchEnabled = bEnable;

	if( m_hUpperTorsoNode != INVALID_TRACKEDNODE )
	{
		g_pClientTrackedNodeMgr->EnableTracking( m_hUpperTorsoNode, bEnable );
	}

	if( m_hLowerTorsoNode != INVALID_TRACKEDNODE )
	{
		g_pClientTrackedNodeMgr->EnableTracking( m_hLowerTorsoNode, bEnable );	
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::Render
//
//	PURPOSE:	Render the Character debug info.
//
// ----------------------------------------------------------------------- //
void CCharacterFX::Render(HOBJECT hCamera)
{

	// Draw the name.
	if (m_pStr && m_pStr->GetLength())
	{
		m_pStr->Render();
	}
}


void CCharacterFX::ChangeModel(ModelId eModelId)
{
	m_cs.eModelId = eModelId;
	m_cs.eModelSkeleton = g_pModelButeMgr->GetModelSkeleton(eModelId);

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	const char* pFilename = g_pModelButeMgr->GetModelFilename(eModelId);
	SAFE_STRCPY(theStruct.m_Filename, pFilename);

	g_pModelButeMgr->CopySkinFilenames(eModelId, 0, theStruct.m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
	g_pModelButeMgr->CopyRenderStyleFilenames(eModelId, &theStruct);

    g_pCommonLT->SetObjectFilenames(m_hServerObject, &theStruct);

	
	CreateAttachClientFX();

	ResetPitchTracking();
}

//step through the things attached to us and see if we should hide any of them
void CCharacterFX::UpdateAttachments()
{
	HLOCALOBJ attachList[20];
    uint32 dwListSize = 0;
    uint32 dwNumAttach = 0;

    g_pCommonLT->GetAttachments(m_hServerObject, attachList, 20, dwListSize, dwNumAttach);
	int nNum = dwNumAttach <= dwListSize ? dwNumAttach : dwListSize;

	for (int i=0; i < nNum; i++)
	{
        uint32 dwUsrFlags;
        g_pCommonLT->GetObjectFlags(attachList[i], OFT_User, dwUsrFlags);
		
		if (g_pVersionMgr->IsLowViolence() && dwUsrFlags & USRFLG_ATTACH_HIDEGORE)
		{
			g_pCommonLT->SetObjectFlags(attachList[i], OFT_Flags, 0, FLAG_VISIBLE);
		}

	}

	m_bUpdateAttachments = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdatePlayerAlphaCycle
//
//	PURPOSE:	Cycles the alpha on the player.
//
// ----------------------------------------------------------------------- //
void CCharacterFX::UpdatePlayerAlphaCycle( )
{
	// Check if this just turned on.
	if( !m_PlayerAlphaCycleTimer.On( ))
	{
		m_PlayerAlphaCycleTimer.Start( );
	}

	// Get the period of the cycle.
	float fPlayerAlphaPeriod = GetConsoleFloat( "PlayerAlphaPeriod", 1.0f );
	fPlayerAlphaPeriod = Max( fPlayerAlphaPeriod, 0.1f );

	// Multiply the period by 2 since abs( cos ) will give us 2 cycles per normal period.
	fPlayerAlphaPeriod *= 2.0f;

	float fAlphaParameterized = 0;

	LTVector vColor;
	g_pLTClient->GetObjectColor( m_hServerObject, &(vColor.x), &(vColor.y), &(vColor.z), &fAlphaParameterized );

	// Get the parameterized value of the alpha.  
	fAlphaParameterized = ( float )fabs( cos( m_PlayerAlphaCycleTimer.GetElapseTime( ) * MATH_CIRCLE / fPlayerAlphaPeriod ));

	g_pLTClient->SetObjectColor( m_hServerObject, (vColor.x), (vColor.y), (vColor.z), fAlphaParameterized );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::RemovePlayerAlphaCycle
//
//	PURPOSE:	Removos the alpha cycle on the player.
//
// ----------------------------------------------------------------------- //
void CCharacterFX::RemovePlayerAlphaCycle( )
{
	LTVector vColor;
	float fAlpha;
	g_pLTClient->GetObjectColor( m_hServerObject, &(vColor.x), &(vColor.y), &(vColor.z), &fAlpha );
	g_pLTClient->SetObjectColor( m_hServerObject, (vColor.x), (vColor.y), (vColor.z), 1.0f );

	m_PlayerAlphaCycleTimer.Stop( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::KillLipSyncSound
//
//	PURPOSE:	Kill the lipsync sound.
//
// ----------------------------------------------------------------------- //

void CCharacterFX::KillLipSyncSound( bool bSendNotification )
{
	if( m_hDialogueSnd )
	{
		g_pLTClient->SoundMgr()->KillSound(m_hDialogueSnd);
		m_hDialogueSnd = LTNULL;
	}

	if (m_bSubtitle)
	{
		g_pSubtitles->Clear();
	}

	if( bSendNotification )
	{
		// Tell the server that the sound finished.
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_OBJECT_MESSAGE );
		cMsg.WriteObject( m_hServerObject );
		cMsg.Writeuint32( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_CHARACTER_ID );
		cMsg.Writeuint8( CFX_DIALOGUE_MSG );
		cMsg.Writeuint8( m_nUniqueDialogueId );
		g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::ShutdownDamageFX
//
//	PURPOSE:	Shutdown any damageFX on this character...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::ShutdownDamageFX()
{
	// Shutdown any damageFX...

	if( m_link3rdPersonInstFX.IsValid() )
	{
		g_pClientFXMgr->ShutdownClientFX( &m_link3rdPersonInstFX );
	}

	if( !m_p3rdPersonDamageFX )
		return;
	
	for( uint32 nCurrDamage = 0; nCurrDamage < m_nNum3rdPersonDamageFX; ++nCurrDamage )
	{
		if( m_p3rdPersonDamageFX[nCurrDamage].IsValid() )
		{
			g_pClientFXMgr->ShutdownClientFX( &m_p3rdPersonDamageFX[nCurrDamage] );
		}
	}	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::OnObjectRotate
//
//	PURPOSE:	Gaive the characterFX a chance to modify the rotation...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::OnObjectRotate( LTRotation *pRot )
{
	if( !pRot )
		return;

	bool bLocalClient = (m_hServerObject == g_pLTClient->GetClientObject());

	// Update the contour rotation of the vehicle...
	// Not the local client though since we never see his vehicle.

	if( !bLocalClient )
		UpdateVehicleContour( *pRot );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateCarrying
//
//	PURPOSE:	Update the player's carrying state...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateCarrying()
{
	// we only care about the local player, so bail for anything else...
	if( !m_cs.bIsPlayer) return;
	if (m_hServerObject != g_pLTClient->GetClientObject()) return;
	
	g_pPlayerMgr->SetCarryingObject(m_cs.nCarrying, true);
}
