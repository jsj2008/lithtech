// ----------------------------------------------------------------------- //
//
// MODULE  : Assassin.cpp
//
// PURPOSE : Assassin - Implementation
//
// CREATED : 10/10/97
//
// ----------------------------------------------------------------------- //

#include "Assassin.h"

BEGIN_CLASS(Assassin)
	ADD_LONGINTPROP(WeaponId, GUN_BULLGUT_ID)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_ASSASSIN_ID), PF_DIMS | PF_HIDDEN )
	ADD_BOOLPROP_FLAG(Small, 0, PF_HIDDEN)
END_CLASS_DEFAULT(Assassin, BaseAI, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Assassin::Assassin()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Assassin::Assassin() : BaseAI()
{
	m_nModelId	 = MI_AI_ASSASSIN_ID;
	m_bIsMecha	 = DTRUE;
	m_nWeaponId	 = GUN_BULLGUT_ID;
}


BEGIN_CLASS(SHOGO_Assassin)
END_CLASS_DEFAULT(SHOGO_Assassin, Assassin, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Assassin::CMC_Assassin()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_Assassin::SHOGO_Assassin() : Assassin()
{
	m_cc = SHOGO;
}

BEGIN_CLASS(CMC_Assassin)
END_CLASS_DEFAULT(CMC_Assassin, Assassin, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Assassin::CMC_Assassin()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_Assassin::CMC_Assassin() : Assassin()
{
	m_cc = CMC;
}

BEGIN_CLASS(UCA_Assassin)
END_CLASS_DEFAULT(UCA_Assassin, Assassin, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Assassin::UCA_Assassin()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Assassin::UCA_Assassin() : Assassin()
{
	m_cc = UCA;
}

BEGIN_CLASS(FALLEN_Assassin)
END_CLASS_DEFAULT(FALLEN_Assassin, Assassin, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Assassin::FALLEN_Assassin()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Assassin::FALLEN_Assassin() : Assassin()
{
	m_cc = FALLEN;
}

BEGIN_CLASS(CRONIAN_Assassin)
END_CLASS_DEFAULT(CRONIAN_Assassin, Assassin, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_Assassin::CRONIAN_Assassin()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_Assassin::CRONIAN_Assassin() : Assassin()
{
	m_cc = CRONIAN;
}
