// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorFlashlight.cpp
//
// PURPOSE : AISensorFlashlight class implementation
//
// CREATED : 09/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorFlashlight.h"
#include "AIStimulusMgr.h"
#include "ServerSoundMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorFlashlight, kSensor_Flashlight );

#define STUNNED_FX				"StunnedFX"
#define GHOST_LOOP_SOUND		"GhostLoopSound"
#define GHOST_VULNERABLE_TIME	"GhostVulnerableTime"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorFlashlight::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorFlashlight::CAISensorFlashlight()
{
	m_bInvulnerable = false;
	m_fBecomeInvulnerableTime = 0.f;
	m_pAIDBFlashlight = NULL;
	m_hLoopSound = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorFlashlight::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorSeeEnemy
//
//----------------------------------------------------------------------------
void CAISensorFlashlight::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL( m_bInvulnerable );
	SAVE_TIME( m_fBecomeInvulnerableTime );
}

void CAISensorFlashlight::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL( m_bInvulnerable );
	LOAD_TIME( m_fBecomeInvulnerableTime );
	CreateExposureFX( !m_bInvulnerable );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorFlashlight::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorFlashlight::UpdateSensor()
{
	super::UpdateSensor();

	// Hack for using the flashlight with Dark Alma instead of the Ghost.
	// Alma dies on impact.  Alma is not invulnerable.

	m_pAI->GetAIBlackBoard()->SetBBHandleTouch( kTouch_Damage );

/****
	// Play the looping sound.
	// Note: There may someday be a separate sound looped while burning 
	// from flashlight exposure.
	// Note: If anyone other than the ghost wants to use this sensor,
	// derive a ghost version, and migrate the sound looping to the new class.

	if( !m_hLoopSound )
	{
		LoopSound();
	}

	// Don't do anything if senses are off.

	if( !m_pAI->HasTarget( kTarget_Character ) )
	{
		return false;
	}

	// Find an existing memory for the desire to be stunned.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Desire );
	factQuery.SetDesireType( kDesire_Stunned );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );

	// AI is invulnerable if he has never been stimulated.

	if( !pFact )
	{
		MakeInvulnerable( true );
	}

	// AI is invulnerable again if we've passed the time to become invulnerable again.

	else if( ( !m_bInvulnerable ) && 
			 ( g_pLTServer->GetTime() > m_fBecomeInvulnerableTime ) )
	{
		MakeInvulnerable( true );
	}

	// AI is vulnerable if stimulation reached 1.

	else if( pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus ) == 1.f )
	{
		MakeInvulnerable( false );

		// Find an existing memory for the desire to flee, or create one.

		factQuery.SetDesireType(kDesire_Flee);
		pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( !pFact )
		{
			pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Desire );
			pFact->SetDesireType( kDesire_Flee, 1.f );

			// Immediately re-evaluate goals if AI wants to flee.

			m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
		}
	}
***/

	// Always allow other sensors to update.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorFlashlight::LoopSound
//
//	PURPOSE:	Play a looping sound.
//
// ----------------------------------------------------------------------- //

void CAISensorFlashlight::LoopSound()
{
	HRECORD hRecord = g_pAIDB->GetMiscRecordLink( GHOST_LOOP_SOUND );
	const char* pszSoundFile = g_pLTDatabase->GetRecordName( hRecord );
	HRECORD hSR = g_pSoundDB->GetSoundDBRecord( pszSoundFile );

	m_hLoopSound = g_pServerSoundMgr->PlayDBSoundFromObject( m_pAI->m_hObject, hSR,
													  -1.0f, SOUNDPRIORITY_MISC_LOW, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE,
													  SMGR_DEFAULT_VOLUME, 1.0f, -1.0f,
													  DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorFlashlight::MakeInvulnerable
//
//	PURPOSE:	Make the AI invulnerable, or vulnerable.
//
// ----------------------------------------------------------------------- //

void CAISensorFlashlight::MakeInvulnerable( bool bInvulnerable )
{
	// Nothing has changed.

	if( m_bInvulnerable == bInvulnerable )
	{
		return;
	}

	// Hack for using the flashlight with Dark Alma instead of the Ghost.
	// Alma dies on impact.  Alma is not invulnerable.

	m_pAI->GetAIBlackBoard()->SetBBHandleTouch( kTouch_Damage );

	// State of invulnerability has changed.

	m_bInvulnerable = bInvulnerable;
///	m_pAI->MakeInvulnerable( m_bInvulnerable );

	// Create exposure FX.

	CreateExposureFX( !m_bInvulnerable );

	// Immediately re-evaluate goals if AI just became vulnerable.

	if( !m_bInvulnerable )
	{
		m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );

		// Calculate time to become invulnerable again.

		float fVulnerableTime = g_pAIDB->GetMiscFloat( GHOST_VULNERABLE_TIME );
		m_fBecomeInvulnerableTime = g_pLTServer->GetTime() + fVulnerableTime;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorFlashlight::CreateExposureFX
//
//	PURPOSE:	Create FX indicating AI is exposed to flashlight.
//
// ----------------------------------------------------------------------- //

void CAISensorFlashlight::CreateExposureFX( bool bExposed )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_pAI->m_hObject );

	if( bExposed )
	{
		const char* pszFX = g_pAIDB->GetMiscString( STUNNED_FX );
		cMsg.WriteBits(CFX_CREATE_LOOP_FX_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
		cMsg.WriteString( pszFX );
	}
	else 
	{
		cMsg.WriteBits(CFX_KILL_LOOP_FX_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	}

	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorFlashlight::GetSenseDistSqr
//
//	PURPOSE:	Return the square of the distance the AI can sense.
//
// ----------------------------------------------------------------------- //

float CAISensorFlashlight::GetSenseDistSqr( float fStimulusRadius )
{
	float fSenseDistanceSqr = fStimulusRadius;
	return fSenseDistanceSqr *= fSenseDistanceSqr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorFlashlight::CreateWorkingMemoryFact
//
//	PURPOSE:	Return the working memory fact that will hold the 
//              memory of sensing this stimulus.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAISensorFlashlight::CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord )
{
	// Find an existing memory for the desire to be stunned.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Desire );
	factQuery.SetDesireType( kDesire_Stunned );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );

	// Create a new memory for the desire to be stunned.

	if( !pFact )
	{
		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Desire );
		pFact->SetDesireType( kDesire_Stunned, 1.f );
	}

	// Keep track of who last exposed us to a flashlight.

	pFact->SetTargetObject( pStimulusRecord->m_hStimulusSource, 1.f );

	return pFact;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorFlashlight::FindUnstimulatedWorkingMemoryFact
//
//	PURPOSE:	Return a working memory fact that has not been 
//              stimulated this update.
//
// ----------------------------------------------------------------------- //

void CAISensorFlashlight::FindUnstimulatedWorkingMemoryFact(AIWORKING_MEMORY_FACT_LIST* pOutFactList)
{
	if (!pOutFactList)
	{
		return;
	}

	// Bail if no fact exists for the desire to be stunned.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Desire );
	factQuery.SetDesireType( kDesire_Stunned );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return;
	}

	// Fact has not been updated since the last stimulus list iteration.

	double fComparisonTime = m_pAI->GetAISensorMgr()->GetStimulusListNewIterationTime();
	if( pFact->GetUpdateTime() < fComparisonTime )
	{
		pOutFactList->push_back( pFact );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorFlashlight::ComplexVisibilityCheck
//
//	PURPOSE:	Return true if the stimulus can be sensed.
//
// ----------------------------------------------------------------------- //

bool CAISensorFlashlight::DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier )
{
return false;
	// Sanity check.

	if( !pStimulusRecord )
	{
		return false;
	}

	// Only stimulated by the player's flashlight.

	HOBJECT hObject = pStimulusRecord->m_hStimulusSource;
	if( !IsPlayer( hObject) )
	{
		return false;
	}

	// Flashlight is not turned on.

	CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hObject );
	if( !( pChar && pChar->IsFlashlightOn() ) )
	{
		return false;
	}

	// Character is not pointing his flashlight at me.

	LTVector vPosition;
	g_pLTServer->GetObjectPos( hObject, &vPosition );
	if( !IsCharacterLookingAtMe( pChar, vPosition ) )
	{
		return false;
	}

	// Limit the flashlight's range.

	float fSenseDistanceSqr = GetSenseDistSqr( pStimulusRecord->m_fDistance );
	if( fSenseDistanceSqr < vPosition.DistSqr( m_pAI->GetPosition() ) )
	{
		return false;
	}

	// Consider the flashlight position to be 3/4 of the way up the dims.
	// This ensures it doesn't get blocked by low obstacles.

	LTVector vDims;
	g_pPhysicsLT->GetObjectDims( hObject, &vDims );
	vPosition.y += vDims.y * 0.5f;

	// Cast a ray from the Player to the AI to see if anything obstructs the beam.

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = vPosition;
	IQuery.m_To = m_pAI->GetPosition();

	if ( m_pAI->CanSeeThrough() )
	{
		IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
		IQuery.m_FilterFn = CAI::SeeThroughFilterFn;
		IQuery.m_PolyFilterFn = CAI::SeeThroughPolyFilterFn;
	}
	else 
	{
		IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
		IQuery.m_FilterFn = CAI::DefaultFilterFn;
		IQuery.m_PolyFilterFn = NULL;
	}

	g_cIntersectSegmentCalls++;

	// Beam hits AI if the ray intersects nothing, or the AI itself.

	bool bVisible = !g_pLTServer->IntersectSegment(IQuery, &IInfo);
	if( ( !bVisible ) && 
		( IInfo.m_hObject == m_pAI->m_hObject ) )
	{
		bVisible = true;
	}

	return bVisible;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorFlashlight::IsCharacterLookingAtMe
//
//	PURPOSE:	Return true if character is looking at me.
//
// ----------------------------------------------------------------------- //

bool CAISensorFlashlight::IsCharacterLookingAtMe( CCharacter* pChar, const LTVector& vPosition )
{
	// Sanity check.

	if( !pChar )
	{
		return false;
	}

	LTRigidTransform tfView;
	pChar->GetViewTransform( tfView );
	LTVector vForward = tfView.m_rRot.Forward();

	LTVector vDir;
	vDir = m_pAI->GetPosition() - vPosition;
	if( vDir != LTVector::GetIdentity() )
	{
		vDir.Normalize();
	}

	// Character is looking at me.

	const static float fThreshhold = 0.98f;
	if ( vDir.Dot(vForward) > fThreshhold )
	{
		return true;
	}

	// Character is not looking at me.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorFlashlight::GetAIDBStimulus()
//
//	PURPOSE:	Get the AIDB stimulus record for a stimulus.
//
// ----------------------------------------------------------------------- //

AIDB_StimulusRecord* CAISensorFlashlight::GetAIDBStimulus( CAIStimulusRecord* pStimulusRecord )
{
	if( !m_pAIDBFlashlight )
	{
		m_pAIDBFlashlight = g_pAIDB->GetAIStimulusRecord( kStim_LightVisible );
	}

	return m_pAIDBFlashlight;
}
