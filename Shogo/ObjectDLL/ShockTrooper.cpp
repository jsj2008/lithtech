// ----------------------------------------------------------------------- //
//
// MODULE  : ShockTrooper.cpp
//
// PURPOSE : ShockTrooper - Implementation
//
// CREATED : 10/10/97
//
// ----------------------------------------------------------------------- //

#include "ShockTrooper.h"

BEGIN_CLASS(ShockTrooper)
	ADD_LONGINTPROP( WeaponId, GUN_ASSAULTRIFLE_ID )
	ADD_STRINGPROP_FLAG( Filename, GetModel(MI_AI_STROOPER_ID), PF_DIMS | PF_HIDDEN )
	PROP_DEFINEGROUP(AvailableStates, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StatePanicked, 0, PF_GROUP3 | PF_HIDDEN)
END_CLASS_DEFAULT( ShockTrooper, BaseAI, NULL, NULL )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShockTrooper::ShockTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ShockTrooper::ShockTrooper() : BaseAI()
{
	m_nModelId	= MI_AI_STROOPER_ID;
	m_nWeaponId	= GUN_ASSAULTRIFLE_ID;
}


BEGIN_CLASS(CMC_ShockTrooper)
END_CLASS_DEFAULT(CMC_ShockTrooper, ShockTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_ShockTrooper::CMC_ShockTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_ShockTrooper::CMC_ShockTrooper() : ShockTrooper()
{
	m_cc = CMC;
}


BEGIN_CLASS(SHOGO_ShockTrooper)
END_CLASS_DEFAULT(SHOGO_ShockTrooper, ShockTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SHOGO_ShockTrooper::SHOGO_ShockTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_ShockTrooper::SHOGO_ShockTrooper() : ShockTrooper()
{
	m_cc = SHOGO;
}


BEGIN_CLASS(UCA_ShockTrooper)
END_CLASS_DEFAULT(UCA_ShockTrooper, ShockTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_ShockTrooper::UCA_ShockTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_ShockTrooper::UCA_ShockTrooper() : ShockTrooper()
{
	m_cc = UCA;
}


BEGIN_CLASS(FALLEN_ShockTrooper)
END_CLASS_DEFAULT(FALLEN_ShockTrooper, ShockTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_ShockTrooper::FALLEN_ShockTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_ShockTrooper::FALLEN_ShockTrooper() : ShockTrooper()
{
	m_cc = FALLEN;
}


BEGIN_CLASS(CRONIAN_ShockTrooper)
END_CLASS_DEFAULT(CRONIAN_ShockTrooper, ShockTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_ShockTrooper::CRONIAN_ShockTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_ShockTrooper::CRONIAN_ShockTrooper() : ShockTrooper()
{
	m_cc = CRONIAN;
}


BEGIN_CLASS(ROGUE_ShockTrooper)
END_CLASS_DEFAULT(ROGUE_ShockTrooper, ShockTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ROGUE_ShockTrooper::ROGUE_ShockTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ROGUE_ShockTrooper::ROGUE_ShockTrooper() : ShockTrooper()
{
	m_cc = ROGUE;
}


BEGIN_CLASS(UCA_BAD_ShockTrooper)
END_CLASS_DEFAULT(UCA_BAD_ShockTrooper, ShockTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_BAD_ShockTrooper::UCA_BAD_ShockTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_BAD_ShockTrooper::UCA_BAD_ShockTrooper() : ShockTrooper()
{
	m_cc = UCA_BAD;
}

