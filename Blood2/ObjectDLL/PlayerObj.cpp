//----------------------------------------------------------
//
// MODULE  : PLAYEROBJ.CPP
//
// PURPOSE : The Player Object.
//
// CREATED : 7/16/97 11:34:30 AM
//
//----------------------------------------------------------

// Includes....
#include <stdio.h>
#include <mbstring.h>
#include "serverobj_de.h"
#include "BaseCharacter.h"
#include "PlayerObj.h"
#include "Commands.h"
#include "BloodServerShell.h"
#include "ObjectUtilities.h"
#include "GameInvItems.h"
#include "GameStartPoint.h"
#include "ClientServerShared.h"
#include "PhysicalAttributes.h"
#include "Trigger.h"
#include "ChosenSounds.h"
#include "SParam.h"
#include "CameraObj.h"
#include "smellhint.h"
#include "VoiceMgrDefs.h"
#include "ClientGibFX.h"
#include "ai_mgr.h"
#include <windows.h>
#include "RotatingDoor.h"
#include "SharedMovement.h"
#include "VolumeBrush.h"
#include "Physics_lt.h"
#include "TeamMgr.h"
#include "FlagObjects.h"
#include "SoundTypes.h"

#define ALPHA_NORMAL		1.0f
#define ALPHA_CLOAKED		0.99f
#define ALPHA_INVISIBLE		0.08f

// Binding defines
#define	SERVER_REGEN_TIME			4.0f
#define	SERVER_REGEN_HEALPOINTS	1
#define	BINDING_REGEN_TIME			4.0f
#define	BINDING_REGEN_HEALPOINTS	1
#define BINDING_INCREASE_DAMAGE_AMT	1.2f		
#define BINDING_QUICKNESS_FACTOR	0.5f

#define PINGPERIOD			0.5f

#define DEFAULT_LEASHLEN	450

#define TELEPORTPERIOD		3.0f

#define DEFAULT_JUMPVEL		600.0f


#define DEFAULT_BULLETS		50

CPlayerObj* g_pPlayerObj = DNULL;

BOOL	g_bLevelChangeCharacter = DFALSE;
int		g_nLevelChangeCharacter = CHARACTER_CALEB;

// Defines....
CharacterValues gCharacterValues[MAX_CHARACTERS] = {
	// Caleb
	{
		"Caleb",
		"Models\\Characters\\Caleb.abc",
		"Skins\\Characters\\Caleb1.dtx",
		82.0f,	// Standing height
		26.0f,	// Crouching height
		16.0f,	// Dead Height
		36.0f,	// Standing eyelevel
		12.0f,	// Crouching eyelevel
		0.0f	// dead eyelevel
	},
	// Ophelia
	{
		"Ophelia",
		"Models\\Characters\\Ophelia.abc",
		"Skins\\Characters\\Ophelia.dtx",
		76.0f,	// Standing height
		26.0f,	// Crouching height
		16.0f,	// Dead Height
		32.0f,	// Standing eyelevel
		12.0f,	// Crouching eyelevel
		0.0f	// dead eyelevel
	},
	// Ishmael
	{
		"Ishmael",
		"Models\\Characters\\Ishmael.abc",
		"Skins\\Characters\\Ishmael.dtx",
		82.0f,	// Standing height
		26.0f,	// Crouching height
		16.0f,	// Dead Height
		36.0f,	// Standing eyelevel
		12.0f,	// Crouching eyelevel
		0.0f	// dead eyelevel
	},
	// Gabriella
	{
		"Gabriella",
		"Models\\Characters\\Gabriella.abc",
		"Skins\\Characters\\Gabriella.dtx",
		88.0f,	// Standing height
		26.0f,	// Crouching height
		16.0f,	// Dead Height
		42.0f,	// Standing eyelevel
		12.0f,	// Crouching eyelevel
		0.0f	// dead eyelevel
	}

#ifdef _ADDON
	,
	// Male Cultist
	{
		"M_Cultist",
		"Models_ao\\Characters_ao\\m_cultist1_bb.abc",
		"Skins\\Enemies\\m_cultist1.dtx",
		82.0f,	// Standing height
		26.0f,	// Crouching height
		16.0f,	// Dead Height
		36.0f,	// Standing eyelevel
		12.0f,	// Crouching eyelevel
		0.0f	// dead eyelevel
	},
	// Female Cultist
	{
		"F_Cultist",
		"Models_ao\\Characters_ao\\f_cultist1_bb.abc",
		"Skins\\Enemies\\f_cultist1.dtx",
		76.0f,	// Standing height
		26.0f,	// Crouching height
		16.0f,	// Dead Height
		32.0f,	// Standing eyelevel
		12.0f,	// Crouching eyelevel
		0.0f	// dead eyelevel
	},
	// Soul Drudge
	{
		"SoulDrudge",
		"Models_ao\\Characters_ao\\SoulDrudge_bb.abc",
		"Skins\\Enemies\\SoulDrudge.dtx",
		85.0f,	// Standing height
		29.0f,	// Crouching height
		16.0f,	// Dead Height
		38.0f,	// Standing eyelevel
		14.0f,	// Crouching eyelevel
		0.0f	// dead eyelevel
	},
	// Prophet
	{
		"Prophet",
		"Models_ao\\Characters_ao\\Prophet_bb.abc",
		"Skins\\Enemies\\Prophet.dtx",
		81.0f,	// Standing height
		25.0f,	// Crouching height
		16.0f,	// Dead Height
		35.0f,	// Standing eyelevel
		11.0f,	// Crouching eyelevel
		0.0f	// dead eyelevel
	}
#endif

};

char *gCharacterSkins[MAX_CHARACTERS][MULTIPLAY_SKIN_MAX+1] =
{
	{
		"Skins\\Characters\\Caleb1.dtx",
		"Skins\\Characters\\CalMULTI_R.dtx",
		"Skins\\Characters\\CalMULTI_B.dtx",
		"Skins\\Characters\\CalMULTI_G.dtx",
		"Skins\\Characters\\CalMULTI_Y.dtx",
		"Skins\\Characters\\CalTEAM_B.dtx",
		"Skins\\Characters\\CalTEAM_R.dtx",
	},
	{
		"Skins\\Characters\\Ophelia.dtx",
		"Skins\\Characters\\OphMULTI_R.dtx",
		"Skins\\Characters\\OphMULTI_B.dtx",
		"Skins\\Characters\\OphMULTI_G.dtx",
		"Skins\\Characters\\OphMULTI_Y.dtx",
		"Skins\\Characters\\OphTEAM_B.dtx",
		"Skins\\Characters\\OphTEAM_R.dtx",
	},
	{
		"Skins\\Characters\\Ishmael.dtx",
		"Skins\\Characters\\IshMULTI_R.dtx",
		"Skins\\Characters\\IshMULTI_B.dtx",
		"Skins\\Characters\\IshMULTI_G.dtx",
		"Skins\\Characters\\IshMULTI_Y.dtx",
		"Skins\\Characters\\IshTEAM_B.dtx",
		"Skins\\Characters\\IshTEAM_R.dtx",
	},
	{
		"Skins\\Characters\\Gabriella.dtx",
		"Skins\\Characters\\GabMULTI_R.dtx",
		"Skins\\Characters\\GabMULTI_B.dtx",
		"Skins\\Characters\\GabMULTI_G.dtx",
		"Skins\\Characters\\GabMULTI_Y.dtx",
		"Skins\\Characters\\GabTEAM_B.dtx",
		"Skins\\Characters\\GabTEAM_R.dtx",
	},

#ifdef _ADDON
	{
		"Skins\\Enemies\\m_cultist1.dtx",	// male cultist
		"Skins_ao\\Enemies_ao\\m_Cultist5.dtx",
		"Skins_ao\\Enemies_ao\\m_Cultist4.dtx",
		"Skins_ao\\Enemies_ao\\m_Cultist6.dtx",
		"Skins\\Enemies\\m_cultist3.dtx",
		"Skins_ao\\Enemies_ao\\mc_team_b.dtx",
		"Skins_ao\\Enemies_ao\\mc_team_r.dtx",
	},
	{
		"Skins\\Enemies\\f_cultist1.dtx",	// female cultist
		"Skins\\Enemies\\f_cultist1.dtx",
		"Skins\\Enemies\\f_cultist1.dtx",
		"Skins\\Enemies\\f_cultist1.dtx",
		"Skins\\Enemies\\f_cultist1.dtx",
		"Skins_ao\\Enemies_ao\\fc_team_b.dtx",
		"Skins_ao\\Enemies_ao\\fc_team_r.dtx",
	},
	{
		"Skins\\Enemies\\souldrudge.dtx",	// soul drudge
		"Skins\\Enemies\\souldrudge.dtx",
		"Skins\\Enemies\\souldrudge.dtx",
		"Skins\\Enemies\\souldrudge.dtx",
		"Skins\\Enemies\\souldrudge.dtx",
		"Skins_ao\\Enemies_ao\\sd_team_b.dtx",
		"Skins_ao\\Enemies_ao\\sd_team_r.dtx",
	},
	{
		"Skins\\Enemies\\prophet.dtx",		// prophet
		"Skins\\Enemies\\prophet.dtx",
		"Skins\\Enemies\\prophet.dtx",
		"Skins\\Enemies\\prophet.dtx",
		"Skins\\Enemies\\prophet.dtx",
		"Skins_ao\\Enemies_ao\\p_team_b.dtx",
		"Skins_ao\\Enemies_ao\\p_team_r.dtx",
	},
#endif

};

// Table of max health and armor values based on strength & magic attributes.
//								   1       2       3       4       5       6
static DFLOAT g_fMaxHealth[6]	 = {100.0f, 100.0f, 125.0f, 150.0f, 200.0f, 250.0f};
static DFLOAT g_fMaxMegaHealth[6]= {100.0f, 150.0f, 200.0f, 250.0f, 300.0f, 400.0f};
static DFLOAT g_fMaxArmor[6]	 = {100.0f, 100.0f, 125.0f, 150.0f, 200.0f, 250.0f};
static DFLOAT g_fMaxMegaArmor[6] = {100.0f, 150.0f, 200.0f, 250.0f, 300.0f, 400.0f};

static DFLOAT s_fCreatedTime = 0.0f;


static char s_szPowerupStartSound[] = "sounds\\powerups\\specialpowerupstart1.wav";
static char s_szPowerupEndSound[] = "sounds\\powerups\\specialpowerupend1.wav";
static char *s_PowerupLoopingSounds[] =
{
	"sounds\\powerups\\invulnerabilityloop1.wav",
	"sounds\\powerups\\revenantloop1b.wav",
	"sounds\\powerups\\tripledamageloop3.wav",
	"",
	""
};

// Values to use for acceleration & velocity
//DFLOAT gMoveAccel[6] = { 3300.0f, 3750.0f, 4200.0f, 4650.0f, 4950.0f, 5250.0f };
DFLOAT gDragCoeff[6] = { 15.0f, 15.0f, 15.0f, 15.0f, 15.0f, 15.0f };

// Max velocity is gMoveAccel/gDragCoeff
DFLOAT gMoveVel[6]	 = { 220.0f, 250.0f, 280.0f, 310.0f, 330.0f, 350.0f };


#define DIST_USE		82
#define DIST_GRAB		82

#define MAX_AIR_LEVEL				100.0f
#define FULL_AIR_LOSS_TIME			15.0f
#define FULL_AIR_REGEN_TIME			2.5f

#define MAX_IDLE_TIME				60.0f

// String constants
#define	STRING_EXPIRED				" has expired."
#define	STRING_INVISIBILITY			"Invisibility"
#define STRING_WILLPOWER			"Willpower"
#define STRING_TRIPLEDAMAGE			"Triple Damage"

char msgbuf[255];

BEGIN_CLASS(CPlayerObj)
END_CLASS_DEFAULT_FLAGS(CPlayerObj, CBaseCharacter, NULL, NULL, CF_HIDDEN)


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CPlayerObj()
//
//	PURPOSE:	constructor
//
// --------------------------------------------------------------------------- //
CPlayerObj::CPlayerObj() : CBaseCharacter(OT_MODEL)
{
	m_hClient				= DNULL;
	m_bFiringWeapon			= 0;
	m_startFall				= -1;

	m_Music.Init(this);

	// Initialize client data
	m_fPitch			= 0.0f;
	m_fYaw			= 0.0f;
	m_hstrPlayerName	= DNULL;
	VEC_INIT(m_vGunMuzzlePos);
	ROT_INIT(m_rRotation);
	VEC_INIT(m_vForward);
	m_byFlags			= CDATA_SHOWGUN;
	m_fMouseAxis0		= 0.0;
	m_fMouseAxis1		= 0.0;

	m_fEyeLevel				= 0.0f;

	m_bAnimating			= DFALSE;

	m_bDemoPlayback			= DFALSE;
	m_nCharacter			= CHARACTER_CALEB;
	m_nSkin					= MULTIPLAY_SKIN_NORMAL;
	m_nAttribStrength		= 1;
	m_nAttribSpeed			= 1;
	m_nAttribResistance		= 1;
	m_nAttribMagic			= 1;

	m_nBindingStrength		= 0;
	m_nBindingSpeed			= 0;
	m_nBindingResistance	= 0;
	m_nBindingMagic			= 0;
	m_bBindingConstitution	= DFALSE;
	m_bBindingBlending		= DFALSE;
	m_bBindingSoulStealing	= DFALSE;
	m_bServerRegeneration	= DFALSE;
	m_fServerRegenPeriod	= BINDING_REGEN_TIME;
	m_bBindingRegeneration	= DFALSE;
	m_fBindingRegenPeriod	= BINDING_REGEN_TIME;
	m_bBindingQuickness		= DFALSE;
	m_bBindingIncreasedDamage = DFALSE;
	m_bBindingImpactResistance = DFALSE;

	m_bPowerupActivated[PU_INVULNERABLE] = DFALSE;
	m_bPowerupActivated[PU_INVISIBLE] = DFALSE;
	m_bPowerupActivated[PU_TRIPLEDAMAGE] = DFALSE;
	m_bPowerupActivated[PU_INCORPOREAL] = DFALSE;
	m_bPowerupActivated[PU_SILENT] = DFALSE;

	m_ePowerupSoundStack[0] = PU_NONE;
	m_nPowerupSoundStackPos	= 0;

	m_hPowerupStartSound	= DNULL;
	m_hPowerupEndSound		= DNULL;
	m_hPowerupLoopSound		= DNULL;

	m_bDead					= DFALSE;
	m_bMovementBlocked		= DFALSE;
	m_bImprisoned			= DFALSE;
	m_hCameraObj			= DNULL;

	m_hstrTrigger			= DNULL;

	m_hCurSound				= DNULL;
	m_hWeapSound			= DNULL;

	VEC_INIT(m_vLastPos);
	VEC_INIT(m_vLastVel);

	VEC_SET(m_vScale, 1.0f, 1.0f, 1.0f);

	m_hEnemyAttach			= DNULL;

	g_pPlayerObj			= this;
	
	m_bActivated			= DFALSE;

	m_byMoveState			= PMOVE_NONE;
	m_nInjuredLeg			= 0;
	m_nNodeHit				= 0;
	m_nSideHit				= 0;

	m_hstrWhoKilledMeLast	= DNULL;
	m_hstrWhoIKilledLast	= DNULL;
	m_bInSlowDeath			= DFALSE;
	m_fSlowDeathSafeTimer   = 0;
	m_fSlowDeathStayTimer   = 0;

	// ADDED BY ANDY 9-20-98
	m_bSlowMode			= DFALSE;
	m_fSlowTime			= 0.0f;

	// For remote bomb list
	m_pBombList			= 0;

	m_hOrbObj			= DNULL;

	// Added by Loki
	m_fImprisonStart	= 0.0f;
	m_fImprisonLength	= 0.0f;

	m_fOldAirLevel			= MAX_AIR_LEVEL;
	m_fAirLevel				= MAX_AIR_LEVEL;

	m_bCanSwimJump			    = DFALSE;
	m_bSwimmingJump				= DFALSE;

	m_eLastSurface		= SURFTYPE_UNKNOWN;
	m_bNextFootstepLeft = DTRUE;
	m_bDisableFootsteps = DFALSE;

	m_fNextPingTime = 0.0f;
	
	m_hClientSaveData = DNULL;
	
	ResetIdleTime();

	memset(&m_CabalLink, 0, sizeof(m_CabalLink));
	memset(&m_MonsterLink, 0, sizeof(m_MonsterLink));

	if( AI_Mgr::m_dwNumCabal == 0 )
	{
		dl_TieOff( &AI_Mgr::m_CabalHead );
	}

	if( AI_Mgr::m_dwNumMonster == 0 )
	{
		dl_TieOff( &AI_Mgr::m_MonsterHead );
	}

	m_CurGun = WEAP_NONE;
	m_CurGunSlot = 0;

	m_bBurning	= DFALSE;
	m_fBurningTime = 0.0f;

	m_byAirborneCount = 0;

	m_nCurContainers = 0;
	m_PStateChangeFlags = PSTATE_ALL;
	m_byClientMoveCode = 0;

	m_dwTeamID    = 0;
	m_hFlagObject = DNULL;
	m_hFlagAttach = DNULL;

	m_fMoveVel = 0;
	m_fLadderVel = 0;
	m_fJumpVel = 0;
	m_fLeashLen = 0;
	m_fMoveMultiplier = 0;
	m_fBaseMoveAccel = 0;
	m_fJumpMultiplier = 0;

	m_bDoorPush = 0;
	VEC_INIT( m_vDoorPush );

	m_fTeleportTime = 0;

	m_bInputMode        = DFALSE;
	m_bExternalCamera   = DFALSE;

	s_fCreatedTime = g_pServerDE->GetTime();
	m_hPlrPosVar   = DNULL;

	dl_InitList( &m_AttachedObjectsList );
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::~CPlayerObj()
//
//	PURPOSE:	destructor
//
// --------------------------------------------------------------------------- //
CPlayerObj::~CPlayerObj()
{
	DLink *pCur, *pNext;

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrTrigger)
		pServerDE->FreeString(m_hstrTrigger);

	if (m_hstrPlayerName)
		pServerDE->FreeString(m_hstrPlayerName);

	if (m_hCurSound)
		pServerDE->KillSound(m_hCurSound);

	if (m_hWeapSound)
		pServerDE->KillSound(m_hWeapSound);

	if (m_hstrWhoKilledMeLast)
		pServerDE->FreeString(m_hstrWhoKilledMeLast);

	if (m_hstrWhoIKilledLast)
		pServerDE->FreeString(m_hstrWhoIKilledLast);

	if (m_hPowerupStartSound)
		pServerDE->KillSound(m_hPowerupStartSound);
	if (m_hPowerupEndSound)
		pServerDE->KillSound(m_hPowerupEndSound);
	if (m_hPowerupLoopSound)
		pServerDE->KillSound(m_hPowerupLoopSound);

	if( m_hClientSaveData )
	{
		pServerDE->EndHMessageRead( m_hClientSaveData );
	}

	if( m_CabalLink.m_pData && AI_Mgr::m_dwNumCabal > 0 )
	{
		dl_Remove( &m_CabalLink );
		AI_Mgr::m_dwNumCabal--;
	}

	if( m_MonsterLink.m_pData && AI_Mgr::m_dwNumMonster > 0 )
	{
		dl_Remove( &m_MonsterLink );
		AI_Mgr::m_dwNumMonster--;
	}

	// Delete the attached objects list
	pCur = m_AttachedObjectsList.m_Head.m_pNext;
	while( pCur != &m_AttachedObjectsList.m_Head )
	{
		pNext = pCur->m_pNext;
		delete pCur;
		pCur = pNext;
	}

	// Dump the bomb list.
	if( m_pBombList )
	{
		ObjectLink* pLink = m_pBombList->m_pFirstLink;
		while( pLink )
		{
			g_pServerDE->RemoveObject( pLink->m_hObject );
			pLink = pLink->m_pNext;
		}

		pServerDE->RelinquishList( m_pBombList );
		m_pBombList = DNULL;
	}

	m_hPlrPosVar = DNULL;
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PostPropRead()
//
//	PURPOSE:	Called after properties are set, do initialization stuff here.
//
// --------------------------------------------------------------------------- //
void CPlayerObj::PostPropRead(ObjectCreateStruct *pStruct)
{
	_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)"Models\\Characters\\Caleb.abc");
	_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)"Skins\\Characters\\Caleb1.dtx");
	_mbscpy((unsigned char*)pStruct->m_Name,		(const unsigned char*)"player");
	pStruct->m_ObjectType = OT_MODEL;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::InitialUpdate()
//
//	PURPOSE:	The first update, set various player object properties.
//
// --------------------------------------------------------------------------- //
DBOOL CPlayerObj::InitialUpdate(int nData)
{
	// NOTE: m_hClient is DNULL when we get here!!!

	// Dev code to help us find the "ghost player" bug...
	m_hPlrPosVar = g_pServerDE->GetGameConVar("playerpos");
	if (!m_hPlrPosVar)
	{
		g_pServerDE->SetGameConVar("playerpos", "0");
		m_hPlrPosVar = g_pServerDE->GetGameConVar("playerpos");
	}


	// Do the initial update...
	CServerDE* pServerDE = GetServerDE();

	pServerDE->SetNetFlags(m_hObject, NETFLAG_POSUNGUARANTEED|NETFLAG_ROTUNGUARANTEED|NETFLAG_ANIMUNGUARANTEED);

	m_InventoryMgr.Init(m_hObject, m_hClient);	// note: m_hClient is NULL, but it both get updated later
	m_damage.Init(m_hObject, &m_InventoryMgr, &m_Anim_Sound);

	// Set the team ID...
	UpdateTeamID();

	//for the internal AI enemy lists
	dl_Insert( &AI_Mgr::m_MonsterHead, &m_MonsterLink );
	m_MonsterLink.m_pData = ( void * )this;
	AI_Mgr::m_dwNumMonster++;

	dl_Insert( &AI_Mgr::m_CabalHead, &m_CabalLink );
	m_CabalLink.m_pData = ( void * )this;
	AI_Mgr::m_dwNumCabal++;

	m_bFirstUpdate = DTRUE;

	m_fMoveVel = 0;
	m_fLadderVel = 0;
	m_fJumpVel = 0;
	m_fLeashLen = 0;
	m_fMoveMultiplier = 0;
	m_fBaseMoveAccel = 0;
	m_fJumpMultiplier = 0;

	m_bDoorPush = DFALSE;
	VEC_INIT( m_vDoorPush );

	m_fTeleportTime = 0;
	
	m_LeashLenTrack.Init(pServerDE, "LeashLen", DNULL, DEFAULT_LEASHLEN);

	// Only init stuff needed when restoring a savegame.
	if (nData == INITIALUPDATE_SAVEGAME)
	{
		return DTRUE;
	}

	// Give max ammo for now
/*	for (int i=AMMO_BULLET; i <= AMMO_MAXAMMOTYPES; i++)
	{
		m_InventoryMgr.AddAmmo(i, (DFLOAT)1000);
	}*/
	m_InventoryMgr.AddAmmo(AMMO_BULLET, 50);

// Change weapon
	m_bSpectatorMode	= DFALSE;
	SendMessageToClient(SMSG_SPECTATORMODE);
	m_bZoomView = DFALSE;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT).001);
	m_bLastJumpCommand = DFALSE;
	m_bLastCrouchCommand = DFALSE;
	m_bForcedCrouch	= DFALSE;
	m_bFalling         = DFALSE;
	m_bServerFalling   = DFALSE;

	m_damage.SetHitPoints(100.0f);
	m_damage.SetArmorPoints(0.0f);
	m_damage.SetMaxHitPoints(100.0f);
	m_damage.SetMaxMegaHitPoints(100.0f);
	m_damage.SetMaxArmorPoints(100.0f);
	m_damage.SetMaxNecroArmorPoints(100.0f);
	m_nLastHitPoints		= 0;
	m_nLastDamage			= 0;
	m_nLastAmmo				= 0;
	m_nLastArmor			= 0;
	m_nLastFocus			= 0;
	m_damage.SetMass(PA_DEFAULT_MASS);
	pServerDE->SetBlockingPriority(m_hObject, BLOCKPRIORITY_PLAYER);

	// Override the friction
	pServerDE->SetFrictionCoefficient(m_hObject, 0.0f);
//	pServerDE->SetFrictionCoefficient(m_hObject, 15.0f);

    // Set the Smell Time
    m_fSmellTime = pServerDE->GetTime();
    m_fSmellDelay = 0.5f;              // Delay between smell drops

	DVector vPos;
    pServerDE->GetObjectPos(m_hObject, &vPos);

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	DRotation rRot;
    ROT_INIT(rRot);

    ROT_COPY(theStruct.m_Rotation, rRot);
	VEC_COPY(theStruct.m_Pos, vPos);

	pServerDE->CreateObject(pServerDE->GetClass("SmellHint"), &theStruct);

	// Send the initial health, armor and ammo values to the client
	// [blg] Andy says we don't need this...m_hClient is NULL at this point anyway
	// [blg] SendMessageToClient(SMSG_HEALTH);

