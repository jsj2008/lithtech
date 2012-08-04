// ----------------------------------------------------------------------- //
//
// MODULE  : ZipHook.cpp
//
// PURPOSE : Implementation of the ZipHook object
//
// CREATED : 03/7/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ZipHook.h"

BEGIN_CLASS(ZipHook)

	// Override base-class properties...
	ADD_REALPROP_FLAG(HitPoints, 100.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(MaxHitPoints, 100.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(Armor, 0.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(MaxArmor, 0.0f, PF_GROUP1 | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanHeal, LTFALSE, PF_GROUP1 | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanRepair, LTFALSE, PF_GROUP1 | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanDamage, LTFALSE, PF_GROUP1 | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(NeverDestroy, LTTRUE, PF_GROUP1 | PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Filename, "Props\\Models\\ZipHook.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Props\\Skins\\ZipHook.dtx", PF_FILENAME)

	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_SOLID_FLAG(1, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(RayHit, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(MoveToFloor, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Additive, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Multiply, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(ZipHook, Prop, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ZipHook::ZipHook()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

ZipHook::ZipHook() : Prop()
{
	m_dwUsrFlgs	|= (USRFLG_GLOW | USRFLG_GADGET_ZIPCORD);

	m_pDebrisOverride = "Metal small";
}
