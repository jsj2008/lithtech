// ----------------------------------------------------------------------- //
//
// MODULE  : AIBlackBoard.h
//
// PURPOSE : AIBlackBoard abstract class definition
//           The BlackBoard is used by AI subsystems to share
//           their requests, intents, and results.
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIBLACK_BOARD_H__
#define __AIBLACK_BOARD_H__

#include "AIClassFactory.h"
#include "AIActionMgr.h"
#include "AIWeaponMgr.h"
#include "AIMovement.h"
#include "NodeTracker.h"
#include "AIEnumNavMeshTypes.h"
#include "AIEnumNavMeshLinkTypes.h"
#include "AnimationProp.h"
#include "NodeTrackerContext.h"
#include "AINavigationMgr.h"
#include "AIPathMgrNavMesh.h"
#include "AIWorkingMemory.h"
#include "AIDB.h"
#include "AIActivityAbstract.h"
#include "AITarget.h"
#include "AISounds.h"
#include "AIEnumStimulusTypes.h"
#include "AIEnumAwarenessModifiers.h"
#include "AIEnumTouch.h"

enum ENUM_AIMovementFlags
{
	  kAIMovementFlag_ObstacleAvoidance		= ( 1 << 0 )
	, kAIMovementFlag_BlockedPath			= ( 1 << 1 )
	, kAIMovementFlag_BlockedDestination	= ( 1 << 2 )
};

// ----------------------------------------------------------------------- //

class CAIBlackBoard : public CAIClassAbstract
{
	public:
		DECLARE_AI_FACTORY_CLASS( CAIBlackBoard );

		CAIBlackBoard();
		~CAIBlackBoard();

		void					Save(ILTMessage_Write *pMsg);
		void					Load(ILTMessage_Read *pMsg);
		void					SaveAIWeaponRecord( ILTMessage_Write *pMsg, ENUM_AIWeaponID eWeaponID );
		ENUM_AIWeaponID			LoadAIWeaponRecord( ILTMessage_Read *pMsg );

		bool					GetBBAutoReload() const { return m_bAutoReload; }
		void					SetBBAutoReload( bool bAutoReload ) { m_bAutoReload = bAutoReload; }

		bool					GetBBBlindFire() const { return m_bBlindFire; }
		void					SetBBBlindFire( bool bBlindFire ) { m_bBlindFire = bBlindFire; }

		double					GetBBFirePauseTimeLimit() const { return m_fFirePauseTimeLimit; }
		void					SetBBFirePauseTimeLimit( double fFirePauseTimeLimit ) { m_fFirePauseTimeLimit = fFirePauseTimeLimit; }

		void					SetBBMovementFire( bool bAttack ) { m_bMovementFire = bAttack; }
		bool					GetBBMovementFire() const { return m_bMovementFire; }

		bool					GetBBSuppressionFire() const { return m_bSuppressionFire; }
		void					SetBBSuppressionFire( bool bSuppressionFire ) { m_bSuppressionFire = bSuppressionFire; }

		bool					GetBBPerfectAccuracy() const { return m_bPerfectAccuracy; }
		void					SetBBPerfectAccuracy( bool bPerfectAccuracy ) { m_bPerfectAccuracy = bPerfectAccuracy; }

		bool					GetBBAllowDirectionalRun() const { return m_bAllowDirectionalRun; }
		void					SetBBAllowDirectionalRun( bool bAllowDirectionalRun ) { m_bAllowDirectionalRun = bAllowDirectionalRun; }
		float					GetBBMinDirectionalRunChangeDistanceSqr() const { return m_flMinDirectionalRunChangeDistanceSqr; }
		void					SetBBMinDirectionalRunChangeDistanceSqr( float flMinDirectionalRunChangeDistanceSqr ) { m_flMinDirectionalRunChangeDistanceSqr = flMinDirectionalRunChangeDistanceSqr; }

		void					SetBBAwareness( EnumAIAwareness eAwareness ) { m_eAwareness = eAwareness; }
		EnumAIAwareness			GetBBAwareness() const { return m_eAwareness; }

		void					SetBBAwarenessMod( EnumAIAwarenessMod eAwarenessMod ) { m_eAwarenessMod = eAwarenessMod; }
		EnumAIAwarenessMod		GetBBAwarenessMod() const { return m_eAwarenessMod; }

