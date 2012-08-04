//////////////////////////////////////////////////////////////////////
// TrackerMgr.cpp - Implementation of the tracker manager

#include "bdefs.h"
#include "trackermgr.h"
#include "globalhotkeydb.h"

CUITrackerMgr::CUITrackerMgr() :
	m_pActiveTracker(NULL)
{
}

CUITrackerMgr::~CUITrackerMgr()
{
	ClearTrackerList();
}

void CUITrackerMgr::ClearTrackerList()
{
	for (DWORD uLoop = 0; uLoop < m_cTrackerList; uLoop++)
		delete m_cTrackerList[uLoop];
	m_cTrackerList.RemoveAll();
}

void CUITrackerMgr::AddTracker(CUITracker *cTracker)
{
	// Don't allow NULL trackers
	if (!cTracker)
		return;

	// Add the tracker to the list
	m_cTrackerList.Add(cTracker);

	SortTrackers();
}

BOOL CUITrackerMgr::RemoveTracker(CUITracker *cTracker)
{
	// Don't try to remove NULL trackers
	if (!cTracker)
		return FALSE;

	// Remove the tracker from the list
	BOOL bResult = FALSE;
	DWORD uLoop = 0;
	while (uLoop < m_cTrackerList)
	{
		if (m_cTrackerList[uLoop] == cTracker)
		{
			m_cTrackerList.Remove(uLoop);
			// Save the result
			bResult = TRUE;
		}
		else
			uLoop++;
	}
	// Handle the active tracker getting removed
	if (GetActiveTracker() == cTracker)
		SetActiveTracker(NULL);

	return bResult;
}

CUITracker *CUITrackerMgr::FindTracker(LPCTSTR pName) const
{
	for (DWORD uLoop = 0; uLoop < m_cTrackerList; uLoop++)
		if (m_cTrackerList[uLoop]->GetName().CompareNoCase(pName) == 0)
			return m_cTrackerList[uLoop];
	return NULL;
}

//utility function that will determine if the first tracker masks the second tracker,
//meaning that it contains all of the events of the second, and several more
static BOOL DoesTrackerMaskOther(CUITracker* pMasker, CUITracker* pMaskee)
{
	//sanity checks
	ASSERT(pMasker);
	ASSERT(pMaskee);

	const CHotKey* pMaskerKey = pMasker->GetName().GetLength() > 0 ? CGlobalHotKeyDB::m_DB.GetHotKey( pMasker->GetName() ) : NULL;
	const CHotKey* pMaskeeKey = pMaskee->GetName().GetLength() > 0 ? CGlobalHotKeyDB::m_DB.GetHotKey( pMaskee->GetName() ) : NULL;

	//if we can't get one, we can't compare
	if((pMaskerKey == NULL) || (pMaskeeKey == NULL))
	{
		return FALSE;
	}

	//if they have less than or equal numbers of events, they can't mask
	if(pMaskerKey->GetNumEvents() <= pMaskeeKey->GetNumEvents())
	{
		return FALSE;
	}

	//now we need to compare all the events in the maskee and see if they match
	//in the masker
	for(uint32 nCurrEvent = 0; nCurrEvent < pMaskeeKey->GetNumEvents(); nCurrEvent++)
	{
		const CUIEvent& Event = pMaskeeKey->GetEvent(nCurrEvent);

		BOOL bMatch = FALSE;

		//see if it matches
		for(uint32 nTestEvent = 0; nTestEvent < pMaskerKey->GetNumEvents(); nTestEvent++)
		{
			const CUIEvent& TestEvent = pMaskerKey->GetEvent(nTestEvent);

			//compare the two
			if(TestEvent == Event)
			{
				bMatch = TRUE;
			}
		}

		//no match? It can't mask then
		if(bMatch == FALSE)
		{
			return FALSE;
		}
	}

	//all events are contained, it is masked
	return TRUE;
}

