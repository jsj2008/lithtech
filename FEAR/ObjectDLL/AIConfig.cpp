// ----------------------------------------------------------------------- //
//
// MODULE  : AIConfig.cpp
//
// PURPOSE : AIConfig aggregate object - Implementation
//           AIConfig wraps a WorldEdit User-Interface around commands
//           that are frequently set as InitialCommands on AI.
//
// CREATED : 01/16/04
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "Stdafx.h"
	#include "AIConfig.h"
	#include "AI.h"
	#include "AIDB.h"
	#include "AINode.h"
	#include "AINodeMgr.h"
	#include "AIEnumAwarenessModifiers.h"
	#include "CharacterDB.h"
	#include "GameModeMgr.h"


CMDMGR_BEGIN_REGISTER_CLASS( CAIConfig )
	ADD_MESSAGE( TEAM,		2,	NULL,	MSG_HANDLER( CAIConfig, HandleTeamMsg ),		"TEAM <0, 1, -1>", "Specifies which team can activate this trigger", "msg Trigger (TEAM 1)" )
CMDMGR_END_REGISTER_CLASS( CAIConfig, IAggregate )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIConfig::CAIConfig()
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

CAIConfig::CAIConfig()
:	IAggregate			( "CAIConfig" ),
	m_hObject			( NULL ),
	m_bSeekEnemy		( false ),
	m_fVehicleYOffset	( 0.f ),
	m_bSensesOn			( true ),
	m_bEnabled			( true ),
	m_nTeamID			( INVALID_TEAM )
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIConfig::~CAIConfig()
//
//	PURPOSE:	Destructor - deallocate weapons
//
// ----------------------------------------------------------------------- //

CAIConfig::~CAIConfig()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //

