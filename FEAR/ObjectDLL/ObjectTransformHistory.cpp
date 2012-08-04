// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectTransformHistory.cpp
//
// PURPOSE : ObjectTransformHistory is used to cache an object's transform on a specific
//			 interval up to a specific amount of time in the past.  Transforms can then 
//			 be retrieved from a specified time stamp.
//
// CREATED : 01/21/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "Stdafx.h"
	#include "ObjectTransformHistory.h"


// Rate at which the history will log updates (ms)...
VarTrack g_vtTransformHistoryLogRate;

// Max amount of latency supported by the history (ms).
// This should include any latency from the prediction code as well as max ping...
VarTrack g_vtTransformHistoryMaxLatency;

VarTrack g_vtTransformHistoryInitialObjects;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectTransformHistory::CObjectTransformHistory
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CObjectTransformHistory::CObjectTransformHistory( )
:	m_hObject			( NULL ),
	m_aEvents			( ),
	m_nMostRecentEvent	( 0 )
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectTransformHistory::~CObjectTransformHistory
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CObjectTransformHistory::~CObjectTransformHistory( )
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectTransformHistory::Init
//
//	PURPOSE:	Initialize the transform history with the specified object...
//
// ----------------------------------------------------------------------- //

bool CObjectTransformHistory::Init( HOBJECT hObject )
{
	if( !hObject )
		return false;

	uint32 nRate = (uint32)g_vtTransformHistoryLogRate.GetFloat( );
	if( nRate < 1 )
		return false;

	// Calculate the event log size based on the frequency of updates and max latency...
	uint32 nEventLogSize = (uint32)g_vtTransformHistoryMaxLatency.GetFloat( ) / nRate;
	m_aEvents.resize( nEventLogSize );

	// Initially set the most recent event to the last element of the history array
	// This ensures proper functionality when logging the first event and when trying to
	// get an event when none have been logged...
	m_nMostRecentEvent = m_aEvents.size( ) - 1;

	m_hObject = hObject;

	return true;
}

