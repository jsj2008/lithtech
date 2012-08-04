// ----------------------------------------------------------------------- //
//
// MODULE  : DamageTypes.cpp
//
// PURPOSE : Implementation of damage types
//
// CREATED : 01/11/00
//
// (c) 2000-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "DamageTypes.h"
#include "AnimationContext.h"
#include "AnimationPropStrings.h"

static const char* pDTNames[kNumDamageTypes] =
{
#define INCLUDE_AS_STRING
#include "DamageTypesEnum.h"
#undef INCLUDE_AS_STRING
};


CDamageTypeDB* g_pDTDB = NULL;

const char* const DTDB_DTCat =				"Damage/Types";

const char* const DTDB_bJarCamera =			"JarCamera";
const char* const DTDB_bAccuracy =			"Accuracy";
const char* const DTDB_nSlowMovement =		"SlowMovement";
const char* const DTDB_fMovementMult =		"MovementMultiplier";
const char* const DTDB_bGrief =				"Grief";
const char* const DTDB_bHurtAnim =			"HurtAnim";
const char* const DTDB_sAnimProp =			"AnimProp";
const char* const DTDB_sStimulus =			"Stimulus";
const char* const DTDB_fGibMinDamage =		"GibMinDamage";
const char* const DTDB_fGibMaxDamage =		"GibMaxDamage";
const char* const DTDB_fGibMinChance =		"GibMinChance";
const char* const DTDB_fGibMaxChance =		"GibMaxChance";
const char* const DTDB_bRandomSever =		"RandomSever";
const char* const DTDB_rSlowMoType =		"SlowMoType";

const char* const DTDB_bConcussionAudioEffect = "ConcussionAudioEffect";
const char* const DTDB_fConcussionTuningRadius = "ConcussionTuningRadius";

const char* const DTDB_DamageMasksCat =		"Damage/Masks";
const char* const DTDB_rDamageTypes =		"DamageTypes";

