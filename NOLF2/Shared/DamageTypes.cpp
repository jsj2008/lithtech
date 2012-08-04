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

DTINFO DTInfoArray[kNumDamageTypes] =
{
#define INCLUDE_AS_STRUCT
#include "DamageTypesEnum.h"
#undef INCLUDE_AS_STRUCT
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDTButeMgr::CDTButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CDTButeMgr::CDTButeMgr()
{

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDTButeMgr::~CDTButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CDTButeMgr::~CDTButeMgr()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDTButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CDTButeMgr::Init(const char* szAttributeFile)
{
    if (!szAttributeFile || !Parse(szAttributeFile)) return LTFALSE;
	std::string sDamageSound = "";
	char szKey[256] = "";

	for (int i = 0; i < kNumDamageTypes; i++)
	{
		DTInfoArray[i].nDamageFlag = ( 1 << ( uint64 )i );

		DTInfoArray[i].bJarCamera = (LTBOOL)m_buteMgr.GetInt(pDTNames[i],"JarCamera",0);
		DTInfoArray[i].bGadget = (LTBOOL)m_buteMgr.GetInt(pDTNames[i],"Gadget",0);
		DTInfoArray[i].bAccuracy = (LTBOOL)m_buteMgr.GetInt(pDTNames[i],"Accuracy",0);
		DTInfoArray[i].fSlowMove = (LTFLOAT)m_buteMgr.GetDouble(pDTNames[i],"SlowMovement",0.0);
		DTInfoArray[i].bScoreTag = (LTBOOL)m_buteMgr.GetInt(pDTNames[i],"ScoreTag",0);
		DTInfoArray[i].bGrief = (LTBOOL)m_buteMgr.GetInt(pDTNames[i],"Grief",0);
		DTInfoArray[i].bHurtAnim = !!m_buteMgr.GetInt(pDTNames[i],"HurtAnim",0);

		// Build the list of damage sounds.
		for( int nDamageSound = 0; ; nDamageSound++ )
		{
			sprintf( szKey, "PlayerDamageSound%d", nDamageSound );
			sDamageSound = m_buteMgr.GetString( pDTNames[i], szKey, "" );
			if( !m_buteMgr.Success( ))
				break;

			DTInfoArray[i].saPlayerDamageSounds.push_back( sDamageSound );
		}
	}
	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDTButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CDTButeMgr::Term()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageFlagToType()
//
//	PURPOSE:	Convert the damage flag to its type equivillent
//
// ----------------------------------------------------------------------- //

DamageType DamageFlagToType(DamageFlags nDamageFlag)
{
	for (int i=0; i < kNumDamageTypes; i++)
	{
		if (DTInfoArray[i].nDamageFlag == nDamageFlag)
		{
			return DTInfoArray[i].eType;
		}
	}

	return DT_INVALID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeToFlag()
//
//	PURPOSE:	Convert the damage type to its flag equivillent
//
// ----------------------------------------------------------------------- //

DamageFlags DamageTypeToFlag(DamageType eType)
{
	for (int i=0; i < kNumDamageTypes; i++)
	{
		if (DTInfoArray[i].eType == eType)
		{
			return DTInfoArray[i].nDamageFlag;
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
	for (int i=0; i < kNumDamageTypes; i++)
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
	for (int i=0; i < kNumDamageTypes; i++)
	{
		// Check if it matches the main name.
		if (_stricmp(DTInfoArray[i].pName, pDTName) == 0)
		{
			return DTInfoArray[i].eType;
		}

		// Check if it matches the alternate name.  This is just
		// used for backwards compatibility with some levels processed
		// with the old names to damagetypes in the VolumeBrush's.
		if( DTInfoArray[i].pAltName && _stricmp( DTInfoArray[i].pAltName, pDTName ) == 0 )
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
	for (int i=0; i < kNumDamageTypes; i++)
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
//	ROUTINE:	IsScoreTagType()
//
//	PURPOSE:	Does this damage type score a tag in deathmatch
//
// ----------------------------------------------------------------------- //

LTBOOL IsScoreTagType(DamageType eType)
{
	for (int i=0; i < kNumDamageTypes; i++)
	{
		if (DTInfoArray[i].eType == eType)
		{
			return DTInfoArray[i].bScoreTag;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsGriefType()
//
//	PURPOSE:	Should the player be protected against multiple instances of 
//					this type in multiplayer.
//
// ----------------------------------------------------------------------- //

LTBOOL IsGriefType(DamageType eType)
{
	for (int i=0; i < kNumDamageTypes; i++)
	{
		if (DTInfoArray[i].eType == eType)
		{
			return DTInfoArray[i].bGrief;
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
	for (int i=0; i < kNumDamageTypes; i++)
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
	for (int i=0; i < kNumDamageTypes; i++)
	{
		if (DTInfoArray[i].eType == eType)
		{
			return DTInfoArray[i].bAccuracy;
		}
	}

	return LTFALSE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SlowMovementDuration()
//
//	PURPOSE:	For how long does this damage type slow movement
//
// ----------------------------------------------------------------------- //

LTFLOAT SlowMovementDuration(DamageType eType)
{
	for (int i=0; i < kNumDamageTypes; i++)
	{
		if (DTInfoArray[i].eType == eType)
		{
			return DTInfoArray[i].fSlowMove;
		}
	}

	return 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDTINFO()
//
//	PURPOSE:	Gets the damagetype structure.
//
// ----------------------------------------------------------------------- //

DTINFO const* GetDTINFO( DamageType eType )
{
	for (int i=0; i < kNumDamageTypes; i++)
	{
		if (DTInfoArray[i].eType == eType)
		{
			return &DTInfoArray[i];
		}
	}

	return NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDamageFlagsWithHurtAnim
//
//	PURPOSE:	Get the list of damageflags that have the hurt flag set.
//
// ----------------------------------------------------------------------- //

DamageFlags GetDamageFlagsWithHurtAnim( )
{
	DamageFlags damageFlags = 0;

	for (int i=0; i < kNumDamageTypes; i++)
	{
		if( DTInfoArray[i].bHurtAnim )
		{
			damageFlags |= DTInfoArray[i].nDamageFlag;
		}
	}

	return damageFlags;
}