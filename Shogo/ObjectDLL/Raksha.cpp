// ----------------------------------------------------------------------- //
//
// MODULE  : Raksha.cpp
//
// PURPOSE : Raksha Mecha - Implementation
//
// CREATED : 10/10/97
//
// ----------------------------------------------------------------------- //

#include "Raksha.h"
	

BEGIN_CLASS(Raksha)
	ADD_LONGINTPROP(WeaponId, GUN_PULSERIFLE_ID)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_RAKSHA_ID), PF_DIMS | PF_HIDDEN )
	ADD_BOOLPROP_FLAG(Small, 0, PF_HIDDEN)
END_CLASS_DEFAULT(Raksha, BaseAI, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Raksha::Raksha()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Raksha::Raksha() : BaseAI()
{
	m_nModelId	 = MI_AI_RAKSHA_ID;
	m_bIsMecha	 = DTRUE;
	m_nWeaponId	 = GUN_PULSERIFLE_ID;
}


BEGIN_CLASS(SHOGO_Raksha)
END_CLASS_DEFAULT(SHOGO_Raksha, Raksha, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Raksha::CMC_Raksha()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_Raksha::SHOGO_Raksha() : Raksha()
{
	m_cc = SHOGO;
}

BEGIN_CLASS(CMC_Raksha)
END_CLASS_DEFAULT(CMC_Raksha, Raksha, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Raksha::CMC_Raksha()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_Raksha::CMC_Raksha() : Raksha()
{
	m_cc = CMC;
}

BEGIN_CLASS(UCA_Raksha)
END_CLASS_DEFAULT(UCA_Raksha, Raksha, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Raksha::UCA_Raksha()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Raksha::UCA_Raksha() : Raksha()
{
	m_cc = UCA;
}

BEGIN_CLASS(FALLEN_Raksha)
END_CLASS_DEFAULT(FALLEN_Raksha, Raksha, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Raksha::FALLEN_Raksha()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Raksha::FALLEN_Raksha() : Raksha()
{
	m_cc = FALLEN;
}

BEGIN_CLASS(CRONIAN_Raksha)
END_CLASS_DEFAULT(CRONIAN_Raksha, Raksha, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_Raksha::CRONIAN_Raksha()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_Raksha::CRONIAN_Raksha() : Raksha()
{
	m_cc = CRONIAN;
}
