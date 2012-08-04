// ----------------------------------------------------------------------- //
//
// MODULE  : Ryo.cpp
//
// PURPOSE : Ryo - Implementation
//
// CREATED : 3/19/98
//
// ----------------------------------------------------------------------- //

#include "Ryo.h"

BEGIN_CLASS(Ryo)
	ADD_LONGINTPROP(WeaponId, GUN_MAC10_ID)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_RYO_ID), PF_DIMS | PF_HIDDEN)
END_CLASS_DEFAULT(Ryo, MajorCharacter, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Ryo::Ryo()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Ryo::Ryo() : MajorCharacter()
{
	m_nModelId	 = MI_AI_RYO_ID;
	m_bIsMecha	 = DFALSE;
	m_nWeaponId	 = GUN_MAC10_ID;
	m_cc		 = SHOGO;

	m_bMoveToFloor = DFALSE;
}

