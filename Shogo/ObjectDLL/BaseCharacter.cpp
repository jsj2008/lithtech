// ----------------------------------------------------------------------- //
//
// MODULE  : BaseCharacter.cpp
//
// PURPOSE : Base class for player and AI
//
// CREATED : 10/6/97
//
// ----------------------------------------------------------------------- //

#include "BaseCharacter.h"
#include "cpp_server_de.h"
#include "RiotObjectUtilities.h"
#include "generic_msg_de.h"
#include "stdio.h"
#include "BodyProp.h"
#include "VolumeBrush.h"
#include "Spawner.h"
#include "PVWeaponModel.h"
#include "SurfaceFunctions.h"
#include "ModelNodes.h"
#include "CharacterMgr.h"
#include "CVarTrack.h"


BEGIN_CLASS(CBaseCharacter)
	ADD_ACTIVATION_AGGREGATE()
	ADD_DESTRUCTABLE_AGGREGATE()
	ADD_REALPROP(HitPoints, -1.0f)
	ADD_REALPROP(ArmorPoints, -1.0f)
	ADD_BOOLPROP(Small, 0)
	ADD_BOOLPROP(MoveToFloor, DTRUE)
	ADD_BOOLPROP(ShowDeadBody, DTRUE)
	ADD_STRINGPROP(SpawnItem, "")
END_CLASS_DEFAULT_FLAGS(CBaseCharacter, BaseClass, NULL, NULL, CF_HIDDEN)

#define KEY_FOOTSTEP_SOUND		"FOOTSTEP_KEY"
#define KEY_SET_DIMS			"SETDIMS"
#define KEY_MOVE				"MOVE"
#define KEY_PLAYSOUND			"PLAYSOUND"
#define TRIGGER_PLAY_SOUND		"PLAYSOUND"

#define DIALOG_SPRITE_OFFSET		15.0f

#define DEFAULT_SOUND_RADIUS		2000.0f
#define FOOTSTEP_SOUND_RADIUS		1000.0f
#define DEFAULT_LADDER_VEL			400.0f
#define DEFAULT_SWIM_VEL			175.0f
#define DEFAULT_RUN_VEL				100.0f
#define DEFAULT_WALK_VEL			60.0f
#define DEFAULT_JUMP_VEL			50.0f
#define DEFAULT_MOVE_ACCEL			1500.0f

#define BIGGUNS_SCALE_VALUE			3.0f
#define DIMS_EPSILON				0.5f
#define MAX_NUM_SQUEAKY_HITS		5
#define FALL_LANDING_TIME			0.5f


// Default animation names...

#define ANIM_WALK_KNIFE						"WK"
#define ANIM_WALK_KNIFE_ATTACK				"WAK"
#define ANIM_WALK_BKNIFE					"WKB"
#define ANIM_WALK_BKNIFE_ATTACK				"WAKB"
#define ANIM_WALK_STRAFEL_KNIFE				"WSLK"
#define ANIM_WALK_STRAFER_KNIFE				"WSRK"
#define ANIM_WALK_STRAFEL_KNIFE_ATTACK		"WSLAK"
#define ANIM_WALK_STRAFER_KNIFE_ATTACK		"WSRAK"

#define ANIM_RUN_KNIFE						"RK"
#define ANIM_RUN_KNIFE_ATTACK				"RAK"
#define ANIM_RUN_BKNIFE						"RKB"
#define ANIM_RUN_BKNIFE_ATTACK				"RAKB"
#define ANIM_RUN_STRAFEL_KNIFE				"RSLK"
#define ANIM_RUN_STRAFER_KNIFE				"RSRK"
#define ANIM_RUN_STRAFEL_KNIFE_ATTACK		"RSLAK"
#define ANIM_RUN_STRAFER_KNIFE_ATTACK		"RSRAK"

#define ANIM_CROUCH_KNIFE					"CK"
#define ANIM_CROUCH_KNIFE_ATTACK			"CAK"
#define ANIM_CROUCH_WALK_KNIFE				"CWK"
#define ANIM_CROUCH_WALK_KNIFE_ATTACK		"CWAK"
#define ANIM_CROUCH_WALK_BKNIFE				"CWKB"
#define ANIM_CROUCH_WALK_BKNIFE_ATTACK		"CWAKB"
#define ANIM_CROUCH_STRAFEL_KNIFE			"CSLK"
#define ANIM_CROUCH_STRAFER_KNIFE			"CSRK"
#define ANIM_CROUCH_STRAFEL_KNIFE_ATTACK	"CSLAK"
#define ANIM_CROUCH_STRAFER_KNIFE_ATTACK	"CSRAK"

#define ANIM_STAND_RIFLE_ATTACK				"SR"
#define ANIM_STAND_KNIFE_ATTACK				"SK"

#define ANIM_WALK_RIFLE						"WR"
#define ANIM_WALK_RIFLE_ATTACK				"WR"
#define ANIM_WALK_BRIFLE					"WRB"
#define ANIM_WALK_BRIFLE_ATTACK				"WRB"
#define ANIM_WALK_STRAFEL_RIFLE				"WSLR"
#define ANIM_WALK_STRAFER_RIFLE				"WSRR"
#define ANIM_WALK_STRAFEL_RIFLE_ATTACK		"WSLR"
#define ANIM_WALK_STRAFER_RIFLE_ATTACK		"WSRR"

#define ANIM_RUN_RIFLE						"RR"
#define ANIM_RUN_RIFLE_ATTACK				"RR"
#define ANIM_RUN_BRIFLE						"RRB"
#define ANIM_RUN_BRIFLE_ATTACK				"RRB"
#define ANIM_RUN_STRAFEL_RIFLE				"RSLR"
#define ANIM_RUN_STRAFER_RIFLE				"RSRR"
#define ANIM_RUN_STRAFEL_RIFLE_ATTACK		"RSLR"
#define ANIM_RUN_STRAFER_RIFLE_ATTACK		"RSRR"


#define ANIM_CROUCH_RIFLE					"CR"
#define ANIM_CROUCH_RIFLE_ATTACK			"CR"
#define ANIM_CROUCH_WALK_RIFLE				"CWR"
#define ANIM_CROUCH_WALK_RIFLE_ATTACK		"CWR"
#define ANIM_CROUCH_WALK_BRIFLE				"CWRB"
#define ANIM_CROUCH_WALK_BRIFLE_ATTACK		"CWRB"
#define ANIM_CROUCH_STRAFEL_RIFLE			"CSLR"
#define ANIM_CROUCH_STRAFER_RIFLE			"CSRR"
#define ANIM_CROUCH_STRAFEL_RIFLE_ATTACK	"CSLR"
#define ANIM_CROUCH_STRAFER_RIFLE_ATTACK	"CSRR"

#define ANIM_POSE1							"POSE1"
#define ANIM_POSE2							"POSE2"

#define ANIM_IDLE_RIFLE1					"IR1"
#define ANIM_IDLE_RIFLE2					"IR2"
#define ANIM_IDLE_RIFLE3					"IR3"
#define ANIM_IDLE_RIFLE4					"IR4"
#define ANIM_IDLE_RIFLE5					"IR5"

#define ANIM_IDLE_KNIFE1					"IK1"
#define ANIM_IDLE_KNIFE2					"IK2"

#define ANIM_JUMP_UP						"JUMP_UP"
#define ANIM_JUMP_DOWN						"JUMP_DOWN"
#define ANIM_LANDING						"LANDING"

#define ANIM_TRANSFORM						"TRANSFORM"
#define ANIM_INVERSE_TRANSFORM				"INVERSE_TRANSFORM"
#define ANIM_VEHICLE						"VEHICLE"

#define ANIM_SWIM							"SWIM"



// Globals (save space) used for parsing messages (used in sub classes as well)...
// g_pCommandPos is global to make sure the command position is correctly
// updated in multiple calls to Parse()

