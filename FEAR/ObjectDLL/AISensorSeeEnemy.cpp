// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSeeEnemy.cpp
//
// PURPOSE : AISensorSeeEnemy class implementation
//
// CREATED : 1/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorSeeEnemy.h"
#include "AI.h"
#include "AIDB.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AIStimulusMgr.h"
#include "AISensorMgr.h"
#include "AIUtils.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorSeeEnemy, kSensor_SeeEnemy );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeEnemy::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorSeeEnemy::CAISensorSeeEnemy()
{
	m_bUseFOV = true;
	m_hLastGridTarget = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorSeeEnemy::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorSeeEnemy
//
//----------------------------------------------------------------------------
void CAISensorSeeEnemy::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_bool( m_bUseFOV );
	SAVE_HOBJECT( m_hLastGridTarget );
	SAVE_INT(m_ptSightGrid.x);
	SAVE_INT(m_ptSightGrid.y);
}

void CAISensorSeeEnemy::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_bool( m_bUseFOV );
	LOAD_HOBJECT( m_hLastGridTarget );
	LOAD_INT(m_ptSightGrid.x);
	LOAD_INT(m_ptSightGrid.y);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeEnemy::InitSensor
//
//	PURPOSE:	Initialize the sensor.
//
// ----------------------------------------------------------------------- //

