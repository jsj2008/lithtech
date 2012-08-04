// ----------------------------------------------------------------------- //
//
// MODULE  : Andra5.cpp
//
// PURPOSE : Andra5 Mecha - Implementation
//
// CREATED : 9/30/97
//
// ----------------------------------------------------------------------- //

#include "Andra5.h"


BEGIN_CLASS(Andra5)
	ADD_LONGINTPROP(WeaponId, GUN_PULSERIFLE_ID)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_ANDRA5_ID), PF_DIMS | PF_HIDDEN )
	ADD_BOOLPROP_FLAG(Small, 0, PF_HIDDEN)
END_CLASS_DEFAULT(Andra5, BaseAI, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Andra5::Andra5()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Andra5::Andra5() : BaseAI()
{
	m_nModelId	 = MI_AI_ANDRA5_ID;
	m_bIsMecha	 = DTRUE;
	m_nWeaponId	 = GUN_PULSERIFLE_ID;
}


BEGIN_CLASS(SHOGO_Andra5)
END_CLASS_DEFAULT(SHOGO_Andra5, Andra5, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Andra5::CMC_Andra5()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_Andra5::SHOGO_Andra5() : Andra5()
{
	m_cc = SHOGO;
}

BEGIN_CLASS(CMC_Andra5)
END_CLASS_DEFAULT(CMC_Andra5, Andra5, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Andra5::CMC_Andra5()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_Andra5::CMC_Andra5() : Andra5()
{
	m_cc = CMC;
}

BEGIN_CLASS(UCA_Andra5)
END_CLASS_DEFAULT(UCA_Andra5, Andra5, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Andra5::UCA_Andra5()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Andra5::UCA_Andra5() : Andra5()
{
	m_cc = UCA;
}

BEGIN_CLASS(FALLEN_Andra5)
END_CLASS_DEFAULT(FALLEN_Andra5, Andra5, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Andra5::FALLEN_Andra5()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Andra5::FALLEN_Andra5() : Andra5()
{
	m_cc = FALLEN;
}

BEGIN_CLASS(CRONIAN_Andra5)
END_CLASS_DEFAULT(CRONIAN_Andra5, Andra5, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_Andra5::CRONIAN_Andra5()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_Andra5::CRONIAN_Andra5() : Andra5()
{
	m_cc = CRONIAN;
}