		void					SetBBSensesOn( bool bSensesOn ) { m_bSensesOn = bSensesOn; }
		bool					GetBBSensesOn() const { return m_bSensesOn; }

		void					SetBBIncapacitated( bool bIncapacitated ) { m_bIncapacitated = bIncapacitated; }
		bool					GetBBIncapacitated() const { return m_bIncapacitated; }

		void					SetBBSeeDistance( float fDistance ) { m_fSeeDistance = fDistance; }
		void					SetBBHearDistance( float fDistance ) { m_fHearDistance = fDistance; }
		float					GetBBSeeDistance() const { return m_fSeeDistance; }
		float					GetBBHearDistance() const { return m_fHearDistance; }

		void					SetBBUpdateDims( bool bUpdate ) { m_bUpdateDims = bUpdate; }
		bool					GetBBUpdateDims() const { return m_bUpdateDims; }

		void					SetBBPosture( EnumAnimProp ePosture );
		EnumAnimProp			GetBBPosture() const { return m_ePosture; }
		double					GetBBPostureChangeTime() const { return m_fPostureChangeTime; }

		void					SetBBMovementDirChangeTime( double fTime ) { m_fMovementDirChangeTime = fTime; }
		double					GetBBMovementDirChangeTime() const { return m_fMovementDirChangeTime; }

		bool					GetBBInvalidateTarget() const { return m_bInvalidateTarget; }
		bool					GetBBSelectTarget() const { return m_bSelectTarget; }
		void					SetBBInvalidateTarget( bool bInvalidateTarget ) { m_bInvalidateTarget = bInvalidateTarget; }
		void					SetBBSelectTarget( bool bSelectTarget ) { m_bSelectTarget = bSelectTarget; }

