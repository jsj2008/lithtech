// ----------------------------------------------------------------------- //
//
// MODULE  : Sparrowhawk.cpp
//
// PURPOSE : Sparrowhawk - Implementation
//
// CREATED : 5/19/98
//
// ----------------------------------------------------------------------- //

#include "Sparrowhawk.h"

BEGIN_CLASS(Sparrowhawk)
	ADD_LONGINTPROP( State, Vehicle::DEFENSIVE )
	ADD_LONGINTPROP( WeaponId, GUN_JUGGERNAUT_ID )
	ADD_STRINGPROP_FLAG( Filename, GetModel(MI_AI_SPARROWHAWK_ID), PF_DIMS | PF_HIDDEN )
END_CLASS_DEFAULT( Sparrowhawk, Vehicle, NULL, NULL )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Sparrowhawk::Sparrowhawk()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Sparrowhawk::Sparrowhawk() : Vehicle()
{
	m_nModelId = MI_AI_SPARROWHAWK_ID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Sparrowhawk::GetTurretFireNodeName()
//
//	PURPOSE:	Get the turret's offset
//
// ----------------------------------------------------------------------- //

char* Sparrowhawk::GetTurretFireNodeName()
{
	return "rocketnode_1";
}


BEGIN_CLASS(CMC_Sparrowhawk)
END_CLASS_DEFAULT(CMC_Sparrowhawk, Sparrowhawk, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Sparrowhawk::CMC_Sparrowhawk()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_Sparrowhawk::CMC_Sparrowhawk() : Sparrowhawk()
{
	m_cc = CMC;
}


BEGIN_CLASS(FALLEN_Sparrowhawk)
END_CLASS_DEFAULT(FALLEN_Sparrowhawk, Sparrowhawk, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Sparrowhawk::FALLEN_Sparrowhawk()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Sparrowhawk::FALLEN_Sparrowhawk() : Sparrowhawk()
{
	m_cc = FALLEN;
}


BEGIN_CLASS(CRONIAN_Sparrowhawk)
END_CLASS_DEFAULT(CRONIAN_Sparrowhawk, Sparrowhawk, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_Sparrowhawk::CRONIAN_Sparrowhawk()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_Sparrowhawk::CRONIAN_Sparrowhawk() : Sparrowhawk()
{
	m_cc = CRONIAN;
}


BEGIN_CLASS(SHOGO_Sparrowhawk)
END_CLASS_DEFAULT(SHOGO_Sparrowhawk, Sparrowhawk, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SHOGO_Sparrowhawk::SHOGO_Sparrowhawk()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_Sparrowhawk::SHOGO_Sparrowhawk() : Sparrowhawk()
{
	m_cc = SHOGO;
}


BEGIN_CLASS(UCA_Sparrowhawk)
END_CLASS_DEFAULT(UCA_Sparrowhawk, Sparrowhawk, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Sparrowhawk::UCA_Sparrowhawk()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Sparrowhawk::UCA_Sparrowhawk() : Sparrowhawk()
{
	m_cc = UCA;
}
