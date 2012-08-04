// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerTracker.cpp
//
// PURPOSE : Tracks list of players.  Used to track players sent messages
//			 that the server needs responses to.	
//
// CREATED : 02/21/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerTracker.h"
#include "PlayerObj.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	PlayerTracker::PlayerTracker
//
//  PURPOSE:	Constructor.
//
// ----------------------------------------------------------------------- //

PlayerTracker::PlayerTracker( )
{
	m_pPlayerTrackerReceiver = NULL;
	m_bAborted = false; 
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	PlayerTracker::~PlayerTracker
//
//  PURPOSE:	Destructor.
//
// ----------------------------------------------------------------------- //

PlayerTracker::~PlayerTracker( )
{
	Term( );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	PlayerTracker::Init
//
//  PURPOSE:	Initializes the object by taking a snapsnot of the current players.  
//				Returns false if no players.
//
// ----------------------------------------------------------------------- //

bool PlayerTracker::Init( IPlayerTrackerReceiver& playerTrackerReceiver )
{
	LTObjRefNotifier ltObjRefNotifier;
	ltObjRefNotifier.SetReceiver( *this );
	bool bRet = false;

	// Start fresh.
	Term( );

	// Add all the players to the list.
	CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
	while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
	{
		CPlayerObj* pPlayerObj = *iter;
		if( pPlayerObj )
		{
			// Make sure the player has a client.
			if( pPlayerObj->GetClient( ))
			{
				// Put objects in notifiers so we know if they get deleted.
				ltObjRefNotifier = pPlayerObj->m_hObject;
				m_PlayerList.push_back( ltObjRefNotifier );

				// Log that we got at least one player.
				bRet = true;
			}
		}
		else
		{
			ASSERT( !"PlayerTracker::Init:  Invalid playerobj entry." );
		}

		iter++;
	}

	// Save the receiver.
	m_pPlayerTrackerReceiver = &playerTrackerReceiver;

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	PlayerTracker::Term
//
//  PURPOSE:	Terminates the object.
//
// ----------------------------------------------------------------------- //

void PlayerTracker::Term( )
{
	m_pPlayerTrackerReceiver = NULL;
	m_bAborted = false;
	m_PlayerList.clear( );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	PlayerTracker::Init
//
//  PURPOSE:	Removes a client from the list.  Returns player if found, null if not.
//
// ----------------------------------------------------------------------- //

CPlayerObj* PlayerTracker::RemoveClient( HCLIENT hClient )
{
	// Check inputs.
	if( !hClient )
	{
		ASSERT( !"PlayerTracker::RemoveClient: Invalid hClient." );
		return NULL;
	}

	// Get the playerobj for this client.
    void *pData = g_pLTServer->GetClientUserData( hClient );
	CPlayerObj* pPlayerObjSender = (CPlayerObj*)pData;
	if( !pPlayerObjSender )
	{
		ASSERT( !"PlayerTracker::RemoveClient: Invalid playerobj sender." );
		return NULL;
	}

	// Assume we don't find it in our list.
	CPlayerObj* pRet = NULL;

	// Search the players told to send us save data and match it with this message.
	PlayerList::iterator iter = m_PlayerList.begin( );
	while( iter != m_PlayerList.end( ))
	{
		HOBJECT hObj = *iter;

		// Check if the player went away.
		if( !hObj )
		{
			// Lost the playerobj, client must have left.
			iter = m_PlayerList.erase( iter );
			continue;
		}

		// Convert to playerobj.
		CPlayerObj* pPlayerObj = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( hObj ));
		if( !pPlayerObj )
		{
			ASSERT( !"PlayerTracker::RemoveClient: Object not a playerobj." );
			iter = m_PlayerList.erase( iter );
			continue;
		}

		// Check if the client is this playerobj.
		if( pPlayerObj != pPlayerObjSender )
		{
			// Go to the next element.
			iter++;
			continue;
		}

		// Remove the player from our list.
		m_PlayerList.erase( iter );

		// Acknowledge we found it in our list.
		pRet = pPlayerObj;
		break;
	}

	// Return the player found.
	return pRet;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerTracker::OnLinkBroken
//
//	PURPOSE:	Handle broken player links.
//
// --------------------------------------------------------------------------- //

void PlayerTracker::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	// Look through the saved references and remove it.
	PlayerList::iterator iter = m_PlayerList.begin( );
	while( iter != m_PlayerList.end( ))
	{
		if( pRef == &( *iter ))
		{
			m_PlayerList.erase( iter );
			break;
		}

		iter++;
	}

	// Check if we just went to zero, which is an abort condition.
	if( m_PlayerList.size( ) == 0 )
	{
		m_bAborted = true;

		// Call the abort.
		if( m_pPlayerTrackerReceiver )
			m_pPlayerTrackerReceiver->OnPlayerTrackerAbort( );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerTracker::Update
//
//	PURPOSE:	Checks if any of the clients are in trouble with 
//				their connections.  If so, it drops them.
//
// --------------------------------------------------------------------------- //

void PlayerTracker::Update( )
{
	// Nothing to do.
	if( m_PlayerList.size( ) == 0 )
		return;

	// Search the players told to send us save data and match it with this message.
	PlayerList::iterator iter = m_PlayerList.begin( );
	while( iter != m_PlayerList.end( ))
	{
		HOBJECT hObj = *iter;

		// Convert to playerobj.
		CPlayerObj* pPlayerObj = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( hObj ));
		if( !pPlayerObj || !pPlayerObj->GetClient( ))
		{
//			ASSERT( !"PlayerTracker::RemoveClient: Invalid playerobj." );
			iter = m_PlayerList.erase( iter );
			continue;
		}

		// Make sure it has a client still.
		if( !pPlayerObj->GetClient( ))
		{
			iter = m_PlayerList.erase( iter );
			continue;
		}

		// Check the client's connection.
		if( g_pGameServerShell->ClientConnectionInTrouble( pPlayerObj->GetClient( )))
		{
			iter = m_PlayerList.erase( iter );
			g_pLTServer->KickClient( pPlayerObj->GetClient( ));
			continue;
		}

		iter++;
	}

	// Check if we just went to zero, which is an abort condition.
	if( m_PlayerList.size( ) == 0 )
	{
		m_bAborted = true;

		// Call the abort.
		if( m_pPlayerTrackerReceiver )
			m_pPlayerTrackerReceiver->OnPlayerTrackerAbort( );
	}
}