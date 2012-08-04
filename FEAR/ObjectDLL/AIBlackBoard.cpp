// ----------------------------------------------------------------------- //
//
// MODULE  : AIBlackBoard.cpp
//
// PURPOSE : AIBlackBoard abstract class implementation.
//           The BlackBoard is used by AI subsystems to share
//           their requests, intents, and results.
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIBlackBoard.h"
#include "AIDB.h"
#include "AIActivityAbstract.h"
#include "AINavMesh.h"
#include "AINavigationMgr.h"
#include "AINavMeshLinkAbstract.h"
#include "AIWorkingMemory.h"
#include "AIWeaponMgr.h"
#include "AISounds.h"
#include "AITarget.h"
#include "AIStimulusMgr.h"
#include "AIUtils.h"
#include "AnimationContext.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS( CAIBlackBoard );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIBlackBoard::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIBlackBoard::CAIBlackBoard()
{
	m_bAutoReload = false;
	m_bBlindFire = false;
	m_bMovementFire = true;
	m_bSuppressionFire = false;
	m_bPerfectAccuracy = false;
	m_fFirePauseTimeLimit = 0.f;

	m_bAllowDirectionalRun = true;
	m_flMinDirectionalRunChangeDistanceSqr = 0.f;

	m_eAwareness = kAware_Relaxed;
	m_eAwarenessMod = kAwarenessMod_Invalid;

	m_bSensesOn = true;
	m_bIncapacitated = false;

	m_fSeeDistance = 0.f;
	m_fHearDistance = 0.f;

	m_bUpdateDims = false;

	m_ePosture = kAP_POS_Stand;
	m_fPostureChangeTime = 0.f;

	m_fMovementDirChangeTime = 0.f;

	m_bInvalidateTarget = false;
	m_bSelectTarget = false;

	m_bFaceTarget = false;
	m_bFaceTargetKnownPos = false;
	m_bUpdateTargetAim = true;
	m_hTargetObject = NULL;
	m_hScriptedTargetObject = NULL;
	m_eTargetTrueNavMeshPoly = kNMPoly_Invalid;
	m_eTargetReachableNavMeshPoly = kNMPoly_Invalid;
	m_eTargetLocation = kAIS_InvalidType;
	m_dwTargetPosTrackingFlags = kTargetTrack_Normal;
	m_eTargetLostNavMeshPoly = kNMPoly_Invalid;
	m_fTargetLostTime = 0.f;
	m_cShotsFiredAtTarget = 0;

	m_eTargetType = kTarget_None;
	m_eTargetedTypeMask = 0;
	m_eTargetStimulusType = kStim_InvalidType;
	m_eTargetStimulusID = kStimID_Unset;
	m_fTargetChangeTime = 0.f;
	m_fTargetPosUpdateTime = 0.f;
	m_fTargetFirstThreatTime = 0.f;

	m_bTargetVisibleFromEye = false;
	m_bTargetVisibleFromWeapon = false;
	m_fTargetLastVisibileTime = 0.f;
	m_eTargetLastVisibleNMPoly = kNMPoly_Invalid;
	m_fTargetVisibilityConfidence = 0.f;

	m_hFaceObject = NULL;
	m_bFaceImmediately = false;
	m_eFaceType = kFaceType_None;

	m_dwTriggerTrackerFlags = kTrackerFlag_None;
	m_dwTargetTrackerFlags = kTrackerFlag_None;
	m_eTargetTrackerType = CNodeTracker::eTarget_Aimer;
	m_hTargetTrackerModel = NULL;
	m_hTargetTrackerModelNode = INVALID_MODEL_NODE;
	m_bTargetTrackerAtLimitX = false;
	m_bTargetTrackerTrackLastVisible = false;

	m_eDestStatus = kNav_Unset;
	m_fDestStatusChangeTime = 0.f;
	m_bDestIsDynamic = false;
	m_fDestRecalculatePathDistSqr = 0.f;
	m_ePathType = kPath_Default;
	m_bReserveNMLinks = false;

	m_eNextNMLink = kNMLink_Invalid;
	m_eEnteringNMLink = kNMLink_Invalid;
	m_eTraversingNMLink = kNMLink_Invalid;

	m_bAIHandlingDeath = false;

	m_hLockedNode = NULL;
	m_hAttachedTo = NULL;
	m_hVehicleKeyframeToRigidBody = NULL;

	m_bUpdateClient = false;

	m_eTaskStatus = kTaskStatus_Unset;

	m_bSelectAction = true;
	m_bInvalidatePlan = false;

	m_fStateChangeTime = 0.f;

	m_ePrimaryWeaponProp = kAP_None;
	m_ePrimaryWeaponType = kAIWeaponType_None;

	m_eLastAIWeaponRecordID = kAIWeaponID_Invalid;
	m_eCurrentAIWeaponRecordID = kAIWeaponID_Invalid;
	m_eHolsteredRightAIWeaponRecordID = kAIWeaponID_Invalid;
	m_eHolsteredLeftAIWeaponRecordID = kAIWeaponID_Invalid;

	m_hHolsteredRightWeaponRecord = NULL;
	m_hHolsteredLeftWeaponRecord = NULL;

	m_nRoundsFired = 0;

	m_eWeaponOverrideSetID = kAIWeaponOverrideSetID_Invalid;

	m_eAIActionSet = kAIActionSet_Invalid;

	m_eAITargetSelectSet = kAITargetSelectSet_Invalid;

	m_eActivitySet = kAIActivitySet_Invalid;

	m_eMovementSet = kAIMovementSetID_Invalid;
	m_bScaleMovement = false;
	m_bInterpolateMovementHeight = false;
	m_dwMovementCollisionFlags = kAIMovementFlag_ObstacleAvoidance;
	m_bMovementEncodeUseRadius = true;
	m_eScriptedBodyState = eBodyStateInvalid;

	for (int i = 0; i < kAIWeaponType_Count; ++i)
	{
		m_aWeaponRangeStatus[i] = kRangeStatus_Invalid;
		m_aWeaponRangeStatusChangeTime[i] = 0.f;
		m_aHasWeapon[i] = false;
	}

	m_bAdvancePath = false;
	m_bInvalidatePath = false;

	m_eHandleTouch = kTouch_None;
	m_bCollidedWithTarget = false;

	m_eSurfaceOverride = ST_UNKNOWN;

	m_cBodyCount = 0;

	m_hLoopSound = NULL;

	m_bInstantDeath = false;

	m_hFinishingSyncAction = NULL;
}

