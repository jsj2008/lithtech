
 /****************************************************************************
 ;
 ;	MODULE:		AI_Mgr.cpp
 ; ;
 ;	HISTORY:	Created by SCHLEGZ on 3/19/98 2:48:28 PM
 ;
 ;	COMMENT:	Copyright (c) 1997, Monolith Productions Inc.
 ;
 ****************************************************************************/

#include <stdio.h>
#include "Destructable.h"
#include "BaseCharacter.h"
#include "PathPoint.h"
#include "AI_Mgr.h"
#include "ObjectUtilities.h"
#include "AIScriptList.h"

#include "PathMgr.h"
#include "PathListData.h"
#include "BloodServerShell.h"
//#include "clientbloodtrail.h"
#include "exithint.h"
#include "smellhint.h"
#include "ammopickups.h"
#include "playerobj.h"
#include "SFXMsgIds.h"
#include "Trigger.h"
#include "corpse.h"
#include "clientgibfx.h"
#include "ObjectUtilities.h"
#include "SParam.h"
#include "VoiceMgrDefs.h"
#include "cameraobj.h"
#include "SoundTypes.h"

#include "windows.h"
#include <mbstring.h>

#undef PlaySound

DLink AI_Mgr::m_CabalHead;
DDWORD AI_Mgr::m_dwNumCabal = 0;
DLink AI_Mgr::m_MonsterHead;
DDWORD AI_Mgr::m_dwNumMonster = 0;

DFLOAT AI_Mgr::m_fLastFrame = 0;

extern CBloodServerShell* g_pBloodServerShell;
extern CPlayerObj* g_pPlayerObj;

extern DBOOL LiquidFilterFn(HOBJECT hObj, void *pUserData);
extern DBOOL CharacterFilterFn(HOBJECT hObj, void *pUserData);

// Used by the player object to set the view if a camera is active
extern HOBJECT g_hActiveCamera;

BEGIN_CLASS(AI_Mgr)
	ADD_BASEAI_AGGREGATE()
END_CLASS_DEFAULT_FLAGS(AI_Mgr, CBaseCharacter, NULL, NULL, CF_HIDDEN)

//Helper functions
DFLOAT MIN_VAL(DFLOAT a, DFLOAT b)
{
	if(a <= b)
		return a;
	else
		return b;
}

DFLOAT MAX_VAL(DFLOAT a, DFLOAT b)
{
	if(a >= b)
		return a;
	else
		return b;
}


char	g_sCacheDir[256] = { "" };


// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_Mgr
// DESCRIPTION	: Constructor
// RETURN TYPE	: void
// ----------------------------------------------------------------------- //

AI_Mgr::AI_Mgr() : CBaseCharacter(OT_MODEL)
{
	m_hstrSpotTriggerTarget	 = DNULL;
	m_hstrSpotTriggerMessage = DNULL;
	m_hstrTriggerRelayTarget = DNULL;
    
	m_AIPathList.Init(DFALSE);
	m_nScriptCmdIndex	= 0;

	// Class pointers
    m_pAnim_Sound   = DNULL;

	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"IDLE" );

	for(int i=0; i < MAX_NUM_WEAPONS; i++)
	  	_mbscpy((unsigned char*)m_szAIWeapon[i], (const unsigned char*)"NONE" );

	m_fAIBullets	= 0.0f;
	m_fAIBMG		= 0.0f;
	m_fAIShells		= 0.0f;
	m_fAIGrenades	= 0.0f;
	m_fAIRockets	= 0.0f;
	m_fAIFlares		= 0.0f;
	m_fAICells		= 0.0f;
	m_fAICharges	= 0.0f;
	m_fAIFuel		= 0.0f;
	m_fProxBombs	= 0.0f;

    // Target object vars
	m_hTarget		= DNULL;
	VEC_INIT(m_vTargetPos);
   	m_hTrackObject  = DNULL;
    VEC_INIT(m_vTrackObjPos);

	memset(&m_fStimuli,0,sizeof(m_fStimuli));		

	m_nState		= STATE_Idle;
    m_nLastState    = STATE_Idle;
    
    // Adjustable vars
    m_fHearingDist  = 0;
    m_fSensingDist  = 0.0f;
    m_fSmellingDist = 0.0f;
    m_fSeeingDist   = 0.0f;

	m_fWalkSpeed		= 5.0f;
	m_fRunSpeed			= 10.0f;
	m_fJumpSpeed		= 500.0f;
	m_fRollSpeed		= 2.0f;
        
    m_fTimeStart        = 0.0f;
    m_fLoadTimeStart    = 0.0f;
        
    m_bAnimating		= DFALSE;
	m_bJumping			= DFALSE;
    Metacmd				= 1;
    
	m_hCurSound			= DNULL;

	m_pServerDE			= DNULL;
	m_bSetShutdown		= DFALSE;

	m_nInjuredLeg		= 0;

	VEC_SET(m_vScale, 1.0f, 1.0f, 1.0f);

	if( m_dwNumCabal == 0 )
	{
		dl_TieOff( &m_CabalHead );
	}

	if( m_dwNumMonster == 0 )
	{
		dl_TieOff( &m_MonsterHead );
	}


	m_hEnemyAttach = DNULL;

	// Greg 9/20/98:  Set this to False for any enemies that need to start off in the air.
	m_bMoveToGround = DTRUE;

	// [blg] 10/05/98
	m_bCabal = DTRUE;

	memset(&m_Link, 0, sizeof(m_Link));

	m_fAIMass        = AI_DEFAULT_MASS;
	m_fAIHitPoints   = 10;
	m_fAIRandomHP    = 10;
	m_fAIArmorPoints = 0;
	m_nAIStrength    = 5;

	m_pCurSmell = DNULL;
	m_hCurSmell = DNULL;

	VEC_INIT(m_vSmellPos);
	VEC_INIT(m_vDestPos);

	m_nDodgeFlags = 0;

	m_nCorpseType     = 0;
	m_fAttackLoadTime = 0;

	m_nCurMetacmd = 1;
	m_dwFlags  = 0;
	m_nBlockFlags = 0;

	m_bUpdateScriptCmd = DFALSE;
	m_dwScriptFlags    = 0;

	m_eScriptMovement = SM_WALK;
	m_fScriptWaitEnd  = 0;

	m_fLastUpdate = 0.0f;

	m_bRemoveMe			= DFALSE;

	m_fLastLedgeCheck	= 0.0f;

	m_nCurAnimation = 999;  // Set to a large number to force a new animation.

	VEC_SET(m_vDims, 1.0, 1.0, 1.0);

	m_bStartFire		= DFALSE;
	m_hFireSource		= DNULL;
	
	m_bProceedAttach	= DFALSE;

	m_fWaitForIdleTime = 0.0f;
	m_bHasTargeted = DFALSE;
}


// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::~AI_Mgr
// DESCRIPTION	: Destructor
// RETURN TYPE	: void
// ----------------------------------------------------------------------- //

AI_Mgr::~AI_Mgr()
{
	if (!m_pServerDE) return;

	if (m_hstrSpotTriggerTarget)
		m_pServerDE->FreeString(m_hstrSpotTriggerTarget);

	if (m_hstrSpotTriggerMessage)
		m_pServerDE->FreeString(m_hstrSpotTriggerMessage);

	if (m_hstrTriggerRelayTarget)
		m_pServerDE->FreeString(m_hstrTriggerRelayTarget);


	// 10/25/98 [gjk] added check for m_bCabal since it's only being added to one list.
	if (m_bCabal)
	{
		if( m_Link.m_pData && m_dwNumCabal > 0 )
		{
			dl_Remove( &m_Link );
			m_dwNumCabal--;
		}
	}
	else
	{
		if( m_Link.m_pData && m_dwNumMonster > 0 )
		{
			dl_Remove( &m_Link );
			m_dwNumMonster--;
		}
	}

	if(m_hCurSound)
		m_pServerDE->KillSound(m_hCurSound);
}


// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::InitStatics
// DESCRIPTION	: Initialization function
// RETURN TYPE	: DBOOL 
// PARAMS		: CAnim_Sound* pAnim_Sound
// PARAMS		: float* pOutputTable
// PARAMS		: float* pInternalTable
// ----------------------------------------------------------------------- //

DBOOL AI_Mgr::InitStatics(CAnim_Sound* pAnim_Sound)
{
    m_pAnim_Sound = pAnim_Sound;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::StateStrToInt
// DESCRIPTION	: 
// RETURN TYPE	: int 
// PARAMS		: char *pState
// ----------------------------------------------------------------------- //

int AI_Mgr::StateStrToInt(char *pState)
{
    int m_nRetState = STATE_Inactive;

     // Make sure its Upper case
 	_strupr(pState);


    if ( _mbsncmp((const unsigned char*)pState, (const unsigned char*)"INACTIVE", 8) == 0)
    {
        m_nRetState = STATE_Inactive;
    }
    else if ( _mbsncmp((const unsigned char*)pState, (const unsigned char*)"IDLE", 4) == 0)
    {
        m_nRetState = STATE_Idle;
    }
    else if ( _mbsncmp((const unsigned char*)pState, (const unsigned char*)"SEARCH", 4) == 0)
    {
        m_nRetState = STATE_SearchVisualTarget;
    }
    else if ( _mbsncmp((const unsigned char*)pState, (const unsigned char*)"ATTACK", 6) == 0)
    {
        m_nRetState = STATE_AttackFar;
    }
    else if ( _mbsncmp((const unsigned char*)pState, (const unsigned char*)"HIDE", 4) == 0)
    {
        m_nRetState = STATE_Escape_Hide;
    }
    else if ( _mbsncmp((const unsigned char*)pState, (const unsigned char*)"GUARD", 5) == 0)
    {
        m_nRetState = STATE_GuardLocation;
    }
    else if ( _mbsncmp((const unsigned char*)pState, (const unsigned char*)"SCRIPT", 6) == 0)
    {
        m_nRetState = STATE_Script;
    }
    else if ( _mbsncmp((const unsigned char*)pState, (const unsigned char*)"SPECIAL1", 8) == 0)
    {
        m_nRetState = STATE_Special1;
    }
    else if ( _mbsncmp((const unsigned char*)pState, (const unsigned char*)"SPECIAL2", 8) == 0)
    {
        m_nRetState = STATE_Special2;
    }
    else if ( _mbsncmp((const unsigned char*)pState, (const unsigned char*)"SPECIAL3", 8) == 0)
    {
        m_nRetState = STATE_Special3;
    }
    else if ( _mbsncmp((const unsigned char*)pState, (const unsigned char*)"SPECIAL4", 8) == 0)
    {
        m_nRetState = STATE_Special4;
    }
    else if ( _mbsncmp((const unsigned char*)pState, (const unsigned char*)"SPECIAL5", 8) == 0)
    {
        m_nRetState = STATE_Special5;
    }
    else if ( _mbsncmp((const unsigned char*)pState, (const unsigned char*)"SPECIAL6", 8) == 0)
    {
        m_nRetState = STATE_Special6;
    }
    else if ( _mbsncmp((const unsigned char*)pState, (const unsigned char*)"SPECIAL7", 8) == 0)
    {
        m_nRetState = STATE_Special7;
    }
    else if ( _mbsncmp((const unsigned char*)pState, (const unsigned char*)"SPECIAL8", 8) == 0)
    {
        m_nRetState = STATE_Special8;
    }
    else if ( _mbsncmp((const unsigned char*)pState, (const unsigned char*)"SPECIAL9", 8) == 0)
    {
        m_nRetState = STATE_Special9;
    }
    else if ( _mbsncmp((const unsigned char*)pState, (const unsigned char*)"PASSIVE", 7) == 0)
    {
        m_nRetState = STATE_Passive;
    }

    return m_nRetState;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::WeaponStrToInt
// DESCRIPTION	: 
// RETURN TYPE	: DDWORD 
// PARAMS		: char *pWeapon
// ----------------------------------------------------------------------- //

DDWORD AI_Mgr::WeaponStrToInt(char *pWeapon)
{

    DDWORD m_nRetWeapon = WEAP_BERETTA;
    
//	WEAP_BERETTA,
    if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"BERET", 5) == 0)
    {
        m_nRetWeapon = WEAP_BERETTA;
    }
//	WEAP_SHOTGUN,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"SHOTGU", 5) == 0)
    {
        m_nRetWeapon = WEAP_SHOTGUN;
    }
//	WEAP_SNIPERRIFLE,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"SNIPE", 5) == 0)
    {
        m_nRetWeapon = WEAP_SNIPERRIFLE;
    }
//	WEAP_ASSAULTRIFLE,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"ASSAULTR", 8) == 0)
    {
        m_nRetWeapon = WEAP_ASSAULTRIFLE;
    }
//	WEAP_SUBMACHINEGUN,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"SUBMA", 5) == 0)
    {
        m_nRetWeapon = WEAP_SUBMACHINEGUN;
    }
//	WEAP_FLAREGUN,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"FLARE", 5) == 0)
    {
        m_nRetWeapon = WEAP_FLAREGUN;
    }
//	WEAP_ASSAULTCANNON,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"HOWITZ", 6) == 0)
    {
        m_nRetWeapon = WEAP_HOWITZER;
    }
//	WEAP_BUGSPRAY,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"FLAME", 5) == 0)
    {
        m_nRetWeapon = WEAP_BUGSPRAY;
    }
//	WEAP_NAPALMCANNON,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"NAPALM", 6) == 0)
    {
        m_nRetWeapon = WEAP_NAPALMCANNON;
    }
//	WEAP_MINIGUN,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"MINIG", 5) == 0)
    {
        m_nRetWeapon = WEAP_MINIGUN;
    }
//	WEAP_VOODOO,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"VOODO", 5) == 0)
    {
        m_nRetWeapon = WEAP_VOODOO;
    }
//	WEAP_ORB,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"ORB", 3) == 0)
    {
        m_nRetWeapon = WEAP_ORB;
    }
//	WEAP_DEATHRAY,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"LASER", 5) == 0)
    {
        m_nRetWeapon = WEAP_DEATHRAY;
    }
//	WEAP_LIFELEECH,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"LIFEL", 5) == 0)
    {
        m_nRetWeapon = WEAP_LIFELEECH;
    }
//	WEAP_TESLACANNON,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"TESLA", 5) == 0)
    {
        m_nRetWeapon = WEAP_TESLACANNON;
    }
//	WEAP_NAPALMCANNON,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"NAPALM", 6) == 0)
    {
        m_nRetWeapon = WEAP_NAPALMCANNON;
    }
//    WEAP_MELEE,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"MELEE", 5) == 0)
    {
        m_nRetWeapon = WEAP_MELEE;
    }
#ifdef _ADD_ON
//	WEAP_COMBATSHOTGUN,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"COMBAT", 6) == 0)
    {
        m_nRetWeapon = WEAP_COMBATSHOTGUN;
    }
//	WEAP_FLAYER,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"FLAYER", 6) == 0)
    {
        m_nRetWeapon = WEAP_FLAYER;
    }
#endif
//    WEAP_SINGULARITY,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"SINGULAR", 8) == 0)
    {
        m_nRetWeapon = WEAP_SINGULARITY;
    }
//    WEAP_NONE,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"NONE", 4) == 0)
    {
        m_nRetWeapon = WEAP_NONE;
    }
//    WEAP_NONE,
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"PROX_BOMB", 9) == 0)
    {
        m_nRetWeapon = WEAP_PROXIMITYBOMB;
    }
//	WEAP_SHIKARI_CLAW
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"SHIKARI_CLAW", 12) == 0)
    {
        m_nRetWeapon = WEAP_SHIKARI_CLAW;
    }
//	WEAP_SHIKARI_SPIT
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"SHIKARI_SPIT", 12) == 0)
    {
        m_nRetWeapon = WEAP_SHIKARI_SPIT;
    }
//	WEAP_SOUL_CROWBAR
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"SOUL_CROWBAR", 12) == 0)
    {
        m_nRetWeapon = WEAP_SOUL_CROWBAR;
    }
//	WEAP_SOUL_AXE
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"SOUL_AXE", 8) == 0)
    {
        m_nRetWeapon = WEAP_SOUL_AXE;
    }
//	WEAP_SOUL_PIPE
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"SOUL_PIPE", 9) == 0)
    {
        m_nRetWeapon = WEAP_SOUL_PIPE;
    }
//	WEAP_SOUL_HOOK
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"SOUL_HOOK", 9) == 0)
    {
        m_nRetWeapon = WEAP_SOUL_HOOK;
    }
//	WEAP_BEHEMOTH_CLAW
    else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"BEHEMOTH_CLAW", 13) == 0)
    {
        m_nRetWeapon = WEAP_BEHEMOTH_CLAW;
    }
//  WEAP_ZEALOT_HEAL
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"HEAL", 4) == 0)
	{
		m_nRetWeapon = WEAP_ZEALOT_HEAL;
	}
//  WEAP_ZEALOT_SHIELD
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"SHIELD", 6) == 0)
	{
		m_nRetWeapon = WEAP_ZEALOT_SHIELD;
	}
//  WEAP_ZEALOT_ENERGYBLAST
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"ENERGY_BLAST", 12) == 0)
	{
		m_nRetWeapon = WEAP_ZEALOT_ENERGYBLAST;
	}
//  WEAP_ZEALOT_GROUNDFIRE
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"GROUND_FIRE", 11) == 0)
	{
		m_nRetWeapon = WEAP_ZEALOT_GROUNDFIRE;
	}
//  WEAP_ZEALOT_SHOCKWAVE
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"SHOCKWAVE", 9) == 0)
	{
		m_nRetWeapon = WEAP_ZEALOT_SHOCKWAVE;
	}
//  WEAP_DRUDGE_FIREBALL
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"FIREBALL", 8 ) == 0)
	{
		m_nRetWeapon = WEAP_DRUDGE_FIREBALL;
	}
//	WEAP_DRUDGE_LIGHTNING
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"LIGHTNING", 9 ) == 0)
	{
		m_nRetWeapon = WEAP_DRUDGE_LIGHTNING;
	}
//  WEAP_HAND_SQUEEZE
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"HAND_SQUEEZE", 12 ) == 0)
	{
		m_nRetWeapon = WEAP_HAND_SQUEEZE;
	}
//  WEAP_THIEF_SUCK
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"THIEF_SUCK", 10 ) == 0)
	{
		m_nRetWeapon = WEAP_THIEF_SUCK;
	}
//  WEAP_BONELEECH_SUCK
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"BONELEECH_SUCK", 14 ) == 0)
	{
		m_nRetWeapon = WEAP_BONELEECH_SUCK;
	}
//  WEAP_NIGHTMARE_BITE
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"NIGHTMARE_BITE", 14 ) == 0)
	{
		m_nRetWeapon = WEAP_NIGHTMARE_BITE;
	}
//  WEAP_BEHEMOTH_SHOCKWAVE
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"BEHEMOTH_SHOCKWAVE", 18 ) == 0)
	{
		m_nRetWeapon = WEAP_BEHEMOTH_SHOCKWAVE;
	}
//	WEAP_DEATHSHROUD_ZAP
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"DEATHSHROUD_ZAP", 15 ) == 0)
	{
		m_nRetWeapon = WEAP_DEATHSHROUD_ZAP;
	}
//	WEAP_NAGA_EYEBLAST
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"NAGA_BLAST", 10 ) == 0)
	{
		m_nRetWeapon = WEAP_NAGA_EYEBEAM;
	}
//	WEAP_NAGA_SPIKE
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"NAGA_SPIKE", 10 ) == 0)
	{
		m_nRetWeapon = WEAP_NAGA_SPIKE;
	}
//	WEAP_NAGA_DEBRIS
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"NAGA_DEBRIS", 11 ) == 0)
	{
		m_nRetWeapon = WEAP_NAGA_DEBRIS;
	}
//	WEAP_GIDEON_SHIELD
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"GIDEON_SHIELD", 13 ) == 0)
	{
		m_nRetWeapon = WEAP_GIDEON_SHIELD;
	}
//	WEAP_GIDEON_WIND
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"GIDEON_WIND", 11 ) == 0)
	{
		m_nRetWeapon = WEAP_GIDEON_WIND;
	}
//	WEAP_GIDEON_GOO
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"GIDEON_GOO", 10 ) == 0)
	{
		m_nRetWeapon = WEAP_GIDEON_GOO;
	}
//	WEAP_GIDEON_VOMIT
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"GIDEON_VOMIT", 12 ) == 0)
	{
		m_nRetWeapon = WEAP_GIDEON_VOMIT;
	}
//	WEAP_GIDEON_SPEAR
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"GIDEON_SPEAR", 12 ) == 0)
	{
		m_nRetWeapon = WEAP_GIDEON_SPEAR;
	}
//	WEAP_ANCIENTONE_BEAM
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"ANCIENTONE_BEAM", 15 ) == 0)
	{
		m_nRetWeapon = WEAP_ANCIENTONE_BEAM;
	}
//	WEAP_ANCIENTONE_TENTACLE
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"ANCIENTONE_TENTACLE", 19 ) == 0)
	{
		m_nRetWeapon = WEAP_ANCIENTONE_TENTACLE;
	}
//	WEAP_SKULL
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"SKULL", 5 ) == 0)
	{
		m_nRetWeapon = WEAP_SKULL;
	}
#ifdef _ADD_ON
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"ROCK", 4 ) == 0)
	{
		m_nRetWeapon = WEAP_GREMLIN_ROCK;
	}
	else if ( _mbsncmp((const unsigned char*)pWeapon, (const unsigned char*)"NIGHTMARE_FIREBALL", 18) == 0)
	{
		m_nRetWeapon = WEAP_NIGHTMARE_FIREBALLS;
	}
#endif


    return m_nRetWeapon;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::EngineMessageFn
// DESCRIPTION	: Handle engine messages
// RETURN TYPE	: DDWORD 
// PARAMS		: DDWORD messageID
// PARAMS		: void *pData
// PARAMS		: DFLOAT fData
// ----------------------------------------------------------------------- //

DDWORD AI_Mgr::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (!AI_Update()) 
            {
				RemoveMe();
            }
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_WORLDFILE)
				m_bMoveToGround = DFALSE;

			InitialUpdate((int)fData);
			break;
		}

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
				ReadProp((ObjectCreateStruct*)pData);
			break;
		}

		case MID_TOUCHNOTIFY:
        {
			HandleTouch((HOBJECT)pData);
			break;
        }

		case MID_MODELSTRINGKEY:
		{
			OnStringKey((ArgList*)pData);
			return 0;
		}

		// If we created a link to the target, this will tell us that it no longer exists
		case MID_LINKBROKEN:
		{
			HOBJECT hObj = (HOBJECT)pData;
			if (hObj == m_hTarget)
			{
				m_hTarget = DNULL;
			}
			if (hObj == m_hTrackObject)
			{
				m_hTrackObject = DNULL;
			}
			if(m_hCurSmell)
			{
				if (hObj == m_hCurSmell)
				{
					m_hTrackObject = DNULL;
					SetNewState(STATE_Idle);
				}
			}

			break;
		}

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			InsertMe();
			break;
		
		default : break;
	}

	return CBaseCharacter::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::ObjectMessageFn
// DESCRIPTION	: Handle inter-object messages
// RETURN TYPE	: DDWORD 
// PARAMS		: HOBJECT hSender
// PARAMS		: DDWORD messageID
// PARAMS		: HMESSAGEREAD hRead
// ----------------------------------------------------------------------- //

DDWORD AI_Mgr::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_DAMAGE:
		{
			if (!m_pServerDE) break;
			
			CBaseCharacter::ObjectMessageFn(hSender, messageID, hRead);
			HandleDamage();

			return 0;
		}

 		case MID_TRIGGER:
		{
			HandleTrigger(hSender, hRead);
			break;
		}
    
		case MID_ADDAMMO:
		{
			if (!m_damage.IsDead())
			{
				if (!(m_dwFlags & FLAG_NOAMMOCOLLECT)) {
					DBYTE nAmmoType = m_pServerDE->ReadFromMessageByte(hRead);
					DFLOAT fAmmoCount = m_pServerDE->ReadFromMessageFloat(hRead);
					// Try to add ammo
					int iRet = m_InventoryMgr.AddAmmo(nAmmoType, fAmmoCount);

					if (iRet == CHWEAP_OK)
						m_InventoryMgr.SendPickedUpMessage(hSender, iRet);
				}
			}
		}
		break;

		default : break;
	}

	return CBaseCharacter::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::ReadProp
// DESCRIPTION	: read properties from ED file
// RETURN TYPE	: DBOOL 
// PARAMS		: BaseClass *pObject
// PARAMS		: ObjectCreateStruct *pStruct
// ----------------------------------------------------------------------- //

