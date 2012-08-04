// ----------------------------------------------------------------------- //
//
// MODULE  : EliteShockTrooper.cpp
//
// PURPOSE : EliteShockTrooper - Implementation
//
// CREATED : 7/24/98
//
// ----------------------------------------------------------------------- //

#include "EliteShockTrooper.h"

BEGIN_CLASS(EliteShockTrooper)
	ADD_LONGINTPROP( WeaponId, GUN_ASSAULTRIFLE_ID )
	ADD_STRINGPROP_FLAG( Filename, GetModel(MI_AI_ESTROOPER_ID), PF_DIMS | PF_HIDDEN )
	PROP_DEFINEGROUP(AvailableStates, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StatePanicked, 0, PF_GROUP3 | PF_HIDDEN)
END_CLASS_DEFAULT( EliteShockTrooper, BaseAI, NULL, NULL )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EliteShockTrooper::EliteShockTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

EliteShockTrooper::EliteShockTrooper() : BaseAI()
{
	m_nModelId	= MI_AI_ESTROOPER_ID;
	m_nWeaponId	= GUN_ASSAULTRIFLE_ID;
}


BEGIN_CLASS(CMC_EliteShockTrooper)
END_CLASS_DEFAULT(CMC_EliteShockTrooper, EliteShockTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_EliteShockTrooper::CMC_EliteShockTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_EliteShockTrooper::CMC_EliteShockTrooper() : EliteShockTrooper()
{
	m_cc = CMC;
}


BEGIN_CLASS(SHOGO_EliteShockTrooper)
END_CLASS_DEFAULT(SHOGO_EliteShockTrooper, EliteShockTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SHOGO_EliteShockTrooper::SHOGO_EliteShockTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_EliteShockTrooper::SHOGO_EliteShockTrooper() : EliteShockTrooper()
{
	m_cc = SHOGO;
}


BEGIN_CLASS(UCA_EliteShockTrooper)
END_CLASS_DEFAULT(UCA_EliteShockTrooper, EliteShockTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_EliteShockTrooper::UCA_EliteShockTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_EliteShockTrooper::UCA_EliteShockTrooper() : EliteShockTrooper()
{
	m_cc = UCA;
}


BEGIN_CLASS(FALLEN_EliteShockTrooper)
END_CLASS_DEFAULT(FALLEN_EliteShockTrooper, EliteShockTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_EliteShockTrooper::FALLEN_EliteShockTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_EliteShockTrooper::FALLEN_EliteShockTrooper() : EliteShockTrooper()
{
	m_cc = FALLEN;
}


BEGIN_CLASS(CRONIAN_EliteShockTrooper)
END_CLASS_DEFAULT(CRONIAN_EliteShockTrooper, EliteShockTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_EliteShockTrooper::CRONIAN_EliteShockTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_EliteShockTrooper::CRONIAN_EliteShockTrooper() : EliteShockTrooper()
{
	m_cc = CRONIAN;
}


BEGIN_CLASS(ROGUE_EliteShockTrooper)
END_CLASS_DEFAULT(ROGUE_EliteShockTrooper, EliteShockTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ROGUE_EliteShockTrooper::ROGUE_EliteShockTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ROGUE_EliteShockTrooper::ROGUE_EliteShockTrooper() : EliteShockTrooper()
{
	m_cc = ROGUE;
}


BEGIN_CLASS(UCA_BAD_EliteShockTrooper)
END_CLASS_DEFAULT(UCA_BAD_EliteShockTrooper, EliteShockTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_BAD_EliteShockTrooper::UCA_BAD_EliteShockTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_BAD_EliteShockTrooper::UCA_BAD_EliteShockTrooper() : EliteShockTrooper()
{
	m_cc = UCA_BAD;
}