		bool					GetBBFaceTarget() const { return m_bFaceTarget; }
		bool					GetBBFaceTargetKnownPos() const { return m_bFaceTargetKnownPos; }
		bool					GetBBUpdateTargetAim() const { return m_bUpdateTargetAim; }
		ENUM_AI_TARGET_TYPE		GetBBTargetType() const { return m_eTargetType; }
		uint32					GetBBTargetedTypeMask() const { return m_eTargetedTypeMask; }
		EnumAIStimulusType		GetBBTargetStimulusType() const { return m_eTargetStimulusType; }
		EnumAIStimulusID		GetBBTargetStimulusID() const { return m_eTargetStimulusID; }
		double					GetBBTargetChangeTime() const { return m_fTargetChangeTime; }
		double					GetBBTargetPosUpdateTime() const { return m_fTargetPosUpdateTime; }
		double					GetBBTargetFirstThreatTime() const { return m_fTargetFirstThreatTime; }
		HOBJECT					GetBBScriptedTargetObject() const { return m_hScriptedTargetObject; }
		HOBJECT					GetBBTargetObject() const { return m_hTargetObject; }
		const LTVector&			GetBBTargetPosition() const { return m_vTargetPosition; }
		const LTVector&			GetBBTargetDims() const { return m_vTargetDims; }
		ENUM_NMPolyID			GetBBTargetTrueNavMeshPoly() const { return m_eTargetTrueNavMeshPoly; }
		ENUM_NMPolyID			GetBBTargetReachableNavMeshPoly() const { return m_eTargetReachableNavMeshPoly; }
		const LTVector&			GetBBTargetReachableNavMeshPosition() const { return m_vTargetReachableNavMeshPosition; }
		EnumAISoundType			GetBBTargetLocation() const { return m_eTargetLocation; }
		uint32					GetBBTargetPosTrackingFlags() const { return m_dwTargetPosTrackingFlags; }
		ENUM_NMPolyID			GetBBTargetLostNavMeshPoly() const { return m_eTargetLostNavMeshPoly; }
		const LTVector&			GetBBTargetLostPosition() const { return m_vTargetLostPosition; }
		double					GetBBTargetLostTime() const { return m_fTargetLostTime; }
		uint32					GetBBShotsFiredAtTarget() const { return m_cShotsFiredAtTarget; }
		void					SetBBFaceTarget( bool bFace ) { m_bFaceTarget = bFace; }
		void					SetBBFaceTargetKnownPos( bool bFace ) { m_bFaceTargetKnownPos = bFace; }
		void					SetBBUpdateTargetAim( bool bUpdate ) { m_bUpdateTargetAim = bUpdate; }
		void					SetBBTargetType( ENUM_AI_TARGET_TYPE eTargetType ) { m_eTargetType = eTargetType; }
		void					SetBBTargetedTypeMask( uint32 dwMask )  { m_eTargetedTypeMask = dwMask; }
		void					SetBBTargetStimulusType( EnumAIStimulusType eStimulusType ) { m_eTargetStimulusType = eStimulusType; }
		void					SetBBTargetStimulusID( EnumAIStimulusID eStimulusID ) { m_eTargetStimulusID = eStimulusID; }
		void					SetBBTargetChangeTime( double fTime ) { m_fTargetChangeTime = fTime; }
		void					SetBBTargetPosUpdateTime( double fTime ) { m_fTargetPosUpdateTime = fTime; }
		void					SetBBTargetFirstThreatTime( double fTime ) { m_fTargetFirstThreatTime = fTime; }
		void					SetBBScriptedTargetObject( HOBJECT hTarget ) { m_hScriptedTargetObject = hTarget; }
		void					SetBBTargetObject( HOBJECT hTarget ) { m_hTargetObject = hTarget; }
		void					SetBBTargetPosition( const LTVector& vPos) { m_vTargetPosition = vPos; }
		void					SetBBTargetDims( const LTVector& vDims ) { m_vTargetDims = vDims; }
		void					SetBBTargetTrueNavMeshPoly( ENUM_NMPolyID ePoly ) { m_eTargetTrueNavMeshPoly = ePoly; }
		void					SetBBTargetReachableNavMeshPoly( ENUM_NMPolyID ePoly ) { m_eTargetReachableNavMeshPoly = ePoly; }
		void					SetBBTargetReachableNavMeshPosition( const LTVector& vPos) { m_vTargetReachableNavMeshPosition = vPos; }
		void					SetBBTargetLocation( EnumAISoundType eTargetLocation ) { m_eTargetLocation = eTargetLocation; }
		void					SetBBTargetPosTrackingFlags( uint32 dwFlags ) { m_dwTargetPosTrackingFlags = dwFlags; }
		void					SetBBTargetLostNavMeshPoly( ENUM_NMPolyID ePoly ) { m_eTargetLostNavMeshPoly = ePoly; }
		void					SetBBTargetLostPosition( const LTVector& vPos ) { m_vTargetLostPosition = vPos; }
		void					SetBBTargetLostTime( double fTime ) { m_fTargetLostTime = fTime; }
		void					SetBBShotsFiredAtTarget( uint32 cShots ) { m_cShotsFiredAtTarget = cShots; }

		bool					GetBBTargetVisibleFromEye() const { return m_bTargetVisibleFromEye; }
		bool					GetBBTargetVisibleFromWeapon() const { return m_bTargetVisibleFromWeapon; }
		double					GetBBTargetLastVisibleTime() const { return m_fTargetLastVisibileTime; }
		ENUM_NMPolyID			GetBBTargetLastVisibleNavMeshPoly() { return m_eTargetLastVisibleNMPoly; }
		const LTVector&			GetBBTargetLastVisiblePosition() { return m_vTargetLastVisiblePosition; }
		float					GetBBTargetVisibilityConfidence() const { return m_fTargetVisibilityConfidence; }
		void					SetBBTargetVisibleFromEye( bool bVisible ) { m_bTargetVisibleFromEye = bVisible; }
		void					SetBBTargetVisibleFromWeapon( bool bVisible ) { m_bTargetVisibleFromWeapon = bVisible; }
		void					SetBBTargetLastVisibleTime( double fTime ) { m_fTargetLastVisibileTime = fTime; }
		void					SetBBTargetLastVisibleNavMeshPoly( ENUM_NMPolyID ePoly ) { m_eTargetLastVisibleNMPoly = ePoly; }
		void					SetBBTargetLastVisiblePosition( const LTVector& vPos ) { m_vTargetLastVisiblePosition = vPos; }
		void					SetBBTargetVisibilityConfidence( float fConfidence ) { m_fTargetVisibilityConfidence = fConfidence; }

		HOBJECT					GetBBCombatOpportunityTarget() { return m_hCombatOpportunityTarget; }
		void					SetBBCombatOpportunityTarget(HOBJECT hObject) { m_hCombatOpportunityTarget = hObject; }