char g_tokenSpace[PARSE_MAXTOKENS*PARSE_MAXTOKENSIZE];
char *g_pTokens[PARSE_MAXTOKENS];
char *g_pCommandPos;



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::CBaseCharacter()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CBaseCharacter::CBaseCharacter() : BaseClass(OT_MODEL) 
{
	AddAggregate(&m_damage);
	AddAggregate(&m_weapons);
	AddAggregate(&m_inventory);
	AddAggregate(&m_activation);

	memset (m_powerups, 0, MAX_TIMED_POWERUPS * sizeof (TimedPowerup*));
	
	m_cc						= UNKNOWN;

	m_dwControlFlags			= 0;
	m_dwLastFrameCtlFlgs		= 0;
	m_dwFlags					= FLAG_STAIRSTEP | FLAG_SHADOW | FLAG_TOUCH_NOTIFY | FLAG_SOLID | FLAG_GRAVITY |
								  FLAG_MODELKEYS | FLAG_MODELGOURAUDSHADE | FLAG_RAYHIT | FLAG_VISIBLE | FLAG_ANIMTRANSITION;

	m_PStateChangeFlags = PSTATE_ALL;

	m_fLadderVel				= DEFAULT_LADDER_VEL;
	m_fSwimVel					= DEFAULT_SWIM_VEL;
	m_fRunVel					= DEFAULT_RUN_VEL;
	m_fWalkVel					= DEFAULT_WALK_VEL;
	m_fJumpVel					= DEFAULT_JUMP_VEL;
	m_fBaseMoveAccel			= DEFAULT_MOVE_ACCEL;
	m_fTimeInAir				= 0.0;
	m_dwLastHitNode				= -1;
	m_bUsingHitDetection		= DTRUE;
	m_bCreateBody				= DTRUE;
	m_bCreateDialogSprite		= DTRUE;

	m_fDimsScale[0] = 1.0f;
	m_fDimsScale[1] = 0.2f;
	m_fDimsScale[2] = 5.0f;
 
	m_fSoundRadius				= DEFAULT_SOUND_RADIUS;
	m_nBasePriority				= SOUNDPRIORITYBASE_AI;
	
	m_bCreateHandHeldWeapon		= DTRUE;
	m_eModelSize				= MS_NORMAL;
	m_bIsMecha					= DFALSE;
	m_bTransforming				= DFALSE;
	m_bLanding					= DFALSE;
	m_bOnGround					= DTRUE;
	m_bLastOnGround				= DTRUE;
	m_bAllowRun					= DTRUE;
	m_bAllowMovement			= DTRUE;
	m_bBipedalLastFrame			= DTRUE;
	m_bBipedal					= DTRUE;
	m_bSpectatorMode			= DFALSE;
	m_eContainerCode			= CC_NONE;
	m_eLastContainerCode		= CC_NONE;
	m_bBodyInLiquid				= DFALSE;
	m_bBodyWasInLiquid			= DFALSE;
	m_bBodyOnLadder				= DFALSE;
	m_bJumping					= DFALSE;
	m_bJumped					= DFALSE;
	m_bRecoiling				= DFALSE;
	m_bLeftFoot					= DTRUE;
	m_bOneHandedWeapon			= DFALSE;

	m_hWalkKnifeAni					= INVALID_ANI;
	m_hWalkKnifeAttackAni			= INVALID_ANI;
	m_hWalkBKnifeAni				= INVALID_ANI;
	m_hWalkBKnifeAttackAni			= INVALID_ANI;
	m_hWalkStrafeLKnifeAni			= INVALID_ANI;
	m_hWalkStrafeRKnifeAni			= INVALID_ANI;
	m_hWalkStrafeLKnifeAttackAni	= INVALID_ANI;
	m_hWalkStrafeRKnifeAttackAni	= INVALID_ANI;
	m_hRunKnifeAni					= INVALID_ANI;
	m_hRunKnifeAttackAni			= INVALID_ANI;
	m_hRunBKnifeAni					= INVALID_ANI;
	m_hRunBKnifeAttackAni			= INVALID_ANI;
	m_hRunStrafeLKnifeAni			= INVALID_ANI;
	m_hRunStrafeRKnifeAni			= INVALID_ANI;
	m_hRunStrafeLKnifeAttackAni		= INVALID_ANI;
	m_hRunStrafeRKnifeAttackAni		= INVALID_ANI;
	m_hCrouchKnifeAni				= INVALID_ANI;
	m_hCrouchKnifeAttackAni			= INVALID_ANI;
	m_hCrouchWalkKnifeAni			= INVALID_ANI;
	m_hCrouchWalkKnifeAttackAni		= INVALID_ANI;
	m_hCrouchWalkBKnifeAni			= INVALID_ANI;
	m_hCrouchWalkBKnifeAttackAni	= INVALID_ANI;
	m_hCrouchStrafeLKnifeAni		= INVALID_ANI;
	m_hCrouchStrafeRKnifeAni		= INVALID_ANI;
	m_hCrouchStrafeLKnifeAttackAni	= INVALID_ANI;
	m_hCrouchStrafeRKnifeAttackAni	= INVALID_ANI;
	m_hWalkRifleAni					= INVALID_ANI;
	m_hWalkRifleAttackAni			= INVALID_ANI;
	m_hWalkBRifleAni				= INVALID_ANI;
	m_hWalkBRifleAttackAni			= INVALID_ANI;
	m_hWalkStrafeLRifleAni			= INVALID_ANI;
	m_hWalkStrafeRRifleAni			= INVALID_ANI;
	m_hWalkStrafeLRifleAttackAni	= INVALID_ANI;
	m_hWalkStrafeRRifleAttackAni	= INVALID_ANI;
	m_hRunRifleAni					= INVALID_ANI;
	m_hRunRifleAttackAni			= INVALID_ANI;
	m_hRunBRifleAni					= INVALID_ANI;
	m_hRunBRifleAttackAni			= INVALID_ANI;
	m_hRunStrafeLRifleAni			= INVALID_ANI;
	m_hRunStrafeRRifleAni			= INVALID_ANI;
	m_hRunStrafeLRifleAttackAni		= INVALID_ANI;
	m_hRunStrafeRRifleAttackAni		= INVALID_ANI;
	m_hCrouchRifleAni				= INVALID_ANI;
	m_hCrouchRifleAttackAni			= INVALID_ANI;
	m_hCrouchWalkRifleAni			= INVALID_ANI;
	m_hCrouchWalkRifleAttackAni		= INVALID_ANI;
	m_hCrouchWalkBRifleAni			= INVALID_ANI;
	m_hCrouchWalkBRifleAttackAni	= INVALID_ANI;
	m_hCrouchStrafeLRifleAni		= INVALID_ANI;
	m_hCrouchStrafeRRifleAni		= INVALID_ANI;
	m_hCrouchStrafeLRifleAttackAni	= INVALID_ANI;
	m_hCrouchStrafeRRifleAttackAni	= INVALID_ANI;
	m_hPose1Ani						= INVALID_ANI;
	m_hPose2Ani						= INVALID_ANI;
	m_hRifleIdleAni[0]				= INVALID_ANI;
	m_hRifleIdleAni[1]				= INVALID_ANI;
	m_hRifleIdleAni[2]				= INVALID_ANI;
	m_hRifleIdleAni[3]				= INVALID_ANI;
	m_hRifleIdleAni[4]				= INVALID_ANI;
	m_hKnifeIdle1Ani				= INVALID_ANI;
	m_hKnifeIdle2Ani				= INVALID_ANI;
	m_hJumpUpAni					= INVALID_ANI;
	m_hJumpDownAni					= INVALID_ANI;
	m_hLandingAni					= INVALID_ANI;
	m_hTransformAni					= INVALID_ANI;
	m_hInverseTransformAni			= INVALID_ANI;
	m_hVehicleAni					= INVALID_ANI;
	m_hSwimAni						= INVALID_ANI;
	m_hTearsAni						= INVALID_ANI;
	m_hStandKnifeAttackAni			= INVALID_ANI;
	m_hStandRifleAttackAni			= INVALID_ANI;

	VEC_INIT(m_vOldCharacterColor);
	m_fOldCharacterAlpha		= 1.0f;
	m_bCharacterHadShadow		= DFALSE;
	
	m_pVehicleSound				= "Sounds\\Player\\Vehicle_idle.wav";

	m_bMoveToFloor				= DTRUE;

	m_hDlgSprite				= DNULL;
	m_bCanPlayDialogSound		= DTRUE;
	m_bCanDamageBody			= DTRUE;

	m_hstrSpawnItem				= DNULL;

	// Debug bounding box...

	m_hBoundingBox				= DNULL;

	m_hHandHeldWeapon			= DNULL;
	m_pHandName					= "GUN_HAND";

	m_hVehicleSound				= DNULL;
	m_hCurDlgSnd				= DNULL;
	m_eCurDlgSndType			= CST_NONE;

	m_bStartedDeath				= DFALSE;
	m_eDeathType				= CD_NORMAL;
	m_bSpawnWeapon				= DTRUE;

	m_nModelId					= MI_UNDEFINED;

	VEC_INIT(m_vLastFiredPos);
	VEC_INIT(m_vLastImpactPos);
	m_nLastFiredWeapon			= GUN_NONE;
	m_fLastFiredTime			= 0.0f;
	m_bLastFireSilenced			= DFALSE;

	m_fDefaultHitPts			= -1.0f;
	m_fDefaultArmor				= -1.0f;

	// Used for humiliation!

	m_bTakesSqueakyDamage		= DFALSE;
	m_nSqueakyCount				= 0;
	m_bCrying					= DFALSE;

	
	m_fMoveMultiplier		= 1.0f;
	m_fJumpMultiplier		= 1.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Reset()
//
//	PURPOSE:	Reset (after death)
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::Reset()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	m_bStartedDeath		= DFALSE;
	m_eCurDlgSndType	= CST_NONE;
	m_fTimeInAir		= 0.0;

	m_dwControlFlags	= 0;

	// Since we were dead, we need to reset our solid flag...

	DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
	dwFlags |= FLAG_SOLID;
	pServerDE->SetObjectFlags(m_hObject, dwFlags);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::~CBaseCharacter()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CBaseCharacter::~CBaseCharacter()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	RemoveHandHeldWeapon();

	if (m_hVehicleSound)
	{
		pServerDE->KillSound(m_hVehicleSound);
	}
	
/*	if (m_hCurDlgSnd)
	{
		pServerDE->KillSound(m_hCurDlgSnd);
	}
*/
	KillDlgSnd( );

	if (m_hBoundingBox)
	{
		HATTACHMENT hAttachment;
		if (pServerDE->FindAttachment(m_hObject, m_hBoundingBox, &hAttachment) == DE_OK)
		{
			pServerDE->RemoveAttachment(hAttachment);
		}

		pServerDE->RemoveObject(m_hBoundingBox);
		m_hBoundingBox = DNULL;
	}

	if (m_hDlgSprite)
	{
		HATTACHMENT hAttachment;
		if (pServerDE->FindAttachment(m_hObject, m_hDlgSprite, &hAttachment) == DE_OK)
		{
			pServerDE->RemoveAttachment(hAttachment);
		}

		g_pServerDE->BreakInterObjectLink( m_hObject, m_hDlgSprite );
		pServerDE->RemoveObject(m_hDlgSprite);
		m_hDlgSprite = DNULL;
	}

	if (m_hstrSpawnItem)
	{
		pServerDE->FreeString(m_hstrSpawnItem);
	}

	RemoveAllPowerups();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CBaseCharacter::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
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

		case MID_PRECREATE:
		{
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
			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);
			InitialUpdate((int)fData);
			CacheFiles();
			return dwRet;
		}
		break;

		case MID_SAVEOBJECT:
		{
			// Let aggregates go first...

			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			Save((HMESSAGEWRITE)pData);

			return dwRet;
		}
		break;

		case MID_LOADOBJECT:
		{
			// Let aggregates go first...

			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			Load((HMESSAGEREAD)pData);

			return dwRet;
		}
		break;

		case MID_LINKBROKEN:
		{
			if( m_hDlgSprite == (HOBJECT)pData )
			{
				m_hDlgSprite = DNULL;
			}
			else if( m_hHandHeldWeapon == (HOBJECT)pData )
			{
				m_hHandHeldWeapon = DNULL;
			}
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::ObjectMessageFn()
//
//	PURPOSE:	Handler for object to object messages.
//
// --------------------------------------------------------------------------- //

DDWORD CBaseCharacter::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return 0;

	switch (messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);
			char *pMsg = pServerDE->GetStringData(hMsg);

			ProcessTriggerMsg(pMsg);

			pServerDE->FreeString(hMsg);
		}
		break;

		case MID_DAMAGE:
		{
			DDWORD dwRet = BaseClass::ObjectMessageFn(hSender, messageID, hRead);
			ProcessDamageMsg(hRead);
			return dwRet;
		}
	}
	
	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL CBaseCharacter::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return DFALSE;

	if ( pServerDE->GetPropGeneric( "Small", &genProp ) == DE_OK )
	{
		m_eModelSize = genProp.m_Bool ? MS_SMALL : m_eModelSize;
	}
	if ( pServerDE->GetPropGeneric( "MoveToFloor", &genProp ) == DE_OK )
	{
		m_bMoveToFloor = genProp.m_Bool;
	}
	if ( pServerDE->GetPropGeneric( "ShowDeadBody", &genProp ) == DE_OK )
	{
		m_bCreateBody = genProp.m_Bool;
	}

	if ( pServerDE->GetPropGeneric( "SpawnItem", &genProp ) == DE_OK )
	{
		if ( genProp.m_String[0] )
			m_hstrSpawnItem = pServerDE->CreateString( genProp.m_String );
	}

	if ( pServerDE->GetPropGeneric( "HitPoints", &genProp ) == DE_OK )
	{
		m_fDefaultHitPts = genProp.m_Float;
	}

	if ( pServerDE->GetPropGeneric( "ArmorPoints", &genProp ) == DE_OK )
	{
		m_fDefaultArmor = genProp.m_Float;
	}

	return DTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::ProcessTriggerMsg()
//
//	PURPOSE:	Process a trigger message messages.
//
// --------------------------------------------------------------------------- //

DBOOL CBaseCharacter::ProcessTriggerMsg(char* pMsg)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pMsg) return DFALSE;
	DBOOL bMore;

	int nArgs;

	char* pCommand = pMsg;
	bMore = DTRUE;
	while( bMore )
	{
		bMore = pServerDE->Parse(pCommand, &g_pCommandPos, g_tokenSpace, g_pTokens, &nArgs);
		ProcessCommand(g_pTokens, nArgs, g_pCommandPos);
		pCommand = g_pCommandPos;
	}

	return DFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::ProcessCommand()
//
//	PURPOSE:	Process a command
//
// --------------------------------------------------------------------------- //

DBOOL CBaseCharacter::ProcessCommand(char** pTokens, int nArgs, char* pNextCommand)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pTokens || !pTokens[0] || nArgs < 1) return DFALSE;

	if (stricmp(TRIGGER_PLAY_SOUND, pTokens[0]) == 0 && nArgs > 1)
	{
		// Get sound name from message...

		char* pSoundName = pTokens[1];

		if (pSoundName)
		{
			PlayDialogSound(pSoundName, CST_EXCLAMATION);
			return DTRUE;
		}
	}

	return DFALSE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::ProcessDamageMsg()
//
//	PURPOSE:	Process a damage message.
//
// --------------------------------------------------------------------------- //

