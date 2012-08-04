// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterFX.cpp
//
// PURPOSE : Character special FX - Implementation
//
// CREATED : 8/24/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
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
#include "DamageFXMgr.h"
#include "VarTrack.h"
#include "ClientButeMgr.h"
#include "MsgIDs.h"
#include "PlayerShared.h"
#include "VolumeBrushFX.h"

extern CGameClientShell* g_pGameClientShell;
extern CClientButeMgr* g_pClientButeMgr;


extern LTVector g_vPlayerCameraOffset;

#define INTERSECT_Y_OFFSET		50.0f
#define FOOTSTEP_SOUND_RADIUS	1000.0f
#define KEY_FOOTSTEP_SOUND		"FOOTSTEP_KEY"

#define SUBTITLE_STRINGID_OFFSET	0
#define DEFAULT_TAUNT_RADIUS		1500.0f
#define DEFAULT_VEHICLE_RADIUS		2500.0f

#define DEFAULT_CIGARETTE_TEXTURE	"SFX\\Impact\\Spr\\Smoke.spr"
#define DEFAULT_ZZZ_TEXTURE			"SFX\\Particle\\Sleep.dtx"
#define DEFAULT_HEART_TEXTURE		"SFX\\Particle\\Heart.dtx"
#define DEFAULT_SMOKEPUFF_TEXTURE	"SFX\\smoke\\sprtex\\smoke2_10.dtx"

SurfaceType g_eClientLastSurfaceType = ST_UNKNOWN;
extern VarTrack	g_vtTestBleedingFX;
extern VarTrack	g_vtTestPoisonFX;
extern VarTrack	g_vtTestStunFX;
extern VarTrack	g_vtTestSleepingFX;
extern VarTrack	g_vtTestBurnFX;
extern VarTrack	g_vtTestChokeFX;
extern VarTrack	g_vtTestElectrocuteFX;

VarTrack g_vtBreathTime;
VarTrack g_vtZipCord1stWidth;
VarTrack g_vtZipCord3rdWidth;
VarTrack g_vtModelKey;
VarTrack g_vtFootPrintBlend;
VarTrack g_vtMinTrailSegment;
VarTrack g_vtTrailSegmentLifetime;
VarTrack g_vtDialogueCinematicSoundRadius;
VarTrack g_vtVehicleTrials;
VarTrack g_vtDingDelay;

const LTVector g_kvPlayerScubaCamOffset(0.0, 41.0, 0.0);

namespace
{
	LTFLOAT fCycleTime = 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::Init
//
//	PURPOSE:	Init the character fx
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	CHARCREATESTRUCT ch;

	ch.hServerObj = hServObj;
    ch.Read(g_pLTClient, hMessage);

	if (fCycleTime < 0.1f)
	{
		fCycleTime = g_pClientButeMgr->GetSpecialFXAttributeFloat("MarkerCycleTime");
		if (fCycleTime < 0.1f) fCycleTime = 1.0f;
	}

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

	// TODO: a more intelligent allocation of anim trackers

	for ( int iAnimTracker = 0 ; iAnimTracker < m_cs.nTrackers ; iAnimTracker++ )
	{
		g_pModelLT->AddTracker(m_hServerObject, &m_aAnimTrackers[iAnimTracker]);
	}

	// Init the node controller

	if ( !m_NodeController.Init(this) )
	{
        return LTFALSE;
	}

	if ( !m_cs.bIsPlayer )
	{
		m_Flashlight.Init(m_hServerObject);
	}

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
	RemoveSmokepuffsFX();
	RemoveZzzFX();
	RemoveHeartsFX();
	RemoveLaserFX();
	RemoveMarkerFX();

    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (m_cs.bIsPlayer && (hPlayerObj == m_hServerObject))
	{
		// Stop all damage fx...

		//g_pGameClientShell->GetDamageFXMgr()->StopBleedingFX();
		//g_pGameClientShell->GetDamageFXMgr()->StopPoisonFX();
		//g_pGameClientShell->GetDamageFXMgr()->StopStunFX();
		//g_pGameClientShell->GetDamageFXMgr()->StopSleepingFX();
		//g_pGameClientShell->GetDamageFXMgr()->StopBurnFX();
		//g_pGameClientShell->GetDamageFXMgr()->StopElectrocuteFX();
	}

	for ( int iAnimTracker = 0 ; iAnimTracker < m_cs.nTrackers ; iAnimTracker++ )
	{
		g_pModelLT->RemoveTracker(m_hServerObject, &m_aAnimTrackers[iAnimTracker]);
	}

	if (m_hDialogueSnd)
	{
		g_pLTClient->KillSound(m_hDialogueSnd);
	}

	if (m_hVehicleSound)
	{
		g_pLTClient->KillSound(m_hVehicleSound);
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

	if (!g_vtZipCord1stWidth.IsInitted())
	{
        g_vtZipCord1stWidth.Init(pClientDE, "ZipCord1stWidth", LTNULL, 2.0f);
	}

	if (!g_vtZipCord3rdWidth.IsInitted())
	{
        g_vtZipCord3rdWidth.Init(pClientDE, "ZipCord3rdWidth", LTNULL, 40.0f);
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

	m_BreathTimer.Start(g_vtBreathTime.GetFloat());

	// NOTE: Since we only use node control for the mouth now, we can safely use CF_INSIDERADIUS
    uint32 dwCFlags = m_pClientDE->GetObjectClientFlags(m_hServerObject);
	dwCFlags |= CF_NOTIFYMODELKEYS | CF_INSIDERADIUS;

	// Set up MoveMgr's point to us, if applicable...

    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (m_cs.bIsPlayer && hPlayerObj == m_hServerObject)
	{
		g_pGameClientShell->GetMoveMgr()->SetCharacterFX(this);
		InitLocalPlayer();
	}
	else
	{
		if (IsMultiplayerGame())
		{
			dwCFlags |= CF_DONTSETDIMS;
		}
		uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hServerObject) | FLAG_SOLID | ((IsMultiplayerGame()) ? (FLAG_STAIRSTEP | FLAG_GRAVITY) : 0);
		g_pLTClient->SetObjectFlags(m_hServerObject, dwFlags);
	}

	m_pClientDE->SetObjectClientFlags(m_hServerObject, dwCFlags);

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

	// See if our server side object is active

	uint32 dwUserFlags;
	g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

	if ( !(dwUserFlags & USRFLG_GAMEBASE_ACTIVE) )
	{
		return LTTRUE;
	}

	// Make us solid if our ai usrflg solid is set

    uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hServerObject);

	if ( dwUserFlags & USRFLG_AI_CLIENT_SOLID )
	{
		dwFlags |= FLAG_SOLID;
	}
	else if ( !m_cs.bIsPlayer )
	{
		dwFlags &= ~FLAG_SOLID;
	}

    g_pLTClient->SetObjectFlags(m_hServerObject, dwFlags);

	// Update

	m_Flashlight.Update();

    g_pLTClient->ProcessAttachments(m_hServerObject);

    LTBOOL bIsLocalClient = LTFALSE;

