// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include <stdio.h>
#include "AIHuman.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "Destructible.h"
#include "Weapons.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "VolumeBrushTypes.h"
#include "HHWeaponModel.h"
#include "Spawner.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"
#include "GameServerShell.h"
#include "AIState.h"
#include "AIButeMgr.h"
#include "SurfaceFunctions.h"
#include "Attachments.h"
#include "AIPathMgr.h"
#include "CharacterHitBox.h"
#include "AITarget.h"
#include "TeleportPoint.h"
#include "AIGoalMgr.h"
#include "AIGoalAbstract.h"
#include "DEditColors.h"
#include "RotatingDoor.h"
#include "AIGoalButeMgr.h"
#include "RelationMgr.h"
#include "AIBrain.h"
#include "AIStimulusMgr.h"
#include "WeaponFireInfo.h"
#include "Weapon.h"
#include "Attachments.h"
#include "Prop.h"
#include "AIUtils.h"
#include "AIVolumeMgr.h"
#include "AIMovement.h"
#include "AnimationMgr.h"
#include "AIHumanState.h"
#include "AIMovementModifier.h"
#include "AINodeSensing.h"
#include "SharedFXStructs.h"
#include "ServerTrackedNodeContext.h"
#include "RelationButeMgr.h"

LINKFROM_MODULE( AIHuman );

extern CAIGoalButeMgr* g_pAIGoalButeMgr;
extern CAIStimulusMgr* g_pAIStimulusMgr;
// Cvars

static CVarTrack g_CheapMovementTrack;

LTFLOAT s_fSenseUpdateBasis = 0.0f;

#define FIND_FLOOR_THRESH_SQR 4.f