#ifdef _SERVERBUILD
VarTrack g_vtBodyGibTest;
#endif



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDamageTypeDB::CDamageTypeDB()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CDamageTypeDB::CDamageTypeDB()
:	CGameDatabaseMgr( ),
	m_hDTCat(NULL),
	m_hDamageMasksCat( NULL )
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDamageTypeDB::~CDamageTypeDB()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CDamageTypeDB::~CDamageTypeDB()
{
	g_pDTDB = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDamageTypeDB::Init()
//
//	PURPOSE:	Initialize the database...
//
// ----------------------------------------------------------------------- //
bool CDamageTypeDB::Init( const char *szDatabaseFile /* = FDB_Default_File  */ )
{
	if( !OpenDatabase( szDatabaseFile ))
		return false;

	// Set the global database pointer...
	g_pDTDB = this;

	// Get handles to all of the categories in the database...
	m_hDTCat = g_pLTDatabase->GetCategory(m_hDatabase,DTDB_DTCat);

	for (int i = 0; i < kNumDamageTypes; i++)
	{
		m_hDT[i] = g_pLTDatabase->GetRecord(m_hDTCat,pDTNames[i]);
		LTASSERT(m_hDT[i],"Missing damage type record");
		if (!m_hDT[i]) 
			return false;
	}

	m_hDamageMasksCat = g_pLTDatabase->GetCategory( m_hDatabase, DTDB_DamageMasksCat );
    
	SERVER_CODE
	(
		if (g_pLTServer && !g_vtBodyGibTest.IsInitted()) 
		{
			g_vtBodyGibTest.Init(g_pLTServer, "BodyGibTest", NULL, 0.0f);
		}
	)

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDamageTypeDB::GetDTRecord()
//
//	PURPOSE:	Get the DamageTypeRecord
//
// ----------------------------------------------------------------------- //
HRECORD CDamageTypeDB::GetDTRecord(DamageType eType) const
{
	if (eType < 0 || eType >= kNumDamageTypes) return NULL;
	return m_hDT[eType];

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDamageTypeDB::GetDamageType()
//
//	PURPOSE:	Get the DamageType from a record
//
// ----------------------------------------------------------------------- //
DamageType CDamageTypeDB::GetDamageType(HRECORD hDT) const
{
	for (int i = 0; i < kNumDamageTypes; i++)
	{
		if (m_hDT[i] == hDT)
		{
			return static_cast<DamageType>(i);
		}
	}

	return DT_INVALID;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDamageTypeDB::GetDamageFlag()
//
//	PURPOSE:	Get the DamageFlags from a record
//
// ----------------------------------------------------------------------- //
DamageFlags CDamageTypeDB::GetDamageFlag(HRECORD hDT) const
{
	return DamageTypeToFlag(GetDamageType(hDT));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDamageTypeDB::GetDamageMaskFlags()
//
//	PURPOSE:	Get the damage flags for a DamageMask record...
//
// ----------------------------------------------------------------------- //

DamageFlags CDamageTypeDB::GetDamageMaskFlags( const char *szDamageMask ) const
{
	if( !szDamageMask || !szDamageMask[0] )
		return 0;

	// Get the requested damage mask record...
	HRECORD hDamageMask = g_pLTDatabase->GetRecord( m_hDamageMasksCat, szDamageMask );

	return GetDamageMaskFlags( hDamageMask );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDamageTypeDB::GetDamageMaskFlags()
//
//	PURPOSE:	Get the damage flags for a DamageMask record...
//
// ----------------------------------------------------------------------- //

DamageFlags CDamageTypeDB::GetDamageMaskFlags( HRECORD hDamageMask ) const
{
	DamageFlags nDamageFlags = 0;

	if( hDamageMask )
	{
		// Run through the list of damage type records and build the flags...
		uint32 nNumTypes = GetNumValues( hDamageMask, DTDB_rDamageTypes );
		for( uint32 nType = 0; nType < nNumTypes; ++nType )
		{
			HRECORD hDamageType = GetRecordLink( hDamageMask, DTDB_rDamageTypes, nType );
			if( hDamageType )
			{
				nDamageFlags |= GetDamageFlag( hDamageType );
			}
		}
	}

	return nDamageFlags;
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
	DamageFlags nTest = 1;
	for (int i=0; i < kNumDamageTypes; i++)
	{
		if (nDamageFlag == nTest) 
		{
			return DamageType(i);
		}
		nTest = ((DamageFlags)1 << i);
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
	if (eType < 0 || eType >= kNumDamageTypes) 
	{
		return 0;
	}

	return ( ( DamageFlags )1 << ( DamageFlags )eType );
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
	if (eType < 0 || eType >= kNumDamageTypes) 
	{
		return "INVALID";
	}

	return pDTNames[eType];
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
		if (LTStrICmp(pDTNames[i], pDTName) == 0)
		{
			return DamageType(i);
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

bool IsJarCameraType(DamageType eType)
{
	HRECORD hDT = g_pDTDB->GetDTRecord(eType);
	if (!hDT) return false;
	return g_pDTDB->GetBool(hDT,DTDB_bJarCamera,0,false);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsGriefType()
//
//	PURPOSE:	Should the player be protected against multiple instances of 
//					this type in multiplayer.
//
// ----------------------------------------------------------------------- //

bool IsGriefType(DamageType eType)
{
	HRECORD hDT = g_pDTDB->GetDTRecord(eType);
	if (!hDT) return false;
	return g_pDTDB->GetBool(hDT,DTDB_bGrief,0,false);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsRandomSeverType()
//
//	PURPOSE:	Does this damage type cause random severs?
//
// ----------------------------------------------------------------------- //
bool IsRandomSeverType(DamageType eType)
{
	HRECORD hDT = g_pDTDB->GetDTRecord(eType);
	if (!hDT) return false;
	return g_pDTDB->GetBool(hDT,DTDB_bRandomSever,0,false);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetGibChance()
//
//	PURPOSE:	Get the chance of gibbing a body for a given damage type 
//				and amount of damage
//
// ----------------------------------------------------------------------- //
float GetGibChance(DamageType eType, float fDamage)
{
	HRECORD hDT = g_pDTDB->GetDTRecord(eType);
	if (!hDT) return 0.0;

	float fMinChance = g_pDTDB->GetFloat(hDT,DTDB_fGibMinChance,0,0.0f);
	float fMaxChance = g_pDTDB->GetFloat(hDT,DTDB_fGibMaxChance,0,0.0f);
	if (fMaxChance <= 0.0f)
		return 0.0f;

	SERVER_CODE
	(
		if (g_vtBodyGibTest.GetFloat() > 0.0f)
		{
			return 1.0f;
		}
	)

	float fMinDamage = g_pDTDB->GetFloat(hDT,DTDB_fGibMinDamage,0,0.0f);
	float fMaxDamage = g_pDTDB->GetFloat(hDT,DTDB_fGibMaxDamage,0,0.0f);
	if (fMinDamage > fDamage)
		return 0.0f;

	fDamage = LTMIN(fDamage,fMaxDamage);

	float fRange = (fMaxDamage - fMinDamage);
	float fScale = ((fRange <= 0.0f) ? 1.0f :(fDamage-fMinDamage)/fRange );

	return LTCLAMP(	(fMinChance + fScale * (fMaxChance - fMinChance)) , 0.0f, 1.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsAccuracyType()
//
//	PURPOSE:	Does this damage type count towards player accuracy
//
// ----------------------------------------------------------------------- //

bool IsAccuracyType(DamageType eType)
{
	HRECORD hDT = g_pDTDB->GetDTRecord(eType);
	if (!hDT) return false;
	return g_pDTDB->GetBool(hDT,DTDB_bAccuracy,0,false);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsConcussionAudioEffectType()
//
//	PURPOSE:	Does this damage type cause the player to get 'ear-ringing'?
//
// ----------------------------------------------------------------------- //

bool IsConcussionAudioEffectType(DamageType eType)
{
	HRECORD hDT = g_pDTDB->GetDTRecord(eType);
	if (!hDT) return false;
	return g_pDTDB->GetBool(hDT,DTDB_bConcussionAudioEffect,0,false);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetConcussionAudioRadius()
//
//	PURPOSE:	Gets the radius of a level of the effect. 
//
// ----------------------------------------------------------------------- //

float GetConcussionAudioRadius(DamageType eType, int32 nLevel)
{
	HRECORD hDT = g_pDTDB->GetDTRecord(eType);
	if (!hDT) return false;
	HATTRIBUTE hRadius =	g_pDTDB->GetAttribute(hDT,DTDB_fConcussionTuningRadius);
	return g_pLTDatabase->GetFloat(hRadius,nLevel,1);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SlowMovementDuration()
//
//	PURPOSE:	For how long does this damage type slow movement
//
// ----------------------------------------------------------------------- //

float SlowMovementDuration(DamageType eType)
{
	HRECORD hDT = g_pDTDB->GetDTRecord(eType);
	if (!hDT) return false;
	return float(g_pDTDB->GetInt32(hDT,DTDB_nSlowMovement,0,0))/1000.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SlowMovementMultiplier()
//
//	PURPOSE:	How much does this damage type slow movement
//
// ----------------------------------------------------------------------- //

float SlowMovementMultiplier(DamageType eType)
{
	HRECORD hDT = g_pDTDB->GetDTRecord(eType);
	if (!hDT) return false;
	return g_pDTDB->GetFloat(hDT,DTDB_fMovementMult,0,1.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSlowMoType()
//
//	PURPOSE:	What kind of slow mo does this DT trigger
//
// ----------------------------------------------------------------------- //

HRECORD GetSlowMoType(DamageType eType)
{
	HRECORD hDT = g_pDTDB->GetDTRecord(eType);
	if (!hDT) return false;
	return g_pDTDB->GetRecordLink(hDT,DTDB_rSlowMoType,0,NULL);
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
		DamageType eDT = static_cast<DamageType>(i);
		HRECORD hDT = g_pDTDB->GetDTRecord(eDT);
		if( hDT && g_pDTDB->GetBool(hDT,DTDB_bHurtAnim,0,false) )
		{
			damageFlags |= DamageTypeToFlag(eDT);
		}
	}

	return damageFlags;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeToDamageProp()
//
//	PURPOSE:	Return the anim prop associated with this damage type.
//
// ----------------------------------------------------------------------- //

EnumAnimProp DamageTypeToDamageProp(DamageType eType)
{
	HRECORD hDT = g_pDTDB->GetDTRecord(eType);
	if (!hDT) return kAP_Invalid;

	const char* pszAnimProp = g_pDTDB->GetString( hDT, DTDB_sAnimProp );
	return AnimPropUtils::Enum( pszAnimProp );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDamageStimulusName()
//
//	PURPOSE:	Return the stimulus associated with this damage type.
//
// ----------------------------------------------------------------------- //

const char* GetDamageStimulusName(DamageType eType)
{
	HRECORD hDT = g_pDTDB->GetDTRecord(eType);
	if (!hDT) return "";

	// Convert the record name to a stimulus type.

	HRECORD hLink = g_pDTDB->GetRecordLink( hDT, DTDB_sStimulus );
	if( hLink )
	{
		return g_pLTDatabase->GetRecordName( hLink );
	}

	// No stimulus specified.

	return "";
}
