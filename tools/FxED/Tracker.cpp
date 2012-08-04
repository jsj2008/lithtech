//------------------------------------------------------------------
//
//   MODULE  : TRACKER.CPP
//
//   PURPOSE : Implements class CTracker
//
//   CREATED : On 11/9/98 At 6:20:11 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "Tracker.h"

//------------------------------------------------------------------
//
//   FUNCTION : CTracker()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CTracker::CTracker()
{
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CTracker
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CTracker::~CTracker()
{
	// Call Term()

	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CTracker
//
//------------------------------------------------------------------

BOOL CTracker::Init()
{
	// Success !!

	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CTracker
//
//------------------------------------------------------------------

void CTracker::Term()
{
}

