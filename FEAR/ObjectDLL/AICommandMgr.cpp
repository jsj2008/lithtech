// ----------------------------------------------------------------------- //
//
// MODULE  : AICommandMgr.cpp
//
// PURPOSE : AICommandMgr class implementation
//
// CREATED : 4/17/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AICommandMgr.h"
#include "AI.h"
#include "AIDB.h"
#include "AINode.h"
#include "AINodeScanner.h"
#include "AINodeGuard.h"
#include "AIBlackBoard.h"
#include "AICoordinator.h"
#include "AIGoalMgr.h"
#include "AIGoalAbstract.h"
#include "AISensorAbstract.h"
#include "AISensorMgr.h"
#include "AISensorSeekEnemy.h"
#include "AIStimulusMgr.h"
#include "AITarget.h"
#include "AIPathKnowledgeMgr.h"
#include "AIWorkingMemoryCentral.h"
#include "AIUtils.h"
#include "Character.h"
#include "CharacterMgr.h"
#include "CharacterDB.h"
#include "PlayerObj.h"
#include "ServerNodeTrackerContext.h"
#include "Destructible.h"
#include "AnimationPropStrings.h"
#include "PhysicsUtilities.h"
#include "WorldModel.h"

// Macros.

#define TASK_SCRIPTED	true
#define TARGET_ENEMY	true

// ----------------------------------------------------------------------- //

//
// Command validation functions.
//

static bool ValidateMsgRemoveGoal( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !g_pAIDB )
		return true;

	// Find goal by name.

	EnumAIGoalType eGoalType = CAIGoalAbstract::GetGoalType(cpMsgParams.m_Args[1]);
	AIDB_GoalRecord* pGoalRecord = g_pAIDB->GetAIGoalRecord( eGoalType );
	if( !pGoalRecord || kGoal_InvalidType == eGoalType )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateMsgRemoveGoal()" );
			pInterface->CPrint( "    MSG - REMOVEGOAL - Invalid goal '%s' not found in Game Data Base!", cpMsgParams.m_Args[1] );
		}

		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //

static bool ValidateMsgAddGoal( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !g_pAIDB )
		return true;

	if( cpMsgParams.m_nArgs < 2 || !cpMsgParams.m_Args[1] )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateMsgAddGoal()" );
			pInterface->CPrint( "    MSG - ADDGOAL - No goal was specified!" );
		}

		return false;
	}

	// Find goal by name.
	EnumAIGoalType eGoalType = CAIGoalAbstract::GetGoalType(cpMsgParams.m_Args[1]);
	AIDB_GoalRecord* pGoalRecord = g_pAIDB->GetAIGoalRecord( eGoalType );
	if( !pGoalRecord || kGoal_InvalidType == eGoalType )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateMsgAddGoal()" );
			pInterface->CPrint( "    MSG - ADDGOAL - Invalid goal '%s' not found in Game Data Base!", cpMsgParams.m_Args[1] );
		}

		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //

static bool ValidateMsgBlitz( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !g_pAIDB  )
		return true;

	// Verify the arguement count is valid.
	
	if ( cpMsgParams.m_nArgs != 2 && cpMsgParams.m_nArgs != 3 )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateMsgBlitz()" );
			pInterface->CPrint( "    MSG - BLITZ - Syntax incorrect: BLITZ <Target Object> <optional: movement style>!" );
		}

		return false;
	}

	// If a blitz type is specified, verify it exists.

	if ( cpMsgParams.m_nArgs == 3 )
	{
		EnumAIContext eContext = AIContextUtils::Enum( cpMsgParams.m_Args[2], AIContextUtils::kNotFound_ReturnInvalid );
		if ( kContext_Invalid == eContext )
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - ValidateMsgBlitz()" );
				pInterface->CPrint( "    MSG - BLITZ - Invalid AI context '%s' not found in Game DataBase!  Valid options are:", cpMsgParams.m_Args[2] );
				for ( int EachContext = 0; EachContext < AIContextUtils::Count(); ++EachContext )
				{
					pInterface->CPrint(	"	%s", AIContextUtils::String( (EnumAIContext)EachContext ) );	
				}
			}

			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //

static bool ValidateMsgGoalset( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !g_pAIDB )
		return true;

	uint32 iGoalSet = g_pAIDB->GetAIGoalSetRecordID( cpMsgParams.m_Args[1] );
	if( iGoalSet == (uint32)-1 )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateMsgGoalset()" );
			pInterface->CPrint( "    MSG - GOALSET - Invalid goalset '%s' not found in Game Data Base!", cpMsgParams.m_Args[1] );
		}

		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //

