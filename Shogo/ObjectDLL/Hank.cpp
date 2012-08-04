// ----------------------------------------------------------------------- //
//
// MODULE  : Hank.cpp
//
// PURPOSE : Hank - Implementation
//
// CREATED : 3/19/98
//
// ----------------------------------------------------------------------- //

#include "Hank.h"

BEGIN_CLASS(Hank)
	ADD_LONGINTPROP(WeaponId, GUN_SHOTGUN_ID)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_HANK_ID), PF_DIMS | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Small, 0, PF_HIDDEN)
END_CLASS_DEFAULT(Hank, MajorCharacter, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Hank::Hank()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Hank::Hank() : MajorCharacter()
{
	m_nModelId	 = MI_AI_HANK_ID;
	m_bIsMecha	 = DFALSE;
	m_nWeaponId	 = GUN_ASSAULTRIFLE_ID;
	m_cc		 = UCA;

	m_fWalkVel = 100.0f;
	m_fRunVel = 200.0f;
}
