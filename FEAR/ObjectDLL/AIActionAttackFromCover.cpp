// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackFromCover.cpp
//
// PURPOSE : AIActionAttackFromCover abstract class implementation
//
// CREATED : 3/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackFromCover.h"
//#include "AIWorldState.h"
#include "AI.h"
//#include "AINode.h"
//#include "AIState.h"
//#include "AITarget.h"
//#include "AIBlackBoard.h"
//#include "AnimationContext.h"
//#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackFromCover, kAct_AttackFromCover );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromCover::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAttackFromCover::CAIActionAttackFromCover()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromCover::SetAttackAnimProps
//
//	PURPOSE:	Set animation props.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackFromCover::SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps )
{
	// Sanity check.

	if( !( pAI && pProps ) )
	{
		return;
	}

	super::SetAttackAnimProps( pAI, pProps );

	// Set the additional anim props for attacking from cover.
	// Go uncovered to fire.

	if( pProps->Get( kAPG_Activity ) == kAP_ATVT_Covered )
	{
		pProps->Set( kAPG_Activity, kAP_ATVT_Uncovered );
		pAI->GetAIWorldState()->SetWSProp( kWSK_CoverStatus, pAI->m_hObject, kWST_EnumAnimProp, kAP_ATVT_Uncovered );
	}
}
