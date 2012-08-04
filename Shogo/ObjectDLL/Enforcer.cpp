// ----------------------------------------------------------------------- //
//
// MODULE  : Enforcer.cpp
//
// PURPOSE : Enforcer - Implementation
//
// CREATED : 1/29/98
//
// ----------------------------------------------------------------------- //

#include "Enforcer.h"

BEGIN_CLASS(Enforcer)
	ADD_LONGINTPROP( WeaponId, GUN_LASERCANNON_ID )
	ADD_STRINGPROP_FLAG( Filename, GetModel(MI_AI_ENFORCER_ID), PF_DIMS | PF_HIDDEN )
	ADD_BOOLPROP_FLAG(Small, 0, PF_HIDDEN)
END_CLASS_DEFAULT( Enforcer, BaseAI, NULL, NULL )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Enforcer::Enforcer()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Enforcer::Enforcer() : BaseAI()
{
	m_nModelId	= MI_AI_ENFORCER_ID;
	m_bIsMecha  = DTRUE;
	m_nWeaponId	= GUN_LASERCANNON_ID;
}


BEGIN_CLASS(SHOGO_Enforcer)
END_CLASS_DEFAULT(SHOGO_Enforcer, Enforcer, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Enforcer::CMC_Enforcer()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_Enforcer::SHOGO_Enforcer() : Enforcer()
{
	m_cc = SHOGO;
}

BEGIN_CLASS(CMC_Enforcer)
END_CLASS_DEFAULT(CMC_Enforcer, Enforcer, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Enforcer::CMC_Enforcer()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_Enforcer::CMC_Enforcer() : Enforcer()
{
	m_cc = CMC;
}

BEGIN_CLASS(UCA_Enforcer)
END_CLASS_DEFAULT(UCA_Enforcer, Enforcer, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Enforcer::UCA_Enforcer()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Enforcer::UCA_Enforcer() : Enforcer()
{
	m_cc = UCA;
}

BEGIN_CLASS(FALLEN_Enforcer)
END_CLASS_DEFAULT(FALLEN_Enforcer, Enforcer, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Enforcer::FALLEN_Enforcer()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Enforcer::FALLEN_Enforcer() : Enforcer()
{
	m_cc = FALLEN;
}

BEGIN_CLASS(CRONIAN_Enforcer)
END_CLASS_DEFAULT(CRONIAN_Enforcer, Enforcer, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_Enforcer::CRONIAN_Enforcer()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_Enforcer::CRONIAN_Enforcer() : Enforcer()
{
	m_cc = CRONIAN;
}
