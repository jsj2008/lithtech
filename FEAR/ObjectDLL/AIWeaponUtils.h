// ----------------------------------------------------------------------- //
//
// MODULE  : AIWeaponUtils.h
//
// PURPOSE : AIWeaponUtils Declaration
//
// CREATED : 10/14/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef AIWEAPONUTILS_H
#define AIWEAPONUTILS_H

#include "AnimationProp.h"
#include "AIEnumAIWeaponTypes.h"
#include "AIDB.h"

// Forward Declarations:

class	CAIWeaponAbstract;
class	CAI;
struct	AIDB_AIWeaponRecord;

//
// CLASS: AIWeaponUtils Helper functions for working with AIWeapon and
//			related structures.  Currently this is a general mess of 
//			functions to avoid coupling or code duplication for often 
//			needed functionality.
//
class AIWeaponUtils
{
public:
	static EnumAIWeaponClassType ConvertAIWeaponStringToEnum(const char* const szAIWeaponType);
	static const char* const ConvertAIWeaponEnumToString(EnumAIWeaponClassType eAIWeaponType);

	static const AIDB_AIWeaponRecord*	GetAIWeaponRecord(ENUM_AIWeaponID);
	static const AIDB_AIWeaponRecord*	GetAIWeaponRecord(HWEAPON hWeapon, ENUM_AIWeaponOverrideSetID eOverrideSet);
	static ENUM_AIWeaponID				GetAIWeaponRecordID(HWEAPON hWeapon, ENUM_AIWeaponOverrideSetID eOverrideSet );

	static ENUM_RangeStatus GetWeaponRangeStatus( const AIDB_AIWeaponRecord* pRecord, const LTVector& vSourcePos, const LTVector& vDestPos);
	static bool				HasWeaponType(CAI* pAI, ENUM_AIWeaponType eType, bool bCheckHolster);
	static bool				IsInRange(CAI* pAI, ENUM_AIWeaponType eType, bool bCheckHolster);
	static bool				IsPosInRange(CAI* pAI, const LTVector& vPos, bool bCheckHolster);
	static float			GetWeaponRange(CAI* pAI, ENUM_AIWeaponType eType, bool bCheckHolster);
	static HAMMO			GetWeaponAmmo(CAI* pAI, ENUM_AIWeaponType eType, bool bCheckHolster);
	static HWEAPON			GetBestPrimaryWeapon(CAI* pAI);

	static bool				HasAmmo(CAI* pAI, ENUM_AIWeaponType eType, bool bCheckHolster);
	static bool				HasAmmo(CAI* pAI, HRECORD hWeapon);

	static ENUM_AIWeaponType GetWeaponType(CAI* pAI, HWEAPON hWeapon);

	// Factory CAIWeaponAbstract derived class creation method:
	static CAIWeaponAbstract* AI_FACTORY_NEW_AIWeapon( EnumAIWeaponClassType );
};

#endif // AIWEAPONUTILS_H
