// ----------------------------------------------------------------------- //
//
// MODULE  : AVC.cpp
//
// PURPOSE : AVC - Implementation
//
// CREATED : 5/18/98
//
// ----------------------------------------------------------------------- //

#include "AVC.h"

BEGIN_CLASS(AVC)
	ADD_LONGINTPROP( WeaponId, GUN_LASERCANNON_ID )
	ADD_STRINGPROP_FLAG( Filename, GetModel(MI_AI_ASSASSIN_ID), PF_DIMS | PF_HIDDEN )
	ADD_BOOLPROP_FLAG(Small, 0, PF_HIDDEN)
END_CLASS_DEFAULT( AVC, BaseAI, NULL, NULL )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AVC::AVC()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

AVC::AVC() : BaseAI()
{
	m_nModelId	= MI_AI_AVC_ID;
	m_bIsMecha	= DTRUE;
	m_nWeaponId	= GUN_LASERCANNON_ID;
}


BEGIN_CLASS(CMC_AVC)
END_CLASS_DEFAULT(CMC_AVC, AVC, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_AVC::CMC_AVC()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_AVC::CMC_AVC() : AVC()
{
	m_cc = CMC;
}

BEGIN_CLASS(CRONIAN_AVC)
END_CLASS_DEFAULT(CRONIAN_AVC, AVC, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_AVC::CRONIAN_AVC()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_AVC::CRONIAN_AVC() : AVC()
{
	m_cc = CRONIAN;
}

BEGIN_CLASS(FALLEN_AVC)
END_CLASS_DEFAULT(FALLEN_AVC, AVC, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_AVC::FALLEN_AVC()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_AVC::FALLEN_AVC() : AVC()
{
	m_cc = FALLEN;
}
