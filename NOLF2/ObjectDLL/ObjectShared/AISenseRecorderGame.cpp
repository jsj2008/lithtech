// ----------------------------------------------------------------------- //
//
// MODULE  : AISenseRecorderGame.cpp
//
// PURPOSE : AISenseRecorderGame implementation
//
// CREATED : 5/25/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AISenseRecorderGame.h"
#include "AI.h"
#include "AIBrain.h"
#include "AIStimulusMgr.h"
#include "AIVolume.h"
#include "AIRegion.h"
#include "PlayerObj.h"
#include "Alarm.h"
#include "AIUtils.h"
#include "AIVolumeMgr.h"


//----------------------------------------------------------------------------
//              
//	CLASS:		CAISenseRecorderTools
//              
//	PURPOSE:	Tool functions to reduce code duplication in the
//				CAISenseRecorderGame.  Many of the checks are checks such as
//				visibility which can be extracted cleanly.  Use these and add
//				to them, as they are likely to be reused.
//              
//----------------------------------------------------------------------------
class CAISenseRecorderTools
{
public:
	static void GetValidatedRecord( 
		const AISENSE_RECORD_MAP& SenseRecords,
		const CAIStimulusRecord* pStimulusRecord,
		AISenseRecord** pRecord
		);

	static LTBOOL ComplexVisibilityCheck(
		AISenseRecord* pRecord,
		const LTVector& vStimulusPos,
		LTBOOL bUseSightGrid,
		LTBOOL bUseInstantSeeDistance,
		CAI* pSensingAI,
		ILTBaseClass *pObject,
		LTFLOAT* pfRateModifier
		);

	static LTBOOL ComplexVisibilityCheck(
		AISenseRecord* pRecord,
		const LTVector& vStimulusPos,
		LTBOOL bUseSightGrid,
		LTBOOL bUseInstantSeeDistance,
		CAI* pSensingAI,
		HOBJECT hObject,
		LTFLOAT* pfRateModifier
		)
	{
		return ComplexVisibilityCheck(pRecord, vStimulusPos, bUseSightGrid, bUseInstantSeeDistance, pSensingAI, 
			g_pLTServer->HandleToObject(hObject), pfRateModifier);
	}

	static LTBOOL IsInPersonalBubble(
		LTFLOAT fVisDistanceSqr,
		const LTVector& vPosition,
		HOBJECT hObject,
		CAI* pSensingAI,
		LTBOOL bDoIntersectSegment
		);
};

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISenseRecorderTools::GetValidatedRecord()
//              
//	PURPOSE:	Insure that a Stimulus Type is in the SenseRecord
//              
//----------------------------------------------------------------------------
void CAISenseRecorderTools::GetValidatedRecord(const AISENSE_RECORD_MAP& SenseRecords,
												const CAIStimulusRecord* pStimulusRecord,
												AISenseRecord** pRecord)
{
	AISENSE_RECORD_MAP::const_iterator it = SenseRecords.find(pStimulusRecord->m_pAIBM_Stimulus->eSenseType);
	ASSERT(it != SenseRecords.end());
	*pRecord = (*it).second;
//	AISenseRecord Record = *( (*it).second );
}



