// ----------------------------------------------------------------------- //
//
// MODULE  : DamageTypes.cpp
//
// PURPOSE : Implementation of damage types
//
// CREATED : 01/11/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DamageTypes.h"

DTINFO DTInfoArray[] =
{
	//		DAMAGE TYPE					FLAG		   NAME					  JAR CAMERA   IS GADGET  PLAYER ACCURACY
	//
	DTINFO(DT_UNSPECIFIED,				(1<<0),		"UNSPECIFIED",				LTFALSE,	LTFALSE,	LTFALSE),
	DTINFO(DT_BLEEDING,					(1<<1),		"BLEEDING",					LTFALSE,	LTFALSE,	LTFALSE),
	DTINFO(DT_BULLET,					(1<<2),		"BULLET",					LTTRUE,		LTFALSE,	LTTRUE),
	DTINFO(DT_BURN,						(1<<3),		"BURN",						LTFALSE,	LTFALSE,	LTFALSE),
	DTINFO(DT_CHOKE,					(1<<4),		"CHOKE",					LTTRUE,		LTFALSE,	LTFALSE),
	DTINFO(DT_CRUSH,					(1<<5),		"CRUSH",					LTFALSE,	LTFALSE,	LTFALSE),
	DTINFO(DT_ELECTROCUTE,				(1<<6),		"ELECTROCUTE",				LTFALSE,	LTFALSE,	LTTRUE),
	DTINFO(DT_EXPLODE,					(1<<7),		"EXPLODE",					LTFALSE,	LTFALSE,	LTFALSE),
	DTINFO(DT_FREEZE,					(1<<8),		"FREEZE",					LTFALSE,	LTFALSE,	LTFALSE),
	DTINFO(DT_POISON,					(1<<9),		"POISON",					LTFALSE,	LTFALSE,	LTFALSE),
	DTINFO(DT_ENDLESS_FALL,				(1<<10),	"ENDLESS FALL",				LTFALSE,	LTFALSE,	LTFALSE),
	DTINFO(DT_SLEEPING,					(1<<11),	"SLEEPING",					LTFALSE,	LTFALSE,	LTFALSE),
	DTINFO(DT_STUN,						(1<<12),	"STUN",						LTFALSE,	LTFALSE,	LTFALSE),
	DTINFO(DT_MELEE,					(1<<13),	"MELEE",					LTFALSE,	LTFALSE,	LTFALSE),
	DTINFO(DT_GADGET_CAMERA_DISABLER,	(1<<14),	"GADGET CAMERA DISABLER",	LTFALSE,	LTTRUE,	LTFALSE),
	DTINFO(DT_GADGET_CODE_DECIPHERER,	(1<<15),	"GADGET CODE DECIPHERER",	LTFALSE,	LTTRUE,	LTFALSE),
	DTINFO(DT_GADGET_POODLE,			(1<<16),	"GADGET POODLE",			LTFALSE,	LTTRUE,	LTFALSE),
	DTINFO(DT_GADGET_LOCK_PICK,			(1<<17),	"GADGET LOCK PICK",			LTFALSE,	LTTRUE,	LTFALSE),
	DTINFO(DT_GADGET_WELDER,			(1<<18),	"GADGET WELDER",			LTFALSE,	LTTRUE,	LTFALSE),
	DTINFO(DT_GADGET_LIGHTER,			(1<<19),	"GADGET LIGHTER",			LTFALSE,	LTTRUE,	LTFALSE),
	DTINFO(DT_GADGET_CAMERA,			(1<<20),	"GADGET CAMERA",			LTFALSE,	LTTRUE,	LTFALSE),
	DTINFO(DT_GADGET_MINE_DETECTOR,		(1<<21),	"GADGET MINE DETECTOR",		LTFALSE,	LTTRUE,	LTFALSE),
	DTINFO(DT_GADGET_INFRA_RED,			(1<<22),	"GADGET INFRA RED",			LTFALSE,	LTTRUE,	LTFALSE),
	DTINFO(DT_GADGET_ZIPCORD,			(1<<23),	"GADGET ZIPCORD",			LTFALSE,	LTTRUE,	LTFALSE),
	DTINFO(DT_GADGET_DECAYPOWDER,		(1<<24),	"GADGET DECAY POWDER",		LTFALSE,	LTTRUE,	LTFALSE),

	// DT_INVALID must be the last entry...lots of bad things will happen if this isn't the last entry.
	DTINFO(DT_INVALID,					0,			"INVALID",					LTFALSE,	LTFALSE)
};

const int c_nDTInfoArraySize = sizeof(DTInfoArray) / sizeof(DTInfoArray[0]);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeToFlag()
//
//	PURPOSE:	Convert the damage type to its flag equivillent
//
// ----------------------------------------------------------------------- //

uint32 DamageTypeToFlag(DamageType eType)
{
	for (int i=0; DTInfoArray[i].eType != DT_INVALID; i++)
	{
		if (DTInfoArray[i].eType == eType)
		{
			return DTInfoArray[i].dwFlag;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeToString()
//
//	PURPOSE:	Convert the damage type to its string equivillent
//
// ----------------------------------------------------------------------- //

const char* DamageTypeToString(DamageType eType)
{
	for (int i=0; DTInfoArray[i].eType != DT_INVALID; i++)
	{
		if (DTInfoArray[i].eType == eType)
		{
			return DTInfoArray[i].pName;
		}
	}

	return "INVALID";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeToString()
//
//	PURPOSE:	Convert the damage type to its string equivillent
//
// ----------------------------------------------------------------------- //

DamageType StringToDamageType(const char* pDTName)
{
	for (int i=0; DTInfoArray[i].eType != DT_INVALID; i++)
	{
		if (_stricmp(DTInfoArray[i].pName, pDTName) == 0)
		{
			return DTInfoArray[i].eType;
		}
	}

	return DT_INVALID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsJarCameraType()
//
//	PURPOSE:	Does this damage type cause the camera to be "jarred"
//
// ----------------------------------------------------------------------- //

LTBOOL IsJarCameraType(DamageType eType)
{
	for (int i=0; DTInfoArray[i].eType != DT_INVALID; i++)
	{
		if (DTInfoArray[i].eType == eType)
		{
			return DTInfoArray[i].bJarCamera;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsGadgetType()
//
//	PURPOSE:	Is this damage type a gadget damage type?
//
// ----------------------------------------------------------------------- //

LTBOOL IsGadgetType(DamageType eType)
{
	for (int i=0; DTInfoArray[i].eType != DT_INVALID; i++)
	{
		if (DTInfoArray[i].eType == eType)
		{
			return DTInfoArray[i].bGadget;
		}
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsAccuracyType()
//
//	PURPOSE:	Does this damage type count towards player accuracy
//
// ----------------------------------------------------------------------- //

LTBOOL IsAccuracyType(DamageType eType)
{
	for (int i=0; DTInfoArray[i].eType != DT_INVALID; i++)
	{
		if (DTInfoArray[i].eType == eType)
		{
			return DTInfoArray[i].bAccuracy;
		}
	}

	return LTFALSE;
}