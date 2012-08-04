// ----------------------------------------------------------------------- //
//
// MODULE  : Sanjuro.cpp
//
// PURPOSE : Sanjuro - Implementation
//
// CREATED : 9/11/98
//
// ----------------------------------------------------------------------- //

#include "Sanjuro.h"

BEGIN_CLASS(Sanjuro)
	ADD_LONGINTPROP(WeaponId, GUN_NONE)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_PLAYER_ONFOOT_ID), PF_DIMS | PF_HIDDEN)
END_CLASS_DEFAULT(Sanjuro, MajorCharacter, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Sanjuro::Sanjuro()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Sanjuro::Sanjuro() : MajorCharacter()
{
	m_nModelId   = MI_PLAYER_ONFOOT_ID;
	m_bIsMecha	 = DFALSE;
	m_nWeaponId	 = GUN_NONE;
	m_cc		 = UCA;
}