    LTVector vPos;
	m_pClientDE->GetObjectPos(m_hServerObject, &vPos);

	// Update an player-specific fx...

	if (m_cs.bIsPlayer)
	{
		// Only do these if this player is the current client...

        HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if (hPlayerObj == m_hServerObject)
		{
		    bIsLocalClient = LTTRUE;

			// Update our last surface...

		 	m_eLastSurface = g_pGameClientShell->GetMoveMgr()->GetStandingOnSurface();

			if (g_pInterfaceMgr->GetGameState() == GS_PLAYING)
			{
				UpdateDamageFX();
			}
		}
		else  //do these things if it is some player other than the local client
		{
			if (g_pGameClientShell->GetGameType() != SINGLE)
			{
			    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
				if ( hPlayerObj != m_hServerObject )
				{
					UpdateMarkerFX();
				}
			}
		}

	}
	else
	{
		UpdateMarkerFX();
	}


    uint32 dwUsrFlags;
	m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwUsrFlags);

	// Update 1.002 [KLS] calculate if we're under on the client so
	// we don't have to send this info from the server...
	LTBOOL bUnderWater = LTFALSE;
	if (IsMultiplayerGame())
	{
		if (m_cs.bIsPlayer)
		{
			if (bIsLocalClient)
			{
				bUnderWater = IsLiquid(g_pGameClientShell->GetCurContainerCode());
			}
			else
			{
				// Find any liquid containers we're in...

				HLOCALOBJ objList[5];
				uint32 dwNum = g_pLTClient->GetPointContainers(&vPos, objList, 5);

				for (uint32 i=0; i < dwNum; i++)
				{
					uint16 code;
					if (g_pLTClient->GetContainerCode(objList[i], &code))
					{
						if (IsLiquid((ContainerCode)code))
						{
							bUnderWater = LTTRUE;
							break;
						}
					}
				}
			}
		}
	}
	else  // Single player
	{
		bUnderWater = ((dwUsrFlags & USRFLG_PLAYER_UNDERWATER) ? LTTRUE : LTFALSE);
	}

	if (bUnderWater)
	{
		UpdateUnderwaterFX(vPos);
	}
	else
	{
		RemoveUnderwaterFX();
	}


	// Update various cartoony FX

	if ( m_cs.byFXFlags & CHARCREATESTRUCT::eSmokepuffs )
	{
		UpdateSmokepuffsFX();
	}
	else
	{
		RemoveSmokepuffsFX();
	}

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

	if ( m_cs.byFXFlags & CHARCREATESTRUCT::eHearts )
	{
		UpdateHeartsFX();
	}
	else
	{
		RemoveHeartsFX();
	}

	// Blood splats

	if (dwUsrFlags & USRFLG_CHAR_BLEEDING)
	{

	}

	// Update weapon laser fx...

	if (dwUsrFlags & USRFLG_CHAR_LASER)
	{
		UpdateLaserFX();
	}
	else
	{
		RemoveLaserFX();
	}

	// Update node controller...

	m_NodeController.Update();


	// Update being on vehicle if necessary...

    UpdateOnVehicle();


	// Update breath fx...

	UpdateBreathFX();


	// Update zipcord fx...

	UpdateZipCordFX();


	// Update our sounds...

	UpdateSounds();


	// Update the dims of the object based on the animation for other players
	if (IsMultiplayerGame() && (g_pLTClient->GetClientObject() != m_hServerObject))
	{
		uint32 nAnim;
		if (m_cs.nDimsTracker < m_cs.nTrackers)
			g_pModelLT->GetCurAnim(&m_aAnimTrackers[m_cs.nDimsTracker], nAnim);
		else
			nAnim = g_pLTClient->GetModelAnimation(m_hServerObject);
		LTVector vDims;
		g_pLTClient->Common()->GetModelAnimUserDims(m_hServerObject, &vDims, nAnim);
		g_pPhysicsLT->SetObjectDims(m_hServerObject, &vDims, 0);
	}

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
	if (!m_hServerObject) return;

    uint32 dwUsrFlags;
	m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwUsrFlags);

	if (dwUsrFlags & USRFLG_CHAR_BLEEDING || g_vtTestBleedingFX.GetFloat() > 0.0f)
	{
		// Start bleeding fx if necessary...
		g_pGameClientShell->GetDamageFXMgr()->StartBleedingFX();
	}
	else
	{
		// Stop any current bleeding fx...
		g_pGameClientShell->GetDamageFXMgr()->StopBleedingFX();
	}

	if (dwUsrFlags & USRFLG_CHAR_POISONED || g_vtTestPoisonFX.GetFloat() > 0.0f)
	{
		// Start poison fx if necessary...
		g_pGameClientShell->GetDamageFXMgr()->StartPoisonFX();
	}
	else
	{
		// Stop any current poison fx...
		g_pGameClientShell->GetDamageFXMgr()->StopPoisonFX();
	}

	if (dwUsrFlags & USRFLG_CHAR_STUNNED || g_vtTestStunFX.GetFloat() > 0.0f)
	{
		// Start stun fx if necessary...
		g_pGameClientShell->GetDamageFXMgr()->StartStunFX();
	}
	else
	{
		// Remove any current stun fx...
		g_pGameClientShell->GetDamageFXMgr()->StopStunFX();
	}

	if (dwUsrFlags & USRFLG_CHAR_SLEEPING || g_vtTestSleepingFX.GetFloat() > 0.0f)
	{
		// Start sleeping fx if necessary...
		g_pGameClientShell->GetDamageFXMgr()->StartSleepingFX();
	}
	else
	{
		// Remove any current sleeping fx...
		g_pGameClientShell->GetDamageFXMgr()->StopSleepingFX();
	}

	if (dwUsrFlags & USRFLG_CHAR_BURN || g_vtTestBurnFX.GetFloat() > 0.0f)
	{
		// Start Burn fx if necessary...
		g_pGameClientShell->GetDamageFXMgr()->StartBurnFX();
	}
	else
	{
		// Stop any current Burn fx...
		g_pGameClientShell->GetDamageFXMgr()->StopBurnFX();
	}

	if (dwUsrFlags & USRFLG_CHAR_ELECTROCUTE || g_vtTestElectrocuteFX.GetFloat() > 0.0f)
	{
		// Start Electrocute fx if necessary...
		g_pGameClientShell->GetDamageFXMgr()->StartElectrocuteFX();
	}
	else
	{
		// Stop any current Electrocute fx...
		g_pGameClientShell->GetDamageFXMgr()->StopElectrocuteFX();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::HandleDialogueMsg
//
//	PURPOSE:	Start/Stop a dialogue sound
//
// ----------------------------------------------------------------------- //

void CCharacterFX::HandleDialogueMsg(HMESSAGEREAD hMessage)
{
    HSTRING hSound = g_pLTClient->ReadFromMessageHString(hMessage);
    LTFLOAT fRadius = g_pLTClient->ReadFromMessageFloat(hMessage);

	char szSound[128];
	*szSound = 0;

	if (hSound)
	{
		strcpy(szSound, g_pLTClient->GetStringData(hSound));
	    g_pLTClient->FreeString(hSound);
	}

	if (m_hDialogueSnd)
	{
		g_pLTClient->KillSound(m_hDialogueSnd);
        m_hDialogueSnd = LTNULL;

        g_pInterfaceMgr->ClearSubtitle();
	}

	if (*szSound && fRadius > 0.0f)
	{
		m_bSubtitle = LTFALSE;
		m_hDialogueSnd = PlayLipSyncSound(szSound, fRadius, m_bSubtitle);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::HandleTauntMsg
//
//	PURPOSE:	Start a taunt sound
//
// ----------------------------------------------------------------------- //

void CCharacterFX::HandleTauntMsg(HMESSAGEREAD hMessage)
{
    uint32 nTauntID = g_pLTClient->ReadFromMessageDWord(hMessage);
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
			g_pLTClient->KillSound(m_hDialogueSnd);
		 m_hDialogueSnd = LTNULL;

			if (m_bSubtitle)
			{
				g_pInterfaceMgr->ClearSubtitle();
			}
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
				g_pLTClient->CPrint("Playing Ding Sound at Time %.2f", m_fNextDingTime[i]);
				m_fNextDingTime[i] = -1.0f;
				char* pSound = "Guns\\snd\\Impacts\\MultiDing.wav";
				g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateLaserFX
//
//	PURPOSE:	Update the laser fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateLaserFX()
{
	if (!m_hServerObject) return;

    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
    LTBOOL bIsLocalClient = (m_cs.bIsPlayer && hPlayerObj == m_hServerObject);

	if (bIsLocalClient)
	{
		if (g_pGameClientShell->IsFirstPerson() ||
			g_pGameClientShell->IsSpectatorMode())
		{
			if (m_pLaser)
			{
				m_pLaser->TurnOff();
				return;
			}
		}
	}

	if (!m_pLaser)
	{
		m_pLaser = debug_new(CLaserBeam);
		if (!m_pLaser) return;
	}

	m_pLaser->TurnOn();



	// Calculate the laser pos/rot...

    LTVector vPos;
    LTRotation rRot;

	if (GetAttachmentSocketTransform(m_hServerObject, "Flash", vPos, rRot))
	{
        m_pLaser->Update(vPos, &rRot, LTTRUE);
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

	HOBJECT hCamera = g_pGameClientShell->GetCamera();
    LTVector vU, vR, vF, vTemp;
    LTRotation rRot;

    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
    LTBOOL bIsLocalClient = (m_cs.bIsPlayer && hPlayerObj == m_hServerObject);

	if ( !bIsLocalClient || !(g_pGameClientShell->IsFirstPerson() && hCamera) )
	{
        ILTPhysics* pPhysics = m_pClientDE->Physics();
        LTVector vDims;

		pPhysics->GetObjectDims(m_hServerObject, &vDims);
        g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);
        g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

		VEC_MULSCALAR(vTemp, vF, 20.0f);
		VEC_ADD(vPos, vPos, vTemp);
		vPos.y += vDims.y * 0.4f;
	}
	else
	{
        g_pLTClient->GetObjectPos(hCamera, &vPos);
        g_pLTClient->GetObjectRotation(hCamera, &rRot);
        g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);
		VEC_MULSCALAR(vTemp, vF, 20.0f);
		VEC_ADD(vPos, vPos, vTemp);

		vPos.y -= 20.0f;

		VEC_MULSCALAR(vTemp, vU, -20.0f);
		VEC_ADD(vPos, vPos, vTemp);
	}

	if (!m_pBubbles)
	{
		CreateUnderwaterFX(vPos);
	}

	if (m_pBubbles)
	{
		m_pClientDE->SetObjectPos(m_pBubbles->GetObject(), &vPos);

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

void CCharacterFX::CreateUnderwaterFX(LTVector & vPos)
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
//	ROUTINE:	CCharacterFX::UpdateSmokepuffsFX
//
//	PURPOSE:	Update the Smokepuffs fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateSmokepuffsFX()
{
	if (!m_pClientDE || !m_hServerObject) return;

    LTVector vPos(0,0,0);

	HMODELSOCKET hSocket;
	if ( LT_OK == g_pModelLT->GetSocket(m_hServerObject, "Buttcrack", hSocket) )
	{
		LTransform transform;
        if ( LT_OK == g_pModelLT->GetSocketTransform(m_hServerObject, hSocket, transform, LTTRUE) )
		{
			g_pTransLT->GetPos(transform, vPos);
		}
	}

	if (!m_pSmokepuffs)
	{
		CreateSmokepuffsFX();
	}

	if (m_pSmokepuffs)
	{
		m_pClientDE->SetObjectPos(m_pSmokepuffs->GetObject(), &vPos);

		m_pSmokepuffs->Update();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateSmokepuffsFX
//
//	PURPOSE:	Create Smokepuffs special fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateSmokepuffsFX()
{
	if (!m_pClientDE || !g_pGameClientShell || !m_hServerObject) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

    LTFLOAT fTime = m_pClientDE->GetTime();

	if (m_fNextSmokepuffTime > 0.0f && fTime < m_fNextSmokepuffTime)
	{
		return;
	}

	m_fNextSmokepuffTime = fTime + GetRandom(0.1f, 1.0f);

	SMCREATESTRUCT sm;

// ************************************************

    LTVector vPos(0,0,0);

	HMODELSOCKET hSocket;
	if ( LT_OK == g_pModelLT->GetSocket(m_hServerObject, "Buttcrack", hSocket) )
	{
		LTransform transform;
        if ( LT_OK == g_pModelLT->GetSocketTransform(m_hServerObject, hSocket, transform, LTTRUE) )
		{
			g_pTransLT->GetPos(transform, vPos);
		}
	}

// *************************************************

    sm.vPos = vPos;//LTVector(0,0,0);
//	sm.vPos.y += 25.0f;

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
	sm.nNumParticles		= GetRandom(1, 2);
    sm.bIgnoreWind          = LTFALSE;
	sm.hstrTexture			= m_pClientDE->CreateString(DEFAULT_SMOKEPUFF_TEXTURE);

	//m_pSmokepuffs =
		psfxMgr->CreateSFX(SFX_SMOKE_ID, &sm);

	m_pClientDE->FreeString(sm.hstrTexture);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::RemoveSmokepuffsFX
//
//	PURPOSE:	Remove the Smokepuffs fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::RemoveSmokepuffsFX()
{
	m_fNextSmokepuffTime = -1.0f;

	if (m_pSmokepuffs)
	{
		debug_delete(m_pSmokepuffs);
        m_pSmokepuffs = LTNULL;
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

	HMODELNODE hNode;
	if ( LT_OK == g_pModelLT->GetNode(m_hServerObject, "eyelid", hNode) )
	{
		LTransform transform;
        if ( LT_OK == g_pModelLT->GetNodeTransform(m_hServerObject, hNode, transform, LTTRUE) )
		{
			g_pTransLT->GetPos(transform, vPos);
		}
	}

	if (!m_pZzz)
	{
		CreateZzzFX();
	}

	if (m_pZzz)
	{
		m_pClientDE->SetObjectPos(m_pZzz->GetObject(), &vPos);

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

// ************************************************

    LTVector vPos(0,0,0);

	HMODELNODE hNode;
	if ( LT_OK == g_pModelLT->GetNode(m_hServerObject, "eyelid", hNode) )
	{
		LTransform transform;
        if ( LT_OK == g_pModelLT->GetNodeTransform(m_hServerObject, hNode, transform, LTTRUE) )
		{
			g_pTransLT->GetPos(transform, vPos);
		}
	}

// *************************************************

    sm.vPos = vPos;//LTVector(0,0,0);
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

	//m_pZzz =
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
			g_pTransLT->GetPos(transform, vPos);
		}
	}

	if (!m_pCigarette)
	{
		CreateCigaretteFX();
	}

	if (m_pCigarette)
	{
		m_pClientDE->SetObjectPos(m_pCigarette->GetObject(), &vPos);
		m_pCigarette->Update();
	}

	HOBJECT hObj = m_CigaretteModel.GetObject();
	if (!hObj)
	{
		// Create fx...

		m_scalecs.dwFlags = FLAG_VISIBLE | FLAG_NOLIGHT;

		// Set up fx flags...

		m_scalecs.pFilename			= "Props\\Models\\CigButt_01.abc";
		m_scalecs.pSkin				= "Props\\Skins\\CigButt.dtx";
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

        uint32 dwFlags = g_pLTClient->GetObjectFlags(hObj);
        g_pLTClient->SetObjectFlags(hObj, dwFlags | FLAG_VISIBLE);
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

// ************************************************

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

// *************************************************

    sm.vPos = vPos;//LTVector(0,0,0);
//	sm.vPos.y += 25.0f;

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

	//m_pCigarette =
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
        uint32 dwFlags = g_pLTClient->GetObjectFlags(hObj);
        g_pLTClient->SetObjectFlags(hObj, dwFlags & ~FLAG_VISIBLE);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateHeartsFX
//
//	PURPOSE:	Update the Hearts fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateHeartsFX()
{
	if (!m_pClientDE || !m_hServerObject) return;

    LTVector vPos(0,0,0);

	HMODELSOCKET hSocket;
	if ( LT_OK == g_pModelLT->GetSocket(m_hServerObject, "Heart", hSocket) )
	{
		LTransform transform;
        if ( LT_OK == g_pModelLT->GetSocketTransform(m_hServerObject, hSocket, transform, LTTRUE) )
		{
			g_pTransLT->GetPos(transform, vPos);
		}
	}

	if (!m_pHearts)
	{
		CreateHeartsFX();
	}

	if (m_pHearts)
	{
		m_pClientDE->SetObjectPos(m_pHearts->GetObject(), &vPos);

		m_pHearts->Update();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateHeartsFX
//
//	PURPOSE:	Create Hearts special fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateHeartsFX()
{
	if (!m_pClientDE || !g_pGameClientShell || !m_hServerObject) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

    LTFLOAT fTime = m_pClientDE->GetTime();

	if (m_fNextHeartTime > 0.0f && fTime < m_fNextHeartTime)
	{
		return;
	}

	m_fNextHeartTime = fTime + GetRandom(0.1f, 1.0f);

	SMCREATESTRUCT sm;

// **************************
    LTVector vPos(0,0,0);

	HMODELSOCKET hSocket;
	if ( LT_OK == g_pModelLT->GetSocket(m_hServerObject, "Heart", hSocket) )
	{
		LTransform transform;
        if ( LT_OK == g_pModelLT->GetSocketTransform(m_hServerObject, hSocket, transform, LTTRUE) )
		{
			g_pTransLT->GetPos(transform, vPos);
		}
	}
// *************************

	sm.vPos = vPos;
//	sm.vPos.y += 25.0f;

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
	sm.nNumParticles		= GetRandom(1, 2);
    sm.bIgnoreWind          = LTFALSE;
	sm.hstrTexture			= m_pClientDE->CreateString(DEFAULT_HEART_TEXTURE);

	//m_pHearts =
		psfxMgr->CreateSFX(SFX_SMOKE_ID, &sm);

	m_pClientDE->FreeString(sm.hstrTexture);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::RemoveHeartsFX
//
//	PURPOSE:	Remove the Hearts fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::RemoveHeartsFX()
{
	m_fNextHeartTime = -1.0f;

	if (m_pHearts)
	{
		debug_delete(m_pHearts);
        m_pHearts = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::RemoveLaserFX
//
//	PURPOSE:	Remove the laser fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::RemoveLaserFX()
{
	if (m_pLaser)
	{
		debug_delete(m_pLaser);
        m_pLaser = LTNULL;
	}
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

	if (stricmp(pKey, KEY_FOOTSTEP_SOUND) == 0)
	{
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
	    m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwFlags);

        if (dwFlags & USRFLG_PLAYER_MOTORCYCLE)
		{
			eModel = PPM_MOTORCYCLE;
		}
		else if (dwFlags & USRFLG_PLAYER_SNOWMOBILE)
		{
			eModel = PPM_SNOWMOBILE;
		}
    }


	CMoveMgr* pMoveMgr = g_pGameClientShell->GetMoveMgr();
	if (!pMoveMgr) return;

    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
    LTBOOL bIsLocalClient = (m_cs.bIsPlayer && hPlayerObj == hObj);

	m_eLastSurface = ST_UNKNOWN;

	if (bIsLocalClient)
	{
		if (g_pGameClientShell->IsSpectatorMode() || !pMoveMgr->CanDoFootstep())
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

    ILTPhysics* pPhysics = g_pLTClient->Physics();

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

    HOBJECT hFilterList[] = {g_pLTClient->GetClientObject(), g_pGameClientShell->GetMoveMgr()->GetObject(), LTNULL};

	if (bIsLocalClient)
	{
		iQuery.m_FilterFn  = ObjListFilterFn;
		iQuery.m_pUserData = hFilterList;
	}

	if (m_pClientDE->IntersectSegment(&iQuery, &iInfo))
	{
		if (IsMainWorld(iInfo.m_hObject) ||
			g_pLTClient->GetObjectType(iInfo.m_hObject) == OT_WORLDMODEL)
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

    LTBOOL bPlaySound = (bForceSound || (bIsLocalClient ? !g_pGameClientShell->IsFirstPerson() : LTTRUE));
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

	CMoveMgr* pMoveMgr = g_pGameClientShell->GetMoveMgr();
	if (!pMoveMgr) return;

	// Don't do movement sounds if in the menu...

	if (g_pInterfaceMgr->GetGameState() != GS_PLAYING) return;

    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
    LTBOOL bIsLocalClient = (m_cs.bIsPlayer && hPlayerObj == m_hServerObject);

	// If we're on a ladder, make sure it plays sounds...

	if (eSurface == ST_LADDER)
	{
		// Find the ladder we're in...

		LTVector vMyPos;
		g_pLTClient->GetObjectPos(m_hServerObject, &vMyPos);

		HLOCALOBJ objList[5];
		uint32 dwNum = g_pLTClient->GetPointContainers(&vMyPos, objList, 5);

		for (uint32 i=0; i < dwNum; i++)
		{
			uint16 code;
		    if (g_pLTClient->GetContainerCode(objList[i], &code))
			{
				if (CC_LADDER == (ContainerCode)code)
				{
					CVolumeBrushFX* pFX = (CVolumeBrushFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_VOLUMEBRUSH_ID, objList[i]);
					if (pFX)
					{
						if (!pFX->CanPlayMovementSounds()) return;
					}

					break;
				}
			}
		}
	}


	// Dead men don't make movement sounds...

	if (bIsLocalClient && g_pGameClientShell->IsPlayerDead()) return;

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
			nVolume /= 2;
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

    LTVector vDir = iInfo.m_Plane.m_Normal;

    LTRotation rRot;
    g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);

    LTVector vU, vR, vF;
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	// Create a sprite...

	BSCREATESTRUCT scale;
	scale.dwFlags = FLAG_VISIBLE | FLAG_ROTATEABLESPRITE | FLAG_NOLIGHT;
    g_pLTClient->AlignRotation(&(scale.rRot), &vDir, &vF);

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
                                      LTVector vStartPoint, LTVector vEndPoint,
                                      LTBOOL bMotorcycle, LTBOOL bNewTrail)
{
    if (!pTrail) return;

    LTRotation rRot;
    g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);

    LTVector vU, vR, vF;
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

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
		pls.fMaxWidth			= (bMotorcycle ? 35.0f : 128.0f);
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
		m_pClientDE->AlignRotation(&rRot, &vF, &vDir);
		pTrail->SetRot(rRot);
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
    m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwFlags);

    m_bOnVehicle = ((dwFlags & USRFLG_PLAYER_MOTORCYCLE) ||
        (dwFlags & USRFLG_PLAYER_SNOWMOBILE));

	if (m_bOnVehicle)
	{
		m_bOnMotorcycle = !!(dwFlags & USRFLG_PLAYER_MOTORCYCLE);
	}

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
		g_pLTClient->KillSound(m_hVehicleSound);
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

        ILTPhysics* pPhysics = g_pLTClient->Physics();

	    iQuery.m_Flags = IGNORE_NONSOLID | INTERSECT_OBJECTS | INTERSECT_HPOLY;
	    iQuery.m_From  = vPos;

    	iQuery.m_To	= iQuery.m_From;
	    iQuery.m_To.y -= (vDims.y + 50.0f);

	    // Don't hit ourself...

        HOBJECT hFilterList[] = {g_pLTClient->GetClientObject(), g_pGameClientShell->GetMoveMgr()->GetObject(), LTNULL};

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

            LTVector vU, vR, vF;
            g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

            LTVector vStartPoint = iInfo.m_Point;
            LTVector vEndPoint   = iInfo.m_Point + (vF * 25.0f);

			// Create trail for the front tire...

			//CreateVehicleTrail(&m_VehicleTrail1, iInfo.m_Plane.m_Normal,
            //    vStartPoint, vEndPoint, bMotorcycle, bNewTrail);

			// Create the trail for the back tire...

 			vStartPoint	= iInfo.m_Point - (vF * 50.0f);
			vEndPoint	= iInfo.m_Point - (vF * 25.0f);

			CreateVehicleTrail(&m_VehicleTrail2, iInfo.m_Plane.m_Normal,
				vStartPoint, vEndPoint, m_bOnMotorcycle, bNewTrail);
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

            const char* pSnd = m_bOnMotorcycle ? "snd\\vehicle\\motorcycle\\startup.wav" : "snd\\vehicle\\snowmobile\\startup.wav";
            g_pClientSoundMgr->PlaySoundFromObject(m_hServerObject, (char *)pSnd,
				DEFAULT_VEHICLE_RADIUS, SOUNDPRIORITY_PLAYER_HIGH);


			// Play running sound...

			uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP;

			pSnd = m_bOnMotorcycle ? "snd\\vehicle\\motorcycle\\MP_Loop.wav" : "snd\\vehicle\\snowmobile\\MP_Loop.wav";
			m_hVehicleSound = g_pClientSoundMgr->PlaySoundFromObject(m_hServerObject,
                (char *)pSnd, DEFAULT_VEHICLE_RADIUS, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
		}
		else
		{
			// Make sure the sound is playing from the correct position

			LTVector vPos;
			g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
			g_pLTClient->SetSoundPosition(m_hVehicleSound, &vPos);
		}
	}
	else if (m_hVehicleSound)
	{
		g_pLTClient->KillSound(m_hVehicleSound);
		m_hVehicleSound = LTNULL;

		// Play turn-off sound

        const char* pSnd = m_bOnMotorcycle ? "snd\\vehicle\\motorcycle\\turnoff.wav" : "snd\\vehicle\\snowmobile\\turnoff.wav";
        g_pClientSoundMgr->PlaySoundFromObject(m_hServerObject, (char *)pSnd,
			DEFAULT_VEHICLE_RADIUS, SOUNDPRIORITY_PLAYER_HIGH);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::OnServerMessage
//
//	PURPOSE:	Handle any messages from our server object...
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterFX::OnServerMessage(HMESSAGEREAD hMessage)
{
    if (!CSpecialFX::OnServerMessage(hMessage)) return LTFALSE;

    uint8 nMsgId = g_pLTClient->ReadFromMessageByte(hMessage);

	switch(nMsgId)
	{
		case CFX_CROSSHAIR_MSG:
		{
			m_cs.eCrosshairCharacterClass = (CharacterClass)g_pLTClient->ReadFromMessageByte(hMessage);
		}
		break;

		case CFX_NODECONTROL_LIP_SYNC:
		case CFX_NODECONTROL_HEAD_FOLLOW_OBJ:
		case CFX_NODECONTROL_HEAD_FOLLOW_POS:
		case CFX_NODECONTROL_SCRIPT:
		{
			m_NodeController.HandleNodeControlMessage(nMsgId, hMessage);
		}
		break;

		case CFX_DIALOGUE_MSG:
		{
			HandleDialogueMsg(hMessage);
		}
		break;

		case CFX_TAUNT_MSG:
		{
			HandleTauntMsg(hMessage);
		}
		break;

		case CFX_RESET_TRACKER:
		{
            uint8 iTracker = g_pLTClient->ReadFromMessageByte(hMessage);
			if ( iTracker == 0 )
			{
                g_pLTClient->ResetModelAnimation(m_hServerObject);
			}
			else
			{
				g_pModelLT->ResetAnim(&m_aAnimTrackers[iTracker-1]);
			}
		}
		break;

		case CFX_STEALTH_MSG:
		{
            m_cs.fStealthPercent = g_pLTClient->ReadFromMessageFloat(hMessage);
		}
		break;

		case CFX_CLIENTID_MSG:
		{
            m_cs.nClientID = g_pLTClient->ReadFromMessageByte(hMessage);
		}
		break;

		case CFX_CHAT_MSG:
		{
            m_cs.SetChatting((LTBOOL)g_pLTClient->ReadFromMessageByte(hMessage));
		}
		break;

		case CFX_SMOKEPUFF_CREATE_MSG:
		{
			_ASSERT(!(m_cs.byFXFlags & CHARCREATESTRUCT::eSmokepuffs));
			m_cs.byFXFlags |= CHARCREATESTRUCT::eSmokepuffs;
		}
		break;

		case CFX_SMOKEPUFF_DESTROY_MSG:
		{
			_ASSERT(m_cs.byFXFlags & CHARCREATESTRUCT::eSmokepuffs);
			m_cs.byFXFlags &= ~CHARCREATESTRUCT::eSmokepuffs;
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

		case CFX_HEART_CREATE_MSG:
		{
			_ASSERT(!(m_cs.byFXFlags & CHARCREATESTRUCT::eHearts));
			m_cs.byFXFlags |= CHARCREATESTRUCT::eHearts;
		}
		break;

		case CFX_HEART_DESTROY_MSG:
		{
			_ASSERT(m_cs.byFXFlags & CHARCREATESTRUCT::eHearts);
			m_cs.byFXFlags &= ~CHARCREATESTRUCT::eHearts;
		}
		break;

		case CFX_ZIPCORD_MSG:
		{
			HandleZipcordMsg(hMessage);
		}
		break;

		case CFX_ALLFX_MSG:
		{
			// Re-init our data...

            m_cs.Read(g_pLTClient, hMessage);

		    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
			if (m_cs.bIsPlayer && hPlayerObj == m_hServerObject)
			{
				InitLocalPlayer();
			}
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

HLTSOUND CCharacterFX::PlayLipSyncSound(char* szSound, LTFLOAT fRadius, LTBOOL & bSubtitle)
{
	bSubtitle = LTFALSE;

    if (!szSound || !szSound[0] || fRadius <= 0.0f) return LTNULL;

    uint32 dwFlags = PLAYSOUND_GETHANDLE;

	SoundPriority ePriority = SOUNDPRIORITY_AI_HIGH;
	if (m_cs.bIsPlayer)
	{
		ePriority = SOUNDPRIORITY_PLAYER_HIGH;

        LTBOOL bIsLocalClient = (g_pLTClient->GetClientObject() == m_hServerObject);
		dwFlags |= bIsLocalClient ? PLAYSOUND_CLIENTLOCAL : 0;
	}


	// Show subtitles? (Dialogue sounds only)...

	LTVector vPos;
	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

	uint32 nStringId = g_pClientSoundMgr->GetSoundIdFromFilename(szSound);

	if (nStringId)
	{
		CString csSound = g_pClientSoundMgr->GetSoundFilenameFromId("Dialogue", nStringId);

		// Okay, make sure that id we got really is a dialogue sound...

		if (!csSound.IsEmpty() && (strcmpi(szSound, csSound) == 0))
		{
			// If we're in a cinematic use the cinematic radius, else
			// use the conversation radius...

			if (g_pGameClientShell->IsUsingExternalCamera())
			{
				fRadius = g_vtDialogueCinematicSoundRadius.GetFloat();

				// Force subtitle to be shown...
				vPos.Init();

				// If the camera isn't the listener, play the sound in
				// the client's head...

				if (!g_pGameClientShell->IsCameraListener())
				{
					dwFlags |= PLAYSOUND_CLIENTLOCAL;
				}
			}

			if (dwFlags & PLAYSOUND_CLIENTLOCAL)
			{
				// Force subtitle to be shown...
				vPos.Init();
			}

			// Unload the dialogue sound after it is played...
			dwFlags |= PLAYSOUND_ONCE;

			nStringId += SUBTITLE_STRINGID_OFFSET;

			bSubtitle = LTTRUE;
		}
	}

	HLTSOUND hSound = g_pClientSoundMgr->PlaySoundFromObject(m_hServerObject,
		szSound, fRadius, ePriority, dwFlags);

	if (bSubtitle && hSound)
	{
		LTFLOAT fDuration = -1.0f;
		g_pLTClient->GetSoundDuration(hSound, &fDuration);
		g_pInterfaceMgr->ShowSubtitle(nStringId, vPos, fRadius, fDuration);
	}

	return hSound;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::PlayLipSyncSound
//
//	PURPOSE:	Play a lip synced sound.
//
// ----------------------------------------------------------------------- //

void CCharacterFX::PlayTaunt(uint32 nTauntId, LTBOOL bForce)
{
	if (!m_cs.bIsPlayer) return;
	if (GetConsoleInt("IgnoreTaunts", 0)) return;

	LTBOOL bIsLocalClient = (g_pLTClient->GetClientObject() == m_hServerObject);
	if (bIsLocalClient && !bForce) return;

	CString csSound = g_pClientSoundMgr->GetSoundFilenameFromId("Dialogue", nTauntId);
	if (csSound.IsEmpty()) return;

	HSTRING hstrSnd = g_pLTClient->CreateString((char*)(LPCSTR)csSound);
	if (hstrSnd)
	{
		m_NodeController.HandleNodeConrolLipSync(hstrSnd, DEFAULT_TAUNT_RADIUS);
	}
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
	if (m_eLastSurface == ST_UNKNOWN) return;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eLastSurface);
	if (!pSurf || !pSurf->bShowBreath) return;

	// If it is time to do a breath, do it...

	if (m_BreathTimer.Stopped())
	{
		HMODELSOCKET hSocket;
		if (g_pModelLT->GetSocket(m_hServerObject, "Chin", hSocket) == LT_OK)
		{
			LTransform transform;
            if (g_pModelLT->GetSocketTransform(m_hServerObject, hSocket, transform, LTTRUE) == LT_OK)
			{
                LTVector vPos;
                LTRotation rRot;
				g_pTransLT->Get(transform, vPos, rRot);

                LTVector vU, vR, vF;
                g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);
                g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

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

				CString str = g_pClientButeMgr->GetBreathFXAttributeString("Sprite");
                sm.hstrTexture = g_pLTClient->CreateString((char *)(LPCSTR)str);

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

				m_BreathTimer.Start(g_vtBreathTime.GetFloat() * GetRandom(0.75f, 1.25f));
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::HandleZipcordMsg
//
//	PURPOSE:	Update the zipcord fx as appropriate
//
// ----------------------------------------------------------------------- //

void CCharacterFX::HandleZipcordMsg(HMESSAGEREAD hMessage)
{
    uint8 nZipState = g_pLTClient->ReadFromMessageByte(hMessage);

	switch (nZipState)
	{
		case ZC_ON :
		{
            LTVector vEndPos;
            g_pLTClient->ReadFromMessageVector(hMessage, &vEndPos);

			TurnOnZipCord(vEndPos);
		}
		break;

		case ZC_OFF :
		{
			TurnOffZipCord();
		}
		break;

		default :
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::TurnOnZipCord
//
//	PURPOSE:	Turn on the zip cord
//
// ----------------------------------------------------------------------- //

void CCharacterFX::TurnOnZipCord(LTVector vEndPoint)
{
	m_vZipCordEndPoint = vEndPoint;
    m_bDrawZipCord = LTTRUE;

    uint32 dwFlags = m_ZipCord.GetFlags();
	m_ZipCord.SetFlags(dwFlags | FLAG_VISIBLE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::TurnOffZipCord
//
//	PURPOSE:	Turn off the zip cord
//
// ----------------------------------------------------------------------- //

void CCharacterFX::TurnOffZipCord()
{
	m_vZipCordEndPoint.Init();
    m_bDrawZipCord = LTFALSE;

    uint32 dwFlags = m_ZipCord.GetFlags();
	m_ZipCord.SetFlags(dwFlags & ~FLAG_VISIBLE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateZipCordFX
//
//	PURPOSE:	Update the zipcord (from us to the end point)
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateZipCordFX()
{
	if (!m_bDrawZipCord) return;

	// Calculate start point (1st person, or 3rd person...)

    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
    LTBOOL bIsLocalClient = (m_cs.bIsPlayer && hPlayerObj == m_hServerObject);

    LTBOOL bFirstPerson = LTFALSE;

	if (bIsLocalClient)
	{
		bFirstPerson = g_pGameClientShell->IsFirstPerson();
	}

	// Use the player-view weapon postion if first person, else use
	// the hand-held weapon position...

	PLFXCREATESTRUCT pls;
	pls.vEndPos	= m_vZipCordEndPoint;

    g_pLTClient->GetObjectPos(m_hServerObject, &(pls.vStartPos));

	if (bFirstPerson)
	{
		HOBJECT hModelObject = g_pGameClientShell->GetWeaponModel()->GetHandle();

		if (hModelObject)
		{
			HMODELSOCKET hSocket;
			if (LT_OK == g_pModelLT->GetSocket(hModelObject, "ZipCord", hSocket))
			{
				LTransform transform;
                if (LT_OK == g_pModelLT->GetSocketTransform(hModelObject, hSocket, transform, LTTRUE))
				{
					g_pTransLT->GetPos(transform, pls.vStartPos);

					// Need to add on the camera pos, because the model is
					// drawn relative to the camera...

					HOBJECT hCamera = g_pGameClientShell->GetCamera();
                    LTVector vPos;
                    g_pLTClient->GetObjectPos(hCamera, &vPos);
					pls.vStartPos += vPos;

                    // g_pLTClient->CPrint("Pos %.2f, %.2f, %.2f", VEC_EXPAND(pls.vStartPos));

					// Add the player-view weapon model pos to the start point to get
					// the camera-relative pos.

                    //LTVector vPos;
                    //g_pLTClient->GetObjectPos(hModelObject, &vPos);

					//pls.vStartPos += vPos;

					// Need to subtract off the camera pos to get the
					// camera-relative pos for this...

					//HOBJECT hCamera = g_pGameClientShell->GetCamera();
                    //g_pLTClient->GetObjectPos(hCamera, &vPos);
					//pls.vEndPos -= vPos;
				}
			}
		}
	}
	else
	{
		// Use the hand-held flash position...

        LTRotation rRot;
		GetAttachmentSocketTransform(m_hServerObject, "Flash", pls.vStartPos, rRot);
	}

	pls.pTexture			= "sfx\\test\\zipcord.dtx";
    pls.dwTexAddr           = LTTEXADDR_CLAMP;
    pls.vInnerColorStart    = LTVector(255, 255, 255);
	pls.vInnerColorEnd		= pls.vInnerColorStart;
	pls.vOuterColorStart	= pls.vInnerColorStart;
	pls.vOuterColorEnd		= pls.vInnerColorStart;
	pls.fAlphaStart			= 1.0f;
	pls.fAlphaEnd			= 1.0f;
	pls.fMinWidth			= 0.0f;
	pls.fMaxWidth			= bFirstPerson ? g_vtZipCord1stWidth.GetFloat() : g_vtZipCord3rdWidth.GetFloat();
	pls.fLifeTime			= 10000000.0f;
	pls.fAlphaLifeTime		= 10000000.0f;
    pls.bAdditive           = LTFALSE;
    pls.bMultiply           = LTFALSE;
	pls.nWidthStyle			= PLWS_CONSTANT;
    pls.bUseObjectRotation  = LTFALSE;
	pls.nNumSegments		= 2;
	pls.fMinDistMult		= 1.0f;
	pls.fMaxDistMult		= 1.0f;
	pls.fPerturb			= 0.0f;
    pls.bAlignFlat          = LTTRUE;
    pls.bNoZ                = LTTRUE;

	// Update the zipcord...

	if (m_ZipCord.HasBeenDrawn())
	{
		m_ZipCord.SetPos(pls.vStartPos);

		// Hide the zipcord in portals if 1st person...Also set flag really
		// close to true...

        uint32 dwFlags2, dwFlags;

		dwFlags = m_ZipCord.GetFlags();
		dwFlags2 = m_ZipCord.GetFlags2();

		if (bFirstPerson)
		{
			//dwFlags |= FLAG_REALLYCLOSE;
			dwFlags2 |= FLAG2_PORTALINVISIBLE;
		}
		else
		{
			//dwFlags &= ~FLAG_REALLYCLOSE;
			dwFlags2 &= ~FLAG2_PORTALINVISIBLE;
		}

		m_ZipCord.SetFlags(dwFlags);
		m_ZipCord.SetFlags2(dwFlags2);

		m_ZipCord.ReInit(&pls);
	}
	else
	{
		// First time, create it...

		m_ZipCord.Init(&pls);
        m_ZipCord.CreateObject(g_pLTClient);
	}


	m_ZipCord.Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateMarkerFX
//
//	PURPOSE:	Update the fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateMarkerFX()
{
	if (!m_pClientDE || !m_hServerObject) return;

    uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hServerObject);
	if (!(dwFlags & FLAG_VISIBLE))
	{
		RemoveMarkerFX();
		return;
	}

    LTFLOAT fTime = m_pClientDE->GetTime();

	static const LTFLOAT s_fNextTimeDelta = 1.0f;

	if (fTime > m_fNextMarkerTime)
	{
		m_fNextMarkerTime = fTime + s_fNextTimeDelta;

		if (NextMarkerState())
		{
			switch (m_eMarkerState)
			{
			case MS_TEAM:
				CreateTeamFX();
				break;
			case MS_CHAT:
				CreateChatFX();
				break;
			case MS_UNKNOWN:
				RemoveMarkerFX();
				return;
			}
		}
	}

    LTVector vU, vR, vF, vTemp, vDims, vPos;
    LTRotation rRot;

    ILTPhysics* pPhysics = m_pClientDE->Physics();

	m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
	pPhysics->GetObjectDims(m_hServerObject, &vDims);
	vPos.y += (vDims.y + 20.0f);


	if (m_hMarker)
	{
		m_pClientDE->SetObjectPos(m_hMarker, &vPos);
//        g_pLTClient->RotateAroundAxis(&rRot, &vU, g_pGameClientShell->GetFrameTime());
//		  g_pLTClient->SetObjectRotation(m_pTeamMarker->GetObject(),&rRot);

	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateTeamFX
//
//	PURPOSE:	Create Team special fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateTeamFX()
{
	if (!m_pClientDE || !g_pGameClientShell || !m_hServerObject) return;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	CString str;
	CLIENT_INFO* pLocalInfo = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
	if (!pLocalInfo)
		return;
	if (pLocalInfo->team == 1)
		str = g_pClientButeMgr->GetSpecialFXAttributeString("Team1Sprite");
	else
		str = g_pClientButeMgr->GetSpecialFXAttributeString("Team2Sprite");

	LTFLOAT fScaleMult = g_pClientButeMgr->GetSpecialFXAttributeFloat("TeamScale");
	LTFLOAT fAlpha = g_pClientButeMgr->GetSpecialFXAttributeFloat("TeamAlpha");

	LTVector	vScale(1.0f,1.0f,1.0f);

	VEC_MULSCALAR(vScale, vScale, fScaleMult);

	SAFE_STRCPY(createStruct.m_Filename, (char *)(LPCSTR)str);
	createStruct.m_ObjectType	= OT_SPRITE;
	createStruct.m_Flags		= FLAG_VISIBLE | FLAG_FOGDISABLE | FLAG_NOLIGHT;

	if (m_hMarker)
	{
		m_pClientDE->Common()->SetObjectFilenames(m_hMarker, &createStruct);
		m_pClientDE->SetObjectFlags(m_hMarker, createStruct.m_Flags);

	}
	else
	{
		m_hMarker = m_pClientDE->CreateObject(&createStruct);
		if (!m_hMarker) return;
	}

	m_pClientDE->SetObjectScale(m_hMarker, &vScale);
	m_pClientDE->SetObjectColor(m_hMarker, 1.0f, 1.0f, 1.0f, fAlpha);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateChatFX
//
//	PURPOSE:	Create chat special fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateChatFX()
{
	if (!m_pClientDE || !g_pGameClientShell || !m_hServerObject) return;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	CString str = g_pClientButeMgr->GetSpecialFXAttributeString("ChatSprite");
	LTFLOAT fScaleMult = g_pClientButeMgr->GetSpecialFXAttributeFloat("ChatScale");
	LTFLOAT fAlpha = g_pClientButeMgr->GetSpecialFXAttributeFloat("ChatAlpha");

	LTVector	vScale(1.0f,1.0f,1.0f);

	VEC_MULSCALAR(vScale, vScale, fScaleMult);

	SAFE_STRCPY(createStruct.m_Filename, (char *)(LPCSTR)str);
	createStruct.m_ObjectType	= OT_SPRITE;
	createStruct.m_Flags		= FLAG_VISIBLE | FLAG_FOGDISABLE | FLAG_NOLIGHT;

	if (m_hMarker)
	{
		m_pClientDE->Common()->SetObjectFilenames(m_hMarker, &createStruct);
		m_pClientDE->SetObjectFlags(m_hMarker, createStruct.m_Flags);

	}
	else
	{
		m_hMarker = m_pClientDE->CreateObject(&createStruct);
		if (!m_hMarker) return;
	}

	m_pClientDE->SetObjectScale(m_hMarker, &vScale);
	m_pClientDE->SetObjectColor(m_hMarker, 1.0f, 1.0f, 1.0f, fAlpha);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::RemoveMarkerFX
//
//	PURPOSE:	Remove the marker fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::RemoveMarkerFX()
{
	if (!m_hMarker) return;

	g_pLTClient->DeleteObject(m_hMarker);
	m_hMarker = LTNULL;
	m_eMarkerState = MS_UNKNOWN;

}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::NextMarkerState
//
//	PURPOSE:	Find next valid marker state, return LTTRUE if it changed
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterFX::NextMarkerState()
{
	CClientInfoMgr *pClientMgr = g_pInterfaceMgr->GetClientInfoMgr();
	if (!pClientMgr) return LTFALSE;

	CLIENT_INFO* pLocalInfo = pClientMgr->GetLocalClient();
	CLIENT_INFO* pInfo = pClientMgr->GetClientByID(m_cs.nClientID);

	int nState = (int)m_eMarkerState;
	// if we don't have any client info, we'd better be in a single player game
	if (!pInfo ||  !pLocalInfo)
	{
		if (g_pGameClientShell->GetGameType() == SINGLE)
		{
			m_eMarkerState = MS_UNKNOWN;

			return (nState != (int)m_eMarkerState);
		}
		else
		{
			_ASSERT(0);
			return LTFALSE;
		}
	}
	if (pInfo->nID == pLocalInfo->nID) return LTFALSE;

	//if we were marking a teammate, and we shouldn't any more, clear the state
	if (nState == MS_TEAM && pInfo->team != pLocalInfo->team)
	{
		nState = MS_UNKNOWN;
	}
	if (nState == MS_CHAT && !m_cs.IsChatting())
	{
		nState = MS_UNKNOWN;
	}


	//cycle through the states to find the next valid one
	int nStartState = nState;
	LTBOOL bDone = LTFALSE;
	while (!bDone)
	{
		nState++;
		switch (nState)
		{
		case MS_NUM_STATES:
			// at the end of the list so start at the beginning
			nState = MS_UNKNOWN;
			break;
		case MS_TEAM:
			// if we're in coop and this is a teammate, show the marker
			if (g_pGameClientShell->GetGameType() == COOPERATIVE_ASSAULT &&
				(pInfo->team == pLocalInfo->team))
			{
				bDone = LTTRUE;

			}
			break;

		case MS_CHAT:
			//hey, I'm talking here...
			if (m_cs.IsChatting())
				bDone = LTTRUE;
			break;
		}
		//back where we started, so stop looking
		if (nState == nStartState)
			bDone = LTTRUE;
	}


	if (nState != (int)m_eMarkerState)
	{
		m_eMarkerState = (eMarkerState)nState;
		return LTTRUE;
	}

	return LTFALSE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::ResetMarkerState
//
//	PURPOSE:	Find next valid marker state, return LTTRUE if it changed
//
// ----------------------------------------------------------------------- //

void CCharacterFX::ResetMarkerState()
{
	m_fNextMarkerTime = 0.0f;
	UpdateMarkerFX();
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
				g_pLTClient->CPrint("Adding Ding Sound at Time %.2f", m_fNextDingTime[i]);
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
		// check for needed overlays
		ModelStyle ms = GetModelStyle();
		if (_stricmp(g_pModelButeMgr->GetStyleName(ms),"Undercover2") == 0)
		{
			g_pInterfaceMgr->BeginSpacesuit();
		}

		// Set the first-person camera offset...

		g_pGameClientShell->GetPlayerCamera()->SetFirstPersonOffset(GetPlayerHeadOffset(g_pModelButeMgr, ms));

		// Update the player-view weapon model so that it uses the correct
		// textures based on the model style...

		g_pGameClientShell->GetWeaponModel()->SetupModel();
	}
}