CAIBlackBoard::~CAIBlackBoard()
{
}

void CAIBlackBoard::Save(ILTMessage_Write *pMsg)
{
	SAVE_bool(m_bAutoReload);
	SAVE_bool(m_bBlindFire);
	SAVE_bool(m_bMovementFire);
	SAVE_bool(m_bSuppressionFire);
	SAVE_bool(m_bPerfectAccuracy);
	SAVE_TIME(m_fFirePauseTimeLimit);

	SAVE_bool(m_bAllowDirectionalRun);
	SAVE_FLOAT(m_flMinDirectionalRunChangeDistanceSqr);

	SAVE_DWORD(m_eAwareness);
	SAVE_DWORD(m_eAwarenessMod);

	SAVE_bool(m_bSensesOn);
	SAVE_bool(m_bIncapacitated);

	SAVE_FLOAT(m_fSeeDistance);
	SAVE_FLOAT(m_fHearDistance);

	SAVE_bool(m_bUpdateDims);

	SAVE_INT(m_ePosture);
	SAVE_TIME(m_fPostureChangeTime);
	SAVE_TIME(m_fMovementDirChangeTime);

	SAVE_bool(m_bInvalidateTarget);
	SAVE_bool(m_bSelectTarget);

	SAVE_bool(m_bFaceTarget);
	SAVE_bool(m_bFaceTargetKnownPos);
	SAVE_bool(m_bUpdateTargetAim);
	SAVE_HOBJECT(m_hTargetObject);
	SAVE_HOBJECT(m_hScriptedTargetObject);
	SAVE_VECTOR(m_vTargetPosition);
	SAVE_VECTOR(m_vTargetDims);
	SAVE_DWORD(m_eTargetTrueNavMeshPoly);
	SAVE_DWORD(m_eTargetReachableNavMeshPoly);
	SAVE_VECTOR(m_vTargetReachableNavMeshPosition);
	SAVE_DWORD(m_eTargetLocation);
	SAVE_DWORD(m_dwTargetPosTrackingFlags);
	SAVE_DWORD(m_eTargetLostNavMeshPoly);
	SAVE_VECTOR(m_vTargetLostPosition);
	SAVE_TIME(m_fTargetLostTime);
	SAVE_DWORD(m_cShotsFiredAtTarget);

	SAVE_DWORD( m_eTargetType );
	SAVE_DWORD( m_eTargetedTypeMask );
	SAVE_DWORD( m_eTargetStimulusType );
	SAVE_DWORD( m_eTargetStimulusID );
	SAVE_TIME( m_fTargetChangeTime );
	SAVE_TIME( m_fTargetPosUpdateTime );
	SAVE_TIME( m_fTargetFirstThreatTime );

	SAVE_bool(m_bTargetVisibleFromEye);
	SAVE_bool(m_bTargetVisibleFromWeapon);
	SAVE_TIME(m_fTargetLastVisibileTime);
	SAVE_DWORD(m_eTargetLastVisibleNMPoly);
	SAVE_VECTOR(m_vTargetLastVisiblePosition);
	SAVE_FLOAT(m_fTargetVisibilityConfidence);

	SAVE_HOBJECT(m_hCombatOpportunityTarget);

	SAVE_INT(m_dwTriggerTrackerFlags);
	SAVE_INT(m_dwTargetTrackerFlags);
	SAVE_INT(m_eTargetTrackerType);
	SAVE_HOBJECT(m_hTargetTrackerModel);

	pMsg->WriteHMODELNODE(m_hTargetTrackerModelNode);

	SAVE_VECTOR(m_vTargetTrackerPos);
	SAVE_bool(m_bTargetTrackerAtLimitX);
	SAVE_bool(m_bTargetTrackerTrackLastVisible);

	SAVE_VECTOR(m_vDest);
	SAVE_INT(m_eDestStatus);
	SAVE_TIME(m_fDestStatusChangeTime);
	SAVE_bool(m_bDestIsDynamic);
	SAVE_FLOAT(m_fDestRecalculatePathDistSqr);
	SAVE_DWORD(m_ePathType);
	SAVE_bool(m_bReserveNMLinks);

	SAVE_VECTOR(m_vDirToPathDest);
	SAVE_VECTOR(m_vDirToNextPathDest);

	SAVE_INT(m_eNextNMLink);
	SAVE_INT(m_eEnteringNMLink);
	SAVE_INT(m_eTraversingNMLink);

	SAVE_HOBJECT(m_hLockedNode);
	SAVE_HOBJECT(m_hAttachedTo);
	SAVE_HOBJECT(m_hVehicleKeyframeToRigidBody);
	SAVE_bool(m_bAIHandlingDeath);

	SAVE_BOOL( m_bUpdateClient );

	SAVE_INT(m_eTaskStatus);

	SAVE_bool(m_bSelectAction);
	SAVE_bool(m_bInvalidatePlan);

	SAVE_INT(m_ePrimaryWeaponProp);
	SAVE_INT(m_ePrimaryWeaponType);

	SaveAIWeaponRecord( pMsg, m_eLastAIWeaponRecordID );
	SaveAIWeaponRecord( pMsg, m_eCurrentAIWeaponRecordID );
	SaveAIWeaponRecord( pMsg, m_eHolsteredRightAIWeaponRecordID );
	SaveAIWeaponRecord( pMsg, m_eHolsteredLeftAIWeaponRecordID );

	SAVE_HRECORD(m_hHolsteredRightWeaponRecord);
	SAVE_HRECORD(m_hHolsteredLeftWeaponRecord);

	// Save the names, because the record indices may change if 
	// records are added to the database.

	std::string strName;
	strName = g_pAIDB->GetAIActionSetRecordName( m_eAIActionSet );
	SAVE_STDSTRING( strName );

	strName = g_pAIDB->GetAITargetSelectSetRecordName( m_eAITargetSelectSet );
	SAVE_STDSTRING( strName );

	strName = g_pAIDB->GetAIActivitySetRecordName( m_eActivitySet );
	SAVE_STDSTRING( strName );

	strName = g_pAIDB->GetAIMovementSetRecordName( m_eMovementSet );
	SAVE_STDSTRING( strName );

	SAVE_bool( m_bScaleMovement );
	SAVE_bool( m_bInterpolateMovementHeight );
	SAVE_INT( m_dwMovementCollisionFlags );
	SAVE_bool( m_bMovementEncodeUseRadius );
	SAVE_INT( m_eScriptedBodyState );

	for (int i = 0; i < kAIWeaponType_Count; ++i)
	{
		SAVE_INT(m_aWeaponRangeStatus[i]);
		SAVE_TIME(m_aWeaponRangeStatusChangeTime[i]);
		SAVE_bool(m_aHasWeapon[i]);
	}

	SAVE_DWORD( m_nRoundsFired );
	SAVE_INT(m_eWeaponOverrideSetID);

	SAVE_INT(m_eFaceType);
	SAVE_VECTOR(m_vFacePos);
	SAVE_VECTOR(m_vFaceDir);
	SAVE_HOBJECT(m_hFaceObject);
	SAVE_bool(m_bFaceImmediately);

	SAVE_TIME(m_fStateChangeTime);

	SAVE_bool(m_bAdvancePath);
	SAVE_bool(m_bInvalidatePath);

	SAVE_DWORD(m_eHandleTouch);
	SAVE_bool(m_bCollidedWithTarget);

	SAVE_DWORD(m_cBodyCount);

	SAVE_DWORD(m_eSurfaceOverride);

	SAVE_bool( m_bInstantDeath );
	m_LastDamage.Save( pMsg );

	SAVE_HRECORD( m_hFinishingSyncAction );
}

