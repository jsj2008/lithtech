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

#include "stdafx.h"
#include "AIStimulusMgr.h"
#include "AISenseRecorderAbstract.h"
#include "AI.h"
#include "PlayerObj.h"
#include "AIVolume.h"
#include "CharacterMgr.h"
#include "AIUtils.h"

#include <algorithm>

extern CCharacterMgr* g_pCharacterMgr;

DEFINE_AI_FACTORY_CLASS(CAIStimulusRecord);


// Globals / Statics

CAIStimulusMgr* g_pAIStimulusMgr = LTNULL;
static STIMULUS_DIST_LIST s_lstStimulusDist;
static HOBJECT s_hStimulusModel[MAX_STIMULUS_RENDER];


#define STIMULUS_MODEL_FILE			"Models\\sphere.ltb"
#define STIMULUS_RADIUS_SMALL		256.0f
#define STIMULUS_RADIUS_LARGE		512.0f
#define INTERSECT_SEGMENT_QUOTA		3


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
	m_eTargetMatchID	= kTMID_Invalid;
	m_vStimulusPos		= LTVector( 0.0f, 0.0f, 0.0f );
	m_vStimulusDir		= LTVector( 0.0f, 0.0f, 0.0f );
	m_pInformationVolume= LTNULL;
	m_nStimulusAlarmLevel = 0;
	m_fDistance			= 0.f;
	m_fTimeStamp		= 0.f;
	m_fExpirationTime	= 0.f;
	m_nMaxResponders	= 0;
	m_lstCurResponders.clear();
	m_dwDynamicPosFlags	= kDynamicPos_Clear;
	m_vDynamicSourceOffset.Init();
	m_RelationData.Clear();
	m_RequiredAlignment.clear();

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