void CObjectTransformHistory::Term()
{
	m_hObject = NULL;
	m_aEvents.clear();
	m_nMostRecentEvent = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectTransformHistory::LogEvent
//
//	PURPOSE:	Add a new event to the history...
//
// ----------------------------------------------------------------------- //

void CObjectTransformHistory::LogEvent( )
{
	uint32 nLogIndex = m_nMostRecentEvent + 1;

	// Handle array wrap around...
	if( nLogIndex >= m_aEvents.size( ))
		nLogIndex = 0;
	
	// Log the objects current transform...
	g_pLTServer->GetObjectTransform( m_hObject, &m_aEvents[nLogIndex].m_Transform );

	// At the current game time...
	m_aEvents[nLogIndex].m_nTimeMS = g_pLTServer->GetRealTimeMS( );

	// Cache index of most recent log...
	m_nMostRecentEvent = nLogIndex;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectTransformHistory::GetTransform
//
//	PURPOSE:	Retrieve the interpolated historical transform the specified number of milliseconds in the past...
//				Returns false and sets the transform to the identity if failed to properly calculate the transform...
//
// ----------------------------------------------------------------------- //

bool CObjectTransformHistory::GetTransform( uint32 nTimeMSInPast, LTRigidTransform &rTransform )
{
	// Sanity check...
	if( m_nMostRecentEvent >= m_aEvents.size( ))
	{
		LTERROR( "Trying to acess an event that is beyond the range of the history!" );
		rTransform = LTRigidTransform::GetIdentity( );
		return false;
	}

	uint32 nTimeA, nTimeB, nTimeMS;
	nTimeA = nTimeB = nTimeMS = g_pLTServer->GetRealTimeMS( ) - nTimeMSInPast;

	LTRigidTransform tA, tB;
    
	// Initially set the first transform as the current...
	if( g_pLTBase->GetObjectTransform( m_hObject, &tA ) != LT_OK )
	{
		LTERROR( "Failed to get the objects current transform" );	
		return false;
	}

	// Initially set both transforms to the current object transform...
	tB = tA;

	// Check the history logs from the most recent entry to the oldest...
	uint32 nEvent = m_nMostRecentEvent;
	
	bool bFoundTransforms = false;
	while( !bFoundTransforms )
	{
		if( nTimeMS > m_aEvents[nEvent].m_nTimeMS )
		{
			tB = m_aEvents[nEvent].m_Transform;
			nTimeB = m_aEvents[nEvent].m_nTimeMS;

			bFoundTransforms = true;
		}
		else
		{
			tA = m_aEvents[nEvent].m_Transform;
			nTimeA = m_aEvents[nEvent].m_nTimeMS;
				
			if( nEvent == 0 )
				nEvent = m_aEvents.size( );

			--nEvent;

			// Check if the end of the history was reached...
			if( nEvent == m_nMostRecentEvent )
			{
				// The time was too far in the past, just return the oldest transform...
				rTransform = tA;
				return true;
			}
		}
	}

		
	// interpolate the transform between the straddled events...
	uint32 nTimeDelta = nTimeA - nTimeB;
	float fT = 0.0f;

	if( nTimeDelta > 0.0f )
		fT = (float)(nTimeA - nTimeMS) / (float)nTimeDelta;

	rTransform.Interpolate( tA, tB, fT );
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectTransformHistoryMgr::CObjectTransformHistoryMgr
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CObjectTransformHistoryMgr::CObjectTransformHistoryMgr( )
:	m_bInitialized	( false )
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectTransformHistoryMgr::~CObjectTransformHistoryMgr
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CObjectTransformHistoryMgr::~CObjectTransformHistoryMgr( )
{
	// Delete any object history logs left...
	TObjectHistoryArray::iterator iterOTH;
	for( iterOTH = m_aObjectHistoryPool.begin( ); iterOTH != m_aObjectHistoryPool.end( ); ++iterOTH )
	{
		debug_delete( *iterOTH );
	}
	m_aObjectHistoryPool.clear( );

	for( iterOTH = m_aObjectHistory.begin( ); iterOTH != m_aObjectHistory.end( ); ++iterOTH )
	{
		debug_delete( *iterOTH );
	}
	m_aObjectHistory.clear( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectTransformHistoryMgr::Init
//
//	PURPOSE:	Initialize...
//
// ----------------------------------------------------------------------- //

void CObjectTransformHistoryMgr::Init( )
{
	if( m_bInitialized )
		return;

	if( !g_vtTransformHistoryLogRate.IsInitted( ))
	{
		g_vtTransformHistoryLogRate.Init( g_pLTBase, "TransformHistoryLogRate", NULL, 50 );
	}

	if( !g_vtTransformHistoryMaxLatency.IsInitted( ))
	{
		g_vtTransformHistoryMaxLatency.Init( g_pLTBase, "TransformHistoryMaxLatency", NULL, 370 );
	}

	if( !g_vtTransformHistoryInitialObjects.IsInitted( ))
	{
		g_vtTransformHistoryInitialObjects.Init( g_pLTBase, "TransformHistoryInitialObjects", NULL, 16 );
	}

	// The log timer needs to be running on real time, not simulation time...
	m_LogTimer.SetEngineTimer( RealTimeTimer::Instance( ));

	uint32 nReservedObjects = (uint32)g_vtTransformHistoryInitialObjects.GetFloat( );
	m_aObjectHistory.reserve( nReservedObjects );
	m_aObjectHistoryPool.reserve( nReservedObjects );

	CObjectTransformHistory *pOTH = NULL;
	for( uint32 nOTH = 0; nOTH < nReservedObjects; ++nOTH )
	{
		pOTH = debug_new( CObjectTransformHistory );
		if( pOTH )
		{
			m_aObjectHistoryPool.push_back( pOTH );
		}
	}

	// No need to initialize any more...
	m_bInitialized = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectTransformHistoryMgr::Update
//
//	PURPOSE:	Check update timer and log events when timer has elapsed...
//
// ----------------------------------------------------------------------- //

void CObjectTransformHistoryMgr::Update( )
{
	if( !m_bInitialized )
		Init( );

	if( m_LogTimer.IsTimedOut( ))
	{
		// Run through each object history and log an event...
		TObjectHistoryArray::iterator iter = m_aObjectHistory.begin( );
		while( iter != m_aObjectHistory.end( ))
		{
			(*iter)->LogEvent( );

			++iter;
		}

		// Restart the timer...
		m_LogTimer.Start( g_vtTransformHistoryLogRate.GetFloat( ) * 0.001f );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectTransformHistoryMgr::AddObject
//
//	PURPOSE:	Add an object to track and log transform events to the history...
//
// ----------------------------------------------------------------------- //

bool CObjectTransformHistoryMgr::AddObject( HOBJECT hObject )
{
	if( !hObject || !m_bInitialized )
		return false;

	// see if we have this one already
	TObjectHistoryArray::iterator iterOTH;
	for( iterOTH = m_aObjectHistory.begin( ); iterOTH != m_aObjectHistory.end( ); ++iterOTH )
	{
		CObjectTransformHistory* pOTH = *iterOTH;
		if (pOTH->m_hObject == hObject)
		{
			LTERROR("Attempt to add existing object to transform history");
			
			// reset the history on this object
			pOTH->Term();
			pOTH->Init(hObject);
			
			return true;
		}
	}
	
	// See if a new history log needs to be allocated...
	if( m_aObjectHistoryPool.empty( ))
	{
		CObjectTransformHistory *pOTH = debug_new( CObjectTransformHistory );
		if( pOTH )
		{
			m_aObjectHistoryPool.push_back( pOTH );
		}
	}

	LTASSERT( !m_aObjectHistoryPool.empty( ), "Failed to allocate new ObjectTransformHistory onto pool" );
	if( m_aObjectHistoryPool.empty( ))
		return false;

	// Grab a free history log from the pool...
	CObjectTransformHistory *pTransHistory = m_aObjectHistoryPool.back( );
	if( !pTransHistory )
	{
		LTERROR( "Failed to track object's transform history" );
		return false;
	}

	pTransHistory->m_hObject.SetReceiver( *this );
	if( !pTransHistory->Init( hObject ))
		return false;

	// Add it to the main object history array to begin logging events...
	m_aObjectHistory.push_back( pTransHistory );
	m_aObjectHistoryPool.pop_back( );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectTransformHistoryMgr::OnLinkBroken
//
//	PURPOSE:	Retrieve the interpolated historical transform of the specified object
//				the specified number of milliseconds in the past...
//				Returns false and sets the transform to the identity if failed to properly calculate the transform
//				or if the object was never specified to log events...
//
// ----------------------------------------------------------------------- //

void CObjectTransformHistoryMgr::GetHistoricalTransform( HOBJECT hObject, uint32 nTimeMSInPast, LTRigidTransform &rTransform )
{
	if( hObject )
	{
		TObjectHistoryArray::iterator iter = m_aObjectHistory.begin( );
		while( iter != m_aObjectHistory.end( ))
		{
			if( (*iter)->m_hObject == hObject )
			{
				(*iter)->GetTransform( nTimeMSInPast, rTransform );
				return;
			}

			++iter;
		}
	}

	// Failed to find the object in the list of tracked objects...
	rTransform = LTRigidTransform::GetIdentity( );
	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectTransformHistoryMgr::OnLinkBroken
//
//	PURPOSE:	If an object goes invalid remove it's transform history and stop logging events for it...
//
// ----------------------------------------------------------------------- //

void CObjectTransformHistoryMgr::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	CObjectTransformHistory *pOTH = NULL;
	TObjectHistoryArray::iterator iter = m_aObjectHistory.begin( );
	while( iter != m_aObjectHistory.end( ))
	{
		pOTH = *iter;
		if( pRef == &(pOTH->m_hObject) )
		{
			// Remove it from the main history array and place it back in the pool...
			m_aObjectHistory.erase( iter );
			pOTH->Term();
			m_aObjectHistoryPool.push_back( pOTH );
			break;
		}

		++iter;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectTransformHistoryMgr::OnLinkBroken
//
//	PURPOSE:	Retrieves the next object tracked in the list of transforms histories...
//				Returns NULL if list is empty or there is no next object...
//				Pass in NULL to get the first object.
//				Use this to iterate through the objects to get each ones transform...
//
// ----------------------------------------------------------------------- //

HOBJECT CObjectTransformHistoryMgr::GetNextTrackedObject( HOBJECT hObject )
{
	if( m_aObjectHistory.empty( ))
		return NULL;

	// The first object in the list has been requested in the specified object is null...
	if( !hObject )
	{
		TObjectHistoryArray::iterator iter = m_aObjectHistory.begin( );
		CObjectTransformHistory *pOTH = *iter;

		return pOTH->m_hObject;
	}

	CObjectTransformHistory *pOTH = NULL;
	TObjectHistoryArray::iterator iter = m_aObjectHistory.begin( );
	while( iter != m_aObjectHistory.end( ))
	{
		pOTH = *iter;
		++iter;

		// If this is the current object the next one is the one they are looking for...
		if( pOTH->m_hObject == hObject )
		{
			// Check if it's the last one...
			if( iter != m_aObjectHistory.end( ))
			{
				pOTH = *iter;
				return pOTH->m_hObject;
			}
		}
	}

    // Reached the end of the list or never found the specified object...
	return NULL;
}

// EOF
