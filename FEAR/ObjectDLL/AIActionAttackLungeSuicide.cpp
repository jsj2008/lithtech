// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackLungeSuicide.cpp
//
// PURPOSE : AIActionAttackLungeSuicide class implementation
//
// CREATED : 11/01/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackLungeSuicide.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackLungeSuicide, kAct_AttackLungeSuicide );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackLungeSuicide::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackLungeSuicide::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Die instantly.

	g_pCmdMgr->QueueMessage( pAI, pAI, "DESTROY" );
}