BOOL CUITrackerMgr::ProcessEvent(const CUIEvent &cEvent)
{
	//determines if trackers should be processing events, or just watching
	//them (they should be in watch mode if another tracker is already active)
	BOOL bWatching = FALSE;

	// Send the event to the active tracker first
	if (GetActiveTracker())
	{
		bWatching = GetActiveTracker()->ProcessEvent(cEvent);
		if(bWatching == FALSE)
		{
			//the tracker stopped being active
			SetActiveTracker(NULL);
		}
	}

	for (uint32 uLoop = 0; uLoop < m_cTrackerList; uLoop++)
	{
		CUITracker* pTracker = m_cTrackerList[uLoop];

		// Don't process the active tracker in the main loop
		if (pTracker == GetActiveTracker())
			continue;

		// Send the event to watching trackers
		if (bWatching)
		{
			//we need to watch for trackers that mask the currently active tracker
			if(GetActiveTracker() && DoesTrackerMaskOther(pTracker, GetActiveTracker()))
			{
				//this one masks, so we should process it instead of just watch
				if(pTracker->ProcessEvent(cEvent))
				{
					GetActiveTracker()->WasMasked();

					//it activated
					SetActiveTracker(pTracker);
				}
			}
			else
			{
				//this one doesn't mask, just watch
				pTracker->WatchEvent(cEvent);
			}
		}
		else
		{
			// If nobody's active yet, process the event
			bWatching = pTracker->ProcessEvent(cEvent);
			// If somebody decided to take over, keep them as the active tracker
			if (bWatching)
			{
				SetActiveTracker(pTracker);
			}
		}
	}

	return bWatching;
}

void CUITrackerMgr::Cancel()
{
	DWORD uLoop;
	
	// Send the cancel notification to all trackers
	for (uLoop = 0; uLoop < m_cTrackerList.GetSize(); uLoop++)
		m_cTrackerList[uLoop]->Cancel();

	// Turn off the active tracker
	SetActiveTracker(NULL);
}

//used to compare two trackers for sorting them into the list
static int CompareTrackers( const void* elem1, const void* elem2 )
{
	CUITracker* pTracker1 = *(CUITracker**)elem1;
	CUITracker* pTracker2 = *(CUITracker**)elem2;

	CString name = pTracker1->GetName();

	const CHotKey* pHotKey1 = pTracker1->GetName().GetLength() > 0 ? CGlobalHotKeyDB::m_DB.GetHotKey( pTracker1->GetName() ) : NULL;
	const CHotKey* pHotKey2 = pTracker2->GetName().GetLength() > 0 ? CGlobalHotKeyDB::m_DB.GetHotKey( pTracker2->GetName() ) : NULL;

	if( pHotKey1 && pHotKey2 )
	{
		if( pHotKey1->GetNumEvents() > pHotKey2->GetNumEvents() )
		{
			return -1;
		}
		else if( pHotKey1->GetNumEvents() < pHotKey2->GetNumEvents() )
		{
			return 1;
		}
		else
		{
			//sort based upon name
			return strcmp(pHotKey1->GetEventName(), pHotKey2->GetEventName());
		}
	}
	else if( pHotKey1 )
	{
		return -1;
	}
	else if( pHotKey2 )
	{
		return 1;
	}
	return 0;
}

void CUITrackerMgr::SortTrackers()
{
	// keep the tracker list sorted by # of keys used for the tracker, in descending order, so that trackers with higher # of keys are processed first
	int nTrackers = m_cTrackerList.GetSize();
	if( nTrackers > 0 )
	{
		int i;
		CUITracker** pTrackers = new CUITracker*[ nTrackers ];
		for( i = 0; i < nTrackers; ++i )
		{
			pTrackers[ i ] = (CUITracker*)m_cTrackerList[ i ];
		}
		::qsort( pTrackers, nTrackers, sizeof(CUITracker*), CompareTrackers );

		for( i = 0; i < nTrackers; ++i )
		{
			m_cTrackerList.SetAt( i, pTrackers[ i ] );
		}
		delete[] pTrackers;
	}
}

void CUITrackerMgr::SetActiveTracker(CUITracker* pTracker)
{
	//stop the current tracker
	if(m_pActiveTracker)
	{
		m_pActiveTracker->SetActive(FALSE);
	}
	m_pActiveTracker = pTracker;
}

void CUITrackerMgr::FlushTrackers()
{
	SortTrackers();	
	for( int i = 0; i < m_cTrackerList; ++i )
	{
		//tell all of the trackers that they need to reset themselves
		m_cTrackerList[ i ]->FlushTracker();
	}
	SetActiveTracker( NULL );
}
