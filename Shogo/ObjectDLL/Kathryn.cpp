// ----------------------------------------------------------------------- //
//
// MODULE  : Kathryn.cpp
//
// PURPOSE : Kathryn - Implementation
//
// CREATED : 3/19/98
//
// ----------------------------------------------------------------------- //

#include "Kathryn.h"

BEGIN_CLASS(Kathryn)
	ADD_LONGINTPROP(WeaponId, GUN_SHOTGUN_ID)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_KATHRYN_ID), PF_DIMS | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Small, 0, PF_HIDDEN)
END_CLASS_DEFAULT(Kathryn, MajorCharacter, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Kathryn::Kathryn()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Kathryn::Kathryn() : MajorCharacter()
{
	m_nModelId	 = MI_AI_KATHRYN_ID;
	m_bIsMecha	 = DFALSE;
	m_nWeaponId	 = GUN_NONE;
	m_cc		 = BYSTANDER;
}
