// ----------------------------------------------------------------------- //
//
// MODULE  : AI.cpp
//
// PURPOSE : Base AI class
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "AI.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "VolumeBrushTypes.h"
#include "PlayerObj.h"
#include "AIState.h"
#include "AIPathMgr.h"
#include "AITarget.h"
#include "TeleportPoint.h"
#include "AINodeMgr.h"
#include "AIRegion.h"
#include "Camera.h"
#include "AISenseRecorderGame.h"
#include "AIGoalMgr.h"
#include "AIGoalAbstract.h"
#include "AIBrain.h"
#include "AIHumanState.h"
#include "AIVolumeMgr.h"

#include "AnimationMgr.h"
#include "AIGoalButeMgr.h"
#include "ServerSoundMgr.h"
#include "SurfaceMgr.h"
#include "AIStimulusMgr.h"
#include "CharacterMgr.h"
#include "SurfaceFunctions.h"
#include "RelationButeMgr.h"
#include "Weapon.h"
#include "Attachments.h"
#include "ServerTrackedNodeMgr.h"
#include "ServerTrackedNodeContext.h"
#include "ObjectRelationMgr.h"
#include "AIAssert.h"
#include "AIMovement.h"
#include "AIPathKnowledgeMgr.h"
#include "KeyItem.h"
#include "Spawner.h"
#include "CharacterHitBox.h"
#include "RelationMgr.h"
#include "AICentralKnowledgeMgr.h"
#include "Searchable.h"
#include "AIGoalSpecialDamage.h"
#include "SharedFXStructs.h"

#include <algorithm>

extern CAIGoalButeMgr* g_pAIGoalButeMgr;
extern CServerSoundMgr* g_pServerSoundMgr;
extern CAIStimulusMgr* g_pAIStimulusMgr;
extern CCharacterMgr* g_pCharacterMgr;

static CVarTrack g_SenseInfoTrack;
static CVarTrack g_AccuracyInfoTrack;
static CVarTrack g_AIInfoTrack;

// Specifies the maximum number of ai's that can be in the level.  If number
// is over this level, no new one's will be spawned in.
static CVarTrack g_AIMaxNumber;

LINKFROM_MODULE( AI );


#define ROTATION_SPEED_STATIC	0.5f
#define ROTATION_SPEED_MOVING	2.0f

#define	BODY_SKIN_INDEX			0
#define HEAD_SKIN_INDEX			1
#define SKIN_EXT_RANDOM			"RANDOM"


// Define our properties (what is available in DEdit)...
#pragma force_active on
BEGIN_CLASS(CAI)

	// Overrides

	ADD_STRINGPROP_FLAG(SpawnItem, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(HitPoints, -1.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(ArmorPoints, -1.0f, PF_HIDDEN)
	ADD_STRINGPROP(BodySkinExtension, "")

	// New properties

	ADD_BOOLPROP_FLAG(IsCinematicAI, LTFALSE, 0)
	ADD_BOOLPROP_FLAG(CanTalk, LTTRUE, 0)

	PROP_DEFINEGROUP(AttributeOverrides, PF_GROUP(3))

		// Basic attributes

		ADD_STRINGPROP_FLAG(SoundRadius,	"", PF_GROUP(3)|PF_RADIUS)
		ADD_STRINGPROP_FLAG(HitPoints,		"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(Armor,			"", PF_GROUP(3))

		ADD_STRINGPROP_FLAG(Accuracy,					"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(FullAccuracyRadius,			"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(AccuracyMissPerturb,		"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(MaxMovementAccuracyPerturb, "", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(MovementAccuracyPerturbTime,"", PF_GROUP(3))

		ADD_STRINGPROP_FLAG(Awareness,		"", PF_GROUP(3))

		ADD_BOOLPROP_FLAG(Senses,			LTTRUE, PF_GROUP(3))

		// Sense attributes

		ADD_STRINGPROP_FLAG(CanSeeEnemy,					"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(SeeEnemyDistance,				"", PF_GROUP(3)|PF_RADIUS)
		ADD_STRINGPROP_FLAG(CanSeeAllyDeath,				"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(SeeAllyDeathDistance,			"", PF_GROUP(3)|PF_RADIUS)
		ADD_STRINGPROP_FLAG(CanHearAllyDeath,				"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(HearAllyDeathDistance,			"", PF_GROUP(3)|PF_RADIUS)
		ADD_STRINGPROP_FLAG(CanHearAllyPain,				"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(HearAllyPainDistance,			"", PF_GROUP(3)|PF_RADIUS)
		ADD_STRINGPROP_FLAG(CanSeeEnemyFlashlight,			"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(SeeEnemyFlashlightDistance,		"", PF_GROUP(3)|PF_RADIUS)
		ADD_STRINGPROP_FLAG(CanSeeEnemyFootprint,			"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(SeeEnemyFootprintDistance,		"", PF_GROUP(3)|PF_RADIUS)
		ADD_STRINGPROP_FLAG(CanHearEnemyFootstep,			"",	PF_GROUP(3))
		ADD_STRINGPROP_FLAG(HearEnemyFootstepDistance,		"", PF_GROUP(3)|PF_RADIUS)
		ADD_STRINGPROP_FLAG(CanHearEnemyDisturbance,		"",	PF_GROUP(3))
		ADD_STRINGPROP_FLAG(HearEnemyDisturbanceDistance,	"", PF_GROUP(3)|PF_RADIUS)
		ADD_STRINGPROP_FLAG(CanHearEnemyWeaponImpact,		"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(HearEnemyWeaponImpactDistance,	"", PF_GROUP(3)|PF_RADIUS)
		ADD_STRINGPROP_FLAG(CanHearEnemyWeaponImpact,		"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(HearEnemyWeaponImpactDistance,	"", PF_GROUP(3)|PF_RADIUS)
		ADD_STRINGPROP_FLAG(CanHearEnemyWeaponFire,			"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(HearEnemyWeaponFireDistance,	"", PF_GROUP(3)|PF_RADIUS)
		ADD_STRINGPROP_FLAG(CanHearAllyWeaponFire,			"", PF_GROUP(3))
		ADD_STRINGPROP_FLAG(HearAllyWeaponFireDistance,		"", PF_GROUP(3)|PF_RADIUS)

	PROP_DEFINEGROUP(Commands, PF_GROUP(4))

		ADD_STRINGPROP_FLAG(Initial,				"", PF_GROUP(4) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(ActivateOn,				"", PF_GROUP(4) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(ActivateOff,			"", PF_GROUP(4) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(ProximityGoalCommand,	"", PF_GROUP(4) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(OnMarkingCommand,		"", PF_GROUP(4) | PF_NOTIFYCHANGE)

END_CLASS_DEFAULT_FLAGS_PLUGIN(CAI, CCharacter, NULL, NULL, 0, CAIPlugin)
#pragma force_active off

static LTBOOL ValidateMsgRemoveGoal( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	uint32 iGoalType;

	if( !g_pAIGoalButeMgr )
		return LTTRUE;

	// Find goal by name.
	for(iGoalType=0; iGoalType < kGoal_Count; ++iGoalType)
	{
		if( stricmp(s_aszGoalTypes[iGoalType], cpMsgParams.m_Args[1]) == 0)
		{
			break;
		}
	}

	AIGBM_GoalTemplate* pGoalTemplate = g_pAIGoalButeMgr->GetTemplate( (EnumAIGoalType)iGoalType );
	if( !pGoalTemplate || iGoalType == kGoal_Count )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateMsgRemoveGoal()" );
			pInterface->CPrint( "    MSG - REMOVEGOAL - Invalid goal '%s' not found in file '%s'!", cpMsgParams.m_Args[1], GOAL_FILE );
		}

		return LTFALSE;
	}

	return LTTRUE;
}

static LTBOOL ValidateMsgAddGoal( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !g_pAIGoalButeMgr )
		return LTTRUE;

	if( cpMsgParams.m_nArgs < 2 || !cpMsgParams.m_Args[1] )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateMsgAddGoal()" );
			pInterface->CPrint( "    MSG - ADDGOAL - No goal was specified!" );
		}

		return LTFALSE;
	}

	// Find goal by name.
	uint32 iGoalType;
	for(iGoalType=0; iGoalType < kGoal_Count; ++iGoalType)
	{
		if( stricmp(s_aszGoalTypes[iGoalType], cpMsgParams.m_Args[1]) == 0)
		{
			break;
		}
	}

	AIGBM_GoalTemplate* pGoalTemplate = g_pAIGoalButeMgr->GetTemplate( (EnumAIGoalType)iGoalType );
	if( !pGoalTemplate || iGoalType == kGoal_Count )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateMsgAddGoal()" );
			pInterface->CPrint( "    MSG - ADDGOAL - Invalid goal '%s' not found in file '%s'!", cpMsgParams.m_Args[1], GOAL_FILE );
		}

		return LTFALSE;
	}

	return LTTRUE;
}

static LTBOOL ValidateMsgGoalset( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !g_pAIGoalButeMgr )
		return LTTRUE;

	uint32 iGoalSet = g_pAIGoalButeMgr->GetGoalSetIndex( cpMsgParams.m_Args[1] );
	if( iGoalSet == (uint32)-1 )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateMsgGoalset()" );
			pInterface->CPrint( "    MSG - GOALSET - Invalid goalset '%s' not found in file '%s'!", cpMsgParams.m_Args[1], GOAL_FILE );
		}

		return LTFALSE;
	}

	return LTTRUE;
}

static LTBOOL ValidateMsgGoalPrefix( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !g_pAIGoalButeMgr )
		return LTTRUE;

	uint8 len = strlen(GOAL_CMD_PREFIX);

	// Check to see if this is a goal prefix command and verrify the goal...

	if( _strnicmp( cpMsgParams.m_Args[0], GOAL_CMD_PREFIX, len ) == 0 )
	{
		char szMsg[128] = {0};
		LTStrCpy( szMsg, cpMsgParams.m_Args[0], ARRAY_LEN( szMsg ));

		const char* pPrefix = strtok( szMsg, "_" );
		const char* pGoal = strtok( NULL, " " );

		if( !pGoal )
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - ValidateMsgGoalPrefix()" );
				pInterface->CPrint( "    MSG - %s - No goal was specified!", GOAL_CMD_PREFIX );

				CCommandMgrPlugin::SetForceDisplayPropInfo( true );
			}

			return LTTRUE;
		}

		// Find goal by name.
		uint32 iGoalType;
		for(iGoalType=0; iGoalType < kGoal_Count; ++iGoalType)
		{
			if( stricmp(s_aszGoalTypes[iGoalType], pGoal) == 0)
			{
				break;
			}
		}


		AIGBM_GoalTemplate* pGoalTemplate = g_pAIGoalButeMgr->GetTemplate( (EnumAIGoalType)iGoalType );
		if( !pGoalTemplate || iGoalType == kGoal_Count )
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - ValidateMsgGoalPrefix()" );
				pInterface->CPrint( "    MSG - %s - Invalid goal '%s' not found in file '%s'!", GOAL_CMD_PREFIX, pGoal, GOAL_FILE );

				CCommandMgrPlugin::SetForceDisplayPropInfo( true );
			}
		}

		// Always return true if we handled a special message
		return LTTRUE;
	}

	// Return false if the special message was not handled
	return LTFALSE;
}

static LTBOOL ValidateFaceObject( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( cpMsgParams.m_Args[1] )
	{
		if( pInterface->FindObject( cpMsgParams.m_Args[1] ) != LT_OK )
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - ValidateFaceObject()" );
				pInterface->CPrint( "    MSG - FACEOBJECT - Could not find object '%s'!", cpMsgParams.m_Args[1] );
			}

			return LTFALSE;
		}

		return LTTRUE;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - ValidateFaceObject()" );
		pInterface->CPrint( "    MSG - FACEOBJECT - No Object specified!");
	}

	return LTFALSE;
}

static LTBOOL ValidateTarget( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( cpMsgParams.m_Args[1] )
	{
		if( pInterface->FindObject( cpMsgParams.m_Args[1] ) != LT_OK )
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - ValidateTarget()" );
				pInterface->CPrint( "    MSG - TARGET - Could not find object '%s'!", cpMsgParams.m_Args[1] );
			}

			return LTFALSE;
		}

		return LTTRUE;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - ValidateTarget()" );
		pInterface->CPrint( "    MSG - TARGET - No Object specified!");
	}

	return LTFALSE;
}

static LTBOOL VelidateDelcmd( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( cpMsgParams.m_Args[1] )
	{
		if( !_stricmp( cpMsgParams.m_Args[1], "ACTIVATEON" ) ||
			!_stricmp( cpMsgParams.m_Args[1], "ACTIVATEOFF" ))
		{
			return LTTRUE;
		}

		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - VelidateDelcmd()" );
			pInterface->CPrint( "    MSG - DELCMD - Invalid Command specified '%s'!", cpMsgParams.m_Args[1] );
		}

		return LTFALSE;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - VelidateDelcmd()" );
		pInterface->CPrint( "    MSG - DELCMD - No Command specified!");
	}

	return LTFALSE;
}

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( CAI )

//					Message		Num Params	Validation FnPtr		Syntax
	CMDMGR_ADD_MSG( ACTIVATE,		1,		NULL,					"ACTIVATE" )
	CMDMGR_ADD_MSG( CINERACT,		2,		NULL,					"CINERACT <Animation Name>" )
	CMDMGR_ADD_MSG( CINERACTLOOP,	2,		NULL,					"CINERACTLOOP <Animation Name>" )
	CMDMGR_ADD_MSG( TRACKLOOKAT,	1,		NULL,					"TRACKLOOKAT" )
	CMDMGR_ADD_MSG( TRACKAIMAT,		1,		NULL,					"TRACKAIMAT" )
	CMDMGR_ADD_MSG( TRACKNONE,		1,		NULL,					"TRACKNONE" )
	CMDMGR_ADD_MSG( ALIGNMENT,		2,		NULL,					"ALIGNMENT <key>" )
	CMDMGR_ADD_MSG( DAMAGEMASK,		2,		NULL,					"DAMAGEMASK <key>" )
	CMDMGR_ADD_MSG( IGNOREVOLUMES,	2,		NULL,					"IGNOREVOLUMES <1-or-0>" )
	CMDMGR_ADD_MSG( ADDGOAL,		-1,		ValidateMsgAddGoal,		"ADDGOAL <goal>" )
	CMDMGR_ADD_MSG( REMOVEGOAL,		2,		ValidateMsgRemoveGoal,	"REMOVEGOAL <goal>" )
	CMDMGR_ADD_MSG( GOALSET,		2,		ValidateMsgGoalset,		"GOALSET <goalset>" )
	CMDMGR_ADD_MSG( GOALSCRIPT,		-1,		NULL,					"GOALSCRIPT <script>" )
	CMDMGR_ADD_MSG( GRAVITY,		2,		NULL,					"GRAVITY <1 or 0>")
	CMDMGR_ADD_MSG( SENSES,			2,		NULL,					"SENSES <1 or 0>")
	CMDMGR_ADD_MSG( ALWAYSACTIVATE,	2,		NULL,					"ALWAYSACTIVATE <1 or 0>")
	CMDMGR_ADD_MSG( CANTALK,		2,		NULL,					"CANTALK <1 or 0>")
	CMDMGR_ADD_MSG( DEBUG,			2,		NULL,					"DEBUG <level>")
	CMDMGR_ADD_MSG( MOVE,			2,		NULL,					"MOVE <X,Y,Z>")
	CMDMGR_ADD_MSG( SHOOTTHROUGH,	2,		NULL,					"SHOOTTHROUGH <1 or 0>")
	CMDMGR_ADD_MSG( SEETHROUGH,		2,		NULL,					"SEETHROUGH <1 0r 0>")
	CMDMGR_ADD_MSG( FOVBIAS,		2,		NULL,					"FOVBIAS <fov>")
	CMDMGR_ADD_MSG( PLAYSOUND,		2,		NULL,					"PLAYSOUND <sound>")
	CMDMGR_ADD_MSG( ATP,			1,		NULL,					"ATP")
	CMDMGR_ADD_MSG( FACEOBJECT,		2,		ValidateFaceObject,		"FACEOBJECT <object>")
	CMDMGR_ADD_MSG( FACEPOS,		2,		NULL,					"FACEPOS <X,Y,Z>" )
	CMDMGR_ADD_MSG( FACEDIR,		2,		NULL,					"FACEDIR <X,Y,Z> ")
	CMDMGR_ADD_MSG( FACETARGET,		1,		NULL,					"FACETARGET" )
	CMDMGR_ADD_MSG( TARGET,			2,		ValidateTarget,			"TARGET <object>" )
	CMDMGR_ADD_MSG( TARGETPLAYER,	1,		NULL,					"TARGETPLAYER" )
	CMDMGR_ADD_MSG( PING,			1,		NULL,					"PING" )
	CMDMGR_ADD_MSG( PRESERVEACTIVATECMDS,	1,	NULL,				"PRESERVEACTIVATECMDS" )
	CMDMGR_ADD_MSG( DELCMD,			2,		VelidateDelcmd,			"DELCMD <activateon or activateoff>")
	CMDMGR_ADD_MSG( HEALTHMOD,		2,		NULL,					"HEALTHMOD <mod>" )
	CMDMGR_ADD_MSG( ARMORMOD,		2,		NULL,					"ARMORMOD <mod>" )
	CMDMGR_ADD_MSG( DESTROY,		1,		NULL,					"DESTROY" )

	CMDMGR_ADD_MSG_SPECIAL( ValidateMsgGoalPrefix )

CMDMGR_END_REGISTER_CLASS( CAI, CCharacter )


// static functions
static void SendInfoString( HOBJECT hObject, const char * szInfo )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(hObject);
	cMsg.Writeuint8(CFX_INFO_STRING);
	cMsg.WriteString(szInfo);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, 0);
}

static CBankedList<INVALID_NODE> s_bankINVALID_NODE;

//
// DeleteExpiredNodes
//
// Deletes a node if it has expired.
//
struct DeleteExpiredNodes :
std::binary_function<INVALID_NODE*, float, INVALID_NODE*>
{
	INVALID_NODE* operator()( INVALID_NODE* pNode, float flTime ) const
	{
		if ( pNode->m_flTime < flTime )
		{
			// If the node timed out, delete it.
			s_bankINVALID_NODE.Delete( pNode );
			pNode = NULL;
		}
		return pNode;
	}
};

// Filter functions

HOBJECT s_hFilterAI = LTNULL;


bool CAI::DefaultFilterFn(HOBJECT hObj, void *pUserData)
{
    if ( !hObj ) return false;
    if ( hObj == s_hFilterAI ) return false;

    static HCLASS hBody = g_pLTServer->GetClass("Body");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hBody))
	{
        return false;
	}

    static HCLASS hHitbox = g_pLTServer->GetClass("CCharacterHitBox");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hHitbox))
	{
        return false;
	}

	return LiquidFilterFn(hObj, pUserData);
}

bool CAI::BodyFilterFn(HOBJECT hObj, void *pUserData)
{
    if (!hObj ) return false;
    if ( hObj == s_hFilterAI ) return false;

    static HCLASS hHitbox = g_pLTServer->GetClass("CCharacterHitBox");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hHitbox))
	{
        return false;
	}

	return LiquidFilterFn(hObj, pUserData);
}

bool CAI::ShootThroughFilterFn(HOBJECT hObj, void *pUserData)
{
    if ( !hObj ) return false;
    if ( hObj == s_hFilterAI ) return false;

	if ( IsMainWorld(hObj) )
	{
        return true;
	}

    static HCLASS hBody = g_pLTServer->GetClass("Body");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hBody))
	{
        return false;
	}

    static HCLASS hHitbox = g_pLTServer->GetClass("CCharacterHitBox");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hHitbox))
	{
        return false;
	}

	return LiquidFilterFn(hObj, pUserData);
}

bool CAI::ShootThroughPolyFilterFn(HPOLY hPoly, void *pUserData)
{
    if ( INVALID_HPOLY == hPoly ) return false;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hPoly));
	if ( !pSurf )
	{
		g_pLTServer->CPrint("Warning, HPOLY had no associated surface!");
		return false;
	}

    if ( pSurf->bCanShootThrough )
	{
		return false;
	}

    return true;
}

bool CAI::SeeThroughFilterFn(HOBJECT hObj, void *pUserData)
{
    if ( !hObj ) return false;
    if ( hObj == s_hFilterAI ) return false;

	if ( IsMainWorld(hObj) )
	{
        return true;
	}

    static HCLASS hBody = g_pLTServer->GetClass("Body");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hBody))
	{
        return false;
	}

    static HCLASS hHitbox = g_pLTServer->GetClass("CCharacterHitBox");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hHitbox))
	{
        return false;
	}

    static HCLASS hCharacter = g_pLTServer->GetClass("CCharacter");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hCharacter))
	{
        return false;
	}

	return LiquidFilterFn(hObj, pUserData);
}

bool CAI::SeeThroughPolyFilterFn(HPOLY hPoly, void *pUserData)
{
    if ( INVALID_HPOLY == hPoly ) return false;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hPoly));
	if ( !pSurf )
	{
		g_pLTServer->CPrint("Warning, HPOLY had no associated surface!");
		return false;
	}

    if ( pSurf->bCanSeeThrough )
	{
		return false;
	}

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CAI()
//
//	PURPOSE:	Constructor - Initialize
//
// ----------------------------------------------------------------------- //

CAI::AIList CAI::m_lstAIs;

