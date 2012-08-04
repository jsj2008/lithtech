// ----------------------------------------------------------------------- //
//
// MODULE  : EliteTrooper.cpp
//
// PURPOSE : EliteTrooper - Implementation
//
// CREATED : 7/24/98
//
// ----------------------------------------------------------------------- //

#include "EliteTrooper.h"

BEGIN_CLASS(EliteTrooper)
	ADD_LONGINTPROP(WeaponId, GUN_ASSAULTRIFLE_ID)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_ETROOPER_ID), PF_DIMS | PF_HIDDEN )
END_CLASS_DEFAULT(EliteTrooper, BaseAI, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EliteTrooper::EliteTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

EliteTrooper::EliteTrooper() : BaseAI()
{
	m_nModelId	= MI_AI_ETROOPER_ID;
	m_nWeaponId	= GUN_ASSAULTRIFLE_ID;
	m_fDimsScale[0] = 1.15f;
}


BEGIN_CLASS(CMC_EliteTrooper)
END_CLASS_DEFAULT(CMC_EliteTrooper, EliteTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_EliteTrooper::CMC_EliteTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_EliteTrooper::CMC_EliteTrooper() : EliteTrooper()
{
	m_cc = CMC;
}


BEGIN_CLASS(SHOGO_EliteTrooper)
END_CLASS_DEFAULT(SHOGO_EliteTrooper, EliteTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SHOGO_EliteTrooper::SHOGO_EliteTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_EliteTrooper::SHOGO_EliteTrooper() : EliteTrooper()
{
	m_cc = SHOGO;
}


BEGIN_CLASS(UCA_EliteTrooper)
END_CLASS_DEFAULT(UCA_EliteTrooper, EliteTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_EliteTrooper::UCA_EliteTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_EliteTrooper::UCA_EliteTrooper() : EliteTrooper()
{
	m_cc = UCA;
}


BEGIN_CLASS(FALLEN_EliteTrooper)
END_CLASS_DEFAULT(FALLEN_EliteTrooper, EliteTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_EliteTrooper::FALLEN_EliteTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_EliteTrooper::FALLEN_EliteTrooper() : EliteTrooper()
{
	m_cc = FALLEN;
}


BEGIN_CLASS(CRONIAN_EliteTrooper)
END_CLASS_DEFAULT(CRONIAN_EliteTrooper, EliteTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_EliteTrooper::CRONIAN_EliteTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_EliteTrooper::CRONIAN_EliteTrooper() : EliteTrooper()
{
	m_cc = CRONIAN;
}


BEGIN_CLASS(UCA_BAD_EliteTrooper)
END_CLASS_DEFAULT(UCA_BAD_EliteTrooper, EliteTrooper, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_BAD_EliteTrooper::UCA_BAD_EliteTrooper()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_BAD_EliteTrooper::UCA_BAD_EliteTrooper() : EliteTrooper()
{
	m_cc = UCA_BAD;
}