//	pServerDE->SetForceIgnoreLimit(m_hObject, 250.0f);

	m_Frags = 0;
	m_MeleeKills = 0;
	m_CurGun = WEAP_NONE;
	m_CurGunSlot = 0;
	m_bMovementBlocked = DFALSE;
	// Wait until we get PLAYERATTRIBUTES from client for first respawn.

	// Set initial dims
	DVector dims;
	VEC_SET(dims, 12, gCharacterValues[m_nCharacter].fHeightStanding/2, 12)
	pServerDE->SetObjectDims(m_hObject, &dims);

	m_hstrTrigger = pServerDE->CreateString("TRIGGER");

	// All done...
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateTeamID
//
//	PURPOSE:	Updates the team ID value that this player is on
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateTeamID()
{
	CTeam* pTeam = g_pBloodServerShell->GetTeamMgr()->GetTeamFromPlayerID(g_pServerDE->GetClientID(GetClient()));
	if (pTeam)
	{
		m_dwTeamID = pTeam->GetID();
	}
}

			
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateControlFlags
//
//	PURPOSE:	Set the movement/firing flags
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateControlFlags()
{
	// Clear control flags...

	m_dwControlFlags = 0; 

	if(m_bDead) return;

	// Make sure it's ok for us to move...

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hClient || m_bDemoPlayback || m_bMovementBlocked) return;
	if (m_bInSlowDeath) return;	// [blg] 01/13/99


	// Determine what commands are currently on...

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_RUN) || m_byFlags & CDATA_RUNLOCK)
	{
		m_dwControlFlags |= CTRLFLAG_RUN;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_JUMP))
	{
		m_dwControlFlags |= CTRLFLAG_JUMP;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_DUCK))
	{
		m_dwControlFlags |= CTRLFLAG_CROUCH;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_FORWARD))
	{
		m_dwControlFlags |= CTRLFLAG_FORWARD;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_BACKWARD))
	{
		m_dwControlFlags |= CTRLFLAG_BACKWARD;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_LEFT))
	{
		m_dwControlFlags |= CTRLFLAG_LEFT;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_RIGHT))
	{
		m_dwControlFlags |= CTRLFLAG_RIGHT;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_STRAFE))
	{
		m_dwControlFlags |= CTRLFLAG_STRAFE;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_STRAFERIGHT))
	{
		m_dwControlFlags |= CTRLFLAG_STRAFERIGHT;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_STRAFELEFT))
	{
		m_dwControlFlags |= CTRLFLAG_STRAFELEFT;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_FIRE))
	{
		m_dwControlFlags |= CTRLFLAG_FIRE;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_ALTFIRE))
	{
		m_dwControlFlags |= CTRLFLAG_ALTFIRE;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_GRAB))
	{
		m_dwControlFlags |= CTRLFLAG_GRAB;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_TAUNT))
	{
		m_dwControlFlags |= CTRLFLAG_TAUNT;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleMovement()
//
//	PURPOSE:	Checks for movement commands and updates acceleration, etc.
//
// --------------------------------------------------------------------------- //
DBOOL CPlayerObj::HandleMovement()
{
	CServerDE* pServerDE = GetServerDE();

	DVector		vMyVel;
	DVector		vAccel;
	DVector		vUp;
	DVector		vRight;
	DVector		vForward;
	DVector		vMyPos;
	DFLOAT		fMaxMoveAccel;
	DFLOAT		fMaxMoveVel;
	DFLOAT		fMoveAccel = 0.0;
	DFLOAT		fJumpSpeed = DEFAULT_JUMPVEL;
	DFLOAT		fStrafeAccel = 0;
	DBOOL		bNothingHappened = DTRUE;

	DBOOL		bHeadInLiquid = IsLiquid(m_eContainerCode);
	DBOOL		bInLiquid	  = bHeadInLiquid || m_bBodyInLiquid;
	DBOOL		bFreeMovement = bHeadInLiquid || m_bBodyOnLadder || m_bSpectatorMode;
	DBOOL		bLateralMovement = DFALSE;

	int nIndex = (m_nAttribSpeed + m_nBindingSpeed - 1);
	nIndex = DCLAMP(nIndex, 0, 6);
	fMaxMoveVel = gMoveVel[nIndex];
	fMaxMoveAccel = fMaxMoveVel * gDragCoeff[nIndex];

	// Zero acceleration to start with
	VEC_INIT(vAccel);

	// Get my position
	pServerDE->GetObjectPos(m_hObject, &vMyPos);

	// See if standing on anything
//	DBOOL bIsStandingOn = DFALSE;
//	m_eLastSurface = SURFTYPE_UNKNOWN;
	CollisionInfo collisionInfo;

	pServerDE->GetStandingOn(m_hObject, &collisionInfo);
//	DBOOL bIsStandingOn = (collisionInfo.m_hObject != DNULL);

	// Don't want to be moved if standing on a base character
	if(m_bOnGround)
	{
		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);

		if(IsBaseCharacter(collisionInfo.m_hObject))
			dwFlags |= FLAG_DONTFOLLOWSTANDING;		
		else
			dwFlags &= ~FLAG_DONTFOLLOWSTANDING;		

		pServerDE->SetObjectFlags(m_hObject, dwFlags);

		// Get the last surface type (for footsteps, etc)
//		m_eLastSurface = ::GetSurfaceType(collisionInfo.m_hObject, collisionInfo.m_hPoly);

		// Damage object we fell on...
		if (m_bFalling)
		{
			DVector vTmp;
			VEC_INIT(vTmp);
			DamageObject(m_hObject, this, collisionInfo.m_hObject, 1.0f, vTmp, vTmp, DAMAGE_TYPE_NORMAL);
		}

		// Set higher blocking priority when on the ground so  we can push stuff
		pServerDE->SetBlockingPriority(m_hObject, BLOCKPRIORITY_PLAYER);
	}
	else
		pServerDE->SetBlockingPriority(m_hObject, BLOCKPRIORITY_PLAYER_AIRBORNE);


	// Get current velocity
	pServerDE->GetVelocity(m_hObject, &vMyVel);

	DFLOAT fMyVelMag = VEC_MAG(vMyVel);

	// Disable footsteps if moving too slowly
	m_bDisableFootsteps = (fMyVelMag < 50) ? DTRUE : DFALSE;


	// BEGIN 10/6/98 Kevin additions ////////////////////////////////////

	m_fSwimVel	 = (m_dwControlFlags & CTRLFLAG_RUN) ? DEFAULT_SWIM_VEL : DEFAULT_SWIM_VEL/2.0f;
	m_fLadderVel = (m_dwControlFlags & CTRLFLAG_RUN) ? DEFAULT_LADDER_VEL : DEFAULT_LADDER_VEL/2.0f;

	if (!m_bBodyInLiquid) m_bSwimmingJump = DFALSE;

	// Limit velocity when in certain containers...
	if (m_bBodyOnLadder)
	{
		if (fMyVelMag > m_fLadderVel)
		{
			VEC_NORM(vMyVel);
			VEC_MULSCALAR(vMyVel, vMyVel, m_fLadderVel);
		}
	}
	else if (m_bBodyInLiquid)
	{
		if (fMyVelMag > m_fSwimVel)
		{
			VEC_NORM(vMyVel);
			VEC_MULSCALAR(vMyVel, vMyVel, m_fSwimVel);
			if (m_bSwimmingJump)	// Keep setting jump speed until I jump out
				vMyVel.y = fJumpSpeed * 0.75f;
		}
	}
	// END 10/6/98 Kevin additions ////////////////////////////////////


	// See if running
	if (!m_damage.IsDead())
	{
		// get the object's vectors
		pServerDE->GetRotationVectors(&m_rRotation, &vUp, &vRight, &vForward);

		if (m_bSpectatorMode)
		{
			DVector vel;
			VEC_INIT(vel);
//			pServerDE->SetVelocity(m_hObject, &vel);
			fMaxMoveAccel = 1.0f;
			fMaxMoveVel = 500.0f;
		}
		else if (!bFreeMovement && !bInLiquid)
		{
			// Only want x and z components for movement
			vRight.y = 0;
			VEC_NORM(vRight)

			vForward.y = 0;
			VEC_NORM(vForward)
		}

		if (m_dwControlFlags & CTRLFLAG_RUN)
		{
			fMaxMoveAccel *= 1.5;
			fMaxMoveVel *= 1.5;
		}

		if (((m_dwControlFlags & CTRLFLAG_CROUCH) || m_bForcedCrouch) && !bFreeMovement)
		{
			fMaxMoveAccel /= 2;
			fMaxMoveVel /= 2;
			bNothingHappened = DFALSE;
		}

		// Set acceleration based on player controls

		// Move forward/backwards
		if ((m_dwControlFlags & CTRLFLAG_FORWARD) && (!m_bMovementBlocked || m_eContainerCode == CC_CRANECONTROL))
		{
			fMoveAccel += fMaxMoveAccel;
			bNothingHappened = DFALSE;
			bLateralMovement = DTRUE;
		}
		if ((m_dwControlFlags & CTRLFLAG_BACKWARD) && (!m_bMovementBlocked || m_eContainerCode == CC_CRANECONTROL))
		{
			fMoveAccel += -fMaxMoveAccel;
			bNothingHappened = DFALSE;
			bLateralMovement = DTRUE;
		}

		if (!(m_byFlags & CDATA_MOUSEAIMING) && !m_bMovementBlocked)
		{
			fMoveAccel += -(m_fMouseAxis1) * fMaxMoveAccel;
			m_dwControlFlags |= CTRLFLAG_FORWARD;
			bLateralMovement = DTRUE;
		}

		if (fMoveAccel > fMaxMoveAccel) 
			fMoveAccel = fMaxMoveAccel;
		else if (fMoveAccel < -fMaxMoveAccel) 
			fMoveAccel = -fMaxMoveAccel;

		VEC_MULSCALAR(vForward, vForward, fMoveAccel)
		VEC_ADD(vAccel, vAccel, vForward)

		// Strafe.. Check for strafe modifier first
		if (m_dwControlFlags & CTRLFLAG_STRAFE)
		{
			if (m_dwControlFlags & CTRLFLAG_LEFT)
				fStrafeAccel += -fMaxMoveAccel;
			if (m_dwControlFlags & CTRLFLAG_RIGHT)
				fStrafeAccel += fMaxMoveAccel;

			fStrafeAccel += (m_fMouseAxis0) * fMaxMoveAccel;

			bNothingHappened = DFALSE;
			bLateralMovement = DTRUE;
		}

		// Check individual strafe commands
		if (m_dwControlFlags & CTRLFLAG_STRAFELEFT)
		{
			fStrafeAccel += -fMaxMoveAccel;
			bNothingHappened = DFALSE;
			bLateralMovement = DTRUE;
		}
		if (m_dwControlFlags & CTRLFLAG_STRAFERIGHT)
		{
			fStrafeAccel += fMaxMoveAccel;
			bNothingHappened = DFALSE;
			bLateralMovement = DTRUE;
		}

		if (fStrafeAccel > fMaxMoveAccel) 
			fStrafeAccel = fMaxMoveAccel;
		else if (fStrafeAccel < -fMaxMoveAccel) 
			fStrafeAccel = -fMaxMoveAccel;

		VEC_MULSCALAR(vRight, vRight, fStrafeAccel)
		VEC_ADD(vAccel, vAccel, vRight)

		// tone down run-strafing, but not too much.
		DFLOAT fAccelMag = VEC_MAG(vAccel);
		DFLOAT fMaxRunStrafeAccel = (DFLOAT)sqrt((fMaxMoveAccel * fMaxMoveAccel) * 2);
		if (fAccelMag > fMaxRunStrafeAccel)
		{
			VEC_MULSCALAR(vAccel, vAccel, (fMaxRunStrafeAccel/fAccelMag));
		}

		DBOOL  bSteep = DFALSE;
		if (!m_bSpectatorMode && !m_bMovementBlocked)
		{
			// Make sure the slope isn't too steep to climb or jump up (prevent jump-skip)

			if (m_bOnGround && !bFreeMovement)
			{
				// If standing on something, tilt the acceleration so it's aligned with
				// the surface.
				DVector &v = collisionInfo.m_Plane.m_Normal;
				if ((v.z + v.x) > v.y)			// Check for slope > 45 degrees
				{
					vAccel.y -= 4000.0f;		// Just to make sure I slide down
					bSteep = DTRUE;
				}
				else
				{
					TiltVectorToPlane(&vAccel, &collisionInfo.m_Plane.m_Normal);
				}
			}

			// See if we just broke the surface of water...
			if ((IsLiquid(m_eLastContainerCode) && !bHeadInLiquid) && !m_bOnGround && !m_bBodyOnLadder)
			{
				m_bSwimmingOnSurface = DTRUE;
			}
			else if (bHeadInLiquid)  // See if we went back under...
			{
				m_bSwimmingOnSurface = DFALSE;
				m_bCanSwimJump		 = DTRUE;
			}	

			// See if we landed (after falling)...
			if (!bFreeMovement && m_bFalling && m_bOnGround)
			{
				// See if we should play a landing sound
				if ((m_fMaxFallPos - m_fMinFallPos) > 56.0f && !m_bPowerupActivated[PU_INCORPOREAL])
				{
					char	szSound[MAX_CS_FILENAME_LEN+1];
					if (GetLandingSound(szSound))
					{
						PlayPlayerSound(szSound, 200.0f, 70, DFALSE, SOUNDPRIORITYMOD_LOW);
					}
				}
            
				m_startFall = -1.0;
			}
			else if (bFreeMovement)
			{
				m_startFall = -1.0;
			}

			// now update m_bServerFalling
			CollisionInfo ci;
			g_pServerDE->GetStandingOn(m_hObject, &ci);
			m_bServerFalling = !ci.m_hObject;

			// now update m_bFalling
			m_bFalling = m_bOnGround ? DFALSE : DTRUE;

			if (m_bFalling)
			{
				if (m_startFall < 0)
				{
					m_startFall = pServerDE->GetTime();
					m_fMinFallPos = m_fMaxFallPos = vMyPos.y;
				}
				else 
				{
					if (vMyPos.y > m_fMaxFallPos)
						m_fMaxFallPos = vMyPos.y;
					if (vMyPos.y < m_fMinFallPos)
						m_fMinFallPos = vMyPos.y;
				}
			}

			// Jumping
			if (m_dwControlFlags & CTRLFLAG_JUMP)
			{
				// If we are in a container that supports free movement, see if we are 
				// moving up or down...
				if (bFreeMovement)
				{
					vAccel.y += fMaxMoveAccel;

//					m_bLastJumpCommand = DFALSE;
					bNothingHappened = DFALSE;
				}
				else if (!m_bLastJumpCommand)
				{
					// BEGIN 10/5/98 Kevin additions ////////////////////////////////////

					// Handling jumping out of water...
					if (m_bBodyInLiquid && !bHeadInLiquid)
					{
						if (m_bCanSwimJump)
						{
							m_bSwimmingJump = DTRUE;
							m_bCanSwimJump  = DFALSE;
						}
						// If our head is out of the liquid and we're standing on the
						// ground, let us jump out of the water...
						else if (m_bOnGround)
						{
							m_bSwimmingJump = DTRUE;
						}

						if (m_bSwimmingJump)
						{
							m_bSwimmingOnSurface = DFALSE;
							bNothingHappened = DFALSE;
							vMyVel.y += fJumpSpeed * 0.75f;
							m_bLastJumpCommand = DTRUE;
							m_bOnGround = DFALSE;
							bFreeMovement = DFALSE;
						}
					}
					else if (m_bOnGround && !bSteep)
					{
						if (m_bForcedCrouch)
							vMyVel.y += fJumpSpeed * 0.75f;
						else
							vMyVel.y += fJumpSpeed;

						m_bLastJumpCommand = DTRUE;
						bNothingHappened = DFALSE;
//						bIsStandingOn = DFALSE;
					}
				}
			}
			else
				m_bLastJumpCommand = DFALSE;


			// Crouching
			if (m_dwControlFlags & CTRLFLAG_CROUCH)
			{
				if ((bInLiquid || bFreeMovement) && !m_bOnGround)
				{
					vAccel.y -= fMaxMoveAccel;
				}
				else
				{
					m_bLastCrouchCommand = DTRUE;
					if (!m_bForcedCrouch)
						m_PStateChangeFlags |= PSTATE_CROUCH;
					m_bForcedCrouch = DTRUE;
				}
				bNothingHappened = DFALSE;
			}
			else	// Stand up again
			{
				m_bLastCrouchCommand = DFALSE;
			}
		}
	}

	// Calculate drag for in-air movement, we want air movement consistent
	// with ground movement. gDragCoeff needs to be the same as the object's 
	// friction coefficient.
	
	DVector vAirVel;
	
	VEC_COPY(vAirVel, vMyVel);
	// Apply drag to y when going down, but not going up.
	if (vAirVel.y >= 0.0f) 
		vAirVel.y = 0.0f;	
	else
	{
		vAirVel.y = DCLAMP(vAirVel.y/2.0f, -25.0f, 0.0f);
	}

	DFLOAT fVel = VEC_MAGSQR(vAirVel);

	if (!bFreeMovement && !m_bOnGround && fVel > 0.01f)
	{
		DFLOAT fDragAccel = gDragCoeff[nIndex];

		VEC_MULSCALAR(vAirVel, vAirVel, -fDragAccel);

		VEC_ADD(vAccel, vAccel, vAirVel);
	}

	if (!m_bSpectatorMode)
	{
		// ADDED BY ANDY 9-20-98
		if(m_bSlowMode)
		{
			VEC_MULSCALAR(vMyVel, vMyVel, 0.5f);

			if(pServerDE->GetTime() > m_fSlowTime)
				m_bSlowMode = DFALSE;
		}

		// Cap horizontal velcity at max to make sure.
		DFLOAT fYVel = vMyVel.y;
		vMyVel.y = 0.0f;

		DFLOAT fMag = VEC_MAG(vMyVel);
		fMaxMoveVel = (DFLOAT)sqrt(fMaxMoveVel * fMaxMoveVel * 2);
		if (fMag > fMaxMoveVel)
		{
			fMag = fMaxMoveVel / fMag;
			VEC_MULSCALAR(vMyVel, vMyVel, fMag);
		}
		vMyVel.y = fYVel;
//		pServerDE->SetVelocity(m_hObject, &vMyVel);
	}
	else	// m_bSpectatorMode
	{
		if (bLateralMovement)
		{
			VEC_NORM(vAccel);
			VEC_MULSCALAR(vAccel, vAccel, fMaxMoveVel);
		}
		else
		{
			VEC_INIT(vAccel);
		}
//		pServerDE->SetVelocity(m_hObject, &vAccel);
	}

	if (m_bLastJumpCommand && vMyVel.y > 0 && vAccel.y > 0)
		vAccel.y = 0;

//	pServerDE->SetAcceleration(m_hObject, &vAccel);
//
//	pServerDE->GetVelocity(m_hObject, &m_vLastVel);

	// Reset these value
	m_bBodyInLiquid				= DFALSE;
	m_bBodyOnLadder				= DFALSE;


//	pServerDE->BPrint("Vel: %.2f, %.2f, %.2f", vMyVel.x, vMyVel.y, vMyVel.z);
//	pServerDE->BPrint("Accel: %.2f, %.2f, %.2f", vAccel.x, vAccel.y, vAccel.z);

	return bNothingHappened;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ResetPlayerInventory()
//
//	PURPOSE:	
//
// --------------------------------------------------------------------------- //

void CPlayerObj::ResetPlayerInventory()
{
	m_InventoryMgr.Term();
	m_InventoryMgr.ObtainWeapon(WEAP_MELEE, 0);
	m_InventoryMgr.ObtainWeapon(WEAP_BERETTA, 0);

	for (int i = AMMO_BULLET; i <= AMMO_MAXAMMOTYPES; i++)
		m_InventoryMgr.SetAmmoCount(i, 0.0f);

	m_InventoryMgr.SetAmmoCount(AMMO_BULLET, DEFAULT_BULLETS);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Slow()
//
//	PURPOSE:	Checks for firing commands and updates the weapon object
//
// --------------------------------------------------------------------------- //
void CPlayerObj::Slow(DFLOAT time)
{
	CServerDE* pServerDE = GetServerDE();
	if(!pServerDE)	return;

	if(time > 0.0f)
	{
		m_bSlowMode = DTRUE;
		m_fSlowTime = pServerDE->GetTime() + time;
	}
	else
		m_bSlowMode = DFALSE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AddRemoteBomb()
//
//	PURPOSE:	Add a remote bomb to the list
//
// --------------------------------------------------------------------------- //
void CPlayerObj::AddRemoteBomb(CProjectile* pBomb)
{
	CServerDE* pServerDE = GetServerDE();
	if(!pServerDE)	return;

	if (!m_pBombList)
		m_pBombList = pServerDE->CreateObjectList();

	HOBJECT hBombObject = pServerDE->ObjectToHandle((LPBASECLASS)pBomb);
	if( hBombObject )
	{
		ObjectLink* ol = pServerDE->AddObjectToList(m_pBombList, hBombObject);
		ol->m_hObject = hBombObject;
		g_pServerDE->CreateInterObjectLink( m_hObject, hBombObject );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AddRemoteBomb()
//
//	PURPOSE:	Add a remote bomb to the list
//
// --------------------------------------------------------------------------- //
void CPlayerObj::RemoveRemoteBomb(CProjectile* pBomb)
{
	CServerDE* pServerDE = GetServerDE();
	if(!pServerDE)	return;

	if(m_pBombList)
	{
		HOBJECT hBombObject = pServerDE->ObjectToHandle((LPBASECLASS)pBomb);
		if( hBombObject )
		{
			pServerDE->RemoveObjectFromList(m_pBombList, hBombObject);
			g_pServerDE->BreakInterObjectLink( m_hObject, hBombObject );
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetOrbObject()
//
//	PURPOSE:	
//
// --------------------------------------------------------------------------- //
void CPlayerObj::SetOrbObject(HOBJECT hObj)
{
	CServerDE* pServerDE = GetServerDE();
	if(!pServerDE)	return;

	m_hOrbObj = hObj;

	if(hObj)
		pServerDE->CreateInterObjectLink(m_hObject, m_hOrbObj);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleFiring()
//
//	PURPOSE:	Checks for firing commands and updates the weapon object
//
// --------------------------------------------------------------------------- //
DBOOL CPlayerObj::HandleFiring()
{
	DBOOL	bNothingHappened = DTRUE;
	CServerDE* pServerDE = GetServerDE();
/*
	DRotation rotGun;
	DVector vRGunPos, vLGunPos, vMyPos;

	pServerDE->GetObjectPos(m_hObject, &vMyPos);
	VEC_ADD(vRGunPos, vMyPos, m_vGunMuzzlePos);
	VEC_ADD(vLGunPos, vMyPos, m_vlGunMuzzlePos);

	ROT_COPY(rotGun, m_rRotation);

	if (!m_damage.IsDead() && !m_bSpectatorMode)
	{
		DBOOL bFiring = DFALSE;
		DBOOL bAltFiring = DFALSE;

		if (!m_bMovementBlocked)
		{
			bFiring = (m_dwControlFlags & CTRLFLAG_FIRE) != 0;
			bAltFiring = (!bFiring && (m_dwControlFlags & CTRLFLAG_ALTFIRE));
		}

		if (bFiring || bAltFiring)
		{
			// Binoculars prevents firing.
			if (IsItemActive(INV_BINOCULARS))
			{
				bFiring = bAltFiring = DFALSE;
			}

			// Firing kills the incorporeal powerup
			if (m_bPowerupActivated[PU_INCORPOREAL])
				m_fIncorporealTime = 0.0f;
			bNothingHappened = DFALSE;
		}

		m_InventoryMgr.UpdateCurrentWeaponFiring(&vRGunPos, &vLGunPos, &rotGun, bFiring, bAltFiring);
	}
	else if (m_damage.IsDead())
	{
		m_InventoryMgr.UpdateCurrentWeaponFiring(&vRGunPos, &vLGunPos, &rotGun, DFALSE, DFALSE);
	}
*/
	return bNothingHappened;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateAnim()
//
//	PURPOSE:	Updates the player animation based on the current movement.
//
// --------------------------------------------------------------------------- //
void CPlayerObj::UpdateAnim()
{
	// Update the PMOVE state.. 
	if (!m_bDead)
	{
		if (m_bInSlowDeath)
		{
			DoHumiliationAnim();
		}
		else if ((m_dwControlFlags & CTRLFLAG_FIRE) || (m_dwControlFlags & CTRLFLAG_ALTFIRE))
		{
			if (m_dwControlFlags & CTRLFLAG_JUMP && !m_bForcedCrouch)
			{
				DoFireJumpAnim();
			}
			else if (m_dwControlFlags & CTRLFLAG_FORWARD || m_dwControlFlags & CTRLFLAG_BACKWARD ||
				m_dwControlFlags & CTRLFLAG_LEFT || m_dwControlFlags & CTRLFLAG_RIGHT || 
				m_dwControlFlags & CTRLFLAG_STRAFELEFT || m_dwControlFlags & CTRLFLAG_STRAFERIGHT)
			{
				if (m_dwControlFlags & CTRLFLAG_CROUCH)
					DoFireCrawlAnim();
				else if (m_dwControlFlags & CTRLFLAG_STRAFELEFT)
					DoStrafeLeftAnim();
				else if (m_dwControlFlags & CTRLFLAG_STRAFERIGHT)
					DoStrafeRightAnim();
				else if (m_dwControlFlags & CTRLFLAG_RUN)
					DoFireRunAnim();
				else 
					DoFireWalkAnim();
			}
			else if (m_dwControlFlags & CTRLFLAG_CROUCH)
			{
				DoFireCrouchAnim();
			}
			else
			{
				DoFireStandAnim();
			}
		}
		else
		{
			if (m_dwControlFlags & CTRLFLAG_JUMP && !m_bForcedCrouch)
			{
				DoJumpAnim();
			}
			else if (m_dwControlFlags & CTRLFLAG_FORWARD || m_dwControlFlags & CTRLFLAG_BACKWARD ||
				m_dwControlFlags & CTRLFLAG_LEFT || m_dwControlFlags & CTRLFLAG_RIGHT || 
				m_dwControlFlags & CTRLFLAG_STRAFELEFT || m_dwControlFlags & CTRLFLAG_STRAFERIGHT)
			{
				if (m_dwControlFlags & CTRLFLAG_CROUCH)
					DoCrawlAnim();
				else if (m_dwControlFlags & CTRLFLAG_STRAFELEFT)
					DoStrafeLeftAnim();
				else if (m_dwControlFlags & CTRLFLAG_STRAFERIGHT)
					DoStrafeRightAnim();
				else if (m_dwControlFlags & CTRLFLAG_RUN)
					DoRunAnim();
				else 
					DoWalkAnim();
			}
			else if (m_dwControlFlags & CTRLFLAG_CROUCH)
			{
				DoCrouchAnim();
			}
			else 
			{
				IdleTime();
			}
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Update()
//
//	PURPOSE:	Handles the update for the player object.
//
// --------------------------------------------------------------------------- //
DBOOL CPlayerObj::Update(DVector *pMovement)
{
	CServerDE* pServerDE = GetServerDE();
	DBOOL	bNothingHappened = DTRUE;


	// Dev code to help us find the "ghost player" bug...
	if (m_hPlrPosVar)
	{
		if( pServerDE->GetVarValueFloat( m_hPlrPosVar ) > 0.0f )
			pServerDE->SetNetFlags( m_hObject, 0 );
		else
			pServerDE->SetNetFlags(m_hObject, NETFLAG_POSUNGUARANTEED|NETFLAG_ROTUNGUARANTEED|NETFLAG_ANIMUNGUARANTEED);
	}

	// Set our next update
	pServerDE->SetNextUpdate(m_hObject, (DFLOAT).001);

	// Sanity check
	if (!m_hClient)
		return DTRUE;

	// Update timers
	m_fSlowDeathSafeTimer -= g_pServerDE->GetFrameTime();
	if (m_fSlowDeathSafeTimer < 0) m_fSlowDeathSafeTimer = 0;

	m_fSlowDeathStayTimer -= g_pServerDE->GetFrameTime();
	if (m_fSlowDeathStayTimer < 0) m_fSlowDeathStayTimer = 0;


	// Check for first update
	if (m_bFirstUpdate)
	{
		SetMoveVel(gMoveVel[m_nAttribSpeed-1]);
		SetJumpVel(DEFAULT_JUMPVEL);
	
		m_PStateChangeFlags = PSTATE_ALL;
		UpdateClientPhysics(); // (to make sure they have their model around)
		TeleportClientToServerPos();

		// Initial update for some stats..
		m_nStatsFlags = PLAYERSTATS_HEALTH | PLAYERSTATS_AMMO | PLAYERSTATS_ALTAMMO | PLAYERSTATS_ARMOR | PLAYERSTATS_FOCUS;
		SendMessageToClient( SMSG_PLAYERSTATSUPDATE );
		
		m_bFirstUpdate = DFALSE;
	}

	// Check for sound completion
	if (m_hCurSound)
	{
		DBOOL bSoundDone = DFALSE;
		if (pServerDE->IsSoundDone(m_hCurSound, &bSoundDone) == LT_OK && bSoundDone)
		{
			g_pServerDE->KillSound( m_hCurSound );
			m_hCurSound = DNULL;
		}
	}
/*
	 // Check to see if the status bar display items have changed
	DDWORD nItem;

	// See if the health has changed, send to client
	nItem = (DDWORD)m_damage.GetHitPoints();
	if (m_nLastHitPoints != nItem)
	{
		// If we took damage, tell the client
		if (m_nLastHitPoints > nItem)
		{
			m_nLastDamage = m_nLastHitPoints - nItem;
			SendMessageToClient(SMSG_TOOKDAMAGE);
		}

		m_nLastHitPoints = nItem;
		SendMessageToClient(SMSG_HEALTH);
	}

	// See if the ammo has changed, send to client
	nItem = (DDWORD)m_InventoryMgr.GetCurrentWeaponAmmoCount();
	if (m_nLastAmmo != nItem)
	{
		m_nLastAmmo = nItem;
		SendMessageToClient(SMSG_AMMO);
	}

	// See if the armor has changed, send to client
	nItem = (DDWORD)m_damage.GetArmorPoints();
	if (m_nLastArmor != nItem)
	{
		m_nLastArmor = nItem;
		SendMessageToClient(SMSG_ARMOR);
	}

	// See if the focus has changed, send to client
	nItem = (DDWORD)m_InventoryMgr.GetAmmoCount(AMMO_FOCUS);
	if (m_nLastFocus != nItem)
	{
		m_nLastFocus = nItem;
		SendMessageToClient(SMSG_FOCUS);
	}
*/

	// Keep the client updated.
	UpdateClientPhysics();

	// Update air level...
	UpdateAirLevel();

	// Update the player stats to the client...
	UpdateStats( );

	// Update any powerup timers..
	CheckPowerups();

	// Update based on active inventory items or spells
	CheckItemsAndSpells();
	UpdateControlFlags();

	// Check for player movement
	if (!HandleMovement())
		bNothingHappened = DFALSE;

//	if (!HandleFiring())
//		bNothingHappened = DFALSE;

	if (m_dwControlFlags & CTRLFLAG_GRAB)
		GrabObject();

	if (m_dwControlFlags & CTRLFLAG_TAUNT)
		DoTaunt();

	UpdateAnim();

// 11/10/97 added auto smell maker...
	if (!m_damage.IsDead() && !m_bSpectatorMode && m_hObject)
	{
		if ((pServerDE->GetTime() > m_fSmellTime + m_fSmellDelay) )
		{
	    	DVector vPos, vHintPos;
    		pServerDE->GetObjectPos(m_hObject, &vPos);

			if(SmellHint::m_SmellHead.m_pPrev && SmellHint::m_SmellHead.m_pPrev->m_pData)
			{
				SmellHint* pPrevHint = (SmellHint*)SmellHint::m_SmellHead.m_pPrev->m_pData;
				pServerDE->GetObjectPos(pPrevHint->m_hObject, &vHintPos);
			
				if(VEC_DIST(vHintPos, vPos) >= 10.0f)
				{
					ObjectCreateStruct theStruct;
					INIT_OBJECTCREATESTRUCT(theStruct);
    
					DRotation rRot;
    				ROT_INIT(rRot);

    				ROT_COPY(theStruct.m_Rotation, rRot);
	    			VEC_COPY(theStruct.m_Pos, vPos);

			        m_fSmellTime = pServerDE->GetTime();

					pServerDE->CreateObject(pServerDE->GetClass("SmellHint"), &theStruct);
				}
			}
			else
			{
				ObjectCreateStruct theStruct;
				INIT_OBJECTCREATESTRUCT(theStruct);
        
				DRotation rRot;
    			ROT_INIT(rRot);

    			ROT_COPY(theStruct.m_Rotation, rRot);
	    		VEC_COPY(theStruct.m_Pos, vPos);
	
				m_fSmellTime = pServerDE->GetTime();

				pServerDE->CreateObject(pServerDE->GetClass("SmellHint"), &theStruct);
			}
		}

	}
	else if (m_damage.IsDead())  // I'm dead according to m_damage.
	{
		if (!m_bDead)	// Just died handled here.
			HandleDeath();

		bNothingHappened = DFALSE;
	}

	// Get current gun ID for the client
	CWeapon *w = m_InventoryMgr.GetCurrentWeapon();
	DBOOL bChange = DFALSE;
	if (!w && m_CurGun != WEAP_NONE)
	{
		m_CurGun = WEAP_NONE;
		m_CurGunSlot = m_InventoryMgr.GetCurrentWeaponSlot();
		bChange = DTRUE;
	}
	if (w && m_CurGun != w->GetType())
	{
		m_CurGun = w->GetType();
		m_CurGunSlot = m_InventoryMgr.GetCurrentWeaponSlot();
		bChange = DTRUE;
	}
	if (bChange)
	{
		// Toggle binoculars off if they are active
		if (m_InventoryMgr.IsItemActive(INV_BINOCULARS))
			m_InventoryMgr.SetActiveItem(INV_BINOCULARS);

		SendMessageToClient(SMSG_CHANGEWEAPON);
	}

	if (m_bDead)
	{
		m_InventoryMgr.ShowHandModels(DFALSE);

		int nRet = m_InventoryMgr.ChangeWeaponSlot(1);

		if (nRet == CHWEAP_NOTAVAIL)
		{
			m_InventoryMgr.ChangeWeaponSlot(0);
		}
	}

	// See if we should be cloaked, and set alpha if so.
/*	if (IsItemActive(INV_CLOAKING) && !m_bInvisible)
	{
		pServerDE->SetObjectColor(m_hObject, 0.0f, 0.0f, 0.0f, ALPHA_CLOAKED);
	}

	if (bNothingHappened)
	{
		IdleTime();
	}
	else
	{
		m_startIdle = pServerDE->GetTime();
	}
*/

	// [blg] Check for idle time

	if (bNothingHappened && (m_dwControlFlags == 0 || m_dwControlFlags == CTRLFLAG_RUN) && !m_bDead)
	{
		m_fIdleTime += pServerDE->GetFrameTime();

		if (m_fIdleTime > MAX_IDLE_TIME)
		{
			ResetIdleTime();
			PlayVoiceGroupEventOnClient(VME_IDLE, DTRUE);
		}
	}
	else
	{
		ResetIdleTime();
	}


	// Set client view pos to the camera level

	pServerDE->GetObjectPos(m_hObject, &m_vLastPos);

	DVector vListenPos;
	if (!g_hActiveCamera)
	{
		if (IsItemActive(INV_THEEYE))
		{ 
			CInvTheEye* pEye = (CInvTheEye*)m_InventoryMgr.GetItem(INV_THEEYE);
			if (pEye)
			{
				DVector vListenPos;
				HOBJECT hObj = pEye->GetEyeObject();
				if (hObj)
				{
					pServerDE->GetObjectPos(hObj, &vListenPos);
					pServerDE->SetClientViewPos(m_hClient, &vListenPos);
				}
			}
		}
		else if(m_hOrbObj)
		{
			DVector vListenPos;
			pServerDE->GetObjectPos(m_hOrbObj, &vListenPos);
			pServerDE->SetClientViewPos(m_hClient, &vListenPos);
		}
		else	// not the eye
		{
			VEC_COPY(vListenPos, m_vLastPos);
			vListenPos.y += m_fEyeLevel;
			pServerDE->SetClientViewPos(m_hClient, &vListenPos);

			if( pServerDE->GetTime( ) > m_fNextPingTime )
			{
				m_fNextPingTime = pServerDE->GetTime( ) + PINGPERIOD;
				pServerDE->PingObjects(m_hObject);
			}
		}
	}
	else
	{
		pServerDE->GetObjectPos(g_hActiveCamera, &vListenPos);
		pServerDE->SetClientViewPos(m_hClient, &vListenPos);

		if( pServerDE->GetTime( ) > m_fNextPingTime )
		{
			m_fNextPingTime = pServerDE->GetTime( ) + PINGPERIOD;
			pServerDE->PingObjects(g_hActiveCamera);
		}

	}


	// If we're outside the world in a multiplayer game, we deserve to die...
		
	if (g_pBloodServerShell->IsMultiplayerGame())
	{
		DVector vPos, vMin, vMax;
		pServerDE->GetWorldBox(vMin, vMax);
		pServerDE->GetObjectPos(m_hObject, &vPos);	

		if (vPos.x < vMin.x || vPos.y < vMin.y || vPos.z < vMin.z ||
			vPos.x > vMax.x || vPos.y > vMax.y || vPos.z > vMax.z)
		{
			DVector vNull;
			VEC_INIT(vNull);
			DamageObject(m_hObject, (LPBASECLASS)this, m_hObject, 100.0f, vNull, vNull, DAMAGE_TYPE_DEATH);
		}
	}


	// All done...

	return DTRUE;
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateStats()
//
//	PURPOSE:	Updates player stats.
//
// --------------------------------------------------------------------------- //
void CPlayerObj::UpdateStats()
{
	 // Check to see if the status bar display items have changed
	DDWORD nItem;

	m_nStatsFlags = 0;

	// See if the health has changed, send to client
	nItem = (DDWORD)m_damage.GetHitPoints();
	if (m_nLastHitPoints != nItem)
	{
		// If we took damage, tell the client
		if (m_nLastHitPoints > nItem)
		{
			m_nLastDamage = m_nLastHitPoints - nItem;
			m_nStatsFlags |= PLAYERSTATS_TOOKDAMAGE;

			if (IsRandomChance(69 + m_nLastDamage))
			{
				if (m_damage.GetLastDamageType() == DAMAGE_TYPE_FIRE)
				{
					PlayVoiceGroupEventOnClient(VME_BURNING, DTRUE);
				}
				else
				{
					PlayVoiceGroupEventOnClient(VME_PAIN, DTRUE);
				}
			}
		}

		m_nLastHitPoints = nItem;
		m_nStatsFlags |= PLAYERSTATS_HEALTH;
	}

	// See if the ammo has changed, send to client
	nItem = (DDWORD)m_InventoryMgr.GetCurrentWeaponAmmoCount();
	if (m_nLastAmmo != nItem)
	{
		m_nLastAmmo = nItem;
		m_nStatsFlags |= PLAYERSTATS_AMMO;
	}

	// See if the ammo has changed, send to client
	nItem = (DDWORD)m_InventoryMgr.GetCurrentWeaponAltFireAmmoCount();
	if (m_nLastAltAmmo != nItem)
	{
		m_nLastAltAmmo = nItem;
		m_nStatsFlags |= PLAYERSTATS_ALTAMMO;
	}

	// See if the armor has changed, send to client
	nItem = (DDWORD)m_damage.GetArmorPoints();
	if (m_nLastArmor != nItem)
	{
		m_nLastArmor = nItem;
		m_nStatsFlags |= PLAYERSTATS_ARMOR;
	}

	// See if the focus has changed, send to client
	nItem = (DDWORD)m_InventoryMgr.GetAmmoCount(AMMO_FOCUS);
	if (m_nLastFocus != nItem)
	{
		m_nLastFocus = nItem;
		m_nStatsFlags |= PLAYERSTATS_FOCUS;
	}


	// See if air level has changed...

	if (m_fOldAirLevel != m_fAirLevel)
	{
		m_fOldAirLevel = m_fAirLevel;
		m_nStatsFlags |= PLAYERSTATS_AIRLEVEL;
	}

	if( m_nStatsFlags )
		SendMessageToClient( SMSG_PLAYERSTATSUPDATE );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ToggleSpectatorMode()
//
//	PURPOSE:	Toggles spectator mode on or off.
//
// --------------------------------------------------------------------------- //
void CPlayerObj::ToggleSpectatorMode()
{
	CServerDE* pServerDE = GetServerDE();

	m_bSpectatorMode = !m_bSpectatorMode;
	SendMessageToClient(SMSG_SPECTATORMODE);

	// Kludge to fix problem with sinking through floor...
	if (m_bSpectatorMode)
	{
		DVector pos;
		
		pServerDE->GetObjectPos(m_hObject, &pos);
		pos.y += 50.0;
		pServerDE->MoveObject(m_hObject, &pos);
	}

	// This is a KLUDGE to fix problem with sinking through
	// the ground when switching to spectator mode.  We'll
	// process a frame before we actually switch...
	if (m_bSpectatorMode)
	{
		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
		DVector dims;

		VEC_INIT(dims)

		HSTRING hstr = pServerDE->FormatString(IDS_GENERAL_SPECTATORON);
		SendConsoleMessage(pServerDE->GetStringData(hstr));
		pServerDE->FreeString(hstr);

//		dwFlags &= ~(FLAG_SHADOW | FLAG_TOUCH_NOTIFY | FLAG_SOLID);
		dwFlags &= ~(FLAG_TOUCH_NOTIFY | FLAG_SOLID);
		dwFlags |= FLAG_GOTHRUWORLD;

		pServerDE->SetObjectFlags(m_hObject, dwFlags);
//		pServerDE->SetObjectDims(m_hObject, &dims);
//		pServerDE->SetVelocity(m_hObject, &dims);
//		pServerDE->SetAcceleration(m_hObject, &dims);
	}
	else
	{
		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
		dwFlags &= ~FLAG_GOTHRUWORLD;
		dwFlags |= FLAG_TOUCH_NOTIFY | FLAG_SOLID;
//		dwFlags |= FLAG_SHADOW | FLAG_TOUCH_NOTIFY | FLAG_SOLID;
//		DVector dims = {12.0, gCharacterValues[m_nCharacter].fHeightStanding/2, 12.0};

		HSTRING hstr = pServerDE->FormatString(IDS_GENERAL_SPECTATOROFF);
		SendConsoleMessage(pServerDE->GetStringData(hstr));
		pServerDE->FreeString(hstr);

		pServerDE->SetObjectFlags(m_hObject, dwFlags);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Respawn()
//
//	PURPOSE:	Resets player attributes
//
// --------------------------------------------------------------------------- //
void CPlayerObj::Respawn()
{
	// Get our server pointer...
	CServerDE* pServerDE = GetServerDE();

	// If we're currently dead in our humiliation any, gib...
	if (pServerDE->GetModelAnimation(m_hObject) == m_Anim_Sound.m_nAnim_HUMILIATION[3])
	{
		if (m_bDead)
		{
			int    nType   = m_damage.GetLastDamageType();
			DFLOAT fDamage = m_damage.GetLastDamageAmount();

			DVector vDir;
			m_damage.GetLastDamageDirection(&vDir);

			CreateGibs(vDir, ((int)m_damage.GetMass())>>5, nType, fDamage);
		}
	}

	// Check for spectator mode...
	if (m_bSpectatorMode)
		ToggleSpectatorMode();

	// Respawn as necessary...
	if (!m_damage.IsDead() || g_pBloodServerShell->GetGameType() != GAMETYPE_SINGLE)
	{
		// Set up the filenames
		SetCharacter(m_nCharacter);

		// Reset any hidden nodes
		AIShared.ResetNodes(m_hObject);

		// Set flags
		DDWORD  dwFlags = FLAG_VISIBLE | FLAG_MODELKEYS | FLAG_YROTATION | FLAG_ANIMTRANSITION | FLAG_TOUCH_NOTIFY | FLAG_SOLID | FLAG_STAIRSTEP | FLAG_SHADOW;
//		DDWORD  dwFlags = FLAG_SOLID | FLAG_VISIBLE | FLAG_TOUCH_NOTIFY |  FLAG_STAIRSTEP | FLAG_SHADOW | FLAG_KEEPALIVE | FLAG_FULLPOSITIONRES | FLAG_FORCECLIENTUPDATE | FLAG_MODELKEYS | FLAG_YROTATION | FLAG_ANIMTRANSITION | FLAG_GRAVITY;
		pServerDE->SetObjectFlags(m_hObject, dwFlags);

		DVector dims;
		VEC_INIT(dims)
		pServerDE->SetVelocity(m_hObject, &dims);
//		m_fEyeLevel = gCharacterValues[m_nCharacter].fEyeLevelStanding;
//		SendMessageToClient(SMSG_EYELEVEL);

//		m_damage.Reset();
		m_damage.SetHitPoints(100);
		m_damage.SetArmorPoints(0);
		m_bDead = DFALSE;
		m_fAirLevel	= MAX_AIR_LEVEL;

		m_fDeathTimer         = 0;
		m_fSlowDeathSafeTimer = 0;
		m_fSlowDeathStayTimer = 0;
		m_bInSlowDeath        = DFALSE;

		m_bBurning = DFALSE;

		if (g_pBloodServerShell->IsMultiplayerGame())
		{
			DFLOAT fDeltaTime = g_pServerDE->GetTime() - s_fCreatedTime;

			if (fDeltaTime > 5.0f && IsRandomChance(65))
			{
				PlayVoiceGroupEventOnClient(VME_SPAWN);
			}

			SetMultiplayerAmmo();			
		}
		else
		{
			for (int i = AMMO_BULLET; i <= AMMO_MAXAMMOTYPES; i++)
				m_InventoryMgr.SetAmmoCount(i, 0.0f);

			m_InventoryMgr.SetAmmoCount(AMMO_BULLET, DEFAULT_BULLETS);
		}
	}

	// Dump the bomb list.
	if( m_pBombList )
	{
		ObjectLink* pLink = m_pBombList->m_pFirstLink;
		while( pLink )
		{
			g_pServerDE->RemoveObject( pLink->m_hObject );
			pLink = pLink->m_pNext;
		}

		pServerDE->RelinquishList( m_pBombList );
		m_pBombList = DNULL;
	}

	UpdateClientPhysics(); // (to make sure they have their model around)
	TeleportClientToServerPos();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GoToStartPoint()
//
//	PURPOSE:	Teleports the player to a startpoint.
//
// --------------------------------------------------------------------------- //
void CPlayerObj::GoToStartPoint()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !g_pBloodServerShell)
		return;

	DVector pos;

	VEC_INIT(pos)

	// Get all the Blood 2 start points...

	ObjectList*	pList = pServerDE->FindNamedObjects("Blood2StartPoint");
	if (!pList || !pList->m_pFirstLink || pList->m_nInList <= 0) 
	{
		return;
	}

	GameStartPoint* pGSP = DNULL;
	ObjectLink* pLink = pList->m_pFirstLink;

	// For a multiplayer game, just get a random startpoint
	if(g_pBloodServerShell->GetGameType() != GAMETYPE_SINGLE)
	{
		int nStartPoint = pServerDE->IntRandom(0, pList->m_nInList - 1);
		for (int i=0; i < nStartPoint; i++)
		{
			pLink = pLink->m_pNext;
		}

		pGSP = (GameStartPoint*)pServerDE->HandleToObject(pLink->m_hObject);

		// For team-based games, find a start point for our team...
		if (g_pBloodServerShell->IsMultiplayerTeamBasedGame())
		{
			int nSafety = pList->m_nInList;

			while (pGSP && pGSP->GetTeamID() != (int)GetTeamID() && pGSP->GetTeamID() != 0)
			{
				pLink = pLink->m_pNext;
				if (!pLink) pLink = pList->m_pFirstLink;

				if (pLink)
				{
					pGSP = (GameStartPoint*)pServerDE->HandleToObject(pLink->m_hObject);
				}
				else
				{
					pGSP = NULL;
				}

				if (--nSafety < 0)
				{
					pGSP = NULL;
				}
			}

			if (!pGSP)
			{
				pLink = pList->m_pFirstLink;
				if (pLink) pGSP = (GameStartPoint*)pServerDE->HandleToObject(pLink->m_hObject);
			}
		}
	}
	else  // Single player, go to a named startpoint, if it exists
	{
		DBOOL bRemoveString = DFALSE;
		GameStartPoint *pGSPSingle = DNULL;
		HSTRING hstrStartPoint = g_pBloodServerShell->GetStartPointName();

		if (!hstrStartPoint) 
		{
			bRemoveString = DTRUE;
			hstrStartPoint = pServerDE->CreateString("start");
		}

		pGSP = DNULL;
		for (int i=0; i < pList->m_nInList; i++)
		{
			pGSP = (GameStartPoint*)pServerDE->HandleToObject(pLink->m_hObject);

			if (!pGSP->IsMultiplayer())
			{
				// save it just in case
				pGSPSingle = pGSP;

				if (pServerDE->CompareStringsUpper(hstrStartPoint, pGSP->GetName()))
				{
					break;
				}
			}
			pLink = pLink->m_pNext;
			pGSP = DNULL;
		}
		
		if( bRemoveString )
			pServerDE->FreeString( hstrStartPoint );

		// Couldn't find a named startpoint, try to use any single player
		if (!pGSP && pGSPSingle) pGSP = pGSPSingle;
	}

	DRotation rRot;
	ROT_INIT(rRot);
	if (pGSP)
	{
		// Set the rotation..
		m_fYaw = pGSP->GetPitchYawRoll().y;
		m_fPitch = 0.0f;
		HOBJECT hGSP = pServerDE->ObjectToHandle((LPBASECLASS)pGSP);
		pServerDE->GetObjectPos(hGSP, &pos);

		// Send any trigger message this is supposed to cause
		pGSP->SendTrigger();
	}

	pServerDE->TeleportObject(m_hObject, &pos);
	SendMessageToClient(SMSG_FORCEROTATION);

	MoveObjectToGround(m_hObject);
	
	pServerDE->RelinquishList(pList);

	UpdateClientPhysics(); // (to make sure they have their model around)
	TeleportClientToServerPos();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ValidateAttributes()
//
//	PURPOSE:	Make sure that the player's attributes are within valid ranges
//
// --------------------------------------------------------------------------- //
void CPlayerObj::ValidateAttributes()
{
	CServerDE* pServerDE = GetServerDE();

	m_nCharacter		= DCLAMP(m_nCharacter, 0, MAX_CHARACTER);
	m_nSkin				= DCLAMP(m_nSkin, MULTIPLAY_SKIN_MIN, MULTIPLAY_SKIN_MAX);
	m_nAttribStrength	= DCLAMP(m_nAttribStrength, 1, 5);
	m_nAttribSpeed		= DCLAMP(m_nAttribSpeed, 1, 5);
	m_nAttribResistance = DCLAMP(m_nAttribResistance, 1, 5);
	m_nAttribMagic		= DCLAMP(m_nAttribMagic, 1, 5);

	if ((m_nAttribStrength + m_nAttribSpeed + m_nAttribResistance + m_nAttribMagic) > MAX_ATTRIBUTES)
	{
		HSTRING hstr = pServerDE->FormatString(IDS_GENERAL_ATTRIBHIGH);
		SendConsoleMessage(pServerDE->GetStringData(hstr));
		pServerDE->FreeString(hstr);

		//  Attribute maximums exceeded
		m_nAttribStrength = DEFAULT_ATTRIBUTE;
		m_nAttribSpeed = DEFAULT_ATTRIBUTE;
		m_nAttribResistance = DEFAULT_ATTRIBUTE;
		m_nAttribMagic = DEFAULT_ATTRIBUTE;
	}

	m_InventoryMgr.SetStrength(m_nAttribStrength + m_nBindingStrength);
	m_InventoryMgr.SetMagic(m_nAttribMagic + m_nBindingMagic);
	m_damage.CalculateResistance(m_nAttribResistance + m_nBindingResistance);

	SetMoveVel(gMoveVel[m_nAttribSpeed-1]);
	SetJumpVel(DEFAULT_JUMPVEL);
	m_PStateChangeFlags = PSTATE_ALL;

	//	m_InventoryMgr.SetFullAmmo();
/*
	if (m_bBindingBlending)
		pServerDE->SetObjectColor(m_hObject, 0.0f, 0.0f, 0.0f, ALPHA_CLOAKED);

	if (m_bBindingConstitution)
	{
		m_damage.SetHitPoints(150);
		m_damage.SetMaxHitPoints(150);
	}

	if (m_bBindingIncreasedDamage)
	{
		m_InventoryMgr.AddDamageMultiplier(BINDING_INCREASE_DAMAGE_AMT);
	}
	
	if (m_bBindingQuickness)
	{
		m_InventoryMgr.AddFireRateMultiplier(BINDING_QUICKNESS_FACTOR);
	}
*/
	// Set attributes
	m_damage.SetMaxHitPoints(g_fMaxHealth[m_nAttribStrength-1]);
	m_damage.SetMaxMegaHitPoints(g_fMaxMegaHealth[m_nAttribStrength-1]);
	m_damage.SetMaxArmorPoints(g_fMaxArmor[m_nAttribMagic-1]);
	m_damage.SetMaxNecroArmorPoints(g_fMaxMegaArmor[m_nAttribMagic-1]);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetCharacter
//
//	PURPOSE:	Sets filenames to refer to the current character.
//
// --------------------------------------------------------------------------- //

void CPlayerObj::SetCharacter(DBYTE nCharacter)
{
	CServerDE* pServerDE = GetServerDE();

	if (nCharacter > MAX_CHARACTER) return;
	
	m_nCharacter = nCharacter;

	pServerDE->SetModelFilenames(m_hObject, 
								 gCharacterValues[m_nCharacter].szModelName, 
								 gCharacterSkins[m_nCharacter][m_nSkin]);

	m_PStateChangeFlags |= PSTATE_MODELFILENAMES;

	// Init the animations
	m_Anim_Sound.SetAnimationIndexes(m_hObject);
	m_Anim_Sound.GenerateHitSpheres(m_hObject);

	sprintf(msgbuf, "sounds\\chosen\\%s", gCharacterValues[m_nCharacter].szName);
	m_Anim_Sound.SetSoundRoot(msgbuf);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ObjectTouch()
//
//	PURPOSE:	Called when the player is touching another TouchNotify object.
//
// --------------------------------------------------------------------------- //
void CPlayerObj::ObjectTouch(HOBJECT hObj, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();
	DBOOL bDoFallingDamage;


	// Hack to see if we are being pushed by a rotating door. If so, set velocity back
	if( hObj )
	{
		HCLASS hObjClass  = pServerDE->GetObjectClass( hObj );
		HCLASS hDoorClass = pServerDE->GetClass( "RotatingDoor" );
		if ( pServerDE->IsKindOf( hObjClass, hDoorClass ) && !m_bDoorPush )
		{
			RotatingDoor *pRotDoor = (RotatingDoor*)pServerDE->HandleToObject(hObj);
			if( pRotDoor && pRotDoor->GetDoorState() == DOORSTATE_OPENING && pRotDoor->CanPushPlayerBack())
			{
				// Get my vectors
				DRotation rRot;
				DVector vF, vU, vR;
				pServerDE->GetObjectRotation(m_hObject, &rRot);
				pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

				// Set some backwards velocity to the player to try to move him out of the way
				VEC_MULSCALAR(m_vDoorPush, vF, -500.0f);
				m_bDoorPush = DTRUE;
//				pServerDE->SetVelocity(m_hObject, &vF);

				return;
			}
		}
	}

	bDoFallingDamage = DTRUE;
	if( g_pBloodServerShell->IsMultiplayerGame( ))
	{
		if( !g_pBloodServerShell->GetNetGameInfo( )->m_bFallDamage )
			bDoFallingDamage = DFALSE;
	}
/*
	if (fData >= PA_MIN_DAMAGE_FORCE && bDoFallingDamage )
	{
		pServerDE->BPrint("impact force=%f", fData);
		// Damage myself
		DVector vFall, vPos;
		VEC_SET(vFall, 0.0f, -1.0f, 0.0f);
		DFLOAT fDamage;
		
		// Set some minimum guidelines
		if (m_bFalling && (pServerDE->GetTime() - m_startFall) >= 2.0f) 
		{
			fDamage = (fData - PA_MIN_DAMAGE_FORCE) / 25;

			pServerDE->GetObjectPos(m_hObject, &vPos);

			DamageObject(m_hObject, this, m_hObject, fDamage, vFall, vPos, DAMAGE_TYPE_NORMAL);
		}
	}
*/
	DVector vCurVel;
	pServerDE->GetVelocity(m_hObject, &vCurVel);

	if (bDoFallingDamage )
	{
		// Damage myself
		// Set some minimum guidelines
		if ((m_bFalling || m_bServerFalling) && vCurVel.y < -1100.0f) 
		{
			DVector vFall, vPos;
			VEC_SET(vFall, 0.0f, -1.0f, 0.0f);
			DFLOAT fDamage;
			
			fDamage = (fData - PA_MIN_DAMAGE_FORCE) / 25;

			if (fDamage > 0)
			{
				pServerDE->GetObjectPos(m_hObject, &vPos);
				DamageObject(m_hObject, this, m_hObject, fDamage, vFall, vPos, DAMAGE_TYPE_NORMAL);
			}
		}
	}
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ObjectCrush()
//
//	PURPOSE:	Called when the player is touching another TouchNotify object.
//
// --------------------------------------------------------------------------- //
void CPlayerObj::ObjectCrush(HOBJECT hObj, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();

	// if we are being crushed by a rotating door, tell it to stop moving.
	if (hObj)
	{
		HCLASS hObjClass  = pServerDE->GetObjectClass( hObj );
		HCLASS hDoorClass = pServerDE->GetClass( "RotatingDoor" );
		if ( pServerDE->IsKindOf( hObjClass, hDoorClass ))
		{
			RotatingDoor *pRotDoor = (RotatingDoor*)pServerDE->HandleToObject(hObj);
			if (pRotDoor->GetDoorState() == DOORSTATE_OPENING)
			{
				// Send the message back to the RotatingDoor
				HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, hObj, MID_DOORBLOCK);
				pServerDE->EndMessage(hMessage);
			}
		}
	}
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::OnMessage()
//
//	PURPOSE:	Processes a message from the client.
//
// --------------------------------------------------------------------------- //
void CPlayerObj::OnMessage(DBYTE messageID, HMESSAGEREAD hMsg)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !g_pBloodServerShell) return;

	switch(messageID)
	{
		case CMSG_PLAYERUPDATE:
		{
			if (!m_bActivated) m_bActivated = DTRUE;

			m_eLastContainerCode = m_eContainerCode;

			DBYTE byUpdateFlags		= pServerDE->ReadFromMessageByte(hMsg);
			if (byUpdateFlags & PLAYERUPDATE_FLAGS)
				m_byFlags	= pServerDE->ReadFromMessageByte(hMsg);

			if (byUpdateFlags & PLAYERUPDATE_PITCH)
				m_fPitch = pServerDE->ReadFromMessageFloat(hMsg);

			if (byUpdateFlags & PLAYERUPDATE_YAW)
				m_fYaw = pServerDE->ReadFromMessageFloat(hMsg);

			if (byUpdateFlags & PLAYERUPDATE_MUZZLE)
				pServerDE->ReadFromMessageCompVector(hMsg, &m_vGunMuzzlePos);
			
			if (byUpdateFlags & PLAYERUPDATE_LMUZZLE)
				pServerDE->ReadFromMessageCompVector(hMsg, &m_vlGunMuzzlePos);
			
			if (byUpdateFlags & PLAYERUPDATE_MOUSE)
			{
				m_fMouseAxis0		= (DFLOAT)(pServerDE->ReadFromMessageByte(hMsg) - 128);
				m_fMouseAxis1		= (DFLOAT)(pServerDE->ReadFromMessageByte(hMsg) - 128);
			}
			
			if (byUpdateFlags & PLAYERUPDATE_CONTAINER)
			{
				DBYTE nContainerCode;

				nContainerCode = (ContainerCode)pServerDE->ReadFromMessageByte(hMsg);
				m_bExternalCamera = ( nContainerCode & 0x80 ) ? DTRUE : DFALSE;
				m_eContainerCode = ( ContainerCode )( nContainerCode & 0x7F );
			}

			if (byUpdateFlags & PLAYERUPDATE_CONTAINERATBOTTOM)
			{
				HOBJECT hContainer;
				hContainer = pServerDE->ReadFromMessageObject(hMsg);
				if( hContainer )
				{
					HMESSAGEWRITE hMsg;

					hMsg = g_pServerDE->StartMessageToObject( this, hContainer, MID_INCONTAINER );
					g_pServerDE->EndMessage( hMsg );
				}
			}

			// Update the rotation
			pServerDE->SetupEuler(&m_rRotation, m_fPitch, m_fYaw, 0.0f);

			// Save the forward vector too
			DVector vUp, vRight;
			pServerDE->GetRotationVectors(&m_rRotation, &vUp, &vRight, &m_vForward);

            if (IsItemActive(INV_THEEYE))
            { 
				CInvTheEye* pEye = (CInvTheEye*)m_InventoryMgr.GetItem(INV_THEEYE);
				if (pEye)
					pEye->SetRotation(&m_rRotation);
            }
            else if (!m_damage.IsDead())
            {
				DRotation rot;
				// Only use the yaw to compute the player model rotation
				pServerDE->SetupEuler(&rot, 0.0f, m_fYaw, 0.0f);
				pServerDE->SetObjectRotation(m_hObject, &rot);
            }

			// Set player weapon visible
			if (m_byFlags & CDATA_SHOWGUN && !m_bDead)
				m_InventoryMgr.ShowViewWeapons(DTRUE);
			else
				m_InventoryMgr.ShowViewWeapons(DFALSE);

		}
		break;

		case CMSG_PLAYERPOSITION:
		{
			HandlePlayerPositionMessage(hMsg);
		}
		break;

		case CMSG_MULTIPLAYER_INIT:
			SendVoiceMessage(VOICE_START_BB, g_pServerDE->IntRandom(0, 0xffff), 0, 0);
			// Fall through
		case CMSG_SINGLEPLAYER_INIT:
		{
			DBYTE nVal;
			int i;
			int nHealingRate;

//			Don't do this, it screws up some stuff (Greg)
//			m_InventoryMgr.Term();
//			m_InventoryMgr.Init(m_hObject, m_hClient);

			// An initialization from the client, respawn when done
			m_hstrPlayerName = pServerDE->ReadFromMessageHString(hMsg);
			m_nCharacter         = pServerDE->ReadFromMessageByte(hMsg);
			m_nSkin				 = pServerDE->ReadFromMessageByte(hMsg);
			m_nAttribStrength    = pServerDE->ReadFromMessageByte(hMsg);
			m_nAttribSpeed       = pServerDE->ReadFromMessageByte(hMsg);
			m_nAttribResistance  = pServerDE->ReadFromMessageByte(hMsg);
			m_nAttribMagic       = pServerDE->ReadFromMessageByte(hMsg);

			ClearPowerupValues();
/*			for(i = 0; i < SLOTCOUNT_BINDINGS; i++)
			{
				nVal = pServerDE->ReadFromMessageByte(hMsg);
				if(nVal)	SetPowerupValue(nVal);
			}
*/
			nHealingRate = g_pBloodServerShell->GetNetGameInfo( )->m_nHealingRate;
			if( g_pBloodServerShell->IsMultiplayerGame( ) && nHealingRate )
			{
				m_fServerRegenPeriod = (( DFLOAT )HEAL_REALLYFAST / nHealingRate );
				m_bServerRegeneration = DTRUE;
			}

			// Check if we need to use a team skin...
			if (g_pBloodServerShell->IsMultiplayerTeamBasedGame())
			{
				CTeam* pTeam = g_pBloodServerShell->GetTeamMgr()->GetTeam(GetTeamID());
				if (pTeam)
				{
					m_nSkin = (DBYTE)(pTeam->GetID() + 4);
				}
			}

			// Make sure attributes are ok, and set inventory values etc.
			ValidateAttributes();

			// Get default weapons;
			m_InventoryMgr.ObtainWeapon(WEAP_MELEE, 0);
			for (i=1;i<SLOTCOUNT_WEAPONS;i++)
			{
				nVal = pServerDE->ReadFromMessageByte(hMsg);
				// See if we should obtain 2
				if (nVal & DUAL_HAND_MASK)
				{
					nVal &= ~DUAL_HAND_MASK;
					m_InventoryMgr.ObtainWeapon(nVal, i);
				}
				m_InventoryMgr.ObtainWeapon(nVal, i);
			}

			// Default to whatever is in slot 1
			m_InventoryMgr.ChangeWeaponSlot(1);

			Respawn();
		}
		break;

		case CMSG_PLAYERMESSAGE:
		{
			char *pszPlayerName;

			if (g_pBloodServerShell->GetGameType() != GAMETYPE_SINGLE)
			{
				pszPlayerName = pServerDE->GetStringData(m_hstrPlayerName);
				if( pszPlayerName )
				{
					_mbscpy(( unsigned char * )msgbuf, ( const unsigned char * )pszPlayerName );
				}
				else
				{
					msgbuf[0] = 0;
				}
				_mbscat((unsigned char*)msgbuf, (const unsigned char*)":");

				char *pMsg = pServerDE->ReadFromMessageString(hMsg);

				// Substitute variables
				while(*pMsg)
				{
					if (*pMsg == '%')
					{
						pMsg++;
						switch(*pMsg)
						{
							case 'n':
							case 'N':
								{
									if (m_hstrPlayerName)
									{
										char *pszPlayerName = pServerDE->GetStringData(m_hstrPlayerName);
										if( pszPlayerName )
											_mbscat((unsigned char*)msgbuf, (const unsigned char*)pszPlayerName);
									}
								}
								break;
							case 'k':
							case 'K': 
								{
									if (m_hstrWhoIKilledLast)
									{
										char *pszWhoIKilledLast = pServerDE->GetStringData(m_hstrWhoIKilledLast);
										if( pszWhoIKilledLast )
											_mbscat((unsigned char*)msgbuf, (const unsigned char*)pszWhoIKilledLast );
									}
								}
								break;
							case 's':
							case 'S':
								{
									if (m_hstrWhoKilledMeLast)
									{
										char *pszWhoKilledMeLast = pServerDE->GetStringData(m_hstrWhoKilledMeLast);
										if( pszWhoKilledMeLast )
											_mbscat((unsigned char*)msgbuf, (const unsigned char*)pszWhoKilledMeLast );
									}
								}
								break;
							default:
								{
									if (strlen((pMsg-1)) >= 2)
									{
										_mbsncat((unsigned char*)msgbuf, (const unsigned char*)(pMsg-1), 2);
									}
									else
									{
										_mbsncat((unsigned char*)msgbuf, (const unsigned char*)(pMsg-1), 1);
										pMsg--;
									}
								}
								break;
						}
					}
					else
						_mbsncat((unsigned char*)msgbuf, (const unsigned char*)pMsg, 1);
					pMsg++;
				}
			}
			else
			{
				_mbscpy((unsigned char*)msgbuf, (const unsigned char*)":");
				_mbscat((unsigned char*)msgbuf, (const unsigned char*)pServerDE->ReadFromMessageString(hMsg));
			}
			g_pBloodServerShell->SendBlood2ServConsoleMessage(msgbuf);
			SendMessageToClient(SMSG_CONSOLEMESSAGE_ALL);
		}
		break;

		case CMSG_CHEAT:
		{
			// Got a cheat message from the client
			DBYTE nCheatCode = pServerDE->ReadFromMessageByte(hMsg);
			DBOOL bState     = pServerDE->ReadFromMessageByte(hMsg);
			ProcessCheat(nCheatCode, bState);
		}
		break;
            
		case CMSG_SETCURRENTWEAPON:		// NEW MESSAGE BY ANDY
		{
			DBYTE nNewSlot = pServerDE->ReadFromMessageByte(hMsg);
			int nChRes;

			if ((nChRes = m_InventoryMgr.ChangeWeaponSlot(nNewSlot)) == CHWEAP_OK)
			{
//				m_byFlags &= ~CDATA_CANFIRE;	// Can't fire until client says so.
			}
			CWeapon *w = m_InventoryMgr.GetCurrentWeapon();
			if (w && !w->IsAltFireZoom() && m_bZoomView)
			{
				if (!IsItemActive(INV_BINOCULARS))
					SetZoomMode(DFALSE);
			}
		}
		break;

		case CMSG_FRAGSELF:
		{
			DVector vNull;
			VEC_INIT(vNull);
			DamageObject(m_hObject, (LPBASECLASS)this, m_hObject, 100.0f, vNull, vNull, DAMAGE_TYPE_DEATH);
		}
		break;

/*
		case CMSG_USECURRENTINVITEM:	// NEW MESSAGE BY ANDY
		{
			DBYTE nItemSlot = pServerDE->ReadFromMessageByte(hMsg);
			if(nItemSlot < 4)
				m_InventoryMgr.SetActiveItem(nItemSlot);
			else
			{
				DDWORD dwWeapon = nItemSlot - 4 + WEAP_PROXIMITYBOMB;
				int nChRes;
				if ((nChRes = m_InventoryMgr.SelectInventoryWeapon(dwWeapon)) == CHWEAP_OK)
				{
//					m_byFlags &= ~CDATA_CANFIRE;	// Can't fire until client says so.
				}

				if (!m_InventoryMgr.GetCurrentWeapon()->IsAltFireZoom() && m_bZoomView)
				{
					if (!IsItemActive(INV_BINOCULARS))
						SetZoomMode(DFALSE);
				}
			}
		}
		break;

		case CMSG_USECURRENTSPELL:		// NEW MESSAGE BY ANDY
		{
			DBYTE nSpellSlot = pServerDE->ReadFromMessageByte(hMsg);
			m_InventoryMgr.SetActiveSpell(nSpellSlot);
		}
		break;

		case CMSG_DROPINVITEM:			// NEW MESSAGE BY ANDY
		{
			DBYTE slot = pServerDE->ReadFromMessageByte(hMsg);
			if(m_InventoryMgr.DropItem(slot))
			{
				m_DropItemSlot = slot;
				SendMessageToClient(SMSG_REMOVEITEM);
			}
		}
		break;

		case CMSG_DROPSPECIFICWEAPON:	// NEW MESSAGE BY ANDY
		{
			DBYTE slot = pServerDE->ReadFromMessageByte(hMsg);
			if(m_InventoryMgr.DropWeapon(slot) == CHWEAP_OK)
			{
				m_DropGunSlot = slot;
				SendMessageToClient(SMSG_DELETESPECIFICWEAPON);
			}
		}
		break;

		case CMSG_DROPCURRENTWEAPON:	// NEW MESSAGE BY ANDY
		{
			if(m_InventoryMgr.DropCurrentWeapon() == CHWEAP_OK)
				SendMessageToClient(SMSG_DELETEWEAPON);
		}
		break;

		case CMSG_DELETESPELL:
		{
			DBYTE type = pServerDE->ReadFromMessageByte(hMsg);
			m_InventoryMgr.RemoveSpell(type);
		}
		break;
*/
		case CMSG_RESETPLAYERINVENTORY:
		{
			ResetPlayerInventory();
		}
		break;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::OnCommandOn()
//
//	PURPOSE:	Handles a command activation.
//
// --------------------------------------------------------------------------- //
void CPlayerObj::OnCommandOn(DDWORD commandNum)
{
	CServerDE* pServerDE = GetServerDE();

	if (m_bDead && commandNum != COMMAND_USE)
		return;

	if (m_bMovementBlocked)
	{
		DBOOL bReturn = DTRUE;
		// These commands need to be checked even if movement is blocked
		// See if eye should be deactivated
		if (IsItemActive(INV_THEEYE) && commandNum == COMMAND_USEINVENTORY)
		{
			CInvItem *item = m_InventoryMgr.GetItem(INV_THEEYE);
			if (item)
				bReturn = DFALSE;
		}
		if (bReturn)
			return;
	}

	switch(commandNum)
	{
		case COMMAND_USE:
			{
				if (m_bDead && g_pBloodServerShell->IsMultiplayerGame())
				{
					Respawn();
					GoToStartPoint();
				}
				else if (m_bInSlowDeath)
				{
					if  (m_fSlowDeathStayTimer <= 0)
					{
						m_bInSlowDeath = DFALSE;	// saved!
						m_fDeathTimer = 0;
						m_damage.Heal(1);
						SendWonkyVisionMsg(0.0f, 0);
					}
				}
				else	// cast a ray to see what we hit, and send it a message
				{
					IntersectQuery iq;
					IntersectInfo  ii;
					DVector	vDir;
					
					pServerDE->GetObjectPos(m_hObject, &iq.m_From);
					iq.m_From.y += m_fEyeLevel;
			
					VEC_MULSCALAR(vDir, m_vForward, DIST_USE);
					VEC_ADD(iq.m_To, iq.m_From, vDir);

					iq.m_Flags = INTERSECT_OBJECTS;
					iq.m_FilterFn = NULL;
					iq.m_pUserData = NULL;	

					if (g_pServerDE->IntersectSegment(&iq, &ii))
					{
						// Hit something, send a trigger message to it (unless it's a door)
						if (ii.m_hObject && !IsDoor(pServerDE->GetObjectClass(ii.m_hObject)))
						{
							HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, ii.m_hObject, MID_TRIGGER);
							pServerDE->WriteToMessageHString(hMessage, m_hstrTrigger);
							pServerDE->EndMessage(hMessage);
						}
					}
				}
			}
			break;

		case COMMAND_ALTFIRE:
			{
				// Alt fire for this weapon is Zoom
				CWeapon *w = m_InventoryMgr.GetCurrentWeapon();
				if (w && w->IsAltFireZoom())
				{
					// Don't toggle it off if binoculars are active
					if (!m_bZoomView || !IsItemActive(INV_BINOCULARS))
					{
//						PlaySoundFromPos(&(m_vGunMuzzlePos), "Sounds\\Weapons\\sniper\\Zoom.wav", 1000.0f, SOUNDTYPE_MISC, SOUNDPRIORITY_MEDIUM);
						PlaySoundFromObject( m_hObject, "Sounds\\Weapons\\sniper\\Zoom.wav", 1000.0f, SOUNDPRIORITY_MISC_MEDIUM, DFALSE,
							DFALSE, DFALSE, 100, DFALSE, DTRUE );
						SetZoomMode(!m_bZoomView, 99); // Number is the cursor type (AM)
					}
				}
			}
			break;

		case COMMAND_WEAPON_0:
		case COMMAND_WEAPON_1:
		case COMMAND_WEAPON_2:
		case COMMAND_WEAPON_3:
		case COMMAND_WEAPON_4:
		case COMMAND_WEAPON_5:
		case COMMAND_WEAPON_6:
		case COMMAND_WEAPON_7:
		case COMMAND_WEAPON_8:
		case COMMAND_WEAPON_9:
			{
				DDWORD nNewSlot = commandNum - COMMAND_WEAPON_0;

				int nChRes;
				if ((nChRes = m_InventoryMgr.ChangeWeaponSlot((DBYTE)nNewSlot)) == CHWEAP_OK)
				{
//					m_byFlags &= ~CDATA_CANFIRE;	// Can't fire until client says so.
				}
				CWeapon *w = m_InventoryMgr.GetCurrentWeapon();
				if (w && !m_InventoryMgr.GetCurrentWeapon()->IsAltFireZoom() && m_bZoomView)
				{
					if (!IsItemActive(INV_BINOCULARS))
						SetZoomMode(DFALSE);
				}
			}
			break;

		case COMMAND_NEXTWEAPON:
			m_InventoryMgr.SelectNextWeapon();
			break;

		case COMMAND_PREVWEAPON:
			m_InventoryMgr.SelectPrevWeapon();
			break;
		
		case COMMAND_DROPWEAPON:
			if(m_InventoryMgr.DropCurrentWeapon() == CHWEAP_OK)
				SendMessageToClient(SMSG_DELETEWEAPON);
			break;

		case COMMAND_NEXTINVENTORY:
			{
				// Select the next item in the inventory
				m_InventoryMgr.SelectNextItem();
//				SendMessageToClient(SMSG_ITEMCHANGED);
				break;
			}
		case COMMAND_PREVINVENTORY:
			{
				// Select the next item in the inventory and send a message to the client
				m_InventoryMgr.SelectPrevItem();
//				SendMessageToClient(SMSG_ITEMCHANGED);
				break;
			}
		case COMMAND_USEINVENTORY:
			{	
//				int nItemIndex=m_InventoryMgr.GetCurrentItemIndex();
				CInvItem *pCurrentItem=m_InventoryMgr.GetCurrentItem();
				if ( pCurrentItem )
				{
					m_InventoryMgr.SetActiveItem(pCurrentItem->GetType());
				}
				break;
			}

		case COMMAND_INVITEM_0:
		case COMMAND_INVITEM_1:
		case COMMAND_INVITEM_2:
		case COMMAND_INVITEM_3:
			{
				if (!(m_dwControlFlags & CTRLFLAG_RUN))	// HACKHACK temporary so it doesn't affect weapon adjust keys
					m_InventoryMgr.SetActiveItem((DBYTE)(commandNum - COMMAND_INVITEM_0));
			}
			break;

		case COMMAND_PROXIMITIES:
		case COMMAND_REMOTES:
		case COMMAND_TIME:
			{
				DDWORD dwWeapon = commandNum - COMMAND_PROXIMITIES + WEAP_PROXIMITYBOMB;
				int nChRes;
				if ((nChRes = m_InventoryMgr.SelectInventoryWeapon(dwWeapon)) == CHWEAP_OK)
				{
//					m_byFlags &= ~CDATA_CANFIRE;	// Can't fire until client says so.
				}

				CWeapon *w = m_InventoryMgr.GetCurrentWeapon();
				if (w && !w->IsAltFireZoom() && m_bZoomView)
				{
					if (!IsItemActive(INV_BINOCULARS))
						SetZoomMode(DFALSE);
				}
				break;
			}
#ifndef _DEMO
		case COMMAND_DETONATE:
			{
				if(m_pBombList)
				{
					ObjectLink* pLink = m_pBombList->m_pFirstLink;
					while(pLink)
					{
						CRemoteBomb* pRemote = (CRemoteBomb*)pServerDE->HandleToObject(pLink->m_hObject);
						pRemote->Detonate();
						g_pServerDE->BreakInterObjectLink( m_hObject, pLink->m_hObject );
						pLink = pLink->m_pNext;
					}

					pServerDE->RelinquishList(m_pBombList);
					m_pBombList = 0;
				}
				break;
			}
#endif

		default:
			return;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::OnCommandOff()
//
//	PURPOSE:	Handles a command deactivation.
//
// --------------------------------------------------------------------------- //
void CPlayerObj::OnCommandOff(DDWORD commandNum)
{
	CServerDE* pServerDE = GetServerDE();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::OnStringKey
//
//	PURPOSE:	Handles a string key in the player model (for footsteps..)
//
// ----------------------------------------------------------------------- //

void CPlayerObj::OnStringKey(ArgList* pArgList)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	if(pArgList->argc > 1 && _mbsicmp((const unsigned char*)pKey, (const unsigned char*)"play_sound") == 0)
	{
		char	szSound[MAX_CS_FILENAME_LEN+1];
		char*	pSound = pArgList->argv[1];

		if( _mbsicmp((const unsigned char*)pSound,(const unsigned char*)"footstep") == 0 )
		{
			if ( !m_bDisableFootsteps && GetFootstepSound( szSound ))
			{
				PlayPlayerSound(szSound, 400.0f, 70, DFALSE, SOUNDPRIORITYMOD_LOW);
			}
		}
		else
		{
			sprintf(szSound, "%s.wav", pSound);
			m_Anim_Sound.GetSoundPath(szSound);

			PlayPlayerSound(szSound, 1000.0f, 100, DFALSE, SOUNDPRIORITYMOD_LOW);
		}
	}
//	else
// 		CBaseCharacter::OnStringKey(pArgList);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SendWonkyVisionMsg
//
//	PURPOSE:	Sends a message to start wonkyvision on the client
//
// --------------------------------------------------------------------------- //

void CPlayerObj::SendWonkyVisionMsg(DFLOAT fTime, DBOOL bNoMove)
{
	CServerDE* pServerDE = GetServerDE();

//	HMESSAGEWRITE hMsg = pServerDE->StartMessage(m_hClient, SMSG_WONKYVISION);
	HMESSAGEWRITE hMsg = StartMessageToClient(SMSG_WONKYVISION);
	if( !hMsg )
		return;
	pServerDE->WriteToMessageFloat(hMsg, fTime);
	pServerDE->WriteToMessageByte(hMsg, (DBYTE)bNoMove);
	pServerDE->EndMessage(hMsg);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SendFadeMsg
//
//	PURPOSE:	Sends a message to start fadein/fadeout on the client
//
// --------------------------------------------------------------------------- //

void CPlayerObj::SendFadeMsg(DBYTE byFadeIn)
{
	CServerDE* pServerDE = GetServerDE();

//	HMESSAGEWRITE hMsg = pServerDE->StartMessage(m_hClient, SMSG_SCREENFADE);
	HMESSAGEWRITE hMsg = StartMessageToClient(SMSG_SCREENFADE);
	if( !hMsg )
		return;
	pServerDE->WriteToMessageByte(hMsg, byFadeIn);
	pServerDE->EndMessage(hMsg);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SendVoiceMessage
//
//	PURPOSE:	Sends the appropriate "the voice" message
//
// --------------------------------------------------------------------------- //

void CPlayerObj::SendVoiceMessage(TheVoice eVoiceType, D_WORD wIndex, DBYTE nAttackerCharacter, DBYTE nVictimCharacter)
{
	CServerDE* pServerDE = GetServerDE();

	DBYTE byFlags = 0;

	switch (nAttackerCharacter)
	{
		case CHARACTER_GABREILLA:
		case CHARACTER_OPHELIA:
			byFlags |= VOICEFLAG_FEMALE_ATTACKER;
			break;
		default:
			byFlags |= VOICEFLAG_MALE_ATTACKER;
			break;
	}
	
	switch (nVictimCharacter)
	{
		case CHARACTER_GABREILLA:
		case CHARACTER_OPHELIA:
			byFlags |= VOICEFLAG_FEMALE_VICTIM;
			break;
		default:
			byFlags |= VOICEFLAG_MALE_VICTIM;
			break;
	}
	
//	HMESSAGEWRITE hMsg = pServerDE->StartMessage(m_hClient, SMSG_THEVOICE);
	HMESSAGEWRITE hMsg = StartMessageToClient(SMSG_THEVOICE);
	if( !hMsg )
		return;
	pServerDE->WriteToMessageByte(hMsg, (DBYTE)eVoiceType);
	pServerDE->WriteToMessageWord(hMsg, wIndex);
	pServerDE->WriteToMessageByte(hMsg, byFlags);
	pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SendMessageToClient()
//
//	PURPOSE:	Sends a message to this player's client shell.
//
// --------------------------------------------------------------------------- //
void CPlayerObj::SendMessageToClient(DBYTE messageID)
{
	CServerDE* pServerDE = GetServerDE();
	HMESSAGEWRITE hMsg;
	
	if (messageID < 0 || messageID >= SMSG_MAXMESSAGES)
		return;

	// Start the message (also check for broadcast message)
	if (messageID == SMSG_CONSOLEMESSAGE_ALL)
		hMsg = pServerDE->StartMessage(NULL, messageID);
	else
//		hMsg = pServerDE->StartMessage(m_hClient, messageID);
		hMsg = StartMessageToClient(messageID);
	if( !hMsg )
		return;

	// Send the appropriate data for this message
	switch(messageID)
	{
		case SMSG_CONSOLEMESSAGE:
		case SMSG_CONSOLEMESSAGE_ALL:
			pServerDE->WriteToMessageString(hMsg, msgbuf);
			break;
 
		case SMSG_EYELEVEL:
			pServerDE->WriteToMessageFloat(hMsg, m_fEyeLevel);
			break;

		case SMSG_UPDATEPLAYER:		// Update message for the frag bar
			pServerDE->WriteToMessageDWord(hMsg, pServerDE->GetClientID(m_hClient));	// Client ID
			pServerDE->WriteToMessageByte(hMsg, (DBYTE)m_nLastHitPoints);						// Health
			pServerDE->WriteToMessageByte(hMsg, m_Frags);
			pServerDE->WriteToMessageByte(hMsg, 0);
			break;

		case SMSG_PLAYERSTATSUPDATE:
			pServerDE->WriteToMessageByte(hMsg, m_nStatsFlags );
			if( m_nStatsFlags & PLAYERSTATS_HEALTH )
			{
				pServerDE->WriteToMessageWord(hMsg, ( D_WORD )DCLAMP( m_nLastHitPoints, 0, 65535 ));
				pServerDE->WriteToMessageWord(hMsg, ( D_WORD )DCLAMP( m_damage.GetMaxHitPoints(), 0, 65535 ));
			}
			if( m_nStatsFlags & PLAYERSTATS_AMMO )
			{
				m_nLastAmmo = DCLAMP( m_nLastAmmo, 0, 999 );
				pServerDE->WriteToMessageWord(hMsg, ( D_WORD )DCLAMP( m_nLastAmmo, 0, 65535 ));
			}
			if( m_nStatsFlags & PLAYERSTATS_ALTAMMO )
			{
				m_nLastAltAmmo = DCLAMP( m_nLastAltAmmo, 0, 999 );
				pServerDE->WriteToMessageWord(hMsg, ( D_WORD )DCLAMP( m_nLastAltAmmo, 0, 65535 ));
			}
			if( m_nStatsFlags & PLAYERSTATS_ARMOR )
			{
				pServerDE->WriteToMessageWord(hMsg, ( D_WORD )DCLAMP( m_nLastArmor, 0, 65535 ));
				pServerDE->WriteToMessageWord(hMsg, ( D_WORD )DCLAMP( m_damage.GetMaxArmorPoints(), 0, 65535 ));
			}
			if( m_nStatsFlags & PLAYERSTATS_FOCUS )
			{
				pServerDE->WriteToMessageWord(hMsg, ( D_WORD )DCLAMP( m_nLastFocus, 0, 65535 ));
			}
			if( m_nStatsFlags & PLAYERSTATS_TOOKDAMAGE )
			{
				m_nLastDamage = DCLAMP(m_nLastDamage, 0, 100);
				pServerDE->WriteToMessageByte(hMsg, (DBYTE)m_nLastDamage);
				pServerDE->WriteToMessageByte(hMsg, m_damage.GetLastDamageType());
			}
			if( m_nStatsFlags & PLAYERSTATS_AIRLEVEL )
			{
				pServerDE->WriteToMessageFloat(hMsg, m_fAirLevel / MAX_AIR_LEVEL);
			}
			break;

		case SMSG_HEALTH:
			pServerDE->WriteToMessageWord(hMsg, ( D_WORD )DCLAMP( m_nLastHitPoints, 0, 65535 ));
			pServerDE->WriteToMessageWord(hMsg, ( D_WORD )DCLAMP( m_damage.GetMaxHitPoints(), 0, 65535 ));
			break;

		case SMSG_ARMOR:
			pServerDE->WriteToMessageWord(hMsg, ( D_WORD )DCLAMP( m_nLastArmor, 0, 65535 ));
			pServerDE->WriteToMessageWord(hMsg, ( D_WORD )DCLAMP( m_damage.GetMaxArmorPoints(), 0, 65535 ));
			break;

		case SMSG_AMMO:
			pServerDE->WriteToMessageWord(hMsg, ( D_WORD )DCLAMP( m_nLastAmmo, 0, 65535 ));
			break;

		case SMSG_FOCUS:
			pServerDE->WriteToMessageWord(hMsg, ( D_WORD )DCLAMP( m_nLastFocus, 0, 65535 ));
			break;

		case SMSG_TOOKDAMAGE:
			m_nLastDamage = DCLAMP(m_nLastDamage, 0, 100);
			pServerDE->WriteToMessageByte(hMsg, (DBYTE)m_nLastDamage);
			pServerDE->WriteToMessageByte(hMsg, m_damage.GetLastDamageType());
			break;

		case SMSG_SPECTATORMODE:
			pServerDE->WriteToMessageByte(hMsg, m_bSpectatorMode);
			break;

		case SMSG_DELETESPECIFICWEAPON:
			pServerDE->WriteToMessageByte(hMsg, (DBYTE)m_DropGunSlot);
			break;

		case SMSG_DELETEWEAPON:
			pServerDE->WriteToMessageByte(hMsg, (DBYTE)m_CurGunSlot);
			break;

		case SMSG_CHANGEWEAPON:
			pServerDE->WriteToMessageByte(hMsg, (DBYTE)m_CurGunSlot);
			break;

		case SMSG_ZOOMVIEW:
			pServerDE->WriteToMessageByte(hMsg, (DBYTE)m_bZoomView);
			pServerDE->WriteToMessageByte(hMsg, m_nZoomType);
			break;

		case SMSG_FORCEROTATION:
			pServerDE->WriteToMessageFloat(hMsg, m_fPitch);
			pServerDE->WriteToMessageFloat(hMsg, m_fYaw);
			break;

		case SMSG_DEAD:			// nothing else..
			break;
	}

	// Finish it off..
	pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SendConsoleMessage()
//
//	PURPOSE:	Sends a message to this player's client shell.
//
// --------------------------------------------------------------------------- //
void CPlayerObj::SendConsoleMessage(char *msg)
{
	_mbscpy((unsigned char*)msgbuf, (const unsigned char*)msg);
	SendMessageToClient(SMSG_CONSOLEMESSAGE);
}

	
// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ProcessCheat()
//
//	PURPOSE:	Processes a cheat message from the client
//
// --------------------------------------------------------------------------- //
void CPlayerObj::ProcessCheat(DBYTE nCheatCode, DBOOL bState)
{
	CServerDE* pServerDE = GetServerDE();

	// No cheats in multiplayer
	if (g_pBloodServerShell->IsMultiplayerGame())
	{
		return;
	}

	// Weapon cheat
	if (nCheatCode >= CHEAT_FIRSTWEAPON && nCheatCode <= CHEAT_LASTWEAPON)
	{
		m_InventoryMgr.ObtainWeapon((nCheatCode - CHEAT_FIRSTWEAPON + 1), 
									m_InventoryMgr.GetCurrentWeaponSlot());
		return;
	}

	switch (nCheatCode)
	{
		case CHEAT_KFA:
			m_InventoryMgr.SetFullAmmo();

			if (!m_InventoryMgr.HasWeapon(WEAP_BERETTA)) m_InventoryMgr.ObtainWeapon(WEAP_BERETTA);
			if (!m_InventoryMgr.HasWeapon(WEAP_SUBMACHINEGUN)) m_InventoryMgr.ObtainWeapon(WEAP_SUBMACHINEGUN);
			if (!m_InventoryMgr.HasWeapon(WEAP_SHOTGUN)) m_InventoryMgr.ObtainWeapon(WEAP_SHOTGUN);
			if (!m_InventoryMgr.HasWeapon(WEAP_ASSAULTRIFLE)) m_InventoryMgr.ObtainWeapon(WEAP_ASSAULTRIFLE);
			if (!m_InventoryMgr.HasWeapon(WEAP_LIFELEECH)) m_InventoryMgr.ObtainWeapon(WEAP_LIFELEECH);
			if (!m_InventoryMgr.HasWeapon(WEAP_TESLACANNON)) m_InventoryMgr.ObtainWeapon(WEAP_TESLACANNON);
			if (!m_InventoryMgr.HasWeapon(WEAP_ORB)) m_InventoryMgr.ObtainWeapon(WEAP_ORB);
			if (!m_InventoryMgr.HasWeapon(WEAP_VOODOO)) m_InventoryMgr.ObtainWeapon(WEAP_VOODOO);
			if (!m_InventoryMgr.HasWeapon(WEAP_DEATHRAY)) m_InventoryMgr.ObtainWeapon(WEAP_DEATHRAY);
			if (!m_InventoryMgr.HasWeapon(WEAP_MINIGUN)) m_InventoryMgr.ObtainWeapon(WEAP_MINIGUN);
			if (!m_InventoryMgr.HasWeapon(WEAP_NAPALMCANNON)) m_InventoryMgr.ObtainWeapon(WEAP_NAPALMCANNON);
			if (!m_InventoryMgr.HasWeapon(WEAP_SNIPERRIFLE)) m_InventoryMgr.ObtainWeapon(WEAP_SNIPERRIFLE);

			PlayVoiceGroupEventOnClient(VME_WEAPON, DTRUE);

			break;

		case CHEAT_KFA2:
			m_InventoryMgr.SetFullAmmo();

			if (!m_InventoryMgr.HasWeapon(WEAP_BERETTA)) m_InventoryMgr.ObtainWeapon(WEAP_BERETTA);
			if (!m_InventoryMgr.HasWeapon(WEAP_FLAREGUN)) m_InventoryMgr.ObtainWeapon(WEAP_FLAREGUN);
			if (!m_InventoryMgr.HasWeapon(WEAP_HOWITZER)) m_InventoryMgr.ObtainWeapon(WEAP_HOWITZER);
			if (!m_InventoryMgr.HasWeapon(WEAP_SINGULARITY)) m_InventoryMgr.ObtainWeapon(WEAP_SINGULARITY);
			if (!m_InventoryMgr.HasWeapon(WEAP_BUGSPRAY)) m_InventoryMgr.ObtainWeapon(WEAP_BUGSPRAY);
			if (!m_InventoryMgr.HasWeapon(WEAP_SNIPERRIFLE)) m_InventoryMgr.ObtainWeapon(WEAP_SNIPERRIFLE);
			if (!m_InventoryMgr.HasWeapon(WEAP_NAPALMCANNON)) m_InventoryMgr.ObtainWeapon(WEAP_NAPALMCANNON);
			if (!m_InventoryMgr.HasWeapon(WEAP_MINIGUN)) m_InventoryMgr.ObtainWeapon(WEAP_MINIGUN);
			if (!m_InventoryMgr.HasWeapon(WEAP_DEATHRAY)) m_InventoryMgr.ObtainWeapon(WEAP_DEATHRAY);
			if (!m_InventoryMgr.HasWeapon(WEAP_VOODOO)) m_InventoryMgr.ObtainWeapon(WEAP_VOODOO);
			if (!m_InventoryMgr.HasWeapon(WEAP_ORB)) m_InventoryMgr.ObtainWeapon(WEAP_ORB);

			PlayVoiceGroupEventOnClient(VME_WEAPON, DTRUE);

			break;

		case CHEAT_GOD:
			m_damage.SetGodMode(bState);
			if (bState)	PlayVoiceGroupEventOnClient(VME_POWERUP, DTRUE);
			break;

		case CHEAT_AMMO:
			m_InventoryMgr.SetFullAmmo();
			break;

		case CHEAT_HEALTH:
			m_damage.Reset();
			if (m_bDead)
			{
				// Reset any hidden nodes
				AIShared.ResetNodes(m_hObject);

				// Set flags
				DDWORD  dwFlags = FLAG_FORCECLIENTUPDATE | FLAG_MODELKEYS | FLAG_YROTATION | FLAG_ANIMTRANSITION | FLAG_TOUCH_NOTIFY | FLAG_SOLID | FLAG_STAIRSTEP | FLAG_SHADOW;
//				DDWORD  dwFlags = FLAG_SOLID | FLAG_VISIBLE | FLAG_TOUCH_NOTIFY | FLAG_STAIRSTEP | FLAG_SHADOW | FLAG_MODELGOURAUDSHADE | FLAG_KEEPALIVE | FLAG_FULLPOSITIONRES | FLAG_FORCECLIENTUPDATE | FLAG_MODELKEYS | FLAG_YROTATION | FLAG_ANIMTRANSITION;
				pServerDE->SetObjectFlags(m_hObject, dwFlags);
				m_bDead = DFALSE;
				m_fAirLevel	= MAX_AIR_LEVEL;
				m_fDeathTimer = 0;
				m_bInSlowDeath = DFALSE;
				m_bBurning = DFALSE;
			}
			break;
/*
		case CHEAT_STEALTH:
			AddPowerup(NULL, POWERUP_STEALTH);
			break;

		case CHEAT_TRIPLEDAMAGE:
			AddPowerup(NULL, POWERUP_ANGER);
			break;
*/
		case CHEAT_CLIP:
			ToggleSpectatorMode();
			break;

		case CHEAT_KILLALLAI:
			{
				HCLASS hClass = pServerDE->GetClass("AI_Mgr");
				HSTRING hMsg = pServerDE->CreateString("KILL");
				if (hClass && hMsg)
				{
					SendTriggerMsgToClass((LPBASECLASS)this, hClass, hMsg);
				}
				// For good measure..
				HSTRING hName = pServerDE->CreateString("mime");
				SendTriggerMsgToObjects(this, hName, hMsg);
				pServerDE->FreeString(hName);
				pServerDE->FreeString(hMsg);
			}
			break;

		case CHEAT_INCSPEED:
			{
				m_nAttribSpeed += 1;
				if (m_nAttribSpeed > 5) m_nAttribSpeed = 1;

				HSTRING hstr = pServerDE->FormatString(IDS_GENERAL_SPEED, m_nAttribSpeed + m_nBindingSpeed);
				SendConsoleMessage(pServerDE->GetStringData(hstr));
				pServerDE->FreeString(hstr);
			}
			break;

		case CHEAT_INCSTRENGTH:
			{
				m_nAttribStrength += 1;
				if (m_nAttribStrength > 5) m_nAttribStrength = 1;
				m_InventoryMgr.SetStrength(m_nAttribStrength + m_nBindingStrength);
				m_damage.SetMaxHitPoints(g_fMaxHealth[m_nAttribStrength-1]);
				m_damage.SetMaxMegaHitPoints(g_fMaxMegaHealth[m_nAttribStrength-1]);

				HSTRING hstr = pServerDE->FormatString(IDS_GENERAL_STRENGTH, m_nAttribStrength);
				SendConsoleMessage(pServerDE->GetStringData(hstr));
				pServerDE->FreeString(hstr);
			}
			break;

		case CHEAT_CALEB:
			SetCharacter(CHARACTER_CALEB);
			break;

		case CHEAT_OPHELIA:
			SetCharacter(CHARACTER_OPHELIA);
			break;

		case CHEAT_ISHMAEL:
			SetCharacter(CHARACTER_ISHMAEL);
			break;

		case CHEAT_GABRIELLA:
			SetCharacter(CHARACTER_GABREILLA);
			break;

		case CHEAT_GIVEALLINV:
		{
			m_InventoryMgr.AddItem(INV_MEDKIT, 100);
			m_InventoryMgr.AddItem(INV_FLASHLIGHT, 100);
			m_InventoryMgr.AddItem(INV_NIGHTGOGGLES, 100);
			m_InventoryMgr.AddItem(INV_THEEYE, 100);
			m_InventoryMgr.AddItem(INV_BINOCULARS, 100);
			break;
		}

		case CHEAT_POW_HEALTH:
		{
			AddPowerup(NULL, POWERUP_HEALTH, 25);
			break;
		}
			
		case CHEAT_POW_MEGAHEALTH:
		{
			AddPowerup(NULL, POWERUP_MEGAHEALTH, 100);
			break;
		}
			
		case CHEAT_POW_WARD:
		{
			AddPowerup(NULL, POWERUP_WARD, 25);
			break;
		}
			
		case CHEAT_POW_NECROWARD:
		{
			AddPowerup(NULL, POWERUP_NECROWARD, 100);
			break;
		}
			
		case CHEAT_POW_INVULN:
		{
			AddPowerup(NULL, POWERUP_INVULNERABILITY, 30);
			break;
		}
			
		case CHEAT_POW_STEALTH:
		{
			AddPowerup(NULL, POWERUP_STEALTH, 30);
			break;
		}
			
		case CHEAT_POW_ANGER:
		{
			AddPowerup(NULL, POWERUP_ANGER, 30);
			break;
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetPowerupValue()
//
//	PURPOSE:	Handles adding a particular player powerup.
//
// --------------------------------------------------------------------------- //
void	CPlayerObj::SetPowerupValue(DBYTE nType)
{
/*	switch(nType)
	{
		case	POWERUP_STRENGTHBINDING:		m_nBindingStrength = 1;			break;
		case	POWERUP_SPEEDBINDING:			m_nBindingSpeed = 1;			break;
		case	POWERUP_RESISTANCEBINDING:		m_nBindingResistance = 1;		break;
		case	POWERUP_MAGICBINDING:			m_nBindingMagic = 1;			break;
		case	POWERUP_JUMPINGBINDING:			break;
		case	POWERUP_CLIMBINGBINDING:		break;
		case	POWERUP_STEALTHBINDING:			break;
		case	POWERUP_PROTECTIONBINDING:		break;
		case	POWERUP_CONSTITUTIONBINDING:	m_bBindingConstitution = 1;		break;
		case	POWERUP_REGENBINDING:			m_bBindingRegeneration = 1;		break;
		case	POWERUP_IMPACTRESBINDING:		m_bBindingImpactResistance = 1;	break;
		case	POWERUP_QUICKNESSBINDING:		m_bBindingQuickness = 1;		break;
		case	POWERUP_DAMAGEBINDING:			m_bBindingIncreasedDamage = 1;	break;
		case	POWERUP_BLENDINGBINDING:		m_bBindingBlending = 1;			break;
		case	POWERUP_SOULSTEALINGBINDING:	m_bBindingSoulStealing = 1;		break;
	}
*/
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ClearPowerupValues()
//
//	PURPOSE:	Handles adding a particular player powerup.
//
// --------------------------------------------------------------------------- //
void	CPlayerObj::ClearPowerupValues()
{
	m_nBindingStrength = 0;
	m_nBindingSpeed = 0;
	m_nBindingResistance = 0;
	m_nBindingMagic = 0;
	m_bBindingConstitution = 0;
	m_bBindingBlending = 0;
	m_bBindingSoulStealing = 0;
	m_bBindingRegeneration = 0;
	m_bBindingQuickness = 0;
	m_bBindingIncreasedDamage = 0;
	m_bBindingImpactResistance = 0;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AddPowerup()
//
//	PURPOSE:	Handles adding a particular player powerup.
//
// --------------------------------------------------------------------------- //
void CPlayerObj::AddPowerup(HOBJECT hSender, DBYTE nType, DFLOAT fValue)
{
	CServerDE* pServerDE = GetServerDE();
	DBOOL bPickedUp = DFALSE;

	switch(nType)
	{
		case POWERUP_HEALTH:
			if (m_damage.Heal(fValue))
				bPickedUp = DTRUE;
			break;
		case POWERUP_MEGAHEALTH:
			if (m_damage.MegaHeal(fValue))
				bPickedUp = DTRUE;
			break;
		case POWERUP_WARD:
			if (m_damage.AddWard(fValue))
				bPickedUp = DTRUE;
			break;
		case POWERUP_NECROWARD:
			if (m_damage.AddNecroWard(fValue))
				bPickedUp = DTRUE;
			break;
		case POWERUP_INVULNERABILITY:
			{
				m_fNighInvulnerableTime = pServerDE->GetTime() + fValue;
				m_damage.SetNighInvulnerable(DTRUE);
				bPickedUp = DTRUE;
				m_bPowerupActivated[PU_INVULNERABLE] = DTRUE;
				StartPowerupSound( PU_INVULNERABLE );

				DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hObject);
				pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags | USRFLG_WILLPOWERGLOW);
			}
			break;
		case POWERUP_STEALTH:
			{
				m_fInvisibleTime = pServerDE->GetTime() + fValue;
				pServerDE->SetObjectColor(m_hObject, 0.0f, 0.0f, 0.0f, ALPHA_INVISIBLE);
				DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
				dwFlags &= ~FLAG_SHADOW;
				pServerDE->SetObjectFlags(m_hObject, dwFlags);

				bPickedUp = DTRUE;
				m_bPowerupActivated[PU_INVISIBLE] = DTRUE;
				StartPowerupSound( PU_INVISIBLE );
			}
			break;
		case POWERUP_ANGER:
			{
				if (!m_bPowerupActivated[PU_TRIPLEDAMAGE])
				{
					m_InventoryMgr.AddDamageMultiplier(3.0f);
					m_bPowerupActivated[PU_TRIPLEDAMAGE] = DTRUE;
				}
				StartPowerupSound( PU_TRIPLEDAMAGE );
				m_fTripleDamageTime = pServerDE->GetTime() + fValue;
				bPickedUp = DTRUE;

				PlayVoiceGroupEventOnClient(VME_POWERUP, DTRUE);

				DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hObject);
				pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags | USRFLG_ANGERGLOW);
			}
			break;
	}
	if (hSender && bPickedUp)
		m_InventoryMgr.SendPickedUpMessage(hSender, CHWEAP_OK);
}

	
// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CheckPowerups()
//
//	PURPOSE:	Updates active powerup states
//
// --------------------------------------------------------------------------- //
void CPlayerObj::CheckPowerups()
{
	DBOOL bDone;
	DBOOL bNewLoopingSound;

	CServerDE* pServerDE = GetServerDE();

	DFLOAT fTime = pServerDE->GetTime();
	DFLOAT fFrameTime = pServerDE->GetFrameTime();

	// Server Regeneration
	if (m_bServerRegeneration && !m_bDead && !m_bInSlowDeath)
	{
		m_fServerRegenTime -= fFrameTime;
		if (m_fServerRegenTime <= 0)
		{
			m_fServerRegenTime = m_fServerRegenPeriod;
			m_damage.Heal(SERVER_REGEN_HEALPOINTS);
		}
	}

	// Regeneration binding
	if (m_bBindingRegeneration && !m_bDead)
	{
		m_fBindingRegenTime -= fFrameTime;
		if (m_fBindingRegenTime <= 0)
		{
			m_fBindingRegenTime = m_fBindingRegenPeriod;
			m_damage.Heal(BINDING_REGEN_HEALPOINTS);
		}
	}

	// Nigh Invulnerability
	if (m_bPowerupActivated[PU_INVULNERABLE] && fTime > m_fNighInvulnerableTime)
	{
		m_bPowerupActivated[PU_INVULNERABLE] = DFALSE;
		m_damage.SetNighInvulnerable(DFALSE);

		HSTRING hstr = pServerDE->FormatString(IDS_GENERAL_WILLPOWER_EXP);
		SendConsoleMessage(pServerDE->GetStringData(hstr));
		pServerDE->FreeString(hstr);

		EndPowerupSound( PU_INVULNERABLE );
		PlayVoiceGroupEventOnClient(VME_POWERUP, DTRUE);
		DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hObject);
		pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags & ~USRFLG_WILLPOWERGLOW);
	}
	
	// Invisibility
	if (m_bPowerupActivated[PU_INVISIBLE])
	{
		DBOOL bVisible = DFALSE;
		if (fTime > m_fInvisibleTime)
		{
			bVisible = DTRUE;
			m_bPowerupActivated[PU_INVISIBLE] = DFALSE;

			HSTRING hstr = pServerDE->FormatString(IDS_GENERAL_INVISIBILITY_EXP);
			SendConsoleMessage(pServerDE->GetStringData(hstr));
			pServerDE->FreeString(hstr);

			EndPowerupSound( PU_INVISIBLE );
		}
		// Blink if time is running out
		else if (fTime > m_fInvisibleTime - 5.0f)
		{
			fTime *= 2;
			fTime = fTime - (DFLOAT)floor(fTime);
			if (fTime*2 > 1.0f)
				bVisible = DTRUE;
		}

		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);

		DFLOAT fAlpha;
		
		fAlpha = ALPHA_NORMAL;
		if (!bVisible)
		{
			pServerDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_SHADOW);
			pServerDE->SetObjectColor(m_hObject, 0.0f, 0.0f, 0.0f, ALPHA_INVISIBLE);
		}
		else
		{
			pServerDE->SetObjectFlags(m_hObject, dwFlags | FLAG_SHADOW);
			pServerDE->SetObjectColor(m_hObject, 0.0f, 0.0f, 0.0f, fAlpha);
		}
	}
	
	// Triple Damage
	if (m_bPowerupActivated[PU_TRIPLEDAMAGE] && fTime > m_fTripleDamageTime)
	{
		m_InventoryMgr.RemoveDamageMultiplier(3.0f);
		m_bPowerupActivated[PU_TRIPLEDAMAGE] = DFALSE;

		HSTRING hstr = pServerDE->FormatString(IDS_GENERAL_TRIPLEDAMAGE_EXP);
		SendConsoleMessage(pServerDE->GetStringData(hstr));
		pServerDE->FreeString(hstr);

		EndPowerupSound( PU_TRIPLEDAMAGE );
		DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hObject);
		pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags & ~USRFLG_ANGERGLOW);
	}

	// Incorporeal
	if ( m_bPowerupActivated[PU_INCORPOREAL] && fTime > m_fIncorporealTime)
	{
		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
		pServerDE->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);
		m_bPowerupActivated[PU_INCORPOREAL] = DFALSE;
		EndPowerupSound( PU_INCORPOREAL );
	}

	// Silence
	if (m_bPowerupActivated[PU_SILENT] && fTime > m_fSilentTime)
	{
		m_bPowerupActivated[PU_SILENT] = DFALSE;
		m_Anim_Sound.SetSilent(DFALSE);
		EndPowerupSound( PU_SILENT );
	}

	// Assume we shouldn't play a new looping sound...
	bNewLoopingSound = DFALSE;

	// Check if start sound is done...
	if( m_hPowerupStartSound )
	{
		// If we just finished a powerup start sound, we can start playing the new
		// looping sound...
		if( g_pServerDE->IsSoundDone( m_hPowerupStartSound, &bDone ) == LT_OK && bDone )
		{
			// Kill the start sound...
			g_pServerDE->KillSound( m_hPowerupStartSound );
			m_hPowerupStartSound = DNULL;
			bNewLoopingSound = DTRUE;
		}
	}

	// Check if end sound is done...
	if( m_hPowerupEndSound )
	{
		// If we just finished a powerup start sound, we can start playing the new
		// looping sound...
		if( g_pServerDE->IsSoundDone( m_hPowerupEndSound, &bDone ) == LT_OK && bDone )
		{
			// Kill the start sound...
			g_pServerDE->KillSound( m_hPowerupEndSound );
			m_hPowerupEndSound = DNULL;
		}
	}

	// Start playing a new looping sound when the start sound is done, or
	// there are still sounds on stack and nothing is playing...
	if( bNewLoopingSound || ( !m_hPowerupStartSound && m_nPowerupSoundStackPos && !m_hPowerupLoopSound ))
	{
		// Any powerup sounds stacked?
		while( m_nPowerupSoundStackPos )
		{	
			PowerupType ePowerupSound;

			ePowerupSound = m_ePowerupSoundStack[m_nPowerupSoundStackPos - 1];

			// Make sure the powerup is actually going and it has a sound file...
			if( m_bPowerupActivated[ePowerupSound] && s_PowerupLoopingSounds[ePowerupSound][0] )
			{
				// Kill the old looping sound...
				if( m_hPowerupLoopSound )
				{
					g_pServerDE->KillSound( m_hPowerupLoopSound );
					m_hPowerupLoopSound = DNULL;
				}

				// Play the new looping sound...
				m_hPowerupLoopSound = PlaySoundFromObject( m_hObject, 
					s_PowerupLoopingSounds[ePowerupSound], 1000.0f, SOUNDPRIORITY_PLAYER_MEDIUM, DTRUE, DTRUE );

				// If the play was successful, then stop popping...
				if( m_hPowerupLoopSound )
					break;
			}
			else
				m_nPowerupSoundStackPos--;
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ResetPowerups()
//
//	PURPOSE:	Resets all active powerups to the OFF state
//
// --------------------------------------------------------------------------- //
void CPlayerObj::ResetPowerups()
{
	CServerDE* pServerDE = GetServerDE();

	DFLOAT fTime = pServerDE->GetTime();
	DFLOAT fFrameTime = pServerDE->GetFrameTime();

	// Nigh Invulnerability
	if (m_bPowerupActivated[PU_INVULNERABLE])
	{
		m_bPowerupActivated[PU_INVULNERABLE] = DFALSE;
		m_damage.SetNighInvulnerable(DFALSE);
		EndPowerupSound( PU_INVULNERABLE );
		DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hObject);
		pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags & ~USRFLG_WILLPOWERGLOW);
	}
	
	// Invisibility
	if (m_bPowerupActivated[PU_INVISIBLE])
	{
		m_bPowerupActivated[PU_INVISIBLE] = DFALSE;
		EndPowerupSound( PU_INVISIBLE );
		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
		pServerDE->SetObjectFlags(m_hObject, dwFlags | FLAG_SHADOW);
		pServerDE->SetObjectColor(m_hObject, 0.0f, 0.0f, 0.0f, ALPHA_NORMAL);
	}
	
	// Triple Damage
	if (m_bPowerupActivated[PU_TRIPLEDAMAGE])
	{
		m_InventoryMgr.RemoveDamageMultiplier(3.0f);
		m_bPowerupActivated[PU_TRIPLEDAMAGE] = DFALSE;
		EndPowerupSound( PU_TRIPLEDAMAGE );
		DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hObject);
		pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags & ~USRFLG_ANGERGLOW);
	}

	// Incorporeal
	if ( m_bPowerupActivated[PU_INCORPOREAL])
	{
		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
		pServerDE->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);
		m_bPowerupActivated[PU_INCORPOREAL] = DFALSE;
		EndPowerupSound( PU_INCORPOREAL );
	}

	// Silence
	if (m_bPowerupActivated[PU_SILENT])
	{
		m_bPowerupActivated[PU_SILENT] = DFALSE;
		m_Anim_Sound.SetSilent(DFALSE);
		EndPowerupSound( PU_SILENT );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::StartPowerupSound()
//
//	PURPOSE:	Switches powerup sound.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::StartPowerupSound( PowerupType ePowerup )
{
	if( m_nPowerupSoundStackPos < PU_MAXSTACK )
	{
		m_ePowerupSoundStack[m_nPowerupSoundStackPos] = ePowerup;
		m_nPowerupSoundStackPos++;
	}

	if( !m_hPowerupStartSound )
		m_hPowerupStartSound = PlaySoundFromObject( m_hObject, s_szPowerupStartSound, 
			1000.0f, SOUNDPRIORITY_PLAYER_LOW, DFALSE, DTRUE, DTRUE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::EndPowerupSound()
//
//	PURPOSE:	Ends powerup sound.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::EndPowerupSound( PowerupType ePowerup )
{
	// If the current powerup looping sound is the one that is ending, then kill the looping sound...
	if( m_ePowerupSoundStack[m_nPowerupSoundStackPos - 1] == ePowerup )
	{
		// Pop it...
		m_nPowerupSoundStackPos--;

		// Kill the old looping sound...
		if( m_hPowerupLoopSound )
		{
			g_pServerDE->KillSound( m_hPowerupLoopSound );
			m_hPowerupLoopSound = DNULL;
		}
	}

	if( !m_hPowerupEndSound )
		m_hPowerupEndSound = PlaySoundFromObject( m_hObject, s_szPowerupEndSound, 
			1000.0f, SOUNDPRIORITY_PLAYER_LOW, DFALSE, DTRUE, DTRUE );
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CheckItemsAndSpells()
//
//	PURPOSE:	Updates the player state based on active inventory items
//				and spells
//
// --------------------------------------------------------------------------- //
void CPlayerObj::CheckItemsAndSpells()
{
	CServerDE* pServerDE = GetServerDE();
    
//	char szFileName[101];
//	char szSkinName[101];
//	DBOOL bNewSkin = DFALSE;
//	DBOOL bNewModel = DFALSE;

//	pServerDE->GetModelFilenames(m_hObject, szFileName, 100, szSkinName, 100);

	m_bMovementBlocked = DFALSE;

	// See if client is telling us we can't move.
	if (!(m_byFlags & CDATA_CANMOVE))
		m_bMovementBlocked = DTRUE;

	// Check items that can block movement
	if (IsItemActive(INV_THEEYE))
		m_bMovementBlocked = DTRUE;

	// Are we imprisoned by an attack?
	if (m_bImprisoned)
	{
		if ((pServerDE->GetTime() - m_fImprisonStart) > m_fImprisonLength)
		{
			m_bImprisoned = DFALSE;

			HSTRING hstr = pServerDE->FormatString(IDS_GENERAL_CANMOVE);
			SendConsoleMessage(pServerDE->GetStringData(hstr));
			pServerDE->FreeString(hstr);
		}
		else
		{
			m_bMovementBlocked = DTRUE;
		}
	}
/*
	if (IsItemActive(SPELL_STONE))
	{
		m_bMovementBlocked = DTRUE;
	}

	if (m_eContainerCode == CC_CRANECONTROL)
	{
		m_bMovementBlocked = DTRUE;
		if (m_eLastContainerCode != CC_CRANECONTROL)
		{
			HMESSAGEWRITE hMsg = pServerDE->StartMessage(m_hClient, SMSG_CAMERA_SELECT);
			pServerDE->WriteToMessageObject(hMsg, m_hCameraObj);
			pServerDE->EndMessage(hMsg);
			SendConsoleMessage("Control the crane: Move left and right.  Move back or forward to stop.");
		}
	}
	else  
	if (m_hCameraObj)
	{
		// Turn off the camera
		HMESSAGEWRITE hMsg = pServerDE->StartMessage(m_hClient, SMSG_CUTSCENE_END);
		pServerDE->EndMessage(hMsg);
		m_hCameraObj = DNULL;
  }

	if (m_hCameraObj)
	{
		DVector vPos;
		pServerDE->GetObjectPos(m_hObject, &vPos);
		pServerDE->SetObjectPos(m_hCameraObj, &vPos);
	}
*/
}
	

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::EngineMessageFn()
//
//	PURPOSE:	Processes a message from the engine.
//
// --------------------------------------------------------------------------- //
DDWORD CPlayerObj::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	CServerDE* pServerDE = GetServerDE();
	DBOOL bResult = 1;

	switch (messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRet = CBaseCharacter::EngineMessageFn(messageID, pData, fData);

			PostPropRead((ObjectCreateStruct*)pData);
			return dwRet;
		}

		case MID_INITIALUPDATE:
			bResult = InitialUpdate((int)fData);
			break;

		case MID_UPDATE:
			bResult = Update((DVector*)pData);
			break;

		case MID_LINKBROKEN:
		{
			DBOOL bFound;
			DLink *pCur;
			ObjectLink *pLink, *pNextLink;

			HOBJECT hObj = (HOBJECT)pData;
			bFound = DFALSE;

			// Check if it's the orb
            if(hObj == m_hOrbObj)
			{
				m_hOrbObj = DNULL;
				bFound = DTRUE;
				break;
			}
			
			// Check if it's an attached object
			pCur = m_AttachedObjectsList.m_Head.m_pNext;
			while( pCur != &m_AttachedObjectsList.m_Head )
			{
				if( hObj == ( HOBJECT )pCur->m_pData )
				{
					dl_RemoveAt( &m_AttachedObjectsList, pCur );
					delete pCur;
					bFound = DTRUE;
					break;
				}
				pCur = pCur->m_pNext;
			}

			// Check the remote bombs
			if( !bFound )
			{
				if( m_pBombList )
				{
					pLink = m_pBombList->m_pFirstLink;
					while( pLink )
					{
						pNextLink = pLink->m_pNext;
						if( pLink->m_hObject == hObj )
						{
							g_pServerDE->RemoveObjectFromList( m_pBombList, pLink->m_hObject );
							bFound = DTRUE;
							break;
						}
						pLink = pNextLink;
					}
				}
			}

			break;
		}

		case MID_GETFORCEUPDATEOBJECTS:
			SetForceUpdateList((ForceUpdate*)pData);
			break;
	
		case MID_TOUCHNOTIFY:
			ObjectTouch((HOBJECT)pData, fData);
			break;

		case MID_CRUSH:
			ObjectCrush((HOBJECT)pData, fData);
			break;

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;

	}
	if (bResult)
		return CBaseCharacter::EngineMessageFn(messageID, pData, fData);
	else
		pServerDE->RemoveObject(m_hObject);
	return bResult;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ObjectMessageFn()
//
//	PURPOSE:	Processes a message from a server object.
//
// --------------------------------------------------------------------------- //
DDWORD CPlayerObj::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	HCLASS hVolumeBrushClass;

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			if( ProcessTriggerMsg(hSender, hRead))
				return 1;
		}
		break;

		case MID_DAMAGE:
		{
			// Special case hack!
			// Prevent volumebrush damage in the first 2 seconds of the level.  In the subway
			// level, because of the way the engine processes collisions with objects (volumebrushes), 
			// then collisions with the world, you can fall through the floor momentarilly and get hurt by
			// a volumebrush.
			if( pServerDE->GetTime( ) < 2.0f )
			{
				hVolumeBrushClass = pServerDE->GetClass( "VolumeBrush" );
				if( pServerDE->IsKindOf( pServerDE->GetObjectClass( hSender ), hVolumeBrushClass ))
				{
					return 0;
				}
			}

			// If slowly dying, this finishes him off...
			if (m_bInSlowDeath && m_fSlowDeathSafeTimer <= 0)
			{
				DoDeath(DTRUE);
			}
		}
		break;

		case MID_IMMOBILIZE:
		{
			if (!m_bImprisoned && !m_bDead) 
			{
				m_fImprisonLength = pServerDE->ReadFromMessageFloat(hRead);
				m_fImprisonStart = pServerDE->GetTime();
				m_bImprisoned = DTRUE;

				HSTRING hstr = pServerDE->FormatString(IDS_GENERAL_IMMOBILE);
				SendConsoleMessage(pServerDE->GetStringData(hstr));
				pServerDE->FreeString(hstr);
			}
		}
		break;

		case MID_ADDPOWERUP:
		{
			if (!m_bDead)
			{
				DBYTE nType = pServerDE->ReadFromMessageByte(hRead);
				DFLOAT fValue = pServerDE->ReadFromMessageFloat( hRead );
				AddPowerup( hSender, nType, fValue );
			}
		}
		break;
		
		case MID_ADDAMMO:
		{
			if (!m_bDead)
			{
				DBYTE nAmmoType = pServerDE->ReadFromMessageByte(hRead);
				DFLOAT fAmmoCount = pServerDE->ReadFromMessageFloat(hRead);
				// Try to add ammo
				int iRet = m_InventoryMgr.AddAmmo(nAmmoType, fAmmoCount);
				m_InventoryMgr.SendPickedUpMessage(hSender, iRet);
			}
		}
		break;

		// Hit an weapon powerup
		case MID_ADDWEAPON:
		{
			if (!m_bDead)
			{
				// Only pick up weapons in single player
				if (g_pBloodServerShell->GetGameType() != GAMETYPE_SINGLE)
					return CHWEAP_NOAVAILSLOTS;

				// Don't pick up something we just dropped.
				DBYTE nWeaponType = pServerDE->ReadFromMessageByte(hRead);
				DFLOAT fAmmoCount = pServerDE->ReadFromMessageFloat(hRead);

				// Try to add weapon...
				int iRet = m_InventoryMgr.ObtainWeapon(nWeaponType, -1);

				if( iRet == CHWEAP_OK && IsRandomChance(15))
					PlayWeaponVoiceSoundOnClient(nWeaponType);

				// Comes with ammo...
				if( iRet != CHWEAP_NOAVAILSLOTS && fAmmoCount > 0.0f )
				{
					// Try to add ammo
					WeaponData *pWeaponData = &g_WeaponDefaults[nWeaponType-1];

					// If we can use the ammo, then we'll pick up the gun regardless...
					if( m_InventoryMgr.AddAmmo( pWeaponData->m_nAmmoType, fAmmoCount ) == CHWEAP_OK )
						iRet = CHWEAP_OK;
				}

				m_InventoryMgr.SendPickedUpMessage(hSender, iRet);
			}
		}
		break;

		// Hit an inventory item pickup
		case MID_INVENTORYITEMTOUCH:
		{
			if( !m_bDead )
			{
				int iRet;
				DBYTE nItemType = pServerDE->ReadFromMessageByte (hRead);
				DBYTE nValue = pServerDE->ReadFromMessageByte( hRead );
				// Try to add item
				if(nItemType >= INV_BASEINVWEAPON && nItemType <= INV_LASTINVWEAPON)
				{
					iRet = m_InventoryMgr.AddInventoryWeapon(nItemType, nValue );
					m_InventoryMgr.AddItem( nItemType, nValue );
				}
				else
				{
					iRet = m_InventoryMgr.AddItem( nItemType, nValue );
				}
				m_InventoryMgr.SendPickedUpMessage(hSender, iRet);
			}
		}
		break;

		// Somebody wants to know if we have a key
		case MID_KEYQUERY:
		{
			if (!m_bDead)
			{
				HSTRING hstrItemName = pServerDE->ReadFromMessageHString (hRead);
				int iRet = m_InventoryMgr.QueryKey(hstrItemName);
				m_InventoryMgr.SendKeyQueryResponse(hSender, hstrItemName, iRet);

				pServerDE->FreeString(hstrItemName);
			}
		}
		break;

		// Hit a key item pickup
		case MID_KEYPICKUP:
		{
			if (!m_bDead)
			{
				HSTRING hstrItemName  = pServerDE->ReadFromMessageHString (hRead);
				HSTRING hstrDisplayName = pServerDE->ReadFromMessageHString (hRead);
				HSTRING hstrIconFile  = pServerDE->ReadFromMessageHString (hRead);
				HSTRING hstrIconFileH = pServerDE->ReadFromMessageHString (hRead);
				DBYTE	byUseCount    = pServerDE->ReadFromMessageByte (hRead);
				// Try to add item
				int iRet = m_InventoryMgr.AddKey(hstrItemName, hstrDisplayName, hstrIconFile, hstrIconFileH, byUseCount);
				m_InventoryMgr.SendPickedUpMessage(hSender, iRet);

				pServerDE->FreeString(hstrItemName);
				pServerDE->FreeString(hstrDisplayName);
				pServerDE->FreeString(hstrIconFile);
				pServerDE->FreeString(hstrIconFileH);
			}
		}

		// An object is attaching to us, like the seeing eye.
		case MID_ATTACH:
		{
			HOBJECT hObj;
			DLink *pLink;

			hObj = pServerDE->ReadFromMessageObject( hRead );
			if( hObj )
			{
				pLink = new DLink;
				dl_AddTail( &m_AttachedObjectsList, pLink, hObj );
				pServerDE->CreateInterObjectLink( m_hObject, hObj );
			}
		}
		break;
#ifdef _ADDON
		// Scoring a goal in soccer games
		case MID_GOAL:
		{
			DBYTE nTeamID;		

			nTeamID = pServerDE->ReadFromMessageByte( hRead );
			SoccerGoal( nTeamID );
		}
		break;
#endif // _ADDON
	}

	return CBaseCharacter::ObjectMessageFn(hSender, messageID, hRead);
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ProcessTriggerMsg()
//
//	PURPOSE:	Process a trigger message messages.
//
// --------------------------------------------------------------------------- //

DBOOL CPlayerObj::ProcessTriggerMsg(HOBJECT hSender, HMESSAGEREAD hRead)
{
	DBOOL bRet = DFALSE;
	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
	char *pMsg = g_pServerDE->GetStringData(hMsg);

	if (!pMsg)
	{
		g_pServerDE->FreeString( hMsg );
		return DFALSE;
	}

	char* pMsgType = strtok(pMsg, " ");
	if (pMsgType)
	{
		if (_mbsicmp((const unsigned char*)"Music", (const unsigned char*)pMsgType) == 0)
		{
			pMsg = strtok( NULL, "" );
			if( m_Music.HandleMusicMessage( pMsg ))
				bRet = DTRUE;
		}
		// Link to a named object
		else if (_mbsicmp((const unsigned char*)"sound", (const unsigned char*)pMsgType) == 0)
		{ 
			if (!g_pBloodServerShell->IsMultiplayerGame())
			{
				pMsg = strtok( NULL, "" );
				if (pMsg)
				{
					SendMessageToClient(SMSG_VOICEMGR_STOPALL);
					PlaySoundLocal(pMsg, SOUNDPRIORITY_PLAYER_HIGH);
				}
			}
		}
		else if (_mbsicmp((const unsigned char*)"fadein", (const unsigned char*)pMsgType) == 0)
		{
			SendFadeMsg(1);
		}
		else if (_mbsicmp((const unsigned char*)"fadeout", (const unsigned char*)pMsgType) == 0)
		{
			SendFadeMsg(0);
		}
		else if (_mbsicmp((const unsigned char*)"flaggrab", (const unsigned char*)pMsgType) == 0)
		{
			GrabFlag(hSender);
		}
		else if (_mbsicmp((const unsigned char*)"flaggive", (const unsigned char*)pMsgType) == 0)
		{
			GiveFlag(hSender);
		}
		else if (_mbsicmp((const unsigned char*)"flagdrop", (const unsigned char*)pMsgType) == 0)
		{
			DropFlag();
		}
	}

	g_pServerDE->FreeString(hMsg);

	return DFALSE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GrabObject()
//
//	PURPOSE:	Trys to grab an object and drag it.
//
// --------------------------------------------------------------------------- //

void CPlayerObj::GrabObject()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !(m_dwControlFlags & CTRLFLAG_GRAB)) return;

	// See if there is an object in front of me.
	IntersectQuery iq;
	IntersectInfo  ii;
	DVector	vDir;
	DVector vMyPos;

	pServerDE->GetObjectPos(m_hObject, &vMyPos);
	VEC_COPY(iq.m_From, vMyPos);
	iq.m_From.y += m_fEyeLevel;

	VEC_MULSCALAR(vDir, m_vForward, DIST_GRAB);
	VEC_ADD(iq.m_To, iq.m_From, vDir);

	iq.m_Flags = INTERSECT_OBJECTS;
	iq.m_FilterFn = NULL;
	iq.m_pUserData = NULL;	

	if (g_pServerDE->IntersectSegment(&iq, &ii) && ii.m_hObject)
	{
		// If object is moveable.. try to move it by the amount I moved since the last update.
		if (pServerDE->GetObjectUserFlags(ii.m_hObject) & USRFLG_MOVEABLE)
		{
			DVector vDist;
			VEC_SUB(vDist, vMyPos, m_vLastPos);

			DVector vObjPos;
			pServerDE->GetObjectPos(ii.m_hObject, &vObjPos);

			VEC_ADD(vObjPos, vObjPos, vDist);

			pServerDE->MoveObject(ii.m_hObject, &vObjPos);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetAnimation
//
//	PURPOSE:	Sets the player's animation state
//
// ----------------------------------------------------------------------- //

DBOOL CPlayerObj::SetAnimation(DDWORD dwAni, DBOOL bLooping)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

    DBOOL bRet = DTRUE;

	DDWORD nOldAnim = pServerDE->GetModelAnimation(m_hObject);

    pServerDE->ResetModelAnimation(m_hObject);
	pServerDE->SetModelAnimation(m_hObject,dwAni);

	DVector vDims,vNewDims;
	pServerDE->GetModelAnimUserDims(m_hObject, &vDims, pServerDE->GetModelAnimation(m_hObject));
	VEC_COPY(vNewDims,vDims);

	vNewDims.x *= m_vScale.x;
	vNewDims.y *= m_vScale.y;
	vNewDims.z *= m_vScale.z;

    DDWORD nModelAnim = pServerDE->GetModelAnimation(m_hObject);
	
	if(pServerDE->SetObjectDims2(m_hObject,&vNewDims) == DE_OK)
	{
		DFLOAT fNewEyeLevel;

		if (m_bForcedCrouch)
			m_PStateChangeFlags |= PSTATE_CROUCH;
		m_bForcedCrouch = DFALSE;
		if (m_bLastCrouchCommand)
			fNewEyeLevel = gCharacterValues[m_nCharacter].fEyeLevelCrouching;
		else if (m_bDead)
			fNewEyeLevel = gCharacterValues[m_nCharacter].fEyeLevelDead;
		else
			fNewEyeLevel = gCharacterValues[m_nCharacter].fEyeLevelStanding;

		if (fNewEyeLevel != m_fEyeLevel)
		{
			m_fEyeLevel = fNewEyeLevel;
			// tell the client to adjust camera offset
			SendMessageToClient(SMSG_EYELEVEL);
		}
	}
	else
	{	
	    pServerDE->SetModelAnimation(m_hObject, nOldAnim);
		bRet = DFALSE;
	}

	if (bRet)
		g_pServerDE->SetModelLooping(m_hObject, bLooping);

	return bRet;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetForceUpdateList
//
//	PURPOSE:	Add all the objects that ALWAYS need to be kept around on 
//				the client
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetForceUpdateList(ForceUpdate* pFU)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (!pFU || !pFU->m_Objects) return;

	// Camera objects..

	DLink* pLink = CameraObj::m_CameraHead.m_pNext;
	if(pLink && (pLink != &CameraObj::m_CameraHead))
	{
		while(pLink != &CameraObj::m_CameraHead && pFU->m_nObjects < MAX_FORCEUPDATE_OBJECTS-1)
		{
			if (HOBJECT hObj = ((CameraObj*)pLink->m_pData)->m_hObject)
				pFU->m_Objects[pFU->m_nObjects++] = hObj;
			pLink = pLink->m_pNext;
		}
	}

	// The EYE
	if (pFU->m_nObjects < MAX_FORCEUPDATE_OBJECTS-1)
	{
		CInvTheEye* pEye = (CInvTheEye*)m_InventoryMgr.GetItem(INV_THEEYE);
		HOBJECT hObj;
		if (pEye && (hObj = pEye->GetEyeObject()))
			pFU->m_Objects[pFU->m_nObjects++] = hObj;
	}

	// The orb
	if (pFU->m_nObjects < MAX_FORCEUPDATE_OBJECTS-1)
	{
		if(m_hOrbObj)
			pFU->m_Objects[pFU->m_nObjects++] = m_hOrbObj;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateFragStatus
//
//	PURPOSE:	Updates the frag status of myself and whoever killed me.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateFragStatus()
{
	if (!g_pBloodServerShell || !g_pServerDE) return;

	HCLIENT hClientKiller;

	if(!m_damage.GetWhoKilledMeLast()) return;

	HOBJECT hKiller = m_damage.GetWhoKilledMeLast();

	// See if the player is still active
	if (hClientKiller = g_pBloodServerShell->FindClient(hKiller))
	{
		CPlayerObj *pKiller = (CPlayerObj*)g_pServerDE->HandleToObject(hKiller);
		HSTRING hstrKillerName;
		if (pKiller && (hstrKillerName = pKiller->GetPlayerName()) )
		{
			if (m_hstrWhoKilledMeLast) g_pServerDE->FreeString(m_hstrWhoKilledMeLast);
			m_hstrWhoKilledMeLast = g_pServerDE->CopyString(hstrKillerName);

			if (pKiller == this)
			{
				RemoveFrag();
			}
			else
			{
				pKiller->AddFrag(m_hstrPlayerName, this);
			}
		}
	}

	if (hClientKiller)
	{
		DDWORD nVictimID = g_pServerDE->GetClientID (GetClient());
		DDWORD nKillerID = hClientKiller ? g_pServerDE->GetClientID (hClientKiller) : (DDWORD)-1;

		HMESSAGEWRITE hWrite = g_pServerDE->StartMessage (DNULL, SMSG_PLAYER_FRAGGED);
		if( hWrite )
		{
			g_pServerDE->WriteToMessageDWord (hWrite, nVictimID);
			g_pServerDE->WriteToMessageDWord (hWrite, nKillerID);
			g_pServerDE->WriteToMessageWord (hWrite, g_pServerDE->IntRandom(0, 0xffff));
			g_pServerDE->EndMessage2(hWrite, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
		}
	}

	// Update server
	if (g_pBloodServerShell)
		g_pBloodServerShell->SetUpdateBlood2Serv();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AddFrag
//
//	PURPOSE:	Chalk up another one.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::AddFrag(HSTRING hstrPlayerName, CPlayerObj* pVictim)
{
	// Sanity checks...
	if (!g_pServerDE) return;

	// Update the team frag count if the killer/victim are on different teams...
	const NetGame* pNetGame = g_pBloodServerShell->GetNetGameInfo();

	if (g_pBloodServerShell->IsMultiplayerTeamBasedGame() && pVictim && pVictim != this)
	{
		if (pVictim->GetTeamID() != GetTeamID())	// different teams
		{
			if (g_pBloodServerShell->IsMultiplayerCtf() && pNetGame->m_bOnlyFlagScores)
			{

			}
			else if (g_pBloodServerShell->IsMultiplayerSoccer() && pNetGame->m_bOnlyGoalScores)
			{

			}
			else
			{
				m_Frags++;
				g_pBloodServerShell->GetTeamMgr()->AddPlayerFrags(g_pServerDE->GetClientID(GetClient()), +1);
			}
		}
		else	// same team
		{
			if (pNetGame->m_bFriendlyFire && pNetGame->m_bNegTeamFrags)
			{
				RemoveFrag();
			}
		}
	}
	else
	{
		// Increment the frag count...
		m_Frags++;
	}

	// Setup the string...
	if (hstrPlayerName)
	{
		if (m_hstrWhoIKilledLast) g_pServerDE->FreeString(m_hstrWhoIKilledLast);
		m_hstrWhoIKilledLast = g_pServerDE->CopyString(hstrPlayerName);	
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetFrags
//
//	PURPOSE:	Sets the player's frag count to the given value
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetFrags(int nFrags)
{
	m_Frags = nFrags;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::RemoveFrag
//
//	PURPOSE:	Removes one frag from this player's frag count
//
// ----------------------------------------------------------------------- //

void CPlayerObj::RemoveFrag()
{
	if (g_pBloodServerShell->IsMultiplayerTeamBasedGame())
	{
		m_Frags--;
		g_pBloodServerShell->GetTeamMgr()->AddPlayerFrags(g_pServerDE->GetClientID(GetClient()), -1);
	}
	else
	{
		m_Frags--;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::DoTaunt
//
//	PURPOSE:	Plays a random taunt sound.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoTaunt()
{
	// Sanity checks...

	if (!g_pBloodServerShell) return;


	// Play the taunt for all clients to hear if we're still alive...

	if (!m_bDead)
	{
		if (g_pBloodServerShell->IsMultiplayerGame())
		{
			g_pBloodServerShell->GetVoiceMgr()->PlayEventSound(this, VME_TAUNT);
		}
		else
		{
			PlayVoiceGroupEventOnClient(VME_TAUNT);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PlaySound
//
//	PURPOSE:	Plays a player sound
//
// ----------------------------------------------------------------------- //

DBOOL CPlayerObj::PlayPlayerSound(char* szSound, DFLOAT fRadius, DBYTE byVolume, DBOOL bStream, DBYTE nPriority)
{
	if (!g_pServerDE) return DFALSE;

	if (m_hCurSound) 
	{
		g_pServerDE->KillSound(m_hCurSound);
		m_hCurSound = DNULL;
	}


	byVolume = DCLAMP(byVolume, 0, 100);

	m_hCurSound = PlaySoundFromObject(m_hObject, szSound, fRadius, nPriority + SOUNDPRIORITYBASE_PLAYER, DFALSE, DTRUE, DFALSE, byVolume, bStream);
	
	// Alert any AIs
	DVector vPos;
	g_pServerDE->GetObjectPos(m_hObject, &vPos);

	SendSoundTrigger(m_hObject, SOUND_PLAYERSOUND, vPos, fRadius);

	return DTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::IdleTime()
//
//	PURPOSE:	Does stuff when the player is idle, plays sounds etc.
//
// --------------------------------------------------------------------------- //
void CPlayerObj::IdleTime()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

    if (!m_bAnimating || m_byMoveState != PMOVE_IDLE)
    {
		DBOOL bRet = SetAnimation(m_Anim_Sound.m_nAnim_IDLE[pServerDE->IntRandom(0,1)], DTRUE);

        m_bAnimating = DTRUE; 
		m_byMoveState = PMOVE_IDLE;
	}
	else
	{
		if(pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
            m_bAnimating = DFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
// ROUTINE		: CPlayerObj::DoWalkAnim
//
// DESCRIPTION	: Run the walk animation
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoWalkAnim()
{
    if (m_bAnimating == DFALSE || m_byMoveState != PMOVE_WALK)
    {
		DBOOL bRet = DFALSE;

		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();

		if(pW)
		{
			if(m_nInjuredLeg == NODE_LLEG)
			{
				if(pW->GetFireType() == TYPE_PISTOL || pW->GetFireType() == TYPE_MELEE)
					bRet = SetAnimation(m_Anim_Sound.m_nAnim_WALK_INJURED_LLEG_PISTOL, DTRUE);
				else
					bRet = SetAnimation(m_Anim_Sound.m_nAnim_WALK_INJURED_LLEG_RIFLE, DTRUE);
			}
			else if(m_nInjuredLeg == NODE_RLEG)
			{
				if(pW->GetFireType() == TYPE_PISTOL || pW->GetFireType() == TYPE_MELEE)
					bRet = SetAnimation(m_Anim_Sound.m_nAnim_WALK_INJURED_RLEG_PISTOL, DTRUE);
				else
					bRet = SetAnimation(m_Anim_Sound.m_nAnim_WALK_INJURED_RLEG_RIFLE, DTRUE);
			}
			else
				bRet = SetAnimation(m_Anim_Sound.m_nAnim_WALK[pW->GetFireType()], DTRUE);
		}
		else
			bRet = SetAnimation(m_Anim_Sound.m_nAnim_WALK[TYPE_MELEE], DTRUE);

        m_bAnimating = DTRUE; 
		m_byMoveState = PMOVE_WALK;
    }
    else
    {   
		if(g_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE; 
        }
    }
	
    return;
}


// ----------------------------------------------------------------------- //
//
// ROUTINE		: CPlayerObj::DoRunAnim
//
// DESCRIPTION	: Do the run animation
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoRunAnim()
{
    if (m_bAnimating == DFALSE || m_byMoveState != PMOVE_RUN)
    {
		DBOOL bRet = DFALSE;

	    CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();

		if(pW)
	        bRet = SetAnimation(m_Anim_Sound.m_nAnim_RUN[pW->GetFireType()], DTRUE);
		else
	        bRet = SetAnimation(m_Anim_Sound.m_nAnim_RUN[TYPE_MELEE], DTRUE);

        m_bAnimating = DTRUE; 
		m_byMoveState = PMOVE_RUN;
    }
    else
    {   
		//Are we done running?
		if(g_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE; 
        }
    }
}


// ----------------------------------------------------------------------- //
//
// ROUTINE		: CPlayerObj::DoJumpAnim
//
// DESCRIPTION	: Do the jumping animation
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoJumpAnim()
{
    if (m_bAnimating == DFALSE || m_byMoveState != PMOVE_JUMP)
    {
	    CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();

		if(pW)
	        SetAnimation(m_Anim_Sound.m_nAnim_JUMP[pW->GetFireType()], DFALSE);
		else
			SetAnimation(m_Anim_Sound.m_nAnim_JUMP[TYPE_PISTOL], DFALSE);

		m_byMoveState = PMOVE_JUMP;
        m_bAnimating = DTRUE; 
    }
}



// ----------------------------------------------------------------------- //
//
// ROUTINE		: CPlayerObj::DoCrouchAnim
//
// DESCRIPTION	: Run the crouch animation
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoCrouchAnim()
{
    if (m_bAnimating == DFALSE || m_byMoveState != PMOVE_CROUCH)
    {
		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();

		if(pW)
			SetAnimation(m_Anim_Sound.m_nAnim_CROUCH[pW->GetFireType()], DFALSE);
		else
			SetAnimation(m_Anim_Sound.m_nAnim_CROUCH[TYPE_PISTOL], DFALSE);

		m_bAnimating = DTRUE;
		m_byMoveState = PMOVE_CROUCH;
	}
	else
    {   
		if(g_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE; 
        }
    }
}


// ----------------------------------------------------------------------- //
//
// ROUTINE		: CPlayerObj::DoCrawlAnim
//
// DESCRIPTION	: Run the crawl animation
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoCrawlAnim()
{
    if (m_bAnimating == DFALSE || m_byMoveState != PMOVE_CRAWL)
    {
	    CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();

		if(pW)
	        SetAnimation(m_Anim_Sound.m_nAnim_CRAWL[pW->GetFireType()], DTRUE);
		else
	        SetAnimation(m_Anim_Sound.m_nAnim_CRAWL[TYPE_PISTOL], DTRUE);

        m_bAnimating = DTRUE; 
		m_byMoveState = PMOVE_CRAWL;
    }
    else
    {   
		if(g_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE; 
        }
    }
}


/*
// ----------------------------------------------------------------------- //
// ROUTINE		: CPlayerObj::Swim
// DESCRIPTION	: Run the swim animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CPlayerObj::PMOVE_Swim()
{
    if (m_bAnimating == DFALSE || m_byMoveState != PMOVE_SWIM)
    {
	    CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();

    	m_fTimeStart = g_pServerDE->GetTime();

		if(pW)
	        SetAnimation(m_Anim_Sound.m_nAnim_SWIM[pW->GetFireType()], DTRUE);
		else
	        SetAnimation(m_Anim_Sound.m_nAnim_SWIM[TYPE_PISTOL], DTRUE);

		Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
        
        m_bAnimating = DTRUE; 
		m_byMoveState = PMOVE_SWIM;
    }
    else
    {   
		//Check for obstruction; otherwise continue on
		if(CheckObstructed(m_MoveObj.GetForwardVector(), m_fWalkSpeed))
		{
			if(!AIShared.TurnToClear(m_hObject))
			{
				m_bAnimating = DFALSE; 
				Metacmd++;
			}
		}

		Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
    
		//Are we done walking?
		if(g_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE; 
            Metacmd++;
        }
    }
	
    return;
}
*/

// ----------------------------------------------------------------------- //
//
// ROUTINE		: CPlayerObj::DoFireStandAnim
//
// DESCRIPTION	: Run the fire_stand animation
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoFireStandAnim()
{
	DFLOAT fTime = g_pServerDE->GetTime();

    if (m_bAnimating == DFALSE || m_byMoveState != PMOVE_FIRE_STAND)
    {
		DBOOL bRet = DFALSE;

		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();		
		
		m_fAttackLoadTime = pW ? pW->GetReloadTime() : 0.0f;

    	m_fLoadTimeStart = fTime;

		if(pW)
	        bRet = SetAnimation(m_Anim_Sound.m_nAnim_FIRE_STAND[pW->GetFireType()], DFALSE);
		else
	        bRet = SetAnimation(m_Anim_Sound.m_nAnim_FIRE_STAND[TYPE_MELEE], DFALSE);

		if(bRet)
		{
			m_bAnimating = DTRUE; 
			m_byMoveState = PMOVE_FIRE_STAND;
		}
    }
    else
    {   //Have we reloaded yet?             

        if (fTime > (m_fLoadTimeStart + m_fAttackLoadTime))
        {
			m_fLoadTimeStart = fTime;
			m_bAnimating = DFALSE;
        }
        
		if(g_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
		{
			m_bAnimating = DFALSE;
		}
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
//
// ROUTINE		: CPlayerObj::DoFireWalkAnim
//
// DESCRIPTION	: Run the fire_walk animation
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoFireWalkAnim()
{
    if (m_bAnimating == DFALSE || m_byMoveState != PMOVE_FIRE_WALK)
    {
		DBOOL bRet = DFALSE;

		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();

		if(pW)
	        bRet = SetAnimation(m_Anim_Sound.m_nAnim_FIRE_WALK[pW->GetFireType()], DTRUE);
		else
	        bRet = SetAnimation(m_Anim_Sound.m_nAnim_FIRE_WALK[TYPE_PISTOL], DTRUE);

		if(bRet)
		{
	        m_bAnimating = DTRUE; 
			m_byMoveState = PMOVE_FIRE_WALK;
		}
    }
    else
    {             
		//Are we done walking?
		if(g_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE; 
        }
    }      
}

// ----------------------------------------------------------------------- //
//
// ROUTINE		: CPlayerObj::DoFireRunAnim
//
// DESCRIPTION	: Run the fire_run animation
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoFireRunAnim()
{
    if (m_bAnimating == DFALSE || m_byMoveState != PMOVE_FIRE_RUN)
    {
		DBOOL bRet = DFALSE;

		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();
		if(pW)
			bRet = SetAnimation(m_Anim_Sound.m_nAnim_FIRE_RUN[pW->GetFireType()], DTRUE);
		else
	        bRet = SetAnimation(m_Anim_Sound.m_nAnim_FIRE_RUN[TYPE_PISTOL], DTRUE);

		if(bRet)
		{
			m_bAnimating = DTRUE; 
			m_byMoveState = PMOVE_FIRE_RUN;
		}
    }
    else
    {             
		//Are we done walking?
		if(g_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE; 
        }
    }      
}


// ----------------------------------------------------------------------- //
//
// ROUTINE		: CPlayerObj::DoFireJumpAnim
//
// DESCRIPTION	: Run the fire_jump animation
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoFireJumpAnim()
{
	DFLOAT fTime = g_pServerDE->GetTime();

    if (m_bAnimating == DFALSE || m_byMoveState != PMOVE_FIRE_JUMP)
    {
		DBOOL bRet = DFALSE;

	    CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();

		if(pW)
			bRet = SetAnimation(m_Anim_Sound.m_nAnim_FIRE_JUMP[pW->GetFireType()], DFALSE);
		else
	        bRet = SetAnimation(m_Anim_Sound.m_nAnim_FIRE_RUN[TYPE_PISTOL], DFALSE);

		if(bRet)
		{
			m_bAnimating = DTRUE; 
			m_byMoveState = PMOVE_FIRE_RUN;
		}
    }
}

// ----------------------------------------------------------------------- //
//
// ROUTINE		: CPlayerObj::DoFireCrouchAnim
//
// DESCRIPTION	: Run the fire_crouch animation
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoFireCrouchAnim()
{
	DFLOAT fTime = g_pServerDE->GetTime();
    if (m_bAnimating == DFALSE || m_byMoveState != PMOVE_FIRE_CROUCH)
    {
		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();		
		m_fAttackLoadTime = pW ? pW->GetReloadTime() : 0;

    	m_fLoadTimeStart = fTime;
   
		if(pW)
	        SetAnimation(m_Anim_Sound.m_nAnim_FIRE_CROUCH[pW->GetFireType()], DFALSE);
		else
	        SetAnimation(m_Anim_Sound.m_nAnim_FIRE_CROUCH[TYPE_PISTOL], DFALSE);

		m_bAnimating = DTRUE; 
		m_byMoveState = PMOVE_FIRE_CROUCH;
    }
    else
    {   //Have we reloaded yet?             
        if (fTime > (m_fLoadTimeStart + m_fAttackLoadTime))
        {
			m_fLoadTimeStart = fTime;
        }
        
		//Are we done walking?
		if(g_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE;
        }
	}      
}

// ----------------------------------------------------------------------- //
//
// ROUTINE		: CPlayerObj::DoFireCrawlAnim
//
// DESCRIPTION	: Run fire_crawl animation
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoFireCrawlAnim()
{
    if(m_bAnimating == DFALSE || m_byMoveState != PMOVE_FIRE_CRAWL)
    {
		DBOOL bRet = DFALSE;

		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();
		if(pW)
	        bRet = SetAnimation(m_Anim_Sound.m_nAnim_FIRE_CRAWL[pW->GetFireType()], DTRUE);
		else
	        bRet = SetAnimation(m_Anim_Sound.m_nAnim_FIRE_CRAWL[TYPE_PISTOL], DTRUE);

		if (bRet)
		{
			m_bAnimating = DTRUE; 
			m_byMoveState = PMOVE_FIRE_CRAWL;
		}
    }
    else
    {             
		//Are we done walking?
		if(g_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE; 
        }
    }      
}


// ----------------------------------------------------------------------- //
//
// ROUTINE		: CPlayerObj::DoStrafeRightAnim()
//
// DESCRIPTION	: run the strafing anim
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoStrafeRightAnim()
{
    if (m_bAnimating == DFALSE || m_byMoveState != PMOVE_STRAFERIGHT)
    {
		DBOOL bRet = DFALSE;

	    CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();

		if(pW)
	        bRet = SetAnimation(m_Anim_Sound.m_nAnim_STRAFERIGHT[pW->GetFireType()], DTRUE);
		else
	        bRet = SetAnimation(m_Anim_Sound.m_nAnim_STRAFERIGHT[TYPE_MELEE], DTRUE);

        m_bAnimating = DTRUE; 
		m_byMoveState = PMOVE_STRAFERIGHT;
    }
    else
    {   
		//Are we done running?
		if(g_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE; 
        }
    }
}


// ----------------------------------------------------------------------- //
//
// ROUTINE		: CPlayerObj::DoStrafeLeftAnim()
//
// DESCRIPTION	: run the strafing anim
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoStrafeLeftAnim()
{
    if (m_bAnimating == DFALSE || m_byMoveState != PMOVE_STRAFELEFT)
    {
		DBOOL bRet = DFALSE;

	    CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();

		if(pW)
	        bRet = SetAnimation(m_Anim_Sound.m_nAnim_STRAFELEFT[pW->GetFireType()], DTRUE);
		else
	        bRet = SetAnimation(m_Anim_Sound.m_nAnim_STRAFELEFT[TYPE_MELEE], DTRUE);

        m_bAnimating = DTRUE; 
		m_byMoveState = PMOVE_STRAFELEFT;
    }
    else
    {   
		//Are we done running?
		if(g_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE; 
        }
    }
}


// ----------------------------------------------------------------------- //
//
// ROUTINE		: CPlayerObj::DoHumiliationAnim()
//
// DESCRIPTION	: run a humiliation animation
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoHumiliationAnim()
{
	if (m_bAnimating && m_byMoveState == PMOVE_HUMILIATION)
	{
		return;
	}

	if (m_bAnimating == DFALSE || m_byMoveState != PMOVE_HUMILIATION)
	{
		SetAnimation(m_Anim_Sound.m_nAnim_HUMILIATION[GetRandom(0, 2)], DTRUE);
		
		m_bAnimating = DTRUE; 
		m_byMoveState = PMOVE_HUMILIATION;
	}
	else
	{   
		//Are we done running?
		if(g_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
		{
			m_bAnimating = DFALSE; 
		}
	}
}


/*
// ----------------------------------------------------------------------- //
// ROUTINE		: CPlayerObj::PMOVE_Taunt_Beg
// DESCRIPTION	: play the taunt_beg animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CPlayerObj::PMOVE_Taunt_Beg()
{
    if (m_bAnimating == DFALSE || m_byMoveState != PMOVE_TAUNT_BEG)
    {
        SetAnimation(m_Anim_Sound.m_nAnim_TAUNT[6], DFALSE);

		Move(m_MoveObj.GetRightVector(),m_fRollSpeed);

		m_bAnimating = DTRUE;
		m_byMoveState = PMOVE_TAUNT_BEG;
	}
	else
	{
		//Are we done?
		if(g_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE;
            Metacmd++;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CPlayerObj::PMOVE_Taunt_Bold
// DESCRIPTION	: Play a random taunt
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CPlayerObj::PMOVE_Taunt_Bold()
{
    if (m_bAnimating == DFALSE || m_byMoveState != PMOVE_TAUNT_BOLD)
    {
        SetAnimation(m_Anim_Sound.m_nAnim_TAUNT[4], DFALSE);

		Move(m_MoveObj.GetRightVector(),m_fRollSpeed);

		m_bAnimating = DTRUE;
		m_byMoveState = PMOVE_TAUNT_BOLD;
	}
	else
	{
		//Are we done?
		if(g_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE;
            Metacmd++;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CPlayerObj::PMOVE_Recoil
// DESCRIPTION	: Run recoil animation based on node hit
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CPlayerObj::PMOVE_Recoil()
{
    if (m_bAnimating == DFALSE || m_byMoveState != PMOVE_RECOIL)
    {
		switch(m_nNodeHit)
		{
			case NODE_NECK:		SetAnimation(m_Anim_Sound.m_nAnim_RECOIL[0 + m_nSideHit], DFALSE);
								break;
			case NODE_TORSO:	SetAnimation(m_Anim_Sound.m_nAnim_RECOIL[1 + m_nSideHit], DFALSE);
								break;
			case NODE_RARM:		SetAnimation(m_Anim_Sound.m_nAnim_RECOIL[2 + m_nSideHit], DFALSE);
								break;
			case NODE_LARM:		SetAnimation(m_Anim_Sound.m_nAnim_RECOIL[3 + m_nSideHit], DFALSE);
								break;
			case NODE_LLEG:		SetAnimation(m_Anim_Sound.m_nAnim_RECOIL[4 + m_nSideHit], DFALSE);
								break;
			case NODE_RLEG:		SetAnimation(m_Anim_Sound.m_nAnim_RECOIL[5 + m_nSideHit], DFALSE);
								break;
		}

		char szTemp[256];
		_mbscpy((unsigned char*)szTemp, (const unsigned char*)SOUND_PAIN);
		m_Anim_Sound.GetSoundPath(szTemp,g_pServerDE->IntRandom(1,3));
		PlayAISound(szTemp, 1000.0f);					

		m_bAnimating = DTRUE;
		m_byMoveState = PMOVE_RECOIL;
	}
	else
	{
		//Are we done?
		if(g_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE;
            Metacmd++;
        }
	}

	return;
}
*/

// ----------------------------------------------------------------------- //
//
// ROUTINE		: CPlayerObj::CreateCorpse
//
// DESCRIPTION	: Convert the player into a corpse object
//
// ----------------------------------------------------------------------- //

DBOOL CPlayerObj::CreateCorpse()
{
	HCLASS hClass = g_pServerDE->GetClass( "CCorpse" );
	if( !hClass )
		return DFALSE;

	DVector vAI_Dims,vDims,vPos;
	DBOOL bStatus = DFALSE;

	g_pServerDE->GetObjectPos(m_hObject, &vPos);
	g_pServerDE->GetObjectDims(m_hObject, &vAI_Dims);

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	g_pServerDE->GetObjectPos(m_hObject, &theStruct.m_Pos);
	g_pServerDE->GetObjectRotation(m_hObject, &theStruct.m_Rotation);

	g_pServerDE->GetModelFilenames(m_hObject,theStruct.m_Filename,MAX_CS_FILENAME_LEN+1,
									         theStruct.m_SkinName,MAX_CS_FILENAME_LEN+1);

	// Allocate an object...
	BaseClass* pObj = g_pServerDE->CreateObject( hClass, &theStruct );
	if( !pObj )	return DFALSE;

	//Set the animation to the corpse animation
	g_pServerDE->SetModelAnimation(pObj->m_hObject,m_nCorpseType);
    g_pServerDE->SetModelLooping(pObj->m_hObject, DFALSE);

	//Get the corpses dims
	g_pServerDE->GetModelAnimUserDims(pObj->m_hObject, &vDims, g_pServerDE->GetModelAnimation(pObj->m_hObject));

	vPos.y -= (vAI_Dims.y - vDims.y) - 0.001f;

	//Put the corpse on the floor
	g_pServerDE->ScaleObject(pObj->m_hObject,&m_vScale);
	g_pServerDE->SetObjectDims(pObj->m_hObject,&vDims);
	g_pServerDE->SetObjectPos(pObj->m_hObject,&vPos);

	//HEAD/NECK
	g_pServerDE->GetModelNodeHideStatus(m_hObject,szNodes[NODE_NECK],&bStatus);
	if(bStatus)
	{
		AIShared.HideLimb(pObj->m_hObject, NODE_NECK);
	}

	//RIGHT ARM
	g_pServerDE->GetModelNodeHideStatus(m_hObject,szNodes[NODE_RARM],&bStatus);
	if(bStatus)
	{
		AIShared.HideLimb(pObj->m_hObject, NODE_RARM);
	}

	//LEFT ARM
	g_pServerDE->GetModelNodeHideStatus(m_hObject,szNodes[NODE_LARM],&bStatus);
	if(bStatus)	
	{
		AIShared.HideLimb(pObj->m_hObject, NODE_LARM);
	}

	//LEFT LEG
	g_pServerDE->GetModelNodeHideStatus(m_hObject,szNodes[NODE_LLEG],&bStatus);
	if(bStatus)	
	{
		AIShared.HideLimb(pObj->m_hObject, NODE_LLEG);
	}

	//RIGHT LEG
	g_pServerDE->GetModelNodeHideStatus(m_hObject,szNodes[NODE_RLEG],&bStatus);
	if(bStatus)	
	{
		AIShared.HideLimb(pObj->m_hObject, NODE_RLEG);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
// ROUTINE		: CPlayerObj::CreateGibs
// DESCRIPTION	: Gib-o-rama
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

DBOOL CPlayerObj::CreateGibs(DVector vDir, int nNumGibs, int nType, DFLOAT fDamage)
{
	if (!g_pServerDE) return DFALSE;

//	HCLASS hClass = g_pServerDE->GetClass( "CClientGibFX" );
//	if( !hClass )
//		return DFALSE;

	VEC_NEGATE(vDir, vDir);

	DFLOAT fVelFactor = 1.0f;

	DFLOAT fVel = 50.0f + fDamage;

	vDir.y -= 1.0f;
	VEC_NORM(vDir);

	VEC_MULSCALAR(vDir, vDir, fVel);

//	ObjectCreateStruct ocStruct;
//	INIT_OBJECTCREATESTRUCT(ocStruct);
	DVector vPos;
	g_pServerDE->GetObjectPos(m_hObject, &vPos);

//	VEC_COPY(ocStruct.m_Pos, vPos);
//	g_pServerDE->GetObjectRotation(m_hObject, &ocStruct.m_Rotation);

//	CClientGibFX* pGib = (CClientGibFX*)g_pServerDE->CreateObject(hClass, &ocStruct);

	DVector vDims;
	g_pServerDE->GetObjectDims(m_hObject, &vDims);

//	pGib->Setup(&vPos, &vDir, &vDims, (SURFTYPE_FLESH/10) | SIZE_SMALL | TRAIL_BLOOD, 1.0f, nNumGibs);
	SetupClientGibFX(&vPos, &vDir, &vDims, (SURFTYPE_FLESH/10) | SIZE_SMALL | TRAIL_BLOOD, 1.0f, nNumGibs);


	// Create body parts

	int rand = g_pServerDE->IntRandom(0, 100);
	// 80 % create a head..
	if (rand = g_pServerDE->IntRandom(0, 100) > 80)
		AIShared.CreateLimb(m_hObject, NODE_NECK, vDir);

	// 70% create 1 arm
	if (rand = g_pServerDE->IntRandom(0, 100) > 70)
		AIShared.CreateLimb(m_hObject, NODE_RARM, vDir);
	
	// 50% create 2 arms
	if (rand = g_pServerDE->IntRandom(0, 100) > 50)
		AIShared.CreateLimb(m_hObject, NODE_LARM, vDir);
	
	// 70% create 1 leg
	if (rand = g_pServerDE->IntRandom(0, 100) > 70)
		AIShared.CreateLimb(m_hObject, NODE_RLEG, vDir);	
	
	// 50% create 2 legs
	if (rand = g_pServerDE->IntRandom(0, 100) > 50)
		AIShared.CreateLimb(m_hObject, NODE_LLEG, vDir);

	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
// ROUTINE		: CPlayerObj::GetFootstepSound
//
// DESCRIPTION	: Determine the proper footstep sound to play
//
// ----------------------------------------------------------------------- //

DBOOL CPlayerObj::GetFootstepSound(char* szBuf)
{
	if (!szBuf || !g_pServerDE) return DFALSE;

	SurfaceType eType = m_eLastSurface;

	// Check to see if I'm standing in water..
	DVector vDims, vPos;
	g_pServerDE->GetObjectPos(m_hObject, &vPos);
	g_pServerDE->GetObjectDims(m_hObject, &vDims);
	vPos.y -= (vDims.y - 1.0f);

	HLOCALOBJ objList[1];
	if (g_pServerDE->GetPointContainers(&vPos, objList, 1))
	{
		D_WORD wCode;
		g_pServerDE->GetContainerCode(objList[0], &wCode);
		if (IsLiquid(( ContainerCode )wCode))
			eType = SURFTYPE_LIQUID;
	}


	// Footstep sound format example:
	// "sounds/player/footsteps/wood/woodleft1m.wav"

	_mbscpy((unsigned char*)szBuf, (const unsigned char*)"sounds/player/footsteps/");

	// Get the sound based on the last footstep..
	switch (eType)
	{
		case SURFTYPE_WOOD:		_mbscat((unsigned char*)szBuf, (const unsigned char*)"wood"); break;
		case SURFTYPE_FLESH:	_mbscat((unsigned char*)szBuf, (const unsigned char*)"flesh"); break;
		case SURFTYPE_GLASS:	_mbscat((unsigned char*)szBuf, (const unsigned char*)"glass"); break;
		case SURFTYPE_METAL:	_mbscat((unsigned char*)szBuf, (const unsigned char*)"metal"); break;
		case SURFTYPE_PLASTIC:	_mbscat((unsigned char*)szBuf, (const unsigned char*)"plastic"); break;
		case SURFTYPE_STONE:	_mbscat((unsigned char*)szBuf, (const unsigned char*)"stone"); break;
		case SURFTYPE_TERRAIN:	_mbscat((unsigned char*)szBuf, (const unsigned char*)"terrain"); break;
		case SURFTYPE_LIQUID:	_mbscat((unsigned char*)szBuf, (const unsigned char*)"water"); break;
		case SURFTYPE_CLOTH:	_mbscat((unsigned char*)szBuf, (const unsigned char*)"carpet"); break;
		default: return DFALSE;
	}

	// Handle left or right foot..
	_mbscat((unsigned char*)szBuf, m_bNextFootstepLeft ? (const unsigned char*)"/left" : (const unsigned char*)"/right");

	m_bNextFootstepLeft = !m_bNextFootstepLeft;

	// and the gender..
	_mbscat((unsigned char*)szBuf, IsFemale() ? (const unsigned char*)"f.wav" : (const unsigned char*)"m.wav");

	return DTRUE;
}

	
// ----------------------------------------------------------------------- //
//
// ROUTINE		: CPlayerObj::GetLandingSound
//
// DESCRIPTION	: Sound to play when landing after a fall/jump
//
// ----------------------------------------------------------------------- //

DBOOL CPlayerObj::GetLandingSound(char* szBuf)
{
	if (!szBuf || !g_pServerDE) return DFALSE;

	SurfaceType eType = m_eLastSurface;

	// Check to see if I'm standing in water..
	DVector vDims, vPos;
	g_pServerDE->GetObjectPos(m_hObject, &vPos);
	g_pServerDE->GetObjectDims(m_hObject, &vDims);
	vPos.y -= (vDims.y - 1.0f);

	HLOCALOBJ objList[1];
	if (g_pServerDE->GetPointContainers(&vPos, objList, 1))
	{
		D_WORD wCode;
		g_pServerDE->GetContainerCode(objList[0], &wCode);
		if (IsLiquid(( ContainerCode )wCode))
			eType = SURFTYPE_LIQUID;
	}


	// Footstep sound format example:
	// "sounds/player/footsteps/wood/woodleft1m.wav"

	_mbscpy((unsigned char*)szBuf, (const unsigned char*)"sounds/player/footsteps/");

	// Get the sound based on the last footstep..
	switch (eType)
	{
		case SURFTYPE_WOOD:		_mbscat((unsigned char*)szBuf, (const unsigned char*)"wood"); break;
		case SURFTYPE_FLESH:	_mbscat((unsigned char*)szBuf, (const unsigned char*)"flesh"); break;
		case SURFTYPE_GLASS:	_mbscat((unsigned char*)szBuf, (const unsigned char*)"glass"); break;
		case SURFTYPE_METAL:	_mbscat((unsigned char*)szBuf, (const unsigned char*)"metal"); break;
		case SURFTYPE_PLASTIC:	_mbscat((unsigned char*)szBuf, (const unsigned char*)"plastic"); break;
		case SURFTYPE_STONE:	_mbscat((unsigned char*)szBuf, (const unsigned char*)"stone"); break;
		case SURFTYPE_TERRAIN:	_mbscat((unsigned char*)szBuf, (const unsigned char*)"terrain"); break;
		case SURFTYPE_LIQUID:	_mbscat((unsigned char*)szBuf, (const unsigned char*)"water"); break;
		case SURFTYPE_CLOTH:	_mbscat((unsigned char*)szBuf, (const unsigned char*)"carpet"); break;
		default: return DFALSE;
	}

	_mbscat((unsigned char*)szBuf, (const unsigned char*)"/land");

	// and the gender..
	_mbscat((unsigned char*)szBuf, IsFemale() ? (const unsigned char*)"f.wav" : (const unsigned char*)"m.wav");

	return DTRUE;
}

	
// ----------------------------------------------------------------------- //
//
// ROUTINE		: CPlayerObj::PlayJumpSound
//
// DESCRIPTION	: Plays a sound when the player jumps
//
// ----------------------------------------------------------------------- //

void CPlayerObj::PlayJumpSound()
{
	if (!g_pServerDE) return;

}

	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetClientSaveData()
//
//	PURPOSE:	Sets the client save data for the next save game...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetClientSaveData( HMESSAGEREAD hClientSaveData )
{
	// Get rid of any old data...
	if( m_hClientSaveData )
	{
		g_pServerDE->EndHMessageRead( m_hClientSaveData );
	}
	m_hClientSaveData = hClientSaveData;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateAirLevel()
//
//	PURPOSE:	Update our air usage
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateAirLevel()
{
	ContainerCode eCode;
	D_WORD wCode;
	HLOCALOBJ objList[1];
	DDWORD dwNum;
	DVector vPos;

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DFLOAT fDeltaTime = pServerDE->GetFrameTime();

	if( m_bExternalCamera )
	{
		pServerDE->GetObjectPos( m_hObject, &vPos );
		dwNum = pServerDE->GetPointContainers( &vPos, objList, 1 );

		if( dwNum > 0 )
		{
			pServerDE->GetContainerCode( objList[0], &wCode );
			eCode = ( ContainerCode )wCode;
		}
		else
			eCode = CC_NOTHING;
	}
	else
		eCode = m_eContainerCode;

	// See if we are in a liquid...

	if ( eCode != CC_FREEFALL && IsLiquid(eCode))
	{
		DFLOAT fDeltaAirLoss = (MAX_AIR_LEVEL/FULL_AIR_LOSS_TIME);

		m_fAirLevel -= fDeltaTime*fDeltaAirLoss;

		if (m_fAirLevel < 0.0f)
		{
			m_fAirLevel = 0.0f;

			// Send damage message...(5 pts/sec)...

			DFLOAT fDamage = 5.0f*fDeltaTime;

			DVector vDir, vPos;
			VEC_INIT(vDir);

			pServerDE->GetObjectPos(m_hObject, &vPos);

			HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, m_hObject, MID_DAMAGE);
			pServerDE->WriteToMessageVector(hMessage, &vDir);
			pServerDE->WriteToMessageFloat(hMessage, fDamage);
			pServerDE->WriteToMessageByte(hMessage, DAMAGE_TYPE_SUFFOCATE);
			pServerDE->WriteToMessageObject(hMessage, m_hObject);
			pServerDE->WriteToMessageVector(hMessage, &vPos);
			pServerDE->EndMessage(hMessage);
		}	
	}
	else if (m_fAirLevel < MAX_AIR_LEVEL)
	{
		DFLOAT fDeltaAirRegen = (MAX_AIR_LEVEL/FULL_AIR_REGEN_TIME);
		m_fAirLevel += fDeltaTime*fDeltaAirRegen;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleDeath()
//
//	PURPOSE:	Damn, I just died..
//
// ----------------------------------------------------------------------- //
void CPlayerObj::HandleDeath()
{
	// Sanity checks
	if (!g_pBloodServerShell || !g_pServerDE) return;

	// Drop the flag if we're carrying it
	DropFlag();

	if(!m_damage.GetWhoKilledMeLast()) return;

	// MULTIPLAYER STUFF : See if we should set the death timer
	HOBJECT hKiller = m_damage.GetWhoKilledMeLast();
	if (g_pBloodServerShell->IsMultiplayerGame() && m_fDeathTimer == 0 && hKiller != m_hObject)
	{
		// Set the death timer 25% of the time
		if (m_fDeathTimer == 0 && m_damage.GetDeathHitPoints() < -5 && IsRandomChance(10))
		{
			m_fDeathTimer         = WONKY_VISION_DEFAULT_TIME;
			m_fSlowDeathSafeTimer = WONKY_VISION_SAFE_TIME;
			m_fSlowDeathStayTimer = WONKY_VISION_STAY_TIME;
			m_bInSlowDeath        = DTRUE;
			SendWonkyVisionMsg(m_fDeathTimer, DTRUE);

			// Give the person who did this the VOICE
			if (HCLIENT hClientKiller = g_pBloodServerShell->FindClient(hKiller))
			{
				CPlayerObj *pKiller = (CPlayerObj*)g_pServerDE->HandleToObject(hKiller);
				pKiller->SendVoiceMessage(VOICE_FINISHHIM, g_pServerDE->IntRandom(0, 0xffff), pKiller->m_nCharacter, m_nCharacter);
			}
		}
		else
		{
			m_fDeathTimer = -1;
		}
	}
	else if (m_bInSlowDeath)
	{
		m_fDeathTimer -= g_pServerDE->GetFrameTime();
		if (m_fDeathTimer <= 0)	// Too late!
		{
			m_bInSlowDeath = DFALSE;
		}
	}
	else
	{
		if (m_bInSlowDeath && m_fSlowDeathSafeTimer > 0)
		{
			return;
		}

		DoDeath();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PlayerVoiceGroupEvent
//
//	PURPOSE:	Tells the client to have VoiceMgr play a random sound
//				from the given event group.
//
// ----------------------------------------------------------------------- //

DBOOL CPlayerObj::PlayVoiceGroupEventOnClient(int iGroupEvent, DBOOL bSinglePlayerOnly)
{
	// Don't play these sounds in multiplayer games if we're not supposed to...

	BOOL bMulti = DFALSE;

	if (g_pBloodServerShell->IsMultiplayerGame())
	{
		bMulti = DTRUE;

		if (bSinglePlayerOnly)
		{
			return(DFALSE);
		}
	}


	// Send the message to the client...

//	HMESSAGEWRITE hMsg = g_pServerDE->StartMessage(m_hClient, SMSG_VOICEMGR);
	HMESSAGEWRITE hMsg = StartMessageToClient(SMSG_VOICEMGR);
	if (!hMsg) return(DFALSE);

	g_pServerDE->WriteToMessageByte(hMsg, 1);				// bEvent
	g_pServerDE->WriteToMessageWord(hMsg, GetCharacter());	// iCharacter
	g_pServerDE->WriteToMessageWord(hMsg, iGroupEvent);		// iEvent

	if (bMulti)
	{
		g_pServerDE->EndMessage2(hMsg, MESSAGE_NAGGLE);
	}
	else
	{
		g_pServerDE->EndMessage(hMsg);
	}


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PlayerVoiceUniqueEvent
//
//	PURPOSE:	Tells the client to have VoiceMgr play the unique sound
//				for the given unique event.
//
// ----------------------------------------------------------------------- //

DBOOL CPlayerObj::PlayVoiceUniqueEventOnClient(int iUniqueEvent)
{
	// Don't play these sounds in multiplayer games...

	if (g_pBloodServerShell->IsMultiplayerGame()) return(DFALSE);


	// Send the message to the client...

//	HMESSAGEWRITE hMsg = g_pServerDE->StartMessage(m_hClient, SMSG_VOICEMGR);
	HMESSAGEWRITE hMsg = StartMessageToClient(SMSG_VOICEMGR);
	if (!hMsg) return(DFALSE);

	g_pServerDE->WriteToMessageByte(hMsg, 0);				// bEvent
	g_pServerDE->WriteToMessageWord(hMsg, GetCharacter());	// iCharacter
	g_pServerDE->WriteToMessageWord(hMsg, iUniqueEvent);	// iEvent

	g_pServerDE->EndMessage(hMsg);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PlayWeaponVoiceSound
//
//	PURPOSE:	Plays an appropriate sound when a weapon is picked up
//
// ----------------------------------------------------------------------- //

void CPlayerObj::PlayWeaponVoiceSoundOnClient(int nWeaponType)
{
	// Don't play these sounds in multiplayer games...

	if (g_pBloodServerShell->IsMultiplayerGame()) return;


	// Check if we need to play one of the unique sounds...

	int iUnique = -1;

	switch (nWeaponType)
	{
		case WEAP_HOWITZER:		iUnique = VMU_HOWITZER; break;
		case WEAP_TESLACANNON:	iUnique = VMU_TESLACANNON; break;
		case WEAP_SNIPERRIFLE:	iUnique = VMU_SNIPERRIFLE; break;
		case WEAP_VOODOO:		iUnique = VMU_VOODOODOLL; break;
		case WEAP_ASSAULTRIFLE: iUnique = VMU_ASSAULTRIFLE; break;
		case WEAP_NAPALMCANNON: iUnique = VMU_NAPALMLAUNCHER; break;
		case WEAP_SINGULARITY:	iUnique = VMU_SINGULARITY; break;
		case WEAP_SHOTGUN:		iUnique = VMU_SHOTGUN; break;
	}

	if (iUnique != -1)
	{
		PlayVoiceUniqueEventOnClient(iUnique);
	}


	// Maybe play a weapon pickup event sound...

	if (IsRandomChance(20))
	{
		PlayVoiceGroupEventOnClient(VME_WEAPON, DTRUE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetMultiplayerAmmo
//
//	PURPOSE:	Set the starting ammo for multiplayer.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetMultiplayerAmmo()
{
	if (!g_pBloodServerShell) return;

	int nAmmoLevel = g_pBloodServerShell->GetNetGameInfo( )->m_nAmmoLevel;

	// Clear the ammo for the next round
	for (int i = AMMO_BULLET; i <= AMMO_MAXAMMOTYPES; i++)
		m_InventoryMgr.SetAmmoCount(i, 0.0f);

	switch(nAmmoLevel)
	{
		case LEVEL_NONE:
		{
		}
		break;

		case LEVEL_HALF:
		{
			m_InventoryMgr.SetAmmoCount(AMMO_BULLET, 25);
			m_InventoryMgr.SetAmmoCount(AMMO_SHELL, 4);
		}
		break;

		default:
		case LEVEL_NORMAL:
		{
			m_InventoryMgr.SetAmmoCount(AMMO_BULLET, 50);
			m_InventoryMgr.SetAmmoCount(AMMO_SHELL, 8);
			m_InventoryMgr.SetAmmoCount(AMMO_BMG, 5);
			m_InventoryMgr.SetAmmoCount(AMMO_BATTERY, 20);
			m_InventoryMgr.SetAmmoCount(AMMO_DIEBUGDIE, 5);
			m_InventoryMgr.SetAmmoCount(AMMO_FLARE, 10);
			m_InventoryMgr.SetAmmoCount(AMMO_FUEL, 10);
		}
		break;

		case LEVEL_DOUBLE:
		{
			m_InventoryMgr.SetAmmoCount(AMMO_BULLET, 100);
			m_InventoryMgr.SetAmmoCount(AMMO_SHELL, 16);
			m_InventoryMgr.SetAmmoCount(AMMO_BMG, 10);
			m_InventoryMgr.SetAmmoCount(AMMO_BATTERY, 40);
			m_InventoryMgr.SetAmmoCount(AMMO_DIEBUGDIE, 10);
			m_InventoryMgr.SetAmmoCount(AMMO_FLARE, 20);
			m_InventoryMgr.SetAmmoCount(AMMO_FUEL, 20);
			m_InventoryMgr.SetAmmoCount(AMMO_HOWITZER, 5);
		}
		break;
		
		case LEVEL_INSANE:
		{
			m_InventoryMgr.SetInfiniteAmmo(DTRUE);
		}
		break;
	}
}


DBOOL IsObjectInList(HOBJECT *theList, DDWORD listSize, HOBJECT hTest)
{
	DDWORD i;
	for(i=0; i < listSize; i++)
	{
		if(theList[i] == hTest)
			return DTRUE;
	}
	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateClientPhysics
//
//	PURPOSE:	Update the client physics.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateClientPhysics()
{
	HMESSAGEWRITE hWrite;
	char fileName[256], skinName[256];
	DVector grav;
	ServerDE *pServerDE = GetServerDE();
	HOBJECT objContainers[40];
	DDWORD i, objContainerFlags[40], nContainers;
	D_WORD containerCode;
	DVector current;
	HCLASS hVolClass;
	VolumeBrush *pVolBrush;
	float fGravity, frigginCoeff;
	DBOOL bHidden;
	DVector tbPos;
	DVector vAddVelocity;

	if(!m_hClient || !m_hObject || !pServerDE)
		return;

	VEC_INIT( vAddVelocity );
	if (m_damage.IsAddedVelocity())
	{
		m_damage.GetAddedVelocity(&vAddVelocity);
		m_PStateChangeFlags |= PSTATE_ADDVELOCITY;
	}
	if( m_bDoorPush )
	{
		m_bDoorPush = DFALSE;
		VEC_ADD( vAddVelocity, vAddVelocity, m_vDoorPush );
		m_PStateChangeFlags |= PSTATE_ADDVELOCITY;
	}

	
	// Update leash length.
	ChangeSpeedsVar(m_fLeashLen, m_LeashLenTrack.GetFloat());

	// Update crouch bit
	if (m_bForcedCrouch)
		m_PStateChangeFlags |= PSTATE_CROUCH;


	// Did our container states change?
	nContainers = pServerDE->GetObjectContainers(m_hObject, objContainers, objContainerFlags,
		sizeof(objContainers)/sizeof(objContainers[0]));
	nContainers = DMIN(nContainers, MAX_TRACKED_CONTAINERS);
	if(nContainers != m_nCurContainers)
	{
		m_PStateChangeFlags |= PSTATE_CONTAINERTYPE;
	}
	else
	{
		// Did we enter a container?
		for(i=0; i < nContainers; i++)
		{
			if(!IsObjectInList(m_CurContainers, m_nCurContainers, objContainers[i]))
			{
				m_PStateChangeFlags |= PSTATE_CONTAINERTYPE;
				break;
			}
		}

		// Did we exit a container?
		if(!(m_PStateChangeFlags & PSTATE_CONTAINERTYPE))
		{
			for(i=0; i < m_nCurContainers; i++)
			{
				if(!IsObjectInList(objContainers, nContainers, m_CurContainers[i]))
				{
					m_PStateChangeFlags |= PSTATE_CONTAINERTYPE;
					break;
				}
			}
		}
	}
	

	if(!m_PStateChangeFlags)
		return;

//	hWrite = pServerDE->StartMessage(m_hClient, SMSG_PHYSICS_UPDATE);
	hWrite = StartMessageToClient(SMSG_PHYSICS_UPDATE);
	if(hWrite)
	{
		pServerDE->WriteToMessageWord(hWrite, (D_WORD)m_PStateChangeFlags);
		
		if(m_PStateChangeFlags & PSTATE_MODELFILENAMES)
		{
			pServerDE->GetModelFilenames(m_hObject, fileName, sizeof(fileName), skinName, sizeof(skinName));
			pServerDE->WriteToMessageString(hWrite, fileName);
			pServerDE->WriteToMessageString(hWrite, skinName);
		}

		if(m_PStateChangeFlags & PSTATE_ADDVELOCITY)
		{
			pServerDE->WriteToMessageVector(hWrite, &vAddVelocity);
		}

		if(m_PStateChangeFlags & PSTATE_GRAVITY)
		{
			pServerDE->GetGlobalForce(&grav);
			pServerDE->WriteToMessageVector(hWrite, &grav);
		}

		if(m_PStateChangeFlags & PSTATE_CONTAINERTYPE)
		{
			// Send the new container info.
			pServerDE->WriteToMessageByte(hWrite, (DBYTE)nContainers);
			for(i=0; i < nContainers; i++)
			{
				// Send container code.
				if(!pServerDE->GetContainerCode(objContainers[i], &containerCode))
					containerCode = CC_NOTHING;

				pServerDE->WriteToMessageByte(hWrite, (DBYTE)containerCode);
				
				// Send current and gravity.
				fGravity = 0.0f;
				current.Init();
				bHidden = DFALSE;
				hVolClass = pServerDE->GetClass("VolumeBrush");
				if(hVolClass)
				{
					if(pServerDE->IsKindOf(pServerDE->GetObjectClass(objContainers[i]), hVolClass))
					{
						pVolBrush = (VolumeBrush*)pServerDE->HandleToObject(objContainers[i]);
						if(pVolBrush)
						{
							current = pVolBrush->GetCurrent();
							fGravity = pVolBrush->GetGravity();
							bHidden = pVolBrush->GetHidden();
						}
					}
				}
				pServerDE->WriteToMessageVector(hWrite, &current);
				pServerDE->WriteToMessageFloat(hWrite, fGravity);
				pServerDE->WriteToMessageByte(hWrite, (DBYTE)bHidden);
			}
			
			// Remember what we sent last.
			memcpy(m_CurContainers, objContainers, sizeof(m_CurContainers[0])*nContainers);
			m_nCurContainers = nContainers;
		}		

		if(m_PStateChangeFlags & PSTATE_SPEEDS)
		{
			pServerDE->WriteToMessageFloat(hWrite, m_fMoveVel);
			pServerDE->WriteToMessageFloat(hWrite, m_fJumpVel);
			pServerDE->WriteToMessageFloat(hWrite, m_fSwimVel);

			// These are paired in here so RunSpeed lets you run as fast as you want.. the
			// client treats the first one as a 'max speed' multiplier and the second one 
			// as an acceleration multiplier.
			pServerDE->WriteToMessageFloat(hWrite, m_fMoveMultiplier);
			pServerDE->WriteToMessageFloat(hWrite, m_fMoveMultiplier);

			pServerDE->WriteToMessageFloat(hWrite, m_LeashLenTrack.GetFloat()); // Leash length (allows for half a second of lag)
			pServerDE->WriteToMessageFloat(hWrite, m_fBaseMoveAccel);
			pServerDE->WriteToMessageFloat(hWrite, m_fJumpMultiplier);
			pServerDE->WriteToMessageFloat(hWrite, m_fLadderVel);

			frigginCoeff = 0.0f;
			pServerDE->Physics()->GetFrictionCoefficient(m_hObject, frigginCoeff);
			pServerDE->WriteToMessageFloat(hWrite, frigginCoeff);
		}

		if(m_PStateChangeFlags & PSTATE_CROUCH)
		{
			pServerDE->WriteToMessageByte(hWrite, m_bForcedCrouch);
		}

		pServerDE->EndMessage2(hWrite, MESSAGE_GUARANTEED);
	}

	m_PStateChangeFlags = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandlePlayerPositionMessage
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
void CPlayerObj::HandlePlayerPositionMessage(HMESSAGEREAD hRead)
{
	DVector newPos, curPos, curVel;
	DBYTE moveCode, nMovementFlags;
	DBOOL bOnGround;
	float fDiff, fTime;

	ServerDE *pServerDE = GetServerDE();
	if(!pServerDE)
		return;

	moveCode = pServerDE->ReadFromMessageByte(hRead);

	pServerDE->ReadFromMessageVector(hRead, &newPos);
	
	curVel.x = (float)(short)pServerDE->ReadFromMessageWord(hRead);
	curVel.y = (float)(short)pServerDE->ReadFromMessageWord(hRead);
	curVel.z = (float)(short)pServerDE->ReadFromMessageWord(hRead);

	nMovementFlags = pServerDE->ReadFromMessageByte(hRead);
	bOnGround = ( nMovementFlags & CLIENTMOVEMENTFLAG_ONGROUND ) ? DTRUE : DFALSE;
	m_eLastSurface = (SurfaceType)pServerDE->ReadFromMessageByte(hRead);

	if(moveCode == m_byClientMoveCode)
	{
		if(m_nTrapped) return;

		SetOnGround(bOnGround);
		pServerDE->SetVelocity(m_hObject, &curVel);
		pServerDE->MoveObject(m_hObject, &newPos);

		fTime = g_pServerDE->GetTime( );
		// Check our leash.
		pServerDE->GetObjectPos(m_hObject, &curPos);
		fDiff = VEC_DIST( curPos, newPos );
		if( fDiff > 0.0f )
		{
			if( fDiff > m_fLeashLen || fTime > m_fTeleportTime )
			{
				pServerDE->TeleportObject( m_hObject, &newPos );
				m_fTeleportTime = fTime + TELEPORTPERIOD;
			}
		}
		else
			m_fTeleportTime = fTime + TELEPORTPERIOD;
	}
}


// ----------------------------------------------------------------------- //
// This sends a message to the client telling it to move to our position.
// Used when loading games and respawning.
// ----------------------------------------------------------------------- //
void CPlayerObj::TeleportClientToServerPos()
{
	HMESSAGEWRITE hWrite;
	DVector myPos;
	ServerDE *pServerDE = GetServerDE();


	pServerDE->GetObjectPos(m_hObject, &myPos);


	// Tell the player about the new move code.
	++m_byClientMoveCode;
//	hWrite = pServerDE->StartMessage(m_hClient, SMSG_FORCEPOS);
	hWrite = StartMessageToClient(SMSG_FORCEPOS);
	if( !hWrite )
		return;
	pServerDE->WriteToMessageByte(hWrite, m_byClientMoveCode);
	pServerDE->WriteToMessageVector(hWrite, &myPos);
	pServerDE->EndMessage(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	// Sanity checks...
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	// Check for some special case AddOn stuff...

#ifdef _ADDON
	if (g_bLevelChangeCharacter)	// character change requested?
	{
		TweakSaveValuesForCharacterChange(g_nLevelChangeCharacter);
		g_bLevelChangeCharacter = DFALSE;
	}
#endif

	// Get the game time...
	DFLOAT fTime = pServerDE->GetTime();

//	pServerDE->WriteToMessageByte(hWrite, m_bFirstUpdate);
	pServerDE->WriteToMessageByte(hWrite, m_bDemoPlayback);
	pServerDE->WriteToMessageDWord(hWrite, m_dwControlFlags);
	pServerDE->WriteToMessageByte(hWrite, m_bFiringWeapon);
	pServerDE->WriteToMessageByte(hWrite, m_bFalling);
	pServerDE->WriteToMessageByte(hWrite, m_bServerFalling);

	pServerDE->WriteToMessageFloat(hWrite, m_startFall - fTime);
	pServerDE->WriteToMessageFloat(hWrite, m_startIdle - fTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fMinFallPos);
	pServerDE->WriteToMessageFloat(hWrite, m_fMaxFallPos);

// Don't save these, we'll init them to something so that they'll get updated on the client
//	pServerDE->WriteToMessageDWord(hWrite, m_nLastHitPoints);
//	pServerDE->WriteToMessageDWord(hWrite, m_nLastDamage);
//	pServerDE->WriteToMessageDWord(hWrite, m_nLastAmmo);
//	pServerDE->WriteToMessageDWord(hWrite, m_nLastArmor);
//	pServerDE->WriteToMessageDWord(hWrite, m_nLastFocus);

	pServerDE->WriteToMessageFloat(hWrite, m_fBindingRegenTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fServerRegenTime);

	pServerDE->WriteToMessageByte(hWrite, m_bSpectatorMode);
	pServerDE->WriteToMessageFloat(hWrite, m_fSmellTime - fTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fSmellDelay);

	pServerDE->WriteToMessageByte(hWrite, m_bDead);
	pServerDE->WriteToMessageByte(hWrite, m_bMovementBlocked);
	pServerDE->WriteToMessageByte(hWrite, m_bPowerupActivated[PU_INVULNERABLE]);
	pServerDE->WriteToMessageByte(hWrite, m_bPowerupActivated[PU_INVISIBLE]);
	pServerDE->WriteToMessageByte(hWrite, m_bPowerupActivated[PU_TRIPLEDAMAGE]);
	pServerDE->WriteToMessageByte(hWrite, m_bPowerupActivated[PU_INCORPOREAL]);
	pServerDE->WriteToMessageByte(hWrite, m_bPowerupActivated[PU_SILENT]);
	pServerDE->WriteToMessageFloat(hWrite, m_fNighInvulnerableTime - fTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fInvisibleTime - fTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fTripleDamageTime - fTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fIncorporealTime - fTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fSilentTime - fTime);

	pServerDE->WriteToMessageByte( hWrite, m_nPowerupSoundStackPos );
	for( int nStackPos = 0; nStackPos < m_nPowerupSoundStackPos; nStackPos++ )
		pServerDE->WriteToMessageByte( hWrite, m_ePowerupSoundStack[ nStackPos ] );

	pServerDE->WriteToMessageHString(hWrite, m_hstrTrigger);
	pServerDE->WriteToMessageVector(hWrite, &m_vLastPos);
	pServerDE->WriteToMessageVector(hWrite, &m_vLastVel);
	pServerDE->WriteToMessageVector(hWrite, &m_vScale);

	pServerDE->WriteToMessageFloat(hWrite, m_fPitch);
	pServerDE->WriteToMessageFloat(hWrite, m_fYaw);
	pServerDE->WriteToMessageRotation(hWrite, &m_rRotation);
	pServerDE->WriteToMessageVector(hWrite, &m_vForward);
	pServerDE->WriteToMessageVector(hWrite, &m_vGunMuzzlePos);
	pServerDE->WriteToMessageVector(hWrite, &m_vlGunMuzzlePos);
	pServerDE->WriteToMessageHString(hWrite, m_hstrPlayerName);
	pServerDE->WriteToMessageByte(hWrite, m_byFlags);
	pServerDE->WriteToMessageFloat(hWrite, m_fMouseAxis0);
	pServerDE->WriteToMessageFloat(hWrite, m_fMouseAxis1);
		
	pServerDE->WriteToMessageByte(hWrite, m_bZoomView);
	pServerDE->WriteToMessageByte(hWrite, m_nZoomType);

	pServerDE->WriteToMessageObject(hWrite, m_hCameraObj);
//	pServerDE->WriteToMessageDWord(hWrite, m_CurGun);
//	pServerDE->WriteToMessageByte(hWrite, m_CurGunSlot);

//	pServerDE->WriteToMessageFloat(hWrite, m_fEyeLevel);
	pServerDE->WriteToMessageByte(hWrite, m_bLastCrouchCommand);
	pServerDE->WriteToMessageByte(hWrite, m_bLastJumpCommand);
	pServerDE->WriteToMessageByte(hWrite, m_bForcedCrouch);

	pServerDE->WriteToMessageDWord(hWrite, m_Frags);
	pServerDE->WriteToMessageDWord(hWrite, m_MeleeKills);

	pServerDE->WriteToMessageByte(hWrite, m_nCharacter);
	pServerDE->WriteToMessageByte(hWrite, m_nSkin);

	pServerDE->WriteToMessageByte(hWrite, m_nAttribStrength);
	pServerDE->WriteToMessageByte(hWrite, m_nAttribSpeed);
	pServerDE->WriteToMessageByte(hWrite, m_nAttribResistance);
	pServerDE->WriteToMessageByte(hWrite, m_nAttribMagic);

	pServerDE->WriteToMessageByte(hWrite, m_nBindingStrength);
	pServerDE->WriteToMessageByte(hWrite, m_nBindingSpeed);
	pServerDE->WriteToMessageByte(hWrite, m_nBindingResistance);
	pServerDE->WriteToMessageByte(hWrite, m_nBindingMagic);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bBindingConstitution);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bBindingBlending);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bBindingSoulStealing);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bBindingRegeneration);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bServerRegeneration);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bBindingQuickness);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bBindingIncreasedDamage);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bBindingImpactResistance);

	pServerDE->WriteToMessageByte(hWrite, m_byMoveState);
	pServerDE->WriteToMessageWord(hWrite, (DBYTE)m_nInjuredLeg);
	pServerDE->WriteToMessageWord(hWrite, (DBYTE)m_nNodeHit);
	pServerDE->WriteToMessageWord(hWrite, (DBYTE)m_nSideHit);

	pServerDE->WriteToMessageFloat(hWrite, m_fDeathTimer);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bInSlowDeath);

	pServerDE->WriteToMessageFloat(hWrite, m_fOldAirLevel);
	pServerDE->WriteToMessageFloat(hWrite, m_fAirLevel);

	pServerDE->WriteToMessageByte(hWrite, m_bSwimmingJump);
	pServerDE->WriteToMessageByte(hWrite, m_bCanSwimJump);

	pServerDE->WriteToMessageByte(hWrite, m_bImprisoned);
	pServerDE->WriteToMessageFloat(hWrite, m_fImprisonStart - fTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fImprisonLength);
	pServerDE->WriteToMessageByte(hWrite, m_bBurning);
	pServerDE->WriteToMessageFloat(hWrite, m_fBurningTime);
	
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)(m_hClientSaveData != DNULL));
	if (m_hClientSaveData)
	{
		pServerDE->WriteToMessageHMessageRead( hWrite, m_hClientSaveData );
		pServerDE->EndHMessageRead( m_hClientSaveData );
	}

	m_hClientSaveData = DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	DFLOAT fTime = pServerDE->GetTime();

//	m_bFirstUpdate			= pServerDE->ReadFromMessageByte(hRead);
	m_bDemoPlayback			= pServerDE->ReadFromMessageByte(hRead);
	m_dwControlFlags		= pServerDE->ReadFromMessageDWord(hRead);
	m_bFiringWeapon			= pServerDE->ReadFromMessageByte(hRead);
	m_bFalling				= pServerDE->ReadFromMessageByte(hRead);
	m_bServerFalling		= pServerDE->ReadFromMessageByte(hRead);

	m_startFall				= pServerDE->ReadFromMessageFloat(hRead) + fTime;
	m_startIdle				= pServerDE->ReadFromMessageFloat(hRead) + fTime;
	m_fMinFallPos			= pServerDE->ReadFromMessageFloat(hRead);
	m_fMaxFallPos			= pServerDE->ReadFromMessageFloat(hRead);

	m_nLastHitPoints		= 0;
	m_nLastDamage			= 0;
	m_nLastAmmo				= 0;
	m_nLastArmor			= 0;
	m_nLastFocus			= 0;

	m_fSlowDeathSafeTimer   = 0;
	m_fSlowDeathStayTimer   = 0;

	m_fBindingRegenTime		= pServerDE->ReadFromMessageFloat(hRead);
	m_fServerRegenTime		= pServerDE->ReadFromMessageFloat(hRead);

	m_bSpectatorMode		= pServerDE->ReadFromMessageByte(hRead);
	m_fSmellTime			= pServerDE->ReadFromMessageFloat(hRead) + fTime;
	m_fSmellDelay			= pServerDE->ReadFromMessageFloat(hRead);

	m_bDead					= pServerDE->ReadFromMessageByte(hRead);
	m_bMovementBlocked		= pServerDE->ReadFromMessageByte(hRead);
	m_bPowerupActivated[PU_INVULNERABLE]		= pServerDE->ReadFromMessageByte(hRead);
	m_bPowerupActivated[PU_INVISIBLE]			= pServerDE->ReadFromMessageByte(hRead);
	m_bPowerupActivated[PU_TRIPLEDAMAGE]		= pServerDE->ReadFromMessageByte(hRead);
	m_bPowerupActivated[PU_INCORPOREAL]			= pServerDE->ReadFromMessageByte(hRead);
	m_bPowerupActivated[PU_SILENT]				= pServerDE->ReadFromMessageByte(hRead);
	m_fNighInvulnerableTime	= pServerDE->ReadFromMessageFloat(hRead) + fTime;
	m_fInvisibleTime		= pServerDE->ReadFromMessageFloat(hRead) + fTime;
	m_fTripleDamageTime		= pServerDE->ReadFromMessageFloat(hRead) + fTime;
	m_fIncorporealTime		= pServerDE->ReadFromMessageFloat(hRead) + fTime;
	m_fSilentTime			= pServerDE->ReadFromMessageFloat(hRead) + fTime;

	m_nPowerupSoundStackPos = pServerDE->ReadFromMessageByte( hRead );
	for( int nStackPos = 0; nStackPos < m_nPowerupSoundStackPos; nStackPos++ )
		m_ePowerupSoundStack[ nStackPos ] = ( PowerupType )pServerDE->ReadFromMessageByte( hRead );

	m_hstrTrigger			= pServerDE->ReadFromMessageHString(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vLastPos);
	pServerDE->ReadFromMessageVector(hRead, &m_vLastVel);
	pServerDE->ReadFromMessageVector(hRead, &m_vScale);

	m_fPitch				= pServerDE->ReadFromMessageFloat(hRead);
	m_fYaw					= pServerDE->ReadFromMessageFloat(hRead);
	pServerDE->ReadFromMessageRotation(hRead, &m_rRotation);
	pServerDE->ReadFromMessageVector(hRead, &m_vForward);
	pServerDE->ReadFromMessageVector(hRead, &m_vGunMuzzlePos);
	pServerDE->ReadFromMessageVector(hRead, &m_vlGunMuzzlePos);
	m_hstrPlayerName		= pServerDE->ReadFromMessageHString(hRead);
	m_byFlags				= pServerDE->ReadFromMessageByte(hRead);
	m_fMouseAxis0			= pServerDE->ReadFromMessageFloat(hRead);
	m_fMouseAxis1			= pServerDE->ReadFromMessageFloat(hRead);

	m_bZoomView				= pServerDE->ReadFromMessageByte(hRead);
	m_nZoomType				= pServerDE->ReadFromMessageByte(hRead);

	m_hCameraObj			= pServerDE->ReadFromMessageObject(hRead);

//	m_CurGun				= pServerDE->ReadFromMessageDWord(hRead); 
//	m_CurGunSlot			= pServerDE->ReadFromMessageByte(hRead); 
//	m_fEyeLevel				= pServerDE->ReadFromMessageFloat(hRead); 
	m_bLastCrouchCommand	= pServerDE->ReadFromMessageByte(hRead);
	m_bLastJumpCommand		= pServerDE->ReadFromMessageByte(hRead);
	m_bForcedCrouch			= pServerDE->ReadFromMessageByte(hRead);

	m_Frags					= pServerDE->ReadFromMessageDWord(hRead); 
	m_MeleeKills			= pServerDE->ReadFromMessageDWord(hRead); 

	m_nCharacter			= pServerDE->ReadFromMessageByte(hRead);
	m_nSkin					= pServerDE->ReadFromMessageByte(hRead);
	m_nAttribStrength		= pServerDE->ReadFromMessageByte(hRead); 
	m_nAttribSpeed			= pServerDE->ReadFromMessageByte(hRead); 
	m_nAttribResistance		= pServerDE->ReadFromMessageByte(hRead); 
	m_nAttribMagic			= pServerDE->ReadFromMessageByte(hRead); 

	m_nBindingStrength		= pServerDE->ReadFromMessageByte(hRead); 
	m_nBindingSpeed			= pServerDE->ReadFromMessageByte(hRead); 
	m_nBindingResistance	= pServerDE->ReadFromMessageByte(hRead); 
	m_nBindingMagic			= pServerDE->ReadFromMessageByte(hRead); 
	m_bBindingConstitution	= (DBOOL)pServerDE->ReadFromMessageByte(hRead); 
	m_bBindingBlending		= (DBOOL)pServerDE->ReadFromMessageByte(hRead); 
	m_bBindingSoulStealing	= (DBOOL)pServerDE->ReadFromMessageByte(hRead); 
	m_bBindingRegeneration	= (DBOOL)pServerDE->ReadFromMessageByte(hRead); 
	m_bServerRegeneration	= (DBOOL)pServerDE->ReadFromMessageByte(hRead); 
	m_bBindingQuickness		= (DBOOL)pServerDE->ReadFromMessageByte(hRead); 
	m_bBindingIncreasedDamage	= (DBOOL)pServerDE->ReadFromMessageByte(hRead); 
	m_bBindingImpactResistance	= (DBOOL)pServerDE->ReadFromMessageByte(hRead); 

	m_byMoveState			= pServerDE->ReadFromMessageByte(hRead); 
	m_nInjuredLeg			= (int)pServerDE->ReadFromMessageWord(hRead);
	m_nNodeHit				= (int)pServerDE->ReadFromMessageWord(hRead);
	m_nSideHit				= (int)pServerDE->ReadFromMessageWord(hRead);

	m_fDeathTimer			= pServerDE->ReadFromMessageFloat(hRead);
	m_bInSlowDeath			= (DBOOL)pServerDE->ReadFromMessageByte(hRead); 

	m_fOldAirLevel			= pServerDE->ReadFromMessageFloat(hRead);
	m_fAirLevel				= pServerDE->ReadFromMessageFloat(hRead);

	m_bSwimmingJump			= pServerDE->ReadFromMessageByte(hRead);
	m_bCanSwimJump			= pServerDE->ReadFromMessageByte(hRead);

	m_bImprisoned			= pServerDE->ReadFromMessageByte(hRead);
	m_fImprisonStart		= pServerDE->ReadFromMessageFloat(hRead) + fTime;
	m_fImprisonLength		= pServerDE->ReadFromMessageFloat(hRead);
	m_bBurning				= pServerDE->ReadFromMessageByte(hRead);
	m_fBurningTime			= pServerDE->ReadFromMessageFloat(hRead);

	DBOOL bClientData		= (DBOOL)pServerDE->ReadFromMessageByte(hRead);
	if (bClientData)
	{
		HMESSAGEREAD hClientLoadData;
		hClientLoadData = pServerDE->ReadFromMessageHMessageRead( hRead );
		// Tell clients about client save data...
		if( hClientLoadData )
		{
			// We don't know the HCLIENT yet, so send it to all clients.  Right now, only
			// single player saves are allowed, so this isn't a big deal.  If we do multiplayer
			// saves, then we'd have to include some unique info, like a name, so each
			// client could filter on it...
			HMESSAGEWRITE hMessage = pServerDE->StartMessage( DNULL, SMSG_CLIENTSAVEDATA);
			if( hMessage )
			{
				pServerDE->WriteToMessageHMessageRead(hMessage, hClientLoadData);
				pServerDE->EndHMessageRead(hClientLoadData);
				pServerDE->EndMessage(hMessage);
			}
		}
	}

	// Flags set means this is a "keep alive" restore
	// Set up the filenames
	SetCharacter(m_nCharacter);

	// Reset any hidden nodes
	AIShared.ResetNodes(m_hObject);

	// Set flags
	DDWORD  dwFlags = FLAG_VISIBLE | FLAG_MODELKEYS | FLAG_YROTATION | FLAG_ANIMTRANSITION | FLAG_TOUCH_NOTIFY | FLAG_SOLID | FLAG_STAIRSTEP;
//	DDWORD  dwFlags = FLAG_FORCECLIENTUPDATE | FLAG_MODELKEYS | FLAG_YROTATION | FLAG_ANIMTRANSITION;
	pServerDE->SetObjectFlags(m_hObject, dwFlags);

	DVector vDims;
	VEC_INIT(vDims)
	pServerDE->SetVelocity(m_hObject, &vDims);

//	SetZoomMode( m_bZoomView, m_nZoomType );
	m_bZoomView = DFALSE;
	m_nZoomType = 0;

//	Respawn();

//	if( dwLoadFlags )
//		GoToStartPoint();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleWeaponFireMessage
//
//	PURPOSE:	Handle player firing weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleWeaponFireMessage(HMESSAGEREAD hRead)
{
	ServerDE *pServerDE = GetServerDE();
	if (!pServerDE) return;

	DVector vFirePos, vDir;
	pServerDE->ReadFromMessageVector(hRead, &vFirePos);

	pServerDE->ReadFromMessageVector(hRead, &vDir);
	DBYTE nRandomSeed	= pServerDE->ReadFromMessageByte(hRead);
	DBYTE nWeaponId		= pServerDE->ReadFromMessageByte(hRead);
	DBYTE byFlags		= pServerDE->ReadFromMessageByte(hRead);

	// Get the alt fire flag..
	DBOOL bAltFire  = (byFlags & 0x01) ? DTRUE : DFALSE;
	DBOOL bLeftHand = (byFlags & 0x02) ? DTRUE : DFALSE;

	//**********************************************************
	// Get some special information about the weapon (Andy 1/7/99)
	DDWORD	dwSpecial = 0;
	DFLOAT	fSpecial = 0.0f;
	DBOOL	bUpdateData = DFALSE;

	if(byFlags & 0x04)
	{
		dwSpecial = pServerDE->ReadFromMessageDWord(hRead);
		bUpdateData = DTRUE;
	}

	if(byFlags & 0x08)
	{
		fSpecial = pServerDE->ReadFromMessageFloat(hRead);
		bUpdateData = DTRUE;
	}
	//**********************************************************

	DRotation rRot;
	pServerDE->AlignRotation(&rRot, &vDir, DNULL);

	srand(nRandomSeed);

	if (!m_bMovementBlocked)
	{
		if(bUpdateData)
			m_InventoryMgr.SetCurrentWeaponSpecialData(dwSpecial, fSpecial);

		m_InventoryMgr.FireCurrentWeapon(&vFirePos, &rRot, bAltFire, bLeftHand);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleWeaponStateMessage
//
//	PURPOSE:	Handle weapon state change message
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleWeaponStateMessage(HMESSAGEREAD hRead)
{
	/*
	ServerDE *pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	DBYTE nWeaponId	= pServerDE->ReadFromMessageByte(hRead);
	if (m_weapons.IsValidWeapon(nWeaponId))
	{
		CWeapon* pWeapon = m_weapons.GetWeapon(nWeaponId);
		if (pWeapon)
		{
			pWeapon->HandleStateChange(hRead);
		}
	}
	*/
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleWeaponSoundMessage
//
//	PURPOSE:	Handle weapon sound message
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleWeaponSoundMessage(HMESSAGEREAD hRead)
{
	ServerDE *pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	DBYTE	byFlags	= pServerDE->ReadFromMessageByte(hRead);
	HSTRING sSound	= DNULL;

	if(byFlags != 0x02)
		sSound	= pServerDE->ReadFromMessageHString(hRead);

	switch(byFlags)
	{
		case	0x00:
			PlaySoundFromObject(m_hObject, pServerDE->GetStringData(sSound), 1000.0f, SOUNDPRIORITY_MISC_HIGH);
			break;

		case	0x01:
			if(m_hWeapSound)
				pServerDE->KillSound(m_hWeapSound);

			m_hWeapSound = PlaySoundFromObject(m_hObject, pServerDE->GetStringData(sSound), 1000.0f, SOUNDPRIORITY_MISC_HIGH, DTRUE, DTRUE);
			break;

		case	0x02:
			if(m_hWeapSound)
			{
				pServerDE->KillSound(m_hWeapSound);
				m_hWeapSound = DNULL;
			}
			break;
	}

	if(sSound)
		pServerDE->FreeString(sSound);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::DoWeaponChange
//
//	PURPOSE:	Change our weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoWeaponChange(DBYTE nWeaponId)
{
//	m_weapons.ChangeWeapon(nWeaponId);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GrabFlag()
//
//	PURPOSE:	Grabs the flag object in CTF play
//
// --------------------------------------------------------------------------- //

void CPlayerObj::GrabFlag(HOBJECT hFlag)
{
	// Sanity checks...

	if (!hFlag) return;
	if (m_damage.IsDead()) return;
	if (m_bDead) return;
	if (m_bSpectatorMode) return;


	// Make sure we're not already carrying this flag...

	if (m_hFlagObject == hFlag) return;


	// Get the flag object...

	FlagObject* pFlag = (FlagObject*)g_pServerDE->HandleToObject(hFlag);
	if (!pFlag) return;


	// Make sure it really is a flag object...

#ifdef _DEBUG
	if (!g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hFlag), g_pServerDE->GetClass("FlagObject")))
	{
		assert(0);
		return;
	}
#endif


	// If this is our flag, trigger it to return to it's base if it's not already there...

	if ((DDWORD)pFlag->GetTeamID() == GetTeamID())
	{
		if (!pFlag->IsInFlagStand())
		{
			SendMessageToClient(SMSG_RETURNEDTHEFLAG);
			pFlag->ReturnToFlagStand();
		}
		return;
	}


	// Remove the current attachment if there is one...

	if (m_hFlagAttach)
	{
		g_pServerDE->RemoveAttachment(m_hFlagAttach);
		m_hFlagAttach = DNULL;
	}


	// If this is the other teams flag, attach it to us...

	DVector vOffset;
	VEC_SET(vOffset, 0, 75, 0);

	DRotation rOffset;
	ROT_INIT(rOffset);

	DRESULT dr = g_pServerDE->CreateAttachment(m_hObject, hFlag, DNULL, &vOffset, &rOffset, &m_hFlagAttach);
	if (dr != LT_OK) return;


	// Send a message to our clients to let him know the flag was grabbed...

	SendMessageToClient(SMSG_GRABBEDTHEFLAG);


	// Let the flag know that it's no longer in it's stand...

	pFlag->GiveToPlayer();


	// Set out flag object since we are now carrying it...

	m_hFlagObject = hFlag;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GiveFlag()
//
//	PURPOSE:	Give the flag object to the stand in CTF play
//
// --------------------------------------------------------------------------- //

void CPlayerObj::GiveFlag(HOBJECT hFlagStand)
{
	// Sanity checks...

	if (!hFlagStand) return;


	// If we aren't carrying a flag, there's nothing to do...

	if (!m_hFlagAttach) return;
	if (!m_hFlagObject) return;


	// Get the flag stand...

	FlagStand* pFlagStand = (FlagStand*)g_pServerDE->HandleToObject(hFlagStand);
	if (!pFlagStand) return;


	// Make sure it really is a flag object...

#ifdef _DEBUG
	if (!g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hFlagStand), g_pServerDE->GetClass("FlagStand")))
	{
		assert(0);
		return;
	}
#endif


	// If this is not our flag stand, there's nothing to do...

	if ((DDWORD)pFlagStand->GetTeamID() != GetTeamID())
	{
		return;
	}


	// Make sure our flag is also in our stand...

	FlagObject* pFlag = (FlagObject*)g_pServerDE->HandleToObject(m_hFlagObject);
	if (!pFlag) return;

	if (!pFlag->IsOtherFlagInStand())
	{
		static DFLOAT fLastTime = 0;
		DFLOAT fCurTime = g_pServerDE->GetTime();

		if ((fLastTime + 4) < fCurTime)
		{
			SendMessageToClient(SMSG_NEEDYOURFLAG);
			fLastTime = fCurTime;
		}

		return;
	}


	// Since this is our flag stand, we get some points...

	CTeamMgr* pTeamMgr = g_pBloodServerShell->GetTeamMgr();

	CTeam* pTeam1 = pTeamMgr->GetTeam(TEAM_1);
	CTeam* pTeam2 = pTeamMgr->GetTeam(TEAM_2);

	int cPlayers = 1;

	if (GetTeamID() == TEAM_1) cPlayers = pTeam2->GetNumPlayers();
	if (GetTeamID() == TEAM_2) cPlayers = pTeam1->GetNumPlayers();

	if (cPlayers < 1) cPlayers = 1;

	if (!g_pBloodServerShell->GetNetGameInfo()->m_bUseTeamSize) 
	{
		cPlayers = 1;
	}

	int nFrags = g_pBloodServerShell->GetNetGameInfo()->m_nFlagValue * cPlayers;

	m_Frags += nFrags;
	g_pBloodServerShell->GetTeamMgr()->AddPlayerFrags(g_pServerDE->GetClientID(GetClient()), nFrags);


	// Play a cool "captured the flag" sound...

	char* sSound = "sounds_multipatch\\CtfFlagCapture.wav";
	PlaySoundFromObject(m_hObject, sSound, 2000.0f, SOUNDPRIORITY_MISC_MEDIUM, DFALSE, DFALSE, DTRUE);


	// Send a message to all clients to let the know a flag was captured...

	HMESSAGEWRITE hMsg = g_pServerDE->StartMessage(NULL, SMSG_CAPTUREDTHEFLAG);
	if (hMsg)
	{
		g_pServerDE->WriteToMessageDWord(hMsg, g_pServerDE->GetClientID(m_hClient));	// Client ID
		g_pServerDE->WriteToMessageDWord(hMsg, (DDWORD)nFrags);						// Frags
		g_pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
	}


	// Destroy our flag attachment...

	g_pServerDE->RemoveAttachment(m_hFlagAttach);

	m_hFlagAttach = DNULL;


	// Send the flag back to it's home base flag stand...

	if (m_hFlagObject)
	{
		FlagObject* pFlag = (FlagObject*)g_pServerDE->HandleToObject(m_hFlagObject);
		if (pFlag)
		{
			pFlag->ReturnToFlagStand();
		}

		m_hFlagObject = NULL;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::DropFlag()
//
//	PURPOSE:	Drops the flag in CTF play if we have a flag
//
// --------------------------------------------------------------------------- //

void CPlayerObj::DropFlag(DBOOL bNotifyClient)
{
	// Remove the attachment if there is one...

	if (m_hFlagAttach)
	{
		g_pServerDE->RemoveAttachment(m_hFlagAttach);
		m_hFlagAttach = DNULL;
	}


	// If we don't actually have a flag object, there's nothing to do...

	if (!m_hFlagObject) return;


	// Tweak the flag's position to use ours...

	DVector vPos;

	g_pServerDE->GetObjectPos(m_hObject, &vPos);
	g_pServerDE->SetObjectPos(m_hFlagObject, &vPos);


	// Send a message to our clients to let him know the flag was dropped...

	if (bNotifyClient)
	{
		SendMessageToClient(SMSG_DROPPEDTHEFLAG);
	}


	// Let the flag object know that it has been dropped to the ground...

	FlagObject* pFlag = (FlagObject*)g_pServerDE->HandleToObject(m_hFlagObject);
	if (pFlag)
	{
		pFlag->DropToGround();
	}


	// Set our flag object member to DNULL showing that we aren't carrying a flag...

	m_hFlagObject = DNULL;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SoccerGoal()
//
//	PURPOSE:	Score a soccer goal
//
// --------------------------------------------------------------------------- //
#ifdef _ADDON

void CPlayerObj::SoccerGoal( DBYTE nTeamID )
{
	const NetGame *pNetGame;
	CTeamMgr *pTeamMgr;
	CTeam *pTeam1, *pTeam2;
	int nPlayersOnMyTeam, nPlayersOnOppTeam, nFrags;

	pNetGame = g_pBloodServerShell->GetNetGameInfo();
	if( !pNetGame )
		return;
	pTeamMgr = g_pBloodServerShell->GetTeamMgr();
	if( !pTeamMgr )
		return;

	// Allow team sizes to affect score
	if( pNetGame->m_bUseTeamSize )
	{
		// Calculate the frag value of the goal based on team sizes.
		pTeam1 = pTeamMgr->GetTeam(TEAM_1);
		if( !pTeam1 )
			return;
		pTeam2 = pTeamMgr->GetTeam(TEAM_2);
		if( !pTeam2 )
			return;

		if( GetTeamID( ) == TEAM_1 )
		{
			nPlayersOnMyTeam = pTeam1->GetNumPlayers();
			nPlayersOnOppTeam = pTeam2->GetNumPlayers();
		}
		else
		{
			nPlayersOnMyTeam = pTeam2->GetNumPlayers();
			nPlayersOnOppTeam = pTeam1->GetNumPlayers();
		}

		// Minimum of one point
		nPlayersOnOppTeam = DMAX( 1, nPlayersOnOppTeam );
	}
	else
	{
		nPlayersOnMyTeam = 1;
		nPlayersOnOppTeam = 1;
	}

	nFrags = 0;

	// Scored a goal in opponent's goal
	if( nTeamID != GetTeamID( ))
	{
		nFrags = pNetGame->m_nGoalValue * nPlayersOnOppTeam;
		m_Frags += nFrags;
		pTeamMgr->AddPlayerFrags( g_pServerDE->GetClientID( GetClient( )), nFrags );
	}
	// Oops, scored a goal in own goal
	else
	{
		if( pNetGame->m_bNegTeamFrags )
		{
			nFrags = -pNetGame->m_nGoalValue * nPlayersOnMyTeam;
			m_Frags += nFrags;
			pTeamMgr->AddPlayerFrags( g_pServerDE->GetClientID( GetClient( )), nFrags );
		}
	}

	// Play a cool "captured the flag" sound...
	char* sSound = "sounds_multipatch\\CtfFlagCapture.wav";
	PlaySoundFromObject( m_hObject, sSound, 2000.0f, SOUNDPRIORITY_MISC_MEDIUM );

	// Send a message to all clients to let them know a flag was captured...
	HMESSAGEWRITE hMsg = g_pServerDE->StartMessage( NULL, SMSG_SOCCERGOAL );
	if (hMsg)
	{
		g_pServerDE->WriteToMessageDWord( hMsg, g_pServerDE->GetClientID( m_hClient ));	// Client ID
		g_pServerDE->WriteToMessageDWord( hMsg, ( DDWORD)nFrags );						// Frags
		g_pServerDE->WriteToMessageByte( hMsg, nTeamID );
		g_pServerDE->EndMessage2( hMsg, MESSAGE_GUARANTEED | MESSAGE_NAGGLE );
	}
}
#endif // _ADDON

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::DoDeath()
//
//	PURPOSE:	Makes the player dead
//
// --------------------------------------------------------------------------- //

void CPlayerObj::DoDeath(DBOOL bHumiliation)
{
	DLink *pCur, *pNext;
	HOBJECT hObj;
	HMESSAGEWRITE hMsg;

	// Flag that we are dead
	m_bDead               = DTRUE;
	m_bInSlowDeath        = DFALSE;
	m_fSlowDeathSafeTimer = 0;
	m_fSlowDeathStayTimer = 0;

	// Reset all powerups
	ResetPowerups();

	for (int i=INV_MEDKIT; i <= INV_LASTINVWEAPON; i++)
	{
		m_InventoryMgr.RemoveItem( i );
	}

	if (m_damage.GetWhoKilledMeLast() == m_hObject)
	{
		PlayVoiceGroupEventOnClient(VME_SUICIDE, DTRUE);
	}
	else
	{
		PlayVoiceGroupEventOnClient(VME_DEATH, DFALSE);
	}

	SendMessageToClient(SMSG_DEAD);

	// Turn off weapon zoom if on
	if (m_bZoomView)
	{
		SetZoomMode(DFALSE);
	}

	// Hide weapon models
	m_InventoryMgr.ShowViewWeapons(DFALSE);

	// Update frag status.
	if (g_pBloodServerShell->IsMultiplayerGame())
	{
		UpdateFragStatus();
	}

	// kill any current sound
	if(m_hCurSound)
	{
		g_pServerDE->KillSound(m_hCurSound);
		m_hCurSound = DNULL;
	}

	if(m_hWeapSound)
	{
		g_pServerDE->KillSound(m_hWeapSound);
		m_hWeapSound = DNULL;
	}

	// Hide the player model

	if (!bHumiliation)
	{
		DDWORD dwFlags = g_pServerDE->GetObjectFlags(m_hObject);
		g_pServerDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE);
	}

	m_bAnimating = DFALSE;
	m_byMoveState = PMOVE_DEAD;

	int nNodeHit = m_damage.GetNodeHit();
	int nType = m_damage.GetLastDamageType();
	DFLOAT fDamage = m_damage.GetLastDamageAmount();

	DVector vDir;
	m_damage.GetLastDamageDirection(&vDir);

	if (bHumiliation)
	{
		SetAnimation(m_Anim_Sound.m_nAnim_HUMILIATION[3], DTRUE);
		m_bAnimating = DTRUE; 
	}
	else
	{
		if(nType & DAMAGE_TYPE_EXPLODE)
		{
			CreateGibs(vDir, ((int)m_damage.GetMass())>>5, nType, fDamage);
		}
		else if(m_damage.GetLastDamagePercent() >= 50.0f)
		{
			CreateGibs(vDir, ((int)m_damage.GetMass())>>5, nType, fDamage);
		}
		else if (nType != DAMAGE_TYPE_FREEFALL)
		{
			if(AIShared.HideLimb(m_hObject,nNodeHit))
			{
				AIShared.CreateLimb(m_hObject, nNodeHit, vDir);

				//Create an arterial blood spurt
	//				CreateBloodSpurt(nNodeHit);
			}

			// See if I should gib, throw an head off or something..
			switch(m_nNodeHit)
			{
				case NODE_NECK:		m_nCorpseType = m_Anim_Sound.m_nAnim_DEATH[0 + m_nSideHit]; break;
				case NODE_TORSO:	m_nCorpseType = m_Anim_Sound.m_nAnim_DEATH[1 + m_nSideHit]; break;
				case NODE_RARM:		m_nCorpseType = m_Anim_Sound.m_nAnim_DEATH[2 + m_nSideHit]; break;
				case NODE_LARM:		m_nCorpseType = m_Anim_Sound.m_nAnim_DEATH[3 + m_nSideHit]; break;
				case NODE_LLEG:		m_nCorpseType = m_Anim_Sound.m_nAnim_DEATH[4 + m_nSideHit]; break;
				case NODE_RLEG:		m_nCorpseType = m_Anim_Sound.m_nAnim_DEATH[5 + m_nSideHit]; break;
				default: m_nCorpseType = m_Anim_Sound.m_nAnim_DEATH[1 + m_nSideHit]; break;
			}

			// Create a corpse or gibs
			CreateCorpse();
		}
	}

	// Unattach the objects on us
	pCur = m_AttachedObjectsList.m_Head.m_pNext;
	while( pCur != &m_AttachedObjectsList.m_Head )
	{
		pNext = pCur->m_pNext;
		hObj = ( HOBJECT )pCur->m_pData;

		if( hObj )
		{
			hMsg = g_pServerDE->StartMessageToObject( this, hObj, MID_UNATTACH );
			g_pServerDE->EndMessage( hMsg );
		}

		dl_RemoveAt( &m_AttachedObjectsList, pCur );
		delete pCur;
		pCur = pNext;
	}

}

HMESSAGEWRITE CPlayerObj::StartMessageToClient(DBYTE msgID)
{
	if(m_hClient)
		return GetServerDE()->StartMessage(m_hClient, msgID);
	else
		return DNULL;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::TweakSaveValuesForCharacterChange()
//
//	PURPOSE:	Tweaks the saved values during a character change
//
// --------------------------------------------------------------------------- //

void CPlayerObj::TweakSaveValuesForCharacterChange(int nNewCharacter)
{
	// Set the new character value...

	m_nCharacter = g_nLevelChangeCharacter;


	// Get the new character attribute values...

	int nStr = 1;
	int nSpd = 1;
	int nRes = 1;
	int nMag = 1;

	switch (m_nCharacter)
	{
		case CHARACTER_CALEB:
		{
			nStr = 5; nSpd = 3; nRes = 3; nMag = 1; break;
			break;
		}

		case CHARACTER_OPHELIA:
		{
			nStr = 2; nSpd = 5; nRes = 1; nMag = 4; break;
			break;
		}

		case CHARACTER_ISHMAEL:
		{
			nStr = 1; nSpd = 3; nRes = 3; nMag = 5; break;
			break;
		}

		case CHARACTER_GABREILLA:
		{
			nStr = 5; nSpd = 1; nRes = 5; nMag = 1; break;
			break;
		}
	}


	// Set the new attribute values...

	m_nAttribStrength   = nStr;
	m_nAttribSpeed      = nSpd;
	m_nAttribResistance = nRes;
	m_nAttribMagic      = nMag;


	// Validate the attributes...

	ValidateAttributes();


	// Clear the powerup values...

	ClearPowerupValues();


	// Set the starting damage values...

	m_damage.SetStartingCharacterValues();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::IsPlayerDamageOk
//
//	PURPOSE:	Determines if it's ok for damage to occur between this
//				player and the given player.
//
// ----------------------------------------------------------------------- //

DBOOL CPlayerObj::IsPlayerDamageOk(CPlayerObj* pPlayer2)
{
	// Sanity checks...

	if (!pPlayer2) return(DTRUE);
	if (this == pPlayer2) return(DTRUE);


	// Check if we are playing a multiplayer game that has the right options set...

	if (!g_pBloodServerShell->IsMultiplayerTeamBasedGame()) return(DTRUE);
	if (g_pBloodServerShell->GetNetGameInfo()->m_bFriendlyFire) return(DTRUE);


	// Check if these players are on the same team...

	if (GetTeamID() == pPlayer2->GetTeamID()) return(DFALSE);


	// If we get here, it's ok to do the damage...

	return(DTRUE);
}

DBOOL CPlayerObj::IsPlayerDamageOk(HOBJECT hPlayer2)
{
	// Sanity checks...

	if (!hPlayer2) return(DTRUE);
	if (!IsPlayer(hPlayer2)) return(DTRUE);


	// Get the CPlayerObj pointer...

	CPlayerObj* pPlayer2 = (CPlayerObj*)g_pServerDE->HandleToObject(hPlayer2);


	// Let the main functioin do the real work...

	return(IsPlayerDamageOk(pPlayer2));
}
