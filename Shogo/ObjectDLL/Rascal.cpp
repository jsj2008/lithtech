// ----------------------------------------------------------------------- //
//
// MODULE  : Rascal.cpp
//
// PURPOSE : Rascal Vehicle - Implementation
//
// CREATED : 5/19/98
//
// ----------------------------------------------------------------------- //

#include "Rascal.h"

BEGIN_CLASS(Rascal)
	ADD_LONGINTPROP(WeaponId, GUN_MAC10_ID)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_RASCAL_ID), PF_DIMS | PF_HIDDEN)
END_CLASS_DEFAULT(Rascal, Vehicle, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rascal::Rascal()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Rascal::Rascal() : Vehicle()
{
	m_nModelId	 = MI_AI_RASCAL_ID;
	m_nWeaponId	 = GUN_MAC10_ID;

	m_fWalkVel	= 175.0f;
	m_fRunVel	= 250.0f;

	m_pIdleSound = "Sounds\\Enemies\\Vehicle\\Rascal\\Idle.wav";
	m_pRunSound	 = "Sounds\\Enemies\\Vehicle\\Rascal\\Run.wav";
}


BEGIN_CLASS(UCA_Rascal)
END_CLASS_DEFAULT(UCA_Rascal, Rascal, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Rascal::UCA_Rascal()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Rascal::UCA_Rascal() : Rascal()
{
	m_cc = UCA;
}

BEGIN_CLASS(CMC_Rascal)
END_CLASS_DEFAULT(CMC_Rascal, Rascal, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Rascal::CMC_Rascal()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_Rascal::CMC_Rascal() : Rascal()
{
	m_cc = CMC;
}

BEGIN_CLASS(FALLEN_Rascal)
END_CLASS_DEFAULT(FALLEN_Rascal, Rascal, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Rascal::FALLEN_Rascal()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Rascal::FALLEN_Rascal() : Rascal()
{
	m_cc = FALLEN;
}

BEGIN_CLASS(CRONIAN_Rascal)
END_CLASS_DEFAULT(CRONIAN_Rascal, Rascal, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_Rascal::CRONIAN_Rascal()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_Rascal::CRONIAN_Rascal() : Rascal()
{
	m_cc = CRONIAN;
}
