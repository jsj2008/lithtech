// ----------------------------------------------------------------------- //
//
// MODULE  : AIWeaponUtils.cpp
//
// PURPOSE : AIWeaponUtils Declaration
//
// CREATED : 10/14/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIWeaponUtils.h"
#include "AI.h"
#include "AIDB.h"
#include "AIBlackBoard.h"
#include "AITarget.h"
#include "Weapon.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIWeaponUtils::ConvertAIWeaponEnumToString
//
//	PURPOSE:	Converts the passed in AIWeaponType enum to the 
//				corresponding string.  Returns NULL if the WeaponType is 
//				invalid.
//
// ----------------------------------------------------------------------- //

const char* const AIWeaponUtils::ConvertAIWeaponEnumToString(EnumAIWeaponClassType eAIWeaponType)
{
	if (eAIWeaponType < 0 || eAIWeaponType >= kAIWeaponClassType_Count)
	{
		return NULL;
	}

	return s_aszAIWeaponClassNames[eAIWeaponType];
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIWeaponUtils::ConvertAIWeaponStringToEnum
//
//	PURPOSE:	Converts the passed in AIWeaponType string to the 
//				cooresponding enumeration.  If there is no valid 
//				cooresponding enumeration, then kAIWeaponClassType_InvalidType
//				is returned.
//
// ----------------------------------------------------------------------- //

EnumAIWeaponClassType AIWeaponUtils::ConvertAIWeaponStringToEnum(const char* const szAIWeaponType)
{
	if (!szAIWeaponType)
	{
		return kAIWeaponClassType_InvalidType;
	}

	for (int i = 0; i < kAIWeaponClassType_Count; ++i)
	{
		if (0==LTStrICmp(s_aszAIWeaponClassNames[i], szAIWeaponType))
		{
			return (EnumAIWeaponClassType)i;
		}
	}

	return kAIWeaponClassType_InvalidType;
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIWeaponUtils::AI_FACTORY_NEW_AIWEAPON
//
//	PURPOSE:	Create an AIWeapon object of the class cooresponding to 
//				the passed in enumeration.
//
// ----------------------------------------------------------------------- //

CAIWeaponAbstract* AIWeaponUtils::AI_FACTORY_NEW_AIWeapon( EnumAIWeaponClassType eAIWeaponClassID )
{
	// Call AI_FACTORY_NEW for the requested type of AIWEAPON.

	switch( eAIWeaponClassID )
	{
		#define AIWEAPONCLASS_TYPE_AS_SWITCH 1
		#include "AIEnumAIWeaponClassTypeValues.h"
		#undef AIWEAPONCLASS_TYPE_AS_SWITCH

		default: AIASSERT( 0, NULL, "CAIWeaponMgr::AI_FACTORY_NEW_AIWEAPON: Unrecognized AIWeapon type." );
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIWeaponUtils::GetAIWeaponTemplateID
//
//	PURPOSE:	Returns the an enumerated value associated with the passed
//				in HWEAPON.  Returns kAIWeaponTemplateID_Invalid if there is
//				no such template.
//
// ----------------------------------------------------------------------- //

ENUM_AIWeaponID AIWeaponUtils::GetAIWeaponRecordID(HWEAPON hWeapon, ENUM_AIWeaponOverrideSetID eOverrideSet )
{
	// Look for an override.

	AIDB_AIWeaponOverrideSetRecord* pSet = g_pAIDB->GetAIWeaponOverrideSetRecord( eOverrideSet );
	if ( pSet )
	{
		for ( uint32 i = 0; i < pSet->AIWeaponOverrideList.size(); ++i )
		{
			if ( hWeapon == pSet->AIWeaponOverrideList[i].hWeapon )
			{
				return pSet->AIWeaponOverrideList[i].eAIWeapon;
			}
		}
	}

	// Get the default.

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, USE_AI_DATA);
	HRECORD hAIWeapon = g_pWeaponDB->GetRecordLink(hWpnData, WDB_WEAPON_hAIWeaponName );
	const char* const pszAIWeaponName = g_pWeaponDB->GetRecordName( hAIWeapon );
	return g_pAIDB->GetAIWeaponRecordID(pszAIWeaponName);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIWeaponUtils::GetAIWeaponRecord
//
//	PURPOSE:	Returns a pointer to the record cooresponding to the 
//				passed in HWEAPON, or NULL if there is no such AIWeapon
//
// ----------------------------------------------------------------------- //

const AIDB_AIWeaponRecord* AIWeaponUtils::GetAIWeaponRecord( HWEAPON hWeapon, ENUM_AIWeaponOverrideSetID eOverrideSet )
{
	if( !hWeapon )
	{
		return NULL;
	}

	return GetAIWeaponRecord( GetAIWeaponRecordID( hWeapon, eOverrideSet ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIWeaponUtils::GetAIWeaponRecord
//
//	PURPOSE:	Returns a pointer to the record cooresponding to the 
//				passed in RecordID, or NULL if there is no such AIWeapon
//
// ----------------------------------------------------------------------- //

const AIDB_AIWeaponRecord* AIWeaponUtils::GetAIWeaponRecord(ENUM_AIWeaponID eRecord)
{
	return g_pAIDB->GetAIWeaponRecord(eRecord);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIWeaponUtils::GetWeaponRangeStatus
//
//	PURPOSE:	Returns a ENUM_RangeStatus describing the range status 
//				based on the passed in parameters.  If the template is
//				invalid, then kRangeStatus_Invalid is returned.
//
// ----------------------------------------------------------------------- //

ENUM_RangeStatus AIWeaponUtils::GetWeaponRangeStatus(
	const AIDB_AIWeaponRecord* pRecord,
	const LTVector& vSourcePos,
	const LTVector& vDestPos)
{
	if ( !pRecord )
	{
		return kRangeStatus_Invalid;
	}

	float fDistanceSqr = vSourcePos.DistSqr(vDestPos);

	if ( fDistanceSqr < pRecord->fMinRangeSqr )
	{
		return kRangeStatus_TooClose;
	}
	else if ( fDistanceSqr > pRecord->fMaxRangeSqr )
	{
		return kRangeStatus_TooFar;
	}
	else
	{
		return kRangeStatus_Ok;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIWeaponUtils::HasWeaponType
//
//	PURPOSE:	Returns true if the AI is either has this weapon drawn,
//				of if they have it holstered.
//
// ----------------------------------------------------------------------- //

bool AIWeaponUtils::HasWeaponType(CAI* pAI, ENUM_AIWeaponType eType, bool bCheckHolster)
{
	// Already have a weapon of this type drawn.

	if (pAI->GetAIBlackBoard()->GetBBHasWeapon(eType))
	{
		return true;
	}

	// See if our holstered weapon meets the requirements if we are 
	// planning.

	if( bCheckHolster )
	{
		// Right holster.

		const AIDB_AIWeaponRecord* pAIRecord = AIWeaponUtils::GetAIWeaponRecord(
			pAI->GetAIBlackBoard()->GetBBHolsterRightAIWeaponRecordID());
		if (!pAIRecord)
		{
			return false;
		}

		// Holsered weapon is the correct type.

		if (pAIRecord->eAIWeaponType == eType)
		{
			return true;
		}

		// Left holster.

		pAIRecord = AIWeaponUtils::GetAIWeaponRecord(
			pAI->GetAIBlackBoard()->GetBBHolsterLeftAIWeaponRecordID());
		if (!pAIRecord)
		{
			return false;
		}

		// Holsered weapon is the correct type.

		if (pAIRecord->eAIWeaponType == eType)
		{
			return true;
		}
	}

	// Does not have weapon type.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAbstract::IsInRange
//
//	PURPOSE:	Returns true if the target is in range of either a drawn
//				or holstered weapon of the passed in time.  Else returns 
//				false.
//
// ----------------------------------------------------------------------- //

bool AIWeaponUtils::IsInRange(CAI* pAI, ENUM_AIWeaponType eType, bool bCheckHolster)
{
	// Already in range 

	if (pAI->GetAIBlackBoard()->GetBBWeaponStatus(eType) == kRangeStatus_Ok)
	{
		return true;
	}

	// See if our holstered weapon meets the requirements if we are 
	// planning.

	if( bCheckHolster )
	{
		// Right holster.

		const AIDB_AIWeaponRecord* pAIRecord = AIWeaponUtils::GetAIWeaponRecord(
			pAI->GetAIBlackBoard()->GetBBHolsterRightAIWeaponRecordID());
		if (!pAIRecord)
		{
			return false;
		}

		// Target is in range holsered weapons range.

		if (kRangeStatus_Ok == AIWeaponUtils::GetWeaponRangeStatus(pAIRecord, 
			pAI->GetPosition(), pAI->GetAIBlackBoard()->GetBBTargetPosition()))
		{
			return true;
		}

		// Left holster.

		pAIRecord = AIWeaponUtils::GetAIWeaponRecord(
			pAI->GetAIBlackBoard()->GetBBHolsterLeftAIWeaponRecordID());
		if (!pAIRecord)
		{
			return false;
		}

		// Target is in range holstered weapons range.

		if (kRangeStatus_Ok == AIWeaponUtils::GetWeaponRangeStatus(pAIRecord, 
			pAI->GetPosition(), pAI->GetAIBlackBoard()->GetBBTargetPosition()))
		{
			return true;
		}
	}

	// Not in range.

	return false;
}

bool AIWeaponUtils::IsPosInRange(CAI* pAI, const LTVector& vPos, bool bCheckHolster)
{
	// In range of current weapon.

	const AIDB_AIWeaponRecord* pAIRecord = AIWeaponUtils::GetAIWeaponRecord(
		pAI->GetAIBlackBoard()->GetBBCurrentAIWeaponRecordID());

	if (kRangeStatus_Ok == AIWeaponUtils::GetWeaponRangeStatus(pAIRecord, 
		pAI->GetPosition(), vPos))
	{
		return true;
	}

	// See if our holstered weapon meets the requirements if we are 
	// planning.

	if( bCheckHolster )
	{
		// Right holster.

		const AIDB_AIWeaponRecord* pAIRecord = AIWeaponUtils::GetAIWeaponRecord(
			pAI->GetAIBlackBoard()->GetBBHolsterRightAIWeaponRecordID());
		if (!pAIRecord)
		{
			return false;
		}

		// Target is in range holsered weapons range.

		if (kRangeStatus_Ok == AIWeaponUtils::GetWeaponRangeStatus(pAIRecord, 
			pAI->GetPosition(), pAI->GetAIBlackBoard()->GetBBTargetPosition()))
		{
			return true;
		}

		// Left holster.

		pAIRecord = AIWeaponUtils::GetAIWeaponRecord(
			pAI->GetAIBlackBoard()->GetBBHolsterLeftAIWeaponRecordID());
		if (!pAIRecord)
		{
			return false;
		}

		// Target is in range holstered weapons range.

		if (kRangeStatus_Ok == AIWeaponUtils::GetWeaponRangeStatus(pAIRecord, 
			pAI->GetPosition(), pAI->GetAIBlackBoard()->GetBBTargetPosition()))
		{
			return true;
		}
	}

	// Not in range.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAbstract::GetWeaponRange
//
//	PURPOSE:	Returns the range a weapon can fire.
//
// ----------------------------------------------------------------------- //

float AIWeaponUtils::GetWeaponRange(CAI* pAI, ENUM_AIWeaponType eType, bool bCheckHolster)
{
	// Get active weapon template.

	ENUM_AIWeaponID eRecordID;
	const AIDB_AIWeaponRecord* pRecord;
	CWeapon* pWeapon = pAI->GetAIWeaponMgr()->GetWeaponOfType( eType );
	if( pWeapon )
	{
		eRecordID = AIWeaponUtils::GetAIWeaponRecordID( pWeapon->GetWeaponRecord(), pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet() );
		pRecord = GetAIWeaponRecord(eRecordID);
		if (pRecord)
		{
			return pRecord->fMaxRange;
		}
	}

	// Get the holstered weapon template.

	if( bCheckHolster )
	{
		// Right holster.

		eRecordID = pAI->GetAIBlackBoard()->GetBBHolsterRightAIWeaponRecordID();
		pRecord = GetAIWeaponRecord(eRecordID);
		if (pRecord)
		{
			return pRecord->fMaxRange;
		}

		// Left holster.

		eRecordID = pAI->GetAIBlackBoard()->GetBBHolsterLeftAIWeaponRecordID();
		pRecord = GetAIWeaponRecord(eRecordID);
		if (pRecord)
		{
			return pRecord->fMaxRange;
		}
	}

	// No weapon. 

	return 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAbstract::GetWeaponAmmo
//
//	PURPOSE:	Returns the handle to the ammo of the specified kind of weapon.
//
// ----------------------------------------------------------------------- //

HAMMO AIWeaponUtils::GetWeaponAmmo(CAI* pAI, ENUM_AIWeaponType eType, bool bCheckHolster)
{
	// Active weapon.

	CWeapon* pWeapon = pAI->GetAIWeaponMgr()->GetWeaponOfType( eType );
	if( pWeapon )
	{
		return pWeapon->GetAmmoRecord();
	}

	// Check for holstered weapon.

	HWEAPON hWeapon;
	HAMMO hAmmo = NULL;
	if( bCheckHolster )
	{
		// Right holster.

		const AIDB_AIWeaponRecord* pRecord = AIWeaponUtils::GetAIWeaponRecord(
			pAI->GetAIBlackBoard()->GetBBHolsterRightAIWeaponRecordID());
		if( pRecord && pRecord->eAIWeaponType == eType )
		{
			hWeapon = pAI->GetAIBlackBoard()->GetBBHolsterRightWeaponRecord();
			if( hWeapon )
			{
				HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, USE_AI_DATA);
				hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName, 0 ); 
			}
		}

		// Left holster.

		if( !hAmmo )
		{
			const AIDB_AIWeaponRecord* pRecord = AIWeaponUtils::GetAIWeaponRecord(
				pAI->GetAIBlackBoard()->GetBBHolsterLeftAIWeaponRecordID());
			if( pRecord && pRecord->eAIWeaponType == eType )
			{
				hWeapon = pAI->GetAIBlackBoard()->GetBBHolsterLeftWeaponRecord();
				if( hWeapon )
				{
					HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, USE_AI_DATA);
					hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName, 0 ); 
				}
			}
		}
	}

	return hAmmo;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIWeaponUtils::GetBestPrimaryWeapon
//
//	PURPOSE:	Returns the HWEAPON of 'best' weapon to use as 
//                      primary.  This selection is based on the 
//                      nAIWeaponAnimPriority and the presence of ammo 
//                      useable by this weapon.
//
// ----------------------------------------------------------------------- //

HWEAPON AIWeaponUtils::GetBestPrimaryWeapon(CAI* pAI)
{
	HWEAPON hWeapon = NULL;
	int32 nMaxPriority = -1;

	for (int i = 0; i < kAIWeaponType_Count; ++i)
	{
		ENUM_AIWeaponType eType = (ENUM_AIWeaponType)i;
		CWeapon* pWeapon = pAI->GetAIWeaponMgr()->GetWeaponOfType(eType);
		if (pWeapon)
		{
			if (0 != pAI->GetArsenal()->GetAmmoCount(pWeapon->GetAmmoRecord()) || 
				g_pWeaponDB->GetBool( pWeapon->GetWeaponData(), WDB_WEAPON_bInfiniteAmmo ) )
			{
				const AIDB_AIWeaponRecord* pAIWeaponRecord = AIWeaponUtils::GetAIWeaponRecord(pWeapon->GetWeaponRecord(), pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet() );
				if (pAIWeaponRecord && ( pAIWeaponRecord->nAIWeaponAnimPriority > nMaxPriority ))
				{
					hWeapon = pWeapon->GetWeaponRecord();
					nMaxPriority = pAIWeaponRecord->nAIWeaponAnimPriority;
				}
			}
		}
	}

	return hWeapon;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIWeaponUtils::HasAmmo
//
//	PURPOSE:	Returns true if the AI possesses ammo for the 
//                      weapon of the passed in type, or false if the AI 
//                      does not (or if there is no such weapon)
//
// ----------------------------------------------------------------------- //

bool AIWeaponUtils::HasAmmo(CAI* pAI, ENUM_AIWeaponType eType, bool bCheckHolster)
{
	HAMMODATA hAmmoData = NULL;
	HWEAPONDATA hWeaponData = NULL;

	// Check for a drawn weapon of this type.

	CWeapon* pWeapon = pAI->GetAIWeaponMgr()->GetWeaponOfType(eType);
	if (pWeapon)
	{
		hAmmoData = pWeapon->GetAmmoRecord();
		hWeaponData = pWeapon->GetWeaponData();
	}
	else if ( bCheckHolster )
	{
		// Check for a right holstered weapon of this type.

		HWEAPON hWeapon = pAI->GetAIBlackBoard()->GetBBHolsterRightWeaponRecord();
		const AIDB_AIWeaponRecord* pRecord = AIWeaponUtils::GetAIWeaponRecord(hWeapon, pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet());
		if (pRecord && pRecord->eAIWeaponType == eType)
		{
			hWeaponData = g_pWeaponDB->GetWeaponData(hWeapon, USE_AI_DATA);
			hAmmoData = g_pWeaponDB->GetRecordLink( hWeaponData, WDB_WEAPON_rAmmoName, 0 ); 
		}
		else
		{
			HWEAPON hWeapon = pAI->GetAIBlackBoard()->GetBBHolsterLeftWeaponRecord();
			const AIDB_AIWeaponRecord* pRecord = AIWeaponUtils::GetAIWeaponRecord(hWeapon, pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet());
			if (pRecord && pRecord->eAIWeaponType == eType)
			{
				hWeaponData = g_pWeaponDB->GetWeaponData(hWeapon, USE_AI_DATA);
				hAmmoData = g_pWeaponDB->GetRecordLink( hWeaponData, WDB_WEAPON_rAmmoName, 0 ); 
			}
		}
	}

	if ( hAmmoData )
	{
		int nAmmoCount = pAI->GetArsenal()->GetAmmoCount(hAmmoData);
		if ( nAmmoCount > 0 )
		{
			return true;
		}
	}

	if ( hWeaponData )
	{
		bool bInfiniteAmmo = g_pWeaponDB->GetBool( hWeaponData, WDB_WEAPON_bInfiniteAmmo );
		if ( bInfiniteAmmo )
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIWeaponUtils::HasAmmo
//
//	PURPOSE:	Returns true if the AI possesses ammo for the 
//                      weapon or can generate ammo for it. 
//
// ----------------------------------------------------------------------- //

bool AIWeaponUtils::HasAmmo( CAI* pAI, HRECORD hWeapon )
{
	HWEAPONDATA hWeaponData = g_pWeaponDB->GetWeaponData(hWeapon, USE_AI_DATA);
	HAMMODATA hAmmoData = g_pWeaponDB->GetRecordLink( hWeaponData, WDB_WEAPON_rAmmoName, 0 ); 

	return ( (hAmmoData && pAI->GetArsenal()->GetAmmoCount(hAmmoData))
		|| (hWeaponData && g_pWeaponDB->GetBool( hWeaponData, WDB_WEAPON_bInfiniteAmmo ) ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIWeaponUtils::GetWeaponType
//
//	PURPOSE:	Returns the AIWeaponType of the passed in hWeapon
//			or kAIWeaponType_None if there is no such value.
//
// ----------------------------------------------------------------------- //

ENUM_AIWeaponType AIWeaponUtils::GetWeaponType( CAI* pAI, HWEAPON hWeapon )
{
	const AIDB_AIWeaponRecord* pRecord = AIWeaponUtils::GetAIWeaponRecord( hWeapon, pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet() );

	if ( !pRecord )
	{
		return kAIWeaponType_None;
	}

	return pRecord->eAIWeaponType;
}
