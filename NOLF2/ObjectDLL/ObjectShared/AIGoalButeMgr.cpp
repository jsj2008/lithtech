// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalButeMgr.cpp
//
// PURPOSE : Read templates for Goals.
//
// CREATED : 6/7/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalButeMgr.h"
#include "AISenseRecorderAbstract.h"
#include "AIGoalAbstract.h"
#include "AINode.h"
#include "AINodeMgr.h"
#include "ButeTools.h"
#include "AIState.h"

// Globals/statics

CAIGoalButeMgr* g_pAIGoalButeMgr = LTNULL;

static char s_aTagName[30];
static char s_aAttName[100];


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIGBM_GoalTemplate::AIGBM_GoalTemplate()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

AIGBM_GoalTemplate::AIGBM_GoalTemplate()
{
	aAttractors = LTNULL;
	flagSenseTriggers = kSense_None; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalButeMgr::CAIGoalButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAIGoalButeMgr::CAIGoalButeMgr()
{
    m_aTemplates	= LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalButeMgr::~CAIGoalButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAIGoalButeMgr::~CAIGoalButeMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalButeMgr::Init(const char* szAttributeFile)
{
    if (g_pAIGoalButeMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile))
	{
		AIASSERT1( 0, NULL, "CAIGoalButeMgr::Init: Failed to parse %s", szAttributeFile );
		return LTFALSE;
	}

	// Set up global pointer

	g_pAIGoalButeMgr = this;

	// Read Goal Sets.
	
	uint32 iGoalSet = 0;
	sprintf(s_aTagName, "%s%d", "GoalSet", iGoalSet);
	
	while (m_buteMgr.Exist(s_aTagName))
	{
		ReadGoalSet();
		++iGoalSet;
		sprintf(s_aTagName, "%s%d", "GoalSet", iGoalSet);
	}


	// Create an array as big as the goal type enum list.
	
	m_aTemplates = debug_newa(AIGBM_GoalTemplate, kGoal_Count);

	// See how many goal templates there are

	uint32 cTemplates = 0;
	sprintf(s_aTagName, "%s%d", "Goal", cTemplates);

	while (m_buteMgr.Exist(s_aTagName))
	{
		++cTemplates;
		sprintf(s_aTagName, "%s%d", "Goal", cTemplates);
	}

	
	// Read the goal templates.

	for ( uint32 iTemplate = 0 ; iTemplate < cTemplates ; ++iTemplate )
	{
		ReadGoalTemplate(iTemplate);
	}

	// Read SmartObject templates.
	
	uint32 iSmartObject = 0;
	sprintf(s_aTagName, "%s%d", "SmartObject", iSmartObject);
	
	while (m_buteMgr.Exist(s_aTagName))
	{
		ReadSmartObjectTemplate(iSmartObject);
		++iSmartObject;
		sprintf(s_aTagName, "%s%d", "SmartObject", iSmartObject);
	}


	m_buteMgr.Term();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CAIGoalButeMgr::Term()
{
	AIGOAL_SET_LIST::iterator it;
	for(it = m_lstGoalSets.begin(); it != m_lstGoalSets.end(); ++it)
	{
		debug_delete(*it);
	}

	if ( m_aTemplates )
	{
		for(uint32 iTemplate=0; iTemplate < kGoal_Count; ++iTemplate)
		{
			debug_deletea(m_aTemplates[iTemplate].aAttractors);			
		}
		debug_deletea(m_aTemplates);
        m_aTemplates = LTNULL;
	}

	AIGBM_SmartObjectTemplate* pSmartObject;
	SMART_OBJECT_LIST::iterator sit;
	SMART_OBJECT_CMD_MAP::iterator cit;
	for(sit = m_lstSmartObjects.begin(); sit != m_lstSmartObjects.end(); ++sit)
	{
		pSmartObject = *sit;
		for(cit = pSmartObject->mapCmds.begin(); cit != pSmartObject->mapCmds.end(); ++cit)
		{
			FREE_HSTRING(cit->second);
		}

		debug_delete(pSmartObject);
	}

	m_lstSmartObjects.clear( );

    g_pAIButeMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalButeMgr::ReadGoalSet
//
//	PURPOSE:	Reads and sets a goal set template
//
// ----------------------------------------------------------------------- //

void CAIGoalButeMgr::ReadGoalSet()
{
	// Create new goal set and add it to list.

	AIGBM_GoalSet* pGoalSet = debug_new(AIGBM_GoalSet);
	m_lstGoalSets.push_back(pGoalSet);

	// Read name.
	CButeTools::GetValidatedString(m_buteMgr, s_aTagName, "Name", pGoalSet->szName, sizeof(pGoalSet->szName) );

	char szGoalString[64];

	// Read included goalsets.

	uint32 iGoalSet = 0;

	while ( true )
	{
		szGoalString[0] = '\0';
		sprintf(s_aAttName, "IncludeGoalSet%d", iGoalSet);
		m_buteMgr.GetString(s_aTagName, s_aAttName, "", szGoalString, sizeof(szGoalString) );
		if( !m_buteMgr.Success( ))
		{
			break;
		}

		// Find a goalset with a matching name, and add it to the list of
		// included goalsets.  Included goalsets must be listed first.

		if( szGoalString[0] )
		{
			AIGOAL_SET_LIST::iterator it;
			for( it = m_lstGoalSets.begin(); it != m_lstGoalSets.end(); ++it )
			{
				if( stricmp( szGoalString, (*it)->szName ) == 0 )
				{
					pGoalSet->lstIncludeGoalSets.push_back( *it );
					break;
				}
			}

			if( it == m_lstGoalSets.end() )
			{
				AIASSERT1( 0, NULL, "CAIGoalButeMgr::ReadGoalSet: Failed to find included goalset named %s", szGoalString );
			}
		}

		++iGoalSet;
	}

	// Read required brains.

	uint32 iBrain = 0;
	int nBrainID;

	while ( true )
	{
		szGoalString[0] = '\0';
		sprintf(s_aAttName, "RequiredBrain%d", iBrain);
		m_buteMgr.GetString(s_aTagName, s_aAttName, "", szGoalString, sizeof(szGoalString) );
		if( !m_buteMgr.Success( ))
		{
			break;
		}

		// Find a brain with a matching name, and add it to the list of
		// required brains.  

		if( szGoalString[0] )
		{
			nBrainID = g_pAIButeMgr->GetBrainIDByName( szGoalString );
			if( nBrainID != -1 )
			{
				pGoalSet->lstRequiredBrains.push_back( nBrainID );
			}
			else {
				AIASSERT1( 0, NULL, "CAIGoalButeMgr::ReadGoalSet: Failed to find Brain named %s", szGoalString );
			}
		}

		++iBrain;
	}


	// Read flags.

	if( CButeTools::GetValidatedBool( m_buteMgr, s_aTagName, "Permanent", LTFALSE ) )
	{
		pGoalSet->dwGoalSetFlags |= AIGBM_GoalSet::kGS_Permanent;
	}

	if( CButeTools::GetValidatedBool( m_buteMgr, s_aTagName, "Hidden", LTFALSE ) )
	{
		pGoalSet->dwGoalSetFlags |= AIGBM_GoalSet::kGS_Hidden;
	}

	// Read goal importances.

	uint32 iGoal = 0;
	
	char* tok;
	EnumAIGoalType eGoalType;
	SGoalSetData gsd;
	while ( true )
	{
		// Read entire goal string.
		sprintf(s_aAttName, "Goal%d", iGoal);
		m_buteMgr.GetString(s_aTagName, s_aAttName, "", szGoalString, sizeof(szGoalString) );
		if( !m_buteMgr.Success( ))
		{
			break;
		}

		// First token is AIGoalType.
		tok = strtok(szGoalString, " ");
		eGoalType = ConvertToGoalTypeEnum(tok);

		// All following tokens are parameters.
		gsd.hstrParams = LTNULL;
		if( g_pLTServer != LTNULL )
		{
			tok = strtok(LTNULL, "");
			if(tok != LTNULL)
			{
				gsd.hstrParams = g_pLTServer->CreateString( tok );
			}
		}

		// Add goal to map.
		pGoalSet->mapGoalSet.insert( AIGOAL_DATA_MAP::value_type(eGoalType, gsd) );

		++iGoal;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalButeMgr::ConvertToGoalTypeEnum
//
//	PURPOSE:	Converts a string to a AIGoalType enum.
//
// ----------------------------------------------------------------------- //

EnumAIGoalType CAIGoalButeMgr::ConvertToGoalTypeEnum(char* szGoalType)
{
	// Look for goal name in enum list.
	uint32 iGoalType;
	for(iGoalType=0; iGoalType < kGoal_Count; ++iGoalType)
	{
		if(0 == stricmp(szGoalType, s_aszGoalTypes[iGoalType]) )
		{
			break;
		}
	}
//	ASSERT((iGoalType != kGoal_InvalidType) && (iGoalType != kGoal_Count));

	return (EnumAIGoalType)iGoalType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalButeMgr::GetGoalSetIndex
//
//	PURPOSE:	Gets index of a Goal Set by name.
//
// ----------------------------------------------------------------------- //

uint32 CAIGoalButeMgr::GetGoalSetIndex(const char* szGoalSetName)
{
	ASSERT(szGoalSetName[0] != LTNULL);

	uint32 iGoalSet = 0;
	AIGBM_GoalSet* pGoalSet = LTNULL;
	AIGOAL_SET_LIST::iterator it;
	for(it = m_lstGoalSets.begin(); it != m_lstGoalSets.end(); ++it)
	{
		if( stricmp((*it)->szName, szGoalSetName) == 0)
		{
			return iGoalSet;
		}

		++iGoalSet;
	}

	return -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalButeMgr::GetGoalSet
//
//	PURPOSE:	Gets pointer to a Goal Set by name.
//
// ----------------------------------------------------------------------- //

AIGBM_GoalSet* CAIGoalButeMgr::GetGoalSet(const char* szGoalSetName)
{
	ASSERT(szGoalSetName[0] != LTNULL);

	AIGBM_GoalSet* pGoalSet = LTNULL;
	AIGOAL_SET_LIST::iterator it;
	for(it = m_lstGoalSets.begin(); it != m_lstGoalSets.end(); ++it)
	{
		if( stricmp((*it)->szName, szGoalSetName) == 0)
		{
			pGoalSet = *it;
		}
	}

	return pGoalSet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalButeMgr::ReadGoalTemplate
//
//	PURPOSE:	Sets a goal template
//
// ----------------------------------------------------------------------- //

void CAIGoalButeMgr::ReadGoalTemplate(uint32 iTemplate)
{
	sprintf(s_aTagName, "Goal%d", iTemplate);
	char szName[128];
	CButeTools::GetValidatedString(m_buteMgr, s_aTagName, "Name",szName,sizeof(szName));

	uint32 iGoal = ConvertToGoalTypeEnum(szName);

	// Make sure that the selected goal is in range before we access it!
	if ( iGoal == kGoal_InvalidType || iGoal == kGoal_Count )
	{
		AIASSERT1( 0, NULL, "Goal name and enumerations out of sync for goal: %s", szName );
		return;
	}

	AIGBM_GoalTemplate& Template = m_aTemplates[iGoal];

	// Get values.

	Template.fImportance			= (LTFLOAT)CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName, "Importance");
	Template.fDecayTime				= (LTFLOAT)CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName, "DecayTime");
	Template.bFreezeDecay			= CButeTools::GetValidatedBool(m_buteMgr, s_aTagName, "FreezeDecay", LTFALSE);
	Template.bLockedAnimIsInterruptable= CButeTools::GetValidatedBool(m_buteMgr, s_aTagName, "LockedAnimIsInterruptable", LTTRUE);
	Template.bForceAnimInterrupt	= CButeTools::GetValidatedBool(m_buteMgr, s_aTagName, "ForceAnimInterrupt", LTFALSE);
	Template.fUpdateRate			= (LTFLOAT)CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName, "UpdateRate");
	Template.bDeleteOnDeactivation	= CButeTools::GetValidatedBool(m_buteMgr, s_aTagName, "DeleteWhenDone", LTFALSE);
	Template.fChanceToActivate		= (LTFLOAT)CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName, "ChanceToActivate", 1.f);
	Template.fFrequencyMin			= (LTFLOAT)CButeTools::GetValidatedRange(m_buteMgr, s_aTagName, "Frequency", CARange(0.f, 0.f)).GetMin();
	Template.fFrequencyMax			= (LTFLOAT)CButeTools::GetValidatedRange(m_buteMgr, s_aTagName, "Frequency", CARange(0.f, 0.f)).GetMax();
	Template.nDamagePriority		= CButeTools::GetValidatedInt(m_buteMgr, s_aTagName, "DamagePriority", 0);

	// Get SenseTriggers. (SenseTypes are bitflags)

	GetBitFlagItems( &(Template.flagSenseTriggers), "SenseTrigger", 
					 kSense_Count, s_aszSenseTypes);

	// Get Attractors. (AttractorTypes are enums)
	Template.cAttractors = 0;
	Template.aAttractors = LTNULL;
	GetEnumItems( (uint32*&)(Template.aAttractors), Template.cAttractors, 
					"Attractor", kNode_Count, s_aszAINodeTypes);

	// Get AttractorDist, if there are attractors.
	if(Template.aAttractors != LTNULL)
	{
		Template.fAttractorDistSqr = (LTFLOAT)CButeTools::GetValidatedDouble(m_buteMgr, s_aTagName, "AttractorDist");
		Template.fAttractorDistSqr *= Template.fAttractorDistSqr;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalButeMgr::ReadSmartObjectTemplate
//
//	PURPOSE:	Reads and sets a SmartObject template
//
// ----------------------------------------------------------------------- //

void CAIGoalButeMgr::ReadSmartObjectTemplate(uint32 nID)
{
	// Create new SmartObject and add it to list.

	AIGBM_SmartObjectTemplate* pSmartObject = debug_new(AIGBM_SmartObjectTemplate);
	m_lstSmartObjects.push_back(pSmartObject);

	// Read name.
	CButeTools::GetValidatedString(m_buteMgr, s_aTagName, "Name", pSmartObject->szName, sizeof(pSmartObject->szName) );

	// Set ID.
	pSmartObject->nID = nID;

	if( g_pLTServer != LTNULL )
	{
		// Read flags and commands.

		EnumAINodeType eNodeType;
		uint32 iFlag = 0;
		char szFlag[128];
		char szCmd[128];
		HSTRING hstr;
		while ( true )
		{
			sprintf(s_aAttName, "Flag%d", iFlag);
			m_buteMgr.GetString(s_aTagName, s_aAttName, "", szFlag, sizeof(szFlag) );
			if( !m_buteMgr.Success( ))
				break;

			// Find flag with matching string.

			eNodeType = CAINodeMgr::NodeTypeFromString( szFlag );

			// Read Cmd string for type flag.
			sprintf(s_aAttName, "Cmd%d", iFlag);
			m_buteMgr.GetString(s_aTagName, s_aAttName, "", szCmd, sizeof(szCmd) );
			if(m_buteMgr.Success( ))
			{
				// Add entry to map of cmds.
				hstr = g_pLTServer->CreateString( szCmd );
				pSmartObject->mapCmds.insert( SMART_OBJECT_CMD_MAP::value_type( eNodeType, hstr ) );
			}
			else 
			{
				pSmartObject->mapCmds.insert( SMART_OBJECT_CMD_MAP::value_type( eNodeType, (HSTRING)LTNULL ) );
			}

			++iFlag;
		}

		// Read states.

		uint32 nSkip = strlen("SmartObject");
		uint32 iStateType;
		for( iStateType = kState_SmartObjectBegin + 1; iStateType != kState_SmartObjectEnd; ++iStateType )
		{
			iFlag = 0;
			while( true )
			{
				sprintf( s_aAttName, "State%s%d", s_aszStateTypes[iStateType] + nSkip, iFlag++ );
				m_buteMgr.GetString(s_aTagName, s_aAttName, "", szFlag, sizeof(szFlag) );
				if( !m_buteMgr.Success( ))
					break;

				eNodeType = CAINodeMgr::NodeTypeFromString( szFlag );
				AIASSERT( pSmartObject->mapCmds.find( eNodeType ) != pSmartObject->mapCmds.end(), LTNULL, "CAIGoalButeMgr::ReadSmartObjectTemplate: State contains flag not foudn in SMartObject." );

				pSmartObject->mapActiveCmds.insert( SMART_OBJECT_ACTIVE_CMD_MAP::value_type( (EnumAIStateType)iStateType, eNodeType ) );
			}
		}

		// Ensure that a default state exists.

		if( pSmartObject->mapActiveCmds.empty() )
		{
			SMART_OBJECT_CMD_MAP::iterator it;
			for( it = pSmartObject->mapCmds.begin(); it != pSmartObject->mapCmds.end(); ++it )
			{
				eNodeType = it->first;
				pSmartObject->mapActiveCmds.insert( SMART_OBJECT_ACTIVE_CMD_MAP::value_type( kState_SmartObjectDefault, eNodeType ) );
			}
		}

		// TERRYF
		// Add childmodel names here.
		uint32 cm_cnt = 0;
		char   szCMFilename[256];
		HMODELDB hmodeldb = LTNULL;

		while( true )
		{
			sprintf(s_aAttName, "AddAnimsLTB%d", cm_cnt );
			m_buteMgr.GetString(s_aTagName, s_aAttName, "", szCMFilename, sizeof(szCMFilename) );
			if( !m_buteMgr.Success( ))
				break;

			// load it here.
			{
				AIChildModelInfo info(hmodeldb, szCMFilename);			
				pSmartObject->addchildmodels.push_back(info);
			}

			cm_cnt++ ;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalButeMgr::GetSmartObjectTemplate
//
//	PURPOSE:	Gets pointer to a SmartObject template by name.
//
// ----------------------------------------------------------------------- //

AIGBM_SmartObjectTemplate* CAIGoalButeMgr::GetSmartObjectTemplate(const char* szSmartObjectName)
{
	ASSERT(szSmartObjectName[0] != LTNULL);

	AIGBM_SmartObjectTemplate* pSmartObject = LTNULL;
	SMART_OBJECT_LIST::iterator it;
	for(it = m_lstSmartObjects.begin(); it != m_lstSmartObjects.end(); ++it)
	{
		if( stricmp((*it)->szName, szSmartObjectName) == 0)
		{
			pSmartObject = *it;
		}
	}

	return pSmartObject;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalButeMgr::GetBitFlagItems
//
//	PURPOSE:	Read in list of items that correspond to bitflags.
//
// ----------------------------------------------------------------------- //

void CAIGoalButeMgr::GetBitFlagItems(uint32* flags, const char* szAttribute, const uint32 nNumFlags, 
								  const char** aszFlags)
{
	// Clear the flags.
	*flags = 0;

	char szItem[128];
	uint32 iFlag;
	uint8  iItem = 0;

	// Find all existing items, listed Item0, Item1, ..., ItemN
	while ( true )
	{
		sprintf(s_aAttName, "%s%d", szAttribute, iItem);
		m_buteMgr.GetString(s_aTagName, s_aAttName, "", szItem, sizeof(szItem) );
		if( !m_buteMgr.Success( ))
			break;

		// Find flag with matching string.
		for(iFlag=0; iFlag < nNumFlags; ++iFlag)
		{
			if( 0 == stricmp(szItem, aszFlags[iFlag]) )
			{
				break;
			}
		}
		AIASSERT(iFlag < nNumFlags, LTNULL, "CAIGoalButeMgr::GetBitFlagItems: No matching flag.");

		// Set flag.
		*flags |= (1 << iFlag);

		++iItem;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalButeMgr::GetEnumItems
//
//	PURPOSE:	Read in list of items that correspond to enums.
//
// ----------------------------------------------------------------------- //

void CAIGoalButeMgr::GetEnumItems(uint32*& aItems, uint32& cItems, const char* szAttribute, const uint32 nNumEnums, 
								  const char** aszEnums)
{
	AIASSERT(aItems == LTNULL, LTNULL, "CAIGoalButeMgr::GetEnumItems: aItems is not NULL.");

	char szItem[128];
	uint32 iEnum;
	uint8  iItem = 0;

	// Count all existing items, listed Item0, Item1, ..., ItemN
	while ( true )
	{
		sprintf(s_aAttName, "%s%d", szAttribute, cItems);
		if( !m_buteMgr.Exist(s_aTagName, s_aAttName))
			break;

		++cItems;
	}
	if(cItems == 0)
	{
		return;
	}

	// Allocate space for items.
	aItems = debug_newa(uint32, cItems);

	// Read in all existing items, listed Item0, Item1, ..., ItemN
	while ( true )
	{
		sprintf(s_aAttName, "%s%d", szAttribute, iItem);
		m_buteMgr.GetString(s_aTagName, s_aAttName, "", szItem, sizeof(szItem) );
		if( !m_buteMgr.Success( ))
			break;

		// Find flag with matching string.
		for(iEnum=0; iEnum < nNumEnums; ++iEnum)
		{
			if( 0 == stricmp(szItem, aszEnums[iEnum]) )
			{
				break;
			}
		}
		AIASSERT(iEnum < nNumEnums, LTNULL, "CAIGoalButeMgr::GetEnumItems: No matching enum.");

		aItems[iItem] = iEnum;

		++iItem;
	}
}

