// ----------------------------------------------------------------------- //
//
// MODULE  : AIWorkingMemory.cpp
//
// PURPOSE : AIWorkingMemory abstract class implementation
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIWorkingMemory.h"
#include "AI.h"
#include "AITarget.h"
#include "AIStimulusMgr.h"
#include "AIPathMgrNavMesh.h"
#include "AIDB.h"
#include "Turret.h"
#include "AIUtils.h"
#include "AINode.h"

DEFINE_AI_FACTORY_CLASS( CAIWMFact );
DEFINE_AI_FACTORY_CLASS( CAIWorkingMemory );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWMFact::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIWMFact::CAIWMFact()
{
	m_fUpdateTime = 0.f;

	m_bDeleted = false;

	m_bitsFactMask.reset();

	m_eFactType = kFact_InvalidType;

	m_dwFactFlags = kFactFlag_None;
	m_hSourceObject = NULL;
	m_hTargetObject = NULL;
	m_eNodeType = kNode_InvalidType;
	m_eDesireType = kDesire_InvalidType;
	m_eTaskType = kTask_InvalidType;
	m_eKnowledgeType = kKnowledge_InvalidType;
	m_nIndex = -1;
	m_factStimulus.eStimulusType = kStim_InvalidType;
	m_factStimulus.eStimulusID = kStimID_Unset;
	m_factStimulus.fStimulationDecreaseTime = 0.f;
	m_fRadius = 0.f;
	m_fTime = 0.f;

	for( int iMask=0; iMask < kFactMask_Count; ++iMask )
	{
		m_fConfidences[iMask] = 0.f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWMFact::Save
//
//	PURPOSE:	Save
//
// ----------------------------------------------------------------------- //

void CAIWMFact::Save(ILTMessage_Write *pMsg)
{
	SAVE_DWORD( m_eFactType );
	SAVE_DWORD( m_eFactID );

	SAVE_bool( m_bDeleted );

	SAVE_DWORD( m_dwFactFlags );
	SAVE_HOBJECT( m_hSourceObject );
	SAVE_HOBJECT( m_hTargetObject );
	SAVE_DWORD(	m_eNodeType );
	SAVE_DWORD( m_eDesireType );
	SAVE_DWORD( m_eTaskType );
	SAVE_DWORD( m_eKnowledgeType );

	SAVE_DWORD( m_factStimulus.eStimulusID );
	SAVE_DWORD( m_factStimulus.eStimulusType );
	SAVE_TIME( m_factStimulus.fStimulationDecreaseTime );

	SAVE_DWORD( m_factDamage.eDamageType );
	SAVE_FLOAT( m_factDamage.fDamageAmount );
	SAVE_VECTOR( m_factDamage.vDamageDir );

	SAVE_INT( m_nIndex );
	SAVE_VECTOR( m_vPos );
	SAVE_VECTOR( m_vDir );
	SAVE_FLOAT(	m_fRadius );
	SAVE_TIME(	m_fTime );

	for( uint32 iMask = 0; iMask < kFactMask_Count; ++iMask )
	{
		SAVE_FLOAT( m_fConfidences[iMask] ); 
	}

	SAVE_DWORD( m_bitsFactMask.to_ulong() );

	SAVE_TIME( m_fUpdateTime );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWMFact::Load
//
//	PURPOSE:	Load
//
// ----------------------------------------------------------------------- //

void CAIWMFact::Load(ILTMessage_Read *pMsg)
{
	LOAD_DWORD_CAST( m_eFactType, ENUM_AIWMFACT_TYPE );
	LOAD_DWORD_CAST( m_eFactID, ENUM_FactID );

	LOAD_bool( m_bDeleted );

	LOAD_DWORD( m_dwFactFlags );
	LOAD_HOBJECT( m_hSourceObject );
	LOAD_HOBJECT( m_hTargetObject );
	LOAD_DWORD_CAST( m_eNodeType, EnumAINodeType );
	LOAD_DWORD_CAST( m_eDesireType, ENUM_AIWMDESIRE_TYPE );
	LOAD_DWORD_CAST( m_eTaskType, ENUM_AIWMTASK_TYPE );
	LOAD_DWORD_CAST( m_eKnowledgeType, ENUM_AIWMKNOWLEDGE_TYPE );

	LOAD_DWORD_CAST( m_factStimulus.eStimulusID, EnumAIStimulusID );
	LOAD_DWORD_CAST( m_factStimulus.eStimulusType, EnumAIStimulusType );
	LOAD_TIME( m_factStimulus.fStimulationDecreaseTime );

	LOAD_DWORD_CAST( m_factDamage.eDamageType, DamageType );
	LOAD_FLOAT( m_factDamage.fDamageAmount );
	LOAD_VECTOR( m_factDamage.vDamageDir );

	LOAD_INT( m_nIndex );
	LOAD_VECTOR( m_vPos );
	LOAD_VECTOR( m_vDir );
	LOAD_FLOAT(	m_fRadius );
	LOAD_TIME(	m_fTime );

	for( uint32 iMask = 0; iMask < kFactMask_Count; ++iMask )
	{
		LOAD_FLOAT( m_fConfidences[iMask] ); 
	}

	uint32 dwFactMask;
	LOAD_DWORD( dwFactMask );
	m_bitsFactMask = std::bitset<kFactMask_Count>( dwFactMask );

	LOAD_TIME( m_fUpdateTime );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWMFact::MarkForDeletion
//
//	PURPOSE:	Set this fact as deleted.
//
// ----------------------------------------------------------------------- //

void CAIWMFact::MarkForDeletion()
{
	m_bDeleted = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWMFact::MatchesQuery
//
//	PURPOSE:	Return true if this Fact matches the query.
//
// ----------------------------------------------------------------------- //

bool CAIWMFact::MatchesQuery( const CAIWMFact& factQuery )
{
	// Deleted facts never match anything.

	if( m_bDeleted )
	{
		return false;
	}

	// FactID.

	if( factQuery.m_bitsFactMask.test( kFactMask_FactID ) )
	{
		if( ( !m_bitsFactMask.test( kFactMask_FactID ) ) ||
			( m_eFactID != factQuery.GetFactID() ) )
		{
			return false;
		}
	}

	// FactType.

	if( factQuery.m_bitsFactMask.test( kFactMask_FactType ) )
	{
		if( ( !m_bitsFactMask.test( kFactMask_FactType ) ) ||
			( m_eFactType != factQuery.GetFactType() ) )
		{
			return false;
		}
	}

	// Fact Flags.

	if( factQuery.m_bitsFactMask.test( kFactMask_FactFlags ) )
	{
		if( ( !m_bitsFactMask.test( kFactMask_FactFlags ) ) ||
			( ( m_dwFactFlags & factQuery.GetFactFlags() ) != factQuery.GetFactFlags() ) )
		{
			return false;
		}
	}

	// SourceObject.

	if( factQuery.m_bitsFactMask.test( kFactMask_SourceObject ) )
	{
		if( ( !m_bitsFactMask.test( kFactMask_SourceObject ) ) ||
			( m_hSourceObject != factQuery.GetSourceObject() ) )
		{
			return false;
		}
	}

	// TargetObject.

	if( factQuery.m_bitsFactMask.test( kFactMask_TargetObject ) )
	{
		if( ( !m_bitsFactMask.test( kFactMask_TargetObject ) ) ||
			( m_hTargetObject != factQuery.GetTargetObject() ) )
		{
			return false;
		}
	}

	// NodeType.

	if( factQuery.m_bitsFactMask.test( kFactMask_NodeType ) )
	{
		if( ( !m_bitsFactMask.test( kFactMask_NodeType ) ) ||
			( m_eNodeType != factQuery.GetNodeType() ) )
		{
			return false;
		}
	}

	// DesireType.

	if( factQuery.m_bitsFactMask.test( kFactMask_DesireType ) )
	{
		if( ( !m_bitsFactMask.test( kFactMask_DesireType ) ) ||
			( m_eDesireType != factQuery.GetDesireType() ) )
		{
			return false;
		}
	}

	// TaskType.

	if( factQuery.m_bitsFactMask.test( kFactMask_TaskType ) )
	{
		if( ( !m_bitsFactMask.test( kFactMask_TaskType ) ) ||
			( m_eTaskType != factQuery.GetTaskType() ) )
		{
			return false;
		}
	}

	// KnowledgeType.

	if( factQuery.m_bitsFactMask.test( kFactMask_KnowledgeType ) )
	{
		if( ( !m_bitsFactMask.test( kFactMask_KnowledgeType ) ) ||
			( m_eKnowledgeType != factQuery.GetKnowledgeType() ) )
		{
			return false;
		}
	}

	// Stimulus.

	if( factQuery.m_bitsFactMask.test( kFactMask_Stimulus ) )
	{
		if( !m_bitsFactMask.test( kFactMask_Stimulus ) )
		{
			return false;
		}

		EnumAIStimulusType	eStimulusType;
		EnumAIStimulusID	eStimulusID;
		factQuery.GetStimulus( &eStimulusType, &eStimulusID );

		if( ( eStimulusType != kStim_InvalidType ) &&
			( eStimulusType != m_factStimulus.eStimulusType ) )
		{
			return false;
		}

		if( ( eStimulusID != kStimID_Invalid ) &&
			( eStimulusID != m_factStimulus.eStimulusID ) )
		{
			return false;
		}
	}

	// Damage.

	if( factQuery.m_bitsFactMask.test( kFactMask_Damage ) )
	{
		if( !m_bitsFactMask.test( kFactMask_Damage ) )
		{
			return false;
		}

		DamageType		eDamageType;
		float			fDamageAmount;
		factQuery.GetDamage( &eDamageType, &fDamageAmount, NULL );

		if( m_factDamage.eDamageType != eDamageType )
		{
			return false;
		}
	}

	// Index.

	if( factQuery.m_bitsFactMask.test( kFactMask_Index ) )
	{
		if( ( !m_bitsFactMask.test( kFactMask_Index ) ) ||
			( m_nIndex != factQuery.GetIndex() ) )
		{	
			return false;
		}
	}
	
	// Position.

	if( factQuery.m_bitsFactMask.test( kFactMask_Position ) )
	{
		if( ( !m_bitsFactMask.test( kFactMask_Position ) ) ||
			( m_vPos != factQuery.GetPos() ) )
		{
			return false;
		}
	}

	// Direction.

	if( factQuery.m_bitsFactMask.test( kFactMask_Direction ) )
	{
		if( ( !m_bitsFactMask.test( kFactMask_Direction ) ) ||
			( m_vDir != factQuery.GetDir() ) )
		{
			return false;
		}
	}

	// Radius.

	if( factQuery.m_bitsFactMask.test( kFactMask_Radius ) )
	{
		if( ( !m_bitsFactMask.test( kFactMask_Radius ) ) ||
			( m_fRadius != factQuery.GetRadius() ) )
		{
			return false;
		}
	}

	// Time.

	if( factQuery.m_bitsFactMask.test( kFactMask_Time ) )
	{
		if( ( !m_bitsFactMask.test( kFactMask_Time ) ) ||
			( m_fTime != factQuery.GetTime() ) )
		{
			return false;
		}
	}

	// Found a match.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWMFact::GetConfidence
//
//	PURPOSE:	Get confidence value for some mask.
//
// ----------------------------------------------------------------------- //

float CAIWMFact::GetConfidence( EnumFactMask eFactMask ) const
{
	if( ( eFactMask < 0 ) || ( eFactMask >= kFactMask_Count ) )
	{
		AIASSERT( 0, NULL, "CAIWMFact::GetConfidence: Invalid FactMask!" );
		return 0.f;
	}

	AIASSERT( m_bitsFactMask.test( eFactMask ), NULL, "CAIWMFact::GetConfidence: FactMask not set!" );
	return m_fConfidences[eFactMask];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWMFact::SetConfidence
//
//	PURPOSE:	Set confidence value for some mask.
//
// ----------------------------------------------------------------------- //

void CAIWMFact::SetConfidence( EnumFactMask eFactMask, float fConfidence )
{
	if( ( eFactMask < 0 ) || ( eFactMask >= kFactMask_Count ) )
	{
		AIASSERT( 0, NULL, "CAIWMFact::SetConfidence: Invalid FactMask!" );
		return;
	}

	AIASSERT( m_bitsFactMask.test( eFactMask ), NULL, "CAIWMFact::SetConfidence: FactMask not set!" );
	m_fConfidences[eFactMask] = fConfidence;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWMFact::SetFact*
//
//	PURPOSE:	Set members.
//
// ----------------------------------------------------------------------- //

void CAIWMFact::SetFactType( ENUM_AIWMFACT_TYPE eFactType, float fConfidence )
{
	m_eFactType = eFactType;
	m_fConfidences[kFactMask_FactType] = fConfidence;
	m_bitsFactMask.set( kFactMask_FactType );
}

// ----------------------------------------------------------------------- //

void CAIWMFact::SetFactID( ENUM_FactID eFactID, float fConfidence )
{
	m_eFactID = eFactID;
	m_fConfidences[kFactMask_FactID] = fConfidence;
	m_bitsFactMask.set( kFactMask_FactID );
}

// ----------------------------------------------------------------------- //

void CAIWMFact::SetFactFlags( uint32 dwFactFlags, float fConfidence )
{
	m_dwFactFlags = dwFactFlags;
	m_fConfidences[kFactMask_FactFlags] = fConfidence;
	m_bitsFactMask.set( kFactMask_FactFlags );
}

// ----------------------------------------------------------------------- //

void CAIWMFact::SetSourceObject( HOBJECT hSourceObject, float fConfidence )
{
	m_hSourceObject = hSourceObject;
	m_fConfidences[kFactMask_SourceObject] = fConfidence;
	m_bitsFactMask.set( kFactMask_SourceObject );
}

// ----------------------------------------------------------------------- //

void CAIWMFact::SetTargetObject( HOBJECT hTargetObject, float fConfidence )
{
	m_hTargetObject = hTargetObject;
	m_fConfidences[kFactMask_TargetObject] = fConfidence;
	m_bitsFactMask.set( kFactMask_TargetObject );
}

// ----------------------------------------------------------------------- //

void CAIWMFact::SetNodeType( EnumAINodeType	eNodeType, float fConfidence )
{
	m_eNodeType = eNodeType;
	m_fConfidences[kFactMask_NodeType] = fConfidence;
	m_bitsFactMask.set( kFactMask_NodeType );
}

// ----------------------------------------------------------------------- //

void CAIWMFact::SetDesireType( ENUM_AIWMDESIRE_TYPE eDesireType, float fConfidence )
{
	m_eDesireType = eDesireType;
	m_fConfidences[kFactMask_DesireType] = fConfidence;
	m_bitsFactMask.set( kFactMask_DesireType );
}

// ----------------------------------------------------------------------- //

void CAIWMFact::SetTaskType( ENUM_AIWMTASK_TYPE eTaskType, float fConfidence )
{
	m_eTaskType = eTaskType;
	m_fConfidences[kFactMask_TaskType] = fConfidence;
	m_bitsFactMask.set( kFactMask_TaskType );
}

// ----------------------------------------------------------------------- //

void CAIWMFact::SetKnowledgeType( ENUM_AIWMKNOWLEDGE_TYPE eKnowledgeType, float fConfidence )
{
	m_eKnowledgeType = eKnowledgeType;
	m_fConfidences[kFactMask_KnowledgeType] = fConfidence;
	m_bitsFactMask.set( kFactMask_KnowledgeType );
}

// ----------------------------------------------------------------------- //

void CAIWMFact::SetStimulus( EnumAIStimulusType	eStimulusType, EnumAIStimulusID	eStimulusID, float fConfidence )
{
	m_factStimulus.eStimulusType = eStimulusType;
	m_factStimulus.eStimulusID = eStimulusID;

	m_fConfidences[kFactMask_Stimulus] = fConfidence;
	m_bitsFactMask.set( kFactMask_Stimulus );
}

// ----------------------------------------------------------------------- //

void CAIWMFact::SetDamage( DamageType eDamageType, float fDamageAmount, const LTVector& vDamageDir, float fConfidence )
{
	m_factDamage.eDamageType = eDamageType;
	m_factDamage.fDamageAmount = fDamageAmount;
	m_factDamage.vDamageDir = vDamageDir;

	m_fConfidences[kFactMask_Damage] = fConfidence;
	m_bitsFactMask.set( kFactMask_Damage );
}

// ----------------------------------------------------------------------- //

void CAIWMFact::SetIndex( int nIndex, float fConfidence )
{
	m_nIndex = nIndex;
	m_fConfidences[kFactMask_Index] = fConfidence;
	m_bitsFactMask.set( kFactMask_Index );
}

// ----------------------------------------------------------------------- //

void CAIWMFact::SetPos( const LTVector& vPos, float fConfidence )
{
	m_vPos = vPos;
	m_fConfidences[kFactMask_Position] = fConfidence;
	m_bitsFactMask.set( kFactMask_Position );
}

// ----------------------------------------------------------------------- //

void CAIWMFact::SetDir( const LTVector& vDir, float fConfidence )
{
	m_vDir = vDir;
	m_fConfidences[kFactMask_Direction] = fConfidence;
	m_bitsFactMask.set( kFactMask_Direction );
}

// ----------------------------------------------------------------------- //

void CAIWMFact::SetRadius( float fRadius, float fConfidence )
{
	m_fRadius = fRadius;
	m_fConfidences[kFactMask_Radius] = fConfidence;
	m_bitsFactMask.set( kFactMask_Radius );
}

// ----------------------------------------------------------------------- //

void CAIWMFact::SetTime( double fTime, float fConfidence )
{
	m_fTime = fTime;
	m_fConfidences[kFactMask_Time] = fConfidence;
	m_bitsFactMask.set( kFactMask_Time );
}

// ----------------------------------------------------------------------- //

void CAIWMFact::SetStimulationDecreaseTime( double fTime )
{
	m_factStimulus.fStimulationDecreaseTime = fTime;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWMFact::GetFact*
//
//	PURPOSE:	Get members.
//
// ----------------------------------------------------------------------- //

ENUM_AIWMFACT_TYPE CAIWMFact::GetFactType() const
{
	AIASSERT( m_bitsFactMask.test( kFactMask_FactType ), NULL, "CAIWMFact::GetFactType: Type not set!" );
	return m_eFactType;
}

// ----------------------------------------------------------------------- //

ENUM_FactID	CAIWMFact::GetFactID() const
{
	AIASSERT( m_bitsFactMask.test( kFactMask_FactID ), NULL, "CAIWMFact::GetFactID: ID not set!" );
	return m_eFactID;
}

// ----------------------------------------------------------------------- //

uint32 CAIWMFact::GetFactFlags() const
{
	AIASSERT( m_bitsFactMask.test( kFactMask_FactFlags ), NULL, "CAIWMFact::GetFactFlags: FactFlags not set!" );
	return m_dwFactFlags;
}

// ----------------------------------------------------------------------- //

HOBJECT	CAIWMFact::GetSourceObject() const
{
	AIASSERT( m_bitsFactMask.test( kFactMask_SourceObject ), NULL, "CAIWMFact::GetSourceObject: Source Object not set!" );
	return m_hSourceObject;
}

// ----------------------------------------------------------------------- //

HOBJECT	CAIWMFact::GetTargetObject() const
{
	AIASSERT( m_bitsFactMask.test( kFactMask_TargetObject ), NULL, "CAIWMFact::GetTargetObject: Target Object not set!" );
	return m_hTargetObject;
}

// ----------------------------------------------------------------------- //

EnumAINodeType CAIWMFact::GetNodeType() const
{
	AIASSERT( m_bitsFactMask.test( kFactMask_NodeType ), NULL, "CAIWMFact::GetNodeType: NodeType not set!" );
	return m_eNodeType;
}

// ----------------------------------------------------------------------- //

ENUM_AIWMDESIRE_TYPE CAIWMFact::GetDesireType() const
{
	AIASSERT( m_bitsFactMask.test( kFactMask_DesireType ), NULL, "CAIWMFact::GetDesireType: DesireType not set!" );
	return m_eDesireType;
}

// ----------------------------------------------------------------------- //

ENUM_AIWMTASK_TYPE CAIWMFact::GetTaskType() const
{
	AIASSERT( m_bitsFactMask.test( kFactMask_TaskType ), NULL, "CAIWMFact::GetTaskType: TaskType not set!" );
	return m_eTaskType;
}

// ----------------------------------------------------------------------- //

ENUM_AIWMKNOWLEDGE_TYPE	CAIWMFact::GetKnowledgeType() const
{
	AIASSERT( m_bitsFactMask.test( kFactMask_KnowledgeType ), NULL, "CAIWMFact::GetKnowledgeType: KnowledgeType not set!" );
	return m_eKnowledgeType;
}

// ----------------------------------------------------------------------- //

void CAIWMFact::GetStimulus( EnumAIStimulusType* peStimulusType, EnumAIStimulusID* peStimulusID ) const
{
	AIASSERT( m_bitsFactMask.test( kFactMask_Stimulus ), NULL, "CAIWMFact::GetStimulus: Stimulus not set!" );

	if( peStimulusType )
	{
		*peStimulusType = m_factStimulus.eStimulusType;
	}

	if( peStimulusID )
	{
		*peStimulusID = m_factStimulus.eStimulusID;
	}
}

// ----------------------------------------------------------------------- //

void CAIWMFact::GetDamage( DamageType* peDamageType, float* pfDamageAmount, LTVector* pvDamageDir ) const
{
	AIASSERT( m_bitsFactMask.test( kFactMask_Damage ), NULL, "CAIWMFact::GetDamage: Damage not set!" );

	if( peDamageType )
	{
		*peDamageType = m_factDamage.eDamageType;
	}

	if( pfDamageAmount )
	{
		*pfDamageAmount = m_factDamage.fDamageAmount;
	}

	if( pvDamageDir )
	{
		*pvDamageDir = m_factDamage.vDamageDir;
	}
}

// ----------------------------------------------------------------------- //

int	CAIWMFact::GetIndex() const
{
	AIASSERT( m_bitsFactMask.test( kFactMask_Index ), NULL, "CAIWMFact::GetIndex: Index not set!" );
	return m_nIndex;
}

// ----------------------------------------------------------------------- //

const LTVector&	CAIWMFact::GetPos() const
{
	AIASSERT( m_bitsFactMask.test( kFactMask_Position ), NULL, "CAIWMFact::GetPos: Position not set!" );
	return m_vPos;
}

// ----------------------------------------------------------------------- //

const LTVector&	CAIWMFact::GetDir() const
{
	AIASSERT( m_bitsFactMask.test( kFactMask_Direction ), NULL, "CAIWMFact::GetDir: Direction not set!" );
	return m_vDir;
}

// ----------------------------------------------------------------------- //

float CAIWMFact::GetRadius() const
{
	AIASSERT( m_bitsFactMask.test( kFactMask_Radius ), NULL, "CAIWMFact::GetRadius: Radius not set!" );
	return m_fRadius;
}

// ----------------------------------------------------------------------- //

double CAIWMFact::GetTime() const
{
	AIASSERT( m_bitsFactMask.test( kFactMask_Time ), NULL, "CAIWMFact::GetTime: Time not set!" );
	return m_fTime;
}


// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemory::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIWorkingMemory::CAIWorkingMemory() : 
	m_eNextUnusedFactID( (ENUM_FactID)0 ),
	m_bGarbageExists( false )
{
	m_plstWMFact = &m_lstWMFacts;
}

CAIWorkingMemory::~CAIWorkingMemory()
{
	// Clear memory.
	ResetWorkingMemory();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemory::ResetWorkingMemory
//
//	PURPOSE:	Destroys all information in AIWorkingMemory, clearing it
//				out and putting it back to its initial state.  Doing this
//				also resets its UnusedFactID, so it is only recommended
//				when a truely new version is desired.
//
// ----------------------------------------------------------------------- //

void CAIWorkingMemory::ResetWorkingMemory()
{
	CAIWMFact* pFact;
	AIWORKING_MEMORY_FACT_LIST::iterator itFact;
	for( itFact = m_lstWMFacts.begin(); itFact != m_lstWMFacts.end(); ++itFact )
	{
		pFact = *itFact;
		AI_FACTORY_DELETE( pFact );
	}

	m_lstWMFacts.resize( 0 );
	m_bGarbageExists = false;

	m_eNextUnusedFactID = (ENUM_FactID)0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemory::Save/Load
//
//	PURPOSE:	Save/Load
//
// ----------------------------------------------------------------------- //

void CAIWorkingMemory::Save(ILTMessage_Write *pMsg)
{
	SAVE_INT(m_lstWMFacts.size());
	for (std::size_t i = 0; i < m_lstWMFacts.size(); ++i)
	{
		m_lstWMFacts[i]->Save(pMsg);
	}

	SAVE_INT(m_eNextUnusedFactID);
	SAVE_bool(m_bGarbageExists);
}

void CAIWorkingMemory::Load(ILTMessage_Read *pMsg)
{
	int nFacts = 0;
	LOAD_INT(nFacts);
	for (int i = 0; i < nFacts; ++i)
	{
		CAIWMFact* pFact = AI_FACTORY_NEW( CAIWMFact );
		pFact->Load(pMsg);
		m_lstWMFacts.push_back(pFact);
	}

	LOAD_INT_CAST(m_eNextUnusedFactID, ENUM_FactID);
	LOAD_bool(m_bGarbageExists);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemory::CreateWMFact
//
//	PURPOSE:	Create a new fact.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAIWorkingMemory::CreateWMFact( ENUM_AIWMFACT_TYPE eFactType )
{
	CAIWMFact* pFact = AI_FACTORY_NEW( CAIWMFact );
	if( pFact )
	{
		pFact->SetFactType( eFactType, 1.f );

		pFact->SetFactID( m_eNextUnusedFactID, 1.f );
		m_eNextUnusedFactID = (ENUM_FactID)(m_eNextUnusedFactID + 1);

		m_lstWMFacts.push_back( pFact );
	}

	return pFact;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemory::ClearWMFacts
//
//	PURPOSE:	Clears the first fact matching the passed in description.
//
// ----------------------------------------------------------------------- //

void CAIWorkingMemory::ClearWMFact( const CAIWMFact& factQuery )
{
	// Find the specified fact, and delete it.

	CAIWMFact* pFact;
	AIWORKING_MEMORY_FACT_LIST::iterator itFact;

	for( itFact = m_lstWMFacts.begin(); itFact != m_lstWMFacts.end(); ++itFact )
	{
		pFact = *itFact;
		if( pFact->MatchesQuery( factQuery ) )
		{
			pFact->MarkForDeletion();
			m_bGarbageExists = true;
			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemory::ClearWMFact
//
//	PURPOSE:	Clears the fact matching the passed in pointer.
//
// ----------------------------------------------------------------------- //

void CAIWorkingMemory::ClearWMFact( CAIWMFact* pFactRemove )
{
	pFactRemove->MarkForDeletion();
	m_bGarbageExists = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemory::ClearWMFacts
//
//	PURPOSE:	Clear facts that match the passed in description.
//
// ----------------------------------------------------------------------- //

void CAIWorkingMemory::ClearWMFacts( const CAIWMFact& factQuery )
{
	CAIWMFact* pFact;
	AIWORKING_MEMORY_FACT_LIST::iterator itFact;
	for( itFact = m_lstWMFacts.begin(); itFact != m_lstWMFacts.end(); ++itFact )
	{
		pFact = *itFact;
		if( pFact->MatchesQuery( factQuery ) )
		{
			pFact->MarkForDeletion();
			m_bGarbageExists = true;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemory::CollectGarbage
//
//	PURPOSE:	Truly delete facts that have been marked for deletion.
//
// ----------------------------------------------------------------------- //

void CAIWorkingMemory::CollectGarbage()
{
	// Bail if no facts are waiting to be deleted.

	if( !m_bGarbageExists )
	{
		return;
	}

	// Delete marked facts.

	CAIWMFact* pFact;
	AIWORKING_MEMORY_FACT_LIST::iterator itFact;
	itFact = m_lstWMFacts.begin();
	while( itFact != m_lstWMFacts.end() )
	{
		pFact = *itFact;
		if( pFact->IsDeleted() )
		{
  			AI_FACTORY_DELETE( pFact );
  			itFact = m_lstWMFacts.erase( itFact );
		}
  		else {
  			++itFact;
  		}
	}

	// Clear flag.

	m_bGarbageExists = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemory::FindWMFact
//
//	PURPOSE:	Return a fact matching the passed in description.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAIWorkingMemory::FindWMFact( const CAIWMFact& factQuery )
{
	CAIWMFact* pFact;

	for( AIWORKING_MEMORY_FACT_LIST::const_iterator itFact = m_lstWMFacts.begin(); 
		itFact != m_lstWMFacts.end(); ++itFact )
	{
		pFact = *itFact;
		if( pFact->MatchesQuery( factQuery ) )
		{
			return pFact;
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemory::CountMatches
//
//	PURPOSE:	Returns the number of facts matching the passed in 
//				description.
//
// ----------------------------------------------------------------------- //

int CAIWorkingMemory::CountMatches( const CAIWMFact& factQuery ) const
{
	int MatchCount = 0;

	CAIWMFact* pFact;
	for( AIWORKING_MEMORY_FACT_LIST::const_iterator itFact = m_lstWMFacts.begin(); 
		itFact != m_lstWMFacts.end(); ++itFact )
	{
		pFact = *itFact;
		if( pFact->MatchesQuery( factQuery ) )
		{
			++MatchCount;
		}
	}

	return MatchCount;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemory::FindFactCharacterMax
//
//	PURPOSE:	Find the most stimulated character.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAIWorkingMemory::FindFactCharacterMax(CAI* pAI)
{
	CAIWMFact* pMaxFact = NULL;
	CAIWMFact* pFact;
	float fMaxConfidence = 0.f;

	AIWORKING_MEMORY_FACT_LIST::iterator itFact;
	for( itFact = m_lstWMFacts.begin(); itFact != m_lstWMFacts.end(); ++itFact )
	{
		// Ignore deleted facts.

		pFact = *itFact;
		if( pFact->IsDeleted() )
		{
			continue;
		}

		// Ignore facts which are not about characters.
		
		if ( pFact->GetFactType() != kFact_Character )
		{
			continue;
		}

		// Ignore character facts older than the most recent.

		if ( fMaxConfidence > pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus ) )
		{
			continue;
		}

		// Ignore dead AI.
		// Clear handles to dead AI.

		if( IsDeadAI( pFact->GetTargetObject() ) )
		{
			pFact->SetTargetObject( NULL );
			continue;
		}

		fMaxConfidence = pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus );
		pMaxFact = pFact;
	}	

	// Remove any character facts with NULL handles, as these characters
	// have been removed.

	CAIWMFact factRemoveQuery;
	factRemoveQuery.SetFactType( kFact_Character );
	factRemoveQuery.SetTargetObject( NULL );
	ClearWMFacts( factRemoveQuery );

	return pMaxFact;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemory::FindFactCharacterNearest
//
//	PURPOSE:	Find the nearest character.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAIWorkingMemory::FindFactCharacterNearest(CAI* pAI)
{
	float fDistSqr;
	float fMinDistSqr = FLT_MAX;
	CAIWMFact* pNearestFact = NULL;
	CAIWMFact* pFact;

	AIWORKING_MEMORY_FACT_LIST::iterator itFact;
	for( itFact = m_lstWMFacts.begin(); itFact != m_lstWMFacts.end(); ++itFact )
	{
		// Ignore deleted facts.

		pFact = *itFact;
		if( pFact->IsDeleted() )
		{
			continue;
		}

		// Ignore facts which are not about characters.

		if ( pFact->GetFactType() != kFact_Character )
		{
			continue;
		}

		// Ignore dead AI.
		// Clear handles to dead AI.

		if( IsDeadAI( pFact->GetTargetObject() ) )
		{
			pFact->SetTargetObject( NULL );
			continue;
		}

		// Record the nearest character.

		fDistSqr = pAI->GetPosition().DistSqr( pFact->GetPos() );
		if( fDistSqr < fMinDistSqr )
		{
			fMinDistSqr = fDistSqr;
			pNearestFact = pFact;
		}
	}	

	// Remove any character facts with NULL handles, as these characters
	// have been removed.

	CAIWMFact factRemoveQuery;
	factRemoveQuery.SetFactType( kFact_Character );
	factRemoveQuery.SetTargetObject( NULL );
	ClearWMFacts( factRemoveQuery );

	return pNearestFact;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemory::FindFactPlayerTurret
//
//	PURPOSE:	Return a fact for a player turret.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAIWorkingMemory::FindFactPlayerTurret(CAI* pAI)
{
	// Remove any character facts with NULL handles, as these objects
	// have been removed.

	CAIWMFact factRemoveQuery;
	factRemoveQuery.SetFactType( kFact_Object );
	factRemoveQuery.SetTargetObject( NULL );
	ClearWMFacts( factRemoveQuery );


	CAIWMFact* pFact;
	Turret* pTurret;
	HOBJECT hObject;
	AIWORKING_MEMORY_FACT_LIST::iterator itFact;
	for( itFact = m_lstWMFacts.begin(); itFact != m_lstWMFacts.end(); ++itFact )
	{
		// Ignore deleted facts.

		pFact = *itFact;
		if( pFact->IsDeleted() )
		{
			continue;
		}

		// Ignore facts which are not about objects.

		if ( pFact->GetFactType() != kFact_Object )
		{
			continue;
		}

		// Ignore facts which are not referring to player turrets.

		hObject = pFact->GetTargetObject();
		if( !IsTurret( hObject ) )
		{
			continue;
		}

		// Ignore turrets that are not in use.

		pTurret = (Turret*)g_pLTServer->HandleToObject( hObject );
		if( !pTurret->IsInUse() )
		{
			continue;
		}

		return pFact;
	}	

	// No player turrets found.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemory::FindFactNodeMax
//
//	PURPOSE:	Return a node fact with the maximum confidence.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAIWorkingMemory::FindFactNodeMax( CAI* pAI, EnumAINodeType eNodeType, uint32 dwNodeStatusMask, HOBJECT hExclude, HOBJECT hThreat )
{
	CAIWMFact* pFact;
	AINode* pNode;

	float fMaxConfidence = 0.f;
	CAIWMFact* pFactMax = NULL;
	bool bMaxInRadiusOrRegion = false;

	// Iterate over all facts, searching for the node fact with
	// the highest positional confidence.

	AIWORKING_MEMORY_FACT_LIST::iterator itFact;
	for( itFact = m_lstWMFacts.begin(); itFact != m_lstWMFacts.end(); ++itFact )
	{
		// Ignore deleted facts.

		pFact = *itFact;
		if( pFact->IsDeleted() )
		{
			continue;
		}

		// Ignore facts that are not node facts.
		// Ignore node facts with the wrong node type.
		// Ignore node facts whose object matches the hExclude.

		if( ( pFact->GetFactType() != kFact_Node ) ||
			( pFact->GetNodeType() != eNodeType ) ||
			( pFact->GetTargetObject() == hExclude ) )
		{
			continue;
		}

		// Ignore nodes that are locked, disabled, or timed out.

		pNode = (AINode*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
		if( ( !pNode ) ||
			( pNode->IsLockedDisabledOrTimedOut( pAI->GetHOBJECT() ) ) )
		{
			continue;
		}

		// Prefer nodes that include the AI in the radius or region.

		float fConfidence = pFact->GetConfidence( CAIWMFact::kFactMask_Position );
		bool bInRadiusOrRegion = pNode->IsAIInRadiusOrRegion( pAI, pAI->GetPosition(), 1.f );
		if( bMaxInRadiusOrRegion )
		{
			if( !bInRadiusOrRegion )
			{
				continue;
			}
			else if( fConfidence <= fMaxConfidence ) 
			{
				continue;
			}
		}
		else if( !bInRadiusOrRegion )
		{
			if( pFactMax && ( fConfidence <= fMaxConfidence ) )
			{
				continue;
			}
		}

		// Ignore nodes that are invalid.

		if( !pNode->IsNodeValid( pAI, pAI->GetPosition(), hThreat, kThreatPos_TargetPos, dwNodeStatusMask ) )
		{
			continue;
		}

		// AI cannot find a path to the node.

		if( !( pNode->AllowOutsideNavMesh() ||
			   g_pAIPathMgrNavMesh->HasPath( pAI, pAI->GetCharTypeMask(), pNode->GetNodeContainingNMPoly() ) ) )
		{
			continue;
		}

		// Keep track of the max pos confidence.

		fMaxConfidence = fConfidence;
		bMaxInRadiusOrRegion = bInRadiusOrRegion;
		pFactMax = pFact;
	}

	// Return the max.

	return pFactMax;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemory::FindFactNodeRandom
//
//	PURPOSE:	Return a node fact with a random confidence.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAIWorkingMemory::FindFactNodeRandom( CAI* pAI, EnumAINodeType eNodeType, uint32 dwNodeStatusMask, HOBJECT hExclude, HOBJECT hThreat )
{
	CAIWMFact* pFact;
	AINode* pNode;

	static AIWORKING_MEMORY_FACT_LIST lstNodeFacts;
	lstNodeFacts.resize( 0 );

	// Iterate over all facts colecting pointers to matching node facts.

	AIWORKING_MEMORY_FACT_LIST::iterator itFact;
	for( itFact = m_lstWMFacts.begin(); itFact != m_lstWMFacts.end(); ++itFact )
	{
		// Ignore deleted facts.

		pFact = *itFact;
		if( pFact->IsDeleted() )
		{
			continue;
		}

		// Ignore facts that are not node facts.
		// Ignore node facts with the wrong node type.
		// Ignore node facts whose object matches the hExclude.
		// Ignore node facts whose object matches the hExclude.

		if( ( pFact->GetFactType() != kFact_Node ) ||
			( pFact->GetNodeType() != eNodeType ) ||
			( pFact->GetTargetObject() == hExclude ) )
		{
			continue;
		}

		// Ignore nodes that are locked, disabled, or timed out.

		pNode = (AINode*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
		if( ( !pNode ) ||
			( pNode->IsLockedDisabledOrTimedOut( pAI->GetHOBJECT() ) ) )
		{
			continue;
		}

		// If AI is aware of a threat, ignore nodes that are invalid.

		if( hThreat && 
			( !pNode->IsNodeValid( pAI, pAI->GetPosition(), hThreat, kThreatPos_TargetPos, dwNodeStatusMask ) ) )
		{
			continue;
		}

		// AI cannot find a path to the node.

		if( !g_pAIPathMgrNavMesh->HasPath( pAI, pAI->GetCharTypeMask(), pNode->GetNodeContainingNMPoly() ) )
		{
			continue;
		}

		// Record facts that match query.

		lstNodeFacts.push_back( pFact );
	}

	// No matches were found.

	if( lstNodeFacts.empty() )
	{
		return NULL;
	}

	// Pick a random node fact.

	int iRandom = GetRandom( 0, lstNodeFacts.size() - 1 );
	return lstNodeFacts[iRandom];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemory::FindFactNodeMaxNearerThreat
//
//	PURPOSE:	Return a node fact with the maximum confidence,
//				that is closer to the threat.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAIWorkingMemory::FindFactNodeMaxNearerThreat( CAI* pAI, EnumAINodeType eNodeType, HOBJECT hExclude, HOBJECT hThreat )
{
	// This query is inly valid if a threat is provided.

	if( !hThreat )
	{
		return NULL;
	}

	CAIWMFact* pFact;
	AINode* pNode;

	float fMaxConfidence = 0.f;
	CAIWMFact* pFactMax = NULL;

	float fThreatDistSqr;
	LTVector vPosThreat;
	g_pLTServer->GetObjectPos( hThreat, &vPosThreat );
	fThreatDistSqr = vPosThreat.DistSqr( pAI->GetPosition() );

	// Iterate over all facts, searching for the node fact with
	// the highest positional confidence.

	AIWORKING_MEMORY_FACT_LIST::iterator itFact;
	for( itFact = m_lstWMFacts.begin(); itFact != m_lstWMFacts.end(); ++itFact )
	{
		// Ignore deleted facts.

		pFact = *itFact;
		if( pFact->IsDeleted() )
		{
			continue;
		}

		// Ignore facts that are not node facts.
		// Ignore node facts with the wrong node type.
		// Ignore node facts whose object matches the hExclude.
		// Ignore node facts whose pos confidence is lower than the max so far.

		if( ( pFact->GetFactType() != kFact_Node ) ||
			( pFact->GetNodeType() != eNodeType ) ||
			( pFact->GetTargetObject() == hExclude ) ||
			( pFact->GetConfidence( CAIWMFact::kFactMask_Position ) <= fMaxConfidence ) )
		{
			continue;
		}

		// Ignore nodes that are locked, disabled, or timed out.

		pNode = (AINode*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
		if( ( !pNode ) ||
			( pNode->IsLockedDisabledOrTimedOut( pAI->GetHOBJECT() ) ) )
		{
			continue;
		}

		// Ignore nodes that are further from the threat.

		if( vPosThreat.DistSqr( pNode->GetPos() ) > fThreatDistSqr )
		{
			continue;
		}

		// If AI is aware of a threat, ignore nodes that are invalid.

		if( hThreat && 
			( !pNode->IsNodeValid( pAI, pAI->GetPosition(), hThreat, kThreatPos_TargetPos, kNodeStatus_All ) ) )
		{
			continue;
		}

		// AI cannot find a path to the node.

		if( !g_pAIPathMgrNavMesh->HasPath( pAI, pAI->GetCharTypeMask(), pNode->GetNodeContainingNMPoly() ) )
		{
			continue;
		}

		// Keep track of the max pos confidence.

		fMaxConfidence = pFact->GetConfidence( CAIWMFact::kFactMask_Position );
		pFactMax = pFact;
	}

	// Return the max.

	return pFactMax;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemory::FindFactDisturbanceMax
//
//	PURPOSE:	Return the most alarming disturbance fact.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAIWorkingMemory::FindFactDisturbanceMax()
{
	CAIWMFact* pFact;

	// Remove any disturbance facts with NULL handles, as these sources
	// have been removed.

	CAIWMFact factRemoveQuery;
	factRemoveQuery.SetFactType( kFact_Disturbance );
	factRemoveQuery.SetTargetObject( NULL );
	ClearWMFacts( factRemoveQuery );


	int nMaxAlarmLevel = 0;
	float fMaxSimulusConfidence = 0.f;
	int iMaxStimulusID = 0;
	CAIWMFact* pFactMax = NULL;

	// Iterate over all facts, searching for the disturbance fact with
	// the highest stimulus confidence, and full positional confidence.

	AIWORKING_MEMORY_FACT_LIST::iterator itFact;
	for( itFact = m_lstWMFacts.begin(); itFact != m_lstWMFacts.end(); ++itFact )
	{
		// Ignore deleted facts.

		pFact = *itFact;
		if( pFact->IsDeleted() )
		{
			continue;
		}

		// Ignore facts that are not disturbance facts.
		// Ignore disturbance facts whose pos confidence is less than 1.0.
		// Keep the most recent, most alarming stimulus.

		if( ( pFact->GetFactType() != kFact_Disturbance ) ||
			( pFact->GetConfidence( CAIWMFact::kFactMask_Position ) < 1.f ) )
		{
			continue;
		}

		// Ignore facts with reaction delays, who have update times in the future.

		if( pFact->GetUpdateTime() > g_pLTServer->GetTime() )
		{
			continue;
		}

		// Keep track of the max alarm level.

		EnumAIStimulusType eStimulusType = kStim_InvalidType;
		EnumAIStimulusID eStimulusID = kStimID_Invalid;
		pFact->GetStimulus(&eStimulusType, &eStimulusID);
		AIDB_StimulusRecord* pStimulus = g_pAIDB->GetAIStimulusRecord( eStimulusType );
		if (!pStimulus)
		{
			continue;
		}

		if (nMaxAlarmLevel > pStimulus->nAlarmLevel)
		{
			continue;
		}

		// If the alarm level is the same, prefer the fact with the greatest stimulus 
		// confidence

		if ( (nMaxAlarmLevel == pStimulus->nAlarmLevel) 
			&& (pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus ) < fMaxSimulusConfidence ) )
		{
			continue;
		}


		// If the alarm level and the confidence are the same, prefer the most recent.
		// This prevents an AI from being 'blocked' by a disturbance they can't react to;
		// a newer stimulus will likely come along soon and pull in their attention.

		if ( (nMaxAlarmLevel == pStimulus->nAlarmLevel) 
			&& (pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus ) == fMaxSimulusConfidence ) 
			&& (eStimulusID < iMaxStimulusID) )
		{
			continue;
		}

		nMaxAlarmLevel = pStimulus->nAlarmLevel;
		fMaxSimulusConfidence = pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus );
		iMaxStimulusID = eStimulusID;

		pFactMax = pFact;
	}

	// Return the max.

	return pFactMax;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemory::CollectFactsUnupdated
//
//	PURPOSE:	Returns by parameter a list of facts of the requested type
//				which were not updated this frame.
//
// ----------------------------------------------------------------------- //

void CAIWorkingMemory::CollectFactsUnupdated(ENUM_AIWMFACT_TYPE eFactType, AIWORKING_MEMORY_FACT_LIST* pOutFactList, double fComparsionTime)
{
	CAIWMFact* pFact;
	AIWORKING_MEMORY_FACT_LIST::iterator itFact;
	for( itFact = m_lstWMFacts.begin(); itFact != m_lstWMFacts.end(); ++itFact )
	{
		// Ignore deleted facts.

		pFact = *itFact;
		if( pFact->IsDeleted() )
		{
			continue;
		}

		// Ignore facts that are not the correct type.

		if ( pFact->GetFactType() != eFactType )
		{
			continue;
		}

		// Ignore facts that have been updated this frame.

		if ( pFact->GetUpdateTime() >= fComparsionTime )
		{
			continue;
		}

		// Ignore facts that have been destimulated this frame.

		if ( pFact->GetStimulationDecreaseTime() >= fComparsionTime )
		{
			continue;
		}

		pOutFactList->push_back(pFact);
	}
}

