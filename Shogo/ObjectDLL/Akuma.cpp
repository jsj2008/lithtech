// ----------------------------------------------------------------------- //
//
// MODULE  : Akuma.cpp
//
// PURPOSE : Akuma - Implementation
//
// CREATED : 1/29/98
//
// ----------------------------------------------------------------------- //

#include "Akuma.h"

BEGIN_CLASS(Akuma)
	ADD_LONGINTPROP( WeaponId, GUN_SNIPERRIFLE_ID )
	ADD_STRINGPROP_FLAG( Filename, GetModel(MI_AI_AKUMA_ID), PF_DIMS | PF_HIDDEN )
	ADD_BOOLPROP_FLAG(Small, 0, PF_HIDDEN)
END_CLASS_DEFAULT( Akuma, BaseAI, NULL, NULL )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Akuma::Akuma()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Akuma::Akuma() : BaseAI()
{
	m_nModelId		= MI_AI_AKUMA_ID;
	m_bIsMecha		= DTRUE;
	m_nWeaponId		= GUN_SNIPERRIFLE_ID;
	m_fDimsScale[0] = 1.2f;
}


BEGIN_CLASS(SHOGO_Akuma)
END_CLASS_DEFAULT(SHOGO_Akuma, Akuma, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Akuma::CMC_Akuma()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_Akuma::SHOGO_Akuma() : Akuma()
{
	m_cc = SHOGO;
}

BEGIN_CLASS(CMC_Akuma)
END_CLASS_DEFAULT(CMC_Akuma, Akuma, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Akuma::CMC_Akuma()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_Akuma::CMC_Akuma() : Akuma()
{
	m_cc = CMC;
}

BEGIN_CLASS(UCA_Akuma)
END_CLASS_DEFAULT(UCA_Akuma, Akuma, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Akuma::UCA_Akuma()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Akuma::UCA_Akuma() : Akuma()
{
	m_cc = UCA;
}

BEGIN_CLASS(FALLEN_Akuma)
END_CLASS_DEFAULT(FALLEN_Akuma, Akuma, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Akuma::FALLEN_Akuma()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Akuma::FALLEN_Akuma() : Akuma()
{
	m_cc = FALLEN;
}

BEGIN_CLASS(CRONIAN_Akuma)
END_CLASS_DEFAULT(CRONIAN_Akuma, Akuma, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_Akuma::CRONIAN_Akuma()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_Akuma::CRONIAN_Akuma() : Akuma()
{
	m_cc = CRONIAN;
}
