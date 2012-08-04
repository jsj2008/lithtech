// ----------------------------------------------------------------------- //
//
// MODULE  : Ordog.cpp
//
// PURPOSE : Ordog - Implementation
//
// CREATED : 1/29/98
//
// ----------------------------------------------------------------------- //

#include "Ordog.h"

BEGIN_CLASS(Ordog)
	ADD_LONGINTPROP( WeaponId, GUN_PULSERIFLE_ID )
	ADD_STRINGPROP_FLAG( Filename, GetModel(MI_AI_ORDOG_ID), PF_DIMS | PF_HIDDEN )
	ADD_BOOLPROP_FLAG(Small, 0, PF_HIDDEN)
END_CLASS_DEFAULT( Ordog, BaseAI, NULL, NULL )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Ordog::Ordog()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Ordog::Ordog() : BaseAI()
{
	m_nModelId	= MI_AI_ORDOG_ID;
	m_bIsMecha  = DTRUE;
	m_nWeaponId	= GUN_PULSERIFLE_ID;
}


BEGIN_CLASS(SHOGO_Ordog)
END_CLASS_DEFAULT(SHOGO_Ordog, Ordog, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Ordog::CMC_Ordog()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_Ordog::SHOGO_Ordog() : Ordog()
{
	m_cc = SHOGO;
}

BEGIN_CLASS(CMC_Ordog)
END_CLASS_DEFAULT(CMC_Ordog, Ordog, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Ordog::CMC_Ordog()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_Ordog::CMC_Ordog() : Ordog()
{
	m_cc = CMC;
}

BEGIN_CLASS(UCA_Ordog)
END_CLASS_DEFAULT(UCA_Ordog, Ordog, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Ordog::UCA_Ordog()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Ordog::UCA_Ordog() : Ordog()
{
	m_cc = UCA;
}

BEGIN_CLASS(UCA_BAD_Ordog)
END_CLASS_DEFAULT(UCA_BAD_Ordog, Ordog, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_BAD_Ordog::UCA_Ordog()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_BAD_Ordog::UCA_BAD_Ordog() : Ordog()
{
	m_cc = UCA_BAD;
}

BEGIN_CLASS(FALLEN_Ordog)
END_CLASS_DEFAULT(FALLEN_Ordog, Ordog, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Ordog::FALLEN_Ordog()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Ordog::FALLEN_Ordog() : Ordog()
{
	m_cc = FALLEN;
}

BEGIN_CLASS(CRONIAN_Ordog)
END_CLASS_DEFAULT(CRONIAN_Ordog, Ordog, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_Ordog::CRONIAN_Ordog()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_Ordog::CRONIAN_Ordog() : Ordog()
{
	m_cc = CRONIAN;
}
