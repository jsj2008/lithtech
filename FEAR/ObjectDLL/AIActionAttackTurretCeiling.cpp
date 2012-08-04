// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackTurretCeiling.cpp
//
// PURPOSE : AIActionAttackTurretCeiling class implementation
//
// CREATED : 7/14/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackTurretCeiling.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackTurretCeiling, kAct_AttackTurretCeiling );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackTurretCeiling::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAttackTurretCeiling::CAIActionAttackTurretCeiling()
{
	// Ceiling mounted turrets spin 360 degrees.

	m_dwTrackerFlags = kTrackerFlag_Arm;
	m_bFaceTarget = true;
}