void CBaseCharacter::ProcessDamageMsg(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	DVector vDir;
	pServerDE->ReadFromMessageVector(hRead, &vDir);
	DFLOAT fDamage   = pServerDE->ReadFromMessageFloat(hRead);
	DamageType eType = (DamageType)pServerDE->ReadFromMessageByte(hRead);
	HOBJECT hHeHitMe = pServerDE->ReadFromMessageObject(hRead);


	// Play a damage sound...

	if (!m_damage.IsDead() && m_damage.GetCanDamage())
	{
		// For now player's don't recoil...

		if (!IsPlayer(m_hObject))
		{
			SetRecoilAnimation();
		}

		// See if we should cry "like a little girl"

		if (eType == DT_SQUEAKY && m_bTakesSqueakyDamage)
		{
			HMODELANIM hCurAni = pServerDE->GetModelAnimation(m_hObject);

			if (m_hTearsAni != INVALID_ANI && hCurAni != m_hTearsAni)
			{
				if (++m_nSqueakyCount >= MAX_NUM_SQUEAKY_HITS)
				{
					SetTearsAnimation();
					m_nSqueakyCount = 0;
				}
			}
		}

		PlayDamageSound(eType);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::InitialUpdate()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //
	
void CBaseCharacter::InitialUpdate(int nInfo)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_activation.IsActive()) return;

	if (nInfo == INITIALUPDATE_SAVEGAME) return;


	// Make sure this object is added to the global CharacterMgr...

	CCharacterMgr* pCharMgr = CCharacterMgr::GetMgr();
	if (!pCharMgr) return;

	pCharMgr->Add(this);
	
	
	// Scale our model based on our model dims scale...

	DVector vScale;
	VEC_SET(vScale, 1.0f, 1.0f, 1.0f);
	VEC_MULSCALAR(vScale, vScale, m_fDimsScale[m_eModelSize]);
	pServerDE->ScaleObject(m_hObject, &vScale);

	pServerDE->SetObjectFlags(m_hObject, m_dwFlags);

	m_damage.Init(m_hObject);
	m_damage.SetMass(GetDefaultMass(m_nModelId, m_eModelSize));
	
	DFLOAT fVal = m_fDefaultHitPts > 0.0 ? m_fDefaultHitPts : GetDefaultHitPts(m_nModelId, m_eModelSize);
	m_damage.SetHitPoints(fVal);
	m_damage.SetMaxHitPoints(GetDefaultMaxHitPts(m_nModelId, m_eModelSize));
	
	fVal = m_fDefaultArmor > 0.0 ? m_fDefaultArmor : GetDefaultArmor(m_nModelId, m_eModelSize);
	m_damage.SetArmorPoints(fVal);
	m_damage.SetMaxArmorPoints(GetDefaultMaxArmor(m_nModelId, m_eModelSize));

	m_weapons.Init(m_hObject, m_eModelSize);

	AdjustDamageAggregate();	// Adjust AI damage based on difficulty
	InitializeWeapons();		// Initialize our weapons
	SetAnimationIndexes();		// Set our animation indexes


	// Set this as an object that can be seen with night/infrared vision...

	DDWORD nFlags = pServerDE->GetObjectUserFlags(m_hObject);
	pServerDE->SetObjectUserFlags(m_hObject, nFlags | USRFLG_MOVEABLE | USRFLG_NIGHT_INFRARED | USRFLG_CHARACTER);


	// Set our initial dims based on the current animation...

	DVector vDims;
	pServerDE->GetModelAnimUserDims(m_hObject, &vDims, pServerDE->GetModelAnimation(m_hObject));
	SetDims(&vDims);


	if (m_bMoveToFloor) 
	{
		MoveObjectToFloor(m_hObject);
	}


	if (m_bCreateDialogSprite && m_eModelSize != MS_SMALL)
	{
		CreateDialogSprite();
	}

	
	if (m_eModelSize == MS_LARGE)
	{
		m_bUsingHitDetection = DFALSE;
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::CacheFiles()
//
//	PURPOSE:	Cache resources used by this object
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::CacheFiles()
{
	char* pFile = DNULL;
	DebrisType eType;
	DVector v;
	int nNumFiles, i;

	// Cache the fixed and random gibs...

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	if( !( pServerDE->GetServerFlags( ) & SS_CACHING ))
		return;

	// Cache the gib models...
		
	for (i= GT_FIRST; i < GT_LAST; i++)
	{
		pFile = GetGibModel(m_nModelId, GibType(i), m_eModelSize);
		if (pFile)
		{
			pServerDE->CacheFile(FT_MODEL, pFile);
		}
	}


	eType = DBT_HUMAN_PARTS;
	switch (GetModelType(m_nModelId))
	{
		case MT_HUMAN:
			eType = DBT_HUMAN_PARTS;
		break;

		case MT_VEHICLE:
			eType = DBT_VEHICLE_PARTS;
		break;

		case MT_MECHA:
			eType = DBT_MECHA_PARTS;
		break;

		default : break;
	}

	// Cache gib parts...
	nNumFiles = GetNumDebrisModels(eType);
	for (i=0; i < nNumFiles; i++)
	{
		pFile = GetDebrisModel(eType, v, i);
		if (pFile)
		{
			pServerDE->CacheFile(FT_MODEL, pFile);
		}
	}
	pFile = GetDebrisSkin(eType);
	if (pFile)
	{
		pServerDE->CacheFile(FT_TEXTURE, pFile);
	}

	// Cache gib sounds...
	nNumFiles = GetNumDebrisBounceSounds(eType);
	for (i=0; i < nNumFiles; i++)
	{
		pFile = GetDebrisBounceSound(eType, i);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}
	nNumFiles = GetNumDebrisExplodeSounds(eType);
	for (i=0; i < nNumFiles; i++)
	{
		pFile = GetDebrisExplodeSound(eType, i);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::CreateDialogSprite()
//
//	PURPOSE:	Create a sprite to be shown over the character's head when
//				they talk.
//
// ----------------------------------------------------------------------- //
	
void CBaseCharacter::CreateDialogSprite()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hDlgSprite)
	{
		HATTACHMENT hAttachment;
		if (pServerDE->FindAttachment(m_hObject, m_hDlgSprite, &hAttachment) == DE_OK)
		{
			pServerDE->RemoveAttachment(hAttachment);
		}

		g_pServerDE->BreakInterObjectLink( m_hObject, m_hDlgSprite );
		pServerDE->RemoveObject(m_hDlgSprite);
		m_hDlgSprite = DNULL;
	}

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);
	VEC_COPY(theStruct.m_Pos, vPos);

	DVector vScale;
	char* pFilename = GetDialogSpriteFilename(vScale);
	if (!pFilename) return;

	SAFE_STRCPY(theStruct.m_Filename, pFilename);

	theStruct.m_Flags		= FLAG_GOTHRUWORLD;
	theStruct.m_ObjectType  = OT_SPRITE;
	theStruct.m_fDeactivationTime = 0.001f;

	HCLASS hClass = pServerDE->GetClass("BaseClass");
	LPBASECLASS pSprite = pServerDE->CreateObject(hClass, &theStruct);
	if (!pSprite) return;

	m_hDlgSprite = pSprite->m_hObject;
	if (!m_hDlgSprite) return;

	g_pServerDE->CreateInterObjectLink( m_hObject, m_hDlgSprite );

	pServerDE->ScaleObject(m_hDlgSprite, &vScale);

	// Attach the sprite to the character...

	DVector vOffset;
	pServerDE->GetObjectDims(m_hObject, &vOffset);

	vOffset.x = vOffset.z = 0.0f;
	vOffset.y += DIALOG_SPRITE_OFFSET;

	DRotation rOffset;
	ROT_INIT(rOffset);

	HATTACHMENT hAttachment;
	DRESULT dRes = pServerDE->CreateAttachment(m_hObject, m_hDlgSprite, DNULL, 
											   &vOffset, &rOffset, &hAttachment);
	if (dRes != DE_OK)
	{
		g_pServerDE->BreakInterObjectLink( m_hObject, m_hDlgSprite );
		pServerDE->RemoveObject(m_hDlgSprite);
		m_hDlgSprite = DNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::GetDialogSpriteFilename()
//
//	PURPOSE:	Get the name of the sprite to display
//
// ----------------------------------------------------------------------- //
	
char* CBaseCharacter::GetDialogSpriteFilename(DVector & vScale)
{
	VEC_SET(vScale, 0.125f, 0.125f, 1.0f);

	char* pFile = "Sprites\\ExclHuman.spr";
	if (IsMecha())
	{
		pFile = "Sprites\\ExclMech.spr";
	}

	return pFile;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::CreateHandHeldWeapon()
//
//	PURPOSE:	Create the hand held weapon (what other players see)
//
// ----------------------------------------------------------------------- //
	
void CBaseCharacter::CreateHandHeldWeapon(char* pFilename, char* pSkin)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || m_hHandHeldWeapon || !pFilename) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);
	VEC_COPY(theStruct.m_Pos, vPos);

	SAFE_STRCPY(theStruct.m_Filename, pFilename);
	SAFE_STRCPY(theStruct.m_SkinName, pSkin);

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD;

	HCLASS hClass = pServerDE->GetClass("CPVWeaponModel");
	LPBASECLASS pModel = pServerDE->CreateObject(hClass, &theStruct);
	if (!pModel) return;

	m_hHandHeldWeapon = pModel->m_hObject;
	g_pServerDE->CreateInterObjectLink( m_hObject, m_hHandHeldWeapon );

	// Attach the gun model to the character...

	DVector vOffset;
	VEC_INIT(vOffset);

	DRotation rOffset;
	ROT_INIT(rOffset);

	HATTACHMENT hAttachment;
	DRESULT dRes = pServerDE->CreateAttachment(m_hObject, m_hHandHeldWeapon, m_pHandName, 
											   &vOffset, &rOffset, &hAttachment);
	if (dRes != DE_OK)
	{
		g_pServerDE->BreakInterObjectLink( m_hObject, m_hHandHeldWeapon );
		pServerDE->RemoveObject(m_hHandHeldWeapon);
		m_hHandHeldWeapon = DNULL;
	}
	else
	{
		DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hHandHeldWeapon);
		pServerDE->SetObjectUserFlags(m_hHandHeldWeapon, dwUsrFlags | USRFLG_NIGHT_INFRARED | USRFLG_ATTACH_HIDE1SHOW3);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::HandHeldWeaponFirePos()
//
//	PURPOSE:	Get the fire (flash) position of the hand-held weapon
//
// ----------------------------------------------------------------------- //
	
DVector	CBaseCharacter::HandHeldWeaponFirePos()
{
	DVector vPos;
//	VEC_INIT(vPos);

	CWeapon* pWeapon = m_weapons.GetCurWeapon();
	CServerDE* pServerDE = GetServerDE();

	pServerDE->GetObjectPos( m_hObject, &vPos );
	if (!pServerDE || !m_hHandHeldWeapon || !pWeapon)
		return vPos;

	DRotation rRot;
	if (!pServerDE->GetModelNodeTransform(m_hObject, m_pHandName, &vPos, &rRot))
	{
		return vPos;
	}

	DVector vFlashOffset;
	CPVWeaponModel* pModel = (CPVWeaponModel*)pServerDE->HandleToObject(m_hHandHeldWeapon);
	if (pModel)
	{
		VEC_COPY(vFlashOffset, pModel->GetFlashOffset());
	}

	// DVector vFlashOffset = pWeapon->GetHandModelFlashOffset();

	DVector vTemp, vU, vR, vF;
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	VEC_MULSCALAR(vTemp, vF, vFlashOffset.z);
	VEC_ADD(vPos, vPos, vTemp);

	VEC_MULSCALAR(vTemp, vR, vFlashOffset.x);
	VEC_ADD(vPos, vPos, vTemp);

	VEC_MULSCALAR(vTemp, vU, vFlashOffset.y);
	VEC_ADD(vPos, vPos, vTemp);

	return vPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::HandleWeaponChange()
//
//	PURPOSE:	Handle our weapon changing...
//
// ----------------------------------------------------------------------- //
	
void CBaseCharacter::HandleWeaponChange()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_bCreateHandHeldWeapon) return;

	CWeapon* pWeapon = m_weapons.GetCurWeapon();
	if (!pWeapon) return;
	
	char* pFilename	= pWeapon->GetHandWeaponName();
	char* pSkin		= pWeapon->GetHandWeaponSkin();

	if (!pFilename) return;

	DVector vScale	= GetHandWeaponScale(pWeapon->GetId(), m_eModelSize);


	// If we don't have a hand held weapon, try to create one...

	if (!m_hHandHeldWeapon) 
	{
		CreateHandHeldWeapon(pFilename, pSkin);
	}
	if (!m_hHandHeldWeapon) return;


	// Check for big guns...

	HCONVAR	hVar  = pServerDE->GetGameConVar("BigGuns");
	char* pVar = pServerDE->GetVarValueString(hVar);

	if (pVar && _stricmp(pVar, "0") != 0)
	{
		VEC_MULSCALAR(vScale, vScale, BIGGUNS_SCALE_VALUE);
	}

	pServerDE->SetModelFilenames(m_hHandHeldWeapon, pFilename, pSkin);
	pServerDE->ScaleObject(m_hHandHeldWeapon, &vScale);

	DDWORD dwHandHeldFlags = pServerDE->GetObjectFlags(m_hHandHeldWeapon);
	DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
	if (dwFlags & FLAG_VISIBLE)
	{
		dwHandHeldFlags |= FLAG_VISIBLE;
	}
	else
	{
		dwHandHeldFlags &= ~FLAG_VISIBLE;
	}

	pServerDE->SetObjectFlags(m_hHandHeldWeapon, dwHandHeldFlags);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::RemoveHandHeldWeapon()
//
//	PURPOSE:	Remove hand held weapon
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::RemoveHandHeldWeapon()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hHandHeldWeapon)
	{
		// Remove the model...

		HATTACHMENT hAttachment;
		if (pServerDE->FindAttachment(m_hObject, m_hHandHeldWeapon, &hAttachment) == DE_OK)
		{
			pServerDE->RemoveAttachment(hAttachment);
		}
		g_pServerDE->BreakInterObjectLink( m_hObject, m_hHandHeldWeapon );
		pServerDE->RemoveObject(m_hHandHeldWeapon);
		m_hHandHeldWeapon = DNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::Update()
//
//	PURPOSE:	Update the object
//
// ----------------------------------------------------------------------- //
	
void CBaseCharacter::Update()
{
	UpdateSounds();

	// check to see if any of our timed powerups have expired

	CheckPowerups();

	// apply any powerups we have

	ApplyPowerups();

	// Keep track of frame to frame changes...

	m_bBipedalLastFrame		= m_bBipedal;	
	m_eLastContainerCode	= m_eContainerCode;
	m_dwLastFrameCtlFlgs	= m_dwControlFlags;
	m_bBodyWasInLiquid		= m_bBodyInLiquid;

	m_bBodyInLiquid			= DFALSE;
	m_bBodyOnLadder			= DFALSE;

	m_bOneHandedWeapon		= IsOneHandedWeapon((RiotWeaponId)m_weapons.GetCurWeaponId());



	// Update our container code info...

	UpdateContainerCode();


	// Update cheat stuff...

	UpdateCheatInfo();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::CheckPowerups()
//
//	PURPOSE:	Checks to see if any timed powerups have expired
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::CheckPowerups()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	DFLOAT time = pServerDE->GetTime();
	
	for (DDWORD i = 0; i < MAX_TIMED_POWERUPS; i++)
	{
		if (m_powerups[i] && m_powerups[i]->fExpirationTime < time)
		{
			// call the notification function
			OnTimedPowerupExpiration (m_powerups[i]->eType);

			// delete the powerup
			delete m_powerups[i];
			m_powerups[i] = DNULL;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::RemoveAllPowerups()
//
//	PURPOSE:	Remove all powerups...
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::RemoveAllPowerups()
{
	for (DDWORD i = 0; i < MAX_TIMED_POWERUPS; i++)
	{
		if (m_powerups[i])
		{
			// call the notification function
			OnTimedPowerupExpiration(m_powerups[i]->eType);

			// delete the powerup
			delete m_powerups[i];
			m_powerups[i] = DNULL;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::AddTimedPowerup()
//
//	PURPOSE:	Adds the timed powerup to our list
//
// ----------------------------------------------------------------------- //

DBOOL CBaseCharacter::AddTimedPowerup (TimedPowerup* pPowerup)
{
	// first check to see if we already have it - if so, replace it

	for (DDWORD i = 0; i < MAX_TIMED_POWERUPS; i++)
	{
		if (m_powerups[i] && m_powerups[i]->eType == pPowerup->eType)
		{
			m_powerups[i]->fExpirationTime = pPowerup->fExpirationTime;
			delete pPowerup;
			return DTRUE;
		}
	}

	// we don't already have it, so create a new entry

	for (int i = 0; i < MAX_TIMED_POWERUPS; i++)
	{
		if (!m_powerups[i])
		{
			m_powerups[i] = pPowerup;
			return DTRUE;
		}
	}
	
	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::HaveTimedPowerup()
//
//	PURPOSE:	Checks to see if we have the named powerup
//
// ----------------------------------------------------------------------- //

DBOOL CBaseCharacter::HaveTimedPowerup (PickupItemType eType)
{
	for (DDWORD i = 0; i < MAX_TIMED_POWERUPS; i++)
	{
		if (m_powerups[i] && m_powerups[i]->eType == eType)
		{
			return DTRUE;
		}
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::SaveModelColor()
//
//	PURPOSE:	Called when character gets stealth powerup - saves
//				current model color and alpha values so they can
//				be restored later
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::SaveModelColor()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	if (!HaveTimedPowerup (PIT_ULTRA_STEALTH))
	{
		pServerDE->GetObjectColor (m_hObject, &m_vOldCharacterColor.x, &m_vOldCharacterColor.y, &m_vOldCharacterColor.z, &m_fOldCharacterAlpha);
		m_bCharacterHadShadow = !!(pServerDE->GetObjectFlags (m_hObject) & FLAG_SHADOW);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::ApplyPowerups()
//
//	PURPOSE:	Applies any timed powerups we have
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::ApplyPowerups()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	for (DDWORD i = 0; i < MAX_TIMED_POWERUPS; i++)
	{
		if (!m_powerups[i]) continue;

		// Stealth

		if (m_powerups[i]->eType == PIT_ULTRA_STEALTH && !(GetControlFlags() & BC_CFLG_FIRING))
		{
			DDWORD nFlags = pServerDE->GetObjectFlags (m_hObject);
			nFlags &= ~FLAG_SHADOW;
			pServerDE->SetObjectFlags (m_hObject, nFlags);
			pServerDE->SetObjectColor (m_hObject, m_vOldCharacterColor.x, m_vOldCharacterColor.y, m_vOldCharacterColor.z, 0.1f);
		}
		else if (m_powerups[i]->eType == PIT_ULTRA_STEALTH)
		{
			DDWORD nFlags = pServerDE->GetObjectFlags (m_hObject);
			if (m_bCharacterHadShadow) nFlags |= FLAG_SHADOW;
			pServerDE->SetObjectFlags (m_hObject, nFlags);
			pServerDE->SetObjectColor (m_hObject, m_vOldCharacterColor.x, m_vOldCharacterColor.y, m_vOldCharacterColor.z, m_fOldCharacterAlpha);
		}

		// NightVision

		// Infrared

		// Silencer
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::OnTimedPowerupExpiration()
//
//	PURPOSE:	Handles the expiration of timed powerups
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::OnTimedPowerupExpiration (PickupItemType eType)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	if (eType == PIT_ULTRA_STEALTH)
	{
		DDWORD nFlags = pServerDE->GetObjectFlags (m_hObject);
		nFlags &= ~FLAG_MODELWIREFRAME;
		if (m_bCharacterHadShadow) nFlags |= FLAG_SHADOW;
		pServerDE->SetObjectFlags (m_hObject, nFlags);
		pServerDE->SetObjectColor (m_hObject, m_vOldCharacterColor.x, m_vOldCharacterColor.y, m_vOldCharacterColor.z, m_fOldCharacterAlpha);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::HandleModelString()
//
//	PURPOSE:	Handles model keyframe strings
//
// ----------------------------------------------------------------------- //
	
void CBaseCharacter::HandleModelString(ArgList* pArgList)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	// Only play footstep sound if we are on the ground...

	if (stricmp(pKey, KEY_FOOTSTEP_SOUND) == 0 && m_bOnGround)
	{
		SurfaceType eSurface = ST_UNKNOWN;
		DBOOL bInWater = (m_bBodyInLiquid && !IsLiquid(m_eContainerCode));

		// Don't play sound if small...
		if( m_eModelSize != MS_SMALL )
		{
			if (bInWater)
			{
				eSurface = ST_LIQUID;
			}
			else if (m_bOnGround)  // See what surface we're standing on
			{
				CollisionInfo Info;
				pServerDE->GetStandingOn(m_hObject, &Info);

				if (Info.m_hObject) 
				{
					if (Info.m_hPoly)
					{
						eSurface = GetSurfaceType(Info.m_hPoly);
					}
					else  // Get the texture flags from the object...
					{
						eSurface = GetSurfaceType(Info.m_hObject);
					}
				}
			}

			PlayFootStepSound(eSurface);
		}
	}
	else if (stricmp(pKey, KEY_PLAYSOUND) == 0 && pArgList->argc > 1)
	{
		// Get sound name from message...

		char* pSoundName = pArgList->argv[1];

		if (pSoundName)
		{
			// See if sound radius was in message..

			DFLOAT fRadius = 1000;

			if (pArgList->argc > 3 && pArgList->argv[2])
			{
				fRadius = (DFLOAT) atoi(pArgList->argv[2]);
			}

			fRadius = fRadius > 0.0f ? fRadius : m_fSoundRadius;

			PlaySound( pSoundName, SOUNDPRIORITYMOD_HIGH, fRadius, DTRUE );
		}
	}
	else if (stricmp(pKey, KEY_SET_DIMS) == 0)
	{
		if (pArgList->argc < 2) return;

		// Set up so we can set one or more dims...

		DVector vDims;
		pServerDE->GetObjectDims(m_hObject, &vDims);

		if (pArgList->argv[1])
		{
			vDims.x = (DFLOAT) atof(pArgList->argv[1]);
		}
		if (pArgList->argc > 2 && pArgList->argv[2])
		{
			vDims.y = (DFLOAT) atof(pArgList->argv[2]);
		}
		if (pArgList->argc > 3 && pArgList->argv[3])
		{
			vDims.z = (DFLOAT) atof(pArgList->argv[3]);
		}

		// Set the new dims

		SetDims(&vDims);
	}
	else if (stricmp(pKey, KEY_MOVE) == 0)
	{
		if (pArgList->argc < 2) return;

		// Set up so we move in one or more directions

		DVector vPos;
		pServerDE->GetObjectPos(m_hObject, &vPos);
		
		DRotation rRot;
		DVector vU, vR, vF;
		pServerDE->GetObjectRotation(m_hObject, &rRot);
		pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

		DFLOAT fOffset;

		if (pArgList->argv[1])
		{
			// Forward...

			fOffset = (DFLOAT) atof(pArgList->argv[1]);

			VEC_MULSCALAR(vF, vF, fOffset);
			VEC_ADD(vPos, vPos, vF);
		}
		if (pArgList->argc > 2 && pArgList->argv[2])
		{
			// Up...

			fOffset = (DFLOAT) atof(pArgList->argv[2]);

			VEC_MULSCALAR(vU, vU, fOffset);
			VEC_ADD(vPos, vPos, vU);
		}
		if (pArgList->argc > 3 && pArgList->argv[3])
		{
			// Right...

			fOffset = (DFLOAT) atof(pArgList->argv[3]);

			VEC_MULSCALAR(vR, vR, fOffset);
			VEC_ADD(vPos, vPos, vR);
		}

		// Set the new position

		pServerDE->MoveObject(m_hObject, &vPos);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::SetAnimationIndexes()
//
//	PURPOSE:	Initialize model animation indexes
//
// ----------------------------------------------------------------------- //
	
void CBaseCharacter::SetAnimationIndexes()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	m_hStandKnifeAttackAni		    = pServerDE->GetAnimIndex(m_hObject, ANIM_STAND_KNIFE_ATTACK);
	m_hStandRifleAttackAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_STAND_RIFLE_ATTACK);

	m_hWalkKnifeAni				    = pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_KNIFE);
	m_hWalkKnifeAttackAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_KNIFE_ATTACK);
	m_hWalkBKnifeAni			    = pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_BKNIFE);
	m_hWalkBKnifeAttackAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_BKNIFE_ATTACK);
	m_hWalkStrafeLKnifeAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_STRAFEL_KNIFE);
	m_hWalkStrafeRKnifeAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_STRAFER_KNIFE);
	m_hWalkStrafeLKnifeAttackAni	= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_STRAFEL_KNIFE_ATTACK);
	m_hWalkStrafeRKnifeAttackAni	= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_STRAFER_KNIFE_ATTACK);

	m_hRunKnifeAni					= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_KNIFE);
	m_hRunKnifeAttackAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_KNIFE_ATTACK);
	m_hRunBKnifeAni					= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_BKNIFE);
	m_hRunBKnifeAttackAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_BKNIFE_ATTACK);
	m_hRunStrafeLKnifeAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_STRAFEL_KNIFE);
	m_hRunStrafeRKnifeAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_STRAFER_KNIFE);
	m_hRunStrafeLKnifeAttackAni		= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_STRAFEL_KNIFE_ATTACK);
	m_hRunStrafeRKnifeAttackAni		= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_STRAFER_KNIFE_ATTACK);

	m_hCrouchKnifeAni				= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_KNIFE);
	m_hCrouchKnifeAttackAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_KNIFE_ATTACK);
	m_hCrouchWalkKnifeAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_WALK_KNIFE);
	m_hCrouchWalkKnifeAttackAni		= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_WALK_KNIFE_ATTACK);
	m_hCrouchWalkBKnifeAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_WALK_BKNIFE);
	m_hCrouchWalkBKnifeAttackAni	= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_WALK_BKNIFE_ATTACK);
	m_hCrouchStrafeLKnifeAni		= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_STRAFEL_KNIFE);
	m_hCrouchStrafeRKnifeAni		= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_STRAFER_KNIFE);
	m_hCrouchStrafeLKnifeAttackAni	= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_STRAFEL_KNIFE_ATTACK);
	m_hCrouchStrafeRKnifeAttackAni	= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_STRAFER_KNIFE_ATTACK);

	m_hWalkRifleAni					= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_RIFLE);
	m_hWalkRifleAttackAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_RIFLE_ATTACK);
	m_hWalkBRifleAni				= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_BRIFLE);
	m_hWalkBRifleAttackAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_BRIFLE_ATTACK);
	m_hWalkStrafeLRifleAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_STRAFEL_RIFLE);
	m_hWalkStrafeRRifleAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_STRAFER_RIFLE);
	m_hWalkStrafeLRifleAttackAni	= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_STRAFEL_RIFLE_ATTACK);
	m_hWalkStrafeRRifleAttackAni	= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_STRAFER_RIFLE_ATTACK);

	m_hRunRifleAni					= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_RIFLE);
	m_hRunRifleAttackAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_RIFLE_ATTACK);
	m_hRunBRifleAni					= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_BRIFLE);
	m_hRunBRifleAttackAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_BRIFLE_ATTACK);
	m_hRunStrafeLRifleAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_STRAFEL_RIFLE);
	m_hRunStrafeRRifleAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_STRAFER_RIFLE);
	m_hRunStrafeLRifleAttackAni		= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_STRAFEL_RIFLE_ATTACK);
	m_hRunStrafeRRifleAttackAni		= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_STRAFER_RIFLE_ATTACK);

	m_hCrouchRifleAni				= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_RIFLE);
	m_hCrouchRifleAttackAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_RIFLE_ATTACK);
	m_hCrouchWalkRifleAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_WALK_RIFLE);
	m_hCrouchWalkRifleAttackAni		= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_WALK_RIFLE_ATTACK);
	m_hCrouchWalkBRifleAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_WALK_BRIFLE);
	m_hCrouchWalkBRifleAttackAni	= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_WALK_BRIFLE_ATTACK);
	m_hCrouchStrafeLRifleAni		= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_STRAFEL_RIFLE);
	m_hCrouchStrafeRRifleAni		= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_STRAFER_RIFLE);
	m_hCrouchStrafeLRifleAttackAni	= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_STRAFEL_RIFLE_ATTACK);
	m_hCrouchStrafeRRifleAttackAni	= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_STRAFER_RIFLE_ATTACK);

	m_hPose1Ani						= pServerDE->GetAnimIndex(m_hObject, ANIM_POSE1);
	m_hPose2Ani						= pServerDE->GetAnimIndex(m_hObject, ANIM_POSE2);
	m_hRifleIdleAni[0]				= pServerDE->GetAnimIndex(m_hObject, ANIM_IDLE_RIFLE1);
	m_hRifleIdleAni[1]				= pServerDE->GetAnimIndex(m_hObject, ANIM_IDLE_RIFLE2);
	m_hRifleIdleAni[2]				= pServerDE->GetAnimIndex(m_hObject, ANIM_IDLE_RIFLE2);
	m_hRifleIdleAni[3]				= pServerDE->GetAnimIndex(m_hObject, ANIM_IDLE_RIFLE2);
	m_hRifleIdleAni[4]				= pServerDE->GetAnimIndex(m_hObject, ANIM_IDLE_RIFLE2);
	m_hKnifeIdle1Ani				= pServerDE->GetAnimIndex(m_hObject, ANIM_IDLE_KNIFE1);
	m_hKnifeIdle2Ani				= pServerDE->GetAnimIndex(m_hObject, ANIM_IDLE_KNIFE2);
	m_hJumpUpAni					= pServerDE->GetAnimIndex(m_hObject, ANIM_JUMP_UP);
	m_hJumpDownAni					= pServerDE->GetAnimIndex(m_hObject, ANIM_JUMP_DOWN);
	m_hLandingAni					= pServerDE->GetAnimIndex(m_hObject, ANIM_LANDING);
	m_hTransformAni					= pServerDE->GetAnimIndex(m_hObject, ANIM_TRANSFORM);
	m_hInverseTransformAni			= pServerDE->GetAnimIndex(m_hObject, ANIM_INVERSE_TRANSFORM);
	m_hVehicleAni					= pServerDE->GetAnimIndex(m_hObject, ANIM_VEHICLE);
	m_hSwimAni						= pServerDE->GetAnimIndex(m_hObject, ANIM_SWIM);
	m_hTearsAni						= pServerDE->GetAnimIndex(m_hObject, ANIM_TEARS);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::UpdateAnimation()
