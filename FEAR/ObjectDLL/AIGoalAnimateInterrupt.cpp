// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAnimateInterrupt.cpp
//
// PURPOSE : AIGoalAnimateInterrupt class implementation
//
// CREATED : 9/4/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

// Includes required for AIGoalAnimate.h

#include "Stdafx.h"
#include "AIGoalAnimateInterrupt.h"
#include "AIWorkingMemory.h"
#include "AI.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalAnimateInterrupt, kGoal_AnimateInterrupt );

CAIWMFact* CAIGoalAnimateInterrupt::GetAnimateTaskFact()
{
	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Task);
	factQuery.SetTaskType(kTask_AnimateInterrupt);
	return m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
}

CAIWMFact* CAIGoalAnimateInterrupt::GetAnimateLoopTaskFact()
{
	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Task);
	factQuery.SetTaskType(kTask_AnimateInterruptLoop);
	return m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
}
