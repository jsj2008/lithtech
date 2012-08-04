// ----------------------------------------------------------------------- //
//
// MODULE  : Vigilance.cpp
//
// PURPOSE : Vigilance Vehicle - Implementation
//
// CREATED : 5/19/98
//
// ----------------------------------------------------------------------- //

#include "Vigilance.h"

BEGIN_CLASS(Vigilance)
	ADD_LONGINTPROP(WeaponId, GUN_TOW_ID)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_VIGILANCE_ID), PF_DIMS | PF_HIDDEN)
END_CLASS_DEFAULT(Vigilance, Vehicle, NULL, NULL)

static char* s_pTurretNodeNames[] =
{
	"rocketnode_1",
	"rocketnode_2",
	"rocketnode_3",
	"rocketnode_4",
	"rocketnode_5",
	"rocketnode_6"
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vigilance::Vigilance()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Vigilance::Vigilance() : Vehicle()
{
	m_nModelId	= MI_AI_VIGILANCE_ID;
	m_nWeaponId	= GUN_TOW_ID;

	m_pIdleSound = "Sounds\\Enemies\\Vehicle\\Vigilance\\Idle.wav";
	m_pRunSound	 = "Sounds\\Enemies\\Vehicle\\Vigilance\\Run.wav";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vigilance::GetTurretFireNodeName()
//
//	PURPOSE:	Get the turret's offset
//
// ----------------------------------------------------------------------- //

char* Vigilance::GetTurretFireNodeName()
{
	return s_pTurretNodeNames[GetRandom(0, 5)];
}

BEGIN_CLASS(UCA_Vigilance)
END_CLASS_DEFAULT(UCA_Vigilance, Vigilance, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Vigilance::UCA_Vigilance()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Vigilance::UCA_Vigilance() : Vigilance()
{
	m_cc = UCA;
}

BEGIN_CLASS(CMC_Vigilance)
END_CLASS_DEFAULT(CMC_Vigilance, Vigilance, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Vigilance::CMC_Vigilance()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_Vigilance::CMC_Vigilance() : Vigilance()
{
	m_cc = CMC;
}

BEGIN_CLASS(FALLEN_Vigilance)
END_CLASS_DEFAULT(FALLEN_Vigilance, Vigilance, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Vigilance::FALLEN_Vigilance()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Vigilance::FALLEN_Vigilance() : Vigilance()
{
	m_cc = FALLEN;
}

BEGIN_CLASS(CRONIAN_Vigilance)
END_CLASS_DEFAULT(CRONIAN_Vigilance, Vigilance, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_Vigilance::CRONIAN_Vigilance()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_Vigilance::CRONIAN_Vigilance() : Vigilance()
{
	m_cc = CRONIAN;
}
