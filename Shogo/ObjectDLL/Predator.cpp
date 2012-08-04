// ----------------------------------------------------------------------- //
//
// MODULE  : Predator.cpp
//
// PURPOSE : Predator - Implementation
//
// CREATED : 1/29/98
//
// ----------------------------------------------------------------------- //

#include "Predator.h"

BEGIN_CLASS(Predator)
	ADD_LONGINTPROP( WeaponId, GUN_SHREDDER_ID )
	ADD_STRINGPROP_FLAG( Filename, GetModel(MI_AI_PREDATOR_ID), PF_DIMS | PF_HIDDEN )
	ADD_BOOLPROP_FLAG(Small, 0, PF_HIDDEN)
END_CLASS_DEFAULT( Predator, BaseAI, NULL, NULL )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Predator::Predator()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Predator::Predator() : BaseAI()
{
	m_nModelId		= MI_AI_PREDATOR_ID;
	m_bIsMecha		= DTRUE;
	m_nWeaponId		= GUN_SHREDDER_ID;
	m_fDimsScale[0] = 0.8f;
}


BEGIN_CLASS(SHOGO_Predator)
END_CLASS_DEFAULT(SHOGO_Predator, Predator, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Predator::CMC_Predator()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_Predator::SHOGO_Predator() : Predator()
{
	m_cc = SHOGO;
}

BEGIN_CLASS(CMC_Predator)
END_CLASS_DEFAULT(CMC_Predator, Predator, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Predator::CMC_Predator()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_Predator::CMC_Predator() : Predator()
{
	m_cc = CMC;
}

BEGIN_CLASS(UCA_Predator)
END_CLASS_DEFAULT(UCA_Predator, Predator, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Predator::UCA_Predator()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Predator::UCA_Predator() : Predator()
{
	m_cc = UCA;
}

BEGIN_CLASS(FALLEN_Predator)
END_CLASS_DEFAULT(FALLEN_Predator, Predator, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Predator::FALLEN_Predator()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Predator::FALLEN_Predator() : Predator()
{
	m_cc = FALLEN;
}

BEGIN_CLASS(CRONIAN_Predator)
END_CLASS_DEFAULT(CRONIAN_Predator, Predator, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_Predator::CRONIAN_Predator()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_Predator::CRONIAN_Predator() : Predator()
{
	m_cc = CRONIAN;
}
