// ----------------------------------------------------------------------- //
//
// MODULE  : Ruin150.cpp
//
// PURPOSE : Ruin150 - Implementation
//
// CREATED : 1/29/98
//
// ----------------------------------------------------------------------- //

#include "Ruin150.h"

BEGIN_CLASS(Ruin150)
	ADD_LONGINTPROP( State, Vehicle::DEFENSIVE )
	ADD_LONGINTPROP( WeaponId, GUN_JUGGERNAUT_ID )
	ADD_STRINGPROP_FLAG( Filename, GetModel(MI_AI_RUIN150_ID), PF_DIMS | PF_HIDDEN )
END_CLASS_DEFAULT( Ruin150, Vehicle, NULL, NULL )

static char* s_pTurretNodeNames[] =
{
	"rocketnode_1",
	"rocketnode_2"
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Ruin150::Ruin150()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Ruin150::Ruin150() : Vehicle()
{
	m_nModelId		    = MI_AI_RUIN150_ID;
	m_bIsMecha			= DTRUE;
	m_nWeaponId			= GUN_JUGGERNAUT_ID;

	m_pIdleSound = "Sounds\\Enemies\\Vehicle\\Ruin150\\Idle.wav";
	m_pRunSound	 = "Sounds\\Enemies\\Vehicle\\Ruin150\\Run.wav";
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Ruin150::GetTurretFireNodeName()
//
//	PURPOSE:	Get the turret's offset
//
// ----------------------------------------------------------------------- //

char* Ruin150::GetTurretFireNodeName()
{
	return s_pTurretNodeNames[GetRandom(0,1)];
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Ruin150::SetWeaponDamageFactor()
//
//	PURPOSE:	Set how much (of the base damage) our weapon does
//
// ----------------------------------------------------------------------- //

void Ruin150::SetWeaponDamageFactor(CWeapon* pWeapon)
{
	if (!pWeapon) return;

	// Since we're using the juggernaut, scale back it's damage a bit...

	DFLOAT fDamageFactor = pWeapon->GetDamageFactor();
	if (fDamageFactor > 0.0f)
	{
		pWeapon->SetDamageFactor(fDamageFactor/5.0f);
	}
}


BEGIN_CLASS(CMC_Ruin150)
END_CLASS_DEFAULT(CMC_Ruin150, Ruin150, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Ruin150::CMC_Ruin150()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_Ruin150::CMC_Ruin150() : Ruin150()
{
	m_cc = CMC;
}


BEGIN_CLASS(FALLEN_Ruin150)
END_CLASS_DEFAULT(FALLEN_Ruin150, Ruin150, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Ruin150::FALLEN_Ruin150()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Ruin150::FALLEN_Ruin150() : Ruin150()
{
	m_cc = FALLEN;
}


BEGIN_CLASS(CRONIAN_Ruin150)
END_CLASS_DEFAULT(CRONIAN_Ruin150, Ruin150, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_Ruin150::CRONIAN_Ruin150()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_Ruin150::CRONIAN_Ruin150() : Ruin150()
{
	m_cc = CRONIAN;
}


BEGIN_CLASS(SHOGO_Ruin150)
END_CLASS_DEFAULT(SHOGO_Ruin150, Ruin150, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SHOGO_Ruin150::SHOGO_Ruin150()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_Ruin150::SHOGO_Ruin150() : Ruin150()
{
	m_cc = SHOGO;
}


BEGIN_CLASS(UCA_Ruin150)
END_CLASS_DEFAULT(UCA_Ruin150, Ruin150, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Ruin150::UCA_Ruin150()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Ruin150::UCA_Ruin150() : Ruin150()
{
	m_cc = UCA;
}
