// ----------------------------------------------------------------------- //
//
// MODULE  : Vandal.cpp
//
// PURPOSE : Vandal Vehicle - Implementation
//
// CREATED : 5/19/98
//
// ----------------------------------------------------------------------- //

#include "Vandal.h"

BEGIN_CLASS(Vandal)
	ADD_LONGINTPROP(WeaponId, GUN_ASSAULTRIFLE_ID)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_VANDAL_ID), PF_DIMS | PF_HIDDEN)
END_CLASS_DEFAULT(Vandal, Vehicle, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vandal::Vandal()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Vandal::Vandal() : Vehicle()
{
	m_nModelId	= MI_AI_VANDAL_ID;
	m_nWeaponId	= GUN_ASSAULTRIFLE_ID;

	m_pIdleSound = "Sounds\\Enemies\\Vehicle\\Vandal\\Idle.wav";
	m_pRunSound	 = "Sounds\\Enemies\\Vehicle\\Vandal\\Run.wav";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vandal::GetTurretFireNodeName()
//
//	PURPOSE:	Get the turret's offset
//
// ----------------------------------------------------------------------- //

char* Vandal::GetTurretFireNodeName()
{
	return "machinegunnode_1";
}

BEGIN_CLASS(UCA_Vandal)
END_CLASS_DEFAULT(UCA_Vandal, Vandal, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Vandal::UCA_Vandal()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Vandal::UCA_Vandal() : Vandal()
{
	m_cc = UCA;
}

BEGIN_CLASS(CMC_Vandal)
END_CLASS_DEFAULT(CMC_Vandal, Vandal, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Vandal::CMC_Vandal()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_Vandal::CMC_Vandal() : Vandal()
{
	m_cc = CMC;
}

BEGIN_CLASS(FALLEN_Vandal)
END_CLASS_DEFAULT(FALLEN_Vandal, Vandal, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Vandal::FALLEN_Vandal()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Vandal::FALLEN_Vandal() : Vandal()
{
	m_cc = FALLEN;
}

BEGIN_CLASS(CRONIAN_Vandal)
END_CLASS_DEFAULT(CRONIAN_Vandal, Vandal, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_Vandal::CRONIAN_Vandal()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_Vandal::CRONIAN_Vandal() : Vandal()
{
	m_cc = CRONIAN;
}
