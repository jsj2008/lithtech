// ----------------------------------------------------------------------- //
//
// MODULE  : Toshiro.cpp
//
// PURPOSE : Toshiro - Implementation
//
// CREATED : 3/19/98
//
// ----------------------------------------------------------------------- //

#include "Toshiro.h"

BEGIN_CLASS(Toshiro)
	ADD_LONGINTPROP(WeaponId, GUN_NONE)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_TOSHIRO_ID), PF_DIMS | PF_HIDDEN)
END_CLASS_DEFAULT(Toshiro, MajorCharacter, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Toshiro::Toshiro()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Toshiro::Toshiro() : MajorCharacter()
{
	m_nModelId   = MI_AI_TOSHIRO_ID;
	m_bIsMecha	 = DFALSE;
	m_nWeaponId	 = GUN_NONE;

	m_fDimsScale[MS_NORMAL] = 1.0f;
	m_fDimsScale[MS_SMALL]  = 1.0f;
	m_fDimsScale[MS_LARGE]  = 1.0f;
}


BEGIN_CLASS(UCA_Toshiro)
END_CLASS_DEFAULT(UCA_Toshiro, Toshiro, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Toshiro::UCA_Toshiro()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Toshiro::UCA_Toshiro() : Toshiro()
{
	m_cc = UCA;
}


BEGIN_CLASS(FALLEN_Toshiro)
END_CLASS_DEFAULT(FALLEN_Toshiro, Toshiro, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Toshiro::FALLEN_Toshiro()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Toshiro::FALLEN_Toshiro() : Toshiro()
{
	m_cc = FALLEN;
}
