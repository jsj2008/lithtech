// ----------------------------------------------------------------------- //
//
// MODULE  : AIDB.cpp
//
// PURPOSE : AI database implementation
//
// CREATED : 03/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "Stdafx.h"
	#include "AIDB.h"
	#include "AIStimulusMgr.h"
	#include "AISensorMgr.h"
	#include "AIWeaponUtils.h"
	#include "AnimationPropStrings.h"
	#include "AIUtils.h"
	#include "AINode.h"
	#include "ServerDB.h"

//
// Defines...
//


//
// Globals...
//

CAIDB* g_pAIDB = NULL;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CAIDB()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CAIDB::CAIDB()
	: CGameDatabaseMgr()
{
	m_cAIActionRecords = 0;
	m_aAIActionRecords = NULL;

	m_cAIActionSetRecords = 0;
	m_aAIActionSetRecords = NULL;

	m_cAIActivitySetRecords = 0;
	m_aAIActivitySetRecords = NULL;

	m_cAIDamageMaskRecords = 0;
	m_aAIDamageMaskRecords = NULL;

	m_cAIBrainRecords = 0;
	m_aAIBrainRecords = NULL;

	m_cAIAttributesRecords = 0;
	m_aAIAttributesRecords = NULL;

	m_cAILimitsRecords = 0;
	m_aAILimitsRecords = NULL;

	m_cAIStimulusRecords = 0;
	m_aAIStimulusRecords = NULL;

	m_cAIStimulusMaskRecords = 0;
	m_aAIStimulusMaskRecords = NULL;

	for( uint32 iAISensor=0; iAISensor < kSensor_Count; ++iAISensor )
	{
		m_aAISensorRecords[iAISensor].eSensorType = kSensor_InvalidType;
	}

	for( uint32 iAIGoal=0; iAIGoal < kGoal_Count; ++iAIGoal )
	{
		m_aAIGoalRecords[iAIGoal].eGoalType = kGoal_InvalidType;
	}

	m_cAITargetSelectRecords = 0;
	m_aAITargetSelectRecords = NULL;

	m_cAITargetSelectSetRecords = 0;
	m_aAITargetSelectSetRecords = NULL;

	m_cAIGoalSetRecords = 0;
	m_aAIGoalSetRecords = NULL;

	m_cAIMovementRecords = 0;
	m_aAIMovementRecords = NULL;

	m_cAIMovementSetRecords = 0;
	m_aAIMovementSetRecords = NULL;

	m_cAISmartObjectRecords = 0;
	m_aAISmartObjectRecords = NULL;

	m_cAIWeaponRecords = 0;
	m_aAIWeaponRecords = NULL;

	m_cAIWeaponOverrideSetRecords = 0;
	m_aAIWeaponOverrideSetRecords = NULL;

	m_cAIAmmoLoadRecords = 0;
	m_aAIAmmoLoadRecords = NULL;

	m_cAINodeRecords = 0;
	m_aAINodeRecords = NULL;

	m_cAIAINavMeshTypeRecords = 0;
	m_aAIAINavMeshTypeRecords = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::~CAIDB()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CAIDB::~CAIDB()
{
	debug_deletea( m_aAIActionRecords );
	debug_deletea( m_aAIActionSetRecords );
	debug_deletea( m_aAIActivitySetRecords );
	debug_deletea( m_aAIDamageMaskRecords );
	debug_deletea( m_aAIBrainRecords );
	debug_deletea( m_aAIAttributesRecords );
	debug_deletea( m_aAILimitsRecords );
	debug_deletea( m_aAIStimulusRecords );
	debug_deletea( m_aAIStimulusMaskRecords );
	debug_deletea( m_aAITargetSelectRecords );
	debug_deletea( m_aAITargetSelectSetRecords );
	debug_deletea( m_aAIGoalSetRecords );
	debug_deletea( m_aAIMovementRecords );
	debug_deletea( m_aAIMovementSetRecords );
	debug_deletea( m_aAISmartObjectRecords );
	debug_deletea( m_aAIWeaponRecords );
	debug_deletea( m_aAIWeaponOverrideSetRecords );
	debug_deletea( m_aAIAmmoLoadRecords );
	debug_deletea( m_aAINodeRecords );
	debug_deletea( m_aAIAINavMeshTypeRecords );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::Init()
//
//	PURPOSE:	Initialize the AI database...
//
// ----------------------------------------------------------------------- //

bool CAIDB::Init( const char *szDatabaseFile /* = DB_Default_File  */ )
{
	if( !OpenDatabase( szDatabaseFile ) )
	{
		return false;
	}

	// Set the global AI database pointer...

	g_pAIDB = this;


	CreateAINavMeshTypeRecords();
	CreateAISmartObjectRecords();
	CreateAIActionRecords();
	CreateAIActionSetRecords();
	CreateAIActivitySetRecords();
	CreateAIDamageMaskRecords();
	CreateAIBrainRecords();
	CreateAIConstantsRecord();
	CreateAILimitsRecords();
	CreateAIAttributesRecords();
	CreateAIStimulusRecords();
	CreateAIStimulusMaskRecords();
	CreateAISensorRecords();
	CreateAITargetRecords();
	CreateAITargetSetRecords();
	CreateAIGoalRecords();
	CreateAIGoalSetRecords();
	CreateAIMovementRecords();
	CreateAIMovementSetRecords();
	CreateAIWeaponRecords();
	CreateAIWeaponOverrideSets();
	CreateAIAmmoLoadRecords();
	CreateAINodeRecords();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAIActionRecords()
//
//	PURPOSE:	Create AIAction records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAIActionRecords()
{
	HCATEGORY hCatAIActions = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_ACTION_CATEGORY );
	if( !hCatAIActions )
	{
		return;
	}

	// Create an array as big as the number of records.

	m_cAIActionRecords = g_pLTDatabase->GetNumRecords( hCatAIActions );
	if( !m_cAIActionRecords )
	{
		return;
	}
	m_aAIActionRecords = debug_newa( AIDB_ActionRecord, m_cAIActionRecords );

	const char* pszName;
	const char* pszClass;
	const char* pszSmartObject;
	uint32 cActionAbility;
	uint32 iActionAbility;
	const char* pszActionAbility;
	ENUM_ActionAbility eActionAbilityType;

	// Read in each record.

	HRECORD hRecord;
	HRECORD hLink;
	HATTRIBUTE hAtt;
	for( uint32 iAIAction=0; iAIAction < m_cAIActionRecords; ++iAIAction )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIActions, iAIAction );
		if( !hRecord )
		{
			continue;
		}

		// Action type.

		AIDB_ActionRecord& Record = m_aAIActionRecords[iAIAction];
		pszName = g_pLTDatabase->GetRecordName( hRecord );
		Record.eActionType = (EnumAIActionType)String2EnumIndex( pszName, kAct_Count, (uint32)kAct_InvalidType, s_aszActionTypes );
		if( Record.eActionType == kAct_InvalidType )
		{
			AIASSERT1( 0, NULL, "CAIDB::CreateAIActionRecords: Unrecognized Action type: %s", pszName );
			continue;
		}

		// Action class.

		pszClass = GetString( hRecord, AIDB_ACTION_sClass );
		Record.eActionClass = (EnumAIActionType)String2EnumIndex( pszClass, kAct_Count, (uint32)kAct_InvalidType, s_aszActionTypes );
		if( Record.eActionClass == kAct_InvalidType )
		{
			AIASSERT1( 0, NULL, "CAIDB::CreateAIActionRecords: Unrecognized Action class: %s", pszClass );
			continue;
		}

		// Node Type

		const char* const pszNodeType = GetString( hRecord, AIDB_ACTION_sNodeType );
		if( !LTStrIEquals( pszNodeType, "None" ) )
		{
			Record.eNodeType = AINodeUtils::GetNodeType( pszNodeType );
		}

		// Awareness

		const char* const pszAwareness = GetString( hRecord, AIDB_ACTION_sAwareness );
		if( !LTStrIEquals( pszAwareness, "None" ) )
		{
			Record.eAwareness = StringToAwareness( pszAwareness );
		}
		

		// Cost.

		Record.fActionCost = GetFloat( hRecord, AIDB_ACTION_fCost );

		// Precedence.

		Record.fActionPrecedence = GetFloat( hRecord, AIDB_ACTION_fPrecedence );

		// Interruptible.

		Record.bActionIsInterruptible = GetBool( hRecord, AIDB_ACTION_bInterruptible );

		// SmartObject.

		hLink = GetRecordLink( hRecord, AIDB_ACTION_sSmartObject );
		if( hLink )
		{
			pszSmartObject = g_pLTDatabase->GetRecordName( hLink );
			Record.eSmartObjectID = GetAISmartObjectRecordID( pszSmartObject );
		}

		// Probability.

		Record.fActionProbability = GetFloat( hRecord, AIDB_ACTION_fProbability );

		// Abilities.

		hAtt = g_pLTDatabase->GetAttribute( hRecord, AIDB_ACTION_sActionAbility );
		cActionAbility = g_pLTDatabase->GetNumValues( hAtt );
		for( iActionAbility=0; iActionAbility < cActionAbility; ++iActionAbility )
		{
			pszActionAbility = GetString( hRecord, AIDB_ACTION_sActionAbility, iActionAbility );
			eActionAbilityType = (ENUM_ActionAbility)String2EnumIndex( pszActionAbility, kActionAbility_Count, (uint32)kActionAbility_InvalidType, s_aszActionAbilityTypes );

			if( eActionAbilityType == kActionAbility_InvalidType )
			{
				if( !LTStrIEquals( pszActionAbility, "None" ) )
				{
					AIASSERT1( 0, NULL, "CAIDB::CreateAIActionRecords: Unrecognized ActionAbility type: %s", pszActionAbility );
				}
				continue;
			}

			// Add ActionAbility to mask.

			Record.dwActionAbilitySet.set( eActionAbilityType, true );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAIActionSetRecords()
//
//	PURPOSE:	Create AIActionSet records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAIActionSetRecords()
{
	HCATEGORY hCatAIActionSets = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_ACTIONSET_CATEGORY );
	if( !hCatAIActionSets )
	{
		return;
	}

	// Create an array as big as the number of records.

	m_cAIActionSetRecords = g_pLTDatabase->GetNumRecords( hCatAIActionSets );
	if( !m_cAIActionSetRecords )
	{
		return;
	}
	m_aAIActionSetRecords = debug_newa( AIDB_ActionSetRecord, m_cAIActionSetRecords );

	uint32 cActions;
	uint32 iAction;
	const char* pszAction;
	EnumAIActionType eActionType;

	//
	// Read records in two passes, because records may include other records.
	// We need to ensure all records exist before we create references.
	//

	// First pass: Read in each record.

	HRECORD hRecord;
	HRECORD hLink;
	HATTRIBUTE hAtt;
	for( uint32 iAIActionSet=0; iAIActionSet < m_cAIActionSetRecords; ++iAIActionSet )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIActionSets, iAIActionSet );
		if( !hRecord )
		{
			continue;
		}

		AIDB_ActionSetRecord& Record = m_aAIActionSetRecords[iAIActionSet];
		Record.eActionSet = (ENUM_AIActionSet)iAIActionSet;

		// ActionSet name.

		Record.strName = g_pLTDatabase->GetRecordName( hRecord );
		if( Record.strName.empty() )
		{
			AIASSERT( 0, NULL, "CAIDB::CreateAIActionSetRecords: Action missing name" );
			continue;
		}

		// Actions.

		hAtt = g_pLTDatabase->GetAttribute( hRecord, AIDB_ACTIONSET_sAction );
		cActions = g_pLTDatabase->GetNumValues( hAtt );
		for( iAction=0; iAction < cActions; ++iAction )
		{
			hLink = GetRecordLink( hRecord, AIDB_ACTIONSET_sAction, iAction );
			if( !hLink )
			{
				continue;
			}

			pszAction = g_pLTDatabase->GetRecordName( hLink );
			eActionType = (EnumAIActionType)String2EnumIndex( pszAction, kAct_Count, (uint32)kAct_InvalidType, s_aszActionTypes );

			if( eActionType == kAct_InvalidType )
			{
				AIASSERT1( 0, NULL, "CAIDB::CreateAIActionSetRecords: Unrecognized Action type: %s", pszAction );
				continue;
			}

			// Add Action to mask.

			Record.ActionMask.set( eActionType, true );
		}
	}

	// Second pass: Reference included records.

	for( uint32 iAIActionSet=0; iAIActionSet < m_cAIActionSetRecords; ++iAIActionSet )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIActionSets, iAIActionSet );
		CollapseIncludedActionSets( hRecord, &( m_aAIActionSetRecords[iAIActionSet].ActionMask ) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CollapseIncludedActionSets()
//
//	PURPOSE:	Recursively include Actions from included ActionsSets.
//
// ----------------------------------------------------------------------- //

void CAIDB::CollapseIncludedActionSets( HRECORD hRecord, AIActionBitSet* pActionMask )
{
	if( !( hRecord && pActionMask ) )
	{
		return;
	}

	// Included ActionSets.

	HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute( hRecord, AIDB_ACTIONSET_sInclude );
	uint32 cIncludes = g_pLTDatabase->GetNumValues( hAtt );

	// For each include, find a record with a matching ActionSet name.

	HRECORD hLink;
	uint32 iRecord;
	const char* pszInclude;
	for( uint32 iInclude=0; iInclude < cIncludes; ++iInclude )
	{
		hLink = GetRecordLink( hRecord, AIDB_ACTIONSET_sInclude, iInclude );
		if( !hLink )
		{
			continue;
		}

		pszInclude = g_pLTDatabase->GetRecordName( hLink );
		if( !pszInclude[0] )
		{
			continue;
		}

		for( iRecord=0; iRecord < m_cAIActionSetRecords; ++iRecord )
		{
			if( !m_aAIActionSetRecords[iRecord].strName.empty() )
			{
				// Found a match. Include referenced ActionSet's children.

				if( LTStrIEquals( pszInclude, m_aAIActionSetRecords[iRecord].strName.c_str() ) )
				{
					*pActionMask |= m_aAIActionSetRecords[iRecord].ActionMask;
					CollapseIncludedActionSets( hLink, pActionMask );
					break;
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAIActivitySetRecords()
//
//	PURPOSE:	Create AIActivitySet records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAIActivitySetRecords()
{
	HCATEGORY hCatAIActivitySets = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_ACTIVITYSET_CATEGORY );
	if( !hCatAIActivitySets )
	{
		return;
	}

	// Create an array as big as the number of records.

	m_cAIActivitySetRecords = g_pLTDatabase->GetNumRecords( hCatAIActivitySets );
	if( !m_cAIActivitySetRecords )
	{
		return;
	}
	m_aAIActivitySetRecords = debug_newa( AIDB_ActivitySetRecord, m_cAIActivitySetRecords );

	uint32 cActivities;
	uint32 iActivity;
	const char* pszActivity;
	EnumAIActivityType eActivityType;

	// Read in each record.

	HRECORD hRecord;
	HATTRIBUTE hAtt;
	for( uint32 iAIActivitySet=0; iAIActivitySet < m_cAIActivitySetRecords; ++iAIActivitySet )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIActivitySets, iAIActivitySet );
		if( !hRecord )
		{
			continue;
		}

		AIDB_ActivitySetRecord& Record = m_aAIActivitySetRecords[iAIActivitySet];
		Record.eActivitySet = (ENUM_AIActivitySet)iAIActivitySet;

		// ActivitySet name.

		Record.strName = g_pLTDatabase->GetRecordName( hRecord );
		if( Record.strName.empty() )
		{
			AIASSERT( 0, NULL, "CAIDB::CreateAIActivitySetRecords: Activity missing name" );
			continue;
		}

		// Activities.

		hAtt = g_pLTDatabase->GetAttribute( hRecord, AIDB_ACTIVITYSET_sActivity );
		cActivities = g_pLTDatabase->GetNumValues( hAtt );
		for( iActivity=0; iActivity < cActivities; ++iActivity )
		{
			pszActivity = GetString( hRecord, AIDB_ACTIVITYSET_sActivity, iActivity );
			if( LTStrIEquals( pszActivity, "None" ) )
			{
				continue;
			}
			eActivityType = (EnumAIActivityType)String2EnumIndex( pszActivity, kActivity_Count, (uint32)kActivity_InvalidType, s_aszActivityTypes );

			if( eActivityType == kActivity_InvalidType )
			{
				AIASSERT1( 0, NULL, "CAIDB::CreateAIActivitySetRecords: Unrecognized Activity type: %s", pszActivity );
				continue;
			}

			// Add Activity to mask.

			Record.ActivityMask.set( eActivityType, true );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAIDamageMaskRecords()
//
//	PURPOSE:	Create AIDamageMask records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAIDamageMaskRecords()
{
	HCATEGORY hCatAIDamageMasks = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_DAMAGEMASK_CATEGORY );
	if( !hCatAIDamageMasks )
	{
		return;
	}

	// Create an array as big as the number of records.

	m_cAIDamageMaskRecords = g_pLTDatabase->GetNumRecords( hCatAIDamageMasks );
	if( !m_cAIDamageMaskRecords )
	{
		return;
	}
	m_aAIDamageMaskRecords = debug_newa( AIDB_DamageMaskRecord, m_cAIDamageMaskRecords );

		
	// Read in each record.

	HRECORD hRecord;
	for( uint32 iDamageMask=0; iDamageMask < m_cAIDamageMaskRecords; ++iDamageMask )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIDamageMasks, iDamageMask );
		if( !hRecord )
		{
			continue;
		}

		// Name.

		AIDB_DamageMaskRecord& Record = m_aAIDamageMaskRecords[iDamageMask];
		Record.strName = g_pLTDatabase->GetRecordName( hRecord );
		Record.eAIDamageMaskID = (ENUM_AIDamageMaskID)iDamageMask;

		// Damage types.
		Record.dfDamageTypes = g_pDTDB->GetDamageMaskFlags( hRecord );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAIBrainRecords()
//
//	PURPOSE:	Create AIBrain records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAIBrainRecords()
{
	HCATEGORY hCatAIBrains = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_BRAIN_CATEGORY );
	if( !hCatAIBrains )
	{
		return;
	}

	// Create an array as big as the number of records.

	m_cAIBrainRecords = g_pLTDatabase->GetNumRecords( hCatAIBrains );
	if( !m_cAIBrainRecords )
	{
		return;
	}
	m_aAIBrainRecords = debug_newa( AIDB_BrainRecord, m_cAIBrainRecords );

	// Read in each record.

	HRECORD hRecord;
	for( uint32 iAIBrain=0; iAIBrain < m_cAIBrainRecords; ++iAIBrain )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIBrains, iAIBrain );
		if( !hRecord )
		{
			continue;
		}

		// Name.

		AIDB_BrainRecord& Record = m_aAIBrainRecords[iAIBrain];
		Record.strName = g_pLTDatabase->GetRecordName( hRecord );
		Record.eAIBrainID = (ENUM_AIBrainID)iAIBrain;

		// Dodge distances.

		Record.fDodgeVectorShuffleDist = GetFloat( hRecord, AIDB_BRAIN_fDodgeVectorShuffleDist );
		Record.fDodgeVectorRollDist = GetFloat( hRecord, AIDB_BRAIN_fDodgeVectorRollDist );

		// Attack time limits.

		Record.fAttackPoseCrouchTime = GetFloat( hRecord, AIDB_BRAIN_fAttackPoseCrouchTime );
		LTVector2 v2AttackGrenadeThrowTime = GetVector2( hRecord, AIDB_BRAIN_fAttackGrenadeThrowTimeRange );
		Record.fAttackGrenadeThrowTimeMin = v2AttackGrenadeThrowTime.x;
		Record.fAttackGrenadeThrowTimeMax = v2AttackGrenadeThrowTime.y;

		// Alarm thresholds.

		Record.nMajorAlarmThreshold = GetInt32( hRecord, AIDB_BRAIN_nMajorAlarmThreshold );
		Record.nImmediateAlarmThreshold = GetInt32( hRecord, AIDB_BRAIN_nImmediateAlarmThreshold );

		// Lip sync.

		Record.bCanLipSync = GetBool( hRecord, AIDB_BRAIN_bCanLipSync );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAIConstantsRecord()
//
//	PURPOSE:	Create an AIConstants record.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAIConstantsRecord()
{
	HCATEGORY hCatAIConstants = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_CONSTANTS_CATEGORY );
	if( !hCatAIConstants )
	{
		return;
	}

	// There should only be 1 AI constants record.

	if( g_pLTDatabase->GetNumRecords( hCatAIConstants ) > 1 )
	{
		AIASSERT( 0, NULL, "CAIDB::CreateAIConstantsRecord: There should only be one AIConstants record" );
	}

	// Read in the record.

	HRECORD hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIConstants, 0 );
	if( !hRecord )
	{
		AIASSERT( 0, NULL, "CAIDB::CreateAIConstantsRecord: Missing the AIConstants record" );
		return;
	}

	// Distances.

	m_AIConstantsRecord.fNoFOVDistanceSqr = GetFloat( hRecord, AIDB_CONSTANTS_fNoFOVDistance );
	m_AIConstantsRecord.fNoFOVDistanceSqr *= m_AIConstantsRecord.fNoFOVDistanceSqr;

	m_AIConstantsRecord.fInstantSeeDistanceSqr = GetFloat( hRecord, AIDB_CONSTANTS_fInstantSeeDistance );
	m_AIConstantsRecord.fInstantSeeDistanceSqr *= m_AIConstantsRecord.fInstantSeeDistanceSqr;

	m_AIConstantsRecord.fAlertImmediateThreatInstantSeeDistanceSqr = GetFloat( hRecord, AIDB_CONSTANTS_fAlertImmediateThreatInstantSeeDistance );
	m_AIConstantsRecord.fAlertImmediateThreatInstantSeeDistanceSqr *= m_AIConstantsRecord.fAlertImmediateThreatInstantSeeDistanceSqr;

	m_AIConstantsRecord.fThreatTooCloseDistance = GetFloat( hRecord, AIDB_CONSTANTS_fThreatTooCloseDistance );
	m_AIConstantsRecord.fThreatTooCloseDistanceSqr = m_AIConstantsRecord.fThreatTooCloseDistance * m_AIConstantsRecord.fThreatTooCloseDistance;

	// Personal bubble.

	m_AIConstantsRecord.fPersonalBubbleDistanceSqr = GetFloat( hRecord, AIDB_CONSTANTS_fPersonalBubbleDistance );
	m_AIConstantsRecord.fPersonalBubbleDistanceSqr *= m_AIConstantsRecord.fPersonalBubbleDistanceSqr;

	m_AIConstantsRecord.fPersonalBubbleStimulusRateMod = GetFloat( hRecord, AIDB_CONSTANTS_fPersonalBubbleStimulusRateMod );

	// FOVs.

	m_AIConstantsRecord.fFOVAlertImmediateThreat = GetFloat( hRecord, AIDB_CONSTANTS_fFOVAlertImmediateThreat );
	ConvertFOV( &m_AIConstantsRecord.fFOVAlertImmediateThreat );

	m_AIConstantsRecord.fFOVAlert = GetFloat( hRecord, AIDB_CONSTANTS_fFOVAlert );
	ConvertFOV( &m_AIConstantsRecord.fFOVAlert );

	m_AIConstantsRecord.fFOVSuspicious = GetFloat( hRecord, AIDB_CONSTANTS_fFOVSuspicious );
	ConvertFOV( &m_AIConstantsRecord.fFOVSuspicious );

	m_AIConstantsRecord.fFOVRelaxed = GetFloat( hRecord, AIDB_CONSTANTS_fFOVRelaxed );
	ConvertFOV( &m_AIConstantsRecord.fFOVRelaxed );

	// Threat unseen time.

	m_AIConstantsRecord.fThreatUnseenTime = GetFloat( hRecord, AIDB_CONSTANTS_fThreatUnseenTime );

	// Hold position constraints.

	m_AIConstantsRecord.fHoldPositionTime = GetFloat( hRecord, AIDB_CONSTANTS_fHoldPositionTime );
	m_AIConstantsRecord.fHoldPositionDistance = GetFloat( hRecord, AIDB_CONSTANTS_fHoldPositionDistance );
	m_AIConstantsRecord.fHoldPositionDistanceSqr = m_AIConstantsRecord.fHoldPositionDistance * m_AIConstantsRecord.fHoldPositionDistance;

	// Avoidance times.

	LTVector2 v2NodeAvoidanceTime = GetVector2( hRecord, AIDB_CONSTANTS_fDamagedAtNodeAvoidanceTime );
	m_AIConstantsRecord.fDamagedAtNodeAvoidanceTimeMin = v2NodeAvoidanceTime.x;
	m_AIConstantsRecord.fDamagedAtNodeAvoidanceTimeMax = v2NodeAvoidanceTime.y;
	m_AIConstantsRecord.fBlockedDoorAvoidanceTime = GetFloat( hRecord, AIDB_CONSTANTS_fBlockedDoorAvoidanceTime );

	// Navigation.

	m_AIConstantsRecord.fBlockedDoorWeightMultiplier = GetFloat( hRecord, AIDB_CONSTANTS_fBlockedDoorWeightMultiplier );

	// Sound frequencies.

	m_AIConstantsRecord.fAISoundFrequencyChatter = GetFloat( hRecord, AIDB_CONSTANTS_fAISoundFrequencyChatter );
	m_AIConstantsRecord.fAISoundFrequencyEvent = GetFloat( hRecord, AIDB_CONSTANTS_fAISoundFrequencyEvent );
	m_AIConstantsRecord.fAISoundFrequencyLocation = GetFloat( hRecord, AIDB_CONSTANTS_fAISoundFrequencyLocation );
	m_AIConstantsRecord.fAISoundFrequencyMelee = GetFloat( hRecord, AIDB_CONSTANTS_fAISoundFrequencyMelee );

	// Difficulty factors.

	m_AIConstantsRecord.fDifficultyFactorEasy = g_pServerDB->GetDifficultyFactor(GD_EASY);
	m_AIConstantsRecord.fDifficultyFactorNormal = g_pServerDB->GetDifficultyFactor(GD_NORMAL);
	m_AIConstantsRecord.fDifficultyFactorHard = g_pServerDB->GetDifficultyFactor(GD_HARD);
	m_AIConstantsRecord.fDifficultyFactorVeryHard = g_pServerDB->GetDifficultyFactor(GD_VERYHARD);
	m_AIConstantsRecord.fDifficultyFactorPlayerIncrease = GetFloat( hRecord, AIDB_CONSTANTS_fDifficultyFactorPlayerIncrease );

	// Cycle angular cut-offs.

	m_AIConstantsRecord.fCycleCutOffForward		= GetFloat( hRecord, AIDB_CONSTANTS_fCycleCutOffForward );
	m_AIConstantsRecord.fCycleCutOffLeft		= GetFloat( hRecord, AIDB_CONSTANTS_fCycleCutOffLeft );
	m_AIConstantsRecord.fCycleCutOffBackward	= GetFloat( hRecord, AIDB_CONSTANTS_fCycleCutOffBackward );
	m_AIConstantsRecord.fCycleCutOffRight		= GetFloat( hRecord, AIDB_CONSTANTS_fCycleCutOffRight );

	m_AIConstantsRecord.fCycleCutOffForward		= MATH_DEGREES_TO_RADIANS( m_AIConstantsRecord.fCycleCutOffForward );
	m_AIConstantsRecord.fCycleCutOffLeft		= MATH_DEGREES_TO_RADIANS( m_AIConstantsRecord.fCycleCutOffLeft );
	m_AIConstantsRecord.fCycleCutOffBackward	= MATH_DEGREES_TO_RADIANS( m_AIConstantsRecord.fCycleCutOffBackward );
	m_AIConstantsRecord.fCycleCutOffRight		= MATH_DEGREES_TO_RADIANS( m_AIConstantsRecord.fCycleCutOffRight );

	// Name of the 'neutral' alignment.  This is used to allow AI's to use 
	// objects such as WeaponItems or CombatOpportunity objects.

	HRECORD hLink = GetRecordLink( hRecord, AIDB_CONSTANTS_sObjectAlignmentName);
	const char* const pszRecordName = GetRecordName(hLink);
	m_AIConstantsRecord.strObjectAlignmentName	= pszRecordName ? pszRecordName : "";

	// Create bitset indicating valid AwarenessMods for this game.

	uint32 bits = BuildBitsetFromAttributeList( hRecord, AIDB_CONSTANTS_sAwarenessMods, (uint32)kAwarenessMod_Invalid, s_aszAIAwarenessMods, kAwarenessMod_Count );
	m_AIConstantsRecord.bitsValidAwarenessMods = AIAWARENESSMOD_BITS( bits );

	// Set to true to allow recoil animations to interrupt other recoil 
	// animations.

	m_AIConstantsRecord.bAllowRecoilsToInterruptRecoils = GetBool( hRecord, AIDB_CONSTANTS_bAllowRecoilsToInterruptRecoils );

	// Set to true to cause AI dropped weapons to inherit the velocity and 
	// angular velocity of the socket they are attached to instead of using 
	// the death velocity and a random angular velocity.

	m_AIConstantsRecord.bWhenDroppedWeaponsInheritSocketVelocities = GetBool( hRecord, AIDB_CONSTANTS_bWhenDroppedWeaponsInheritSocketVelocities );

	// StringPulling max iterations.

	m_AIConstantsRecord.nStringPullingMaxIterations = GetInt32( hRecord, AIDB_CONSTANTS_nStringPullingMaxIterations );

	// When doing collision between AI (or AI and the player), this buffer is 
	// used as a scalar to prevent clipping.

	m_AIConstantsRecord.fAntiPenetrationRadiusBuffer = GetFloat( hRecord, AIDB_CONSTANTS_fAntiPenetrationRadiusBuffer );

	// Weapon property used when the AI has no weapon.  Moved into the 
	// database to allow different projects to use different weapon names.

	const char* const pszAnimPropName = GetString( hRecord, AIDB_CONSTANTS_sUnarmedWeaponProp );
	m_AIConstantsRecord.eUnarmedWeaponProp = AnimPropUtils::Enum( pszAnimPropName );

	// Combat opportunity frequency.

	m_AIConstantsRecord.fCombatOpportunityFrequency = GetFloat( hRecord, AIDB_CONSTANTS_fCombatOpportunityFrequency );

	// If the AI hears/sees a disturbance but can't search due to lack of nodes,
	// he can either play his investigate animation a single time, or he can
	// loop it.  Currently this is a setting that varies between games.

	m_AIConstantsRecord.bLoopInvestigateIfCantSearch = GetBool( hRecord, AIDB_CONSTANTS_bLoopInvestigateIfCantSearch );

	// If the AI hars/sees a disturbance and is running a lower priority target 
	// action such as Follow, nothing causes a reselected.  This results in AIs 
	// acting oblivious to gunshots/people running up behind them. Currently 
	// this is a setting that varies between games.

	m_AIConstantsRecord.bDisturbancesCauseTargetSelection = GetBool( hRecord, AIDB_CONSTANTS_bDisturbancesCauseTargetSelection );

	// NavMeshLink death delay.

	m_AIConstantsRecord.fNavMeshLinkDeathDelay = GetFloat( hRecord, AIDB_CONSTANTS_fNavMeshLinkDeathDelay );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAIAttributesRecords()
//
//	PURPOSE:	Create AIAttributes records.
//
// ----------------------------------------------------------------------- //

void CAIDB::ConvertFOV( float* pFOV )
{
	if( !pFOV )
	{
		return;
	}

	// It's easier to compare a 180 FOV if it is exact,
	// without any floating point error.  This alleviates
	// issues in CAI::IsInsideFOV.

	if( *pFOV == 180.f )
	{
		*pFOV = c_fFOV180;
		return;
	}

	*pFOV = DEG2RAD( *pFOV );
	*pFOV = cos( 0.5f * *pFOV );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAIAttributesRecords()
//
//	PURPOSE:	Create AIAttributes records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAIAttributesRecords()
{
	HCATEGORY hCatAIAttributes = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_ATTRIBUTES_CATEGORY );
	if( !hCatAIAttributes )
	{
		return;
	}

	// Create an array as big as the number of records.

	m_cAIAttributesRecords = g_pLTDatabase->GetNumRecords( hCatAIAttributes );
	if( !m_cAIAttributesRecords )
	{
		return;
	}
	m_aAIAttributesRecords = debug_newa( AIDB_AttributesRecord, m_cAIAttributesRecords );

	const char* pszDamageMask;
	const char* pszLimpLimits;

	// Read in each record.

	HRECORD hRecord;
	HRECORD hLink;
	for( uint32 iRecord=0; iRecord < m_cAIAttributesRecords; ++iRecord )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIAttributes, iRecord );
		if( !hRecord )
		{
			continue;
		}

		// Name.

		AIDB_AttributesRecord& Record = m_aAIAttributesRecords[iRecord];
		Record.strName = g_pLTDatabase->GetRecordName( hRecord );
		Record.eAIAttributesID = (ENUM_AIAttributesID)iRecord;

		// Speeds.

		Record.fRunSpeed = GetFloat( hRecord, AIDB_ATTRIBUTES_fRunSpeed );
		Record.fJumpSpeed = GetFloat( hRecord, AIDB_ATTRIBUTES_fJumpSpeed );
		Record.fJumpOverSpeed = GetFloat( hRecord, AIDB_ATTRIBUTES_fJumpOverSpeed );
		Record.fFallSpeed = GetFloat( hRecord, AIDB_ATTRIBUTES_fFallSpeed );
		Record.fWalkSpeed = GetFloat( hRecord, AIDB_ATTRIBUTES_fWalkSpeed );
		Record.fSwimSpeed = GetFloat( hRecord, AIDB_ATTRIBUTES_fSwimSpeed );

		// Rotation times.

		Record.fRotationTimeLimited = GetFloat( hRecord, AIDB_ATTRIBUTES_fRotationTimeLimited );
		Record.fRotationTimeStatic = GetFloat( hRecord, AIDB_ATTRIBUTES_fRotationTimeStatic );
		Record.fRotationTimeMoving = GetFloat( hRecord, AIDB_ATTRIBUTES_fRotationTimeMoving );

		// Sound radius.

		Record.fSoundOuterRadius = GetFloat( hRecord, AIDB_ATTRIBUTES_fSoundRadius );
		Record.fSoundInnerRadius = GetFloat( hRecord, AIDB_ATTRIBUTES_fSoundInnerRadius );

		// Accuracy.

		Record.fAccuracy = GetFloat( hRecord, AIDB_ATTRIBUTES_fAccuracy );
		Record.fAccuracyIncreaseRate = GetFloat( hRecord, AIDB_ATTRIBUTES_fAccuracyIncreaseRate );
		Record.fAccuracyDecreaseRate = GetFloat( hRecord, AIDB_ATTRIBUTES_fAccuracyDecreaseRate );
		Record.fAccuracyMissPerturb = GetFloat( hRecord, AIDB_ATTRIBUTES_fAccuracyMissPerturb );
		Record.fMaxMovementAccuracyPerturb = GetFloat( hRecord, AIDB_ATTRIBUTES_fMaxMovementAccuracyPerturb );
		Record.fMovementAccuracyPerturbDecay = GetFloat( hRecord, AIDB_ATTRIBUTES_fMovementAccuracyPerturbTime );
		Record.fFullAccuracyRadiusSqr = GetFloat( hRecord, AIDB_ATTRIBUTES_fFullAccuracyRadius );

		Record.fFullAccuracyRadiusSqr *= Record.fFullAccuracyRadiusSqr;
		Record.fMovementAccuracyPerturbDecay = Record.fMaxMovementAccuracyPerturb / Record.fMovementAccuracyPerturbDecay;

		// Update rate.

		Record.fUpdateRate = GetFloat( hRecord, AIDB_ATTRIBUTES_fUpdateRate );

		// Vertical threshold.

		Record.fVerticalThreshold = GetFloat( hRecord, AIDB_ATTRIBUTES_fVerticalThreshold );

		// Sense distances.

		Record.fSeeDistance = GetFloat( hRecord, AIDB_ATTRIBUTES_fSeeDistance );
		Record.fHearDistance = GetFloat( hRecord, AIDB_ATTRIBUTES_fHearDistance );

		// Sense FOVs

		float flTargetIsAimingAtMeFOV		= GetFloat( hRecord, AIDB_CONSTANTS_fTargetIsAimingAtMeFOV );
		Record.fTargetIsAimingAtMeFOVDp		= FOV2DP(flTargetIsAimingAtMeFOV); 

		float flTargetIsLookingAtMeFOV		= GetFloat( hRecord, AIDB_CONSTANTS_fTargetIsLookingAtMeFOV );
		Record.fTargetIsLookingAtMeFOVDp	= FOV2DP(flTargetIsLookingAtMeFOV); 

		// Directional movement.

		Record.fMinDirectionalRunChangeDistanceSqr = GetFloat( hRecord, AIDB_CONSTANTS_fMinDirectionalRunChangeDistance );
		Record.fMinDirectionalRunChangeDistanceSqr *= Record.fMinDirectionalRunChangeDistanceSqr;

		// Animation Blending

		Record.bEnableAnimationBlending	= GetBool( hRecord, AIDB_ATTRIBUTES_bEnableAnimationBlending );

		// If true, the AI will die instantly on taking damage.

		Record.bInstantDeath	= GetBool( hRecord, AIDB_ATTRIBUTES_bInstantDeath );

		// Alignment.

		hLink = GetRecordLink( hRecord, AIDB_ATTRIBUTES_sAlignment );
		if( hLink )
		{
			Record.strAlignment = g_pLTDatabase->GetRecordName( hLink );
		}

		// Add this AIAttribute entry to its NavMeshTypes mask

		hLink = GetRecordLink( hRecord, AIDB_ATTRIBUTES_sNavMeshType );
		if( hLink )
		{
			uint32 iNavMeshType = g_pLTDatabase->GetRecordIndex( hLink );
			AIASSERT( iNavMeshType < m_cAIAINavMeshTypeRecords, NULL, "CAIDB::CreateAIAttributesRecords: " );
			if ( iNavMeshType < m_cAIAINavMeshTypeRecords )
			{
				m_aAIAINavMeshTypeRecords[ iNavMeshType ].m_dwCharacterTypeMask |= ( 1 << iRecord );
			}
		}

		// Action set.

		hLink = GetRecordLink( hRecord, AIDB_ATTRIBUTES_sAIActionSet );
		if( hLink )
		{
			Record.strAIActionSet = g_pLTDatabase->GetRecordName( hLink );
		}

		// Target Selection Action set.

		hLink = GetRecordLink( hRecord, AIDB_ATTRIBUTES_sTargetSelectSet );
		if( hLink )
		{
			Record.strTargetSelectSet = g_pLTDatabase->GetRecordName( hLink );
		}

		// Activity set.

		hLink = GetRecordLink( hRecord, AIDB_ATTRIBUTES_sAIActivitySet );
		if( hLink )
		{
			Record.strAIActivitySet = g_pLTDatabase->GetRecordName( hLink );
		}

		// Can join squads.

		Record.bCanJoinSquads = GetBool( hRecord, AIDB_ATTRIBUTES_bCanJoinSquads );

		// Activity set.

		hLink = GetRecordLink( hRecord, AIDB_ATTRIBUTES_sAIActivitySet );
		if( hLink )
		{
			Record.strAIActivitySet = g_pLTDatabase->GetRecordName( hLink );
		}

		// AIMovement set.

		hLink = GetRecordLink( hRecord, AIDB_ATTRIBUTES_sAIMovementSet );
		if( hLink )
		{
			Record.strAIMovementSet = g_pLTDatabase->GetRecordName( hLink );
		}

		// AIWeaponOverrideSet set.

		hLink = GetRecordLink( hRecord, AIDB_ATTRIBUTES_sAIWeaponOverrideSet );
		if( hLink )
		{
			Record.strAIWeaponOverrideSet = g_pLTDatabase->GetRecordName( hLink );
		}

		// DroppedItems set.

		hLink = GetRecordLink( hRecord, AIDB_ATTRIBUTES_sDroppedItems );
		if( hLink )
		{
			Record.strDroppedItems = g_pLTDatabase->GetRecordName( hLink );
		}

		// Damage mask.

		hLink = GetRecordLink( hRecord, AIDB_ATTRIBUTES_sDamageMask );
		Record.eDamageMaskID = kAIDamageMaskID_Invalid;
		if( hLink )
		{
			pszDamageMask = g_pLTDatabase->GetRecordName( hLink );
			Record.eDamageMaskID = GetAIDamageMaskRecordID( pszDamageMask );
		}

		// Limp limits.

		hLink = GetRecordLink( hRecord, AIDB_ATTRIBUTES_sLimpLimits );
		Record.eLimpLimitsID = kAILimitsID_Invalid;
		if( hLink )
		{
			pszLimpLimits = g_pLTDatabase->GetRecordName( hLink );
			Record.eLimpLimitsID = GetAILimitsRecordID( pszLimpLimits );
		}

		// Lip sync.

		Record.bCanLipSync = GetBool( hRecord, AIDB_ATTRIBUTES_bCanLipSync );

		// Set use prevent movement into obstacles to true to block movement 
		// which would cause characters to exist inside of each other.

		Record.bUsePreventMovementIntoObstacles = GetBool( hRecord, AIDB_ATTRIBUTES_bPreventMovementIntoObstacles );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAILimitsRecords()
//
//	PURPOSE:	Create AILimits records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAILimitsRecords()
{
	HCATEGORY hCatAILimits = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_LIMITS_CATEGORY );
	if( !hCatAILimits )
	{
		return;
	}

	// Create an array as big as the number of records.

	m_cAILimitsRecords = g_pLTDatabase->GetNumRecords( hCatAILimits );
	if( !m_cAILimitsRecords )
	{
		return;
	}
	m_aAILimitsRecords = debug_newa( AIDB_LimitsRecord, m_cAILimitsRecords );

	// Read in each record.

	HRECORD hRecord;
	for( uint32 iRecord=0; iRecord < m_cAILimitsRecords; ++iRecord )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAILimits, iRecord );
		if( !hRecord )
		{
			continue;
		}

		// Name.

		AIDB_LimitsRecord& Record = m_aAILimitsRecords[iRecord];
		Record.strName = g_pLTDatabase->GetRecordName( hRecord );
		Record.eAILimitsID = (ENUM_AILimitsID)iRecord;

		// Frequency.

		Record.fFrequency = GetFloat( hRecord, AIDB_LIMITS_fFrequency );

		// Probability.

		Record.fProbability = GetFloat( hRecord, AIDB_LIMITS_fProbability );

		// Threshold.

		Record.fThreshold = GetFloat( hRecord, AIDB_LIMITS_fThreshold );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAIStimulusRecords()
//
//	PURPOSE:	Create AIStimulus records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAIStimulusRecords()
{
	HCATEGORY hCatAIStimuli = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_STIMULUS_CATEGORY );
	if( !hCatAIStimuli )
	{
		return;
	}

	// Create an array as big as the number of records.

	m_cAIStimulusRecords = g_pLTDatabase->GetNumRecords( hCatAIStimuli );
	if( !m_cAIStimulusRecords )
	{
		return;
	}
	m_aAIStimulusRecords = debug_newa( AIDB_StimulusRecord, m_cAIStimulusRecords );

	const char* pszName;
	const char* pszSense;
	uint32 iStance;
	uint32 cStances;
	const char* pszStance;
	EnumCharacterStance eStance;

	// Read in each record.

	HRECORD hRecord;
	HATTRIBUTE hAtt;
	for( uint32 iAIStimulus=0; iAIStimulus < m_cAIStimulusRecords; ++iAIStimulus )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIStimuli, iAIStimulus );
		if( !hRecord )
		{
			continue;
		}

		// Stimulus type.

		AIDB_StimulusRecord& Record = m_aAIStimulusRecords[iAIStimulus];
		pszName = g_pLTDatabase->GetRecordName( hRecord );
		Record.eStimulusType = (EnumAIStimulusType)String2BitFlag( pszName, kStim_Count, s_aszStimulusTypes );
		AIASSERT1( Record.eStimulusType < (1 << kStim_Count), NULL, "CAIDB::CreateAIStimulusRecords: Unrecognized stimulus type: %s", pszName );

		// Sense type.

		pszSense = GetString( hRecord, AIDB_STIMULUS_sSense );
		Record.eSenseType = (EnumAISenseType)String2BitFlag( pszSense, kSense_Count, s_aszSenseTypes );
		AIASSERT1( Record.eSenseType < (1 << kSense_Count), NULL, "CAIDB::CreateAIStimulusRecords: Unrecognized sense type: %s", pszSense );

		// Required stance.

		hAtt = g_pLTDatabase->GetAttribute( hRecord, AIDB_STIMULUS_sRequiredStance );
		cStances = g_pLTDatabase->GetNumValues( hAtt );
		for( iStance=0; iStance < cStances; ++iStance )
		{
			pszStance = GetString( hRecord, AIDB_STIMULUS_sRequiredStance, iStance );
			eStance = CharacterAlignment::ConvertNameToStance( pszStance );
			Record.bitsRequiredStance.set( eStance );
		}

		// Distance.

		Record.fDistance = GetFloat( hRecord, AIDB_STIMULUS_fDistance );

		// Vertical radius.

		Record.fVerticalRadius = GetFloat( hRecord, AIDB_STIMULUS_fVerticalRadius );

		// Duration.

		Record.fDuration = GetFloat( hRecord, AIDB_STIMULUS_fDuration );

		// Alarm level.

		Record.nAlarmLevel = GetInt32( hRecord, AIDB_STIMULUS_nAlarmLevel );

		// Stimulation increase/decrease.

		Record.fStimulationIncreaseRateAlert = GetFloat( hRecord, AIDB_STIMULUS_fStimulationIncreaseRateAlert );
		Record.fStimulationIncreaseRateSuspicious = GetFloat( hRecord, AIDB_STIMULUS_fStimulationIncreaseRateSuspicious );
		Record.fStimulationIncreaseRateUnalert = GetFloat( hRecord, AIDB_STIMULUS_fStimulationIncreaseRateUnalert );
		Record.fStimulationDecreaseRateAlert = GetFloat( hRecord, AIDB_STIMULUS_fStimulationDecreaseRateAlert );
		Record.fStimulationDecreaseRateSuspicious = GetFloat( hRecord, AIDB_STIMULUS_fStimulationDecreaseRateSuspicious );
		Record.fStimulationDecreaseRateUnalert = GetFloat( hRecord, AIDB_STIMULUS_fStimulationDecreaseRateUnalert );

		// False stimulation limit.

		Record.nFalseStimulationLimit = GetInt32( hRecord, AIDB_STIMULUS_nFalseStimulationLimit );

		// Reaction delay.

		Record.v2ReactionDelay = GetVector2( hRecord, AIDB_STIMULUS_v2ReactionDelay );

		// Source is self.

		Record.bRequireSourceIsNotSelf = GetBool( hRecord, AIDB_STIMULUS_bRequireSourceIsNotSelf );
		Record.bRequireSourceIsSelf = GetBool( hRecord, AIDB_STIMULUS_bRequireSourceIsSelf );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAIStimulusMaskRecords()
//
//	PURPOSE:	Create AIStimulusMask records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAIStimulusMaskRecords()
{
	HCATEGORY hCatAIStimulusMasks = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_STIMULUSMASK_CATEGORY );
	if( !hCatAIStimulusMasks )
	{
		return;
	}

	// Create an array as big as the number of records.

	m_cAIStimulusMaskRecords = g_pLTDatabase->GetNumRecords( hCatAIStimulusMasks );
	if( !m_cAIStimulusMaskRecords )
	{
		return;
	}
	m_aAIStimulusMaskRecords = debug_newa( AIDB_StimulusMaskRecord, m_cAIStimulusMaskRecords );

	uint32 cStimuli;
	uint32 iStimulus;
	const char* pszStimulus;

	// Read in each record.

	HRECORD hRecord;
	HRECORD hLink;
	HATTRIBUTE hAtt;
	for( uint32 iAIStimulusMask=0; iAIStimulusMask < m_cAIStimulusMaskRecords; ++iAIStimulusMask )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIStimulusMasks, iAIStimulusMask );
		if( !hRecord )
		{
			continue;
		}

		// Name.

		AIDB_StimulusMaskRecord& Record = m_aAIStimulusMaskRecords[iAIStimulusMask];
		Record.strName = g_pLTDatabase->GetRecordName( hRecord );

		// Stimuli to block.

		hAtt = g_pLTDatabase->GetAttribute( hRecord, AIDB_STIMULUSMASK_sStimulus );
		cStimuli = g_pLTDatabase->GetNumValues( hAtt );
		for( iStimulus=0; iStimulus < cStimuli; ++iStimulus )
		{
			hLink = GetRecordLink( hRecord, AIDB_STIMULUSMASK_sStimulus, iStimulus );
			if( hLink )
			{
				pszStimulus = g_pLTDatabase->GetRecordName( hLink );
				Record.dwBlockedStimuli |= String2BitFlag( pszStimulus, kStim_Count, s_aszStimulusTypes );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAISensorRecords()
//
//	PURPOSE:	Create AISensor records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAISensorRecords()
{
	HCATEGORY hCatAISensors = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_SENSOR_CATEGORY );
	if( !hCatAISensors )
	{
		return;
	}

	// Get the total record count.

	uint32 cAISensorRecords = g_pLTDatabase->GetNumRecords( hCatAISensors );
	if( !cAISensorRecords )
	{
		return;
	}


	const char* pszSensorType;
	EnumAISensorType eSensorType;
	const char* pszSensorClass;
	const char* pszNodeType;
	const char* pszSmartObject;
	uint32 cStimuli;
	uint32 iStimulus;
	const char* pszStimulus;

	// Read in each record.

	HRECORD hRecord;
	HRECORD hLink;
	HATTRIBUTE hAtt;
	for( uint32 iAISensor=0; iAISensor < cAISensorRecords; ++iAISensor )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAISensors, iAISensor );
		if( !hRecord )
		{
			continue;
		}

		// Sensor type.

		pszSensorType = g_pLTDatabase->GetRecordName( hRecord );
		eSensorType = (EnumAISensorType)String2EnumIndex( pszSensorType, kSensor_Count, (uint32)kSensor_InvalidType, s_aszSensorTypes );
		if( eSensorType == kSensor_InvalidType )
		{
			AIASSERT1( 0, NULL, "CAIDB::CreateAISensorRecords: Unrecognized sensor type: %s", pszSensorType );
			continue;
		}
		AIDB_SensorRecord& Record = m_aAISensorRecords[eSensorType];
		Record.eSensorType = eSensorType;

		// Sensor class.

		pszSensorClass = GetString( hRecord, AIDB_SENSOR_sClass );
		Record.eSensorClass = (EnumAISensorType)String2EnumIndex( pszSensorClass, kSensor_Count, (uint32)kSensor_InvalidType, s_aszSensorTypes );

		// Update rate.

		Record.fSensorUpdateRate = GetFloat( hRecord, AIDB_SENSOR_fUpdateRate );

		// Node type.

		pszNodeType = GetString( hRecord, AIDB_SENSOR_sNodeType );
		if( !LTStrIEquals( pszNodeType, "None" ) )
		{
			Record.eNodeType = AINodeUtils::GetNodeType( pszNodeType );
		}

		// SmartObject.

		hLink = GetRecordLink( hRecord, AIDB_SENSOR_sSmartObject );
		if( hLink )
		{
			pszSmartObject = g_pLTDatabase->GetRecordName( hLink );
			Record.eSmartObjectID = GetAISmartObjectRecordID( pszSmartObject );
		}
		else
		{
			Record.eSmartObjectID = kAISmartObjectID_Invalid;
		}

		// Stimuli.

		hAtt = g_pLTDatabase->GetAttribute( hRecord, AIDB_SENSOR_sStimulus );
		cStimuli = g_pLTDatabase->GetNumValues( hAtt );
		for( iStimulus=0; iStimulus < cStimuli; ++iStimulus )
		{
			hLink = GetRecordLink( hRecord, AIDB_SENSOR_sStimulus, iStimulus );
			if( hLink )
			{
				pszStimulus = g_pLTDatabase->GetRecordName( hLink );
				Record.dwStimulusTypes |= String2BitFlag( pszStimulus, kStim_Count, s_aszStimulusTypes );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAITargetRecords()
//
//	PURPOSE:	Create AITarget records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAITargetRecords()
{
	HCATEGORY hCatAITargets = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_TARGETSELECT_CATEGORY );
	if( !hCatAITargets )
	{
		return;
	}

	// Create an array as big as the number of records.

	m_cAITargetSelectRecords = g_pLTDatabase->GetNumRecords( hCatAITargets );
	if( !m_cAITargetSelectRecords )
	{
		return;
	}
	m_aAITargetSelectRecords = debug_newa( AIDB_TargetSelectRecord, m_cAITargetSelectRecords );

	const char* pszName;
	const char* pszClass;

	// Read in each record.

	HRECORD hRecord;
	for( uint32 iAITarget=0; iAITarget < m_cAITargetSelectRecords; ++iAITarget )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAITargets, iAITarget );
		if( !hRecord )
		{
			continue;
		}

		// TargetSelect type.

		AIDB_TargetSelectRecord& Record = m_aAITargetSelectRecords[iAITarget];
		pszName = g_pLTDatabase->GetRecordName( hRecord );
		Record.eTargetSelectType = (EnumAITargetSelectType)String2EnumIndex( pszName, kTargetSelect_Count, (uint32)kTargetSelect_InvalidType, s_aszTargetSelectTypes );
		if( Record.eTargetSelectType == kTargetSelect_InvalidType )
		{
			AIASSERT1( 0, NULL, "CAIDB::CreateAITargetRecords: Unrecognized TargetSelect type: %s", pszName );
			continue;
		}

		// TargetSelect class.

		pszClass = GetString( hRecord, AIDB_TARGETSELECT_sClass );
		Record.eTargetSelectClass = (EnumAITargetSelectType)String2EnumIndex( pszClass, kTargetSelect_Count, (uint32)kTargetSelect_InvalidType, s_aszTargetSelectTypes );
		if( Record.eTargetSelectClass == kTargetSelect_InvalidType )
		{
			AIASSERT1( 0, NULL, "CAIDB::CreateAITargetRecords: Unrecognized TargetSelect class: %s", pszClass );
			continue;
		}

		// Awareness

		const char* const pszAwareness = GetString( hRecord, AIDB_TARGETSELECT_sAwareness );
		if( !LTStrIEquals( pszAwareness, "None" ) )
		{
			Record.eAwareness = StringToAwareness( pszAwareness );
		}
		

		// Cost.

		Record.fCost = GetFloat( hRecord, AIDB_TARGETSELECT_fCost );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAITargetSetRecords()
//
//	PURPOSE:	Create AITargetSet records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAITargetSetRecords()
{
	HCATEGORY hCatAITargetSets = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_TARGETSELECTSET_CATEGORY );
	if( !hCatAITargetSets )
	{
		return;
	}

	// Create an array as big as the number of records.

	m_cAITargetSelectSetRecords = g_pLTDatabase->GetNumRecords( hCatAITargetSets );
	if( !m_cAITargetSelectSetRecords )
	{
		return;
	}
	m_aAITargetSelectSetRecords = debug_newa( AIDB_TargetSelectSetRecord, m_cAITargetSelectSetRecords );

	uint32 cTargets;
	uint32 iTarget;
	const char* pszTarget;
	EnumAITargetSelectType eTargetSelectType;

	//
	// Read records in two passes, because records may include other records.
	// We need to ensure all records exist before we create references.
	//

	// First pass: Read in each record.

	HRECORD hRecord;
	HRECORD hLink;
	HATTRIBUTE hAtt;
	for( uint32 iAITargetSet=0; iAITargetSet < m_cAITargetSelectSetRecords; ++iAITargetSet )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAITargetSets, iAITargetSet );
		if( !hRecord )
		{
			continue;
		}

		AIDB_TargetSelectSetRecord& Record = m_aAITargetSelectSetRecords[iAITargetSet];
		Record.eTargetSelectSet = (ENUM_AITargetSelectSet)iAITargetSet;

		// TargetSet name.

		Record.strName = g_pLTDatabase->GetRecordName( hRecord );
		if( Record.strName.empty() )
		{
			AIASSERT( 0, NULL, "CAIDB::CreateAITargetSetRecords: Target missing name" );
			continue;
		}

		// TargetSelects.

		hAtt = g_pLTDatabase->GetAttribute( hRecord, AIDB_TARGETSELECTSET_sTarget );
		cTargets = g_pLTDatabase->GetNumValues( hAtt );
		for( iTarget=0; iTarget < cTargets; ++iTarget )
		{
			hLink = GetRecordLink( hRecord, AIDB_TARGETSELECTSET_sTarget, iTarget );
			if( !hLink )
			{
				continue;
			}

			pszTarget = g_pLTDatabase->GetRecordName( hLink );
			eTargetSelectType = (EnumAITargetSelectType)String2EnumIndex( pszTarget, kTargetSelect_Count, (uint32)kTargetSelect_InvalidType, s_aszTargetSelectTypes );

			if( eTargetSelectType == kTargetSelect_InvalidType )
			{
				AIASSERT1( 0, NULL, "CAIDB::CreateAITargetSetRecords: Unrecognized TargetSelect type: %s", pszTarget );
				continue;
			}

			// Add TargetSelect to mask.

			Record.TargetSelectMask.set( eTargetSelectType, true );
		}
	}

	// Second pass: Reference included records.

	for( uint32 iAITargetSet=0; iAITargetSet < m_cAITargetSelectSetRecords; ++iAITargetSet )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAITargetSets, iAITargetSet );
		CollapseIncludedTargetSelectSets( hRecord, &( m_aAITargetSelectSetRecords[iAITargetSet].TargetSelectMask ) );
	}
}
		
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CollapseIncludedTargetSelectSets()
//
//	PURPOSE:	Recursively include TargetSelects from included TargetSelectSets.
//
// ----------------------------------------------------------------------- //