void CAISensorSeeEnemy::InitSensor( EnumAISensorType eSensorType, CAI* pAI )
{
	super::InitSensor( eSensorType, pAI );

	m_ptSightGrid.x = pAI->GetSightGridRangeX().GetMin();
	m_ptSightGrid.y = pAI->GetSightGridRangeY().GetMin();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeEnemy::GetSenseDistSqr
//
//	PURPOSE:	Return the square of the distance the AI can sense.
//
// ----------------------------------------------------------------------- //

float CAISensorSeeEnemy::GetSenseDistSqr( float fStimulusRadius )
{
	float fSenseDistanceSqr = m_pAI->GetAIBlackBoard()->GetBBSeeDistance() + fStimulusRadius;
	return fSenseDistanceSqr * fSenseDistanceSqr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeEnemy::CreateWorkingMemoryFact
//
//	PURPOSE:	Return the working memory fact that will hold the 
//              memory of sensing this stimulus.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAISensorSeeEnemy::CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord )
{
	// Find an existing memory for this character.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Character);
	factQuery.SetTargetObject(pStimulusRecord->m_hStimulusSource);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );

	// Create a new memory for this character.

	if( !pFact )
	{
		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Character );
	}

	return pFact;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeEnemy::IncreaseStimulation()
//
//	PURPOSE:	Increase stimulation of a sense.
//
// ----------------------------------------------------------------------- //

float CAISensorSeeEnemy::IncreaseStimulation( float fCurStimulation, float fRateModifier, CAIStimulusRecord* pStimulusRecord, AIDB_StimulusRecord* pStimulus )
{
	float fStimulation = super::IncreaseStimulation( fCurStimulation, fRateModifier, pStimulusRecord, pStimulus );

	// Alert allies of a threat if AI is fully stimulated by a new character
	// or if we have not targeted a character before.  This is required 
	// because if an AI fails to target the character the first frame the 
	// stimulus goes to 1, nothing causes the target selection to reevaluate.
	// This may occur if a character is targetting a disturbance and sees 
	// the character while they are outside the nav mesh; this causes the
	// target action to fail.  All of the other target actions fail because
	// a character has not been targetted previously.

	if( ( fCurStimulation < 1.f ) && ( fStimulation >= 1.f ) )
	{
		// Become alert when we see an enemy!

		m_pAI->GetAIBlackBoard()->SetBBAwareness( kAware_Alert );

		// Re-evaluate targets when a new character is discovered.

		m_pAI->GetAIBlackBoard()->SetBBInvalidateTarget( true );

		// Consider current target to be NULL if we have lost track of the target.

		HOBJECT hCurTarget = ( m_pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags() & kTargetTrack_Normal ) ? m_pAI->GetAIBlackBoard()->GetBBTargetObject() : NULL;
		if( !m_pAI->HasTarget( kTarget_Character ) )
		{
			hCurTarget = NULL;
		}
		if( hCurTarget != pStimulusRecord->m_hStimulusSource )
		{
			// Stimulus comes from the AI, and references the threat.

			StimulusRecordCreateStruct scs( kStim_SquadCommunicationThreat, m_pAI->GetAlignment(), pStimulusRecord->m_vStimulusPos, m_pAI->m_hObject );
			scs.m_hStimulusTarget = pStimulusRecord->m_hStimulusSource;

			// Send stimulus out next update.
			// Can't send it within the handling of a stimulus.

			DispatchStimulus( scs );
		}
	}

	if ( ( fStimulation >= 1.0 ) && !( m_pAI->GetAIBlackBoard()->GetBBTargetedTypeMask() & kTarget_Character ) )
	{
		// Re-evaluate targets when a new character is discovered.

		m_pAI->GetAIBlackBoard()->SetBBInvalidateTarget( true );
	}

	return fStimulation;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeEnemy::FindUnstimulatedWorkingMemoryFact
//
//	PURPOSE:	Return a working memory fact that has not been 
//              stimulated this update.
//
// ----------------------------------------------------------------------- //

void CAISensorSeeEnemy::FindUnstimulatedWorkingMemoryFact(AIWORKING_MEMORY_FACT_LIST* pOutFactList)
{
	if (!pOutFactList)
	{
		return;
	}

	double fComparisonTime = m_pAI->GetAISensorMgr()->GetStimulusListNewIterationTime();
	m_pAI->GetAIWorkingMemory()->CollectFactsUnupdated(kFact_Character, pOutFactList, fComparisonTime);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeEnemy::ComplexVisibilityCheck
//
//	PURPOSE:	Return true if the stimulus can be sensed.
//
// ----------------------------------------------------------------------- //

bool CAISensorSeeEnemy::DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier )
{
	float fSenseDistanceSqr = GetSenseDistSqr( pStimulusRecord->m_fDistance );
	
	HOBJECT hObject = pStimulusRecord->m_hStimulusSource;

	// Instead of looking right at the center of the target, we look at a grid of 
	// points. The grid is a plane with normal equal to the forward vector of the 
	// object, in the center of the object, clipped to the objects dims. We scan 
	// the grid at a given resolution and simply advance our scan col/row every 
	// frame. Note that the grid is aligned with the objects rotation, not the 
	// bounding boxes, since all the bounding boxes are axis aligned.

	CRange<int> rngGridX = m_pAI->GetSightGridRangeX();
	CRange<int> rngGridY = m_pAI->GetSightGridRangeY();

	int nXRange = rngGridX.GetMax() - rngGridX.GetMin();
	int nYRange = rngGridY.GetMax() - rngGridY.GetMin();

	float fXIncr = 2.f / (float)nXRange;
	float fYIncr = 2.f / (float)nYRange;

	// Use rotation of object.

	LTVector vPosition = pStimulusRecord->m_vStimulusPos;

	LTVector vDims;
	g_pPhysicsLT->GetObjectDims( hObject, &vDims );

	LTVector vDirToTarget = vPosition - m_pAI->GetEyePosition();
	if ( vDirToTarget.NearlyEquals( LTVector::GetIdentity(), 0.1f ) )
	{
		// If your eye is inside of the target, you can see it.  Don't 
		// bother checking further.  This simplifies cross/dot 
		// calculations below
		return true;
	}

	vDirToTarget.Normalize();

	LTVector vPretendUp( 0.f, 1.f, 0.f );

	// Find a stable Up and Right.

	LTVector vUp;
	LTVector vRight;
	if ( fabs( vPretendUp.Dot( vDirToTarget ) ) > 0.98 )
	{
		// If our direction to the target is aligned with our PretendUp, we 
		// actually don't easily have enough knowledge here to determine the 
		// 'real' up and right for the eyes.  As we still need valid vectors,
		// get a pair of vectors perpendicular to the direction we are looking
		// and use them.  This may result in the grids flipping when the target
		// is straight up or down, but this is adequate for handling this 
		// boundary condition.
		vRight	= vDirToTarget.BuildOrthonormal().Cross( vDirToTarget );
		vRight.Normalize();
		
		vUp		= vRight.Cross( vDirToTarget );
		vUp.Normalize();
	}
	else
	{
		// Preserve the old behavior when it is safe to do so.
		vRight	= vPretendUp.Cross( vDirToTarget );
		vRight.Normalize();

		vUp		= vPretendUp;
	}

	float fX = vDims.x * ( -1.f + ( ((float)( m_ptSightGrid.x - rngGridX.GetMin() )) * fXIncr ) );
	float fY = vDims.y * ( -1.f + ( ((float)( m_ptSightGrid.y - rngGridY.GetMin() )) * fYIncr ) );

	vPosition += vRight*fX;
	vPosition += vUp*fY;


	float fVisDistanceSqr;
	bool bVisible = CheckVisibility( hObject, vPosition, fSenseDistanceSqr, &fVisDistanceSqr );

	if ( bVisible )
	{
		if( pfRateModifier )
		{
			//
			// Target is outside of the InstantSeeDistance.
			// Modify rate for distance and darkness.
			//

			float fInstantSeeDistSqr;
			if( ( m_pAI->GetAIBlackBoard()->GetBBAwareness() == kAware_Alert ) &&
				( m_pAI->GetAIBlackBoard()->GetBBAwarenessMod() == kAwarenessMod_ImmediateThreat ) )
			{
				fInstantSeeDistSqr = g_pAIDB->GetAIConstantsRecord()->fAlertImmediateThreatInstantSeeDistanceSqr;
			}
			else
			{
				fInstantSeeDistSqr = g_pAIDB->GetAIConstantsRecord()->fInstantSeeDistanceSqr;
			}

			if( fVisDistanceSqr > fInstantSeeDistSqr )
			{
				// Modify the rate by the distance to the target.

				*pfRateModifier = ( 1.0f - (fVisDistanceSqr/ fSenseDistanceSqr) );
			}

			//
			// Target is inside of the InstantSeeDistance.
			// See intantly.
			//

			else
			{
				// The target is the seen instantly.

				*pfRateModifier = 99999999.0f;
			}
		}
	}

	else 
	{
		// Update our grid col/row values.  
		
		// Randomize grid position when targets change as an AI may be iterating over several targets.

		if( m_hLastGridTarget != hObject )
		{
			m_hLastGridTarget = hObject;
			m_ptSightGrid.y = GetRandom(rngGridY.GetMin(), rngGridY.GetMax());
			m_ptSightGrid.x = GetRandom(rngGridX.GetMin(), rngGridX.GetMax());
		}
		
		// Iterate grid positions over the same target.

		else if ( ++m_ptSightGrid.y > rngGridY.GetMax() )
		{
  			m_ptSightGrid.y = rngGridY.GetMin();
  	
  			if ( ++m_ptSightGrid.x > rngGridX.GetMax() )
  			{
  				m_ptSightGrid.x = rngGridX.GetMin();
  			}
		}
	}

	return bVisible;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeEnemy::CheckVisibility
//
//	PURPOSE:	Return true if the stimulus can be sensed.
//
// ----------------------------------------------------------------------- //

bool CAISensorSeeEnemy::CheckVisibility( HOBJECT hObject, const LTVector& vPosition, float fSenseDistanceSqr, float* pfVisDistanceSqr )
{
	bool bVisible;
	float fVisDistanceSqr;
	if ( m_pAI->CanSeeThrough() )
	{
		bVisible = !!m_pAI->IsObjectPositionVisible( CAI::SeeThroughFilterFn, CAI::SeeThroughPolyFilterFn, m_pAI->GetEyePosition(), hObject, vPosition, fSenseDistanceSqr, m_bUseFOV, false, NULL, &fVisDistanceSqr );
	}
	else
	{
		bVisible = !!m_pAI->IsObjectPositionVisible( CAI::DefaultFilterFn, NULL, m_pAI->GetEyePosition(), hObject, vPosition, fSenseDistanceSqr, m_bUseFOV, false, NULL, &fVisDistanceSqr );
	}

	if( pfVisDistanceSqr )
	{
		*pfVisDistanceSqr = fVisDistanceSqr;
	}
	return bVisible;
}