static bool ValidateMsgLead( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	const char* const pszFollowerClass = CCommandMgrPlugin::GetObjectClass( pInterface, cpMsgParams.m_Args[1] );
	const char* const pszDestinationClass = CCommandMgrPlugin::GetObjectClass( pInterface, cpMsgParams.m_Args[2] );

	if ( NULL == pszFollowerClass 
		|| !LTStrIEquals(pszFollowerClass, "CPlayerObj" ) )
	{
		WORLDEDIT_ERROR_MSG( pInterface, cpMsgParams, "Only player objects may be lead." );
		return false;
	}

	if ( NULL == pszDestinationClass 
		|| !LTStrIEquals(pszDestinationClass, "AINodeLead" ) )
	{
		WORLDEDIT_ERROR_MSG( pInterface, cpMsgParams, "Destination object must be an AINodeLead instance." );
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //

static bool ValidateDelcmd( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( cpMsgParams.m_Args[1] )
	{
		if( !LTStrICmp( cpMsgParams.m_Args[1], "ACTIVATEON" ) ||
			!LTStrICmp( cpMsgParams.m_Args[1], "ACTIVATEOFF" ))
		{
			return true;
		}

		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateDelcmd()" );
			pInterface->CPrint( "    MSG - DELCMD - Invalid Command specified '%s'!", cpMsgParams.m_Args[1] );
		}

		return false;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - ValidateDelcmd()" );
		pInterface->CPrint( "    MSG - DELCMD - No Command specified!");	
	}

	return false;
}

// ----------------------------------------------------------------------- //

static bool ValidateMsgDebugDamage(ILTPreInterface *pInterface, ConParse &cpMsgParams)
{
	// Validate the overrides if they exist.

	// 0 - message name		(always valid)
	// 1 - amount override	(always valid)
	// 2 - damagetype override	(must be valid if specified)

	if (cpMsgParams.m_nArgs > 2)
	{
		const char* const szDamageTypeName = cpMsgParams.m_Args[2];
		if (szDamageTypeName && szDamageTypeName)
		{
			DamageType eDT = StringToDamageType(szDamageTypeName);
			
			if (eDT == DT_INVALID)
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - ValidateMsgDebugDamage()" );
				pInterface->CPrint( "    MSG - DEBUGDAMAGE - Optional parameter damagetype specified, but invalid: %s!", szDamageTypeName);	
				return false;
			}
		}
	}
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateMsgSolid
//
//  PURPOSE:	Validation message for SOLID...
//
// ----------------------------------------------------------------------- //

static bool ValidateMsgSolid( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( (LTStrIEquals( cpMsgParams.m_Args[1], "1" )) ||
		(LTStrIEquals( cpMsgParams.m_Args[1], "TRUE" )) ||
		(LTStrIEquals( cpMsgParams.m_Args[1], "0" )) ||
		(LTStrIEquals( cpMsgParams.m_Args[1], "FALSE" )) )
	{
		return true;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->CPrint( "ERROR! - %s", __FUNCTION__ );
		pInterface->CPrint( "    MSG - %s - 2nd argument '%s' is not a valid bool value.", LTStrUpr(cpMsgParams.m_Args[0]), cpMsgParams.m_Args[1] );
	}

	return false;
}

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( CAI )

	ADD_MESSAGE_FLAGS( ACTIVATE,				1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleActivateMsg ),				0,						"ACTIVATE", "Run the AIs Activate command", "msg CAI01 ACTIVATE" )
	ADD_MESSAGE_FLAGS( ADDGOAL,					-1,		ValidateMsgAddGoal,		MSG_HANDLER( CAICommandMgr, HandleAddGoalMsg ),					0,						"ADDGOAL <goal>", "Add a Goal to the AI.", "msg CAI01 (ADDGOAL chase)" )
	ADD_MESSAGE_FLAGS( ADDWEAPON,				3,		NULL,					MSG_HANDLER( CAICommandMgr, HandleAddWeaponMsg ),				0,						"ADDWEAPON <Weapon> [Ammo]", "Adds the passed in weapon and (optional) ammo pair to the AI.", "msg CAI01 (ADDWEAPON Pistol PistolAmmo)" )
	ADD_MESSAGE_FLAGS( ADVANCE,					-1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleAdvanceMsg ),					0,						"ADVANCE <Node Name>", "AI will lead an orderly advance to a node.", "msg CAI01 (ADVANCE ainodegoto00)" )
	ADD_MESSAGE_FLAGS( ALERT,					-1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleAlertMsg ),					0,						"ALERT <1 or 0>", "Sets whether the AI is alert or not.", "msg CAI01 (ALERT 1)" )
	ADD_MESSAGE_FLAGS( ALIGNMENT,				2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleAlignmentMsg ),				0,						"ALIGNMENT <alignment>", "Change an AI’s alignment.", "msg CAI01 (ALIGNMENT GoodCharacter)" )
	ADD_MESSAGE_FLAGS( AMBUSH,					2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleAmbushMsg ),					CMDMGR_MF_BLOCKINGMSG,	"AMBUSH <Node Name>", "Order an AI to use a specific ambush node.  Call off the ambush order by setting <node name> to NONE.", "msg CAI01 (AMBUSH ainodeambush03)<BR>msg CAI01 (AMBUSH none)" )
	ADD_MESSAGE_FLAGS( ANIMATE,					-1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleAnimateMsg ),					CMDMGR_MF_BLOCKINGMSG,	"ANIMATE <Animation Name> [loop]", "Play an animation.  Used for scripted gameplay.", "msg CAI01 (ANIMATE jump)<BR>msg CAI01 (ANIMATE dance loop)<BR>msg CAI01 (ANIMATE jump blend)<BR>msg CAI01 (ANIMATE dance loop blend)" )
	ADD_MESSAGE_FLAGS( AWARENESS,				2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleAwarenessMsg ),				0,						"AWARENESS <RELAXED, SUSPICIOUS, or ALERT>", "Change the AI’s awareness level.", "msg CAI01 (AWARENESS suspicious)")
	ADD_MESSAGE_FLAGS( AWARENESSMOD,			2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleAwarenessModMsg ),			0,						"AWARENESSMOD <NONE, IMMEDIATETHREAT or INJURED>", "Modifies the AI awareness level to form an awareness matrix", "msg CAI01 (AWARENESSMOD Injured)" )
	ADD_MESSAGE_FLAGS( BLITZ,					-1,		ValidateMsgBlitz,		MSG_HANDLER( CAICommandMgr, HandleBlitzMsg ),					0,						"BLITZ <Target Object> [Movement Style]", "Instructs the AI to run towards the specified target object (player or AINode).  If the node is a ‘SmartObject’ node, the AI will ‘use’ the node.  If it is not, the AI will run to its position.  The AIContext parameter can specify a custom movement style for the AI.  If unspecified, the Default movement style is used.", "msg CAI01 (BLITZ player)<BR>msg CAI01 (BLITZ player blitzing)<BR>msg CAI01 (BLITZ gotonode blitzing)<BR>msg CAI01 (BLITZ pickupweaponnode blitzing" )
	ADD_MESSAGE_FLAGS( CANACTIVATE,				2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleCanActivateMsg ),				0,						"CANACTIVATE <1 or 0>", "Sets whether the AI is activatable by the player or not", "msg CAI01 (CANACTIVATE 1)" )
	ADD_MESSAGE_FLAGS( CINEFIRE,				1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleCinefireMsg ),				0,						"CINEFIRE", "Fires the AI's primary weapon during a cinematic.", "msg CAI01 CINEFIRE" )
	ADD_MESSAGE_FLAGS( CINERACT,				2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleCineractMsg ),				0,						"CINERACT <Animation Name>", "Play a cineractive animation.  Never use for gameplay scripting, only for non-interactive cinematics.  Cineractive animations do not use movement encoding.", "msg CAI01 (CINERACT jump)" )
	ADD_MESSAGE_FLAGS( CINERACTLOOP,			2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleCineractLoopMsg ),			0,						"CINERACTLOOP <Animation Name>", "Play a looping cineractive animation.  Never use for gameplay scripting, only for non-interactive cinematics.  Cineractive animations do not use movement encoding.", "msg CAI01 (CINERACTLOOP dance)" )
	ADD_MESSAGE_FLAGS( CLEARTASK,				1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleClearTaskMsg ),				0,						"CLEARTASK", "Clears AI’s knowledge of tasks.", "msg CAI01 CLEARTASK" )
	ADD_MESSAGE_FLAGS( CLEARTHREAT,				1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleClearThreatMsg ),				0,						"CLEARTHREAT", "Clears AI’s knowledge of threats, and forces AI to re-evaluate his target.", "msg CAI01 CLEARTHREAT" )
	ADD_MESSAGE_FLAGS( COVER,					2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleCoverMsg ),					CMDMGR_MF_BLOCKINGMSG,	"COVER <Node Name>", "Order an AI to use a specific cover node.  Call off the cover order by setting <node name> to NONE.", "msg CAI01 (cover ainodecover03)<BR>msg CAI01 (cover none)" )
	ADD_MESSAGE_FLAGS( DAMAGEMASK,				2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleDamageMaskMsg ),				0,						"DAMAGEMASK <key>", "Set an AI’s DamageMask, listed in AI.Attributes", "msg CAI01 (DAMAGEMASK robot)" )
	ADD_MESSAGE_FLAGS( DEBUGACTIONS,			1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleDebugActions ),				0,						"DEBUGACTIONS", "Debugging message", "<b>Do not use</b>" )
	ADD_MESSAGE_FLAGS( DEBUGGOALS,				1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleDebugGoals ),					0,						"DEBUGGOALS", "Debugging message", "<b>Do not use</b>" )
	ADD_MESSAGE_FLAGS( DEBUGDAMAGE,				-1,		ValidateMsgDebugDamage,	MSG_HANDLER( CAICommandMgr, HandleDebugDamage ),				0,						"DEBUGDAMAGE", "Debugging message", "<b>Do not use</b>" )
	ADD_MESSAGE_FLAGS( DEBUGNODESOFTYPE,		2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleDebugNodesOfType ),			0,						"DEBUGNODESOFTYPE", "Debugging message", "<b>Do not use</b>" )
	ADD_MESSAGE_FLAGS( DEBUGSENSORS,			1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleDebugSensors ),				0,						"DEBUGSENSORS", "Debugging message", "<b>Do not use</b>" )
	ADD_MESSAGE_FLAGS( DEBUGSHOVE,				1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleDebugShoveMsg ),				0,						"DEBUGSHOVE", "Debugging message", "<b>Do not use</b>" )
	ADD_MESSAGE_FLAGS( DELCMD,					2,		ValidateDelcmd,			MSG_HANDLER( CAICommandMgr, HandleDelCmdMsg ),					0,						"DELCMD <activateon or activateoff>", "AIs can be configured to have commands which are fired off when the AI is activated. This command can be sent to an AI to clear those commands. DelCmd ACTIVATEON clears the AIs ACTIVATEON command, while DelCmd ACTIVATEOFF clears the AIs ACTIVATEOFF command.", "msg CAI01 (DELCMD ACTIVATEOFF)")
	ADD_MESSAGE_FLAGS( DROPWEAPONS,				1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleDropWeaponsMsg ),				0,						"DROPWEAPONS", "Force an AI to drop all it's weapons.", "msg CAI01 DROPWEAPONS" )
	ADD_MESSAGE_FLAGS( DESIRE,					3,		NULL,					MSG_HANDLER( CAICommandMgr, HandleDesireMsg ),					0,						"DESIRE <Desire Name> <1 or 0>", "This command can be used to script assassin cloaking. ‘Desire UNCLOAK’ will cause an assassin to uncloak, while ‘desire NEVERCLOAK’ prevents an assassin from cloaking completely.", "msg CAI01 (DESIRE Cloak 1)" )
	ADD_MESSAGE_FLAGS( DESTROY,					1,		NULL,					MSG_HANDLER( CAICommandMgr, NULLHandler ),						0,						"DESTROY", "Kill an AI instantly.", "msg CAI01 DESTROY")
	ADD_MESSAGE_FLAGS( DISMOUNT,				1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleDismountMsg ),				0,						"DISMOUNT", "Dismount from a vehicle.", "msg CAI01 DISMOUNT")
	ADD_MESSAGE_FLAGS( FACEPLAYER,				1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleFacePlayerMsg ),				0,						"FACEPLAYER", "Have the AI face the player", "msg CAI01 FACEPLAYER")
	ADD_MESSAGE_FLAGS( FOLLOW,					-1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleFollowMsg ),					0,						"FOLLOW <Object Name or None>", "Follow the specified object or clear a previous follow task.", "msg CAI01 (FOLLOW CAI02)" )
	ADD_MESSAGE_FLAGS( FORCEGROUND,				2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleForceGroundMsg ),				0,						"FORCEGROUND <1 or 0>", "Set whether AI sticks to the ground.", "msg CAI01 (FORCEGROUND 1)")
	ADD_MESSAGE_FLAGS( GOALSET,					2,		ValidateMsgGoalset,		MSG_HANDLER( CAICommandMgr, HandleGoalSetMsg ),					0,						"GOALSET <goalset>", "Change an AI’s current GoalSet, listed in AI.GoalSets", "msg CAI01 (GOALSET guard)" )
	ADD_MESSAGE_FLAGS( GOTO,					2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleGotoMsg ),					CMDMGR_MF_BLOCKINGMSG,	"GOTO <Node Name>", "Order an AI to goto a node.", "msg CAI01 (GOTO ainodegoto08)" )
	ADD_MESSAGE_FLAGS( GUARD,					2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleGuardMsg ),					0,						"GUARD <Node Name>", "Order an AI to guard a node.", "msg CAI01 (GUARD ainodeguard11)" )
	ADD_MESSAGE_FLAGS( HIDEPIECE,				3,		NULL,					MSG_HANDLER( CAICommandMgr, HandleHidePieceMsg ),				0,						"HIDEPIECE <Piece Name> <1 or 0>", "Hide or show the specified piece on the AI model.", "msg CAI01 (HIDEPIECE Head 1)" )
	ADD_MESSAGE_FLAGS( INTRO,					2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleIntroMsg ),					CMDMGR_MF_BLOCKINGMSG,	"INTRO <Node Name>", "Order an AI to play a scripted introductory animation at a node.", "msg CAI01 (INTRO ainodeintro11)" )
	ADD_MESSAGE_FLAGS( LEAD,					3,		ValidateMsgLead,		MSG_HANDLER( CAICommandMgr, HandleLeadMsg ),					0,						"LEAD <character to lead> <node to lead object to>", "This message allows you to task an AI to lead a character to a destination.  Currently only players are supported.  The destination object must be an AINodeLead instance.", "msg CAI01 (LEAD player ainodelead00)" )
	ADD_MESSAGE_FLAGS( MATERIAL,				3,		NULL,					MSG_HANDLER( CAICommandMgr, HandleMaterialMsg ),				0,						"MATERIAL <material index> <group index>", "Change a material on the AI model", "msg CAI01 (MATERIAL 3 1)" )
	ADD_MESSAGE_FLAGS( MENACE,					2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleMenaceMsg ),					CMDMGR_MF_BLOCKINGMSG,	"MENACE <Node Name>", "Order AI to a menace node.", "msg CAI01 (MENACE ainodemenace04)" )
	ADD_MESSAGE_FLAGS( MOVE,					2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleMoveMsg ),					0,						"MOVE <X,Y,Z>", "Move an AI to some position.", "msg CAI01 (MOVE 20,30,40)")
	ADD_MESSAGE_FLAGS( MOUNT,					-1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleMountMsg ),					0,						"MOUNT <Vehicle Name> <Vehicle Type> [Y-offset]", "Start a level with an AI already on a vehicle (rather than walking to a node and mounting).", "msg CAI01 (MOUNT motorcycle01 motorcycle 45)")
	ADD_MESSAGE_FLAGS( MOUNTATOBJECT,			4,		NULL, 					MSG_HANDLER( CAICommandMgr, HandleMountAtObjectMsg ),			0,						"MOUNTATOBJECT <WorldModel Name> <Position Object Name> <Vehicle Type>", "Mount a vehicle from a specific WorldModel", "msg CAI01 (MOUNTATOBJECT WorldModel01 45 motorcycle)")
	ADD_MESSAGE_FLAGS( MOUNTNODE,				2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleMountNodeMsg ),				0,						"MOUNTNODE <Node Name>", "Order an AI to mount a vehicle from some specific node.", "msg CAI01 (MOUNTNODE ainodevehicle00)")
	ADD_MESSAGE_FLAGS( NEVERDESTROY,			2,		NULL,					MSG_HANDLER( CAICommandMgr, NULLHandler ),						0,						"NEVERDESTROY <1 or 0>", "This message allows you to set the NeverDestroy flag through commands.  This message does not actually destroy the object but allows destruction if NeverDestroy was set to true initially.", "msg CAI01 (NEVERDESTROY 1)" )
	ADD_MESSAGE_FLAGS( OBSTACLEAVOIDANCE,		2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleObstacleAvoidanceMsg ),		0,						"OBSTACLEAVOIDANCE <1 or 0>", "Toggle obstacle avoidance.", "msg CAI01 (OBSTACLEAVOIDANCE 1)" )
	ADD_MESSAGE_FLAGS( PATROL,					2,		NULL,					MSG_HANDLER( CAICommandMgr, HandlePatrolMsg ),					0,						"PATROL <Node Name>", "Order an AI to patrol the route that includes some node.", "msg CAI01 (PATROL ainodepatrol03)" )
	ADD_MESSAGE_FLAGS( PICKUPWEAPON,			2,		NULL,					MSG_HANDLER( CAICommandMgr, HandlePickupWeaponMsg ),			CMDMGR_MF_BLOCKINGMSG,	"PICKUPWEAPON <Node Name>", "Order AI to pickup a weapon at some node.", "msg CAI01 (PICKUPWEAPON node03)" )
	ADD_MESSAGE_FLAGS( PLAYSOUND,				2,		NULL,					MSG_HANDLER( CAICommandMgr, HandlePlaySoundMsg ),				0,						"PLAYSOUND <sound>", "Play a sound.", "msg CAI01 (PLAYSOUND yell)")
	ADD_MESSAGE_FLAGS( PRESERVEACTIVATECMDS,	1,		NULL,					MSG_HANDLER( CAICommandMgr, HandlePreserveActivateCmdsMsg ),	0,						"PRESERVEACTIVATECMDS", "Informs the AI to hold onto their ACTIVATE messages after use.", "msg CAI01 PRESERVEACTIVATECMDS" )
	ADD_MESSAGE_FLAGS( PRIVATE_AMBUSH,			2,		NULL,					MSG_HANDLER( CAICommandMgr, HandlePrivate_AmbushMsg ),			CMDMGR_MF_BLOCKINGMSG,	"PRIVATE_AMBUSH <Node Name>", "Private internal message.", "<b>Do not use</b>" )
	ADD_MESSAGE_FLAGS( PRIVATE_COVER,			2,		NULL,					MSG_HANDLER( CAICommandMgr, HandlePrivate_CoverMsg ),			CMDMGR_MF_BLOCKINGMSG,	"PRIVATE_COVER <Node Name>", "Private internal message.", "<b>Do not use</b>" )
	ADD_MESSAGE_FLAGS( PRIVATE_EXCHANGEWEAPON,	2,		NULL,					MSG_HANDLER( CAICommandMgr, HandlePrivate_ExchangeWeaponMsg ),	CMDMGR_MF_BLOCKINGMSG,	"PRIVATE_EXCHANGEWEAPON <WeaponItem Name>", "Private internal message.", "<b>Do not use</b>" )
	ADD_MESSAGE_FLAGS( PRIVATE_SEARCH,			2,		NULL,					MSG_HANDLER( CAICommandMgr, HandlePrivate_SearchMsg ),			CMDMGR_MF_BLOCKINGMSG,	"PRIVATE_SEARCH <Node Name>", "Private internal message.", "<b>Do not use</b>" )
	ADD_MESSAGE_FLAGS( PRIVATE_SEARCH_LOST_TARGET, 1,	NULL,					MSG_HANDLER( CAICommandMgr, HandlePrivate_SearchLostTargetMsg ),CMDMGR_MF_BLOCKINGMSG,	"PRIVATE_SEARCH_LOST_TARGET", "Private internal message.", "<b>Do not use</b>" )
	ADD_MESSAGE_FLAGS( PRIVATE_SUPPRESSIONFIRE,	1,		NULL,					MSG_HANDLER( CAICommandMgr, HandlePrivate_SuppressionFireMsg ),	CMDMGR_MF_BLOCKINGMSG,	"PRIVATE_SUPPRESSIONFIRE", "Private internal message.", "<b>Do not use</b>" )
	ADD_MESSAGE_FLAGS( REMOVEGOAL,				2,		ValidateMsgRemoveGoal,	MSG_HANDLER( CAICommandMgr, HandleRemoveGoalMsg ),				0,						"REMOVEGOAL <goal>", "Remove a Goal from an AI.", "msg CAI01 (REMOVEGOAL killenemy)" )
	ADD_MESSAGE_FLAGS( REMOVEWEAPON,			2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleRemoveWeaponMsg ),			0,						"REMOVEWEAPON <weapon name>", "Remove a weapon from an AI", "msg CAI01 REMOVEGOAL" )
	ADD_MESSAGE_FLAGS( RIGIDBODY,				2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleRigidBodyMsg ),				0,						"RIGIDBODY <bool>", "Toggles the AI's physics weightset between RigidBody and it's default.", "msg CAI01 (RIGIDBODY 1)" )
	ADD_MESSAGE_FLAGS( SCAN,					2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleScanMsg ),					0,						"SCAN <Node Name>", "Similar to Patrol, this command assigns a Scanner a AINodeScan instance to use. Sending this command with any other object type will result in a crash.", "msg CAI01 (SCAN node03)" )
	ADD_MESSAGE_FLAGS( SEARCH,					-1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleSearchMsg ),					0,						"SEARCH <Node Name>", "AI will lead an orderly advance to a node that represents the search origin.  The squad will search from this node.", "msg CAI01 (SEARCH ainodegoto00)" )
	ADD_MESSAGE_FLAGS( SEEKENEMY,				1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleSeekEnemyMsg ),				0,						"SEEKENEMY", "Order AI to immediately seek out an enemy, without any stimulation.", "msg CAI01 SEEKENEMY")
	ADD_MESSAGE_FLAGS( SEEKSQUADENEMY,			1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleSeekSquadEnemyMsg ),			0,						"SEEKSQUADENEMY", "Same as seek enemy", "Same as seek enemy")
	ADD_MESSAGE_FLAGS( SENSES,					2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleSensesMsg ),					0,						"SENSES <1 or 0>", "Turn on or off an AI’s senses.  Senses may be turned off initially in Dedit AI attribute overrides.", "msg CAI01 (SENSES 0)")
	ADD_MESSAGE_FLAGS( SOLID,					2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleSolidMsg),					0,						"SOLID <bool>", "Toggles whether the AI is solid.", "msg <AIName> (SOLID 1)<BR>msg <AIName> (SOLID 0)")
	ADD_MESSAGE_FLAGS( SQUADNAMES,				1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleSquadNamesMsg),				0,						"SQUADNAMES", "Print the names of all squad members.", "msg <AIName> (SQUADNAMES)")
	ADD_MESSAGE_FLAGS( TARGET,					2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleTargetMsg ),					0,						"TARGET <Object Name", "Force AI to target some specific object.  The object may or may not be a character.", "msg CAI01 (TARGET player)")
	ADD_MESSAGE_FLAGS( TRACKAIMAT,				-1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleTrackAimAtMsg ),				0,						"TRACKAIMAT [Object Name]", "AI torso tracks the character.  If no character is specified, track the Player by default.", "msg CAI01 TRACKAIMAT<BR>msg CAI01 (TRACKAIMAT CAI00)" )
	ADD_MESSAGE_FLAGS( TRACKARM,				-1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleTrackArmMsg ),				0,						"TRACKARM [Object Name]", "AI tracks the character with his arm.  If no character is specified, track the Player by default.", "msg CAI01 TRACKARM<BR>msg CAI01 (TRACKARM CAI00)" )
	ADD_MESSAGE_FLAGS( TRACKLOOKAT,				-1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleTrackLookAtMsg ),				0,						"TRACKLOOKAT <Object Name>", "AI head tracks the character.  If no character is specified, track the Player by default.", "msg CAI01 TRACKLOOKAT<BR>msg CAI01 (TRACKLOOKAT CAI00)" )
	ADD_MESSAGE_FLAGS( TRACKNONE,				1,		NULL,					MSG_HANDLER( CAICommandMgr, HandleTrackNoneMsg ),				0,						"TRACKNONE", "AI turns off head or torso tracking.", "msg CAI01 TRACKNONE" )
	ADD_MESSAGE_FLAGS( TEAM,					2,		NULL,					MSG_HANDLER( CAICommandMgr, HandleTeamMsg ),					0,						"TEAM <0/1/2>", "Sets team for AI.  2 is no team", "msg <AIName> (TEAM 1)<BR>msg <AIName> (TEAM 0)" )

