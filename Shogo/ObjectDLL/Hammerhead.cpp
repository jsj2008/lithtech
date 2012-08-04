// ----------------------------------------------------------------------- //
//
// MODULE  : HammerHead.cpp
//
// PURPOSE : HammerHead Vehicle - Implementation
//
// CREATED : 5/19/98
//
// ----------------------------------------------------------------------- //

#include "HammerHead.h"

BEGIN_CLASS(HammerHead)
	ADD_LONGINTPROP(WeaponId, GUN_ENERGYGRENADE_ID)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_HAMMERHEAD_ID), PF_DIMS | PF_HIDDEN)
END_CLASS_DEFAULT(HammerHead, Vehicle, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HammerHead::HammerHead()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

HammerHead::HammerHead() : Vehicle()
{
	m_nModelId	= MI_AI_HAMMERHEAD_ID;
	m_nWeaponId	= GUN_ENERGYGRENADE_ID;

	m_pIdleSound = "Sounds\\Enemies\\Vehicle\\HammerHead\\Idle.wav";
	m_pRunSound	 = "Sounds\\Enemies\\Vehicle\\HammerHead\\Run.wav";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HammerHead::GetTurretFireNodeName()
//
//	PURPOSE:	Get the turret's offset
//
// ----------------------------------------------------------------------- //

char* HammerHead::GetTurretFireNodeName()
{
	return "rocketnode_1";
}


BEGIN_CLASS(UCA_HammerHead)
END_CLASS_DEFAULT(UCA_HammerHead, HammerHead, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_HammerHead::UCA_HammerHead()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_HammerHead::UCA_HammerHead() : HammerHead()
{
	m_cc = UCA;
}

BEGIN_CLASS(CMC_HammerHead)
END_CLASS_DEFAULT(CMC_HammerHead, HammerHead, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_HammerHead::CMC_HammerHead()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_HammerHead::CMC_HammerHead() : HammerHead()
{
	m_cc = CMC;
}

BEGIN_CLASS(FALLEN_HammerHead)
END_CLASS_DEFAULT(FALLEN_HammerHead, HammerHead, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_HammerHead::FALLEN_HammerHead()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_HammerHead::FALLEN_HammerHead() : HammerHead()
{
	m_cc = FALLEN;
}

BEGIN_CLASS(CRONIAN_HammerHead)
END_CLASS_DEFAULT(CRONIAN_HammerHead, HammerHead, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_HammerHead::CRONIAN_HammerHead()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_HammerHead::CRONIAN_HammerHead() : HammerHead()
{
	m_cc = CRONIAN;
}