//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISenseRecorderTools::ComplexVisibilityCheck()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
LTBOOL CAISenseRecorderTools::ComplexVisibilityCheck(AISenseRecord* pSenseRecord,
													 const LTVector& vStimulusPos,
													 LTBOOL bUseSightGrid,
													 LTBOOL bUseInstantSeeDistance,
													 CAI* pSensingAI,
													 ILTBaseClass* pObject,
													 LTFLOAT* pfRateModifier)
{
	LTFLOAT fSenseDistanceSqr = pSenseRecord->fSenseDistance * pSenseRecord->fSenseDistance;

	// Instead of looking right at the center of the target, we look at a grid of 
	// points. The grid is a plane with normal equal to the forward vector of the 
	// object, in the center of the object, clipped to the objects dims. We scan 
	// the grid at a given resolution and simply advance our scan col/row every 
	// frame. Note that the grid is aligned with the objects rotation, not the 
	// bounding boxes, since all the bounding boxes are axis aligned.

	CRange<int> rngGridX = pSensingAI->GetSightGridRangeX();
	CRange<int> rngGridY = pSensingAI->GetSightGridRangeY();

	int nXRange = rngGridX.GetMax() - rngGridX.GetMin();
	int nYRange = rngGridY.GetMax() - rngGridY.GetMin();

	LTVector vRight;
	LTVector vUp;

	// Choose rotation of volume based on location of AI.

    LTVector vDims;
	if( IsAIVolume( pObject ) )
	{
		// Fudge the volume's y dims to be approx height of a room.

		AIVolume* pVolume = (AIVolume*)pObject;

		vDims = pVolume->GetDims();
		vDims.y = AIVolume::kApproxVolumeYDim;

		// Check which side of the volume we should scan.

		LTVector vDir = pVolume->GetCenter() - pSensingAI->GetPosition();
		vDir.y = 0.f;

		LTVector vForward(0.f, 0.f, 1.f);
		LTFLOAT fRot = (LTFLOAT)acos( vForward.Dot( vDir ) );
		if( ( ( fRot > DEGREES_TO_RADIANS( 45.f ) ) && ( fRot < DEGREES_TO_RADIANS( 135.f ) ) ) ||
			( ( fRot > DEGREES_TO_RADIANS( 225.f ) ) && ( fRot < DEGREES_TO_RADIANS( 315.f ) ) ) )
		{
			vRight = LTVector(1.f, 0.f, 0.f);
		}
		else {
			vRight = vForward;
		}

		vUp = LTVector(0.f, 1.f, 0.f);
	}

	// Use rotation of object.

	else if( pObject->m_hObject )
	{
		g_pPhysicsLT->GetObjectDims(pObject->m_hObject, &vDims);

		LTRotation rRot;
		g_pLTServer->GetObjectRotation(pObject->m_hObject, &rRot);

		vRight = rRot.Right();
		vUp = rRot.Up();
	}
	else
	{
		AIASSERT( false, LTNULL , "Unknown object provided to ComplexVisibilityCheck" );
		return LTFALSE;
	}


	// Do not use the volume check optimization if looking at the player, and the player
	// is not in a volume, or under their current volume.

	LTBOOL bDoVolumeCheck = LTTRUE;
	if( IsPlayer( pObject->m_hObject ) )
	{
		CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( pObject->m_hObject );
		AIVolume* pCurVol = pPlayer->GetCurrentVolume();

		if( ( !pCurVol ) || ( ( vStimulusPos.y + vDims.y ) < pCurVol->GetBackBottomRight().y ) )
		{
			bDoVolumeCheck = LTFALSE;
		}
	}

	LTVector vPosition = vStimulusPos;

	CPoint ptGrid;
	if( bUseSightGrid )
	{
		ptGrid = pSenseRecord->ptSightGrid;

		LTFLOAT fX = vDims.x * ((LTFLOAT)ptGrid.x/(LTFLOAT)nXRange);
		LTFLOAT fY = vDims.y * ((LTFLOAT)ptGrid.y/(LTFLOAT)nYRange);

		vPosition += vRight*fX;
		vPosition += vUp*fY;
	}

	// Update the point

    LTFLOAT fVisDistanceSqr;
    LTBOOL bVisible;

	if ( pSensingAI->CanSeeThrough() )
	{
		bVisible = pSensingAI->IsObjectPositionVisibleFromEye(CAI::SeeThroughFilterFn, CAI::SeeThroughPolyFilterFn, pObject->m_hObject, vPosition, fSenseDistanceSqr, LTTRUE, bDoVolumeCheck, LTNULL, &fVisDistanceSqr);
	}
	else
	{
		bVisible = pSensingAI->IsObjectPositionVisibleFromEye(CAI::DefaultFilterFn, NULL, pObject->m_hObject, vPosition, fSenseDistanceSqr, LTTRUE, LTFALSE, LTNULL, &fVisDistanceSqr);
	}

	if ( bVisible )
	{
		if( pfRateModifier )
		{
			CCharacter* pCharacter = LTNULL;
			if ( IsCharacter( pObject ) )
			{
				pCharacter = (CCharacter*)pObject;
			}

			//
			// Ignore InstantSeeDistance.
			// See instantly only if target is within personal bubble.
			// Otherwise do not see if hidden.
			// Modify for distance and darkness.
			//

			if( !bUseInstantSeeDistance )
			{
				// The target is the seen instantly in the personal bubble.

				if( IsInPersonalBubble( fVisDistanceSqr, vPosition, 0, pSensingAI, LTFALSE ) )
				{
					AITRACE( AIShowSenses, ( pSensingAI->m_hObject, "AI noticed target in personal bubble." ) );
					*pfRateModifier = 99999999.0f;
					return LTTRUE;
				}

				// Target is not seen if hidden.

				if ( pCharacter && pCharacter->IsHidden() )
				{
					return LTFALSE;
				}

				// Modify the rate by the distance to the target.

				*pfRateModifier = ( 1.0f - (fVisDistanceSqr/ fSenseDistanceSqr) );

				// Check if target is a character standing in the dark.

				if( pCharacter )
				{
					if( pCharacter->HasLastVolume() 
						&& ( !pCharacter->GetLastVolume()->IsLit() ) 
						&& ( !pCharacter->IsFlashLightOn() ) )
					{
						*pfRateModifier *= g_pAIButeMgr->GetSenses()->fDarknessMultiplier;
					}
				}

				return LTTRUE;
			}

			//
			// Target is outside of the InstantSeeDistance.
			// Do not see if hidden.
			// Modify rate for distance and darkness.
			//

			else if( fVisDistanceSqr > g_pAIButeMgr->GetSenses()->fInstantSeeDistanceSqr )
			{
				// Target is not seen if hidden.

				if ( pCharacter && pCharacter->IsHidden() )
				{
					return LTFALSE;
				}

				// Modify the rate by the distance to the target.

				*pfRateModifier = ( 1.0f - (fVisDistanceSqr/ fSenseDistanceSqr) );

				// Check if target is a character standing in the dark.

				if( pCharacter )
				{
					if( pCharacter->HasLastVolume() 
						&& ( !pCharacter->GetLastVolume()->IsLit() ) 
						&& ( !pCharacter->IsFlashLightOn() ) )
					{
						*pfRateModifier *= g_pAIButeMgr->GetSenses()->fDarknessMultiplier;
					}
				}
			}

			//
			// Target is inside of the InstantSeeDistance.
			// See intantly.
			// Do not see if hidden and outside of personal bubble.
			//

			else
			{
				// Check if target is hidden.
				// If target is within the AIs personal bubble, he is seen
				// even if hidden.

				if( pCharacter && pCharacter->IsHidden() &&
					( fVisDistanceSqr > g_pAIButeMgr->GetSenses()->fPersonalBubbleDistanceSqr ) )
				{
					return LTFALSE;
				}

				// The target is the seen instantly.

				*pfRateModifier = 99999999.0f;
			}
		}
	}

	else {

		// If the target is outside the AIs FOV, but inside his personal bubble,
		// the AI senses the target's presence (6th sense) gradually.  If the target
		// stays there, he will eventually be noticed.

		if( ( !pSensingAI->IsAlert() ) && IsInPersonalBubble( fVisDistanceSqr, vPosition, pObject->m_hObject, pSensingAI, LTTRUE ) )
		{
			AITRACE( AIShowSenses, ( pSensingAI->m_hObject, "AI noticed target in personal bubble." ) );
			if( pfRateModifier )
			{
				*pfRateModifier *= g_pAIButeMgr->GetSenses()->fPersonalBubbleMultiplier;
			}

			return LTTRUE;
		}

		if( bUseSightGrid )
		{
			// Update our grid col/row values

			if ( ++ptGrid.x > rngGridX.GetMax() )
			{
				ptGrid.x = rngGridX.GetMin();
	
				if ( ++ptGrid.y > rngGridY.GetMax() )
				{
					ptGrid.y = rngGridY.GetMin();
				}
			}
	
			pSenseRecord->ptSightGrid = ptGrid;
		}
	}

	return bVisible;
}


