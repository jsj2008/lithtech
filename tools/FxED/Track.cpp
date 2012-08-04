//------------------------------------------------------------------
//
//   MODULE  : TRACK.CPP
//
//   PURPOSE : Implements class CTrack
//
//   CREATED : On 11/9/98 At 4:52:30 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "Track.h"

//------------------------------------------------------------------
//
//   FUNCTION : CTrack()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CTrack::CTrack()
{
	m_bSelected = FALSE;
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CTrack
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CTrack::~CTrack()
{
	// Call Term()

	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CTrack
//
//------------------------------------------------------------------

BOOL CTrack::Init()
{
	// Success !!

	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CTrack
//
//------------------------------------------------------------------

void CTrack::Term()
{
	CLinkListNode<CKey *> *pNode = m_collKeys.GetHead();

	while (pNode)
	{
		delete pNode->m_Data;

		pNode = pNode->m_pNext;
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : ArrangeKeys()
//
//   PURPOSE  : 
//
//------------------------------------------------------------------

void CTrack::ArrangeKeys(CKey *pSquishKey)
{
	// Firstly, sort the keys based on ascending order of start time

	BOOL bSorted = FALSE;

	while (!bSorted)
	{
		bSorted = TRUE;

		CLinkListNode<CKey *> *pNode = m_collKeys.GetHead();

		while (pNode->m_pNext)
		{
			if (pNode->m_Data->GetStartTime() > pNode->m_pNext->m_Data->GetStartTime())
			{
				// Swap the two nodes

				CKey *pTmpKey = pNode->m_Data;
				pNode->m_Data = pNode->m_pNext->m_Data;
				pNode->m_pNext->m_Data = pTmpKey;

				bSorted = FALSE;
			}
			
			pNode = pNode->m_pNext;
		}
	}

	// Now, put the squish key into the right place

	CLinkListNode<CKey *> *pNode = m_collKeys.Find(pSquishKey);

	if (pNode)
	{
		// Squish the beginning
		
		if ((!pNode->m_pPrev) && (pSquishKey->GetStartTime() < 0))
		{
			pSquishKey->SetStartTime(0);
		}

		if ((pNode->m_pPrev) && (pSquishKey->GetStartTime() < pNode->m_pPrev->m_Data->GetEndTime()))
		{
			pSquishKey->SetStartTime(pNode->m_pPrev->m_Data->GetEndTime());
		}

		// Squish the end

		if ((pNode->m_pNext) && (pSquishKey->GetEndTime() > pNode->m_pNext->m_Data->GetStartTime()))
		{
			pSquishKey->SetEndTime(pNode->m_pNext->m_Data->GetStartTime());
		}
	}
}