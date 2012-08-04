// ----------------------------------------------------------------------- //
//
// MODULE  : Tenma.cpp
//
// PURPOSE : Tenma Mecha - Implementation
//
// CREATED : 5/18/98
//
// ----------------------------------------------------------------------- //

#include "Tenma.h"

BEGIN_CLASS(Tenma)
	ADD_LONGINTPROP(WeaponId, GUN_SHREDDER_ID)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_TENMA_ID), PF_DIMS | PF_HIDDEN )
	ADD_BOOLPROP_FLAG(Small, 0, PF_HIDDEN)
END_CLASS_DEFAULT(Tenma, BaseAI, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Tenma::Tenma()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Tenma::Tenma() : BaseAI()
{
	m_nModelId   = MI_AI_TENMA_ID;
	m_bIsMecha	 = DTRUE;
	m_nWeaponId	 = GUN_SHREDDER_ID;
}


BEGIN_CLASS(UCA_Tenma)
END_CLASS_DEFAULT(UCA_Tenma, Tenma, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Tenma::UCA_Tenma()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Tenma::UCA_Tenma() : Tenma()
{
	m_cc = UCA;
}

BEGIN_CLASS(CMC_Tenma)
END_CLASS_DEFAULT(CMC_Tenma, Tenma, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Tenma::CMC_Tenma()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_Tenma::CMC_Tenma() : Tenma()
{
	m_cc = CMC;
}

BEGIN_CLASS(FALLEN_Tenma)
END_CLASS_DEFAULT(FALLEN_Tenma, Tenma, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Tenma::FALLEN_Tenma()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Tenma::FALLEN_Tenma() : Tenma()
{
	m_cc = FALLEN;
}

BEGIN_CLASS(CRONIAN_Tenma)
END_CLASS_DEFAULT(CRONIAN_Tenma, Tenma, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_Tenma::CRONIAN_Tenma()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_Tenma::CRONIAN_Tenma() : Tenma()
{
	m_cc = CRONIAN;
}


BEGIN_CLASS(SHOGO_Tenma)
END_CLASS_DEFAULT(SHOGO_Tenma, Tenma, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SHOGO_Tenma::SHOGO_Tenma()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_Tenma::SHOGO_Tenma() : Tenma()
{
	m_cc = SHOGO;
}