LTBOOL CAISenseRecorderTools::IsInPersonalBubble(LTFLOAT fVisDistanceSqr,
												 const LTVector& vPosition,
												 HOBJECT hObject,
												 CAI* pSensingAI,
												 LTBOOL bDoIntersectSegment)
{
	LTFLOAT fBubbleDistSqr = g_pAIButeMgr->GetSenses()->fPersonalBubbleDistanceSqr;

	if( fVisDistanceSqr < fBubbleDistSqr )
	{
		if( !bDoIntersectSegment )
		{
			return LTTRUE;
		}

		if ( pSensingAI->CanSeeThrough() )
		{
			return pSensingAI->IsObjectPositionVisibleFromEye(CAI::SeeThroughFilterFn, CAI::SeeThroughPolyFilterFn, hObject, vPosition, fBubbleDistSqr, LTFALSE, LTTRUE);
		}
		else
		{
			return pSensingAI->IsObjectPositionVisibleFromEye(CAI::DefaultFilterFn, NULL, hObject, vPosition, fBubbleDistSqr, LTFALSE, LTFALSE);
		}
	}

	return LTFALSE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderGame::HandleSpecificStimuli()
//
//	PURPOSE:	Check if senses were updated this cycle, and react accordingly.
//
// ----------------------------------------------------------------------- //
LTBOOL CAISenseRecorderGame::HandleSpecificStimuli(CAIStimulusRecord* pStimulusRecord, LTFLOAT* pfRateModifier)
{
	ASSERT(pStimulusRecord);

	switch(pStimulusRecord->m_eStimulusType)
	{
		case kStim_EnemyWeaponImpactVisible:	return HandleEnemyWeaponImpactVisible(pStimulusRecord);
		case kStim_EnemyFootprintVisible:		return HandleEnemyFootprintVisible(pStimulusRecord);
		case kStim_EnemyVisible:				return HandleEnemyVisible(pStimulusRecord, LTTRUE, LTTRUE, pfRateModifier);
		case kStim_EnemyLeanVisible:			return HandleEnemyVisible(pStimulusRecord, LTFALSE, LTFALSE, pfRateModifier);
		case kStim_EnemyDangerVisible:			return HandleEnemyDangerVisible(pStimulusRecord);
		case kStim_EnemyDisturbanceVisible:		return HandleEnemyDisturbanceVisible(pStimulusRecord);
		case kStim_EnemyLightDisturbanceVisible:return HandleEnemyLightDisturbanceVisible(pStimulusRecord);
		case kStim_EnemyAlarmSound:				return HandleEnemyAlarmSound(pStimulusRecord);
		case kStim_AllyDisturbanceVisible:		return HandleAllyDisturbanceVisible(pStimulusRecord);
		case kStim_AllySpecialDamageVisible:	return HandleAllySpecialDamageVisible(pStimulusRecord);
		case kStim_AllyDeathVisible:			return HandleAllyDeathVisible(pStimulusRecord);
		case kStim_UndeterminedVisible:			return HandleUndeterminedVisible(pStimulusRecord, pfRateModifier);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderGame::HandleEnemyWeaponImpactVisible()
//
//	PURPOSE:	Do visibility checks.
//
// ----------------------------------------------------------------------- //
LTBOOL CAISenseRecorderGame::HandleEnemyWeaponImpactVisible(CAIStimulusRecord* pStimulusRecord)
{
	AIASSERT( pStimulusRecord, m_pSensing->GetSensingObject(), "CAISenseRecorderGame::HandleEnemyWeaponImpactVisible: No stimulus record." );
		
	if( !IsAI( m_pSensing->GetSensingObject() ) )
	{
		return LTFALSE;
	}

	CAI* pAI = (CAI*)m_pSensing;
		
	// If the Target is NULL, the AI was just deleted and replaced with a Body.

	if( IsCharacter(pStimulusRecord->m_hStimulusTarget) )
	{
		// We need to use the hStimulusTarget, because the visibility check is concerned
		// with who got hit.  The SenseMgr checks who fired the weapon.
		HOBJECT hObject = pStimulusRecord->m_hStimulusTarget;

		if( hObject )
		{
			if ( pAI->CanSeeThrough() )
			{
				return pAI->IsObjectVisibleFromEye(CAI::SeeThroughFilterFn, CAI::SeeThroughPolyFilterFn, hObject, (250000.0f), LTTRUE, LTTRUE);
			}
			else
			{
				return pAI->IsObjectVisibleFromEye(CAI::DefaultFilterFn, NULL, hObject, (250000.0f), LTTRUE, LTFALSE);
			}
		}
	}

	// Look at the position of the impact if the AI is dead.

	return pAI->IsPositionVisibleFromEye(CAI::DefaultFilterFn, NULL, pStimulusRecord->m_vStimulusPos, (250000.0f), LTTRUE, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderGame::HandleAllyDeathVisible()
//
//	PURPOSE:	Do visibility check.
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecorderGame::HandleAllyDeathVisible(CAIStimulusRecord* pStimulusRecord)
{
	if( !IsAI( m_pSensing->GetSensingObject() ) )
	{
		return LTFALSE;
	}

	CAI* pAI = (CAI*)m_pSensing;

	AISENSE_RECORD_MAP::iterator it = m_mapSenseRecords.find(pStimulusRecord->m_pAIBM_Stimulus->eSenseType);
	ASSERT(it != m_mapSenseRecords.end());

	LTFLOAT fDistance = it->second->fSenseDistance;

	if( IsBody( pStimulusRecord->m_hStimulusSource ) )
	{
		return pAI->IsObjectVisibleFromEye(CAI::BodyFilterFn, NULL, pStimulusRecord->m_hStimulusSource, fDistance*fDistance, LTTRUE, LTTRUE);
	}
	else if( IsAI( pStimulusRecord->m_hStimulusSource ) )
	{
		return pAI->IsObjectVisibleFromEye(CAI::DefaultFilterFn, NULL, pStimulusRecord->m_hStimulusSource, fDistance*fDistance, LTTRUE, LTTRUE);
	}

	AIASSERT( 0, pStimulusRecord->m_hStimulusSource, "CAISenseRecorderGame::HandleAllyDeathVisible: Stimulus must be an AI or body." );
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderGame::HandleEnemyFootprintVisible()
//
//	PURPOSE:	Do visibility check.
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecorderGame::HandleEnemyFootprintVisible(CAIStimulusRecord* pStimulusRecord)
{
	ASSERT(pStimulusRecord && IsCharacter(pStimulusRecord->m_hStimulusSource));

	if( !IsAI( m_pSensing->GetSensingObject() ) )
	{
		return LTFALSE;
	}

	CAI* pAI = (CAI*)m_pSensing;

	AISENSE_RECORD_MAP::iterator it = m_mapSenseRecords.find(pStimulusRecord->m_pAIBM_Stimulus->eSenseType);
	ASSERT(it != m_mapSenseRecords.end());

	LTFLOAT fDistance = it->second->fSenseDistance;
	return pAI->IsPositionVisibleFromEye(CAI::DefaultFilterFn, NULL, pStimulusRecord->m_vStimulusPos, fDistance*fDistance, LTTRUE, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderGame::HandleEnemyDangerVisible()
//
//	PURPOSE:	Do visibility check.
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecorderGame::HandleEnemyDangerVisible(CAIStimulusRecord* pStimulusRecord)
{
	ASSERT(pStimulusRecord && IsCharacter(pStimulusRecord->m_hStimulusSource));

	if( !IsAI( m_pSensing->GetSensingObject() ) )
	{
		return LTFALSE;
	}

	CAI* pAI = (CAI*)m_pSensing;

	// We need to use the hStimulusTarget, because the visibility check is concerned
	// with the object of danger.
	HOBJECT hObject = pStimulusRecord->m_hStimulusTarget;

	AISENSE_RECORD_MAP::iterator it = m_mapSenseRecords.find(pStimulusRecord->m_pAIBM_Stimulus->eSenseType);
	ASSERT(it != m_mapSenseRecords.end());

	LTFLOAT fDistance = it->second->fSenseDistance + pStimulusRecord->m_fDistance;
	return pAI->IsObjectVisibleFromEye(CAI::DefaultFilterFn, NULL, hObject, fDistance*fDistance, LTTRUE, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderGame::HandleEnemyDisturbanceVisible()
//
//	PURPOSE:	Do visibility check.
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecorderGame::HandleEnemyDisturbanceVisible(CAIStimulusRecord* pStimulusRecord)
{
	ASSERT(pStimulusRecord && IsCharacter(pStimulusRecord->m_hStimulusSource));

	if( !IsAI( m_pSensing->GetSensingObject() ) )
	{
		return LTFALSE;
	}

	CAI* pAI = (CAI*)m_pSensing;

	// We need to use the hStimulusTarget, because the visibility check is concerned
	// with what got disturbed.  The SenseMgr checks who disturbed it.
	HOBJECT hObject = pStimulusRecord->m_hStimulusTarget;

	AISENSE_RECORD_MAP::iterator it = m_mapSenseRecords.find(pStimulusRecord->m_pAIBM_Stimulus->eSenseType);
	ASSERT(it != m_mapSenseRecords.end());

	LTFLOAT fDistance = it->second->fSenseDistance + pStimulusRecord->m_fDistance;
	return pAI->IsObjectVisibleFromEye(CAI::DefaultFilterFn, NULL, hObject, fDistance*fDistance, LTTRUE, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderGame::HandleEnemyLightDisturbanceVisible()
//
//	PURPOSE:	Do visibility check.
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecorderGame::HandleEnemyLightDisturbanceVisible(CAIStimulusRecord* pStimulusRecord)
{
	// Light disturbances are caused by AIVolumes becoming lit / unlit.

	//AIASSERT(pStimulusRecord && IsAIVolume(pStimulusRecord->m_hStimulusSource), m_pSensing->GetSensingObject(), "CAISenseRecorderGame::HandleEnemyLightDisturbanceVisible: Only AIVolumes can cause light disturbances.");

	if( !IsAI( m_pSensing->GetSensingObject() ) )
	{
		return LTFALSE;
	}

	CAI* pAI = (CAI*)m_pSensing;

	LTBOOL bCanSee = LTFALSE;

	// Check if the AI is inside the volume.

	AIVolume* pVolume = g_pAIVolumeMgr->FindContainingVolume( LTNULL, pStimulusRecord->m_vStimulusPos, eAxisAll );
	if( !pVolume )
	{
		return LTFALSE;
	}

	if( pVolume->InsideMasked( pAI->GetPosition(), eAxisAll, pAI->GetVerticalThreshold() ) )
	{
		bCanSee = LTTRUE;
	}

	// Raise the stimulus pos to approx center of room.

	LTVector vCheckPos = pStimulusRecord->m_vStimulusPos;
	vCheckPos.y = pVolume->GetBackBottomLeft().y + AIVolume::kApproxVolumeYDim;

	// Check if the AI can see the volume.

	AISenseRecord* pSelectedSenseRecord;

	CAISenseRecorderTools::GetValidatedRecord( 
		m_mapSenseRecords,
		pStimulusRecord,
		&pSelectedSenseRecord );

	// Do ComplexVisibilityCheck to scan for volume visibility.

	if( CAISenseRecorderTools::ComplexVisibilityCheck(
		pSelectedSenseRecord,
		vCheckPos,
		LTTRUE,
		LTTRUE,
		pAI,
		pVolume,
		LTNULL ) )
	{
		bCanSee = LTTRUE;
	}

	// AI should ignore other stimulus for volumes referring to the same light.

	if( bCanSee )
	{
		g_pAIStimulusMgr->IgnoreStimulusByTarget( m_pSensing->GetSensingObject(), kStim_EnemyLightDisturbanceVisible, pVolume->GetLightSwitchUseObjectNode() );
	}

	return bCanSee;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderGame::HandleEnemyAlarmSound()
//
//	PURPOSE:	Do region check.
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecorderGame::HandleEnemyAlarmSound(CAIStimulusRecord* pStimulusRecord)
{
	if( !IsAI( m_pSensing->GetSensingObject() ) )
	{
		return LTFALSE;
	}

	CAI* pAI = (CAI*)m_pSensing;

	// AI must be in a region.

	if ( ( !pAI->HasLastVolume() ) || ( !pAI->GetLastVolume()->HasRegion() ) )
	{
		return LTFALSE;
	}

	// Region must exist.

	AIRegion* pRegion = pAI->GetLastVolume()->GetRegion();
	if( !pRegion )
	{
		AIASSERT( 0, pAI->m_hObject, "CAISenseRecorderGame::HandleEnemyAlarmSound: Region is NULL" );
		return LTFALSE;
	}

	// Stimulus must have an Alarm as Target.

	HOBJECT hAlarm = pStimulusRecord->m_hStimulusTarget;
	AIASSERT( IsAlarm( hAlarm ), pAI->m_hObject, "CAISenseRecorderGame::HandleEnemyAlarmSound: Target is not an Alarm" );

	// Alarm must exist.

	Alarm* pAlarm = (Alarm*)g_pLTServer->HandleToObject( hAlarm );
	if( !pAlarm )
	{
		AIASSERT( 0, pAI->m_hObject, "CAISenseRecorderGame::HandleEnemyAlarmSound: Alarm is NULL" );
		return LTFALSE;
	}
	
	// Alarm must have the region in its alert or respond group.

	return pAlarm->IsRegionCovered( pRegion->m_hObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderGame::HandleAllyDisturbanceVisible()
//
//	PURPOSE:	Do visibility check.
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecorderGame::HandleAllyDisturbanceVisible(CAIStimulusRecord* pStimulusRecord)
{
	ASSERT(pStimulusRecord && IsAI(pStimulusRecord->m_hStimulusSource));

	if( !IsAI( m_pSensing->GetSensingObject() ) )
	{
		return LTFALSE;
	}

	CAI* pAI = (CAI*)m_pSensing;

	// Ignore disturbed allies if already suspicious.

	if( pAI->IsSuspicious() )
	{
		return LTFALSE;
	}

	// Ignore stimulus that occured before AI returned to a relaxed state.

	if( !IsAI( pStimulusRecord->m_hStimulusSource ) )
	{
		return LTFALSE;
	}

	CAI* pAlly = (CAI*)g_pLTServer->HandleToObject(pStimulusRecord->m_hStimulusSource);
	if( pAlly && ( pAI->GetLastRelaxedTime() > pAlly->GetLastStimulusTime() ) )
	{
		return LTFALSE;
	}
	

	AISENSE_RECORD_MAP::iterator it = m_mapSenseRecords.find(pStimulusRecord->m_pAIBM_Stimulus->eSenseType);
	ASSERT(it != m_mapSenseRecords.end());

	// Check the line of sight from the stimulus to the AI, since it is the suspicious AI
	// that needs to tell the other AI about his suspicion.
	
	LTFLOAT fDistance = it->second->fSenseDistance + pStimulusRecord->m_pAIBM_Stimulus->fDistance;
	LTBOOL bSee = pAlly->IsObjectVisibleFromEye(CAI::DefaultFilterFn, NULL, pAI->m_hObject, fDistance*fDistance, LTFALSE, LTTRUE);

	return bSee;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderGame::HandleAllySpecialDamageVisible()
//
//	PURPOSE:	Do visibility check.
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecorderGame::HandleAllySpecialDamageVisible(CAIStimulusRecord* pStimulusRecord)
{
	ASSERT(pStimulusRecord && IsCharacter(pStimulusRecord->m_hStimulusSource));

	if( !IsAI( m_pSensing->GetSensingObject() ) )
	{
		return LTFALSE;
	}

	CAI* pAI = (CAI*)m_pSensing;

	HOBJECT hObject = pStimulusRecord->m_hStimulusSource;

	AISENSE_RECORD_MAP::iterator it = m_mapSenseRecords.find(pStimulusRecord->m_pAIBM_Stimulus->eSenseType);
	ASSERT(it != m_mapSenseRecords.end());

	LTFLOAT fDistance = it->second->fSenseDistance + pStimulusRecord->m_pAIBM_Stimulus->fDistance;
	return pAI->IsObjectVisibleFromEye(CAI::DefaultFilterFn, NULL, hObject, fDistance*fDistance, LTTRUE, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderGame::HandleEnemyVisible()
//
//	PURPOSE:	Do visibility check.
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecorderGame::HandleEnemyVisible(CAIStimulusRecord* pStimulusRecord, LTBOOL bUseSightGrid, LTBOOL bUseInstantSeeDistance, LTFLOAT* pfRateModifier)
{
	ASSERT(pStimulusRecord && IsCharacter(pStimulusRecord->m_hStimulusSource));

	if( !IsAI( m_pSensing->GetSensingObject() ) )
	{
		return LTFALSE;
	}

	CAI* pAI = (CAI*)m_pSensing;

	// If we are neutral with respect to the other character...
	HOBJECT hObject = pStimulusRecord->m_hStimulusSource;
	CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(hObject);
	if( !pCharacter )
	{
		AIASSERT( 0, pAI->m_hObject, "HandleEnemyVisible: Unable to convert m_hStimulusSource to pChar" );
		return LTFALSE;
	}

	if ( NEUTRAL == GetRelativeAlignment( pAI->GetRelationSet(), pCharacter->GetRelationData()) )
	{
		// If we are neutral with respect to the other character, and if the 
		// character we are neutral to does NOT have a dangerous looking weapon
		// aimed at me, fail the check.
		if ( !pAI->GetBrain()->CharacterIsAimingAtMe( pCharacter ) )
		{
			return LTFALSE;
		}
	}

	AISenseRecord* pSelectedSenseRecord;

	CAISenseRecorderTools::GetValidatedRecord( 
		m_mapSenseRecords,
		pStimulusRecord,
		&pSelectedSenseRecord );

	// Return the result of the ComplexVisibilityCheck
	if( CAISenseRecorderTools::ComplexVisibilityCheck(
			pSelectedSenseRecord,
			pStimulusRecord->m_vStimulusPos,
			bUseSightGrid,
			bUseInstantSeeDistance,
			pAI,
			pStimulusRecord->m_hStimulusSource,
			&(*pfRateModifier)
			))
	{
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderGame::HandleUndeterminedVisible()
//
//	PURPOSE:	Do visibility check.
//
// ----------------------------------------------------------------------- //
LTBOOL CAISenseRecorderGame::HandleUndeterminedVisible(CAIStimulusRecord* pStimulusRecord,
													   LTFLOAT* pfRateModifier)
{
	ASSERT(pStimulusRecord);

	if( !IsAI( m_pSensing->GetSensingObject() ) )
	{
		return LTFALSE;
	}

	CAI* pAI = (CAI*)m_pSensing;

	if ( IsCharacter(pStimulusRecord->m_hStimulusSource) )
	{

		// If we are neutral with respect to the other character...
		HOBJECT hObject = pStimulusRecord->m_hStimulusSource;
		CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(hObject);
		UBER_ASSERT( pCharacter != NULL, "HandleEnemyVisible: Unable to convert m_hStimulusSource to pChar" );

		if ( NEUTRAL == GetRelativeAlignment( pAI->GetRelationSet(), pCharacter->GetRelationData()) )
		{
			// If we are neutral with respect to the other character, and if the 
			// character we are neutral to does NOT has a dangerous looking weapon
			// the fail the check
			if ( !pCharacter->HasDangerousWeapon() )
			{
				return LTFALSE;
			}
		}
	}

	AISenseRecord* pSelectedSenseRecord;

	CAISenseRecorderTools::GetValidatedRecord( 
		m_mapSenseRecords,
		pStimulusRecord,
		&pSelectedSenseRecord );

	// Return the result of the ComplexVisibilityCheck
	return CAISenseRecorderTools::ComplexVisibilityCheck(
		pSelectedSenseRecord,
		pStimulusRecord->m_vStimulusPos,
		LTTRUE,
		LTTRUE,
		pAI,
		pStimulusRecord->m_hStimulusSource,
		&(*pfRateModifier)
		);
}
