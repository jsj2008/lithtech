// ----------------------------------------------------------------------- //
//
// MODULE  : Dialogue.cpp
//
// PURPOSE : The Dialogue object implementation
//
// CREATED : 10/15/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//
	
	#include "stdafx.h"
	#include "ObjectMsgs.h"
	#include "AI.h"
	#include "Dialogue.h"
	#include "AIGoalMgr.h"
	#include "AIGoalTalk.h"
	#include "AIGoalSpecialDamage.h"
	#include "AIGoalFlee.h"
	#include "ParsedMsg.h"
	#include "AIUtils.h"
	#include "CharacterMgr.h"
	#include "PlayerObj.h"

LINKFROM_MODULE( Dialogue );

//
// Defines...
//

	#define ADD_DIALOGUE_PROP(num, group) \
		ADD_REALPROP_FLAG(Delay##num##, 0.0f, group)\
		ADD_STRINGPROP_FLAG(Dialogue##num##, 0, group)\
		ADD_STRINGPROP_FLAG(StartDialogueCommand##num##, "", group | PF_NOTIFYCHANGE)
		

BEGIN_CLASS( Dialogue )

	ADD_STRINGPROP_FLAG( StartCommand, "", PF_NOTIFYCHANGE )
	ADD_STRINGPROP_FLAG( FinishedCommand, "", PF_NOTIFYCHANGE )
	ADD_STRINGPROP_FLAG( CleanUpCommand, "", PF_NOTIFYCHANGE )
	ADD_BOOLPROP( RemoveWhenComplete, 1 )
	
	PROP_DEFINEGROUP(Dialogue, PF_GROUP(1))
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

END_CLASS_DEFAULT_FLAGS_PLUGIN( Dialogue, GameBase, NULL, NULL, 0, CDialoguePlugin )

// Register our messages with the command mgr plugin

CMDMGR_BEGIN_REGISTER_CLASS( Dialogue )

	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )

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


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::Dialogue
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

Dialogue::Dialogue( )
:	GameBase				( OT_NORMAL ),
	m_bNotifiedSpeakers		( LTFALSE ),
	m_hCurSpeaker			( LTNULL ),
	m_nCurDialogue			( 0 ),
	m_fNextDialogueStart	( -1.0f ),
	m_hstrStartCommand		( LTNULL ),
	m_hstrFinishedCommand	( LTNULL ),
	m_hstrCleanUpCommand	( LTNULL ),
	m_bOn					( LTFALSE )
{
	for( int i = 0; i < MAX_DIALOGUES; ++i )
	{
		m_fDialogueDelay[i]			= 0;
		m_hstrDialogue[i]			= LTNULL;
		m_hstrDialogueStartCmd[i]	= LTNULL;
	}

	m_bRemoveWhenComplete = true;
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
	// Free all created hstrings...

	FREE_HSTRING( m_hstrStartCommand );
	FREE_HSTRING( m_hstrFinishedCommand );
	FREE_HSTRING( m_hstrCleanUpCommand );

	for( int i = 0; i < MAX_DIALOGUES; ++i )
	{
		FREE_HSTRING( m_hstrDialogue[i] );
		FREE_HSTRING( m_hstrDialogueStartCmd[i] );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::EngineMessageFn
//
//  PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //

uint32 Dialogue::EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData )
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
					ReadProps( pOCS );
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

void Dialogue::ReadProps( ObjectCreateStruct *pOCS )
{
	ASSERT( pOCS != LTNULL );

	char		szPropName[32] = {0};
	GenericProp gProp;

	if( g_pLTServer->GetPropGeneric( "StartCommand", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
		{
			m_hstrStartCommand = g_pLTServer->CreateString( gProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "FinishedCommand", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
		{
			m_hstrFinishedCommand = g_pLTServer->CreateString( gProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "CleanUpCommand", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
		{
			m_hstrCleanUpCommand = g_pLTServer->CreateString( gProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "RemoveWhenComplete", &gProp ) == LT_OK )
	{
		m_bRemoveWhenComplete = !!gProp.m_Bool;
	}
	
	// Read every dialogue property value...

	for( int i = 0; i < MAX_DIALOGUES; ++i )
	{
		sprintf( szPropName, "Delay%i", i+1 );
		if( g_pLTServer->GetPropGeneric( szPropName, &gProp ) == LT_OK )
		{
			m_fDialogueDelay[i] = gProp.m_Float;
		}

		sprintf( szPropName, "Dialogue%i", i+1 );
		if( g_pLTServer->GetPropGeneric( szPropName, &gProp ) == LT_OK )
		{
			if( gProp.m_String[0] )
			{
				m_hstrDialogue[i] = g_pLTServer->CreateString( gProp.m_String );
			}
		}

		sprintf( szPropName, "StartDialogueCommand%i", i+1 );
		if( g_pLTServer->GetPropGeneric( szPropName, &gProp ) == LT_OK )
		{
			if( gProp.m_String[0] )
			{
				m_hstrDialogueStartCmd[i] = g_pLTServer->CreateString( gProp.m_String );
			}
		}
	}
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
		m_bOn = LTFALSE;
		return;
	}

	SetNextUpdate( UPDATE_NEXT_FRAME );

	if( !UpdateDialogue( ) )
	{
		// This must be done before the finish or cleanup commands are processed.

		ResetTalkGoals();

		// Dialogue finished so send command...

		if( m_hstrFinishedCommand )
		{
			const char *pCmd = g_pLTServer->GetStringData( m_hstrFinishedCommand );

			if( g_pCmdMgr->IsValidCmd( pCmd ))
			{
				g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
			}
		}

		// Do some cleanup work

		HandleOff();
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

LTBOOL Dialogue::NotifySpeakers( )
{
	// If we haven't yet done so, let all the dialogue participants know
	// they're under Dialogue control

	LTBOOL bEveryoneIsReady = LTTRUE;
	if( !m_bNotifiedSpeakers )
	{
		for( int iWho = 0; iWho < MAX_DIALOGUES; ++iWho )
		{
			if( m_hstrDialogue[iWho] )
			{
				// Dialogue string is in the form <sound> <speaker>

				ConParse cpDialogue;
				cpDialogue.Init( g_pLTServer->GetStringData( m_hstrDialogue[iWho] ));
				
				g_pCommonLT->Parse( &cpDialogue );
				
				if( cpDialogue.m_nArgs != 2 || !cpDialogue.m_Args[1] )
					continue;

				HOBJECT hSpeaker;
				if( LT_OK == FindNamedObject( cpDialogue.m_Args[1], hSpeaker, LTTRUE ) )
				{
					if( IsAI( hSpeaker ))
					{
						CAI* pAI = (CAI*)g_pLTServer->HandleToObject( hSpeaker );
						pAI->LinkDialogueObject( m_hObject );

						// If the AI has a Talk goal, check if he is able to talk.
						// CanTalk will return false if the AI is busy doing something else.

						AIASSERT( pAI->GetGoalMgr(), hSpeaker, "Dialogue::NotifySpeakers: AI does not have goal mgr" );
						CAIGoalTalk* pGoalTalk = (CAIGoalTalk*)pAI->GetGoalMgr()->FindGoalByType( kGoal_Talk );
						if( pGoalTalk && pGoalTalk->HasTalkNode() )
						{
							if( !pGoalTalk->RequestDialogue( m_hObject ) )
							{
								return LTFALSE;
							}

							pGoalTalk->StartDialogue();

							// Do not set m_bNotifiedSpeakers until speakers
							// are in position.

							if( !pGoalTalk->IsInTalkPosition() )
							{
								bEveryoneIsReady = LTFALSE;
							}
						}

						// Wake up knocked out AI, if they do not hate the player.
						// Stop AI from panicking, if they do not hate the player.

						CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
						if( pPlayer )
						{
							CharacterAlignment eAlignment = GetAlignment( pPlayer->GetRelationSet(), pAI->GetRelationData() );
							if( eAlignment != HATE )
							{
								CAIGoalSpecialDamage* pGoalSpecialDamage = (CAIGoalSpecialDamage*)pAI->GetGoalMgr()->FindGoalByType( kGoal_SpecialDamage );
								if( pGoalSpecialDamage )
								{
									pGoalSpecialDamage->InterruptSpecialDamage( LTFALSE );
								}

								CAIGoalFlee* pGoalFlee = (CAIGoalFlee*)pAI->GetGoalMgr()->FindGoalByType( kGoal_Flee );
								if( pGoalFlee )
								{
									pGoalFlee->Relax();
								}
							}
						}
					}
				}
				else {

					// Return false if a participant does not exist 
					// (so an AI does not have a conversation with his dead buddy).

					ResetTalkGoals();

					HandleOff();

					return LTFALSE;
				}
			}
		}
	}

	m_bNotifiedSpeakers = bEveryoneIsReady;
	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::UpdateDialogue
//
//  PURPOSE:	Handle updating the dialogues.  Return false when all dialogues
//				are done playing.
//
// ----------------------------------------------------------------------- //

LTBOOL Dialogue::UpdateDialogue( )
{
	// Wait until speakers are ready.

	if( !m_bNotifiedSpeakers )
	{
		return LTTRUE;
	}

	LTFLOAT	fTime = g_pLTServer->GetTime();
	LTBOOL	bDone = LTFALSE;

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
			CAI* pAI = (CAI*)g_pLTServer->HandleToObject( m_hCurSpeaker );

			AIASSERT( pAI->GetGoalMgr(), m_hCurSpeaker, "Dialogue::UpdateDialogue: AI does not have goal mgr" );
			CAIGoalTalk* pGoalTalk = (CAIGoalTalk*)pAI->GetGoalMgr()->FindGoalByType( kGoal_Talk );
			if( pGoalTalk && pGoalTalk->HasTalkNode() )
			{
				pGoalTalk->StopTalking();
			}
		}

		// Clear our speaker...

        m_hCurSpeaker = LTNULL;

		++m_nCurDialogue;

		// Set up the time to start the next dialogue...

		if( m_nCurDialogue < MAX_DIALOGUES )
		{
			m_fNextDialogueStart = fTime + m_fDialogueDelay[m_nCurDialogue];
		}
		else
		{
			return LTFALSE;
		}
	}

	if( !m_hCurSpeaker )
	{
		// See if we are done...

		if( m_nCurDialogue >= MAX_DIALOGUES || !m_hstrDialogue[m_nCurDialogue] )
		{
			return LTFALSE;
		}

		// Is it time to start next dialogue...

		if( m_fNextDialogueStart >= 0.0f && m_fNextDialogueStart <= fTime )
		{
			return StartDialogue();
		}
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::StartDialogue
//
//  PURPOSE:	Start the appropriate Dialogue
//
// ----------------------------------------------------------------------- //

LTBOOL Dialogue::StartDialogue( )
{
	// Check for an initial message when the whole dialogue sequence starts...

	if( m_nCurDialogue == 0 )
	{
		if( m_hstrStartCommand )
		{
            const char* pCmd = g_pLTServer->GetStringData( m_hstrStartCommand );

			if( g_pCmdMgr->IsValidCmd( pCmd ))
			{
				g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
			}
		}
	}

	// Send the current dialogue start command...

	if( m_nCurDialogue < MAX_DIALOGUES )
	{
		if( m_hstrDialogueStartCmd[m_nCurDialogue] )
		{
            const char *pCmd = g_pLTServer->GetStringData( m_hstrDialogueStartCmd[m_nCurDialogue] );

			if( g_pCmdMgr->IsValidCmd( pCmd ))
			{
				g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
			}
		}
	}

	// Get the current dialogue sound and who is going to play it...
	// Dialogue string is in the form <sound> <speaker>

	ConParse cpDialogue;
	cpDialogue.Init( g_pLTServer->GetStringData( m_hstrDialogue[m_nCurDialogue] ));
	
	g_pCommonLT->Parse( &cpDialogue );
				
	if( cpDialogue.m_nArgs != 2 )
		return LTFALSE;

	char *szSound	= cpDialogue.m_Args[0];
	char *szSpeaker	= cpDialogue.m_Args[1];

	if( !szSound[0] || !szSpeaker[0] )
		return LTFALSE;

	HOBJECT hSpeaker;
	if( LT_OK == FindNamedObject( szSpeaker, hSpeaker) )
	{
		// If the speaker is an AI, animate the conversation.

		if( IsAI( hSpeaker ) )
		{
			CAI* pAI = (CAI*)g_pLTServer->HandleToObject( hSpeaker );

			AIASSERT( pAI->GetGoalMgr(), hSpeaker, "Dialogue::StartDialogue: AI does not have goal mgr" );
			CAIGoalTalk* pGoalTalk = (CAIGoalTalk*)pAI->GetGoalMgr()->FindGoalByType( kGoal_Talk );
			if( pGoalTalk && pGoalTalk->HasTalkNode() )
			{
				pGoalTalk->StartTalking();
			}
		}

		if( IsCharacter( hSpeaker ))
		{
			CCharacter *pChar = (CCharacter*)g_pLTServer->HandleToObject( hSpeaker );
			if( pChar )
			{
				pChar->PlayDialogue( szSound );
				
				m_hCurSpeaker = hSpeaker;

				return LTTRUE;
			}
		}
	}
	
	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::OnTrigger
//
//  PURPOSE:	Handle messages from other objects...
//
// ----------------------------------------------------------------------- //

bool Dialogue::OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");

	if( cMsg.GetArg(0) == s_cTok_On )
	{
		HandleOn();
	}
	else if( cMsg.GetArg(0) == s_cTok_Off )
	{
		// This must be done before the finish or cleanup commands are processed.

		ResetTalkGoals();

		HandleOff();
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::HandleOn
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

void Dialogue::HandleOn( )
{
	if( m_bOn ) return;
	m_bOn = LTTRUE;

	m_fNextDialogueStart = g_pLTServer->GetTime() + m_fDialogueDelay[0];

	SetNextUpdate( UPDATE_NEXT_FRAME );
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
	for( int iWho = 0; iWho < MAX_DIALOGUES; ++iWho )
	{
		if( m_hstrDialogue[iWho] )
		{
			// Dialogue string is in the form <sound> <speaker>

			ConParse cpDialogue;
			cpDialogue.Init( g_pLTServer->GetStringData( m_hstrDialogue[iWho] ));
				
			g_pCommonLT->Parse( &cpDialogue );
				
			if( cpDialogue.m_nArgs != 2 || !cpDialogue.m_Args[1] )
				continue;

			HOBJECT hSpeaker;
			if( LT_OK == FindNamedObject( cpDialogue.m_Args[1], hSpeaker) )
			{
				if( IsAI( hSpeaker ))
				{
					CAI* pAI = (CAI*)g_pLTServer->HandleToObject( hSpeaker );

					// Reset the talk goal for all participants.

					CAIGoalTalk* pGoalTalk = (CAIGoalTalk*)pAI->GetGoalMgr()->FindGoalByType( kGoal_Talk );
					if( pGoalTalk && pGoalTalk->HasTalkNode() )
					{
						pGoalTalk->ResetDialogue();
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Dialogue::HandleOff
//
//  PURPOSE:	Handle shutting down and cleaning up the dialogue
//
// ----------------------------------------------------------------------- //

void Dialogue::HandleOff( )
{
	// [KLS 3/3/02] - Updated so you could turn of a dialogue object that hasn't
	// yet been turned on (makes setting up cinematics easier)...

	if (m_bOn)
	{
		// Unlink all the dialogue participants...

		for( int iWho = 0; iWho < MAX_DIALOGUES; ++iWho )
		{
			if( m_hstrDialogue[iWho] )
			{
				// Dialogue string is in the form <sound> <speaker>

				ConParse cpDialogue;
				cpDialogue.Init( g_pLTServer->GetStringData( m_hstrDialogue[iWho] ));
				
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

			m_hCurSpeaker = LTNULL;
		}
	}  // if (m_bOn)


	m_bOn = LTFALSE;


	// Process the clean up command...

	if( m_hstrCleanUpCommand )
	{
		const char *pCmd = g_pLTServer->GetStringData( m_hstrCleanUpCommand );
		if( g_pCmdMgr->IsValidCmd( pCmd ))
		{
			g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
		}
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
	SAVE_HSTRING( m_hstrStartCommand );
	SAVE_HSTRING( m_hstrFinishedCommand );
	SAVE_HSTRING( m_hstrCleanUpCommand );
	SAVE_BOOL( m_bOn );

	// Save Dialogue data...

	for( int i = 0; i < MAX_DIALOGUES; ++i )
	{
		SAVE_FLOAT( m_fDialogueDelay[i] );
		SAVE_HSTRING( m_hstrDialogue[i] );
		SAVE_HSTRING( m_hstrDialogueStartCmd[i] );
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
	LOAD_HSTRING( m_hstrStartCommand );
	LOAD_HSTRING( m_hstrFinishedCommand );
	LOAD_HSTRING( m_hstrCleanUpCommand );
	LOAD_BOOL( m_bOn );

	// Load Dialogue data...

	for( int i = 0; i < MAX_DIALOGUES; ++i )
	{
		LOAD_FLOAT( m_fDialogueDelay[i] );
		LOAD_HSTRING( m_hstrDialogue[i] );
		LOAD_HSTRING( m_hstrDialogueStartCmd[i] );
	}

	LOAD_BOOL( m_bRemoveWhenComplete );
}