LTBOOL CAIStimulusRecord::IsAIResponding(HOBJECT hAI) const
{
	RESPONDER_LIST::const_iterator it;
	for( it = m_lstCurResponders.begin(); it != m_lstCurResponders.end(); ++it )
	{
		if( *it == hAI )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
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
	SAVE_DWORD(m_eTargetMatchID);
	SAVE_VECTOR(m_vStimulusPos);
	SAVE_VECTOR(m_vStimulusDir);
	SAVE_COBJECT(m_pInformationVolume);
	SAVE_DWORD(m_nStimulusAlarmLevel);
	SAVE_FLOAT(m_fDistance);
	SAVE_TIME(m_fTimeStamp);
	SAVE_TIME(m_fExpirationTime);

	SAVE_BYTE(m_nMaxResponders);
	SAVE_DWORD(m_lstCurResponders.size());
	RESPONDER_LIST::iterator rlit;
	for( rlit = m_lstCurResponders.begin(); rlit != m_lstCurResponders.end(); ++rlit )
	{
		SAVE_HOBJECT( *rlit );
	}

	SAVE_DWORD(m_dwDynamicPosFlags);

	// Save the number of Alignments in the list so we know how many
	// to restore on load
	SAVE_DWORD(m_RequiredAlignment.size());
	_listAlignments::iterator it = m_RequiredAlignment.begin();
	while ( it != m_RequiredAlignment.end() )
	{
		SAVE_DWORD(*it);
		it++;
	}

	m_RelationData.Save(pMsg);
}

void CAIStimulusRecord::Load(ILTMessage_Read *pMsg)
{
	LOAD_DWORD_CAST(m_eStimulusType, EnumAIStimulusType);
	LOAD_DWORD_CAST(m_eStimulusID, EnumAIStimulusID);
	LOAD_HOBJECT(m_hStimulusSource);
	LOAD_HOBJECT(m_hStimulusTarget);
	LOAD_DWORD_CAST(m_eTargetMatchID, EnumAITargetMatchID);
	LOAD_VECTOR(m_vStimulusPos);
	LOAD_VECTOR(m_vStimulusDir);
	LOAD_COBJECT(m_pInformationVolume, AIVolume);
	LOAD_DWORD(m_nStimulusAlarmLevel);
	LOAD_FLOAT(m_fDistance);
	LOAD_TIME(m_fTimeStamp);
	LOAD_TIME(m_fExpirationTime);

	LOAD_BYTE(m_nMaxResponders);
	uint32 cResponders;
	LOAD_DWORD(cResponders);
	m_lstCurResponders.resize(cResponders);
	RESPONDER_LIST::iterator it;
	for( it = m_lstCurResponders.begin(); it != m_lstCurResponders.end(); ++it )
	{
		LOAD_HOBJECT( *it );
	}

	LOAD_DWORD(m_dwDynamicPosFlags);

	int nAlignements;
	CharacterAlignment AlignmentLoadBuffer;
	// Find out how many alignments we stored, and then load that many
	LOAD_DWORD(nAlignements);
	for ( int i = 0; i < nAlignements; i++ )
	{
		LOAD_DWORD_CAST(AlignmentLoadBuffer, CharacterAlignment);
		m_RequiredAlignment.push_back( AlignmentLoadBuffer );
	}


	m_RelationData.Load(pMsg);

	m_pAIBM_Stimulus = g_pAIButeMgr->GetStimulus(m_eStimulusType);
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
	m_bRenderStimulus	= LTFALSE;
	m_nNextTargetMatchID = 0;

	// ResponseIndex differentiates instances of AIs responding to stimulus.
	// AIs get the next available index when they activate a stimulated goal.
	// This index can be used by AI to determine if an Ally is alert due to
	// witnessing the same event, or seeing another ally disturbed by the 
	// same event.

	m_iNextStimulationResponseIndex = 1;

	m_bStimulusCriticalSection = LTFALSE;

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

	g_pAIStimulusMgr = LTNULL;
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

	// Keep the MAX NextStimulusID and NextStimulusResponseIndex.  
	// These need to count up forever, and never overlap due to 
	// level transitions (because AIs and players may carry stimuli
	// back and forth).

	m_nCycle			= 0;
	m_nNextStimulusID	= Max( (uint32)1, m_nNextStimulusID );
	m_bRenderStimulus	= LTFALSE;

	m_nNextTargetMatchID = 0;

	// ResponseIndex differentiates instances of AIs responding to stimulus.
	// AIs get the next available index when they activate a stimulated goal.
	// This index can be used by AI to determine if an Ally is alert due to
	// witnessing the same event, or seeing another ally disturbed by the 
	// same event.

	m_iNextStimulationResponseIndex = Max( (uint32)1, m_iNextStimulationResponseIndex );

	m_bStimulusCriticalSection = LTFALSE;
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

	AISTIMULUS_MAP::iterator it;
	for(it = m_stmStimuliMap.begin(); it != m_stmStimuliMap.end(); ++it )
	{
		AI_FACTORY_DELETE(it->second);
	}

	// Remove all entries.

	m_stmStimuliMap.clear( );

	// Remove all target matches.

	m_mapTargetMatch.clear();

	// Remove all sensing objects.

	m_lstSensing.clear();
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
	AISTIMULUS_MAP::iterator it;
	SAVE_DWORD(m_stmStimuliMap.size());
	for(it = m_stmStimuliMap.begin(); it != m_stmStimuliMap.end(); ++it)
	{
		SAVE_BYTE(it->first);
		it->second->Save(pMsg);
	}

	SAVE_DWORD(m_nNextTargetMatchID);
	SAVE_DWORD(m_mapTargetMatch.size());
	AITARGET_MATCH_MAP::iterator mit;
	for(mit = m_mapTargetMatch.begin(); mit != m_mapTargetMatch.end(); ++mit)
	{
		SAVE_HOBJECT(mit->first);
		SAVE_DWORD(mit->second);
	}

	HOBJECT hTemp = LTNULL;
	AITRACE( AIShowStimulus, ( hTemp, "StimulusMgr: Saved Next StimID = %d", m_nNextStimulusID ) );

	SAVE_DWORD(m_nCycle);
	SAVE_DWORD(m_nNextStimulusID);
	SAVE_DWORD(m_iNextStimulationResponseIndex);
	SAVE_BOOL(m_bRenderStimulus);
	SAVE_BOOL(m_bStimulusCriticalSection);
}

void CAIStimulusMgr::Load(ILTMessage_Read *pMsg)
{
	uint8 nAlarmLevel;
	uint32 cStimulus;
	LOAD_DWORD(cStimulus);

	HOBJECT hTemp = LTNULL;

	CAIStimulusRecord* pAIStimulusRecord;
	for(uint32 iStimulus=0; iStimulus < cStimulus; ++iStimulus)
	{
		LOAD_BYTE(nAlarmLevel);
		pAIStimulusRecord = AI_FACTORY_NEW(CAIStimulusRecord);
		pAIStimulusRecord->Load(pMsg);

		// Some stimulus records may have handles to objects that have transitioned
		// to a new level.  Delete these records.

		AITRACE( AIShowStimulus, ( hTemp, "StimulusMgr: Loaded StimulusRecord = %d", pAIStimulusRecord->m_eStimulusID ) );

		if( !pAIStimulusRecord->m_hStimulusSource )
		{
			AITRACE( AIShowStimulus, ( hTemp, "StimulusMgr: No Source - Deleting StimulusRecord = %d", pAIStimulusRecord->m_eStimulusID ) );

			AI_FACTORY_DELETE( pAIStimulusRecord );
		}
		else {
			m_stmStimuliMap.insert( AISTIMULUS_MAP::value_type(nAlarmLevel, pAIStimulusRecord) );
		}
	}

	HOBJECT hTarget;
	EnumAITargetMatchID eTargetMatchID;
	uint32 cTargetMatches;
	LOAD_DWORD(m_nNextTargetMatchID);
	LOAD_DWORD(cTargetMatches);
	for(uint32 iMatch=0; iMatch < cTargetMatches; ++iMatch)
	{
		LOAD_HOBJECT(hTarget);
		LOAD_DWORD_CAST(eTargetMatchID, EnumAITargetMatchID);

		// Some target records may have handles to objects that have transitioned
		// to a new level.  Do not add these records.

		if( hTarget )
		{
			m_mapTargetMatch.insert( AITARGET_MATCH_MAP::value_type(hTarget, eTargetMatchID) );
		}
	}

	LOAD_DWORD(m_nCycle);


	// We need to keep the max of the current or loaded Next values.
	// These should count up forever.  Loading the existing value may be 
	// a problem if we are returning to a previously visited level, where
	// the next values were incremented.  Keeping the max avoids collisions.

	uint32 nTempNext;
	LOAD_DWORD(nTempNext);
	m_nNextStimulusID = Max( m_nNextStimulusID, nTempNext );

	AITRACE( AIShowStimulus, ( hTemp, "StimulusMgr: Loaded Next StimID = %d (T=%d)", m_nNextStimulusID, nTempNext ) );

	LOAD_DWORD(nTempNext);
	m_iNextStimulationResponseIndex = Max( m_iNextStimulationResponseIndex, nTempNext );


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

void CAIStimulusMgr::AddSensingObject(IAISensing* pSensing)
{
	if( !pSensing )
	{
		AIASSERT( 0, LTNULL, "CAIStimulusMgr::AddSensingObject: NULL sensing object." );
		return;
	}
	
	m_lstSensing.push_back( pSensing );
}

void CAIStimulusMgr::RemoveSensingObject(IAISensing* pSensing)
{
	if( !pSensing )
	{
		AIASSERT( 0, LTNULL, "CAIStimulusMgr::RemoveSensingObject: NULL sensing object." );
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
//	ROUTINE:	CAIStimulusMgr::_RegisterStimulus()
//
//	PURPOSE:	Register a stimulus in the world.
//				Private routine called by specific RegisterStimulus() for
//				given parameters.
//
// ----------------------------------------------------------------------- //

EnumAIStimulusID CAIStimulusMgr::_RegisterStimulus(CAIStimulusRecord* pAIStimulusRecord, uint32 nAlarmLevelFactor, 
									 LTFLOAT fRadiusFactor, LTFLOAT fDurationFactor)
{
	AIASSERT( pAIStimulusRecord, LTNULL, "CAIStimulusMgr::_RegisterStimulus: NULL record." );

	if( m_bStimulusCriticalSection )
	{
		AIASSERT( m_bStimulusCriticalSection == LTFALSE, LTNULL, "CAIStimulusMgr::_RegisterStimulus: In critical section." );
		return kStimID_Invalid;
	}

	AIBM_Stimulus* pStimulus = g_pAIButeMgr->GetStimulus(pAIStimulusRecord->m_eStimulusType);

	// Stimulus not found in bute file, so it's unsupported.
	if( !pStimulus )
	{
		return kStimID_Invalid;
	}

	uint32 nAlarmLevel = pStimulus->nAlarmLevel * nAlarmLevelFactor;

	// Fill in stimulus record from stimulus.
	
	pAIStimulusRecord->m_RequiredAlignment
		= pStimulus->caRequiredAlignment; // Copy the alignment list
	pAIStimulusRecord->m_fDistance			= pStimulus->fDistance * fRadiusFactor;
	pAIStimulusRecord->m_fTimeStamp			= g_pLTServer->GetTime();
	pAIStimulusRecord->m_nMaxResponders		= pStimulus->nMaxResponders;
	pAIStimulusRecord->m_nStimulusAlarmLevel= nAlarmLevel;
	pAIStimulusRecord->m_pAIBM_Stimulus		= pStimulus;

	// Stimluli with duration of 0 do not ever expire.
	if(pStimulus->fDuration != 0.f)
	{
		pAIStimulusRecord->m_fExpirationTime	= pAIStimulusRecord->m_fTimeStamp + (pStimulus->fDuration * fDurationFactor);
	}

	// InformationVolumes are used for sense masks.

	if( IsCharacter( pAIStimulusRecord->m_hStimulusSource ) )
	{
		CCharacter* pCharacter = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject( pAIStimulusRecord->m_hStimulusSource ));
		if( pCharacter && pCharacter->HasCurrentInformationVolume() )
		{
			pAIStimulusRecord->m_pInformationVolume = (AIVolume*)pCharacter->GetCurrentInformationVolume();
		}
	}

	// TargetMatchID is used to identify stimuli that apply to the same object.
	// It is not safe to go by the handle alone, because handles for deleted
	// objects get re-used.

	pAIStimulusRecord->m_eTargetMatchID = GetTargetMatchID( pAIStimulusRecord->m_hStimulusTarget );

	// Keep a unique identifier for all stimulus.
	pAIStimulusRecord->m_eStimulusID = (EnumAIStimulusID)( m_nNextStimulusID++ );

	// Stimuli records are sorted by AIAlarmLevel.
	m_stmStimuliMap.insert( AISTIMULUS_MAP::value_type( nAlarmLevel, pAIStimulusRecord ) );

	return pAIStimulusRecord->m_eStimulusID;
}

//
// Various public overloads of RegisterStimulus().  Call one depending on 
// minimum available parameters.
//

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::RegisterStimulus()
//
//	PURPOSE:	Register a stimulus in the world.
//				Overload for position and radius factor.
//
// ----------------------------------------------------------------------- //

EnumAIStimulusID CAIStimulusMgr::RegisterStimulus(EnumAIStimulusType eStimulusType, HOBJECT hStimulusSource,  
									 const LTVector& vPos, LTFLOAT fRadiusFactor)
{
	ASSERT( eStimulusType > kStim_InvalidType );

	ASSERT( hStimulusSource && IsCharacter(hStimulusSource) );
    CCharacter* pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hStimulusSource));
	if( !pChar )
	{
		return kStimID_Invalid;
	}

	// Create a stimulus record, and fill it in.
	CAIStimulusRecord* pAIStimulusRecord	= AI_FACTORY_NEW(CAIStimulusRecord);
	pAIStimulusRecord->m_eStimulusType		= eStimulusType;
	pAIStimulusRecord->m_hStimulusSource	= hStimulusSource;
	pAIStimulusRecord->m_vStimulusPos		= vPos;
	pAIStimulusRecord->m_RelationData		= pChar->GetRelationData();

	EnumAIStimulusID eAIStimulusID = _RegisterStimulus(pAIStimulusRecord, 1, fRadiusFactor, 1.0f);

	// Check if it registered ok.
	if( eAIStimulusID == kStimID_Invalid )
	{
		AI_FACTORY_DELETE( pAIStimulusRecord );
	}

	return eAIStimulusID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::RegisterStimulus()
//
//	PURPOSE:	Register a stimulus in the world.
//				Overload for target, position, and radius factor.
//
// ----------------------------------------------------------------------- //

EnumAIStimulusID CAIStimulusMgr::RegisterStimulus(EnumAIStimulusType eStimulusType, HOBJECT hStimulusSource,  
									 HOBJECT hStimulusTarget, const LTVector& vPos, 
									 LTFLOAT fRadiusFactor, LTFLOAT fDurationFactor)
{
	AIASSERT( eStimulusType > kStim_InvalidType, LTNULL, "CAIStimulusMgr::RegisterStimulus: stimulus is invalid type." );

	AIASSERT( hStimulusSource && IsCharacter(hStimulusSource), LTNULL, "CAIStimulusMgr::RegisterStimulus: stimulus is not a character." );
    CCharacter* pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hStimulusSource));
	if( !pChar )
	{
		return kStimID_Invalid;
	}

	// Create a stimulus record, and fill it in.
	CAIStimulusRecord* pAIStimulusRecord	= AI_FACTORY_NEW(CAIStimulusRecord);
	pAIStimulusRecord->m_eStimulusType		= eStimulusType;
	pAIStimulusRecord->m_hStimulusSource	= hStimulusSource;
	pAIStimulusRecord->m_hStimulusTarget	= hStimulusTarget;
	pAIStimulusRecord->m_vStimulusPos		= vPos;
	pAIStimulusRecord->m_RelationData		= pChar->GetRelationData();

	// Calculate direction vector pointing from the stimulus pos to its source.
	// (e.g. from where a bullet hit to who fired it).
	LTVector vSourcePos;
	g_pLTServer->GetObjectPos(hStimulusSource, &vSourcePos);
	pAIStimulusRecord->m_vStimulusDir = vSourcePos - vPos;
	pAIStimulusRecord->m_vStimulusDir.Normalize();

	EnumAIStimulusID eAIStimulusID = _RegisterStimulus(pAIStimulusRecord, 1, fRadiusFactor, fDurationFactor);

	// Check if it registered ok.
	if( eAIStimulusID == kStimID_Invalid )
	{
		AI_FACTORY_DELETE( pAIStimulusRecord );
	}

	return eAIStimulusID;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::RegisterStimulus()
//
//	PURPOSE:	Register a stimulus in the world.
//				Overload for alarmLevel, target, position, and radius factor.
//
// ----------------------------------------------------------------------- //

EnumAIStimulusID CAIStimulusMgr::RegisterStimulus(EnumAIStimulusType eStimulusType, uint32 nAlarmLevelFactor, HOBJECT hStimulusSource,  
									 HOBJECT hStimulusTarget, const LTVector& vPos, LTFLOAT fRadiusFactor)
{
	ASSERT( eStimulusType > kStim_InvalidType );

	ASSERT( hStimulusSource && IsCharacter(hStimulusSource) );
    CCharacter* pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hStimulusSource));
	if( !pChar )
	{
		return kStimID_Invalid;
	}

	// Create a stimulus record, and fill it in.
	CAIStimulusRecord* pAIStimulusRecord	= AI_FACTORY_NEW(CAIStimulusRecord);
	pAIStimulusRecord->m_eStimulusType		= eStimulusType;
	pAIStimulusRecord->m_hStimulusSource	= hStimulusSource;
	pAIStimulusRecord->m_hStimulusTarget	= hStimulusTarget;
	pAIStimulusRecord->m_vStimulusPos		= vPos;
	pAIStimulusRecord->m_RelationData		= pChar->GetRelationData();

	// Calculate direction vector pointing from the stimulus pos to its source.
	// (e.g. from where a bullet hit to who fired it).
	LTVector vSourcePos;
	g_pLTServer->GetObjectPos(hStimulusSource, &vSourcePos);
	pAIStimulusRecord->m_vStimulusDir = vSourcePos - vPos;
	pAIStimulusRecord->m_vStimulusDir.Normalize();

	EnumAIStimulusID eAIStimulusID = _RegisterStimulus(pAIStimulusRecord, nAlarmLevelFactor, fRadiusFactor, 1.0f);

	// Check if it registered ok.
	if( eAIStimulusID == kStimID_Invalid )
	{
		AI_FACTORY_DELETE( pAIStimulusRecord );
	}

	return eAIStimulusID;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::RegisterStimulus()