void CAIBlackBoard::Load(ILTMessage_Read *pMsg)
{
	LOAD_bool(m_bAutoReload);
	LOAD_bool(m_bBlindFire);
	LOAD_bool(m_bMovementFire);
	LOAD_bool(m_bSuppressionFire);
	LOAD_bool(m_bPerfectAccuracy);
	LOAD_TIME(m_fFirePauseTimeLimit);

	LOAD_bool(m_bAllowDirectionalRun);
	LOAD_FLOAT(m_flMinDirectionalRunChangeDistanceSqr);

	LOAD_DWORD_CAST(m_eAwareness, EnumAIAwareness);
	LOAD_DWORD_CAST(m_eAwarenessMod, EnumAIAwarenessMod);

	LOAD_bool(m_bSensesOn);
	LOAD_bool(m_bIncapacitated);

	LOAD_FLOAT(m_fSeeDistance);
	LOAD_FLOAT(m_fHearDistance);

	LOAD_bool(m_bUpdateDims);

	LOAD_INT_CAST(m_ePosture, EnumAnimProp);
	LOAD_TIME(m_fPostureChangeTime);
	LOAD_TIME(m_fMovementDirChangeTime);

	LOAD_bool(m_bInvalidateTarget);
	LOAD_bool(m_bSelectTarget);

	LOAD_bool(m_bFaceTarget);
	LOAD_bool(m_bFaceTargetKnownPos);
	LOAD_bool(m_bUpdateTargetAim);
	LOAD_HOBJECT(m_hTargetObject);
	LOAD_HOBJECT(m_hScriptedTargetObject);
	LOAD_VECTOR(m_vTargetPosition);
	LOAD_VECTOR(m_vTargetDims);
	LOAD_DWORD_CAST(m_eTargetTrueNavMeshPoly, ENUM_NMPolyID);
	LOAD_DWORD_CAST(m_eTargetReachableNavMeshPoly, ENUM_NMPolyID);
	LOAD_VECTOR(m_vTargetReachableNavMeshPosition);
	LOAD_DWORD_CAST(m_eTargetLocation, EnumAISoundType);
	LOAD_DWORD(m_dwTargetPosTrackingFlags);
	LOAD_DWORD_CAST(m_eTargetLostNavMeshPoly, ENUM_NMPolyID);
	LOAD_VECTOR(m_vTargetLostPosition);
	LOAD_TIME(m_fTargetLostTime);
	LOAD_DWORD(m_cShotsFiredAtTarget);

	LOAD_DWORD_CAST( m_eTargetType, ENUM_AI_TARGET_TYPE );
	LOAD_DWORD( m_eTargetedTypeMask );
	LOAD_DWORD_CAST( m_eTargetStimulusType, EnumAIStimulusType );
	LOAD_DWORD_CAST( m_eTargetStimulusID, EnumAIStimulusID );
	LOAD_TIME( m_fTargetChangeTime );
	LOAD_TIME( m_fTargetPosUpdateTime );
	LOAD_TIME( m_fTargetFirstThreatTime );

	LOAD_bool(m_bTargetVisibleFromEye);
	LOAD_bool(m_bTargetVisibleFromWeapon);
	LOAD_TIME(m_fTargetLastVisibileTime);
	LOAD_DWORD_CAST(m_eTargetLastVisibleNMPoly, ENUM_NMPolyID);
	LOAD_VECTOR(m_vTargetLastVisiblePosition);
	LOAD_FLOAT(m_fTargetVisibilityConfidence);

	LOAD_HOBJECT(m_hCombatOpportunityTarget);

	LOAD_INT(m_dwTriggerTrackerFlags);
	LOAD_INT(m_dwTargetTrackerFlags);
	LOAD_INT_CAST(m_eTargetTrackerType, CNodeTracker::ETargetType);
	LOAD_HOBJECT(m_hTargetTrackerModel);
	
	m_hTargetTrackerModelNode = pMsg->ReadHMODELNODE();

	LOAD_VECTOR(m_vTargetTrackerPos);
	LOAD_bool(m_bTargetTrackerAtLimitX);
	LOAD_bool(m_bTargetTrackerTrackLastVisible);

	LOAD_VECTOR(m_vDest);
	LOAD_INT_CAST(m_eDestStatus, ENUM_AI_NAV_STATUS);
	LOAD_TIME(m_fDestStatusChangeTime);
	LOAD_bool(m_bDestIsDynamic);
	LOAD_FLOAT(m_fDestRecalculatePathDistSqr);
	LOAD_DWORD_CAST(m_ePathType, ENUM_AI_PATH_TYPE);
	LOAD_bool(m_bReserveNMLinks);

	LOAD_VECTOR(m_vDirToPathDest);
	LOAD_VECTOR(m_vDirToNextPathDest);

	LOAD_INT_CAST(m_eNextNMLink, ENUM_NMLinkID);
	LOAD_INT_CAST(m_eEnteringNMLink, ENUM_NMLinkID);
	LOAD_INT_CAST(m_eTraversingNMLink, ENUM_NMLinkID);

	LOAD_HOBJECT(m_hLockedNode);
	LOAD_HOBJECT(m_hAttachedTo);
	LOAD_HOBJECT(m_hVehicleKeyframeToRigidBody);
	LOAD_bool(m_bAIHandlingDeath);

	LOAD_BOOL( m_bUpdateClient );

	LOAD_INT_CAST(m_eTaskStatus, ENUM_AIWMTASK_STATUS);

	LOAD_bool(m_bSelectAction);
	LOAD_bool(m_bInvalidatePlan);

	LOAD_INT_CAST(m_ePrimaryWeaponProp, EnumAnimProp);
	LOAD_INT_CAST(m_ePrimaryWeaponType, ENUM_AIWeaponType);

	m_eLastAIWeaponRecordID = LoadAIWeaponRecord( pMsg );
	m_eCurrentAIWeaponRecordID = LoadAIWeaponRecord( pMsg );
	m_eHolsteredRightAIWeaponRecordID = LoadAIWeaponRecord( pMsg );
	m_eHolsteredLeftAIWeaponRecordID = LoadAIWeaponRecord( pMsg );

	LOAD_HRECORD(m_hHolsteredRightWeaponRecord, g_pWeaponDB->GetWeaponsCategory());
	LOAD_HRECORD(m_hHolsteredLeftWeaponRecord, g_pWeaponDB->GetWeaponsCategory());
	
	// Load the names, because the record indices may change if 
	// records are added to the database.

	std::string strName;
	LOAD_STDSTRING( strName );
	m_eAIActionSet = g_pAIDB->GetAIActionSetRecordID( strName.c_str() );

	LOAD_STDSTRING( strName );
	m_eAITargetSelectSet = g_pAIDB->GetAITargetSelectSetRecordID( strName.c_str() );

	LOAD_STDSTRING( strName );
	m_eActivitySet = g_pAIDB->GetAIActivitySetRecordID( strName.c_str() );

	LOAD_STDSTRING( strName );
	m_eMovementSet = g_pAIDB->GetAIMovementSetRecordID( strName.c_str() );

	LOAD_bool( m_bScaleMovement );
	LOAD_bool( m_bInterpolateMovementHeight );
	LOAD_INT( m_dwMovementCollisionFlags );
	LOAD_bool( m_bMovementEncodeUseRadius );
	LOAD_INT_CAST( m_eScriptedBodyState, BodyState );

	for (int i = 0; i < kAIWeaponType_Count; ++i)
	{
		LOAD_INT_CAST(m_aWeaponRangeStatus[i], ENUM_RangeStatus);
		LOAD_TIME(m_aWeaponRangeStatusChangeTime[i]);
		LOAD_bool(m_aHasWeapon[i]);
	}

	LOAD_DWORD( m_nRoundsFired );
	LOAD_INT_CAST(m_eWeaponOverrideSetID, ENUM_AIWeaponOverrideSetID);

	LOAD_INT_CAST(m_eFaceType, ENUM_FaceType);
	LOAD_VECTOR(m_vFacePos);
	LOAD_VECTOR(m_vFaceDir);
	LOAD_HOBJECT(m_hFaceObject);
	LOAD_bool(m_bFaceImmediately);

	LOAD_TIME(m_fStateChangeTime);

	LOAD_bool(m_bAdvancePath);
	LOAD_bool(m_bInvalidatePath);	

	LOAD_DWORD_CAST(m_eHandleTouch, EnumAITouch);
	LOAD_bool(m_bCollidedWithTarget);

	LOAD_DWORD(m_cBodyCount);

	LOAD_DWORD_CAST(m_eSurfaceOverride, SurfaceType);

	LOAD_bool( m_bInstantDeath );
	m_LastDamage.Load( pMsg );

	LOAD_HRECORD( m_hFinishingSyncAction, g_pModelsDB->GetSyncActionCategory() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIBlackBoard::SaveAIWeaponRecord
//
//	PURPOSE:	Save the name of an AI weapon record.
//
// ----------------------------------------------------------------------- //

void CAIBlackBoard::SaveAIWeaponRecord( ILTMessage_Write *pMsg, ENUM_AIWeaponID eWeaponID )
{
	// Sanity check.

	if( !pMsg )
	{
		return;
	}

	static std::string strInvalid = "Invalid";

	AIDB_AIWeaponRecord* pAIWeaponRecord = g_pAIDB->GetAIWeaponRecord( eWeaponID );
	if( pAIWeaponRecord )
	{
		SAVE_STDSTRING( pAIWeaponRecord->strName );
	}
	else {
		SAVE_STDSTRING( strInvalid );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIBlackBoard::LoadAIWeaponRecord
//
//	PURPOSE:	Load the ID of an AI weapon record.
//
// ----------------------------------------------------------------------- //

ENUM_AIWeaponID CAIBlackBoard::LoadAIWeaponRecord( ILTMessage_Read *pMsg )
{
	// Sanity check.

	if( !pMsg )
	{
		return kAIWeaponID_Invalid;
	}

	static std::string strWeaponName;
	LOAD_STDSTRING( strWeaponName );

	return g_pAIDB->GetAIWeaponRecordID( strWeaponName.c_str() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIBlackBoard::SetBBPosture
//
//	PURPOSE:	Set the current posture, and keep track of when changes occur.
//
// ----------------------------------------------------------------------- //

void CAIBlackBoard::SetBBPosture( EnumAnimProp ePosture )
{
	if( ePosture != m_ePosture )
	{
		m_ePosture = ePosture;
		m_fPostureChangeTime = g_pLTServer->GetTime();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIBlackBoard::SetBBDestStatus
//
//	PURPOSE:	Set the navigation dest status.
//
// ----------------------------------------------------------------------- //

void CAIBlackBoard::SetBBDestStatus( ENUM_AI_NAV_STATUS eStatus )
{
	m_eDestStatus = eStatus;
	m_fDestStatusChangeTime = g_pLTServer->GetTime();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIBlackBoard::SetBBDest
//
//	PURPOSE:	Set the navigation dest.
//
// ----------------------------------------------------------------------- //

void CAIBlackBoard::SetBBDest( const LTVector& vDest )
{
	m_vDest = vDest;
	m_eDestStatus = kNav_Set;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIBlackBoard::SetBBWeaponStatus
//
//	PURPOSE:	Set the status of a weapon type.
//
// ----------------------------------------------------------------------- //

void CAIBlackBoard::SetBBWeaponStatus(ENUM_AIWeaponType eType, ENUM_RangeStatus eStatus)
{
	m_aWeaponRangeStatus[eType] = eStatus; 
	m_aWeaponRangeStatusChangeTime[eType] = g_pLTServer->GetTime();
} 
