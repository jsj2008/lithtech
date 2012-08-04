// ----------------------------------------------------------------------- //
//
// MODULE  : DamageTypes.h
//
// PURPOSE : Definition of damage types
//
// CREATED : 11/26/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DAMAGE_TYPES_H__
#define __DAMAGE_TYPES_H__

#include "ltbasedefs.h"
#include "GameDatabaseMgr.h"
#include "AnimationProp.h"
#include <bitset>

typedef uint16 DamageFlags;

enum DamageType
{
#define INCLUDE_AS_ENUM
#include "DamageTypesEnum.h"
#undef INCLUDE_AS_ENUM
	DT_INVALID,
	kNumDamageTypes = DT_INVALID
};

class CDamageTypeDB;
extern CDamageTypeDB* g_pDTDB;

class CDamageTypeDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( CDamageTypeDB );

public :	// Methods...

	bool	Init( const char *szDatabaseFile = DB_Default_File );
	void	Term() {};

	HRECORD		GetDTRecord(DamageType eType) const;
	DamageType	GetDamageType(HRECORD hDT) const;
	DamageFlags GetDamageFlag(HRECORD hDT) const;

	// Get the damage flags for a DamageMask record...
	DamageFlags GetDamageMaskFlags( const char *szDamageMask ) const;
	DamageFlags GetDamageMaskFlags( HRECORD hDamageMask ) const;

	HCATEGORY GetDamageMasksCategory( ) const { return m_hDamageMasksCat; }


private	:	// Members...

	HCATEGORY	m_hDTCat;
	HCATEGORY	m_hDamageMasksCat;

	HRECORD		m_hDT[kNumDamageTypes];

};

bool			IsJarCameraType(DamageType eType);
bool			IsGriefType(DamageType eType);
bool			IsAccuracyType(DamageType eType);
bool			IsConcussionAudioEffectType(DamageType eType);
float			GetConcussionAudioRadius(DamageType eType, int32 nLevel);
float			SlowMovementDuration(DamageType eType);
float			SlowMovementMultiplier(DamageType eType);
HRECORD			GetSlowMoType(DamageType eType);
bool			IsRandomSeverType(DamageType eType);
float			GetGibChance(DamageType eType, float fDamage);

DamageType		DamageFlagToType(DamageFlags nDamageFlag);
DamageFlags		DamageTypeToFlag(DamageType eType);
const char*		DamageTypeToString(DamageType eType);
DamageType		StringToDamageType(const char* pDTName);

DamageFlags			GetDamageFlagsWithHurtAnim( );
EnumAnimProp		DamageTypeToDamageProp(DamageType eType);
const char*			GetDamageStimulusName(DamageType eType);


#endif // __DAMAGE_TYPES_H__
