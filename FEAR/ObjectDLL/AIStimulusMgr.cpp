// ----------------------------------------------------------------------- //
//
// MODULE  : AIStimulusMgr.cpp
//
// PURPOSE : AIStimulusMgr implementation
//
// CREATED : 5/17/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIStimulusMgr.h"
#include "AI.h"
#include "AIDB.h"
#include "PlayerObj.h"
#include "CharacterMgr.h"
#include "AIUtils.h"
#include "resourceextensions.h"
#include "CharacterDB.h"

#include <algorithm>

extern CCharacterMgr* g_pCharacterMgr;

DEFINE_AI_FACTORY_CLASS(CAIStimulusRecord);


// Globals / Statics

CAIStimulusMgr* g_pAIStimulusMgr = NULL;
static STIMULUS_DIST_LIST s_lstStimulusDist;
static HOBJECT s_hStimulusModel[MAX_STIMULUS_RENDER];


#define STIMULUS_MODEL_FILE			"Models\\sphere." RESEXT_MODEL_PACKED
#define STIMULUS_RADIUS_SMALL		256.0f
#define STIMULUS_RADIUS_LARGE		512.0f
#define INTERSECT_SEGMENT_QUOTA		3


//
// StimulusRecordCreateStruct functions.
//

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StimulusRecordCreateStruct::Save/load
//
//	PURPOSE:	Save and load
//
// ----------------------------------------------------------------------- //

void StimulusRecordCreateStruct::Save(ILTMessage_Write *pMsg)
{
	SAVE_DWORD( m_eStimulusType );
	SAVE_HOBJECT( m_hStimulusSource );
	SAVE_VECTOR( m_vStimulusPos );
	SAVE_DWORD(	m_eAlignment );
	SAVE_HOBJECT( m_hStimulusTarget );
	SAVE_FLOAT( m_flAlarmScalar );
	SAVE_FLOAT(	m_flRadiusScalar );
	SAVE_FLOAT( m_flDurationScalar );
	SAVE_DWORD( m_dwDynamicPosFlags );
	SAVE_VECTOR( m_vOffset );
	SAVE_DWORD(	m_eDamageType );
	SAVE_FLOAT(	m_fDamageAmount );
	SAVE_VECTOR( m_vDamageDir );
}

// ----------------------------------------------------------------------- //

void StimulusRecordCreateStruct::Load(ILTMessage_Read *pMsg)
{
	LOAD_DWORD_CAST( m_eStimulusType, EnumAIStimulusType );
	LOAD_HOBJECT( m_hStimulusSource );
	LOAD_VECTOR( m_vStimulusPos );
	LOAD_DWORD_CAST( m_eAlignment, EnumCharacterAlignment );
	LOAD_HOBJECT( m_hStimulusTarget );
	LOAD_FLOAT( m_flAlarmScalar );
	LOAD_FLOAT(	m_flRadiusScalar );
	LOAD_FLOAT( m_flDurationScalar );
	LOAD_DWORD( m_dwDynamicPosFlags );
	LOAD_VECTOR( m_vOffset );
	LOAD_DWORD_CAST( m_eDamageType, DamageType );
	LOAD_FLOAT(	m_fDamageAmount );
	LOAD_VECTOR( m_vDamageDir );
}



//
// CAIStimulusRecord functions.
//

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusRecord::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIStimulusRecord::CAIStimulusRecord()
{
	if( g_pAIStimulusMgr )
	{
		m_hStimulusSource.SetReceiver( *g_pAIStimulusMgr );
		m_hStimulusTarget.SetReceiver( *g_pAIStimulusMgr );
	}
	else
	{
		AIASSERT( 0, NULL, "No g_pAIStimulusMgr." );
	}

	m_eStimulusType		= kStim_InvalidType;
	m_eStimulusID		= kStimID_Unset;
	m_vStimulusPos		= LTVector( 0.0f, 0.0f, 0.0f );
	m_vStimulusDir		= LTVector( 0.0f, 0.0f, 0.0f );
	m_nStimulusAlarmLevel = 0;
	m_fDistance			= 0.f;
	m_eDamageType		= DT_INVALID;
	m_fDamageAmount		= 0.f;
	m_vDamageDir		= LTVector( 0.0f, 0.0f, 0.0f );
	m_fTimeStamp		= 0.f;
	m_fExpirationTime	= 0.f;
	m_eAlignment		= kCharAlignment_Invalid;
	m_lstCurResponders.resize( 0 );
	m_dwDynamicPosFlags	= kDynamicPos_Clear;
	m_vDynamicSourceOffset.Init();
	m_bitsRequiredStance.reset();

	m_pStimulusRecordNext = NULL;
}

CAIStimulusRecord::~CAIStimulusRecord()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusRecord::IsAIResponding
//
//	PURPOSE:	Check if an AI is already responding to this stimulus
//
// ----------------------------------------------------------------------- //