		uint32						GetBBTriggerTrackerFlags() const { return m_dwTriggerTrackerFlags; }
		uint32						GetBBTargetTrackerFlags() const { return m_dwTargetTrackerFlags; }
		CNodeTracker::ETargetType	GetBBTargetTrackerType() const { return m_eTargetTrackerType; }
		HOBJECT						GetBBTargetTrackerModel() const { return m_hTargetTrackerModel; }
		HMODELNODE					GetBBTargetTrackerModelNode() const { return m_hTargetTrackerModelNode; }
		const LTVector&				GetBBTargetTrackerPos() const { return m_vTargetTrackerPos; }
		bool						GetBBTargetTrackerAtLimitX() const { return m_bTargetTrackerAtLimitX; }
		bool						GetBBTargetTrackerTrackLastVisible() const { return m_bTargetTrackerTrackLastVisible; }
		void						SetBBTriggerTrackerFlags( uint32 dwFlags ) { m_dwTriggerTrackerFlags = dwFlags; }
		void						SetBBTargetTrackerFlags( uint32 dwFlags ) { m_dwTargetTrackerFlags = dwFlags; }
		void						SetBBTargetTrackerType( CNodeTracker::ETargetType eTargetType ) { m_eTargetTrackerType = eTargetType; }
		void						SetBBTargetTrackerModel( HOBJECT hModel ) { m_hTargetTrackerModel = hModel; }
		void						SetBBTargetTrackerModelNode( HMODELNODE hNode ) { m_hTargetTrackerModelNode = hNode; }
		void						SetBBTargetTrackerPos( const LTVector& vPos ) { m_vTargetTrackerPos = vPos; }
		void						SetBBTargetTrackerAtLimitX( bool bAtLimit ) { m_bTargetTrackerAtLimitX = bAtLimit; }
		void						SetBBTargetTrackerTrackLastVisible( bool bTrackLastVisible ) { m_bTargetTrackerTrackLastVisible = bTrackLastVisible; }

		void					SetBBFaceType( ENUM_FaceType eFaceType ) { m_eFaceType = eFaceType; }
		ENUM_FaceType			GetBBFaceType() const { return m_eFaceType; }
		void					SetBBFaceObject(HOBJECT hObj) { m_eFaceType = kFaceType_Object; m_hFaceObject = hObj; }
		HOBJECT					GetBBFaceObject() const { return m_hFaceObject; }
		void					SetBBFaceDir(const LTVector& vTargetDir) { m_eFaceType = kFaceType_Dir; m_vFaceDir = vTargetDir; }
        const LTVector&			GetBBFaceDir() const { return m_vFaceDir; }
		void					SetBBFacePos(const LTVector& vTargetPos) { m_eFaceType = kFaceType_Pos; m_vFacePos = vTargetPos; }
		const LTVector&			GetBBFacePos() const { return m_vFacePos; }
		void					SetBBFaceTargetRotImmediately(bool bFaceImmediately) { m_bFaceImmediately = bFaceImmediately; }
		bool					GetBBFaceTargetRotImmediately() const { return m_bFaceImmediately; }

		ENUM_AI_NAV_STATUS		GetBBDestStatus() const { return m_eDestStatus; }
		void					SetBBDestStatus( ENUM_AI_NAV_STATUS eStatus );
		const LTVector&			GetBBDest() const { return m_vDest; }
		void					SetBBDest( const LTVector& vDest );
		double					GetBBDestStatusChangeTime() const { return m_fDestStatusChangeTime; }
		bool					GetBBDestIsDynamic() const  {return m_bDestIsDynamic; }
		void					SetBBDestIsDynamic(bool bIsDynamic) {m_bDestIsDynamic = bIsDynamic; }
		float					GetBBDestRepathDistanceSqr() const  {return m_fDestRecalculatePathDistSqr; }
		void					SetBBDestRepathDistance(float fDist) {m_fDestRecalculatePathDistSqr = fDist*fDist; }
		ENUM_AI_PATH_TYPE		GetBBPathType() const { return m_ePathType; }
		void					SetBBPathType( ENUM_AI_PATH_TYPE ePathType ) { m_ePathType = ePathType; }
		bool					GetBBReserveNMLinks() const { return m_bReserveNMLinks; }
		void					SetBBReserveNMLinks( bool bReserve ) { m_bReserveNMLinks = bReserve; }