// Define our properties (what is available in DEdit)...
#pragma force_active on
BEGIN_CLASS(CAIHuman)

	ADD_VECTORPROP_VAL_FLAG(Dims, 24.0f, 53.0f, 24.0f, PF_DIMS|PF_HIDDEN)
	ADD_DEDIT_COLOR( CAIHuman )

	// New properties

	ADD_STRINGPROP_FLAG(ModelTemplate, "Default", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(Brain, "Default", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(GoalSet, "None", PF_STATICLIST)

	ADD_BOOLPROP_FLAG(UseDefaultAttachments,	LTTRUE,	PF_GROUP(6))
	ADD_BOOLPROP_FLAG(HolsterWeapons,			LTFALSE,PF_GROUP(6))
	ADD_HUMANATTACHMENTS_AGGREGATE()

	PROP_DEFINEGROUP(AttributeOverrides, PF_GROUP(3))

		ADD_STRINGPROP_FLAG(JumpSpeed,		"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(WalkSpeed,		"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(SwimSpeed,		"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(RunSpeed,		"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(SentryChallengeScanDistMax,		"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(SentryChallengeDistMax,			"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(SentryMarkDistMax,				"", PF_GROUP(3))

	ADD_SEARCHABLE_AGGREGATE(PF_GROUP(5), 0)
	ADD_STRINGPROP_FLAG(RandomItemSet, "Bodies General", PF_GROUP(5) | PF_STATICLIST  ) 

END_CLASS_DEFAULT_FLAGS_PLUGIN(CAIHuman, CAI, NULL, NULL, NULL, CAIHumanPlugin)
#pragma force_active off


//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( CAIHuman )

//					Message		Num Params	Validation FnPtr		Syntax

CMDMGR_END_REGISTER_CLASS( CAIHuman, CAI )







// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::CAIHuman()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

#ifndef __PSX2
#pragma warning (push)
#pragma warning (disable: 4355)
#endif  // !__PSX2
CAIHuman::CAIHuman() : CAI()
#ifndef __PSX2
#pragma warning (pop)
#endif  // !__PSX2
{
	m_bForceGround = LTFALSE;

	if (!g_CheapMovementTrack.IsInitted())
	{
        g_CheapMovementTrack.Init(g_pLTServer, "AICheapMovement", LTNULL, 0.0f);
	}

	m_fJumpVel		= 0.0f;
	m_fJumpOverVel	= 0.0f;
	m_fFallVel		= 0.0f;
	m_fWalkVel		= 0.0f;
	m_fSwimVel		= 0.0f;
	m_fRunVel		= 0.0f;

	m_bBlink			= LTTRUE;
	m_bShortRecoil		= LTTRUE;

	m_bCanShortRecoil	= LTTRUE;

	m_pHumanState = LTNULL;

	m_bSyncPosition = LTFALSE;

	m_bPosDirty = LTTRUE;

	m_vLastFindFloorPos = LTVector( 99999999.f, 99999999.f, 99999999.f );

	m_hHeadNode = INVALID_MODEL_NODE;

	m_hLastDoor[0] = LTNULL;
	m_hLastDoor[1] = LTNULL;

	m_bUseDefaultAttachments = LTTRUE;

	m_eHumanType = eHT_Invalid;

	m_pMovementModifier = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::~CAIHuman()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAIHuman::~CAIHuman()
{
	if ( m_pHumanState )
	{
		AI_FACTORY_DELETE(m_pHumanState);
		m_pHumanState = LTNULL;
		m_pState = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //
LTBOOL CAIHuman::ReadProp(ObjectCreateStruct *pData)
{
	AIASSERT( GetAITemplate(), m_hObject, "CAIHuman::ReadProp: Requires CAI::AITemplate to be set." );
	AIASSERT( g_pLTServer, m_hObject, "CAIHuman::ReadProp: No g_pLTServer." );
	AIASSERT( g_pLTServer, m_hObject, "CAIHuman::ReadProp: No pData." );

    if (!g_pLTServer || !pData)
	{
		return LTFALSE;
	}

	// Set the default values:
	m_flSentryChallengeScanDistMax = GetAITemplate()->flSentryChallengeScanDistMax;
	m_flSentryMarkDistMax = GetAITemplate()->flSentryMarkDistMax;
	m_flSentryChallengeDistMax = GetAITemplate()->flSentryChallengeDistMax;

	GenericProp genProp;
	const char* pszValue = NULL;

    if ( g_pLTServer->GetPropGeneric("JumpSpeed", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fJumpVel = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("JumpOverSpeed", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fJumpOverVel = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("FallSpeed", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fFallVel = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("WalkSpeed", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fWalkVel = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("SwimSpeed", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fSwimVel = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("RunSpeed", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fRunVel = genProp.m_Float;

	pszValue = pData->m_cProperties.GetPropString( "SentryChallengeScanDistMax", NULL );
	if ( pszValue && pszValue[0] )
	{
		m_flSentryChallengeScanDistMax = static_cast<float>(atof( pszValue ));
	}

	pszValue = pData->m_cProperties.GetPropString( "SentryChallengeDistMax", NULL );
	if ( pszValue && pszValue[0] )
	{
		m_flSentryChallengeDistMax = static_cast<float>(atof( pszValue ));
	}

	pszValue = pData->m_cProperties.GetPropString( "SentryMarkDistMax", NULL );
	if ( pszValue && pszValue[0] )
	{
		m_flSentryMarkDistMax = static_cast<float>(atof( pszValue ));
	}

    if ( g_pLTServer->GetPropGeneric( "Brain", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
            m_sBrain = g_gp.m_String;
			if ( !m_pBrain )
			{
				SetBrain( m_sBrain );
			}
		}
	}

    if ( g_pLTServer->GetPropGeneric( "GoalSet", &genProp ) == LT_OK )
	{
		if ( genProp.m_String[0] )
		{
			// Initialize the GoalMgr.
			m_pGoalMgr->Init(this);		
			m_pGoalMgr->SetGoalSet( genProp.m_String, LTFALSE );
		}
	}

	if (g_pLTServer->GetPropGeneric("ModelTemplate", &genProp) == LT_OK)
	{
		if ( genProp.m_String[0] )
		{
			m_eModelId = g_pModelButeMgr->GetModelId(genProp.m_String);
		}
	}
	else {
		AIASSERT( 0, m_hObject, "CAIHuman::ReadProp: No ModelTemplate was set!" );
	}

	// Default attachments.

    if ( g_pLTServer->GetPropGeneric( "UseDefaultAttachments", &genProp ) == LT_OK )
	{
		m_bUseDefaultAttachments = genProp.m_Bool;
	}

	// Create holster string.

	char szHolster[128];
	szHolster[0] = LTNULL;
	if( g_pLTServer->GetPropGeneric( "RightHand", &genProp ) == LT_OK )
	{
		if( genProp.m_String[0] && stricmp(genProp.m_String, NO_ATTACHMENT) )
		{
			AIASSERT( 1 + strlen( genProp.m_String ) < 128, m_hObject, "CAIHuman::ReadProp: Holster string exceeds 128 chars." );
			strcat( szHolster, genProp.m_String );
		}

		// Left handed-weapon is secondary to right-handed.
		// Must have a right-handed weapon first.

		if( g_pLTServer->GetPropGeneric( "LeftHand", &genProp ) == LT_OK )
		{
			if( genProp.m_String[0] && stricmp(genProp.m_String, NO_ATTACHMENT) )
			{
				AIASSERT( 1 + strlen( szHolster ) + strlen( genProp.m_String ) < 128, m_hObject, "CAIHuman::ReadProp: Holster string exceeds 128 chars." );
				strcat( szHolster, ";" );
				strcat( szHolster, genProp.m_String );
			}
		}

		// Create a backup copy of the holster string, for use if 
		// the AIs weapon is stolen (while he is knocked out).

		if( szHolster[0] )
		{
			m_hstrHolsterBackup = g_pLTServer->CreateString( szHolster );
		}
	}

	// Holster weapons.

	if ( g_pLTServer->GetPropGeneric( "HolsterWeapons", &genProp ) == LT_OK )
	{
		if( genProp.m_Bool && szHolster[0] )
		{
			char szHolsterCopy[129];
			szHolsterCopy[0] = '-';
			szHolsterCopy[1] = '\0';
			strcat( szHolsterCopy, szHolster );

			if( szHolsterCopy[0] )
			{
				m_hstrHolster = g_pLTServer->CreateString( szHolsterCopy );
			}
		}
	}

	// Setup model and AI based on model template.

	m_eModelSkeleton = g_pModelButeMgr->GetModelSkeleton(m_eModelId);

	m_pAnimationMgr = g_pAnimationMgrList->GetAnimationMgr( g_pModelButeMgr->GetModelAnimationMgr(m_eModelId) );

	const AIBM_Template* const pTemplate = GetAITemplate();
	m_fJumpVel		 = pTemplate->fJumpSpeed;
	m_fJumpOverVel	 = pTemplate->fJumpOverSpeed;
	m_fFallVel		 = pTemplate->fFallSpeed;
	m_fWalkVel		 = pTemplate->fWalkSpeed;
	m_fSwimVel		 = pTemplate->fSwimSpeed;
	m_fRunVel		 = pTemplate->fRunSpeed;

	// Set the Update Rate in the Attributes now, as this information
	// is not related to the characters alignment in any way.  Default
	// value should
	m_fSenseUpdateRate = pTemplate->fUpdateRate;

	// Set next sense update.
	m_fNextSenseUpdate = g_pLTServer->GetTime() + s_fSenseUpdateBasis;
	s_fSenseUpdateBasis += .02f;
	if ( s_fSenseUpdateBasis > 0.5f )
	{
		s_fSenseUpdateBasis = 0.0f;
	}

	// Set up the NOLF simulating AI Type for distinctly different 
	// character types such as swimming, hovering or paratrooping
	SetAIType( pTemplate->szHumanType );

	if (!m_pSearch->GetRandomItemSet())
	{
		m_pSearch->SetRandomItemSet("Bodies General");
	}

	return CAI::ReadProp(pData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::PreCreateSpecialFX()
//
//	PURPOSE:	Last chance to change our characterfx struct
//
// ----------------------------------------------------------------------- //

void CAIHuman::PreCreateSpecialFX(CHARCREATESTRUCT& cs)
{
	CAI::PreCreateSpecialFX(cs);

	cs.nTrackers = 2;
	cs.nDimsTracker = MAIN_TRACKER;
//	cs.SetSleeping(GetAnimationContext() ? GetAnimationContext()->IsPropSet(kAPG_Action, kAP_Asleep) : LTFALSE);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHuman::CreateArmor()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHuman::CreateArmor()
{
	CAI::CreateArmor();

	// Get a list of all of the prop-objects attatched as the m_ap* versions
	// are unreliable
	BaseClass* apObjects[AI_MAX_OBJECTS];
	CAttachmentPosition* apObjectPositions[AI_MAX_OBJECTS];
	int cObjects = m_pAttachments->EnumerateObjects(apObjects, apObjectPositions, AI_MAX_OBJECTS);

	// Send each peice of the armor a 'on' command
	for(int i = 0; i < cObjects; i++ )
	{
		AIASSERT( apObjects[i], m_hObject, "CAIHuman::CreateArmor: Invalid object referenced" );
		SendTriggerMsgToObject( LTNULL, apObjects[i]->m_hObject, LTFALSE, "ARMOR_ON" );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHuman::DestroyArmor()
//              
//	PURPOSE:	Turns off all of the armor
//              
//----------------------------------------------------------------------------
void CAIHuman::DestroyArmor()
{
	CAI::DestroyArmor();

	// Get a list of all of the prop-objects attatched as the m_ap* versions
	// are unreliable
	BaseClass* apObjects[AI_MAX_OBJECTS];
	CAttachmentPosition* apObjectPositions[AI_MAX_OBJECTS];
	int cObjects = m_pAttachments->EnumerateObjects(apObjects, apObjectPositions, AI_MAX_OBJECTS);

	// Send each peice of the armor a 'on' command
	for(int i = 0; i < cObjects; i++ )
	{
		AIASSERT( apObjects[i], m_hObject, "CAIHuman::DestroyArmor: Invalid object referenced" );
		SendTriggerMsgToObject( LTNULL, apObjects[i]->m_hObject, LTFALSE, "ARMOR_OFF" );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::GetDeathAni()
//
//	PURPOSE:	Get the death animation
//
// ----------------------------------------------------------------------- //

HMODELANIM CAIHuman::GetDeathAni(LTBOOL bFront)
{
	HMODELANIM hAni = INVALID_MODEL_ANIM;

	if ( m_pState )
	{
		if ( INVALID_MODEL_ANIM != (hAni = m_pState->GetDeathAni(bFront)) )
		{
			return hAni;
		}
	}

	if ( GetAnimationContext()->IsPropSet(kAPG_Movement, kAP_Run) )
	{
		char* aszDeathRunsFront[] = { "DRun", "DRun2" };
		char* aszDeathRunsBack[]  = { "DRunBack", "DRunBack2" };

		int cDeathRuns = sizeof(aszDeathRunsBack)/sizeof(const char*);
		char** pDeathRuns = (bFront ? aszDeathRunsFront : aszDeathRunsBack);

        if ( INVALID_MODEL_ANIM != (hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)pDeathRuns[GetRandom(0, cDeathRuns-1)])) )
		{
			return hAni;
		}
	}

	if ( GetAnimationContext()->IsPropSet(kAPG_Posture, kAP_Crouch) )
	{
        if ( INVALID_MODEL_ANIM != (hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)GetCrouchDeathAni())) )
		{
			return hAni;
		}
	}

	if ( GetAnimationContext()->IsPropSet(kAPG_Posture, kAP_Prone) )
	{
        if ( INVALID_MODEL_ANIM != (hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)GetProneDeathAni())) )
		{
			return hAni;
		}
	}

	return CAI::GetDeathAni(bFront);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::GetCrouchDeathAni()
//
//	PURPOSE:	Get a crouching death animation
//
// ----------------------------------------------------------------------- //

const char* CAIHuman::GetCrouchDeathAni()
{
	static const char* aszDeathCrouches[] = { "DCrouch2", "DCrouch3", "DCrouch4", "DCrouch5" };
    static const int cDeathCrouches = sizeof(aszDeathCrouches)/sizeof(const char*);
	return aszDeathCrouches[GetRandom(0, cDeathCrouches-1)];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::GetProneDeathAni()
//
//	PURPOSE:	Get a Proneing death animation
//
// ----------------------------------------------------------------------- //

const char* CAIHuman::GetProneDeathAni()
{
	static const char* aszDeathPronees[] = { "DProne", };
    static const int cDeathPronees = sizeof(aszDeathPronees)/sizeof(const char*);
	return aszDeathPronees[GetRandom(0, cDeathPronees-1)];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::GetBodyState()
//
//	PURPOSE:	Gets the state of our body
//
// ----------------------------------------------------------------------- //

EnumAIStateType CAIHuman::GetBodyState()
{
	EnumAIStateType eBodyState = CAI::GetBodyState();

	if ( GetAnimationContext()->IsPropSet(kAPG_Action, kAP_Unconscious) )
	{
		return kState_BodyNormal;
	}
	else if ( GetAnimationContext()->IsPropSet(kAPG_Posture, kAP_Sit) )
	{
		return GetPriorityBodyState(eBodyState, kState_BodyChair);
	}
	else if ( m_pState )
	{
		return GetPriorityBodyState(eBodyState, m_pState->GetBodyState());
	}
	else
	{
		return eBodyState;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::PlayDeathSound()
//
//	PURPOSE:	Play the death sound
//
// ----------------------------------------------------------------------- //

void CAIHuman::PlayDeathSound()
{
	char* szDeathSound = LTNULL;

	// If we're unconscious make no noise

	if ( GetAnimationContext()->IsPropSet(kAPG_Action, kAP_Unconscious) || m_bUnconscious )
	{
		return;
	}

	// Check for "silent kill" -- hit in head and not alert

	if ( WasSilentKill() ||
		 GetAnimationContext()->IsPropSet(kAPG_Action, kAP_Asleep) )
	{
		szDeathSound = GetDeathSilentSound();
	}
	else
	{
		szDeathSound = GetDeathSound();
	}

	CCharacter::PlayDialogSound(szDeathSound, CST_DEATH);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CanSearch()
//
//	PURPOSE:	Returns TRUE if AI can search from where it is.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHuman::CanSearch()
{
	if( ( GetPrimaryWeapon() || HasHolsterString() ) &&
		super::CanSearch() )
	{
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::WasSilentKill()
//
//	PURPOSE:	Determines if we have been killed silently or not
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHuman::WasSilentKill()
{
	if ( IsAlert() )
	{
		return LTFALSE;
	}
	else if ( eModelNodeInvalid == m_eModelNodeLastHit )
	{
		return LTFALSE;
	}
	else
	{
		return NODEFLAG_CRITICALHIT & g_pModelButeMgr->GetSkeletonNodeFlags(m_eModelSkeleton, m_eModelNodeLastHit) ? LTTRUE : LTFALSE;
	}
}

// ----------------------------------------------------------------------- //

void CAIHuman::HandleTeleport(const LTVector& vPos)
{
	CAI::HandleTeleport(vPos);
	m_bPosDirty = LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHuman::HandleTeleport(TeleportPoint* pTeleportPoint)
{
	CAI::HandleTeleport(pTeleportPoint);

	if ( pTeleportPoint->IsMoveToFloor() )
	{
		// [KLS 8/8/02] Make sure we get moved to the floor right now...
		m_bPosDirty = LTTRUE;
		m_bForceGround = LTTRUE;
		UpdateMovement();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::TransitioningInMultiplayerWhileCarryingCateTeleportHack
//
//	PURPOSE:	HACK to handle teleporting Cate through TransAMs in Multiplayer
//	
//	NOTE:		This should get removed and handled much smoother in the patch ;)
//
// ----------------------------------------------------------------------- //

void CAIHuman::TransitioningInMultiplayerWhileCarryingCateTeleportHack( const LTVector &vPos )
{
	// Make sure the AI is actually Cate...

	if( _stricmp( GetName(), "Cate" ) != 0 )
		return;

	// Just teleport immediately without saving any teleport state or position so on the
	// next update, after the load,  it won't try to teleport Cate to a position from the
	// previous level....

	HandleTeleport( vPos );

	m_bMove = LTFALSE;
	m_bPosDirty = LTFALSE;
	m_bMoveToFloor = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::HandleGadget()
//
//	PURPOSE:	Handle having a gadget used on us
//
// ----------------------------------------------------------------------- //

void CAIHuman::HandleGadget(uint8 nAmmoID)
{
	CCharacter::HandleGadget(nAmmoID);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::HandleCommand()
//
//	PURPOSE:	Handle a command
//
// --------------------------------------------------------------------------- //

bool CAIHuman::HandleCommand(const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Pose("POSE");
	static CParsedMsg::CToken s_cTok_Sit("SIT");
	static CParsedMsg::CToken s_cTok_TrackPlayer("TRACKPLAYER");
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");
	static CParsedMsg::CToken s_cTok_ForceGround("FORCEGROUND");
	static CParsedMsg::CToken s_cTok_Cigarette("CIGARETTE");
	static CParsedMsg::CToken s_cTok_Smoke("SMOKE");
	static CParsedMsg::CToken s_cTok_CineFire("CINEFIRE");
	static CParsedMsg::CToken s_cTok_CopySense("COPYSENSE");

	// Let base class have a whack at it...

	if (CAI::HandleCommand(cMsg)) return true;

	// State-changing message

	for ( int iState = 0 ; iState < kState_Count ; ++iState )
	{
		// In case this was an optional command, skip past the ?
		uint32 bOptional = LTFALSE;
		if ( '?' == *cMsg.GetArg(0) )
		{
			bOptional = LTTRUE;
		}

		if ( !_stricmp(&cMsg.GetArg(0)[bOptional], s_aszStateTypes[iState]) )
		{
			if ( bOptional )
			{
				return true;
			}

			// Found a matching state, let's see if we can change.

			if ( m_pState && m_pState->DelayChangeState() )
			{
				// We can't change states right now...

				char szBuffer[4096];
				szBuffer[0] = 0;

				// Re-concatenate the tokens into a single message...

				for ( uint iArg = 0 ; iArg < cMsg.GetArgCount(); iArg++ )
				{
					strcat(szBuffer, cMsg.GetArg(iArg));
					strcat(szBuffer, " ");
				}

				// And stuff it into the NEXT= parameter for the current state

				m_pState->HandleNameValuePair("NEXT", szBuffer);
			}
			else if ( m_pState && m_pState->RejectChangeState() )
			{
			}
			else
			{
				// We can change states, go ahead and do so...

				SetState( (EnumAIStateType)iState );
				HandleCommandParameters(cMsg);
			}

			return true;
		}
	}

	// Non-state-changing messages

	if ( cMsg.GetArg(0) == s_cTok_Pose )
	{
		if((cMsg.GetArgCount() > 1) && (m_pHumanState != LTNULL))
		{
			if ( cMsg.GetArg(1) == s_cTok_Sit )
			{
				m_pHumanState->SetPose(kAP_Sit);
			}
			else {
				m_pHumanState->SetPose(kAP_Stand);
			}
		}
		return LTTRUE;
	}
	else if ( cMsg.GetArg(0) == s_cTok_TrackPlayer )
	{
		LTBOOL bOn = LTTRUE;
		LTFLOAT fTime = 0.0f;
        HSTRING hstr = g_pLTServer->CreateString("Head");

		if(cMsg.GetArgCount() > 1)
		{
			if ( cMsg.GetArg(1) == s_cTok_On )
				bOn = LTTRUE;
			else if ( cMsg.GetArg(1) == s_cTok_Off )
				bOn = LTFALSE;
			else
				fTime = (LTFLOAT)atof(cMsg.GetArg(1));
		}

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(CFX_NODECONTROL_HEAD_FOLLOW_OBJ);
		cMsg.WriteObject(g_pCharacterMgr->FindPlayer()->m_hObject);
		cMsg.WriteHString(hstr);
		cMsg.Writefloat(130.0f);
		cMsg.Writefloat(fTime);
		cMsg.Writeuint8(bOn);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

        g_pLTServer->FreeString(hstr);

		return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_ForceGround )
	{
		m_bForceGround = IsTrueChar(*cMsg.GetArg(1));
		return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_Cigarette )
	{
		if ( IsTrueChar(*cMsg.GetArg(1)) )
		{
			DestroyCigarette();
			CreateCigarette(LTFALSE);
		}
		else
		{
			DestroyCigarette();
		}
		return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_CopySense )
	{
		if( cMsg.GetArgCount() > 1 )
		{
			HOBJECT hObject;
			if( LT_OK == FindNamedObject( cMsg.GetArg(1), hObject) )
			{
				AINodeSensing* pNodeSensing = (AINodeSensing*)g_pLTServer->HandleToObject( hObject );
				if( pNodeSensing )
				{
					pNodeSensing->CopySense( this );
				}
			}
		}
		return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_Smoke )
	{
		if ( IsTrueChar(*cMsg.GetArg(1)) )
		{
			DestroyCigarette();
			CreateCigarette(LTTRUE);
		}
		else
		{
			DestroyCigarette();
			CreateCigarette(LTFALSE);
		}
		return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_CineFire )
	{
		CWeapon* pWeapon = GetWeapon(0);
		if ( pWeapon )
		{
			HMODELSOCKET hRightHand;
			if ( LT_OK == g_pModelLT->GetSocket(m_hObject, "RightHand", hRightHand) )
			{
				LTransform tf;

				if ( LT_OK == g_pModelLT->GetSocketTransform(m_hObject, hRightHand, tf, LTTRUE) )
				{
					// Now fire the weapon

					WeaponFireInfo weaponFireInfo;
					weaponFireInfo.hFiredFrom = m_hObject;
					LTRotation rRot = tf.m_Rot;
					weaponFireInfo.vPath		= rRot.Forward();
					weaponFireInfo.vFirePos	= m_vPos;
					weaponFireInfo.vFlashPos	= m_vPos;
					weaponFireInfo.fPerturbR	= 0.0f;
					weaponFireInfo.fPerturbU	= 0.0f;

					pWeapon->ReloadClip(LTFALSE);
					pWeapon->GetParent()->AddAmmo(pWeapon->GetAmmoId(), 999999);
					pWeapon->UpdateWeapon(weaponFireInfo, LTTRUE);
				}
			}
		}

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::GetKneePosition()
//
//	PURPOSE:	Compute our 'knee level'
//
// ----------------------------------------------------------------------- //

LTVector CAIHuman::GetKneePosition() const
{
    LTVector vPos, vDims;
	vPos = m_vPos;

    if (!g_pLTServer || !m_hObject) return vPos;

	g_pPhysicsLT->GetObjectDims(m_hObject, &vDims);

	vPos.y -= vDims.y / 2.0f;
	vPos.y += 2.0f;

	return vPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::IsObjectVisible*()
//
//	PURPOSE:	Is the test object visible
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHuman::IsObjectVisibleFromKnee(ObjectFilterFn ofn, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV /* = LTTRUE */)
{
	return IsObjectVisible(ofn, NULL, GetKneePosition(), hObj, fVisibleDistanceSqr, bFOV, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::IsPositionVisible*()
//
//	PURPOSE:	Is the test position visible to us (regardless of direction)
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHuman::IsPositionVisibleFromKnee(ObjectFilterFn ofn, const LTVector& vSourcePosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV /* = LTTRUE */)
{
	return IsPositionVisible(ofn, NULL, GetKneePosition(), vSourcePosition, fVisibleDistanceSqr, bFOV, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::OpenDoor()
//
//	PURPOSE:	Attempts to open a door
//
// ----------------------------------------------------------------------- //

void CAIHuman::OpenDoors(HOBJECT hDoor1, HOBJECT hDoor2)
{
	MarkDoors( hDoor1, hDoor2 );

	for( uint8 iDoor=0; iDoor < 2; ++iDoor )
	{
		if( m_hLastDoor[iDoor] )
		{
			// Do not activate a door that is already open.

			if( IsKindOf( m_hLastDoor[iDoor], "RotatingDoor") )
			{
				RotatingDoor* pDoor = (RotatingDoor*)g_pLTServer->HandleToObject( m_hLastDoor[iDoor] );
				if( pDoor && ( ( pDoor->GetState() == DOORSTATE_OPEN ) || ( pDoor->GetState() == DOORSTATE_OPENING ) ) )
				{
					continue;
				}
			}

			SendTriggerMsgToObject(this, m_hLastDoor[iDoor], LTFALSE, "ACTIVATE");

			// Record which door was opened, and the original position
			// because the doors position changes as it swings open.

			g_pLTServer->GetObjectPos(m_hLastDoor[iDoor], &m_vLastDoorPos[iDoor]);		
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::KickDoor()
//
//	PURPOSE:	Attempts to kick open a door
//
// ----------------------------------------------------------------------- //

void CAIHuman::KickDoors(HOBJECT hDoor1, HOBJECT hDoor2)
{
	MarkDoors( hDoor1, hDoor2 );

	for( uint8 iDoor=0; iDoor < 2; ++iDoor )
	{
		if( m_hLastDoor[iDoor] )
		{
			// Do not activate a door that is already open.

			if( IsKindOf( m_hLastDoor[iDoor], "RotatingDoor") )
			{
				RotatingDoor* pDoor = (RotatingDoor*)g_pLTServer->HandleToObject( m_hLastDoor[iDoor] );
				if( pDoor && ( ( pDoor->GetState() == DOORSTATE_OPEN ) || ( pDoor->GetState() == DOORSTATE_OPENING ) ) )
				{
					continue;
				}
			}

			SendTriggerMsgToObject(this, m_hLastDoor[iDoor], LTFALSE, "SETPOWERONMULTIPLIER 0.1");
			SendTriggerMsgToObject(this, m_hLastDoor[iDoor], LTFALSE, "ACTIVATE");
		
			// Add a pusher.

			if( HasTarget() && IsPlayer( GetTarget()->GetObject() ) )
			{
				LTVector vPos;
				g_pLTServer->GetObjectPos(m_hLastDoor[iDoor], &vPos);		

				LTVector vDims;
				g_pPhysicsLT->GetObjectDims(m_hLastDoor[iDoor], &vDims);
	
				LTFLOAT fWidth = (vDims.x > vDims.z) ? vDims.x : vDims.z;

				CPlayerObj* pPlayer = (CPlayerObj*)GetTarget()->GetCharacter();
				pPlayer->PushCharacter( vPos, 96.f, 0.f, 1.f, 1000.f );	
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::MarkDoors()
//
//	PURPOSE:	Mark doors AI is opening
//
// ----------------------------------------------------------------------- //

void CAIHuman::MarkDoors(HOBJECT hDoor1, HOBJECT hDoor2)
{
	// Doors have already been marked.

	if( ( hDoor1 && ( m_hLastDoor[0] == hDoor1 ) ) || 
		( hDoor2 && ( m_hLastDoor[1] == hDoor2 ) ) )
	{
		return;
	}

	// Record and mark doors.

	m_hLastDoor[0] = hDoor1;
	m_hLastDoor[1] = hDoor2;

	Door* pDoor;
	for( uint32 iDoor=0; iDoor < 2; ++iDoor )
	{
		pDoor = (Door*)g_pLTServer->HandleToObject( m_hLastDoor[iDoor] );
		if( pDoor )
		{
			pDoor->SetAIUser( m_hObject );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::UnmarkDoors()
//
//	PURPOSE:	Unmark doors AI is opening
//
// ----------------------------------------------------------------------- //

void CAIHuman::UnmarkDoors()
{
	if( m_hLastDoor[0] || m_hLastDoor[1] )
	{
		Door* pDoor;
		for( uint32 iDoor=0; iDoor < 2; ++iDoor )
		{
			pDoor = (Door*)g_pLTServer->HandleToObject( m_hLastDoor[iDoor] );
			if( pDoor && ( pDoor->GetAIUser() == m_hObject ) )
			{
				pDoor->SetAIUser( LTNULL );
			}

			m_hLastDoor[iDoor] = LTNULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::CloseDoors()
//
//	PURPOSE:	Attempts to close doors
//
// ----------------------------------------------------------------------- //

void CAIHuman::CloseDoors()
{
	Door* pDoor;
	for( uint8 iDoor=0; iDoor < 2; ++iDoor )
	{
		if( m_hLastDoor[iDoor] )
		{
			pDoor = (Door*)g_pLTServer->HandleToObject( m_hLastDoor[iDoor] );
			if( pDoor && ( pDoor->GetAIUser() == m_hObject ) )
			{
				SendTriggerMsgToObject(this, m_hLastDoor[iDoor], LTFALSE, "ACTIVATE");
			}
		}
	}

	UnmarkDoors();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::CanCloseDoors()
//
//	PURPOSE:	Returns true of the AI is not blocking the last doors opened.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHuman::CanCloseDoors()
{
	if( m_pBrain->GetAIDataExist( kAIData_DoNotCloseDoors ) &&
		( m_pBrain->GetAIDataExist( kAIData_DoNotCloseDoors ) != 0.f ) )
	{
		return LTFALSE;
	}

	LTBOOL bCanCloseDoors = LTFALSE;

	// Did we open any doors?

	for( uint8 iDoor=0; iDoor < 2; ++iDoor )
	{
		if( m_hLastDoor[iDoor] )
		{
			// Is the door still open?

			Door* pDoor = (Door*)g_pLTServer->HandleToObject(m_hLastDoor[iDoor]);
			if ( pDoor && ( pDoor->GetState() == DOORSTATE_OPEN ) )
			{
				// Has a last door that is open.

				bCanCloseDoors = LTTRUE;

				// Are we out of the way of its swing?

				LTVector vDims;
				g_pPhysicsLT->GetObjectDims(m_hLastDoor[iDoor], &vDims);

				LTFLOAT fWidth = (vDims.x > vDims.z) ? vDims.x : vDims.z;
				fWidth *= 2.0f;
				fWidth += m_vDims.x;

				m_vLastDoorPos[iDoor].y = 0.f;
				LTVector vPos = m_vPos;
				vPos.y = 0.f;

				if( m_vLastDoorPos[iDoor].DistSqr( vPos ) <= ( fWidth * fWidth ) )
				{
					return LTFALSE;
				}
			}
		}
	}

	return bCanCloseDoors;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::HandleDamage()
//
//	PURPOSE:	Notification that we are hit by something
//
// ----------------------------------------------------------------------- //

void CAIHuman::HandleDamage(const DamageStruct& damage)
{
	if ( (DT_MELEE == damage.eType || DT_SWORD == damage.eType) && m_damage.IsCantDamageType(damage.eType) )
	{
		if ( !IsAlert() && (damage.fDamage > 100.0f) )
		{
			m_fLastPainVolume = 0.1f;
			m_damage.SetHitPoints(0.0f);
			m_damage.HandleDestruction(damage.hDamager);
			return;
		}
		else
		{
			return;
		}
	}
	if ( DT_WORLDONLY == damage.eType || m_damage.IsCantDamageType(damage.eType) || !m_damage.GetCanDamage() )
	{
		m_fLastPainVolume = 0.0f;
		return;
	}

	if ( DT_SLEEPING == damage.eType )
	{
		m_fLastPainVolume = 0.1f;
	}

	// If hit by a tracking device just be completely oblivious to the actual damage and begin tracking... 

	if( damage.eType == DT_GADGET_TRACKER )
	{
		// Send message to begin tracking...

		m_bStuckWithTrackDart = true;
		SetTracking( true );
		m_fLastPainVolume = 0.0f;
		return;
	}

	// Ignore shots from camera disabler.

	if( damage.eType == DT_CAMERA_DISABLER )
	{
		m_fLastPainVolume = 0.0f;
		return;
	}

	if ( HandleResurrectingGoalConditions(damage) )
	{
		// See if this goal meets the criteria for adding the Resurrecting Goal
		// Note: This call needs to be before the DamageType checks, as we
		// return from them.  This has priority over them.
		return;
	}
	
	// Give goals an opportunity to handle damage.  GoalMgr returns TRUE
	// if a goal does not want processing of damage to continue.

	AIASSERT( m_pGoalMgr, m_hObject, "CAIHuman::HandleDamage: GoalMgr is NULL." );
	if( m_pGoalMgr->HandleDamage( damage ) )
	{
		return;
	}

	CAI::HandleDamage(damage);

	if ( !m_damage.IsDead() )
	{
		// Do not recoil if affected by special damage.

		if( ( m_bCanShortRecoil ) &&
			( !GetDamageFlags() ) )
		{
			HandleShortRecoil();
		}
	}
	else
	{
        if ( WasSilentKill() )
		{
			m_fLastPainVolume = 0.1f;
		}

		// Move to wherever our torso is

		SyncPosition();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHuman::HandleResurrectingGoalConditions()
//              
//	PURPOSE:	If this AI supports resurrection, see if they ought to start
//				the process at this point.
//
//	NOTE:		If the AI is currently 
//
//----------------------------------------------------------------------------
bool CAIHuman::HandleResurrectingGoalConditions(const DamageStruct& damage)
{
//	g_pLTServer->CPrint( "HP: %f\tMAX_HP: %f", m_damage.GetHitPoints(), m_damage.GetMaxHitPoints() );

	// If this is a resurrecting AI, then check to see if we ought to start
	// the resurrecting Goal.  Check the hitpoints against the 
	if ( !GetBrain()->GetAIDataExist( kAIData_Resurrecting ) )
		return false;

	if ( LTTRUE != m_damage.GetNeverDestroy() )
		return false;

	// Add the resurrecting goal if the health is now at a certain point.
	float flPercent = GetBrain()->GetAIData(kAIData_Resurrecting);
	AIASSERT( flPercent < 1.0f && flPercent > 0.0f, m_hObject, "" );
	if ( m_damage.GetMaxHitPoints()*flPercent < m_damage.GetHitPoints() )
		return false;

	CAIGoalAbstract* pGoal = GetGoalMgr()->FindGoalByType( kGoal_Resurrecting );
	if ( pGoal != NULL )
	{
		// Issue the ResetGoal so that the goal will potentially reActivate if
		// the AI is currently in a search or aware state.
		pGoal->HandleNameValuePair( "RESETGOAL", NULL );
	}
	else
	{
		AIGBM_GoalTemplate* pGoalTemplate = g_pAIGoalButeMgr->GetTemplate(kGoal_Resurrecting);
		GetGoalMgr()->AddGoal( kGoal_Resurrecting, pGoalTemplate->fImportance, g_pLTServer->GetTime(), LTFALSE ); 
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::PreUpdate
//
//	PURPOSE:	Does our Preupdate
//
// ----------------------------------------------------------------------- //

void CAIHuman::PreUpdate()
{
	CAI::PreUpdate();

	if ( m_bFirstUpdate )
	{
		// If this character resurrects, then validate the required data 
		// and configure them for this added ability.

		if ( GetBrain()->GetAIDataExist(kAIData_Resurrecting) )
		{
			// Make sure that the AIData is within the ranges of use now,
			// instead of waiting for use to validate it.
			float flPercent = GetBrain()->GetAIData(kAIData_Resurrecting);
			if ( flPercent > 1.0f || flPercent < 0.0f )
			{
				AIASSERT2( 0, m_hObject, "kAIData_Resurrecting (%f) out of range (0,1) for brain %s", flPercent, GetBrain()->GetName() );
			}

			// Resurrecting characters are not normally destroyable
			m_damage.SetNeverDestroy(LTTRUE);
		}

		if ( !m_bMoveToFloor )
		{
			m_bPosDirty = LTFALSE;
		}

		// Add default attachments.

		if( m_bUseDefaultAttachments )
		{
			const char* pszAttachmentPos;
			const char* pszAttachment;
			char szTrigger[128];

			uint32 cAttachments = g_pModelButeMgr->GetNumDefaultAttachments( m_eModelId );
			for( uint32 iAttachment = 0; iAttachment < cAttachments; ++iAttachment )
		{
				g_pModelButeMgr->GetDefaultAttachment( m_eModelId, iAttachment, pszAttachmentPos, pszAttachment );

				sprintf( szTrigger, "%s %s %s", KEY_ATTACH, pszAttachmentPos, pszAttachment );
				SendTriggerMsgToObject( this, m_hObject, LTFALSE, szTrigger );
			}
		}

		// If AI has a holster string that starts with a minus,
		// then flag was set in dedit to holster weapons.
		// So remove hand attachments, and recreate holster string without a minus.

		if( m_hstrHolster )
		{
			const char* szHolster = g_pLTServer->GetStringData(m_hstrHolster);
			if( szHolster[0] == '-' )
			{
				char szTrigger[128];

				// Attach holstered models.

				CAttachment* pAttachment = GetAttachments()->GetAttachment("RightHand");
				if( pAttachment && ( pAttachment->GetType() == ATTACHMENT_TYPE_WEAPON ) )
				{
					CAttachmentWeapon* pWeaponAttachment = (CAttachmentWeapon*)pAttachment;
					const WEAPON* pWeapon = g_pWeaponMgr->GetWeapon( pWeaponAttachment->GetWeapons()->GetCurWeaponId() );
					if( pWeapon && pWeapon->szHolsterAttachment[0] )
					{
						sprintf( szTrigger, "%s %s", KEY_ATTACH, pWeapon->szHolsterAttachment );
						SendTriggerMsgToObject( this, m_hObject, LTFALSE, szTrigger );
					}
				}

				pAttachment = GetAttachments()->GetAttachment("LeftHand");
				if( pAttachment && ( pAttachment->GetType() == ATTACHMENT_TYPE_WEAPON ) )
				{
					CAttachmentWeapon* pWeaponAttachment = (CAttachmentWeapon*)pAttachment;
					const WEAPON* pWeapon = g_pWeaponMgr->GetWeapon( pWeaponAttachment->GetWeapons()->GetCurWeaponId() );
					if( pWeapon && pWeapon->szHolsterAttachment[0] )
					{
						sprintf( szTrigger, "%s %s", KEY_ATTACH, pWeapon->szHolsterAttachment );
						SendTriggerMsgToObject( this, m_hObject, LTFALSE, szTrigger );
					}
				}

				// Detach weapons from hands.

				char szDetachMsg[128];
				
				sprintf( szDetachMsg, "%s RIGHTHAND", KEY_DETACH );
				SendTriggerMsgToObject( this, m_hObject, LTFALSE, szDetachMsg );

				sprintf( szDetachMsg, "%s LEFTHAND", KEY_DETACH );
				SendTriggerMsgToObject( this, m_hObject, LTFALSE, szDetachMsg );
			
				// Create new holster string without minus.

				HSTRING hstrNew = g_pLTServer->CreateString( ((char*)szHolster) + 1 );
				FREE_HSTRING( m_hstrHolster );
				m_hstrHolster = hstrNew;
			}
		}
	}

	if ( m_bSyncPosition )
	{
		LTVector vNewPosition(m_vPos.x, m_vPos.y, m_vPos.z);
		g_pLTServer->TeleportObject(m_hObject, &vNewPosition);

		m_bPosDirty = LTTRUE;
		m_bSyncPosition = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::UpdateNodes
//
//  PURPOSE:	Update the position of important nodes (eye, torso, etc)
//
// ----------------------------------------------------------------------- //

void CAIHuman::UpdateNodes()
{
	CAI::UpdateNodes();

	if( m_bUpdateNodes )
	{
		if ( INVALID_MODEL_NODE == m_hHeadNode )
		{
			g_pModelLT->GetNode(m_hObject, "Head", m_hHeadNode);

			AIASSERT( INVALID_MODEL_NODE != m_hHeadNode, m_hObject,
				"Unable to find node named Head for this character.  Invalid Eye position" );
		}

		LTransform transform;
		g_pModelLT->GetNodeTransform(m_hObject, m_hHeadNode, transform, LTTRUE);

		// Use the actual height of the eyes, but center them in the model.
		// This keeps the eyes from entering volumes (e.g. junctions) before the body.

		m_vEyePos = m_vPos;
		m_vEyePos.y = transform.m_Pos.y;

		LTRotation rRot = transform.m_Rot;
		//m_vEyeForward = rRot.Forwad();  // Forward in LT2... but Up in LT3... ?
		m_vEyeForward = rRot.Up();

	//	g_pLTServer->CPrint("Eye forward = %f,%f,%f .... forward = %f,%f,%f", EXPANDVEC(m_vEyeForward), EXPANDVEC(m_vForward));

		// Get the current Torso node basis, which may be different from
		// the AI's if Torso tracking is enabled.

		if( ( m_pTrackedNodeContext->GetActiveTrackingGroup() == kTrack_AimAt ) &&
			m_pTrackedNodeContext->IsValidTrackingGroup( kTrack_AimAt ) )
		{
			m_vTorsoRight = -rRot.Forward();
			m_vTorsoRight.y = 0.f;
			m_vTorsoForward = m_vEyeForward;
		}
		else {
			m_vTorsoRight = m_vRight;
			m_vTorsoForward = m_vForward;
		}


		// Do not update nodes again until someone clears this flag.

		m_bUpdateNodes = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::UpdateTarget
//
//	PURPOSE:	Performs the movement of our object
//
// ----------------------------------------------------------------------- //

void CAIHuman::UpdateTarget()
{
	CAI::UpdateTarget();
}

void CAIHuman::UpdateMovement()
{
	LTBOOL bLastCheapMovement = m_bCheapMovement;
	m_pAIMovement->Update();

	// If last update did not use cheap movement,
	// but this update does, force the AI to the ground.

	if( ( !bLastCheapMovement ) && m_bCheapMovement )
	{
		m_bForceGround = LTTRUE;
	}

	// Force dead AI to the ground, if they were not sitting or crouching.

	if( m_damage.IsDead() && m_pAnimationContext->IsPropSet( kAPG_Posture, kAP_Stand ) )
	{
		m_bForceGround = LTTRUE;
	}

	if ( !m_bMove || m_bForceGround || m_bPosDirty)
	{
		if ( !m_bForceGround && !m_bPosDirty )
		{
			return;
		}
		else if ( !m_bMove )
		{
			m_vMovePos = m_vPos;
		}
	}

	// If we have a movement modifier, then apply it.
	// Otherwise do the standard adjustments.
	if ( m_pMovementModifier )
	{
		m_vMovePos = m_pMovementModifier->Update( m_hObject, GetDims(), GetPosition(), m_vMovePos, GetLastVolume() ); 
	}
	else if ( m_bCheapMovement || m_bForceGround )
	{
		AIASSERT( !IsSwimming() && !IsHovering(), m_hObject, "AI should be using a movement modifier!" );
		if ( !GetAnimationContext()->IsPropSet(kAPG_Movement, kAP_ClimbUp) && !GetAnimationContext()->IsPropSet(kAPG_Movement, kAP_ClimbDown) &&
			 !GetAnimationContext()->IsPropSet(kAPG_Movement, kAP_ClimbUpFast) && !GetAnimationContext()->IsPropSet(kAPG_Movement, kAP_ClimbDownFast) )
		{
			// Only call FindFloorHeight() (which calls $$ IntersectSegment() $$ ) 
			// if the AI has moved past a threshold distance.

			LTFLOAT fDiffSqr = m_vLastFindFloorPos.DistSqr( m_vMovePos );
			if( m_bForceGround || ( fDiffSqr >= FIND_FLOOR_THRESH_SQR ) )
			{
				// Find the floor underneath us

				LTFLOAT fFloorHeight;
				if( FindFloorHeight(m_vMovePos, &fFloorHeight) )
				{
					m_vMovePos.y = fFloorHeight;
				}

				m_vLastFindFloorPos = m_vMovePos;
			}
		}
	}

	g_pLTServer->MoveObject(m_hObject, &m_vMovePos );

	// Clear out movement info

	m_bForceGround = LTFALSE;
	m_bPosDirty = LTFALSE;
	m_bMove = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::UpdateCharacterFx
//
//	PURPOSE:	Handles any pending Animator changes
//
// ----------------------------------------------------------------------- //

void CAIHuman::UpdateCharacterFx()
{
//	if ( m_pcs->IsSleeping() && !GetAnimationContext()->IsPropSet(kAPG_Action, kAP_Asleep) )
//	{
//		DestroyZzz();
//		SetSleeping(LTFALSE);
//		CreateSpecialFX();
//	}
//	else if ( !m_pcs->IsSleeping() && GetAnimationContext()->IsPropSet(kAPG_Action, kAP_Asleep) )
//	{
//		CreateZzz();
//		SetSleeping(LTTRUE);
//		CreateSpecialFX();
//	}

	// Should we armor up?
	if ( !IsArmored() && IsAlert() && !IsDead())
	{
		CreateArmor();
	}
	if ( IsArmored() && !IsAlert() && !IsDead())
	{
		DestroyArmor();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::UpdateAnimator
//
//	PURPOSE:	Handles any pending Animator changes
//
// ----------------------------------------------------------------------- //

void CAIHuman::UpdateAnimation()
{
	CAI::UpdateAnimation();
	if(m_damage.IsDead())
	{
		return;
	}

	// If the Weapon is not set to None, the state code already set it.

	if( GetAnimationContext()->GetProp( kAPG_Weapon ) == kAP_None )
	{
		GetAnimationContext()->SetProp( kAPG_Weapon, GetCurrentWeaponProp() );
	}

	if ( IsSwimming() )
	{
		GetAnimationContext()->SetProp(kAPG_Movement, kAP_Swim);
	}
	else if ( IsHovering() )
	{
		GetAnimationContext()->SetProp(kAPG_Movement, kAP_Hover);
	}

	if ( 0 ) // !stricmp(GetName(), "a") )
	{
		char szBuffer[256];
		szBuffer[0] = 0;
		GetAnimationContext()->GetPropsString(szBuffer, 256);
		g_pLTServer->CPrint("%s props = %s", GetName(), szBuffer);
	}

	GetAnimationContext()->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::GetCurrentWeaponProp
//
//	PURPOSE:	Get animation prop for primary weapon.
//
// ----------------------------------------------------------------------- //

EnumAnimProp CAIHuman::GetCurrentWeaponProp()
{
	return m_ePrimaryWeaponProp;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CAIHuman::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
    if (!g_pLTServer || !pMsg) return;

	CAI::Save(pMsg, dwSaveFlags);

	// Save state...

	uint32 dwState = (uint32)-1;
	if (m_pHumanState)
	{
		dwState = (uint32) m_pHumanState->GetStateType();
	}

	SAVE_DWORD(dwState);

	if (m_pHumanState)
	{
		m_pHumanState->Save(pMsg);
	}

	// [JO] 8/16/02 
	// Intentionally save/load CAIs member m_pAIMovement in CAIHuman.
	// It needs to be saved/loaded AFTER the CAIState, because
	// Init on state calls Init on Strategy, which will overwrite
	// the current movement.

	m_pAIMovement->Save(pMsg);

	SAVE_BOOL(m_bSyncPosition);
	SAVE_BOOL(m_bPosDirty);
	SAVE_VECTOR(m_vLastFindFloorPos);
	SAVE_FLOAT(m_fSpeed);
	SAVE_BOOL(m_bCanShortRecoil);
	SAVE_BOOL(m_bForceGround);

	SAVE_HOBJECT(m_hLastDoor[0]);
	SAVE_HOBJECT(m_hLastDoor[1]);
	SAVE_VECTOR(m_vLastDoorPos[0]);
	SAVE_VECTOR(m_vLastDoorPos[1]);

	SAVE_BOOL(m_bUseDefaultAttachments);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CAIHuman::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
    if (!g_pLTServer || !pMsg) return;

	CAI::Load(pMsg, dwLoadFlags);

	// Load state...

	uint32 dwState;
	LOAD_DWORD(dwState);

	if (dwState != (DWORD)-1)
	{
		SetState((EnumAIStateType)dwState, LTFALSE);

		if (m_pHumanState)
		{
			m_pHumanState->Load(pMsg);
		}
	}

	// [JO] 8/16/02 
	// Intentionally save/load CAIs member m_pAIMovement in CAIHuman.
	// It needs to be saved/loaded AFTER the CAIState, because
	// Init on state calls Init on Strategy, which will overwrite
	// the current movement.

	m_pAIMovement->Load(pMsg);

	LOAD_BOOL(m_bSyncPosition);
	LOAD_BOOL(m_bPosDirty);
	LOAD_VECTOR(m_vLastFindFloorPos);

	// If we just transitioned, then our position information is invalid.
	if( dwLoadFlags == LOAD_TRANSITION )
	{
		m_vLastFindFloorPos = LTVector( 99999999.f, 99999999.f, 99999999.f );
	}

	LOAD_FLOAT(m_fSpeed);
	LOAD_BOOL(m_bCanShortRecoil);
	LOAD_BOOL(m_bForceGround);

	LOAD_HOBJECT(m_hLastDoor[0]);
	LOAD_HOBJECT(m_hLastDoor[1]);
	LOAD_VECTOR(m_vLastDoorPos[0]);
	LOAD_VECTOR(m_vLastDoorPos[1]);

	LOAD_BOOL(m_bUseDefaultAttachments);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::SetBrain
//
//	PURPOSE:	Changes our current Brain
//
// ----------------------------------------------------------------------- //

bool CAIHuman::SetBrain( char const* pszBrain )
{
	// Delete the old Brain

	if ( m_pBrain )
	{
		AI_FACTORY_DELETE(m_pBrain);
		m_pBrain = LTNULL;
	}

	m_pBrain = AI_FACTORY_NEW(CAIBrain);

	if ( !pszBrain || (-1 == g_pAIButeMgr->GetBrainIDByName( pszBrain )) )
	{
		g_pLTServer->CPrint("AI ''%s'' does not have a valid brain specified! Using ''Default''.", GetName());
		m_pBrain->Init(this, "Default");
	}
	else
	{
		m_pBrain->Init( this, pszBrain );
	}

	// Set DamageMask.

	if( m_pBrain->GetDamageMaskID() != -1 )
	{
		CDestructible* pDestructable = GetDestructible();
		AIASSERT( pDestructable, m_hObject, "CAIHuman::SetBrain: No destructable." );
		if( pDestructable )
		{
			int nDamageMaskID = m_pBrain->GetDamageMaskID();
			pDestructable->SetCantDamageFlags( ~( g_pAIButeMgr->GetDamageMask( nDamageMaskID )->dfDamageTypes ) );
		}
	}

	// Set music mood boundaries.

	if( m_pBrain->GetAIDataExist( kAIData_MusicMoodMin ) )
	{
		m_eMusicMoodMin = (CMusicMgr::Mood)((int)m_pBrain->GetAIData( kAIData_MusicMoodMin ));
	}
	if( m_pBrain->GetAIDataExist( kAIData_MusicMoodMax ) )
	{
		m_eMusicMoodMax = (CMusicMgr::Mood)((int)m_pBrain->GetAIData( kAIData_MusicMoodMax ));
	}

	SetState(kState_HumanIdle);

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHuman::SetMovementModifier()
//              
//	PURPOSE:	Sets the movement modifier to the passed in modifier, and
//				handles the initialization of the modifier
//
//              
//----------------------------------------------------------------------------
void CAIHuman::SetMovementModifier(IMovementModifier* pMovementModifier)
{
	AIASSERT( pMovementModifier != NULL, m_hObject, "NULL MovementModifier passed in" );
	if ( pMovementModifier == NULL )
	{
		return;
	}

	AIASSERT( m_pMovementModifier == NULL, m_hObject, "MovementModifier already set!" );
	if ( m_pMovementModifier != NULL )
	{
		return;
	}

	m_pMovementModifier = pMovementModifier;
	m_pMovementModifier->Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::AI_FACTORY_NEW_Strategy
//
//	PURPOSE:	Create a strategy
//
// ----------------------------------------------------------------------- //

CAIHumanStrategy* CAIHuman::AI_FACTORY_NEW_Strategy(EnumAIStrategyType eStrategyType)
{
	// Call AI_FACTORY_NEW for the requested type of strategy.

	switch( eStrategyType )
	{
		#define STRATEGY_TYPE_AS_SWITCH 1
		#include "AIStrategyTypeEnums.h"
		#undef STRATEGY_TYPE_AS_SWITCH

		default: AIASSERT( 0, m_hObject, "CAIHuman::AI_FACTORY_NEW_Strategy: Unrecognized strategy type.");
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::AI_FACTORY_NEW_State
//
//	PURPOSE:	Create a state
//
// ----------------------------------------------------------------------- //

CAIHumanState* CAIHuman::AI_FACTORY_NEW_State(EnumAIStateType eStateType)
{
	// Call AI_FACTORY_NEW for the requested type of state.

	switch( eStateType )
	{
		#define STATE_TYPE_AS_SWITCH_HUMAN 1
		#include "AIStateTypeEnums.h"
		#undef STATE_TYPE_AS_SWITCH_HUMAN

		default: AIASSERT( 0, m_hObject, "CAIHuman::AI_FACTORY_NEW_State: Unrecognized state type.");
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::ClearState
//
//	PURPOSE:	Clears our current state
//
// ----------------------------------------------------------------------- //

void CAIHuman::ClearState()
{
	AI_FACTORY_DELETE(m_pHumanState);
	m_pHumanState = LTNULL;
	m_pState = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::SetState
//
//	PURPOSE:	Changes our current state
//
// ----------------------------------------------------------------------- //


#define AITRACE2(a, b)

void CAIHuman::SetState(EnumAIStateType eState, LTBOOL bUnlockAnimation /* = LTTRUE */)
{
	AITRACE2(m_pState, (LTNULL, LTNULL) );


	if ( m_pState && m_pState->DelayChangeState() )
	{
		_ASSERT(!"Got direct SetState() in state that can't change states!!!");
	}

	// Delete the old state if state is changing.

	if ( m_pHumanState )
	{
		// Bail if requested state is the same as the current state.
		if(m_pHumanState->GetStateType() == eState)
		{
			return;
		}

		ClearState();
	}

	m_pHumanState = AI_FACTORY_NEW_State( eState );
	AIASSERT( m_pHumanState, m_hObject, "CAIHuman::SetState: Could not create state.");

	m_pState = m_pHumanState;

	if ( !m_pHumanState )
	{
		_ASSERT(LTFALSE);
		return;
	}

	// Unlock the animation context so we don't get stuck playing some dippy animation

	if ( bUnlockAnimation && GetAnimationContext() )
	{
		GetAnimationContext()->Unlock();
	}

	// Init the state

	if ( !m_pHumanState->Init(this) )
	{
		AIASSERT(0, m_hObject, "Failed to Init state" );
		AI_FACTORY_DELETE(m_pHumanState);
		m_pHumanState = LTNULL;
		m_pState = LTNULL;
	}

	// Update our user flags

	UpdateUserFlagCanActivate();

	AITRACE(AIShowStates, ( m_hObject, "Setting State %s (G:%s L:%d)\n", 
		s_aszStateTypes[m_pState->GetStateType()],
		m_pGoalMgr->GetCurrentGoal() ? s_aszGoalTypes[m_pGoalMgr->GetCurrentGoal()->GetGoalType()] : "None",
		m_pGoalMgr->HasLockedGoal() ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::ClearAndSetState()
//
//	PURPOSE:	Forces the clearing of the state, and then calls SetState to
//				change the state.  This may be needed in cases where the setter
//				requires that the state be reinitialized (which may not happen
//				if the current state is active.)
//
//----------------------------------------------------------------------------
void CAIHuman::ClearAndSetState(EnumAIStateType eState, LTBOOL bUnlockAnimation /* = LTTRUE */)
	{
	if ( m_pState && m_pState->DelayChangeState() )
	{
		_ASSERT(!"Got direct SetState() in state that can't change states!!!");
	}

	// Require deletion of the old state
	if ( m_pHumanState )
	{
		ClearState();
	}

	SetState( eState, bUnlockAnimation );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::CreateAttachments
//
//	PURPOSE:	Creates our attachments aggregate
//
// ----------------------------------------------------------------------- //

void CAIHuman::CreateAttachments()
{
	if ( !m_pAttachments )
	{
		m_pAttachments = static_cast<CHumanAttachments*>(CAttachments::Create(ATTACHMENTS_TYPE_HUMAN));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::GetWeaponPosition()
//
//	PURPOSE:	returns the position of our "Weapon" (could be anything)
//
// ----------------------------------------------------------------------- //
LTVector CAIHuman::GetWeaponPosition(CWeapon* pWeapon)
{
	if ( g_pAIButeMgr->GetRules()->bUseTrueWeaponPosition )
	{
		return GetTrueWeaponPosition(pWeapon);
	}
	else
	{
		return GetApproximateWeaponPosition(pWeapon);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHuman::GetApproximateWeaponPosition()
//              
//	PURPOSE:	Returns a 'rough' position of the weapon 
//              
//----------------------------------------------------------------------------
LTVector CAIHuman::GetApproximateWeaponPosition(CWeapon* pWeapon)
{
	LTVector vPos = m_vPos;
	vPos.y += m_vDims.y;
	return vPos;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHuman::GetTrueWeaponPosition()
//              
//	PURPOSE:	Returns the exact position of the wapon based on the socket
//				position.
//              
//----------------------------------------------------------------------------
LTVector CAIHuman::GetTrueWeaponPosition(CWeapon* pWeapon)
{
	HMODELSOCKET hSocket = GetWeaponSocket( pWeapon );
	LTransform transform;
	LTRESULT SocketTransform = g_pModelLT->GetSocketTransform(m_hObject, hSocket, transform, LTTRUE);
	AIASSERT( SocketTransform == LT_OK, m_hObject, "Unable to get socket for transform" );
	return transform.m_Pos;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::StartDeath()
//
//	PURPOSE:	Start dying - spawn holstered items
//
// ----------------------------------------------------------------------- //

void CAIHuman::StartDeath()
{
	if (m_bStartedDeath) return;
	super::StartDeath();

	SpawnHolsteredItems();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::SetUnconscious()
//
//	PURPOSE:	Go to sleep/wake up
//
// ----------------------------------------------------------------------- //

void CAIHuman::SetUnconscious(bool bUnconscious)
{

	if (bUnconscious == m_bUnconscious) return;

	if (bUnconscious)
	{
		SpawnHolsteredItems();
		super::SetUnconscious(bUnconscious);
	}
	else
	{
		super::SetUnconscious(bUnconscious);
		HolsterSpawnedItems();
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::SpawnHolsteredItems()
//
//	PURPOSE:	spawn holstered items when dying or getting knocked out
//
// ----------------------------------------------------------------------- //

void CAIHuman::SpawnHolsteredItems()
{
	if ( !m_pAttachments ) return;

	const char *szHolsterString = GetHolsterString();
	char *pszHolsterString = (char *)szHolsterString;

	if ( !szHolsterString ) return;

	// The tilda '~' indicates that this holster string was created from within
	// the code, and should be cleared after drawing a weapon.

	uint32 iStartHolster = 0;
	if( szHolsterString[0] == '~' )
	{
		pszHolsterString++;
	}

	char szHolsterRight[64];
	char szHolsterLeft[64];

	char* pColon = strchr(pszHolsterString, ';');
	if( !pColon )
	{
		strncpy( szHolsterRight, pszHolsterString, 64 );
		szHolsterLeft[0] = '\0';
	}
	else {
		strncpy( szHolsterRight, pszHolsterString, 64 );
		szHolsterRight[(pColon - pszHolsterString)] = '\0';
		strncpy( szHolsterLeft, pColon + 1, 64 );
	}


	
	if (!m_pAttachments->HasWeapons())
	{

		char szWpn[64] = "";
		if( szHolsterRight[0] )
		{
			SAFE_STRCPY(szWpn,szHolsterRight);
			char* pAmmo = strchr(szWpn, ',');
			if( pAmmo )
			{
				*pAmmo = '\0';
				pAmmo++;
			}

			const WEAPON* pWeapon = g_pWeaponMgr->GetWeapon( szWpn );
			if( pWeapon && pWeapon->szHolsterAttachment[0] )
			{
				char szPos[64] = "";
				SAFE_STRCPY(szPos,pWeapon->szHolsterAttachment);
				char* pPos = strchr(szPos, ' ');
				if( pPos )
				{
					*pPos = '\0';
				}

				CAttachment *pAttach = m_pAttachments->GetAttachment(szPos);
				uint32 dwAni = INVALID_ANI;
				if (pAttach)
				{
					HOBJECT hWpnModel = pAttach->GetModel();
					dwAni = g_pLTServer->GetModelAnimation(hWpnModel);
				m_pAttachments->Detach(szPos);
				}
				m_pAttachments->Attach(szPos, szHolsterRight);

				pAttach = m_pAttachments->GetAttachment(szPos);
				if (pAttach && dwAni != INVALID_ANI)
				{
					HOBJECT hWpnModel = pAttach->GetModel();
					dwAni = g_pLTServer->GetModelAnimation(hWpnModel);
					LTVector vDims;
					g_pCommonLT->GetModelAnimUserDims(hWpnModel, &vDims, dwAni);
					g_pPhysicsLT->SetObjectDims(hWpnModel, &vDims, 0);
					g_pLTServer->SetModelAnimation(hWpnModel, dwAni);
				}

			}
		}
		if( szHolsterLeft[0] )
		{
			SAFE_STRCPY(szWpn,szHolsterLeft);
			char* pAmmo = strchr(szWpn, ',');
			if( pAmmo )
			{
				*pAmmo = '\0';
				pAmmo++;
			}

			const WEAPON* pWeapon = g_pWeaponMgr->GetWeapon( szWpn );
			if( pWeapon && pWeapon->szHolsterAttachment[0] )
			{
				char szPos[64] = "";
				SAFE_STRCPY(szPos,pWeapon->szHolsterAttachment);
				char* pPos = strchr(szPos, ' ');
				if( pPos )
				{
					*pPos = '\0';
				}

				CAttachment *pAttach = m_pAttachments->GetAttachment(szPos);
				uint32 dwAni = INVALID_ANI;
				if (pAttach)
				{
					HOBJECT hWpnModel = pAttach->GetModel();
					dwAni = g_pLTServer->GetModelAnimation(hWpnModel);
				m_pAttachments->Detach(szPos);
				}
				m_pAttachments->Attach(szPos, szHolsterLeft);

				pAttach = m_pAttachments->GetAttachment(szPos);
				if (pAttach && dwAni != INVALID_ANI)
				{
					HOBJECT hWpnModel = pAttach->GetModel();
					dwAni = g_pLTServer->GetModelAnimation(hWpnModel);
					LTVector vDims;
					g_pCommonLT->GetModelAnimUserDims(hWpnModel, &vDims, dwAni);
					g_pPhysicsLT->SetObjectDims(hWpnModel, &vDims, 0);
					g_pLTServer->SetModelAnimation(hWpnModel, dwAni);
				}
			}
		}

	}

	ClearHolsterString();

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::HolsterSpawnedItems()
//
//	PURPOSE:	holster spawned items when waking up
//
// ----------------------------------------------------------------------- //

void CAIHuman::HolsterSpawnedItems()
{
	if ( !m_pAttachments ) return;

	if (m_pAttachments->HasWeapons())
	{
		CWeapon* apWeapons[kMaxWeapons];
		CAttachmentPosition* apAttachmentPositions[kMaxWeapons];
		uint32 nWeapons = m_pAttachments->EnumerateWeapons(apWeapons, apAttachmentPositions, kMaxWeapons);

		char szHolster[128] = "";

		uint32 nWpn = 0;
		char szTrigger[256];
		char szPos[64];

		while ( nWpn < nWeapons )
		{
			
			SAFE_STRCPY(szPos,apAttachmentPositions[nWpn]->GetName());


			
			// if it's in a hand, don't reholster
			if (stricmp(szPos,"RightHand") && stricmp(szPos,"LeftHand"))
			{
				const WEAPON* pWeapon = apWeapons[nWpn]->GetWeaponData();
				const AMMO* pAmmo = apWeapons[nWpn]->GetAmmoData();
				if( pWeapon && pWeapon->szHolsterAttachment[0] )
				{
					m_pAttachments->Detach(szPos);
					
					sprintf( szTrigger, "ATTACH %s", pWeapon->szHolsterAttachment );
					SendTriggerMsgToObject( this, m_hObject, LTFALSE, szTrigger );

					if( szHolster[0] )
						strcat(szHolster,";");

					char szWpn[64];
					sprintf(szWpn,"%s,%s",pWeapon->szName,pAmmo->szName);

					strcat(szHolster,szWpn);

				}
	
			}

			nWpn++;
		}

		if( szHolster[0] )
		{
			m_hstrHolster = g_pLTServer->CreateString( szHolster );
		}
	}

}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHuman::SetAIType()
//              
//	PURPOSE:	A very ugly little function to handle per Type AI
//				customizations.  This functions the same way that NOLFs
//				inheritance override worked: 
//              
//----------------------------------------------------------------------------
void CAIHuman::SetAIType( const char* const szType )
{
	if ( 0 == strcmp(szType, "Ground") )
	{
		SetHumanType(eHT_Ground);
	}
	else if (0 == strcmp(szType, "Swimming"))
	{
		SetHumanType(eHT_Swimming);
	}
	else if (0 == strcmp(szType, "Paratrooper"))
	{
		SetHumanType(eHT_Paratrooping);
	}
	else if (0 == strcmp(szType, "Hovering"))
	{
		// If this is a hovering character, then we need to use the special
		// superdooper hover movement modifier.  This modifier will cause the 
		// AI to interpolate the vertical movememnt, and to, occationally, not
		// stop instantly when doing things like following.
		SetMovementModifier( new CHoverMovementModifier );
		SetHumanType(eHT_Hovering);
	}
	else
	{
		g_pLTServer->CPrint( "Unknown CharacterType: %s, defaulting to ground", szType );
		SetHumanType(eHT_Ground);
	}
}


/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::SetSleeping()
//
//	PURPOSE:	Handle sleep mode
//
// ----------------------------------------------------------------------- //

void CAIHuman::SetSleeping(LTBOOL bSleeping)
{
    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_SLEEP_MSG);
	g_pLTServer->WriteToMessageByte(hMessage, bSleeping );
	g_pLTServer->EndMessage2(hMessage, 0);

	// Update our special fx message so new clients will get the updated
	// info as well...

	CreateSpecialFX();
}
*/
// Plugin statics
#ifndef __PSX2
CAIButeMgr s_AIButeMgr;
CAIGoalButeMgr s_AIGoalButeMgr;
CRelationButeMgr s_RelationButeMgr;


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanPlugin::CAIHumanPlugin, ~CAIHumanPlugin,
//				GetAttachmentsPlugin
//              
//	PURPOSE:	Now handles pointers instead to the CHumanAttachmentsPlugin
//				instead of instances, as this lets us avoid coupling in the
//				header with attachments.h
//              
//----------------------------------------------------------------------------
CAIHumanPlugin::CAIHumanPlugin()
{
	m_pHumanAttachmentsPlugin = debug_new( CHumanAttachmentsPlugin );
}
CAIHumanPlugin::~CAIHumanPlugin()
{
	if ( m_pHumanAttachmentsPlugin )
	{
		debug_delete( m_pHumanAttachmentsPlugin );
		m_pHumanAttachmentsPlugin = NULL;
	}
}
CAttachmentsPlugin* CAIHumanPlugin::GetAttachmentsPlugin()
{	
	ASSERT( m_pHumanAttachmentsPlugin != NULL ); 
	return m_pHumanAttachmentsPlugin;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //

LTRESULT CAIHumanPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	static bool bPluginInitted = false;

	if ( !bPluginInitted )
	{
		char szFile[256];

		// Make sure the weaponmgr plugin is inited.
		m_WeaponMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		// Make sure the modelbutemgr plugin is inited.
		m_ModelButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		sprintf(szFile, "%s\\Attributes\\AIButes.txt", szRezPath);
		s_AIButeMgr.SetInRezFile(LTFALSE);
        s_AIButeMgr.Init(szFile);

		if ( g_pAIGoalButeMgr == LTNULL )
		{
			sprintf(szFile, "%s\\Attributes\\AIGoals.txt", szRezPath);
			s_AIGoalButeMgr.SetInRezFile(LTFALSE);
			s_AIGoalButeMgr.Init(szFile);
		}

		bPluginInitted = true;
	}

	// See if the base class wishes to handle it

	if ( LT_OK == CAIPlugin::PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}

	// See if the attachments plugin will handle the property

	if ( LT_OK == GetAttachmentsPlugin()->PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}

	if ( !_strcmpi("GoalSet", szPropName) )
	{
		// TODO: make sure we don't overflow cMaxStringLength or cMaxStrings
		uint32 cGoalSets = s_AIGoalButeMgr.GetNumGoalSets();
		_ASSERT(cMaxStrings >= cGoalSets);
		AIGBM_GoalSet* pGoalSet;
		for ( uint32 iGoalSet = 0 ; iGoalSet < cGoalSets ; iGoalSet++ )
		{
			pGoalSet = s_AIGoalButeMgr.GetGoalSet(iGoalSet);
			if( pGoalSet && !( pGoalSet->dwGoalSetFlags & AIGBM_GoalSet::kGS_Hidden ) )
			{
				strcpy(aszStrings[(*pcStrings)++], pGoalSet->szName);
			}
		}

		return LT_OK;
	}
	else if ( !_strcmpi("ModelTemplate", szPropName) )
	{
		uint32 cModels = g_pModelButeMgr->GetNumModels();
		_ASSERT(cMaxStrings >= cModels);
		for ( uint32 iModel = 0 ; iModel < cModels ; iModel++ )
		{
			strcpy(aszStrings[(*pcStrings)++], g_pModelButeMgr->GetModelName((ModelId)iModel));
		}

		return LT_OK;
	}
	else if ( !_strcmpi("Brain", szPropName) )
	{
		// TODO: make sure we don't overflow cMaxStringLength or cMaxStrings
		uint32 cBrains = s_AIButeMgr.GetNumBrains();
		_ASSERT(cMaxStrings >= cBrains);
		for ( uint32 iBrain = 0 ; iBrain < cBrains ; iBrain++ )
		{
			strcpy(aszStrings[(*pcStrings)++], s_AIButeMgr.GetBrain(iBrain)->szName);
		}

		return LT_OK;
	}

	if (m_SearchItemPlugin.PreHook_EditStringList(szRezPath,
		szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
	{
		return LT_OK;
	}


	// No one wants it

	return LT_UNSUPPORTED;
}
#endif

