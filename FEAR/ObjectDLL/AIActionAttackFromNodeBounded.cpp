// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackFromNodeBounded.cpp
//
// PURPOSE : AIActionAttackFromNodeBounded class implementation
//
// CREATED : 6/27/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackFromNodeBounded.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackFromNodeBounded, kAct_AttackFromNodeBounded );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromNodeBounded::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAttackFromNodeBounded::CAIActionAttackFromNodeBounded()
{
	m_dwNodeStatus = kNodeStatus_ThreatOutsideFOV | kNodeStatus_ThreatOutsideBoundary;
}

