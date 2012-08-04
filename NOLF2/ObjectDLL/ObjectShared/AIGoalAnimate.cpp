// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAnimate.cpp
//
// PURPOSE : AIGoalAnimate implementation
//
// CREATED : 7/30/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAnimate.h"
#include "AIHumanState.h"
#include "AIGoalMgr.h"
#include "AI.h"
#include "AIUtils.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAnimate, kGoal_Animate);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAnimate::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAnimate::CAIGoalAnimate()
{
	m_bLoop		= LTFALSE;
	m_hstrAnim	= LTNULL;
	m_bResetAnim= LTTRUE;
}

CAIGoalAnimate::~CAIGoalAnimate()
{
	FREE_HSTRING(m_hstrAnim);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAnimate::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalAnimate::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL( m_bLoop );
	SAVE_HSTRING( m_hstrAnim );
	SAVE_BOOL( m_bResetAnim );
}

void CAIGoalAnimate::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL( m_bLoop );
	LOAD_HSTRING( m_hstrAnim );
	LOAD_BOOL( m_bResetAnim );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAnimate::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAnimate::ActivateGoal()
{
	super::ActivateGoal();

	AIASSERT(m_hstrAnim != LTNULL, m_pAI->m_hObject, "CAIGoalAnimate::ActivateGoal: Anim is NULL.");

	m_pGoalMgr->LockGoal(this);
	m_pAI->SetState( kState_HumanAnimate );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAnimate::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAnimate::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanAnimate:
			HandleStateAnimate();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAnimate::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAnimate::HandleStateAnimate
//
//	PURPOSE:	Determine what to do when in state Animate.
//
// ----------------------------------------------------------------------- //

void CAIGoalAnimate::HandleStateAnimate()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			if(	m_bResetAnim )
			{
				CAIHumanStateAnimate* pStateAnimate = (CAIHumanStateAnimate*)(m_pAI->GetState());
				pStateAnimate->SetAnimation(m_hstrAnim, m_bLoop);
				m_bResetAnim = LTFALSE;
			}
			break;

		case kSStat_StateComplete:
			m_pGoalMgr->UnlockGoal(this);
			m_fCurImportance = 0.f;
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAnimate::HandleStateAnimate: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAnimate::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalAnimate::HandleNameValuePair(const char *szName, const char *szValue)
{
	AIASSERT(szName && szValue, m_pAI->m_hObject, "CAIGoalAnimate::HandleNameValuePair: Missing name or value.");

	if ( !_stricmp(szName, "ANIM") )
	{
		FREE_HSTRING(m_hstrAnim);
		m_hstrAnim = g_pLTServer->CreateString((char*)szValue);
		m_bResetAnim = LTTRUE;
		return LTTRUE;
	}
	else if ( !_stricmp(szName, "LOOP") )
	{
		m_bLoop = IsTrueChar(szValue[0]);
		m_bResetAnim = LTTRUE;
		return LTTRUE;
	}

	return LTFALSE;
}
