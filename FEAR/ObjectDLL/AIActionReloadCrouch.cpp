// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionReloadCrouch.cpp
//
// PURPOSE : AIActionReloadCrouch class implementation
//
// CREATED : 6/23/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionReloadCrouch.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionReloadCrouch, kAct_ReloadCrouch );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReloadCrouch::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionReloadCrouch::CAIActionReloadCrouch()
{
	m_ePosture = kAP_POS_Crouch;
}
