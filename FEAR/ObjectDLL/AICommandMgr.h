// ----------------------------------------------------------------------- //
//
// MODULE  : AICommandMgr.h
//
// PURPOSE : AICommandMgr class definition
//
// CREATED : 4/17/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AICOMMAND_MGR_H__
#define __AICOMMAND_MGR_H__

#include "AI.h"
#include "AINodeTypes.h"
#include "AIWorkingMemory.h"

class CAICommandMgr
{
	public:

		CAICommandMgr();
		~CAICommandMgr();

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

		void	InitAICommandMgr( CAI* pAI );
		

	protected:

		CAI*	m_pAI;
		
		// Message Handlers...

		DECLARE_MSG_HANDLER_AS_PROXY( CAI, NULLHandler );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleAllMsgs );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleActivateMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleAddGoalMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleAddWeaponMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleAdvanceMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleAlertMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleAlignmentMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleAmbushMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleAnimateMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleAwarenessMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleAwarenessModMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleBlitzMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleCanActivateMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleCinefireMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleCineractMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleCineractLoopMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleClearTaskMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleClearThreatMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleCoverMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleDamageMaskMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleDebugActions );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleDebugGoals );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleDebugDamage );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleDebugNodesOfType );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleDebugSensors );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleDebugShoveMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleDelCmdMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleDesireMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleDropWeaponsMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleDismountMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleFacePlayerMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleFollowMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleForceGroundMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleGoalSetMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleGotoMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleGuardMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleHidePieceMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleIntroMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleLeadMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleMaterialMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleMenaceMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleMoveMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleMountMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleMountAtObjectMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleMountNodeMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleObstacleAvoidanceMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandlePatrolMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandlePickupWeaponMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandlePlaySoundMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandlePreserveActivateCmdsMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandlePrivate_AmbushMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandlePrivate_ExchangeWeaponMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandlePrivate_CoverMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandlePrivate_SearchMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandlePrivate_SearchLostTargetMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandlePrivate_SuppressionFireMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleRemoveGoalMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleRemoveWeaponMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleRigidBodyMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleScanMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleSearchMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleSeekEnemyMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleSeekSquadEnemyMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleSensesMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleSolidMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleSquadNamesMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleTargetMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleTrackAimAtMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleTrackArmMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleTrackLookAtMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleTrackNoneMsg );
		DECLARE_MSG_HANDLER_AS_PROXY( CAI, HandleTeamMsg );

		// Shared message processing.

		void	ProcessNodeOrderMsg( const CParsedMsg &crParsedMsg, EnumAINodeType eNodeType, ENUM_AIWMTASK_TYPE eTaskType, bool bTargetEnemy, bool bScripted );
};

// ----------------------------------------------------------------------- //

#endif
