// ----------------------------------------------------------------------- //
//
// MODULE  : Character.cpp
//
// PURPOSE : Base class for player and AI
//
// CREATED : 10/6/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Character.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "stdio.h"
#include "Body.h"
#include "VolumeBrush.h"
#include "Spawner.h"
#include "SurfaceFunctions.h"
#include "CharacterMgr.h"
#include "CVarTrack.h"
#include "SoundMgr.h"
#include "iltmodel.h"
#include "ilttransform.h"
#include "ObjectMsgs.h"
#include "DialogueWindow.h"
#include "SFXMsgIds.h"
#include "Attachments.h"
#include "SurfaceMgr.h"
#include "Animator.h"
#include "MsgIDs.h"
#include "GameServerShell.h"
#include "AIVolumeMgr.h"
#include "CharacterHitBox.h"
#include "Camera.h"

BEGIN_CLASS(CCharacter)
	ADD_DESTRUCTIBLE_AGGREGATE(PF_GROUP1, 0)
	ADD_ATTACHMENTS_AGGREGATE()
	ADD_REALPROP(HitPoints, -1.0f)
	ADD_REALPROP(ArmorPoints, -1.0f)
    ADD_BOOLPROP(MoveToFloor, LTTRUE)
	ADD_REALPROP(ShowDeadBody, -1)
	ADD_STRINGPROP(SpawnItem, "")
	ADD_STRINGPROP(HeadExtension, "_head")
END_CLASS_DEFAULT_FLAGS(CCharacter, GameBase, NULL, NULL, CF_HIDDEN)

#define KEY_FOOTSTEP_SOUND		"FOOTSTEP_KEY"
#define KEY_SET_DIMS			"SETDIMS"
#define KEY_MOVE				"MOVE"
#define KEY_PLAYSOUND			"PLAYSOUND"
#define KEY_COMMAND				"CMD"

#define TRIGGER_PLAY_SOUND		"PLAYSOUND"
#define TRIGGER_TELEPORT		"TELEPORT"

#define DEFAULT_SOUND_RADIUS		1000.0f
#define FOOTSTEP_SOUND_RADIUS		1000.0f
#define DEFAULT_LADDER_VEL			400.0f
#define DEFAULT_SWIM_VEL			175.0f
#define DEFAULT_RUN_VEL				100.0f
#define DEFAULT_WALK_VEL			60.0f
#define DEFAULT_ROLL_VEL			50.0f
#define DEFAULT_JUMP_VEL			50.0f
#define DEFAULT_MOVE_ACCEL			3000.0f

#define DIMS_EPSILON				0.5f
#define FALL_LANDING_TIME			0.5f

static CBankedList<CharFootprintInfo> s_bankCharFootprintInfo;

static CVarTrack g_VolumeDebugTrack;
static CVarTrack s_BodyStickAngle;

CVarTrack g_BodyStickDist;
CVarTrack g_BodyStateTimeout;

// Globals (save space) used for parsing messages (used in sub classes as well)...
// g_pCommandPos is global to make sure the command position is correctly
// updated in multiple calls to Parse()

char g_tokenSpace[PARSE_MAXTOKENS*PARSE_MAXTOKENSIZE];
char *g_pTokens[PARSE_MAXTOKENS];
char *g_pCommandPos;

extern CGameServerShell* g_pGameServerShell;