		const LTVector&			GetBBDirToPathDest() const { return m_vDirToPathDest; }
		void					SetBBDirToPathDest( const LTVector& vDir ) { m_vDirToPathDest = vDir; }
		const LTVector&			GetBBDirToNextPathDest() const { return m_vDirToNextPathDest; }
		void					SetBBDirToNextPathDest( const LTVector& vDir ) { m_vDirToNextPathDest = vDir; }

		bool					GetBBAdvancePath() const { return m_bAdvancePath; }
		void					SetBBAdvancePath(bool bAdvance) { m_bAdvancePath = bAdvance; }
		bool					GetBBInvalidatePath() const { return m_bInvalidatePath; }
		void					SetBBInvalidatePath(bool bRecacl) { m_bInvalidatePath = bRecacl; }

		ENUM_NMLinkID			GetBBNextNMLink() const { return m_eNextNMLink; }
		void					SetBBNextNMLink( ENUM_NMLinkID eNextNMLink ) { m_eNextNMLink = eNextNMLink; }
		ENUM_NMLinkID			GetBBEnteringNMLink() const { return m_eEnteringNMLink; }
		void					SetBBEnteringNMLink( ENUM_NMLinkID eEnteringNMLink ) { m_eEnteringNMLink = eEnteringNMLink; }
		ENUM_NMLinkID			GetBBTraversingNMLink() const { return m_eTraversingNMLink; }
		void					SetBBTraversingNMLink( ENUM_NMLinkID eTraversingNMLink ) { m_eTraversingNMLink = eTraversingNMLink; }

		HOBJECT					GetBBLockedNode() const { return m_hLockedNode; }
		void					SetBBLockedNode(HOBJECT hObject) { m_hLockedNode = hObject; }

		HOBJECT					GetBBAttachedTo() const { return m_hAttachedTo; }
		void					SetBBAttachedTo(HOBJECT hObject) { m_hAttachedTo = hObject; }

		HOBJECT					GetBBVehicleKeyframeToRigidBody() const { return m_hVehicleKeyframeToRigidBody; }
		void					SetBBVehicleKeyframeToRigidBody( HOBJECT hKeyframeToRigidBody ) { m_hVehicleKeyframeToRigidBody = hKeyframeToRigidBody; }

		bool					GetBBAIHandlingDeath() const { return m_bAIHandlingDeath; }
		void					SetBBAIHandlingDeath( bool bAIHandlingDeath ) { m_bAIHandlingDeath = bAIHandlingDeath; }

		bool					GetBBUpdateClient() const { return m_bUpdateClient; }
		void					SetBBUpdateClient( bool bUpdateClient ) { m_bUpdateClient = bUpdateClient; }

		ENUM_AIWMTASK_STATUS	GetBBTaskStatus() const { return m_eTaskStatus; }
		void					SetBBTaskStatus( ENUM_AIWMTASK_STATUS eStatus ) { m_eTaskStatus = eStatus; }

		bool					GetBBSelectAction() const { return m_bSelectAction; }
		void					SetBBSelectAction( bool bSelectAction ) { m_bSelectAction = bSelectAction; }

		bool					GetBBInvalidatePlan() const { return m_bInvalidatePlan; }
		void					SetBBInvalidatePlan( bool bInvalidatePlan ) { m_bInvalidatePlan = bInvalidatePlan; }

		double					GetBBStateChangeTime() const { return m_fStateChangeTime; }
		void					SetBBStateChangeTime( double fStateChangeTime ) { m_fStateChangeTime = fStateChangeTime; }

		ENUM_AIActionSet		GetBBAIActionSet() const { return m_eAIActionSet; }
		void					SetBBAIActionSet(ENUM_AIActionSet eAIActionSet) { m_eAIActionSet = eAIActionSet; }

		ENUM_AITargetSelectSet	GetBBAITargetSelectSet() const { return m_eAITargetSelectSet; }
		void					SetBBAITargetSelectSet(ENUM_AITargetSelectSet eAITargetSelectSet) { m_eAITargetSelectSet = eAITargetSelectSet; }

		ENUM_AIActivitySet		GetBBAIActivitySet() const { return m_eActivitySet; }
		void					SetBBAIActivitySet( ENUM_AIActivitySet eActivitySet ) { m_eActivitySet = eActivitySet; }