bool CAIStimulusRecord::IsAIResponding(HOBJECT hAI) const
{
	RESPONDER_LIST::const_iterator it;
	for( it = m_lstCurResponders.begin(); it != m_lstCurResponders.end(); ++it )
	{
		if( *it == hAI )
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusRecord::ClearResponder
//
//	PURPOSE:	Remove record of an AI responding to this stimulus,
//				so that the AI may respond again.
//
// ----------------------------------------------------------------------- //

void CAIStimulusRecord::ClearResponder(HOBJECT hAI)
{
	RESPONDER_LIST::iterator it;
	for( it = m_lstCurResponders.begin(); it != m_lstCurResponders.end(); ++it )
	{
		if( *it == hAI )
		{
			m_lstCurResponders.erase( it );
			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusRecord::Save/load
//
//	PURPOSE:	Save and load
//
// ----------------------------------------------------------------------- //

void CAIStimulusRecord::Save(ILTMessage_Write *pMsg)
{
	SAVE_DWORD(m_eStimulusType);
	SAVE_DWORD(m_eStimulusID);
	SAVE_HOBJECT(m_hStimulusSource);
	SAVE_HOBJECT(m_hStimulusTarget);
	SAVE_VECTOR(m_vStimulusPos);
	SAVE_VECTOR(m_vStimulusDir);
	SAVE_DWORD(m_nStimulusAlarmLevel);

	SAVE_DWORD(m_eDamageType);
	SAVE_FLOAT(m_fDamageAmount);
	SAVE_VECTOR(m_vDamageDir);

	// Save the string, because indices may change if alignment records are added.

	std::string strAlignment = g_pCharacterDB->Alignment2String( m_eAlignment );
	SAVE_STDSTRING( strAlignment );

	SAVE_FLOAT(m_fDistance);
	SAVE_TIME(m_fTimeStamp);
	SAVE_TIME(m_fExpirationTime);

	SAVE_DWORD(m_lstCurResponders.size());
	RESPONDER_LIST::iterator rlit;
	for( rlit = m_lstCurResponders.begin(); rlit != m_lstCurResponders.end(); ++rlit )
	{
		SAVE_HOBJECT( *rlit );
	}

	SAVE_DWORD(m_dwDynamicPosFlags);

	SAVE_DWORD( m_bitsRequiredStance.to_ulong() );
}

void CAIStimulusRecord::Load(ILTMessage_Read *pMsg)
{
	LOAD_DWORD_CAST(m_eStimulusType, EnumAIStimulusType);
	LOAD_DWORD_CAST(m_eStimulusID, EnumAIStimulusID);
	LOAD_HOBJECT(m_hStimulusSource);
	LOAD_HOBJECT(m_hStimulusTarget);
	LOAD_VECTOR(m_vStimulusPos);
	LOAD_VECTOR(m_vStimulusDir);
	LOAD_DWORD(m_nStimulusAlarmLevel);

	LOAD_DWORD_CAST(m_eDamageType, DamageType);
	LOAD_FLOAT(m_fDamageAmount);
	LOAD_VECTOR(m_vDamageDir);

	// Load the string, because indices may change if alignment records are added.

	std::string strAlignment;
	LOAD_STDSTRING( strAlignment );
	m_eAlignment = g_pCharacterDB->String2Alignment( strAlignment.c_str() );

	LOAD_FLOAT(m_fDistance);
	LOAD_TIME(m_fTimeStamp);
	LOAD_TIME(m_fExpirationTime);
	
	uint32 cResponders;
	LOAD_DWORD(cResponders);
	m_lstCurResponders.resize(cResponders);
	RESPONDER_LIST::iterator it;
	for( it = m_lstCurResponders.begin(); it != m_lstCurResponders.end(); ++it )
	{
		LOAD_HOBJECT( *it );
	}

	LOAD_DWORD(m_dwDynamicPosFlags);

	uint32 dwRequiredStance;
	LOAD_DWORD( dwRequiredStance );
	m_bitsRequiredStance = STANCE_BITS( dwRequiredStance );

	m_pAIDB_Stimulus = g_pAIDB->GetAIStimulusRecord(m_eStimulusType);
}



//
// CAIStimulusMgr functions.
//

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::CAIStimulusMgr()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CAIStimulusMgr::CAIStimulusMgr()
{
	ASSERT(g_pAIStimulusMgr == NULL);
	g_pAIStimulusMgr	= this;

	m_nCycle			= 0;
	m_nNextStimulusID	= 1;
	m_bRenderStimulus	= false;

	// ResponseIndex differentiates instances of AIs responding to stimulus.
	// AIs get the next available index when they activate a stimulated goal.
	// This index can be used by AI to determine if an Ally is alert due to
	// witnessing the same event, or seeing another ally disturbed by the 
	// same event.

	m_iNextStimulationResponseIndex = 1;

	m_bStimulusCriticalSection = false;

	Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::~CAIStimulusMgr()
//
//	PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

CAIStimulusMgr::~CAIStimulusMgr()
{
	ASSERT(g_pAIStimulusMgr != NULL);

	Term();

	g_pAIStimulusMgr = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::Init()
//
//	PURPOSE:	Init object
//
// ----------------------------------------------------------------------- //

void CAIStimulusMgr::Init()
{
	ASSERT(g_pAIStimulusMgr != NULL);

	m_pStimuliListHead = NULL;

	// Keep the MAX NextStimulusID and NextStimulusResponseIndex.  
	// These need to count up forever, and never overlap due to 
	// level transitions (because AIs and players may carry stimuli
	// back and forth).

	m_nCycle			= 0;
	m_nNextStimulusID	= LTMAX( (uint32)1, m_nNextStimulusID );
	m_bRenderStimulus	= false;

	// ResponseIndex differentiates instances of AIs responding to stimulus.
	// AIs get the next available index when they activate a stimulated goal.
	// This index can be used by AI to determine if an Ally is alert due to
	// witnessing the same event, or seeing another ally disturbed by the 
	// same event.

	m_iNextStimulationResponseIndex = LTMAX( (uint32)1, m_iNextStimulationResponseIndex );

	m_bStimulusCriticalSection = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::Term()
//
//	PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

void CAIStimulusMgr::Term()
{
	ASSERT(g_pAIStimulusMgr != NULL);

	// Remove all entries.

	CAIStimulusRecord* pStimNext;
	while( m_pStimuliListHead )
	{
		pStimNext = m_pStimuliListHead->m_pStimulusRecordNext;
		AI_FACTORY_DELETE( m_pStimuliListHead );
		m_pStimuliListHead = pStimNext;
	}
	m_pStimuliListHead = NULL;

	// Remove all sensing objects.

	m_lstSensing.resize( 0 );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::Save/Load()
//
//	PURPOSE:	Save and Load
//
// ----------------------------------------------------------------------- //

void CAIStimulusMgr::Save(ILTMessage_Write *pMsg)
{	
	CAIStimulusRecord* pStimCur;
	SAVE_DWORD( CountStimulusRecords() );
	for( pStimCur = m_pStimuliListHead; pStimCur; pStimCur = pStimCur->m_pStimulusRecordNext )
	{
		pStimCur->Save(pMsg);
	}

	HOBJECT hTemp = NULL;
	AITRACE( AIShowStimulus, ( hTemp, "StimulusMgr: Saved Next StimID = %d", m_nNextStimulusID ) );

	SAVE_DWORD(m_nCycle);
	SAVE_DWORD(m_nNextStimulusID);
	SAVE_DWORD(m_iNextStimulationResponseIndex);
	SAVE_BOOL(m_bRenderStimulus);
	SAVE_BOOL(m_bStimulusCriticalSection);
}

void CAIStimulusMgr::Load(ILTMessage_Read *pMsg)
{
	uint32 cStimulus;
	LOAD_DWORD(cStimulus);

	HOBJECT hTemp = NULL;

	CAIStimulusRecord* pStimLast = NULL;
	CAIStimulusRecord* pAIStimulusRecord;
	for(uint32 iStimulus=0; iStimulus < cStimulus; ++iStimulus)
	{
		pAIStimulusRecord = AI_FACTORY_NEW(CAIStimulusRecord);
		pAIStimulusRecord->Load(pMsg);

		if( !pStimLast )
		{
			m_pStimuliListHead = pAIStimulusRecord;
		}
		else {
			pStimLast->m_pStimulusRecordNext = pAIStimulusRecord;
		}
		pStimLast = pAIStimulusRecord;

		// Some stimulus records may have handles to objects that have transitioned
		// to a new level.  Delete these records.

		AITRACE( AIShowStimulus, ( hTemp, "StimulusMgr: Loaded StimulusRecord = %d", pAIStimulusRecord->m_eStimulusID ) );

		if( !pAIStimulusRecord->m_hStimulusSource )
		{
			AITRACE( AIShowStimulus, ( hTemp, "StimulusMgr: No Source - Deleting StimulusRecord = %d", pAIStimulusRecord->m_eStimulusID ) );

			AI_FACTORY_DELETE( pAIStimulusRecord );
		}
		else {
			InsertStimulusRecord( pAIStimulusRecord );
		}
	}

	LOAD_DWORD(m_nCycle);


	// We need to keep the max of the current or loaded Next values.
	// These should count up forever.  Loading the existing value may be 
	// a problem if we are returning to a previously visited level, where
	// the next values were incremented.  Keeping the max avoids collisions.

	uint32 nTempNext;
	LOAD_DWORD(nTempNext);
	m_nNextStimulusID = LTMAX( m_nNextStimulusID, nTempNext );

	AITRACE( AIShowStimulus, ( hTemp, "StimulusMgr: Loaded Next StimID = %d (T=%d)", m_nNextStimulusID, nTempNext ) );

	LOAD_DWORD(nTempNext);
	m_iNextStimulationResponseIndex = LTMAX( m_iNextStimulationResponseIndex, nTempNext );


	LOAD_BOOL(m_bRenderStimulus);
	LOAD_BOOL(m_bStimulusCriticalSection);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::Add/RemoveSensingObject()
//
//	PURPOSE:	Adds/Removes an object to that can recieve stimulus.
//
// ----------------------------------------------------------------------- //

void CAIStimulusMgr::AddSensingObject(CAI* pSensing)
{
	if( !pSensing )
	{
		AIASSERT( 0, NULL, "CAIStimulusMgr::AddSensingObject: NULL sensing object." );
		return;
	}
	
	m_lstSensing.push_back( pSensing );
}

void CAIStimulusMgr::RemoveSensingObject(CAI* pSensing)
{
	if( !pSensing )
	{
		AIASSERT( 0, NULL, "CAIStimulusMgr::RemoveSensingObject: NULL sensing object." );
		return;
	}

	AISENSING_LIST::iterator it;
	for( it = m_lstSensing.begin(); it != m_lstSensing.end(); ++it )
	{
		if( pSensing == *it )
		{
			m_lstSensing.erase( it );
			break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::GetNextStimulusID()
//
//	PURPOSE:	Return the next available unique stimulus ID.
//
// ----------------------------------------------------------------------- //

EnumAIStimulusID CAIStimulusMgr::GetNextStimulusID()
{
	// Keep a unique identifier for all stimulus.
	EnumAIStimulusID eNextStimID = (EnumAIStimulusID)( m_nNextStimulusID++ );
	return eNextStimID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::RegisterStimulus()
//
//	PURPOSE:	Register a stimulus in the world.
//
// ----------------------------------------------------------------------- //

EnumAIStimulusID CAIStimulusMgr::RegisterStimulus(const StimulusRecordCreateStruct& rDesc)
{
	// Prevent addition of stimulus while inside a stimulus critical section.  
	// This prevents modification of the object containing the CAIStimulusRecord
	// instances while someone is iterating over them.

	if( m_bStimulusCriticalSection )
	{
		AIASSERT( m_bStimulusCriticalSection == false, NULL, "CAIStimulusMgr::RegisterStimulus: In critical section." );
		return kStimID_Invalid;
	}

	// Stimulus not found in bute file, so it's unsupported.

	AIDB_StimulusRecord* pStimulus = g_pAIDB->GetAIStimulusRecord(rDesc.m_eStimulusType);
	if( !pStimulus )
	{
		AIASSERT( 0, NULL, "CAIStimulusMgr::RegisterStimulus: StimulusType not found." );
		return kStimID_Invalid;
	}

	// Create a new record.

	CAIStimulusRecord* pAIStimulusRecord	= AI_FACTORY_NEW(CAIStimulusRecord);
	if (!pAIStimulusRecord)
	{
		return kStimID_Invalid;
	}

	pAIStimulusRecord->m_eStimulusType		= rDesc.m_eStimulusType;
	pAIStimulusRecord->m_hStimulusSource	= rDesc.m_hStimulusSource;
	pAIStimulusRecord->m_hStimulusTarget	= rDesc.m_hStimulusTarget;
	pAIStimulusRecord->m_vStimulusPos		= rDesc.m_vStimulusPos;
	pAIStimulusRecord->m_eAlignment			= rDesc.m_eAlignment;
	pAIStimulusRecord->m_dwDynamicPosFlags	= rDesc.m_dwDynamicPosFlags;
	pAIStimulusRecord->m_eDamageType		= rDesc.m_eDamageType;
	pAIStimulusRecord->m_fDamageAmount		= rDesc.m_fDamageAmount;
	pAIStimulusRecord->m_vDamageDir			= rDesc.m_vDamageDir;

	// Assert if both TrackSource and TrackTarget are specifed, as they are exclusive.

	AIASSERT( ( CAIStimulusRecord::kDynamicPos_TrackSource | CAIStimulusRecord::kDynamicPos_TrackTarget ) != 
		(pAIStimulusRecord->m_dwDynamicPosFlags & (CAIStimulusRecord::kDynamicPos_TrackSource | CAIStimulusRecord::kDynamicPos_TrackTarget)), NULL, 
		"CAIStimulusMgr::RegisterStimulus: StimulusType not found." );

	// If this there is a dynamic hobject in the mix, be sure to update its 
	// position correctly.  This needs to be done before the offset is applied, 
	// and before the direction is determined.

	if( pAIStimulusRecord->m_dwDynamicPosFlags & CAIStimulusRecord::kDynamicPos_TrackSource )
	{
		g_pLTServer->GetObjectPos( pAIStimulusRecord->m_hStimulusSource, &(pAIStimulusRecord->m_vStimulusPos) );
	}
	else if (pAIStimulusRecord->m_dwDynamicPosFlags & CAIStimulusRecord::kDynamicPos_TrackTarget)
	{
		g_pLTServer->GetObjectPos( pAIStimulusRecord->m_hStimulusTarget, &(pAIStimulusRecord->m_vStimulusPos) );
	}

	// Apply the offset after the position has been determined, and before direction.

	if( rDesc.m_vOffset.MagSqr() > 0.1f )
	{
		pAIStimulusRecord->m_dwDynamicPosFlags |= CAIStimulusRecord::kDynamicPos_HasOffset;

		LTRotation	rRot;
		g_pLTServer->GetObjectRotation( pAIStimulusRecord->m_hStimulusSource, &rRot );
		
		pAIStimulusRecord->m_vDynamicSourceOffset = (~rRot) * rDesc.m_vOffset;
		pAIStimulusRecord->m_vStimulusPos += rRot * pAIStimulusRecord->m_vDynamicSourceOffset;
	}

	// Determine the direction after the final position modifications.

	LTVector vSourcePos;
	LTVector vStimulusDir;
	g_pLTServer->GetObjectPos(pAIStimulusRecord->m_hStimulusSource, &vSourcePos);
	vStimulusDir = vSourcePos - pAIStimulusRecord->m_vStimulusPos;
	if( vStimulusDir != LTVector::GetIdentity() )
	{
		vStimulusDir.Normalize();
	}
	pAIStimulusRecord->m_vStimulusDir		= vStimulusDir;

	// Fill in fields using the stimulus

	uint32 nAlarmLevel = (uint32)(pStimulus->nAlarmLevel * rDesc.m_flAlarmScalar);
	pAIStimulusRecord->m_bitsRequiredStance = pStimulus->bitsRequiredStance;
	pAIStimulusRecord->m_fDistance			= pStimulus->fDistance * rDesc.m_flRadiusScalar;
	pAIStimulusRecord->m_fTimeStamp			= g_pLTServer->GetTime();
	pAIStimulusRecord->m_nStimulusAlarmLevel= nAlarmLevel;
	pAIStimulusRecord->m_pAIDB_Stimulus		= pStimulus;

	// Stimluli with duration of 0 do not ever expire.
	if(pStimulus->fDuration * rDesc.m_flDurationScalar != 0.f)
	{
		pAIStimulusRecord->m_fExpirationTime= pAIStimulusRecord->m_fTimeStamp + (pStimulus->fDuration * rDesc.m_flDurationScalar);
	}

	// Keep a unique identifier for all stimulus.
	pAIStimulusRecord->m_eStimulusID = GetNextStimulusID();

	// Stimuli records are sorted by AIAlarmLevel.
	InsertStimulusRecord( pAIStimulusRecord );

	// Print some debug info about this simulus (useful in detecting missing 
	// stimulii such as footstep sounds.
	AITRACE( AIShowStimulus, ( rDesc.m_hStimulusSource, 
		"Stimulus: Type: %s, Radius: %f, Duration: %f", 
			StimulusToString( pAIStimulusRecord->m_eStimulusType ) ? StimulusToString( pAIStimulusRecord->m_eStimulusType ) : "<invalid>"
			, pAIStimulusRecord->m_fDistance
			,  pAIStimulusRecord->m_fExpirationTime - g_pLTServer->GetTime() ) );

	return pAIStimulusRecord->m_eStimulusID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::InsertStimulusRecord
//
//	PURPOSE:	Insert a stimulus record into the list.
//
// ----------------------------------------------------------------------- //

void CAIStimulusMgr::InsertStimulusRecord( CAIStimulusRecord* pStimulusRecord )
{
	// Sanity check.

	if( !pStimulusRecord )
	{
		return;
	}

	//
	// List is sorted by Alarm Level, highest to lowest.
	// Perform an insertion sort.
	// If the insertion sort becomes a performance problem in
	// the future, consider adding a binary search for the insertion 
	// point, or just sorting the entire list once per frame, or 
	// maybe not sorting at all.
	//

	// List is empty.

	if( !m_pStimuliListHead )
	{
		m_pStimuliListHead = pStimulusRecord;
		return;
	}

	// Insert new record at the head.

	if( pStimulusRecord->m_nStimulusAlarmLevel >= m_pStimuliListHead->m_nStimulusAlarmLevel )
	{
		pStimulusRecord->m_pStimulusRecordNext = m_pStimuliListHead;
		m_pStimuliListHead = pStimulusRecord;
		return;
	}

	// Insert new record into the list.

	CAIStimulusRecord* pStimCur;
	for( pStimCur = m_pStimuliListHead; pStimCur->m_pStimulusRecordNext; pStimCur = pStimCur->m_pStimulusRecordNext )
	{
		if( pStimulusRecord->m_nStimulusAlarmLevel >= pStimCur->m_pStimulusRecordNext->m_nStimulusAlarmLevel )
		{
			pStimulusRecord->m_pStimulusRecordNext = pStimCur->m_pStimulusRecordNext;
			pStimCur->m_pStimulusRecordNext = pStimulusRecord;
			return;
		}
	}

	// Append the new record to the end of the list.

	pStimCur->m_pStimulusRecordNext = pStimulusRecord;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::RemoveStimulus
//
//	PURPOSE:	Remove a stimulus with the specified stimulus ID.
//
// ----------------------------------------------------------------------- //

void CAIStimulusMgr::RemoveStimulus(EnumAIStimulusID eStimulusID)
{
	ASSERT(eStimulusID != kStimID_Unset);

	if( m_bStimulusCriticalSection )
	{
		ASSERT(m_bStimulusCriticalSection == false);
		return;
	}

	// Find the record in the list and delete it.

	CAIStimulusRecord* pStimLast = NULL;
	CAIStimulusRecord* pStimCur;
	for( pStimCur = m_pStimuliListHead; pStimCur; pStimCur = pStimCur->m_pStimulusRecordNext )
	{
		if( pStimCur->m_eStimulusID == eStimulusID )
		{
			// Remove from the head of the list.

			if( !pStimLast )
			{
				m_pStimuliListHead = pStimCur->m_pStimulusRecordNext;
			}

			// Remove from within the list.

			else
			{
				pStimLast->m_pStimulusRecordNext = pStimCur->m_pStimulusRecordNext;
			}
			AI_FACTORY_DELETE( pStimCur );
			return;
		}
		pStimLast = pStimCur;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::StimulusExists
//
//	PURPOSE:	Check if stimulud with specified ID is in the system.
//
// ----------------------------------------------------------------------- //

bool CAIStimulusMgr::StimulusExists(EnumAIStimulusID eStimulusID)
{
	if( GetStimulusRecord( eStimulusID ) )
	{
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::CountStimulusRecords
//
//	PURPOSE:	Return total count of stimulus records.
//
// ----------------------------------------------------------------------- //

uint32 CAIStimulusMgr::CountStimulusRecords() const
{
	uint32 cRecords = 0;
	CAIStimulusRecord* pStimCur;
	for( pStimCur = m_pStimuliListHead; pStimCur; pStimCur = pStimCur->m_pStimulusRecordNext )
	{
		++cRecords;
	}
	
	return cRecords;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::GetStimulusRecord
//
//	PURPOSE:	Return a stimulus record with the specified stimulus ID.
//
// ----------------------------------------------------------------------- //

CAIStimulusRecord* CAIStimulusMgr::GetStimulusRecord( EnumAIStimulusID eStimulusID )
{
	CAIStimulusRecord* pStimCur;
	for( pStimCur = m_pStimuliListHead; pStimCur; pStimCur = pStimCur->m_pStimulusRecordNext )
	{
		if( pStimCur->m_eStimulusID == eStimulusID )
		{
			return pStimCur;
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::GetNumResponders
//
//	PURPOSE:	Get number of responders to a stimulus with the specified stimulus ID.
//
// ----------------------------------------------------------------------- //

uint32 CAIStimulusMgr::GetNumResponders(EnumAIStimulusID eStimulusID)
{
	AIASSERT(eStimulusID != kStimID_Unset, NULL, "CAIStimulusMgr::GetNumResponders: Stimulus ID is 0");

	CAIStimulusRecord* pRecord = GetStimulusRecord( eStimulusID );
	if( pRecord )
	{
		return pRecord->m_lstCurResponders.size();
	}

	AIASSERT(0, NULL, "CAIStimulusMgr::GetNumResponders: Could not find stimulus.");
	return (uint32)-1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::ClearResponder
//
//	PURPOSE:	Clear a responder to a stimulus with the specified stimulus ID.
//				The responder may then respond again to the same stimulus.
//
// ----------------------------------------------------------------------- //

void CAIStimulusMgr::ClearResponder(EnumAIStimulusID eStimulusID, HOBJECT hResponder)
{
	AIASSERT(eStimulusID != kStimID_Unset, NULL, "CAIStimulusMgr::ClearResponder: Stimulus ID is 0");

	CAIStimulusRecord* pRecord = GetStimulusRecord( eStimulusID );
	if( pRecord )
	{
		pRecord->ClearResponder( hResponder );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::IgnoreStimulusByTarget
//
//	PURPOSE:	Mark stimuli as handled by some AI based on the stimulus target.
//
// ----------------------------------------------------------------------- //

void CAIStimulusMgr::IgnoreStimulusByTarget(HOBJECT hAI, EnumAIStimulusType eStimulusType, HOBJECT hStimulusTarget)
{
	CAIStimulusRecord* pStimCur;
	for( pStimCur = m_pStimuliListHead; pStimCur; pStimCur = pStimCur->m_pStimulusRecordNext )
	{
		if( ( pStimCur->m_eStimulusType == eStimulusType ) && ( pStimCur->m_hStimulusTarget == hStimulusTarget ) )
		{
			if( !pStimCur->IsAIResponding( hAI ) )
			{
				pStimCur->m_lstCurResponders.push_back( hAI );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::OnLinkBroken
//
//	PURPOSE:	Handle broken links by deleting related stimulus records.
//
// ----------------------------------------------------------------------- //

void CAIStimulusMgr::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if( m_bStimulusCriticalSection )
	{
		ASSERT(m_bStimulusCriticalSection == false);
		return;
	}

	// Look for stimulus records with matching handles.
	// It is OK for the StimulusTarget to go away.
	// (e.g. when something was destroyed by the enemy)

	bool bDeleteStim;
	CAIStimulusRecord* pStimLast = NULL;
	CAIStimulusRecord* pStimCur = m_pStimuliListHead;
	while( pStimCur )
	{
		// Source is going away.

		bDeleteStim = false;
		if( &pStimCur->m_hStimulusSource == pRef )
		{
			bDeleteStim = true;
		}

		// Target is going away, and we are tracking the target.

		else if( ( &pStimCur->m_hStimulusTarget == pRef ) &&
				 ( pStimCur->m_dwDynamicPosFlags & CAIStimulusRecord::kDynamicPos_TrackTarget ) )
		{
			bDeleteStim = true;
		}

		if( bDeleteStim )
		{
			CAIStimulusRecord* pStimNext = pStimCur->m_pStimulusRecordNext;
			if( !pStimLast )
			{
				m_pStimuliListHead = pStimNext;
			}
			else {
				pStimLast->m_pStimulusRecordNext = pStimNext;
			}

			AI_FACTORY_DELETE( pStimCur );
			pStimCur = pStimNext;
		}
		else 
		{
			pStimLast = pStimCur;
			pStimCur = pStimCur->m_pStimulusRecordNext;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::Update()
//
//	PURPOSE:	Send appropriate stimuli to AIs.
//
// ----------------------------------------------------------------------- //

void CAIStimulusMgr::Update()
{
	// Get the current time.
	double fCurTime = g_pLTServer->GetTime();

	//
	// Delete expired stimulus records.
	// Record with expiration time 0 never expire.
	//
	// For records that are not expired, update position
	// if the dynamic flag is set.
	//
	CAIStimulusRecord* pStimLast = NULL;
	CAIStimulusRecord* pStimCur = m_pStimuliListHead;
	while( pStimCur )
	{
		if( (pStimCur->m_fExpirationTime != 0.f) && (pStimCur->m_fExpirationTime < fCurTime) )
		{
			CAIStimulusRecord* pStimNext = pStimCur->m_pStimulusRecordNext;
			if( !pStimLast )
			{
				m_pStimuliListHead = pStimNext;
			}
			else {
				pStimLast->m_pStimulusRecordNext = pStimNext;
			}

			AI_FACTORY_DELETE( pStimCur );
			pStimCur = pStimNext;
		}
		else 
		{
			// Update position if one of the dynamic flags is set.
			// Reset time-stamp.
			// Clear list of responders.
			uint32 dwMask = CAIStimulusRecord::kDynamicPos_TrackTarget | CAIStimulusRecord::kDynamicPos_TrackSource;
			AIASSERT( !!((pStimCur->m_dwDynamicPosFlags & dwMask) ^ dwMask), NULL, "TrackTarget and TrackPosition set" );

			// Get a handle to the dynamic part of the stimulus if there is
			// one.  It could be the target or the source (source is the
			// creator)
			HOBJECT hUpdatingObject = NULL;
			if ( pStimCur->m_dwDynamicPosFlags & CAIStimulusRecord::kDynamicPos_TrackTarget )
			{
				hUpdatingObject = pStimCur->m_hStimulusTarget;
			}
			else if ( pStimCur->m_dwDynamicPosFlags & CAIStimulusRecord::kDynamicPos_TrackSource )
			{
				hUpdatingObject = pStimCur->m_hStimulusSource;
			}

			// If we do have an updating object, then update it.
			if ( hUpdatingObject != NULL)
			{
				// TODO: Replace this with a more robust check supported by both the AI and character.

				g_pLTServer->GetObjectPos(hUpdatingObject, &(pStimCur->m_vStimulusPos) );
				if (IsAI(hUpdatingObject))
				{
					// Use the y-position of the object, but the x and z of the eye.
					// This allows us to shift the vision checks horizontally
					// if the AI is leaning around cover.

					CAI* pAI = (CAI*)g_pLTServer->HandleToObject(hUpdatingObject);
					pStimCur->m_vStimulusPos.x = pAI->GetEyePosition().x;
					pStimCur->m_vStimulusPos.z = pAI->GetEyePosition().z;
				}
				
				// If the updating object has a offset, then find it.
				if( pStimCur->m_dwDynamicPosFlags & CAIStimulusRecord::kDynamicPos_HasOffset )
				{
					LTRotation	rRot;
					g_pLTServer->GetObjectRotation( hUpdatingObject, &rRot );
					
					pStimCur->m_vStimulusPos += rRot * pStimCur->m_vDynamicSourceOffset;
				}

				pStimCur->m_fTimeStamp = fCurTime;
				pStimCur->m_lstCurResponders.resize( 0 );
			}

			pStimLast = pStimCur;
			pStimCur = pStimCur->m_pStimulusRecordNext;
		}		
	}

	// Render stimulus closest to the player for debugging.
	if(m_bRenderStimulus)
	{
		UpdateRenderStimulus();
	}

	//
	// Stimulate AIs.
	//

	// The cycle counter is used to mark each sense with the current cycle.  After each 
	// update, AIs can check their sense recorders' sense records.  If the cycle counter 
	// does not match the SenseMgr's cycle counter, that sense was not updated.
	++m_nCycle;

	// Make sure stimulus are not getting removed while in this loop.
	m_bStimulusCriticalSection = true;

	// Loop thru sensing objects lists.
	UpdateSensingList();

	m_bStimulusCriticalSection = false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIStimulusMgr::UpdateSensingList()
//              
//	PURPOSE:	Updates sensing list.  Does NOT set the
//				StimulusCriticalSecond.  If this should be set, it should be
//				set around the calling of this function.
//              
//----------------------------------------------------------------------------
void CAIStimulusMgr::UpdateSensingList()
{	
	CAI* pSensing;

	bool bNewSenseUpdate;
	int cPermittedIntersectSegmentCalls;
	int cIntersectSegmentCallsPrev;

	double fCurTime = g_pLTServer->GetTime();

	// The list of sensing AI is treated like a time-share system.
	// Each AI gets to process the stimuli until a relevant one is found,
	// or an expensive check has been performed.
	// If the AI does not find a stimulus within one update, it will
	// continue to process the list in subsequent updates.

	AISENSING_LIST::iterator it;
	for( it = m_lstSensing.begin(); it != m_lstSensing.end(); ++it )
	{
		pSensing = *it;
		
		// Ignore AIs who are not sensing.

		if( ( !pSensing ) || 
			( !pSensing->IsSensing() ) ||
			( pSensing->GetSenseUpdateRate() <= 0.f ) )
		{
			continue;
		}


		// Each AI is allowed one intersect segment call per update.

		cPermittedIntersectSegmentCalls = 1;
		bNewSenseUpdate = false;

		// Check if it's time for this AI to start a new sense update.

		if( fCurTime > pSensing->GetNextSenseUpdate() )
		{
			double fNextSenseUpdate = pSensing->GetNextSenseUpdate() + pSensing->GetSenseUpdateRate();
			while( fNextSenseUpdate < fCurTime )
			{
				fNextSenseUpdate += pSensing->GetSenseUpdateRate();
			}
			pSensing->SetNextSenseUpdate( fNextSenseUpdate );

			// If no stimulus was found last sense update, and the AI did not
			// finish processing the list, call HandleSenses to decrement sense values,
			// and allow more intersect segments if quota was not reached.

			if( !pSensing->GetDoneProcessingStimuli() )
			{
				// Handle senses in the AI's sense recorder.  This will check the cycle stamp to
				// clear/decrement values for un-updated senses.  (false stimulations).
				pSensing->HandleSenses(m_nCycle);

				// Allow additional IntersectSegment calls this update if AI
				// did not reach minimal quota of calls last sense update.

				if( pSensing->GetIntersectSegmentCount() < INTERSECT_SEGMENT_QUOTA )
				{
					cPermittedIntersectSegmentCalls += INTERSECT_SEGMENT_QUOTA - pSensing->GetIntersectSegmentCount();
				}
			}
			else
			{
				// Finished processing the entire list, clear this information.

				pSensing->ClearProcessedStimuli();
				pSensing->SetDoneProcessingStimuli( false );
			}

			// Clear records from last sense update.

			pSensing->ClearIntersectSegmentCount();
			pSensing->UpdateSensingMembers();

			bNewSenseUpdate = true;
		}

		// Check if we have already finished processing stimuli for
		// this sense update (because one has already been found, 
		// or list has been exhausted).

		else if( pSensing->GetDoneProcessingStimuli() ) 
		{
			continue;
		}

		cIntersectSegmentCallsPrev = g_cIntersectSegmentCalls;


		// Try to sense the nearest player, so that AI in multiplayer
		// games behave appropriately.

		if( bNewSenseUpdate && SenseNearestPlayer( pSensing ) )
		{
			// [jeffo 05/02/04] This prevents AI from noticing grenades while in combat.
////			pSensing->SetDoneProcessingStimuli( true );
		}

		// Iterate over existing stimulus records.
		
		CAIStimulusRecord* pStimCur;
		for( pStimCur = m_pStimuliListHead; pStimCur; pStimCur = pStimCur->m_pStimulusRecordNext )
		{
			if( !pSensing->ProcessStimulus( pStimCur ) )
			{
				continue;
			}

			if ( CanSense( pSensing, pStimCur ) )
			{
				// UpdateSenseRecord returns true if the AI recorded the sense.
				// False is returned when either the AI has already recorded this sense,
				// or if the record failed to pass further tests specific to the stimulus.
				if( pSensing->HandleSenseRecord( pStimCur, m_nCycle ) )
				{
					// Count number of AIs responding, and keep track of who they are.

					pStimCur->m_lstCurResponders.push_back( pSensing->GetSensingObject() );
				}

				// Stop processing this update after hitting a failed (expensive)
				// IntersectSegment call, and limit has been reached.

				if( cIntersectSegmentCallsPrev < g_cIntersectSegmentCalls )
				{
					pSensing->IncrementIntersectSegmentCount();
					cPermittedIntersectSegmentCalls -= g_cIntersectSegmentCalls - cIntersectSegmentCallsPrev;
					if( cPermittedIntersectSegmentCalls <= 0 )
					{
						break;
					}
				}
			}
		}
		
		// Call HandleSenses to increment/decrement sense values after a
		// a stimulus has been found, or the list has been exhausted.

		if( ( pStimCur == NULL ) ||
			( pSensing->GetDoneProcessingStimuli() ) )
		{
			// Handle senses in the AI's sense recorder.  This will check the cycle stamp to
			// clear/decrement values for un-updated senses.  (false stimulations).
			pSensing->HandleSenses(m_nCycle);

			pSensing->SetDoneProcessingStimuli( true );
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIStimulusMgr::SenseNearestPlayer()
//              
//	PURPOSE:	Try to sense the nearest player. This is important for 
//              AI behavior in multiplayer, so that AIs don't lock onto
//              whatever player happens to come earlier in the stimulus list.
//				
//----------------------------------------------------------------------------

bool CAIStimulusMgr::SenseNearestPlayer(CAI* pSensing)
{
	CAIStimulusRecord* pRecord = NULL;
	CAIStimulusRecord* pNearestPlayerRecord = NULL;
	float fNearestPlayerDistSqr = FLT_MAX;
	float fPlayerDistSqr;

	CAIStimulusRecord* pStimCur;
	for( pStimCur = m_pStimuliListHead; pStimCur; pStimCur = pStimCur->m_pStimulusRecordNext )
	{
		if( pStimCur->m_eStimulusType != kStim_CharacterVisible )
		{
			break;
		}

		if( !IsPlayer( pStimCur->m_hStimulusSource ) )
		{
			continue;
		}

		if( !CanSense( pSensing, pStimCur ) )
		{
			continue;
		}

		fPlayerDistSqr = pSensing->GetSensingPosition().DistSqr( pStimCur->m_vStimulusPos );
		if( fPlayerDistSqr < fNearestPlayerDistSqr )
		{
			fNearestPlayerDistSqr = fPlayerDistSqr;
			pNearestPlayerRecord = pStimCur;
		}
	}

	if( pNearestPlayerRecord )
	{
		pSensing->ProcessStimulus( pNearestPlayerRecord );

		if( pSensing->HandleSenseRecord( pNearestPlayerRecord, m_nCycle ) )
		{
			// Count number of AIs responding, and keep track of who they are.

			pNearestPlayerRecord->m_lstCurResponders.push_back( pSensing->GetSensingObject() );

			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIStimulusMgr::CanSense()
//              
//	PURPOSE:	Checks to see if the AI is capable of sensing this stimulus
//				Assumes a valid Stimulus Record and a valid sensing AI.
//				returns true if the AI is capable, false if it is not.
//				
//----------------------------------------------------------------------------
bool CAIStimulusMgr::CanSense( CAI* pSensing, CAIStimulusRecord* pRecord ) const
{
	AIASSERT( pSensing != NULL, NULL, "CAIStimulusMgr::CanSense NULL sensing object" );
	AIASSERT( pRecord != NULL, NULL, "CAIStimulusMgr::CanSense NULL Record" );

	HOBJECT hSensing = pSensing->GetSensingObject();

	EnumAISenseType eSenseType = pRecord->m_pAIDB_Stimulus->eSenseType;

	// Check for an AI Volume's stimulus mask, 
	// muting certain stimuli.
	
	uint32 flagsStimulusMask = 0xffffffff;

	// Check if this AI responds to this stimulus.
	if( ( pRecord->m_pAIDB_Stimulus->eStimulusType & flagsStimulusMask ) && 
		( eSenseType & pSensing->GetCurSenseFlags() ) )
	{
		// Check that stimulus is not the AI itself IF we the stimulus cares
		// about it
		if (pRecord->m_pAIDB_Stimulus->bRequireSourceIsNotSelf && 
			(HOBJECT)pRecord->m_hStimulusSource == hSensing )
		{
			return false;
		}

		if (pRecord->m_pAIDB_Stimulus->bRequireSourceIsSelf && 
			(HOBJECT)pRecord->m_hStimulusSource != hSensing )
		{
			return false;
		}
	
		// Check if the AI is already responding to this stimulus.
		if( !pRecord->IsAIResponding( hSensing ) )
		{
			// See if the AI feels such a way about the stimulus record
			// that they care about it (that they receive it).
			if( IsStanceInList( pSensing->GetAlignment(),
					pRecord->m_eAlignment,
					pRecord->m_bitsRequiredStance ) )
			{
				LTVector vSensingPos = pSensing->GetSensingPosition();

				// Check vertical cut-off.

				if( pRecord->m_pAIDB_Stimulus->fVerticalRadius > 0.f )
				{
					// AI is above the cut-off.

					if( vSensingPos.y > pRecord->m_vStimulusPos.y + pRecord->m_pAIDB_Stimulus->fVerticalRadius )
					{
						return false;
					}

					// AI is below the cut-off.

					if( vSensingPos.y < pRecord->m_vStimulusPos.y - pRecord->m_pAIDB_Stimulus->fVerticalRadius )
					{
						return false;
					}
				}

//				// Check radius.
//				float fDistance = pRecord->m_vStimulusPos.Dist( vSensingPos );
//				if(fDistance < (pSensing->GetSenseDistance(eSenseType) + pRecord->m_fDistance) )
//				{
				return true;
//				}
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIStimulusMgr::IsStanceInList()
//              
//	PURPOSE:	Returns true if the stance of AlignmentA to AlignmentB is
//              foudn in the list fo required stances.
//
//				Returns true if the Stance is in the list, false if it
//				is not.				
//              
//----------------------------------------------------------------------------
bool CAIStimulusMgr::IsStanceInList( EnumCharacterAlignment eAlignmentA,
									 EnumCharacterAlignment eAlignmentB,
									 const STANCE_BITS& bitsStanceRequirements ) const
{
	EnumCharacterStance eStance = g_pCharacterDB->GetStance( eAlignmentA, eAlignmentB );

	return bitsStanceRequirements.test( eStance );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::RenderStimulus()
//
//	PURPOSE:	Toggle debug rendering of stimulus.
//
// ----------------------------------------------------------------------- //

void CAIStimulusMgr::RenderStimulus(bool bRender)
{
	if( (!m_bRenderStimulus) && bRender )
	{
		m_bRenderStimulus = bRender;

		ObjectCreateStruct theStruct;

		theStruct.SetFileName(STIMULUS_MODEL_FILE);
		theStruct.m_ObjectType = OT_MODEL;


		// Create MAX_STIMULUS_RENDER objects, and hide them.

		for(uint8 iModel=0; iModel < MAX_STIMULUS_RENDER; ++iModel)
		{
			HCLASS hClass = g_pLTServer->GetClass("BaseClass");
			LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
			AIASSERT(pModel, NULL, "CAIStimulusMgr::RenderStimulus: Could not load sphere.");
			if (!pModel) return;

			// Don't eat ticks please...
			::SetNextUpdate(pModel->m_hObject, UPDATE_NEVER);

			s_hStimulusModel[iModel] = pModel->m_hObject;
	
			g_pLTServer->SetObjectColor(pModel->m_hObject, 0.f, 1.f, 0.f, 0.9f);
		}
	}
	else if( m_bRenderStimulus && (!bRender) ) 
	{
		m_bRenderStimulus = bRender;

		for(uint8 iModel=0; iModel < MAX_STIMULUS_RENDER; ++iModel)
		{
			g_pLTServer->RemoveObject(s_hStimulusModel[iModel]);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::OnAIDebugCmd()
//
//	PURPOSE:	Handle various debug commands for examining the state of 
//				this system.
//
// ----------------------------------------------------------------------- //

static int StimulusFlagToIndex( EnumAIStimulusType eStimulusType )
{
	if ( eStimulusType == kStim_InvalidType 
		|| eStimulusType == 0 )
	{
		AIASSERT( 0, NULL, "StimulusFlagToIndex : Invalid stimulustype." );
		return 0;
	}

	uint32 iIndex = 0;
	while ( 0 == ( (1 << iIndex) & eStimulusType ) )
	{
		++iIndex;
	}

	return iIndex;
}

void CAIStimulusMgr::OnAIDebugCmd( HOBJECT hSender, const CParsedMsg& cParsedMsg )
{
	static CParsedMsg::CToken s_cTok_Stats("Stats");
	static CParsedMsg::CToken s_cTok_List("List");

	if ( s_cTok_Stats == cParsedMsg.GetArg(1) )
	{
		// Total the stimulus count.

		int aStimulusCount[ kStim_Count ];
		memset( &aStimulusCount[0], 0, kStim_Count*sizeof(int));

		CAIStimulusRecord* pStimCur;
		for( pStimCur = m_pStimuliListHead; pStimCur; pStimCur = pStimCur->m_pStimulusRecordNext )
		{
			// Convert the flag to an index.

			int iIndex = StimulusFlagToIndex( pStimCur->m_eStimulusType );
			if ( iIndex < 0 || iIndex > kStim_Count )
			{
				continue;
			}

			++aStimulusCount[iIndex];
		}

		// Print out the stats.

		int iStimulusMapSize = CountStimulusRecords();
		int iSensingListSize = m_lstSensing.size();
		g_pLTServer->CPrint( "AIStimulusMgr stats - Total Stimuli: %d, Total sensing objects: %d", iStimulusMapSize, iSensingListSize );
		for ( int iEachType = 0; iEachType < kStim_Count; ++iEachType )
		{
			const char* const pszStimulusType = StimulusToString((EnumAIStimulusType)(1<<iEachType));
			int iStimulusCount = aStimulusCount[iEachType];
			g_pLTServer->CPrint( "\tStimulusType: %s, Count %d", pszStimulusType ? pszStimulusType : "null", iStimulusCount );
		}
	}
	else if ( s_cTok_List == cParsedMsg.GetArg(1) )
	{
		// Print all of the stimuli out.
		
		g_pLTServer->CPrint( "Printing all stimuli..." );
		CAIStimulusRecord* pStimCur;
		for( pStimCur = m_pStimuliListHead; pStimCur; pStimCur = pStimCur->m_pStimulusRecordNext )
		{
			char szName[MAX_PATH] = { '\0' };
			g_pLTServer->GetObjectName( pStimCur->m_hStimulusSource, szName, LTARRAYSIZE(szName) );

			const char* const pszStimulusType = StimulusToString(pStimCur->m_eStimulusType);

			g_pLTServer->CPrint( "\tAlarmLevel %d, StimulusType: %s, StimulusSource: %s", 
				pStimCur->m_nStimulusAlarmLevel,
				pszStimulusType ? pszStimulusType : "null", 
				szName );
		}
	}
	else
	{
		g_pLTServer->CPrint( "AIStimusMgr: No command named: %s", cParsedMsg.GetArg(1).c_str() );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::UpdateRenderStimulus()
//
//	PURPOSE:	Debug rendering of stimulus.
//
// ----------------------------------------------------------------------- //

void CAIStimulusMgr::UpdateRenderStimulus()
{
	LTVector vPlayerPos;
	CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
	g_pLTServer->GetObjectPos(pPlayer->m_hObject, &vPlayerPos);

	// Make dist list at least big enough to hold all stimulus.
	if(s_lstStimulusDist.size() < CountStimulusRecords())
	{
		s_lstStimulusDist.resize(CountStimulusRecords());
	}

	// Calculate and record the distance from the player to each stimulus.

	float fDistSqr;
	CAIStimulusRecord* pRecord;
	STIMULUS_DIST_LIST::iterator lit = s_lstStimulusDist.begin();
	CAIStimulusRecord* pStimCur;
	for( pStimCur = m_pStimuliListHead; pStimCur; pStimCur = pStimCur->m_pStimulusRecordNext )
	{
		fDistSqr = vPlayerPos.DistSqr(pStimCur->m_vStimulusPos);

		lit->pRecord  = pStimCur;
		lit->fDistSqr = fDistSqr;

		++lit;
	}

	// Clear any extras in the dist list.
	while(lit != s_lstStimulusDist.end())
	{
		lit->pRecord  = NULL;
		lit->fDistSqr = 99999999.f;
		++lit;
	}

	// Sort the stimulus by distance.
	s_lstStimulusDist.sort();

	// Turn on rendering for closest MAX_STIMULUS_RENDER stimulus.
	// Turn off the rest.

	uint32 nStimulus = 0;
	for(lit = s_lstStimulusDist.begin(); lit != s_lstStimulusDist.end(); ++lit, ++nStimulus)
	{
		if(nStimulus >= MAX_STIMULUS_RENDER)
		{
			break;
		}
		
		pRecord	= lit->pRecord;

		if(pRecord == NULL)
		{
			g_pCommonLT->SetObjectFlags(s_hStimulusModel[nStimulus], OFT_Flags, 0, FLAGMASK_ALL);
		}
		else 
		{ 
			g_pLTServer->SetObjectPos( s_hStimulusModel[nStimulus], pRecord->m_vStimulusPos );
			g_pCommonLT->SetObjectFlags(s_hStimulusModel[nStimulus], OFT_Flags, FLAG_VISIBLE, FLAGMASK_ALL);

			float fRadius = LTMIN<float>(STIMULUS_RADIUS_SMALL, pRecord->m_fDistance);
			g_pLTServer->SetObjectScale(s_hStimulusModel[nStimulus], fRadius);

			// Assume green
			LTVector vColor(0, 1, 0);

			if (pRecord->m_fDistance > STIMULUS_RADIUS_SMALL &&
				pRecord->m_fDistance <= STIMULUS_RADIUS_LARGE)
			{
				// bigger than we can show, but not huge (yellow)
				vColor = LTVector(1, 1, 0);
			}
			else if (pRecord->m_fDistance > STIMULUS_RADIUS_LARGE)
			{
				// bigger than we can show, and actually pretty big (red)
				vColor = LTVector(1, 0, 0);
			}

			g_pLTServer->SetObjectColor(s_hStimulusModel[nStimulus], vColor.x, vColor.y, vColor.z, 0.9f);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::StimulusToString/StimulusFromString
//
//	PURPOSE:	Convert a stimulus enum to a string and viceversa.
//
// ----------------------------------------------------------------------- //

const char* CAIStimulusMgr::StimulusToString(EnumAIStimulusType eStimulusType)
{
	for( uint32 iStimulus=0; iStimulus < kStim_Count; ++iStimulus )
	{
		if( eStimulusType & (1 << iStimulus) )
		{
			return s_aszStimulusTypes[iStimulus];
		}
	}

	return NULL;
}

EnumAIStimulusType CAIStimulusMgr::StimulusFromString(const char* szStimulusType)
{
	for(uint32 iStimulusType = 0; iStimulusType < kStim_Count; ++iStimulusType)
	{
		if( LTStrICmp(szStimulusType, s_aszStimulusTypes[iStimulusType]) == 0 )
		{
			return (EnumAIStimulusType)(1 << iStimulusType);
		}
	}

	return kStim_InvalidType;
}