//
//	PURPOSE:	Register a stimulus in the world.
//				Overload for position, radius factor, and duration factor.
//
// ----------------------------------------------------------------------- //

EnumAIStimulusID CAIStimulusMgr::RegisterStimulus(EnumAIStimulusType eStimulusType, HOBJECT hStimulusSource,  
									 const LTVector& vPos, LTFLOAT fRadiusFactor, LTFLOAT fDurationFactor)
{
	ASSERT( eStimulusType > kStim_InvalidType );

	ASSERT( hStimulusSource && IsCharacter(hStimulusSource) );
    CCharacter* pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hStimulusSource));
	if( !pChar )
	{
		return kStimID_Invalid;
	}

	// Create a stimulus record, and fill it in.
	CAIStimulusRecord* pAIStimulusRecord	= AI_FACTORY_NEW(CAIStimulusRecord);
	pAIStimulusRecord->m_eStimulusType		= eStimulusType;
	pAIStimulusRecord->m_hStimulusSource	= hStimulusSource;
	pAIStimulusRecord->m_vStimulusPos		= vPos;
	pAIStimulusRecord->m_RelationData		= pChar->GetRelationData();

	EnumAIStimulusID eAIStimulusID = _RegisterStimulus(pAIStimulusRecord, 1, fRadiusFactor, fDurationFactor);

	// Check if it registered ok.
	if( eAIStimulusID == kStimID_Invalid )
	{
		AI_FACTORY_DELETE( pAIStimulusRecord );
	}

	return eAIStimulusID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::RegisterStimulus()
