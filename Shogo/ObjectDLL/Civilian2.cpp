// ----------------------------------------------------------------------- //
//
// MODULE  : Civilian2.cpp
//
// PURPOSE : Civilian2 - Implementation
//
// CREATED : 1/29/98
//
// ----------------------------------------------------------------------- //

#include "Civilian2.h"

BEGIN_CLASS(Civilian2)
	ADD_LONGINTPROP(WeaponId, GUN_NONE)
	PROP_DEFINEGROUP(AvailableSounds, PF_GROUP1)
		ADD_BOOLPROP_FLAG(SetPanicked, 1, PF_GROUP1)
		ADD_BOOLPROP_FLAG(Panicked, 1, PF_GROUP1)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_CIVILIAN2_ID), PF_DIMS | PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(Evasive, BaseAI::NON_EVASIVE, PF_HIDDEN)
END_CLASS_DEFAULT(Civilian2, BaseAI, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Civilian2::Civilian2()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Civilian2::Civilian2() : BaseAI()
{
	m_nModelId		= MI_AI_CIVILIAN2_ID;
	m_nWeaponId		= GUN_NONE;
	m_eEvasive		= NON_EVASIVE;
}


BEGIN_CLASS(BYSTANDER_Civilian2)
END_CLASS_DEFAULT(BYSTANDER_Civilian2, Civilian2, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BYSTANDER_Civilian2::BYSTANDER_Civilian2()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

BYSTANDER_Civilian2::BYSTANDER_Civilian2() : Civilian2()
{
	m_cc = BYSTANDER;
}


BEGIN_CLASS(ROGUE_Civilian2)
END_CLASS_DEFAULT(ROGUE_Civilian2, Civilian2, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ROGUE_Civilian2::ROGUE_Civilian2()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ROGUE_Civilian2::ROGUE_Civilian2() : Civilian2()
{
	m_cc = ROGUE;
}


BEGIN_CLASS(CMC_Civilian2)
END_CLASS_DEFAULT(CMC_Civilian2, Civilian2, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Civilian2::CMC_Civilian2()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_Civilian2::CMC_Civilian2() : Civilian2()
{
	m_cc = CMC;
}


BEGIN_CLASS(SHOGO_Civilian2)
END_CLASS_DEFAULT(SHOGO_Civilian2, Civilian2, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SHOGO_Civilian2::SHOGO_Civilian2()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_Civilian2::SHOGO_Civilian2() : Civilian2()
{
	m_cc = SHOGO;
}


BEGIN_CLASS(UCA_Civilian2)
END_CLASS_DEFAULT(UCA_Civilian2, Civilian2, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Civilian2::UCA_Civilian2()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Civilian2::UCA_Civilian2() : Civilian2()
{
	m_cc = UCA;
}


BEGIN_CLASS(FALLEN_Civilian2)
END_CLASS_DEFAULT(FALLEN_Civilian2, Civilian2, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Civilian2::FALLEN_Civilian2()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Civilian2::FALLEN_Civilian2() : Civilian2()
{
	m_cc = FALLEN;
}


BEGIN_CLASS(CRONIAN_Civilian2)
END_CLASS_DEFAULT(CRONIAN_Civilian2, Civilian2, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_Civilian2::CRONIAN_Civilian2()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_Civilian2::CRONIAN_Civilian2() : Civilian2()
{
	m_cc = CRONIAN;
}


BEGIN_CLASS(STRAGGLER_Civilian2)
END_CLASS_DEFAULT(STRAGGLER_Civilian2, Civilian2, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	STRAGGLER_Civilian2::STRAGGLER_Civilian2()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

STRAGGLER_Civilian2::STRAGGLER_Civilian2() : Civilian2()
{
	m_cc = STRAGGLER;
}


BEGIN_CLASS(UCA_BAD_Civilian2)
END_CLASS_DEFAULT(UCA_BAD_Civilian2, Civilian2, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_BAD_Civilian2::UCA_BAD_Civilian2()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_BAD_Civilian2::UCA_BAD_Civilian2() : Civilian2()
{
	m_cc = UCA_BAD;
}