DBOOL AI_Mgr::ReadProp(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !pStruct) return DFALSE;	

	char buf[MAX_CS_FILENAME_LEN];

	pServerDE->GetPropReal("RandomHitPoints", &m_fAIRandomHP);
    
	buf[0] = '\0';
	pServerDE->GetPropString("AIState", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) _mbsncpy((unsigned char*)m_szAIState, (const unsigned char*)buf, 32);

	//SCHLEGZ 5/25/98 2:22:03 AM: new DEdit fields for the level designers
	for(int i = 1; i <= MAX_NUM_WEAPONS; i++)
	{
		char szTemp[32];

		sprintf(szTemp,"AIWeapon%d",i);

		buf[0] = '\0';
		pServerDE->GetPropString(szTemp, buf, MAX_CS_FILENAME_LEN);
		if (buf[0]) 
			_mbsncpy((unsigned char*)m_szAIWeapon[i - 1], (const unsigned char*)buf, 32);
	}

	pServerDE->GetPropReal("Bullets", &m_fAIBullets);
	pServerDE->GetPropReal("BMG", &m_fAIBMG);
	pServerDE->GetPropReal("Shells", &m_fAIShells);
	pServerDE->GetPropReal("Grenades", &m_fAIGrenades);
	pServerDE->GetPropReal("Rockets", &m_fAIRockets);
	pServerDE->GetPropReal("Flares", &m_fAIFlares);
	pServerDE->GetPropReal("Cells", &m_fAICells);
	pServerDE->GetPropReal("Charges", &m_fAICharges);
	pServerDE->GetPropReal("Fuel", &m_fAIFuel);
	pServerDE->GetPropReal("Proximity_Bombs", &m_fProxBombs);

	buf[0] = '\0';
	pServerDE->GetPropString("SpotTriggerTarget", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrSpotTriggerTarget = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("SpotTriggerMessage", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrSpotTriggerMessage = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("TriggerRelayTarget", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrTriggerRelayTarget = pServerDE->CreateString(buf);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::OnStringKey
// DESCRIPTION	: read and parse string keys
// RETURN TYPE	: void 
// PARAMS		: ArgList* pArgList
// ----------------------------------------------------------------------- //

void AI_Mgr::OnStringKey(ArgList* pArgList)
{
	if (!m_pServerDE || !pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	char szTemp[32];

	//fire current weapon 
	if(Sparam_Get(szTemp,pKey,"fire_key"))
	{
		if(m_hTarget == DNULL)
			return;

		Fire();
	}
	else if(Sparam_Get(szTemp,pKey,"show_weapon"))
	{
		char szTmp[8];
		int nNum = atoi(szTmp);

		CWeapon* pW = m_InventoryMgr.GetCurrentWeapon();

		if(pW)
			pW->ShowHandModel(nNum);
	}
	else if(Sparam_Get(szTemp,pKey,"extra_key"))
	{
		MC_Extra(szTemp);
	}
	else if(Sparam_Get(szTemp,pKey,"play_sound"))
	{
		//scripted scene? disable!!
		if(g_hActiveCamera && m_nState != STATE_Script ) // && m_nState != STATE_Passive && m_nState != STATE_Idle)
		{
			CameraObj* pCameraObj = (CameraObj*)m_pServerDE->HandleToObject(g_hActiveCamera);
			if(!pCameraObj->AllowPlayerMovement())
			{
				return;
			}
		}

		if(m_nState != STATE_Script)
		{
			char szSound[256];
			char szTmp[8];

			if(Sparam_Get(szTmp,pKey,"sound_random"))
			{
				int nMax = atoi(szTmp);
				int nNum = m_pServerDE->IntRandom(1,NRES(nMax));
				sprintf(szSound, "%s%d.wav", szTemp, nNum);
			}
			else
			{
				sprintf(szSound, "%s.wav", szTemp);
			}

			DFLOAT fRadius = 1000.0f;

			if(Sparam_Get(szTmp,pKey,"sound_radius"))
			{
				fRadius = (DFLOAT)atof(szTmp);
			}

			int nVol = 100;

			if(Sparam_Get(szTmp,pKey,"sound_volume"))
			{
				nVol = atoi(szTmp);
			}

			int nChance = 100;

			if(Sparam_Get(szTmp,pKey,"sound_chance"))
			{
				nChance = atoi(szTmp);
			}

			DBOOL bVoice = DFALSE;

			if(Sparam_Get(szTmp,pKey,"sound_voice"))
			{
				bVoice = atoi(szTmp);
			}

			if(IsRandomChance(nChance))
			{
				if(bVoice)
					m_pAnim_Sound->PlayVoice(m_hObject, szSound, fRadius, nVol);
				else
					m_pAnim_Sound->PlaySound(m_hObject, szSound, fRadius, nVol);
			}
		}
	}

	CBaseCharacter::OnStringKey(pArgList);
	
	return;
}


// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::HandleTrigger
// DESCRIPTION	: Handle MID_TRIGGER messages
// RETURN TYPE	: DBOOL 
// PARAMS		:  HOBJECT hSender
// PARAMS		: HMESSAGEREAD hRead
// ----------------------------------------------------------------------- //

DBOOL AI_Mgr::HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead )
{
	if (!m_hObject ) return DFALSE;
    
	HSTRING hMsg = m_pServerDE->ReadFromMessageHString(hRead);

//	char *pCommand = _strupr(m_pServerDE->GetStringData(hMsg));
	char *pCommand = m_pServerDE->GetStringData(hMsg);
	    
	int nArgs;
    
    char tokenSpace[64*20];
    char *pTokens[64];
    char *pCommandPos;

	// got a plain trigger message, relay it..
	if (_mbsicmp((const unsigned char*)TRIGGER, (const unsigned char*)pCommand) == 0 && m_hstrTriggerRelayTarget)
	{
		SendTriggerMsgToObjects(this, m_hstrTriggerRelayTarget, hMsg);
		g_pServerDE->FreeString( hMsg );
		return DTRUE;
	}
    
	DBOOL bContinue = DTRUE;
	while (bContinue)
	{
		bContinue = m_pServerDE->Parse(pCommand, &pCommandPos, tokenSpace, pTokens, &nArgs);

    	// Okay, see if we can handle the message...
		if(_mbsicmp((const unsigned char*)TRIGGER_SOUND, (const unsigned char*)pTokens[0]) == 0 && nArgs > 1)
		{
			if(hSender != m_hObject)
			{
				if(m_nState == STATE_Script && !(m_dwScriptFlags & AI_SCRFLG_INT))
				{
					m_pServerDE->FreeString( hMsg );
					return DTRUE;
				}

				int nId = atoi(pTokens[1]);

				switch(nId)
				{
					case SOUND_GUNFIRE:
					{		
						if(m_nState == STATE_Idle)
						{
							DVector vPos;
							m_pServerDE->GetObjectPos(hSender, &vPos);

							if(VEC_DIST(vPos, m_MoveObj.GetPos()) <= m_fHearingDist)
							{
								HCLASS hTest = m_pServerDE->GetClass("CPlayerObj");

								if(m_pServerDE->IsKindOf(hTest, m_pServerDE->GetObjectClass(hSender)))
								{
									VEC_COPY(m_vTargetPos, vPos);
									SetNewState(STATE_SearchVisualTarget);
								}
/*								else
								{
									AI_Mgr* pAI = (AI_Mgr*)m_pServerDE->HandleToObject(hSender);
									m_hTrackObject = pAI->GetTarget();

									if(VEC_DIST(vPos, m_MoveObj.GetPos()) > (m_fHearingDist * 0.1) 
										&& m_dwFlags & FLAG_CANASSIST)
									{
										VEC_COPY(m_vTrackObjPos, vPos);
										SetNewState(STATE_AssistAlly);
									}
									else
									{
										m_hTarget = m_hTrackObject;
										m_pServerDE->GetObjectPos(m_hTarget, &m_vTargetPos);
										MC_FaceTarget();
									}
								}
*/							}
						}

						break;
					}

					case SOUND_PLAYERSOUND:
					{		
						if(m_nState == STATE_Idle)
						{
							DVector vPos;
							m_pServerDE->GetObjectPos(hSender, &vPos);

							if(VEC_DIST(vPos, m_MoveObj.GetPos()) <= m_fHearingDist)
							{
								if(VEC_DIST(vPos, m_MoveObj.GetPos()) < m_fHearingDist/2)
								{
									VEC_COPY(m_vTargetPos, vPos);
									SetNewState(STATE_SearchVisualTarget);
								}
								else
								{
									MC_FacePos(vPos);
									SetNewState(STATE_Idle);
								}
							}
						}

						break;
					}

					case SOUND_GUNIMPACT:
					{	
						if(m_dwFlags & FLAG_DODGE)
						{
							if(m_nState != STATE_Dodge && m_nLastState != STATE_Dodge 
								&& m_nLastState != STATE_Passive)
							{
								m_vTrackObjPos.x = (DFLOAT)atof(pTokens[2]);
								m_vTrackObjPos.y = (DFLOAT)atof(pTokens[3]);
								m_vTrackObjPos.z = (DFLOAT)atof(pTokens[4]);

								SetNewState(STATE_Dodge);
							}
						}

						break;
					}
				}
			}
		}
    	else if (_mbsicmp((const unsigned char*)TRIGGER_PLAY_SOUND, (const unsigned char*)pTokens[0]) == 0 && nArgs > 1)
	    {
    		// Get sound name from message...
	    	char* pSoundName = pTokens[1];

		    if (pSoundName)
    		{
	    		// See if sound radius was in message..
		    	DFLOAT fRadius = 5000;
			    if (nArgs > 2 && pTokens[2])
    			{
	    			fRadius = (DFLOAT) atoi(pTokens[2]);
		    	}
    
	    		fRadius = fRadius > 0.0f ? fRadius : 1000.0f;
                PlaySoundFromObject( m_hObject, pSoundName, fRadius, SOUNDPRIORITY_MISC_HIGH, DFALSE, DFALSE, DFALSE );

				m_pServerDE->FreeString( hMsg );
                return DTRUE;
	    	}
    	}
	    else if (_mbsicmp((const unsigned char*)TRIGGER_SET_STATE, (const unsigned char*)pTokens[0]) == 0 && nArgs > 1)
    	{
	    	// Get state name from message...

		    char* pStateName = pTokens[1];
    		if (pStateName)
	    	{
                SetNewState(StateStrToInt(pStateName));
				m_pServerDE->FreeString( hMsg );
    			return DTRUE;
	    	}
    	}
	    else if (_mbsicmp((const unsigned char*)TRIGGER_TARGET_OBJECT, (const unsigned char*)pTokens[0]) == 0 && nArgs > 1)
    	{
	    	// Get target name from message...

		    char* pName = pTokens[1];
    		if (pName)
	    	{
		    	SetTarget(pName);
				SetNewState(STATE_AttackFar);
				m_pServerDE->FreeString( hMsg );
			    return DTRUE;
    		}
	    }
    	else if (_mbsicmp((const unsigned char*)TRIGGER_PLAY_ANIMATION, (const unsigned char*)pTokens[0]) == 0 && nArgs > 1) 
	    {
    		// Get ani name from message...

	    	char* pName = pTokens[1];
		    if (pName)
    		{
	    		PlayAnimation(pName);
				m_pServerDE->FreeString( hMsg );
		    	return DTRUE;
    		}
	    }
    	else if (_mbsicmp((const unsigned char*)TRIGGER_SCRIPT, (const unsigned char*)pTokens[0]) == 0)
	    {
		    // Get script type from message...

			m_dwScriptFlags &= ~AI_SCRFLG_INT;  // Not interruptable

			if (nArgs > 1)
			{
				char* pType = pTokens[1];
				if (pType)
				{
					if (_mbsicmp((const unsigned char*)TRIGGER_STYPE_INTERRUPTABLE, (const unsigned char*)pType) == 0)
					{
						m_dwScriptFlags |= AI_SCRFLG_INT;
					}
					else  // Assume the flags were specified as a number
					{
						m_dwScriptFlags = atoi(pType);
					}
				}
			}

	        m_scriptCmdList.RemoveAll();

//        	char* pCommand = pScriptBody;

            // process the Next Command
	        pCommand = pCommandPos;

        	int nArgs;
        	while (m_pServerDE->Parse(pCommand, &pCommandPos, tokenSpace, pTokens, &nArgs))
        	{
		        if (nArgs != 2)
				{
					m_pServerDE->FreeString( hMsg );
					return DFALSE;
				}

        		AISCRIPTCMD* pCmd = new AISCRIPTCMD;
		        if (!pCmd) 
				{
					m_pServerDE->FreeString( hMsg );
					return DFALSE;
				}

        		pCmd->command = StringToAIScriptCmdType(pTokens[0]);

		        char* pArgs = pTokens[1];
        		if (pArgs) _mbsncpy((unsigned char*)pCmd->args, (const unsigned char*)pArgs, MAX_AI_ARGS_LENGTH);

		        m_scriptCmdList.Add(pCmd);

        		pCommand = pCommandPos;
        	}

            // Set the State to Script
            SetNewState(STATE_Script);
        	m_bUpdateScriptCmd = DTRUE;
			m_nScriptCmdIndex  = 0;

    		// Force a Update
	    	AI_Update();     
            
    	} // else if


	    pCommand = pCommandPos;
        
	} // While commands

	m_pServerDE->FreeString( hMsg );

	return DFALSE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::PlayAnimation
// DESCRIPTION	: 
// RETURN TYPE	: void 
// PARAMS		: char* pAniName
// ----------------------------------------------------------------------- //

void AI_Mgr::PlayAnimation(char* pAniName)
{
	DDWORD dwAniIndex = m_pServerDE->GetAnimIndex(m_hObject, pAniName);

	m_pServerDE->SetModelLooping(m_hObject, DFALSE);
//	m_pServerDE->SetModelAnimation(m_hObject, dwAniIndex);
	SetAnimation(dwAniIndex);
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::SetTarget
// DESCRIPTION	: 
// RETURN TYPE	: void 
// PARAMS		: char* pTargetName
// ----------------------------------------------------------------------- //

void AI_Mgr::SetTarget(char* pTargetName)
{
	ObjectList*	pList = m_pServerDE->FindNamedObjects(pTargetName);
	if (!pList) return;

	ObjectLink* pLink = pList->m_pFirstLink;
	if (pLink)
	{
		SetNewTarget(pLink->m_hObject);

		m_pServerDE->GetObjectPos(m_hTarget, &m_vTargetPos);
		MC_FaceTarget();
		Metacmd--;
	}

	m_pServerDE->RelinquishList(pList);
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::SetNewTarget
// DESCRIPTION	: 
// RETURN TYPE	: void 
// PARAMS		: HOBJECT hNewTarget
// ----------------------------------------------------------------------- //

void AI_Mgr::SetNewTarget(HOBJECT hNewTarget)
{
	if (m_hTarget != hNewTarget)
	{
		if (m_hTarget)
		{
			m_pServerDE->BreakInterObjectLink(m_hObject, m_hTarget);
		}

		m_hTarget = hNewTarget;

		if (m_hTarget)
		{
			m_pServerDE->CreateInterObjectLink(m_hObject, m_hTarget);
		}
	}
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::UpdateScriptCommand
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::UpdateScriptCommand()
{
	m_bUpdateScriptCmd = DFALSE;

	m_curScriptCmd.command = AI_SCMD_DONE;

	int nNumItems = m_scriptCmdList.GetNumItems();
	if (nNumItems > 0)
	{
		m_curScriptCmd = *(m_scriptCmdList[m_nScriptCmdIndex]);
		
		// Determine what the next script cmd index should be...

		if (m_dwScriptFlags & AI_SCRFLG_LOOP)
		{
			m_nScriptCmdIndex = (m_nScriptCmdIndex + 1 < nNumItems) ? m_nScriptCmdIndex + 1 : 0;
		}
		else  // Non-looping, remove current script command...
		{
			m_nScriptCmdIndex = 0;
			m_scriptCmdList.Remove(0);
		}


		switch(m_curScriptCmd.command)
		{
			case AI_SCMD_SETMOVEMENT:
				SetSetMovementCmd();
			break;

			case AI_SCMD_FOLLOWPATH:
				SetFollowPathCmd();
			break;
			
			case AI_SCMD_PLAYSOUND:
				SetPlaysoundCmd();
			break;
			
			case AI_SCMD_SETSTATE:
				SetSetStateCmd();
			break;
			
			case AI_SCMD_TARGET:
				SetTargetCmd();
			break;
			
			case AI_SCMD_WAIT:
				SetWaitCmd();
			break;
			
			case AI_SCMD_PLAYANIMATION:
				SetPlayAnimationCmd(DFALSE);
			break;
			
			case AI_SCMD_PLAYANIMATION_LOOPING:
				SetPlayAnimationCmd(DTRUE);
			break;
			
			case AI_SCMD_MOVETOOBJECT:
				SetMoveToObjectCmd();
			break;
			
			case AI_SCMD_DONE:
			default: 
				m_dwScriptFlags	= 0;
				ComputeState();
			break;
		}
	}
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::SetSetMovementCmd
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::SetSetMovementCmd()
{
	if (_mbsicmp((const unsigned char*)m_curScriptCmd.args, (const unsigned char*)SCRIPT_MOVEMENT_WALK) == 0)
	{
		m_eScriptMovement = SM_WALK;
	}
	else if (_mbsicmp((const unsigned char*)m_curScriptCmd.args, (const unsigned char*)SCRIPT_MOVEMENT_RUN) == 0)
	{
		m_eScriptMovement = SM_RUN;
	}	
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::SetFollowPathCmd
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::SetFollowPathCmd()
{
	m_AIPathList.RemoveAll();

	if (!g_pBloodServerShell) return;

	PathMgr* pPathMgr = g_pBloodServerShell->GetPathMgr();
	if (!pPathMgr) return;
	
	pPathMgr->GetPath(m_curScriptCmd.args, &m_AIPathList);

	Metacmd = 1;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::UpdateFollowPathCmd
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::UpdateFollowPathCmd()
{
	if (m_AIPathList.IsEmpty())
	{
		m_bUpdateScriptCmd = DTRUE;
		return;
	}

	PathListData* pCurKey = m_AIPathList[0];
	if (!pCurKey)
	{
		m_bUpdateScriptCmd = DTRUE;
		return;
	}

	DVector vTargetPos;
	VEC_COPY(vTargetPos, pCurKey->m_vPos);

	DVector vPos;
	VEC_COPY(vPos, m_MoveObj.GetPos());

	// Don't let y be a factor...
	vTargetPos.y = vPos.y;

	MC_FacePos(vTargetPos);

	DFLOAT fSpeed = m_fWalkSpeed;
	if (m_eScriptMovement == SM_RUN)
	{
		fSpeed = m_fRunSpeed;
	}

	DFLOAT fDistLeft = VEC_DIST(vTargetPos, vPos);
	DFLOAT fDistPredict = m_pServerDE->GetFrameTime() * fSpeed;

	if (fDistLeft <= fDistPredict * 2.0f)
	{
		// Move to the position for completeness
		if(!m_nTrapped)
			m_pServerDE->MoveObject(m_hObject, &vTargetPos);

		// Check for Target and messages from this pathpoint...
		if (pCurKey->m_hstrActionMessage)
		{
			// If there is a target, use that.  If not, send to myself.
			if (pCurKey->m_hstrActionTarget)
			{
				SendTriggerMsgToObjects(this, 
										pCurKey->m_hstrActionTarget, 
										pCurKey->m_hstrActionMessage);
			}
			else
			{
				HMESSAGEWRITE hMessage;

				hMessage = m_pServerDE->StartMessageToObject(this, m_hObject, MID_TRIGGER);
				m_pServerDE->WriteToMessageHString(hMessage, pCurKey->m_hstrActionMessage);
				m_pServerDE->EndMessage(hMessage);
			}
		}

		// Remove it from the pathpoint list
		m_AIPathList.Remove(0);
	}
	else
	{
		DBOOL bOkToMove = DTRUE;
		if (m_dwScriptFlags & AI_SCRFLG_OPPORTFIRE)
		{
			bOkToMove = !Script_Fire_Stand();
		}

		if (bOkToMove)
		{
			if (m_eScriptMovement == SM_RUN)
			{
				Script_Run();
			}
			else
			{
				Script_Walk();
			}
		}
	}

}


// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::SetMoveToObjectCmd
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::SetMoveToObjectCmd()
{
	char *pTargetName = m_curScriptCmd.args;
	ObjectList*	pList = m_pServerDE->FindNamedObjects(pTargetName);
	if (!pList) return;

	ObjectLink* pLink = pList->m_pFirstLink;
	if (pLink)
	{
		SetNewTarget(pLink->m_hObject);
	}
	else
	{
		m_bUpdateScriptCmd = DTRUE;
	}

	m_pServerDE->RelinquishList(pList);

	Metacmd = 1;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::UpdateMoveToObjectCmd
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::UpdateMoveToObjectCmd()
{
	// Face where we are going...

	if (!m_hTarget)
	{
		m_bUpdateScriptCmd = DTRUE;
		return;
	}

	DVector vTargetPos;
	m_pServerDE->GetObjectPos(m_hTarget, &vTargetPos);
    
 	DVector vPos;
	VEC_COPY(vPos, m_MoveObj.GetPos());

	// Don't let y be a factor...
	vTargetPos.y = vPos.y;

   switch(Metacmd)
    {
		case 1:
		{
			m_MoveObj.CalculatePath(vTargetPos);
			Metacmd++;									
		}
		break;
		
		case 2:		
		{
			MC_FacePos(*m_MoveObj.GetNextPathPoint());
		}
		break;

		case 3:		
		{
			DFLOAT fSpeed = m_fWalkSpeed;
			if (m_eScriptMovement == SM_RUN)
			{
				fSpeed = m_fRunSpeed;
			}

			DFLOAT fDistLeft = VEC_DIST(*m_MoveObj.GetNextPathPoint(), vPos);
			DFLOAT fDistPredict = m_pServerDE->GetFrameTime() * fSpeed;

			if (fDistLeft <= fDistPredict * 2.0f)
			{
				if (!m_MoveObj.MoveToNextPathPoint())
				{
					Metacmd++;
				}
				else
				{
					Metacmd = 2;
				}
			}
			else
			{
				if (m_eScriptMovement == SM_RUN)
				{
					if (m_dwScriptFlags & AI_SCRFLG_OPPORTFIRE)
					{
						Script_Fire_Run();
					}
					else
					{
						Script_Run();
					}
				}
				else
				{
					if (m_dwScriptFlags & AI_SCRFLG_OPPORTFIRE)
					{
						Script_Fire_Walk();
					}
					else
					{
						Script_Walk();
					}
				}

				Metacmd = 2;
			}
		}
		break;
						
		case 4:		
		{
			// Move to the position for completeness
			if(!m_nTrapped)
				m_pServerDE->MoveObject(m_hObject, &vTargetPos);
			m_bUpdateScriptCmd = DTRUE;
		}
		break;
	}
}


// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::SetPlaysoundCmd
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::SetPlaysoundCmd()
{
	char* pSoundName = m_curScriptCmd.args;
	if (pSoundName) 
    {
        PlaySoundFromObject( m_hObject, pSoundName, 5000.0f, SOUNDPRIORITY_MISC_HIGH, DFALSE, DFALSE, DFALSE );
    }                          
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::SetSetStateCmd
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::SetSetStateCmd()
{
    SetNewState(StateStrToInt(m_curScriptCmd.args));
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::SetTargetCmd
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::SetTargetCmd()
{
	SetTarget(m_curScriptCmd.args);
	SetNewState(STATE_AttackFar);
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::SetWaitCmd
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::SetWaitCmd()
{
	m_fScriptWaitEnd = m_pServerDE->GetTime() + atoi(m_curScriptCmd.args);
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::UpdateWaitCmd
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::UpdateWaitCmd()
{
	if (m_pServerDE->GetTime() >= m_fScriptWaitEnd)
	{
		m_bUpdateScriptCmd = DTRUE;
	}
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::SetPlayAnimationCmd
// DESCRIPTION	: 
// RETURN TYPE	: void 
// PARAMS		: DBOOL bLooping - Animation should be played looping.
// ----------------------------------------------------------------------- //

void AI_Mgr::SetPlayAnimationCmd(DBOOL bLooping)
{
	DDWORD dwIndex = m_pServerDE->GetAnimIndex(m_hObject, m_curScriptCmd.args);
	m_pServerDE->SetModelLooping(m_hObject, bLooping);
//	m_pServerDE->SetModelAnimation(m_hObject, dwIndex);
	SetAnimation(dwIndex);
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::UpdatePlayAnimationCmd
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::UpdatePlayAnimationCmd()
{
	DDWORD dwState = m_pServerDE->GetModelPlaybackState(m_hObject);

	if ((dwState & MS_PLAYDONE))
	{
		m_bUpdateScriptCmd = DTRUE;
	}
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::FacePos
// DESCRIPTION	: 
// RETURN TYPE	: void 
// PARAMS		: DVector vTargetPos
// ----------------------------------------------------------------------- //

void AI_Mgr::FacePos(DVector vTargetPos)
{
	DVector vDir, vPos;

	// GJK 7/29/98 - Made this puppy much simpler.  More importantly, it works now.

	m_pServerDE->GetObjectPos(m_hObject, &vPos);
	vTargetPos.y = vPos.y; // Don't look up/down.

	VEC_SUB(vDir, vTargetPos, vPos);
	VEC_NORM(vDir);

	DRotation rRot;
	m_pServerDE->AlignRotation(&rRot, &vDir, NULL);
	m_pServerDE->SetObjectRotation(m_hObject, &rRot);
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::HandleDamage
// DESCRIPTION	: Handle MID_DAMAGE messages
// RETURN TYPE	: void 
// PARAMS		: HOBJECT hSender
// PARAMS		: DDWORD messageID
// PARAMS		: HMESSAGEREAD hRead
// ----------------------------------------------------------------------- //

DBOOL AI_Mgr::HandleDamage()
{
	if(m_nState == STATE_Dying || m_bRemoveMe)
		return DTRUE;

	//should we limp?
	int nNodeHit = m_damage.GetNodeHit();

	//didn't hit so return
	if(nNodeHit == -1)
		return DTRUE;

	int nType = m_damage.GetLastDamageType();
	DFLOAT fDamage = m_damage.GetLastDamageAmount();

	DVector vDir;
	m_damage.GetLastDamageDirection(&vDir);

	if(m_dwFlags & FLAG_LIMP && m_nInjuredLeg == 0 && 
	   (nNodeHit == NODE_LLEG || nNodeHit == NODE_RLEG))
	{
		if(m_damage.GetHitPoints() < (m_damage.GetMaxHitPoints() * 0.5f))
		{
			m_nInjuredLeg = nNodeHit;
		}
	}

	//SCHLEGZ 3/10/98 12:44:32 AM: check for limb loss
	if (m_damage.GetHitPoints() <= 0.0f) 
	{
		if(m_bCabal)
			m_InventoryMgr.DropCurrentWeapon();		//SCHLEGZ 4/26/98 10:14:32 PM: i'm dead; no need for weapon

		//see what we are standing on
		CollisionInfo collisionInfo;
		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);

		if(collisionInfo.m_hObject && (collisionInfo.m_hObject != m_pServerDE->GetWorldObject()))
		{
			DDWORD dwFlags = m_pServerDE->GetObjectFlags(collisionInfo.m_hObject);

			if(!(dwFlags & FLAG_SOLID))
			{
				m_dwFlags |= FLAG_ALWAYSGIB;		
			}
		}

		//do we need to start the barbecue?
		if(nType == DAMAGE_TYPE_FIRE && !(m_dwFlags & FLAG_ALWAYSGIB))
		{
			ObjectCreateStruct ocStruct;
			INIT_OBJECTCREATESTRUCT(ocStruct);

			ocStruct.m_ObjectType = OT_NORMAL;
			ocStruct.m_NextUpdate = 0.01f;
			m_pServerDE->GetModelNodeTransform(m_hObject, "torso",&ocStruct.m_Pos,&ocStruct.m_Rotation);
			ocStruct.m_Flags = FLAG_FORCECLIENTUPDATE;
			
			HCLASS hClass = m_pServerDE->GetClass("BaseClass");
			BaseClass* pObj = m_pServerDE->CreateObject(hClass, &ocStruct);

			if(pObj)
			{
				m_hFireSource = pObj->m_hObject;
			}
		}
		else if((nType == DAMAGE_TYPE_EXPLODE || m_dwFlags & FLAG_ALWAYSGIB) && !(m_dwFlags & FLAG_NEVERGIB))
		{
			CreateGibs(vDir, ((int)m_damage.GetMass())>>5, nType, fDamage);

			m_bRemoveMe = DTRUE;

			if (IsRandomChance(7) && (m_damage.GetWhoKilledMeLast() == g_pPlayerObj->m_hObject))
			{
				g_pPlayerObj->PlayVoiceGroupEventOnClient(VME_BIGGIB, DTRUE);	// [blg]
			}
		}
		else if((m_damage.GetLastDamagePercent() >= 0.90f) && !(m_dwFlags & FLAG_NEVERGIB))
		{
			CreateGibs(vDir, ((int)m_damage.GetMass())>>5, nType, fDamage);

			m_bRemoveMe = DTRUE;

			if (IsRandomChance(7) && (m_damage.GetWhoKilledMeLast() == g_pPlayerObj->m_hObject))
			{
				g_pPlayerObj->PlayVoiceGroupEventOnClient(VME_BIGGIB, DTRUE);	// [blg]
			}
		}
/*		else if(nNodeHit > 0 && m_damage.GetLastDamagePercent() >= 0.25f)	// 10% of the time, blow off the head.  25% for other limbs
		{
			if(AIShared.HideLimb(m_hObject,nNodeHit) && (m_dwFlags & FLAG_LIMBLOSS))
			{
				AIShared.CreateLimb(m_hObject, nNodeHit, vDir);
			}

		}
*/
		if (IsRandomChance(6) && (m_damage.GetWhoKilledMeLast() == g_pPlayerObj->m_hObject))
		{
			g_pPlayerObj->PlayVoiceGroupEventOnClient(VME_KILL, DTRUE);	// [blg]
		}
	}

	if (m_nState == STATE_Script)
	{
		if(!(m_dwScriptFlags & AI_SCRFLG_INT))
			return DTRUE;
	}

	if(m_dwFlags & FLAG_ALWAYSRECOIL)
		SetNewState(STATE_Recoil);
	else if(fDamage >= m_damage.GetHitPoints() * 0.25f && m_nState != STATE_Teleport && m_nState != STATE_Dodge)	
		SetNewState(STATE_Recoil);
	else if(m_nState != STATE_AttackClose && m_nState != STATE_AttackFar)
		ComputeState();

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::SetNewState
// DESCRIPTION	: Set new state and reset metacmd 
// RETURN TYPE	: void 
// PARAMS		: int nState
// ----------------------------------------------------------------------- //

void AI_Mgr::SetNewState(int nState)
{
	float fTime;

	if(m_nState != STATE_WalkAroundObj && m_nState != STATE_RunAroundObj
	   && m_nState != STATE_JumpOverObj && m_nState != STATE_CrawlUnderObj
	   && m_nState != STATE_SearchVisualTarget && m_nState != STATE_SearchSmellTarget)
	{
		if(m_nLastState != m_nState)
			m_nLastState = m_nState; 
	}

	fTime = g_pServerDE->GetTime( );

	// Don't switch to the idle state too fast
	if( nState == STATE_Idle && m_bHasTargeted )
	{
		if( fTime > m_fWaitForIdleTime )
			m_nState = nState;
		else
			m_nState = STATE_SearchVisualTarget;
	}
	else
	{
		m_nState = nState;
		m_fWaitForIdleTime = fTime + WAITFORIDLETIME;
	}
	Metacmd = 1; 
	m_nCurMetacmd = 999;
	m_bAnimating = DFALSE;

	StopVelocity();

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SetAnimation
//
//	PURPOSE:	Set the current animation
//
// ----------------------------------------------------------------------- //
DBOOL AI_Mgr::SetAnimation(DDWORD nAni)
{
	if(nAni == m_nCurAnimation)
	{
	    m_pServerDE->ResetModelAnimation(m_hObject);
//		pServerDE->SetObjectDims(m_hObject,&m_vDims);
	}
	else
	{
		DVector vDims;
		VEC_COPY(vDims, m_vDims);		//safety

		if(m_pServerDE->GetModelAnimUserDims(m_hObject, &vDims, nAni) == DE_OK)
		{
			vDims.x *= m_vScale.x;
			vDims.y *= m_vScale.y;
			vDims.z *= m_vScale.z;
		}
		
		m_pServerDE->SetObjectDims2(m_hObject,&vDims);
		m_pServerDE->SetModelAnimation(m_hObject,nAni);
		VEC_COPY(m_vDims,vDims);
		m_nCurAnimation = nAni;
	}

    return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::HandleTouch
// DESCRIPTION	: Handle MID_TOUCHNOTIFY messages
// RETURN TYPE	: void 
// PARAMS		: HOBJECT hObj
// ----------------------------------------------------------------------- //

void AI_Mgr::HandleTouch(HOBJECT hObj)
{
	if (!m_hObject || !hObj) return;

	HCLASS hClass = m_pServerDE->GetObjectClass(hObj);

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::InsertMe
// DESCRIPTION	: Insert to appropriate list
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //
void AI_Mgr::InsertMe()
{
	// insert it into the list
	if(m_bCabal)
	{
		dl_Insert( &m_CabalHead, &m_Link );
		m_Link.m_pData = ( void * )this;
		m_dwNumCabal++;
	}
	else
	{
		dl_Insert( &m_MonsterHead, &m_Link );
		m_Link.m_pData = ( void * )this;
		m_dwNumMonster++;
	}
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_Init
// DESCRIPTION	: Initial update
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::InitialUpdate(int nData)
{
	//initialize the inventory aggregate
	m_InventoryMgr.Init(m_hObject);
	//initialize the damage aggregate
	m_damage.Init(m_hObject, &m_InventoryMgr, m_pAnim_Sound);

	// Only init stuff needed when restoring a savegame.
	if (nData == INITIALUPDATE_SAVEGAME)
	{
		return;
	}

	m_InventoryMgr.SetStrength(m_nAIStrength);

	for(int i =0; i < 5; i++)
	{
		int nWeap = WeaponStrToInt(m_szAIWeapon[i]);

		if(!m_InventoryMgr.HasWeapon(nWeap))
			m_InventoryMgr.ObtainWeapon(nWeap);
	}

	m_InventoryMgr.ChangeWeapon(WeaponStrToInt(m_szAIWeapon[0]));

	m_InventoryMgr.AddAmmo(AMMO_BULLET, m_fAIBullets);
	m_InventoryMgr.AddAmmo(AMMO_BMG, m_fAIBMG);
	m_InventoryMgr.AddAmmo(AMMO_SHELL, m_fAIShells);
	m_InventoryMgr.AddAmmo(AMMO_DIEBUGDIE, m_fAIGrenades);
	m_InventoryMgr.AddAmmo(AMMO_HOWITZER, m_fAIRockets);
	m_InventoryMgr.AddAmmo(AMMO_FLARE, m_fAIFlares);
	m_InventoryMgr.AddAmmo(AMMO_BATTERY, m_fAICharges);
	m_InventoryMgr.AddAmmo(AMMO_BATTERY, m_fAICells);
	m_InventoryMgr.AddAmmo(AMMO_FUEL, m_fAIFuel);
	m_InventoryMgr.AddAmmo(AMMO_FOCUS, 200.0f);
	m_InventoryMgr.AddInventoryWeapon(INV_PROXIMITY, (int)m_fProxBombs);

	switch(GetGameDifficulty())
	{
		case DIFFICULTY_EASY:		m_InventoryMgr.AddDamageMultiplier(0.25f);	break;
		case DIFFICULTY_MEDIUM:		m_InventoryMgr.AddDamageMultiplier(1.0f);	break;
		case DIFFICULTY_HARD:		m_InventoryMgr.AddDamageMultiplier(1.5f);	break;
	}

	//Set hit points
	DFLOAT fHP = m_fAIHitPoints + m_pServerDE->Random(1,m_fAIRandomHP);

	switch(GetGameDifficulty())
	{
		case DIFFICULTY_EASY:		fHP *= 0.25f;	break;
		case DIFFICULTY_MEDIUM:		fHP *= 1.0f;	break;
		case DIFFICULTY_HARD:		fHP *= 1.5f;	break;
	}

	m_damage.SetHitPoints(fHP);
	m_damage.SetMaxHitPoints(m_damage.GetHitPoints());
	m_damage.SetMaxMegaHitPoints(m_damage.GetHitPoints());

	//Set armor point
	switch(GetGameDifficulty())
	{
		case DIFFICULTY_EASY:		m_fAIArmorPoints *= 0.25f;	break;
		case DIFFICULTY_MEDIUM:		m_fAIArmorPoints *= 1.0f;	break;
		case DIFFICULTY_HARD:		m_fAIArmorPoints *= 1.5f;	break;
	}

	m_damage.SetArmorPoints(m_fAIArmorPoints);
	m_damage.SetMaxArmorPoints(m_fAIArmorPoints);
	m_damage.SetMaxNecroArmorPoints(m_fAIArmorPoints);
	m_damage.SetMass(m_fAIMass);

	// Added by Kevin S. 9/29/98 to better support scripted AIs...
	m_pServerDE->SetBlockingPriority(m_hObject, BLOCKPRIORITY_AI);

	m_MoveObj.Init(m_hObject);

    // Set the State
    m_nState = StateStrToInt(m_szAIState);
	if(m_nState == STATE_GuardLocation)
		m_pServerDE->GetObjectPos(m_hObject, &m_vGuardPos);

	DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
	dwFlags |= FLAG_VISIBLE | FLAG_SOLID | FLAG_TOUCH_NOTIFY | FLAG_GRAVITY | FLAG_STAIRSTEP |
			   FLAG_SHADOW | FLAG_MODELKEYS | FLAG_REMOVEIFOUTSIDE;
	m_pServerDE->SetObjectFlags(m_hObject, dwFlags);

	// insert it into the list
	if(m_bCabal)
	{
		dl_Insert( &m_CabalHead, &m_Link );
		m_Link.m_pData = ( void * )this;
		m_dwNumCabal++;
	}
	else
	{
		dl_Insert( &m_MonsterHead, &m_Link );
		m_Link.m_pData = ( void * )this;
		m_dwNumMonster++;
	}

	//randomize the size/weight
	DFLOAT fScale = m_pServerDE->Random(-0.05f, 0.1f);
	VEC_SET(m_vScale,m_vScale.x - fScale, m_vScale.y + fScale, m_vScale.z - fScale);
	m_pServerDE->ScaleObject(m_hObject,&m_vScale);

	SetAnimation(0);

	m_pServerDE->SetNextUpdate(m_hObject, 0.001f);

	//SCHLEGZ 4/27/98 2:51:47 PM: for vis list activation
	m_bSetShutdown = DFALSE;

	// Greg 9/28/98 Move this object to the ground
	if (m_bMoveToGround)
		MoveObjectToGround(m_hObject);

	m_fLastUpdate = m_pServerDE->GetTime();

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_Update
// DESCRIPTION	: Main update function
// RETURN TYPE	: DBOOL 
// ----------------------------------------------------------------------- //

DBOOL AI_Mgr::AI_Update()
{
	if (!m_hObject || m_bRemoveMe) return DFALSE;
	
	//this is so we can desync activating AI and lessen tick spikes
	if(m_pServerDE->GetTime() - m_fLastUpdate >= 1.0f && m_bSetShutdown)
	{
		m_fLastUpdate = m_pServerDE->GetTime();
		m_pServerDE->SetNextUpdate(m_hObject, m_pServerDE->Random(0.01f,0.5f));
		return DTRUE;
	}

	//de-sync method for optimizing AI ticks per frame
/*	DFLOAT fFrameTime = m_pServerDE->GetFrameTime();
	if(fFrameTime == m_fLastFrame)
	{	
		m_pServerDE->SetNextUpdate(m_hObject, m_pServerDE->Random(fFrameTime,fFrameTime * 10));
		return DTRUE;
	}

	m_fLastFrame = fFrameTime;
*/	m_fLastUpdate = m_pServerDE->GetTime();
	
	m_pServerDE->SetNextUpdate(m_hObject, 0.001f);

	//scripted scene? disable!!
	if(g_hActiveCamera && m_nState != STATE_Script && m_nState != STATE_Passive && m_nState != STATE_Idle)
	{
		CameraObj* pCameraObj = (CameraObj*)m_pServerDE->HandleToObject(g_hActiveCamera);
		if(!pCameraObj->AllowPlayerMovement())
		{
			StopVelocity();
			return DTRUE;
		}
	}

	//SCHLEGZ 4/27/98 2:51:47 PM: for vis list activation
	if(!m_bSetShutdown)
	{
		m_pServerDE->SetDeactivationTime(m_hObject,3.0f);
		m_pServerDE->SetObjectState(m_hObject, OBJSTATE_AUTODEACTIVATE_NOW);
		m_bSetShutdown = DTRUE;
	}

	if(m_hTarget)
	{
		// This stuff helps the ai keep the target.  It was getting too easy to lose the target.
		m_bHasTargeted = DTRUE;
		if( m_fSeeingDist > m_fSensingDist && m_fSensingDist > 0.0f )
			m_fSensingDist = m_fSeeingDist;

		m_pServerDE->GetObjectPos(m_hTarget,&m_vTargetPos);

		CBaseCharacter* pAI = (CBaseCharacter*)m_pServerDE->HandleToObject(m_hTarget);
		if(pAI->IsDead())
		{
			m_hTarget = DNULL;
			SetNewState(STATE_Idle);
		}
		else
		{
			if (!(m_dwFlags & FLAG_ALWAYSCLEAR))
			{
				if( !CanSee( m_hTarget ))
				{
					m_hTarget = DNULL;
				}
			}
		}
	}

	if(m_hTrackObject)
		m_pServerDE->GetObjectPos(m_hTrackObject,&m_vTrackObjPos);

    // If we are DYING... then start that STATE!
    if(m_damage.GetHitPoints() <= 0.0f && m_nState != STATE_Dying)
		SetNewState(STATE_Dying);

	//see what we are standing on
	CollisionInfo collisionInfo;
	m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);

	if(collisionInfo.m_hObject && (collisionInfo.m_hObject != m_pServerDE->GetWorldObject()))
	{
		HCLASS hCollClass = m_pServerDE->GetObjectClass(collisionInfo.m_hObject);
		HCLASS hBaseChar = m_pServerDE->GetClass("CBaseCharacter");

		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);

		if(m_pServerDE->IsKindOf(hCollClass, hBaseChar))
		{
			dwFlags |= FLAG_DONTFOLLOWSTANDING;		
		}
		else
		{
			dwFlags &= ~FLAG_DONTFOLLOWSTANDING;		
		}

		m_pServerDE->SetObjectFlags(m_hObject, dwFlags);
	}

	//sanity check
	if(Metacmd <= 0)
		Metacmd = 1;

	switch(m_nState)
    {
		case STATE_AttackClose:			AI_STATE_AttackClose();		break;
		case STATE_AttackFar:			AI_STATE_AttackFar();		break;
		case STATE_SearchVisualTarget:	AI_STATE_SearchVisualTarget();	break;
		case STATE_Escape_Hide:			AI_STATE_Escape_Hide();		break;
		case STATE_Escape_RunAway:		AI_STATE_Escape_RunAway();	break;
		case STATE_GuardLocation:		AI_STATE_GuardLocation();	break;
		case STATE_Idle:				AI_STATE_Idle();			break;
		case STATE_Dodge:				AI_STATE_Dodge();			break;
		case STATE_Teleport:			AI_STATE_Teleport();		break;
		case STATE_EnemyAttach:			AI_STATE_EnemyAttach();		break;
		case STATE_AssistAlly:			AI_STATE_AssistAlly();		break;
		case STATE_SearchSmellTarget:	AI_STATE_SearchSmellTarget();	break;
		case STATE_FindAmmo:			AI_STATE_FindAmmo();		break;
		case STATE_FindHealth:			AI_STATE_FindHealth();		break;
		case STATE_Special1:			AI_STATE_Special1();		break;
		case STATE_Special2:			AI_STATE_Special2();		break;
		case STATE_Special3:			AI_STATE_Special3();		break;
		case STATE_Special4:			AI_STATE_Special4();		break;
		case STATE_Special5:			AI_STATE_Special5();		break;
		case STATE_Special6:			AI_STATE_Special6();		break;
		case STATE_Special7:			AI_STATE_Special7();		break;
		case STATE_Special8:			AI_STATE_Special8();		break;
		case STATE_Passive:				AI_STATE_Passive();			break;

		case STATE_Dying:				AI_STATE_Dying();			break;
		case STATE_Recoil:				AI_STATE_Recoil();			break;
		case STATE_WalkAroundObj:		AI_STATE_WalkAroundObj();	break;
		case STATE_RunAroundObj:		AI_STATE_RunAroundObj();	break;
		case STATE_JumpOverObj:			AI_STATE_JumpOverObj();		break;
		case STATE_CrawlUnderObj:		AI_STATE_CrawlUnderObj();	break;
		case STATE_WalkToPos:			AI_STATE_WalkToPos();		break;
		case STATE_RunToPos:			AI_STATE_RunToPos();		break;
		case STATE_Script:				AI_STATE_Script();			break;

        case STATE_Inactive:			return DTRUE;				break;
        default:						AI_STATE_Idle();			break;
    
    }

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::ComputeStimuli 
// DESCRIPTION	: fill in stimuli set
// RETURN TYPE	: DBOOL 
// ----------------------------------------------------------------------- //

int AI_Mgr::ComputeStimuli()
{
	if (!m_hObject) return DFALSE;

	HOBJECT	hSightTarget = DNULL,hSmellTarget = DNULL, hSenseTarget = DNULL;
	DVector vSight, vSmell, vSense;
	HOBJECT hNewTarget = DNULL;
	int nStimType = 0;

	//Check if hit recently
	if(( hNewTarget = m_damage.GetLastDamager()) != DNULL )
	{
		if( !IsBaseCharacter( hNewTarget ))
			m_damage.ClearLastDamager( );

		nStimType = STIM_SIGHT;
	}
	else
	{
		VEC_COPY(vSight, m_MoveObj.GetPos());
		VEC_COPY(vSmell, m_MoveObj.GetPos());
		VEC_COPY(vSense, m_MoveObj.GetPos());

		//Zero out the stimulus set
		memset(&m_fStimuli,0,sizeof(m_fStimuli));

		// Release current target if any
		SetNewTarget(DNULL);

		HCLASS hEnemyTest = DNULL;
		HCLASS hThis = m_pServerDE->GetObjectClass(m_hObject);

		if(m_pServerDE->IsKindOf(hThis, m_pServerDE->GetClass("BoneLeech")) ||
		   m_pServerDE->IsKindOf(hThis, m_pServerDE->GetClass("TheHandAI")) ||
		   m_pServerDE->IsKindOf(hThis, m_pServerDE->GetClass("Thief")))
		{
			hEnemyTest = m_pServerDE->GetClass("CPlayerObj");
		}
		else
		{
			hEnemyTest = m_pServerDE->GetClass("AI_Mgr");
		}

		HOBJECT hSightObj = FindObjectInRadius(hEnemyTest, m_fSeeingDist, FIND_VISIBLE | FIND_FACE_OBJECT);
		HOBJECT hSmellObj = FindObjectInRadius(m_pServerDE->GetClass("SmellHint"), m_fSmellingDist);
		HOBJECT hSenseObj = FindObjectInRadius(hEnemyTest, m_fSensingDist);

		if(hSightObj)
			m_pServerDE->GetObjectPos(hSightObj, &vSight);
		
		if(hSmellObj)
			m_pServerDE->GetObjectPos(hSmellObj, &vSmell);
		
		if(hSenseObj)
			m_pServerDE->GetObjectPos(hSenseObj, &vSense);

		DFLOAT fSight = VEC_DIST(vSight,m_MoveObj.GetPos());
		DFLOAT fSmell = VEC_DIST(vSmell,m_MoveObj.GetPos());
		DFLOAT fSense = VEC_DIST(vSense,m_MoveObj.GetPos());

		//Check smell
		if(hSmellObj && m_fSmellingDist > 0 && fSmell <= m_fSmellingDist)
		{
			m_fStimuli[SMELL]	= 1.0f - (fSmell/m_fSmellingDist);
			hNewTarget = hSmellObj;
			nStimType = STIM_SMELL;

			if(m_fStimuli[SMELL] <= 0.0f)
			{
				m_fStimuli[SMELL] = 0.0f;	
				hNewTarget = DNULL;
			}
		}

		//Check sense
		if(hSenseObj && m_fSensingDist > 0 && fSense <= m_fSensingDist)
		{
			m_fStimuli[SENSE]	= 1.0f - (fSense/m_fSensingDist);
			hNewTarget = hSenseObj;
			nStimType = STIM_SENSE;

			if(m_fStimuli[SENSE] <= 0.0f)
			{
				m_fStimuli[SENSE] = 0.0f;	
				hNewTarget = DNULL;
			}
		}

		//Check sight
		if(hSightObj && m_fSeeingDist > 0 && fSight <= m_fSeeingDist)
		{
			m_fStimuli[SIGHT]	= 1.0f - (fSight/m_fSeeingDist);
			hNewTarget = hSightObj;
			nStimType = STIM_SIGHT;

			if(m_fStimuli[SIGHT] <= 0.0f)
			{
				m_fStimuli[SIGHT] = 0.0f;	
				hNewTarget = DNULL;
			}
		}
	}

	//Health
	m_fStimuli[HEALTH] = (DFLOAT)m_damage.GetHitPoints()/(DFLOAT)m_damage.GetMaxHitPoints();

	if (hNewTarget)
	{
		if(nStimType != STIM_SMELL)
		{
			CBaseCharacter* pB = (CBaseCharacter*)m_pServerDE->HandleToObject(hNewTarget);

			if( pB->IsDead())
			{
				hNewTarget = DNULL;
    			return 0;
			}

			SetNewTarget(hNewTarget);
//			m_fStimuli[THREAT] = ComputeThreat();
		}

		m_damage.ClearLastDamager();;

		return nStimType;
	}

	return 0;		//no applicable stimuli found
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::ChooseOpportunityTarget 
// DESCRIPTION	: fill in stimuli set
// RETURN TYPE	: DBOOL 
// ----------------------------------------------------------------------- //

DBOOL AI_Mgr::ChooseOpportunityTarget()
{
	if (!m_hObject || !m_InventoryMgr.GetCurrentWeapon()) return DFALSE;

	HOBJECT	hSightTarget = DNULL;
	DVector vSight;

	//Zero out the stimulus set
	memset(&m_fStimuli,0,sizeof(m_fStimuli));

	// Release current target if any
	SetNewTarget(DNULL);

	HCLASS hEnemyTest = m_pServerDE->GetClass("AI_Mgr");
	HOBJECT hSightObj = FindObjectInRadius(hEnemyTest, m_fSeeingDist, FIND_VISIBLE | FIND_FACE_OBJECT);
	
	if(hSightObj)
	{
		m_pServerDE->GetObjectPos(hSightObj, &vSight);
	}

	DFLOAT fSight = VEC_DIST(vSight,m_MoveObj.GetPos());
	HOBJECT hNewTarget = DNULL;

	//Check sight
    if(hSightObj && m_fSeeingDist > 0 && fSight <= m_fSeeingDist)
    {
		m_fStimuli[SIGHT]	= 1.0f - (fSight/m_fSeeingDist);
		hNewTarget = hSightObj;
	
		if(m_fStimuli[SIGHT] <= 0.0f)
		{
			m_fStimuli[SIGHT] = 0.0f;	
			return DFALSE;
		}
    }

	if (hNewTarget)
	{
        CBaseCharacter* pB = (CBaseCharacter*)m_pServerDE->HandleToObject(hNewTarget);

        if (pB->IsDead())
        {
			hNewTarget = DNULL;
    	    return DFALSE;
        }

		SetNewTarget(hNewTarget);
		return DTRUE;
	}

	return DFALSE;	// no target found
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::ComputeThreat
// DESCRIPTION	: Compute threat value
// RETURN TYPE	: DFLOAT 
// ----------------------------------------------------------------------- //

DFLOAT AI_Mgr::ComputeThreat()
{
	if (!m_hObject || m_hTarget == DNULL) return 0.0f;

	DFLOAT m_fTheThreat = 0.0f;

    // If the target a BaseCharacter?
	if(!m_pServerDE->IsKindOf(m_pServerDE->GetObjectClass(m_hTarget),m_pServerDE->GetClass("CBaseCharacter")))
		return 0.0f;
	
    CBaseCharacter* pB = (CBaseCharacter*)m_pServerDE->HandleToObject(m_hTarget);

    if (pB) 
    {        
        // AI values
    	CWeapon *p_Weapon = m_InventoryMgr.GetCurrentWeapon();
       	CWeapon *p_TargetWeapon = pB->CurrentWeapon();

		DFLOAT m_fTargetThreatValue = 0.0f, m_fAIThreatValue = 0.0f;

        // Get the distance from the Target (see if they are in Range)
        DFLOAT m_fTargetDistance = VEC_DIST(m_MoveObj.GetPos(), m_vTargetPos);
        
        // Add a Threat if target weapon is in range and is greater than AI Health
        if (p_TargetWeapon && m_fTargetDistance < p_TargetWeapon->GetWeaponRange())
        {
            m_fTargetThreatValue++;
            
            if (p_TargetWeapon ->GetWeaponDamage() > (DDWORD)m_damage.GetHitPoints())
            {
                m_fTargetThreatValue += 5;
            }
        }

        // Check from the AI side!        
        if (p_Weapon && m_fTargetDistance < p_Weapon->GetWeaponRange())
        {
            m_fAIThreatValue++;
            
            if (p_Weapon ->GetWeaponDamage() > (DDWORD)pB->HitPoints())
            {
                m_fAIThreatValue += 5;
            }
        }
        
        // If Target as low health then Plus our Threat
        if (pB->HitPoints() < m_damage.GetHitPoints())
        {
            m_fAIThreatValue += 10;
        }

		//Check for friends who can help
/*		ObjectList* pList = m_pServerDE->FindObjectsTouchingSphere(&m_vLastPos,m_fSeeingDist);
		ObjectLink* pObject = pList->m_pFirstLink;
		HCLASS hType = m_pServerDE->GetObjectClass(m_hObject);
		while(pObject && pObject->m_hObject)
		{			 
			if(m_pServerDE->IsKindOf(hType, m_pServerDE->GetObjectClass(pObject->m_hObject)))
				m_fAIThreatValue += 3;

			pObject = pObject->m_pNext;
		}
		m_pServerDE->RelinquishList(pList);
*/
		//Put it all together
		//The higher the value, the less threat present
		if(m_fAIThreatValue <= m_fTargetThreatValue)
	        m_fTheThreat = 1.0f - m_fAIThreatValue/m_fTargetThreatValue;
    }        

	return m_fTheThreat;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::CheckObstructed
// DESCRIPTION	: Check if we are obstructed in forward
// RETURN TYPE	: DBOOL 
// PARAMS		: DFLOAT fSpeed
// ----------------------------------------------------------------------- //

DBOOL AI_Mgr::CheckObstructed(DVector vDir, DFLOAT fSpeed)
{
	DFLOAT fDist = VEC_DIST(m_MoveObj.GetLastPos(),m_MoveObj.GetPos());
	DFLOAT fUp = (DFLOAT)fabs(m_MoveObj.GetLastPos().y - m_MoveObj.GetPos().y);

	DVector vVel;
	m_pServerDE->GetVelocity(m_hObject, &vVel);
	DFLOAT fVel = VEC_MAG(vVel);

	if((fVel < (fSpeed * 0.1f)) && (fUp <= 0.01f))
	{
		return DTRUE;
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::CalculateObstruction
// DESCRIPTION	: 
// RETURN TYPE	: int 
// ----------------------------------------------------------------------- //

int AI_Mgr::CalculateObstruction(IntersectInfo* IInfo)
{
	//sanity check
	if(IInfo == DNULL)
		return 0;

	DVector vStart[9];
	DVector vEnd[9];
	DVector vDims,vRight,vLeft,vPos;
	IntersectQuery IQuery;

	m_nBlockFlags = 0;

	//get the AIs dims
	VEC_COPY(vDims, m_vDims);
	VEC_COPY(vPos, m_MoveObj.GetPos());

	DFLOAT fFeeler = (DFLOAT)sqrt((vDims.x * vDims.x) + (vDims.z * vDims.z)) + 1.0f;
	fFeeler *= 2.0f;

	//initalize the left and right boundaries
	DFLOAT fAvg = (vDims.x + vDims.z)/2;
	VEC_MULSCALAR(vRight, m_MoveObj.GetRightVector(), fAvg);
	VEC_MULSCALAR(vLeft, m_MoveObj.GetRightVector(), fAvg * -1.0f);

	//move the start positions
	//RIGHT_UPPER								//CENTER_UPPER					//LEFT_UPPER
	VEC_ADD(vStart[0],vPos,vRight);		VEC_COPY(vStart[3], vPos);		VEC_ADD(vStart[6],vPos,vLeft);
	vStart[0].y += vDims.y - 1.0f;		vStart[3].y += vDims.y - 1.0f;	vStart[6].y += vDims.y - 1.0f;

	//RIGHT_MIDDLE								//CENTER_MIDDLE					//LEFT_MIDDLE
	VEC_ADD(vStart[1],vPos,vRight);		VEC_COPY(vStart[4], vPos);		VEC_ADD(vStart[7],vPos,vLeft);

	//RIGHT_LOWER								//CENTER_LOWER					//LEFT_LOWER
	VEC_ADD(vStart[2],vPos,vRight);		VEC_COPY(vStart[5], vPos);		VEC_ADD(vStart[8],vPos,vLeft);
	vStart[2].y -= vDims.y - 1.0f;		vStart[5].y -= vDims.y - 1.0f;	vStart[8].y -= vDims.y - 1.0f;

	//initialize the end positions
	for(int i = 0; i < 9; i++)
	{
		VEC_COPY(vEnd[i],vStart[i]);
		VEC_ADDSCALED(vEnd[i],vEnd[i], m_MoveObj.GetForwardVector(),fFeeler);
	}

	//cast the rays
	for(i = 0; i < 9; i++)
	{
		IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
		IQuery.m_FilterFn = DNULL;

		VEC_COPY(IQuery.m_From,vStart[i]);
		VEC_COPY(IQuery.m_To,vEnd[i]);

		if(m_pServerDE->IntersectSegment(&IQuery, &IInfo[i]))
		{
			if(m_hObject != IInfo[i].m_hObject)
			{
				switch(i)
				{
					case 0:		m_nBlockFlags |= BLOCK_RIGHT | BLOCK_UPPER;		break;
					case 1:		m_nBlockFlags |= BLOCK_RIGHT | BLOCK_MIDDLE;	break;
					case 2:		m_nBlockFlags |= BLOCK_RIGHT | BLOCK_LOWER;		break;
					case 3:		m_nBlockFlags |= BLOCK_CENTER | BLOCK_UPPER;	break;
					case 4:		m_nBlockFlags |= BLOCK_CENTER | BLOCK_MIDDLE;	break;
					case 5:		m_nBlockFlags |= BLOCK_CENTER | BLOCK_LOWER;	break;
					case 6:		m_nBlockFlags |= BLOCK_LEFT | BLOCK_UPPER;		break;
					case 7:		m_nBlockFlags |= BLOCK_LEFT | BLOCK_MIDDLE;		break;
					case 8:		m_nBlockFlags |= BLOCK_LEFT | BLOCK_LOWER;		break;
				}
			}
		}
		else
			memset(&IInfo[i],0,sizeof(IntersectInfo));
	}

	return m_nBlockFlags;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::CalculateTurn
// DESCRIPTION	: 
// RETURN TYPE	: DFLOAT 
// ----------------------------------------------------------------------- //

DFLOAT AI_Mgr::CalculateTurn(IntersectInfo* IInfo)
{
	//sanity check
	if(IInfo == DNULL)
		return 0.0f;

	DVector vNormal,vUp,vRight,vForward,vLeft,vDims;
	DRotation rRot;
	IntersectQuery IQuery;
	IntersectInfo ii;

	DFLOAT fLeftDist = 0.0f, fRightDist = 0.0f, fMaxDist = 0.0f, fAngle = MATH_PI;

	IQuery.m_Flags	  = INTERSECT_OBJECTS | INTERSECT_HPOLY;
	IQuery.m_FilterFn = DNULL;

	VEC_COPY(vDims, m_vDims);
	DFLOAT fMaxWidth = (DFLOAT)sqrt((vDims.x * vDims.x) + (vDims.z * vDims.z));

	//look through the intersections to get a poly normal for the obstacle
	for(int i = 0; i < 12; i++)
	{
		if(IInfo[i].m_hPoly)
		{
			VEC_MULSCALAR(vNormal,IInfo[i].m_Plane.m_Normal,-1.0f);
			break;
		}
	}

	//align a test rotation to the obstacles normal and retrieve the rotation vectors
	m_pServerDE->AlignRotation(&rRot, &vNormal, DNULL);
	m_pServerDE->GetRotationVectors(&rRot,&vUp,&vRight,&vForward);
	VEC_MULSCALAR(vLeft,vRight,-1.0f);		//create the left rotation vector

	//get the farthest left we could travel
	VEC_COPY(IQuery.m_From,m_MoveObj.GetPos());
	VEC_COPY(IQuery.m_Direction,vLeft);

	if(m_pServerDE->CastRay(&IQuery,&ii))
	{
		fMaxDist = fLeftDist = VEC_DIST(m_MoveObj.GetPos(),ii.m_Point);
	}

	//now get the farthest right
	VEC_COPY(IQuery.m_Direction,vRight);

	if(m_pServerDE->CastRay(&IQuery,&ii))
	{
		fRightDist = VEC_DIST(m_MoveObj.GetPos(),ii.m_Point);

		if(fRightDist > fMaxDist)
			fMaxDist = fRightDist;
	}

	//travel the obstacle in both directions looking for a clearing
	DVector vTmpRight,vTmpLeft,vDir;
	DVector vStartRight, vStartLeft,vEnd;

	VEC_INIT(vDir);

	VEC_MULSCALAR(vEnd,vForward,fMaxWidth + 5.0f);

	for(float fWidth = fMaxWidth; !(fWidth >= fRightDist && fWidth >= fLeftDist); fWidth += fMaxWidth)
	{
		//Check the right side
		if(fWidth < fRightDist)
		{
			VEC_MULSCALAR(vTmpRight,vRight,fWidth);
			VEC_ADD(vStartRight,m_MoveObj.GetPos(),vTmpRight);

			VEC_COPY(IQuery.m_From,vStartRight);

			if(m_nBlockFlags & BLOCK_UPPER)
				IQuery.m_From.y += vDims.y - 0.01f;
			else if(m_nBlockFlags & BLOCK_LOWER)
				IQuery.m_From.y -= vDims.y - 0.01f;

			VEC_ADD(IQuery.m_To,IQuery.m_From,vEnd);

			if(!m_pServerDE->IntersectSegment(&IQuery,&ii))
			{
				VEC_SUB(vDir, m_MoveObj.GetPos(), vStartRight);
//				VEC_SUB(vDir, vStartRight, m_vPos);
				VEC_NORM(vDir);

				DFLOAT fAmount = (DFLOAT) atan2(vDir.x, vDir.z);
				DFLOAT fAmount2 = (DFLOAT) atan2(m_MoveObj.GetForwardVector().x, m_MoveObj.GetForwardVector().z);

				if(fAmount < -0.001f)
					fAmount2 *= -1.0f;
				else if(fAmount2 < -0.001f)
					fAmount *= -1.0f;

				fAngle = fAmount2 - fAmount;
				return fAngle;
			}
		}

		//Check the left side
		if(fWidth < fLeftDist)
		{
			VEC_MULSCALAR(vTmpLeft,vLeft,fWidth);
			VEC_ADD(vStartLeft,m_MoveObj.GetPos(),vTmpLeft);

			VEC_COPY(IQuery.m_From,vStartLeft);

			if(m_nBlockFlags & BLOCK_UPPER)
				IQuery.m_From.y += vDims.y - 0.01f;
			else if(m_nBlockFlags & BLOCK_LOWER)
				IQuery.m_From.y -= vDims.y - 0.01f;

			VEC_ADD(IQuery.m_To,IQuery.m_From,vEnd);

			if(!m_pServerDE->IntersectSegment(&IQuery,&ii))
			{
				VEC_SUB(vDir, m_MoveObj.GetPos(), vStartRight);
//				VEC_SUB(vDir, vStartLeft, m_vPos);
				VEC_NORM(vDir);

				DFLOAT fAmount = (DFLOAT) atan2(vDir.x, vDir.z);
				DFLOAT fAmount2 = (DFLOAT) atan2(m_MoveObj.GetForwardVector().x, m_MoveObj.GetForwardVector().z);

				if(fAmount < -0.001f)
					fAmount2 *= -1.0f;
				else if(fAmount2 < -0.001f)
					fAmount *= -1.0f;

				fAngle = fAmount2 - fAmount;

				return fAngle;
			}
		}
	}

	//couldn't find a way around wall
	if(fRightDist < fLeftDist)
	{
		VEC_MULSCALAR(vTmpLeft,vLeft,fWidth);
		VEC_ADD(vStartLeft,m_MoveObj.GetPos(),vTmpLeft);

		VEC_SUB(vDir, vStartLeft, m_MoveObj.GetPos());
	}
	else
	{
		VEC_MULSCALAR(vTmpRight,vRight,fWidth);
		VEC_ADD(vStartRight,m_MoveObj.GetPos(),vTmpRight);

		VEC_SUB(vDir, vStartRight, m_MoveObj.GetPos());
	}

	VEC_NORM(vDir);

	DFLOAT fAmount = (DFLOAT) atan2(vDir.x, vDir.z);
	DFLOAT fAmount2 = (DFLOAT) atan2(m_MoveObj.GetForwardVector().x, m_MoveObj.GetForwardVector().z);

	if(fAmount < -0.001f)
		fAmount2 *= -1.0f;
	else if(fAmount2 < -0.001f)
		fAmount *= -1.0f;

	fAngle = fAmount2 - fAmount;

	return fAngle;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::NavigateObstacle
// DESCRIPTION	: 
// RETURN TYPE	: DBOOL 
// ----------------------------------------------------------------------- //

DBOOL AI_Mgr::NavigateObstacle()
{
	IntersectInfo IInfo[9];

	if(CalculateObstruction(&IInfo[0]))
	{
		if(m_nState == STATE_SearchVisualTarget)
		{
			SetNewState(STATE_SearchVisualTarget);
		}
		else if(m_nState == STATE_SearchSmellTarget)
		{
			SetNewState(STATE_SearchSmellTarget);
		}
		else if(m_nBlockFlags & BLOCK_UPPER && (m_dwFlags & FLAG_CRAWL))
		{
			if(!(m_nBlockFlags & BLOCK_LOWER) && !(m_nBlockFlags & BLOCK_MIDDLE))
			{
				SetNewState(STATE_CrawlUnderObj);
				return DTRUE;
			}
		}
		else if(m_nBlockFlags & BLOCK_LOWER && (m_dwFlags & FLAG_JUMP))
		{
			if(!(m_nBlockFlags & BLOCK_UPPER) && !(m_nBlockFlags & BLOCK_MIDDLE))
			{
				SetNewState(STATE_JumpOverObj);
				return DTRUE;
			}
		}
		else 
		{
			for(int i = 0; i <= 8; i++)
			{
				if(IInfo[i].m_hObject)
				{
					VEC_COPY(m_vDestPos, m_MoveObj.FindPosAroundObj(IInfo[i].m_Point, m_MoveObj.GetForwardVector()));

					if(VEC_DIST(m_vDestPos, IInfo[i].m_Point) != 0.0)
					{
						SetNewState(STATE_RunAroundObj);
					}
					else
					{
						SetNewState(m_nLastState);
					}
				}
			}
		}
	}
	else
	{
		SetNewState(m_nLastState);
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::FindObjectInRadius
// DESCRIPTION	: Find closest ExitHint object and return radians to turn to obj
// RETURN TYPE	: DFLOAT 
// ----------------------------------------------------------------------- //

HOBJECT AI_Mgr::FindObjectInRadius(HCLASS hObjectTest, DFLOAT fRange, DDWORD dwFlags)
{
    DFLOAT fObjDist, fSaveDist = 0;

    HOBJECT hExitObj = DNULL;
	DLink *pLink = DNULL, *pLinkHead = DNULL;
	HOBJECT hObject = DNULL;
	DBOOL bValid = DTRUE;
	DVector vMyPos;
	DRotation rRot;

	if(!m_pServerDE->GetModelNodeTransform(m_hObject,"head",&vMyPos,&rRot))
		VEC_COPY(vMyPos, m_MoveObj.GetPos());

	// 10/25 [gjk] added check for hObjectTest NULL
	if(!hObjectTest || fRange <= 0.0f)
		return DNULL;

	// 10/25 [gjk] added better link checking (check for pLink == pLinkHead for end of list, 
	// rather than pLink->m_pData since m_pData isn't gauranteed to be NULL for the head
	if(ExitHint::m_dwNumExits && m_pServerDE->IsKindOf(hObjectTest,m_pServerDE->GetClass("ExitHint")))
		pLinkHead = &ExitHint::m_ExitHead;
	else if(SmellHint::m_dwNumSmells && m_pServerDE->IsKindOf(hObjectTest,m_pServerDE->GetClass("SmellHint")))
		pLinkHead = &SmellHint::m_SmellHead;
	else if(AmmoPickup::m_dwNumPU && m_pServerDE->IsKindOf(hObjectTest,m_pServerDE->GetClass("AmmoPickup")))
		pLinkHead = &AmmoPickup::m_PUHead;
	else if(m_pServerDE->IsKindOf(hObjectTest, m_pServerDE->GetClass("AI_Mgr")) || 
			m_pServerDE->IsKindOf(hObjectTest, m_pServerDE->GetClass("CPlayerObj")))
	{
		if(m_bCabal && m_dwNumMonster)
			pLinkHead = &AI_Mgr::m_MonsterHead;
		else if (m_dwNumCabal)
			pLinkHead = &AI_Mgr::m_CabalHead;
	}

	if (pLinkHead)
		pLink = pLinkHead->m_pNext;

	if(pLink && (pLink != pLinkHead))
		hObject = m_pServerDE->ObjectToHandle((BaseClass*)pLink->m_pData);
	else
		return DNULL;

	while(pLink != pLinkHead && hObject)
	{
        DVector vObjPos;
		DVector vTgtDir, vObjDir;
		DVector vForward;
		DFLOAT fObjectDp = 0.0f;

		m_pServerDE->GetObjectPos(hObject,&vObjPos);

		bValid = DTRUE;

		if(VEC_DIST(vObjPos, vMyPos) > fRange)
		{
			bValid = DFALSE;
		}	
		else if(dwFlags & FIND_SPECIFIC_OBJ)
		{
			HCLASS hObjClass = m_pServerDE->GetObjectClass(hObject);

			if(!m_pServerDE->IsKindOf(hObjectTest, hObjClass))
				bValid = DFALSE;
		}
		else
		{
			VEC_SUB(vObjDir, vObjPos, vMyPos);
			VEC_NORM(vObjDir);

			VEC_COPY(vForward, m_MoveObj.GetForwardVector());
			fObjectDp = (vObjDir.x * vForward.x) + (vObjDir.y * vForward.y) + (vObjDir.z * vForward.z);
		}

		//check if the target is between us
		if((dwFlags & FIND_AVOID_TARGET) && m_hTarget && bValid)
		{
			VEC_SUB(vTgtDir, vMyPos, m_vTargetPos);
			VEC_NORM(vTgtDir);

			// Now test to see if AI is facing test object...
			DFLOAT fTargetDp = (vTgtDir.x * vForward.x) + (vTgtDir.y * vForward.y) + (vTgtDir.z * vForward.z);
			
			if (fTargetDp > 0 && fObjectDp > 0)
			{
				//we are facing the target AND exit object
				bValid = DFALSE;
			}
		}

		if((dwFlags & FIND_FACE_OBJECT) && bValid)
		{
			if(fObjectDp < 0)
			{
				//not facing object
				bValid = DFALSE;
			}
		}

		//check if the object is visible
		if((dwFlags & FIND_VISIBLE) && bValid)
		{
			if(IsPlayer(hObject))
			{
				CPlayerObj* pObj = (CPlayerObj*)m_pServerDE->HandleToObject(hObject);
				if(pObj->IsPowerupActive(PU_INVISIBLE))
					bValid = DFALSE;
			}

			if(bValid)
			{
				bValid = CanSee( hObject );
			}
		}

		if(bValid)
		{
    		fObjDist = VEC_DIST(vMyPos, vObjPos);

			if (fSaveDist == 0)
			{
				fSaveDist = fObjDist;
				hExitObj = hObject;
			}
			else
			{
				if (fObjDist < fSaveDist)
				{
					fSaveDist = fObjDist;
					hExitObj = hObject;
				}
			}
		}

        // Next object
		pLink = pLink->m_pNext;

		if(pLink != pLinkHead)
			hObject = m_pServerDE->ObjectToHandle((BaseClass*)pLink->m_pData);
		else
		{
			return hExitObj;
		}
	}

	return hExitObj;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::CanSee
// DESCRIPTION	: Check if we have visual of target
// RETURN TYPE	: DBOOL 
// PARAMS		: HOBJECT hTarget - who we want to see
// ----------------------------------------------------------------------- //
DBOOL AI_Mgr::CanSee( HOBJECT hTarget )
{
	IntersectQuery IQuery;
	IntersectInfo IInfo;
	DVector vTargetHeadPos;
	DRotation rRot;
	DVector vMyPos, vObjPos;
	GlobalFilterFnData globalFilterFnData;

#ifdef _ADD_ON
	// The nightmare ALWAYS sees you.
	if(m_pServerDE->IsKindOf(m_pServerDE->GetObjectClass(m_hObject),
		m_pServerDE->GetClass("Nightmare")))
		return DTRUE;
#endif

	if(!m_pServerDE->GetModelNodeTransform(m_hObject,"head",&vMyPos,&rRot))
		VEC_COPY(vMyPos, m_MoveObj.GetPos());

	m_pServerDE->GetObjectPos(hTarget, &vObjPos);
	
	VEC_COPY(IQuery.m_From, vMyPos);
	VEC_COPY(IQuery.m_To, vObjPos);

	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = GlobalFilterFn;
	globalFilterFnData.m_dwFlags = IGNORE_LIQUID | IGNORE_GLASS;
	globalFilterFnData.m_nIgnoreObjects = 1;
	globalFilterFnData.m_hIgnoreObjects = &m_hObject;
	IQuery.m_pUserData = &globalFilterFnData;	

	if(m_pServerDE->IntersectSegment(&IQuery, &IInfo))
	{
		if( hTarget == IInfo.m_hObject)
			return DTRUE;
		else
		{
			// His body is blocked.  Check if we can see the little camper's head
			if( m_pServerDE->GetModelNodeTransform( hTarget, "head", &vTargetHeadPos, &rRot ))
			{
				VEC_COPY( IQuery.m_From, vMyPos );
				VEC_COPY( IQuery.m_To, vTargetHeadPos );

				if( m_pServerDE->IntersectSegment( &IQuery, &IInfo ))
				{
					if( hTarget == IInfo.m_hObject )
						return DTRUE;
				}
			}
		}
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::Move
// DESCRIPTION	: Move in vDir direction at fSpeed velocity
// RETURN TYPE	: DBOOL 
// PARAMS		: DVector vDir
// PARAMS		: DFLOAT fSpeed
// ----------------------------------------------------------------------- //

DBOOL AI_Mgr::Move(DVector &vDir, DFLOAT fSpeed)
{
	DVector vTemp;

	if(m_nTrapped)
		{ VEC_INIT(vTemp); }
	else
	{
		if(fSpeed == MATH_EPSILON)
			{ VEC_COPY(vTemp, vDir); }
		else
			{ VEC_MULSCALAR(vTemp, vDir, fSpeed); }
	}

	m_pServerDE->SetVelocity(m_hObject, &vTemp);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::Jump
// DESCRIPTION	: 
// RETURN TYPE	: DBOOL 
// PARAMS		: DFLOAT m_fYSpeed
// PARAMS		: DFLOAT m_fZSpeed
// ----------------------------------------------------------------------- //

DBOOL AI_Mgr::Jump(DFLOAT m_fYSpeed, DFLOAT m_fZSpeed)
{
    // check for jump completed
    if (m_bJumping == DTRUE)
    {    
	    CollisionInfo collisionInfo;

	    m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);
    
        if (collisionInfo.m_hObject)
        {
            // Do not change until I hit the ground...
            // return to this substate
    		m_bJumping = DFALSE;
        }        
    }
    else
    {
		DVector vVel;

        vVel.y = m_MoveObj.GetUpVector().y * m_fYSpeed;
        vVel.x = m_MoveObj.GetForwardVector().x * m_fZSpeed;
        vVel.z = m_MoveObj.GetForwardVector().z * m_fZSpeed;

		Move(vVel, MATH_EPSILON);

		m_bJumping = DTRUE;
    }
    
    return m_bJumping;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::Fire
// DESCRIPTION	: Fire current weapon forward
// RETURN TYPE	: DBOOL 
// ----------------------------------------------------------------------- //

DBOOL AI_Mgr::Fire(DBOOL bAltFire)
{
	if(!m_hObject)	return DFALSE;

	DVector vPos, vDir;
	DRotation rRot;
	DBOOL bClearShot = DTRUE;

	m_pServerDE->GetObjectRotation(m_hObject, &rRot);

	// Sanity check (GK 9/18/98)
	if (!m_InventoryMgr.GetCurrentWeapon())
		return DFALSE;

	switch(m_InventoryMgr.GetCurrentWeapon()->GetType())
	{
		case WEAP_PROXIMITYBOMB:
		{
			m_pServerDE->GetModelNodeTransform(m_hObject,"torso",&vPos,&rRot);
			VEC_MULSCALAR(vDir, m_MoveObj.GetForwardVector(), 1.0f);
			break;
		}

		case WEAP_SHIKARI_CLAW:
		{
			m_pServerDE->GetObjectPos(m_hObject,&vPos);
			VEC_COPY(vDir, m_MoveObj.GetForwardVector());
			break;
		}

		case WEAP_SHIKARI_SPIT:
		{
			m_pServerDE->GetModelNodeTransform(m_hObject,"head",&vPos,&rRot);
			VEC_SUB(vDir, m_vTargetPos, vPos);

			break;
		}

		case WEAP_SOUL_CROWBAR:
		{
			m_pServerDE->GetObjectPos(m_hObject,&vPos);
			VEC_COPY(vDir, m_MoveObj.GetForwardVector());
			break;
		}

		case WEAP_SOUL_AXE:
		{
			m_pServerDE->GetObjectPos(m_hObject,&vPos);
			VEC_COPY(vDir, m_MoveObj.GetForwardVector());
			break;
		}

		case WEAP_SOUL_PIPE:
		{
			m_pServerDE->GetObjectPos(m_hObject,&vPos);
			VEC_COPY(vDir, m_MoveObj.GetForwardVector());
			break;
		}

		case WEAP_SOUL_HOOK:
		{
			m_pServerDE->GetObjectPos(m_hObject,&vPos);
			VEC_COPY(vDir, m_MoveObj.GetForwardVector());
			break;
		}

		case WEAP_BEHEMOTH_CLAW:
		{
			m_pServerDE->GetObjectPos(m_hObject,&vPos);
			VEC_COPY(vDir, m_MoveObj.GetForwardVector());
			break;
		}

		case WEAP_BEHEMOTH_SHOCKWAVE:
		{
			m_pServerDE->GetObjectPos(m_hObject,&vPos);
			VEC_SUB(vDir, m_vTargetPos, vPos);
			break;
		}

		case WEAP_ZEALOT_HEAL:
		{
			m_pServerDE->GetObjectPos(m_hObject,&vPos);
			VEC_SUB(vDir, m_vTargetPos, vPos);
			break;
		}

		case WEAP_ZEALOT_SHIELD:
		{
			m_pServerDE->GetObjectPos(m_hObject,&vPos);
			VEC_COPY(vDir, m_MoveObj.GetForwardVector());
			break;
		}
		
		case WEAP_ZEALOT_ENERGYBLAST:
		{
			m_pServerDE->GetModelNodeTransform(m_hObject,"r_hand_extra2",&vPos,&rRot);
			VEC_SUB(vDir, m_vTargetPos, vPos);

			bClearShot = DFALSE;

			break;
		}

		case WEAP_ZEALOT_GROUNDFIRE:
		{
			m_pServerDE->GetModelNodeTransform(m_hObject,"r_hand_extra2",&vPos,&rRot);
			VEC_SUB(vDir, m_vTargetPos, vPos);
			break;
		}

		case WEAP_ZEALOT_SHOCKWAVE:
		{
			m_pServerDE->GetObjectPos(m_hObject,&vPos);
			VEC_COPY(vDir, m_MoveObj.GetForwardVector());
			break;
		}

		case WEAP_DRUDGE_FIREBALL:
		{
			m_pServerDE->GetModelNodeTransform(m_hObject,"head",&vPos,&rRot);

			HCLASS hLord = m_pServerDE->GetClass("DrudgeLord");
			HCLASS hThis = m_pServerDE->GetObjectClass(m_hObject);

			if(m_pServerDE->IsKindOf(hThis, hLord))
			{
				VEC_SUB(vDir, m_vTargetPos, vPos);
				VEC_NORM(vDir);

				vDir.x = m_MoveObj.GetForwardVector().x;
				vDir.z = m_MoveObj.GetForwardVector().z;
			}
			else
			{
				VEC_SUB(vDir, m_vTargetPos, vPos);
			}

			bClearShot = DFALSE;

			break;
		}

		case WEAP_DRUDGE_LIGHTNING:
		{
			m_pServerDE->GetModelNodeTransform(m_hObject,"head",&vPos,&rRot);
			VEC_SUB(vDir, m_vTargetPos, vPos);

			break;
		}

		case WEAP_HAND_SQUEEZE:
		case WEAP_BONELEECH_SUCK:
		case WEAP_THIEF_SUCK:
		
		case WEAP_NAGA_EYEBEAM:
		{
			m_pServerDE->GetModelNodeTransform(m_hObject,"head_gun",&vPos,&rRot);
			VEC_SUB(vDir, m_vTargetPos, vPos);
			break;
		}

		case WEAP_NAGA_SPIKE:
		{
			m_pServerDE->GetModelNodeTransform(m_hObject,"l_gun",&vPos,&rRot);
			VEC_SUB(vDir, m_vTargetPos, vPos);
			break;
		}

		case WEAP_GIDEON_VOMIT:
		{
			m_pServerDE->GetModelNodeTransform(m_hObject,"head",&vPos,&rRot);
			VEC_SUB(vDir, m_vTargetPos, vPos);

			break;
		}

		case WEAP_GIDEON_GOO:
		{
			m_pServerDE->GetModelNodeTransform(m_hObject,"head",&vPos,&rRot);
			VEC_SUB(vDir, m_vTargetPos, vPos);

			break;
		}

		case WEAP_GIDEON_SPEAR:
		{
			m_pServerDE->GetModelNodeTransform(m_hObject,"head",&vPos,&rRot);
			VEC_COPY(vDir, m_MoveObj.GetForwardVector());

			break;
		}

		case WEAP_ANCIENTONE_BEAM:
		{
			m_pServerDE->GetModelNodeTransform(m_hObject,"head_gun",&vPos,&rRot);
			VEC_SUB(vDir, m_vTargetPos, vPos);
			VEC_NORM(vDir);

			vDir.x = m_MoveObj.GetForwardVector().x;
			vDir.z = m_MoveObj.GetForwardVector().z;

			bClearShot = DFALSE;
			break;
		}

		case WEAP_ANCIENTONE_TENTACLE:
		{
			m_pServerDE->GetObjectPos(m_hObject,&vPos);
			VEC_SUB(vDir, m_vTargetPos, vPos);
			bClearShot = DFALSE;
			break;
		}

		case WEAP_SKULL:
		{
			m_pServerDE->GetModelNodeTransform(m_hObject,"head_gun",&vPos,&rRot);
			VEC_SUB(vDir, m_vTargetPos, vPos);
			break;
		}

#ifdef _ADD_ON
		case WEAP_NIGHTMARE_BITE:
		case WEAP_NIGHTMARE_FIREBALLS:
		{
			m_pServerDE->GetModelNodeTransform(m_hObject,"jaw",&vPos,&rRot);
			VEC_SUB(vDir, m_vTargetPos, vPos);
			break;
		}

#endif


		default:
		{	
			if(!m_pServerDE->GetModelNodeTransform(m_hObject,"r_gun",&vPos,&rRot))
			{
				m_pServerDE->GetObjectPos(m_hObject,&vPos);
				VEC_COPY(vDir, m_MoveObj.GetForwardVector());
			}
			else
			{
				VEC_SUB(vDir, m_vTargetPos, vPos);
			}

			break;
		}
	}

	VEC_NORM(vDir);
	m_pServerDE->AlignRotation(&rRot, &vDir, DNULL);

	if(CheckClearShot(vPos, rRot, bAltFire, IGNORE_GLASS | IGNORE_LIQUID | IGNORE_DESTRUCTABLE) || !bClearShot)
		DDWORD m_nFiredWeapon = m_InventoryMgr.FireCurrentWeapon(&vPos, &rRot, bAltFire);
	else
		return DFALSE;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::CheckClearShot
// DESCRIPTION	: Check for a clear shot
// RETURN TYPE	: DBOOL 
// ----------------------------------------------------------------------- //

DBOOL AI_Mgr::CheckClearShot(DVector &vPos, DRotation &rRot, DBOOL bAltFire, DDWORD dwFlags)
{
	DFLOAT fRange = 0.0f;
	DVector vU, vR, vF;
	DRotation rTargetRot;
	GlobalFilterFnData globalFilterFnData;

	CWeapon* pW = m_InventoryMgr.GetCurrentWeapon();

	if(pW == DNULL)
		return DFALSE;

	if(!bAltFire)
		fRange = pW->GetWeaponRange();
	else
		fRange = pW->GetAltWeaponRange();

	m_pServerDE->GetRotationVectors(&rRot, &vU,&vR,&vF);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = GlobalFilterFn;
	globalFilterFnData.m_dwFlags = dwFlags;
	globalFilterFnData.m_nIgnoreObjects = 1;
	globalFilterFnData.m_hIgnoreObjects = &m_hObject;
	IQuery.m_pUserData = &globalFilterFnData;	

	VEC_COPY(IQuery.m_From, vPos);
	VEC_ADDSCALED(IQuery.m_To, vPos, vF, fRange);

	// Nothing blocking us, it's clear
	if( !m_pServerDE->IntersectSegment( &IQuery, &IInfo ))
		return DTRUE;

	if(m_hTarget == IInfo.m_hObject)
		return DTRUE;
	else
	{
		if( m_hTarget )
		{
			// His body is blocked.  Check if we can see the little camper's head
			if( m_pServerDE->GetModelNodeTransform( m_hTarget, "head", &m_vTargetPos, &rTargetRot ))
			{
				VEC_SUB( vF, m_vTargetPos, IQuery.m_From );
				VEC_NORM( vF );
				VEC_ADDSCALED( IQuery.m_To, IQuery.m_From, vF, fRange );

				if( m_pServerDE->IntersectSegment( &IQuery, &IInfo ))
				{
					if( m_hTarget == IInfo.m_hObject )
					{
						// Modify the direction to fire.
						g_pServerDE->AlignRotation( &rRot, &vF, DNULL );
						return DTRUE;
					}
				}
			}
		}
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::IsClearXZ
// DESCRIPTION	: Check if the ray intersects in vDir to fDist
// RETURN TYPE	: DBOOL 
// PARAMS		: DVector vDir
// PARAMS		: DFLOAT fDist
// ----------------------------------------------------------------------- //

DBOOL AI_Mgr::IsClearXZ(DVector vDir, DFLOAT fDist)
{
	//get the objects dims
	DVector vDims,vPos;
	VEC_COPY(vDims, m_vDims);
	DFLOAT fFeeler = (DFLOAT)sqrt((vDims.x * vDims.x) + (vDims.z * vDims.z)) + fDist;

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_Flags	  = INTERSECT_OBJECTS;
	IQuery.m_FilterFn = DNULL;

	//check above
	VEC_COPY(vPos,m_MoveObj.GetPos());
	vPos.y += (vDims.y - 0.1f);
	VEC_COPY(IQuery.m_From, vPos);
	VEC_COPY(IQuery.m_To,vPos);
	IQuery.m_To.x += (vDir.x * fFeeler);
	IQuery.m_To.z += (vDir.z * fFeeler);

	DFLOAT fTest = VEC_DIST(IQuery.m_From,IQuery.m_To);

	if(m_pServerDE->IntersectSegment(&IQuery, &IInfo))
	{
		return DFALSE;
	}

	//check below
	VEC_COPY(vPos,m_MoveObj.GetPos());
	vPos.y -= (vDims.y - 0.1f);
	VEC_COPY(IQuery.m_From, vPos);
	VEC_COPY(IQuery.m_To,vPos);
	IQuery.m_To.x += (vDir.x * fFeeler);
	IQuery.m_To.z += (vDir.z * fFeeler);

	if(m_pServerDE->IntersectSegment(&IQuery, &IInfo))
	{
		return DFALSE;
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::CalculateDodge
// DESCRIPTION	: Calculate how to dodge object
// RETURN TYPE	: DBOOL 
// PARAMS		: DVector vPos
// ----------------------------------------------------------------------- //

int	AI_Mgr::CalculateDodge(DVector vPos)
{
	int nFlags = 0;
	DVector vDir, vLeft, vRight, vForward, vBackward;

	VEC_COPY(vRight, m_MoveObj.GetRightVector());
	VEC_MULSCALAR(vLeft, vRight, -1.0f);

	VEC_COPY(vForward, m_MoveObj.GetForwardVector());
	VEC_MULSCALAR(vBackward, vForward, -1.0f);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_Flags	  = INTERSECT_OBJECTS;
	IQuery.m_FilterFn = DNULL;

	VEC_COPY(IQuery.m_From, m_MoveObj.GetPos());

	//calculate direction to dodge
	VEC_SUB(vDir, vPos, m_MoveObj.GetPos());
	VEC_NORM(vDir);

	DFLOAT fRight		= (vDir.x * vRight.x) + (vDir.y * vRight.y) + (vDir.z * vRight.z);
	DFLOAT fLeft		= (vDir.x * vLeft.x) + (vDir.y * vLeft.y) + (vDir.z * vLeft.z);
	DFLOAT fForward		= (vDir.x * vForward.x) + (vDir.y * vForward.y) + (vDir.z * vForward.z);
	DFLOAT fBackward	= (vDir.x * vBackward.x) + (vDir.y * vBackward.y) + (vDir.z * vBackward.z);
	
	//check if right
	if(fRight > 0)
	{
		nFlags = LEFT;
		VEC_ADDSCALED(IQuery.m_To, m_MoveObj.GetPos(), vLeft, m_fRollSpeed);
	}
	else if(fLeft > 0)
	{
		nFlags = RIGHT;
		VEC_ADDSCALED(IQuery.m_To, m_MoveObj.GetPos(), vRight, m_fRollSpeed);
	}			

	//check if forward	
	if(fForward > 0 && fForward > fRight && fForward > fLeft)
	{
		nFlags = BACKWARD;
		VEC_ADDSCALED(IQuery.m_To, m_MoveObj.GetPos(), vBackward, m_fRollSpeed);
	}
	else if(fBackward > 0 && fBackward > fRight && fBackward > fLeft)
	{
		nFlags = FORWARD;
		VEC_ADDSCALED(IQuery.m_To, m_MoveObj.GetPos(), vForward, m_fRollSpeed);
	}

	//check if there will be an obstruction
	if(m_pServerDE->IntersectSegment(&IQuery, &IInfo))
		nFlags |= DODGE;
	else
		nFlags |= ROLL;

	return nFlags;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::IsLedge
// DESCRIPTION	: Check for a drop off in vDir at fDist
// RETURN TYPE	: DBOOL 
// PARAMS		: DVector vDir
// PARAMS		: DFLOAT fDist
// ----------------------------------------------------------------------- //

DBOOL AI_Mgr::IsLedge(DVector vDir)
{
	if(m_pServerDE->GetTime() - m_fLastLedgeCheck < 0.05f)
		return DFALSE;

	m_fLastLedgeCheck = m_pServerDE->GetTime();

	DFLOAT fDist = (DFLOAT)sqrt((m_vDims.x * m_vDims.x) + (m_vDims.z * m_vDims.z));

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_ADDSCALED(IQuery.m_From,m_MoveObj.GetPos(), vDir, fDist * 1.25f);
	VEC_COPY(IQuery.m_To,IQuery.m_From);
	IQuery.m_To.y -= m_vDims.y * 2.0f;
	
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = DNULL;	//GlobalFilterFn;

//	DDWORD dwFlags = IGNORE_LIQUID;
//	IQuery.m_pUserData = &dwFlags;	

	if(m_pServerDE->IntersectSegment(&IQuery, &IInfo))
	{
		return DFALSE;
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::CreateBloodSpurt
// DESCRIPTION	: 
// RETURN TYPE	: DBOOL 
// PARAMS		: int nNodeHit
// ----------------------------------------------------------------------- //

DBOOL AI_Mgr::CreateBloodSpurt(int nNodeHit)
{
/*
	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	DVector vColor,vNormal;
	DVector vUp,vRight;
	char *pTextureFile = DNULL;

	switch(nNodeHit)
	{
		case NODE_NECK:		m_pServerDE->GetModelNodeTransform(m_hObject,szNodes[NODE_NECK],
															   &ocStruct.m_Pos,&ocStruct.m_Rotation);
							break;
		case NODE_RARM:		m_pServerDE->GetModelNodeTransform(m_hObject,szNodes[NODE_RARM],
															   &ocStruct.m_Pos,&ocStruct.m_Rotation);
							break;
		case NODE_LARM:		m_pServerDE->GetModelNodeTransform(m_hObject,szNodes[NODE_LARM],
															   &ocStruct.m_Pos,&ocStruct.m_Rotation);
							break;
		case NODE_RLEG:		m_pServerDE->GetModelNodeTransform(m_hObject,szNodes[NODE_RLEG],
															   &ocStruct.m_Pos,&ocStruct.m_Rotation);
							break;
		case NODE_LLEG:		m_pServerDE->GetModelNodeTransform(m_hObject,szNodes[NODE_LLEG],
															   &ocStruct.m_Pos,&ocStruct.m_Rotation);						
							break;
	}

	m_pServerDE->GetRotationVectors(&ocStruct.m_Rotation,&vUp,&vRight,&vNormal);

	//create a splash of blood
	HCLASS hClass = m_pServerDE->GetClass("CClientSparksSFX");

	CClientSparksSFX *pSpark = DNULL;

	if (hClass)
	{
		pSpark = (CClientSparksSFX *)m_pServerDE->CreateObject(hClass, &ocStruct);
	}

	VEC_SET(vColor, 255.0f, 0.0f, 0.0f);

	switch(m_pServerDE->IntRandom(1,3))
	{
		case 1:		pTextureFile = "spritetextures\\blooddrop1.dtx";	break;
		case 2:		pTextureFile = "spritetextures\\blooddrop2.dtx";	break;
		case 3:		pTextureFile = "spritetextures\\blooddrop3.dtx";	break;
	}

	pSpark->Setup(&vNormal, &vColor, &vColor, pTextureFile, 20, 1.0f, 2.0f, 300.0f, -200.0f);
*/
	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::CreateCorpse
// DESCRIPTION	: Convert the AI into a corpse object
// RETURN TYPE	: DBOOL 
// ----------------------------------------------------------------------- //

DBOOL AI_Mgr::CreateCorpse()
{
	HCLASS hClass = g_pServerDE->GetClass( "CCorpse" );
	if( !hClass )
		return DFALSE;

	DVector vAI_Dims,vDims,vPos;
	DBOOL bStatus = DFALSE;

	VEC_COPY(vPos,m_MoveObj.GetPos());
	VEC_COPY(vAI_Dims, m_vDims);

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	VEC_COPY(theStruct.m_Pos,m_MoveObj.GetPos());
	ROT_COPY(theStruct.m_Rotation,m_MoveObj.GetRotation());

	m_pServerDE->GetModelFilenames(m_hObject,theStruct.m_Filename,MAX_CS_FILENAME_LEN+1,
											 theStruct.m_SkinName,MAX_CS_FILENAME_LEN+1);

	if(m_damage.GetLastDamageType() == DAMAGE_TYPE_FIRE)
	{
		_mbscpy((unsigned char*)theStruct.m_SkinName, (const unsigned char*)"skins\\enemies\\burned.dtx");
		m_nCorpseType = m_pAnim_Sound->m_nAnim_DEATH[m_pServerDE->IntRandom(0,11)];
	}

	theStruct.m_UserData = (DDWORD)(m_pAnim_Sound->GetSoundRoot());

	// Allocate an object...
	BaseClass* pObj = m_pServerDE->CreateObject( hClass, &theStruct );
	if( !pObj )	return DFALSE;

	//shoudl we limb loss while corpse?
	HCLASS hSoulClass = m_pServerDE->GetClass("SoulDrudge");

	if(m_bCabal || m_pServerDE->IsKindOf(hSoulClass, m_pServerDE->GetObjectClass(m_hObject)))
	{
		((CCorpse*)pObj)->Setup(m_hFireSource, DTRUE);
	}
	else
	{
		((CCorpse*)pObj)->Setup(m_hFireSource, DFALSE);
	}

	//sanity check for a bug I can't find :)
	if(m_nCorpseType < m_pAnim_Sound->m_nAnim_DEATH[0])
		m_nCorpseType = m_pAnim_Sound->m_nAnim_DEATH[m_pServerDE->IntRandom(0,11)];

	//Set the animation to the corpse animation
	m_pServerDE->SetModelAnimation(pObj->m_hObject,m_nCorpseType);
    m_pServerDE->SetModelLooping(pObj->m_hObject, DFALSE);

	//Get the corpses dims
	m_pServerDE->GetModelAnimUserDims(pObj->m_hObject, &vDims, m_nCorpseType);

	//Put the corpse on the floor
	m_pServerDE->ScaleObject(pObj->m_hObject,&m_vScale);
	m_pServerDE->SetObjectDims2(pObj->m_hObject,&vDims);

	vPos.y -= (vAI_Dims.y - vDims.y) - 0.001f;

	m_pServerDE->SetObjectPos(pObj->m_hObject,&vPos);

	//HEAD/NECK
	m_pServerDE->GetModelNodeHideStatus(m_hObject,szNodes[NODE_NECK],&bStatus);
	if(bStatus)
	{
		AIShared.HideLimb(pObj->m_hObject, NODE_NECK);
	}

	//RIGHT ARM
	m_pServerDE->GetModelNodeHideStatus(m_hObject,szNodes[NODE_RARM],&bStatus);
	if(bStatus)
	{
		AIShared.HideLimb(pObj->m_hObject, NODE_RARM);
	}

	//LEFT ARM
	m_pServerDE->GetModelNodeHideStatus(m_hObject,szNodes[NODE_LARM],&bStatus);
	if(bStatus)	
	{
		AIShared.HideLimb(pObj->m_hObject, NODE_LARM);
	}

	//LEFT LEG
	m_pServerDE->GetModelNodeHideStatus(m_hObject,szNodes[NODE_LLEG],&bStatus);
	if(bStatus)	
	{
		AIShared.HideLimb(pObj->m_hObject, NODE_LLEG);
	}

	//RIGHT LEG
	m_pServerDE->GetModelNodeHideStatus(m_hObject,szNodes[NODE_RLEG],&bStatus);
	if(bStatus)	
	{
		AIShared.HideLimb(pObj->m_hObject, NODE_RLEG);
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::CreateGibs
// DESCRIPTION	: Run an idle animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

DBOOL AI_Mgr::CreateGibs(DVector vDir, int nNumGibs, int nType, DFLOAT fDamage)
{
//	HCLASS hClass = m_pServerDE->GetClass( "CClientGibFX" );
//	if( !hClass )
//		return DFALSE;

	// Add a little more vertical oomph
	vDir.y += 2.0f;
	VEC_NORM(vDir);

	VEC_NEGATE(vDir, vDir);

	DFLOAT fVel = 50.0f + fDamage*2;

	VEC_MULSCALAR(vDir, vDir, fVel);

//	ObjectCreateStruct ocStruct;
//	INIT_OBJECTCREATESTRUCT(ocStruct);
	DVector vPos;

//	VEC_COPY(ocStruct.m_Pos, m_MoveObj.GetPos());
//	ROT_COPY(ocStruct.m_Rotation, m_MoveObj.GetRotation());
	VEC_COPY( vPos, m_MoveObj.GetPos( ));

//	CClientGibFX* pGib = (CClientGibFX*)m_pServerDE->CreateObject(hClass, &ocStruct);

	DVector vDims;
	VEC_COPY(vDims, m_vDims);

	DFLOAT fScale = 1.0f;
	// Scale down for the little creatures..
	if (m_fAIMass && m_fAIMass < AI_DEFAULT_MASS)
		fScale = m_fAIMass / AI_DEFAULT_MASS;


//	pGib->Setup(&m_MoveObj.GetPos(), &vDir, &vDims, (SURFTYPE_FLESH/10) | SIZE_SMALL | TRAIL_BLOOD, fScale, nNumGibs);
	SetupClientGibFX(&m_MoveObj.GetPos(), &vDir, &vDims, (SURFTYPE_FLESH/10) | SIZE_SMALL | TRAIL_BLOOD, fScale, nNumGibs);

	// Create body parts

	HCLASS hSoulClass = m_pServerDE->GetClass("SoulDrudge");

	if(m_bCabal || m_pServerDE->IsKindOf(hSoulClass, m_pServerDE->GetObjectClass(m_hObject)))
	{
		int rand = m_pServerDE->IntRandom(0, 100);
		// 80 % create a head..
		if (rand = m_pServerDE->IntRandom(0, 100) > 80)
			AIShared.CreateLimb(m_hObject, NODE_NECK, vDir);

		// 70% create 1 arm
		if (rand = m_pServerDE->IntRandom(0, 100) > 70)
			AIShared.CreateLimb(m_hObject, NODE_RARM, vDir);
		
		// 50% create 2 arms
		if (rand = m_pServerDE->IntRandom(0, 100) > 50)
			AIShared.CreateLimb(m_hObject, NODE_LARM, vDir);
		
		// 70% create 1 leg
		if (rand = m_pServerDE->IntRandom(0, 100) > 70)
			AIShared.CreateLimb(m_hObject, NODE_RLEG, vDir);	
		
		// 50% create 2 legs
		if (rand = m_pServerDE->IntRandom(0, 100) > 50)
			AIShared.CreateLimb(m_hObject, NODE_LLEG, vDir);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::RemoveMe
// DESCRIPTION	: Safely removes the AI and removes him from the obj lists.
//				: Solves problem of RemoveObject m_hObject, and then something
//				: iterates through the AI lists which still has a handle pointing
//				: to m_hObject. This fn insures they both get removed simultaneously.
//				: sets m_Link.m_Data to NULL so destructor doesn't try toremove it again.
//				:         (gk 9/18/98)
// RETURN TYPE	: void
// PARAMS		: none
// ----------------------------------------------------------------------- //

void AI_Mgr::RemoveMe()
{
	if (!m_pServerDE)	return;

	m_pServerDE->RemoveObject(m_hObject);
}


// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Idle
// DESCRIPTION	: Run an idle animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Idle()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_IDLE)
    {
		DBOOL bRet = DFALSE;

		bRet = SetAnimation(m_pAnim_Sound->m_nAnim_IDLE[m_pServerDE->IntRandom(0,3)]);

		m_pServerDE->SetModelLooping(m_hObject, DFALSE);

        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_IDLE;
	}
	else
	{
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE;
            Metacmd++;
            return;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Talk
// DESCRIPTION	: Run an talk animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Talk()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_IDLE)
    {
		DBOOL bRet = DFALSE;

		bRet = SetAnimation(m_pAnim_Sound->m_nAnim_TALK[m_pServerDE->IntRandom(0,4)]);

		m_pServerDE->SetModelLooping(m_hObject, DFALSE);

        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_IDLE;
	}
	else
	{
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE;
            Metacmd++;
            return;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Walk
// DESCRIPTION	: Run the walk animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Walk()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_WALK)
    {
		DBOOL bRet = DFALSE;

		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();

    	m_fTimeStart = m_pServerDE->GetTime();

		if(pW)
		{
			if(m_nInjuredLeg == NODE_LLEG)
			{
				if(pW->GetFireType() == TYPE_PISTOL || pW->GetFireType() == TYPE_MELEE)
					bRet = SetAnimation(m_pAnim_Sound->m_nAnim_WALK_INJURED_LLEG_PISTOL);
				else
					bRet = SetAnimation(m_pAnim_Sound->m_nAnim_WALK_INJURED_LLEG_RIFLE);
			}
			else if(m_nInjuredLeg == NODE_RLEG)
			{
				if(pW->GetFireType() == TYPE_PISTOL || pW->GetFireType() == TYPE_MELEE)
					bRet = SetAnimation( m_pAnim_Sound->m_nAnim_WALK_INJURED_RLEG_PISTOL);
				else
					bRet = SetAnimation( m_pAnim_Sound->m_nAnim_WALK_INJURED_RLEG_RIFLE);
			}
			else
				bRet = SetAnimation( m_pAnim_Sound->m_nAnim_WALK[pW->GetFireType()]);
		}
		else
			bRet = SetAnimation( m_pAnim_Sound->m_nAnim_WALK[TYPE_MELEE]);

		//SCHLEGZ 4/22/98 3:27:12 PM: if we can't stand up, crawl out from underneath
		if(!bRet)
		{
			SetNewState(STATE_CrawlUnderObj);
			return;
		}

        m_pServerDE->SetModelLooping(m_hObject, DTRUE);

		if(m_nInjuredLeg)
			Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed/3);
		else
			Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
			
        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_WALK;
    }
    else
    {   
		//Check for obstruction; otherwise continue on
		if(CheckObstructed(m_MoveObj.GetForwardVector(), m_fWalkSpeed))
		{
			NavigateObstacle();
		}

		CollisionInfo collisionInfo;
		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);
		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);

		if(collisionInfo.m_hObject && (dwFlags & FLAG_GRAVITY))
		{		
			Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
		}
		
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE; 
            Metacmd++;
        }
    }
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::Script_Walk
// DESCRIPTION	: Run the walk animation (when in the Script state)
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::Script_Walk()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_WALK)
    {
		DBOOL bRet = DFALSE;

		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();

    	m_fTimeStart = m_pServerDE->GetTime();

		if (pW)
		{
			if(m_nInjuredLeg == NODE_LLEG)
			{
				if(pW->GetFireType() == TYPE_PISTOL || pW->GetFireType() == TYPE_MELEE)
					bRet = SetAnimation( m_pAnim_Sound->m_nAnim_WALK_INJURED_LLEG_PISTOL);
				else
					bRet = SetAnimation( m_pAnim_Sound->m_nAnim_WALK_INJURED_LLEG_RIFLE);
			}
			else if(m_nInjuredLeg == NODE_RLEG)
			{
				if(pW->GetFireType() == TYPE_PISTOL || pW->GetFireType() == TYPE_MELEE)
					bRet = SetAnimation( m_pAnim_Sound->m_nAnim_WALK_INJURED_RLEG_PISTOL);
				else
					bRet = SetAnimation( m_pAnim_Sound->m_nAnim_WALK_INJURED_RLEG_RIFLE);
			}
			else
				bRet = SetAnimation( m_pAnim_Sound->m_nAnim_WALK[pW->GetFireType()]);
		}
		else
		{
			bRet = SetAnimation( m_pAnim_Sound->m_nAnim_WALK[TYPE_MELEE]);
		}

        m_pServerDE->SetModelLooping(m_hObject, DTRUE);

		if(m_nInjuredLeg)
			Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed/3);
		else
			Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
			
        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_WALK;
    }
    else
    {   
		Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
   
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE; 
        }
    }
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Run
// DESCRIPTION	: Run the run animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Run()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_RUN)
    {
		DBOOL bRet = DFALSE;

	    CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();

    	m_fTimeStart = m_pServerDE->GetTime();

		if(pW)
	        bRet = SetAnimation( m_pAnim_Sound->m_nAnim_RUN[pW->GetFireType()]);
		else
	        bRet = SetAnimation( m_pAnim_Sound->m_nAnim_RUN[TYPE_MELEE]);

		//SCHLEGZ 4/22/98 3:27:12 PM: if we can't stand up, crawl out from underneath
		if(!bRet)
		{
			SetNewState(STATE_CrawlUnderObj);
			return;
		}

        m_pServerDE->SetModelLooping(m_hObject, DTRUE);

		Move(m_MoveObj.GetForwardVector(),m_fRunSpeed);
        
        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_RUN;
    }
    else
    {   
		//Check for obstruction; otherwise continue on
		if(CheckObstructed(m_MoveObj.GetForwardVector(), m_fRunSpeed))
		{
			NavigateObstacle();
		}

		CollisionInfo collisionInfo;
		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);
		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);

		if(collisionInfo.m_hObject && (dwFlags & FLAG_GRAVITY))
		{		
			Move(m_MoveObj.GetForwardVector(),m_fRunSpeed);
		}

		//Are we done running?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE; 
            Metacmd++;
        }
    }
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::Script_Run
// DESCRIPTION	: Run the run animation (when in the Script state)
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::Script_Run()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_RUN)
    {
		DBOOL bRet = DFALSE;

	    CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();

    	m_fTimeStart = m_pServerDE->GetTime();

		if(pW)
	        bRet = SetAnimation( m_pAnim_Sound->m_nAnim_RUN[pW->GetFireType()]);
		else
	        bRet = SetAnimation( m_pAnim_Sound->m_nAnim_RUN[TYPE_MELEE]);

        m_pServerDE->SetModelLooping(m_hObject, DTRUE);

		Move(m_MoveObj.GetForwardVector(),m_fRunSpeed);
        
        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_RUN;
    }
    else
    {   
		Move(m_MoveObj.GetForwardVector(),m_fRunSpeed);
    
		//Are we done running?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE; 
         }
    }
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Jump
// DESCRIPTION	: Run the jump animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Jump()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_JUMP)
    {
	    CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();

		if(pW)
	        SetAnimation( m_pAnim_Sound->m_nAnim_JUMP[pW->GetFireType()]);
		else
			SetAnimation( m_pAnim_Sound->m_nAnim_JUMP[TYPE_PISTOL]);

        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		DVector vVel;

		vVel.y = m_MoveObj.GetUpVector().y * m_fJumpSpeed;
		vVel.x = m_MoveObj.GetForwardVector().x * m_fRunSpeed;
		vVel.z = m_MoveObj.GetForwardVector().z * m_fRunSpeed;

		Move(vVel, MATH_EPSILON);

		m_nCurMetacmd = MC_JUMP;
        m_bAnimating = DTRUE; 
    }
    else
    {        
		CollisionInfo collisionInfo;
		DVector vVel;

		m_pServerDE->GetVelocity(m_hObject,&vVel);
		vVel.x = m_MoveObj.GetForwardVector().x * m_fRunSpeed;
		vVel.z = m_MoveObj.GetForwardVector().z * m_fRunSpeed;

		Move(vVel, MATH_EPSILON);

		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);

		if (collisionInfo.m_hObject)
		{
			StopVelocity();

			m_bAnimating = DFALSE; 
			Metacmd++;
		}
    }               
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::Crouch
// DESCRIPTION	: Run the crouch animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Crouch()
{
	CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();

	if(pW)
	    SetAnimation( m_pAnim_Sound->m_nAnim_CROUCH[pW->GetFireType()]);
	else
		SetAnimation( m_pAnim_Sound->m_nAnim_CROUCH[TYPE_PISTOL]);

	Metacmd++;

	m_nCurMetacmd = MC_CROUCH;

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Crawl
// DESCRIPTION	: Run the crawl animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Crawl()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_CRAWL)
    {

	    CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();

		m_fTimeStart = m_pServerDE->GetTime();

		if(pW)
	        SetAnimation( m_pAnim_Sound->m_nAnim_CRAWL[pW->GetFireType()]);
		else
	        SetAnimation( m_pAnim_Sound->m_nAnim_CRAWL[TYPE_PISTOL]);

        m_pServerDE->SetModelLooping(m_hObject, DTRUE);

		Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
        
        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_CRAWL;
    }
    else
    {   
		//Check for obstruction; otherwise continue on
		if(CheckObstructed(m_MoveObj.GetForwardVector(), m_fWalkSpeed))
		{
			NavigateObstacle();
			SetNewState(STATE_WalkAroundObj);
		}

		CollisionInfo collisionInfo;
		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);
		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);

		if(collisionInfo.m_hObject && (dwFlags & FLAG_GRAVITY))
		{		
			Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
		}

		//Are we done walking?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE; 
            Metacmd++;
        }
    }
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::Swim
// DESCRIPTION	: Run the swim animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Swim()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_SWIM)
    {
	    CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();

    	m_fTimeStart = m_pServerDE->GetTime();

		if(pW)
	        SetAnimation( m_pAnim_Sound->m_nAnim_SWIM[pW->GetFireType()]);
		else
	        SetAnimation( m_pAnim_Sound->m_nAnim_SWIM[TYPE_PISTOL]);

        m_pServerDE->SetModelLooping(m_hObject, DTRUE);

		Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
        
        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_SWIM;
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
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE; 
            Metacmd++;
        }
    }
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Fire_Stand
// DESCRIPTION	: Run the fire_stand animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Fire_Stand()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_STAND)
    {
		DBOOL bRet = DFALSE;

		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();		
		if (pW)
		{
			m_fAttackLoadTime = pW->GetReloadTime();
		}

    	m_fLoadTimeStart = m_fTimeStart = 0;

		if(pW)
	        bRet = SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_STAND[pW->GetFireType()]);
		else
	        bRet = SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_MELEE]);

		//SCHLEGZ 4/22/98 3:27:12 PM: if we can't stand up, crawl out from underneath
		if(!bRet)
		{
			SetNewState(STATE_CrawlUnderObj);
		}
		else
		{
			m_pServerDE->SetModelLooping(m_hObject, DFALSE);
    
			m_bAnimating = DTRUE; 
			m_nCurMetacmd = MC_FIRE_STAND;
		}

		m_pServerDE->SetNextUpdate(m_hObject, 0.1f);
    }
    else
    {   
		DFLOAT fTemp = m_pServerDE->GetTime();

        if (fTemp > (m_fLoadTimeStart + m_fAttackLoadTime))
        {
            if(Fire())
				m_fLoadTimeStart = m_pServerDE->GetTime();
			else
			{
				m_bAnimating = DFALSE;
				Metacmd++;
			}
        }
        
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
		{
			m_bAnimating = DFALSE;
			Metacmd++;
		}
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Fire_Walk
// DESCRIPTION	: Run the fire_walk animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Fire_Walk()
{
	DFLOAT fTime = m_pServerDE->GetTime();

    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_WALK)
    {
		DBOOL bRet = DFALSE;

		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();
		if (pW)
		{
			m_fAttackLoadTime = pW->GetReloadTime();
		}

    	m_fLoadTimeStart = m_fTimeStart = 0;

		if(pW)
	        bRet = SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_WALK[pW->GetFireType()]);
		else
	        bRet = SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_WALK[TYPE_PISTOL]);

		//SCHLEGZ 4/22/98 3:27:12 PM: if we can't stand up, crawl out from underneath
		if(!bRet)
		{
			SetNewState(STATE_CrawlUnderObj);
			return;
		}

        m_pServerDE->SetModelLooping(m_hObject, DTRUE);
                    
		Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
        
        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FIRE_WALK;
		m_pServerDE->SetNextUpdate(m_hObject, 0.1f);
    }
    else
    {             
		//Check for obstruction; otherwise continue on
		if(CheckObstructed(m_MoveObj.GetForwardVector(), m_fWalkSpeed))
		{
			NavigateObstacle();
		}

		CollisionInfo collisionInfo;
		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);
		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);

		if(collisionInfo.m_hObject && (dwFlags & FLAG_GRAVITY))
		{		
			Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
		}

		//Have we reloaded yet?
        if (fTime > (m_fLoadTimeStart + m_fAttackLoadTime))
        {
            if(Fire())
				m_fLoadTimeStart = m_pServerDE->GetTime();
			else
			{
				m_bAnimating = DFALSE;
				Metacmd++;
			}
        }
	
		//Are we done walking?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE; 
            Metacmd++;
        }
    }      
	
    return;	
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::Script_Fire_Stand
// DESCRIPTION	: Run the fire_stand animation (when scripted)
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

DBOOL AI_Mgr::Script_Fire_Stand()
{
	if (!ChooseOpportunityTarget())  // Make sure we have a target...
	{
		return DFALSE;
	}
	else
	{
		m_pServerDE->GetObjectPos(m_hTarget, &m_vTargetPos);
		MC_FaceTarget();
		Metacmd--;
	}

	if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_STAND)
    {
		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();		
		if (pW)
		{
			m_fAttackLoadTime = pW->GetReloadTime();
		}

    	m_fLoadTimeStart = m_fTimeStart = 0;

		if (pW) SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_STAND[pW->GetFireType()]);
		else SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_MELEE]);

		m_pServerDE->SetModelLooping(m_hObject, DFALSE);
    
		m_bAnimating  = DTRUE; 
		m_nCurMetacmd = MC_FIRE_STAND;
    }
    else
    {   //Have we reloaded yet?             

		DFLOAT fTime = m_pServerDE->GetTime();

  		//Have we reloaded yet?
		if (fTime > (m_fLoadTimeStart + m_fAttackLoadTime))
		{
			MC_FaceTarget();
            if (Fire())
			{
				m_fLoadTimeStart = fTime;
			}
			else
			{
				m_bAnimating = DFALSE;
			}
		}

		if (m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
		{
			m_bAnimating = DFALSE;
		}
    }      
	
    return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::Script_Fire_Walk
// DESCRIPTION	: Run the fire_walk animation (when scripted)
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::Script_Fire_Walk()
{
	DFLOAT fTime = m_pServerDE->GetTime();

    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_WALK)
    {
		DBOOL bRet = DFALSE;

		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();
		if (pW)
		{
			m_fAttackLoadTime = pW->GetReloadTime();
		}

    	m_fLoadTimeStart = m_fTimeStart = 0;

		if (pW)
		{
	        bRet = SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_WALK[pW->GetFireType()]);
		}
		else
		{
	        bRet = SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_WALK[TYPE_PISTOL]);
		}

        m_pServerDE->SetModelLooping(m_hObject, DTRUE);
                    
		Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
        
        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FIRE_WALK;
    }
    else
    {             
		Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);

		//Have we reloaded yet?
		if (fTime > (m_fLoadTimeStart + m_fAttackLoadTime))
		{
			if (ChooseOpportunityTarget())  // Make sure we have a target...
			{
				MC_FaceTarget();
				Fire();
				m_fLoadTimeStart = fTime;
			}
		}
	
		//Are we done walking?
		if (m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE; 
        }
    }      
	
    return;	
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Fire_Run
// DESCRIPTION	: Run the fire_run animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Fire_Run()
{
	DFLOAT fTime = m_pServerDE->GetTime();

    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_RUN)
    {
		DBOOL bRet = DFALSE;

		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();
		if (pW)
		{
			m_fAttackLoadTime = pW->GetReloadTime();
		}

    	m_fLoadTimeStart = m_fTimeStart = 0;

		if(pW)
			bRet = SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_RUN[pW->GetFireType()]);
		else
	        bRet = SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_RUN[TYPE_PISTOL]);

		//SCHLEGZ 4/22/98 3:27:12 PM: if we can't stand up, crawl out from underneath
		if(!bRet)
		{
			SetNewState(STATE_CrawlUnderObj);
			return;
		}

        m_pServerDE->SetModelLooping(m_hObject, DTRUE);

		Move(m_MoveObj.GetForwardVector(),m_fRunSpeed);
        
        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FIRE_RUN;
		m_pServerDE->SetNextUpdate(m_hObject, 0.1f);
    }
    else
    {             
		//Check for obstruction; otherwise continue on
		if(CheckObstructed(m_MoveObj.GetForwardVector(), m_fRunSpeed))
		{
			NavigateObstacle();
		}

		CollisionInfo collisionInfo;
		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);
		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);

		if(collisionInfo.m_hObject && (dwFlags & FLAG_GRAVITY))
		{		
			Move(m_MoveObj.GetForwardVector(),m_fRunSpeed);
		}

		//Have we reloaded yet?
        if (fTime > (m_fLoadTimeStart + m_fAttackLoadTime))
        {
            if(Fire())	
				m_fLoadTimeStart = m_pServerDE->GetTime();
			else
			{
				m_bAnimating = DFALSE;
				Metacmd++;
			}
        }
	
		//Are we done walking?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE; 
            Metacmd++;
        }
    }      
	
    return;	
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::Script_Fire_Run
// DESCRIPTION	: Run the fire_run animation (when scripted)
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::Script_Fire_Run()
{
	DFLOAT fTime = m_pServerDE->GetTime();

    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_RUN)
    {
		DBOOL bRet = DFALSE;

		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();
		if (pW)
		{
			m_fAttackLoadTime = pW->GetReloadTime();
		}

    	m_fLoadTimeStart = m_fTimeStart = 0;

		if (pW)
		{
			bRet = SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_RUN[pW->GetFireType()]);
		}
		else
		{
	        bRet = SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_RUN[TYPE_PISTOL]);
		}

        m_pServerDE->SetModelLooping(m_hObject, DTRUE);
		Move(m_MoveObj.GetForwardVector(),m_fRunSpeed);
        
        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FIRE_RUN;
    }
    else
    {             
		Move(m_MoveObj.GetForwardVector(),m_fRunSpeed);

		//Have we reloaded yet?
		if (fTime > (m_fLoadTimeStart + m_fAttackLoadTime))
		{
			if (ChooseOpportunityTarget())  // Make sure we have a target...
			{
				MC_FaceTarget();
				Fire();
				m_fLoadTimeStart = fTime;
			}
		}
	
		//Are we done walking?
		if (m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE; 
        }
    }      
	
    return;	
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Fire_Jump
// DESCRIPTION	: Run the fire_jump animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Fire_Jump()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_JUMP)
    {
	    CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();
		if (pW)
		{
			m_fAttackLoadTime = pW->GetReloadTime();
		}

    	m_fLoadTimeStart = m_fTimeStart = 0;

		if(pW)
	        SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_JUMP[pW->GetFireType()]);
		else
	        SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_RUN[TYPE_PISTOL]);

        Jump(m_fJumpSpeed, m_fRunSpeed);

        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FIRE_JUMP;
		m_pServerDE->SetNextUpdate(m_hObject, 0.1f);
    }
    else
    {
		//Have we reloaded yet?
        if (m_pServerDE->GetTime() > (m_fLoadTimeStart + m_fAttackLoadTime))
        {
            if(Fire())
				m_fLoadTimeStart = m_pServerDE->GetTime();
			else
			{
				m_bAnimating = DFALSE;
				Metacmd++;
			}
        }
		
        if ( Jump(m_fJumpSpeed, m_fRunSpeed) == DFALSE )
        {
			StopVelocity();

            m_bAnimating = DFALSE; 
            Metacmd++;
        }
    }               
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Fire_Crouch
// DESCRIPTION	: Run the fire_crouch animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Fire_Crouch()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_CROUCH)
    {
		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();
		if (pW)
		{
			m_fAttackLoadTime = pW->GetReloadTime();
		}

    	m_fLoadTimeStart = m_fTimeStart = 0;
   
		if(pW)
	        SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_CROUCH[pW->GetFireType()]);
		else
	        SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_CROUCH[TYPE_PISTOL]);

        m_pServerDE->SetModelLooping(m_hObject, DFALSE);
        
		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FIRE_CROUCH;
		m_pServerDE->SetNextUpdate(m_hObject, 0.1f);
    }
    else
    {   //Have we reloaded yet?             
        if (m_pServerDE->GetTime() > (m_fLoadTimeStart + m_fAttackLoadTime))
        {
            if(Fire())
				m_fLoadTimeStart = m_pServerDE->GetTime();
			else
			{
				m_bAnimating = DFALSE;
				Metacmd++;
			}
        }
        
		//Are we done walking?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE;
            Metacmd++;
            return;
        }
    
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Fire_Crawl
// DESCRIPTION	: Run fire_crawl animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Fire_Crawl()
{
	DFLOAT fTime = m_pServerDE->GetTime();

    if(m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_CRAWL)
    {
		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();
		if (pW)
		{
			m_fAttackLoadTime = pW->GetReloadTime();
		}

    	m_fLoadTimeStart = m_fTimeStart = 0;

		if(pW)
	        SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_CRAWL[pW->GetFireType()]);
		else
	        SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_CRAWL[TYPE_PISTOL]);

        m_pServerDE->SetModelLooping(m_hObject, DTRUE);
                    
		Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
        
        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FIRE_CRAWL;
		m_pServerDE->SetNextUpdate(m_hObject, 0.1f);
    }
    else
    {             
		//Check for obstruction; otherwise continue on
		if(CheckObstructed(m_MoveObj.GetForwardVector(), m_fWalkSpeed))
		{
			NavigateObstacle();
			SetNewState(STATE_WalkAroundObj);
		}

		CollisionInfo collisionInfo;
		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);
		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);

		if(collisionInfo.m_hObject && (dwFlags & FLAG_GRAVITY))
		{		
			Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
		}

		//Have we reloaded yet?
        if (fTime > (m_fLoadTimeStart + m_fAttackLoadTime))
        {
            if(Fire())
				m_fLoadTimeStart = m_pServerDE->GetTime();
			else
			{
				m_bAnimating = DFALSE;
				Metacmd++;
			}
        }
	
		//Are we done walking?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE; 
            Metacmd++;
        }
    }      
	
    return;	
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Roll_Forward
// DESCRIPTION	: run the roll_forward animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Roll_Forward(DBOOL bLoop)
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_ROLL_FORWARD)
    {
        SetAnimation( m_pAnim_Sound->m_nAnim_ROLL_FORWARD);
        
		m_pServerDE->SetModelLooping(m_hObject, bLoop);

		Move(m_MoveObj.GetForwardVector(),m_fRollSpeed);

		m_bAnimating = DTRUE;
		m_nCurMetacmd = MC_ROLL_FORWARD;
	}
	else
	{
		//Check for obstruction; otherwise continue on
		if(CheckObstructed(m_MoveObj.GetForwardVector(), m_fRollSpeed))
		{
            m_bAnimating = DFALSE; 
            Metacmd++;
		}

		CollisionInfo collisionInfo;
		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);
		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);

		if(collisionInfo.m_hObject && (dwFlags & FLAG_GRAVITY))
		{		
			Move(m_MoveObj.GetForwardVector(),m_fRollSpeed);
		}

		//Are we done?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE;
            Metacmd++;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Roll_Backward
// DESCRIPTION	: runthe roll_backward animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Roll_Backward()
{
	DVector vBackward;
	VEC_MULSCALAR(vBackward,m_MoveObj.GetForwardVector(),-1);

    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_ROLL_BACKWARD)
    {
        SetAnimation( m_pAnim_Sound->m_nAnim_ROLL_BACK);
        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		Move(vBackward,m_fRollSpeed);

		m_bAnimating = DTRUE;
		m_nCurMetacmd = MC_ROLL_BACKWARD;
	}
	else
	{
		//Check for obstruction; otherwise continue on
		if(CheckObstructed(vBackward, m_fRollSpeed))
		{
            m_bAnimating = DFALSE; 
            Metacmd++;
		}

		CollisionInfo collisionInfo;
		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);
		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);

		if(collisionInfo.m_hObject && (dwFlags & FLAG_GRAVITY))
		{		
			Move(vBackward,m_fRollSpeed);
		}

		//Are we done?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE;
            Metacmd++;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Roll_Left
// DESCRIPTION	: run the roll_left animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Roll_Left()
{
	DVector vLeft;
	VEC_MULSCALAR(vLeft,m_MoveObj.GetRightVector(),-1);

    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_ROLL_LEFT)
    {
        SetAnimation( m_pAnim_Sound->m_nAnim_ROLL_LEFT);
        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		Move(vLeft,m_fRollSpeed);

		m_bAnimating = DTRUE;
		m_nCurMetacmd = MC_ROLL_LEFT;
	}
	else
	{
		//Check for obstruction; otherwise continue on
		if(CheckObstructed(vLeft, m_fRollSpeed))
		{
            m_bAnimating = DFALSE; 
            Metacmd++;
		}

		CollisionInfo collisionInfo;
		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);
		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);

		if(collisionInfo.m_hObject && (dwFlags & FLAG_GRAVITY))
		{		
			Move(vLeft,m_fRollSpeed);
		}

		//Are we done?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE;
            Metacmd++;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Roll_Right
// DESCRIPTION	: run the roll_right animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Roll_Right()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_ROLL_RIGHT)
    {
        SetAnimation( m_pAnim_Sound->m_nAnim_ROLL_RIGHT);
        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		Move(m_MoveObj.GetRightVector(),m_fRollSpeed);

		m_bAnimating = DTRUE;
		m_nCurMetacmd = MC_ROLL_RIGHT;
	}
	else
	{
		//Check for obstruction; otherwise continue on
		if(CheckObstructed(m_MoveObj.GetRightVector(), m_fRollSpeed))
		{
            m_bAnimating = DFALSE; 
            Metacmd++;
		}

		CollisionInfo collisionInfo;
		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);
		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);

		if(collisionInfo.m_hObject && (dwFlags & FLAG_GRAVITY))
		{		
			Move(m_MoveObj.GetRightVector(),m_fRollSpeed);
		}

		//Are we done?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE;
            Metacmd++;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Dodge_Left
// DESCRIPTION	: run the dodge left animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Dodge_Left()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_DODGE_LEFT)
    {
		m_fTimeStart = m_pServerDE->GetTime();

        SetAnimation( m_pAnim_Sound->m_nAnim_DODGE_LEFT);
        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		DVector vLeft;
		VEC_MULSCALAR(vLeft, m_MoveObj.GetRightVector(), -1.0f);
		Move(vLeft, m_fRollSpeed);

		m_bAnimating = DTRUE;
		m_nCurMetacmd = MC_DODGE_LEFT;
	}
	else
	{
		//Are we done?
		if(m_pServerDE->GetTime() - m_fTimeStart >= 1.0f)
        {
            m_bAnimating = DFALSE;
            Metacmd++;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Dodge_Right
// DESCRIPTION	: run the dodge right animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Dodge_Right()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_DODGE_RIGHT)
    {
		m_fTimeStart = m_pServerDE->GetTime();

        SetAnimation( m_pAnim_Sound->m_nAnim_DODGE_RIGHT);
        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		Move(m_MoveObj.GetRightVector(),m_fRollSpeed);

		m_bAnimating = DTRUE;
		m_nCurMetacmd = MC_DODGE_RIGHT;
	}
	else
	{
		//Are we done?
		if(m_pServerDE->GetTime() - m_fTimeStart >= 1.0f)
        {
            m_bAnimating = DFALSE;
            Metacmd++;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Taunt_Beg
// DESCRIPTION	: play the taunt_beg animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Taunt_Beg()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_TAUNT_BEG)
    {
        SetAnimation( m_pAnim_Sound->m_nAnim_TAUNT[6]);
        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		Move(m_MoveObj.GetRightVector(),m_fRollSpeed);

		m_bAnimating = DTRUE;
		m_nCurMetacmd = MC_TAUNT_BEG;
	}
	else
	{
		//Are we done?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE;
            Metacmd++;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Taunt_Bold
// DESCRIPTION	: Play a random taunt
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Taunt_Bold()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_TAUNT_BOLD)
    {
        SetAnimation( m_pAnim_Sound->m_nAnim_TAUNT[4]);
        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		Move(m_MoveObj.GetRightVector(),m_fRollSpeed);

		m_bAnimating = DTRUE;
		m_nCurMetacmd = MC_TAUNT_BOLD;
	}
	else
	{
		//Are we done?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE;
            Metacmd++;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Recoil
// DESCRIPTION	: Run recoil animation based on node hit
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Recoil()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_RECOIL)
    {
		int nSideHit = m_damage.GetSideHit();
		int nNodeHit = m_damage.GetNodeHit();

		if(m_damage.GetLastDamageType() == DAMAGE_TYPE_FIRE)
		{
			nNodeHit = m_pServerDE->IntRandom(0, NUM_ALL_NODES - 3);
		}

		switch(nNodeHit)
		{
			case NODE_NECK:		SetAnimation( m_pAnim_Sound->m_nAnim_RECOIL[0 + nSideHit]);
								break;
			case NODE_TORSO:	SetAnimation( m_pAnim_Sound->m_nAnim_RECOIL[1 + nSideHit]);
								break;
			case NODE_RARM:		SetAnimation( m_pAnim_Sound->m_nAnim_RECOIL[2 + nSideHit]);
								break;
			case NODE_LARM:		SetAnimation( m_pAnim_Sound->m_nAnim_RECOIL[3 + nSideHit]);
								break;
			case NODE_LLEG:		SetAnimation( m_pAnim_Sound->m_nAnim_RECOIL[4 + nSideHit]);
								break;
			case NODE_RLEG:		SetAnimation( m_pAnim_Sound->m_nAnim_RECOIL[5 + nSideHit]);
								break;
		}

        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		m_bAnimating = DTRUE;
		m_nCurMetacmd = MC_RECOIL;
	}
	else
	{
		//Are we done?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE;
            Metacmd++;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Dead
// DESCRIPTION	: Run death animation based on node hit
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Dead()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_DEAD)
    {
		int nSideHit = m_damage.GetSideHit();

		// 01/13/1999 No more speech-after-death --Loki
		if(m_hCurSound)
		{
			m_pServerDE->KillSound(m_hCurSound);
			m_hCurSound = DNULL;
		}


		if (!(m_dwFlags & FLAG_NIGHTMAREDEATH))
		{
			switch(m_damage.GetNodeHit())
			{
				case NODE_NECK:		m_nCorpseType = m_pAnim_Sound->m_nAnim_DEATH[0 + nSideHit]; break;
				case NODE_TORSO:	m_nCorpseType = m_pAnim_Sound->m_nAnim_DEATH[1 + nSideHit]; break;
				case NODE_RARM:		m_nCorpseType = m_pAnim_Sound->m_nAnim_DEATH[2 + nSideHit]; break;
				case NODE_LARM:		m_nCorpseType = m_pAnim_Sound->m_nAnim_DEATH[3 + nSideHit]; break;
				case NODE_LLEG:		m_nCorpseType = m_pAnim_Sound->m_nAnim_DEATH[4 + nSideHit]; break;
				case NODE_RLEG:		m_nCorpseType = m_pAnim_Sound->m_nAnim_DEATH[5 + nSideHit]; break;
			}
		} 
		else 
		{
			if (IsRandomChance(50)) 
				m_nCorpseType = m_pAnim_Sound->m_nAnim_DEATH[0];
			else
				m_nCorpseType = m_pAnim_Sound->m_nAnim_DEATH[1];
		}
       
//		m_nCorpseType = m_pAnim_Sound->m_nAnim_DEATH[0];
		
		m_bAnimating = DTRUE;
		m_nCurMetacmd = MC_DEAD;
	}
	else
	{
		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
		dwFlags &= ~FLAG_VISIBLE;
		m_pServerDE->SetObjectFlags(m_hObject, dwFlags);

		CreateCorpse();
		m_bRemoveMe = DTRUE;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Special
// DESCRIPTION	: play special animation nIndex
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_Special(int nIndex)
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_SPECIAL)
    {
        SetAnimation( m_pAnim_Sound->m_nAnim_SPECIAL[nIndex]);
        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		m_bAnimating = DTRUE;
		m_nCurMetacmd = MC_SPECIAL;
	}
	else
	{
		//Are we done?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE;
            Metacmd++;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_FaceTarget
// DESCRIPTION	: Turn and face m_hTarget
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_FaceTarget()
{
	if(m_hTarget)
		MC_FacePos(m_vTargetPos);
	else
		Metacmd++;

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_FaceTrackObj
// DESCRIPTION	: Turn and face m_hTrackObj
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_FaceTrackObj()
{
	MC_FacePos(m_vTrackObjPos);

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_FacePos
// DESCRIPTION	: Turn and face vPos
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_FacePos(DVector vPos)
{
	DVector vDir;

	VEC_SUB(vDir, vPos, m_MoveObj.GetPos());
	VEC_NORM(vDir);

	DFLOAT fAmount = (DFLOAT) atan2(vDir.x, vDir.z);
	DFLOAT fAmount2 = (DFLOAT) atan2(m_MoveObj.GetForwardVector().x, m_MoveObj.GetForwardVector().z);
    
	DRotation rRot;
	ROT_COPY(rRot, m_MoveObj.GetRotation());

	m_pServerDE->RotateAroundAxis(&rRot, &m_MoveObj.GetUpVector(), -(fAmount2 - fAmount));
	m_pServerDE->SetObjectRotation(m_hObject, &rRot);

	Metacmd++;

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_BestWeapon
// DESCRIPTION	: Choose best weapon for attacking
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_BestWeapon()
{
    if (m_bAnimating == DFALSE)
    {
		if(m_InventoryMgr.FindBestWeapon(VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos)) == DNULL)
		{            
			CWeapon* pW	= m_InventoryMgr.GetCurrentWeapon();

			if(pW)
			{
				if(pW->GetType() <= WEAP_LASTPLAYERWEAPON)
				{
					if(m_InventoryMgr.GetAmmoCount(pW->GetAmmoType(DFALSE)) <= 0.0)
					{
						SetNewState(STATE_FindAmmo);
						return;
					}
				}
			}
			
			m_bAnimating = DFALSE; 
			Metacmd++;
		}
		else
		{
			CWeapon* pW	= m_InventoryMgr.GetCurrentWeapon();

			if(pW)
				SetAnimation( m_pAnim_Sound->m_nAnim_SWITCH_WEAPON[pW->GetFireType()]);
			else
				SetAnimation( m_pAnim_Sound->m_nAnim_SWITCH_WEAPON[TYPE_PISTOL]);

			m_pServerDE->SetModelLooping(m_hObject, DFALSE);

			m_bAnimating = DTRUE;
		}
	}
	else
	{
		//Are we done?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			CWeapon* pWeapon = m_InventoryMgr.FindBestWeapon(VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos));

			if(pWeapon)
			{
				m_InventoryMgr.ChangeWeapon(pWeapon->GetType());
				pWeapon->ShowHandModel(DFALSE);
		        SetAnimation( m_pAnim_Sound->m_nAnim_SWITCH_WEAPON[pWeapon->GetFireType()]);
			}
			else
			{
				m_bAnimating = DFALSE;
			}
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_LayProximity
// DESCRIPTION	: set proximity bomb on ground
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::MC_LayProximity()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_LAYPROXIMITY)
    {
		m_InventoryMgr.SelectInventoryWeapon(WEAP_PROXIMITYBOMB);
        SetAnimation( m_pAnim_Sound->m_nAnim_PICKUP_WEAPON);
        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		m_bAnimating = DTRUE;
		m_nCurMetacmd = MC_LAYPROXIMITY;
	}
	else
	{
		//Are we done?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			Fire(DTRUE);
			m_InventoryMgr.SelectNextWeapon();
            m_bAnimating = DFALSE;
            Metacmd++;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::MC_Extra
// DESCRIPTION	: Handle extra functionality
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //
void AI_Mgr::MC_Extra(const char *lpszTemp)
{
	// This is an empty function, only implemented in child classes.

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_STATE_Dying
// DESCRIPTION	: STATE_Dying function
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::AI_STATE_Dying()
{
    switch(Metacmd)
    {
        case 1: MC_Dead();							break;
        case 2: m_nState = STATE_Inactive;			break;
    }
	
	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_STATE_Script
// DESCRIPTION	: STATE_Script function
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::AI_STATE_Script()
{
	if (m_bUpdateScriptCmd) 
	{
		UpdateScriptCommand();
	}

	switch(m_curScriptCmd.command)
	{
		case AI_SCMD_SETMOVEMENT:
		case AI_SCMD_PLAYSOUND:
		case AI_SCMD_SETSTATE:
		case AI_SCMD_TARGET:
			m_bUpdateScriptCmd = DTRUE;
		break;
		
		case AI_SCMD_FOLLOWPATH:
			UpdateFollowPathCmd();
		break;

		case AI_SCMD_WAIT:
			UpdateWaitCmd();
		break;
		
		case AI_SCMD_PLAYANIMATION:
			UpdatePlayAnimationCmd();
		break;
		
		case AI_SCMD_MOVETOOBJECT:
			UpdateMoveToObjectCmd();
		break;

		case AI_SCMD_DONE:
		default: 
			m_dwScriptFlags = 0;
			ComputeState();
		break;
	}
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_STATE_AssistAlly
// DESCRIPTION	: AI_STATE_AssistAlly function
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::AI_STATE_AssistAlly()
{
	int nStimType = ComputeStimuli();

	if(nStimType != STIM_SMELL && nStimType > 0)
	{
		IntersectInfo IInfo;
		m_pServerDE->GetObjectPos(m_hTarget, &m_vTargetPos);
		if(m_MoveObj.ClearToPoint(m_MoveObj.GetPos(), m_vTargetPos, m_vDims, &IInfo))
		{
			ComputeState(nStimType);
			return;
		}
	}

	DFLOAT fDim = (DFLOAT)sqrt((m_vDims.x * m_vDims.x) + (m_vDims.z * m_vDims.z)) + 0.1f;

	switch(Metacmd)
	{
		case 1:		MC_FacePos(m_vTrackObjPos);
					
					if(!m_MoveObj.CalculatePath(m_vTrackObjPos))
						SetNewState(STATE_Idle);

					break;
		case 2:	
		{
					DVector* pvPos = m_MoveObj.GetNextPathPoint();
					
					if(pvPos)
						MC_FacePos(*pvPos);	
					else
						SetNewState(STATE_SearchSmellTarget);

					break;
		}
		case 3:		
		{
					if(m_MoveObj.GetNextPathPoint() == DNULL)
						ComputeState();
					else
					{
						DVector vPoint = *m_MoveObj.GetNextPathPoint();
						vPoint.y = m_MoveObj.GetPos().y;

						DFLOAT fThresh = 0.0f;
						if(VEC_DIST(vPoint,m_vTrackObjPos) == 0)
							fThresh = fDim * 2.0f;
						else
							fThresh = fDim/2.0f;

						if(IsLedge(m_MoveObj.GetForwardVector()))
						{
							Metacmd++;
						}
						else if(VEC_DIST(vPoint, m_MoveObj.GetPos()) <= fThresh)
						{
							if(!m_MoveObj.MoveToNextPathPoint())
							{
								Metacmd++;
							}
							else
							{
								Metacmd = 2;
							}
						}
						else
							MC_Run();
					}

					break;
		}
		case 4:		m_hTarget = m_hTrackObject;
					m_pServerDE->GetObjectPos(m_hTarget, &m_vTargetPos);
					MC_FaceTarget();

					break;
		case 5:		ComputeState();					break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_STATE_SearchTarget
// DESCRIPTION	: AI_STATE_SearchTarget function
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::AI_STATE_SearchVisualTarget()
{
	int nStimType = ComputeStimuli();

	// 01/13/1999 Sanity check -- Loki
	if (!m_hTarget) 
	{
		ComputeState();
		return;
	}

	if(nStimType != STIM_SMELL && nStimType > 0)
	{
		IntersectInfo IInfo;
		m_pServerDE->GetObjectPos(m_hTarget, &m_vTargetPos);
		if(m_MoveObj.ClearToPoint(m_MoveObj.GetPos(), m_vTargetPos, m_vDims, &IInfo))
		{
			ComputeState(nStimType);
			return;
		}
	}

	DFLOAT fDim = (DFLOAT)sqrt((m_vDims.x * m_vDims.x) + (m_vDims.z * m_vDims.z)) + 0.1f;

	switch(Metacmd)
	{
		case 1:		MC_FacePos(m_vTargetPos);
					
					if(!m_MoveObj.CalculatePath(m_vTargetPos))
						SetNewState(STATE_Idle);

					break;
		case 2:	
		{
					DVector* pvPos = m_MoveObj.GetNextPathPoint();
					
					if(pvPos)
						MC_FacePos(*pvPos);	
					else
						SetNewState(STATE_SearchSmellTarget);

					break;
		}
		case 3:		
		{
					if(m_MoveObj.GetNextPathPoint() == DNULL || IsLedge(m_MoveObj.GetForwardVector()))
						ComputeState();
					else
					{
						DVector vPoint = *m_MoveObj.GetNextPathPoint();
						vPoint.y = m_MoveObj.GetPos().y;

						if(IsLedge(m_MoveObj.GetForwardVector()))
						{
							Metacmd++;
						}
						else if(VEC_DIST(vPoint, m_MoveObj.GetPos()) <= fDim/2)
						{
							if(!m_MoveObj.MoveToNextPathPoint())
							{
								ComputeState();
							}
							else
							{
								Metacmd = 2;
							}
						}
						else
							MC_Run();
					}

					break;
		}
		case 4:		ComputeState();					break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_STATE_SearchTarget
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::AI_STATE_SearchSmellTarget()
{
	int nStimType = ComputeStimuli();

	// 01/13/1999 Sanity check -- Loki
	if (!m_hTarget) 
	{
		ComputeState();
		return;
	}

	if(nStimType != STIM_SMELL && nStimType > 0)
	{
		ComputeState(nStimType);
		return;
	}

	if(m_pCurSmell == DNULL)
	{
		ComputeState();
		return;
	}

	DFLOAT fDim = (DFLOAT)sqrt((m_vDims.x * m_vDims.x) + (m_vDims.z * m_vDims.z)) + 0.1f;

	switch(Metacmd)
	{
		case 1:	
		{
					HCLASS hClass = m_pServerDE->GetClass("SmellHint");
					HOBJECT hSmell = FindObjectInRadius(hClass, m_fSmellingDist, FIND_VISIBLE);

					if(hSmell == DNULL)
					{
						ComputeState();
						return;
					}

					if(m_pCurSmell)
					{
						m_hCurSmell = ((SmellHint*)m_pCurSmell->m_pData)->m_hObject;
						m_pServerDE->BreakInterObjectLink(m_hObject, m_hCurSmell);
					}

					m_pCurSmell = SmellHint::HandleToLink(hSmell);
					m_pServerDE->CreateInterObjectLink(m_hObject, ((SmellHint*)m_pCurSmell->m_pData)->m_hObject);

					if(m_pCurSmell == DNULL)
					{
						SetNewState(m_nLastState);
					}
					else if(m_pCurSmell->m_pData)
					{
						SmellHint* pHint = (SmellHint*)m_pCurSmell->m_pData;
				
						HOBJECT hObj = m_pServerDE->ObjectToHandle(pHint);
						m_pServerDE->GetObjectPos(hObj, &m_vSmellPos);
						MC_FacePos(m_vSmellPos);	
					}
					else
						SetNewState(m_nLastState);

					break;
		}
		case 2:		
		{
					if(IsLedge(m_MoveObj.GetForwardVector()))
						ComputeState();
					else
					{
						SmellHint* pHint = (SmellHint*)m_pCurSmell->m_pData;

						DVector vPoint;
						m_pServerDE->GetObjectPos(pHint->m_hObject, &vPoint);
						vPoint.y = m_MoveObj.GetPos().y;

						if(IsLedge(m_MoveObj.GetForwardVector()))
						{
							Metacmd++;
						}
						else if(VEC_DIST(vPoint, m_MoveObj.GetPos()) <= fDim/2)
						{
							if(m_pCurSmell->m_pNext->m_pData == DNULL)
								Metacmd++;
							else
							{
								m_pCurSmell = m_pCurSmell->m_pNext;

								if(m_pCurSmell)
								{
									m_hCurSmell = ((SmellHint*)m_pCurSmell->m_pData)->m_hObject;
									m_pServerDE->BreakInterObjectLink(m_hObject, m_hCurSmell);
								}

								if(m_pCurSmell->m_pData)
								{
									SmellHint* pHint = (SmellHint*)m_pCurSmell->m_pData;
							
									HOBJECT hObj = m_pServerDE->ObjectToHandle(pHint);
									m_pServerDE->GetObjectPos(hObj, &m_vSmellPos);
									MC_FacePos(m_vSmellPos);	
								}
							}
						}
						else
							MC_Run();
					}

					break;
		}
		case 3:		ComputeState();					break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_STATE_FindAmmo
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::AI_STATE_FindAmmo()
{
	DFLOAT fDim = (DFLOAT)sqrt((m_vDims.x * m_vDims.x) + (m_vDims.z * m_vDims.z)) + 0.1f;

	switch(Metacmd)
	{
		case 1:		
		{
					HCLASS hAmmo = DNULL;

					CWeapon* pW = m_InventoryMgr.GetCurrentWeapon();

					if(pW == DNULL)
					{
						SetNewState(STATE_Idle);
						return;
					}

					switch(pW->GetAmmoType())
					{
						case AMMO_BULLET:		hAmmo = m_pServerDE->GetClass("BulletAmmoPU");	break;
						case AMMO_SHELL:		hAmmo = m_pServerDE->GetClass("ShellAmmoPU");	break;
						case AMMO_BMG:			hAmmo = m_pServerDE->GetClass("BMGAmmoPU");		break;
						case AMMO_FLARE:		hAmmo = m_pServerDE->GetClass("FlareAmmoPU");	break;
						case AMMO_DIEBUGDIE:	hAmmo = m_pServerDE->GetClass("DieBugDieAmmoPU");	break;
						case AMMO_HOWITZER:		hAmmo = m_pServerDE->GetClass("RocketAmmoPU");	break;
						case AMMO_FUEL:			hAmmo = m_pServerDE->GetClass("FuelAmmoPU");	break;
						case AMMO_BATTERY:		hAmmo = m_pServerDE->GetClass("BatteryAmmoPU");	break;
					}

					m_hTrackObject = FindObjectInRadius(hAmmo, m_fSeeingDist, FIND_AVOID_TARGET | FIND_VISIBLE | FIND_SPECIFIC_OBJ);

					if(m_hTrackObject == DNULL)
					{
						SetNewState(STATE_Escape_RunAway);
					}
					else
					{
						m_pServerDE->GetObjectPos(m_hTrackObject,&m_vTrackObjPos);

						MC_FacePos(m_vTrackObjPos);
						m_MoveObj.CalculatePath(m_vTrackObjPos);		
					}

					break;
		}
		case 2:	
		{
					DVector* pvPos = m_MoveObj.GetNextPathPoint();
					
					if(pvPos)
						MC_FacePos(*pvPos);	
					else
						SetNewState(STATE_Escape_RunAway);

					break;
		}
		case 3:		
		{			
					if(m_MoveObj.GetNextPathPoint() == DNULL)
						ComputeState();
					else
					{
						DVector vPoint = *m_MoveObj.GetNextPathPoint();
						vPoint.y = m_MoveObj.GetPos().y;

						if(IsLedge(m_MoveObj.GetForwardVector()))
						{
							Metacmd++;
						}
						else if(VEC_DIST(vPoint, m_MoveObj.GetPos()) <= fDim/2)
						{
							if(!m_MoveObj.MoveToNextPathPoint() || IsLedge(m_MoveObj.GetForwardVector()))
							{
								MC_FacePos(m_vTargetPos);
							}
							else
							{
								Metacmd = 2;
							}
						}
						else
							MC_Run();
					}

					break;
		}
		case 4:		ComputeState();					break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_STATE_FindHealth
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::AI_STATE_FindHealth()
{
	if(Metacmd > 1)
	{
		int nStimType = ComputeStimuli();

		if(nStimType == STIM_SMELL)
		{
			SetNewState(STATE_SearchSmellTarget);
			return;
		}
		else if(nStimType > 0)
		{
			ComputeState(nStimType);
			return;
		}

	}

	DFLOAT fDim = (DFLOAT)sqrt((m_vDims.x * m_vDims.x) + (m_vDims.z * m_vDims.z)) + 0.1f;

	switch(Metacmd)
	{
		case 1:		
		{
					HCLASS hAmmo = m_pServerDE->GetClass("HealthPU");
					m_hTrackObject = FindObjectInRadius(hAmmo, m_fSeeingDist, FIND_AVOID_TARGET | FIND_VISIBLE);

					if(m_hTrackObject == DNULL)
					{
						SetNewState(STATE_Escape_RunAway);
					}
					else
					{
						m_pServerDE->GetObjectPos(m_hTrackObject,&m_vTrackObjPos);

						MC_FacePos(m_vTrackObjPos);
						m_MoveObj.CalculatePath(m_vTrackObjPos);		
					}

					break;
		}
		case 2:	
		{
					DVector* pvPos = m_MoveObj.GetNextPathPoint();
					
					if(pvPos)
						MC_FacePos(*pvPos);	
					else
						SetNewState(STATE_Escape_RunAway);

					break;
		}
		case 3:		
		{	
					if(m_MoveObj.GetNextPathPoint() == DNULL)
						ComputeState();
					else
					{
						DVector vPoint = *m_MoveObj.GetNextPathPoint();
						vPoint.y = m_MoveObj.GetPos().y;

						if(IsLedge(m_MoveObj.GetForwardVector()))
						{
							Metacmd++;
						}
						else if(VEC_DIST(vPoint, m_MoveObj.GetPos()) <= fDim/2)
						{
							if(!m_MoveObj.MoveToNextPathPoint() || IsLedge(m_MoveObj.GetForwardVector()))
							{
								MC_FacePos(m_vTargetPos);
							}
							else
							{
								Metacmd = 2;
							}
						}
						else
							MC_Run();
					}

					break;
		}
		case 4:		ComputeState();					break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_STATE_GuardLocation
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::AI_STATE_GuardLocation()
{
	int nStimType = ComputeStimuli();

	if(nStimType == STIM_SMELL)
	{
		SetNewState(STATE_SearchSmellTarget);
		return;
	}
	else if(nStimType > 0)
	{
		ComputeState(nStimType);
		return;
	}

	DFLOAT fDim = (DFLOAT)sqrt((m_vDims.x * m_vDims.x) + (m_vDims.z * m_vDims.z)) + 0.1f;

	switch(Metacmd)
	{
		case 1:		if(VEC_DIST(m_MoveObj.GetPos(), m_vGuardPos) <= fDim/2)
						Metacmd = 4;
					else
					{
						MC_FacePos(m_vGuardPos);

						if(!m_MoveObj.CalculatePath(m_vGuardPos))
							SetNewState(STATE_Idle);
					}

					break;
		case 2:	
		{
					DVector* pvPos = m_MoveObj.GetNextPathPoint();
					
					if(pvPos)
						MC_FacePos(*pvPos);	
					else
						SetNewState(STATE_Escape_RunAway);

					break;
		}
		case 3:		
		{			if(m_MoveObj.GetNextPathPoint() == DNULL || IsLedge(m_MoveObj.GetForwardVector()))
						ComputeState();
					else
					{
						DVector vPoint = *m_MoveObj.GetNextPathPoint();
						vPoint.y = m_MoveObj.GetPos().y;

						if(IsLedge(m_MoveObj.GetForwardVector()))
						{
							Metacmd++;
						}
						else if(VEC_DIST(vPoint, m_MoveObj.GetPos()) <= fDim/2)
						{
							if(!m_MoveObj.MoveToNextPathPoint())
							{
								MC_FacePos(m_vTargetPos);
							}
							else
							{
								Metacmd = 2;
							}
						}
						else
							MC_Run();
					}

					break;
		}
		case 4:		MC_Idle();				break;
		case 5:		ComputeState();			break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_STATE_Idle
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::AI_STATE_Idle()
{
	int nStimType = ComputeStimuli();

	if(nStimType == STIM_SMELL)
	{
		SetNewState(STATE_SearchSmellTarget);
		return;
	}
	else if(nStimType > 0)
	{
		m_pServerDE->SetNextUpdate(m_hObject, 0.001f);
		ComputeState(nStimType);
		return;
	}

	m_pServerDE->SetNextUpdate(m_hObject, 0.1f);

	switch(Metacmd)
	{
		case 1:		if(m_bCabal)
						MC_BestWeapon();
					else
						Metacmd++;
				
					break;
		case 2:		MC_Idle();			break;
		case 3:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_STATE_Passive
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::AI_STATE_Passive()
{
	if(m_damage.GetLastDamager())
	{
		ComputeState();
		return;
	}

	m_pServerDE->SetNextUpdate(m_hObject, 0.1f);

	switch(Metacmd)
	{
		case 1:		if(m_bCabal)
						MC_BestWeapon();
					else
						Metacmd++;
				
					break;
		case 2:		MC_Idle();			break;
		case 3:		Metacmd = 1;		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_STATE_Recoil
// DESCRIPTION	: STATE_Recoil function
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::AI_STATE_Recoil()
{
    switch(Metacmd)
    {
        case 1: MC_Recoil();						break;
        case 2: ComputeState();						break;
    }
	
	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_STATE_WalkAroundObj
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::AI_STATE_WalkAroundObj()
{
	int nStimType = ComputeStimuli();

	if(nStimType != STIM_SMELL && nStimType > 0)
	{
		IntersectInfo IInfo;
		DVector vPos;

		VEC_COPY(vPos, m_MoveObj.GetPos());
		vPos.y = m_vDestPos.y;

		if(m_MoveObj.ClearToPoint(vPos, m_vDestPos, m_vDims, &IInfo))
		{
			ComputeState(nStimType);
			return;
		}
	}

	DVector vDims;
	VEC_COPY(vDims, m_vDims);
	DFLOAT fDim = (DFLOAT)sqrt((vDims.x * vDims.x) + (vDims.z * vDims.z)) + 0.1f;

	switch(Metacmd)
	{
		case 1:		MC_FacePos(m_vDestPos);

					if(!m_MoveObj.CalculatePath(m_vDestPos))
						SetNewState(STATE_Idle);

					break;
		case 2:	
		{
					DVector* pvPos = m_MoveObj.GetNextPathPoint();
					
					if(pvPos)
						MC_FacePos(*pvPos);	
					else
						SetNewState(STATE_SearchSmellTarget);

					break;
		}
		case 3:		
		{
					if(m_MoveObj.GetNextPathPoint() == DNULL)
						ComputeState();
					else
					{
						DVector vPoint = *m_MoveObj.GetNextPathPoint();
						vPoint.y = m_MoveObj.GetPos().y;

						if(VEC_DIST(vPoint, m_MoveObj.GetPos()) <= fDim/2)
						{
							if(!m_MoveObj.MoveToNextPathPoint() || IsLedge(m_MoveObj.GetForwardVector()))
							{
								ComputeState();
							}
							else
							{
								Metacmd = 2;
							}
						}
						else
							MC_Walk();
					}

					break;
		}
		case 4:		ComputeState();					break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_STATE_RunAroundObj
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::AI_STATE_RunAroundObj()
{
	int nStimType = ComputeStimuli();

	if(nStimType != STIM_SMELL && nStimType > 0)
	{
		IntersectInfo IInfo;
		DVector vPos;

		VEC_COPY(vPos, m_MoveObj.GetPos());
		vPos.y = m_vDestPos.y;

		if(m_MoveObj.ClearToPoint(vPos, m_vDestPos, m_vDims, &IInfo))
		{
			ComputeState(nStimType);
			return;
		}
	}

	DVector vDims;
	VEC_COPY(vDims, m_vDims);
	DFLOAT fDim = (DFLOAT)sqrt((vDims.x * vDims.x) + (vDims.z * vDims.z)) + 0.1f;

	switch(Metacmd)
	{
		case 1:		MC_FacePos(m_vDestPos);

					if(!m_MoveObj.CalculatePath(m_vDestPos))
						SetNewState(STATE_Idle);

					break;
		case 2:	
		{
					DVector* pvPos = m_MoveObj.GetNextPathPoint();
					
					if(pvPos)
						MC_FacePos(*pvPos);	
					else
						SetNewState(STATE_SearchSmellTarget);

					break;
		}
		case 3:		
		{
					if(m_MoveObj.GetNextPathPoint() == DNULL)
						ComputeState();
					else
					{
						DVector vPoint = *m_MoveObj.GetNextPathPoint();
						vPoint.y = m_MoveObj.GetPos().y;

						if(VEC_DIST(vPoint, m_MoveObj.GetPos()) <= fDim/2)
						{
							if(!m_MoveObj.MoveToNextPathPoint() || IsLedge(m_MoveObj.GetForwardVector()))
							{
								ComputeState();
							}
							else
							{
								Metacmd = 2;
							}
						}
						else
							MC_Run();
					}

					break;
		}
		case 4:		ComputeState();					break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_STATE_JumpOverObj
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::AI_STATE_JumpOverObj()
{
    switch(Metacmd)
    {
        case 1: MC_Jump();							break;
        case 2: SetNewState(m_nLastState);			break;
    }
	
	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_STATE_CrawlUnderObj
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::AI_STATE_CrawlUnderObj()
{
    switch(Metacmd)
    {
        case 1: if(m_nLastState == STATE_AttackClose)
					MC_Fire_Crawl();						
				else
					MC_Crawl();
				
				break;
        case 2: SetNewState(m_nLastState);			break;
    }
	
	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_STATE_WalkToPos
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::AI_STATE_WalkToPos()
{
	DVector vDims;
	VEC_COPY(vDims, m_vDims);
	DFLOAT fDim = (DFLOAT)sqrt((vDims.x * vDims.x) + (vDims.z * vDims.z)) + 0.1f;

    switch(Metacmd)
    {
		case 1:		if(!m_MoveObj.CalculatePath(m_vDestPos))
						SetNewState(STATE_Idle);
					else
						Metacmd++;									
					
					break;
		case 2:	
		{
					DVector* pvPos = m_MoveObj.GetNextPathPoint();
					
					if(pvPos)
						MC_FacePos(*pvPos);	
					else
						SetNewState(m_nLastState);

					break;
		}
		case 3:		
		{
					DVector vPoint = *m_MoveObj.GetNextPathPoint();
					vPoint.y = m_MoveObj.GetPos().y;

					if(VEC_DIST(vPoint, m_MoveObj.GetPos()) <= fDim/2)
					{
						if(!m_MoveObj.MoveToNextPathPoint() || IsLedge(m_MoveObj.GetForwardVector()))
							Metacmd++;
						else
						{
							Metacmd = 2;
						}
					}
					else
						MC_Walk();

					break;
		}
		case 4:		SetNewState(m_nLastState);					break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Mgr::AI_STATE_RunToPos
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AI_Mgr::AI_STATE_RunToPos()
{
	DVector vDims;
	VEC_COPY(vDims, m_vDims);
	DFLOAT fDim = (DFLOAT)sqrt((vDims.x * vDims.x) + (vDims.z * vDims.z)) + 0.1f;

    switch(Metacmd)
    {
		case 1:		if(!m_MoveObj.CalculatePath(m_vDestPos))
						SetNewState(STATE_Idle);
					else
						Metacmd++;									
					
					break;
		case 2:	
		{
					DVector* pvPos = m_MoveObj.GetNextPathPoint();
					
					if(pvPos)
						MC_FacePos(*pvPos);	
					else
						SetNewState(m_nLastState);

					break;
		}
		case 3:		
		{
					DVector vPoint = *m_MoveObj.GetNextPathPoint();
					vPoint.y = m_MoveObj.GetPos().y;

					if(VEC_DIST(vPoint, m_MoveObj.GetPos()) <= fDim/2)
					{
						if(!m_MoveObj.MoveToNextPathPoint() || IsLedge(m_MoveObj.GetForwardVector()))
							Metacmd++;
						else
						{
							Metacmd = 2;
						}
					}
					else
						MC_Run();

					break;
		}
		case 4:		SetNewState(m_nLastState);					break;
	}

	return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Mgr::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void AI_Mgr::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	DFLOAT fTime = pServerDE->GetTime();

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hTarget);
	pServerDE->WriteToMessageVector(hWrite, &m_vTargetPos);
	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hTrackObject);
	pServerDE->WriteToMessageVector(hWrite, &m_vTrackObjPos);
	pServerDE->WriteToMessageVector(hWrite, &m_vGuardPos);

	pServerDE->WriteToMessageVector(hWrite, &m_vDestPos);

	pServerDE->WriteToMessageDWord(hWrite, (DDWORD)m_nState);
	pServerDE->WriteToMessageDWord(hWrite, (DDWORD)m_nLastState);

	for(int i = 0; i < NUM_STIMULI; i++)
		pServerDE->WriteToMessageFloat(hWrite, m_fStimuli[i]);

	pServerDE->WriteToMessageDWord(hWrite, (DDWORD) m_nDodgeFlags);
 
	//conditions
	pServerDE->WriteToMessageHString(hWrite, m_hstrSpotTriggerTarget);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSpotTriggerMessage);
	pServerDE->WriteToMessageHString(hWrite, m_hstrTriggerRelayTarget);

    // Adjustable vars
	pServerDE->WriteToMessageFloat(hWrite, m_fHearingDist);
	pServerDE->WriteToMessageFloat(hWrite, m_fSmellingDist);
	pServerDE->WriteToMessageFloat(hWrite, m_fSensingDist);
	pServerDE->WriteToMessageFloat(hWrite, m_fSeeingDist);
				
	pServerDE->WriteToMessageFloat(hWrite, m_fWalkSpeed);
	pServerDE->WriteToMessageFloat(hWrite, m_fRunSpeed);
	pServerDE->WriteToMessageFloat(hWrite, m_fJumpSpeed);
	pServerDE->WriteToMessageFloat(hWrite, m_fRollSpeed);

	pServerDE->WriteToMessageFloat(hWrite, m_fTimeStart - fTime);			
	pServerDE->WriteToMessageFloat(hWrite, m_fLoadTimeStart - fTime);			
												
	pServerDE->WriteToMessageDWord(hWrite, (DDWORD) m_nCorpseType);
				
	pServerDE->WriteToMessageFloat(hWrite, m_fAttackLoadTime);
	
	pServerDE->WriteToMessageByte(hWrite, m_bAnimating);
	pServerDE->WriteToMessageByte(hWrite, m_bJumping);

	pServerDE->WriteToMessageDWord(hWrite, (DDWORD) Metacmd);
	pServerDE->WriteToMessageDWord(hWrite, (DDWORD) m_nCurMetacmd);

	pServerDE->WriteToMessageDWord(hWrite, m_dwFlags);

	pServerDE->WriteToMessageDWord(hWrite, (DDWORD) m_nBlockFlags);

	pServerDE->WriteToMessageByte(hWrite, m_bSetShutdown);

	pServerDE->WriteToMessageFloat(hWrite, m_fLastUpdate);

	pServerDE->WriteToMessageDWord(hWrite, (DDWORD) m_nInjuredLeg);

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hEnemyAttach);

	pServerDE->WriteToMessageVector(hWrite, &m_vScale);

	pServerDE->WriteToMessageFloat(hWrite, m_fLastLedgeCheck);

	pServerDE->WriteToMessageDWord(hWrite, m_nCurAnimation);
	pServerDE->WriteToMessageVector(hWrite, &m_vDims);

	pServerDE->WriteToMessageByte(hWrite, m_bStartFire);

	// Scripting stuff

	pServerDE->WriteToMessageFloat(hWrite, (DFLOAT)m_nScriptCmdIndex);
	pServerDE->WriteToMessageFloat(hWrite, m_fScriptWaitEnd);
	pServerDE->WriteToMessageDWord(hWrite, m_dwScriptFlags);
	pServerDE->WriteToMessageByte(hWrite, m_eScriptMovement);
	pServerDE->WriteToMessageByte(hWrite, m_bUpdateScriptCmd);

	m_curScriptCmd.Save(hWrite);
	m_scriptCmdList.Save(hWrite);
	m_AIPathList.Save(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Mgr::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void AI_Mgr::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	DFLOAT fTime = pServerDE->GetTime();

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hTarget);
	pServerDE->ReadFromMessageVector(hRead, &m_vTargetPos);
	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hTrackObject);
	pServerDE->ReadFromMessageVector(hRead, &m_vTrackObjPos);
	pServerDE->ReadFromMessageVector(hRead, &m_vGuardPos);

	pServerDE->ReadFromMessageVector(hRead, &m_vDestPos);

	m_nState = (int)pServerDE->ReadFromMessageDWord(hRead);
	m_nLastState= (int)pServerDE->ReadFromMessageDWord(hRead);
    
	for(int i = 0; i < NUM_STIMULI; i++)
		m_fStimuli[i] = pServerDE->ReadFromMessageFloat(hRead);

	m_nDodgeFlags		= (int)pServerDE->ReadFromMessageDWord(hRead);

	//conditions
	m_hstrSpotTriggerTarget= pServerDE->ReadFromMessageHString(hRead);
	m_hstrSpotTriggerMessage= pServerDE->ReadFromMessageHString(hRead);
	m_hstrTriggerRelayTarget= pServerDE->ReadFromMessageHString(hRead);

    // Adjustable vars				
	m_fHearingDist		= pServerDE->ReadFromMessageFloat(hRead);
	m_fSmellingDist		= pServerDE->ReadFromMessageFloat(hRead);
	m_fSensingDist		= pServerDE->ReadFromMessageFloat(hRead);
	m_fSeeingDist		= pServerDE->ReadFromMessageFloat(hRead);
				
	m_fWalkSpeed		= pServerDE->ReadFromMessageFloat(hRead);
	m_fRunSpeed			= pServerDE->ReadFromMessageFloat(hRead);
	m_fJumpSpeed		= pServerDE->ReadFromMessageFloat(hRead);
	m_fRollSpeed		= pServerDE->ReadFromMessageFloat(hRead);

	m_fTimeStart		= pServerDE->ReadFromMessageFloat(hRead) + fTime;			
	m_fLoadTimeStart	= pServerDE->ReadFromMessageFloat(hRead) + fTime;			

	m_nCorpseType		= (int)pServerDE->ReadFromMessageDWord(hRead);
				
	m_fAttackLoadTime	= pServerDE->ReadFromMessageFloat(hRead);
												
	m_bAnimating		= pServerDE->ReadFromMessageByte(hRead);
	m_bJumping			= pServerDE->ReadFromMessageByte(hRead);

	Metacmd				= (int)pServerDE->ReadFromMessageDWord(hRead);
	m_nCurMetacmd		= (int)pServerDE->ReadFromMessageDWord(hRead);

	m_dwFlags			= pServerDE->ReadFromMessageDWord(hRead);

	m_nBlockFlags		= (int)pServerDE->ReadFromMessageDWord(hRead);

	m_bSetShutdown		= pServerDE->ReadFromMessageByte(hRead);

	m_fLastUpdate		= pServerDE->ReadFromMessageFloat(hRead) + fTime;

	m_nInjuredLeg		= (int)pServerDE->ReadFromMessageDWord(hRead);

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hEnemyAttach);

	pServerDE->ReadFromMessageVector(hRead, &m_vScale);

	m_fLastLedgeCheck	= pServerDE->ReadFromMessageFloat(hRead) + fTime;

	m_nCurAnimation		= pServerDE->ReadFromMessageDWord(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vDims);

	m_bStartFire		= pServerDE->ReadFromMessageByte(hRead);

	// Scripting stuff...

	m_nScriptCmdIndex	= (int)	pServerDE->ReadFromMessageFloat(hRead);
	m_fScriptWaitEnd	= pServerDE->ReadFromMessageFloat(hRead);
	m_dwScriptFlags		= pServerDE->ReadFromMessageDWord(hRead);
	m_eScriptMovement	= (AIScriptMovement) pServerDE->ReadFromMessageByte(hRead);
	m_bUpdateScriptCmd	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);

	m_curScriptCmd.Load(hRead);
	m_scriptCmdList.Load(hRead);
	m_AIPathList.Load(hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Mgr::CacheFileRange
//
//	PURPOSE:	Caches a range of files
//
// ----------------------------------------------------------------------- //

void AI_Mgr::CacheSoundFileRange(char* sBase, int nFirst, int nLast)
{
	// Sanity checks...

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if(!(pServerDE->GetServerFlags() & SS_CACHING))
	{
		return;
	}

	if (!sBase) return;
	if (sBase[0] == '\0') return;
	if (nFirst > nLast) return;


	// Cache the file range...

	char sFile[512];

	for (int i = nFirst; i <= nLast; i++)
	{
		if (g_sCacheDir[0] == '\0')
		{
			sprintf(sFile, "%s%i.wav", sBase, i);
		}
		else
		{
			sprintf(sFile, "%s\\%s%i.wav", g_sCacheDir, sBase, i);
		}

		pServerDE->CacheFile(FT_SOUND, sFile);
	}
}

void AI_Mgr::CacheSoundFile(char* sBase)
{
	// Sanity checks...

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if(!(pServerDE->GetServerFlags() & SS_CACHING))
	{
		return;
	}

	if (!sBase) return;
	if (sBase[0] == '\0') return;


	// Cache the file...

	char sFile[512];

	if (g_sCacheDir[0] == '\0')
	{
		sprintf(sFile, "%s.wav", sBase);
	}
	else
	{
		sprintf(sFile, "%s\\%s.wav", g_sCacheDir, sBase);
	}

	pServerDE->CacheFile(FT_SOUND, sFile);
}

void AI_Mgr::SetCacheDirectory(char* sDir)
{
	strcpy(g_sCacheDir, sDir);
}