int32 CCharacter::sm_cAISnds = 0;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CCharacter()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CCharacter::CCharacter() : GameBase(OT_MODEL)
{
	AddAggregate(&m_damage);
	AddAggregate(&m_editable);

	m_bInitializedAnimation		= LTFALSE;

	m_bBlink					= LTFALSE;

	m_bShortRecoil				= LTFALSE;
	m_bShortRecoiling			= LTFALSE;

	m_cc						= UNKNOWN;
	m_ccCrosshair				= UNKNOWN;

	m_dwFlags					= FLAG_STAIRSTEP | FLAG_SHADOW | FLAG_TOUCH_NOTIFY | FLAG_SOLID | FLAG_GRAVITY |
								  FLAG_MODELKEYS | FLAG_RAYHIT | FLAG_VISIBLE | FLAG_ANIMTRANSITION;

	m_fLadderVel				= DEFAULT_LADDER_VEL;
	m_fSwimVel					= DEFAULT_SWIM_VEL;
	m_fRunVel					= DEFAULT_RUN_VEL;
	m_fWalkVel					= DEFAULT_WALK_VEL;
	m_fRollVel					= DEFAULT_ROLL_VEL;
	m_fJumpVel					= DEFAULT_JUMP_VEL;
	m_fBaseMoveAccel			= DEFAULT_MOVE_ACCEL;
	m_eModelNodeLastHit			= eModelNodeInvalid;
    m_bUsingHitDetection        = LTTRUE;
	m_fBodyLifetime					= -1.0f;

	m_fSoundRadius				= DEFAULT_SOUND_RADIUS;
	m_eSoundPriority			= SOUNDPRIORITY_AI_HIGH;

	m_byFXFlags					= 0;

    m_bRolling                  = LTFALSE;
    m_bPivoting                 = LTFALSE;
    m_bOnGround                 = LTTRUE;
	m_eStandingOnSurface		= ST_UNKNOWN;
    m_bAllowRun                 = LTTRUE;
    m_bAllowMovement            = LTTRUE;
    m_bSpectatorMode            = LTFALSE;
	m_eContainerCode			= CC_NO_CONTAINER;
	m_eLastContainerCode		= CC_NO_CONTAINER;
    m_bBodyInLiquid             = LTFALSE;
    m_bBodyWasInLiquid          = LTFALSE;
    m_bBodyOnLadder             = LTFALSE;
    m_bLeftFoot                 = LTTRUE;
    m_bPlayingTextDialogue      = LTFALSE;

	m_fLastPainTime				= -(float)INT_MAX;
	m_fLastPainVolume			= 0.0f;

	VEC_INIT(m_vOldCharacterColor);
	m_fOldCharacterAlpha		= 1.0f;
    m_bCharacterHadShadow       = LTFALSE;

    m_bMoveToFloor              = LTTRUE;

    m_bCanPlayDialogSound       = LTTRUE;
    m_bCanDamageBody            = LTTRUE;

    m_hstrSpawnItem             = LTNULL;

	m_hstrHeadExtension			= LTNULL;

    m_pAttachments              = LTNULL;

	// Debug bounding box...

	m_pHandName					= "GUNHAND";

    m_hCurDlgSnd                = LTNULL;
	m_eCurDlgSndType			= CST_NONE;

    m_bStartedDeath             = LTFALSE;
	m_eDeathType				= CD_NORMAL;

	m_eModelId					= eModelIdInvalid;
	m_eModelStyle				= eModelStyleInvalid;
	m_eModelSkeleton			= eModelSkeletonInvalid;

	m_fDefaultHitPts			= -1.0f;
	m_fDefaultArmor				= -1.0f;
	m_fMoveMultiplier			= 1.0f;
	m_fJumpMultiplier			= 1.0f;

	m_iLastVolume				= -1;
    m_vLastVolumePos            = LTVector(0,0,0);

    m_hHitBox                   = LTNULL;

	m_cSpears					= 0;

	m_bWallStick				= LTFALSE;

    m_pAnimator                 = LTNULL;

	m_cActive					= 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::Reset()
//
//	PURPOSE:	Reset (after death)
//
// ----------------------------------------------------------------------- //

void CCharacter::Reset()
{
    m_bStartedDeath     = LTFALSE;

	KillDlgSnd();

	// Since we were dead, we need to reset our solid flag...

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	dwFlags |= FLAG_SOLID;
	g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

	// Also update our hit box in case we have moved...

	UpdateHitBox();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::~CCharacter()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CCharacter::~CCharacter()
{
	// This should be made an automatic data member to localize this
	// object's memory allocation...

	DestroyAttachments();

	KillDlgSnd();

	FREE_HSTRING(m_hstrHeadExtension);
	FREE_HSTRING(m_hstrSpawnItem);

	if (m_hHitBox)
	{
		g_pLTServer->RemoveObject(m_hHitBox);
	}

	if ( m_LastFireInfo.hObject )
	{
		g_pLTServer->BreakInterObjectLink(m_hObject, m_LastFireInfo.hObject);
		m_LastFireInfo.hObject = LTNULL;
	}

	CharFootprintInfo** ppFootprint = m_listFootprints.GetItem(TLIT_FIRST);
	while ( ppFootprint && *ppFootprint )
	{
		CharFootprintInfo* pFootprint = *ppFootprint;
		ppFootprint = m_listFootprints.GetItem(TLIT_NEXT);

		s_bankCharFootprintInfo.Delete(pFootprint);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CCharacter::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
        case MID_ACTIVATING:
		{
			m_cActive++;
			g_pCharacterMgr->Add(this);

			if ( m_cActive != 1 )
			{
				g_pLTServer->CPrint("Active count out of sync!!!!!!");
			}
		}
		break;

		case MID_DEACTIVATING:
		{
			m_cActive--;
			g_pCharacterMgr->Remove(this);

			if ( m_cActive != 0 )
			{
				g_pLTServer->CPrint("Active count out of sync!!!!!!");
			}
		}
		break;

		case MID_PARENTATTACHMENTREMOVED:
		{
			RemoveObject();
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_MODELSTRINGKEY:
		{
			HandleModelString((ArgList*)pData);
		}
		break;

		case MID_LINKBROKEN :
		{
			if ( (HOBJECT)pData == m_LastFireInfo.hObject )
			{
				m_LastFireInfo.hObject = LTNULL;
			}

			for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
			{
				HOBJECT hSpear = (HOBJECT)pData;
				if ( m_aSpears[iSpear].hObject == hSpear )
				{
					HATTACHMENT hAttachment;
					if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hSpear, &hAttachment) )
					{
						if ( LT_OK == g_pLTServer->RemoveAttachment(hAttachment) )
						{
						}
					}

					m_aSpears[iSpear].hObject = LTNULL;
				}
			}

		}
		break;

		case MID_PRECREATE:
		{
			CreateAttachments();
			if ( m_pAttachments )
			{
				AddAggregate(m_pAttachments);
			}

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;

			if (pStruct)
			{
				if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
				{
					ReadProp(pStruct);
				}
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
            uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);
			InitialUpdate((int)fData);
			CacheFiles();
			return dwRet;
		}
		break;

		case MID_SAVEOBJECT:
		{
			// Let aggregates go first...

            uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			Save((HMESSAGEWRITE)pData);

			return dwRet;
		}
		break;

		case MID_LOADOBJECT:
		{
			// Let aggregates go first...

            uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			Load((HMESSAGEREAD)pData);

			return dwRet;
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ObjectMessageFn()
//
//	PURPOSE:	Handler for object to object messages.
//
// --------------------------------------------------------------------------- //

uint32 CCharacter::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			ProcessTriggerMsg(szMsg);
		}
		break;

		case MID_DAMAGE:
		{
            uint32 dwRet = GameBase::ObjectMessageFn(hSender, messageID, hRead);
			ProcessDamageMsg(hRead);
			return dwRet;
		}
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;
    if (!pStruct) return LTFALSE;

    if ( g_pLTServer->GetPropGeneric( "MoveToFloor", &genProp ) == LT_OK )
	{
		m_bMoveToFloor = genProp.m_Bool;
	}
    if ( g_pLTServer->GetPropGeneric( "ShowDeadBody", &genProp ) == LT_OK )
	{
		m_fBodyLifetime = genProp.m_Float;
	}

    if ( g_pLTServer->GetPropGeneric( "SpawnItem", &genProp ) == LT_OK )
	{
		if ( genProp.m_String[0] )
			m_hstrSpawnItem = g_pLTServer->CreateString( genProp.m_String );
	}

    if ( g_pLTServer->GetPropGeneric( "HitPoints", &genProp ) == LT_OK )
	{
		m_fDefaultHitPts = genProp.m_Float;
	}

    if ( g_pLTServer->GetPropGeneric( "ArmorPoints", &genProp ) == LT_OK )
	{
		m_fDefaultArmor = genProp.m_Float;
	}

    if ( g_pLTServer->GetPropGeneric( "HeadExtension", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrHeadExtension = g_pLTServer->CreateString( genProp.m_String );

    return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ProcessTriggerMsg()
//
//	PURPOSE:	Process a trigger message messages.
//
// --------------------------------------------------------------------------- //

LTBOOL CCharacter::ProcessTriggerMsg(const char* szMsg)
{
    if (!szMsg) return LTFALSE;

	// ILTServer::Parse does not destroy pCommand, so this is safe
	char* pCommand = (char*)szMsg;

    LTBOOL bMore = LTTRUE;
	while (bMore)
	{
		int nArgs;
		bMore = g_pLTServer->Parse(pCommand, &g_pCommandPos, g_tokenSpace, g_pTokens, &nArgs);
	
		if ( !ProcessCommand(g_pTokens, nArgs, g_pCommandPos) )
		{
#ifndef _FINAL
			g_pLTServer->CPrint("Unrecognized command (\"%s\")", g_pTokens[0]);
#endif
		}

		pCommand = g_pCommandPos;
	}

    return LTFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ProcessCommand()
//
//	PURPOSE:	Process a command
//
// --------------------------------------------------------------------------- //

LTBOOL CCharacter::ProcessCommand(char** pTokens, int nArgs, char* pNextCommand)
{
    if (!pTokens || !pTokens[0] || nArgs < 1) return LTFALSE;

	if (stricmp(TRIGGER_PLAY_SOUND, pTokens[0]) == 0 && nArgs > 1)
	{
		// Get sound name from message...

		char* pSoundName = pTokens[1];

		if (pSoundName)
		{
			PlayDialogSound(pSoundName, CST_EXCLAMATION);
            return LTTRUE;
		}
	}
	else if (stricmp(TRIGGER_TELEPORT, pTokens[0]) == 0)
	{
		if ( !IsVector(pTokens[1]) )
		{
            TeleportPoint* pTeleportPt = LTNULL;

			HOBJECT hObject;
			if ( LT_OK == FindNamedObject(pTokens[1], hObject) )
			{
                if ( !IsKindOf(hObject, "TeleportPoint") ) return LTTRUE;

				pTeleportPt = (TeleportPoint*) g_pLTServer->HandleToObject(hObject);
				HandleTeleport(pTeleportPt);
		        return LTTRUE;
			}
		}
	}
	else if ( !_stricmp(pTokens[0], "REMOVE") )
	{
		RemoveObject();
        return LTTRUE;
	}
	else if ( !_stricmp("ATTACH", pTokens[0]) )
	{
		m_pAttachments->Attach(pTokens[1], pTokens[2]);
		HandleAttach();
        return LTTRUE;
	}
	else if ( !_stricmp("DETACH", pTokens[0]) )
	{
		m_pAttachments->Detach(pTokens[1]);
		HandleDetach();
        return LTTRUE;
	}
	else if ( !_stricmp("GADGET", pTokens[0]) )
	{
		HandleGadget(atoi(pTokens[1]));
        return LTTRUE;
	}
	else if ( !_stricmp("CANDAMAGE", pTokens[0]) )
	{
		m_damage.SetCanDamage(IsTrueChar(*pTokens[1]));
		return LTTRUE;
	}
	else if ( !_stricmp("CROSSHAIR", pTokens[0]) )
	{
		if ( !_stricmp("GOOD", pTokens[1]) )
		{
			m_ccCrosshair = GOOD;
		}
		else if ( !_stricmp("BAD", pTokens[1]) )
		{
			m_ccCrosshair = BAD;
		}
		else if ( !_stricmp("NEUTRAL", pTokens[1]) )
		{
			m_ccCrosshair = NEUTRAL;
		}
		else if ( !_stricmp("UNKNOWN", pTokens[1]) )
		{
			m_ccCrosshair = UNKNOWN;
		}

		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
		g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
		g_pLTServer->WriteToMessageByte(hMessage, CFX_CROSSHAIR_MSG);
		g_pLTServer->WriteToMessageByte(hMessage, m_ccCrosshair == UNKNOWN ? m_cc : m_ccCrosshair);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

		CreateSpecialFX();
	}

    return LTFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ProcessDamageMsg()
//
//	PURPOSE:	Process a damage message.
//
// --------------------------------------------------------------------------- //

void CCharacter::ProcessDamageMsg(HMESSAGEREAD hRead)
{
	if (!hRead || Camera::IsActive()) return;

	DamageStruct damage;
	damage.InitFromMessage(hRead);

	if ( !m_damage.IsCantDamageType(damage.eType) && m_damage.GetCanDamage() )
	{
		// Set our pain information

		m_fLastPainTime = g_pLTServer->GetTime();
		m_fLastPainVolume = 1.0f;

		// Play a damage sound...

		if (!m_damage.IsDead() && m_damage.GetCanDamage())
		{
			// Play our damage sound

			PlayDamageSound(damage.eType);
		}
	}

	if ( m_damage.IsDead() && m_bWallStick )
	{
		// Should we do it still?

		m_bWallStick = ShouldWallStick();
	}
	else
	{
		m_bWallStick = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ShouldWallStick()
//
//	PURPOSE:	Should we wall stick
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::ShouldWallStick()
{
	// No see if we're going to get pinned on the wall

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	LTVector vNull, vForward;
	g_pMathLT->GetRotationVectors(rRot, vNull, vNull, vForward);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, vPos);
	// TODO: bute wall distance
	VEC_COPY(IQuery.m_To, vPos - vForward*g_BodyStickDist.GetFloat());
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	IQuery.m_FilterFn = WorldFilterFn;
	IQuery.m_PolyFilterFn = LTNULL;

	// Has to hit something

	if ( g_pLTServer->IntersectSegment(&IQuery, &IInfo) )
	{
		// Can the arrow stick into the surface?

		SurfaceType eSurf = GetSurfaceType(IInfo);
		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurf);

		if (pSurf && (pSurf->eType != ST_SKY))
		{
			// Has to be more or less same plane normal as character's forward

			// TODO: bute normal/fwd dp threshhold
			if ( IInfo.m_Plane.Normal().Dot(vForward) > s_BodyStickAngle.GetFloat() )
			{
				// g_pLTServer->CPrint("plane dot charfwd > %.2f", s_BodyStickAngle.GetFloat());

				// We already know arrow.fwd is within the threshhold, we tested this in CCharacter::AddSpear

				return LTTRUE;
			}
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::InitialUpdate()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

void CCharacter::InitialUpdate(int nInfo)
{
	// Volume debugging

	if (IsPlayer(m_hObject))
	{
		if (!g_VolumeDebugTrack.IsInitted())
		{
            g_VolumeDebugTrack.Init(g_pLTServer, "VolumeDebug", LTNULL, 0.0f);
		}
	}

	if (!s_BodyStickAngle.IsInitted())
	{
        s_BodyStickAngle.Init(g_pLTServer, "BodyStickAngle", LTNULL, 0.9f);
	}

	if(!g_BodyStickDist.IsInitted())
	{
        g_BodyStickDist.Init(g_pLTServer, "BodyStickDist", NULL, 150.0f);
	}

	if(!g_BodyStateTimeout.IsInitted())
	{
        g_BodyStateTimeout.Init(g_pLTServer, "BodyStateTimeout", NULL, 5.0f);
	}


	// Init the animator

	if (nInfo == INITIALUPDATE_SAVEGAME) return;

	// Create the box used for weapon impact detection...

	CreateHitBox();

	// Do we need environment mapping?

    if (LTTRUE == g_pModelButeMgr->GetModelEnvironmentMap(m_eModelId))
	{
		m_dwFlags |= FLAG_ENVIRONMENTMAP;
	}

	// Make sure this object is added to the global CharacterMgr...

//	g_pCharacterMgr->Add(this);

	g_pLTServer->SetObjectFlags(m_hObject, m_dwFlags);

	m_damage.Init(m_hObject);
	m_damage.SetMass(g_pModelButeMgr->GetModelMass(m_eModelId));

	if (m_fDefaultHitPts >= 0.0f)
	{
		m_damage.SetHitPoints(m_fDefaultHitPts);
		m_damage.SetMaxHitPoints(m_fDefaultHitPts);
	}
	else
	{
		m_damage.SetHitPoints(g_pModelButeMgr->GetModelHitPoints(m_eModelId));
		m_damage.SetMaxHitPoints(g_pModelButeMgr->GetModelMaxHitPoints(m_eModelId));
	}

	if (m_fDefaultArmor >= 0.0f)
	{
		m_damage.SetArmorPoints(m_fDefaultArmor);
		m_damage.SetMaxArmorPoints(m_fDefaultArmor);
	}
	else
	{
		m_damage.SetArmorPoints(g_pModelButeMgr->GetModelArmor(m_eModelId));
		m_damage.SetMaxArmorPoints(g_pModelButeMgr->GetModelMaxArmor(m_eModelId));
	}

	// Set this as an object that can be seen with night/infrared vision...

    uint32 nFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
	g_pLTServer->SetObjectUserFlags(m_hObject, nFlags | /*USRFLG_CAN_ACTIVATE |*/ USRFLG_MOVEABLE | USRFLG_NIGHT_INFRARED | USRFLG_CHARACTER);

	// Set our initial dims based on the current animation...
	// TODO! does this need to change?

    LTVector vDims;
	g_pLTServer->GetModelAnimUserDims(m_hObject, &vDims, g_pLTServer->GetModelAnimation(m_hObject));
	SetDims(&vDims);

	if (m_bMoveToFloor)
	{
		MoveObjectToFloor(m_hObject);
	}


	// Create the special fx message...

	CreateSpecialFX();
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CreateSpecialFX()
//
//	PURPOSE:	Add client-side special fx
//
// ----------------------------------------------------------------------- //

void CCharacter::CreateSpecialFX(LTBOOL bUpdateClients /* =LTFALSE */)
{
	// Create the special fx...

	m_cs.Clear();

	m_cs.eModelId					= m_eModelId;
	m_cs.byFXFlags					= m_byFXFlags;
	m_cs.eModelStyle				= m_eModelStyle;
	m_cs.nTrackers					= 0;								// Subclasses need to fill this in in precreate
	m_cs.nDimsTracker				= 0;								// Main tracker
	m_cs.fStealthPercent			= m_damage.GetStealthModifier();
	m_cs.eCrosshairCharacterClass	= m_ccCrosshair == UNKNOWN ? m_cc : m_ccCrosshair;

	PreCreateSpecialFX(m_cs);

	if (m_bShortRecoil)
	{
		// Add an extra tracker for short recoils
        m_iRecoilAnimTracker = ++m_cs.nTrackers;
	}

	if (m_bBlink)
	{
		// Add an extra tracker for blinking
        m_iBlinkAnimTracker = ++m_cs.nTrackers;
	}

	HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
    m_cs.Write(g_pLTServer, hMessage);
	g_pLTServer->EndMessage(hMessage);


	// Tell the client about the new info...

	if (bUpdateClients)
	{
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
		g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
		g_pLTServer->WriteToMessageByte(hMessage, CFX_ALLFX_MSG);
        m_cs.Write(g_pLTServer, hMessage);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SendStealthToClients()
//
//	PURPOSE:	Send our stealth variable to the clients
//
// ----------------------------------------------------------------------- //

void CCharacter::SendStealthToClients()
{
	// Update clients with new info...

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_STEALTH_MSG);
	g_pLTServer->WriteToMessageFloat(hMessage, m_damage.GetStealthModifier());
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

	// Update our special fx message so new clients will get the updated
	// info as well...

	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::*Cigarette()
//
//	PURPOSE:	Creates/Destroys hearts sfx on client
//
// ----------------------------------------------------------------------- //

void CCharacter::CreateCigarette(LTBOOL bSmoke)
{
	_ASSERT(!(m_byFXFlags & CHARCREATESTRUCT::eCigarette));

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_CIGARETTE_CREATE_MSG);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

	m_byFXFlags |= CHARCREATESTRUCT::eCigarette;

	if ( bSmoke )
	{
		m_byFXFlags |= CHARCREATESTRUCT::eCigaretteSmoke;

		hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
		g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
		g_pLTServer->WriteToMessageByte(hMessage, CFX_CIGARETTESMOKE_CREATE_MSG);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
	}
	else
	{
		m_byFXFlags &= ~CHARCREATESTRUCT::eCigaretteSmoke;
	}

	CreateSpecialFX();
}

void CCharacter::DestroyCigarette()
{
	_ASSERT(m_byFXFlags & CHARCREATESTRUCT::eCigarette);

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_CIGARETTE_DESTROY_MSG);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

    hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_CIGARETTESMOKE_DESTROY_MSG);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

	m_byFXFlags &= ~CHARCREATESTRUCT::eCigarette;
	m_byFXFlags &= ~CHARCREATESTRUCT::eCigaretteSmoke;

	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::*Smokepuffs()
//
//	PURPOSE:	Creates/Destroys hearts sfx on client
//
// ----------------------------------------------------------------------- //

void CCharacter::CreateSmokepuffs()
{
	_ASSERT(!(m_byFXFlags & CHARCREATESTRUCT::eSmokepuffs));

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_SMOKEPUFF_CREATE_MSG);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

	m_byFXFlags |= CHARCREATESTRUCT::eSmokepuffs;

	CreateSpecialFX();
}

void CCharacter::DestroySmokepuffs()
{
	_ASSERT(m_byFXFlags & CHARCREATESTRUCT::eSmokepuffs);

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_SMOKEPUFF_DESTROY_MSG);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

	m_byFXFlags &= ~CHARCREATESTRUCT::eSmokepuffs;

	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::*Zzz()
//
//	PURPOSE:	Creates/Destroys hearts sfx on client
//
// ----------------------------------------------------------------------- //

void CCharacter::CreateZzz()
{
	_ASSERT(!(m_byFXFlags & CHARCREATESTRUCT::eZzz));

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_ZZZ_CREATE_MSG);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

	m_byFXFlags |= CHARCREATESTRUCT::eZzz;

	CreateSpecialFX();
}

void CCharacter::DestroyZzz()
{
	_ASSERT(m_byFXFlags & CHARCREATESTRUCT::eZzz);

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_ZZZ_DESTROY_MSG);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

	m_byFXFlags &= ~CHARCREATESTRUCT::eZzz;

	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::*Hearts()
//
//	PURPOSE:	Creates/Destroys hearts sfx on client
//
// ----------------------------------------------------------------------- //

void CCharacter::CreateHearts()
{
	_ASSERT(!(m_byFXFlags & CHARCREATESTRUCT::eHearts));

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_HEART_CREATE_MSG);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

	m_byFXFlags |= CHARCREATESTRUCT::eHearts;

	CreateSpecialFX();
}

void CCharacter::DestroyHearts()
{
	_ASSERT(m_byFXFlags & CHARCREATESTRUCT::eHearts);

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_HEART_DESTROY_MSG);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

	m_byFXFlags &= ~CHARCREATESTRUCT::eHearts;

	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CacheFiles()
//
//	PURPOSE:	Cache resources used by this object
//
// ----------------------------------------------------------------------- //

void CCharacter::CacheFiles()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandHeldWeaponFirePos()
//
//	PURPOSE:	Get the fire (flash) position of the hand-held weapon
//
// ----------------------------------------------------------------------- //

LTVector CCharacter::HandHeldWeaponFirePos(CWeapon *pWeapon)
{
    LTRotation rRot;
    LTVector vPos;

	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	if (!g_pLTServer || !pWeapon || !g_pWeaponMgr) return vPos;

	HATTACHMENT hAttachment;
    if (g_pLTServer->FindAttachment(m_hObject, pWeapon->GetModelObject(), &hAttachment) != LT_OK)
	{
		return vPos;
	}

	HMODELSOCKET hSocket;

    if (g_pModelLT->GetSocket(pWeapon->GetModelObject(), "Flash", hSocket) == LT_OK)
	{
		LTransform transform;
        ILTCommon* pCommonLT = g_pLTServer->Common();
		pCommonLT->GetAttachedModelSocketTransform(hAttachment, hSocket, transform);

        ILTTransform* pTransLT = g_pLTServer->GetTransformLT();
		g_pTransLT->GetPos(transform, vPos);
	}

	return vPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::InitAnimation()
//
//	PURPOSE:	Initializes our animation
//
// ----------------------------------------------------------------------- //

void CCharacter::InitAnimation()
{
	// Init the animator if we haven't done so yet

	if ( m_pAnimator && !m_pAnimator->IsInitialized() )
	{
		m_pAnimator->Init(g_pLTServer, m_hObject);
	}

	// Add the recoil tracker

	if ( m_bShortRecoil )
	{
        g_pModelLT->AddTracker(m_hObject, &m_RecoilAnimTracker);

		if ( LT_OK != g_pModelLT->FindWeightSet(m_hObject, "Null", m_hNullWeightset) )
		{
#ifndef _FINAL
			g_pLTServer->CPrint("Critical error, no Null weightset on Character!");
#endif
		}

		if ( LT_OK != g_pModelLT->FindWeightSet(m_hObject, "Blink", m_hBlinkWeightset) )
		{
#ifndef _FINAL
			g_pLTServer->CPrint("Critical error, no Blink weightset on Character!");
#endif
			m_hBlinkWeightset = m_hNullWeightset;
		}

		if ( LT_OK != g_pModelLT->FindWeightSet(m_hObject, "Twitch", m_hTwitchWeightset) )
		{
#ifndef _FINAL
			g_pLTServer->CPrint("Critical error, no Twitch weightset on Character!");
#endif
			m_hTwitchWeightset = m_hNullWeightset;
		}
		else
		{
			g_pModelLT->SetWeightSet(&m_RecoilAnimTracker, m_hTwitchWeightset);
		}

        g_pModelLT->SetCurAnim(&m_RecoilAnimTracker, g_pLTServer->GetAnimIndex(m_hObject, "Base"));
        g_pModelLT->SetLooping(&m_RecoilAnimTracker, LTFALSE);
	}

	// Add the blink tracker

	if ( m_bBlink )
	{
        g_pModelLT->AddTracker(m_hObject, &m_BlinkAnimTracker);
        g_pModelLT->SetCurAnim(&m_BlinkAnimTracker, g_pLTServer->GetAnimIndex(m_hObject, "Blink"));
        g_pModelLT->SetLooping(&m_BlinkAnimTracker, LTTRUE);

		SetBlinking(LTTRUE);
	}

	// We're initted

	m_bInitializedAnimation = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::Update()
//
//	PURPOSE:	Update the object
//
// ----------------------------------------------------------------------- //

void CCharacter::Update()
{
	extern LTBOOL g_bAutoSaved;
	if ( !g_bAutoSaved ) return;

	if ( !m_bInitializedAnimation )
	{
		InitAnimation();
	}

	// Update the recoil

	if ( m_bShortRecoiling )
	{
		uint32 dwFlags;
        if ( LT_OK == g_pModelLT->GetPlaybackState(&m_RecoilAnimTracker, dwFlags) )
		{
			if ( MS_PLAYDONE & dwFlags )
			{
				g_pModelLT->SetWeightSet(&m_RecoilAnimTracker, m_hNullWeightset);
				m_bShortRecoiling = LTFALSE;
			}
		}
	}

	// Update our last volume position

	if ( g_pAIVolumeMgr->IsInitialized() )
	{
        LTVector vPos, vDims;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		g_pLTServer->GetObjectDims(m_hObject, &vDims);

		CAIVolume* pVolume = g_pAIVolumeMgr->FindContainingVolume(vPos, vDims.y*2.0f, m_iLastVolume == -1 ? LTNULL : g_pAIVolumeMgr->GetVolumeByIndex(m_iLastVolume));

		if ( pVolume )
		{
			m_iLastVolume = pVolume->GetIndex();
			m_vLastVolumePos = vPos;

#ifndef _FINAL
			if ( IsPlayer(m_hObject) && g_VolumeDebugTrack.GetFloat(0.0f) == 1.0f )
			{
				g_pLTServer->CPrint("Player in volume \"%s\"", pVolume->GetName());
			}
#endif
		}
		else
		{
#ifndef _FINAL
			if ( IsPlayer(m_hObject) && g_VolumeDebugTrack.GetFloat(0.0f) == 1.0f )
			{
				g_pLTServer->CPrint("Player not in volume");
			}
#endif
		}
	}

	// Update our footprints

	UpdateFootprints();

	// Update our sounds

	UpdateSounds();

	// Keep track of frame to frame changes...

	m_eLastContainerCode	= m_eContainerCode;
	m_bBodyWasInLiquid		= m_bBodyInLiquid;

    m_bBodyInLiquid         = LTFALSE;
    m_bBodyOnLadder         = LTFALSE;

	// Update our container code info...

	UpdateContainerCode();

	// Make sure our hit box is in the correct position...

	UpdateHitBox();

	// Update our animation

	UpdateAnimation();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleShortRecoil()
//
//	PURPOSE:	Handles doing a short recoil
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleShortRecoil()
{
	if ( !m_bShortRecoil || m_bShortRecoiling ) return;
	if ( eModelNodeInvalid == m_eModelNodeLastHit ) return;

	const char* szRecoil = LTNULL;
	if ( HitFromFront(m_damage.GetLastDamageDir()) )
	{
		szRecoil = g_pModelButeMgr->GetSkeletonNodeFrontShortRecoilAni(m_eModelSkeleton, m_eModelNodeLastHit);
	}
	else
	{
		szRecoil = g_pModelButeMgr->GetSkeletonNodeBackShortRecoilAni(m_eModelSkeleton, m_eModelNodeLastHit);
	}

	HMODELANIM hAni;
	if ( !szRecoil || (INVALID_MODEL_ANIM == (hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)szRecoil))) )
	{
		return;
	}

    g_pModelLT->SetCurAnim(&m_RecoilAnimTracker, hAni);
    g_pModelLT->SetWeightSet(&m_RecoilAnimTracker, m_hTwitchWeightset);
    g_pModelLT->ResetAnim(&m_RecoilAnimTracker);

	// TODO: it'd be nice if we didn't have to EXPLICITLY TELL THE CLIENT TO RESET THE ANIMATION
    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_RESET_TRACKER);
    g_pLTServer->WriteToMessageByte(hMessage, (uint8)m_iRecoilAnimTracker);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

	m_bShortRecoiling = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleModelString()
//
//	PURPOSE:	Handles model keyframe strings
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleModelString(ArgList* pArgList)
{
	if (!pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	// Only update the footstep info if we are on the ground...(NOTE:  The
	// character footstep sounds are now played on the client. see CCharacterFX
	// if you're interested)

	if (stricmp(pKey, KEY_FOOTSTEP_SOUND) == 0 && m_bOnGround || m_bBodyOnLadder)
	{
        LTBOOL bInWater = (m_bBodyInLiquid && !IsLiquid(m_eContainerCode));

		if (m_bBodyOnLadder)
		{
			m_eStandingOnSurface = ST_LADDER;
		}
		else if (bInWater)
		{
			m_eStandingOnSurface = ST_LIQUID;
		}
		else if ( ST_UNKNOWN == m_eStandingOnSurface )
		{
			CollisionInfo Info;
			g_pLTServer->GetStandingOn(m_hObject, &Info);

			if (Info.m_hPoly && Info.m_hPoly != INVALID_HPOLY)
			{
				m_eStandingOnSurface = GetSurfaceType(Info.m_hPoly);
			}
			else if (Info.m_hObject) // Get the texture flags from the object...
			{
				m_eStandingOnSurface = GetSurfaceType(Info.m_hObject);
			}
		}

		m_LastMoveInfo.fTime = g_pLTServer->GetTime();
		m_LastMoveInfo.eSurfaceType = m_eStandingOnSurface;

		// TODO! this is a bit sloppy

		m_LastMoveInfo.fVolume = GetFootstepVolume();

		// Adjust the footstep volume by our stealth modifier...

		m_LastMoveInfo.fVolume *= m_damage.GetStealthModifier();

		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eStandingOnSurface);
		_ASSERT(pSurf);
		if (pSurf)
		{
			m_LastMoveInfo.fVolume *= pSurf->fMovementNoiseModifier;

			// TODO: reduce this to a bool function call
			if ( *pSurf->szLtFootPrintSpr && *pSurf->szRtFootPrintSpr )
			{
				// If this is a surface that creates footprints, add a footprint to our list

				// TODO: better memory management here

				CharFootprintInfo* pFootprint = s_bankCharFootprintInfo.New();
				g_pLTServer->GetObjectPos(m_hObject, &pFootprint->vPos);
				pFootprint->fDuration = pSurf->fFootPrintLifetime;
				pFootprint->eSurface = m_eStandingOnSurface;
				pFootprint->fTimeStamp = g_pLTServer->GetTime();

				m_listFootprints.Add(pFootprint);
			}
		}
	}
	else if (stricmp(pKey, KEY_PLAYSOUND) == 0 && pArgList->argc > 1)
	{
		// Get sound name from message...

		char* pSoundName = pArgList->argv[1];

		if (pSoundName)
		{
			// See if sound radius was in message..

			LTFLOAT fRadius = 1000;

			if (pArgList->argc > 3 && pArgList->argv[2])
			{
				fRadius = (LTFLOAT) atoi(pArgList->argv[2]);
			}

			fRadius = fRadius > 0.0f ? fRadius : m_fSoundRadius;

            PlaySound(pSoundName, fRadius, LTTRUE);
		}
	}
	else if (stricmp(pKey, KEY_SET_DIMS) == 0)
	{
		if (pArgList->argc < 2) return;

		// Set up so we can set one or more dims...

        LTVector vDims;
		g_pLTServer->GetObjectDims(m_hObject, &vDims);

		if (pArgList->argv[1])
		{
			vDims.x = (LTFLOAT) atof(pArgList->argv[1]);
		}
		if (pArgList->argc > 2 && pArgList->argv[2])
		{
			vDims.y = (LTFLOAT) atof(pArgList->argv[2]);
		}
		if (pArgList->argc > 3 && pArgList->argv[3])
		{
			vDims.z = (LTFLOAT) atof(pArgList->argv[3]);
		}

		// Set the new dims

		SetDims(&vDims);
	}
	else if (stricmp(pKey, KEY_MOVE) == 0)
	{
		if (pArgList->argc < 2) return;

		// Set up so we move in one or more directions

        LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

        LTRotation rRot;
        LTVector vU, vR, vF;
		g_pLTServer->GetObjectRotation(m_hObject, &rRot);
		g_pLTServer->GetRotationVectors(&rRot, &vU, &vR, &vF);

		LTFLOAT fOffset;

		if (pArgList->argv[1])
		{
			// Forward...

			fOffset = (LTFLOAT) atof(pArgList->argv[1]);

			VEC_MULSCALAR(vF, vF, fOffset);
			VEC_ADD(vPos, vPos, vF);
		}
		if (pArgList->argc > 2 && pArgList->argv[2])
		{
			// Up...

			fOffset = (LTFLOAT) atof(pArgList->argv[2]);

			VEC_MULSCALAR(vU, vU, fOffset);
			VEC_ADD(vPos, vPos, vU);
		}
		if (pArgList->argc > 3 && pArgList->argv[3])
		{
			// Right...

			fOffset = (LTFLOAT) atof(pArgList->argv[3]);

			VEC_MULSCALAR(vR, vR, fOffset);
			VEC_ADD(vPos, vPos, vR);
		}

		// Set the new position

		g_pLTServer->MoveObject(m_hObject, &vPos);
	}
	else if (stricmp(pKey, KEY_COMMAND) == 0)
	{
		LTBOOL bAddParen = LTFALSE;

		char buf[255] = "";
		sprintf(buf, "%s", pArgList->argv[1]);
		for (int i=2; i < pArgList->argc; i++)
		{
			bAddParen = LTFALSE;
			strcat(buf, " ");
			if (strstr(pArgList->argv[i], " "))
			{
				strcat(buf, "(");
				bAddParen = LTTRUE;
			}

			strcat(buf, pArgList->argv[i]);

			if (bAddParen)
			{
				strcat(buf, ")");
			}
		}

		g_pLTServer->CPrint("KEY COMMAND: %s", buf);
		if (buf[0] && g_pCmdMgr->IsValidCmd(buf))
		{
			g_pCmdMgr->Process(buf);
		}
    }

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateAnimation()
//
//	PURPOSE:	Update the current animation
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateAnimation()
{
	// If we're dead, we do that first

	if (m_damage.IsDead())
	{
		SetDeathAnimation();
		return;
	}
    else
         if (m_pAnimator)
	{
		m_pAnimator->Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetDeathAnimation()
//
//	PURPOSE:	Set animation to death
//
// ----------------------------------------------------------------------- //

void CCharacter::SetDeathAnimation()
{
	if (m_bStartedDeath) return;

	StartDeath();

	// Figure out if this was a death from behind or from front

    LTBOOL bFront = HitFromFront(m_damage.GetDeathDir());

	// Choose the appropriate death ani

	m_eDeathType = CD_NORMAL;

	LTFLOAT fDeathDamage = m_damage.GetDeathDamage();
	LTFLOAT fMaxHitPts   = m_damage.GetMaxHitPoints();

	DamageType eDType = m_damage.GetDeathType();
    LTBOOL bGibDeath   = LTFALSE;

	HMODELANIM hAni = INVALID_ANI;

	if ( eDType == DT_EXPLODE && (fDeathDamage > fMaxHitPts/2.0f) )
	{
		hAni = g_pLTServer->GetAnimIndex(m_hObject, "DBnF");

		if ( hAni != INVALID_ANI )
		{
			hAni = GetDeathAni(bFront);
		}
	}
	else
	{
		hAni = GetDeathAni(bFront);
	}

	// Set the death animation

	if ( hAni != INVALID_ANI )
	{
        g_pLTServer->SetModelLooping(m_hObject, LTFALSE);

		// Set model dims based on animation...
/*
        LTVector vDims;
        if (g_pLTServer->GetModelAnimUserDims(m_hObject, &vDims, hAni) == LT_OK)
		{
			// If we could update the dims, or we're forcing the animation, set it

            SetDims(&vDims, LTFALSE);

		}
*/
		g_pLTServer->SetModelAnimation(m_hObject, hAni);
	}

	// Make us nonsolid...

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	dwFlags &= ~FLAG_SOLID;
	g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

	// Handle dead

    HandleDead(LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetDeathAni()
//
//	PURPOSE:	Gets the location based death animation
//
// ----------------------------------------------------------------------- //

HMODELANIM CCharacter::GetDeathAni(LTBOOL bFront)
{
	HMODELANIM hAni = INVALID_ANI;

	if ( bFront )
	{
		// Look for a death ani specific to this node

		if ( eModelNodeInvalid != m_eModelNodeLastHit )
		{
			const char* szDeathAni = g_pModelButeMgr->GetSkeletonNodeFrontDeathAni(m_eModelSkeleton, m_eModelNodeLastHit);
			if ( szDeathAni )
			{
				hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)szDeathAni);
			}
		}

		// If the given node-specific ani could not be found, just use the default (which better be there)

		if ( hAni == INVALID_ANI )
		{
			const char* szDeathAni = g_pModelButeMgr->GetSkeletonDefaultFrontDeathAni(m_eModelSkeleton);
			if ( szDeathAni )
			{
				hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)szDeathAni);
			}
		}
	}
	else
	{
		// Look for a death ani specific to this node

		if ( eModelNodeInvalid != m_eModelNodeLastHit )
		{
			const char* szDeathAni = g_pModelButeMgr->GetSkeletonNodeBackDeathAni(m_eModelSkeleton, m_eModelNodeLastHit);
			if ( szDeathAni )
			{
				hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)szDeathAni);
			}
		}

		// If the given node-specific ani could not be found, just use the default (which better be there)

		if ( hAni == INVALID_ANI )
		{
			const char* szDeathAni = g_pModelButeMgr->GetSkeletonDefaultBackDeathAni(m_eModelSkeleton);
			if ( szDeathAni )
			{
				hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)szDeathAni);
			}
		}
	}

	return hAni;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateMovement
//
//	PURPOSE:	Update character movement
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateMovement(LTBOOL bUpdatePhysics)
{
	// Update m_bOnGround data member...

	UpdateOnGround();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateOnLadder
//
//	PURPOSE:	Update if we're on a ladder
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateOnLadder(VolumeBrush* pBrush, ContainerPhysics* pCPStruct)
{
    m_bBodyOnLadder = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateInLiquid
//
//	PURPOSE:	Update if we're in liquid
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateInLiquid(VolumeBrush* pBrush, ContainerPhysics* pCPStruct)
{
    m_bBodyInLiquid = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateOnGround
//
//	PURPOSE:	Update m_bOnGround data member
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateOnGround()
{
	// See if we're standing on any breakable objects...

	CollisionInfo Info;
	g_pLTServer->GetStandingOn(m_hObject, &Info);

	if (Info.m_hObject && IsKindOf(Info.m_hObject, "Breakable"))
	{
        SendTriggerMsgToObject(this, Info.m_hObject, LTFALSE, "TOUCHNOTIFY");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateFootprints()
//
//	PURPOSE:	Update our footprint list
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateFootprints()
{
	CharFootprintInfo** ppFootprint = m_listFootprints.GetItem(TLIT_FIRST);
	while ( ppFootprint && *ppFootprint )
	{
		CharFootprintInfo* pFootprint = *ppFootprint;
		ppFootprint = m_listFootprints.GetItem(TLIT_NEXT);

		pFootprint->fDuration -= g_pLTServer->GetFrameTime();
		if ( pFootprint->fDuration < 0.0f )
		{
			m_listFootprints.Remove(pFootprint);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateSounds()
//
//	PURPOSE:	Update the currently playing sounds
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateSounds()
{
	// See if it is time to shut up...

	if (m_hCurDlgSnd)
	{
        LTBOOL bIsDone = LTFALSE;
		if (g_pLTServer->IsSoundDone(m_hCurDlgSnd, &bIsDone) != LT_OK || bIsDone)
		{
			KillDlgSnd();
		}
	}
	else
	{
		m_eCurDlgSndType = CST_NONE;
	}

	// See if we are coming out of a liquid...

	if (!m_bBodyInLiquid && m_bBodyWasInLiquid)
	{
        PlaySound("Chars\\Snd\\splash1.wav", m_fSoundRadius, LTFALSE);
	}
	else if (!m_bBodyWasInLiquid && m_bBodyInLiquid)  // or going into
	{
        PlaySound("Chars\\Snd\\splash2.wav", m_fSoundRadius, LTFALSE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlaySound
//
//	PURPOSE:	Play the specified sound
//
// ----------------------------------------------------------------------- //

void CCharacter::PlaySound(char *pSoundName, LTFLOAT fRadius, LTBOOL bAttached)
{
    LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pServerSoundMgr->PlaySoundFromPos(vPos, pSoundName, fRadius, m_eSoundPriority);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayDamageSound
//
//	PURPOSE:	Play a damage sound
//
// ----------------------------------------------------------------------- //

void CCharacter::PlayDamageSound(DamageType eType)
{
	if (m_eCurDlgSndType == CST_DAMAGE) return;

	PlayDialogSound(GetDamageSound(eType), CST_DAMAGE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayDialogSound
//
//	PURPOSE:	Play a dialog sound
//
// ----------------------------------------------------------------------- //

void CCharacter::PlayDialogSound(char* pSound, CharacterSoundType eType)
{
	if (!pSound || !pSound[0]) return;
	if ((m_damage.IsDead() && eType != CST_DEATH)) return;
	if (eType == CST_AI_SOUND && (sm_cAISnds >= 2)) return;
	if (eType == CST_EXCLAMATION && !m_bCanPlayDialogSound) return;


	// Kill current sound...

	KillDlgSnd();


	// Only lip sync if single player...

    LTBOOL bLipSync = (CanLipSync() && (g_pGameServerShell->GetGameType() == SINGLE));

	if (bLipSync)
	{
		// Tell the client to play the sound (lip synched)...
		// TO DO: Remove sending of sound path, send sound id instead...

		HSTRING hStr = g_pLTServer->CreateString(pSound);

        HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
		g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
		g_pLTServer->WriteToMessageByte(hMessage, CFX_NODECONTROL_LIP_SYNC);
		g_pLTServer->WriteToMessageHString(hMessage, hStr);
		g_pLTServer->WriteToMessageFloat(hMessage, m_fSoundRadius);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

		g_pLTServer->FreeString(hStr);
	}
	else if (DoDialogueSubtitles())
	{
		HSTRING hStr = g_pLTServer->CreateString(pSound);

        HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
		g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
		g_pLTServer->WriteToMessageByte(hMessage, CFX_DIALOGUE_MSG);
		g_pLTServer->WriteToMessageHString(hMessage, hStr);
		g_pLTServer->WriteToMessageFloat(hMessage, m_fSoundRadius);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

		g_pLTServer->FreeString(hStr);
	}


	// Always play the player dialog sounds in the local client's head
	// (unless we're lipsyncing)...

    uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_TIME;

	if (IsPlayer(m_hObject) && !bLipSync)
	{
		dwFlags |= PLAYSOUND_CLIENTLOCAL;
	}


	// If we're lip-syncing the sound is played on the client.  We'll still
	// play the sound on the server to time it, however we don't want to
	// hear it or have it actually sent to the sound card (with 0 radius
	// volume and 0 radius it *hopefully* won't be sent to any of the
	// clients)...

	LTFLOAT fRadius = bLipSync ? 0.0f : m_fSoundRadius;
	uint8 nVolume = bLipSync ? 0 : SMGR_DEFAULT_VOLUME;

	m_hCurDlgSnd = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, pSound,
		fRadius, m_eSoundPriority, dwFlags, nVolume);

	m_eCurDlgSndType = eType;

	if ( m_eCurDlgSndType == CST_AI_SOUND )
	{
		sm_cAISnds++;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::KillDlgSnd
//
//	PURPOSE:	Kill the current dialog sound
//
// ----------------------------------------------------------------------- //

void CCharacter::KillDlgSnd()
{
	if (m_hCurDlgSnd)
	{
		g_pLTServer->KillSound(m_hCurDlgSnd);
        m_hCurDlgSnd = LTNULL;

	}

	if ( m_eCurDlgSndType == CST_AI_SOUND )
	{
		sm_cAISnds--;
	}

	m_eCurDlgSndType = CST_NONE;

	// Make sure the client knows to stop lip syncing...

    LTBOOL bLipSync = CanLipSync() && (g_pGameServerShell->GetGameType() == SINGLE);

	if (bLipSync)
	{
        HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
		g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
		g_pLTServer->WriteToMessageByte(hMessage, CFX_NODECONTROL_LIP_SYNC);
        g_pLTServer->WriteToMessageHString(hMessage, LTNULL);
		g_pLTServer->WriteToMessageFloat(hMessage, 0.0f);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
	}
	else if (DoDialogueSubtitles())
	{
        HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
		g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
		g_pLTServer->WriteToMessageByte(hMessage, CFX_DIALOGUE_MSG);
        g_pLTServer->WriteToMessageHString(hMessage, LTNULL);
		g_pLTServer->WriteToMessageFloat(hMessage, 0.0f);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayDialogue
//
//	PURPOSE:	Do dialogue sound/window
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::PlayDialogue(DWORD dwID, CinematicTrigger* pCinematic,
	BOOL bWindow, BOOL bStayOpen, const char *szCharOverride, char *szDecisions,
    unsigned char byMood)
{
    if (!dwID) return LTFALSE;

	CString csSound = g_pServerSoundMgr->GetSoundFilenameFromId("Dialogue", dwID);
	PlayDialogSound((char *)(LPCSTR)csSound, CST_EXCLAMATION);

	if (bWindow)
	{
		return(DoDialogueWindow(pCinematic,dwID,bStayOpen,szCharOverride,szDecisions));
	}

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayDialogue
//
//	PURPOSE:	Do dialogue sound/window
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::PlayDialogue(char *szDialogue, CinematicTrigger* pCinematic,
	BOOL bWindow, BOOL bStayOpen, const char* szCharOverride, char *szDecisions,
    unsigned char byMood)
{
    if (!szDialogue) return LTFALSE;

	CString csSound;
	DWORD dwID = 0;
	if ((szDialogue[0] >= '0') && (szDialogue[0] <= '9'))
	{
		// It's an ID
		dwID = atoi(szDialogue);
		csSound = g_pServerSoundMgr->GetSoundFilenameFromId("Dialogue", dwID);
	}
	else
	{
		// It's a sound
		csSound = szDialogue;
	}

	PlayDialogSound((char *)(LPCSTR)csSound, CST_EXCLAMATION);

	if (bWindow)
	{
		return(DoDialogueWindow(pCinematic,dwID,bStayOpen,szCharOverride,szDecisions));
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::DoDialogueWindow
//
//	PURPOSE:	Bring up the dialogue window
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::DoDialogueWindow(CinematicTrigger* pCinematic, DWORD dwID,
	BOOL bStayOpen, const char *szCharOverride, char *szDecisions)
{
	const char *szTag = m_csName;
	if (szCharOverride)
	{
		szTag = szCharOverride;
	}

	if (g_DialogueWindow.IsPlaying())
	{
		TRACE("ERROR - Dialogue window was already up when we tried to play dialogue!\n");
        return LTFALSE;
	}

	if (!g_DialogueWindow.PlayDialogue(dwID,bStayOpen,szTag,szDecisions,pCinematic))
	{
		TRACE("ERROR - Could not play dialogue ID: %d\n",dwID);
        return LTFALSE;
	}

    m_bPlayingTextDialogue = LTTRUE;

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::StopDialogue
//
//	PURPOSE:	Stop the dialogue
//
// ----------------------------------------------------------------------- //

void CCharacter::StopDialogue(LTBOOL bCinematicDone)
{
	KillDialogueSound();
    m_bPlayingTextDialogue = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayDeathSound()
//
//	PURPOSE:	Play the death sound
//
// ----------------------------------------------------------------------- //

void CCharacter::PlayDeathSound()
{
	KillDlgSnd();
    PlaySound(GetDeathSound(), m_fSoundRadius, LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleDead()
//
//	PURPOSE:	Okay, death animation is done...
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleDead(LTBOOL bRemoveObj)
{
	if (m_fBodyLifetime != 0.0f)
	{
		// > 0 means finite body lifetime, < 0 means infinite body lifetime

		CreateBody();
	}

	if (bRemoveObj)
	{
		RemoveObject();
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleVectorImpact()
//
//	PURPOSE:	Last chance to reject getting hit
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleVectorImpact(IntersectInfo& iInfo, LTVector& vDir, LTVector& vFrom, ModelNode& eModelNode)
{
	// Let the character/body prop's attachments handle the impact...

	if ( m_pAttachments )
	{
//		m_pAttachments->HandleProjectileImpact(iInfo, vDir, vFrom, m_eModelSkeleton, eModelNode);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::RemoveObject()
//
//	PURPOSE:	Handle removing character objects
//
// ----------------------------------------------------------------------- //

void CCharacter::RemoveObject()
{
	// Get rid of the spears if they're still around

	for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
	{
		HATTACHMENT hAttachment;
		HOBJECT hSpear = m_aSpears[iSpear].hObject;
		if ( hSpear )
		{
			if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hSpear, &hAttachment) )
			{
				if ( LT_OK == g_pLTServer->RemoveAttachment(hAttachment) )
				{
				}
			}

			g_pLTServer->BreakInterObjectLink(m_hObject, hSpear);
			g_pLTServer->RemoveObject(hSpear);
			m_aSpears[iSpear].hObject = LTNULL;
		}
	}

	// Take us out of the charactermgr

	g_pCharacterMgr->Remove(this);

	// Remove the engine object...

	g_pLTServer->RemoveObject(m_hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CreateBody()
//
//	PURPOSE:	Create the body prop
//
// ----------------------------------------------------------------------- //

void CCharacter::CreateBody()
{
    LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	HCLASS hClass = g_pLTServer->GetClass("Body");
	if (!hClass) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	g_pLTServer->GetModelFilenames(m_hObject, theStruct.m_Filename, MAX_CS_FILENAME_LEN, theStruct.m_SkinName, MAX_CS_FILENAME_LEN);

	// Setup the head skin...
	const char* pSkin2 = GetHeadSkinFilename();
	SAFE_STRCPY(theStruct.m_SkinNames[1], pSkin2);

	sprintf(theStruct.m_Name, "%s_body", g_pLTServer->GetObjectName(m_hObject));

	VEC_SET(theStruct.m_Pos, 0.0f, 0.1f, 0.0f);
	VEC_ADD(theStruct.m_Pos, theStruct.m_Pos, vPos);
	g_pLTServer->GetObjectRotation(m_hObject, &theStruct.m_Rotation);

	// Allocate an object...

	Body* pProp = (Body *)g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pProp) return;

	SetupBody(pProp);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetupBody()
//
//	PURPOSE:	Sets up the body prop
//
// ----------------------------------------------------------------------- //

void CCharacter::SetupBody(Body* pProp)
{
	_ASSERT(pProp);
	if ( !pProp ) return;

	BODYINITSTRUCT bi;
	bi.eBodyState = GetBodyState();
	bi.pCharacter = this;
	bi.fLifetime = m_fBodyLifetime;

	pProp->Init(bi);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetBodyState()
//
//	PURPOSE:	Gets the state of our body prop
//
// ----------------------------------------------------------------------- //

BodyState CCharacter::GetBodyState()
{
	if ( m_bWallStick )
	{
		return eBodyStateArrow;
	}
	else if ( m_damage.GetLastDamageType() == DT_EXPLODE )
	{
		return eBodyStateExplode;
	}
	else if ( m_damage.GetLastDamageType() == DT_BURN )
	{
		return eBodyStateAcid;
	}
	else if ( m_damage.GetLastDamageType() == DT_ELECTROCUTE )
	{
		return eBodyStateLaser;
	}
	else if ( m_damage.GetLastDamageType() == DT_CRUSH )
	{
		return eBodyStateCrush;
	}
	else if ( m_damage.GetLastDamageType() == DT_POISON )
	{
		return eBodyStatePoison;
	}
	else if ( HasLastVolume() )
	{
		CAIVolume* pVolume = GetLastVolume();
		if ( pVolume->HasStairs() )
		{
			return eBodyStateStairs;
		}
		else if ( pVolume->HasLedge() )
		{
			return eBodyStateLedge;
		}
		else if ( IsLiquid(m_eContainerCode) )
		{
			return eBodyStateUnderwater;
		}
	}

	return eBodyStateNormal;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetPriorityBodyState()
//
//	PURPOSE:	Returns the body state with higher "priority"
//
// ----------------------------------------------------------------------- //

BodyState CCharacter::GetPriorityBodyState(BodyState bs1, BodyState bs2)
{
	switch ( bs1 )
	{
		case eBodyStateArrow:
			return bs1;
		case eBodyStateExplode:
			return bs1;
		case eBodyStateCrush:
			return bs1;
		case eBodyStateChair:
			return bs1;
		case eBodyStateUnderwater:
			return bs1;
		case eBodyStateLedge:
			return bs1;
		case eBodyStateStairs:
			return bs1;
		case eBodyStateLaser:
			return bs1;
		case eBodyStatePoison:
			return bs1;
		case eBodyStateAcid:
			return bs1;
	}

	switch ( bs2 )
	{
		case eBodyStateArrow:
			return bs2;
		case eBodyStateExplode:
			return bs2;
		case eBodyStateCrush:
			return bs2;
		case eBodyStateChair:
			return bs2;
		case eBodyStateUnderwater:
			return bs2;
		case eBodyStateLedge:
			return bs2;
		case eBodyStateStairs:
			return bs2;
		case eBodyStateLaser:
			return bs2;
		case eBodyStatePoison:
			return bs2;
		case eBodyStateAcid:
			return bs2;
	}

	_ASSERT(bs1 == bs2);

	return bs1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetDims()
//
//	PURPOSE:	Set the dims for the character
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::SetDims(LTVector* pvDims, LTBOOL bSetLargest)
{
    if (!pvDims) return LTFALSE;

    LTBOOL bRet = LTTRUE;

	// Calculate what the dims should be based on our model size...

    LTVector vNewDims;
	VEC_MULSCALAR(vNewDims, *pvDims, 1.0f);


    LTVector vOldDims;
	g_pLTServer->GetObjectDims(m_hObject, &vOldDims);


	// Only update dims if they have changed...

	if ((vNewDims.x > vOldDims.x - DIMS_EPSILON && vNewDims.x < vOldDims.x + DIMS_EPSILON) &&
		(vNewDims.y > vOldDims.y - DIMS_EPSILON && vNewDims.y < vOldDims.y + DIMS_EPSILON) &&
		(vNewDims.z > vOldDims.z - DIMS_EPSILON && vNewDims.z < vOldDims.z + DIMS_EPSILON))
	{
        return LTTRUE;  // Setting of dims didn't actually fail
	}


	// Try to set our new dims...

    if (g_pLTServer->SetObjectDims2(m_hObject, &vNewDims) == LT_ERROR)
	{
		if (bSetLargest)
		{
			g_pLTServer->SetObjectDims2(m_hObject, &vNewDims);
		}

        bRet = LTFALSE; // Didn't set to new dims...
	}


	// See if we need to move the object down...

	if (vNewDims.y < vOldDims.y)
	{
        LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		vPos.y -= (vOldDims.y - vNewDims.y);
		vPos.y += .01f; // Fudge factor

		// This forces the client to move to the server's position because it's teleporting.
		//g_pLTServer->SetObjectPos(m_hObject, &vPos);
		g_pLTServer->MoveObject(m_hObject, &vPos);
	}


	// Update the dims of our hit box...

	if (m_hHitBox)
	{
		// For now just make the hit box 20% larger than our dims...

        LTVector vHitDims = vNewDims;
		vHitDims *= 1.2f;

		g_pLTServer->SetObjectDims(m_hHitBox, &vHitDims);
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CreateHitBox()
//
//	PURPOSE:	Create our hit box
//
// ----------------------------------------------------------------------- //

void CCharacter::CreateHitBox()
{
	if (m_hHitBox) return;

    LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	HCLASS hClass = g_pLTServer->GetClass("CCharacterHitBox");
	if (!hClass) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_Pos = vPos;
	g_pLTServer->GetObjectRotation(m_hObject, &theStruct.m_Rotation);

	// Allocate an object...

	CCharacterHitBox* pHitBox = (CCharacterHitBox *)g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pHitBox) return;

	m_hHitBox = pHitBox->m_hObject;

	pHitBox->Init(m_hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateHitBox()
//
//	PURPOSE:	Update our hit box position
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateHitBox()
{
	if (!m_hHitBox) return;

	CCharacterHitBox* pHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(m_hHitBox);
	if (pHitBox)
	{
		pHitBox->Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SpawnItem()
//
//	PURPOSE:	Spawn the specified item
//
// ----------------------------------------------------------------------- //

void CCharacter::SpawnItem(char* pItem, LTVector & vPos, LTRotation & rRot)
{
	if (!pItem) return;

	LPBASECLASS pObj = SpawnObject(pItem, vPos, rRot);

	if (pObj && pObj->m_hObject)
	{
        LTVector vAccel;
		VEC_SET(vAccel, GetRandom(0.0f, 300.0f), GetRandom(100.0f, 200.0f), GetRandom(0.0f, 300.0f));
		g_pLTServer->SetAcceleration(pObj->m_hObject, &vAccel);

        LTVector vVel;
		VEC_SET(vVel, GetRandom(0.0f, 100.0f), GetRandom(200.0f, 400.0f), GetRandom(0.0f, 100.0f));
		g_pLTServer->SetVelocity(pObj->m_hObject, &vVel);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::StartDeath()
//
//	PURPOSE:	Start dying
//
// ----------------------------------------------------------------------- //

void CCharacter::StartDeath()
{
	if (m_bStartedDeath) return;

    m_bStartedDeath = LTTRUE;

	PlayDeathSound();

	// Spawn any special item we were instructed to

	if (m_hstrSpawnItem)
	{
		char* pItem = g_pLTServer->GetStringData(m_hstrSpawnItem);
		if (pItem)
		{
			// Add gravity to the item...

			char buf[300];
			sprintf(buf, "%s Gravity 1", pItem);

            LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);

            LTRotation rRot;
            rRot.Init();

			SpawnItem(buf, vPos, rRot);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateContainerCode()
//
//	PURPOSE:	Update our container code
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateContainerCode()
{
    LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	vPos += GetHeadOffset();

    m_eContainerCode = ::GetContainerCode(g_pLTServer, vPos);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetHeadOffset()
//
//	PURPOSE:	Update the offset from our position to our head
//
// ----------------------------------------------------------------------- //

LTVector CCharacter::GetHeadOffset()
{
    LTVector vOffset;
	VEC_INIT(vOffset);

    LTVector vDims;
	g_pLTServer->GetObjectDims(m_hObject, &vDims);

	// Just make the default offset a bit above the waist...

	vOffset.y = vDims.y * 0.75f;

	return vOffset;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetLastMoveInfo()
//
//	PURPOSE:	Set the last Move info
//
// ----------------------------------------------------------------------- //

void CCharacter::SetLastMoveInfo(CharMoveInfo* pInfo)
{
	if (!pInfo) return;

	m_LastMoveInfo.fTime		= pInfo->fTime;
	m_LastMoveInfo.eSurfaceType	= pInfo->eSurfaceType;
	m_LastMoveInfo.fVolume		= pInfo->fVolume;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetLastMoveInfo()
//
//	PURPOSE:	Get the last Move info
//
// ----------------------------------------------------------------------- //

void CCharacter::GetLastMoveInfo(CharMoveInfo & info)
{
	info.fTime			= m_LastMoveInfo.fTime;
	info.eSurfaceType	= m_LastMoveInfo.eSurfaceType;
	info.fVolume		= m_LastMoveInfo.fVolume;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetLastCoinInfo()
//
//	PURPOSE:	Set the last Coin info
//
// ----------------------------------------------------------------------- //

void CCharacter::SetLastCoinInfo(CharCoinInfo* pInfo)
{
	if (!pInfo) return;

	m_LastCoinInfo.fTime		= pInfo->fTime;
	m_LastCoinInfo.eSurfaceType	= pInfo->eSurfaceType;
	m_LastCoinInfo.fVolume		= pInfo->fVolume;
	m_LastCoinInfo.vPosition	= pInfo->vPosition;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetLastCoinInfo()
//
//	PURPOSE:	Get the last Coin info
//
// ----------------------------------------------------------------------- //

void CCharacter::GetLastCoinInfo(CharCoinInfo & info)
{
	info.fTime			= m_LastCoinInfo.fTime;
	info.eSurfaceType	= m_LastCoinInfo.eSurfaceType;
	info.fVolume		= m_LastCoinInfo.fVolume;
	info.vPosition		= m_LastCoinInfo.vPosition;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetLastFireInfo()
//
//	PURPOSE:	Set the last fire info
//
// ----------------------------------------------------------------------- //

void CCharacter::SetLastFireInfo(CharFireInfo* pInfo)
{
	if (!pInfo) return;

	if ( m_LastFireInfo.hObject )
	{
		g_pLTServer->BreakInterObjectLink(m_hObject, m_LastFireInfo.hObject);
	}

	if ( pInfo->hObject )
	{
		g_pLTServer->CreateInterObjectLink(m_hObject, pInfo->hObject);
	}

	m_LastFireInfo.hObject		= pInfo->hObject;
	m_LastFireInfo.vFiredPos	= pInfo->vFiredPos;
	m_LastFireInfo.vImpactPos	= pInfo->vImpactPos;
	m_LastFireInfo.nWeaponId	= pInfo->nWeaponId;
	m_LastFireInfo.nAmmoId		= pInfo->nAmmoId;
	m_LastFireInfo.fTime		= pInfo->fTime;
	m_LastFireInfo.bSilenced	= pInfo->bSilenced;
	m_LastFireInfo.eSurface		= pInfo->eSurface;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetLastFireInfo()
//
//	PURPOSE:	Get the last fire info
//
// ----------------------------------------------------------------------- //

void CCharacter::GetLastFireInfo(CharFireInfo & info)
{
	info.hObject	= m_LastFireInfo.hObject;
	info.vFiredPos  = m_LastFireInfo.vFiredPos;
	info.vImpactPos = m_LastFireInfo.vImpactPos;
	info.nWeaponId	= m_LastFireInfo.nWeaponId;
	info.nAmmoId	= m_LastFireInfo.nAmmoId;
	info.fTime		= m_LastFireInfo.fTime;
	info.bSilenced	= m_LastFireInfo.bSilenced;
	info.eSurface	= m_LastFireInfo.eSurface;
}

LTBOOL FnSaveFootprintList(HMESSAGEWRITE hWrite, void* pPtDataItem)
{
	CharFootprintInfo** ppFootprint = (CharFootprintInfo**)pPtDataItem;
	(*ppFootprint)->Save(hWrite);
    return LTTRUE;
}

LTBOOL FnLoadFootprintList(HMESSAGEREAD hRead, void* pPtDataItem)
{
	CharFootprintInfo* pFootprint = s_bankCharFootprintInfo.New();
	*((CharFootprintInfo**)pPtDataItem) = pFootprint;
	pFootprint->Load(hRead);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CCharacter::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    m_cs.Write(g_pLTServer, hWrite);

	m_LastFireInfo.Save(hWrite);
	m_LastMoveInfo.Save(hWrite);

    m_listFootprints.Save(g_pLTServer, hWrite, FnSaveFootprintList);

	g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hHitBox);

	g_pLTServer->WriteToMessageFloat(hWrite, m_fBodyLifetime);
	g_pLTServer->WriteToMessageByte(hWrite, m_bMoveToFloor);
	g_pLTServer->WriteToMessageByte(hWrite, m_eDeathType);
	g_pLTServer->WriteToMessageByte(hWrite, m_bStartedDeath);
	g_pLTServer->WriteToMessageByte(hWrite, m_bRolling);
	g_pLTServer->WriteToMessageByte(hWrite, m_bPivoting);
	g_pLTServer->WriteToMessageByte(hWrite, m_bAllowRun);
	g_pLTServer->WriteToMessageByte(hWrite, m_bAllowMovement);
	g_pLTServer->WriteToMessageByte(hWrite, m_eStandingOnSurface);
	g_pLTServer->WriteToMessageByte(hWrite, m_bOnGround);
	g_pLTServer->WriteToMessageByte(hWrite, m_bSpectatorMode);
	g_pLTServer->WriteToMessageByte(hWrite, m_eContainerCode);
	g_pLTServer->WriteToMessageByte(hWrite, m_eLastContainerCode);
	g_pLTServer->WriteToMessageByte(hWrite, m_bBodyInLiquid);
	g_pLTServer->WriteToMessageByte(hWrite, m_bBodyWasInLiquid);
	g_pLTServer->WriteToMessageByte(hWrite, m_bBodyOnLadder);
	g_pLTServer->WriteToMessageVector(hWrite, &m_vOldCharacterColor);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fOldCharacterAlpha);
	g_pLTServer->WriteToMessageByte(hWrite, m_bCharacterHadShadow);
	g_pLTServer->WriteToMessageByte(hWrite, m_eModelNodeLastHit);
	g_pLTServer->WriteToMessageByte(hWrite, m_bLeftFoot);
	g_pLTServer->WriteToMessageByte(hWrite, m_eModelId);
	g_pLTServer->WriteToMessageByte(hWrite, m_eModelSkeleton);
	g_pLTServer->WriteToMessageByte(hWrite, m_eModelStyle);
	g_pLTServer->WriteToMessageHString(hWrite, m_hstrSpawnItem);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fDefaultHitPts);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fDefaultArmor);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fSoundRadius);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fBaseMoveAccel);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fLadderVel);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fSwimVel);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fRunVel);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fWalkVel);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fRollVel);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fJumpVel);
	g_pLTServer->WriteToMessageByte(hWrite, m_bUsingHitDetection);
	g_pLTServer->WriteToMessageByte(hWrite, m_bCanPlayDialogSound);
	g_pLTServer->WriteToMessageByte(hWrite, m_bCanDamageBody);
	g_pLTServer->WriteToMessageByte(hWrite, m_cc);
	g_pLTServer->WriteToMessageByte(hWrite, m_ccCrosshair);
	g_pLTServer->WriteToMessageDWord(hWrite, m_dwFlags);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fLastPainTime);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fLastPainVolume);

	SAVE_VECTOR(m_vLastVolumePos);
	SAVE_BOOL(m_byFXFlags);
	SAVE_BOOL(m_bShortRecoiling);
	SAVE_HSTRING(m_hstrHeadExtension);
	SAVE_DWORD(m_cSpears);
	for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
	{
		SAVE_HOBJECT(m_aSpears[iSpear].hObject);
		SAVE_DWORD(m_aSpears[iSpear].eModelNode);
		SAVE_ROTATION(m_aSpears[iSpear].rRot);
	}

	SAVE_DWORD(m_iRecoilAnimTracker);
	SAVE_DWORD(m_iBlinkAnimTracker);
	SAVE_BOOL(m_bWallStick);
	SAVE_INT(m_cActive);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CCharacter::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

    m_cs.Read(g_pLTServer, hRead);

	m_LastFireInfo.Load(hRead);
	m_LastMoveInfo.Load(hRead);

    m_listFootprints.Load(g_pLTServer, hRead, FnLoadFootprintList);

	g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hHitBox);

	m_fBodyLifetime			= g_pLTServer->ReadFromMessageFloat(hRead);
	m_bMoveToFloor			= g_pLTServer->ReadFromMessageByte(hRead);
	m_eDeathType			= (CharacterDeath) g_pLTServer->ReadFromMessageByte(hRead);
	m_bStartedDeath			= g_pLTServer->ReadFromMessageByte(hRead);
	m_bRolling				= g_pLTServer->ReadFromMessageByte(hRead);
	m_bPivoting				= g_pLTServer->ReadFromMessageByte(hRead);
	m_bAllowRun				= g_pLTServer->ReadFromMessageByte(hRead);
	m_bAllowMovement		= g_pLTServer->ReadFromMessageByte(hRead);
	m_eStandingOnSurface	= (SurfaceType) g_pLTServer->ReadFromMessageByte(hRead);
	m_bOnGround				= g_pLTServer->ReadFromMessageByte(hRead);
	m_bSpectatorMode		= g_pLTServer->ReadFromMessageByte(hRead);
	m_eContainerCode		= (ContainerCode) g_pLTServer->ReadFromMessageByte(hRead);
	m_eLastContainerCode	= (ContainerCode) g_pLTServer->ReadFromMessageByte(hRead);
	m_bBodyInLiquid			= g_pLTServer->ReadFromMessageByte(hRead);
	m_bBodyWasInLiquid		= g_pLTServer->ReadFromMessageByte(hRead);
	m_bBodyOnLadder			= g_pLTServer->ReadFromMessageByte(hRead);
	g_pLTServer->ReadFromMessageVector(hRead, &m_vOldCharacterColor);
	m_fOldCharacterAlpha	= g_pLTServer->ReadFromMessageFloat(hRead);
	m_bCharacterHadShadow	= g_pLTServer->ReadFromMessageByte(hRead);
	m_eModelNodeLastHit		= (ModelNode) g_pLTServer->ReadFromMessageByte(hRead);
	m_bLeftFoot				= g_pLTServer->ReadFromMessageByte(hRead);
	m_eModelId				= (ModelId) g_pLTServer->ReadFromMessageByte(hRead);
	m_eModelSkeleton		= (ModelSkeleton) g_pLTServer->ReadFromMessageByte(hRead);
	m_eModelStyle			= (ModelStyle) g_pLTServer->ReadFromMessageByte(hRead);
	m_hstrSpawnItem			= g_pLTServer->ReadFromMessageHString(hRead);
	m_fDefaultHitPts		= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fDefaultArmor			= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fSoundRadius			= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fBaseMoveAccel		= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fLadderVel			= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fSwimVel				= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fRunVel				= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fWalkVel				= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fRollVel				= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fJumpVel				= g_pLTServer->ReadFromMessageFloat(hRead);
    m_bUsingHitDetection    = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bCanPlayDialogSound   = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bCanDamageBody        = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
	m_cc					= (CharacterClass) g_pLTServer->ReadFromMessageByte(hRead);
	m_ccCrosshair			= (CharacterClass) g_pLTServer->ReadFromMessageByte(hRead);
	m_dwFlags				= g_pLTServer->ReadFromMessageDWord(hRead);
	m_fLastPainTime			= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fLastPainVolume		= g_pLTServer->ReadFromMessageFloat(hRead);
	m_iLastVolume			= -1;

	LOAD_VECTOR(m_vLastVolumePos);
	LOAD_BOOL(m_byFXFlags);
	LOAD_BOOL(m_bShortRecoiling);
	LOAD_HSTRING(m_hstrHeadExtension);

	LOAD_DWORD(m_cSpears);
	for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
	{
		LOAD_HOBJECT(m_aSpears[iSpear].hObject);
		LOAD_DWORD_CAST(m_aSpears[iSpear].eModelNode, ModelNode);
		LOAD_ROTATION(m_aSpears[iSpear].rRot);
	}

	LOAD_DWORD(m_iRecoilAnimTracker);
	LOAD_DWORD(m_iBlinkAnimTracker);
	LOAD_BOOL(m_bWallStick);
	LOAD_INT(m_cActive);

	// Make sure this object is added to the global CharacterMgr...

//	g_pCharacterMgr->Add(this);

	InitAnimation();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::DestroyAttachments
//
//	PURPOSE:	Destroys our attachments aggregate
//
// ----------------------------------------------------------------------- //

void CCharacter::DestroyAttachments()
{
	if ( m_pAttachments )
	{
		CAttachments::Destroy(m_pAttachments);
        m_pAttachments = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::TransferAttachments
//
//	PURPOSE:	Transfer our attachments aggregate (i.e., clear our
//				attachments aggregate, but don't remove it)
//
// ----------------------------------------------------------------------- //

CAttachments* CCharacter::TransferAttachments()
{
	if (m_pAttachments)
	{
		m_pAttachments->HandleDeath();
	}

	CAttachments* pAtt = m_pAttachments;
	if (m_pAttachments)
	{
		RemoveAggregate(m_pAttachments);
        m_pAttachments = LTNULL;
	}

	return pAtt;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetWeaponPowerup
//
//	PURPOSE:	Finds the name of a weapon and the socket if we have one
//
// ----------------------------------------------------------------------- //

HOBJECT CCharacter::TransferWeapon(HOBJECT hBody)
{
	if ( m_pAttachments )
	{
		CWeapon* apWeapons[1];
		CAttachmentPosition* apAttachmentPositions[1];
		uint32 cWeapons = m_pAttachments->EnumerateWeapons(apWeapons, apAttachmentPositions, 1);

		if ( cWeapons )
		{
			char szSpawn[1024];
			apAttachmentPositions[0]->GetAttachment()->CreateSpawnString(szSpawn);

            BaseClass* pObj = SpawnObject(szSpawn, LTVector(-10000,-10000,-10000), LTRotation(0,0,0,1));
			if ( !pObj || !pObj->m_hObject ) return LTNULL;

			HATTACHMENT hAttachment;
			if ( LT_OK != g_pLTServer->CreateAttachment(hBody, pObj->m_hObject, (char *)apAttachmentPositions[0]->GetName(), &LTVector(0,0,0), &LTRotation(0,0,0,1), &hAttachment) )
			{
				g_pLTServer->RemoveObject(pObj->m_hObject);
				return LTNULL;
			}

			g_pLTServer->CreateInterObjectLink(hBody, pObj->m_hObject);

			apAttachmentPositions[0]->RemoveWeapon();

			return pObj->m_hObject;

		}
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::TransferSpears
//
//	PURPOSE:	Transfer our attachments aggregate (i.e., clear our
//				attachments aggregate, but don't remove it)
//
// ----------------------------------------------------------------------- //

void CCharacter::TransferSpears(Body* pBody)
{
	for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
	{
		HATTACHMENT hAttachment;
		HOBJECT hSpear = m_aSpears[iSpear].hObject;

		if ( hSpear )
		{
			if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hSpear, &hAttachment) )
			{
				if ( LT_OK == g_pLTServer->RemoveAttachment(hAttachment) )
				{
					// Attach it to the body prop and break our link

					pBody->AddSpear(hSpear, m_aSpears[iSpear].rRot, m_aSpears[iSpear].eModelNode);

					g_pLTServer->BreakInterObjectLink(m_hObject, hSpear);
					m_aSpears[iSpear].hObject= LTNULL;

					continue;
				}
			}

			// If any of this failed, just remove the spear

			g_pLTServer->RemoveObject(hSpear);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetLastVolume
//
//	PURPOSE:	Gets the last attachment we were standing in
//
// ----------------------------------------------------------------------- //

CAIVolume* CCharacter::GetLastVolume()
{
	if ( g_pAIVolumeMgr->IsInitialized() && m_iLastVolume != -1)
	{
		return g_pAIVolumeMgr->GetVolumeByIndex(m_iLastVolume);
	}
	else
	{
        return LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HitFromFront
//
//	PURPOSE:	Tells whether the vector is coming at us from the front
//				or back assuming it passes through us.
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::HitFromFront(const LTVector& vDir)
{
    LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);
    LTVector vNull, vForward;
	g_pMathLT->GetRotationVectors(rRot, vNull, vNull, vForward);
	return vDir.Dot(vForward) < 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ComputeDamageModifier
//
//	PURPOSE:	Adjust the amount of damage based on the node hit...
//
// ----------------------------------------------------------------------- //

LTFLOAT CCharacter::ComputeDamageModifier(ModelNode eModelNode)
{
	LTFLOAT fModifier = g_pModelButeMgr->GetSkeletonNodeDamageFactor(m_eModelSkeleton, eModelNode);
	return fModifier;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::AddSpear
//
//	PURPOSE:	Stick a spear into us
//
// ----------------------------------------------------------------------- //

void CCharacter::AddSpear(HOBJECT hSpear, ModelNode eModelNode, const LTRotation& rRot)
{
	// g_pLTServer->CPrint("-----------------------------------");
	if ( m_cSpears < kMaxSpears )
	{
		char* szNode = (char *)g_pModelButeMgr->GetSkeletonNodeName(m_eModelSkeleton, eModelNode);

		// Get the node transform because we need to make rotation relative

		HMODELNODE hNode;
		if ( szNode && LT_OK == g_pModelLT->GetNode(m_hObject, szNode, hNode) )
		{
			LTransform transform;
			if ( LT_OK == g_pModelLT->GetNodeTransform(m_hObject, hNode, transform, LTTRUE) )
			{
				LTRotation rRotNode;
				if ( LT_OK == g_pTransLT->GetRot(transform, rRotNode) )
				{
					LTRotation rAttachment = ~rRotNode*rRot;
					LTVector vAttachment, vNull;
					g_pMathLT->GetRotationVectors(rAttachment, vNull, vNull, vAttachment);
					vAttachment *= -16.0f;

					HATTACHMENT hAttachment;
					if ( LT_OK == g_pLTServer->CreateAttachment(m_hObject, hSpear, szNode, &vAttachment, &rAttachment, &hAttachment) )
					{
						g_pLTServer->CreateInterObjectLink(m_hObject, hSpear);
						m_aSpears[m_cSpears].hObject = hSpear;
						m_aSpears[m_cSpears].eModelNode = eModelNode;
						m_aSpears[m_cSpears].rRot = rRot;

						m_cSpears++;

						if ( NODEFLAG_WALLSTICK & g_pModelButeMgr->GetSkeletonNodeFlags(m_eModelSkeleton, eModelNode) )
						{
							// Try to stick us to a wall on the next damage msg.
							// TODO: this might be a bit dangerous... can't really think of a better way to do this right now.
							// Only stick if projectile forward is within threshhold of character forward

							LTRotation rCharacterRot;
							LTVector vNull, vSpearForward, vCharacterForward;

							g_pLTServer->GetObjectRotation(m_hObject, &rCharacterRot);
							g_pMathLT->GetRotationVectors(rCharacterRot, vNull, vNull, vCharacterForward);
							g_pMathLT->GetRotationVectors((LTRotation&)rRot, vNull, vNull, vSpearForward);

							// TODO: bute -normal/fdw dp threshhold
							if ( -vSpearForward.Dot(vCharacterForward) > s_BodyStickAngle.GetFloat() )
							{
								//g_pLTServer->CPrint("Spear dot charfwd < -%.2f", s_BodyStickAngle.GetFloat());
								m_bWallStick = LTTRUE;
							}
						}

						return;
					}
				}
			}
		}
	}

	// Unless we actually stuck the spear into us, we'll fall through into here.

	g_pLTServer->RemoveObject(hSpear);
}

void CCharacter::SetBlinking(LTBOOL bBlinking)
{
    g_pModelLT->SetWeightSet(&m_BlinkAnimTracker, bBlinking ? m_hBlinkWeightset : m_hNullWeightset);
}