		ENUM_AIMovementSetID	GetBBAIMovementSet() const { return m_eMovementSet; }
		bool					GetBBScaleMovement() const { return m_bScaleMovement; }
		bool					GetBBInterpolateMovementHeight() const { return m_bInterpolateMovementHeight; }
		uint32					GetBBMovementCollisionFlags( ) const { return m_dwMovementCollisionFlags; }
		bool					GetBBMovementEncodeUseRadius( ) const { return m_bMovementEncodeUseRadius; }
		void					SetBBAIMovementSet( ENUM_AIMovementSetID eMovementSet ) { m_eMovementSet = eMovementSet; }
		void					SetBBScaleMovement( bool bScale ) { m_bScaleMovement = bScale; }
		void					SetBBInterpolateMovementHeight( bool bInterpolate ) { m_bInterpolateMovementHeight = bInterpolate; }
		void					SetBBMovementCollisionFlags( uint32 dwFlags ) { m_dwMovementCollisionFlags = dwFlags; }
		void					SetBBMovementEncodeUseRadius( bool bEncodeUseRadius ) { m_bMovementEncodeUseRadius = bEncodeUseRadius; }

		void					SetBBScriptedBodyState( BodyState eBodyState ) { m_eScriptedBodyState = eBodyState; }
		BodyState				GetBBScriptedBodyState( ) const { return m_eScriptedBodyState; }

		// Weapon Related

		EnumAnimProp			GetBBPrimaryWeaponProp() const { return m_ePrimaryWeaponProp; }
		void					SetBBPrimaryWeaponProp( EnumAnimProp eProp ) { m_ePrimaryWeaponProp = eProp; }

		ENUM_AIWeaponType		GetBBPrimaryWeaponType() const { return m_ePrimaryWeaponType; }
		void					SetBBPrimaryWeaponType( ENUM_AIWeaponType eType ) { m_ePrimaryWeaponType = eType; }

		ENUM_AIWeaponID			GetBBLastAIWeaponRecordID() const { return m_eLastAIWeaponRecordID; }
		void					SetBBLastAIWeaponRecordID(ENUM_AIWeaponID eRecord) { m_eLastAIWeaponRecordID = eRecord; }

		ENUM_AIWeaponID			GetBBCurrentAIWeaponRecordID() const { return m_eCurrentAIWeaponRecordID; }
		void					SetBBCurrentAIWeaponRecordID(ENUM_AIWeaponID eRecord) { m_eCurrentAIWeaponRecordID = eRecord; }

		ENUM_AIWeaponID			GetBBHolsterRightAIWeaponRecordID() const {return m_eHolsteredRightAIWeaponRecordID; }
		ENUM_AIWeaponID			GetBBHolsterLeftAIWeaponRecordID() const {return m_eHolsteredLeftAIWeaponRecordID; }
		void					SetBBHolsterRightAIWeaponRecordID(ENUM_AIWeaponID eType) { m_eHolsteredRightAIWeaponRecordID = eType;}
		void					SetBBHolsterLeftAIWeaponRecordID(ENUM_AIWeaponID eType) { m_eHolsteredLeftAIWeaponRecordID = eType;}

		HWEAPON					GetBBHolsterRightWeaponRecord() const {return m_hHolsteredRightWeaponRecord; }
		HWEAPON					GetBBHolsterLeftWeaponRecord() const {return m_hHolsteredLeftWeaponRecord; }
		void					SetBBHolsterRightWeaponRecord(HWEAPON hWeapon) { m_hHolsteredRightWeaponRecord = hWeapon; }
		void					SetBBHolsterLeftWeaponRecord(HWEAPON hWeapon) { m_hHolsteredLeftWeaponRecord = hWeapon; }

		bool					GetBBHasWeapon(ENUM_AIWeaponType eType) { return m_aHasWeapon[eType]; }
		void					SetBBHasWeapon(ENUM_AIWeaponType eType, bool b) { m_aHasWeapon[eType] = b; }
		
		ENUM_RangeStatus		GetBBWeaponStatus(ENUM_AIWeaponType eType)const { return m_aWeaponRangeStatus[eType]; } 
		double					GetBBWeaponStatusChangeTime(ENUM_AIWeaponType eType) const { return m_aWeaponRangeStatusChangeTime[eType]; } 
		void					SetBBWeaponStatus(ENUM_AIWeaponType eType, ENUM_RangeStatus eStatus);

