// ----------------------------------------------------------------------- //
//
// MODULE  : Admiral.cpp
//
// PURPOSE : Admiral - Implementation
//
// CREATED : 3/19/98
//
// ----------------------------------------------------------------------- //

#include "Admiral.h"

BEGIN_CLASS(Admiral)
	ADD_LONGINTPROP(WeaponId, GUN_NONE)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_ADMIRAL_ID), PF_DIMS | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Small, 0, PF_HIDDEN)
END_CLASS_DEFAULT(Admiral, MajorCharacter, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Admiral::Admiral()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Admiral::Admiral() : MajorCharacter()
{
	m_nModelId	 = MI_AI_ADMIRAL_ID;
	m_bIsMecha	 = DFALSE;
	m_nWeaponId	 = GUN_NONE;
	m_cc		 = UCA;
}


BEGIN_CLASS(UCA_BAD_Admiral)
END_CLASS_DEFAULT(UCA_BAD_Admiral, Admiral, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_BAD_Admiral::UCA_BAD_Admiral()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_BAD_Admiral::UCA_BAD_Admiral() : Admiral()
{
	m_cc = UCA_BAD;
}