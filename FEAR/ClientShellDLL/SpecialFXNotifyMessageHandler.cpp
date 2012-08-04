// ----------------------------------------------------------------------- //
//
// MODULE  : SpecialFXNotifyMessageHandler.cpp
//
// PURPOSE : Defines the handler of special effect messages that need to retry
//				to read themselves due to being dependent on other objects.
//
// CREATED : 8/05/05
//
// (c) 1998-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "SpecialFXNotifyMessageHandler.h"
#include "GameClientShell.h"

SpecialFXNotifyMessageHandler::SpecialFXNotifyMessageHandler( )
{
}

SpecialFXNotifyMessageHandler::~SpecialFXNotifyMessageHandler( )
{
	Term( );
}


bool SpecialFXNotifyMessageHandler::Init( )
{
	Term( );
	m_SpecialFXNotifyMsgBank.Init();

	return true;
}

void SpecialFXNotifyMessageHandler::Term( )
{
	m_aObjectDependentMessages.clear( );
	m_SpecialFXNotifyMsgBank.Term( );
}

bool SpecialFXNotifyMessageHandler::AddMessage( ILTMessage_Read& msg, HOBJECT hNewObject )
{
	SpecialFXNotifyMessage* pSFXMsg = m_SpecialFXNotifyMsgBank.New( );

	pSFXMsg->m_SFXMsg = msg.Clone( );
	pSFXMsg->m_hObject = hNewObject;
	m_aObjectDependentMessages.push_back( pSFXMsg );

	return true;
}

SpecialFXNotifyMessageHandler::SpecialFXNotifyMessage* SpecialFXNotifyMessageHandler::GetSFXObjectEntry( HOBJECT hPollingObject )
{
	SpecialFXNotifyMessage *pSFXNotifyMsg = NULL;
	TSFXNotifyMsgList::iterator itSFXNotifyMsgCur = m_aObjectDependentMessages.begin( );
	for( ; itSFXNotifyMsgCur != m_aObjectDependentMessages.end( ); itSFXNotifyMsgCur++ )
	{
		pSFXNotifyMsg = *itSFXNotifyMsgCur;

		// Check if object matches the polling message.
		if( pSFXNotifyMsg->m_hObject == hPollingObject )
			return pSFXNotifyMsg;
	}

	return NULL;
}

// If the conditions of the SFX Message have changed, then the server can
// send a new sfx message to override the old one.
bool SpecialFXNotifyMessageHandler::ChangeMessage( ILTMessage_Read& msg, HOBJECT hPollingObject )
{
	SpecialFXNotifyMessage *pSFXNotifyMsg = GetSFXObjectEntry( hPollingObject );
	if( !pSFXNotifyMsg )
		return false;

	// Change the store sfx msg to the new one.
	pSFXNotifyMsg->m_SFXMsg = msg.Clone( );

	return true;
}

// Appends messages to object to read when polling is complete.
bool SpecialFXNotifyMessageHandler::AppendMessage( ILTMessage_Read& msg, HOBJECT hPollingObject )
{
	SpecialFXNotifyMessage *pSFXNotifyMsg = GetSFXObjectEntry( hPollingObject );
	if( !pSFXNotifyMsg )
		return false;

	// Add the message to the saved off sfx msgs to be read off later
	// when the object is fully initialized.
	CLTMsgRef_Read newMsg;
	pSFXNotifyMsg->m_AppendedMsgs.push_back(newMsg);
	pSFXNotifyMsg->m_AppendedMsgs.back() = msg.Clone( );

	return true;
}

SpecialFXNotifyMessageHandler::TAppendedMsgList* SpecialFXNotifyMessageHandler::GetAppendedMessages( HOBJECT hPollingObject )
{
	SpecialFXNotifyMessage *pSFXNotifyMsg = GetSFXObjectEntry( hPollingObject );
	if( !pSFXNotifyMsg )
		return NULL;

	return &pSFXNotifyMsg->m_AppendedMsgs;
}

void SpecialFXNotifyMessageHandler::Update( )
{
	if( m_aObjectDependentMessages.empty( ) )
		return;

	// Reprocess messages that are dependent on objects.
	// Move the full list to a new one so messages may be re-added if the object is still invalid.
	TSFXNotifyMsgList SFXNotifyMsgList;
	SFXNotifyMsgList.swap( m_aObjectDependentMessages );

	TSFXNotifyMsgList::iterator itSFXNotifyMsgCur = SFXNotifyMsgList.begin( );
	while( itSFXNotifyMsgCur != SFXNotifyMsgList.end( ) )
	{
		// Try to create the SpecialFX that are dependent on target objects but
		// previously had invalid target objects.
		SpecialFXNotifyMessage *pSFXNotifyMsg = *itSFXNotifyMsgCur;

		// Make sure the object is still valid.
		if( pSFXNotifyMsg->m_hObject )
		{
			// Tell the object about the sfx message.
			g_pGameClientShell->GetSFXMgr()->HandleSFXMsg( pSFXNotifyMsg->m_hObject, pSFXNotifyMsg->m_SFXMsg );

			// Tell the object about each of the appended messages.
			TAppendedMsgList::iterator itAppendedMsg = pSFXNotifyMsg->m_AppendedMsgs.begin( );
			for( ; itAppendedMsg != pSFXNotifyMsg->m_AppendedMsgs.end( ); itAppendedMsg++ )
			{
				CLTMsgRef_Read& msg = *itAppendedMsg;
				g_pGameClientShell->GetSFXMgr()->OnSFXMessage( msg );
			}
		}

		itSFXNotifyMsgCur = SFXNotifyMsgList.erase( itSFXNotifyMsgCur );
		m_SpecialFXNotifyMsgBank.Delete( pSFXNotifyMsg );
	}
}