CMDMGR_END_REGISTER_CLASS_HANDLER( CAI, CCharacter, 0, MSG_HANDLER( CAICommandMgr, HandleAllMsgs ) )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAICommandMgr::CAICommandMgr()
{
	m_pAI = NULL;
}

CAICommandMgr::~CAICommandMgr()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAICommandMgr::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAICommandMgr
//              
//----------------------------------------------------------------------------
void CAICommandMgr::Save(ILTMessage_Write *pMsg)
{
	SAVE_COBJECT(m_pAI);
}

void CAICommandMgr::Load(ILTMessage_Read *pMsg)
{
	LOAD_COBJECT(m_pAI, CAI);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::InitAICommandMgr
//
//	PURPOSE:	Initialize AICommandMgr.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::InitAICommandMgr( CAI* pAI )
{
	m_pAI = pAI;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleAllMsgs
//
//	PURPOSE:	Handle all commands.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleAllMsgs( HOBJECT /*hSender*/, const CParsedMsg& crParsedMsg )
{
	switch( crParsedMsg.GetArgCount() )
	{
		case 1: AITRACE( AIShowMessages, ( m_pAI->m_hObject, "Received Msg: %s", crParsedMsg.GetArg( 0 ).c_str() ) );
			break;
		case 2: AITRACE( AIShowMessages, ( m_pAI->m_hObject, "Received Msg: %s %s", crParsedMsg.GetArg( 0 ).c_str(), crParsedMsg.GetArg( 1 ).c_str() ) );
			break;
		case 3: AITRACE( AIShowMessages, ( m_pAI->m_hObject, "Received Msg: %s %s %s", crParsedMsg.GetArg( 0 ).c_str(), crParsedMsg.GetArg( 1 ).c_str(), crParsedMsg.GetArg( 2 ).c_str() ) );
			break;
		case 4: AITRACE( AIShowMessages, ( m_pAI->m_hObject, "Received Msg: %s %s %s %s", crParsedMsg.GetArg( 0 ).c_str(), crParsedMsg.GetArg( 1 ).c_str(), crParsedMsg.GetArg( 2 ).c_str(), crParsedMsg.GetArg( 3 ).c_str() ) );
			break;
		default: AITRACE( AIShowMessages, ( m_pAI->m_hObject, "Received Msg: %s %s %s %s ...", crParsedMsg.GetArg( 0 ).c_str(), crParsedMsg.GetArg( 1 ).c_str(), crParsedMsg.GetArg( 2 ).c_str(), crParsedMsg.GetArg( 3 ).c_str() ) );
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::NULLHandler
//
//	PURPOSE:	NULL handler for messages that do no special processing in CAI.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::NULLHandler( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// This NULL handler prevents errors in the console about missing handler functions.
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleActivateMsg
//
//	PURPOSE:	Handle an Activate command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleActivateMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_pAI->Activate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleAddGoalMsg
//
//	PURPOSE:	Handle a ADDGOAL message...
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleAddGoalMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		EnumAIGoalType eGoalType = CAIGoalAbstract::GetGoalType( crParsedMsg.GetArg(1).c_str() );
		if( eGoalType == kGoal_InvalidType )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleAddGoalMsg: Unrecognized Goal type: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		m_pAI->GetGoalMgr()->AddGoal( eGoalType, g_pLTServer->GetTime() );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleAddWeaponMsg
//
//	PURPOSE:	Adds the passed in weapon and (optional) ammo pair to the 
//				AI.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleAddWeaponMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// trigger AI (AddWeapon weapon_name socket_name)

	HWEAPON hWeapon = NULL;

	const char* const pszWeaponName = crParsedMsg.GetArg(1);
	const char* const pszSocketName = crParsedMsg.GetArg(2);
	hWeapon = g_pWeaponDB->GetWeaponRecord( pszWeaponName );

	if (hWeapon != NULL)
	{
		m_pAI->GetAIWeaponMgr()->AddWeapon( hWeapon, pszSocketName, FAILURE_IS_ERROR );
		m_pAI->HandleArsenalChange();

		// Re-evaluate goals.

		m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleAdvance
//
//	PURPOSE:	Handle an Advance command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleAdvanceMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Order AI to advance to some node.

	ProcessNodeOrderMsg( crParsedMsg, kNode_Goto, kTask_Advance, !TARGET_ENEMY, TASK_SCRIPTED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleAlert
//
//	PURPOSE:	Handle an Alert command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleAlertMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	bool bAlert = true;
	if( crParsedMsg.GetArgCount() > 1 )
	{
		bAlert = IsTrueChar( *crParsedMsg.GetArg(1) );
	}

	// Clear any existing alert task.

	if( !bAlert )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Task );
		factQuery.SetTaskType( kTask_Alert );
		m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
	}

	// Create an Alert task.

	else 
	{
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Task );
		if( pFact )
		{
			pFact->SetTaskType( kTask_Alert, 1.f );
		}
	}

	// Re-evaluate goals.

	m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleAlignment
//
//	PURPOSE:	Handle an Alignment command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleAlignmentMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		EnumCharacterAlignment eAlignment = g_pCharacterDB->String2Alignment( crParsedMsg.GetArg(1) );
		if ( kCharAlignment_Invalid == eAlignment )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleAlignmentMsg: Unrecognized alignment type: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		// Persistent stimuli need to be re-registered with correct alignment.

		m_pAI->RemovePersistentStimuli();

		m_pAI->SetAlignment( eAlignment );

		// Invalidate our crosshair.  We'll rediscover our crosshair
		// status in ResetCrosshair.
		m_pAI->ResetCrosshair( kCharStance_Undetermined );

		// Invalidate the target; the target object may no longer be valid (ie
		// the AI was targeting a bad guy to kill them, but they are now a 
		// friend).  If the target changes, this will invalidate the AIs 
		// behavior/action, so we don't need to do this explicitly.  Only do 
		// this when the AI has a target, as this code path is used by the AI
		// at startup.
		if ( m_pAI->GetAIBlackBoard()->GetBBTargetObject() )
		{
			m_pAI->GetAIBlackBoard()->SetBBInvalidateTarget( true );
		}

		m_pAI->RegisterPersistentStimuli();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleAmbush
//
//	PURPOSE:	Handle an Ambush command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleAmbushMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Order AI to ambush at some node.

	ProcessNodeOrderMsg( crParsedMsg, kNode_Ambush, kTask_Ambush, TARGET_ENEMY, TASK_SCRIPTED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleAnimate
//
//	PURPOSE:	Handle an Animate command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleAnimateMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		// Clear any scripted animation.

		if( LTStrIEquals( crParsedMsg.GetArg(1), "None" ) )
		{
			// Clear any existing animate task.

			CAIWMFact factQuery;
			factQuery.SetFactType( kFact_Task );
			factQuery.SetTaskType( kTask_Animate );
			m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
			factQuery.SetTaskType( kTask_AnimateLoop );
			m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

			// Re-evaluate goals.

			m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
			return;
		}

		// Get a handle to the triggered animation.
		// Bail if it does not exist.

		HMODELANIM hAni = g_pLTServer->GetAnimIndex( m_pAI->m_hObject, crParsedMsg.GetArg(1) );
		if( INVALID_MODEL_ANIM == hAni )
		{
			ANIMERROR((m_pAI->GetHOBJECT(), m_pAI->GetModel(), "Could not find triggered animation: '%s'", crParsedMsg.GetArg(1).c_str()));
			return;
		}

		// Ensure that the AI has an animate goal.

		m_pAI->GetGoalMgr()->AddGoal( kGoal_Animate, g_pLTServer->GetTime() );

		// Clear any existing animate task.

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Task );
		factQuery.SetTaskType( kTask_Animate );
		m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
		factQuery.SetTaskType( kTask_AnimateLoop );
		m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

		// Create an Animate task.

		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Task );
		if( pFact )
		{
			// Animate has an optional second parameter: loop.

			static CParsedMsg::CToken s_cTok_Loop( "Loop" );
			if( ( crParsedMsg.GetArgCount() > 2 ) &&
				( crParsedMsg.GetArg(2) == s_cTok_Loop ) )
			{
				pFact->SetTaskType( kTask_AnimateLoop, 1.f );
			}
			else {
				pFact->SetTaskType( kTask_Animate, 1.f );
			}

			pFact->SetIndex( hAni, 1.f );
			pFact->SetFactFlags( kFactFlag_Scripted, 1.f );

			// Re-evaluate goals.

			m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleAwareness
//
//	PURPOSE:	Handle an Awareness command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleAwarenessMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		static CParsedMsg::CToken s_cTok_Relaxed( "Relaxed" );
		static CParsedMsg::CToken s_cTok_Suspicious( "Suspicious" );
		static CParsedMsg::CToken s_cTok_Alert( "Alert" );
		if( crParsedMsg.GetArg(1) == s_cTok_Relaxed )
		{
			m_pAI->GetAIBlackBoard()->SetBBAwareness( kAware_Relaxed );
		}
		else if( crParsedMsg.GetArg(1) == s_cTok_Suspicious )
		{
			m_pAI->GetAIBlackBoard()->SetBBAwareness( kAware_Suspicious );
		}
		else if( crParsedMsg.GetArg(1) == s_cTok_Alert )
		{
			m_pAI->GetAIBlackBoard()->SetBBAwareness( kAware_Alert );
		}
		else {
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleAwareness: Unrecognized awareness '%s'", crParsedMsg.GetArg(1).c_str() );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleAwarenessMod
//
//	PURPOSE:	Handle an AwarenessMod command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleAwarenessModMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		EnumAIAwarenessMod eAwarenessMod = StringToAwarenessMod( crParsedMsg.GetArg(1).c_str() );
		m_pAI->GetAIBlackBoard()->SetBBAwarenessMod( eAwarenessMod );

		// Some NavMeshLinks are no longer passable to a limping AI.

		m_pAI->GetPathKnowledgeMgr()->ClearPathKnowledge();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleBlitz
//
//	PURPOSE:	Handle an Blitz command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleBlitzMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	const char* const pszObjectName = crParsedMsg.GetArg( 1 );
	const char* const pszAIContextName = crParsedMsg.GetArg( 2 );

	// Get the objects involved.

	HOBJECT hTargetObject = NULL;
	if ( LT_OK != FindNamedObject( pszObjectName, hTargetObject ) )
	{
		AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleBlitzMsg: Could not find object: %s", pszObjectName );
		return;
	}

	EnumAIContext eContext = kContext_None;
	if ( crParsedMsg.GetArgCount() > 2 )
	{
		eContext = AIContextUtils::Enum( pszAIContextName, AIContextUtils::kNotFound_ReturnInvalid );
		if ( kContext_Invalid == eContext )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleBlitzMsg: Could not find AIContext: %s", pszAIContextName );
			return;
		}
	}

	// Determine the task type.

	ENUM_AIWMTASK_TYPE eTaskType = kTask_InvalidType;
	bool bForceTargetToTargetObject = false;
	if ( IsKindOf( hTargetObject, "CCharacter" ) )
	{
		eTaskType = kTask_BlitzCharacter;
		bForceTargetToTargetObject = true;
	}
	else if ( IsKindOf( hTargetObject, "AINodeSmartObject" ) )
	{
		eTaskType = kTask_BlitzUseNode;
	}
	else if ( IsKindOf( hTargetObject, "AINode" ) )
	{
		eTaskType = kTask_BlitzNode;
	}
	else 
	{
		AIASSERT( 0, m_pAI->m_hObject, "CAICommandMgr::HandleBlitzMsg: Target object is not an AINode or character." );
		return;
	}

	// Clear any existing task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( eTaskType );
	m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

	// Create a new task.

	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Task );
	if ( !pFact )
	{
		AIASSERT( 0, m_pAI->m_hObject, "CAICommandMgr::HandleBlitzMsg: Failed to allocate task." );
		return;
	}

	// Set up the new task.

	pFact->SetTaskType( eTaskType );
	pFact->SetTargetObject( hTargetObject );
	pFact->SetIndex( eContext );
	pFact->SetFactFlags( kFactFlag_Scripted, 1.f );

	// Re-evaluate goals.

	m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );

	//
	// If the AI was scripted to blitz a character, force the target to the 
	// character.  This should be cleaned up by the goal which handles this
	// fact.
	//

	if ( bForceTargetToTargetObject )
	{
		// Force the target to the targeted object.

		m_pAI->GetAIBlackBoard()->SetBBScriptedTargetObject( hTargetObject );

		// Re-evaluate target immediately.

		m_pAI->GetAIBlackBoard()->SetBBSelectTarget( true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleCanActivate
//
//	PURPOSE:	Handle a CanActivate command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleCanActivateMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// NOTE: In the future we should probably separate out CanTalk and CanActivate.
	// Currently talking and activation are initimitely coupled.

	bool bCanTalk = !!IsTrueChar( *crParsedMsg.GetArg(1) );
	m_pAI->SetCanTalk( bCanTalk );

	// Verify that we have stuff to say.

	if( bCanTalk && !m_pAI->HasActivateCommand() )
	{
		g_pLTServer->CPrint( "%s set to CANTALK, but has no Activate commands", m_pAI->GetName() );
	}

	m_pAI->UpdateUserFlagCanActivate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleCinefire
//
//	PURPOSE:	Handle a Cinefire command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleCinefireMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_pAI->GetAIWeaponMgr()->Cinefire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleCineract
//
//	PURPOSE:	Handle a Cineract command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleCineractMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		m_pAI->Cineract( crParsedMsg.GetArg(1), false );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleCineractLoop
//
//	PURPOSE:	Handle a CineractLoop command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleCineractLoopMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		m_pAI->Cineract( crParsedMsg.GetArg(1), true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleClearTask
//
//	PURPOSE:	Handle a ClearTask command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleClearTaskMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Task);
	m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleClearThreat
//
//	PURPOSE:	Handle a ClearThreat command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleClearThreatMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Clear knowledge of characters.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Character);
	m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

	// Clear knowledge of objects.

	factQuery.SetFactType(kFact_Object);
	m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

	// Re-evaluate target.

	m_pAI->GetAIBlackBoard()->SetBBInvalidateTarget( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleCover
//
//	PURPOSE:	Handle a Cover command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleCoverMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Order AI to cover at some node.

	ProcessNodeOrderMsg( crParsedMsg, kNode_Cover, kTask_Cover, TARGET_ENEMY, TASK_SCRIPTED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleDamageMask
//
//	PURPOSE:	Handle a DamageMask command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleDamageMaskMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		m_pAI->SetDamageMask( crParsedMsg.GetArg(1) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleDebugDamage
//
//	PURPOSE:	Handle a DebugDamage message.  This message should only be
//				sent for debugging purposes.  It deals damage described
//				by the message to the AI.
//
// ----------------------------------------------------------------------- //
void CAICommandMgr::HandleDebugDamage( HOBJECT hSender, const CParsedMsg& crParsedMsg )
{
	DamageStruct damage;

	// Set up the default values in case we don't find an overload below.

	damage.eType	= DT_UNSPECIFIED;
	damage.fDamage	= damage.kInfiniteDamage;
	damage.hDamager = NULL;

	// Check for overrides

	if (crParsedMsg.GetArgCount() > 1)
	{
		damage.fDamage = (float)atof(crParsedMsg.GetArg(1).c_str());
	}

	if (crParsedMsg.GetArgCount() > 2)
	{
		const char* const szDamageTypeName = crParsedMsg.GetArg(2).c_str();
		damage.eType = StringToDamageType(szDamageTypeName);
	}

	// Apply the damage
	damage.DoDamage(hSender, m_pAI->GetHOBJECT());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleDebugActions
//
//	PURPOSE:	Handle a DebugActions message.  This prints out all of the 
//				actions an AI can use.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleDebugActions( HOBJECT hSender, const CParsedMsg& crMsg )
{
	int nActions = g_pAIActionMgr->GetNumAIActions();
	g_pLTServer->CPrint( "ActionSet: %s", m_pAI->GetAIAttributes()->strAIActionSet.c_str() );
	for (int i = 0; i < nActions; ++i)
	{
		if (g_pAIActionMgr->IsActionInAIActionSet(m_pAI->GetAIBlackBoard()->GetBBAIActionSet(), (EnumAIActionType)i))
		{
			if ( i >= 0 && i <  kAct_Count)
			{
				g_pLTServer->CPrint( "\t%s", s_aszActionTypes[i] );
			}
		}
	}
	g_pLTServer->CPrint( "\n" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleDebugGoals
//
//	PURPOSE:	Handle a DebugGoals message.  This prints out all of the 
//				Goals an AI can use.  This functionality was added because
//				individual goals can be added/removed.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleDebugGoals( HOBJECT hSender, const CParsedMsg& crMsg )
{
	// Print the goalset by name.

	AIDB_GoalSetRecord* pGoalSetRecord = g_pAIDB->GetAIGoalSetRecord( m_pAI->GetGoalMgr()->GetGoalSetIndex() );
	g_pLTServer->CPrint( "Goalset: %s\n", (pGoalSetRecord ? pGoalSetRecord->strName.c_str() : "None") );

	// Print the goal type of each of the goals the AI has.

	AIGOAL_LIST::const_iterator itEachGoal = m_pAI->GetGoalMgr()->BeginGoals();
	AIGOAL_LIST::const_iterator itEndGoal = m_pAI->GetGoalMgr()->EndGoals();
	while (itEachGoal != itEndGoal)
	{
		if (*itEachGoal)
		{
			g_pLTServer->CPrint( "\t%s", CAIGoalAbstract::GetGoalTypeName((*itEachGoal)->GetGoalType()) );
		}

		++itEachGoal;
	}

	g_pLTServer->CPrint( "\n" );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleDebugNodesOfType
//
//	PURPOSE:	Handle a DebugNodesOfType message.  This message prints 
//				out all nodes of a particular type that an AI knows 
//				about, along with a validity (note that the validity may 
//				not reflect the test which truly occurs when determining
//				when to use the node)
//
// ----------------------------------------------------------------------- //

struct AllNodesOfType
{
	typedef std::vector<HOBJECT, LTAllocator<HOBJECT, LT_MEM_TYPE_OBJECTSHELL> > ObjectListType;

	AllNodesOfType(EnumAINodeType eNodeType)
	{
		m_eNodeType = eNodeType;
	}

	int operator()(const CAIWMFact* pFact)
	{
		if ( kFact_Node != pFact->GetFactType() 
			|| pFact->GetNodeType() != m_eNodeType)
		{
			return 0;
		}

		m_NodeList.push_back(pFact->GetTargetObject());
		return 0;
	}

	EnumAINodeType m_eNodeType;
	ObjectListType m_NodeList;
};

void CAICommandMgr::HandleDebugNodesOfType( HOBJECT hSender, const CParsedMsg& crMsg )
{
	// Translate the node name into a node type
	
	EnumAINodeType eType = AINodeUtils::GetNodeType(crMsg.GetArg(1).c_str());
	if (kNode_InvalidType == eType)
	{
		g_pLTServer->CPrint( "Invalid node type: %s", crMsg.GetArg(1).c_str() );
		return;
	}

	// Gather all of the nodes of this type that the AI knows about

	AllNodesOfType Collector(eType);
	m_pAI->GetAIWorkingMemory()->CollectFact(Collector);

	// Print out the status of each node

	g_pLTServer->CPrint( "NodeOfType: %s, %d found", crMsg.GetArg(1).c_str(), Collector.m_NodeList.size() );
	AllNodesOfType::ObjectListType::iterator itEach = Collector.m_NodeList.begin();
	for (;itEach != Collector.m_NodeList.end(); ++itEach)
	{
		AINode* pNode = AINode::HandleToObject(*itEach);
		bool bValid = false;
		if (pNode)
		{
			bValid = pNode->IsNodeValid(m_pAI, m_pAI->GetPosition(), NULL, kThreatPos_TruePos, kNodeStatus_All);
		}
		g_pLTServer->CPrint( "	\t%s\t%s", pNode->GetNodeName(), bValid ? "Valid" : "Invalid" );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleDebugSensors
//
//	PURPOSE:	Handle a DebugSensors message.  This message prints 
//				out all of the types of sensors an AI has.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleDebugSensors( HOBJECT hSender, const CParsedMsg& crMsg )
{
	g_pLTServer->CPrint( "Sensors:" );
	for (int i = 0; i < kSensor_Count; ++i)
	{
		if (NULL != m_pAI->GetAISensorMgr()->FindSensor( (EnumAISensorType)i ))
		{
			g_pLTServer->CPrint( "\t%s ", CAISensorAbstract::GetSensorTypeName((EnumAISensorType)i) );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleDebugShoveMsg
//
//	PURPOSE:	Handle a debug shove message.  This command will cause the 
//				AI to react to a shove from the players (senders) direction.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleDebugShoveMsg( HOBJECT hSender, const CParsedMsg& crMsg )
{
	StimulusRecordCreateStruct scs( kStim_Shoved, m_pAI->GetAlignment(), m_pAI->GetPosition(), m_pAI->GetHOBJECT() ); 
	scs.m_hStimulusTarget = hSender;
	g_pAIStimulusMgr->RegisterStimulus( scs );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleDelCmd
//
//	PURPOSE:	Handle a DelCmd command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleDelCmdMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() == 1 )
	{
		g_pLTServer->CPrint( "DELCMD missing argument" );
		return;
	}

	static CParsedMsg::CToken s_cTok_ActivateOn( "ACTIVATEON" );
	static CParsedMsg::CToken s_cTok_ActivateOff( "ACTIVATEOFF" );

	if( crParsedMsg.GetArg(1) == s_cTok_ActivateOn )
	{
		m_pAI->ClearActivateOnCommand();
		m_pAI->UpdateUserFlagCanActivate();
	}
	else if( crParsedMsg.GetArg(1) == s_cTok_ActivateOff )
	{
		m_pAI->ClearActivateOffCommand();
		m_pAI->UpdateUserFlagCanActivate();
	}
	else
	{
		g_pLTServer->CPrint( "DELCMD %s - invalid command to delete", crParsedMsg.GetArg(1).c_str() );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleDesire
//
//	PURPOSE:	Handle a Desire command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleDesireMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 2 )
	{
		// Desire type.

		ENUM_AIWMDESIRE_TYPE eDesire = kDesire_InvalidType;
		if( LTStrIEquals( crParsedMsg.GetArg(1), "UNCLOAK" ) )
		{
			eDesire = kDesire_Uncloak;
		}
		else if( LTStrIEquals( crParsedMsg.GetArg(1), "NEVERCLOAK" ) )
		{
			eDesire = kDesire_NeverCloak;
		}

		// Desire is valid.

		if( eDesire != kDesire_InvalidType )
		{
			// Last parameter is the confidence.

			float fConfidence = (float)atof( crParsedMsg.GetArg(2) );

			CAIWMFact factQuery;
			factQuery.SetFactType( kFact_Desire );
			factQuery.SetDesireType( eDesire );

			// Clear the desire.

			if( fConfidence <= 0.f )
			{
				m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
			}

			// Create or update the desire.

			else
			{
				CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
				if( !pFact )
				{
					pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Desire );
				}
				if( pFact )
				{
					pFact->SetDesireType( eDesire, fConfidence );
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleDropWeaponsMsg
//
//	PURPOSE:	Handle a DropWeapons message.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleDropWeaponsMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_pAI->GetAIWeaponMgr()->HandleDropWeapons();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleDismount
//
//	PURPOSE:	Handle a Dismount command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleDismountMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Ensure that the AI has a DismountVehicle goal.

	m_pAI->GetGoalMgr()->AddGoal( kGoal_DismountVehicle, g_pLTServer->GetTime() );

	// Create a DismountVehicle task.

	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Task );
	if( pFact )
	{
		pFact->SetTaskType( kTask_DismountVehicle, 1.f );
		pFact->SetFactFlags( kFactFlag_Scripted, 1.f );

		// Re-evaluate goals.

		m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleFacePlayer
//
//	PURPOSE:	Handle a FacePlayer command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleFacePlayerMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
	if ( pPlayer )
	{

		m_pAI->GetAIBlackBoard()->SetBBFaceObject( pPlayer->m_hObject );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleFollow
//
//	PURPOSE:	Handle a Follow command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleFollowMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		// Clear any existing task.

		if( LTStrIEquals( crParsedMsg.GetArg(1), "None" ) )
		{
			CAIWMFact factQuery;
			factQuery.SetFactType( kFact_Task );
			factQuery.SetTaskType( kTask_Follow );
			m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

			// Re-evaluate goals.

			m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );

			// Re-evaluate target.

			m_pAI->GetAIBlackBoard()->SetBBInvalidateTarget( true );
			return;
		}

		// Find the named object.
		// Bail if it does not exist.

		HOBJECT hTarget;
		if( LT_OK != FindNamedObject( crParsedMsg.GetArg(1), hTarget ) )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleFollow: Could not find object: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		// Clear any existing task.

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Task );
		factQuery.SetTaskType( kTask_Follow );
		m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

		// Create a new task.

		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Task );
		if( pFact )
		{
			pFact->SetTaskType( kTask_Follow, 1.f );
			pFact->SetTargetObject( hTarget, 1.f );

			// Second argument represents the following order.

			pFact->SetIndex( 0, 1.f );
			if( crParsedMsg.GetArgCount() > 2 )
			{
				int nPosition = atoi( crParsedMsg.GetArg(2) );
				pFact->SetIndex( nPosition, 1.f );
			}

			// Last argument represents the leader of the orderly advance.

			pFact->SetSourceObject( hTarget, 1.f );
			if( crParsedMsg.GetArgCount() > 3 )
			{
				HOBJECT hLeader = NULL;
				if( LT_OK != FindNamedObject( crParsedMsg.GetArg(3), hLeader ) )
				{
					AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleFollow: Could not find leader: %s", crParsedMsg.GetArg(2).c_str() );
				}
				pFact->SetSourceObject( hLeader, 1.f );
			}

			// Re-evaluate goals.

			m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );

			// Re-evaluate target.

			m_pAI->GetAIBlackBoard()->SetBBInvalidateTarget( true );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleForceGround
//
//	PURPOSE:	Handle a ForceGround command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleForceGroundMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		m_pAI->SetForceGround( IsTrueChar( *crParsedMsg.GetArg(1) ) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleGoalSet
//
//	PURPOSE:	Handle a GoalSet command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleGoalSetMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		m_pAI->GetGoalMgr()->SetGoalSet( crParsedMsg.GetArg(1), m_pAI->GetName(), true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleGoto
//
//	PURPOSE:	Handle a Goto command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleGotoMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		// Clear any existing task.

		if( LTStrIEquals( crParsedMsg.GetArg(1), "None" ) )
		{
			CAIWMFact factQuery;
			factQuery.SetFactType( kFact_Task );
			factQuery.SetTaskType( kTask_Goto );
			m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

			// Re-evaluate goals.

			m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
			return;
		}

		// Find the named node.
		// Bail if it does not exist.

		HOBJECT hNode;
		if( LT_OK != FindNamedObject( crParsedMsg.GetArg(1), hNode ) )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleGoto: Could not find node: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		if ( hNode && !IsKindOf(hNode, "AINode") )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleGotoMsg: Object is not an AI node: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
		if( !pNode )
		{
			return;
		}

		// Ensure that the AI has a goto goal.

		m_pAI->GetGoalMgr()->AddGoal( kGoal_TraverseLink, g_pLTServer->GetTime() );
		m_pAI->GetGoalMgr()->AddGoal( kGoal_Goto, g_pLTServer->GetTime() );

		// Clear any existing goto task.

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Task );
		factQuery.SetTaskType( kTask_Goto );
		m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

		// Create a Goto task.

		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Task );
		if( pFact )
		{
			pFact->SetTaskType( kTask_Goto, 1.f );
			pFact->SetTargetObject( hNode, 1.f );
			pFact->SetNodeType( pNode->GetType(), 1.f );
			pFact->SetPos( pNode->GetPos(), 1.f );
			pFact->SetFactFlags( kFactFlag_Scripted, 1.f );

			// Re-evaluate goals.

			m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleGuard
//
//	PURPOSE:	Handle a Guard command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleGuardMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		// Find the named node.
		// Bail if it does not exist.

		HOBJECT hNewGuardNode;
		if( LT_OK != FindNamedObject( crParsedMsg.GetArg(1), hNewGuardNode ) )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleGuard: Could not find node: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		if ( hNewGuardNode && !IsKindOf(hNewGuardNode, "AINode") )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleGuard: Object is not an AI node: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		AINode* pNewGuardNode = (AINode*)g_pLTServer->HandleToObject( hNewGuardNode );
		if( !pNewGuardNode )
		{
			return;
		}

		// Bail if node is not a Guard node.

		if( pNewGuardNode->GetType() != kNode_Guard )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleGuard: Node '%s' is node a Guard Node", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		// Clear current node owner's memory of guard node.

		CAIWMFact* pFact;
		if( IsAI( pNewGuardNode->GetNodeOwner() ) )
		{
			CAI* pOldOwner = (CAI*)g_pLTServer->HandleToObject( pNewGuardNode->GetNodeOwner() );
			if( pOldOwner )
			{
				CAIWMFact factQuery;
				factQuery.SetFactType(kFact_Node);
				factQuery.SetNodeType(kNode_Guard);
				pOldOwner->GetAIWorkingMemory()->ClearWMFact( factQuery );
			}
		}

		// Take immediate ownership of the node.

		pNewGuardNode->SetNodeOwner( m_pAI->m_hObject );

		// Find an existing guard node memory.

		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Node);
		factQuery.SetNodeType(kNode_Guard);
		pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact )
		{
			// Clear the owner of this guard node.

			AINodeGuard* pGuardNode = (AINodeGuard*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
			if( pGuardNode && ( pGuardNode->GetNodeOwner() == m_pAI->m_hObject ) )
			{
				pGuardNode->SetNodeOwner( NULL );
			}
		}		

		// Create a memory for the guard node.

		if( !pFact )
		{
			pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Node );
		}

		if( pFact )
		{
			pFact->SetNodeType( kNode_Guard, 1.f );
			pFact->SetTargetObject( pNewGuardNode->m_hObject, 1.f );
			pFact->SetPos( pNewGuardNode->GetPos(), 1.f );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleHidePieceMsg
//
//	PURPOSE:	Handle a HidePiece command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleHidePieceMsg(HOBJECT hSender, const CParsedMsg &crParsedMsg)
{
	if( crParsedMsg.GetArgCount() > 2 )
	{
		HMODELPIECE hPiece = (HMODELPIECE)NULL;	
		if( LT_OK == g_pModelLT->GetPiece( m_pAI->m_hObject, crParsedMsg.GetArg(1), hPiece ) )
		{
			// hide it
			LTRESULT ltResult;
			bool bHiddenStatus = IsTrueChar( *crParsedMsg.GetArg(2) );
			ltResult = g_pModelLT->SetPieceHideStatus( m_pAI->m_hObject, hPiece, bHiddenStatus );
			AIASSERT( ( LT_OK == ltResult) || ( LT_NOCHANGE == ltResult ), m_pAI->m_hObject, "CAICommandMgr::HandlePieceMsg : Failed to set piece status." );
		}
		else
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandlePieceMsg : No such piece on model: %s", crParsedMsg.GetArg(1).c_str() );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleIntro
//
//	PURPOSE:	Handle an Intro command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleIntroMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Order AI to play an intro animation at some node.

	ProcessNodeOrderMsg( crParsedMsg, kNode_Intro, kTask_Intro, !TARGET_ENEMY, TASK_SCRIPTED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleLeadMsg
//
//	PURPOSE:	Handle a Lead command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleLeadMsg(HOBJECT hSender, const CParsedMsg &crParsedMsg)
{
	// Verify the message, translating it into objects.

	HOBJECT hFollower = NULL;
	const char* pszFollowerName = crParsedMsg.GetArg(1).c_str();
	if( LT_OK != FindNamedObject( pszFollowerName, hFollower ) )
	{
		AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleLeadMsg: Could not find follower object: %s", pszFollowerName );
		return;
	}

	HOBJECT hDestination = NULL;
	const char* pszDestinationName = crParsedMsg.GetArg(2).c_str();
	if( LT_OK != FindNamedObject( pszDestinationName, hDestination ) )
	{
		AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleLeadMsg: Could not find follower object: %s", pszDestinationName );
		return;
	}

	// Clear any existing LeadCharacter tasks.

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Task );
	queryFact.SetTaskType( kTask_LeadCharacter );
	m_pAI->GetAIWorkingMemory()->ClearWMFacts( queryFact );

	// Create a new LeadCharacter task.

	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Task );
	if ( pFact )
	{
		pFact->SetTaskType( kTask_LeadCharacter );
		pFact->SetSourceObject( hFollower );
		pFact->SetTargetObject( hDestination );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleMove
//
//	PURPOSE:	Handle a Material command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleMaterialMsg(HOBJECT hSender, const CParsedMsg &crParsedMsg)
{
	if( crParsedMsg.GetArgCount() >= 2 )
	{
		if( crParsedMsg.GetArgCount() >= 2 )
		{
			// Get the material index to change, and the group to change it to.
			int iMaterialIndex = atoi(crParsedMsg.GetArg(1));
			int iGroup = atoi(crParsedMsg.GetArg(2));

			// Get the filename in the animation group at the requested index.

			int iGroupStride = GetObjectMaterialCount(m_pAI->m_hObject);
			uint32 iButeMaterialListIndex = iGroup*iGroupStride + iMaterialIndex;

			// Set the selected material.

			if (iButeMaterialListIndex >= 0 && 
				iButeMaterialListIndex < g_pModelsDB->GetNumMaterials(m_pAI->GetModel()))
			{
				const char* pszFileName = g_pModelsDB->GetMaterialFilename(m_pAI->GetModel(), iButeMaterialListIndex);
				g_pModelLT->SetMaterialFilename(m_pAI->m_hObject, iMaterialIndex, pszFileName);
			}
			else
			{
				AIASSERT1(0, m_pAI->m_hObject, "CAICommandMgr::HandleMaterialMsg : Material Index out of bounds: %d", iButeMaterialListIndex);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleMenaceMsg
//
//	PURPOSE:	Handle an Menace command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleMenaceMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Order AI to ambush at some node.

	ProcessNodeOrderMsg( crParsedMsg, kNode_MenacePlace, kTask_Menace, TARGET_ENEMY, TASK_SCRIPTED );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleMount
//
//	PURPOSE:	Handle a Mount command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleMountMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 2 )
	{
		// Find the named vehicle.
		// Bail if it does not exist.

		HOBJECT hVehicle = NULL;
		if( LT_OK != FindNamedObject( crParsedMsg.GetArg(1), hVehicle ) )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleMount: Could not find vehicle: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		// Bail if vehicle type does not match an anim prop.

		std::string strVehicle = "ATVT_";
		strVehicle += crParsedMsg.GetArg(2);

		EnumAnimProp eVehicle = GetAnimationPropFromName( strVehicle.c_str() );
		if( ( eVehicle == kAP_Invalid ) ||
			( eVehicle == kAP_None ) )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleMount: Invalid vehicle: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		// Record the vehicle object that the AI is attached to on the Blackboard.

		m_pAI->GetAIBlackBoard()->SetBBAttachedTo( hVehicle );

		// Record the vehicle type in the WorldState.

		m_pAI->GetAIWorldState()->SetWSProp( kWSK_RidingVehicle, m_pAI->m_hObject, kWST_EnumAnimProp, eVehicle );

		// Re-evaluate goals.

		m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
		m_pAI->GetAIBlackBoard()->SetBBInvalidatePlan( true );

		// Move AI up some optional Y-offset.

		if( crParsedMsg.GetArgCount() > 3 )
		{
			// Move the AI immediately (rather than with the Move() function),
			// because the ATTACH command will use the current relative offset.

			float fOffset = (float)atof( crParsedMsg.GetArg(3) );
			if( fOffset != 0.f )
			{
				LTVector vPos = m_pAI->GetPosition();
				vPos.y += (float)atof( crParsedMsg.GetArg(3) );
				g_pLTServer->Physics()->MoveObject( m_pAI->m_hObject, vPos, 0 );
			}
		}

		// Assign AI a KeyframeToRigidBody object, for detaching on death.

		if( crParsedMsg.GetArgCount() > 4 )
		{
			HOBJECT hKeyframeToRigidBody;
			if( LT_OK == FindNamedObject( crParsedMsg.GetArg(4), hKeyframeToRigidBody ) )
			{
				m_pAI->GetAIBlackBoard()->SetBBVehicleKeyframeToRigidBody( hKeyframeToRigidBody );
			}
		}

		// Attach AI to the vehicle.

		std::string strCmd = "ATTACH ";
		strCmd += m_pAI->GetName();
		g_pCmdMgr->QueueMessage( m_pAI, g_pLTServer->HandleToObject(hVehicle), strCmd.c_str() );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleMountAtObjectMsg
//
//	PURPOSE:	Handle a MountNode command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleMountAtObjectMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	//
	// Read the parameter names.
	//

	const char* const pszWorldModelObjectName	= crParsedMsg.GetArg( 1 );
	const char* const pszPositionObjectName		= crParsedMsg.GetArg( 2 );
	const char* const pszActivity				= crParsedMsg.GetArg( 3 );

	//
	// Convert parameter names into data and validate.
	//

	HOBJECT hWorldModel = NULL;
	if( LT_OK != FindNamedObject( pszWorldModelObjectName, hWorldModel ) )
	{
		AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleMountAtObjectMsg: Could not find WorldModel: %s", pszWorldModelObjectName );
		return;
	}
	WorldModel* pWorldModel = WorldModel::DynamicCast( hWorldModel );
	if ( NULL == pWorldModel )
	{
		AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleMountAtObjectMsg: Object %s is not a WorldModel: ", pszWorldModelObjectName );
		return;
	}

	HOBJECT hPositionObject = NULL;
	if( LT_OK != FindNamedObject( pszPositionObjectName, hPositionObject ) )
	{
		AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleMountAtObjectMsg: Could not find position object: %s", pszPositionObjectName );
		return;
	}
	LTVector vPosition;
	g_pLTServer->GetObjectPos( hPositionObject, &vPosition );

	std::string strActivity = "ATVT_";
	strActivity += pszActivity;
	EnumAnimProp eVehicle = GetAnimationPropFromName( strActivity.c_str() );

	//
	// Apply the messages effects.
	//

	// Record the vehicle object that the AI is attached to on the Blackboard.

	m_pAI->GetAIBlackBoard()->SetBBAttachedTo( hWorldModel );

	// Record the vehicle type in the WorldState.

	m_pAI->GetAIWorldState()->SetWSProp( kWSK_RidingVehicle, m_pAI->m_hObject, kWST_EnumAnimProp, eVehicle );

	// Re-evaluate goals.

	m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
	m_pAI->GetAIBlackBoard()->SetBBInvalidatePlan( true );

	// Instantly move the AI to the objects position

	g_pLTServer->Physics()->MoveObject( m_pAI->m_hObject, vPosition, 0 );

	// Attach AI to the vehicle.  Normally we would rely on a queued message
	// to do this.  Queued messages don't get dispatched for a frame however.
	// This may be problematic if the worldmodel is moving at a high rate -- 
	// we will end up with an offset

	pWorldModel->AttachObject( m_pAI->GetHOBJECT() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleMountNode
//
//	PURPOSE:	Handle a MountNode command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleMountNodeMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		// Find the named node.
		// Bail if it does not exist.

		HOBJECT hNode;
		if( LT_OK != FindNamedObject( crParsedMsg.GetArg(1), hNode ) )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleMountNode: Could not find node: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		if ( hNode && !IsKindOf(hNode, "AINode") )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleMountNodeMsg: Object is not an AI node: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
		if( !pNode )
		{
			return;
		}

		// Ensure that the AI has a MountVehicle goal.

		m_pAI->GetGoalMgr()->AddGoal( kGoal_MountVehicle, g_pLTServer->GetTime() );

		// Clear any existing MountVehicle tasks.

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Task );
		factQuery.SetTaskType( kTask_MountVehicle );
		m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

		// Create a MountVehicle task.

		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Task );
		if( pFact )
		{
			pFact->SetTaskType( kTask_MountVehicle, 1.f );
			pFact->SetTargetObject( hNode, 1.f );
			pFact->SetNodeType( pNode->GetType(), 1.f );
			pFact->SetPos( pNode->GetPos(), 1.f );
			pFact->SetFactFlags( kFactFlag_Scripted, 1.f );

			// Re-evaluate goals.

			m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleMove
//
//	PURPOSE:	Handle a Move command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleMoveMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		LTVector vPos;
		sscanf( crParsedMsg.GetArg(1), "%f,%f,%f", &vPos.x, &vPos.y, &vPos.z );
		vPos += m_pAI->GetPosition();

		g_pLTServer->Physics()->MoveObject( m_pAI->m_hObject, vPos, 0 );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleObstacleAvoidance
//
//	PURPOSE:	Handle an ObstacleAvoidance command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleObstacleAvoidanceMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		uint32 dwFlags = m_pAI->GetAIBlackBoard()->GetBBMovementCollisionFlags();
		if( IsTrueChar( *crParsedMsg.GetArg(1) ) )
		{
			dwFlags |= kAIMovementFlag_ObstacleAvoidance;
		}
		else {
			dwFlags &= ~kAIMovementFlag_ObstacleAvoidance;
		}

		m_pAI->GetAIBlackBoard()->SetBBMovementCollisionFlags( dwFlags );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandlePatrol
//
//	PURPOSE:	Handle a Patrol command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandlePatrolMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		// Find the named node.
		// Bail if it does not exist.

		HOBJECT hNewPatrolNode;
		if( LT_OK != FindNamedObject( crParsedMsg.GetArg(1), hNewPatrolNode ) )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandlePatrol: Could not find node: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		AINodePatrol* pNewPatrolNode = (AINodePatrol*)g_pLTServer->HandleToObject( hNewPatrolNode );
		if( !pNewPatrolNode )
		{
			return;
		}

		// Clear current node owner's memory of patrol node.

		CAIWMFact* pFact;
		if( IsAI( pNewPatrolNode->GetNodeOwner() ) )
		{
			CAI* pOldOwner = (CAI*)g_pLTServer->HandleToObject( pNewPatrolNode->GetNodeOwner() );
			if( pOldOwner )
			{
				CAIWMFact factQuery;
				factQuery.SetFactType(kFact_Node);
				factQuery.SetNodeType(kNode_Patrol);
				pOldOwner->GetAIWorkingMemory()->ClearWMFact( factQuery );

				// Unlock the patrol path.

				pNewPatrolNode->ClaimPatrolPath( pOldOwner, false );
			}
		}

		// Take immediate ownership of the node.

		pNewPatrolNode->SetNodeOwner( m_pAI->m_hObject );

		// Find an existing patrol node memory.

		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Node);
		factQuery.SetNodeType(kNode_Patrol);
		pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact )
		{
			// Clear the owner of this patrol node.

			AINodePatrol* pPatrolNode = (AINodePatrol*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
			if( pPatrolNode && ( pPatrolNode->GetNodeOwner() == m_pAI->m_hObject ) )
			{
				pPatrolNode->SetNodeOwner( NULL );

				// Unlock the old patrol path.

				pPatrolNode->ClaimPatrolPath( m_pAI, false );
			}
		}

		// Lock the new patrol path.

		pNewPatrolNode->ClaimPatrolPath( m_pAI, true );

		// Create a memory for the patrol node.

		if( !pFact )
		{
			pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Node );
		}

		if( pFact )
		{
			pFact->SetNodeType( kNode_Patrol, 1.f );
			pFact->SetTargetObject( pNewPatrolNode->m_hObject, 1.f );
			pFact->SetPos( pNewPatrolNode->GetPos(), 1.f );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandlePickupWeaponMsg
//
//	PURPOSE:	Handle an PickupWeapon command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandlePickupWeaponMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Order AI to pickup a weapon at some node.

	ProcessNodeOrderMsg( crParsedMsg, kNode_PickupWeapon, kTask_PickupWeapon, TARGET_ENEMY, TASK_SCRIPTED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandlePlaySound
//
//	PURPOSE:	Handle a PlaySound command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandlePlaySoundMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		m_pAI->PlayDialogSound( crParsedMsg.GetArg(1) );
	}
	else
	{
		g_pLTServer->CPrint( "PLAYSOUND missing argument" );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandlePreserveActivateCmds
//
//	PURPOSE:	Handle a PreserveActivateCmds command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandlePreserveActivateCmdsMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_pAI->SetPreserveActivateCmds( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandlePrivate_Ambush
//
//	PURPOSE:	Handle a Private_Ambush command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandlePrivate_AmbushMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Order AI to ambush at some node.

	ProcessNodeOrderMsg( crParsedMsg, kNode_Ambush, kTask_Ambush, TARGET_ENEMY, !TASK_SCRIPTED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandlePrivate_Cover
//
//	PURPOSE:	Handle a Private_Cover command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandlePrivate_CoverMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Order AI to cover at some node.

	ProcessNodeOrderMsg( crParsedMsg, kNode_Cover, kTask_Cover, TARGET_ENEMY, !TASK_SCRIPTED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandlePrivate_ExchangeWeaponMsg
//
//	PURPOSE:	Handle a Private_ExchangeWeapon command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandlePrivate_ExchangeWeaponMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Sanity Check.

	if( crParsedMsg.GetArgCount() <= 1 )
	{
		return;
	}

	// Find the WeaponItem.  This may be NULL, as it may have already been picked up.
	// (this should be rare - it should only happen when the WeaponItem is picked up
	// between the time the message is sent and message is handled).

	HOBJECT hWeaponItem = NULL;
	if( LT_OK != FindNamedObject( crParsedMsg.GetArg(1), hWeaponItem ) )
	{
		return;
	}

	if ( !hWeaponItem )
	{
		return;
	}

	// Create a ExchangeWeapon task.  This does NOT require re-evaluating the
	// goals/actions/target, as this is handled by the sensor which responds 
	// to this task.

	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Task );
	if( pFact )
	{
		// Set the task type.

		pFact->SetTaskType( kTask_ExchangeWeapon, 1.f );
		
		// Set the named WeaponItem as the target.

		pFact->SetTargetObject( hWeaponItem );

		// Task is not scripted.

		pFact->SetFactFlags( kFactFlag_None, 1.f );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandlePrivate_Search
//
//	PURPOSE:	Handle a Private_Search command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandlePrivate_SearchMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Order AI to search at some node.

	ProcessNodeOrderMsg( crParsedMsg, kNode_Search, kTask_Search, !TARGET_ENEMY, !TASK_SCRIPTED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandlePrivate_SearchLostTarget
//
//	PURPOSE:	Handle a Private_SearchLostTarget command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandlePrivate_SearchLostTargetMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Clear any existing search task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_Search );
	m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

	// Create a Search task.

	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Task );
	if( pFact )
	{
		pFact->SetTaskType( kTask_Search, 1.f );

		// Re-evaluate goals.

		m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandlePrivate_SuppressionFire
//
//	PURPOSE:	Handle a Private_SuppressionFire command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandlePrivate_SuppressionFireMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Clear any existing SuppressionFire task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_SuppressionFire );
	m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

	// Create a SuppressionFire task.

	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Task );
	if( pFact )
	{
		pFact->SetTaskType( kTask_SuppressionFire, 1.f );

		// Task is not scripted.

		pFact->SetFactFlags( kFactFlag_None, 1.f );

		// Re-evaluate goals.

		m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleRemoveGoalMsg
//
//	PURPOSE:	Handle a REMOVEGOAL message...
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleRemoveGoalMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		EnumAIGoalType eGoalType = CAIGoalAbstract::GetGoalType( crParsedMsg.GetArg(1).c_str() );
		if( eGoalType == kGoal_InvalidType )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleRemoveGoalMsg: Unrecognized Goal type: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		m_pAI->GetGoalMgr()->RemoveGoal( eGoalType );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleRemoveWeaponMsg
//
//	PURPOSE:	Handle a REMOVEWEAPON message...
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleRemoveWeaponMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// trigger AI (AddWeapon weapon_name ammo_name)

	HWEAPON hWeapon = NULL;

	const char* const pszWeaponName = crParsedMsg.GetArg(1);
	hWeapon = g_pWeaponDB->GetWeaponRecord( pszWeaponName );

	if (hWeapon != NULL)
	{
		m_pAI->GetAIWeaponMgr()->RemoveWeapon( hWeapon );
		m_pAI->HandleArsenalChange();

		// Re-evaluate goals.

		m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleRigidBodyMsg
//
//	PURPOSE:	Handle a RIGIDBODY message...
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleRigidBodyMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		if( IsTrueChar( *crParsedMsg.GetArg(1) ) )
		{
			m_pAI->SetPhysicsWeightSet( PhysicsUtilities::WEIGHTSET_RIGID_BODY );
		}
		else 
		{
			const char* pPhysicsWeightSetName = g_pModelsDB->GetDefaultPhysicsWeightSet( m_pAI->GetModel() );
			if( pPhysicsWeightSetName && pPhysicsWeightSetName[0] )
			{
				m_pAI->SetPhysicsWeightSet(pPhysicsWeightSetName);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleScan
//
//	PURPOSE:	Handle a Scan command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleScanMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		// Find the named node.
		// Bail if it does not exist.

		HOBJECT hNewScannerNode;
		if( LT_OK != FindNamedObject( crParsedMsg.GetArg(1), hNewScannerNode ) )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleScan: Could not find node: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		AINodeScanner* pNewScannerNode = (AINodeScanner*)g_pLTServer->HandleToObject( hNewScannerNode );
		if( !pNewScannerNode )
		{
			return;
		}

		// Clear current node owner's memory of scanner node.

		CAIWMFact* pFact;
		if( IsAI( pNewScannerNode->GetNodeOwner() ) )
		{
			CAI* pOldOwner = (CAI*)g_pLTServer->HandleToObject( pNewScannerNode->GetNodeOwner() );
			if( pOldOwner )
			{
				CAIWMFact factQuery;
				factQuery.SetFactType(kFact_Node);
				factQuery.SetNodeType(kNode_Scanner);
				pOldOwner->GetAIWorkingMemory()->ClearWMFact( factQuery );

				// Unlock the patrol path.

				pNewScannerNode->ClaimPatrolPath( pOldOwner, false );
			}
		}

		// Take immediate ownership of the node.

		pNewScannerNode->SetNodeOwner( m_pAI->m_hObject );

		// Find an existing scanner node memory.

		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Node);
		factQuery.SetNodeType(kNode_Scanner);
		pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact )
		{
			// Clear the owner of this scanner node.

			AINodeScanner* pScannerNode = (AINodeScanner*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
			if( pScannerNode && ( pScannerNode->GetNodeOwner() == m_pAI->m_hObject ) )
			{
				pScannerNode->SetNodeOwner( NULL );

				// Unlock the old patrol path.

				pScannerNode->ClaimPatrolPath( m_pAI, false );
			}
		}

		// Lock the new patrol path.

		pNewScannerNode->ClaimPatrolPath( m_pAI, true );

		// Create a memory for the scanner node.

		if( !pFact )
		{
			pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Node );
		}

		if( pFact )
		{
			pFact->SetNodeType( kNode_Scanner, 1.f );
			pFact->SetTargetObject( pNewScannerNode->m_hObject, 1.f );
			pFact->SetPos( pNewScannerNode->GetPos(), 1.f );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleSearch
//
//	PURPOSE:	Handle a Search command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleSearchMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Order AI to advance to some node.

	ProcessNodeOrderMsg( crParsedMsg, kNode_Goto, kTask_LeadSearch, !TARGET_ENEMY, TASK_SCRIPTED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleSeekEnemy
//
//	PURPOSE:	Handle a SeekEnemy command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleSeekEnemyMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Find a player.
	// Eventually this command should find anyone the AI hates.

	CPlayerObj* pPlayer = g_pCharacterMgr->FindPlayer();
	if( !pPlayer )
	{
		return;
	}

	// Create a SeekEnemy sensor if none exists.

	CAISensorAbstract* pSensor = m_pAI->GetAISensorMgr()->FindSensor( kSensor_SeekEnemy );
	if( !pSensor )
	{
		pSensor = m_pAI->GetAISensorMgr()->AddAISensor( kSensor_SeekEnemy );
	}

	// Setup the SeekEnemy sensor.

	if (pSensor)
	{
		CAISensorSeekEnemy* pSensorSeekEnemy = (CAISensorSeekEnemy*)pSensor;
		pSensorSeekEnemy->SetEnemy( pPlayer->m_hObject, false );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleSeekSquadEnemy
//
//	PURPOSE:	Handle a SeekSquadEnemy command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleSeekSquadEnemyMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Find a player.
	// Eventually this command should find anyone the AI hates.

	CPlayerObj* pPlayer = g_pCharacterMgr->FindPlayer();
	if( !pPlayer )
	{
		return;
	}

	// Create a SeekEnemy sensor if none exists.

	CAISensorAbstract* pSensor = m_pAI->GetAISensorMgr()->FindSensor( kSensor_SeekEnemy );
	if( !pSensor )
	{
		pSensor = m_pAI->GetAISensorMgr()->AddAISensor( kSensor_SeekEnemy );
	}

	// Setup the SeekEnemy sensor.

	if (pSensor)
	{
		CAISensorSeekEnemy* pSensorSeekEnemy = (CAISensorSeekEnemy*)pSensor;
		pSensorSeekEnemy->SetEnemy( pPlayer->m_hObject, true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleSenses
//
//	PURPOSE:	Handle a Senses command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleSensesMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		// Turn on/off senses.

		m_pAI->GetAIBlackBoard()->SetBBSensesOn( IsTrueChar( *crParsedMsg.GetArg(1) ) );

		// Re-evaluate target.

		m_pAI->GetAIBlackBoard()->SetBBInvalidateTarget( true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleSolidMsg
//
//	PURPOSE:	Handle a SOLID message...
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleSolidMsg(  HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	static CParsedMsg::CToken s_cTok_1( "1" );
	static CParsedMsg::CToken s_cTok_True( "TRUE" );
	static CParsedMsg::CToken s_cTok_0( "0" );
	static CParsedMsg::CToken s_cTok_False( "FALSE" );

	m_pAI->HandleSolidMsg( hSender, crParsedMsg );

	if( (crParsedMsg.GetArg(1) == s_cTok_1) ||
		(crParsedMsg.GetArg(1) == s_cTok_True) )
	{
		m_pAI->SetClientSolid( true );
	}
	else if( (crParsedMsg.GetArg(1) == s_cTok_0) ||
			 (crParsedMsg.GetArg(1) == s_cTok_False) )
	{
		m_pAI->SetClientSolid( false );		
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleSquadNamesMsg
//
//	PURPOSE:	Handle a SQUADNAMES message...
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleSquadNamesMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	ENUM_AI_SQUAD_ID eSquadID = g_pAICoordinator->GetSquadID( m_pAI->m_hObject );
	CAISquad* pSquad = g_pAICoordinator->FindSquad( eSquadID );

	// No squad.

	if( !pSquad )
	{
		g_pLTServer->CPrint( "%s is not in a squad.", m_pAI->GetName() );
		return;
	}

	// Print names of squad members.

	CAI* pAI;
	HOBJECT hMember;
	LTObjRef* pMembers = pSquad->GetSquadMembers();
	uint32 cMembers = pSquad->GetNumSquadMembers();
	for( uint32 iMember=0; iMember < cMembers; ++iMember )
	{
		hMember = pMembers[iMember];
		if( IsAI( hMember ) )
		{
			pAI = (CAI*)g_pLTServer->HandleToObject( hMember );
			g_pLTServer->CPrint( "  %s", pAI->GetName() );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleTarget
//
//	PURPOSE:	Handle a Target command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleTargetMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		// Clear any existing target.

		if( LTStrIEquals( crParsedMsg.GetArg(1), "None" ) )
		{
			m_pAI->GetAIBlackBoard()->SetBBScriptedTargetObject( NULL );

			// Re-evaluate target immediately.

			m_pAI->GetAIBlackBoard()->SetBBInvalidateTarget( true );
			return;
		}

		// Bail if targeting ourself!

		if( LTStrIEquals( crParsedMsg.GetArg(1), m_pAI->GetName() ) )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleTargetMsg: Object cannot target itself: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		// Find the named object.
		// Bail if it does not exist.

		HOBJECT hTargetObject;
		if( LT_OK != FindNamedObject( crParsedMsg.GetArg(1), hTargetObject ) )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleTargetMsg: Could not find target object: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		m_pAI->GetAIBlackBoard()->SetBBScriptedTargetObject( hTargetObject );

		// Re-evaluate target immediately.

		m_pAI->GetAIBlackBoard()->SetBBInvalidateTarget( true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleTrackAimAt
//
//	PURPOSE:	Handle a TrackAimAt command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleTrackAimAtMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Optional parameter specifies who to aim at.

	HOBJECT hTarget = NULL;
	if( crParsedMsg.GetArgCount() > 1 )
	{
		// Aim at someone other than the player.

		if( !LTStrIEquals( crParsedMsg.GetArg(1), "Player" ) )
		{
			if( LT_OK != FindNamedObject( crParsedMsg.GetArg(1), hTarget ) )
			{
				AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleTrackLookAtMsg: Could not find target object: %s", crParsedMsg.GetArg(1).c_str() );
				return;
			}
		}
	}

	// Aim at the player by default.

	if( hTarget == NULL )
	{
		CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
		if ( pPlayer )
		{
			hTarget = pPlayer->m_hObject;
		}
	}

	HMODELNODE hNodeHead;
	g_pModelLT->GetNode( hTarget, "Eye_cam", hNodeHead);
	if( hNodeHead == INVALID_MODEL_NODE )
	{
		g_pModelLT->GetNode( hTarget, "Head", hNodeHead);
	}

	m_pAI->GetAIBlackBoard()->SetBBTriggerTrackerFlags( kTrackerFlag_AimAt );
	m_pAI->GetAIBlackBoard()->SetBBTargetTrackerType( CNodeTracker::eTarget_Node );
	m_pAI->GetAIBlackBoard()->SetBBTargetTrackerModel( hTarget );
	m_pAI->GetAIBlackBoard()->SetBBTargetTrackerModelNode( hNodeHead );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleTrackArm
//
//	PURPOSE:	Handle a TrackArm command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleTrackArmMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Optional parameter specifies who to aim at.

	HOBJECT hTarget = NULL;
	if( crParsedMsg.GetArgCount() > 1 )
	{
		// Aim at someone other than the player.

		if( !LTStrIEquals( crParsedMsg.GetArg(1), "Player" ) )
		{
			if( LT_OK != FindNamedObject( crParsedMsg.GetArg(1), hTarget ) )
			{
				AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleTrackLookAtMsg: Could not find target object: %s", crParsedMsg.GetArg(1).c_str() );
				return;
			}
		}
	}

	// Aim at the player by default.

	if( hTarget == NULL )
	{
		CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
		if ( pPlayer )
		{
			hTarget = pPlayer->m_hObject;
		}
	}

	HMODELNODE hNodeHead;
	g_pModelLT->GetNode( hTarget, "Eye_cam", hNodeHead);
	if( hNodeHead == INVALID_MODEL_NODE )
	{
		g_pModelLT->GetNode( hTarget, "Head", hNodeHead);
	}

	m_pAI->GetAIBlackBoard()->SetBBTriggerTrackerFlags( kTrackerFlag_Arm );
	m_pAI->GetAIBlackBoard()->SetBBTargetTrackerType( CNodeTracker::eTarget_Node );
	m_pAI->GetAIBlackBoard()->SetBBTargetTrackerModel( hTarget );
	m_pAI->GetAIBlackBoard()->SetBBTargetTrackerModelNode( hNodeHead );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleTrackLookAt
//
//	PURPOSE:	Handle a TrackLookAt command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleTrackLookAtMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Optional parameter specifies who to look at.

	HOBJECT hTarget = NULL;
	if( crParsedMsg.GetArgCount() > 1 )
	{
		// Look at someone other than the player.

		if( !LTStrIEquals( crParsedMsg.GetArg(1), "Player" ) )
		{
			if( LT_OK != FindNamedObject( crParsedMsg.GetArg(1), hTarget ) )
			{
				AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::HandleTrackLookAtMsg: Could not find target object: %s", crParsedMsg.GetArg(1).c_str() );
				return;
			}
		}
	}

	// Look at the player by default.

	if( hTarget == NULL )
	{
		CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
		if ( pPlayer )
		{
			hTarget = pPlayer->m_hObject;
		}
	}

	HMODELNODE hNodeHead;
	g_pModelLT->GetNode( hTarget, "Eye_cam", hNodeHead);
	if( hNodeHead == INVALID_MODEL_NODE )
	{
		g_pModelLT->GetNode( hTarget, "Head", hNodeHead);
	}

	m_pAI->GetAIBlackBoard()->SetBBTriggerTrackerFlags( kTrackerFlag_LookAt );
	m_pAI->GetAIBlackBoard()->SetBBTargetTrackerType( CNodeTracker::eTarget_Node );
	m_pAI->GetAIBlackBoard()->SetBBTargetTrackerModel( hTarget );
	m_pAI->GetAIBlackBoard()->SetBBTargetTrackerModelNode( hNodeHead );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleTrackNone
//
//	PURPOSE:	Handle a TrackNone command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::HandleTrackNoneMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_pAI->GetAIBlackBoard()->SetBBTriggerTrackerFlags( kTrackerFlag_None );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::HandleTeamMsg()
//
//	PURPOSE:	Handle a TEAM message...
//
// --------------------------------------------------------------------------- //

void CAICommandMgr::HandleTeamMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Check parameters are correct.
	if( crParsedMsg.GetArgCount() < 2 )
		return;

	// Change their team designation.
	uint8 nTeamId = atoi( crParsedMsg.GetArg( 1 ));
	m_pAI->SetTeamID(( nTeamId > 1 ) ? INVALID_TEAM : nTeamId );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICommandMgr::ProcessNodeOrderMsg
//
//	PURPOSE:	Process a node order command.
//
// ----------------------------------------------------------------------- //

void CAICommandMgr::ProcessNodeOrderMsg( const CParsedMsg &crParsedMsg, EnumAINodeType eNodeType, ENUM_AIWMTASK_TYPE eTaskType, bool bTargetEnemy, bool bScripted )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		// Clear any existing task.

		if( LTStrIEquals( crParsedMsg.GetArg(1), "None" ) )
		{
			CAIWMFact factQuery;
			factQuery.SetFactType( kFact_Task );
			factQuery.SetTaskType( eTaskType );
			m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

			// Re-evaluate goals.

			m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
			return;
		}

		// Find the named node.
		// Bail if it does not exist.

		HOBJECT hNode;
		if( LT_OK != FindNamedObject( crParsedMsg.GetArg(1), hNode ) )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::ProcessNodeOrderMsg: Could not find node: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		if ( hNode && !IsKindOf(hNode, "AINode") )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::ProcessNodeOrderMsg: Object is not an AI node: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		AINode* pNode = AINode::HandleToObject( hNode );
		if( !( pNode && pNode->GetType() == eNodeType ) )
		{
			AIASSERT1( 0, m_pAI->m_hObject, "CAICommandMgr::ProcessNodeOrderMsg: Node is not correct type: %s", crParsedMsg.GetArg(1).c_str() );
			return;
		}

		// AI is not currently targeting anyone.
		// Tell him to target whomever his squad is targeting.

		CAIWMFact* pFact;
		if( bTargetEnemy )
		{
			if( !m_pAI->HasTarget( kTarget_Character | kTarget_Object ) )
			{
				ENUM_AI_SQUAD_ID eSquadID = g_pAICoordinator->GetSquadID( m_pAI->m_hObject );
				if( eSquadID != kSquad_Invalid )
				{
					CAIWMFact factQuery;
					factQuery.SetFactType( kFact_Knowledge );
					factQuery.SetKnowledgeType( kKnowledge_SquadTarget );
					factQuery.SetIndex( eSquadID );

					pFact = g_pAIWorkingMemoryCentral->FindWMFact( factQuery );
					if( pFact )
					{
						HOBJECT hTarget = pFact->GetTargetObject();

						// Create a SeekEnemy sensor if none exists.

						CAISensorAbstract* pSensor = m_pAI->GetAISensorMgr()->FindSensor( kSensor_SeekEnemy );
						if( !pSensor )
						{
							pSensor = m_pAI->GetAISensorMgr()->AddAISensor( kSensor_SeekEnemy );
						}

						// Setup the SeekEnemy sensor.
						if (pSensor)
						{
							CAISensorSeekEnemy* pSensorSeekEnemy = (CAISensorSeekEnemy*)pSensor;
							pSensorSeekEnemy->SetEnemy( hTarget, false );
						}
					}
				}
			}
		}

		// Clear any existing task.

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Task );
		factQuery.SetTaskType( eTaskType );
		m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

		// Create a new task.

		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Task );
		if( pFact )
		{
			pFact->SetTaskType( eTaskType, 1.f );
			pFact->SetTargetObject( hNode, 1.f );
			pFact->SetNodeType( pNode->GetType(), 1.f );
			pFact->SetPos( pNode->GetPos(), 1.f );

			if( ( crParsedMsg.GetArgCount() > 2 ) &&
				( LTStrIEquals( crParsedMsg.GetArg(2), "COVERED" ) ) )
			{
				pFact->SetDesireType( kDesire_Covered, 1.f );
			}

			// Task may be scripted or internally generated.

			ENUM_AIWMFACT_FLAG eWMFlag = bScripted ? kFactFlag_Scripted : kFactFlag_None;
			pFact->SetFactFlags( eWMFlag, 1.f );

			// Re-evaluate goals.

			m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
		}
	}
}