void CAIDB::CollapseIncludedTargetSelectSets( HRECORD hRecord, AITargetSelectBitSet* pTargetSelectMask )
{
	if( !( hRecord && pTargetSelectMask ) )
	{
		return;
	}

	// Included TargetSelectSets.

	HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute( hRecord, AIDB_TARGETSELECTSET_sInclude );
	uint32 cIncludes = g_pLTDatabase->GetNumValues( hAtt );

	// For each include, find a record with a matching TargetSelectSet name.

	HRECORD hLink;
	uint32 iRecord;
	const char* pszInclude;
	for( uint32 iInclude=0; iInclude < cIncludes; ++iInclude )
	{
		hLink = GetRecordLink( hRecord, AIDB_TARGETSELECTSET_sInclude, iInclude );
		if( !hLink )
		{
			continue;
		}

		pszInclude = g_pLTDatabase->GetRecordName( hLink );
		if( !pszInclude[0] )
		{
			continue;
		}

		for( iRecord=0; iRecord < m_cAITargetSelectSetRecords; ++iRecord )
		{
			if( !m_aAITargetSelectSetRecords[iRecord].strName.empty() )
			{
				// Found a match. Include referenced TargetSelectSet's children.

				if( LTStrIEquals( pszInclude, m_aAITargetSelectSetRecords[iRecord].strName.c_str() ) )
				{
					*pTargetSelectMask |= m_aAITargetSelectSetRecords[iRecord].TargetSelectMask;
					CollapseIncludedTargetSelectSets( hLink, pTargetSelectMask );
					break;
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAIGoalRecords()
//
//	PURPOSE:	Create AIGoal records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAIGoalRecords()
{
	HCATEGORY hCatAIGoals = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_GOAL_CATEGORY );
	if( !hCatAIGoals )
	{
		return;
	}

	// Get the total record count.

	uint32 cAIGoalRecords = g_pLTDatabase->GetNumRecords( hCatAIGoals );
	if( !cAIGoalRecords )
	{
		return;
	}


	const char* pszGoalType;
	EnumAIGoalType eGoalType;
	const char* pszGoalClass;
	const char* pszContext;
	const char* pszNodeType;
	LTVector2 v2RecalcRate;
	uint32 cSensors;
	uint32 iSensor;
	const char* pszSensor;
	const char* pszSmartObject;
	EnumAISensorType eSensorType;

	// Read in each record.

	HRECORD hRecord;
	HRECORD hLink;
	HATTRIBUTE hAtt;
	for( uint32 iAIGoal=0; iAIGoal < cAIGoalRecords; ++iAIGoal )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIGoals, iAIGoal );
		if( !hRecord )
		{
			continue;
		}

		// Goal type.

		pszGoalType = g_pLTDatabase->GetRecordName( hRecord );
		eGoalType = CAIGoalAbstract::GetGoalType( pszGoalType );
		if( eGoalType == kGoal_InvalidType )
		{
			AIASSERT1( 0, NULL, "CAIDB::CreateAIGoalRecords: Unrecognized goal type: %s", pszGoalType );
			continue;
		}
		AIDB_GoalRecord& Record = m_aAIGoalRecords[eGoalType];
		Record.eGoalType = eGoalType;

		// Goal class.

		pszGoalClass = GetString( hRecord, AIDB_GOAL_sClass );
		if( !LTStrIEquals( pszGoalClass, "None" ) )
		{
			Record.eGoalClass = CAIGoalAbstract::GetGoalType( pszGoalClass );
		}

		// Context.

		pszContext = GetString( hRecord, AIDB_GOAL_sContext );
		Record.eAIContext = AIContextUtils::Enum( pszContext, AIContextUtils::kNotFound_Add );

		// Relevance.

		Record.fIntrinsicRelevance = GetFloat( hRecord, AIDB_GOAL_fRelevance );

		// Re-eval on satisfaction.

		Record.bReEvalOnSatisfaction = GetBool( hRecord, AIDB_GOAL_bReEvalOnSatisfaction );

		// Node type.

		pszNodeType = GetString( hRecord, AIDB_GOAL_sNodeType );
		if( !LTStrIEquals( pszNodeType, "None" ) )
		{
			Record.eNodeType = AINodeUtils::GetNodeType( pszNodeType );
		}

		// SmartObject.

		hLink = GetRecordLink( hRecord, AIDB_GOAL_sSmartObject );
		if( hLink )
		{
			pszSmartObject = g_pLTDatabase->GetRecordName( hLink );
			Record.eSmartObjectID = GetAISmartObjectRecordID( pszSmartObject );
		}
		else
		{
			Record.eSmartObjectID = kAISmartObjectID_Invalid;
		}

		// Sensors

		hAtt = g_pLTDatabase->GetAttribute( hRecord, AIDB_GOAL_sSensor );
		cSensors = g_pLTDatabase->GetNumValues( hAtt );
		for( iSensor=0; iSensor < cSensors; ++iSensor )
		{
			hLink = GetRecordLink( hRecord, AIDB_GOAL_sSensor, iSensor );
			if( hLink )
			{
				pszSensor = g_pLTDatabase->GetRecordName( hLink );
				eSensorType = (EnumAISensorType)String2EnumIndex( pszSensor, kSensor_Count, (uint32)kSensor_InvalidType, s_aszSensorTypes );
				if (eSensorType == kSensor_InvalidType)
				{
					AIASSERT1( 0, NULL, "CAIDB::CreateAIGoalRecords: No sensor named %s", (pszSensor ? pszSensor : "<null>") );
					continue;
				}

				Record.lstSensorTypes.push_back( eSensorType );
			}
		}

		// Recalc rate.

		v2RecalcRate = GetVector2( hRecord, AIDB_GOAL_v2RecalcRate );
		Record.fRecalcRateMin = v2RecalcRate.x;
		Record.fRecalcRateMax = v2RecalcRate.y;

		// Activate chance.

		Record.fActivateChance = GetFloat( hRecord, AIDB_GOAL_fActivateChance );

		// Frequency.

		Record.fFrequency = GetFloat( hRecord, AIDB_GOAL_fFrequency );

		// InterruptPriority.

		Record.fInterruptPriority = GetFloat( hRecord, AIDB_GOAL_fInterruptPriority );

		// CanReactivateDuringTransitions
	
		Record.bCanReactivateDuringTransitions = GetBool( hRecord, AIDB_GOAL_bCanReactivateDuringTransitions );

		// Read the min and max awareness's

		const char* const pszMinAwareness = GetString( hRecord, AIDB_GOAL_sMinAwareness );
		if( LTStrIEquals( pszMinAwareness, "None" ) )
		{
			Record.eMinAwareness = kAware_Relaxed;
		}
		else
		{
			Record.eMinAwareness = StringToAwareness( pszMinAwareness );
		}

		const char* const pszMaxAwareness = GetString( hRecord, AIDB_GOAL_sMaxAwareness );
		if( LTStrIEquals( pszMaxAwareness, "None" ) )
		{
			Record.eMinAwareness = kAware_Alert;
		}
		else
		{
			Record.eMaxAwareness = StringToAwareness( pszMaxAwareness );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAIGoalSetRecords()
//
//	PURPOSE:	Create AIGoalSet records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAIGoalSetRecords()
{
	HCATEGORY hCatAIGoalSets = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_GOALSET_CATEGORY );
	if( !hCatAIGoalSets )
	{
		return;
	}

	// Create an array as big as the number of records.

	m_cAIGoalSetRecords = g_pLTDatabase->GetNumRecords( hCatAIGoalSets );
	if( !m_cAIGoalSetRecords )
	{
		return;
	}
	m_aAIGoalSetRecords = debug_newa( AIDB_GoalSetRecord, m_cAIGoalSetRecords );

	uint32 cAITypes;
	uint32 iAIType;
	const char* pszAIType;
	ENUM_AIAttributesID eAIType;
	uint32 cGoals;
	uint32 iGoal;
	const char* pszGoal;
	EnumAIGoalType eGoalType;

	//
	// Read records in two passes, because records may include other records.
	// We need to ensure all records exist before we create references.
	//

	// First pass: Read in each record.

	HRECORD hRecord;
	HRECORD hLink;
	HATTRIBUTE hAtt;
	uint32 iAIGoalSet;
	for( iAIGoalSet=0; iAIGoalSet < m_cAIGoalSetRecords; ++iAIGoalSet )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIGoalSets, iAIGoalSet );
		if( !hRecord )
		{
			continue;
		}

		AIDB_GoalSetRecord& Record = m_aAIGoalSetRecords[iAIGoalSet];
		Record.eGoalSet = (ENUM_AIGoalSetID)iAIGoalSet;

		// GoalSet name.

		Record.strName = g_pLTDatabase->GetRecordName( hRecord );
		if( Record.strName.empty() )
		{
			AIASSERT( 0, NULL, "CAIDB::CreateAIGoalSetRecords: GoalSet missing name" );
			continue;
		}

		// Required AI types.

		hAtt = g_pLTDatabase->GetAttribute( hRecord, AIDB_GOALSET_sRequiredAIType );
		cAITypes = g_pLTDatabase->GetNumValues( hAtt );
		for( iAIType=0; iAIType < cAITypes; ++iAIType )
		{
			hLink = GetRecordLink( hRecord, AIDB_GOALSET_sRequiredAIType, iAIType );
			if( !hLink )
			{
				continue;
			}

			pszAIType = g_pLTDatabase->GetRecordName( hLink );
			eAIType = g_pAIDB->GetAIAttributesRecordID( pszAIType );
			if( eAIType != kAIAttributesID_Invalid )
			{
				Record.lstRequiredAITypes.push_back( eAIType );
			}
			else {
				AIASSERT1( 0, NULL, "CAIDB::CreateAIGoalSetRecords: Failed to find AI Type named %s", pszAIType );
			}
		}

		// Hidden.

		Record.dwGoalSetFlags = AIDB_GoalSetRecord::kGS_None;
		if( GetBool( hRecord, AIDB_GOALSET_bHidden ) )
		{
			Record.dwGoalSetFlags |= AIDB_GoalSetRecord::kGS_Hidden;
		}

		// Permanent.

		if( GetBool( hRecord, AIDB_GOALSET_bPermanent ) )
		{
			Record.dwGoalSetFlags |= AIDB_GoalSetRecord::kGS_Permanent;
		}

		// Goals.

		hAtt = g_pLTDatabase->GetAttribute( hRecord, AIDB_GOALSET_sGoal );
		cGoals = g_pLTDatabase->GetNumValues( hAtt );
		for( iGoal=0; iGoal < cGoals; ++iGoal )
		{
			hLink = GetRecordLink( hRecord, AIDB_GOALSET_sGoal, iGoal );
			if( !hLink )
			{
				continue;
			}

			pszGoal = g_pLTDatabase->GetRecordName( hLink );
			eGoalType = CAIGoalAbstract::GetGoalType( pszGoal );

			if( eGoalType == kGoal_InvalidType )
			{
				AIASSERT1( 0, NULL, "CAIDB::CreateAIGoalSetRecords: Unrecognized Goal type: %s", pszGoal );
				continue;
			}

			// Add Goal to list.

			Record.lstGoalSet.push_back( eGoalType );
		}
	}

	// Second pass: Reference included records.

	for( iAIGoalSet=0; iAIGoalSet < m_cAIGoalSetRecords; ++iAIGoalSet )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIGoalSets, iAIGoalSet );
		CollapseIncludedGoalSets( hRecord, &( m_aAIGoalSetRecords[iAIGoalSet].lstGoalSet ) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CollapseIncludedGoalSets()
//
//	PURPOSE:	Recursively include Goals from included GoalSets.
//
// ----------------------------------------------------------------------- //

void CAIDB::CollapseIncludedGoalSets( HRECORD hRecord, AIGOAL_TYPE_LIST* plstGoalSet )
{
	if( !( hRecord && plstGoalSet ) )
	{
		return;
	}

	// Included ActionSets.

	HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute( hRecord, AIDB_GOALSET_sIncludeGoalSet );
	uint32 cIncludes = g_pLTDatabase->GetNumValues( hAtt );

	// For each include, find a record with a matching GoalSet name.

	HRECORD hLink;
	uint32 iRecord;
	const char* pszInclude;
	for( uint32 iInclude=0; iInclude < cIncludes; ++iInclude )
	{
		hLink = GetRecordLink( hRecord, AIDB_GOALSET_sIncludeGoalSet, iInclude );
		if( !hLink )
		{
			continue;
		}

		pszInclude = g_pLTDatabase->GetRecordName( hLink );
		if( !pszInclude[0] )
		{
			continue;
		}

		for( iRecord=0; iRecord < m_cAIGoalSetRecords; ++iRecord )
		{
			if( !m_aAIGoalSetRecords[iRecord].strName.empty() )
			{
				// Found a match. Include referenced GoalSet's children.

				if( LTStrIEquals( pszInclude, m_aAIGoalSetRecords[iRecord].strName.c_str() ) )
				{
					plstGoalSet->insert( plstGoalSet->begin(), 
										 m_aAIGoalSetRecords[iRecord].lstGoalSet.begin(),
										 m_aAIGoalSetRecords[iRecord].lstGoalSet.end() );
					CollapseIncludedGoalSets( hLink, plstGoalSet );
					break;
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAIMovementRecords()
//
//	PURPOSE:	Create AIMovement records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAIMovementRecords()
{
	HCATEGORY hCatAIMovements = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_MOVEMENT_CATEGORY );
	if( !hCatAIMovements )
	{
		return;
	}

	// Create an array as big as the number of records.

	m_cAIMovementRecords = g_pLTDatabase->GetNumRecords( hCatAIMovements );
	if( !m_cAIMovementRecords )
	{
		return;
	}
	m_aAIMovementRecords = debug_newa( AIDB_MovementRecord, m_cAIMovementRecords );


	const char* pszContext;
	const char* pszProp;
	const char* pszAwareness;
	const char* pszAwarenessMod;
	EnumAnimProp eProp;
	uint32 cOverrides;
	uint32 iOverride;

	// Read in each record.

	HRECORD hRecord;
	char szAtt[80];
	for( uint32 iAIMovement=0; iAIMovement < m_cAIMovementRecords; ++iAIMovement )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIMovements, iAIMovement );
		if( !hRecord )
		{
			continue;
		}

		AIDB_MovementRecord& Record = m_aAIMovementRecords[iAIMovement];
		Record.eAIMovement = (ENUM_AIMovementID)iAIMovement;

		// Movement name.

		Record.strName = g_pLTDatabase->GetRecordName( hRecord );
		if( Record.strName.empty() )
		{
			AIASSERT( 0, NULL, "CAIDB::CreateAIMovementRecords: Movement missing name" );
			continue;
		}

		// Context.

		pszContext = GetString( hRecord, AIDB_MOVEMENT_sContext );
		Record.eAIContext = AIContextUtils::Enum( pszContext, AIContextUtils::kNotFound_Add );

		// Relaxed anim props.

		LTSNPrintF( &szAtt[0], LTARRAYSIZE(szAtt), "%s.0.%s", AIDB_MOVEMENT_sRelaxed, s_aszAnimPropGroup[kAPG_Activity] );
		pszProp = GetString( hRecord, szAtt );
		eProp = AnimPropUtils::Enum( pszProp );
		Record.Props[kAware_Relaxed].Set( kAPG_Activity, eProp );

		LTSNPrintF( &szAtt[0], LTARRAYSIZE(szAtt), "%s.0.%s", AIDB_MOVEMENT_sRelaxed, s_aszAnimPropGroup[kAPG_Movement] );
		pszProp = GetString( hRecord, szAtt );
		eProp = AnimPropUtils::Enum( pszProp );
		Record.Props[kAware_Relaxed].Set( kAPG_Movement, eProp );

		LTSNPrintF( &szAtt[0], LTARRAYSIZE(szAtt), "%s.0.%s", AIDB_MOVEMENT_sRelaxed, s_aszAnimPropGroup[kAPG_Action] );
		pszProp = GetString( hRecord, szAtt );
		eProp = AnimPropUtils::Enum( pszProp );
		Record.Props[kAware_Relaxed].Set( kAPG_Action, eProp );

		// Suspicious anim props.

		LTSNPrintF( &szAtt[0], LTARRAYSIZE(szAtt), "%s.0.%s", AIDB_MOVEMENT_sSuspicious, s_aszAnimPropGroup[kAPG_Activity] );
		pszProp = GetString( hRecord, szAtt );
		eProp = AnimPropUtils::Enum( pszProp );
		Record.Props[kAware_Suspicious].Set( kAPG_Activity, eProp );

		LTSNPrintF( &szAtt[0], LTARRAYSIZE(szAtt), "%s.0.%s", AIDB_MOVEMENT_sSuspicious, s_aszAnimPropGroup[kAPG_Movement] );
		pszProp = GetString( hRecord, szAtt );
		eProp = AnimPropUtils::Enum( pszProp );
		Record.Props[kAware_Suspicious].Set( kAPG_Movement, eProp );

		LTSNPrintF( &szAtt[0], LTARRAYSIZE(szAtt), "%s.0.%s", AIDB_MOVEMENT_sSuspicious, s_aszAnimPropGroup[kAPG_Action] );
		pszProp = GetString( hRecord, szAtt );
		eProp = AnimPropUtils::Enum( pszProp );
		Record.Props[kAware_Suspicious].Set( kAPG_Action, eProp );

		// Alert anim props.

		LTSNPrintF( &szAtt[0], LTARRAYSIZE(szAtt), "%s.0.%s", AIDB_MOVEMENT_sAlert, s_aszAnimPropGroup[kAPG_Activity] );
		pszProp = GetString( hRecord, szAtt );
		eProp = AnimPropUtils::Enum( pszProp );
		Record.Props[kAware_Alert].Set( kAPG_Activity, eProp );

		LTSNPrintF( &szAtt[0], LTARRAYSIZE(szAtt), "%s.0.%s", AIDB_MOVEMENT_sAlert, s_aszAnimPropGroup[kAPG_Movement] );
		pszProp = GetString( hRecord, szAtt );
		eProp = AnimPropUtils::Enum( pszProp );
		Record.Props[kAware_Alert].Set( kAPG_Movement, eProp );

		LTSNPrintF( &szAtt[0], LTARRAYSIZE(szAtt), "%s.0.%s", AIDB_MOVEMENT_sAlert, s_aszAnimPropGroup[kAPG_Action] );
		pszProp = GetString( hRecord, szAtt );
		eProp = AnimPropUtils::Enum( pszProp );
		Record.Props[kAware_Alert].Set( kAPG_Action, eProp );

		// Overrides.

		cOverrides = CountMovementOverrides( hRecord );
		Record.lstMovementOverrides.resize( cOverrides );

		AIMovementOverride* pOverride;
		for( iOverride=0; iOverride < cOverrides; ++iOverride )
		{
			pOverride = &( Record.lstMovementOverrides[iOverride] );

			// Awareness.

			LTSNPrintF( &szAtt[0], LTARRAYSIZE(szAtt), AIDB_MOVEMENT_sOverrideAwareness, iOverride );
			pszAwareness = GetString( hRecord, szAtt );
			pOverride->eAwareness = StringToAwareness( pszAwareness );

			// Awareness modifier.

			LTSNPrintF( &szAtt[0], LTARRAYSIZE(szAtt), AIDB_MOVEMENT_sOverrideAwarenessModifier, iOverride );
			pszAwarenessMod = GetString( hRecord, szAtt );
			pOverride->eAwarenessMod = (EnumAIAwarenessMod)String2EnumIndex( pszAwarenessMod, kAwarenessMod_Count, (uint32)kAwarenessMod_Invalid, s_aszAIAwarenessMods );

			// Anim props.

			LTSNPrintF( &szAtt[0], LTARRAYSIZE(szAtt), AIDB_MOVEMENT_sOverrideAnimProps, iOverride, s_aszAnimPropGroup[kAPG_Activity] );
			pszProp = GetString( hRecord, szAtt );
			eProp = AnimPropUtils::Enum( pszProp );
			pOverride->Props.Set( kAPG_Activity, eProp );

			LTSNPrintF( &szAtt[0], LTARRAYSIZE(szAtt), AIDB_MOVEMENT_sOverrideAnimProps, iOverride, s_aszAnimPropGroup[kAPG_Movement] );
			pszProp = GetString( hRecord, szAtt );
			eProp = AnimPropUtils::Enum( pszProp );
			pOverride->Props.Set( kAPG_Movement, eProp );

			LTSNPrintF( &szAtt[0], LTARRAYSIZE(szAtt), AIDB_MOVEMENT_sOverrideAnimProps, iOverride, s_aszAnimPropGroup[kAPG_Action] );
			pszProp = GetString( hRecord, szAtt );
			eProp = AnimPropUtils::Enum( pszProp );
			pOverride->Props.Set( kAPG_Action, eProp );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CountMovementOverrides()
//
//	PURPOSE:	Return the number of movement overrides for a movement record.
//
// ----------------------------------------------------------------------- //

uint32 CAIDB::CountMovementOverrides( HRECORD hRecord )
{
	uint32 cOverrides = 0;
	HATTRIBUTE hAtt;
	while( true )
	{
		// Determine if there is another override struct to read.  If not, break.

		char szBuffer[80];
		LTSNPrintF( &szBuffer[0], LTARRAYSIZE(szBuffer), AIDB_MOVEMENT_sOverrideAwareness, cOverrides );
		hAtt = g_pLTDatabase->GetAttribute( hRecord, szBuffer );
		if( !hAtt )
		{
			return cOverrides;
		}

		// Count the override.

		++cOverrides;
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAIMovementSetRecords()
//
//	PURPOSE:	Create AIMovementSet records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAIMovementSetRecords()
{
	HCATEGORY hCatAIMovementSets = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_MOVEMENTSET_CATEGORY );
	if( !hCatAIMovementSets )
	{
		return;
	}

	// Create an array as big as the number of records.

	m_cAIMovementSetRecords = g_pLTDatabase->GetNumRecords( hCatAIMovementSets );
	if( !m_cAIMovementSetRecords )
	{
		return;
	}
	m_aAIMovementSetRecords = debug_newa( AIDB_MovementSetRecord, m_cAIMovementSetRecords );


	const char* pszDefault;
	const char* pszMovement;
	uint32 cMovements;
	uint32 iMovement;
	ENUM_AIMovementID eAIMovement;

	// Read in each record.

	HRECORD hRecord;
	HRECORD hLink;
	HATTRIBUTE hAtt;
	for( uint32 iAIMovementSet=0; iAIMovementSet < m_cAIMovementSetRecords; ++iAIMovementSet )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIMovementSets, iAIMovementSet );
		if( !hRecord )
		{
			continue;
		}

		AIDB_MovementSetRecord& Record = m_aAIMovementSetRecords[iAIMovementSet];
		Record.eAIMovementSet = (ENUM_AIMovementSetID)iAIMovementSet;

		// Movement name.

		Record.strName = g_pLTDatabase->GetRecordName( hRecord );
		if( Record.strName.empty() )
		{
			AIASSERT( 0, NULL, "CAIDB::CreateAIMovementSetRecords: MovementSet missing name" );
			continue;
		}

		// Default.

		hLink = GetRecordLink( hRecord, AIDB_MOVEMENTSET_sDefault );
		if( hLink )
		{
			pszDefault = g_pLTDatabase->GetRecordName( hLink );
			Record.eDefaultAIMovement = GetAIMovementRecordID( pszDefault );
		}

		// Movements.

		hAtt = g_pLTDatabase->GetAttribute( hRecord, AIDB_MOVEMENTSET_sMovement );
		cMovements = g_pLTDatabase->GetNumValues( hAtt );
		for( iMovement=0; iMovement < cMovements; ++iMovement )
		{
			hLink = GetRecordLink( hRecord, AIDB_MOVEMENTSET_sMovement, iMovement );
			if( !hLink )
			{
				continue;
			}

			pszMovement = g_pLTDatabase->GetRecordName( hLink );
			eAIMovement = GetAIMovementRecordID( pszMovement );

			if( eAIMovement == kAIMovementID_Invalid )
			{
				AIASSERT1( 0, NULL, "CAIDB::CreateAIMovementSetRecords: Unrecognized Movement record: %s", pszMovement );
				continue;
			}

			// Add Movement to list.

			Record.lstAIMovementSet.push_back( eAIMovement );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAISmartObjectRecords()
//
//	PURPOSE:	Create AISmartObject records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAISmartObjectRecords()
{
	HCATEGORY hCatAISmartObjects = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_SMARTOBJECT_CATEGORY );
	if( !hCatAISmartObjects )
	{
		return;
	}

	// Create an array as big as the number of records.

	m_cAISmartObjectRecords = g_pLTDatabase->GetNumRecords( hCatAISmartObjects );
	if( !m_cAISmartObjectRecords )
	{
		return;
	}
	m_aAISmartObjectRecords = debug_newa( AIDB_SmartObjectRecord, m_cAISmartObjectRecords );


	const char* pszFlag;
	uint32 iGroup;
	const char* pszAnimProp;
	EnumAnimProp eProp;	
	LTVector2 v2LoopTime;
	LTVector2 v2FidgetFreq;
	const char* pszWeaponState;
	const char* pszDependencyType;
	uint32 cChildModels;
	uint32 iChildModel;
	AIChildModelInfo ChildModeInfo;

	// Read in each record.

	HRECORD hRecord;
	HATTRIBUTE hAtt;
	for( uint32 iAISmartObject=0; iAISmartObject < m_cAISmartObjectRecords; ++iAISmartObject )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAISmartObjects, iAISmartObject );
		if( !hRecord )
		{
			continue;
		}

		// Name.

		AIDB_SmartObjectRecord& Record = m_aAISmartObjectRecords[iAISmartObject];
		Record.strName = g_pLTDatabase->GetRecordName( hRecord );
		Record.eSmartObjectID = (ENUM_AISmartObjectID)iAISmartObject;

		// Flag.

		pszFlag = GetString( hRecord, AIDB_SMARTOBJECT_sFlag );
		Record.eNodeType = AINodeUtils::GetNodeType( pszFlag );

		// AnimProps.

		for( iGroup=0; iGroup < kAPG_Count; ++iGroup )
		{
			if( !g_pLTDatabase->GetAttribute( hRecord, s_aszAnimPropGroup[iGroup] ) )
			{
				continue;
			}

			pszAnimProp = GetString( hRecord, s_aszAnimPropGroup[iGroup] );
			eProp = AnimPropUtils::Enum( pszAnimProp );
			Record.Props.Set( (EnumAnimPropGroup)iGroup, eProp );
		}

		// Loop time.

		Record.bLooping = GetBool( hRecord, AIDB_SMARTOBJECT_bLooping );
		v2LoopTime = GetVector2( hRecord, AIDB_SMARTOBJECT_v2LoopTime );
		Record.fMinLoopTime = v2LoopTime.x;
		Record.fMaxLoopTime = v2LoopTime.y;

		// Fidget frequency.

		v2FidgetFreq = GetVector2( hRecord, AIDB_SMARTOBJECT_v2FidgetFreq );
		Record.fMinFidgetTime = v2FidgetFreq.x;
		Record.fMaxFidgetTime = v2FidgetFreq.y;

		// Timeout.

		Record.fTimeout = GetFloat( hRecord, AIDB_SMARTOBJECT_fTimeout );

		// Unpreferred time.

		Record.fUnpreferredTime = GetFloat( hRecord, AIDB_SMARTOBJECT_fUnpreferredTime );

		// Lock node.

		Record.bLockNode = GetBool( hRecord, AIDB_SMARTOBJECT_bLockNode );

		// Weapon state.

		pszWeaponState = GetString( hRecord, AIDB_SMARTOBJECT_sWeaponState );
		if( pszWeaponState[0] && LTStrIEquals( pszWeaponState, "Holstered" ) )
		{
			Record.eIsArmedRequirement = AIDB_SmartObjectRecord::kIsArmedRequirement_Holstered;
		}
		else if( pszWeaponState[0] && LTStrIEquals( pszWeaponState, "Drawn" ) )
		{
			Record.eIsArmedRequirement = AIDB_SmartObjectRecord::kIsArmedRequirement_Drawn;
		}
		else {
			Record.eIsArmedRequirement = AIDB_SmartObjectRecord::kIsArmedRequirement_Any;
		}

		// Dependency type.

		pszDependencyType = GetString( hRecord, AIDB_SMARTOBJECT_sDependencyType );
		Record.eDependencyType = (EnumAINodeDependencyType)String2EnumIndex( pszDependencyType, kDependency_Count, (uint32)kDependency_InvalidType, s_aszNodeDependencyTypes );

		// EntryOffsetDists.

		Record.fEntryOffsetDistA = GetFloat( hRecord, AIDB_SMARTOBJECT_fEntryOffsetDistA );
		Record.fEntryOffsetDistB = GetFloat( hRecord, AIDB_SMARTOBJECT_fEntryOffsetDistB );

		// ExitOffsetDists.

		Record.fExitOffsetDistA = GetFloat( hRecord, AIDB_SMARTOBJECT_fExitOffsetDistA );
		Record.fExitOffsetDistB = GetFloat( hRecord, AIDB_SMARTOBJECT_fExitOffsetDistB );

		// Min and max dists.

		Record.fMinDist = GetFloat( hRecord, AIDB_SMARTOBJECT_fMinDist );
		Record.fMaxDist = GetFloat( hRecord, AIDB_SMARTOBJECT_fMaxDist );

		// Node offset.

		Record.fNodeOffset = GetFloat( hRecord, AIDB_SMARTOBJECT_fNodeOffset );

		// FindFloor offset.

		Record.fFindFloorOffset = GetFloat( hRecord, AIDB_SMARTOBJECT_fFindFloorOffset );

		// Child models.

		hAtt = g_pLTDatabase->GetAttribute( hRecord, AIDB_SMARTOBJECT_sChildModelName );
		cChildModels = g_pLTDatabase->GetNumValues( hAtt );
		for( iChildModel=0; iChildModel < cChildModels; ++iChildModel )
		{
			ChildModeInfo.strFilename = GetString( hRecord, AIDB_SMARTOBJECT_sChildModelName, iChildModel );
			if( !ChildModeInfo.strFilename.empty() )
			{
				Record.lstChildModels.push_back( ChildModeInfo );
			}
		}

		// WorldEdit model.

		Record.strWorldEditModelName = GetString( hRecord, AIDB_SMARTOBJECT_sWorldEditModelName );

		// Action ability.

		const char* const pszActionAbilityRequired = GetString( hRecord, AIDB_SMARTOBJECT_sActionAbility );
		Record.eActionAbilityRequired = (ENUM_ActionAbility)String2EnumIndex( pszActionAbilityRequired, kActionAbility_Count, (uint32)kActionAbility_InvalidType, s_aszActionAbilityTypes );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAIWeaponRecords()
//
//	PURPOSE:	Create AIWeapon records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAIWeaponRecords()
{
	HCATEGORY hCatAIWeapons = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_WEAPON_CATEGORY );
	if( !hCatAIWeapons )
	{
		return;
	}

	// Create an array as big as the number of records.

	m_cAIWeaponRecords = g_pLTDatabase->GetNumRecords( hCatAIWeapons );
	if( !m_cAIWeaponRecords )
	{
		return;
	}
	m_aAIWeaponRecords = debug_newa( AIDB_AIWeaponRecord, m_cAIWeaponRecords );


	const char* pszClass;
	const char* pszType;
	const char* pszAnimProp;
	LTVector2 v2Range;
	LTVector2 v2BurstInterval;
	LTVector2 v2BurstShots;

	// Read in each record.

	HRECORD hRecord;
	for( uint32 iAIWeapon=0; iAIWeapon < m_cAIWeaponRecords; ++iAIWeapon )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIWeapons, iAIWeapon );
		if( !hRecord )
		{
			continue;
		}

		// Name.

		AIDB_AIWeaponRecord& Record = m_aAIWeaponRecords[iAIWeapon];
		Record.strName = g_pLTDatabase->GetRecordName( hRecord );
		Record.eAIWeaponID = (ENUM_AIWeaponID)iAIWeapon;

		// Class (specifies the C++ class used for the handler)

		pszClass = GetString( hRecord, AIDB_WEAPON_sClass );
		Record.eAIWeaponClass = (EnumAIWeaponClassType)String2EnumIndex( pszClass, kAIWeaponClassType_Count, (uint32)kAIWeaponClassType_InvalidType, s_aszAIWeaponClassNames );

		// Type (Defines the 'slot' in the AIWeaponMgr and the basic usage 
		// by the AI)

		pszType = GetString( hRecord, AIDB_WEAPON_sType );
		Record.eAIWeaponType = (ENUM_AIWeaponType)String2EnumIndex( pszType, kAIWeaponType_Count, (uint32)kAIWeaponType_None, s_aszAIWeaponTypeNames );

		// Anim prop.

		pszAnimProp = GetString( hRecord, AIDB_WEAPON_sAnimProp );
		Record.eAIWeaponAnimProp = AnimPropUtils::Enum( pszAnimProp );

		// Change weapon action prop.

		pszAnimProp = GetString( hRecord, AIDB_WEAPON_sChangeWeaponActionAnimProp );
		Record.eAIChangeWeaponActionAnimProp = AnimPropUtils::Enum( pszAnimProp );
		
		// Anim priority.

		Record.nAIWeaponAnimPriority = GetInt32( hRecord, AIDB_WEAPON_nAnimPriority );

		// Range.

		v2Range = GetVector2( hRecord, AIDB_WEAPON_v2Range );
		Record.fMinRange = v2Range.x;
		Record.fMaxRange = v2Range.y;
		Record.fMinRangeSqr = Record.fMinRange * Record.fMinRange;
		Record.fMaxRangeSqr = Record.fMaxRange * Record.fMaxRange;

		// Allow directional run.

		Record.bAllowDirectionalRun = GetBool( hRecord, AIDB_WEAPON_bAllowDirectionalRun );

		// Is dangerous.

		Record.bIsDangerous = GetBool( hRecord, AIDB_WEAPON_bIsDangerous );

		// AI animates reload.

		Record.bAIAnimatesReload = GetBool( hRecord, AIDB_WEAPON_bAIAnimatesReload );

		// Force miss to floor.

		Record.bForceMissToFloor = GetBool( hRecord, AIDB_WEAPON_bForceMissToFloor );

		// Burst interval.

		v2BurstInterval = GetVector2( hRecord, AIDB_WEAPON_v2BurstInterval );
		Record.fAIMinBurstInterval = v2BurstInterval.x;
		Record.fAIMaxBurstInterval = v2BurstInterval.y;
		Record.bStrictBursts = GetBool( hRecord, AIDB_WEAPON_bStrictBursts );
		Record.bSuppressionBursts = GetBool( hRecord, AIDB_WEAPON_bSuppressionBursts );

		// Burst shots.

		v2BurstShots = GetVector2( hRecord, AIDB_WEAPON_v2BurstShots );
		Record.nAIMinBurstShots = (uint32)v2BurstShots.x;
		Record.nAIMaxBurstShots = (uint32)v2BurstShots.y;

		// Player pusher.

		Record.fPlayerPusherRadius = GetFloat( hRecord, AIDB_WEAPON_fPlayerPusherRadius );
		Record.fPlayerPusherForce = GetFloat( hRecord, AIDB_WEAPON_fPlayerPusherForce );

		// Allow the AI to generate ammo

		Record.bAllowAmmoGeneration = GetBool( hRecord, AIDB_WEAPON_bAllowAmmoGeneration );

		// Allow the AI to automatically reload their weapon in certain 
		// cinematic situations.  If an AI respecting his weapons clip size 
		// is important, this should not be used.

		Record.bAllowAutoReload = GetBool( hRecord, AIDB_WEAPON_bAllowAmmoGeneration );

		// Read the value of this weapon to the AI.

		Record.nPreference = GetInt32( hRecord, AIDB_WEAPON_nPreference );

		// Accuracy scalar.

		LTVector2 v2Accuracy = GetVector2( hRecord, AIDB_WEAPON_rAccuracyScalar );
		Record.fAccuracyScalarMin = LTMAX( v2Accuracy.x, v2Accuracy.y );
		Record.fAccuracyScalarMax = LTMIN( v2Accuracy.x, v2Accuracy.y );

		// Damage scalar.

		LTVector2 v2Damage = GetVector2( hRecord, AIDB_WEAPON_rDamageScalar );
		Record.fDamageScalarMin = LTMAX( v2Damage.x, v2Damage.y );
		Record.fDamageScalarMax = LTMIN( v2Damage.x, v2Damage.y );

		// Does the weapon sync with the users animation by name

		Record.bSyncToUserAnimation = GetBool(hRecord, AIDB_WEAPON_bSyncToUserAnimation);

		// Probability AI will fire this weapon while dying.

		Record.fFireDuringDeathProbability = GetFloat( hRecord, AIDB_WEAPON_fFireDuringDeathProbability );

		// True if weapon fire should be restricted to a 60 degree cone.

		Record.bRestrictFireCone = GetBool(hRecord, AIDB_WEAPON_bRestrictFireCone);

		// When an AI attempts to fire, we prevent it (forcing him into aim) 
		// if the socket the weapon is attached to does not point towards the 
		// enemy.  In the case of some weapons, the socket may not point 
		// forward (ie holding a shotgun).  To work around this, provide a 
		// rotational offset.  Ideally, we would have a 'fire node', similar 
		// to the 'aimer node' to do this all in content.

		LTVector vOffsetAngles = GetVector3( hRecord, AIDB_WEAPON_vFireRotationOffset );
		Record.mFireRotationOffset = LTRotation( 
			DEG2RAD(vOffsetAngles.x), DEG2RAD(vOffsetAngles.y), DEG2RAD(vOffsetAngles.z) );

		// True if the fire animation should be looped/unlocked, false if the 
		// animation should be played through once/locked.

		Record.bLoopFireAnimation = GetBool(hRecord, AIDB_WEAPON_bLoopFireAnimation);

		// This is the similar to the arsenals WDB_GLOBAL_sDefaultAnimationName 
		// property; it defines the name of the animation to play if animation 
		// syncing is turned on but there is no match.  In the past, this was 
		// named the players animation was used.

		LTStrCpy( Record.szDefaultAnimationName, 
			GetString(hRecord, AIDB_WEAPON_szDefaultAnimationName, 0, "base" ),
			LTARRAYSIZE(Record.szDefaultAnimationName) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIWeaponOverrideSetRecordID
//
//	PURPOSE:	Return the AIWeaponContextID of the record with a matching name.
//
// ----------------------------------------------------------------------- //

ENUM_AIWeaponOverrideSetID CAIDB::GetAIWeaponOverrideSetRecordID( const char* const szName )
{
	for( uint32 iAIWeaponOverrideSet=0; iAIWeaponOverrideSet < m_cAIWeaponOverrideSetRecords; ++iAIWeaponOverrideSet )
	{
		if( LTStrIEquals( szName, m_aAIWeaponOverrideSetRecords[iAIWeaponOverrideSet].strName.c_str() ) )
		{
			return (ENUM_AIWeaponOverrideSetID)iAIWeaponOverrideSet;
		}
	}

	return kAIWeaponOverrideSetID_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetDroppedItemRecord
//
//	PURPOSE:	Return the dropped item record
//
// ----------------------------------------------------------------------- //

HRECORD CAIDB::GetDroppedItemRecord( const char* const szName )
{
	return g_pLTDatabase->GetRecord(m_hDatabase,AIDB_DROPPEDITEMS_CATEGORY,szName);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIWeaponOverrideSetRecord
//
//	PURPOSE:	Return a pointer to the specified 
//				AIWeaponOverrideSetRecord record.
//
// ----------------------------------------------------------------------- //

AIDB_AIWeaponOverrideSetRecord* CAIDB::GetAIWeaponOverrideSetRecord( ENUM_AIWeaponOverrideSetID eRecord )
{
	if( eRecord >= 0 && eRecord < (int)m_cAIWeaponOverrideSetRecords )
	{
		return &( m_aAIWeaponOverrideSetRecords[eRecord] );
	}

	// Record out of range.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAIWeaponOverrideSets()
//
//	PURPOSE:	Create AIAmmoLoad records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAIWeaponOverrideSets()
{
	HCATEGORY hCatAIWeaponOverrideSets = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_WEAPONOVERRIDESET_CATEGORY );
	if( !hCatAIWeaponOverrideSets )
	{
		return;
	}

	// Create an array as big as the number of records.

	m_cAIWeaponOverrideSetRecords = g_pLTDatabase->GetNumRecords( hCatAIWeaponOverrideSets );
	if( !m_cAIWeaponOverrideSetRecords )
	{
		return;
	}
	m_aAIWeaponOverrideSetRecords = debug_newa( AIDB_AIWeaponOverrideSetRecord, m_cAIWeaponOverrideSetRecords );

	for ( uint32 i = 0; i < m_cAIWeaponOverrideSetRecords; ++i )
	{
		HRECORD hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIWeaponOverrideSets, i );
		if( !hRecord )
		{
			continue;
		}

		AIDB_AIWeaponOverrideSetRecord& Record = m_aAIWeaponOverrideSetRecords[i];

		// Store the identity 

		const char* const pszRecordName = g_pLTDatabase->GetRecordName( hRecord );
		Record.strName = pszRecordName ? pszRecordName : "";

		// Read each of the overrides.

		for (int i = 0; ; ++i)
		{
			// Determine if there is another struct attribute to read.  If not, break.
			// If there is, read it.

			char szBuffer[80];

			LTSNPrintF(&szBuffer[0], LTARRAYSIZE(szBuffer), AIDB_WEAPONOVERRIDESET_hAIWeapon, i);
			HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute( hRecord, szBuffer );
			if ( NULL == hAtt )
			{
				break;
			}

			AIDB_AIWeaponOverrideSetRecord::WeaponPair Temp;

			// Read in the AI weapon

			LTSNPrintF(&szBuffer[0], LTARRAYSIZE(szBuffer), AIDB_WEAPONOVERRIDESET_hAIWeapon, i);
			HRECORD hAIWeapon = GetRecordLink(hRecord, szBuffer);
			Temp.eAIWeapon = GetAIWeaponRecordID( GetRecordName(hAIWeapon) );

			// Read in each of the bindings.  Print out a warning message if there are
			// multiple for a single weapon.

			LTSNPrintF(&szBuffer[0], LTARRAYSIZE(szBuffer), AIDB_WEAPONOVERRIDESET_hWeapon, i);
			
			int nValues = GetNumValues( hRecord, szBuffer );
			for ( int iValue = 0; iValue < nValues; ++iValue )
			{
				// Read in the Arsenal weapon

				Temp.hWeapon = GetRecordLink(hRecord, szBuffer, iValue);

				for ( uint32 iOverride = 0 ; iOverride < Record.AIWeaponOverrideList.size(); ++iOverride )
				{
					if ( Temp.hWeapon == Record.AIWeaponOverrideList[iOverride].hWeapon )
					{
						const char* const pszRecordName = GetRecordName( Temp.hWeapon );
						AIASSERT2(0, NULL, "CreateAIWeaponOverrideSets : Arsenal weapon associated with multiple AIWeapons.  Record: %s, Weapon: %s", 
						Record.strName.c_str(), pszRecordName ? pszRecordName : "<null>");
					}
				}

				// Add the ammo amount object to the list.

				Record.AIWeaponOverrideList.push_back(Temp);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAIAmmoLoadRecords()
//
//	PURPOSE:	Create AIAmmoLoad records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAIAmmoLoadRecords()
{
	HCATEGORY hCatAIAmmoLoads = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_AMMOLOAD_CATEGORY );
	if( !hCatAIAmmoLoads )
	{
		return;
	}

	// Create an array as big as the number of records.

	m_cAIAmmoLoadRecords = g_pLTDatabase->GetNumRecords( hCatAIAmmoLoads );
	if( !m_cAIAmmoLoadRecords )
	{
		return;
	}
	m_aAIAmmoLoadRecords = debug_newa( AIDB_AIAmmoLoadRecord, m_cAIAmmoLoadRecords );

        // Create each of the AmmoLoad records.

	for( uint32 iAIAmmoLoad=0; iAIAmmoLoad < m_cAIAmmoLoadRecords; ++iAIAmmoLoad )
	{
		HRECORD hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIAmmoLoads, iAIAmmoLoad );
		if( !hRecord )
		{
			continue;
		}

		AIDB_AIAmmoLoadRecord& Record = m_aAIAmmoLoadRecords[iAIAmmoLoad];

		// Set the name of the record.

		const char* const pszRecordName = g_pLTDatabase->GetRecordName( hRecord );
		Record.strName = pszRecordName ? pszRecordName : "";

		// Read in the ammo type/amount pairs.

		for (int i = 0; ; ++i)
		{
			// Determine if there is another struct attribute to read.  If not, break.
			// If there is, read it.

			char szBuffer[80];

			LTSNPrintF(&szBuffer[0], LTARRAYSIZE(szBuffer), AIDB_AMMOLOAD_sAmmo, i);
			HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute( hRecord, szBuffer );
			if ( NULL == hAtt )
			{
				break;
			}

			AIDB_AIAmmoLoadRecord::AmmoAmount Temp;

			// Read in the AmmoName

			Temp.m_hAmmo = GetRecordLink(hRecord, szBuffer);

			// Read in the ammo count.

			LTSNPrintF(&szBuffer[0], LTARRAYSIZE(szBuffer), AIDB_AMMOLOAD_nAmount, i);
			Temp.m_nRounds = GetInt32(hRecord, szBuffer);

			// Add the ammo amount object to the list.

			Record.m_AmmoAmountList.push_back(Temp);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAINodeRecords()
//
//	PURPOSE:	Create AINode records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAINodeRecords()
{
	HCATEGORY hCatAINodes = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_NODE_CATEGORY );
	if( !hCatAINodes )
	{
		return;
	}

	// Create an array as big as the number of records.  Default construct 
	// them to initialize them into a stable state in case the data for them
	// does not exist.  All values should be treated as overrides.

	m_cAINodeRecords = kNode_Count;
	m_aAINodeRecords = debug_newa( AIDB_AINodeRecord, kNode_Count );

	// Read in each of the nodes

	uint32 nRecords = g_pLTDatabase->GetNumRecords( hCatAINodes );
	for( uint32 iRecord=0; iRecord < nRecords; ++iRecord )
	{
		HRECORD hRecord = g_pLTDatabase->GetRecordByIndex( hCatAINodes, iRecord );
		if( !hRecord )
		{
			continue;
		}

		// Translate the name of the record into a node enum.

		const char* pszNodeClassName = g_pLTDatabase->GetRecordName( hRecord );
		EnumAINodeType eNodeType = AINodeUtils::GetNodeType( pszNodeClassName );
		if (kNode_InvalidType == eNodeType)
		{
			AIASSERT1(0, NULL, "CAIDB::CreateAINodeRecords : Unrecognized node class name: %s", pszNodeClassName);
			continue;
		}

		// Get the Record from the name.

		AIDB_AINodeRecord& Record = m_aAINodeRecords[eNodeType];
	
		// Convert the data.

		Record.dwRelevantNotAtNodeStatusFlags |= BuildMaskFromAttributeList(hRecord, AIDB_NODE_sNotAtNodeStatusFlags, s_aszAINodeStatusFlag, kNodeStatus_Count);
		Record.dwRelevantAtNodeStatusFlags |= BuildMaskFromAttributeList(hRecord, AIDB_NODE_sAtNodeStatusFlags, s_aszAINodeStatusFlag, kNodeStatus_Count);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::CreateAINavMeshTypeRecords()
//
//	PURPOSE:	Create AINavMeshType records.
//
// ----------------------------------------------------------------------- //

void CAIDB::CreateAINavMeshTypeRecords()
{
	HCATEGORY hCatAINavMeshTypes = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_AINAVMESHTYPE_CATEGORY );
	if( !hCatAINavMeshTypes )
	{
		return;
	}

	m_cAIAINavMeshTypeRecords = g_pLTDatabase->GetNumRecords( hCatAINavMeshTypes );
	m_aAIAINavMeshTypeRecords = debug_newa( AIDB_AINavMeshTypeRecord, m_cAIAINavMeshTypeRecords );

	for ( uint32 iRecord = 0; iRecord < m_cAIAINavMeshTypeRecords; ++iRecord )
	{
		AIDB_AINavMeshTypeRecord& rAINavMeshRecord = m_aAIAINavMeshTypeRecords[iRecord];
		HRECORD hRecord = g_pLTDatabase->GetRecordByIndex( hCatAINavMeshTypes, iRecord );

		const char* const pszRecordName = g_pLTDatabase->GetRecordName( hRecord );
		AIASSERT( pszRecordName, NULL, "CAIDB::CreateAINavMeshTypeRecords: Record has no name" );
	
		// Copy the record name.

		LTStrCpy( rAINavMeshRecord.m_szName, pszRecordName, LTARRAYSIZE( rAINavMeshRecord.m_szName ) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::BuildMaskFromAttributeList()
//
//	PURPOSE:	Returns a uint32 mask representing the elements present in 
//				an attribute list.  Given a record, it gets each of the 
//				values for a named attribute, calls String2Flag to convert
//				each of the names to a flag, then adds it to the mask.
//
// ----------------------------------------------------------------------- //

uint32 CAIDB::BuildMaskFromAttributeList(HRECORD hRecord, const char* const pszAttributeName, const char** pszStringList, const int nStringListSize)
{
	uint32 nResultMask = 0;

	// Fill in the Record

	int cValues = g_pLTDatabase->GetNumValues( GetAttribute( hRecord, pszAttributeName ) );
	for( int iValue=0; iValue < cValues; ++iValue )
	{
		const char* const pszValue = GetString( hRecord, pszAttributeName, iValue );
		uint32 BitFlag = String2BitFlag( pszValue, nStringListSize, pszStringList);

		if (0 == BitFlag)
		{
			AIASSERT(0, NULL, "");
			continue;
		}

		// Add the flag to the relevant flag mask.

		nResultMask |= BitFlag;
	}

	return nResultMask;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::BuildBitsetFromAttributeList()
//
//	PURPOSE:	Returns a uint32 bitset representing the elements present in 
//				an attribute list.  Given a record, it gets each of the 
//				values for a named attribute, calls String2EnumIndex to convert
//				each of the names to a flag, then adds it to the bitset.
//
// ----------------------------------------------------------------------- //

uint32 CAIDB::BuildBitsetFromAttributeList(HRECORD hRecord, const char* const pszAttributeName, uint32 iInvalid, const char** pszStringList, const int nStringListSize)
{
	uint32 nResultBits = 0;

	// Fill in the Record

	int cValues = g_pLTDatabase->GetNumValues( GetAttribute( hRecord, pszAttributeName ) );
	for( int iValue=0; iValue < cValues; ++iValue )
	{
		const char* const pszValue = GetString( hRecord, pszAttributeName, iValue );
		if( LTStrIEquals( pszValue, "None" ) )
		{
			continue;
		}

		uint32 iEnumIndex = String2EnumIndex( pszValue, nStringListSize, iInvalid, pszStringList);

		if( iInvalid == iEnumIndex )
		{
			AIASSERT1( 0, NULL, "CAIDB::BuildBitsetFromAttributeList: Unrecognized enum '%s'", pszValue );
			continue;
		}

		// Add the flag to the relevant flag mask.

		nResultBits |= ( 1 << iEnumIndex );
	}

	return nResultBits;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIActionRecord
//
//	PURPOSE:	Return a pointer to the specified action record.
//
// ----------------------------------------------------------------------- //

AIDB_ActionRecord* CAIDB::GetAIActionRecord( uint32 iRecord )
{
	if( iRecord < m_cAIActionRecords )
	{
		return &( m_aAIActionRecords[iRecord] );
	}

	// Record out of range.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIActionSetRecord
//
//	PURPOSE:	Return a pointer to the specified action set record.
//
// ----------------------------------------------------------------------- //

AIDB_ActionSetRecord* CAIDB::GetAIActionSetRecord( uint32 iRecord )
{
	if( iRecord < m_cAIActionSetRecords )
	{
		return &( m_aAIActionSetRecords[iRecord] );
	}

	// Record out of range.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIActionSetRecordID
//
//	PURPOSE:	Return the ActionSet ID for the ActionSet with a matching name.
//
// ----------------------------------------------------------------------- //

ENUM_AIActionSet CAIDB::GetAIActionSetRecordID( const char* const szName )
{
	for( uint32 iActionSet = 0; iActionSet != m_cAIActionSetRecords; ++iActionSet )
	{
		if( LTStrIEquals( szName, m_aAIActionSetRecords[iActionSet].strName.c_str() ) )
		{
			return m_aAIActionSetRecords[iActionSet].eActionSet;
		}
	}

	return kAIActionSet_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIActionSetRecordName
//
//	PURPOSE:	Return the ActionSet Name for the ActionSet with a matching ID.
//
// ----------------------------------------------------------------------- //

const char* CAIDB::GetAIActionSetRecordName( ENUM_AIActionSet eActionSet )
{
	if( ( eActionSet > kAIActionSet_Invalid ) &&
		( (uint32)eActionSet < m_cAIActionSetRecords ) )
	{
		return m_aAIActionSetRecords[eActionSet].strName.c_str();
	}

	return "";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIActivitySetRecord
//
//	PURPOSE:	Return a pointer to the specified activity set record.
//
// ----------------------------------------------------------------------- //

AIDB_ActivitySetRecord* CAIDB::GetAIActivitySetRecord( uint32 iRecord )
{
	if( iRecord < m_cAIActivitySetRecords )
	{
		return &( m_aAIActivitySetRecords[iRecord] );
	}

	// Record out of range.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIActivitySetRecordID
//
//	PURPOSE:	Return the ActivitySet ID for the ActivitySet with a matching name.
//
// ----------------------------------------------------------------------- //

ENUM_AIActivitySet CAIDB::GetAIActivitySetRecordID( const char* const szName )
{
	for( uint32 iActivitySet = 0; iActivitySet != m_cAIActivitySetRecords; ++iActivitySet )
	{
		if( LTStrIEquals( szName, m_aAIActivitySetRecords[iActivitySet].strName.c_str() ) )
		{
			return m_aAIActivitySetRecords[iActivitySet].eActivitySet;
		}
	}

	return kAIActivitySet_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIActivitySetRecordName
//
//	PURPOSE:	Return the ActivitySet Name for the ActivitySet with a matching ID.
//
// ----------------------------------------------------------------------- //

const char* CAIDB::GetAIActivitySetRecordName( ENUM_AIActivitySet eActivitySet )
{
	if( ( eActivitySet > kAIActivitySet_Invalid ) &&
		( (uint32)eActivitySet < m_cAIActivitySetRecords ) )
	{
		return m_aAIActivitySetRecords[eActivitySet].strName.c_str();
	}

	return "";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIAttributesRecord
//
//	PURPOSE:	Return a pointer to the specified attributes record.
//
// ----------------------------------------------------------------------- //

AIDB_AttributesRecord* CAIDB::GetAIAttributesRecord( uint32 iRecord )
{
	if( iRecord < m_cAIAttributesRecords )
	{
		return &( m_aAIAttributesRecords[iRecord] );
	}

	// Record out of range.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIAttributesRecordID
//
//	PURPOSE:	Return the AIAttributes ID for the AIAttributes record with a matching name.
//
// ----------------------------------------------------------------------- //

ENUM_AIAttributesID CAIDB::GetAIAttributesRecordID( const char* const szName )
{
	for( uint32 iRecord=0; iRecord < m_cAIAttributesRecords; ++iRecord )
	{
		if( LTStrIEquals( szName, m_aAIAttributesRecords[iRecord].strName.c_str() ) )
		{
			return m_aAIAttributesRecords[iRecord].eAIAttributesID;
		}
	}

	return kAIAttributesID_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAILimitsRecord
//
//	PURPOSE:	Return a pointer to the specified limits record.
//
// ----------------------------------------------------------------------- //

AIDB_LimitsRecord* CAIDB::GetAILimitsRecord( uint32 iRecord )
{
	if( iRecord < m_cAILimitsRecords )
	{
		return &( m_aAILimitsRecords[iRecord] );
	}

	// Record out of range.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAILimitsRecordID
//
//	PURPOSE:	Return the AILimits ID for the AILimits record with a matching name.
//
// ----------------------------------------------------------------------- //

ENUM_AILimitsID CAIDB::GetAILimitsRecordID( const char* const szName )
{
	for( uint32 iRecord=0; iRecord < m_cAILimitsRecords; ++iRecord )
	{
		if( LTStrIEquals( szName, m_aAILimitsRecords[iRecord].strName.c_str() ) )
		{
			return m_aAILimitsRecords[iRecord].eAILimitsID;
		}
	}

	return kAILimitsID_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIBrainRecord
//
//	PURPOSE:	Return a pointer to the specified brain record.
//
// ----------------------------------------------------------------------- //

AIDB_BrainRecord* CAIDB::GetAIBrainRecord( uint32 iRecord )
{
	if( iRecord < m_cAIBrainRecords )
	{
		return &( m_aAIBrainRecords[iRecord] );
	}

	// Record out of range.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIBrainRecordID
//
//	PURPOSE:	Return the AIBrain ID for the AIBrain with a matching name.
//
// ----------------------------------------------------------------------- //

ENUM_AIBrainID CAIDB::GetAIBrainRecordID( const char* const szName )
{
	for( uint32 iAIBrain=0; iAIBrain < m_cAIBrainRecords; ++iAIBrain )
	{
		if( LTStrIEquals( szName, m_aAIBrainRecords[iAIBrain].strName.c_str() ) )
		{
			return m_aAIBrainRecords[iAIBrain].eAIBrainID;
		}
	}

	return kAIBrainID_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIDamageMaskRecord
//
//	PURPOSE:	Return a pointer to the specified damage mask record.
//
// ----------------------------------------------------------------------- //

AIDB_DamageMaskRecord* CAIDB::GetAIDamageMaskRecord( uint32 iRecord )
{
	if( iRecord < m_cAIDamageMaskRecords )
	{
		return &( m_aAIDamageMaskRecords[iRecord] );
	}

	// Record out of range.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIDamageMaskRecordID
//
//	PURPOSE:	Return the AIDamageMask ID for the AIDamageMask with a matching name.
//
// ----------------------------------------------------------------------- //

ENUM_AIDamageMaskID CAIDB::GetAIDamageMaskRecordID( const char* const szName )
{
	for( uint32 iAIDamageMask=0; iAIDamageMask < m_cAIDamageMaskRecords; ++iAIDamageMask )
	{
		if( LTStrIEquals( szName, m_aAIDamageMaskRecords[iAIDamageMask].strName.c_str() ) )
		{
			return m_aAIDamageMaskRecords[iAIDamageMask].eAIDamageMaskID;
		}
	}

	return kAIDamageMaskID_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIStimulusRecord
//
//	PURPOSE:	Return a pointer to the specified activity set record.
//
// ----------------------------------------------------------------------- //

AIDB_StimulusRecord* CAIDB::GetAIStimulusRecord( EnumAIStimulusType eStimulusType )
{
	for( uint32 iStimulus=0; iStimulus < m_cAIStimulusRecords; ++iStimulus )
	{
		if( m_aAIStimulusRecords[iStimulus].eStimulusType == eStimulusType )
		{
			return &( m_aAIStimulusRecords[iStimulus] );
		}
	}

	// No match found.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAISensorRecord
//
//	PURPOSE:	Return a pointer to the specified sensor record.
//
// ----------------------------------------------------------------------- //

AIDB_SensorRecord* CAIDB::GetAISensorRecord( EnumAISensorType eSensorType )
{
	if( ( eSensorType > kSensor_InvalidType ) &&
		( eSensorType < kSensor_Count ) )
	{
		return &m_aAISensorRecords[eSensorType];
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAITargetSelectRecord
//
//	PURPOSE:	Return a pointer to the specified target record.
//
// ----------------------------------------------------------------------- //

AIDB_TargetSelectRecord* CAIDB::GetAITargetSelectRecord( uint32 iRecord )
{
	if( iRecord < m_cAITargetSelectRecords )
	{
		return &( m_aAITargetSelectRecords[iRecord] );
	}

	// Record out of range.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAITargetSelectSetRecord
//
//	PURPOSE:	Return a pointer to the specified target set record.
//
// ----------------------------------------------------------------------- //

AIDB_TargetSelectSetRecord* CAIDB::GetAITargetSelectSetRecord( uint32 iRecord )
{
	if( iRecord < m_cAITargetSelectSetRecords )
	{
		return &( m_aAITargetSelectSetRecords[iRecord] );
	}

	// Record out of range.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAITargetSelectSetRecordID
//
//	PURPOSE:	Return the TargetSet ID for the TargetSet with a matching name.
//
// ----------------------------------------------------------------------- //

ENUM_AITargetSelectSet CAIDB::GetAITargetSelectSetRecordID( const char* const szName )
{
	for( uint32 iTargetSet = 0; iTargetSet != m_cAITargetSelectSetRecords; ++iTargetSet )
	{
		if( LTStrIEquals( szName, m_aAITargetSelectSetRecords[iTargetSet].strName.c_str() ) )
		{
			return m_aAITargetSelectSetRecords[iTargetSet].eTargetSelectSet;
		}
	}

	return kAITargetSelectSet_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAITargetSelectSetRecordName
//
//	PURPOSE:	Return the TargetSet Name for the TargetSet with a matching ID.
//
// ----------------------------------------------------------------------- //

const char* CAIDB::GetAITargetSelectSetRecordName( ENUM_AITargetSelectSet eTargetSelectSet )
{
	if( ( eTargetSelectSet > kAITargetSelectSet_Invalid ) &&
		( (uint32)eTargetSelectSet < m_cAITargetSelectSetRecords ) )
	{
		return m_aAITargetSelectSetRecords[eTargetSelectSet].strName.c_str();
	}

	return "";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIGoalRecord
//
//	PURPOSE:	Return a pointer to the specified goal record.
//
// ----------------------------------------------------------------------- //

AIDB_GoalRecord* CAIDB::GetAIGoalRecord( EnumAIGoalType eGoalType )
{
	if( ( eGoalType > kGoal_InvalidType ) &&
		( eGoalType < kGoal_Count ) )
	{
		return &m_aAIGoalRecords[eGoalType];
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIGoalSetRecord
//
//	PURPOSE:	Return a pointer to the specified goal set record.
//
// ----------------------------------------------------------------------- //

AIDB_GoalSetRecord* CAIDB::GetAIGoalSetRecord( uint32 iRecord )
{
	if( iRecord < m_cAIGoalSetRecords )
	{
		return &( m_aAIGoalSetRecords[iRecord] );
	}

	// Record out of range.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIGoalSetRecordID
//
//	PURPOSE:	Return the AIGoalSet ID for the AIGoalSet with a matching name.
//
// ----------------------------------------------------------------------- //

ENUM_AIGoalSetID CAIDB::GetAIGoalSetRecordID( const char* const szName )
{
	for( uint32 iAIGoalSet=0; iAIGoalSet < m_cAIGoalSetRecords; ++iAIGoalSet )
	{
		if( LTStrIEquals( szName, m_aAIGoalSetRecords[iAIGoalSet].strName.c_str() ) )
		{
			return m_aAIGoalSetRecords[iAIGoalSet].eGoalSet;
		}
	}

	return kAIGoalSetID_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIGoalSetRecordName
//
//	PURPOSE:	Return the GoalSet Name for the GoalSet with a matching ID.
//
// ----------------------------------------------------------------------- //

const char* CAIDB::GetAIGoalSetRecordName( ENUM_AIGoalSetID eGoalSet )
{
	if( ( eGoalSet > kAIGoalSetID_Invalid ) &&
		( (uint32)eGoalSet < m_cAIGoalSetRecords ) )
	{
		return m_aAIGoalSetRecords[eGoalSet].strName.c_str();
	}

	return "";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIMovementRecord
//
//	PURPOSE:	Return a pointer to the specified movement record.
//
// ----------------------------------------------------------------------- //

AIDB_MovementRecord* CAIDB::GetAIMovementRecord( uint32 iRecord )
{
	if( iRecord < m_cAIMovementRecords )
	{
		return &( m_aAIMovementRecords[iRecord] );
	}

	// Record out of range.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIMovementRecordID
//
//	PURPOSE:	Return the AIMovement ID for the AIMovement record with a matching name.
//
// ----------------------------------------------------------------------- //

ENUM_AIMovementID CAIDB::GetAIMovementRecordID( const char* const szName )
{
	for( uint32 iAIMovement=0; iAIMovement < m_cAIMovementRecords; ++iAIMovement )
	{
		if( LTStrIEquals( szName, m_aAIMovementRecords[iAIMovement].strName.c_str() ) )
		{
			return m_aAIMovementRecords[iAIMovement].eAIMovement;
		}
	}

	return kAIMovementID_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIMovementSetRecord
//
//	PURPOSE:	Return a pointer to the specified movement set record.
//
// ----------------------------------------------------------------------- //

AIDB_MovementSetRecord* CAIDB::GetAIMovementSetRecord( uint32 iRecord )
{
	if( iRecord < m_cAIMovementSetRecords )
	{
		return &( m_aAIMovementSetRecords[iRecord] );
	}

	// Record out of range.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIMovementSetRecordID
//
//	PURPOSE:	Return the AIMovementSet ID for the AIMovementSet record 
//              with a matching name.
//
// ----------------------------------------------------------------------- //

ENUM_AIMovementSetID CAIDB::GetAIMovementSetRecordID( const char* const szName )
{
	for( uint32 iAIMovementSet=0; iAIMovementSet < m_cAIMovementSetRecords; ++iAIMovementSet )
	{
		if( LTStrIEquals( szName, m_aAIMovementSetRecords[iAIMovementSet].strName.c_str() ) )
		{
			return m_aAIMovementSetRecords[iAIMovementSet].eAIMovementSet;
		}
	}

	return kAIMovementSetID_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIMovementSetRecordName
//
//	PURPOSE:	Return the MovementSet Name for the MovementSet with a matching ID.
//
// ----------------------------------------------------------------------- //

const char* CAIDB::GetAIMovementSetRecordName( ENUM_AIMovementSetID eMovementSet )
{
	if( ( eMovementSet > kAIMovementSetID_Invalid ) &&
		( (uint32)eMovementSet < m_cAIMovementSetRecords ) )
	{
		return m_aAIMovementSetRecords[eMovementSet].strName.c_str();
	}

	return "";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAISmartObjectRecord
//
//	PURPOSE:	Return a pointer to the specified SmartObject record.
//
// ----------------------------------------------------------------------- //

AIDB_SmartObjectRecord* CAIDB::GetAISmartObjectRecord( uint32 iRecord )
{
	if( iRecord < m_cAISmartObjectRecords )
	{
		return &( m_aAISmartObjectRecords[iRecord] );
	}

	// Record out of range.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAISmartObjectRecordID
//
//	PURPOSE:	Return the AISmartObject ID for the AISmartObject with a matching name.
//
// ----------------------------------------------------------------------- //

ENUM_AISmartObjectID CAIDB::GetAISmartObjectRecordID( const char* const szName )
{
	for( uint32 iAISmartObject=0; iAISmartObject < m_cAISmartObjectRecords; ++iAISmartObject )
	{
		if( LTStrIEquals( szName, m_aAISmartObjectRecords[iAISmartObject].strName.c_str() ) )
		{
			return m_aAISmartObjectRecords[iAISmartObject].eSmartObjectID;
		}
	}

	return kAISmartObjectID_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAISmartObjectRecordName
//
//	PURPOSE:	Return the SmartObject Name for the SmartObject with a matching ID.
//
// ----------------------------------------------------------------------- //

const char* CAIDB::GetAISmartObjectRecordName( ENUM_AISmartObjectID eSmartObject )
{
	if( ( eSmartObject > kAISmartObjectID_Invalid ) &&
		( (uint32)eSmartObject < m_cAISmartObjectRecords ) )
	{
		return m_aAISmartObjectRecords[eSmartObject].strName.c_str();
	}

	return "";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIWeaponRecord
//
//	PURPOSE:	Return a pointer to the specified AIWeapon record.
//
// ----------------------------------------------------------------------- //

AIDB_AIWeaponRecord* CAIDB::GetAIWeaponRecord( uint32 iRecord )
{
	if( iRecord < m_cAIWeaponRecords )
	{
		return &( m_aAIWeaponRecords[iRecord] );
	}

	// Record out of range.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIWeaponRecordID
//
//	PURPOSE:	Return the AIWeapon ID for the AIWeapon with a matching name.
//
// ----------------------------------------------------------------------- //

ENUM_AIWeaponID CAIDB::GetAIWeaponRecordID( const char* const szName )
{
	if ( !szName )
	{
		return kAIWeaponID_Invalid;
	}

	for( uint32 iAIWeapon=0; iAIWeapon < m_cAIWeaponRecords; ++iAIWeapon )
	{
		if( LTStrIEquals( szName, m_aAIWeaponRecords[iAIWeapon].strName.c_str() ) )
		{
			return m_aAIWeaponRecords[iAIWeapon].eAIWeaponID;
		}
	}

	return kAIWeaponID_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIAmmoLoadRecord
//
//	PURPOSE:	Return a pointer to the specified AIAmmoLoad record.
//
// ----------------------------------------------------------------------- //

AIDB_AIAmmoLoadRecord* CAIDB::GetAIAmmoLoadRecord( ENUM_AIAmmoLoadRecordID eID )
{
	if (eID < 0 || eID >= (int)m_cAIAmmoLoadRecords)
	{
		return NULL;
	}

	return &m_aAIAmmoLoadRecords[eID];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAIAmmoLoadRecord
//
//	PURPOSE:	Return a the ID of the ammoload record with the passed in 
//				name.
//
// ----------------------------------------------------------------------- //

ENUM_AIAmmoLoadRecordID CAIDB::GetAIAmmoLoadRecordID( const char* const szName )
{
	for( uint32 iAIAmmoLoad=0; iAIAmmoLoad < m_cAIAmmoLoadRecords; ++iAIAmmoLoad )
	{
		if( LTStrIEquals( szName, m_aAIAmmoLoadRecords[iAIAmmoLoad].strName.c_str() ) )
		{
			return (ENUM_AIAmmoLoadRecordID)iAIAmmoLoad;
		}
	}

	return kAIAmmoLoadRecordID_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAINodeRecord
//
//	PURPOSE:	Return a pointer to the specified AINode record.
//
// ----------------------------------------------------------------------- //

AIDB_AINodeRecord* CAIDB::GetAINodeRecord( EnumAINodeType eNodeType )
{
	uint32 iNodeIndex = (uint32)eNodeType;
	if( iNodeIndex < m_cAINodeRecords )
	{
		return &( m_aAINodeRecords[iNodeIndex] );
	}

	// Record out of range.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetAINavMeshTypeRecord
//
//	PURPOSE:	Return a pointer to the specified AINavMeshTypeRecord record.
//
// ----------------------------------------------------------------------- //

const AIDB_AINavMeshTypeRecord* CAIDB::GetAINavMeshTypeRecord( uint32 iAINavMeshType ) const
{
	if( iAINavMeshType < m_cAINodeRecords )
	{
		return &( m_aAIAINavMeshTypeRecords[iAINavMeshType] );
	}

	// Record out of range.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::String2EnumIndex
//
//	PURPOSE:	Converts a string to an enum index.
//
// ----------------------------------------------------------------------- //

uint32 CAIDB::String2EnumIndex( const char* szName, uint32 cMax, uint32 iInvalid, const char* aszEnums[] )
{
	// Look for name in enum list.

	for( uint32 iEnum=0; iEnum < cMax; ++iEnum )
	{
		if( LTStrIEquals( szName, aszEnums[iEnum] ) )
		{
			return iEnum;
		}
	}

	// No match found.

	return iInvalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::String2BitFlag
//
//	PURPOSE:	Converts a string to a bit flag.
//
// ----------------------------------------------------------------------- //

uint32 CAIDB::String2BitFlag( const char* szName, uint32 cMax, const char* aszEnums[] )
{
	// Look for name in enum list.
	uint32 iEnum;
	for( iEnum=0; iEnum < cMax; ++iEnum )
	{
		if( LTStrIEquals( szName, aszEnums[iEnum]) )
		{
			break;
		}
	}

	return (1 << iEnum);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetMiscRecord()
//
//	PURPOSE:	Return a miscellaneous record.  This is used internally 
//				and externally for getting this HRECORD.
//
// ----------------------------------------------------------------------- //

HRECORD CAIDB::GetMiscRecord()
{
	HCATEGORY hCatAIMisc = g_pLTDatabase->GetCategory( m_hDatabase, AIDB_MISC_CATEGORY );
	if( !hCatAIMisc )
	{
		return NULL;
	}

	// There should only be 1 AI misc record.

	if( g_pLTDatabase->GetNumRecords( hCatAIMisc ) > 1 )
	{
		AIASSERT( 0, NULL, "CAIDB::GetMiscRecord: There should only be one AIMisc record" );
	}

	// Access the record.

	HRECORD hRecord = g_pLTDatabase->GetRecordByIndex( hCatAIMisc, 0 );
	if( !hRecord )
	{
		AIASSERT( 0, NULL, "CAIDB::GetMiscRecord: Missing the AIMisc record" );
		return NULL;
	}

	return hRecord;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetMiscRecordLink()
//
//	PURPOSE:	Return a miscellaneous record link.
//
// ----------------------------------------------------------------------- //

HRECORD CAIDB::GetMiscRecordLink( const char* pszLink )
{
	return GetRecordLink( GetMiscRecord(), pszLink );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetMiscString()
//
//	PURPOSE:	Return a miscellaneous string.
//
// ----------------------------------------------------------------------- //

const char* CAIDB::GetMiscString( const char* pszString )
{
	return GetString( GetMiscRecord(), pszString );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDB::GetMiscFloat()
//
//	PURPOSE:	Return a miscellaneous float.
//
// ----------------------------------------------------------------------- //

float CAIDB::GetMiscFloat( const char* pszFloat )
{
	return GetFloat( GetMiscRecord(), pszFloat );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIDB_ActionRecord::AIDB_ActionRecord()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

AIDB_ActionRecord::AIDB_ActionRecord()
{
	eActionClass = kAct_InvalidType;
	eNodeType = kNode_InvalidType;
	fActionCost = 1.f;
	fActionPrecedence = 1.f;
	bActionIsInterruptible = true;
	eSmartObjectID = kAISmartObjectID_Invalid;
	fActionProbability = 1.f;
	eAwareness = kAware_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIDB_TargetSelectRecord::AIDB_TargetSelectRecord()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

AIDB_TargetSelectRecord::AIDB_TargetSelectRecord()
{
	eTargetSelectClass = kTargetSelect_InvalidType;
	eTargetSelectType = kTargetSelect_InvalidType;
//	eNodeType = kNode_InvalidType;
	fCost = 1.f;
//	fPrecedence = 1.f;
//	eSmartObjectID = kAISmartObjectID_Invalid;
//	fProbability = 1.f;
	eAwareness = kAware_Invalid;
}

