// ----------------------------------------------------------------------- //
//
// MODULE  : Dialogue.cpp
//
// PURPOSE : The Dialogue object implementation
//
// CREATED : 10/15/01
//
// (c) 2001-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//
	
	#include "Stdafx.h"
	#include "ObjectMsgs.h"
	#include "AI.h"
	#include "Dialogue.h"
	#include "AIGoalMgr.h"
	#include "ParsedMsg.h"
	#include "AIUtils.h"
	#include "CharacterMgr.h"
	#include "PlayerObj.h"
	#include "DialogueDB.h"
	#include "StringEditMgr.h"

LINKFROM_MODULE( Dialogue );

//
// Defines...
//

	#define ADD_DIALOGUE_PROP(num, group) \
		ADD_REALPROP_FLAG(Delay##num, 0.0f, group, "The amount of time, in seconds, to wait before starting this portion of the conversation.")\
		ADD_STRINGPROP_FLAG(Dialogue##num, "", group, "This specifies the actual sound that gets played and the object that plays it.  You must enter in the String ID of the voice file to play followed by the character who plays it.  EX: (IDS_DIALOGUE_1000) (Player)")\
		ADD_STRINGPROP_FLAG(Icon##num, "", group | PF_STATICLIST, "This specifies the transmission icon for this line.") \
		ADD_COMMANDPROP_FLAG(StartDialogueCommand##num, "", group | PF_NOTIFYCHANGE, "A command that will be executed when this portion of the conversation begins to play.")
		

BEGIN_CLASS( Dialogue )

	ADD_COMMANDPROP_FLAG( StartCommand, "", PF_NOTIFYCHANGE, "A command that will be executed when the Dialogue object is first turned on." )
	ADD_COMMANDPROP_FLAG( FinishedCommand, "", PF_NOTIFYCHANGE, "A command that will be executed when the Dialogue natrually finishes playing all the dialogues." )
	ADD_COMMANDPROP_FLAG( CleanUpCommand, "", PF_NOTIFYCHANGE, "A command that will always be executed whenever the Dialogue stops, either naturally or by messaged to stop." )
	ADD_BOOLPROP( RemoveWhenComplete, 1, "Remove the dialogue object after dialogue is complete." )
	
	PROP_DEFINEGROUP(Dialogue, PF_GROUP(1), "This is a subset that allows you to create a conversation between characters.")
		ADD_DIALOGUE_PROP(1, PF_GROUP(1))
		ADD_DIALOGUE_PROP(2, PF_GROUP(1))
		ADD_DIALOGUE_PROP(3, PF_GROUP(1))
		ADD_DIALOGUE_PROP(4, PF_GROUP(1))
		ADD_DIALOGUE_PROP(5, PF_GROUP(1))
		ADD_DIALOGUE_PROP(6, PF_GROUP(1))
		ADD_DIALOGUE_PROP(7, PF_GROUP(1))
		ADD_DIALOGUE_PROP(8, PF_GROUP(1))
		ADD_DIALOGUE_PROP(9, PF_GROUP(1))
		ADD_DIALOGUE_PROP(10, PF_GROUP(1))
		ADD_DIALOGUE_PROP(11, PF_GROUP(1))
		ADD_DIALOGUE_PROP(12, PF_GROUP(1))
		ADD_DIALOGUE_PROP(13, PF_GROUP(1))
		ADD_DIALOGUE_PROP(14, PF_GROUP(1))
		ADD_DIALOGUE_PROP(15, PF_GROUP(1))
		ADD_DIALOGUE_PROP(16, PF_GROUP(1))
		ADD_DIALOGUE_PROP(17, PF_GROUP(1))
		ADD_DIALOGUE_PROP(18, PF_GROUP(1))
		ADD_DIALOGUE_PROP(19, PF_GROUP(1))
		ADD_DIALOGUE_PROP(20, PF_GROUP(1))

	ADD_PREFETCH_RESOURCE_PROPS()

END_CLASS_FLAGS_PLUGIN_PREFETCH(Dialogue, GameBase, 0, CDialoguePlugin, DefaultPrefetch<Dialogue>, "Dialogue objects are used to create conversations between characters." )

// Register our messages with the command mgr plugin

CMDMGR_BEGIN_REGISTER_CLASS( Dialogue )

	ADD_MESSAGE( ON, 1, NULL, MSG_HANDLER( Dialogue, HandleOnMsg ), "ON", "Tells the objects specified in the Dialogue object to play their dialogue", "msg Dialogue ON" )
	ADD_MESSAGE( OFF, 1, NULL, MSG_HANDLER( Dialogue, HandleOffMsg ), "OFF", "Tells the objects specified in the Dialogue object to stop playing their dialogue, sends the CleanUpCommand if one was specified, and removes the object if RemoveWhenComplete was set to true.", "msg Dialogue OFF" )

CMDMGR_END_REGISTER_CLASS( Dialogue, GameBase )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDialoguePlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //

LTRESULT CDialoguePlugin::PreHook_PropChanged( const char *szObjName,
											   const char *szPropName, 
											   const int  nPropType, 
											   const GenericProp &gpPropValue,
											   ILTPreInterface *pInterface,
											   const char *szModifiers )
{
	// Only our commands are marked for change notification so just send it to the CommandMgr..

	if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
												szPropName, 
												nPropType, 
												gpPropValue,
												pInterface,
												szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

LTRESULT CDialoguePlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
	uint32* pcStrings,
	const uint32 cMaxStrings,
	const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	// Handle team...

	if( LTSubStrIEquals("Icon", szPropName, 4 ))
	{
		LTStrCpy( aszStrings[(*pcStrings)++], "<none>", cMaxStringLength );

		uint32 nNumIcons = DATABASE_CATEGORY( Dialogue ).GetNumRecords();
		if (cMaxStrings < nNumIcons + 1)
		{
			LTERROR("Too many Dialogue icons");
			nNumIcons = cMaxStrings - 1;
		}
		
		for( uint32 i = 0; i < nNumIcons; ++i )
		{
			HRECORD hRec = DATABASE_CATEGORY( Dialogue ).GetRecordByIndex(i);
			if (hRec)
			{
				LTStrCpy( aszStrings[(*pcStrings)++], g_pLTDatabase->GetRecordName(hRec), cMaxStringLength );
			}
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}




// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::Dialogue
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

Dialogue::Dialogue( )
:	GameBase				( OT_NORMAL ),
	m_bNotifiedSpeakers		( false ),
	m_hCurSpeaker			( NULL ),
	m_nCurDialogue			( 0 ),
	m_fNextDialogueStart	( -1.0f ),
	m_sStartCommand			( ),
	m_sFinishedCommand		( ),
	m_sCleanUpCommand		( ),
	m_bOn					( false ),
	m_faDialogueDelay		( ),
	m_saDialogue			( ),
	m_saIcon				( ),
	m_saDialogueStartCmd	( ),
	m_bRemoveWhenComplete	( true )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::~Dialogue
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

Dialogue::~Dialogue( )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::EngineMessageFn
//
//  PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //

uint32 Dialogue::EngineMessageFn( uint32 messageID, void *pData, float fData )
{
	switch( messageID )
	{
		case MID_UPDATE :
		{
			Update( );
		}
		break;

		case MID_PRECREATE :
		{
			// Let the GameBase handle the message first

			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			ObjectCreateStruct *pOCS = (ObjectCreateStruct*)pData;

			if( pOCS )
			{
				if( PRECREATE_WORLDFILE == fData )
				{
					ReadProps( &pOCS->m_cProperties );
				}
			}

			// Important!! - We already sent the message to the GameBase so DONT do it again.

			return dwRet;
		}
		break;

		case MID_OBJECTCREATED :
		{
			if( OBJECTCREATED_SAVEGAME != fData )
			{
				SetNextUpdate( UPDATE_NEVER );
			}
		}
		break;

		case MID_SAVEOBJECT :
		{
			Save( (ILTMessage_Write*)pData );
		}
		break;

		case MID_LOADOBJECT :
		{
			Load( (ILTMessage_Read*)pData );
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn( messageID, pData, fData );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::ReadProps
//
//  PURPOSE:	Read the property values...
//
// ----------------------------------------------------------------------- //

void Dialogue::ReadProps( const GenericPropList *pProps )
{

	LTASSERT( pProps != NULL, "Invalid Object Create Struct" );

	m_sStartCommand			= pProps->GetCommand( "StartCommand", "" );
	m_sFinishedCommand		= pProps->GetCommand( "FinishedCommand", "" );
	m_sCleanUpCommand		= pProps->GetCommand( "CleanUpCommand", "" );
	m_bRemoveWhenComplete	= pProps->GetBool( "RemoveWhenComplete", m_bRemoveWhenComplete );
		
	// Read every dialogue property value...

	m_saDialogue.reserve( MAX_DIALOGUES );
	m_saIcon.reserve( MAX_DIALOGUES );
	m_faDialogueDelay.reserve( MAX_DIALOGUES );
	m_saDialogueStartCmd.reserve( MAX_DIALOGUES );

	char szPropName[32] = {0};
	for( uint8 nDialogue = 1; nDialogue < MAX_DIALOGUES; ++nDialogue )
	{
		LTSNPrintF( szPropName, ARRAY_LEN(szPropName), "Dialogue%i", nDialogue );
		const char *pszDialogue = pProps->GetString( szPropName, "" );
		
		if( pszDialogue && pszDialogue[0] )
		{
			m_saDialogue.push_back( pszDialogue );
		

			// The delay and start commands are only valid if there is dialogue...

			LTSNPrintF( szPropName, ARRAY_LEN(szPropName), "Delay%i", nDialogue );
			m_faDialogueDelay.push_back( pProps->GetReal( szPropName, 0.0f ));
			
			LTSNPrintF( szPropName, ARRAY_LEN(szPropName), "StartDialogueCommand%i", nDialogue );
			m_saDialogueStartCmd.push_back( pProps->GetCommand( szPropName, "" ));

			LTSNPrintF( szPropName, ARRAY_LEN(szPropName), "Icon%i", nDialogue );
			m_saIcon.push_back( pProps->GetString( szPropName, "" ));

		}
	}

	// Shrink-to-fit the arrays..

	StringArray( m_saDialogue ).swap( m_saDialogue );
	FloatArray( m_faDialogueDelay ).swap( m_faDialogueDelay );
	StringArray( m_saDialogueStartCmd ).swap( m_saDialogueStartCmd );
	StringArray( m_saIcon ).swap( m_saIcon );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::Update
//
//  PURPOSE:	Handle the update...
//
// ----------------------------------------------------------------------- //

void Dialogue::Update( )
{
	// Cancel dialog if speakers are not ready.
	// Do not run the cleanup yet, because dialog may 
	// be valid later, if speakers return to position.

	if( !NotifySpeakers() )
	{
		m_bOn = false;
		return;
	}

	SetNextUpdate( UPDATE_NEXT_FRAME );

	if( !UpdateDialogue( ) )
	{
		// This must be done before the finish or cleanup commands are processed.

		ResetTalkGoals();

		// Dialogue finished so send command...

		if( !m_sFinishedCommand.empty() )
		{
			g_pCmdMgr->QueueCommand( m_sFinishedCommand.c_str(), m_hCurSpeaker, m_hObject );
		}

		// Do some cleanup work

		TurnOff( );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::NotifySpeakers
//
//  PURPOSE:	Notify participants that a dialog is starting.
//              Return false if unable to notify someone. 
//
// ----------------------------------------------------------------------- //

bool Dialogue::NotifySpeakers( )
{
	// If we haven't yet done so, let all the dialogue participants know
	// they're under Dialogue control

	bool bEveryoneIsReady = true;
	if( !m_bNotifiedSpeakers )
	{
		StringArray::iterator iter;
		for( iter = m_saDialogue.begin(); iter != m_saDialogue.end(); ++iter )
		{
			if( !(*iter).empty() )
			{
				// Dialogue string is in the form <sound> <speaker>

				ConParse cpDialogue;
				cpDialogue.Init( (*iter).c_str() );
				
				g_pCommonLT->Parse( &cpDialogue );
				
				if( cpDialogue.m_nArgs != 2 || !cpDialogue.m_Args[1] )
					continue;

				HOBJECT hSpeaker;
				if( LT_OK == FindNamedObject( cpDialogue.m_Args[1], hSpeaker, true ) )
				{
					if( IsAI( hSpeaker ))
					{
						CAI* pAI = (CAI*)g_pLTServer->HandleToObject( hSpeaker );
						pAI->LinkDialogueObject( m_hObject );
					}
				}
				else {

					// Return false if a participant does not exist 
					// (so an AI does not have a conversation with his dead buddy).

					ResetTalkGoals();

					TurnOff( );

					return false;
				}
			}
		}
	}

	m_bNotifiedSpeakers = bEveryoneIsReady;
	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::UpdateDialogue
//
//  PURPOSE:	Handle updating the dialogues.  Return false when all dialogues
//				are done playing.
//
// ----------------------------------------------------------------------- //

bool Dialogue::UpdateDialogue( )
{
	// Wait until speakers are ready.

	if( !m_bNotifiedSpeakers )
	{
		return true;
	}

	double	fTime = g_pLTServer->GetTime();
	bool	bDone = false;

	if( m_hCurSpeaker )
	{
		// If sound is done, stop it and wait for new sound...
		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( m_hCurSpeaker );
		if( pChar )
		{
			bDone = !pChar->IsPlayingDialogue();
		}
	}

	if( bDone )
	{
		// If the speaker is an AI, stop animating the conversation.

		if( IsAI( m_hCurSpeaker ) )
		{
/*			CAI* pAI = (CAI*)g_pLTServer->HandleToObject( m_hCurSpeaker );

			AIASSERT( pAI->GetGoalMgr(), m_hCurSpeaker, "Dialogue::UpdateDialogue: AI does not have goal mgr" );
			CAIGoalTalk* pGoalTalk = (CAIGoalTalk*)pAI->GetGoalMgr()->FindGoalByType( kGoal_Talk );
			if( pGoalTalk && pGoalTalk->HasTalkNode() )
			{
				pGoalTalk->StopTalking();
			}
*/
		}

		// Clear our speaker...

		m_hCurSpeaker = NULL;

		++m_nCurDialogue;

		// Set up the time to start the next dialogue...

		if( m_nCurDialogue < m_faDialogueDelay.size() )
		{
			m_fNextDialogueStart = fTime + m_faDialogueDelay[m_nCurDialogue];
		}
		else
		{
			return false;
		}
	}

	if( !m_hCurSpeaker )
	{
		// See if we are done...

		if( m_nCurDialogue >= m_saDialogue.size() || m_saDialogue[m_nCurDialogue].empty() )
		{
			return false;
		}

		// Is it time to start next dialogue...

		if( m_fNextDialogueStart >= 0.0f && m_fNextDialogueStart <= fTime )
		{
			return StartDialogue();
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::StartDialogue
//
//  PURPOSE:	Start the appropriate Dialogue
//
// ----------------------------------------------------------------------- //

bool Dialogue::StartDialogue( )
{
	// Check for an initial message when the whole dialogue sequence starts...

	if( m_nCurDialogue == 0 )
	{
		if( !m_sStartCommand.empty() )
		{
			g_pCmdMgr->QueueCommand( m_sStartCommand.c_str(), m_hCurSpeaker, m_hObject );
		}
	}

	// Send the current dialogue start command...

	if( m_nCurDialogue < m_saDialogueStartCmd.size() )
	{
		if( !m_saDialogueStartCmd[m_nCurDialogue].empty() )
		{
			g_pCmdMgr->QueueCommand( m_saDialogueStartCmd[m_nCurDialogue].c_str(), m_hCurSpeaker, m_hObject );
		}
	}

	// Get the current dialogue sound and who is going to play it...
	// Dialogue string is in the form <sound> <speaker>

	if( m_nCurDialogue >= m_saDialogue.size() )
		return false;

	ConParse cpDialogue;
	cpDialogue.Init( m_saDialogue[m_nCurDialogue].c_str() );
	
	g_pCommonLT->Parse( &cpDialogue );
				
	if( (cpDialogue.m_nArgs < 2)  ||
		(cpDialogue.m_nArgs > 3) )
		return false;

	char *szSound	= cpDialogue.m_Args[0];
	char *szSpeaker	= cpDialogue.m_Args[1];
	bool bUseRadioSound=false;

	if (cpDialogue.m_nArgs == 3)
	{
		if (LTStrIEquals(cpDialogue.m_Args[2], "Radio") )
		{
			bUseRadioSound = true;
		}
	}

	if( !szSound[0] || !szSpeaker[0] )
		return false;

	HOBJECT hSpeaker;
	if( LT_OK == FindNamedObject( szSpeaker, hSpeaker) )
	{
		// If the speaker is an AI, animate the conversation.

		if( IsAI( hSpeaker ) )
		{
/*			CAI* pAI = (CAI*)g_pLTServer->HandleToObject( hSpeaker );

			AIASSERT( pAI->GetGoalMgr(), hSpeaker, "Dialogue::StartDialogue: AI does not have goal mgr" );
			CAIGoalTalk* pGoalTalk = (CAIGoalTalk*)pAI->GetGoalMgr()->FindGoalByType( kGoal_Talk );
			if( pGoalTalk && pGoalTalk->HasTalkNode() )
			{
				pGoalTalk->StartTalking();
			}
*/
		}

		if( IsCharacter( hSpeaker ))
		{
			CCharacter *pChar = (CCharacter*)g_pLTServer->HandleToObject( hSpeaker );
			if( pChar )
			{
				pChar->PlayDialogue( szSound, m_saIcon[m_nCurDialogue].c_str() , bUseRadioSound);
				
				m_hCurSpeaker = hSpeaker;

				return true;
			}
		}
	}
	
	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::HandleOnMsg
//
//  PURPOSE:	Handle a ON message...
//
// ----------------------------------------------------------------------- //

void Dialogue::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( m_bOn )
		return;

	m_bOn = true;

	m_fNextDialogueStart = g_pLTServer->GetTime() + ( !m_faDialogueDelay.empty() ? m_faDialogueDelay[0] : 0.0f );

	SetNextUpdate( UPDATE_NEXT_FRAME );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::HandleOffMsg
//
//  PURPOSE:	Handle a OFF message...
//
// ----------------------------------------------------------------------- //

void Dialogue::HandleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// This must be done before the finish or cleanup commands are processed...
	ResetTalkGoals( );
	TurnOff( );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::ResetTalkGoals
//
//  PURPOSE:	Reset Talk goals if dialogue has ended.
//				This must be done before the finish or cleanup commands are processed.
//
// ----------------------------------------------------------------------- //

void Dialogue::ResetTalkGoals( )
{
	StringArray::iterator iter;
	for( iter = m_saDialogue.begin(); iter != m_saDialogue.end(); ++iter )
	{
		if( !(*iter).empty() )
		{
			// Dialogue string is in the form <sound> <speaker>

			ConParse cpDialogue;
			cpDialogue.Init( (*iter).c_str() );
				
			g_pCommonLT->Parse( &cpDialogue );
				
			if( cpDialogue.m_nArgs != 2 || !cpDialogue.m_Args[1] )
				continue;

			HOBJECT hSpeaker;
			if( LT_OK == FindNamedObject( cpDialogue.m_Args[1], hSpeaker) )
			{
				if( IsAI( hSpeaker ))
				{
/*					CAI* pAI = (CAI*)g_pLTServer->HandleToObject( hSpeaker );

					// Reset the talk goal for all participants.

					CAIGoalTalk* pGoalTalk = (CAIGoalTalk*)pAI->GetGoalMgr()->FindGoalByType( kGoal_Talk );
					if( pGoalTalk && pGoalTalk->HasTalkNode() )
					{
						pGoalTalk->ResetDialogue();
					}
*/
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::TurnOff
//
//  PURPOSE:	Handle shutting down and cleaning up the dialogue
//
// ----------------------------------------------------------------------- //

void Dialogue::TurnOff( )
{
	if (m_bOn)
	{
		// Unlink all the dialogue participants...

		StringArray::iterator iter;
		for( iter = m_saDialogue.begin(); iter != m_saDialogue.end(); ++iter )
		{
			if( !(*iter).empty() )
			{
				// Dialogue string is in the form <sound> <speaker>

				ConParse cpDialogue;
				cpDialogue.Init( (*iter).c_str() );
				
				g_pCommonLT->Parse( &cpDialogue );
				
				if( cpDialogue.m_nArgs != 2 || !cpDialogue.m_Args[1] )
					continue;

				HOBJECT hSpeaker;
				if( LT_OK == FindNamedObject( cpDialogue.m_Args[1], hSpeaker) )
				{
					if( IsAI( hSpeaker ))
					{
						CAI* pAI = (CAI*)g_pLTServer->HandleToObject( hSpeaker );
						pAI->UnlinkDialogueObject( m_hObject);
					}
				}
			}
		}

		// If we have a current speaker, make sure he is done talking

		if( m_hCurSpeaker )
		{
			CCharacter *pChar = (CCharacter*)g_pLTServer->HandleToObject( m_hCurSpeaker );
			if( pChar )
			{
				pChar->StopDialogue( );
			}

			m_hCurSpeaker = NULL;
		}
	}  // if (m_bOn)


	m_bOn = false;


	// Process the clean up command...

	if( !m_sCleanUpCommand.empty() )
	{
		g_pCmdMgr->QueueCommand( m_sCleanUpCommand.c_str(), m_hCurSpeaker, m_hObject );
	}

	// Check if we should remove ourselves our reset to be turned on again.
	if( m_bRemoveWhenComplete )
	{
		g_pLTServer->RemoveObject( m_hObject );
	}
	else
	{
		m_bNotifiedSpeakers = false;
		m_hCurSpeaker = NULL;
		m_nCurDialogue = 0;
		m_fNextDialogueStart = -1.0f;
		SetNextUpdate( UPDATE_NEVER );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::Save
//
//  PURPOSE:	Save the object...
//
// ----------------------------------------------------------------------- //

void Dialogue::Save( ILTMessage_Write *pMsg )
{
	if( !pMsg ) return;

	SAVE_BOOL( m_bNotifiedSpeakers );
	SAVE_HOBJECT( m_hCurSpeaker );
	SAVE_BYTE( m_nCurDialogue );
	SAVE_TIME( m_fNextDialogueStart );
	SAVE_STDSTRING( m_sStartCommand );
	SAVE_STDSTRING( m_sFinishedCommand );
	SAVE_STDSTRING( m_sCleanUpCommand );
	SAVE_BOOL( m_bOn );

	// Save Dialogue data...

	{
		SAVE_DWORD( m_faDialogueDelay.size() );
		
		FloatArray::iterator iter;
		for( iter = m_faDialogueDelay.begin(); iter != m_faDialogueDelay.end(); ++iter )
		{
			SAVE_FLOAT( (*iter) );
		}
	}

	{
		SAVE_DWORD( m_saDialogue.size() );

		StringArray::iterator iter;
		for( iter = m_saDialogue.begin(); iter != m_saDialogue.end(); ++iter )
		{
			SAVE_STDSTRING( (*iter) );
		}
	}

	{
		SAVE_DWORD( m_saIcon.size() );

		StringArray::iterator iter;
		for( iter = m_saIcon.begin(); iter != m_saIcon.end(); ++iter )
		{
			SAVE_STDSTRING( (*iter) );
		}
	}

	{
		SAVE_DWORD( m_saDialogueStartCmd.size() );

		StringArray::iterator iter;
		for( iter = m_saDialogueStartCmd.begin(); iter != m_saDialogueStartCmd.end(); ++iter )
		{
			SAVE_STDSTRING( (*iter) );
		}
	}

	SAVE_BOOL( m_bRemoveWhenComplete );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::Load
//
//  PURPOSE:	Load the object...
//
// ----------------------------------------------------------------------- //

void Dialogue::Load( ILTMessage_Read *pMsg )
{
	if( !pMsg ) return;

	LOAD_BOOL( m_bNotifiedSpeakers );
	LOAD_HOBJECT( m_hCurSpeaker );
	LOAD_BYTE( m_nCurDialogue );
	LOAD_TIME( m_fNextDialogueStart );
	LOAD_STDSTRING( m_sStartCommand );
	LOAD_STDSTRING( m_sFinishedCommand );
	LOAD_STDSTRING( m_sCleanUpCommand );
	LOAD_BOOL( m_bOn );

	// Load Dialogue data...

	uint32 nSize;
	
	{
		LOAD_DWORD( nSize );
	
		m_faDialogueDelay.clear();
		m_faDialogueDelay.resize( nSize );
		
		for( uint i = 0; i < nSize; ++i )
		{
			LOAD_FLOAT( m_faDialogueDelay[i] );
		}
	}

	{
		LOAD_DWORD( nSize );

		m_saDialogue.clear();
		m_saDialogue.resize( nSize );

		for( uint i = 0; i < nSize; ++i )
		{
			LOAD_STDSTRING( m_saDialogue[i] );
		}
	}

	{
		LOAD_DWORD( nSize );

		m_saIcon.clear();
		m_saIcon.resize( nSize );

		for( uint i = 0; i < nSize; ++i )
		{
			LOAD_STDSTRING( m_saIcon[i] );
		}
	}

	{
		LOAD_DWORD( nSize );

		m_saDialogueStartCmd.clear();
		m_saDialogueStartCmd.resize( nSize );

		for( uint i = 0; i < nSize; ++i )
		{
			LOAD_STDSTRING( m_saDialogueStartCmd[i] );
		}
	}

	LOAD_BOOL( m_bRemoveWhenComplete );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Dialogue::GetPrefetchResourceList
//
//	PURPOSE:	Determines the list of all needed resources
//
// ----------------------------------------------------------------------- //

void Dialogue::GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources )
{
	// loop through the dialog entries
	char szPropName[32] = {0};
	for (uint8 nDialogueIndex = 1; nDialogueIndex < MAX_DIALOGUES; ++nDialogueIndex)
	{
		LTSNPrintF(szPropName, ARRAY_LEN(szPropName), "Dialogue%i", nDialogueIndex);

		char szDialogString[MAX_PATH];
		pInterface->GetPropString(pszObjectName, szPropName, szDialogString, LTARRAYSIZE(szDialogString), "");

		if (!LTStrEmpty(szDialogString))
		{
			// get the first argument in the string
			std::string strDialogString(szDialogString);
			size_t nSpacePos = strDialogString.find(" ");

			if (nSpacePos != strDialogString.npos)
			{
				// look for the "radio" specification
				bool bUseRadio = (strDialogString.find("radio") != strDialogString.npos);

				std::string strStringId = strDialogString.substr(0, nSpacePos);

				// get the voice file from the StringEdit runtime
				std::string strVoiceFile = g_pLTIStringEdit->GetVoicePath(g_pLTDBStringEdit, strStringId.c_str());
				if( g_pLTIStringEdit->DoesVoicePathExist(g_pLTDBStringEdit, strStringId.c_str()) &&
					!LTStrEmpty(strVoiceFile.c_str()) )
				{
					pInterface->AddResourceToRegion(strVoiceFile.c_str(), NULL);

					// if it's a radio sound we need to insert an 'R' and add that file as well
					if (bUseRadio)
					{
						size_t tpos = strVoiceFile.find(".");
						if( tpos != strVoiceFile.npos )
							strVoiceFile.insert(tpos, "R");
						else
							strVoiceFile += "R";
						pInterface->AddResourceToRegion(strVoiceFile.c_str(), NULL);
					}
				}
				else
				{
					if( pInterface )
						pInterface->ErrorMsg( "Invalid voice path '%s' for StringId '%s' used by dialogue object '%s', property '%s', value '%s'", strVoiceFile.c_str(), strStringId.c_str(), pszObjectName, szPropName, szDialogString );
				}
			}
		}
	}
}
