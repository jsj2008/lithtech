//----------------------------------------------------------------------------
//              
//	MODULE:		AIGoalReclassifyToEnemy.cpp
//              
//	PURPOSE:	- implementation
//              
//	CREATED:	27.12.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#include "AIGoalReclassifyToEnemy.h"		
#include "AIGoalMgr.h"
#include "AI.h"
#include "ObjectRelationMgr.h"
#include "RelationMgr.h"

// Forward declarations

// Globals

// Statics

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalReclassifyToEnemy, kGoal_ReclassifyToEnemy);


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReclassifyToEnemy::ActivateGoal()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void CAIGoalReclassifyToEnemy::ActivateGoal()
{
	CAIGoalAbstractStimulated::ActivateGoal();

	if ( !m_hStimulusSource )
	{
		AIASSERT( 0, m_pAI->m_hObject, "ReclassifyToEnemy on invalid m_hStimulusSource" );
		m_fCurImportance = 0.f;
		return;
	}

	CObjectRelationMgr* pORM = CRelationMgr::GetGlobalRelationMgr()->GetObjectRelationMgr(m_hStimulusSource);
	if ( !pORM )
	{
		AIASSERT1( 0, m_pAI->m_hObject, "ReclassifyToEnemy on unregistered hObject: %s", ::ToString(m_hStimulusSource.operator HOBJECT()));
		m_fCurImportance = 0.f;
		return;
	}

	RelationDescription RD;
	RD.eTrait = RelationTraits::kTrait_Name;
	pORM->GetData().GetTraitValue(RelationTraits::kTrait_Name, RD.szValue, RELATION_VALUE_LENGTH );
	RD.eAlignment = HATE;

	m_pAI->GetRelationMgr()->AddRelation( RD );
	m_pGoalMgr->LockGoal(this);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReclassifyToEnemy::UpdateGoal()
//              
//	PURPOSE:	Intentionally does nothing!
//              
//----------------------------------------------------------------------------
/*virtual*/ void CAIGoalReclassifyToEnemy::UpdateGoal()
{
	m_fCurImportance = 0;
	m_pGoalMgr->UnlockGoal(this);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReclassifyToEnemy::HandleGoalSenseTrigger()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ LTBOOL CAIGoalReclassifyToEnemy::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if (super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		if ( m_hStimulusSource && IsCharacter(m_hStimulusSource) )
		{
			return true;
		}
	}

	return false;
}

