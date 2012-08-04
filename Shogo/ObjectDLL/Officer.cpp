// ----------------------------------------------------------------------- //
//
// MODULE  : Officer.cpp
//
// PURPOSE : Officer - Implementation
//
// CREATED : 7/24/98
//
// ----------------------------------------------------------------------- //

#include "Officer.h"

BEGIN_CLASS(Officer)
	ADD_LONGINTPROP( WeaponId, GUN_COLT45_ID )
	ADD_STRINGPROP_FLAG( Filename, GetModel(MI_AI_OFFICER_ID), PF_DIMS | PF_HIDDEN )
END_CLASS_DEFAULT( Officer, BaseAI, NULL, NULL )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Officer::Officer()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Officer::Officer() : BaseAI()
{
	m_nModelId	= MI_AI_OFFICER_ID;
	m_nWeaponId	= GUN_COLT45_ID;
}


BEGIN_CLASS(CMC_Officer)
END_CLASS_DEFAULT(CMC_Officer, Officer, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Officer::CMC_Officer()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_Officer::CMC_Officer() : Officer()
{
	m_cc = CMC;
}


BEGIN_CLASS(SHOGO_Officer)
END_CLASS_DEFAULT(SHOGO_Officer, Officer, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SHOGO_Officer::SHOGO_Officer()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_Officer::SHOGO_Officer() : Officer()
{
	m_cc = SHOGO;
}


BEGIN_CLASS(UCA_Officer)
END_CLASS_DEFAULT(UCA_Officer, Officer, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Officer::UCA_Officer()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Officer::UCA_Officer() : Officer()
{
	m_cc = UCA;
}


BEGIN_CLASS(FALLEN_Officer)
END_CLASS_DEFAULT(FALLEN_Officer, Officer, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Officer::FALLEN_Officer()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Officer::FALLEN_Officer() : Officer()
{
	m_cc = FALLEN;
}


BEGIN_CLASS(CRONIAN_Officer)
END_CLASS_DEFAULT(CRONIAN_Officer, Officer, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_Officer::CRONIAN_Officer()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_Officer::CRONIAN_Officer() : Officer()
{
	m_cc = CRONIAN;
}


BEGIN_CLASS(ROGUE_Officer)
END_CLASS_DEFAULT(ROGUE_Officer, Officer, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ROGUE_Officer::ROGUE_Officer()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ROGUE_Officer::ROGUE_Officer() : Officer()
{
	m_cc = ROGUE;
}


BEGIN_CLASS(UCA_BAD_Officer)
END_CLASS_DEFAULT(UCA_BAD_Officer, Officer, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_BAD_Officer::UCA_BAD_Officer()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_BAD_Officer::UCA_BAD_Officer() : Officer()
{
	m_cc = UCA_BAD;
}