		uint32					GetBBRoundsFired() const { return m_nRoundsFired; }
		void					SetBBRoundsFired( uint32 nRoundsFired ) { m_nRoundsFired = nRoundsFired; }

		ENUM_AIWeaponOverrideSetID GetBBAIWeaponOverrideSet() const { return m_eWeaponOverrideSetID; }
		void					SetBBAIWeaponOverrideSet(ENUM_AIWeaponOverrideSetID eWeaponOverrideSetID ) { m_eWeaponOverrideSetID = eWeaponOverrideSetID; }

		// Touch related

		EnumAITouch				GetBBHandleTouch() const { return m_eHandleTouch; }
		void					SetBBHandleTouch( EnumAITouch eHandleTouch ) { m_eHandleTouch = eHandleTouch; }

		
		bool					GetBBCollidedWithTarget() const { return m_bCollidedWithTarget; }
		void					SetBBCollidedWithTarget( bool bCollided ) { m_bCollidedWithTarget = bCollided; }

		HLTSOUND				GetBBLoopingSound() const { return m_hLoopSound; }
		void					SetBBLoopingSound( HLTSOUND hSound ) { m_hLoopSound = hSound; }

		SurfaceType				GetBBSurfaceOverride() const { return m_eSurfaceOverride; }
		void					SetBBSurfaceOverride( SurfaceType eSurface ) { m_eSurfaceOverride = eSurface; }

		uint32					GetBBBodyCount() const { return m_cBodyCount; }
		void					SetBBBodyCount( uint32 cCount ) { m_cBodyCount = cCount; }

		// Damage related 

		bool					GetBBInstantDeath() const { return m_bInstantDeath; }
		void					SetBBInstantDeath( bool bInstantDeath ) { m_bInstantDeath = bInstantDeath; }

		const DamageStruct&		GetBBLastDamage() const { return m_LastDamage; }
		void					SetBBLastDamage( const DamageStruct& rDamage ) { m_LastDamage = rDamage; }

		HRECORD					GetBBFinishingSyncAction() const { return m_hFinishingSyncAction; }
		void					SetBBFinishingSyncAction( HRECORD hFinishingSyncAction ) { m_hFinishingSyncAction = hFinishingSyncAction; }

	protected:

		bool					m_bAutoReload;
		bool					m_bBlindFire;
		bool					m_bMovementFire;
		bool					m_bSuppressionFire;
		bool					m_bPerfectAccuracy;
		double					m_fFirePauseTimeLimit;

		bool					m_bSensesOn;
		bool					m_bIncapacitated;

		float					m_fSeeDistance;
		float					m_fHearDistance;

		bool					m_bUpdateDims;

		EnumAnimProp			m_ePosture;
		double					m_fPostureChangeTime;

		double					m_fMovementDirChangeTime;

		bool					m_bAllowDirectionalRun;
		float					m_flMinDirectionalRunChangeDistanceSqr;

		EnumAIAwareness			m_eAwareness;
		EnumAIAwarenessMod		m_eAwarenessMod;

		bool					m_bInvalidateTarget;
		bool					m_bSelectTarget;

		bool					m_bFaceTarget;
		bool					m_bFaceTargetKnownPos;
		bool					m_bUpdateTargetAim;
		LTObjRef				m_hTargetObject;
		LTObjRef				m_hScriptedTargetObject;
		LTVector				m_vTargetPosition;
		LTVector				m_vTargetDims;
		ENUM_NMPolyID			m_eTargetTrueNavMeshPoly;
		ENUM_NMPolyID			m_eTargetReachableNavMeshPoly;
		LTVector				m_vTargetReachableNavMeshPosition;
		EnumAISoundType			m_eTargetLocation;
		uint32					m_dwTargetPosTrackingFlags;
		ENUM_NMPolyID			m_eTargetLostNavMeshPoly;
		LTVector				m_vTargetLostPosition;
		double					m_fTargetLostTime;
		uint32					m_cShotsFiredAtTarget;

		ENUM_AI_TARGET_TYPE		m_eTargetType;
		uint32					m_eTargetedTypeMask;
		EnumAIStimulusType		m_eTargetStimulusType;
		EnumAIStimulusID		m_eTargetStimulusID;
		double					m_fTargetChangeTime;
		double					m_fTargetPosUpdateTime;
		double					m_fTargetFirstThreatTime;

