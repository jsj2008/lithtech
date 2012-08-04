// ----------------------------------------------------------------------- //
//
// MODULE  : Trooper.cpp
//
// PURPOSE : Trooper - Implementation
//
// CREATED : 10/10/97
//
// ----------------------------------------------------------------------- //

#include "Trooper.h"

BEGIN_CLASS(Trooper)
	ADD_LONGINTPROP(WeaponId, GUN_MAC10_ID)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_TROOPER_ID), PF_DIMS | PF_HIDDEN )
END_CLASS_DEFAULT(Trooper, BaseAI, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trooper::Trooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Trooper::Trooper() : BaseAI()
{
	m_nModelId	= MI_AI_TROOPER_ID;
	m_nWeaponId	= GUN_MAC10_ID;
}


BEGIN_CLASS(CMC_Trooper)
END_CLASS_DEFAULT(CMC_Trooper, Trooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Trooper::CMC_Trooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_Trooper::CMC_Trooper() : Trooper()
{
	m_cc = CMC;
}


BEGIN_CLASS(SHOGO_Trooper)
END_CLASS_DEFAULT(SHOGO_Trooper, Trooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SHOGO_Trooper::SHOGO_Trooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_Trooper::SHOGO_Trooper() : Trooper()
{
	m_cc = SHOGO;
}


BEGIN_CLASS(UCA_Trooper)
END_CLASS_DEFAULT(UCA_Trooper, Trooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Trooper::UCA_Trooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Trooper::UCA_Trooper() : Trooper()
{
	m_cc = UCA;
}


BEGIN_CLASS(FALLEN_Trooper)
END_CLASS_DEFAULT(FALLEN_Trooper, Trooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Trooper::FALLEN_Trooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Trooper::FALLEN_Trooper() : Trooper()
{
	m_cc = FALLEN;
}


BEGIN_CLASS(CRONIAN_Trooper)
END_CLASS_DEFAULT(CRONIAN_Trooper, Trooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_Trooper::CRONIAN_Trooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_Trooper::CRONIAN_Trooper() : Trooper()
{
	m_cc = CRONIAN;
}


BEGIN_CLASS(STRAGGLER_Trooper)
END_CLASS_DEFAULT(STRAGGLER_Trooper, Trooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	STRAGGLER_Trooper::STRAGGLER_Trooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

STRAGGLER_Trooper::STRAGGLER_Trooper() : Trooper()
{
	m_cc = STRAGGLER;
}

BEGIN_CLASS(UCA_BAD_Trooper)
END_CLASS_DEFAULT(UCA_BAD_Trooper, Trooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_BAD_Trooper::UCA_BAD_Trooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_BAD_Trooper::UCA_BAD_Trooper() : Trooper()
{
	m_cc = UCA_BAD;
}