CAI::CAI() : CCharacter()
{
	m_pTarget = AI_FACTORY_NEW(CAITarget);

    m_bFirstUpdate = LTTRUE;

	VEC_INIT(m_vPos);

    m_rRot.Init();

	VEC_INIT(m_vRight);
	VEC_INIT(m_vUp);
	VEC_INIT(m_vForward);

	VEC_INIT(m_vEyePos);
	VEC_INIT(m_vEyeForward);

	VEC_INIT(m_vDims);
	m_fRadius = 1.0f;

    m_rTargetRot.Init();
	VEC_INIT(m_vTargetRight);
	VEC_INIT(m_vTargetUp);
	VEC_INIT(m_vTargetForward);
    m_bRotating = LTFALSE;
	m_fRotationTime = -1.0f;
	m_fRotationTimer = 0.0f;

	m_bUpdateNodes = LTTRUE;

	m_fNextCombatSoundTime = 0.f;
	m_bMuteAISounds = LTFALSE;

	m_pState = LTNULL;

	m_pAnimationMgr = LTNULL;
	m_pAnimationContext = LTNULL;
	m_hAnimObject = LTNULL;

	m_bCheapMovement = LTTRUE;
	VEC_INIT(m_vMovePos);
	m_bMove = LTFALSE;
	m_fJumpOverVel = 0.f;

	m_flHoverAccelerationRate = 50;
	m_flLastHoverTime = 0;
	m_flCurrentHoverSpeed = 0;

	m_fSpeed = 250.0f;
	m_bClearMovementHint = LTTRUE;
	m_hHintAnim = INVALID_MODEL_ANIM;
	m_bUseMovementEncoding = LTFALSE;
	m_bTimeToUpdate = LTFALSE;

	m_pAIMovement = debug_new( CAIMovement );

	m_pPathKnowledgeMgr = AI_FACTORY_NEW( CAIPathKnowledgeMgr );
	m_pPathKnowledgeMgr->Init(this);

	m_pGoalMgr = AI_FACTORY_NEW(CAIGoalMgr);

	m_hstrHolster = LTNULL;
	m_hstrHolsterBackup = LTNULL;
	m_fRestoreBackupWeaponTime = 0.f;

	m_iCurrentWeapon = 0;
	m_iPrimaryWeapon = -1;
	m_ePrimaryWeaponProp = kAP_Weapon3;
	m_cWeapons = -1;
	memset(m_apWeapons, LTNULL, sizeof(CWeapon*)*AI_MAX_WEAPONS);
	memset(m_apWeaponPositions, LTNULL, sizeof(CAttachmentPosition*)*AI_MAX_WEAPONS);
	m_cDroppedWeapons = 0;
	// Initialize our spears to notify us if their objects get removed.
	for( int nWpn = 0; nWpn < ARRAY_LEN( m_aDroppedWeapons ); nWpn++ )
	{
		m_aDroppedWeapons[nWpn].hPickupItem = (HOBJECT)NULL;
		m_aDroppedWeapons[nWpn].hPickupItem.SetReceiver( *this );
	}

	m_cObjects = -1;
	memset(m_apObjects, LTNULL, sizeof(BaseClass*)*AI_MAX_OBJECTS);
	memset(m_apObjectPositions, LTNULL, sizeof(CAttachmentPosition*)*AI_MAX_OBJECTS);

	m_hstrBodySkinExtension = LTNULL;

	m_hstrNextStateMessage = LTNULL;
	m_fNextStateTime = 0.0f;

	m_fAccuracy = 0.0f;
	m_fAccuracyIncreaseRate = 0.0f;
	m_fAccuracyDecreaseRate = 0.0f;
	m_fAccuracyModifier = 1.0f;
	m_fAccuracyModifierTimer = 0.0f;
	m_fAccuracyModifierTime = 1.0f;
	m_fFullAccuracyRadiusSqr = 256.f;
	m_fAccuracyMissPerturb = 64.f;
	m_fMaxMovementAccuracyPerturb = 10.f;
	m_fMovementAccuracyPerturbDecay = 3.f;

	m_eAwareness = kAware_Relaxed;
	m_nAlarmLevel = 0;
	m_eAlarmStimID = kStimID_Unset;
	m_fLastStimulusTime = 0.f;
	m_fLastRelaxedTime = 0.f;

    m_bSeeThrough = LTTRUE;
    m_bShootThrough = LTTRUE;

	m_hstrCmdInitial = LTNULL;
	m_hstrCmdActivateOn = LTNULL;
	m_hstrCmdActivateOff = LTNULL;
	m_hstrCmdOnMarking = LTNULL;
	m_hstrCmdProximityGoal = LTNULL;

    m_bActivated = LTFALSE;
	m_bAlwaysActivate = LTFALSE;
	m_bCanTalk = true;

	m_bUnconscious = false;

	m_fFOVBias = 0.0f;

	m_bPreserveActiveCmds = LTFALSE;

	m_bInitializedAttachments = LTFALSE;

	m_fSenseUpdateRate	= 0.0f;
	m_fNextSenseUpdate	= 0.0f;
	m_flagsBaseSenses	= kSense_None;
	m_flagsCurSenses	= kSense_None;
	m_bSensesOn			= LTTRUE;
	m_pAISenseRecorder	= debug_new(CAISenseRecorderGame);
	m_pAISenseRecorder->Init(this);

	m_rngSightGridX.Set(-2, 2);
	m_rngSightGridY.Set(-2, 2);

	m_pTrackedNodeContext = debug_new(CServerTrackedNodeContext);
	m_eTriggerNodeTrackingGroup = kTrack_None;

	m_pBrain = NULL;
	m_pAITemplate = NULL;

	m_dwBaseValidVolumeTypes = AIVolume::kVolumeType_Count;
	m_dwCurValidVolumeTypes = AIVolume::kVolumeType_Count;

	// Debug level

	m_nDebugLevel = 0;

	m_vPathingPosition = LTVector(0,0,0);

	// init defense variables
	m_bDefenseOn = false;

	m_bCanCarry = false;
	m_bBeingCarried = false;
	m_bIsCinematicAI = false;
	m_bCanWake = true;

	m_bStuckWithTrackDart	= false;

	if( !g_AIMaxNumber.IsInitted( ))
	{
		g_AIMaxNumber.Init( g_pLTServer, "AIMaxNumber", LTNULL, 30.0f );
	}

	// Add this instance to a list of all AI's.
	m_lstAIs.push_back( this );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::~CAI()
//
//	PURPOSE:	Deallocate data
//
// ----------------------------------------------------------------------- //

CAI::~CAI()
{
    if (!g_pLTServer) return;

	// Take us out of the stimulusmgr
	g_pAIStimulusMgr->RemoveSensingObject( this );

	if ( m_pState )
	{
		AI_FACTORY_DELETE(m_pState);
		m_pState = LTNULL;
	}

	if ( m_pAnimationContext )
	{
		m_pAnimationMgr->DestroyAnimationContext(m_pAnimationContext);
		m_pAnimationContext = LTNULL;
	}

	if ( m_pAIMovement )
	{
		debug_delete( m_pAIMovement );
	}

	if( m_pPathKnowledgeMgr )
	{
		AI_FACTORY_DELETE( m_pPathKnowledgeMgr );
		m_pPathKnowledgeMgr = LTNULL;
	}

	if( m_pGoalMgr)
	{
		AI_FACTORY_DELETE(m_pGoalMgr);
		m_pGoalMgr = LTNULL;
	}

	m_hDialogueObject = NULL;

	AI_FACTORY_DELETE(m_pTarget);
	debug_delete(m_pAISenseRecorder);

	FREE_HSTRING(m_hstrBodySkinExtension);
	FREE_HSTRING(m_hstrCmdInitial);
	FREE_HSTRING(m_hstrCmdOnMarking);
	FREE_HSTRING(m_hstrCmdActivateOn);
	FREE_HSTRING(m_hstrCmdActivateOff);
	FREE_HSTRING(m_hstrCmdProximityGoal);

	FREE_HSTRING(m_hstrHolster);
	FREE_HSTRING(m_hstrHolsterBackup);

	if ( m_pBrain )
	{
		AI_FACTORY_DELETE(m_pBrain);
		m_pBrain = LTNULL;
	}

	debug_delete( m_pTrackedNodeContext );

	std::vector<INVALID_NODE*>::iterator it;
	for ( it = m_InvalidNodeList.begin(); it != m_InvalidNodeList.end(); ++it )
	{
		s_bankINVALID_NODE.Delete( *it );
		*it = NULL;
	}
	m_InvalidNodeList.erase( m_InvalidNodeList.begin(), m_InvalidNodeList.end() );

	// Erase this instance from the list of all PlayerObj's.
	AIList::iterator itAI = m_lstAIs.begin( );
	while( itAI != m_lstAIs.end( ))
	{
		if( *itAI == this )
		{
			m_lstAIs.erase( itAI );
			break;
		}

		itAI++;
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CAI::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			LTBOOL bPaused = LTFALSE;
			if( m_pState->GetStateStatus() == kSStat_Paused )
			{
				bPaused = LTTRUE;
			}

			if( !bPaused )
			{
				g_pLTServer->SetNextUpdate(m_hObject, c_fUpdateDelta);
			}

			// If model has movement encoding, do not update until after
			// MID_TRANSFORMHINT comes. Call Update() from end of MID_TRANSFORMHINT.
			// This is due to ordering issues of when MID_UPDATE and MID_TRANSFORMHINT
			// are sent. The movement hint transform needs to be updated before calling
			// Update().

			if( !m_bUseMovementEncoding )
			{
				PreUpdate();
				Update();
			}
			else if( !m_bTimeToUpdate )
			{
				PreUpdate();
				m_bTimeToUpdate = LTTRUE;
			}
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			HandleTouch((HOBJECT)pData);
		}
		break;

		case MID_MODELSTRINGKEY:
		{
			HandleModelString((ArgList*)pData);
		}
		break;

		case MID_ACTIVATING:
		{
			g_pAIStimulusMgr->AddSensingObject( this );
		}
		break;

		case MID_DEACTIVATING:
		{
			g_pAIStimulusMgr->RemoveSensingObject( this );
		}
		break;

		case MID_INITIALUPDATE:
		{
			m_pAIMovement->Init(this);

			g_pLTServer->SetNextUpdate(m_hObject, c_fUpdateDelta);

			g_pLTServer->SetNetFlags(m_hObject, NETFLAG_POSUNGUARANTEED|NETFLAG_ROTUNGUARANTEED|NETFLAG_ANIMUNGUARANTEED);

			m_pTarget->Init(this);

			int nInfo = (int)fData;
			if (nInfo != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			// Set up movement encoding.

			HMODELNODE hNode = INVALID_MODEL_NODE;
			g_pModelLT->GetNode(m_hObject, "movement", hNode);
			if( hNode != INVALID_MODEL_NODE )
			{
				ANIMTRACKERID nTracker;
				if ( LT_OK == g_pModelLT->GetMainTracker(m_hObject, nTracker) )
				{
					m_bUseMovementEncoding = LTTRUE;
					g_pModelLT->SetHintNode( m_hObject, nTracker, hNode );
				}
			}

			uint32 dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);

			if (nInfo != INITIALUPDATE_SAVEGAME)
			{
				// Update our position, rotation, etc.  Need to do this
				// after the character does the initialupdate, because character
				// sets up our dims.
				UpdatePosition();

				// Update our can activate flags.  Need to do this after the character does the
				// initialupdate because character creates our hitbox.
				UpdateUserFlagCanActivate();
			}

			return dwRet;
		}
		break;

		case MID_PRECREATE:
		{
			int nInfo = (int)fData;

			// Check if we're being spawned in.
			if( nInfo == PRECREATE_STRINGPROP || nInfo == PRECREATE_NORMAL )
			{
				// If this ai puts us over the limit, then fail to create.
				if( m_lstAIs.size( ) >= ( uint32 )g_AIMaxNumber.GetFloat( ))
				{
					g_pLTServer->CPrint("Spawned AI over the max limit");
					return 0;
				}
			}

            uint32 dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);

			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				PreReadProp((ObjectCreateStruct*)pData);
				ReadProp((ObjectCreateStruct*)pData);
				PostPropRead((ObjectCreateStruct*)pData);
			}
			return dwRet;
		}
		break;

		case MID_SAVEOBJECT:
		{
            uint32 dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);
            Save((ILTMessage_Write*)pData, (uint32)fData);
			return dwRet;
		}
		break;

		case MID_LOADOBJECT:
		{
            uint32 dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);
			m_pAnimationMgr = g_pAnimationMgrList->GetAnimationMgr( g_pModelButeMgr->GetModelAnimationMgr(m_eModelId) );

            Load((ILTMessage_Read*)pData, (uint32)fData);
			return dwRet;
		}
		break;

		case MID_TRANSFORMHINT:
		{
			// Movement encoding hint.

			m_hHintAnim = g_pLTServer->GetModelAnimation(m_hObject);

			if(m_bClearMovementHint)
			{
				m_tfLastMovementHint = *(LTransform*)pData;
				m_bClearMovementHint = LTFALSE;
			}
			else {
				LTransform tfNew;
				g_pTransLT->Multiply(tfNew, m_tfLastMovementHint, *(LTransform*)pData);
				m_tfLastMovementHint = tfNew;
			}

			if( m_bTimeToUpdate )
			{
				Update();
				m_bTimeToUpdate = LTFALSE;
			}
		}
		break;

		case MID_ALLOBJECTSCREATED:
		{
			// Go through the list of specific search items and check to see if any of them are keys we should add...

			const HSTRING hStr = m_pSearch->GetSpecificItem();
			const char *pSearchObj = g_pLTServer->GetStringData( hStr );
			if( pSearchObj )
			{
				ObjArray<HOBJECT, 1> objArray;
				g_pLTServer->FindNamedObjects( pSearchObj, objArray );

				if( objArray.NumObjects() > 0 )
				{
					if( IsKindOf( objArray.GetObject(0), "KeyItem" ))
					{
						KeyItem *pKeyItem = (KeyItem*)g_pLTServer->HandleToObject( objArray.GetObject(0) );
						if( pKeyItem )
						{
							UpdateKeys( ITEM_ADD_ID, pKeyItem->GetKeyId() );
						}
					}
				}
			}

		}
		break;

		case MID_PARENTATTACHMENTREMOVED:
		{
			if( m_bBeingCarried )
			{
				// If we are being carried don't let CCharacter process this message since it will remove us.
				// We know our parent attachment is a Player and they will drop us when they get removed.

				return 1;
			}
		}
		break;

		default:
		{
		}
		break;
	}

	return CCharacter::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CAI::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	int nResult = CCharacter::ObjectMessageFn(hSender, pMsg);

	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			if ( !Camera::IsActive() )
			{
				DamageStruct damage;
				damage.InitFromMessage(pMsg);

				HandleDamage(damage);

				// Register AllyPainSound stimulus.
				if (m_fLastPainVolume > 0.0f)
				{
					LTVector vPainPos;
					g_pLTServer->GetObjectPos(m_hObject, &vPainPos);
					g_pAIStimulusMgr->RegisterStimulus( kStim_AllyPainSound, m_hObject, vPainPos, m_fLastPainVolume );
				}
			}
		}
		break;

		default : break;
	}

	return nResult;
}


