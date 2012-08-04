// ----------------------------------------------------------------------- //
//
// MODULE  : Civilian1.cpp
//
// PURPOSE : Civilian1 - Implementation
//
// CREATED : 10/10/97
//
// ----------------------------------------------------------------------- //

#include "Civilian1.h"

BEGIN_CLASS(Civilian1)
	ADD_LONGINTPROP(WeaponId, GUN_NONE)
	PROP_DEFINEGROUP(AvailableSounds, PF_GROUP1)
		ADD_BOOLPROP_FLAG(SetPanicked, 1, PF_GROUP1)
		ADD_BOOLPROP_FLAG(Panicked, 1, PF_GROUP1)

	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_CIVILIAN1_ID), PF_DIMS | PF_HIDDEN)
	ADD_BOOLPROP(UseModelB, DFALSE)
	ADD_LONGINTPROP_FLAG(Evasive, BaseAI::NON_EVASIVE, PF_HIDDEN)
END_CLASS_DEFAULT(Civilian1, BaseAI, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Civilian1::Civilian1()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Civilian1::Civilian1() : BaseAI()
{
	m_nModelId	 = MI_AI_CIVILIAN1_ID;
	m_nWeaponId	 = GUN_NONE;
	m_eEvasive   = NON_EVASIVE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Civilian1::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Civilian1::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == 1.0f || fData == 2.0f)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		default : break;
	}


	return BaseAI::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Civilian1::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL Civilian1::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pData) return DFALSE;

	if (pServerDE->GetPropGeneric("UseModelB", &genProp) == DE_OK)
	{
		m_nModelId = genProp.m_Bool ? MI_AI_CIVILIAN1B_ID : MI_AI_CIVILIAN1_ID;
	}

	return DTRUE;
}


BEGIN_CLASS(BYSTANDER_Civilian1)
END_CLASS_DEFAULT(BYSTANDER_Civilian1, Civilian1, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BYSTANDER_Civilian1::BYSTANDER_Civilian1()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

BYSTANDER_Civilian1::BYSTANDER_Civilian1() : Civilian1()
{
	m_cc = BYSTANDER;
}


BEGIN_CLASS(ROGUE_Civilian1)
END_CLASS_DEFAULT(ROGUE_Civilian1, Civilian1, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ROGUE_Civilian1::ROGUE_Civilian1()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ROGUE_Civilian1::ROGUE_Civilian1() : Civilian1()
{
	m_cc = ROGUE;
}


BEGIN_CLASS(CMC_Civilian1)
END_CLASS_DEFAULT(CMC_Civilian1, Civilian1, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMC_Civilian1::CMC_Civilian1()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMC_Civilian1::CMC_Civilian1() : Civilian1()
{
	m_cc = CMC;
}


BEGIN_CLASS(SHOGO_Civilian1)
END_CLASS_DEFAULT(SHOGO_Civilian1, Civilian1, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SHOGO_Civilian1::SHOGO_Civilian1()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SHOGO_Civilian1::SHOGO_Civilian1() : Civilian1()
{
	m_cc = SHOGO;
}


BEGIN_CLASS(UCA_Civilian1)
END_CLASS_DEFAULT(UCA_Civilian1, Civilian1, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_Civilian1::UCA_Civilian1()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_Civilian1::UCA_Civilian1() : Civilian1()
{
	m_cc = UCA;
}


BEGIN_CLASS(FALLEN_Civilian1)
END_CLASS_DEFAULT(FALLEN_Civilian1, Civilian1, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FALLEN_Civilian1::FALLEN_Civilian1()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FALLEN_Civilian1::FALLEN_Civilian1() : Civilian1()
{
	m_cc = FALLEN;
}


BEGIN_CLASS(CRONIAN_Civilian1)
END_CLASS_DEFAULT(CRONIAN_Civilian1, Civilian1, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRONIAN_Civilian1::CRONIAN_Civilian1()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRONIAN_Civilian1::CRONIAN_Civilian1() : Civilian1()
{
	m_cc = CRONIAN;
}


BEGIN_CLASS(STRAGGLER_Civilian1)
END_CLASS_DEFAULT(STRAGGLER_Civilian1, Civilian1, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	STRAGGLER_Civilian1::STRAGGLER_Civilian1()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

STRAGGLER_Civilian1::STRAGGLER_Civilian1() : Civilian1()
{
	m_cc = STRAGGLER;
}


BEGIN_CLASS(UCA_BAD_Civilian1)
END_CLASS_DEFAULT(UCA_BAD_Civilian1, Civilian1, NULL, NULL)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UCA_BAD_Civilian1::UCA_BAD_Civilian1()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UCA_BAD_Civilian1::UCA_BAD_Civilian1() : Civilian1()
{
	m_cc = UCA_BAD;
}