//
//	PURPOSE:	Register a stimulus in the world.
//				Overload for alarm level factor, alignment, position, 
//              radius factor, and duration factor.
//
// ----------------------------------------------------------------------- //

EnumAIStimulusID CAIStimulusMgr::RegisterStimulus(EnumAIStimulusType eStimulusType, uint32 nAlarmLevelFactor,
													HOBJECT hStimulusSource, HOBJECT hStimulusTarget,
													RelationData RelationData, const LTVector& vPos, 
													LTFLOAT fRadiusFactor, LTFLOAT fDurationFactor)
{
	ASSERT( eStimulusType > kStim_InvalidType );

	// Create a stimulus record, and fill it in.
	CAIStimulusRecord* pAIStimulusRecord	= AI_FACTORY_NEW(CAIStimulusRecord);
	pAIStimulusRecord->m_eStimulusType		= eStimulusType;
	pAIStimulusRecord->m_hStimulusSource	= hStimulusSource;
	pAIStimulusRecord->m_hStimulusTarget	= hStimulusTarget;
	pAIStimulusRecord->m_vStimulusPos		= vPos;
	pAIStimulusRecord->m_RelationData		= RelationData;

	EnumAIStimulusID eAIStimulusID = _RegisterStimulus(pAIStimulusRecord, nAlarmLevelFactor, fRadiusFactor, fDurationFactor);

	// Check if it registered ok.
	if( eAIStimulusID == kStimID_Invalid )
	{
		AI_FACTORY_DELETE( pAIStimulusRecord );
	}

	return eAIStimulusID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::RegisterStimulus()
//
//	PURPOSE:	Register a stimulus in the world.
//				Overload for looking up source position every frame.
//
// ----------------------------------------------------------------------- //

EnumAIStimulusID CAIStimulusMgr::RegisterStimulus(EnumAIStimulusType eStimulusType,
												  HOBJECT hStimulusSource,
												  HOBJECT hStimulusTarget,
												  CAIStimulusRecord::kDynamicPos_Flag DynamicPosFlag,
												  LTFLOAT fRadiusFactor)
{
	return RegisterStimulus( eStimulusType, hStimulusSource, hStimulusTarget, DynamicPosFlag, LTVector( 0.0f, 0.0f, 0.0f ), fRadiusFactor);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::RegisterStimulus()
//
//	PURPOSE:	Register a stimulus in the world.
//				Overload for looking up source position every frame with an offset
//				from the source position.
//
// ----------------------------------------------------------------------- //

EnumAIStimulusID CAIStimulusMgr::RegisterStimulus(EnumAIStimulusType eStimulusType,
												  HOBJECT hStimulusSource,
												  HOBJECT hStimulusTarget,
												  CAIStimulusRecord::kDynamicPos_Flag dwDynamicPosFlags,
												  const LTVector &vOffset,
												  LTFLOAT fRadiusFactor)
{
	ASSERT( eStimulusType > kStim_InvalidType );
	ASSERT( hStimulusSource && IsCharacter( hStimulusSource ));

	CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject( hStimulusSource ));
	if( !pChar )
	{
		return kStimID_Invalid;
	}

	// Create a stimulus record and fill it in...
	CAIStimulusRecord *pAIStimulusRecord	= AI_FACTORY_NEW( CAIStimulusRecord );
	pAIStimulusRecord->m_eStimulusType		= eStimulusType;
	pAIStimulusRecord->m_hStimulusSource	= hStimulusSource;
	pAIStimulusRecord->m_hStimulusTarget	= hStimulusTarget;
	pAIStimulusRecord->m_RelationData		= pChar->GetRelationData();
	pAIStimulusRecord->m_dwDynamicPosFlags	|= dwDynamicPosFlags;

	// If this there is a dynamic hobject in the mix, be sure to update its 
	// position correctly
	if( pAIStimulusRecord->m_dwDynamicPosFlags & CAIStimulusRecord::kDynamicPos_TrackSource )
	{
		g_pLTServer->GetObjectPos( hStimulusSource, &(pAIStimulusRecord->m_vStimulusPos) );
	}
	else if (pAIStimulusRecord->m_dwDynamicPosFlags & CAIStimulusRecord::kDynamicPos_TrackTarget)
	{
		g_pLTServer->GetObjectPos( hStimulusTarget, &(pAIStimulusRecord->m_vStimulusPos) );
	}

	// If we had an offset add it in...

	if( vOffset.MagSqr() > 0.1f )
	{
		pAIStimulusRecord->m_dwDynamicPosFlags |= CAIStimulusRecord::kDynamicPos_HasOffset;

		LTRotation	rRot;
		LTMatrix	mMat;

		g_pLTServer->GetObjectRotation( hStimulusSource, &rRot );
		rRot.ConvertToMatrix( mMat );
		
		pAIStimulusRecord->m_vDynamicSourceOffset = ~mMat * vOffset;
		pAIStimulusRecord->m_vStimulusPos += mMat * pAIStimulusRecord->m_vDynamicSourceOffset;
	}

	EnumAIStimulusID eAIStimulusID = _RegisterStimulus(pAIStimulusRecord, 1, fRadiusFactor, 1.0f);	

	// Check if it registered ok.
	if( eAIStimulusID == kStimID_Invalid )
	{
		AI_FACTORY_DELETE( pAIStimulusRecord );
	}	

	return eAIStimulusID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::GetTargetMatchID
//
//	PURPOSE:	Find the TargetMatchID for an HOBJECT.
//
// ----------------------------------------------------------------------- //

EnumAITargetMatchID CAIStimulusMgr::GetTargetMatchID(HOBJECT hTarget)
{
	AITARGET_MATCH_MAP::iterator it;
	it = m_mapTargetMatch.find( hTarget );

	// Find an existing ID for the hTarget, or create a new one.

	EnumAITargetMatchID eTargetMatchID;
	if( it == m_mapTargetMatch.end() )
	{
		eTargetMatchID = (EnumAITargetMatchID)m_nNextTargetMatchID++;
		m_mapTargetMatch.insert( AITARGET_MATCH_MAP::value_type( hTarget, eTargetMatchID ) );
	}
	else {
		eTargetMatchID = it->second;
	}

	return eTargetMatchID;
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
		ASSERT(m_bStimulusCriticalSection == LTFALSE);
		return;
	}

	AISTIMULUS_MAP::iterator it = m_stmStimuliMap.begin();
	while( it != m_stmStimuliMap.end() )
	{
		if( it->second->m_eStimulusID == eStimulusID )
		{
			AI_FACTORY_DELETE(it->second);
			m_stmStimuliMap.erase(it);
			return;
		}
		else ++it;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::StimulusExists
//
//	PURPOSE:	Check if stimulud with specified ID is in the system.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIStimulusMgr::StimulusExists(EnumAIStimulusID eStimulusID)
{
	ASSERT(eStimulusID != kStimID_Unset);

	AISTIMULUS_MAP::iterator it;
	for( it = m_stmStimuliMap.begin(); it != m_stmStimuliMap.end(); ++it )
	{
		if( it->second->m_eStimulusID == eStimulusID )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
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
	AIASSERT(eStimulusID != kStimID_Unset, LTNULL, "CAIStimulusMgr::GetNumResponders: Stimulus ID is 0");

	CAIStimulusRecord* pRecord;
	AISTIMULUS_MAP::iterator it;
	for( it = m_stmStimuliMap.begin(); it != m_stmStimuliMap.end(); ++it )
	{
		pRecord = it->second;
		if( pRecord->m_eStimulusID == eStimulusID )
		{
			return pRecord->m_lstCurResponders.size();
		}
	}

	AIASSERT(0, LTNULL, "CAIStimulusMgr::GetNumResponders: Could not find stimulus.");
	return -1;
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
	AIASSERT(eStimulusID != kStimID_Unset, LTNULL, "CAIStimulusMgr::ClearResponder: Stimulus ID is 0");

	CAIStimulusRecord* pRecord;
	AISTIMULUS_MAP::iterator it;
	for( it = m_stmStimuliMap.begin(); it != m_stmStimuliMap.end(); ++it )
	{
		pRecord = it->second;
		if( pRecord->m_eStimulusID == eStimulusID )
		{
			pRecord->ClearResponder( hResponder );
			break;
		}
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
	CAIStimulusRecord* pRecord;
	AISTIMULUS_MAP::iterator it;
	for( it = m_stmStimuliMap.begin(); it != m_stmStimuliMap.end(); ++it )
	{
		pRecord = it->second;
		if( ( pRecord->m_eStimulusType == eStimulusType ) && ( pRecord->m_hStimulusTarget == hStimulusTarget ) )
		{
			if( !pRecord->IsAIResponding( hAI ) )
			{
				pRecord->m_lstCurResponders.push_back( hAI );
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
		ASSERT(m_bStimulusCriticalSection == LTFALSE);
		return;
	}

	// Look for stimulus records with matching handles.
	// It is OK for the StimulusTarget to go away.
	// (e.g. when something was destroyed by the enemy)

	AISTIMULUS_MAP::iterator it = m_stmStimuliMap.begin();
	while( it != m_stmStimuliMap.end() )
	{
		if( &it->second->m_hStimulusSource == pRef )
		{
			AI_FACTORY_DELETE(it->second);
			AISTIMULUS_MAP::iterator next = it;
			++next;
			m_stmStimuliMap.erase(it);
			it = next;
		}
		else ++it;
	}


	// Look for TargetMatch records with matching handles.
	// If one is found, remove it.

	AITARGET_MATCH_MAP::iterator mit = m_mapTargetMatch.find( hObj );
	if( mit != m_mapTargetMatch.end() )
	{
		m_mapTargetMatch.erase( mit );
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
	float fCurTime = g_pLTServer->GetTime();

	//
	// Delete expired stimulus records.
	// Record with expiration time 0 never expire.
	//
	// For records that are not expired, update position
	// if the dynamic flag is set.
	//
	AISTIMULUS_MAP::iterator it = m_stmStimuliMap.begin();
	while( it != m_stmStimuliMap.end() )
	{
		CAIStimulusRecord* pRecord = it->second;
		if( (pRecord->m_fExpirationTime != 0.f) && (pRecord->m_fExpirationTime < fCurTime) )
		{
			AI_FACTORY_DELETE(pRecord);
			AISTIMULUS_MAP::iterator next = it;
			++next;
			m_stmStimuliMap.erase(it);
			it = next;
		}
		else 
		{
			// Update position if one of the dynamic flags is set.
			// Reset time-stamp.
			// Clear list of responders.
			uint32 dwMask = CAIStimulusRecord::kDynamicPos_TrackTarget | CAIStimulusRecord::kDynamicPos_TrackSource;
			AIASSERT( !!((pRecord->m_dwDynamicPosFlags & dwMask) ^ dwMask), NULL, "TrackTarget and TrackPosition set" );

			// Get a handle to the dynamic part of the stimulus if there is
			// one.  It could be the target or the source (source is the
			// creator)
			HOBJECT hUpdatingObject = NULL;
			if ( pRecord->m_dwDynamicPosFlags & CAIStimulusRecord::kDynamicPos_TrackTarget )
			{
				hUpdatingObject = pRecord->m_hStimulusTarget;
			}
			else if ( pRecord->m_dwDynamicPosFlags & CAIStimulusRecord::kDynamicPos_TrackSource )
			{
				hUpdatingObject = pRecord->m_hStimulusSource;
			}

			// If we do have an updating object, then update it.
			if ( hUpdatingObject != NULL)
			{
				g_pLTServer->GetObjectPos(hUpdatingObject, &(pRecord->m_vStimulusPos) );
				
				// If the updating object has a offset, then find it.
				if( pRecord->m_dwDynamicPosFlags & CAIStimulusRecord::kDynamicPos_HasOffset )
				{
					LTRotation	rRot;
					LTMatrix	mMat;

					g_pLTServer->GetObjectRotation( hUpdatingObject, &rRot );
					rRot.ConvertToMatrix( mMat );

					pRecord->m_vStimulusPos += mMat * pRecord->m_vDynamicSourceOffset;
				}

				pRecord->m_fTimeStamp = fCurTime;
				pRecord->m_lstCurResponders.clear();

				// Update the information volume handle to correspond to the new pos.

				if( IsCharacter( pRecord->m_hStimulusSource ) )
				{
					CCharacter* pCharacter = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject( pRecord->m_hStimulusSource ));
					if( pCharacter && pCharacter->HasCurrentInformationVolume() )
					{
						pRecord->m_pInformationVolume = (AIVolume*)pCharacter->GetCurrentInformationVolume();
					}
					else
					{
						pRecord->m_pInformationVolume = LTNULL;
					}
				}
			}

			++it;
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
	m_bStimulusCriticalSection = LTTRUE;

	// Loop thru sensing objects lists.
	UpdateSensingList();

	m_bStimulusCriticalSection = LTFALSE;
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
	AISTIMULUS_MAP::iterator itRecordPair;
	
	CAIStimulusRecord* pRecord = LTNULL;
	IAISensing* pSensing;

	LTBOOL bNewSenseUpdate;
	int cPermittedIntersectSegmentCalls;
	int cIntersectSegmentCallsPrev;

	LTFLOAT fCurTime = g_pLTServer->GetTime();

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
		bNewSenseUpdate = LTFALSE;

		// Check if it's time for this AI to start a new sense update.

		if( fCurTime > pSensing->GetNextSenseUpdate() )
		{
			LTFLOAT fNextSenseUpdate = pSensing->GetNextSenseUpdate() + pSensing->GetSenseUpdateRate();
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

			// Clear records from last sense update.

			pSensing->ClearIntersectSegmentCount();
			pSensing->ClearProcessedStimuli();
			pSensing->SetDoneProcessingStimuli( LTFALSE );

			pSensing->UpdateSensingMembers();

			bNewSenseUpdate = LTTRUE;
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
			pSensing->SetDoneProcessingStimuli( LTTRUE );
		}

		// Iterate over existing stimulus records.
		
		else {

			for(itRecordPair = m_stmStimuliMap.begin();
				itRecordPair != m_stmStimuliMap.end();
				++itRecordPair)
			{
				pRecord = itRecordPair->second;

				if( !pSensing->ProcessStimulus( pRecord ) )
				{
					continue;
				}

				if ( CanSense( pSensing, pRecord ) )
				{
					// UpdateSenseRecord returns true if the AI recorded the sense.
					// False is returned when either the AI has already recorded this sense,
					// or if the record failed to pass further tests specific to the stimulus.
					if( pSensing->HandleSenseRecord( pRecord, m_nCycle ) )
					{
						// Count number of AIs responding, and keep track of who they are.

						pRecord->m_lstCurResponders.push_back( pSensing->GetSensingObject() );

						pSensing->SetDoneProcessingStimuli( LTTRUE );

						// Only pay attention to the most alarming 
						// stimulus at any one instant.
						break;
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
		}
		
		// Call HandleSenses to increment/decrement sense values after a
		// a stimulus has been found, or the list has been exhausted.

		if( ( pRecord == NULL) ||
			( pSensing->GetDoneProcessingStimuli() ) )
		{
			// Handle senses in the AI's sense recorder.  This will check the cycle stamp to
			// clear/decrement values for un-updated senses.  (false stimulations).
			pSensing->HandleSenses(m_nCycle);

			pSensing->SetDoneProcessingStimuli( LTTRUE );
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

LTBOOL CAIStimulusMgr::SenseNearestPlayer(IAISensing* pSensing)
{
	CAIStimulusRecord* pRecord = LTNULL;
	CAIStimulusRecord* pNearestPlayerRecord = LTNULL;
	LTFLOAT fNearestPlayerDistSqr = FLT_MAX;
	LTFLOAT fPlayerDistSqr;

	AISTIMULUS_MAP::iterator itRecordPair;
	for(itRecordPair = m_stmStimuliMap.begin();
		itRecordPair != m_stmStimuliMap.end();
		++itRecordPair)
	{
		pRecord = itRecordPair->second;

		if( pRecord->m_eStimulusType != kStim_EnemyVisible )
		{
			break;
		}

		if( !IsPlayer( pRecord->m_hStimulusSource ) )
		{
			continue;
		}

		if( !CanSense( pSensing, pRecord ) )
		{
			continue;
		}

		fPlayerDistSqr = pSensing->GetSensingPosition().DistSqr( pRecord->m_vStimulusPos );
		if( fPlayerDistSqr < fNearestPlayerDistSqr )
		{
			fNearestPlayerDistSqr = fPlayerDistSqr;
			pNearestPlayerRecord = pRecord;
		}
	}

	if( pNearestPlayerRecord )
	{
		pSensing->ProcessStimulus( pNearestPlayerRecord );

		if( pSensing->HandleSenseRecord( pNearestPlayerRecord, m_nCycle ) )
		{
			// Count number of AIs responding, and keep track of who they are.

			pNearestPlayerRecord->m_lstCurResponders.push_back( pSensing->GetSensingObject() );

			return LTTRUE;
		}
	}

	return LTFALSE;
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
bool CAIStimulusMgr::CanSense( IAISensing* pSensing, CAIStimulusRecord* pRecord ) const
{
	UBER_ASSERT( pSensing != NULL, "CAIStimulusMgr::CanSense NULL sensing object" );
	UBER_ASSERT( pRecord != NULL, "CAIStimulusMgr::CanSense NULL Record" );

	HOBJECT hSensing = pSensing->GetSensingObject();

	// Some stimuli have a max number of AIs that can respond.

	if( (pRecord->m_nMaxResponders == 0) ||
		(pRecord->m_lstCurResponders.size() < pRecord->m_nMaxResponders) )
	{
		EnumAISenseType eSenseType = pRecord->m_pAIBM_Stimulus->eSenseType;

		// Check for an AI Volume's sense mask, 
		// muting certain senses.
		
		uint32 flagsSenseMask = 0xffffffff;
		if( pRecord->m_pInformationVolume && IsAI( hSensing ) )
		{
			CAI* pAI = (CAI*)pSensing;
			if( !pAI->IsSuspicious() )
			{
				AIInformationVolume* pInfoVolume = dynamic_cast<AIInformationVolume*>(pRecord->m_pInformationVolume );
				if( pInfoVolume	&& pInfoVolume->IsOn() )
				{
					flagsSenseMask = pInfoVolume->GetSenseMask();
				}
			}
		}

		// Check if this AI responds to this stimulus.
		if( (eSenseType & flagsSenseMask) & pSensing->GetCurSenseFlags() )
		{
			// Check that stimulus is not the AI itself IF we the stimulus cares
			// about it
			if (pRecord->m_pAIBM_Stimulus->bRequireSourceIsNotSelf && 
				(HOBJECT)pRecord->m_hStimulusSource == hSensing )
			{
				return false;
			}

			if (pRecord->m_pAIBM_Stimulus->bRequireSourceIsSelf && 
				(HOBJECT)pRecord->m_hStimulusSource != hSensing )
			{
				return false;
			}
			
			// Check if the AI is already responding to this stimulus.
			if( !pRecord->IsAIResponding( hSensing ) )
			{
				// See if the AI feels such a way about the stimulus record
				// that they care about it (that they receive it)
				if( IsAlignmentInList( pSensing->GetSenseRelationSet(),
						pRecord->m_RelationData,
						pRecord->m_RequiredAlignment ) )
				{
					LTVector vSensingPos = pSensing->GetSensingPosition();

					// Check vertical cut-off.

					if( pRecord->m_pAIBM_Stimulus->fVerticalRadius > 0.f )
					{
						// AI is above the cut-off.

						if( vSensingPos.y > pRecord->m_vStimulusPos.y + pRecord->m_pAIBM_Stimulus->fVerticalRadius )
						{
							return false;
						}

						// AI is below the cut-off.

						if( vSensingPos.y < pRecord->m_vStimulusPos.y - pRecord->m_pAIBM_Stimulus->fVerticalRadius )
						{
							return false;
						}
					}

					// Check radius.
					float fDistance = VEC_DIST(pRecord->m_vStimulusPos, vSensingPos );
					if(fDistance < (pSensing->GetSenseDistance(eSenseType) + pRecord->m_fDistance) )
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIStimulusMgr::IsAlignmentInList()
//              
//	PURPOSE:	Returns true if the relation set contains a relation to the 
//				specified relation data defined Records list of acceptable
//				relations.
//
//				Returns true if the Relationship is in the list, false if it
//				is not.				
//              
//----------------------------------------------------------------------------
bool CAIStimulusMgr::IsAlignmentInList(const RelationSet& RelationSet,
													  const RelationData& RelationData,
													  const CAIStimulusRecord::_listAlignments& AlignmentRequirement ) const
{
	// Check if alignment requirement is met.
	CharacterAlignment ca = GetAlignment(RelationSet, RelationData);

	// See if the determined alignement is in the list of required
	// alignments the stimulus
	CAIStimulusRecord::_listAlignments::const_iterator it =
		std::find( AlignmentRequirement.begin(), AlignmentRequirement.end(), ca );

	return ( it != AlignmentRequirement.end() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStimulusMgr::RenderStimulus()
//
//	PURPOSE:	Toggle debug rendering of stimulus.
//
// ----------------------------------------------------------------------- //

void CAIStimulusMgr::RenderStimulus(LTBOOL bRender)
{
	if( (!m_bRenderStimulus) && bRender )
	{
		m_bRenderStimulus = bRender;

		ObjectCreateStruct theStruct;

		INIT_OBJECTCREATESTRUCT(theStruct);

		SAFE_STRCPY(theStruct.m_Filename, STIMULUS_MODEL_FILE);
		theStruct.m_ObjectType = OT_MODEL;


		// Create MAX_STIMULUS_RENDER objects, and hide them.

		for(uint8 iModel=0; iModel < MAX_STIMULUS_RENDER; ++iModel)
		{
			HCLASS hClass = g_pLTServer->GetClass("BaseClass");
			LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
			AIASSERT(pModel, LTNULL, "CAIStimulusMgr::RenderStimulus: Could not load sphere.");
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
	if(s_lstStimulusDist.size() < m_stmStimuliMap.size())
	{
		s_lstStimulusDist.resize(m_stmStimuliMap.size());
	}

	// Calculate and record the distance from the player to each stimulus.

	LTFLOAT fDistSqr;
	CAIStimulusRecord* pRecord;
	AISTIMULUS_MAP::iterator it;
	STIMULUS_DIST_LIST::iterator lit;
	for(it = m_stmStimuliMap.begin(), lit = s_lstStimulusDist.begin(); it != m_stmStimuliMap.end(); ++it, ++lit)
	{
		pRecord	 = it->second;
		fDistSqr = vPlayerPos.DistSqr(pRecord->m_vStimulusPos);

		lit->pRecord  = pRecord;
		lit->fDistSqr = fDistSqr;
	}

	// Clear any extras in the dist list.
	while(lit != s_lstStimulusDist.end())
	{
		lit->pRecord  = LTNULL;
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

		if(pRecord == LTNULL)
		{
			g_pCommonLT->SetObjectFlags(s_hStimulusModel[nStimulus], OFT_Flags, 0, FLAGMASK_ALL);
		}
		else 
		{ 
			g_pLTServer->SetObjectPos( s_hStimulusModel[nStimulus], &(pRecord->m_vStimulusPos) );
			g_pCommonLT->SetObjectFlags(s_hStimulusModel[nStimulus], OFT_Flags, FLAG_VISIBLE, FLAGMASK_ALL);

			LTFLOAT fRadius = Min<LTFLOAT>(STIMULUS_RADIUS_SMALL, pRecord->m_fDistance);
			LTVector vScale(fRadius, fRadius, fRadius);
		    g_pLTServer->ScaleObject(s_hStimulusModel[nStimulus], &vScale);

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

	return LTNULL;
}

EnumAIStimulusType CAIStimulusMgr::StimulusFromString(char* szStimulusType)
{
	for(uint32 iStimulusType = 0; iStimulusType < kStim_Count; ++iStimulusType)
	{
		if( stricmp(szStimulusType, s_aszStimulusTypes[iStimulusType]) == 0 )
		{
			return (EnumAIStimulusType)(1 << iStimulusType);
		}
	}

	return kStim_InvalidType;
}
