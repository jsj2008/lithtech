// ----------------------------------------------------------------------- //
//
// MODULE  : DoorKnob.cpp
//
// PURPOSE : Implementation of the DoorKnob object
//
// CREATED : 03/23/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DoorKnob.h"

LINKFROM_MODULE( DoorKnob );

#pragma force_active on
BEGIN_CLASS(DoorKnob)

	// Override base-class properties...
	ADD_REALPROP_FLAG(HitPoints, 100.0f, PF_GROUP(1) | PF_HIDDEN)
	ADD_REALPROP_FLAG(MaxHitPoints, 100.0f, PF_GROUP(1) | PF_HIDDEN)
	ADD_REALPROP_FLAG(Armor, 0.0f, PF_GROUP(1) | PF_HIDDEN)
	ADD_REALPROP_FLAG(MaxArmor, 0.0f, PF_GROUP(1) | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanHeal, LTFALSE, PF_GROUP(1) | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanRepair, LTFALSE, PF_GROUP(1) | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanDamage, LTFALSE, PF_GROUP(1) | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(NeverDestroy, LTTRUE, PF_GROUP(1) | PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Filename, "Props\\Models\\DoorKnob_01R.ltb", PF_DIMS | PF_LOCALDIMS | PF_FILENAME | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, "Props\\Skins\\DoorKnob_01b.dtx", PF_FILENAME)

	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(MoveToFloor, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Additive, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Multiply, LTFALSE, PF_HIDDEN)
	
	ADD_BOOLPROP_FLAG(CanTransition, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(DoorKnob, Prop, NULL, NULL)
#pragma force_active off

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoorKnob::DoorKnob()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

DoorKnob::DoorKnob() : Prop()
{
	m_dwUsrFlgs |= USRFLG_CAN_ACTIVATE;

	m_pDebrisOverride = "Metal small";
	
	// Do not allow gadget targets to transition...

	DestroyTransitionAggregate();
}