uint32 CAIConfig::EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE :
		{
			int nInfo = (int)fData;
			if( nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP )
			{
				ReadProp( pObject, &((ObjectCreateStruct*)pData)->m_cProperties );
			}
		}
		break;

		case MID_OBJECTCREATED :
		{
			if( !pObject || !pObject->m_hObject )
				break;
			
			Init( pObject->m_hObject );
		}
		break;
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIConfig::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool CAIConfig::ReadProp(LPBASECLASS pObject, const GenericPropList *pProps)
{
	LTASSERT( pObject && pProps, "Invalid ReadProp call encountered" );
	if( !pObject || !pProps )
	{
		return false;
	}

	// Get the team the AI belongs to, but only when in a team game...
	if( GameModeMgr::Instance( ).m_grbUseTeams )
	{
		m_nTeamID = TeamStringToTeamId( pProps->GetString( "Team", "" ));
	}
	else
	{
		m_nTeamID = INVALID_TEAM;
	}

	// Alignment.

	const char* pszAlignment = pProps->GetString( "Alignment", "Default" );
	if( !LTStrIEquals( pszAlignment, "Default" ) )
	{
		m_strAlignment = pszAlignment;
	}

	// Awareness.

	const char* pszAwareness = pProps->GetString( "Awareness", "Relaxed" );
	if( !LTStrIEquals( pszAwareness, "Relaxed" ) )
	{
		m_strAwareness = pszAwareness;
	}

	// Awareness modifier.

	const char* pszAwarenessMod = pProps->GetString( "AwarenessMod", "None" );
	if( !LTStrIEquals( pszAwarenessMod, "None" ) )
	{
		m_strAwarenessMod = pszAwarenessMod;
	}

	// SeekEnemy.

	m_bSeekEnemy = pProps->GetBool( "SeekEnemy", m_bSeekEnemy );

	// Initial Node.

	const char* pszNode = pProps->GetString( "InitialNode", "" );
	if( pszNode[0] )
	{
		m_strInitialNode = pszNode;
	}

	// Behavior Node.

	pszNode = pProps->GetString( "BehaviorNode", "" );
	if( pszNode[0] )
	{
		m_strBehaviorNode = pszNode;
	}

	// Vehicle Object.

	const char* pszVehicleObject = pProps->GetString( "VehicleObject", "" );
	if( pszVehicleObject[0] )
	{
		m_strVehicleObject = pszVehicleObject;
	}

	// Vehicle Type.

	const char* pszVehicleType = pProps->GetString( "VehicleType", "None" );
	m_strVehicleType = pszVehicleType;

	// Vehicle Y-Offset.

	m_fVehicleYOffset = pProps->GetReal( "VehicleYOffset", m_fVehicleYOffset );

	// Vehicle KeyframeToRigidBody.

	const char* pszVehicleKeyframeToRigidBody = pProps->GetString( "VehicleKeyframeToRigidBody", "" );
	m_strVehicleKeyframeToRigidBody = pszVehicleKeyframeToRigidBody;

	// EntranceAnim.

	const char* pszEntryAnim = pProps->GetString( "EntranceAnim", "" );
	if( pszEntryAnim[0] )
	{
		m_strEntranceAnim = pszEntryAnim;
	}

	// Senses on

	m_bSensesOn = pProps->GetBool( "OV_Senses", m_bSensesOn );

	// Damaged player message

	const char* pszCmdDamagedPlayer = pProps->GetCommand( "DamagedPlayerCommand", "" );
	if ( pszCmdDamagedPlayer[0] )
	{
		m_strCmdDamagedPlayer = pszCmdDamagedPlayer;
	}

	m_nDamagedPlayerActivationCount = pProps->GetLongInt( "DamagedPlayerNumActivations", 1 );


	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIConfig::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CAIConfig::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	bool bDummyConfigured = false;

	SAVE_bool( m_bEnabled );
	SAVE_bool( bDummyConfigured );
	SAVE_HOBJECT( m_hObject );
	SAVE_STDSTRING( m_strAlignment );
	SAVE_STDSTRING( m_strAwareness );
	SAVE_STDSTRING( m_strAwarenessMod );
	SAVE_bool( m_bSeekEnemy );
	SAVE_STDSTRING( m_strInitialNode );
	SAVE_STDSTRING( m_strBehaviorNode );
	SAVE_STDSTRING( m_strVehicleObject );
	SAVE_STDSTRING( m_strVehicleType );
	SAVE_FLOAT(	m_fVehicleYOffset );
	SAVE_STDSTRING( m_strVehicleKeyframeToRigidBody );
	SAVE_STDSTRING( m_strEntranceAnim );
	SAVE_bool( m_bSensesOn );
	SAVE_STDSTRING( m_strCmdDamagedPlayer );
	SAVE_INT( m_nDamagedPlayerActivationCount );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIConfig::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CAIConfig::Load(ILTMessage_Read *pMsg )
{
	if (!pMsg) return;

	bool bDummyConfigured = false;

	LOAD_bool( m_bEnabled );
	LOAD_bool( bDummyConfigured );
	LOAD_HOBJECT( m_hObject );
	LOAD_STDSTRING( m_strAlignment );
	LOAD_STDSTRING( m_strAwareness );
	LOAD_STDSTRING( m_strAwarenessMod );
	LOAD_bool( m_bSeekEnemy );
	LOAD_STDSTRING( m_strInitialNode );
	LOAD_STDSTRING( m_strBehaviorNode );
	LOAD_STDSTRING( m_strVehicleObject );
	LOAD_STDSTRING( m_strVehicleType );
	LOAD_FLOAT(	m_fVehicleYOffset );
	LOAD_STDSTRING( m_strVehicleKeyframeToRigidBody );
	LOAD_STDSTRING( m_strEntranceAnim );
	LOAD_bool( m_bSensesOn );
	LOAD_STDSTRING( m_strCmdDamagedPlayer );
	LOAD_INT( m_nDamagedPlayerActivationCount );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIConfig::Init()
//
//	PURPOSE:	Initialize weapons
//
// ----------------------------------------------------------------------- //

bool CAIConfig::Init( HOBJECT hObject )
{
	// Sanity check.

	if( !hObject )
	{
		return false;
	}

	LTASSERT( m_hObject == NULL, "AIConfig already an aggraget of an object." );
	m_hObject = hObject;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIConfig::ConfigureAI()
//
//	PURPOSE:	Configure an AI by sending commands.
//
// ----------------------------------------------------------------------- //

void CAIConfig::ConfigureAI( CAI* pAI )
{
	// Bail if not enabled.
	// If an AI is spawned from an AISpawner, the AIConfig
	// on the AI template is disabled, and only the AIConfig
	// on the AISpawner should run.

	if( !m_bEnabled )
	{
		return;
	}

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	std::string strCmd;

	// Team ID

	if (m_nTeamID != INVALID_TEAM)
	{
		strCmd = "TEAM ";
		char szBuffer[32];
		sprintf( szBuffer, " %d", m_nTeamID );
		strCmd += szBuffer;
		g_pCmdMgr->QueueMessage( pAI, pAI, strCmd.c_str() );
	}

	// Alignment.

	if( !m_strAlignment.empty() )
	{
		strCmd = "ALIGNMENT ";
		strCmd += m_strAlignment;
		g_pCmdMgr->QueueMessage( pAI, pAI, strCmd.c_str() );
	}

	// Awareness.

	if( !m_strAwareness.empty() )
	{
		strCmd = "AWARENESS ";
		strCmd += m_strAwareness;
		g_pCmdMgr->QueueMessage( pAI, pAI, strCmd.c_str() );
	}

	// Awareness modifier.

	if( !m_strAwarenessMod.empty() )
	{
		strCmd = "AWARENESSMOD ";
		strCmd += m_strAwarenessMod;
		g_pCmdMgr->QueueMessage( pAI, pAI, strCmd.c_str() );
	}

	// SeekEnemy.

	if( m_bSeekEnemy )
	{
		g_pCmdMgr->QueueMessage( pAI, pAI, "SEEKENEMY" );
	}

	// Behavior Node.

	if( !m_strBehaviorNode.empty() )
	{
		QueueNodeMessage( pAI, m_strBehaviorNode.c_str() );
	}

	// Initial Node.

	if( !m_strInitialNode.empty() )
	{
		QueueNodeMessage( pAI, m_strInitialNode.c_str() );
	}

	// Vehicle.

	if( !m_strVehicleObject.empty() )
	{
		strCmd = "MOUNT ";
		strCmd += m_strVehicleObject;
		strCmd += " ";
		strCmd += m_strVehicleType;

		char szBuffer[32];
		sprintf( szBuffer, " %.2f", m_fVehicleYOffset );
		strCmd += szBuffer;

		if( !m_strVehicleKeyframeToRigidBody.empty() )
		{
			strCmd += " ";
			strCmd += m_strVehicleKeyframeToRigidBody;
		}

		g_pCmdMgr->QueueMessage( pAI, pAI, strCmd.c_str() );
	}

	// Entrance Anim.

	if( !m_strEntranceAnim.empty() )
	{
		strCmd = "ANIMATE ";
		strCmd += m_strEntranceAnim;
		g_pCmdMgr->QueueMessage( pAI, pAI, strCmd.c_str() );
	}

	// Senses

	pAI->GetAIBlackBoard()->SetBBSensesOn( m_bSensesOn );

	// Damaged player command.

	pAI->SetDamagedPlayerCommand( m_strCmdDamagedPlayer.c_str(), m_nDamagedPlayerActivationCount );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIConfig::QueueNodeMessage()
//
//	PURPOSE:	Queue a message to an AI based on the type of node.
//
// ----------------------------------------------------------------------- //

void CAIConfig::QueueNodeMessage( CAI* pAI, const char* pszNodeName )
{
	// Sanity check.

	if( !( pAI && pszNodeName[0] ) )
	{
		return;
	}

	std::string strCmd;
	AINode* pNode = g_pAINodeMgr->GetNode( pszNodeName );
	if( pNode )
	{
		// Node has an owner.

		AINode* pOwner = NULL;
		if( IsAINode( pNode->GetNodeOwner() ) )
		{
			pOwner = (AINode*)g_pLTServer->HandleToObject( pNode->GetNodeOwner() );
		}

		// Node is owned by a Guard node.

		if( pOwner && ( pOwner->GetType() == kNode_Guard ) )
		{
			strCmd = "GUARD ";
			strCmd += pOwner->GetNodeName();
		}

		// Node is NOT owned by a Guard node.

		else {
			switch( pNode->GetType() )
			{
				// Node is a guard node.

			case kNode_Guard:	strCmd = "GUARD ";
				break;

				// Node is a patrol node.

			case kNode_Patrol:	strCmd = "PATROL ";
				break;

				// Node is a scanner node.

			case kNode_Scanner:	strCmd = "SCAN ";
				break;

				// Node is an ambush node.

			case kNode_Ambush:	strCmd = "AMBUSH ";
				break;

				// Node is a cover node.

			case kNode_Cover:	strCmd = "COVER ";
				break;

				// Node is a vehicle node.

			case kNode_Vehicle: strCmd = "MOUNTNODE ";
				break;

				// Node is a menace node.

			case kNode_MenacePlace: strCmd = "MENACE ";
				break;

				// Node is a menace node.

			case kNode_PickupWeapon: strCmd = "PICKUPWEAPON ";
				break;

				// Node is an intro node.

			case kNode_Intro: strCmd = "INTRO ";
				break;

				// Node is something else.

			default:			strCmd = "GOTO ";
				break;
			}

			strCmd += pszNodeName;
		}

		g_pCmdMgr->QueueMessage( pAI, pAI, strCmd.c_str() );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIConfigPlugin::PreHook_EditStringList
//
//	PURPOSE:	Callback handler for EditStringList plugin
//
// ----------------------------------------------------------------------- //

LTRESULT CAIConfigPlugin::PreHook_EditStringList(const char* szRezPath,
												const char* szPropName,
												char** aszStrings,
												uint32* pcStrings,
												const uint32 cMaxStrings,
												const uint32 cMaxStringLength)
{
	// Alignment.

	if ( LTStrIEquals( "Alignment", szPropName ) )
	{
		strcpy( aszStrings[(*pcStrings)++], "Default" );

		int cAlignments = g_pCharacterDB->GetNumAlignments();
		for( int iAlignment=0; iAlignment < cAlignments; ++iAlignment )
		{
			strcpy( aszStrings[(*pcStrings)++], g_pCharacterDB->Alignment2String( (EnumCharacterAlignment)iAlignment ) );
		}

		return LT_OK;
	}

	// Awareness.

	if ( LTStrIEquals( "Awareness", szPropName ) )
	{
		strcpy( aszStrings[(*pcStrings)++], "Relaxed" );
		strcpy( aszStrings[(*pcStrings)++], "Suspicious" );
		strcpy( aszStrings[(*pcStrings)++], "Alert" );

		return LT_OK;
	}

	// Awareness modifier.

	if ( LTStrIEquals( "AwarenessMod", szPropName ) )
	{
		strcpy( aszStrings[(*pcStrings)++], "None" );

		for(uint32 iAwarenessMod=0; iAwarenessMod < kAwarenessMod_Count; ++iAwarenessMod)
		{
			strcpy( aszStrings[(*pcStrings)++], s_aszAIAwarenessMods[iAwarenessMod] );
		}

		return LT_OK;
	}

	// Vehicle Type.

	if ( LTStrIEquals( "VehicleType", szPropName ) )
	{
		strcpy( aszStrings[(*pcStrings)++], "None" );

		uint32 cSmartObjects = g_pAIDB->GetNumAISmartObjectRecords();
		for(uint32 iSmartObject=0; iSmartObject < cSmartObjects; ++iSmartObject)
		{
			// Out of space to add more strings.

			if ( (*pcStrings) + 1 == cMaxStrings )
			{
				break;
			}

			// If the currently indexed smartobject is of the same type as the node, 
			// then add it to the list.

			const AIDB_SmartObjectRecord* pSmartObject = g_pAIDB->GetAISmartObjectRecord(iSmartObject);
			if ( !pSmartObject )
			{
				continue;
			}

			if( pSmartObject->eNodeType == kNode_Vehicle )
			{
				LTStrCpy(aszStrings[(*pcStrings)++], pSmartObject->strName.c_str(), cMaxStringLength);
			}
		}

		// Alphabetize the strings, skipping the 'None' entry which is always first.

		if (*pcStrings > 1)
		{
			qsort( aszStrings+1, (*pcStrings)-1, sizeof( char * ), CaseInsensitiveCompare );		
		}

		return LT_OK;
	}

	// Handle team...
	if( LTStrIEquals( "Team", szPropName ))
	{
		TeamPopulateEditStringList( aszStrings, pcStrings, cMaxStrings, cMaxStringLength );
		return LT_OK;
	}

	// Unsupported.

	return LT_UNSUPPORTED;
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIConfigPlugin::PreHook_PropChanged
//
//	PURPOSE:	Callback handler for PropChanged plugin
//
// ----------------------------------------------------------------------- //

LTRESULT CAIConfigPlugin::PreHook_PropChanged( const char *szObjName,
											 const char *szPropName, 
											 const int  nPropType, 
											 const GenericProp &gpPropValue,
											 ILTPreInterface *pInterface,
											 const char *szModifiers )
{
	if ( LTStrIEquals( szPropName, "DamagedPlayerCommand" ) )
	{
		if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName,
													szPropName,
													nPropType,
													gpPropValue,
													pInterface,
													szModifiers ) == LT_OK )
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIConfig::HandleTeamMsg()
//
//	PURPOSE:	Handle a TEAM message...
//
// ----------------------------------------------------------------------- //

void CAIConfig::HandleTeamMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	int nTeamId = atoi( crParsedMsg.GetArg( 1 ));
	if( nTeamId < MAX_TEAMS )
	{
		ASSERT( nTeamId == ( uint8 )nTeamId );
		m_nTeamID = ( uint8 )LTCLAMP( nTeamId, 0, 255 );
	}
	else
	{
		m_nTeamID = INVALID_TEAM;
	}
}

