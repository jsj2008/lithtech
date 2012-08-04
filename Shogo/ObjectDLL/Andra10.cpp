// ----------------------------------------------------------------------- //
//
// MODULE  : Andra10.cpp
//
// PURPOSE : Andra10 Mecha - Implementation
//
// CREATED : 3/19/98
//
// ----------------------------------------------------------------------- //

#include "Andra10.h"


BEGIN_CLASS(Andra10)
	ADD_LONGINTPROP(WeaponId, GUN_PULSERIFLE_ID)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_ANDRA10_ID), PF_DIMS | PF_HIDDEN )
	ADD_BOOLPROP_FLAG(Small, 0, PF_HIDDEN)
END_CLASS_DEFAULT(Andra10, BaseAI, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Andra10::Andra10()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Andra10::Andra10() : BaseAI()
{
	m_nModelId	 = MI_AI_ANDRA10_ID;
	m_bIsMecha	 = DTRUE;
	m_nWeaponId	 = GUN_PULSERIFLE_ID;
}


BEGIN_CLASS(SHOGO_Andra10)
END_CLASS_DEFAULT(SHOGO_Andra10, Andra10, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Andra10::CMC_Andra10()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_Andra10::SHOGO_Andra10() : Andra10()
{
	m_cc = SHOGO;
}

BEGIN_CLASS(CMC_Andra10)
END_CLASS_DEFAULT(CMC_Andra10, Andra10, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Andra10::CMC_Andra10()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_Andra10::CMC_Andra10() : Andra10()
{
	m_cc = CMC;
}

BEGIN_CLASS(UCA_Andra10)
END_CLASS_DEFAULT(UCA_Andra10, Andra10, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Andra10::UCA_Andra10()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Andra10::UCA_Andra10() : Andra10()
{
	m_cc = UCA;
}

BEGIN_CLASS(FALLEN_Andra10)
END_CLASS_DEFAULT(FALLEN_Andra10, Andra10, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Andra10::FALLEN_Andra10()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Andra10::FALLEN_Andra10() : Andra10()
{
	m_cc = FALLEN;
}

BEGIN_CLASS(CRONIAN_Andra10)
END_CLASS_DEFAULT(CRONIAN_Andra10, Andra10, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_Andra10::CRONIAN_Andra10()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_Andra10::CRONIAN_Andra10() : Andra10()
{
	m_cc = CRONIAN;
}