//
//	PURPOSE:	Update the current animation
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::UpdateAnimation()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	if (m_damage.IsDead())
	{
		SetDeathAnimation();
	}
	else if (m_bLanding)
	{
		SetLandingAnimation();
	}
	else if (IsMecha() && (m_bTransforming || !m_bBipedal || (m_bBipedal != m_bBipedalLastFrame)))
	{
		SetTransformAnimation();
	}
	else if (m_bCrying)
	{
		UpdateTearsAnimation();
	}
	else if (m_bRecoiling)
	{
		UpdateRecoilAnimation();
	}
	else if (m_bAllowMovement)
	{
		if (IsLiquid(m_eContainerCode))
		{
			SetSwimAnimation();
		}
		else if ((m_dwControlFlags & BC_CFLG_DUCK) && !m_bBodyOnLadder)
		{
			SetCrouchAnimation();
		}
		else if ((m_dwControlFlags & BC_CFLG_MOVING))
		{
			if ((m_dwControlFlags & BC_CFLG_RUN)) 
			{
				SetRunAnimation();
			}
			else 
			{
				SetWalkAnimation();
			}
		}
		else
		{
			SetStandAnimation();
		}
		
		UpdateInAir();
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::SetSwimAnimation()
//
//	PURPOSE:	Set animation to swimming
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::SetSwimAnimation()
{
	SetAnimation(m_hSwimAni, DTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::SetTearsAnimation()
//
//	PURPOSE:	Set animation to tears
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::SetTearsAnimation()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	if (m_hTearsAni != INVALID_ANI)
	{
		m_bAllowMovement = DFALSE;
		m_bCrying		 = DTRUE;

		char* pTearSounds[] = { "tears1.wav", "tears2.wav", "tears3.wav", "tears4.wav", "tears5.wav" };
		char buf[100];
		sprintf(buf, "Sounds\\Player\\Onfoot\\%s", pTearSounds[GetRandom(0,4)]);
		PlaySound(buf, SOUNDPRIORITYMOD_HIGH, m_fSoundRadius, DFALSE);
		SetAnimation(m_hTearsAni, DFALSE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::SetRecoilAnimation()
//
//	PURPOSE:	Set animation to recoil
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::SetRecoilAnimation()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	HMODELANIM hRecoilAni = GetRecoilAni();

	if (hRecoilAni != INVALID_ANI)
	{
		m_bAllowMovement = DFALSE;
		m_bRecoiling	 = DTRUE;

		SetAnimation(hRecoilAni, DFALSE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::UpdateTearsAnimation()
//
//	PURPOSE:	Update tears animation
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::UpdateTearsAnimation()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	DDWORD dwState = pServerDE->GetModelPlaybackState(m_hObject);

	if (dwState & MS_PLAYDONE)
	{
		m_bAllowMovement = DTRUE;
		m_bCrying	     = DFALSE;  
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::UpdateRecoilAnimation()
//
//	PURPOSE:	Update recoiling animation
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::UpdateRecoilAnimation()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	DDWORD dwState = pServerDE->GetModelPlaybackState(m_hObject);

	if (dwState & MS_PLAYDONE)
	{
		m_bAllowMovement = DTRUE;
		m_bRecoiling	 = DFALSE;  // Done recoiling
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::SetStandAnimation()
//
//	PURPOSE:	Set animation to standing (firing or idle)
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::SetStandAnimation()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	HMODELANIM hAni = DNULL;


	// See if we are currently firing...

	DDWORD bAttacking = (m_dwControlFlags & BC_CFLG_FIRING);

	if (bAttacking)
	{
		if (m_bOneHandedWeapon)
		{
			hAni = m_hStandKnifeAttackAni;
		}
		else
		{
			hAni = m_hStandRifleAttackAni;
		}

		SetAnimation(hAni, DTRUE);
		return;
	}	
	

	// Make sure idle animation is done if one is currently playing...

	hAni = pServerDE->GetModelAnimation(m_hObject);

	if (m_bOneHandedWeapon)
	{
		if (hAni == m_hKnifeIdle1Ani || hAni == m_hKnifeIdle2Ani)
		{ 
			if (!(pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE))
			{
				return;
			}
		}
	
		hAni = (GetRandom(0, 1) == 0 ? m_hKnifeIdle1Ani : m_hKnifeIdle2Ani);
	}
	else
	{
		if (hAni == m_hRifleIdleAni[0] || hAni == m_hRifleIdleAni[1] ||
			hAni == m_hRifleIdleAni[2] || hAni == m_hRifleIdleAni[3] ||
			hAni == m_hRifleIdleAni[4])
		{ 
			if (!(pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE))
			{
				return;
			}
		}
	 
		hAni = m_hRifleIdleAni[GetRandom(0, 4)];

		if (hAni == INVALID_ANI)
		{
			hAni = m_hRifleIdleAni[GetRandom(0,1)];  // Should always be valid
		}
	}

	SetAnimation(hAni, DFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::SetWalkAnimation()
//
//	PURPOSE:	Set animation to walking
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::SetWalkAnimation()
{
	HMODELANIM hAni = INVALID_ANI;
	
	DDWORD bStrafeLeft  = (m_dwControlFlags & BC_CFLG_STRAFE_LEFT);
	DDWORD bStrafeRight = (m_dwControlFlags & BC_CFLG_STRAFE_RIGHT);
	DDWORD bAttacking   = (m_dwControlFlags & BC_CFLG_FIRING);
	DDWORD bBackward	= (m_dwControlFlags & BC_CFLG_REVERSE);

	if (m_bOneHandedWeapon)
	{
		hAni = bAttacking ? ((bStrafeLeft ? m_hWalkStrafeLKnifeAttackAni : (bStrafeRight ? m_hWalkStrafeRKnifeAttackAni : (bBackward ? m_hWalkBKnifeAttackAni : m_hWalkKnifeAttackAni)))) : 
				((bStrafeLeft ? m_hWalkStrafeLKnifeAni : (bStrafeRight ? m_hWalkStrafeRKnifeAni : (bBackward ? m_hWalkBKnifeAni : m_hWalkKnifeAni))));
	}
	else
	{
		hAni = bAttacking ? ((bStrafeLeft ? m_hWalkStrafeLRifleAttackAni : (bStrafeRight ? m_hWalkStrafeRRifleAttackAni : (bBackward ? m_hWalkBRifleAttackAni : m_hWalkRifleAttackAni)))) : 
				((bStrafeLeft ? m_hWalkStrafeLRifleAni : (bStrafeRight ? m_hWalkStrafeRRifleAni : (bBackward ? m_hWalkBRifleAni : m_hWalkRifleAni))));
	}

	SetAnimation(hAni, DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::SetRunAnimation()
//
//	PURPOSE:	Set animation to running
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::SetRunAnimation()
{
	HMODELANIM hAni = INVALID_ANI;
	
	DDWORD bStrafeLeft  = (m_dwControlFlags & BC_CFLG_STRAFE_LEFT);
	DDWORD bStrafeRight = (m_dwControlFlags & BC_CFLG_STRAFE_RIGHT);
	DDWORD bAttacking   = (m_dwControlFlags & BC_CFLG_FIRING);
	DDWORD bBackward	= (m_dwControlFlags & BC_CFLG_REVERSE);

	if (m_bOneHandedWeapon)
	{
		hAni = bAttacking ? ((bStrafeLeft ? m_hRunStrafeLKnifeAttackAni : (bStrafeRight ? m_hRunStrafeRKnifeAttackAni : (bBackward ? m_hRunBKnifeAttackAni : m_hRunKnifeAttackAni)))) : 
				((bStrafeLeft ? m_hRunStrafeLKnifeAni : (bStrafeRight ? m_hRunStrafeRKnifeAni : (bBackward ? m_hRunBKnifeAni : m_hRunKnifeAni))));
	}
	else
	{
		hAni = bAttacking ? ((bStrafeLeft ? m_hRunStrafeLRifleAttackAni : (bStrafeRight ? m_hRunStrafeRRifleAttackAni : (bBackward ? m_hRunBRifleAttackAni : m_hRunRifleAttackAni)))) : 
				((bStrafeLeft ? m_hRunStrafeLRifleAni : (bStrafeRight ? m_hRunStrafeRRifleAni : (bBackward ? m_hRunBRifleAni : m_hRunRifleAni))));
	}

	SetAnimation(hAni, DTRUE);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::SetCrouchAnimation()
//
//	PURPOSE:	Set animation to crouching
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::SetCrouchAnimation()
{
	HMODELANIM hAni = INVALID_ANI;
	
	DDWORD bStrafeLeft  = (m_dwControlFlags & BC_CFLG_STRAFE_LEFT);
	DDWORD bStrafeRight = (m_dwControlFlags & BC_CFLG_STRAFE_RIGHT);
	DDWORD bAttacking   = (m_dwControlFlags & BC_CFLG_FIRING);
	DDWORD bMoving		= (m_dwControlFlags & BC_CFLG_MOVING);
	DDWORD bBackward	= (m_dwControlFlags & BC_CFLG_REVERSE);

	if (bMoving)
	{
		if (m_bOneHandedWeapon)
		{
			hAni = bAttacking ? ((bStrafeLeft ? m_hCrouchStrafeLKnifeAttackAni : (bStrafeRight ? m_hCrouchStrafeRKnifeAttackAni : (bBackward ? m_hCrouchWalkBKnifeAttackAni : m_hCrouchWalkKnifeAttackAni)))) : 
					((bStrafeLeft ? m_hCrouchStrafeLKnifeAni : (bStrafeRight ? m_hCrouchStrafeRKnifeAni : (bBackward ? m_hCrouchWalkBKnifeAni : m_hCrouchWalkKnifeAni))));
		}
		else
		{
			hAni = bAttacking ? ((bStrafeLeft ? m_hCrouchStrafeLRifleAttackAni : (bStrafeRight ? m_hCrouchStrafeRRifleAttackAni : (bBackward ? m_hCrouchWalkBRifleAttackAni : m_hCrouchWalkRifleAttackAni)))) : 
					((bStrafeLeft ? m_hCrouchStrafeLRifleAni : (bStrafeRight ? m_hCrouchStrafeRRifleAni : (bBackward ? m_hCrouchWalkBRifleAni : m_hCrouchWalkRifleAni))));
		}
	}
	else
	{
		if (m_bOneHandedWeapon)
		{
			hAni = bAttacking ? m_hCrouchKnifeAttackAni : m_hCrouchKnifeAni;
		}
		else
		{
			hAni = bAttacking ? m_hCrouchRifleAttackAni : m_hCrouchRifleAni;
		}
	}

	SetAnimation(hAni, DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::SetDeathAnimation()
//
//	PURPOSE:	Set animation to death
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::SetDeathAnimation()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || m_bStartedDeath) return;

	StartDeath();

	m_eDeathType = CD_NORMAL;

	DFLOAT fDeathDamage = m_damage.GetDeathDamage();
	DFLOAT fMaxHitPts   = m_damage.GetMaxHitPoints();

	DamageType eDType = m_damage.GetDeathType();
	DBOOL bGibDType = (eDType == DT_EXPLODE || eDType == DT_IMPACT || (IsMecha() && eDType == DT_MELEE));

	// Make us nonsolid.
	DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
	dwFlags &= ~FLAG_SOLID;
	pServerDE->SetObjectFlags(m_hObject, dwFlags);

	if ((bGibDType && fDeathDamage > fMaxHitPts * .1f)) // || fDeathDamage > fMaxHitPts * .5f)
	{
		m_eDeathType = CD_GIB;
		SetAnimation(pServerDE->GetAnimIndex(m_hObject, "DEATH1"), DFALSE);
	}
	else
	{
		if (eDType == DT_FREEZE)
		{
			m_eDeathType = CD_FREEZE;
		}
		else if (eDType == DT_KATO)
		{
			m_eDeathType = CD_VAPORIZE;
		}

		HMODELANIM hAni = INVALID_ANI;
		char* deathAnis[] = {"DEATH1", "DEATH2", "DEATH3", "DEATH4", "DEATH5"};
	
		int nIndex = GetRandom(0, 4);

		hAni = pServerDE->GetAnimIndex(m_hObject, deathAnis[nIndex]);

		if (hAni == INVALID_ANI)
		{
			hAni = pServerDE->GetAnimIndex(m_hObject, "DEATH1");
		}

		SetAnimation(hAni, DFALSE);
	}

	HandleDead(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::SetTransformAnimation()
//
//	PURPOSE:	Set animation to transform
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::SetTransformAnimation()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	if (!m_bTransforming || (m_bBipedal != m_bBipedalLastFrame))
	{
		m_bTransforming = DTRUE;

		HMODELANIM hAni = m_bBipedal ? m_hInverseTransformAni : m_hTransformAni;

		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
		pServerDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_ANIMTRANSITION);

		SetAnimation(hAni, DFALSE);
	}
	else 
	{
		HMODELANIM hCurAni = pServerDE->GetModelAnimation(m_hObject);
		DDWORD dwState = pServerDE->GetModelPlaybackState(m_hObject);
	
		if (hCurAni == m_hTransformAni && (dwState & MS_PLAYDONE))
		{
			PlayVehicleSound();
			SetAnimation(m_hVehicleAni, DTRUE);
		}
		else if (hCurAni == m_hInverseTransformAni && (dwState & MS_PLAYDONE))
		{
			m_bTransforming = DFALSE;

			DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
			pServerDE->SetObjectFlags(m_hObject, dwFlags | FLAG_ANIMTRANSITION);
		}
		else if (hCurAni != m_hVehicleAni && hCurAni != m_hTransformAni && hCurAni != m_hInverseTransformAni)
		{
			// Saftey net...If we're in this state, we'd better be playing one
			// these anis, if not we must not be transforming...

			m_bTransforming = DFALSE;

			DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
			pServerDE->SetObjectFlags(m_hObject, dwFlags | FLAG_ANIMTRANSITION);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::SetLandingAnimation()
//
//	PURPOSE:	Set animation to landing
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::SetLandingAnimation()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	if (m_hLandingAni == INVALID_ANI || !m_bBipedal)
	{
		m_bLanding = DFALSE;
		return;
	}

	HMODELANIM hCurAni = pServerDE->GetModelAnimation(m_hObject);
	DDWORD dwState     = pServerDE->GetModelPlaybackState(m_hObject);

	if ((hCurAni == m_hLandingAni) && (dwState & MS_PLAYDONE))
	{
		m_bAllowRun	= DTRUE;
		m_bLanding	= DFALSE;

		// Turn animation transitions back on...

		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
		pServerDE->SetObjectFlags(m_hObject, dwFlags | FLAG_ANIMTRANSITION);
	}
	else if (hCurAni != m_hLandingAni)
	{
		m_bAllowRun = DFALSE;
		m_bLanding	= DTRUE;

		// Don't transition into this ani...

		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
		pServerDE->SetObjectFlags(m_hObject, dwFlags &= ~FLAG_ANIMTRANSITION);

		SetAnimation(m_hLandingAni, DFALSE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::SetAnimation
//
//	PURPOSE:	Set the characters animation
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::SetAnimation(HMODELANIM hAni, DBOOL bLoop)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	pServerDE->SetModelLooping(m_hObject, bLoop);

	// Set the animation...

	HMODELANIM hCurAni = pServerDE->GetModelAnimation(m_hObject);

	if ((hCurAni != hAni) && (hAni != INVALID_ANI))
	{
		// Set model dims based on animation...

		DVector vDims;
		if (pServerDE->GetModelAnimUserDims(m_hObject, &vDims, hAni) == DE_OK)
		{
			// If we could update the dims, set the new animation...

			if (SetDims(&vDims, DFALSE))
			{
				pServerDE->SetModelAnimation(m_hObject, hAni);
			}
			else
			{
				// If we were ducking, and we tried to stand but couldn't
				// make us continue to duck...

				if ((m_dwLastFrameCtlFlgs & BC_CFLG_DUCK) &&
					!(m_dwControlFlags & BC_CFLG_DUCK))
				{
					m_dwControlFlags |= BC_CFLG_DUCK;
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::UpdateMovement
//
//	PURPOSE:	Update character movement
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::UpdateMovement(DBOOL bUpdatePhysics)
{
	// Update m_bOnGround data member...

	UpdateOnGround();

	// Update current animation...

	UpdateAnimation();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::CheckForceJump()
//
//	PURPOSE:	See if we should force a jump
//
// ----------------------------------------------------------------------- //

DBOOL CBaseCharacter::CheckForceJump(DVector* pvAccel, DVector* pvForward)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	DBOOL bRet = DFALSE;

	// If we are coming out of liquid, and facing a wall, and have an upward
	// velocity...give us a boost...

	if (m_bBodyInLiquid && !IsLiquid(m_eContainerCode))
	{
		// If we have an upward acceleration, see if we are facing a wall...

		if (pvAccel && pvForward && pvAccel->y > 0.0)
		{
			// Cast ray to see if against wall...
			
			DVector vPos, vDims, vTemp, vPos2;
			pServerDE->GetObjectPos(m_hObject, &vPos);
			pServerDE->GetObjectDims(m_hObject, &vDims);

			VEC_MULSCALAR(vTemp, *pvForward, vDims.x + 10.0f);
			VEC_ADD(vPos2, vPos, vTemp);
    
			IntersectQuery IQuery;
			IntersectInfo IInfo;

			VEC_COPY(IQuery.m_From, vPos);
			VEC_COPY(IQuery.m_To, vPos2);
    
			IQuery.m_Flags	   = IGNORE_NONSOLID;
			IQuery.m_FilterFn  = NULL;
			IQuery.m_pUserData = NULL;	

			bRet = pServerDE->IntersectSegment(&IQuery, &IInfo);
		}
	}

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::UpdateInLiquid
//
//	PURPOSE:	Update movement when in liquid
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::UpdateInLiquid(VolumeBrush* pBrush, ContainerPhysics* pCPStruct)
{
	m_bBodyInLiquid = DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::UpdateOnLadder
//
//	PURPOSE:	Update movement when on a ladder
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::UpdateOnLadder(VolumeBrush* pBrush, ContainerPhysics* pCPStruct)
{
	m_bBodyOnLadder = DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::UpdateOnGround
//
//	PURPOSE:	Update m_bOnGround data member
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::UpdateOnGround()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DBOOL bFreeMovement = (IsFreeMovement(m_eContainerCode) || 
						   m_bBodyOnLadder || m_bBodyInLiquid);

	if (m_bOnGround && !m_bLastOnGround && !m_damage.IsDead()) 
	{
		if (m_fTimeInAir > FALL_LANDING_TIME) 
		{
			m_fTimeInAir = 0.0;

			if (!bFreeMovement)
			{
				SetLandingAnimation();
			}
		} 
	} 
	
	DVector vVel;
	pServerDE->GetVelocity(m_hObject, &vVel);

	if (!m_bOnGround && !bFreeMovement && vVel.y < 0.0f) 
	{
		m_fTimeInAir += pServerDE->GetFrameTime();
	}
	else
	{
		m_fTimeInAir = 0.0f;
	}

/*
	// Lets see if we are in the ground or in the air.

	if (m_bOnGround || bFreeMovement) 
	{
		m_fTimeInAir = 0.0;
	} 
	else 
	{
		m_fTimeInAir += pServerDE->GetFrameTime();
	}
*/
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::UpdateInAir()
//
//	PURPOSE:	Determine if we are jumping/in air/landing
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::UpdateInAir()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DBOOL bFreeMovement = (IsFreeMovement(m_eContainerCode) || m_bBodyOnLadder || m_bBodyInLiquid);
	if (bFreeMovement) return;

	if ((m_dwControlFlags & BC_CFLG_JUMP || m_dwControlFlags & BC_CFLG_DOUBLEJUMP))
	{
		m_bJumping = DTRUE;
	}

	DDWORD bAttacking = (m_dwControlFlags & BC_CFLG_FIRING);

	
	// Check to see if we are in the air or not...and if so, are we
	// moving up, or down...

	if (!m_bOnGround)
	{
		DVector vVel;
		pServerDE->GetVelocity(m_hObject, &vVel);

		if (vVel.y > 0.0f && m_bJumping && !m_bJumped)
		{
			m_bJumped = DTRUE;

			if (IsMecha())
			{
				char* pSounds[] = { "Sounds\\Player\\Mcajump1.wav",
									"Sounds\\Player\\Mcajump2.wav" };
				PlaySound(pSounds[GetRandom(0,1)], SOUNDPRIORITYMOD_HIGH, m_fSoundRadius, DTRUE);
			}
			else
			{
				char* pSounds[] = { "Sounds\\Player\\OnFoot\\jump1.wav",
									"Sounds\\Player\\OnFoot\\jump2.wav" };
				PlaySound(pSounds[GetRandom(0,1)], SOUNDPRIORITYMOD_HIGH, m_fSoundRadius, DTRUE);
			}
		}
	}
	else if (m_bJumped)
	{
		m_bJumped  = DFALSE;
		m_bJumping = DFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::UpdateControlFlags
//
//	PURPOSE:	Set the control flags
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::UpdateControlFlags()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	if ((m_dwControlFlags & BC_CFLG_RIGHT) && (m_dwControlFlags & BC_CFLG_STRAFE))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_RIGHT;
	}

	if ((m_dwControlFlags & BC_CFLG_LEFT) && (m_dwControlFlags & BC_CFLG_STRAFE))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_LEFT;
	}

	if ( (m_dwControlFlags & BC_CFLG_FORWARD) ||
		 (m_dwControlFlags & BC_CFLG_REVERSE) ||
		 (m_dwControlFlags & BC_CFLG_STRAFE_LEFT) ||
		 (m_dwControlFlags & BC_CFLG_STRAFE_RIGHT) )
	{
		m_dwControlFlags |= BC_CFLG_MOVING;
	}
	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::UpdateSounds()
//
//	PURPOSE:	Update the currently playing sounds
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::UpdateSounds()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;
	
	
	// Turn off vehicle mode sound if we aren't in vehicle mode...
	
	if (m_hVehicleSound && m_bBipedal)
	{
		pServerDE->KillSound(m_hVehicleSound);
		m_hVehicleSound = DNULL;
	}


	// See if it is time to shut up...

	if (m_hCurDlgSnd)
	{
		DBOOL bIsDone = DFALSE;
		if (pServerDE->IsSoundDone(m_hCurDlgSnd, &bIsDone) != LT_OK || bIsDone)
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
		PlaySound("Sounds\\Player\\splash1.wav", SOUNDPRIORITYMOD_HIGH, m_fSoundRadius, DFALSE);
	}
	else if (!m_bBodyWasInLiquid && m_bBodyInLiquid)  // or going into
	{
		PlaySound("Sounds\\Player\\splash2.wav", SOUNDPRIORITYMOD_HIGH, m_fSoundRadius, DFALSE);
	}
		
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::PlaySound
//
//	PURPOSE:	Play the specified sound
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::PlaySound(char *pSoundName, DBYTE nPriorityMod, DFLOAT fRadius, DBOOL bAttached)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DVector vPos;

	if (bAttached)
	{
		PlaySoundFromObject( m_hObject, pSoundName, fRadius, m_nBasePriority + nPriorityMod, DFALSE, DFALSE, DFALSE, 100 );
	}
	else
	{
		pServerDE->GetObjectPos( m_hObject, &vPos );
		PlaySoundFromPos( &vPos, pSoundName, fRadius, m_nBasePriority + nPriorityMod, DFALSE, DFALSE, DFALSE, 100 );
	}
}

void CBaseCharacter::PlaySound(HSTRING hstrSoundName, DBYTE nPriorityMod, DFLOAT fRadius, DBOOL bAttached )
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (hstrSoundName)
	{
		char* pSound = pServerDE->GetStringData(hstrSoundName);
		if (pSound && pSound[0] != '\0')
		{
			PlaySound( pSound, nPriorityMod, fRadius, bAttached );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::PlayDamageSound
//
//	PURPOSE:	Play a damage sound
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::PlayDamageSound(DamageType eType)
{
	if (m_eCurDlgSndType == CST_DAMAGE) return;

	// OPTIMIZATION - timing...
	//DCounter dcounter;
	//g_pServerDE->StartCounter(&dcounter);
	// OPTIMIZATION - timing...

	PlayDialogSound(GetDamageSound(eType), CST_DAMAGE);

	// OPTIMIZATION - timing...
	//g_pServerDE->BPrint("PlayDamageSound: %d ticks", g_pServerDE->EndCounter(&dcounter));
	// OPTIMIZATION - timing...
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::PlayDialogSound
//
//	PURPOSE:	Play a dialog sound
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::PlayDialogSound(char* pSound, CharacterSoundType eType,
									 DBOOL bAtObjectPos)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pSound || !pSound[0]) return;
	if ((m_damage.IsDead() && eType != CST_DEATH)) return;
	if (eType == CST_EXCLAMATION && !m_bCanPlayDialogSound) return;


	// Kill current sound...

	KillDlgSnd();

	if (bAtObjectPos)
	{
		PlaySound( pSound, SOUNDPRIORITYMOD_HIGH, m_fSoundRadius, DFALSE );
	}
	else
	{
		m_hCurDlgSnd = PlaySoundFromObject(m_hObject, pSound, m_fSoundRadius, m_nBasePriority + SOUNDPRIORITYMOD_HIGH, 
			DFALSE, DTRUE, DTRUE);
	}

	if (m_hCurDlgSnd && m_hDlgSprite && eType == CST_EXCLAMATION)
	{
		// Reset the filename in case the file has changed somehow...

		DVector vScale;
		char* pFilename = GetDialogSpriteFilename(vScale);
		if (!pFilename) return;
		pServerDE->SetObjectFilenames(m_hDlgSprite, pFilename, "");

		// Show the sprite...

		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hDlgSprite);
		pServerDE->SetObjectFlags(m_hDlgSprite, dwFlags | FLAG_VISIBLE);
	}
	
	m_eCurDlgSndType = eType;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::KillDlgSnd
//
//	PURPOSE:	Kill the current dialog sound
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::KillDlgSnd()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hCurDlgSnd)
	{
		pServerDE->KillSound(m_hCurDlgSnd);
		m_hCurDlgSnd	 = DNULL;
		m_eCurDlgSndType = CST_NONE;
	}

	if (m_hDlgSprite)
	{
		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hDlgSprite);
		pServerDE->SetObjectFlags(m_hDlgSprite, dwFlags & ~FLAG_VISIBLE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::GetDamageSound
//
//	PURPOSE:	Determine what damage sound to play
//
// ----------------------------------------------------------------------- //

char* CBaseCharacter::GetDamageSound(DamageType eType)
{
	return DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::GetDeathSound
//
//	PURPOSE:	Determine what death sound to play
//
// ----------------------------------------------------------------------- //

char* CBaseCharacter::GetDeathSound()
{	
	return DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::GetFootStepSound
//
//	PURPOSE:	Determine what foot step sound to play
//
// ----------------------------------------------------------------------- //

char* CBaseCharacter::GetFootStepSound(SurfaceType eSurface)
{
	if (!m_bOnGround) return DNULL;

	m_bLeftFoot = !m_bLeftFoot;
	int nMode = IsMecha() ? PM_CURRENT_MCA : PM_MODE_FOOT;

	return ::GetFootStepSound(eSurface, nMode, m_bLeftFoot);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::PlayVehicleSound()
//
//	PURPOSE:	Play the vehicle sound
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::PlayVehicleSound()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hVehicleSound)
	{
		pServerDE->KillSound(m_hVehicleSound);
	}

	m_hVehicleSound = PlaySoundFromObject( m_hObject, m_pVehicleSound, m_fSoundRadius, m_nBasePriority + SOUNDPRIORITYMOD_LOW, DTRUE, DTRUE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::PlayFootStepSound()
//
//	PURPOSE:	Play foot step sound
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::PlayFootStepSound(SurfaceType eSurface)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	char* pSound = GetFootStepSound(eSurface);

	if (pSound) 
	{
		PlaySoundFromObject(m_hObject, pSound, FOOTSTEP_SOUND_RADIUS, m_nBasePriority + SOUNDPRIORITYMOD_HIGH, DFALSE, DFALSE, DFALSE, 100, DTRUE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::PlayDeathSound()
//
//	PURPOSE:	Play the death sound
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::PlayDeathSound()
{
	PlaySound(GetDeathSound(), SOUNDPRIORITYMOD_HIGH, m_fSoundRadius, DFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::HandleDead()
//
//	PURPOSE:	Okay, death animation is done...
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::HandleDead(DBOOL bRemoveObj)
{
	if (!m_hObject) return;

	if (m_bCreateBody) 
	{
		CreateBody();
	}

	if (bRemoveObj)
	{
		RemoveObject();
	}

	RemoveAllPowerups();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::RemoveObject()
//
//	PURPOSE:	Handle removing character objects
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::RemoveObject()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	// Remove Attachments...

	if (m_hDlgSprite)
	{
		HATTACHMENT hAttachment;
		if (pServerDE->FindAttachment(m_hObject, m_hDlgSprite, &hAttachment) == DE_OK)
		{
			pServerDE->RemoveAttachment(hAttachment);
		}

		g_pServerDE->BreakInterObjectLink( m_hObject, m_hDlgSprite );
		pServerDE->RemoveObject(m_hDlgSprite);
		m_hDlgSprite = DNULL;
	}

	// Make sure this object is removed from the global CharacterMgr

	CCharacterMgr* pCharMgr = CCharacterMgr::GetMgr();
	if (!pCharMgr) return;

	pCharMgr->Remove(this);

	// Remove the engine object...

	pServerDE->RemoveObject(m_hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::CreateBody()
//
//	PURPOSE:	Create the body prop
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::CreateBody()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

	HCLASS hClass = pServerDE->GetClass("BodyProp");
	if (!hClass) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);
	
	pServerDE->GetModelFilenames(m_hObject, theStruct.m_Filename, MAX_CS_FILENAME_LEN, theStruct.m_SkinName, MAX_CS_FILENAME_LEN);
	VEC_SET(theStruct.m_Pos, 0.0f, 0.1f, 0.0f);
	VEC_ADD(theStruct.m_Pos, theStruct.m_Pos, vPos);
	pServerDE->GetObjectRotation(m_hObject, &theStruct.m_Rotation);
	
	// Allocate an object...

	BodyProp* pProp = (BodyProp *)pServerDE->CreateObject(hClass, &theStruct);
	if (!pProp) return;

	pProp->Setup(this);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::CreateBoundingBox()
//
//	PURPOSE:	Create a bounding box
//
// ----------------------------------------------------------------------- //
	
void CBaseCharacter::CreateBoundingBox()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);
	VEC_COPY(theStruct.m_Pos, vPos);

	SAFE_STRCPY(theStruct.m_Filename, "Models\\Props\\1x1_square.abc");
	SAFE_STRCPY(theStruct.m_SkinName, "SpecialFX\\smoke.dtx");
	theStruct.m_ObjectType  = OT_MODEL;
	theStruct.m_Flags = FLAG_VISIBLE | FLAG_MODELGOURAUDSHADE |  FLAG_MODELWIREFRAME;
	theStruct.m_fDeactivationTime = 0.001f;

	HCLASS hClass = pServerDE->GetClass("BaseClass");
	LPBASECLASS pModel = pServerDE->CreateObject(hClass, &theStruct);

	if (pModel)
	{
		m_hBoundingBox = pModel->m_hObject;

		DVector vDims;
		pServerDE->GetObjectDims(m_hObject, &vDims);

		DVector vScale;
		VEC_DIVSCALAR(vScale, vDims, 0.5f);
		pServerDE->ScaleObject(m_hBoundingBox, &vScale);
	}

	DVector vOffset;
	DRotation rOffset;
	VEC_INIT(vOffset);
	ROT_INIT(rOffset);

	HATTACHMENT hAttachment;
	DRESULT dRes = pServerDE->CreateAttachment(m_hObject, m_hBoundingBox, DNULL, 
											   &vOffset, &rOffset, &hAttachment);
	if (dRes != DE_OK)
	{
		pServerDE->RemoveObject(m_hBoundingBox);
		m_hBoundingBox = DNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::UpdateBoundingBox()
//
//	PURPOSE:	Update the bounding box
//
// ----------------------------------------------------------------------- //
	
void CBaseCharacter::UpdateBoundingBox()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hBoundingBox) return;

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);
	pServerDE->SetObjectPos(m_hBoundingBox, &vPos);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::SetDims()
//
//	PURPOSE:	Set the dims for the character
//
// ----------------------------------------------------------------------- //
	
DBOOL CBaseCharacter::SetDims(DVector* pvDims, DBOOL bSetLargest)
{
	CServerDE* pServerDE = GetServerDE();
	if (!m_hObject || !pServerDE || !pvDims) return DFALSE;

	DBOOL bRet = DTRUE;

	// Calculate what the dims should be based on our model size...

	DVector vNewDims;
	VEC_MULSCALAR(vNewDims, *pvDims, m_fDimsScale[m_eModelSize]);


	DVector vOldDims;
	pServerDE->GetObjectDims(m_hObject, &vOldDims);


	// Only update dims if they have changed...

	if ((vNewDims.x > vOldDims.x - DIMS_EPSILON && vNewDims.x < vOldDims.x + DIMS_EPSILON) &&
		(vNewDims.y > vOldDims.y - DIMS_EPSILON && vNewDims.y < vOldDims.y + DIMS_EPSILON) &&
		(vNewDims.z > vOldDims.z - DIMS_EPSILON && vNewDims.z < vOldDims.z + DIMS_EPSILON))
	{
		return DTRUE;  // Setting of dims didn't actually fail
	}


	// Try to set our new dims...

	if (pServerDE->SetObjectDims2(m_hObject, &vNewDims) == DE_ERROR)
	{
		if (bSetLargest)
		{
			pServerDE->SetObjectDims2(m_hObject, &vNewDims);
		}

		bRet = DFALSE; // Didn't set to new dims...
	}


	// See if we need to move the object down...

	if (vNewDims.y < vOldDims.y)
	{
		DVector vPos;
		pServerDE->GetObjectPos(m_hObject, &vPos);

		vPos.y -= (vOldDims.y - vNewDims.y);
		vPos.y += .01f; // Fudge factor

		// This forces the client to move to the server's position because it's teleporting.
		//pServerDE->SetObjectPos(m_hObject, &vPos);
		pServerDE->MoveObject(m_hObject, &vPos);
	}


	// If we have a bounding box, update it...

	if (m_hBoundingBox)
	{
		DVector vScale;
		VEC_DIVSCALAR(vScale, vNewDims, 0.5f);
		pServerDE->ScaleObject(m_hBoundingBox, &vScale);
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::SpawnWeapon()
//
//	PURPOSE:	Spawn the appropriate weapon
//
// ----------------------------------------------------------------------- //
	
void CBaseCharacter::SpawnWeapon()
{
	CServerDE* pServerDE = GetServerDE();
	if (!m_hObject || !pServerDE || (m_eModelSize == MS_SMALL)) return;

	CWeapon* pWeapon = m_weapons.GetCurWeapon();

	if (pWeapon)
	{
		// If this is a melee weapon, we don't want to spawn a powerup...

		RiotWeaponId nId = pWeapon->GetId();

		if (GetWeaponType(nId) == MELEE) return;

		DVector vPos;
		DRotation rRot;
		if (!pServerDE->GetModelNodeTransform(m_hObject, m_pHandName, &vPos, &rRot))
		{
			pServerDE->GetObjectPos(m_hObject, &vPos);
			pServerDE->GetObjectRotation(m_hObject, &rRot);
		}

		DDWORD nAmmo = GetSpawnedAmmo(pWeapon->GetId());

		char* pName = pServerDE->GetObjectName(m_hObject);
		pName = (pName && strlen(pName) ? pName : "noname");

		char buf[200];
		sprintf(buf, "WeaponPowerup Gravity 1;Ammo %d;Small %d;Large %d;WeaponType %d;Name %s_gun", 
			    nAmmo, (int)(m_eModelSize == MS_SMALL), (int)(m_eModelSize == MS_LARGE), nId, pName);

		SpawnItem(buf, vPos, rRot);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::SpawnItem()
//
//	PURPOSE:	Spawn the specified item
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::SpawnItem(char* pItem, DVector & vPos, DRotation & rRot)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pItem) return;

	BaseClass* pObj = SpawnObject(pItem, &vPos, &rRot);

	if (pObj && pObj->m_hObject)
	{
		DVector vAccel;
		VEC_SET(vAccel, GetRandom(0.0f, 300.0f), GetRandom(100.0f, 200.0f), GetRandom(0.0f, 300.0f));
		pServerDE->SetAcceleration(pObj->m_hObject, &vAccel);

		DVector vVel;
		VEC_SET(vVel, GetRandom(0.0f, 100.0f), GetRandom(200.0f, 400.0f), GetRandom(0.0f, 100.0f));
		pServerDE->SetVelocity(pObj->m_hObject, &vVel);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::StartDeath()
//
//	PURPOSE:	Start dying
//
// ----------------------------------------------------------------------- //
	
void CBaseCharacter::StartDeath()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || m_bStartedDeath) return;

	m_bStartedDeath = DTRUE;

	PlayDeathSound();

	if (m_bSpawnWeapon) 
	{
		SpawnWeapon();
	}

	if (m_hstrSpawnItem)
	{
		char* pItem = pServerDE->GetStringData(m_hstrSpawnItem);
		if (pItem)
		{
			// Add gravity to the item...

			char buf[300];
			sprintf(buf, "%s Gravity 1", pItem);

			DVector vPos;
			pServerDE->GetObjectPos(m_hObject, &vPos);

			DRotation rRot;
			ROT_INIT(rRot);

			SpawnItem(buf, vPos, rRot);
		}
	}

	RemoveHandHeldWeapon();

	m_weapons.DeselectCurWeapon();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::UpdateContainerCode()
//
//	PURPOSE:	Update our container code
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::UpdateContainerCode()
{
	m_eContainerCode = CC_NONE;

	CServerDE* pServerDE = GetServerDE();
	if (!m_hObject || !pServerDE) return;

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

	VEC_ADD(vPos, vPos, GetHeadOffset());

	HOBJECT objList[1];
	DDWORD dwNum = pServerDE->GetPointContainers(&vPos, objList, 1);

	DDWORD dwUserFlags = 0;
	if (dwNum > 0 && objList[0])
	{
		dwUserFlags = pServerDE->GetObjectUserFlags(objList[0]);
	}

	if (dwNum > 0 && (dwUserFlags & USRFLG_VISIBLE))
	{
		D_WORD code;
		if (pServerDE->GetContainerCode(objList[0], &code))
		{
			m_eContainerCode = (ContainerCode)code;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::GetHeadOffset()
//
//	PURPOSE:	Update the offset from our position to our head
//
// ----------------------------------------------------------------------- //

DVector CBaseCharacter::GetHeadOffset()
{
	DVector vOffset;
	VEC_INIT(vOffset);

	CServerDE* pServerDE = GetServerDE();
	if (!m_hObject || !pServerDE) return vOffset;
	
	DVector vDims;
	pServerDE->GetObjectDims(m_hObject, &vDims);

	// Just make the default offset a bit above the waist...

	vOffset.y = vDims.y * 0.75f;

	return vOffset;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::UpdateCheatInfo()
//
//	PURPOSE:	Update cheat info
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::UpdateCheatInfo()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// See if we should show our bounding box...

	HCONVAR	hVar  = pServerDE->GetGameConVar("ShowDims");
	char* pVar = pServerDE->GetVarValueString(hVar);

	if (!pVar) return;

	if (_stricmp(pVar, "0") != 0)
	{
		if (!m_hBoundingBox) 
		{
			CreateBoundingBox();
		}
	}
	else if (m_hBoundingBox)
	{
		HATTACHMENT hAttachment;
		if (pServerDE->FindAttachment(m_hObject, m_hBoundingBox, &hAttachment) == DE_OK)
		{
			pServerDE->RemoveAttachment(hAttachment);
		}

		pServerDE->RemoveObject(m_hBoundingBox);
		m_hBoundingBox = DNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::HandleBigGunsCheat()
//
//	PURPOSE:	Handle big guns cheat
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::HandleBigGunsCheat()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hHandHeldWeapon) return;

	CWeapon* pWeapon = m_weapons.GetCurWeapon();
	if (!pWeapon) return;

	DVector vScale = GetHandWeaponScale(pWeapon->GetId(), m_eModelSize);

	// Check for big guns...

	HCONVAR	hVar  = pServerDE->GetGameConVar("BigGuns");
	char* pVar = pServerDE->GetVarValueString(hVar);

	hVar = pServerDE->GetGameConVar("BigGunsScale");

	DFLOAT fVal = BIGGUNS_SCALE_VALUE;
	if (hVar)
	{
		fVal = pServerDE->GetVarValueFloat(hVar);
	}

	if (pVar && _stricmp(pVar, "0") != 0)
	{
		VEC_MULSCALAR(vScale, vScale, fVal);
	}

	pServerDE->ScaleObject(m_hHandHeldWeapon, &vScale);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::GetRecoilAni()
//
//	PURPOSE:	Get a recoil ani based on the last hit location
//
// ----------------------------------------------------------------------- //

HMODELANIM CBaseCharacter::GetRecoilAni()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return INVALID_ANI;

	HMODELANIM hAni = INVALID_ANI;

	char* pNodeName = GetModelNodeName(m_dwLastHitNode, m_nModelId, m_eModelSize);
	if (!pNodeName) return hAni;

	switch (GetNodeType(m_dwLastHitNode))
	{
		case NT_HEAD:
		{
			char* AniName[] = { "head_forward", "head_side", "head_back", "head_exaggerated" };
			hAni = pServerDE->GetAnimIndex(m_hObject, AniName[GetRandom(0,3)]);
		}
		break;

		case NT_LARM:
			hAni = pServerDE->GetAnimIndex(m_hObject, "l_shoulder");
		break;

		case NT_RARM:
			hAni = pServerDE->GetAnimIndex(m_hObject, "r_shoulder");
		break;

		case NT_LLEG:
			hAni = pServerDE->GetAnimIndex(m_hObject, "l_foot");
		break;

		case NT_RLEG:
			hAni = pServerDE->GetAnimIndex(m_hObject, "r_foot");
		break;

		case NT_TORSO:
			hAni = pServerDE->GetAnimIndex(m_hObject, "general_body");
		break;

		case NT_PELVIS:
		{
			char* AniName[] = { "ass", "groin" };
			hAni = pServerDE->GetAnimIndex(m_hObject, AniName[GetRandom(0,1)]);
		}
		break;

		default:
		break;
	}

	return hAni;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::GetNodeType()
//
//	PURPOSE:	Get the type of node 
//
// ----------------------------------------------------------------------- //

NodeType CBaseCharacter::GetNodeType(DDWORD nNode)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return NT_TORSO;

	NodeType eType = NT_TORSO;

	char* pNodeName = GetModelNodeName(nNode, m_nModelId, m_eModelSize);
	if (!pNodeName) return NT_TORSO;

	if (strnicmp(pNodeName, "torso", 5) == 0)
	{
		 eType = NT_TORSO;
	}
	else if (strnicmp(pNodeName, "left_arm", 8) == 0)
	{
		 eType = NT_LARM;
	}
	else if (strnicmp(pNodeName, "left_leg", 8) == 0)
	{
		 eType = NT_LLEG;
	}
	else if (strnicmp(pNodeName, "right_arm", 9) == 0)
	{
		 eType = NT_RARM;
	}
	else if (strnicmp(pNodeName, "right_leg", 9) == 0)
	{
		 eType = NT_RLEG;
	}
	else if (strnicmp(pNodeName, "pelvis", 6) == 0)
	{
		 eType = NT_PELVIS;
	}
	else if (strnicmp(pNodeName, "head", 4) == 0)
	{
		 eType = NT_HEAD;
	}
	
	return eType;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::CanPickupWeapon()
//
//	PURPOSE:	Can I pick up the specified weapon
//
// ----------------------------------------------------------------------- //

DBOOL CBaseCharacter::CanCarryWeapon(DBYTE nWeaponId)
{
	if ((GUN_FIRSTMECH_ID <= nWeaponId && nWeaponId <= GUN_LASTMECH_ID))
	{
		if (IsMecha()) return DTRUE;
		else return DFALSE;
	}
	else if ((GUN_FIRSTONFOOT_ID <= nWeaponId && nWeaponId <= GUN_LASTONFOOT_ID))
	{
		if (IsMecha()) return DFALSE;
		else return DTRUE;
	}

	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::SetLastFireInfo()
//
//	PURPOSE:	Set the last fire info
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::SetLastFireInfo(DVector* pvFiredPos, DVector* pvImpactPos, 
									 DBYTE nWeaponId, DBOOL bSilenced)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pvFiredPos || !pvImpactPos) return;

	VEC_COPY(m_vLastFiredPos, *pvFiredPos);
	VEC_COPY(m_vLastImpactPos, *pvImpactPos);
	m_nLastFiredWeapon	= nWeaponId;
	m_fLastFiredTime	= pServerDE->GetTime();
	m_bLastFireSilenced = bSilenced;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::GetLastFireInfo()
//
//	PURPOSE:	Get the last fire info
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::GetLastFireInfo(DVector & vFiredPos, DVector & vImpactPos, 
									 DBYTE & nWeaponId, DFLOAT & fTime,
									 DBOOL & bSilenced)
{
	VEC_COPY(vFiredPos, m_vLastFiredPos);
	VEC_COPY(vImpactPos, m_vLastImpactPos);
	nWeaponId = m_nLastFiredWeapon;
	fTime = m_fLastFiredTime;
	bSilenced = m_bLastFireSilenced;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::Save(HMESSAGEWRITE hWrite)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	// Save name of hand-held weapon model...

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hHandHeldWeapon);
	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hDlgSprite);

	pServerDE->WriteToMessageByte(hWrite, m_bTakesSqueakyDamage);
	pServerDE->WriteToMessageByte(hWrite, m_bCrying);
	pServerDE->WriteToMessageByte(hWrite, m_nSqueakyCount);
	pServerDE->WriteToMessageByte(hWrite, m_bCreateBody);
	pServerDE->WriteToMessageByte(hWrite, m_bMoveToFloor);
	pServerDE->WriteToMessageByte(hWrite, m_eModelSize);
	pServerDE->WriteToMessageDWord(hWrite, m_dwControlFlags);
	pServerDE->WriteToMessageDWord(hWrite, m_dwLastFrameCtlFlgs);
	pServerDE->WriteToMessageFloat(hWrite, m_fTimeInAir);
	pServerDE->WriteToMessageByte(hWrite, m_eDeathType);
	pServerDE->WriteToMessageByte(hWrite, m_bStartedDeath);
	pServerDE->WriteToMessageByte(hWrite, m_bOneHandedWeapon);
	pServerDE->WriteToMessageByte(hWrite, m_bTransforming);
	pServerDE->WriteToMessageByte(hWrite, m_bLanding);
	pServerDE->WriteToMessageByte(hWrite, m_bAllowRun);
	pServerDE->WriteToMessageByte(hWrite, m_bAllowMovement);
	pServerDE->WriteToMessageByte(hWrite, m_bOnGround);
	pServerDE->WriteToMessageByte(hWrite, m_bLastOnGround);
	pServerDE->WriteToMessageByte(hWrite, m_bBipedal);
	pServerDE->WriteToMessageByte(hWrite, m_bBipedalLastFrame);
	pServerDE->WriteToMessageByte(hWrite, m_bSpectatorMode);
	pServerDE->WriteToMessageByte(hWrite, 0);
	pServerDE->WriteToMessageByte(hWrite, 0);
	pServerDE->WriteToMessageByte(hWrite, m_eContainerCode);
	pServerDE->WriteToMessageByte(hWrite, m_eLastContainerCode);
	pServerDE->WriteToMessageByte(hWrite, m_bBodyInLiquid);
	pServerDE->WriteToMessageByte(hWrite, m_bBodyWasInLiquid);
	pServerDE->WriteToMessageByte(hWrite, m_bBodyOnLadder);
	pServerDE->WriteToMessageByte(hWrite, 0);
	pServerDE->WriteToMessageByte(hWrite, m_bJumping);
	pServerDE->WriteToMessageByte(hWrite, m_bJumped);
	pServerDE->WriteToMessageByte(hWrite, m_bRecoiling);
	pServerDE->WriteToMessageFloat(hWrite, 0.0f);
	pServerDE->WriteToMessageVector(hWrite, &m_vOldCharacterColor);
	pServerDE->WriteToMessageFloat(hWrite, m_fOldCharacterAlpha);
	pServerDE->WriteToMessageByte(hWrite, m_bCharacterHadShadow);
	pServerDE->WriteToMessageDWord(hWrite, m_dwLastHitNode);
	pServerDE->WriteToMessageByte(hWrite, m_bLeftFoot);
	pServerDE->WriteToMessageByte(hWrite, m_nModelId);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSpawnItem);
	pServerDE->WriteToMessageVector(hWrite, &m_vLastFiredPos);
	pServerDE->WriteToMessageVector(hWrite, &m_vLastImpactPos);
	pServerDE->WriteToMessageByte(hWrite, m_nLastFiredWeapon);
	pServerDE->WriteToMessageFloat(hWrite, m_fLastFiredTime);
	pServerDE->WriteToMessageByte(hWrite, m_bLastFireSilenced);
	pServerDE->WriteToMessageFloat(hWrite, m_fDefaultHitPts);
	pServerDE->WriteToMessageFloat(hWrite, m_fDefaultArmor);
	pServerDE->WriteToMessageFloat(hWrite, m_fSoundRadius);
	pServerDE->WriteToMessageFloat(hWrite, 0.0f);
	pServerDE->WriteToMessageFloat(hWrite, m_fBaseMoveAccel);
	pServerDE->WriteToMessageFloat(hWrite, m_fLadderVel);
	pServerDE->WriteToMessageFloat(hWrite, m_fSwimVel);
	pServerDE->WriteToMessageFloat(hWrite, m_fRunVel);
	pServerDE->WriteToMessageFloat(hWrite, m_fWalkVel);
	pServerDE->WriteToMessageFloat(hWrite, m_fJumpVel);
	pServerDE->WriteToMessageFloat(hWrite, m_fDimsScale[0]);
	pServerDE->WriteToMessageFloat(hWrite, m_fDimsScale[1]);
	pServerDE->WriteToMessageFloat(hWrite, m_fDimsScale[2]);
	pServerDE->WriteToMessageByte(hWrite, m_bCreateDialogSprite);
	pServerDE->WriteToMessageByte(hWrite, m_bCreateHandHeldWeapon);
	pServerDE->WriteToMessageByte(hWrite, m_bSpawnWeapon);
	pServerDE->WriteToMessageByte(hWrite, m_bIsMecha);
	pServerDE->WriteToMessageByte(hWrite, m_bUsingHitDetection);
	pServerDE->WriteToMessageByte(hWrite, m_bCanPlayDialogSound);
	pServerDE->WriteToMessageByte(hWrite, m_bCanDamageBody);
	pServerDE->WriteToMessageByte(hWrite, m_cc);
	pServerDE->WriteToMessageDWord(hWrite, m_dwFlags);

	int nNumPowerups = 0;
	for (DDWORD i = 0; i < MAX_TIMED_POWERUPS; i++)
	{
		if (m_powerups[i])
		{
			nNumPowerups++;
		}
	}

	pServerDE->WriteToMessageByte(hWrite, nNumPowerups);

	if (nNumPowerups)
	{
		for (DDWORD i = 0; i < MAX_TIMED_POWERUPS; i++)
		{
			if (m_powerups[i])
			{
				m_powerups[i]->Save(hWrite);
			}
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CBaseCharacter::Load(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	float dummyFloat;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hHandHeldWeapon);
	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hDlgSprite);

	m_bTakesSqueakyDamage	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bCrying				= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_nSqueakyCount			= pServerDE->ReadFromMessageByte(hRead);
	m_bCreateBody			= pServerDE->ReadFromMessageByte(hRead);
	m_bMoveToFloor			= pServerDE->ReadFromMessageByte(hRead);
	m_eModelSize			= (ModelSize) pServerDE->ReadFromMessageByte(hRead);
	m_dwControlFlags		= pServerDE->ReadFromMessageDWord(hRead);
	m_dwLastFrameCtlFlgs	= pServerDE->ReadFromMessageDWord(hRead);
	m_fTimeInAir			= pServerDE->ReadFromMessageFloat(hRead);
	m_eDeathType			= (CharacterDeath) pServerDE->ReadFromMessageByte(hRead);
	m_bStartedDeath			= pServerDE->ReadFromMessageByte(hRead);
	m_bOneHandedWeapon		= pServerDE->ReadFromMessageByte(hRead);
	m_bTransforming			= pServerDE->ReadFromMessageByte(hRead);
	m_bLanding				= pServerDE->ReadFromMessageByte(hRead);
	m_bAllowRun				= pServerDE->ReadFromMessageByte(hRead);
	m_bAllowMovement		= pServerDE->ReadFromMessageByte(hRead);
	m_bOnGround				= pServerDE->ReadFromMessageByte(hRead);
	m_bLastOnGround			= pServerDE->ReadFromMessageByte(hRead);
	m_bBipedal				= pServerDE->ReadFromMessageByte(hRead);
	m_bBipedalLastFrame		= pServerDE->ReadFromMessageByte(hRead);
	m_bSpectatorMode		= pServerDE->ReadFromMessageByte(hRead);
	DBYTE dummyByte			= pServerDE->ReadFromMessageByte(hRead);
	dummyByte				= pServerDE->ReadFromMessageByte(hRead);
	m_eContainerCode		= (ContainerCode) pServerDE->ReadFromMessageByte(hRead);
	m_eLastContainerCode	= (ContainerCode) pServerDE->ReadFromMessageByte(hRead);
	m_bBodyInLiquid			= pServerDE->ReadFromMessageByte(hRead);
	m_bBodyWasInLiquid		= pServerDE->ReadFromMessageByte(hRead);
	m_bBodyOnLadder			= pServerDE->ReadFromMessageByte(hRead);
	dummyByte				= pServerDE->ReadFromMessageByte(hRead);
	m_bJumping				= pServerDE->ReadFromMessageByte(hRead);
	m_bJumped				= pServerDE->ReadFromMessageByte(hRead);
	m_bRecoiling			= pServerDE->ReadFromMessageByte(hRead);
	dummyFloat				= pServerDE->ReadFromMessageFloat(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vOldCharacterColor);
	m_fOldCharacterAlpha	= pServerDE->ReadFromMessageFloat(hRead);
	m_bCharacterHadShadow	= pServerDE->ReadFromMessageByte(hRead);
	m_dwLastHitNode			= pServerDE->ReadFromMessageDWord(hRead);
	m_bLeftFoot				= pServerDE->ReadFromMessageByte(hRead);
	m_nModelId				= pServerDE->ReadFromMessageByte(hRead);
	m_hstrSpawnItem			= pServerDE->ReadFromMessageHString(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vLastFiredPos);
	pServerDE->ReadFromMessageVector(hRead, &m_vLastImpactPos);
	m_nLastFiredWeapon		= pServerDE->ReadFromMessageByte(hRead);
	m_fLastFiredTime		= pServerDE->ReadFromMessageFloat(hRead);
	m_bLastFireSilenced		= pServerDE->ReadFromMessageByte(hRead);
	m_fDefaultHitPts		= pServerDE->ReadFromMessageFloat(hRead);
	m_fDefaultArmor			= pServerDE->ReadFromMessageFloat(hRead);
	m_fSoundRadius			= pServerDE->ReadFromMessageFloat(hRead);
	dummyFloat				= pServerDE->ReadFromMessageFloat(hRead);
	m_fBaseMoveAccel		= pServerDE->ReadFromMessageFloat(hRead);
	m_fLadderVel			= pServerDE->ReadFromMessageFloat(hRead);
	m_fSwimVel				= pServerDE->ReadFromMessageFloat(hRead);
	m_fRunVel				= pServerDE->ReadFromMessageFloat(hRead);
	m_fWalkVel				= pServerDE->ReadFromMessageFloat(hRead);
	m_fJumpVel				= pServerDE->ReadFromMessageFloat(hRead);
	m_fDimsScale[0]			= pServerDE->ReadFromMessageFloat(hRead);
	m_fDimsScale[1]			= pServerDE->ReadFromMessageFloat(hRead);
	m_fDimsScale[2]			= pServerDE->ReadFromMessageFloat(hRead);
	m_bCreateDialogSprite	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bCreateHandHeldWeapon = (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bSpawnWeapon			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bIsMecha				= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bUsingHitDetection	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bCanPlayDialogSound	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bCanDamageBody		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_cc					= (CharacterClass) pServerDE->ReadFromMessageByte(hRead);
	m_dwFlags				= pServerDE->ReadFromMessageDWord(hRead);
	
	DBYTE nNumPowerups		= pServerDE->ReadFromMessageByte(hRead);

	if (nNumPowerups)
	{
		for (DBYTE i = 0; i < nNumPowerups; i++)
		{
			TimedPowerup* pPowerup = new TimedPowerup; 
			
			if (pPowerup)
			{
				pPowerup->Load(hRead);
				AddTimedPowerup(pPowerup);
			}
		}
	}


	// Since these don't change, just reset them here...

	SetAnimationIndexes();
	InitializeWeapons();


	// Make sure this object is added to the global CharacterMgr...

	CCharacterMgr* pCharMgr = CCharacterMgr::GetMgr();
	if (!pCharMgr) return;

	pCharMgr->Add(this);
}
