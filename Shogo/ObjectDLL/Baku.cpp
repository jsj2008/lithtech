// ----------------------------------------------------------------------- //
//
// MODULE  : Baku.cpp
//
// PURPOSE : Baku - Implementation
//
// CREATED : 3/19/98
//
// ----------------------------------------------------------------------- //

#include "Baku.h"

BEGIN_CLASS(Baku)
	ADD_LONGINTPROP(WeaponId, GUN_SHOTGUN_ID)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_BAKU_ID), PF_DIMS | PF_HIDDEN)
END_CLASS_DEFAULT(Baku, MajorCharacter, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Baku::Baku()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Baku::Baku() : MajorCharacter()
{
	m_nModelId	 = MI_AI_BAKU_ID;
	m_bIsMecha	 = DFALSE;
	m_nWeaponId	 = GUN_SHOTGUN_ID;
	m_cc		 = FALLEN;

	m_fDimsScale[MS_NORMAL] = 1.0f;
	m_fDimsScale[MS_SMALL]  = 1.0f;
	m_fDimsScale[MS_LARGE]  = 1.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Baku::PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void Baku::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	if (m_eModelSize == MS_SMALL)
	{
		m_bIsMecha	= DFALSE;
		m_cc		= UCA;
		m_nWeaponId	= GUN_NONE;
	}
	else if (m_eModelSize == MS_LARGE)
	{
		m_bIsMecha	= DTRUE;
		m_cc		= FALLEN;
		m_nWeaponId	= GUN_JUGGERNAUT_ID;
	}

	BaseAI::PostPropRead(pStruct);
}
