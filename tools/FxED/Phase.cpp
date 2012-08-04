//------------------------------------------------------------------
//
//   MODULE  : PHASE.CPP
//
//   PURPOSE : Implements class CPhase
//
//   CREATED : On 11/9/98 At 4:57:50 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "phase.h"

//------------------------------------------------------------------
//
//   FUNCTION : CPhase()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CPhase::CPhase()
{
	m_nPhaseLength = 10000;
	m_dwUniqueID   = 0;
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CPhase
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CPhase::~CPhase()
{
	// Call Term()

	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CPhase
//
//------------------------------------------------------------------

BOOL CPhase::Init()
{
	// Success !!

	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CPhase
//
//------------------------------------------------------------------

void CPhase::Term()
{
	CLinkListNode<CTrack *> *pNode = m_collTracks.GetHead();

	while (pNode)
	{
		delete pNode->m_Data;

		pNode = pNode->m_pNext;
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : AddTrack()
//
//   PURPOSE  : Adds a track to the phase
//
//------------------------------------------------------------------

CTrack* CPhase::AddTrack()
{
	CTrack *pTrack = new CTrack;
	if (!pTrack) return NULL;

	m_collTracks.AddTail(pTrack);

	return pTrack;
}

//------------------------------------------------------------------
//
//   FUNCTION : GetKeyByID()
//
//   PURPOSE  : Returns a key given an ID
//
//------------------------------------------------------------------

CKey* CPhase::GetKeyByID(DWORD dwID)
{
	CLinkListNode<CTrack *> *pTrackNode = m_collTracks.GetHead();

	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		while (pKeyNode)
		{	
			if (pKeyNode->m_Data->GetID() == dwID)
			{
				return pKeyNode->m_Data;
			}

			pKeyNode = pKeyNode->m_pNext;
		}

		pTrackNode = pTrackNode->m_pNext;
	}

	// Failure !!

	return NULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : SetupUniqueID()
//
//   PURPOSE  : Makes sure we are ready for the next unique ID
//
//------------------------------------------------------------------

void CPhase::SetupUniqueID()
{
	CLinkListNode<CTrack *> *pTrackNode = m_collTracks.GetHead();

	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		while (pKeyNode)
		{	
			if (pKeyNode->m_Data->GetID() > m_dwUniqueID)
			{
				m_dwUniqueID = pKeyNode->m_Data->GetID();
			}

			pKeyNode = pKeyNode->m_pNext;
		}

		pTrackNode = pTrackNode->m_pNext;
	}

	m_dwUniqueID ++;
}


//------------------------------------------------------------------
//
//   FUNCTION : GetNumFX()
//
//   PURPOSE  : Returns the total number of FX in this phase
//
//------------------------------------------------------------------

int CPhase::GetNumFX()
{
	int nCount = 0;
	
	CLinkListNode<CTrack *> *pTrackNode = m_collTracks.GetHead();

	while (pTrackNode)
	{
		CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

		while (pKeyNode)
		{
			nCount ++;
			
			pKeyNode = pKeyNode->m_pNext;
		}
		
		pTrackNode = pTrackNode->m_pNext;
	}

	return nCount;
}