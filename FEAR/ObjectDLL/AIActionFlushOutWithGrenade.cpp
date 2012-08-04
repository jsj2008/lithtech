// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionFlushOutWithGrenade.cpp
//
// PURPOSE : AIActionFlushOutWithGrenade class implementation
//
// CREATED : 01/30/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionFlushOutWithGrenade.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionFlushOutWithGrenade, kAct_AttackFlushOutWithGrenade );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFlushOutWithGrenade::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionFlushOutWithGrenade::CAIActionFlushOutWithGrenade()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFlushOutWithGrenade::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionFlushOutWithGrenade::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set effects.
	// Target is flushed out.

	m_wsWorldStateEffects.ClearWSProp( kWSK_TargetIsDead, NULL );
	m_wsWorldStateEffects.SetWSProp( kWSK_TargetIsFlushedOut, NULL, kWST_bool, true );
}