//----------------------------------------------------------------------------
//
//	ROUTINE:	CAI::PreReadProp()
//
//	PURPOSE:	Hacked in PreReadProp so that the CAI class can get the first
//				shot at accessing the ObjectCreateStruct, since the derived
//				class needs a few pointers in CAI initialized first.  This is
//				a dirty, ugly hack which needs to be addressed later by either
//				increasing or decreasing the coupling with the derived class.
//
//----------------------------------------------------------------------------
void CAI::PreReadProp(ObjectCreateStruct* pData)
{
	AIASSERT( g_pLTServer && pData, m_hObject, "Server or ObjectCreateStruct is NULL" );

	// Set the template before calling ReadProp, as the template
	// is used in both the
	GenericProp genProp;
	if (g_pLTServer->GetPropGeneric("ModelTemplate", &genProp) == LT_OK)
	{
		AIASSERT( genProp.m_String[0], m_hObject, "No Model template specified!  Fatal Error." );
		ModelId eModelId = g_pModelButeMgr->GetModelId(genProp.m_String);

		// Set the AIs template
		SetAITemplate( g_pModelButeMgr->GetModelAIName(eModelId) );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::ReadProp(ObjectCreateStruct *pData)
{
	// If we have an attribute template, fill in the info

	const AIBM_Template* const pTemplate = GetAITemplate();

	m_fDefaultHitPts = RAISE_BY_DIFFICULTY(pTemplate->fHitPoints);
	m_fDefaultArmor  = RAISE_BY_DIFFICULTY(pTemplate->fArmor);

	// TOOD: multiply Accuracy/lag by difficulty factor
	m_fAccuracy = pTemplate->fAccuracy;

	m_fAccuracyIncreaseRate = pTemplate->fAccuracyIncreaseRate;
	m_fAccuracyDecreaseRate = pTemplate->fAccuracyDecreaseRate;

	m_fFullAccuracyRadiusSqr = pTemplate->fFullAccuracyRadiusSqr;
	m_fAccuracyMissPerturb = pTemplate->fAccuracyMissPerturb;
	m_fMaxMovementAccuracyPerturb = pTemplate->fMaxMovementAccuracyPerturb;
	m_fMovementAccuracyPerturbDecay = pTemplate->fMovementAccuracyPerturbDecay;

	m_dwBaseValidVolumeTypes = pTemplate->iUseableVolumesMask;
	m_dwCurValidVolumeTypes = pTemplate->iUseableVolumesMask;

    LTFLOAT fSndRadius = pTemplate->fSoundRadius;
	m_fSoundRadius = fSndRadius <= 0.0 ? m_fSoundRadius : fSndRadius;

	// Set flags for stimuli that this AI responds to.
	// Add stimuli to AI's SenseRecorder.
	AISenseDistanceMap::const_iterator it;
	EnumAISenseType eSenseType = kSense_InvalidType;
	for(uint8 nSenseType = 0; nSenseType < kSense_Count; ++nSenseType)
	{
		eSenseType = (EnumAISenseType)(1 << nSenseType);
		it = pTemplate->mapSenseDistance.find(eSenseType);
		if(it != pTemplate->mapSenseDistance.end())
		{
			m_flagsBaseSenses |= eSenseType;
			m_pAISenseRecorder->AddSense(eSenseType, it->second);
		}
	}

	// Distance up and down an AI checks for volumes
	m_flVerticalThreshold = pTemplate->fVerticalThreshold;

	// BodySkin extension

    if ( g_pLTServer->GetPropGeneric( "BodySkinExtension", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
            m_hstrBodySkinExtension = g_pLTServer->CreateString( g_gp.m_String );

    if ( g_pLTServer->GetPropGeneric( "IsCinematicAI", &g_gp ) == LT_OK )
	{
		m_bIsCinematicAI = g_gp.m_Bool;
	}
    if ( g_pLTServer->GetPropGeneric( "CanTalk", &g_gp ) == LT_OK )
	{
		m_bCanTalk = g_gp.m_Bool;
	}

	// Commands

    if ( g_pLTServer->GetPropGeneric( "Initial", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
            m_hstrCmdInitial = g_pLTServer->CreateString( g_gp.m_String );

    if ( g_pLTServer->GetPropGeneric( "ActivateOn", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
            m_hstrCmdActivateOn = g_pLTServer->CreateString( g_gp.m_String );

    if ( g_pLTServer->GetPropGeneric( "ActivateOff", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
            m_hstrCmdActivateOff = g_pLTServer->CreateString( g_gp.m_String );

    if ( g_pLTServer->GetPropGeneric( "OnMarkingCommand", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
            m_hstrCmdOnMarking = g_pLTServer->CreateString( g_gp.m_String );

    if ( g_pLTServer->GetPropGeneric( "ProximityGoalCommand", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
            m_hstrCmdProximityGoal = g_pLTServer->CreateString( g_gp.m_String );

	// Overrides

    if ( g_pLTServer->GetPropGeneric("Senses", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_bSensesOn = g_gp.m_Bool;

    if ( g_pLTServer->GetPropGeneric("SoundRadius", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fSoundRadius = g_gp.m_Float;

    if ( g_pLTServer->GetPropGeneric("HitPoints", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fDefaultHitPts = RAISE_BY_DIFFICULTY(g_gp.m_Float);

    if ( g_pLTServer->GetPropGeneric("Armor", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fDefaultArmor = RAISE_BY_DIFFICULTY(g_gp.m_Float);

    if ( g_pLTServer->GetPropGeneric("Accuracy", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fAccuracy = g_gp.m_Float;

    if ( g_pLTServer->GetPropGeneric("AccuracyIncreaseRate", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )

			m_fAccuracyIncreaseRate = g_gp.m_Float;

    if ( g_pLTServer->GetPropGeneric("AccuracyDecreaseRate", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fAccuracyDecreaseRate = g_gp.m_Float;

    if ( g_pLTServer->GetPropGeneric("FullAccuracyRadius", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
		{
			m_fFullAccuracyRadiusSqr = g_gp.m_Float;
			m_fFullAccuracyRadiusSqr *= m_fFullAccuracyRadiusSqr;
		}

    if ( g_pLTServer->GetPropGeneric("AccuracyMissPerturb", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fAccuracyMissPerturb = g_gp.m_Float;

    if ( g_pLTServer->GetPropGeneric("MaxMovementAccuracyPerturb", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fMaxMovementAccuracyPerturb = g_gp.m_Float;

    if ( g_pLTServer->GetPropGeneric("MovementAccuracyPerturbTime", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
		{
			m_fMovementAccuracyPerturbDecay = m_fMaxMovementAccuracyPerturb / g_gp.m_Float;
		}

	// Sense overrides.

	char szTemp[64];
	for( uint32 iSense=0; iSense < kSense_Count; ++iSense )
	{
		eSenseType = (EnumAISenseType)(1 << iSense);

		sprintf( szTemp, "Can%s", s_aszSenseTypes[iSense] );
		if( g_pLTServer->GetPropGeneric( szTemp, &g_gp ) == LT_OK )
		{
			if( g_gp.m_String[0] )
			{
				if( IsTrueChar( g_gp.m_String[0] ) &&
					( !m_pAISenseRecorder->HasSense( eSenseType ) ) )
				{
					m_flagsBaseSenses |= eSenseType;
					m_pAISenseRecorder->AddSense( eSenseType, 0.f );
				}
				else if( IsFalseChar( g_gp.m_String[0] ) &&
					( m_pAISenseRecorder->HasSense( eSenseType ) ) )
				{
					m_flagsBaseSenses &= ~eSenseType;
					m_pAISenseRecorder->RemoveSense( eSenseType );
				}
			}
		}

		sprintf( szTemp, "%sDistance", s_aszSenseTypes[iSense] );
		if( g_pLTServer->GetPropGeneric( szTemp, &g_gp ) == LT_OK )
		{
			if( g_gp.m_String[0] && m_pAISenseRecorder->HasSense( eSenseType ) )
			{
				m_pAISenseRecorder->SetSenseDistance( eSenseType, (LTFLOAT)atof( g_gp.m_String ) );
			}
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PostPropRead()
//
//	PURPOSE:	Initialize model
//
// ----------------------------------------------------------------------- //

void CAI::PostPropRead(ObjectCreateStruct *pStruct)
{
    if (!g_pLTServer || !pStruct) return;

    const char* pFilename   = g_pModelButeMgr->GetModelFilename(m_eModelId);

	if (pFilename && pFilename[0])
	{
		SAFE_STRCPY(pStruct->m_Filename, pFilename);
	}


	g_pModelButeMgr->CopySkinFilenames(m_eModelId, 0, pStruct->m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
	g_pModelButeMgr->CopyRenderStyleFilenames(m_eModelId, pStruct);


	// Add all our editables

    m_editable.AddFloatProp("SoundRadius",  &m_fSoundRadius);

	// Set the time we were born as the last time we relaxed.

	m_fLastRelaxedTime = g_pLTServer->GetTime();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::InitialUpdate()
//
//	PURPOSE:	Initialize the AI routines
//
// ----------------------------------------------------------------------- //
void CAI::InitialUpdate()
{
    if (!m_hObject) return;

    m_damage.SetApplyDamagePhysics(LTFALSE);

	ObjectCreateStruct createstruct;
	createstruct.Clear();

    const char* pFilename = g_pModelButeMgr->GetModelFilename(m_eModelId);
	SAFE_STRCPY(createstruct.m_Filename, pFilename);

	// load up the createstruct with the info from bute file.
	g_pModelButeMgr->CopySkinFilenames(m_eModelId, 0, createstruct.m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
	// load up the createstruct with the info from bute file.
	g_pModelButeMgr->CopyRenderStyleFilenames(m_eModelId, &createstruct);

	// fix the skinnames for body texturemaps.
	if( m_hstrBodySkinExtension &&
		(strlen(g_pLTServer->GetStringData(m_hstrBodySkinExtension)) &&
		strlen(createstruct.m_SkinNames[BODY_SKIN_INDEX])) )
	{
		// insert the value into into pStruct.
		// get the head ext value
		const char *ext_val = g_pLTServer->GetStringData(m_hstrBodySkinExtension) ;

		if( !_stricmp( ext_val, SKIN_EXT_RANDOM ))
		{
			// Randomly use one of the 'AltBodySkin#' for the model...

			uint8 nSkins = g_pModelButeMgr->GetNumAltBodySkins( m_eModelId );
			if( nSkins > 0 )
			{
				const char *pSkin = g_pModelButeMgr->GetAltBodySkin( m_eModelId, GetRandom( 0, nSkins - 1 ));
				if( pSkin )
				{
					SAFE_STRCPY( createstruct.m_SkinNames[BODY_SKIN_INDEX], pSkin );
				}
			}
		}
		else
		{
			// back search for the extention indicator
			const char *szExt   = strrchr(createstruct.m_SkinNames[BODY_SKIN_INDEX],'.');
			// get the position of the extention
			uint32 pos = szExt - (char*)(createstruct.m_SkinNames[BODY_SKIN_INDEX]) + 1 ;

			// add the headextension.
			SAFE_STRCPY( createstruct.m_SkinNames[BODY_SKIN_INDEX] +pos -1, ext_val);
			// put back the file extension.
			SAFE_STRCPY( (createstruct.m_SkinNames[BODY_SKIN_INDEX]) + pos + (strlen(ext_val)-1), ".dtx");
		}
	}

	// fix the skinnames if head extension or body extension have values.
	if( m_hstrHeadExtension &&
		(strlen(g_pLTServer->GetStringData(m_hstrHeadExtension)) &&
		strlen(createstruct.m_SkinNames[HEAD_SKIN_INDEX])) )
	{
		// insert the value into into pStruct.
		// get the head ext value
		const char *ext_val = g_pLTServer->GetStringData(m_hstrHeadExtension) ;

		if( !_stricmp( ext_val, SKIN_EXT_RANDOM ))
		{
			// Randomly use one of the 'AltHeadSkin#' for the model...

			uint8 nSkins = g_pModelButeMgr->GetNumAltHeadSkins( m_eModelId );
			if( nSkins > 0 )
			{
				const char *pSkin = g_pModelButeMgr->GetAltHeadSkin( m_eModelId, GetRandom( 0, nSkins - 1 ));
				if( pSkin )
				{
					SAFE_STRCPY( createstruct.m_SkinNames[HEAD_SKIN_INDEX], pSkin );
				}
			}
		}
		else
		{
			// back search for the extention indicator
			const char *szExt   = strrchr(createstruct.m_SkinNames[HEAD_SKIN_INDEX],'.');
			// get the position of the extention
			uint32 pos = szExt - (char*)(createstruct.m_SkinNames[HEAD_SKIN_INDEX]) + 1 ;

			// add the headextension.
			SAFE_STRCPY( createstruct.m_SkinNames[HEAD_SKIN_INDEX] +pos -1, ext_val);
			// put back the file extension.
			SAFE_STRCPY( (createstruct.m_SkinNames[HEAD_SKIN_INDEX]) + pos + (strlen(ext_val)-1), ".dtx");
		}
	}


	// add child models but since we are not guarenteed that the client side object
	// is created yet, we're going to put the new child models on the objectcreatestruct.
	// also we are going to cache our new file...
	for( int cm_cnt = 0 ; cm_cnt < m_nExtraChildModels ; cm_cnt++ )
	{
		if( m_hstrExtraChildModels[cm_cnt] != 0 )
		{
			HMODELDB hmodeldb ;
			const char *filename = g_pLTServer->GetStringData(m_hstrExtraChildModels[cm_cnt] );
			// cache the requested child model and get a ref to it
			createstruct.m_Filenames[cm_cnt+1][0] = '\0' ;
			if( g_pModelLT->CacheModelDB( filename, hmodeldb ) == LT_OK )
			{
				SAFE_STRCPY( createstruct.m_Filenames[cm_cnt+1], filename );
			}
		}
	}

	// change object information based on stuff gotten from bute file.
    g_pCommonLT->SetObjectFilenames(m_hObject, &createstruct);


	g_pLTServer->GetObjectRotation(m_hObject, &m_rTargetRot);
	m_vTargetRight = m_rTargetRot.Right();
	m_vTargetUp = m_rTargetRot.Up();
	m_vTargetForward = m_rTargetRot.Forward();

	if (!g_SenseInfoTrack.IsInitted())
	{
        g_SenseInfoTrack.Init(g_pLTServer, "AISenseInfo", LTNULL, 0.0f);
	}

	if (!g_AccuracyInfoTrack.IsInitted())
	{
        g_AccuracyInfoTrack.Init(g_pLTServer, "AIAccuracyInfo", LTNULL, 0.0f);
	}

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_AI_CLIENT_SOLID, USRFLG_AI_CLIENT_SOLID);

	UpdateUserFlagCanActivate();

	if ( m_bCheapMovement )
	{
		m_dwFlags &= ~FLAG_GRAVITY;
		m_dwFlags &= ~FLAG_STAIRSTEP;
		m_dwFlags |= FLAG_GOTHRUWORLD;
		m_dwFlags &= ~FLAG_SOLID;
	}

	// Set up the Character Relation Information --
	//	(use the model name as the key)

	// This includes the RelationData and the RelationSet.  We want
	// to share this with non AI systems, so the mgr is NOT liked to the
	// attribute template.  Scanners (searchlights and cameras) both are
	// to use this system as well as the AI, so the structures should abstract
	// away the implementor by setting up the data driven side as much in the
	// shared data mgr.

	// Reuse the AIButeMgr name for the AI instances (trying to use it as a
	// more consistant 'access key' for the AI.

	AIASSERT( GetRelationMgr() != NULL, m_hObject,  "No RelationMgr" );
	GetRelationMgr()->Init( m_hObject, GetAITemplate()->szAlignment );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CAI::HandleTouch(HOBJECT hObj)
{
	if ( m_pState )
	{
		m_pState->HandleTouch(hObj);
	}
}

// ----------------------------------------------------------------------- //

void CAI::HandleVolumeEnter(AIVolume* pVolume)
{
	super::HandleVolumeEnter(pVolume);

	AITRACE( AIShowPaths, ( m_hObject, "Entered volume: %s", pVolume->GetName() ) );

	if ( m_pState )
	{
		m_pState->HandleVolumeEnter(pVolume);
	}

	m_pGoalMgr->HandleVolumeEnter(pVolume);

	// Keep track of how many AI are in volumes corresponding to light switches.

	if( pVolume && pVolume->GetLightSwitchUseObjectNode() )
	{
		AINode* pLightSwitchNode = (AINode*)g_pLTServer->HandleToObject( pVolume->GetLightSwitchUseObjectNode() );
		if( pLightSwitchNode )
		{
			pLightSwitchNode->Lock( m_hObject );
		}
	}
}

// ----------------------------------------------------------------------- //

void CAI::HandleVolumeExit(AIVolume* pVolume)
{
	super::HandleVolumeExit(pVolume);

	if ( m_pState )
	{
		m_pState->HandleVolumeExit(pVolume);
	}

	m_pGoalMgr->HandleVolumeExit(pVolume);

	// If we just travelled through an JumpUp volume flagged as OnlyJumpDown,
	// clear path knowledge because volumes above JumpUp volume are now unreachable.

	if( pVolume && pVolume->GetVolumeType() == AIVolume::kVolumeType_JumpUp )
	{
		AIVolumeJumpUp* pJumpVol = (AIVolumeJumpUp*)pVolume;
		if( pJumpVol->OnlyJumpDown() )
		{
			m_pPathKnowledgeMgr->ClearPathKnowledge();
		}
	}

	// Keep track of how many AI are in volumes corresponding to light switches.

	if( pVolume && pVolume->GetLightSwitchUseObjectNode() )
	{
		AINode* pLightSwitchNode = (AINode*)g_pLTServer->HandleToObject( pVolume->GetLightSwitchUseObjectNode() );
		if( pLightSwitchNode )
		{
			pLightSwitchNode->Unlock( m_hObject );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::FindFloorHeight
//
//	PURPOSE:	Find height of the floor under AI at some position.
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::FindFloorHeight(const LTVector& vPos, LTFLOAT* pfFloorHeight)
{
	AIASSERT( pfFloorHeight, m_hObject, "CAIHuman::FindFloorHeight: fFloorHeight is NULL" );

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = LTVector(vPos.x, vPos.y + m_vDims.y, vPos.z);
	IQuery.m_To = LTVector(vPos.x, vPos.y - m_vDims.y*10.0f, vPos.z);

	IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	IQuery.m_FilterFn = GroundFilterFn;

	g_cIntersectSegmentCalls++;
    if (g_pLTServer->IntersectSegment(&IQuery, &IInfo) && (IsMainWorld(IInfo.m_hObject) || (OT_WORLDMODEL == GetObjectType(IInfo.m_hObject))))
	{
		*pfFloorHeight = IInfo.m_Point.y + m_vDims.y;

		// [KLS 8/8/02] Set the standing on surface based on whatever we're standing on.
		// This is used in CCharacter::HandleModelString() to determine the stimuli
		// to register for AI walking around...

		m_eStandingOnSurface = GetSurfaceType(IInfo);

		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::*ProcessingStimulus()
//
//	PURPOSE:	Handle stimuli
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::GetDoneProcessingStimuli() const
{
	if( m_pAISenseRecorder )
	{
		return m_pAISenseRecorder->GetDoneProcessingStimuli();
	}

	return LTFALSE;
}

void CAI::SetDoneProcessingStimuli(LTBOOL bDone)
{
	if( m_pAISenseRecorder )
	{
		m_pAISenseRecorder->SetDoneProcessingStimuli( bDone );
	}
}

void CAI::ClearProcessedStimuli()
{
	if( m_pAISenseRecorder )
	{
		m_pAISenseRecorder->ClearProcessedStimuli();
	}
}

LTBOOL CAI::ProcessStimulus(CAIStimulusRecord* pRecord)
{
	if( m_pAISenseRecorder )
	{
		return m_pAISenseRecorder->ProcessStimulus( pRecord );
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::*IntersectSegmentCount()
//
//	PURPOSE:	Handle IntersectSegment Counting.
//
// ----------------------------------------------------------------------- //

int	CAI::GetIntersectSegmentCount() const
{
	if( m_pAISenseRecorder )
	{
		return m_pAISenseRecorder->GetIntersectSegmentCount();
	}

	return 0;
}

void CAI::ClearIntersectSegmentCount()
{
	if( m_pAISenseRecorder )
	{
		m_pAISenseRecorder->ClearIntersectSegmentCount();
	}
}

void CAI::IncrementIntersectSegmentCount()
{
	if( m_pAISenseRecorder )
	{
		m_pAISenseRecorder->IncrementIntersectSegmentCount();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleSense*()
//
//	PURPOSE:	Handle senses
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::HandleSenseRecord(CAIStimulusRecord* pStimulusRecord, uint32 nCycle)
{
	if( m_pAISenseRecorder )
	{
		return m_pAISenseRecorder->UpdateSenseRecord( pStimulusRecord, nCycle );
	}

	return LTFALSE;
}

void CAI::HandleSenses(uint32 nCycle)
{
	if( m_pAISenseRecorder )
	{
		m_pAISenseRecorder->HandleSenses( nCycle );
	}
}

void CAI::HandleSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( m_pGoalMgr )
	{
		m_pGoalMgr->HandleGoalSenseTriggers(pSenseRecord);
	}
}

LTFLOAT CAI::GetSenseDistance(EnumAISenseType eSenseType)
{
	if( m_pAISenseRecorder )
	{
		return m_pAISenseRecorder->GetSenseDistance( eSenseType );
	}

	return 0.f;
}

const RelationSet& CAI::GetSenseRelationSet() const
{
	if( m_pRelationMgr )
	{
		return m_pRelationMgr->GetRelationUser()->GetRelations();
	}

	static RelationSet rs;
	return rs;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::OnTrigger()
//
//	PURPOSE:	Handle a trigger message
//
// ----------------------------------------------------------------------- //

bool CAI::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Activate(c_szActivate);
	static CParsedMsg::CToken s_cTok_Cineract("CINERACT");
	static CParsedMsg::CToken s_cTok_CineractLoop("CINERACTLOOP");
	static CParsedMsg::CToken s_cTok_TrackLookAt("TRACKLOOKAT");
	static CParsedMsg::CToken s_cTok_TrackAimAt("TRACKAIMAT");
	static CParsedMsg::CToken s_cTok_TrackNone("TRACKNONE");
	static CParsedMsg::CToken s_cTok_Alignment("ALIGNMENT");
	static CParsedMsg::CToken s_cTok_DamageMask("DAMAGEMASK");
	static CParsedMsg::CToken s_cTok_IgnoreVolumes("IGNOREVOLUMES");
	static CParsedMsg::CToken s_cTok_WakeUp("WAKEUP");
	static CParsedMsg::CToken s_cTok_CineractAI("CINERACTAI");



	// If we haven't updated then queue our commands for later processing...

	if ( m_bFirstUpdate )
	{
		QueueTriggerMsg( cMsg );

		UpdatePosition();

		return true;
	}



	if( cMsg.GetArg(0) == s_cTok_Activate )
	{
		Activate();
		return true;
	}

	else if( cMsg.GetArg(0) == s_cTok_Cineract )
	{
		if( cMsg.GetArgCount() > 1 )
		{
			Cineract( cMsg.GetArg(1), LTFALSE );
		}
		return true;
	}

	else if( cMsg.GetArg(0) == s_cTok_CineractLoop )
	{
		if( cMsg.GetArgCount() > 1 )
		{
			Cineract( cMsg.GetArg(1), LTTRUE );
		}
		return true;
	}

	else if( cMsg.GetArg(0) == s_cTok_Alignment )
	{
		if( cMsg.GetArgCount() > 1 )
		{
			// Persistent stimuli need to be re-registered with correct alignment.

			RemovePersistentStimuli();

			GetRelationMgr()->ClearRelationSystem();
			GetRelationMgr()->Init( m_hObject, cMsg.GetArg(1) );

			// Invalidate our crosshair.  We'll rediscover our crosshair
			// status in ResetCrosshair.
			m_ccCrosshair = UNKNOWN;
			ResetCrosshair( );

			RegisterPersistentStimuli();
		}
		return true;
	}

	else if( cMsg.GetArg(0) == s_cTok_DamageMask )
	{
		if( cMsg.GetArgCount() > 1 )
		{
			SetDamageMask( cMsg.GetArg(1) );
		}
		return true;
	}

	else if( cMsg.GetArg(0) == s_cTok_IgnoreVolumes )
	{
		if( ( cMsg.GetArgCount() > 1 ) &&
			( m_pAIMovement ) )
		{
			m_pAIMovement->IgnoreVolumes( IsTrueChar( cMsg.GetArg(1)[0] ) );
		}
		return true;
	}

	else if( cMsg.GetArg(0) == s_cTok_TrackLookAt )
	{
		CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
		if ( pPlayer )
		{
			m_pTrackedNodeContext->SetTrackedTarget( kTrack_LookAt, pPlayer->m_hObject, "Head", LTVector(0.f, 0.f, 0.f) );
			m_pTrackedNodeContext->SetActiveTrackingGroup( kTrack_LookAt );
			m_eTriggerNodeTrackingGroup = kTrack_LookAt;
		}
		return true;
	}

	else if( cMsg.GetArg(0) == s_cTok_TrackAimAt )
	{
		CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
		if ( pPlayer )
		{
			m_pTrackedNodeContext->SetTrackedTarget( kTrack_AimAt, pPlayer->m_hObject, "Head", LTVector(0.f, 0.f, 0.f) );
			m_pTrackedNodeContext->SetActiveTrackingGroup( kTrack_AimAt );
			m_eTriggerNodeTrackingGroup = kTrack_AimAt;
		}
		return true;
	}

	else if( cMsg.GetArg(0) == s_cTok_TrackNone )
	{
		m_pTrackedNodeContext->SetActiveTrackingGroup( kTrack_None );
		m_eTriggerNodeTrackingGroup = kTrack_None;
		return true;
	}

	else if( cMsg.GetArg(0) == s_cTok_WakeUp )
	{
		if( IsUnconscious() )
		{
			CAIGoalSpecialDamage* pGoal = dynamic_cast<CAIGoalSpecialDamage*>(m_pGoalMgr->FindGoalByType( kGoal_SpecialDamage ));
			if( pGoal && m_pGoalMgr->IsCurGoal( pGoal ))
			{
				pGoal->InterruptSpecialDamage( LTFALSE );
			}
		}
	}

	else if( cMsg.GetArg(0) == s_cTok_CineractAI )
	{
		if( ( cMsg.GetArgCount() > 1 ) )
		{
			m_bIsCinematicAI = (IsTrueChar( cMsg.GetArg(1)[0] ) ? true : false);

			// Update client with new info (easiest to do this way and only
			// happens during cinematics, so no worries about network overhead)...
			CreateSpecialFX(LTTRUE);
		}
		return true;
	}

	// Wake us back up if we get a message

///	g_pLTServer->SetNextUpdate(m_hObject, c_fUpdateDelta);

	// Let character have a crack at it now

	if ( CCharacter::OnTrigger(hSender, cMsg) )
	{
        return true;
	}

	return HandleCommand(cMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Activate
//
//	PURPOSE:	Play a cineractive animation.
//
// ----------------------------------------------------------------------- //

void CAI::Activate()
{
	if (m_bCanTalk &&  (m_bAlwaysActivate || (m_pState && m_pState->CanActivate())) )
	{
		m_bActivated = !m_bActivated;

		if ( m_bActivated && m_hstrCmdActivateOn )
		{
			SendMixedTriggerMsgToObject(this, m_hObject, m_hstrCmdActivateOn);

			if ( !m_bPreserveActiveCmds )
			{
				FREE_HSTRING(m_hstrCmdActivateOn);
			}

			UpdateUserFlagCanActivate();
		}
		else if ( !m_bActivated && m_hstrCmdActivateOff )
		{
			SendMixedTriggerMsgToObject(this, m_hObject, m_hstrCmdActivateOff);

			if ( !m_bPreserveActiveCmds )
			{
				FREE_HSTRING(m_hstrCmdActivateOff);
			}

			UpdateUserFlagCanActivate();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Cineract
//
//	PURPOSE:	Play a cineractive animation.
//
// ----------------------------------------------------------------------- //

void CAI::Cineract(const char* szAnim, LTBOOL bLoop)
{
	if( !m_pAnimationContext )
		return;

	m_pAnimationContext->SetSpecial( szAnim );

	if( bLoop )
	{
		m_pAnimationContext->LoopSpecial();
	}
	else {
		m_pAnimationContext->PlaySpecial();
	}

	m_pAnimationContext->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HideCharacter
//
//	PURPOSE:	Hide/Show character.
//
// ----------------------------------------------------------------------- //

void CAI::HideCharacter(LTBOOL bHide)
{
	super::HideCharacter( bHide );

	// If we're being tracked, make sure our blip hides with us.
	SetTracking( !bHide && m_bStuckWithTrackDart );

	if( bHide )
	{
		SetClientSolid( LTFALSE );
	}
	else
	{
		SetClientSolid( LTTRUE );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateUserFlagCanActivate()
//
//	PURPOSE:	Updates our CAN_ACTIVATE user flag.
//
// ----------------------------------------------------------------------- //

void CAI::UpdateUserFlagCanActivate()
{
	if (m_bCanTalk && !m_bUnconscious && (m_hstrCmdActivateOff || m_hstrCmdActivateOn ))
	{
		if ( m_bAlwaysActivate || (m_pState && m_pState->CanActivate()) )
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);

			return;
		}
	}

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetUnconscious()
//
//	PURPOSE:	Updates flags for sleeping/waking AI
//
// ----------------------------------------------------------------------- //

void CAI::SetUnconscious(bool bUnconscious)
{

	if (bUnconscious == m_bUnconscious) return;

	m_bUnconscious = bUnconscious;

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, (bUnconscious ? USRFLG_CAN_SEARCH : 0), USRFLG_CAN_SEARCH);

	if (bUnconscious)
	{
		if (m_hHitBox)
		{
			CCharacterHitBox* pHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(m_hHitBox);
			if ( pHitBox )
			{
				LTVector vNewDims,vDims;
				pHitBox->GetDefaultModelDims( vDims );

				vNewDims = vDims;
				if (GetAnimationContext()->IsPropSet(kAPG_Posture,kAP_Stand))
				{
					//if we were standing, adjust our hitbox to cover us lying down

					vNewDims.x *= 2.0f;
					vNewDims.z *= 2.0f;
					vNewDims.y = 15.0f;
					g_pPhysicsLT->SetObjectDims(m_hHitBox, &vNewDims, 0);
				}

				LTVector vOffset(0, vNewDims.y - vDims.y, 0);
				pHitBox->SetOffset(vOffset);
				pHitBox->SetCanBeSearched(LTTRUE);
				pHitBox->Update();
			}

			UpdateClientHitBox();
		}

		//spawn pickups...
		if ( m_pAttachments )
		{
			CWeapon* apWeapons[kMaxWeapons];
			CAttachmentPosition* apAttachmentPositions[kMaxWeapons];
			uint32 cWeapons = m_pAttachments->EnumerateWeapons(apWeapons, apAttachmentPositions, kMaxWeapons);

			while ( cWeapons )
			{
				char szSpawn[1024];
				cWeapons--;

				int nId = apWeapons[cWeapons]->GetId();
				if (!g_pWeaponMgr->IsPlayerWeapon(nId))
					continue;


				HOBJECT hWpnModel = apWeapons[cWeapons]->GetModelObject();
				uint32 dwAni = g_pLTServer->GetModelAnimation(hWpnModel);

				apAttachmentPositions[cWeapons]->GetAttachment()->CreateSpawnString(szSpawn);

				BaseClass* pObj = SpawnObject(szSpawn, LTVector(0,0,0), LTRotation());

				if ( pObj && pObj->m_hObject )
				{
			 		if (dwAni != INVALID_ANI)
					{
						LTVector vDims;
						g_pCommonLT->GetModelAnimUserDims(pObj->m_hObject, &vDims, dwAni);
						g_pPhysicsLT->SetObjectDims(pObj->m_hObject, &vDims, 0);

						g_pLTServer->SetModelAnimation(pObj->m_hObject, dwAni);
					}

					apAttachmentPositions[cWeapons]->GetAttachment()->CreateAttachString(apAttachmentPositions[cWeapons],szSpawn);

					//remove associated attachment
					m_pAttachments->Detach(apAttachmentPositions[cWeapons]->GetName());
					HandleDetach();

					//attach pickup item
					HATTACHMENT hAttachment;
					if ( LT_OK != g_pLTServer->CreateAttachment(m_hObject, pObj->m_hObject, (char *)apAttachmentPositions[cWeapons]->GetName(), &LTVector(0,0,0), &LTRotation(), &hAttachment) )
					{
						g_pLTServer->RemoveObject(pObj->m_hObject);
					}
					else
					{
						m_aDroppedWeapons[m_cDroppedWeapons].sAttachString = szSpawn;
						m_aDroppedWeapons[m_cDroppedWeapons].hPickupItem = pObj->m_hObject;
						m_cDroppedWeapons++;

						//force pickup is set to true here so that you may disarm unconscious AI
						m_pSearch->AddPickupItem(pObj->m_hObject,true);
						g_pCommonLT->SetObjectFlags(pObj->m_hObject, OFT_User, USRFLG_ATTACH_HIDE1SHOW3, USRFLG_CAN_ACTIVATE | USRFLG_ATTACH_HIDE1SHOW3);
					}

				}
			}

			memset(m_apWeapons, LTNULL, sizeof(CWeapon*)*AI_MAX_WEAPONS);
			memset(m_apWeaponPositions, LTNULL, sizeof(CAttachmentPosition*)*AI_MAX_WEAPONS);
			m_cWeapons = m_pAttachments->EnumerateWeapons(m_apWeapons, m_apWeaponPositions, AI_MAX_WEAPONS);

			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);
		}

		// Add our spears to the searchobject...

		for( uint32 iSpear = 0 ; iSpear < m_cSpears ; ++iSpear )
		{
			HOBJECT hSpear = m_aSpears[iSpear].hObject;

			if( hSpear )
			{
				m_pSearch->AddPickupItem( hSpear );
			}
		}

	}
	else
	{
		if (m_hHitBox)
		{
			CCharacterHitBox* pHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(m_hHitBox);
			if ( pHitBox )
			{
				LTVector vDims;
				pHitBox->GetDefaultModelDims( vDims );

				LTVector vHitDims = vDims;
				vHitDims *= 1.2f;
				g_pPhysicsLT->SetObjectDims(m_hHitBox, &vHitDims, 0);

				pHitBox->SetOffset( LTVector( 0.0f, 0.0f, 0.0f ) );
				pHitBox->SetCanBeSearched(LTFALSE);
				pHitBox->Update();
			}

			UpdateClientHitBox();
		}

		UpdateUserFlagCanActivate();

		//destroy any uncollected pickups and show their weapons...
		//remove any weapons associated with collected pickups...
		for ( int iWpn = 0 ; iWpn < m_cDroppedWeapons ; iWpn++ )
		{
			if (NULL != (HOBJECT)m_aDroppedWeapons[iWpn].hPickupItem)
			{
				HATTACHMENT hAttachment;
				if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, m_aDroppedWeapons[iWpn].hPickupItem, &hAttachment) )
				{
					g_pLTServer->RemoveAttachment(hAttachment);
				}
				g_pLTServer->RemoveObject(m_aDroppedWeapons[iWpn].hPickupItem);

				m_aDroppedWeapons[iWpn].hPickupItem = LTNULL;

				SendTriggerMsgToObject( this, m_hObject, LTFALSE, m_aDroppedWeapons[iWpn].sAttachString );
			}

			m_aDroppedWeapons[iWpn].Clear();

		}

		m_cDroppedWeapons = 0;

		// Remove our spears from the search object. Do this before ClearPickupItems() since that will
		// remove the object from the world but we want our spears to remain when we wake up...

		for( uint32 iSpear = 0 ; iSpear < m_cSpears ; ++iSpear )
		{
			HOBJECT hSpear = m_aSpears[iSpear].hObject;

			if( hSpear )
			{
				m_pSearch->RemovePickupItem( hSpear );
			}
		}

		m_pSearch->ClearPickupItems();

	}

	m_pSearch->Enable(bUnconscious & m_pSearch->HasItem());

	SetCanCarry( m_bUnconscious );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::OnLinkBroken
//
//	PURPOSE:	Add to/Remove from/Clear the key list
//
// ----------------------------------------------------------------------- //

void CAI::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	for ( int iWpn = 0 ; iWpn < m_cDroppedWeapons ; iWpn++ )
	{
		if ( &m_aDroppedWeapons[iWpn].hPickupItem == pRef )
		{
			HATTACHMENT hAttachment;
			if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hObj, &hAttachment) )
			{
				g_pLTServer->RemoveAttachment(hAttachment);
			}

			m_aDroppedWeapons[iWpn].hPickupItem = LTNULL;
		}
	}
	super::OnLinkBroken( pRef, hObj );

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleDamage()
//
//	PURPOSE:	Notification that we are hit by something
//
// ----------------------------------------------------------------------- //

void CAI::HandleDamage(const DamageStruct& damage)
{
	if ( m_damage.IsCantDamageType(damage.eType) || !m_damage.GetCanDamage() ) return;

	if ( !m_damage.IsDead() )
	{
		// Do not play pain sounds if affected by special damage.

		if( ( damage.fDamage > 0.0f ) &&
			( !GetDamageFlags() ) )
		{
			// TODO: all the time?
			PlaySound( kAIS_Pain, LTTRUE );
		}

		if ( m_pState )
		{
			m_pState->HandleDamage(damage);
		}
	}
	else
	{
		AITRACE( AIShowStates, ( m_hObject, "AI is Dead" ) );

		g_pMusicMgr->DoEvent(CMusicMgr::eEventAIDie);

		// Clean up the armor FX/attachments if they are still running.
		DestroyArmor();

		m_cWeapons = 0;
		memset(m_apWeapons, LTNULL, sizeof(CWeapon*)*AI_MAX_WEAPONS);
		memset(m_apWeaponPositions, LTNULL, sizeof(CAttachmentPosition*)*AI_MAX_WEAPONS);

		m_cObjects = 0;
		memset(m_apObjects, LTNULL, sizeof(BaseClass*)*AI_MAX_OBJECTS);
		memset(m_apObjectPositions, LTNULL, sizeof(CAttachmentPosition*)*AI_MAX_OBJECTS);

		if ( IsControlledByDialogue() )
		{
			StopDialogue();
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::QueueTriggerMsg()
//
//	PURPOSE:	Stores a command on the command queue
//
// --------------------------------------------------------------------------- //

void CAI::QueueTriggerMsg(const CParsedMsg &cMsg)
{
	// If this isn't the first command in the queue, prefix it with a separator

	if ( !m_sQueuedCommands.IsEmpty( ))
	{
		m_sQueuedCommands += ";";
	}

	// Re-build the message
	for ( uint32 nCurArg = 0; nCurArg < cMsg.GetArgCount(); ++nCurArg )
	{
		if ( nCurArg > 0 )
			m_sQueuedCommands += " ";
		m_sQueuedCommands += cMsg.GetArg(nCurArg).c_str();
	}

	AIASSERT( m_sQueuedCommands.GetLength( ) < 2048, m_hObject, "CAI::QueueTriggerMsg: Trigger Msg > 2048 characters!" );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::QueueTriggerMsg()
//
//	PURPOSE:	Stores a command on the command queue
//
// --------------------------------------------------------------------------- //

void CAI::QueueTriggerMsg(const char* szMsg)
{
	// If this isn't the first command in the queue, prefix it with a separator

	if ( !m_sQueuedCommands.IsEmpty( ))
	{
		m_sQueuedCommands += ";";
	}

	// Re-build the message

	m_sQueuedCommands += szMsg;

	AIASSERT( m_sQueuedCommands.GetLength( ) < 2048, m_hObject, "CAI::QueueTriggerMsg: Trigger Msg > 2048 characters!" );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleCommand()
//
//	PURPOSE:	Handles a command
//
// --------------------------------------------------------------------------- //

bool CAI::HandleCommand(const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Gravity("GRAVITY");
	static CParsedMsg::CToken s_cTok_Senses("SENSES");
	static CParsedMsg::CToken s_cTok_AlwaysActivate("ALWAYSACTIVATE");
	static CParsedMsg::CToken s_cTok_CanTalk("CANTALK");
	static CParsedMsg::CToken s_cTok_Debug("DEBUG");
	static CParsedMsg::CToken s_cTok_Move("MOVE");
	static CParsedMsg::CToken s_cTok_ShootThrough("SHOOTTHROUGH");
	static CParsedMsg::CToken s_cTok_SeeThrough("SEETHROUGH");
	static CParsedMsg::CToken s_cTok_FOVBias("FOVBIAS");
	static CParsedMsg::CToken s_cTok_PlaySound("PLAYSOUND");
	static CParsedMsg::CToken s_cTok_ATP("ATP");
	static CParsedMsg::CToken s_cTok_FaceObject("FACEOBJECT");
	static CParsedMsg::CToken s_cTok_FacePos("FACEPOS");
	static CParsedMsg::CToken s_cTok_FaceDir("FACEDIR");
	static CParsedMsg::CToken s_cTok_FaceTarget("FACETARGET");
	static CParsedMsg::CToken s_cTok_Target("TARGET");
	static CParsedMsg::CToken s_cTok_TargetPlayer("TARGETPLAYER");
	static CParsedMsg::CToken s_cTok_Ping("PING");
	static CParsedMsg::CToken s_cTok_PreserveActivateCmds("PRESERVEACTIVATECMDS");
	static CParsedMsg::CToken s_cTok_DelCmd("DELCMD");
	static CParsedMsg::CToken s_cTok_ActivateOn("ACTIVATEON");
	static CParsedMsg::CToken s_cTok_ActivateOff("ACTIVATEOFF");
	static CParsedMsg::CToken s_cTok_HealthMod("HEALTHMOD");
	static CParsedMsg::CToken s_cTok_ArmorMod("ARMORMOD");

	// Let the goals have a whack at it.

	if( m_pGoalMgr->HandleCommand(cMsg) )
	{
		return true;
	}

	// Let the state have a whack at it

	if ( m_pState )
	{
		if ( m_pState->HandleCommand(cMsg) )
		{
            return true;
		}
	}

	if ( cMsg.GetArg(0) == s_cTok_Gravity )
	{
		_ASSERT(cMsg.GetArgCount() > 1);

		if ( cMsg.GetArgCount() > 1 )
		{
			bool bGravity;

			if ( IsTrueChar(*cMsg.GetArg(1)) )
			{
				bGravity = true;
			}
			else
			{
                g_pPhysicsLT->SetVelocity(m_hObject, &LTVector(0,0,0));

				bGravity = false;
			}

			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, bGravity ? FLAG_GRAVITY : 0, FLAG_GRAVITY);
		}

        return true;
	}
	if ( cMsg.GetArg(0) == s_cTok_Senses )
	{
		// Turn on/off senses.

		m_bSensesOn = IsTrueChar(*cMsg.GetArg(1));
		return true;
	}
	if ( cMsg.GetArg(0) == s_cTok_AlwaysActivate )
	{
		m_bAlwaysActivate = IsTrueChar(*cMsg.GetArg(1));
		UpdateUserFlagCanActivate();
	}
	if ( cMsg.GetArg(0) == s_cTok_CanTalk )
	{
		m_bCanTalk = !!IsTrueChar(*cMsg.GetArg(1));
		//verify that we have stuff to say
		if (m_bCanTalk && !m_hstrCmdActivateOff && !m_hstrCmdActivateOn )
		{
			g_pLTServer->CPrint("%s set to CANTALK, but has no Activate commands",GetName());
		}
		UpdateUserFlagCanActivate();
	}
	if ( cMsg.GetArg(0) == s_cTok_Debug )
	{
		m_nDebugLevel = (uint32)atoi(cMsg.GetArg(1));
	}
	if ( cMsg.GetArg(0) == s_cTok_Move )
	{
        LTVector vPos;
		sscanf(cMsg.GetArg(1), "%f,%f,%f", &vPos.x, &vPos.y, &vPos.z);
		vPos += m_vPos;

        g_pLTServer->MoveObject(m_hObject, &vPos);
        return true;
	}
	if ( cMsg.GetArg(0) == s_cTok_ShootThrough )
	{
		_ASSERT(cMsg.GetArgCount() > 1);

		if ( cMsg.GetArgCount() > 1 )
		{
			m_bShootThrough = IsTrueChar(*cMsg.GetArg(1));

            return true;
		}
		else
		{
            g_pLTServer->CPrint("SHOOTTHROUGH missing argument");
		}
	}
	if ( cMsg.GetArg(0) == s_cTok_SeeThrough )
	{
		_ASSERT(cMsg.GetArgCount() > 1);

		if ( cMsg.GetArgCount() > 1 )
		{
			m_bSeeThrough = IsTrueChar(*cMsg.GetArg(1));

            return true;
		}
		else
		{
            g_pLTServer->CPrint("SEETHROUGH missing argument");
		}
	}
	if ( cMsg.GetArg(0) == s_cTok_FOVBias )
	{
		_ASSERT(cMsg.GetArgCount() > 1);

		if ( cMsg.GetArgCount() > 1 )
		{
			m_fFOVBias = FOV2DP((LTFLOAT)atof(cMsg.GetArg(1)));

            return true;
		}
		else
		{
            g_pLTServer->CPrint("FOVBIAS missing argument");
		}
	}
	if ( cMsg.GetArg(0) == s_cTok_PlaySound )
	{
		_ASSERT(cMsg.GetArgCount() > 1);

		if ( cMsg.GetArgCount() > 1 )
		{
			PlayDialogSound(cMsg.GetArg(1));
		}
		else
		{
            g_pLTServer->CPrint("PLAYSOUND missing argument");
		}

        return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_ATP )
	{
        SendTriggerMsgToObject(this, m_hObject, LTFALSE, "TARGETPLAYER;ATTACK");
        return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_FaceObject )
	{
		HOBJECT hObject;
		if ( LT_OK == FindNamedObject(cMsg.GetArg(1), hObject) )
		{
			FaceObject(hObject);
		}

        return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_FacePos )
	{
        LTVector vPos;
		sscanf(cMsg.GetArg(1), "%f,%f,%f", &vPos.x, &vPos.y, &vPos.z);
		FacePos(vPos);

        return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_FaceDir )
	{
        LTVector vDir;
		sscanf(cMsg.GetArg(1), "%f,%f,%f", &vDir.x, &vDir.y, &vDir.z);
		FaceDir(vDir);

        return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_FaceTarget )
	{
		FaceTarget();

        return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_Target )
	{
		_ASSERT(cMsg.GetArgCount() > 1);
		if ( cMsg.GetArgCount() == 1 )
		{
            g_pLTServer->CPrint("TARGET missing argument");
		}
		else
		{
			HOBJECT hObject;
			if ( LT_OK == FindNamedObject(cMsg.GetArg(1), hObject) )
			{
				Target(hObject);
			}
		}

        return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_TargetPlayer )
	{
		CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
		if ( pPlayer )
		{
			Target(pPlayer->m_hObject);
		}

        return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_Ping )
	{
		return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_PreserveActivateCmds )
	{
		m_bPreserveActiveCmds = LTTRUE;
	}
	else if ( cMsg.GetArg(0) == s_cTok_DelCmd )
	{
		_ASSERT(cMsg.GetArgCount() > 1);
		if ( cMsg.GetArgCount() == 1 )
		{
            g_pLTServer->CPrint("DELCMD missing argument");
		}
		else
		{
			if ( cMsg.GetArg(1) == s_cTok_ActivateOn )
			{
				FREE_HSTRING(m_hstrCmdActivateOn);
				UpdateUserFlagCanActivate();
			}
			else if ( cMsg.GetArg(1) == s_cTok_ActivateOff )
			{
				FREE_HSTRING(m_hstrCmdActivateOff);
				UpdateUserFlagCanActivate();
			}
			else
			{
	            g_pLTServer->CPrint("DELCMD %s - invalid command to delete", cMsg.GetArg(1).c_str());
			}
		}

        return true;
	}
	else if( cMsg.GetArg(0) == s_cTok_HealthMod )
	{
		float fMod = (float)atof( cMsg.GetArg(1).c_str() );
		SetHitPointsMod( fMod );
	}
	else if( cMsg.GetArg(0) == s_cTok_ArmorMod )
	{
		float fMod = (float)atof( cMsg.GetArg(1).c_str() );
		SetArmorPointsMod( fMod );
	}
	else
	{
		return false;
	}

    return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ProcessCommandParameters()
//
//	PURPOSE:	Process command parameters
//
// --------------------------------------------------------------------------- //

void CAI::HandleCommandParameters(const CParsedMsg &cMsg)
{
	uint iToken = 1;
	while ( iToken < cMsg.GetArgCount() )
	{
		char szName[64];
		char szValue[256];

		const char *pEqual = strchr(cMsg.GetArg(iToken), '=');

		if ( !pEqual )
		{
            g_pLTServer->CPrint("Garbage name/value pair = %s", cMsg.GetArg(iToken).c_str());
			iToken++;
			continue;
		}

		strncpy(szName, cMsg.GetArg(iToken), pEqual - cMsg.GetArg(iToken));
		szName[pEqual - cMsg.GetArg(iToken)] = 0;
		strcpy(szValue, pEqual+1);

		if ( m_pState )
		{
			m_pState->HandleNameValuePair(szName, szValue);
		}

		iToken++;
	}
}

// ----------------------------------------------------------------------- //

void CAI::HandleAttach()
{
	InitAttachments();
}

// ----------------------------------------------------------------------- //

void CAI::HandleDetach()
{
	InitAttachments();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleTeleport()
//
//	PURPOSE:	Teleport the ai to the specified position
//
// ----------------------------------------------------------------------- //

void CAI::HandleTeleport(const LTVector& vPos)
{
	g_pLTServer->SetObjectPos(m_hObject, const_cast<LTVector*>(&vPos));
	m_vMovePos = vPos;
	SetPosition( vPos, LTTRUE );

	LTVector vDeditPos = vPos;
	vDeditPos = ConvertToDEditPos( vDeditPos );
	AITRACE( AIShowVolumes, ( m_hObject, "Teleporting to %.2f %.2f %.2f",
		vDeditPos.x, vDeditPos.y, vDeditPos.z ) );

	m_pPathKnowledgeMgr->ClearPathKnowledge();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleTeleport()
//
//	PURPOSE:	Teleport the ai to the specified point
//
// ----------------------------------------------------------------------- //

void CAI::HandleTeleport(TeleportPoint* pTeleportPoint)
{
	// Set our starting values...

    LTVector vPos;
	g_pLTServer->GetObjectPos(pTeleportPoint->m_hObject, &vPos);

    LTRotation rRot;
	g_pLTServer->GetObjectRotation(pTeleportPoint->m_hObject, &rRot);

	HandleTeleport( vPos );
	g_pLTServer->SetObjectRotation(m_hObject, &rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetSpeed
//
//	PURPOSE:	Gets our current movement speed
//
// ----------------------------------------------------------------------- //

LTFLOAT CAI::GetSpeed()
{
	// We basically want to slow down if we're not pointing the way we're moving

	LTFLOAT fModifier = (1.0f + m_vTargetForward.Dot(m_vForward))/2.0f;
	if( fModifier <= 0.f )
	{
		fModifier = 1.f;
	}

	return fModifier*m_fSpeed;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Move()
//
//	PURPOSE:	Sets our new position for this frame
//
// ----------------------------------------------------------------------- //

void CAI::Move(const LTVector& vPos)
{
	m_vMovePos = vPos;
	m_bMove = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsWalkingOrRunning()
//
//	PURPOSE:	Query to determine if an AI is walking or running.
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsWalkingOrRunning()
{
	if( !m_pAnimationContext )
	{
		return LTFALSE;
	}

	// Look at anim props to determine if AI is walking or running.

	if(	( m_pAnimationContext->IsPropSet( kAPG_Movement, kAP_Walk ) ) ||
		( m_pAnimationContext->IsPropSet( kAPG_Movement, kAP_Run ) ) ||
		( m_pAnimationContext->IsPropSet( kAPG_Movement, kAP_BackUp ) ) ||
		( m_pAnimationContext->IsPropSet( kAPG_Evasive, kAP_ShuffleLeft ) ) ||
		( m_pAnimationContext->IsPropSet( kAPG_Evasive, kAP_ShuffleRight ) ) ||
		( m_pAnimationContext->IsPropSet( kAPG_Action, kAP_OpenDoor ) ) ||
		( m_pAnimationContext->IsPropSet( kAPG_Action, kAP_KickDoor ) ) )
	{
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsStanding()
//
//	PURPOSE:	Query to determine if an AI is standing.
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsStanding()
{
	if( !m_pAnimationContext )
	{
		return LTFALSE;
	}

	// Look at anim props to determine if AI is standing...

	if( m_pAnimationContext->IsPropSet( kAPG_Posture, kAP_Stand ))
	{
		return LTTRUE;
	}

	return LTFALSE;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PreUpdate
//
//	PURPOSE:	Does our Preupdate
//
// ----------------------------------------------------------------------- //

void CAI::PreUpdate()
{
	if( m_bFirstUpdate )
	{
		if ( !m_pAnimationContext )
		{
			m_flagsCurSenses = m_flagsBaseSenses;
			m_pAnimationContext = m_pAnimationMgr->CreateAnimationContext(m_hObject);
		}

		// Set up tracking nodes.

		m_pTrackedNodeContext->Init( m_hObject, m_eModelSkeleton, g_pServerTrackedNodeMgr );

		m_cstrNameInfo = GetName();
	}

	// Do any queued commands

	else if ( !m_sQueuedCommands.IsEmpty( ))
	{
        SendMixedTriggerMsgToObject(this, m_hObject, LTTRUE, m_sQueuedCommands);
		m_sQueuedCommands.Empty( );
	}

	// Refresh client's tracked node settings.
	// This only happens after a load.

	if( m_pTrackedNodeContext->NeedToRefreshClient() )
	{
		m_pTrackedNodeContext->UpdateClient();
	}

	// Restore a weapon from backup if it was stolen (while AI was knocked out),
	// and AI has returned to a relaxed state.
	// RestoreBackupWeaponTime is set in SetAwareness().

	if( ( m_fRestoreBackupWeaponTime != 0.f ) &&
		( m_fRestoreBackupWeaponTime < g_pLTServer->GetTime() ) &&
		( m_eAwareness == kAware_Relaxed ) )
	{
		m_fRestoreBackupWeaponTime = 0.f;
		SetHolsterString( GetBackupHolsterString() );
	}

	// AllyDisturbance stimulus.

	if( IsSuspicious() && (m_eAlarmStimID == kStimID_Unset) && m_pState && m_pState->CausesAllyDisturbances() )
	{
		m_eAlarmStimID = g_pAIStimulusMgr->RegisterStimulus( kStim_AllyDisturbanceVisible, m_hObject, LTNULL, CAIStimulusRecord::kDynamicPos_TrackSource, 1.f );
	}
	else if( ( m_eAwareness == kAware_Relaxed ) && ( m_eAlarmStimID != kStimID_Unset ) )
	{
		g_pAIStimulusMgr->RemoveStimulus( m_eAlarmStimID );
		m_eAlarmStimID = kStimID_Unset;

		// Clear target when the AI returns to a relaxed state.

		Target( LTNULL );
	}
}

// ----------------------------------------------------------------------- //

void CAI::InitAttachments()
{
	if ( m_pAttachments->HasWeapons() || (m_cWeapons > 0 && !m_pAttachments->HasWeapons()) )
	{
		// Give the AI lots of ammo.

		m_pAttachments->GetInfiniteAmmo();

		// Record all of our weapons and positions

		memset(m_apWeapons, LTNULL, sizeof(CWeapon*)*AI_MAX_WEAPONS);
		memset(m_apWeaponPositions, LTNULL, sizeof(CAttachmentPosition*)*AI_MAX_WEAPONS);
		m_cWeapons = m_pAttachments->EnumerateWeapons(m_apWeapons, m_apWeaponPositions, AI_MAX_WEAPONS);

		// The right-hand weapon attachment is the primary weapon.
		m_iPrimaryWeapon = -1;
		for(int32 iAttachment=0; iAttachment < m_cWeapons; ++iAttachment)
		{
			if( stricmp(m_apWeaponPositions[iAttachment]->GetName(), "RightHand") == 0 )
			{
				m_iPrimaryWeapon = iAttachment;
				break;
			}
		}
	}

	// Store the primary weapon prop.
	// This eliminates repeated lookups, and allows the primary
	// weapon type to be looked up after the AI dies (and its weapons
	// are removed), which is needed for finding death anims.

	WEAPON const* pWeaponData = LTNULL;
	if( m_iPrimaryWeapon != -1 )
	{
		CWeapon* pWeapon = m_apWeapons[m_iPrimaryWeapon];
		if ( pWeapon )
		{
			pWeaponData = g_pWeaponMgr->GetWeapon(pWeapon->GetId());
		}
	}
	m_ePrimaryWeaponProp = GetWeaponProp( pWeaponData );


	if ( (m_cObjects < 0 && m_pAttachments->HasObjects()) || (m_cObjects > 0 && !m_pAttachments->HasObjects()) )
	{
		// Record all of our Objects and positions

		memset(m_apObjects, LTNULL, sizeof(CWeapon*)*AI_MAX_OBJECTS);
		memset(m_apObjectPositions, LTNULL, sizeof(CAttachmentPosition*)*AI_MAX_OBJECTS);
		m_cObjects = m_pAttachments->EnumerateObjects(m_apObjects, m_apObjectPositions, AI_MAX_OBJECTS);
	}

	m_bInitializedAttachments = LTTRUE;
}

// ----------------------------------------------------------------------- //

LTBOOL CAI::HasWeapon(EnumAIWeaponType eWeaponType)
{
	WEAPON const *pWeaponData;
	for( int32 iWeapon=0; iWeapon < m_cWeapons; ++iWeapon )
	{
		pWeaponData = g_pWeaponMgr->GetWeapon( m_apWeapons[iWeapon]->GetId() );
		if (pWeaponData && ( pWeaponData->nAIWeaponType == eWeaponType ) )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HasDangerousWeapon
//
//	PURPOSE:	Determine if the weapon we are holding is dangerous
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::HasDangerousWeapon()
{
	WEAPON const *pWeaponData;
	for( int32 iWeapon=0; iWeapon < m_cWeapons; ++iWeapon )
	{
		pWeaponData = g_pWeaponMgr->GetWeapon( m_apWeapons[iWeapon]->GetId() );
		if (pWeaponData && pWeaponData->bLooksDangerous )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

EnumAIWeaponType CAI::GetPrimaryWeaponType()
{
	CWeapon* pWeapon = GetPrimaryWeapon();
	if ( pWeapon )
	{
		WEAPON const* pWeaponData = g_pWeaponMgr->GetWeapon(pWeapon->GetId());
		if( pWeaponData )
		{
			return (EnumAIWeaponType)pWeaponData->nAIWeaponType;
		}
	}

	return kAIWeap_Invalid;
}

EnumAIWeaponType CAI::GetCurrentWeaponType()
{
	CWeapon* pWeapon = GetCurrentWeapon();
	if ( pWeapon )
	{
		WEAPON const* pWeaponData = g_pWeaponMgr->GetWeapon(pWeapon->GetId());
		if( pWeaponData )
		{
			return (EnumAIWeaponType)pWeaponData->nAIWeaponType;
		}
	}

	return kAIWeap_Invalid;
}

CWeapon* CAI::GetWeapon(int iWeapon)
{
	AIASSERT(iWeapon >= 0 && iWeapon < m_cWeapons, m_hObject, "CAI::GetWeapon: Invalid weapon index.");
	return m_apWeapons[iWeapon];
}

CWeapon* CAI::GetWeapon(EnumAIWeaponType eWeaponType)
{
	WEAPON const *pWeaponData;
	for( int32 iWeapon=0; iWeapon < m_cWeapons; ++iWeapon )
	{
		pWeaponData = g_pWeaponMgr->GetWeapon( m_apWeapons[iWeapon]->GetId() );
		if (pWeaponData && ( pWeaponData->nAIWeaponType == eWeaponType ) )
		{
			return m_apWeapons[iWeapon];
		}
	}

	return LTNULL;
}

EnumAnimProp CAI::GetWeaponProp(WEAPON const* pWeaponData)
{
	if (pWeaponData)
	{
		return GetWeaponProp( (EnumAIWeaponType)pWeaponData->nAIWeaponType );
	}

	return kAP_Weapon3;
}

EnumAnimProp CAI::GetWeaponProp(EnumAIWeaponType eWeaponType)
{
	switch ( eWeaponType )
	{
		case kAIWeap_Ranged:
			return kAP_Weapon1;
		case kAIWeap_Melee:
// HACK: for TO2 AIs with grenades, comment out Thrown weapons so AI choose correct animations.
//		case kAIWeap_Thrown:
			return kAP_Weapon2;
	}

	return kAP_Weapon3;
}

// ----------------------------------------------------------------------- //

LTBOOL CAI::SetCurrentWeapon( EnumAIWeaponType eWeaponType )
{
	WEAPON const *pWeaponData;
	for( int32 iWeapon=0; iWeapon < m_cWeapons; ++iWeapon )
	{
		pWeaponData = g_pWeaponMgr->GetWeapon( m_apWeapons[iWeapon]->GetId() );
		if (pWeaponData && ( pWeaponData->nAIWeaponType == eWeaponType ) )
		{
			m_iCurrentWeapon = iWeapon;
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetDeflecting
//
//	PURPOSE:	Sets the time we will be deflecting.
//
// ----------------------------------------------------------------------- //

void CAI::SetDeflecting( float fDuration )
{
	// Deflect based on random chance set in brain, and affected by difficulty.

	if( m_pBrain->GetAIDataExist( kAIData_DeflectChance ) )
	{
		LTFLOAT fChance = RAISE_BY_DIFFICULTY( m_pBrain->GetAIData( kAIData_DeflectChance ) );
		if( GetRandom(0.0f, 1.0f) <= fChance )
		{
			super::SetDeflecting( fDuration );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateSensingMembers()
//
//	PURPOSE:	Update the AI's variables for sensing.
//
// ----------------------------------------------------------------------- //

void CAI::UpdateSensingMembers()
{
	// Update the position of important nodes (eye, torso, etc)
	// Nodes only need to update before each sensing iteration.
	// Nodes are only updated on-demand - if someone requests their pos or rot.

	m_bUpdateNodes = LTTRUE;

	// It is OK to update the target's visibility if new sense info has come in.

	m_pTarget->SetCanUpdateVisibility( LTTRUE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Update()
//
//	PURPOSE:	Update the AI
//
// ----------------------------------------------------------------------- //

#define TIME(x) x;
//#define TIME(x) StartTimingCounter(); x; EndTimingCounter(#x);

// The following Macro is very useful for debugging one AI at a time.
#define ONLYONE(x) if( stricmp( GetName(), #x ) ) { PostUpdate(); return; }

void CAI::Update()
{
	// USE THE FOLLOWING BLOCK FOR DEBUGGING 1 AI:
//	ONLYONE(enemy01);

	// Perform any movement found in the animation.

	if( m_pState->GetStateStatus() != kSStat_Paused )
	{
		TIME(UpdateMovement());
	}

	/// TIME( m_pTrackedNodeContext->Update() );

	// Update our goals

	TIME( m_pGoalMgr->UpdateGoals());

	// Update our state if we have any pending state changes

	TIME(UpdateState());

	// Update our lag/accuracy modifiers

	TIME(UpdateAccuracy());

	// Update our position, rotation, etc

	TIME(UpdatePosition());

	// Initial message

	if ( g_pAIPathMgr->IsInitialized() )
	{
		// Send the initial message to ourselves if we have one

		if ( m_hstrCmdInitial )
		{
			SendMixedTriggerMsgToObject(this, m_hObject, m_hstrCmdInitial);

			FREE_HSTRING(m_hstrCmdInitial);
		}
	}
/*
	// Update our debug iseg junk

	static int s_cTotalCalls = 0;
	static int s_cTotalUpdates = 0;
	s_cTotalCalls += g_cIntersectSegmentCalls;
	s_cTotalUpdates++;
    g_pLTServer->CPrint("iseg calls = %3.3d avg = %f", g_cIntersectSegmentCalls, (float)s_cTotalCalls/(float)s_cTotalUpdates);
	g_cIntersectSegmentCalls = 0;
*/

	if ( !m_bInitializedAttachments )
	{
		InitAttachments();
	}

	// Record if we had a target before we update.

    LTBOOL bHadTarget = m_pTarget->IsValid();

	// Don't do any of this business if we're dead or locked

	if ( !m_damage.IsDead() )
	{
		// Update any target we're locked in on

		if ( m_pTarget->IsValid() )
		{
			TIME(UpdateTarget());
		}

		// Update the state if we have one

		if ( m_pState )
		{
			// Update our state.

			if ( g_pAINodeMgr->IsInitialized() )
			{
				// Update the list of invalid nodes this AI knows about.
				// specificly, time out any that haven't been checked for a
				// long time, as they may now be valid again.
				TIME(UpdateInvalidNodeList());

				if ( GetDebugLevel() > 0 )
				{
                    g_pLTServer->CPrint("%s in %s state.", GetName(), m_pState->GetName());

					if ( GetDebugLevel() > 1 )
					{
						if ( HasLastVolume() )
						{
							g_pLTServer->CPrint("%s in volume %s", GetName(), GetLastVolume()->GetName());

							if ( GetLastVolume()->HasRegion() )
							{
								g_pLTServer->CPrint("%s in region %s %s",
									GetName(),
									GetLastVolume()->GetRegion()->GetName(),
									GetLastVolume()->GetRegion()->IsSearchable() ? " (searchable)" : "");
							}
						}
						else
						{
							g_pLTServer->CPrint("%s not in volume", GetName());
						}
					}
				}

				TIME(m_pState->PreUpdate());
				TIME(m_pState->Update());
				TIME(m_pState->PostUpdate());
			}
		}

		// If we had a target and no longer have one, play our sheepish anim

		if ( bHadTarget && !m_pTarget->IsValid() )
		{
			HandleTargetDied();
		}
	}
/*
	// Update animator

	UpdateAnimation();
*/

	// Update our ground info

	TIME(UpdateOnGround());

	// Update our state

	TIME(UpdateState());

	// Update our character fx

	TIME(UpdateCharacterFx());

	// Update the music

	TIME(UpdateMusic());

	// Update debug info

	TIME(UpdateInfo());

	// Post update

	TIME(PostUpdate());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PostUpdate
//
//	PURPOSE:	Does our postupdate
//
// ----------------------------------------------------------------------- //

void CAI::PostUpdate()
{
	// Set first update flag

	m_bFirstUpdate = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateAnimation
//
//	PURPOSE:	Handles any pending animator changes
//
// ----------------------------------------------------------------------- //

void CAI::UpdateAnimation()
{
	CCharacter::UpdateAnimation();

	if ( m_pState && ( !m_damage.IsDead() ) )
	{
		m_pState->UpdateAnimation();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateMusic
//
//	PURPOSE:	Updates the dynamic music
//
// ----------------------------------------------------------------------- //

void CAI::UpdateMusic()
{
	if ( m_pState )
	{
		m_pState->UpdateMusic();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateOnGround
//
//	PURPOSE:	Update AI on ground
//
// ----------------------------------------------------------------------- //

void CAI::UpdateOnGround()
{
	CCharacter::UpdateOnGround();

	// Lets see if we are in the ground or in the air.

	CollisionInfo Info;
    g_pLTServer->GetStandingOn(m_hObject, &Info);

	// Reset standing on info

	m_eStandingOnSurface = ST_UNKNOWN;
    m_bOnGround = LTTRUE;

	if (Info.m_hObject)
	{
		if (Info.m_Plane.m_Normal.y < 0.76)
		{
			// Get rid of our XZ velocities

            LTVector vVel;
            g_pPhysicsLT->GetVelocity(m_hObject, &vVel);

			vVel.x = vVel.z = 0.0f;
            g_pPhysicsLT->SetVelocity(m_hObject, &vVel);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateState
//
//	PURPOSE:	Handles any pending state changes
//
// ----------------------------------------------------------------------- //

void CAI::UpdateState()
{
    if ( m_hstrNextStateMessage && g_pLTServer->GetTime() >= m_fNextStateTime )
	{
		SendMixedTriggerMsgToObject(this, m_hObject, m_hstrNextStateMessage);
		FREE_HSTRING(m_hstrNextStateMessage);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateInfo
//
//	PURPOSE:	Updates info sent down for debugging.
//
// ----------------------------------------------------------------------- //

void CAI::UpdateInfo()
{
	if (!g_AIInfoTrack.IsInitted())
	{
		g_AIInfoTrack.Init(g_pLTServer, "AIInfo", LTNULL, 0.0f);
	}

	// Update our info string.
	if( g_AIInfoTrack.GetFloat() > 0.0f )
	{
		char szTemp[128];

		CString info = m_cstrNameInfo;
		info += "\n";

		if( g_AIInfoTrack.GetFloat() == 1.0f )
		{
			sprintf( szTemp, "HitPoints : %.0f\nArmor : %.0f\n", GetHitPoints( ),
				m_damage.GetArmorPoints( ));
			info += szTemp;

			// Show Senses 0 or 1.

			if( m_bSensesOn )
			{
				info += "Senses : 1\n";
			} else {
				info += "Senses : 0\n";
			}

			// Show alignment.
			info += "Algnmnt : ";
			info += CRelationMgr::GetGlobalRelationMgr()->GetButeMgr()->GetObjectRelationMgrTemplateName( GetRelationMgr()->GetTemplateID() );
			info += "\n";

			// Show goalset, goal and state.
			info += "GS : ";
			if( m_pGoalMgr )
			{
				info += g_pAIGoalButeMgr->GetGoalSet( m_pGoalMgr->GetGoalSetIndex() )->szName;
			}
			else
			{
				info += "none";
			}

			info += "\n";

			info += "G : ";
			if( m_pGoalMgr && m_pGoalMgr->GetCurrentGoal() )
			{
				info += s_aszGoalTypes[m_pGoalMgr->GetCurrentGoal()->GetGoalType()];
			}
			else
			{
				info += "none";
			}

			info += "\n";

			info += "S : ";
			info += s_aszStateTypes[m_pState->GetStateType()];
		}
		else if( g_AIInfoTrack.GetFloat() == 2.0f )
		{
			// Show script.
			GOAL_SCRIPT_LIST::const_iterator iter = m_pGoalMgr->BeginScript();
			uint32 nStep = 0;
			for( ;iter != m_pGoalMgr->EndScript(); ++iter, ++nStep )
			{
				const GOAL_SCRIPT_STRUCT & element = *iter;

				sprintf( szTemp, " %s %s %s",
					(nStep == m_pGoalMgr->GetCurrentScriptStep()) ? "*" : " ",
					s_aszGoalTypes[element.eGoalType],
					element.hstrNameValuePairs ?
					          g_pLTServer->GetStringData(element.hstrNameValuePairs)
							: "" );
				info += szTemp;
				info += "\n";
			}

			if( !nStep )
			{
				info += "None\n";
			}

		}
		else if( g_AIInfoTrack.GetFloat() == 3.0f )
		{
			// Show goals.
			for( AIGOAL_LIST::const_iterator iter = m_pGoalMgr->BeginGoals();
			     iter != m_pGoalMgr->EndGoals(); ++iter )
			{
				sprintf(szTemp, " %s %f : %s %s",
					m_pGoalMgr->IsCurGoal(*iter) ? "*" : " ",
					(*iter)->GetCurImportance(),
					s_aszGoalTypes[(*iter)->GetGoalType()],
					(*iter)->IsScripted() ? "(scripted)" : "" );

				info += szTemp;
				info += "\n";
			}
		}
		else if( g_AIInfoTrack.GetFloat() == 4.0f )
		{
			// Show Relations.
#if _MSC_VER >= 1300
			std::ostrstream out;
#else
			ostrstream out;
#endif // VC7
			out << *(m_pRelationMgr->GetRelationUser()) << '\n' << '\0';
			info += out.str();
		}

		if( info != m_cstrCurrentInfo )
		{
			m_cstrCurrentInfo = info;

			SendInfoString( m_hObject, m_cstrCurrentInfo );
		}
	}
	else
	{
		if( !m_cstrCurrentInfo.IsEmpty() )
		{
			m_cstrCurrentInfo = "";
			SendInfoString(m_hObject, m_cstrCurrentInfo);
		}
	}
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAI::UpdateInvalidNodeList()
//
//	PURPOSE:	Check for invalid nodes which have expired, and now are
//				potentially valid again.
//
//	NOTE:		Ugly early return to allow enabling/disabling of feature
//
//----------------------------------------------------------------------------
void CAI::UpdateInvalidNodeList( )
{
	if ( !g_pAIButeMgr->GetRules()->bKeepInvalidNodeHistory )
	{
		return;
	}

	using std::transform;
	using std::bind2nd;
	using std::remove_if;
	using std::equal_to;

	transform(
		m_InvalidNodeList.begin(),
		m_InvalidNodeList.end(),
		m_InvalidNodeList.begin(),
		bind2nd( DeleteExpiredNodes(), g_pLTServer->GetTime() ));

	std::vector<INVALID_NODE*>::iterator pos;
	INVALID_NODE *pNull = NULL;
	pos = remove_if(
		m_InvalidNodeList.begin(),
		m_InvalidNodeList.end(),
		bind2nd( equal_to<INVALID_NODE*>(), pNull ));

	m_InvalidNodeList.erase(pos, m_InvalidNodeList.end());
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAI::AddNewInvalidNode()
//
//	PURPOSE:	Returns a new INVALIDNODE, after adding it to the list so that
//				it will definitely be cleaned up later.
//
//----------------------------------------------------------------------------
INVALID_NODE* CAI::AddNewInvalidNode()
{
	INVALID_NODE* pNode = s_bankINVALID_NODE.New();
	m_InvalidNodeList.push_back(pNode);
	return pNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetDefensePercentage()
//
//	PURPOSE:	Determine the "intensity" of the AI's defense against projectiles
//
// ----------------------------------------------------------------------- //

float CAI::GetDefensePercentage( LTVector const *pIncomingProjectilePosition ) const
{
	if ( m_bDefenseOn )
	{
		return GetAITemplate()->fDefensePercentage;
	}
	else
	{
		return 0.0f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::FilterDamage()
//
//	PURPOSE:	Do preprocessing of the damage before it gets dealt
//
// ----------------------------------------------------------------------- //

bool CAI::FilterDamage( DamageStruct *pDamageStruct )
{
	// Filter out damage caused by myself if brain says so.

	if(	m_pBrain &&
		m_pBrain->GetAIDataExist( kAIData_DoNotDamageSelf ) &&
		( m_pBrain->GetAIData( kAIData_DoNotDamageSelf ) > 0.f ) &&
		( pDamageStruct->hDamager == m_hObject ) )
	{
		return false;
	}

	if ( IsDefending() )
	{
		// the AI is defending, remove some of the damage
		pDamageStruct->fDamage *= ( 1.0f - GetDefensePercentage( &pDamageStruct->vDir ) );
	}

	return CCharacter::FilterDamage( pDamageStruct );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetWeaponPosition()
//
//	PURPOSE:	Is the position of our "Weapon" (could be anything)
//
// ----------------------------------------------------------------------- //

LTVector CAI::GetWeaponPosition(CWeapon* pWeapon)
{
	LTVector vPos = m_vPos;
	vPos.y += m_vDims.y;

	return vPos;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAI::GetWeaponSocket()
//
//	PURPOSE:	Returns the socket the weapon is attached to.  Asserts if the
//				socket cannot be found (error condition).
//
//----------------------------------------------------------------------------
HMODELSOCKET CAI::GetWeaponSocket(CWeapon* pWeapon)
{
	AIASSERT( pWeapon, m_hObject, "CAIHuman::GetWeaponPosition: No weapon" );

	// Set up the weapon and attachment arrays so we can Enumerate the weapons
	CWeapon* apWeapons[kMaxWeapons];
	CAttachmentPosition* apAttachmentPositions[kMaxWeapons];
	const uint32 cWeapons = GetAttachments()->EnumerateWeapons(apWeapons,
		apAttachmentPositions, kMaxWeapons);

	// Loop through the weapon array, looking for the weapon with the same ID.
	// When found, save it to the selectedattachmentposition.
	CAttachmentPosition* pSelectedAttachment = NULL;
	for ( uint32 i = 0; i < cWeapons; i++ )
	{
		AIASSERT( apWeapons[i], m_hObject, "GetWeaponSocket: Invalid weapon pointer" );
		if ( apWeapons[i]->GetId() == pWeapon->GetId() )
		{
			AIASSERT( pSelectedAttachment == NULL, m_hObject, "GetWeaponSocket: Attachment found at multiple positions" );
			pSelectedAttachment = apAttachmentPositions[i];
		}
	}

	AIASSERT( pSelectedAttachment, m_hObject, "GetWeaponSocket: Position not found" );

	// Retreive the socket based on the attachment name
	HMODELSOCKET hSocket;
	LTRESULT SocketResult = g_pModelLT->GetSocket(m_hObject, const_cast<char*>(pSelectedAttachment->GetName()), hSocket);
	AIASSERT( SocketResult == LT_OK, m_hObject, "GetWeaponSocket: Unable to get socket for transform" );

	return hSocket;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetEye*()
//
//	PURPOSE:	Get eye node position and forward.
//
// ----------------------------------------------------------------------- //

const LTVector& CAI::GetEyePosition()
{
	// Nodes will only really update once per sense update.
	UpdateNodes();
	return m_vEyePos;
}

const LTVector& CAI::GetEyeForward()
{
	// Nodes will only really update once per sense update.
	UpdateNodes();
	return m_vEyeForward;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetTorso*()
//
//	PURPOSE:	Get torso vectors.
//
// ----------------------------------------------------------------------- //

const LTVector& CAI::GetTorsoRight()
{
	// Nodes will only really update once per sense update.
	UpdateNodes();
	return m_vTorsoRight;
}

const LTVector& CAI::GetTorsoForward()
{
	// Nodes will only really update once per sense update.
	UpdateNodes();
	return m_vTorsoForward;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetNodeTrackingTarget()
//
//	PURPOSE:	Use Head and Torso tracking to look at something.
//
// ----------------------------------------------------------------------- //

void CAI::SetNodeTrackingTarget(EnumTrackedNodeGroup eGroup, const LTVector& vPos)
{
	m_pTrackedNodeContext->SetTrackedTarget( eGroup, vPos );
}

void CAI::SetNodeTrackingTarget(EnumTrackedNodeGroup eGroup, HOBJECT hModel, const char* pszNodeName)
{
	if( hModel == m_hObject )
	{
		AIASSERT( 0, m_hObject, "AI told to track self." );
		return;
	}

	m_pTrackedNodeContext->SetTrackedTarget( eGroup, hModel, pszNodeName, LTVector(0.f, 0.f, 0.f) );
}

void CAI::SetNodeTrackingTarget(EnumTrackedNodeGroup eGroup, HOBJECT hModel)
{
	if( hModel == m_hObject )
	{
		AIASSERT( 0, m_hObject, "AI told to track self." );
		return;
	}

	m_pTrackedNodeContext->SetTrackedTarget( eGroup, hModel, LTVector(0.f, 0.f, 0.f) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::EnableNodeTracking()
//
//	PURPOSE:	Turn on Head and Torso tracking.
//				Return TRUE if AI is facing his target.
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::EnableNodeTracking(EnumTrackedNodeGroup eGroup, HOBJECT hFace)
{
	// Do not use tracking nodes if movement is locked (e.g. jumping, climbing ladder)

	if( m_pAIMovement->IsMovementLocked() )
	{
		m_pTrackedNodeContext->SetActiveTrackingGroup( kTrack_None );
		return LTFALSE;
	}

	// Do not use tracking of the target is too close.

	if( hFace && HasTarget() && ( m_vPos.DistSqr( m_pTarget->GetVisiblePosition() ) < 128.f*128.f ) )
	{
		m_pTrackedNodeContext->SetActiveTrackingGroup( kTrack_None );
		return FaceObject( hFace );
	}

	// Set the active tracking group, if not already set.

	if( m_pTrackedNodeContext->GetActiveTrackingGroup() != eGroup )
	{
		m_pTrackedNodeContext->SetActiveTrackingGroup( eGroup );
	}


	if( hFace )
	{
		LTBOOL bAtLimit = LTTRUE;
		if( m_pTrackedNodeContext->IsValidTrackingGroup( eGroup ) )
		{
			LTVector vFace;
			g_pLTServer->GetObjectPos( hFace, &vFace );

			LTVector vDir = vFace - m_vPos;
			vDir.y = 0.f;
			vDir.Normalize();

			LTFLOAT fDp;
			fDp = vDir.Dot( m_vForward );

			if( fDp > c_fFOV120 )
			{
				bAtLimit = LTFALSE;
			}
		}

		if( bAtLimit )
		{
			return FaceObject( hFace );
		}

		return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::DisableNodeTracking()
//
//	PURPOSE:	Turn off Head and Torso tracking.
//
// ----------------------------------------------------------------------- //

void CAI::DisableNodeTracking()
{
	m_pTrackedNodeContext->SetActiveTrackingGroup( kTrack_None );
	m_bRotating = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::DisableNodeTracking()
//
//	PURPOSE:	Turn off Head and Torso tracking.
//
// ----------------------------------------------------------------------- //

EnumTrackedNodeGroup CAI::GetActiveNodeTrackingGroup() const
{
	return m_pTrackedNodeContext->GetActiveTrackingGroup();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsNodeTrackingEnabled()
//
//	PURPOSE:	Is Head and Torso tracking turned on?
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsNodeTrackingEnabled() const
{
	return( m_pTrackedNodeContext->GetActiveTrackingGroup() != kTrack_None );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsNodeTrackingAtLimit()
//
//	PURPOSE:	Determine if nodes have hit their limits.
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsNodeTrackingAtLimit() const
{
	return m_pTrackedNodeContext->IsAtLimit( m_pTrackedNodeContext->GetActiveTrackingGroup() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsObjectVisible*()
//
//	PURPOSE:	Is the test object visible
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsObjectVisibleFromEye(ObjectFilterFn ofn, PolyFilterFn pfn, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTBOOL bDoVolumeCheck, HOBJECT* phBlockingObject /*= LTNULL*/, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)
{
	return IsObjectVisible(ofn, pfn, GetEyePosition(), hObj, fVisibleDistanceSqr, bFOV, bDoVolumeCheck, phBlockingObject, pfDistanceSqr, pfDp, pvDir);
}

LTBOOL CAI::IsObjectVisibleFromWeapon(ObjectFilterFn ofn, PolyFilterFn pfn, CWeapon* pWeapon, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTBOOL bDoVolumeCheck, HOBJECT* phBlockingObject /*= LTNULL*/, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)
{
	return IsObjectVisible(ofn, pfn, GetWeaponPosition(pWeapon), hObj, fVisibleDistanceSqr, bFOV, bDoVolumeCheck, phBlockingObject, pfDistanceSqr, pfDp, pvDir);
}

LTBOOL CAI::IsObjectVisible(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vPosition, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTBOOL bDoVolumeCheck, HOBJECT* phBlockingObject /*= LTNULL*/, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)
{
    LTVector vObjectPosition;
	g_pLTServer->GetObjectPos(hObj, &vObjectPosition);

	return IsObjectPositionVisible(ofn, pfn, vPosition, hObj, vObjectPosition, fVisibleDistanceSqr, bFOV, bDoVolumeCheck, phBlockingObject, pfDistanceSqr, pfDp, pvDir);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsObjectPositionVisible*()
//
//	PURPOSE:	Is a given position on the test object visible
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsObjectPositionVisibleFromEye(ObjectFilterFn ofn, PolyFilterFn pfn, HOBJECT hObj, const LTVector& vObjectPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTBOOL bDoVolumeCheck, HOBJECT* phBlockingObject /*= LTNULL*/, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)
{
	return IsObjectPositionVisible(ofn, pfn, GetEyePosition(), hObj, vObjectPosition, fVisibleDistanceSqr, bFOV, bDoVolumeCheck, phBlockingObject, pfDistanceSqr, pfDp, pvDir);
}

LTBOOL CAI::IsObjectPositionVisibleFromWeapon(ObjectFilterFn ofn, PolyFilterFn pfn, CWeapon* pWeapon, HOBJECT hObj, const LTVector& vObjectPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTBOOL bDoVolumeCheck, HOBJECT* phBlockingObject /*= LTNULL*/, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)
{
	return IsObjectPositionVisible(ofn, pfn, GetWeaponPosition(pWeapon), hObj, vObjectPosition, fVisibleDistanceSqr, bFOV, bDoVolumeCheck, phBlockingObject, pfDistanceSqr, pfDp, pvDir);
}

LTBOOL CAI::IsObjectPositionVisible(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vSourcePosition, HOBJECT hObj, const LTVector& vObjectPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTBOOL bDoVolumeCheck, HOBJECT* phBlockingObject /*= LTNULL*/, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)
{
    if (!g_pLTServer || !m_hObject) return LTFALSE;

	if ( hObj != LTNULL )
	{
		LTVector vObjectDims;
		g_pPhysicsLT->GetObjectDims(hObj, &vObjectDims);

		// See if the position is inside the object...

		if ( vSourcePosition.x > vObjectPosition.x - vObjectDims.x && vSourcePosition.x < vObjectPosition.x + vObjectDims.x &&
			 vSourcePosition.y > vObjectPosition.y - vObjectDims.y && vSourcePosition.y < vObjectPosition.y + vObjectDims.y &&
			 vSourcePosition.z > vObjectPosition.z - vObjectDims.z && vSourcePosition.z < vObjectPosition.z + vObjectDims.z)
		{
			// Gotta fudge some of these values

			if ( pfDp )	*pfDp = 1.0;
			if ( pfDistanceSqr ) *pfDistanceSqr = MATH_EPSILON;
			if ( pvDir ) *pvDir = GetEyeForward();

			return LTTRUE;
		}
	}

    LTVector vDir;
	VEC_SUB(vDir, vObjectPosition, vSourcePosition);
    LTFLOAT fDistanceSqr = VEC_MAGSQR(vDir);

	// Record the dist.

	if ( pfDistanceSqr ) *pfDistanceSqr = fDistanceSqr;

	// Make sure it is close enough if we aren't alert

	if( fDistanceSqr >= fVisibleDistanceSqr)
	{
        return LTFALSE;
	}


    LTVector vDirNorm = vDir;
	vDirNorm.Normalize();

    LTFLOAT fNoFOVDistanceSqr = g_pAIButeMgr->GetSenses()->fNoFOVDistanceSqr;
    LTFLOAT fDp;


	// Make sure it is in our FOV

	if ( bFOV )
	{
		// First check horizontal FOV

		fDp = vDirNorm.Dot(GetEyeForward());

		if ( IsAlert() || fDistanceSqr < fNoFOVDistanceSqr )
		{
			// FOV is 180' if we're alert

            if (fDp <= c_fFOV180-m_fFOVBias)
			{
				return LTFALSE;
			}
		}
		else
		{
			// FOV is 140' if we're not alert

            if (fDp <= c_fFOV140-m_fFOVBias) return LTFALSE;
		}

		// Now check vertical FOV

		fDp = vDirNorm.Dot(m_vUp);

		// We already know that the object is in front of us at this point, so no need
		// to test if object is within 180 vertical fov if we're not alert

		if ( !IsAlert() )
		{
			// FOV is 120' if we're not alert

            if (fDp >= c_fFOV120-m_fFOVBias) return LTFALSE;
		}
	}

	// If there is a straight path thru the volumes (open space) then
	// the position must be visible.

	if( bDoVolumeCheck )
	{
		uint32 dwExcludeVolumes =	AIVolume::kVolumeType_Ladder |
									AIVolume::kVolumeType_Stairs |
									AIVolume::kVolumeType_JumpOver |
									AIVolume::kVolumeType_JumpUp |
									AIVolume::kVolumeType_AmbientLife |
									AIVolume::kVolumeType_Teleport;

		if( g_pAIVolumeMgr->StraightPathExists( this, vSourcePosition, vObjectPosition, GetVerticalThreshold(), dwExcludeVolumes, GetLastVolume() ) )
		{
			if( phBlockingObject ) *phBlockingObject = LTNULL;
			if( pfDp ) *pfDp = fDp;
			if( pvDir ) *pvDir = vDirNorm;
			return LTTRUE;
		}
	}


	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, vSourcePosition);
	VEC_COPY(IQuery.m_To, vObjectPosition);

	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID | (pfn ? INTERSECT_HPOLY : 0);
	IQuery.m_FilterFn = ofn;
	IQuery.m_PolyFilterFn = pfn;

	s_hFilterAI = m_hObject;

	g_cIntersectSegmentCalls++;

	LTBOOL bIntersected = g_pLTServer->IntersectSegment(&IQuery, &IInfo);
    if ( (bIntersected == LTFALSE) || ((hObj != LTNULL) && (IInfo.m_hObject == hObj)) )
	{
		if ( pfDp ) *pfDp = fDp;
		if ( pvDir ) *pvDir = vDirNorm;

		return LTTRUE;
	}

	if( phBlockingObject )
	{
		*phBlockingObject = IInfo.m_hObject;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsPositionVisible*()
//
//	PURPOSE:	Is the test position visible to us
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsPositionVisibleFromEye(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTBOOL bDoVolumeCheck, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)
{
	return IsPositionVisible(ofn, pfn, GetEyePosition(), vPosition, fVisibleDistanceSqr, bFOV, bDoVolumeCheck, pfDistanceSqr, pfDp, pvDir);
}

LTBOOL CAI::IsPositionVisibleFromWeapon(ObjectFilterFn ofn, PolyFilterFn pfn, CWeapon* pWeapon, const LTVector& vPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTBOOL bDoVolumeCheck, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)
{
	return IsPositionVisible(ofn, pfn, GetWeaponPosition(pWeapon), vPosition, fVisibleDistanceSqr, bFOV, bDoVolumeCheck, pfDistanceSqr, pfDp, pvDir);
}

LTBOOL CAI::IsPositionVisible(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vFrom, const LTVector& vTo, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTBOOL bDoVolumeCheck, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)
{
    if (!g_pLTServer || !m_hObject) return LTFALSE;

    LTVector vDir;
	VEC_SUB(vDir, vTo, vFrom);
    LTFLOAT fDistanceSqr = VEC_MAGSQR(vDir);

	// Make sure it is close enough

	if( fDistanceSqr >= fVisibleDistanceSqr )
	{
        return LTFALSE;
	}

    LTVector vDirNorm = vDir;
	vDirNorm.Normalize();

    LTFLOAT fNoFOVDistanceSqr = g_pAIButeMgr->GetSenses()->fNoFOVDistanceSqr;
    LTFLOAT fDp;

	// Make sure it is in our FOV

	if ( bFOV )
	{
		fDp = vDirNorm.Dot(GetEyeForward());

		if ( ( m_eAwareness == kAware_Alert ) || ( fDistanceSqr < fNoFOVDistanceSqr ) )
		{
			// FOV is 180' if we're alert

            if (fDp <= c_fFOV180-m_fFOVBias) return LTFALSE;
		}
		else
		{
			// FOV is 140' if we're not alert

            if (fDp <= c_fFOV140-m_fFOVBias) return LTFALSE;
		}

		// Now check vertical FOV

		fDp = vDirNorm.Dot(m_vUp);

		// We already know that the object is in front of us at this point, so no need
		// to test if object is within 180 vertical fov if we're not alert

		if ( !IsAlert() )
		{
			// FOV is 120' if we're not alert

            if (fDp >= c_fFOV120-m_fFOVBias) return LTFALSE;
		}
	}


	// If there is a straight path thru the volumes (open space) then
	// the position must be visible.

	if( bDoVolumeCheck )
	{
		uint32 dwExcludeVolumes =	AIVolume::kVolumeType_Ladder |
									AIVolume::kVolumeType_Stairs |
									AIVolume::kVolumeType_JumpOver |
									AIVolume::kVolumeType_JumpUp |
									AIVolume::kVolumeType_AmbientLife |
									AIVolume::kVolumeType_Teleport;

		if( g_pAIVolumeMgr->StraightPathExists( this, vFrom, vTo, GetVerticalThreshold(), dwExcludeVolumes, GetLastVolume() ) )
		{
			if( pfDp ) *pfDp = fDp;
			if( pvDir ) *pvDir = vDirNorm;
			return LTTRUE;
		}
	}


	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, vFrom);
	VEC_COPY(IQuery.m_To, vTo);
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID | (pfn ? INTERSECT_HPOLY : 0);
	IQuery.m_FilterFn = ofn;
	IQuery.m_PolyFilterFn = pfn;

	s_hFilterAI = m_hObject;

	g_cIntersectSegmentCalls++;
    if (g_pLTServer->IntersectSegment(&IQuery, &IInfo))
	{
        return LTFALSE;
	}

	if ( pfDp ) *pfDp = fDp;
	if ( pfDistanceSqr ) *pfDistanceSqr = fDistanceSqr;
	if ( pvDir ) *pvDir = vDirNorm;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CanSearch()
//
//	PURPOSE:	Returns TRUE if AI can search from where it is.
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::CanSearch()
{
	if ( HasLastVolume() )
	{
		if ( GetLastVolume()->HasRegion() )
		{
			AIRegion* pRegion = GetLastVolume()->GetRegion();
			if ( pRegion->IsSearchable() )
			{
				return LTTRUE;
			}
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::RequestCrouch()
//
//	PURPOSE:	Returns TRUE if ally AI agrees to crouch.
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::RequestCrouch(HOBJECT hRequestingAI)
{
	if( !( m_pState && m_pState->GetStateType() == kState_HumanAttack ) )
	{
		return LTFALSE;
	}

	CAIHumanStateAttack* pStateAttack = (CAIHumanStateAttack*)m_pState;
	return pStateAttack->RequestCrouch( hRequestingAI );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::RequestDodge()
//
//	PURPOSE:	Returns TRUE if ally AI agrees to dodge.
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::RequestDodge(HOBJECT hRequestingAI)
{
	if( !( m_pState && m_pState->GetStateType() == kState_HumanAttack ) )
	{
		return LTFALSE;
	}

	CAIHumanStateAttack* pStateAttack = (CAIHumanStateAttack*)m_pState;
	return pStateAttack->RequestDodge( hRequestingAI );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::FaceObject()
//
//	PURPOSE:	Turn to face a specific object
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::FaceObject(HOBJECT hObj)
{
    LTVector vTargetPos;
	g_pLTServer->GetObjectPos(hObj, &vTargetPos);

	return FacePos(vTargetPos);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::FaceDir()
//
//	PURPOSE:	Turn to face a specific direciton
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::FaceDir(const LTVector& vDir)
{
	return FacePos(m_vPos + ( vDir * 10.f ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::FacePos()
//
//	PURPOSE:	Turn to face a specific pos
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::FacePos(const LTVector& vTargetPos)
{
	return FacePos( vTargetPos, ROTATION_SPEED_STATIC );
}

LTBOOL CAI::FacePosMoving(const LTVector& vTargetPos)
{
	LTVector vDir = vTargetPos - m_vPos;
	vDir.y = 0.f;
	vDir.Normalize();

	LTVector vNewTargetPos = m_vPos + ( vDir * 10.f );

	if( m_eAwareness == kAware_Relaxed )
	{
		if( vDir.Dot( m_vForward ) > c_fFOV45 )
		{
			return FacePos( vNewTargetPos, ROTATION_SPEED_MOVING );
		}
	}

	return FacePos( vNewTargetPos, ROTATION_SPEED_STATIC );
}

LTBOOL CAI::FacePos(const LTVector& vTargetPos, LTFLOAT fRotationSpeed)
{
    LTVector vDir;
  	VEC_SUB(vDir, vTargetPos, m_vPos);
   	vDir.y = 0.0f; // Don't look up/down

 	if ( vDir.MagSqr() < 1.f )
   	{
 		// Ignore requests to to face a pos very close to where we already are.
 		// They are likely to make the AI pop the wrong direction.

        //_ASSERT(LTFALSE);
        return LTTRUE;
   	}

  	VEC_NORM(vDir);

    LTFLOAT fDpTargetForward = vDir.Dot(m_vTargetForward);
    LTFLOAT fDpForward = vDir.Dot(m_vForward);

  	if ( m_bRotating && (fabs(fDpTargetForward) > c_fFacingThreshhold) )
   	{
   		// We're already (going to be) faceing "close enough", so just ignore this request

  		return (fabs(fDpForward) > c_fFacingThreshhold);
   	}
  	else
  	{
  		// The time we have to turn is based on how far we have to turn. Turning 0' will take 0
  		// seconds, and turning 180' will take 1 second.

          m_bRotating = LTTRUE;
          //m_fRotationTime = (LTFLOAT)fabs(0.5f - fDpForward/2.0f)/m_fRotationSpeed + 1.0e-5f;

  		g_pLTServer->GetObjectRotation(m_hObject, &m_rStartRot);
  		m_rTargetRot = LTRotation(vDir, LTVector(0.0f, 1.0f, 0.0f));
  		m_vTargetRight = m_rTargetRot.Right();
  		m_vTargetUp = m_rTargetRot.Up();
  		m_vTargetForward = m_rTargetRot.Forward();

  		if( fDpForward >= c_fFacingThreshhold )
  		{
  			return LTTRUE;
  		}
  		else if( fDpForward < -1.f )
  		{
  			fDpForward = -1.f;
  		}

  		m_fRotationTime = ( ((LTFLOAT)acos( fDpForward )) * MATH_ONE_OVER_PI ) * fRotationSpeed;

  		m_fRotationTimer = 0.0f;

          return LTFALSE;
  	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::FaceTargetRotImmediately()
//
//	PURPOSE:	Turn to the TargetRot NOW!!
//
// ----------------------------------------------------------------------- //

void CAI::FaceTargetRotImmediately()
{
	// Set our rotation
	g_pLTServer->SetObjectRotation(m_hObject, &m_rTargetRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleModelString()
//
//	PURPOSE:	Handles model keyframe strings
//
// ----------------------------------------------------------------------- //

void CAI::HandleModelString(ArgList* pArgList)
{
    if (!g_pLTServer || !pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	if (stricmp(pKey, c_szKeyBodySlump) == 0)
	{
		// Hitting the ground noise

		SurfaceType eSurface = ST_UNKNOWN;

		CollisionInfo Info;
        g_pLTServer->GetStandingOn(m_hObject, &Info);

		if (Info.m_hPoly != INVALID_HPOLY)
		{
			eSurface = (SurfaceType)GetSurfaceType(Info.m_hPoly);
		}
		else if (Info.m_hObject) // Get the texture flags from the object...
		{
			eSurface = (SurfaceType)GetSurfaceType(Info.m_hObject);
		}

		// Play the noise

        LTVector vPos;
        g_pLTServer->GetObjectPos(m_hObject, &vPos);

		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurface);
		_ASSERT(pSurf);
		if (pSurf && pSurf->szBodyFallSnd[0])
		{
			g_pServerSoundMgr->PlaySoundFromPos(vPos, pSurf->szBodyFallSnd, pSurf->fBodyFallSndRadius, SOUNDPRIORITY_MISC_LOW);
		}
	}
	else if( !_stricmp( pKey, c_szKeyClose ) )
	{
		if( m_hAnimObject )
		{
			SendTriggerMsgToObject( this, m_hAnimObject, LTFALSE, "OFF" );
		}
	}
	else if( !_stricmp( pKey, c_szKeyOpen ) )
	{
		if( m_hAnimObject )
		{
			SendTriggerMsgToObject( this, m_hAnimObject, LTFALSE, "ON" );
		}
	}
	else if( !_stricmp( pKey, c_szActivate ) )
	{
		if( m_hAnimObject )
		{
			SendTriggerMsgToObject( this, m_hAnimObject, LTFALSE, "ACTIVATE" );
		}
	}
	else if ( !_stricmp( pKey, c_szAttachmentAnim  ) )
	{
		// Argument 1:	Attachment position/name to send the animation to
		// Argument 2:	Name of the animation to send it
		AIASSERT( pArgList->argc >= 2, m_hObject, "Format: ATTACHMENTANIM SocketName AnimationName" );

		const char* const pszAttachmentPos = pArgList->argv[1];
		AIASSERT( pszAttachmentPos, m_hObject, "missing SocketName (ATTACHMENTANIM SocketName AnimationName)" );

		const char* const pszAnimationName = pArgList->argv[2];
		AIASSERT( pszAnimationName, m_hObject, "missing AnimationName (ATTACHMENTANIM SocketName AnimationName)" );

		CAttachment* pAttachment = GetAttachments()->GetAttachment( pszAttachmentPos );
		AIASSERT1( pAttachment, m_hObject, "Unable to find Attachment: %s", pszAttachmentPos );

		if ( pAttachment->GetModel() )
		{
			CAutoMessage cMsg;
			cMsg.Writeuint32( MID_ATTACHMENTANIM );
			cMsg.WriteStringAsHString( pszAnimationName );

			LTRESULT ltResult;
			ltResult = g_pLTServer->SendToObject( cMsg.Read(), m_hObject, pAttachment->GetModel(), MESSAGE_GUARANTEED );
			AIASSERT( ltResult == LT_OK, m_hObject, "Failed to send message" );
		}
	}

	// Let state handle it.

	if ( m_pState )
	{
		m_pState->HandleModelString(pArgList);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CAI::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
    if (!g_pLTServer || !pMsg) return;

	pMsg->WriteString( m_sBrain );
	if ( m_pBrain )
	{
		SAVE_BOOL(LTTRUE);
		m_pBrain->Save(pMsg);
	}
	else
	{
		SAVE_BOOL(LTFALSE);
	}

	SAVE_BOOL(m_bCheapMovement);
	SAVE_FLOAT(m_flVerticalThreshold);

	SAVE_HSTRING(m_hstrBodySkinExtension);

	SAVE_HSTRING(m_hstrNextStateMessage);
	SAVE_TIME(m_fNextStateTime);

	m_pTarget->Save(pMsg);

	SAVE_VECTOR(m_vEyePos);
	SAVE_VECTOR(m_vEyeForward);
	SAVE_VECTOR(m_vTorsoRight);
	SAVE_VECTOR(m_vTorsoForward);
	SAVE_BOOL(m_bUpdateNodes);

	SAVE_VECTOR(m_vPos);
	SAVE_VECTOR(m_vPathingPosition);
	SAVE_ROTATION(m_rRot);
	SAVE_VECTOR(m_vRight);
	SAVE_VECTOR(m_vUp);
	SAVE_VECTOR(m_vForward);
	SAVE_VECTOR(m_vDims);
	SAVE_FLOAT(m_fRadius);
	SAVE_ROTATION(m_rStartRot);
	SAVE_ROTATION(m_rTargetRot);
	SAVE_VECTOR(m_vTargetRight);
	SAVE_VECTOR(m_vTargetUp);
	SAVE_VECTOR(m_vTargetForward);
	SAVE_BOOL(m_bRotating);
	SAVE_FLOAT(m_fRotationTime);
	SAVE_FLOAT(m_fRotationTimer);

	SAVE_BOOL(m_bSeeThrough);
	SAVE_BOOL(m_bShootThrough);

	SAVE_TIME(m_fNextCombatSoundTime);

	SAVE_HSTRING(m_hstrCmdInitial);
	SAVE_HSTRING(m_hstrCmdActivateOn);
	SAVE_HSTRING(m_hstrCmdActivateOff);
	SAVE_HSTRING(m_hstrCmdOnMarking);
	SAVE_HSTRING(m_hstrCmdProximityGoal);

	SAVE_BOOL(m_bActivated);
	SAVE_BOOL(m_bAlwaysActivate);
	SAVE_bool(m_bCanTalk);

	SAVE_bool(m_bUnconscious);

	SAVE_BOOL(m_bFirstUpdate);

	if ( m_bFirstUpdate )
	{
		if (!m_sQueuedCommands.IsEmpty( ))
		{
			SAVE_BOOL(LTTRUE);
			SAVE_CHARSTRING(m_sQueuedCommands);
		}
		else
		{
			SAVE_BOOL(LTFALSE);
		}
	}

	SAVE_BOOL(m_bInitializedAttachments);

	SAVE_INT(m_cDroppedWeapons);
	for ( int iWpn = 0 ; iWpn < m_cDroppedWeapons ; iWpn++ )
	{
		SAVE_HOBJECT(m_aDroppedWeapons[iWpn].hPickupItem);
		SAVE_CHARSTRING(m_aDroppedWeapons[iWpn].sAttachString);
	}


	SAVE_DWORD(m_eAwareness);
	SAVE_DWORD(m_nAlarmLevel);
	SAVE_DWORD(m_eAlarmStimID);
	SAVE_TIME(m_fLastStimulusTime);
	SAVE_TIME(m_fLastRelaxedTime);
	SAVE_FLOAT(m_fFOVBias);

	SAVE_HSTRING(m_hstrHolster);
	SAVE_HSTRING(m_hstrHolsterBackup);
	SAVE_TIME(m_fRestoreBackupWeaponTime);

	SAVE_DWORD(m_iCurrentWeapon);
	SAVE_DWORD(m_iPrimaryWeapon);
	SAVE_DWORD(m_ePrimaryWeaponProp);

	SAVE_FLOAT(m_fAccuracy);
	SAVE_FLOAT(m_fAccuracyIncreaseRate);
	SAVE_FLOAT(m_fAccuracyDecreaseRate);
	SAVE_FLOAT(m_fAccuracyModifier);
	SAVE_FLOAT(m_fAccuracyModifierTime);
	SAVE_FLOAT(m_fAccuracyModifierTimer);
	SAVE_FLOAT(m_fFullAccuracyRadiusSqr);
	SAVE_FLOAT(m_fAccuracyMissPerturb);
	SAVE_FLOAT(m_fMaxMovementAccuracyPerturb);
	SAVE_FLOAT(m_fMovementAccuracyPerturbDecay);

	SAVE_HOBJECT(m_hDialogueObject);

	SAVE_FLOAT(m_fSenseUpdateRate);
	SAVE_TIME(m_fNextSenseUpdate);
	SAVE_DWORD(m_flagsCurSenses);
	SAVE_DWORD(m_flagsBaseSenses);
	SAVE_BOOL(m_bSensesOn);
	m_pAISenseRecorder->Save(pMsg);

	SAVE_RANGE(m_rngSightGridX);
	SAVE_RANGE(m_rngSightGridY);

	SAVE_BOOL(m_bPreserveActiveCmds);

	SAVE_BOOL((LTBOOL)(!!m_pAnimationContext));

	if ( m_pAnimationContext )
	{
		m_pAnimationContext->Save(pMsg);
	}
	SAVE_HOBJECT(m_hAnimObject);

	m_pGoalMgr->Save(pMsg);

	// [JO] 8/16/02
	// Intentionally do not save/load member m_pAIMovement in CAI.
	// It needs to be saved/loaded AFTER the CAIState, because
	// Init on state calls Init on Strategy, which will overwrite
	// the current movement.
	//	m_pAIMovement->Save(pMsg);

	m_pPathKnowledgeMgr->Save( pMsg );

	m_pTrackedNodeContext->Save(pMsg);
	SAVE_DWORD(m_eTriggerNodeTrackingGroup);

	SAVE_VECTOR(m_vMovePos);
	SAVE_BOOL(m_bMove);
	SAVE_FLOAT(m_fSpeed);
	SAVE_FLOAT(m_fJumpOverVel);
	SAVE_BOOL(m_bClearMovementHint);
	SAVE_DWORD(m_hHintAnim);
	SAVE_BOOL(m_bUseMovementEncoding);
	SAVE_BOOL(m_bTimeToUpdate);
	SAVE_DWORD(m_dwBaseValidVolumeTypes);
	SAVE_DWORD(m_dwCurValidVolumeTypes);

	// save the defense variables
	SAVE_BOOL(m_bDefenseOn);

	SAVE_INT( m_InvalidNodeList.size() );

	std::vector<INVALID_NODE*>::iterator it;
	for( it = m_InvalidNodeList.begin(); it != m_InvalidNodeList.end(); ++it )
	{
		INVALID_NODE* pInvalidNode = *it;
		SAVE_BOOL( pInvalidNode->m_bBlocked );
		SAVE_TIME( pInvalidNode->m_flTime );
		SAVE_HOBJECT( pInvalidNode->m_hNode );
	}

	SAVE_FLOAT( m_flHoverAccelerationRate );
	SAVE_TIME( m_flLastHoverTime );
	SAVE_FLOAT( m_flCurrentHoverSpeed );

	SAVE_FLOAT( m_flSentryChallengeScanDistMax );
	SAVE_FLOAT( m_flSentryChallengeDistMax );
	SAVE_FLOAT( m_flSentryMarkDistMax );

	SAVE_bool( m_bCanCarry );
	SAVE_bool( m_bBeingCarried );
	SAVE_bool( m_bStuckWithTrackDart );
	SAVE_bool( m_bIsCinematicAI );
	SAVE_bool( m_bCanWake );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CAI::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
    if (!g_pLTServer || !pMsg) return;

	pMsg->ReadString( m_sBrain.GetBuffer( 256 ), 256 );
	m_sBrain.ReleaseBuffer( );
	LTBOOL bBrain = LTFALSE;
	LOAD_BOOL(bBrain);
	if ( bBrain )
	{
		SetBrain(m_sBrain);
		if ( m_pBrain )
		{
			m_pBrain->Load(pMsg);
		}
	}

	LOAD_BOOL(m_bCheapMovement);
	LOAD_FLOAT(m_flVerticalThreshold);

	LOAD_HSTRING(m_hstrBodySkinExtension);

	LOAD_HSTRING(m_hstrNextStateMessage);
	LOAD_TIME(m_fNextStateTime);

	m_pTarget->Load(pMsg);

	LOAD_VECTOR(m_vEyePos);
	LOAD_VECTOR(m_vEyeForward);
	LOAD_VECTOR(m_vTorsoRight);
	LOAD_VECTOR(m_vTorsoForward);
	LOAD_BOOL(m_bUpdateNodes);

	LOAD_VECTOR(m_vPos);
	LOAD_VECTOR(m_vPathingPosition);
	LOAD_ROTATION(m_rRot);
	LOAD_VECTOR(m_vRight);
	LOAD_VECTOR(m_vUp);
	LOAD_VECTOR(m_vForward);
	LOAD_VECTOR(m_vDims);
	LOAD_FLOAT(m_fRadius);
	LOAD_ROTATION(m_rStartRot);
	LOAD_ROTATION(m_rTargetRot);
	LOAD_VECTOR(m_vTargetRight);
	LOAD_VECTOR(m_vTargetUp);
	LOAD_VECTOR(m_vTargetForward);
	LOAD_BOOL(m_bRotating);
	LOAD_FLOAT(m_fRotationTime);
	LOAD_FLOAT(m_fRotationTimer);

	LOAD_BOOL(m_bSeeThrough);
	LOAD_BOOL(m_bShootThrough);

	LOAD_TIME(m_fNextCombatSoundTime);

	LOAD_HSTRING(m_hstrCmdInitial);
	LOAD_HSTRING(m_hstrCmdActivateOn);
	LOAD_HSTRING(m_hstrCmdActivateOff);
	LOAD_HSTRING(m_hstrCmdOnMarking);
	LOAD_HSTRING(m_hstrCmdProximityGoal);

	LOAD_BOOL(m_bActivated);
	LOAD_BOOL(m_bAlwaysActivate);
	LOAD_bool(m_bCanTalk);

	LOAD_bool(m_bUnconscious);

	LOAD_BOOL(m_bFirstUpdate);

	if ( m_bFirstUpdate )
	{
		LTBOOL bQueuedCommands = LTFALSE;
		LOAD_BOOL(bQueuedCommands);

		if (bQueuedCommands)
		{
			char szBuf[1024];
			LOAD_CHARSTRING( szBuf, ARRAY_LEN( szBuf ));
			m_sQueuedCommands = szBuf;
		}
	}

	LOAD_BOOL(m_bInitializedAttachments);

	// Record all of our weapons and positions

	memset(m_apWeapons, LTNULL, sizeof(CWeapon*)*AI_MAX_WEAPONS);
	memset(m_apWeaponPositions, LTNULL, sizeof(CAttachmentPosition*)*AI_MAX_WEAPONS);
	m_cWeapons = m_pAttachments->EnumerateWeapons(m_apWeapons, m_apWeaponPositions, AI_MAX_WEAPONS);

	// Record all of our Objects and positions

	memset(m_apObjects, LTNULL, sizeof(CWeapon*)*AI_MAX_OBJECTS);
	memset(m_apObjectPositions, LTNULL, sizeof(CAttachmentPosition*)*AI_MAX_OBJECTS);
	m_cObjects = m_pAttachments->EnumerateObjects(m_apObjects, m_apObjectPositions, AI_MAX_OBJECTS);

	LOAD_INT(m_cDroppedWeapons);
	char szSpawn[1024];

	for ( int iWpn = 0 ; iWpn < AI_MAX_WEAPONS ; iWpn++ )
	{
		if (iWpn < m_cDroppedWeapons)
		{
			LOAD_HOBJECT(m_aDroppedWeapons[iWpn].hPickupItem);
			LOAD_CHARSTRING(szSpawn,sizeof(szSpawn));
			m_aDroppedWeapons[iWpn].sAttachString = szSpawn;
		}
		else
		{
			m_aDroppedWeapons[iWpn].Clear();
		}
	}


	LOAD_DWORD_CAST(m_eAwareness, EnumAIAwareness);
	LOAD_DWORD(m_nAlarmLevel);
	LOAD_DWORD_CAST(m_eAlarmStimID, EnumAIStimulusID);
	LOAD_TIME(m_fLastStimulusTime);
	LOAD_TIME(m_fLastRelaxedTime);
	LOAD_FLOAT(m_fFOVBias);

	LOAD_HSTRING(m_hstrHolster);
	LOAD_HSTRING(m_hstrHolsterBackup);
	LOAD_TIME(m_fRestoreBackupWeaponTime);

	LOAD_DWORD(m_iCurrentWeapon);
	LOAD_DWORD(m_iPrimaryWeapon);
	LOAD_DWORD_CAST(m_ePrimaryWeaponProp, EnumAnimProp);

	LOAD_FLOAT(m_fAccuracy);
	LOAD_FLOAT(m_fAccuracyIncreaseRate);
	LOAD_FLOAT(m_fAccuracyDecreaseRate);
	LOAD_FLOAT(m_fAccuracyModifier);
	LOAD_FLOAT(m_fAccuracyModifierTime);
	LOAD_FLOAT(m_fAccuracyModifierTimer);
	LOAD_FLOAT(m_fFullAccuracyRadiusSqr);
	LOAD_FLOAT(m_fAccuracyMissPerturb);
	LOAD_FLOAT(m_fMaxMovementAccuracyPerturb);
	LOAD_FLOAT(m_fMovementAccuracyPerturbDecay);

	LOAD_HOBJECT(m_hDialogueObject);

	LOAD_FLOAT(m_fSenseUpdateRate);
	LOAD_TIME(m_fNextSenseUpdate);
	LOAD_DWORD(m_flagsCurSenses);
	LOAD_DWORD(m_flagsBaseSenses);
	LOAD_BOOL(m_bSensesOn);
	m_pAISenseRecorder->Load(pMsg);

	LOAD_RANGE_CAST(m_rngSightGridX, int);
	LOAD_RANGE_CAST(m_rngSightGridY, int);

	LOAD_BOOL(m_bPreserveActiveCmds);

	LTBOOL bAnimationContext = LTFALSE;
	LOAD_BOOL(bAnimationContext);

	if ( m_pAnimationContext )
	{
		m_pAnimationMgr->DestroyAnimationContext(m_pAnimationContext);
		m_pAnimationContext = LTNULL;
	}

	if (bAnimationContext)
	{
		m_pAnimationContext = m_pAnimationMgr->CreateAnimationContext(m_hObject);

		if ( m_pAnimationContext )
		{
			m_pAnimationContext->Load(pMsg);
		}
	}
	LOAD_HOBJECT(m_hAnimObject);

	m_pGoalMgr->Init(this);
	m_pGoalMgr->Load(pMsg);

	// [JO] 8/16/02
	// Intentionally do not save/load member m_pAIMovement in CAI.
	// It needs to be saved/loaded AFTER the CAIState, because
	// Init on state calls Init on Strategy, which will overwrite
	// the current movement.
	//	m_pAIMovement->Load(pMsg);

	m_pPathKnowledgeMgr->Load( pMsg );

	m_pTrackedNodeContext->Init( m_hObject, m_eModelSkeleton, g_pServerTrackedNodeMgr );
	m_pTrackedNodeContext->Load(pMsg);
	LOAD_DWORD_CAST(m_eTriggerNodeTrackingGroup, EnumTrackedNodeGroup);

	// Go back to tracking none, since we reset this all the time anyway.
	m_pTrackedNodeContext->SetActiveTrackingGroup( kTrack_None );

	LOAD_VECTOR(m_vMovePos);
	LOAD_BOOL(m_bMove);
	LOAD_FLOAT(m_fSpeed);
	LOAD_FLOAT(m_fJumpOverVel);
	LOAD_BOOL(m_bClearMovementHint);
	LOAD_DWORD(m_hHintAnim);
	LOAD_BOOL(m_bUseMovementEncoding);
	LOAD_BOOL(m_bTimeToUpdate);
	LOAD_DWORD(m_dwBaseValidVolumeTypes);
	LOAD_DWORD(m_dwCurValidVolumeTypes);

	// load the defense variables
	LOAD_BOOL(m_bDefenseOn);

	int InvalidNodeCount;
	LOAD_INT( InvalidNodeCount );
	for( int x = 0; x < InvalidNodeCount; ++x )
	{
		INVALID_NODE* pInvalidNode = AddNewInvalidNode();
		LOAD_BOOL( pInvalidNode->m_bBlocked );
		LOAD_TIME( pInvalidNode->m_flTime );
		LOAD_HOBJECT( pInvalidNode->m_hNode );
	}

	m_cstrCurrentInfo = "";
	m_cstrNameInfo = GetName();

	LOAD_FLOAT( m_flHoverAccelerationRate );
	LOAD_TIME( m_flLastHoverTime );
	LOAD_FLOAT( m_flCurrentHoverSpeed );

	LOAD_FLOAT( m_flSentryChallengeScanDistMax );
	LOAD_FLOAT( m_flSentryChallengeDistMax );
	LOAD_FLOAT( m_flSentryMarkDistMax );

	LOAD_bool( m_bCanCarry );
	LOAD_bool( m_bBeingCarried );
	LOAD_bool( m_bStuckWithTrackDart );
	LOAD_bool( m_bIsCinematicAI );
	LOAD_bool( m_bCanWake );

	// Recreate the specialfx message since we know all of our data is loaded and
	// it may have gotten created inadvertently with default data by loading other
	// data such as the GoalMgr...

	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::*Target
//
//	PURPOSE:	Target methods
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::FaceTarget()
{
	_ASSERT(HasTarget());

	return FaceObject(GetTarget()->GetObject());
}

CAITarget* CAI::GetTarget()
{
	AIASSERT(m_pTarget->IsValid(), m_hObject, "CAI::GetTarget: AI has no valid target.");
	return m_pTarget;
}

LTBOOL CAI::HasTarget()
{
	if( !m_pTarget->IsValid() || !GetTarget( )->GetObject( ))
		return LTFALSE;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetRightAndLeftHolsterStrings
//
//	PURPOSE:	Get separate right and left holster strings,
//              which are stored concatenated.
//
// ----------------------------------------------------------------------- //

void CAI::GetRightAndLeftHolsterStrings(char* szBufferRight, char* szBufferLeft, uint32 nBufferSize)
{
	if( !( szBufferRight || szBufferLeft ) )
	{
		AIASSERT( 0, m_hObject, "CAI::GetRightAndLeftHolsterStrings: Buffers are NULL" );
		return;
	}

	if( !m_hstrHolster )
	{
		szBufferRight[0] = '\0';
		szBufferLeft[0] = '\0';
	}

	const char* szHolsterString = g_pLTServer->GetStringData( m_hstrHolster );

	// The tilda '~' indicates that this holster string was created from within
	// the code, and should be cleared after drawing a weapon.

	uint32 iStartHolster = 0;
	if( szHolsterString[0] == '~' )
	{
		iStartHolster = 1;
	}

	// Optional semi-colon separates a RightHand attachment from a LeftHand.

	const char* pColon = strchr(szHolsterString, ';');
	if( !pColon )
	{
		strncpy( szBufferRight, szHolsterString + iStartHolster, nBufferSize );
		szBufferLeft[0] = '\0';
	}
	else {
		strncpy( szBufferRight, szHolsterString + iStartHolster, nBufferSize );
		szBufferRight[(pColon - szHolsterString) - iStartHolster] = '\0';
		strncpy( szBufferLeft, pColon + 1, nBufferSize );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetRightAndLeftHolsterStrings
//
//	PURPOSE:	Get the weapon type of the holstered weapon.
//
// ----------------------------------------------------------------------- //

EnumAIWeaponType CAI::GetHolsterWeaponType()
{
	if( m_hstrHolster )
	{
		char szHolsterRight[64];
		char szHolsterLeft[64];
		GetRightAndLeftHolsterStrings( szHolsterRight, szHolsterLeft, 64);

		// Optional comma separates a weapon from its ammo type.

		char* pComma = strchr(szHolsterRight, ',');
		if( pComma )
		{
			*pComma = '\0';
		}

		const WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon( szHolsterRight );
		return (EnumAIWeaponType)pWeaponData->nAIWeaponType;
	}

	return kAIWeap_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetAwareness
//
//	PURPOSE:	Awareness methods
//
// ----------------------------------------------------------------------- //

void CAI::SetAwareness(EnumAIAwareness eAwareness)
{
	// No change.

	if( eAwareness == m_eAwareness )
	{
		return;
	}

	// If we are returning to a relaxed state, we want to ignore disturbed allies
	// who were stimulated before we returned to relaxed.

	if( ( eAwareness == kAware_Relaxed ) && ( m_eAwareness > kAware_Relaxed ) )
	{
		m_fLastRelaxedTime = g_pLTServer->GetTime();

		// Restore a new holster string from backup if we are relaxed and have no weapon.

		if( HasBackupHolsterString() &&
			( !GetCurrentWeapon() ) &&
			( !HasHolsterString() ) )
		{
			m_fRestoreBackupWeaponTime = g_pLTServer->GetTime() + 30.f;
		}
	}

	// If we going into or out of an alert state, clear path knowledge
	// because some volumes may only be useable for pathfinding when alert.

	if( ( eAwareness == kAware_Alert ) ||
		( m_eAwareness == kAware_Alert ) )
	{
		m_pPathKnowledgeMgr->ClearPathKnowledge();
	}

	// Record new awareness.

	m_eAwareness = eAwareness;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Get*Sound
//
//	PURPOSE:	Gets various sounds
//
// ----------------------------------------------------------------------- //

char* CAI::GetDeathSound()
{
	switch( GetBodyState() )
	{
		case kState_BodyCrush:
			return ::GetSound(this, kAIS_Crush);

		case kState_BodyLaser:
			return ::GetSound(this, kAIS_Electrocute);

		case kState_BodyLedge:
			return ::GetSound(this, kAIS_Fall);

		case kState_BodyLadder:
			return ::GetSound(this, kAIS_Fall);

		case kState_BodyExplode:
			return ::GetSound(this, kAIS_Explode);

		case kState_BodyStairs:
			return ::GetSound(this, kAIS_FallStairs);

		default:
			return ::GetSound(this, kAIS_Death);
	}
}

char* CAI::GetDeathSilentSound()
{
	return GetSound(this, kAIS_DeathQuiet);
}

char* CAI::GetDamageSound(DamageType eType)
{
	return ::GetSound(this, kAIS_Pain);
}

void CAI::PlayCombatSound(EnumAISoundType eSound)
{
	LTFLOAT fCurTime = g_pLTServer->GetTime();

	// Do not always play a combat sound when requested.
	// Play depending on frequency set in brain, and the last time an AI played a combat sound.

	LTFLOAT fLastSoundTime = g_pAICentralKnowledgeMgr->GetKnowledgeFloat( kCK_LastCombatSoundTime, LTNULL );
	if( ( fLastSoundTime == 0.f ) ||
		( fLastSoundTime + m_pBrain->GetAttackSoundTimeMin() < fCurTime ) )
	{
		PlaySound( eSound, LTFALSE );
		g_pAICentralKnowledgeMgr->RemoveAllKnowledge( kCK_LastCombatSoundTime, this );
		g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_LastCombatSoundTime, this, LTNULL, LTFALSE, fCurTime, LTTRUE );
	}
}

void CAI::PlaySearchSound(EnumAISoundType eSound)
{
	// Don't play search sound if other AI are already attacking the same target.
	// Sounds funny to the player to hear search sounds while getting attacked.

	uint32 cAttackers = 0;
	if( HasTarget() )
	{
		cAttackers = g_pAICentralKnowledgeMgr->CountTargetMatches( kCK_Attacking, this, g_pLTServer->HandleToObject( GetTarget()->GetObject() ) );
	}

	if( cAttackers == 0 )
	{
		PlaySound( eSound, LTFALSE );
	}
}

void CAI::PlaySound(EnumAISoundType eSound, LTBOOL bInterrupt)
{
	// HACK:  for TO2 to make AI shutup after killing someone one co-op.

	if( m_bMuteAISounds )
	{
		return;
	}

	// END HACK

	// [KLS 7/2/02] - If too many AI sounds are playing, don't play the sound.
	// Note: we do this here so that GetSound() isn't called (since GetSound()
	// keeps track of which sound was played last)...

	if (sm_cAISnds >= 2)
	{
		return;
	}

	// Do not interrupt currently playing AISound, unless instructed.

	if( ( !bInterrupt ) && IsPlayingDialogSound() )
	{
		return;
	}

	char* pSound = GetSound(this, eSound);
	PlayDialogSound(pSound, GetCharacterSoundType(eSound));
	AITRACE( AIShowSounds, ( m_hObject, "Playing AISound type: %s (%s)", s_aszAISoundTypes[eSound], pSound ) );
}

LTBOOL CAI::CanLipSync()
{
	return ( m_pBrain ) ? m_pBrain->CanLipSync() : LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Target
//
//	PURPOSE:	Targets an enemy
//
// ----------------------------------------------------------------------- //

void CAI::Target(HOBJECT hObject)
{
	// Do not reset everything for redundant calls to Target.

	if( hObject == m_pTarget->GetObject() )
	{
		return;
	}

	// Do not set target to a non-character.
	// jeffo - removed assert because some goal are triggered by bodies.

	if ( hObject && ( !IsCharacter(hObject) ) )
	{
		return;
	}

	if ( !hObject )
	{
		m_pTarget->SetObject(LTNULL);

		return;
	}

    LTVector vPosition;
	g_pLTServer->GetObjectPos(hObject, &vPosition);
    LTVector vDirection;
	vDirection.Init();

	m_pTarget->SetObject(hObject);
	m_pTarget->SetVisiblePosition(vPosition);
	// TODO: UpdateSHootPositioN?

	// Set the min dist the target must stay from AI before being pushed.

	LTVector vDims;
	g_pPhysicsLT->GetObjectDims(hObject, &vDims);
	m_pTarget->SetPushMinDist( m_fRadius + Max<LTFLOAT>( vDims.x, vDims.z ) );

    m_pTarget->SetVisibleFromEye(LTFALSE);
    m_pTarget->SetVisibleFromWeapon(LTFALSE);

    m_pTarget->SetAttacking(LTFALSE);

	UpdateTarget();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateTarget
//
//	PURPOSE:	Updates the target info
//
// ----------------------------------------------------------------------- //

void CAI::UpdateTarget()
{
    if ( !m_pTarget->IsValid() ) { _ASSERT(LTFALSE); return; }

	HOBJECT hObject = m_pTarget->GetObject();
	_ASSERT(hObject);

    CCharacter *pCharacter = (CCharacter*)g_pLTServer->HandleToObject(m_pTarget->GetObject());
	_ASSERT(pCharacter);

	if ( !pCharacter || pCharacter->IsDead() )
	{
		// Our target has died.

		m_pTarget->SetObject( NULL );

		return;
	}

	m_pTarget->UpdateVisibility();

	m_pTarget->UpdatePush( pCharacter );


	// Update the firing information. This is fairly expensive.

    m_pTarget->SetAttacking(LTFALSE);

	CharFireInfo info;
	pCharacter->GetLastFireInfo(info);

	// If they've shot recently enough

    if ( info.fTime + 0.5f > g_pLTServer->GetTime() && info.nWeaponId != WMGR_INVALID_ID )
	{
		// See if the shot passed close to us

        LTVector vFire = info.vImpactPos - info.vFiredPos;
        LTVector vNearestPoint = info.vImpactPos + vFire*(((m_vPos - info.vImpactPos).Dot(vFire))/vFire.Dot(vFire));
        const static LTFLOAT fMinDistance = 100.0f*100.0f;

		if ( VEC_DISTSQR(vNearestPoint, m_vPos) < fMinDistance )
		{
            m_pTarget->SetAttacking(LTTRUE);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateAccuracy
//
//	PURPOSE:	Updates any accuracy modifiers on the AI
//
// ----------------------------------------------------------------------- //

void CAI::UpdateAccuracy()
{
	if ( g_AccuracyInfoTrack.GetFloat(0.0f) == 1.0f )
	{
        g_pLTServer->CPrint("am=%f amt=%f a=%f", m_fAccuracyModifier, m_fAccuracyModifierTimer, GetAccuracy());
	}

	// TODO: rate at which accuracy is regained should be affected
	// by AI's skill somehow

    m_fAccuracyModifierTimer = Max<LTFLOAT>(0.0f, m_fAccuracyModifierTimer - g_pLTServer->GetFrameTime()*RAISE_BY_DIFFICULTY(m_fAccuracyIncreaseRate));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetAccuracyModifier
//
//	PURPOSE:	Sets a temporal accuracy modifier
//
// ----------------------------------------------------------------------- //

void CAI::SetAccuracyModifier(LTFLOAT fModifier, LTFLOAT fTime)
{
	m_fAccuracyModifier = fModifier;
	m_fAccuracyModifierTime = LOWER_BY_DIFFICULTY(fTime);
	m_fAccuracyModifierTimer = LOWER_BY_DIFFICULTY(fTime);
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAI::GetAccuracy()
//
//	PURPOSE:	returns the heavily modified accuracy modifier
//
//----------------------------------------------------------------------------
LTFLOAT CAI::GetAccuracy()
{
	LTFLOAT fAccuracy = RAISE_BY_DIFFICULTY(m_fAccuracy) + (RAISE_BY_DIFFICULTY(m_fAccuracy)*RAISE_BY_DIFFICULTY(m_fAccuracyModifier)*(m_fAccuracyModifierTimer/m_fAccuracyModifierTime));
	if( fAccuracy < 0.f )
	{
		fAccuracy = 0.f;
	}
	else if( fAccuracy > 1.f )
	{
		fAccuracy = 1.f;
	}
	return fAccuracy;
}

LTFLOAT	CAI::GetAccuracyMissPerturb()
{
	return LOWER_BY_DIFFICULTY(m_fAccuracyMissPerturb);
}

LTFLOAT	CAI::GetMaxMovementAccuracyPerturb()
{
	return LOWER_BY_DIFFICULTY(m_fMaxMovementAccuracyPerturb);
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAI::SetPosition()
//
//	PURPOSE:	Function to set the AIs position -- added so that all setting
//				of the position can be trapped at a single point.
//
//----------------------------------------------------------------------------
void CAI::SetPosition(const LTVector& vPos, LTBOOL bFindFloorHeight)
{
	// Set the new position
	m_vPos = vPos;

	// Update the pathing position (position with height offset) for hovering
	// characters.
	LTVector vPathingPos = m_vPos;
	float flHeight;
	if ( bFindFloorHeight && ( LTTRUE == FindFloorHeight( m_vPos, &flHeight ) ) )
	{
		vPathingPos.y = flHeight;
	}

	SetPathingPosition( vPathingPos );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdatePosition
//
//	PURPOSE:	Updates the position, orientation, etc of our object
//
// ----------------------------------------------------------------------- //

void CAI::UpdatePosition()
{
	// Set the base AI's position

	LTVector vPosition;
	g_pLTServer->GetObjectPos(m_hObject, &vPosition);
	SetPosition( vPosition, LTFALSE );

	// Retrieve AI vectors for current frame..

	g_pLTServer->GetObjectRotation(m_hObject, &m_rRot);
	m_vRight = m_rRot.Right();
	m_vUp = m_rRot.Up();
	m_vForward = m_rRot.Forward();

	// Get our dims+radius

    g_pPhysicsLT->GetObjectDims(m_hObject, &m_vDims);
    m_fRadius = Max<LTFLOAT>(m_vDims.x, m_vDims.z);

	// Get rid of any acceleration the server is applying to us

    LTVector vAccel;
	g_pPhysicsLT->GetAcceleration(m_hObject, &vAccel);
	vAccel.x = vAccel.z = 0.0f;
	if (vAccel.y > 0.0f) vAccel.y = 0.0f;
	g_pPhysicsLT->SetAcceleration(m_hObject, &vAccel);

    LTVector vVelocity;
	g_pPhysicsLT->GetVelocity(m_hObject, &vVelocity);
	vVelocity.x = vVelocity.z = 0.0f;
	if (vVelocity.y > 0.0f) vVelocity.y = 0.0f;
	g_pPhysicsLT->SetVelocity(m_hObject, &vVelocity);

  	if ( ( !m_pAIMovement->IsRotationLocked() ) && ( m_fRotationTimer < m_fRotationTime ) )
   	{
		m_fRotationTimer += g_pLTServer->GetFrameTime();
        m_fRotationTimer = Min<LTFLOAT>(m_fRotationTime, m_fRotationTimer);

        LTFLOAT fRotationInterpolation = GetRotationInterpolation(m_fRotationTimer/m_fRotationTime);

		// Rotate us if our timer is going

        LTRotation rNewRot;
		rNewRot.Slerp(m_rStartRot, m_rTargetRot, fRotationInterpolation);

		// Set our rotation

		g_pLTServer->SetObjectRotation(m_hObject, &rNewRot);

		// Update our rotation vectors

		m_vRight = rNewRot.Right();
		m_vUp = rNewRot.Up();
		m_vForward = rNewRot.Forward();
	}
	else
	{
        m_bRotating = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ChangeState
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //

void CAI::ChangeState(const char* szFormat, ...)
{
	if ( m_hstrNextStateMessage )
	{
		return;
	}

	static char szBuffer[1024];
	va_list val;
	va_start(val, szFormat);
	vsprintf(szBuffer, szFormat, val);
	va_end(val);

	// $STRING
    m_hstrNextStateMessage = g_pLTServer->CreateString((char*)szBuffer);
    m_fNextStateTime = g_pLTServer->GetTime() + 0.0f /* used to be a delay */;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ComputeDamageModifier
//
//	PURPOSE:	Adjust the amount of damage based on the node hit...
//
// ----------------------------------------------------------------------- //

LTFLOAT CAI::ComputeDamageModifier(ModelNode eModelNode)
{
    LTFLOAT fModifier = CCharacter::ComputeDamageModifier(eModelNode);

	if ( IsAlert() )
	{
        fModifier = Min<LTFLOAT>(2.0f, fModifier);
	}
	else
	{
		fModifier *= g_pModelButeMgr->GetUnalertDamageFactor( m_eModelId );
	}

	return fModifier;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsAlert
//
//	PURPOSE:	Determines if we are alert or not
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsAlert()
{
//	return (m_eAwareness >= kAware_Alert)? LTTRUE : LTFALSE;
	return (m_eAwareness >= kAware_Suspicious)? LTTRUE : LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsSuspicious
//
//	PURPOSE:	Determines if we are suspicious or not
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsSuspicious()
{
	return (m_eAwareness >= kAware_Suspicious)? LTTRUE : LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IncrementAlarmLevel
//
//	PURPOSE:	Raises alarm level. Registers suspicious stimulus if reached threshold.
//
// ----------------------------------------------------------------------- //

void CAI::IncrementAlarmLevel(uint32 nIncr)
{
	m_nAlarmLevel += nIncr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsMajorlyAlarmed
//
//	PURPOSE:	Determines if we are majorly alarmed
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsMajorlyAlarmed()
{
	return ( GetBrain() && ( m_nAlarmLevel >= GetBrain()->GetMajorAlarmThreshold() ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsImmediatelyAlarmed
//
//	PURPOSE:	Determines if we are immediately alarmed
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsImmediatelyAlarmed()
{
	return ( GetBrain() && ( m_nAlarmLevel >= GetBrain()->GetImmediateAlarmThreshold() ) );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::LinkDialogueObject()
//
//	PURPOSE:	Links us to a dialogue object
//
// --------------------------------------------------------------------------- //

void CAI::LinkDialogueObject(HOBJECT hDialogueObject)
{
	if ( m_hDialogueObject )
	{
		// We allow this because the dialogue is messaging us per everytime
		// we're in the list rather than be smart and only doing it once.
		// g_pLTServer->CPrint("AI in simultaneous dialogues!");
	}
	else
	{
		m_hDialogueObject = hDialogueObject;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UnlinkDialogueObject()
//
//	PURPOSE:	Unlinks us from a dialogue object
//
// --------------------------------------------------------------------------- //

void CAI::UnlinkDialogueObject(HOBJECT hDialogueObject)
{
	if (!m_hDialogueObject) return;

	_ASSERT(hDialogueObject == m_hDialogueObject);
	if ( hDialogueObject != m_hDialogueObject )
	{
		g_pLTServer->CPrint("AI ''%s'' was unlinked from dialogue object it was not linked to!", GetName());
		return;
	}

	m_hDialogueObject = LTNULL;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CanBeDamagedAsAttachment()
//
//	PURPOSE:	Gives a chance to reject damage we receive when we are an attachment
//
// --------------------------------------------------------------------------- //

LTBOOL CAI::CanBeDamagedAsAttachment()
{
	if ( m_pState && !m_pState->CanBeDamagedAsAttachment() )
	{
		return LTFALSE;
	}
	else
	{
		return LTTRUE;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetAlternateDeathAnimation()
//
//	PURPOSE:	Gives goals a chance to choose an appropriate death animation.
//
// --------------------------------------------------------------------------- //

HMODELANIM CAI::GetAlternateDeathAnimation()
{
	if( m_pGoalMgr )
	{
		return m_pGoalMgr->GetAlternateDeathAnimation();
	}

	// No alternate.

	return INVALID_ANI;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetBrain
//
//	PURPOSE:	Changes our current Brain
//
// ----------------------------------------------------------------------- //

bool CAI::SetBrain( char const* pszBrain )
{
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetDamageMask
//
//	PURPOSE:	Changes our current DamageMask
//
// ----------------------------------------------------------------------- //

void CAI::SetDamageMask( char const* pszMask )
{
	CDestructible* pDestructable = GetDestructible();
	if( !pDestructable )
	{
		AIASSERT( 0, m_hObject, "CAI::SetDamageMask: No destructable." );
		return;
	}

	// No mask.

	if( stricmp( pszMask, "None" ) == 0 )
	{
		pDestructable->SetCantDamageFlags( 0 );
		return;
	}

	// Find mask in aibutes.txt

	int nDamageMaskID = g_pAIButeMgr->GetDamageMaskIDByName( pszMask );
	if( nDamageMaskID != -1 )
	{
		pDestructable->SetCantDamageFlags( ~( g_pAIButeMgr->GetDamageMask( nDamageMaskID )->dfDamageTypes ) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::StartDeath()
//
//	PURPOSE:	Start dying - handle weapons dropped by unconscious AI
//
// ----------------------------------------------------------------------- //

void CAI::StartDeath()
{
	super::StartDeath();

	//handle weapons dropped when AI was knocked out
	SetUnconscious(false);
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAI::SetPathingPosition()
//
//	PURPOSE:	Sets the pathing position for the AI.  Certain AIs, such as
//				floating AIs, use this offset to keep consistant with ground
//				dwellers
//
//----------------------------------------------------------------------------
void CAI::SetPathingPosition( const LTVector& vPos )
{
	m_vPathingPosition = vPos;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAI::GetPathingPosition()
//
//	PURPOSE:	Returns the anchor point the AI uses in conjunction with the
//				pathing system.
//
//----------------------------------------------------------------------------
LTVector CAI::GetPathingPosition() const
{
	return m_vPathingPosition;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAI::GetVerticalThreshold()
//
//	PURPOSE:	Returns the distance up and down the AI checks for volumes
//
//----------------------------------------------------------------------------
float CAI::GetVerticalThreshold() const
{
	return m_flVerticalThreshold;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAI::GetInfoVerticalThreshold()
//
//	PURPOSE:	Returns the distance up and down the AI checks for information volumes
//
//----------------------------------------------------------------------------
float CAI::GetInfoVerticalThreshold() const
{
	// AIs use the same threshold for both types of volumes.
	// The player may not.

	return GetVerticalThreshold();
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAI::SetAITemplate()
//
//	PURPOSE:	Sets the AIs
//
//----------------------------------------------------------------------------
void CAI::SetAITemplate(const char* pszName )
{
	AIASSERT( m_pAITemplate == NULL, m_hObject, "AITemplate already set." );
	AIASSERT( pszName != NULL, m_hObject, "Setting AITemplate to NULL template." );

	int nTemplate = g_pAIButeMgr->GetTemplateIDByName( pszName );
	AIASSERT( nTemplate >= 0 && nTemplate < g_pAIButeMgr->GetNumTemplates(), m_hObject, "Template out of bounds" );

	AIBM_Template* pTemplate = g_pAIButeMgr->GetTemplate( nTemplate );

	AIASSERT( pTemplate != NULL, m_hObject, "pTemplate is NULL" );

	m_pAITemplate = pTemplate;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAI::GetHoverSpeed()
//
//	PURPOSE:	Returns the AIs Hover Speed
//
//----------------------------------------------------------------------------
LTFLOAT CAI::GetHoverSpeed()
{
	// Find the difference between the last time we swam, and the current time
	// After half a second of not Hoverming, reset the speed to 0

	float flTimeDifference = g_pLTServer->GetTime() - m_flLastHoverTime;
	if ( flTimeDifference >= GetBrain()->GetAIData( kAIData_HoverResetSpeedTime ) )
	{
		m_flCurrentHoverSpeed = 0.0f;
	}

	// Remember the last time we got the hover speed.
	m_flLastHoverTime = g_pLTServer->GetTime();

	float flUncappedSpeed = m_flCurrentHoverSpeed + GetHoverAcceleration() * g_pLTServer->GetFrameTime();
	float flMinSpeed = GetBrain()->GetAIData(kAIData_HoverMinSpeed);
	float flMaxSpeed = GetBrain()->GetAIData(kAIData_HoverMaxSpeed);

	m_flCurrentHoverSpeed = ( Clamp<float>( flUncappedSpeed, flMinSpeed, flMaxSpeed ) );

	return m_flCurrentHoverSpeed;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAI::HoverIsDrifting()
//
//	PURPOSE:	This can be used to see if the AI has stopped drifting
//
//----------------------------------------------------------------------------
bool CAI::HoverIsDrifting()
{
	return ( GetHoverAcceleration() < 0 && m_flCurrentHoverSpeed <= GetBrain()->GetAIData( kAIData_HoverMinSpeed ) );
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAI::GetAITemplate()
//
//	PURPOSE:	Returns a pointer to this AIs template.  Asserts if either
//				the AI does not have a template, or if the template is invalid
//				or for some reason NULL
//
//----------------------------------------------------------------------------
const AIBM_Template* const CAI::GetAITemplate() const
{
	return m_pAITemplate;

//	char* szAttributeTemplate = g_pLTServer->GetStringData(m_hstrAttributeTemplate);
//	AIASSERT( szAttributeTemplate != NULL, m_hObject, "No Template name specified" );
//
//	int nTemplateID = g_pAIButeMgr->GetTemplateIDByName(szAttributeTemplate);
//	AIASSERT( nTemplateID >= 0, m_hObject, "Template does not exist" );
//	AIASSERT( nTemplateID < g_pAIButeMgr->GetNumTemplates(), m_hObject, "Template out of range" );
//
//	AIBM_Template* pTemplate = g_pAIButeMgr->GetTemplate(nTemplateID);
//	AIASSERT( pTemplate != NULL, m_hObject, "NULL AI Template" );
//
//	return pTemplate;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAI::UpdateRelationButeMgr()
//
//	PURPOSE:	If we can see our target, then preserve any relationship with
//				them that we have.  This MAY allow relationships to fail when
//				they should be preserved if are unable to maintain their
//				targets.
//
//----------------------------------------------------------------------------
void CAI::UpdateRelationMgr()
{
	if ( HasTarget() && GetTarget()->IsVisibleFromEye() )
	{
		// Find out if the Object is entered in the ObjectRelationMgrs
		// database of relation objects.  It should be.
		CObjectRelationMgr* pORM = CRelationMgr::GetGlobalRelationMgr()->GetObjectRelationMgr(GetTarget()->GetObject());
		if ( pORM )
		{
			m_pRelationMgr->ResetRelationTime(pORM);
		}
	}

	CCharacter::UpdateRelationMgr();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetCanCarry
//
//	PURPOSE:	Sets the ai to be carried or not
//
// ----------------------------------------------------------------------- //

void CAI::SetCanCarry( bool bCarry )
{
	if( m_bCanCarry != bCarry )
	{
		m_bCanCarry = (bCarry && g_pModelButeMgr->CanModelBeCarried( m_eModelId ));

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(CFX_CAN_CARRY);
		cMsg.Writeuint8(m_bCanCarry);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

		CreateSpecialFX();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetCanWake
//
//	PURPOSE:	Sets the ai to be wake-able or not
//
// ----------------------------------------------------------------------- //

void CAI::SetCanWake( bool bWake )
{
	if( m_bCanWake != bWake )
	{
		m_bCanWake = bWake;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(CFX_CAN_WAKE);
		cMsg.Writeuint8(m_bCanWake);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

		CreateSpecialFX();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PreCreateSpecialFX
//
//	PURPOSE:	Last chance to change our characterfx struct
//
// ----------------------------------------------------------------------- //

void CAI::PreCreateSpecialFX( CHARCREATESTRUCT& cs )
{
	CCharacter::PreCreateSpecialFX( cs );

	cs.bCanCarry		= m_bCanCarry;
	cs.bIsCinematicAI	= m_bIsCinematicAI;
	cs.bCanWake			= m_bCanWake;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //
#ifndef __PSX2

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAIPlugin::CAIPlugin, ~CAIPlugin, GetAttachmentsPlugin
//
//	PURPOSE:	Now handles pointers instead to the CHumanAttachmentsPlugin
//				instead of instances, as this lets us avoid coupling in the
//				header with attachments.h
//
//----------------------------------------------------------------------------
CAIPlugin::CAIPlugin()
{
	m_pAttachmentsPlugin = debug_new( CAttachmentsPlugin );
}
CAIPlugin::~CAIPlugin()
{
	if ( m_pAttachmentsPlugin )
	{
		debug_delete( m_pAttachmentsPlugin );
		m_pAttachmentsPlugin = NULL;
	}
}
CAttachmentsPlugin* CAIPlugin::GetAttachmentsPlugin()
{
	ASSERT( m_pAttachmentsPlugin != NULL );
	return m_pAttachmentsPlugin;
}

LTRESULT CAIPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	// See if the base class wishes to handle it
	if ( LT_OK == CCharacterPlugin::PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}
	// See if the attachments plugin will handle the property
	if ( LT_OK == GetAttachmentsPlugin()->PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}

	// See if it's the priority fields

	if (strlen(szPropName) > 5 && _strcmpi("Priority", &szPropName[5]) == 0)
	{
		for ( uint32 iPriority = 0 ; iPriority < s_cPriorities ; iPriority++ )
		{
			strcpy(aszStrings[(*pcStrings)++], s_aszPriorities[iPriority]);
		}

		return LT_OK;
	}

	// See if the model style plugin will handle the property

	// No one wants it

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAIPlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //

LTRESULT CAIPlugin::PreHook_PropChanged( const char *szObjName,
											 const char *szPropName,
											 const int  nPropType,
											 const GenericProp &gpPropValue,
											 ILTPreInterface *pInterface,
											 const char *szModifiers )
{
	if( gpPropValue.m_String[0] &&
		(!_stricmp( szPropName, "Initial" ) ||
		!_stricmp( szPropName, "ActivateOn" ) ||
		!_stricmp( szPropName, "ActivateOff" ) ||
		!_stricmp( szPropName, "ProximityGoalCommand" ) ))
	{
		ConParse cpCmd;
		cpCmd.Init( gpPropValue.m_String );

		while( LT_OK == pInterface->Parse( &cpCmd ))
		{
			if( m_CommandMgrPlugin.CommandExists( cpCmd.m_Args[0] ))
			{
				if( LT_OK != m_CommandMgrPlugin.PreHook_PropChanged( szObjName,
																	 szPropName,
																	 nPropType,
																	 gpPropValue,
																	 pInterface,
																	 szModifiers ))
				{
					return LT_UNSUPPORTED;
				}
			}
			else if( cpCmd.m_nArgs > 0 )
			{
				// Since we can send commands to AIs without using the command syntax,
				// build the command like we were using propper syntax and and try to validate it...

				std::string sCmd = "";
				for( int i = 0; i < cpCmd.m_nArgs; ++i )
				{
					sCmd += cpCmd.m_Args[i];
					sCmd += " ";
				}

				char szPropVal[MAX_GP_STRING_LEN] = {0};
				LTSNPrintF( szPropVal, ARRAY_LEN( szPropVal ), "msg <CAIHuman> (%s)", sCmd.c_str() );

				GenericProp gp;
				LTStrCpy( gp.m_String, szPropVal, ARRAY_LEN( gp.m_String ));

				if( LT_OK != m_CommandMgrPlugin.PreHook_PropChanged( szObjName,
																	 szPropName,
																	 nPropType,
																	 gp,
																	 pInterface,
																	 szModifiers ))
					{
						return LT_UNSUPPORTED;
					}
			}
		}

		return LT_OK;
	}
	else if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName,
													 szPropName,
													 nPropType,
													 gpPropValue,
													 pInterface,
													 szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

#endif