		bool					m_bTargetVisibleFromEye;
		bool					m_bTargetVisibleFromWeapon;
		double					m_fTargetLastVisibileTime;
		ENUM_NMPolyID			m_eTargetLastVisibleNMPoly;
		LTVector				m_vTargetLastVisiblePosition;
		float					m_fTargetVisibilityConfidence;

		LTObjRef				m_hCombatOpportunityTarget;

		uint32						m_dwTriggerTrackerFlags;
		uint32						m_dwTargetTrackerFlags;
		CNodeTracker::ETargetType	m_eTargetTrackerType;
		LTObjRef					m_hTargetTrackerModel;
		HMODELNODE					m_hTargetTrackerModelNode;
		LTVector					m_vTargetTrackerPos;
		bool						m_bTargetTrackerAtLimitX;
		bool						m_bTargetTrackerTrackLastVisible;

		LTVector				m_vDest;
		ENUM_AI_NAV_STATUS		m_eDestStatus;
		double					m_fDestStatusChangeTime;
		bool					m_bDestIsDynamic;
		float					m_fDestRecalculatePathDistSqr;
		ENUM_AI_PATH_TYPE		m_ePathType;
		bool					m_bReserveNMLinks;

		LTVector				m_vDirToPathDest;
		LTVector				m_vDirToNextPathDest;

		ENUM_NMLinkID			m_eNextNMLink;
		ENUM_NMLinkID			m_eEnteringNMLink;
		ENUM_NMLinkID			m_eTraversingNMLink;

		LTObjRef				m_hLockedNode;
		LTObjRef				m_hAttachedTo;
		LTObjRef				m_hVehicleKeyframeToRigidBody;
		bool					m_bAIHandlingDeath;

		bool					m_bUpdateClient;

		ENUM_AIWMTASK_STATUS	m_eTaskStatus;

		bool					m_bSelectAction;
		bool					m_bInvalidatePlan;

		double					m_fStateChangeTime;

		EnumAnimProp			m_ePrimaryWeaponProp;
		ENUM_AIWeaponType		m_ePrimaryWeaponType;
		ENUM_AIWeaponID			m_eLastAIWeaponRecordID;
		ENUM_AIWeaponID			m_eCurrentAIWeaponRecordID;
		ENUM_AIWeaponID			m_eHolsteredRightAIWeaponRecordID;
		ENUM_AIWeaponID			m_eHolsteredLeftAIWeaponRecordID;
		HWEAPON					m_hHolsteredRightWeaponRecord;
		HWEAPON					m_hHolsteredLeftWeaponRecord;
		uint32					m_nRoundsFired;

		ENUM_AIWeaponOverrideSetID m_eWeaponOverrideSetID;

		ENUM_AIActionSet		m_eAIActionSet;
		ENUM_AITargetSelectSet	m_eAITargetSelectSet;

		ENUM_AIActivitySet		m_eActivitySet;

		ENUM_AIMovementSetID	m_eMovementSet;
		bool					m_bScaleMovement;
		bool					m_bInterpolateMovementHeight;
		uint32					m_dwMovementCollisionFlags;
		bool					m_bMovementEncodeUseRadius;

		BodyState				m_eScriptedBodyState;

		ENUM_RangeStatus		m_aWeaponRangeStatus[kAIWeaponType_Count];
		double					m_aWeaponRangeStatusChangeTime[kAIWeaponType_Count];
		bool					m_aHasWeapon[kAIWeaponType_Count];

		ENUM_FaceType			m_eFaceType;
		LTVector				m_vFacePos;
		LTVector				m_vFaceDir;
		LTObjRef				m_hFaceObject;
		bool					m_bFaceImmediately;
		bool					m_bAdvancePath;
		bool					m_bInvalidatePath;

		EnumAITouch				m_eHandleTouch;
		bool					m_bCollidedWithTarget;

		SurfaceType				m_eSurfaceOverride;

		uint32					m_cBodyCount;

		HLTSOUND				m_hLoopSound;

		bool					m_bInstantDeath;
		DamageStruct			m_LastDamage;

		HRECORD					m_hFinishingSyncAction;
};

// ----------------------------------------------------------------------- //

#endif
