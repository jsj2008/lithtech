// ----------------------------------------------------------------------- //
//
// MODULE  : Samantha.cpp
//
// PURPOSE : Samantha - Implementation
//
// CREATED : 5/17/98
//
// ----------------------------------------------------------------------- //

#include "Samantha.h"

BEGIN_CLASS(Samantha)
	ADD_LONGINTPROP(WeaponId, GUN_JUGGERNAUT_ID)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_SAMANTHA_ID), PF_DIMS | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Small, 0, PF_HIDDEN)
END_CLASS_DEFAULT(Samantha, MajorCharacter, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Samantha::Samantha()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Samantha::Samantha() : MajorCharacter()
{
	m_nModelId	 = MI_AI_SAMANTHA_ID;
	m_bIsMecha	 = DTRUE;
	m_nWeaponId	 = GUN_JUGGERNAUT_ID;
	m_cc		 = CRONIAN;
}
