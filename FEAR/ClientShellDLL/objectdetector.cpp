// **************************************************************************** //
//
//	Project:	The Dark
//	File:		objectdetector.cpp
//
//	Purpose:	This class manages the tracking of a single object instance.
//
//	Copyright (C) 2004 (Monolith Productions)
//
// **************************************************************************** //

#include "stdafx.h"

#ifndef __OBJECTDETECTOR_H__
#include "objectdetector.h"
#endif//__OBJECTDETECTOR_H__

#ifndef __LTINTERSECT_H__
#include "ltintersect.h"
#endif//__LTINTERSECT_H__

// **************************************************************************** //

#define OD_INVALID_CLEAR_DELAY			-1000.0f

// **************************************************************************** //

ObjectDetectorLink::ObjectDetectorLink()
{
	m_pDetector = NULL;
	m_pPrev = NULL;
	m_pNext = NULL;
	m_pUserData = NULL;
}

// **************************************************************************** //

ObjectDetectorLink::~ObjectDetectorLink()
{
	ObjectDetector::ReleaseLink( *this );
}

// **************************************************************************** //

ObjectDetector::ObjectDetector()
{
	m_tTransform.Init();

	m_fParamsForward = 0.0f;
	m_vParamsDirection.Init();
	m_vParamsFOV.Init();
	m_vParamsFOVWeights.Init();
	m_vParamsSphere.Init();
	m_vParamsCylinder.Init();

	m_nBehaviorFlags = 0;
	m_nUserFlagVerification = 0;
	memset( m_fVerifyFailureDelays, 0, sizeof( float ) * ODBF_TESTSAVAILABLE );

	m_pLineOfSiteFn = NULL;
	m_pLineOfSiteUserData = NULL;
	m_pSpatialFn = NULL;
	m_pSpatialUserData = NULL;
	m_pCustomTestFn = NULL;
	m_pCustomTestUserData = NULL;

	m_nRegisteredObjects = 0;

	m_pTrackedLink = NULL;
	m_vTrackedSpatialPosition.Init();
	m_vTrackedSpatialDimensions.Init();

	memset( m_fActiveVerifyFailureDelays, 0, sizeof( float ) * ODBF_TESTSAVAILABLE );
	m_fActiveClearDelay = OD_INVALID_CLEAR_DELAY;
}

// **************************************************************************** //

ObjectDetector::~ObjectDetector()
{
	// Detach all the links
	while( m_iRootLink.m_pNext )
	{
		ReleaseLink( *m_iRootLink.m_pNext );
	}
}

// **************************************************************************** //

void ObjectDetector::RegisterObject( ObjectDetectorLink& iLink, HOBJECT hObject, void* pUserData )
{
	// Make sure it isn't registered somewhere else first...
	ReleaseLink( iLink );

	// Make sure the object is valid
	if( !hObject )
	{
		return;
	}

	// Assign the new object and detector
	iLink.m_hObject = hObject;
	iLink.m_pDetector = this;
	iLink.m_pUserData = pUserData;
	++m_nRegisteredObjects;

	// Now hook it up to the root link...
	iLink.m_pPrev = &m_iRootLink;
	iLink.m_pNext = m_iRootLink.m_pNext;

	if( m_iRootLink.m_pNext )
	{
		m_iRootLink.m_pNext->m_pPrev = &iLink;
	}

	m_iRootLink.m_pNext = &iLink;
}

// **************************************************************************** //

void ObjectDetector::ReleaseLink( ObjectDetectorLink& iLink )
{
	if( iLink.m_pPrev )
	{
		iLink.m_pPrev->m_pNext = iLink.m_pNext;
	}

	if( iLink.m_pNext )
	{
		iLink.m_pNext->m_pPrev = iLink.m_pPrev;
	}

	if( iLink.m_pDetector )
	{
		--iLink.m_pDetector->m_nRegisteredObjects;

		if( iLink.m_pDetector->m_pTrackedLink == &iLink )
		{
			iLink.m_pDetector->m_pTrackedLink = NULL;
		}
	}

	iLink.m_pDetector = NULL;
	iLink.m_pPrev = NULL;
	iLink.m_pNext = NULL;
}

// **************************************************************************** //

uint32 ObjectDetector::GetNumRegisteredObjects()
{
	return m_nRegisteredObjects;
}

// **************************************************************************** //

void ObjectDetector::SetTransform( HOBJECT hObject )
{
	if( hObject )
	{
		g_pLTClient->GetObjectTransform( hObject, &m_tTransform );
	}
}

// **************************************************************************** //

void ObjectDetector::SetTransform( const LTRigidTransform& tTrans )
{
	m_tTransform = tTrans;
}

// **************************************************************************** //

void ObjectDetector::SetTransform( const LTVector& vPos, const LTRotation& rRot )
{
	m_tTransform.Init( vPos, rRot );
}

// **************************************************************************** //

void ObjectDetector::SetParamsForward( float fDistance )
{
	m_fParamsForward = fDistance;
}

// **************************************************************************** //

void ObjectDetector::SetParamsDirection( const LTVector& vDirection )
{
	m_vParamsDirection = vDirection;
}

// **************************************************************************** //

void ObjectDetector::SetParamsFOV( float fXDegrees, float fYDegrees, float fNearDist, float fFarDist, float fDistWeight, float fXWeight, float fYWeight )
{
	m_vParamsFOV.Init( cosf( DEG2RAD( fXDegrees * 0.5f ) ), cosf( DEG2RAD( fYDegrees * 0.5f ) ), ( fNearDist * fNearDist ), ( fFarDist * fFarDist ) );

	float fWeightTotal = ( fDistWeight + fXWeight + fYWeight );
	m_vParamsFOVWeights.Init( ( fDistWeight / fWeightTotal ), ( fXWeight / fWeightTotal ), ( fYWeight / fWeightTotal ) );
}

// **************************************************************************** //

void ObjectDetector::SetParamsSphere( float fInnerRadius,  float fOuterRadius )
{
	m_vParamsSphere.Init( ( fInnerRadius * fInnerRadius ), ( fOuterRadius * fOuterRadius ) );
}

// **************************************************************************** //

void ObjectDetector::SetParamsCylinder( float fInnerRadius,  float fOuterRadius, float fHalfHeight )
{
	m_vParamsCylinder.Init( ( fInnerRadius * fInnerRadius ), ( fOuterRadius * fOuterRadius ), fHalfHeight );
}

// **************************************************************************** //

void ObjectDetector::SetBehaviorFlags( uint32 nFlags )
{
	m_nBehaviorFlags = nFlags;
}

// **************************************************************************** //

void ObjectDetector::SetVerifyFailureDelay( uint32 nVerifyFlags, float fSeconds )
{
	if( fSeconds < 0.0f ) fSeconds = 0.0f;

	if( nVerifyFlags & ODBF_VERIFYLINEOFSITE )		m_fVerifyFailureDelays[ 0 ] = fSeconds;
	if( nVerifyFlags & ODBF_VERIFYFORWARD )			m_fVerifyFailureDelays[ 1 ] = fSeconds;
	if( nVerifyFlags & ODBF_VERIFYDIRECTION )		m_fVerifyFailureDelays[ 2 ] = fSeconds;
	if( nVerifyFlags & ODBF_VERIFYFOV )				m_fVerifyFailureDelays[ 3 ] = fSeconds;
	if( nVerifyFlags & ODBF_VERIFYSPHERE )			m_fVerifyFailureDelays[ 4 ] = fSeconds;
	if( nVerifyFlags & ODBF_VERIFYCYLINDER )		m_fVerifyFailureDelays[ 5 ] = fSeconds;
	if( nVerifyFlags & ODBF_VERIFYCUSTOM )			m_fVerifyFailureDelays[ 6 ] = fSeconds;
}

// **************************************************************************** //

void ObjectDetector::SetUserFlagVerification( uint32 nFlag )
{
	// If we already have an object being tracked, make sure it gets updated
	if( m_pTrackedLink )
	{
		// Turn off the old one...
		if( m_nUserFlagVerification )
		{
			g_pLTClient->Common()->SetObjectFlags( m_pTrackedLink->m_hObject, OFT_User, 0, m_nUserFlagVerification );
		}

		// Turn on the new one...
		if( nFlag )
		{
			g_pLTClient->Common()->SetObjectFlags( m_pTrackedLink->m_hObject, OFT_User, nFlag, nFlag );
		}
	}

	// Set the current flag to update
	m_nUserFlagVerification = nFlag;
}

// **************************************************************************** //

void ObjectDetector::SetLineOfSiteFn( ObjectDetectorLOSFn pFn, void* pUserData )
{
	m_pLineOfSiteFn = pFn;
	m_pLineOfSiteUserData = pUserData;
}

// **************************************************************************** //

void ObjectDetector::SetSpatialFn( ObjectDetectorSpatialFn pFn, void* pUserData )
{
	m_pSpatialFn = pFn;
	m_pSpatialUserData = pUserData;
}

// **************************************************************************** //

void ObjectDetector::SetCustomTestFn( ObjectDetectorCustomTestFn pFn, void* pUserData )
{
	m_pCustomTestFn = pFn;
	m_pCustomTestUserData = pUserData;
}

// **************************************************************************** //

void ObjectDetector::Update( float fFrameTime )
{
	// If we don't have a tracked link... there's nothing to update!
	if( !m_pTrackedLink )
	{
		return;
	}

	// Make sure the link is still valid
	if( !m_pTrackedLink->m_hObject.GetData() )
	{
		ReleaseLink( *m_pTrackedLink );
		ClearObject();
		return;
	}

	// If we don't have any verify flags set... then don't do anything
	if( !( m_nBehaviorFlags & ODBF_VERIFYMASK ) )
	{
		return;
	}

	// Requirement ranges and other data tracking variables
	float fTempRR;
	uint32 nAttemptedTests = 0;
	uint32 nPassedTests = 0;

	UpdateObjectSpatialData();

	// Handle each verification type
	if( m_nBehaviorFlags & ODBF_VERIFYLINEOFSITE )
	{
		++nAttemptedTests;

		if( TestLineOfSite( m_pTrackedLink ) )
		{
			m_fActiveVerifyFailureDelays[ 0 ] = m_fVerifyFailureDelays[ 0 ];
			++nPassedTests;
		}
		else
		{
			if( m_fActiveVerifyFailureDelays[ 0 ] > 0.0f )
			{
				m_fActiveVerifyFailureDelays[ 0 ] -= fFrameTime;
				++nPassedTests;
			}
		}
	}

	if( m_nBehaviorFlags & ODBF_VERIFYFORWARD )
	{
		++nAttemptedTests;

		if( TestParamsForward( m_pTrackedLink, m_vTrackedSpatialPosition, m_vTrackedSpatialDimensions, fTempRR ) )
		{
			m_fActiveVerifyFailureDelays[ 1 ] = m_fVerifyFailureDelays[ 1 ];
			++nPassedTests;
		}
		else
		{
			if( m_fActiveVerifyFailureDelays[ 1 ] > 0.0f )
			{
				m_fActiveVerifyFailureDelays[ 1 ] -= fFrameTime;
				++nPassedTests;
			}
		}
	}

	if( m_nBehaviorFlags & ODBF_VERIFYDIRECTION )
	{
		++nAttemptedTests;

		if( TestParamsDirection( m_pTrackedLink, m_vTrackedSpatialPosition, m_vTrackedSpatialDimensions, fTempRR ) )
		{
			m_fActiveVerifyFailureDelays[ 2 ] = m_fVerifyFailureDelays[ 2 ];
			++nPassedTests;
		}
		else
		{
			if( m_fActiveVerifyFailureDelays[ 2 ] > 0.0f )
			{
				m_fActiveVerifyFailureDelays[ 2 ] -= fFrameTime;
				++nPassedTests;
			}
		}
	}

	if( m_nBehaviorFlags & ODBF_VERIFYFOV )
	{
		++nAttemptedTests;

		if( TestParamsFOV( m_pTrackedLink, m_vTrackedSpatialPosition, m_vTrackedSpatialDimensions, fTempRR ) )
		{
			m_fActiveVerifyFailureDelays[ 3 ] = m_fVerifyFailureDelays[ 3 ];
			++nPassedTests;
		}
		else
		{
			if( m_fActiveVerifyFailureDelays[ 3 ] > 0.0f )
			{
				m_fActiveVerifyFailureDelays[ 3 ] -= fFrameTime;
				++nPassedTests;
			}
		}
	}

	if( m_nBehaviorFlags & ODBF_VERIFYSPHERE )
	{
		++nAttemptedTests;

		if( TestParamsSphere( m_pTrackedLink, m_vTrackedSpatialPosition, m_vTrackedSpatialDimensions, fTempRR ) )
		{
			m_fActiveVerifyFailureDelays[ 4 ] = m_fVerifyFailureDelays[ 4 ];
			++nPassedTests;
		}
		else
		{
			if( m_fActiveVerifyFailureDelays[ 4 ] > 0.0f )
			{
				m_fActiveVerifyFailureDelays[ 4 ] -= fFrameTime;
				++nPassedTests;
			}
		}
	}

	if( m_nBehaviorFlags & ODBF_VERIFYCYLINDER )
	{
		++nAttemptedTests;

		if( TestParamsCylinder( m_pTrackedLink, m_vTrackedSpatialPosition, m_vTrackedSpatialDimensions, fTempRR ) )
		{
			m_fActiveVerifyFailureDelays[ 5 ] = m_fVerifyFailureDelays[ 5 ];
			++nPassedTests;
		}
		else
		{
			if( m_fActiveVerifyFailureDelays[ 5 ] > 0.0f )
			{
				m_fActiveVerifyFailureDelays[ 5 ] -= fFrameTime;
				++nPassedTests;
			}
		}
	}

	if( m_nBehaviorFlags & ODBF_VERIFYCUSTOM )
	{
		++nAttemptedTests;

		if( TestParamsCustom( m_pTrackedLink, fTempRR ) )
		{
			m_fActiveVerifyFailureDelays[ 6 ] = m_fVerifyFailureDelays[ 6 ];
			++nPassedTests;
		}
		else
		{
			if( m_fActiveVerifyFailureDelays[ 6 ] > 0.0f )
			{
				m_fActiveVerifyFailureDelays[ 6 ] -= fFrameTime;
				++nPassedTests;
			}
		}
	}

	// Make sure we passed the required tests...
	if( ( nPassedTests == 0 ) || ( ( m_nBehaviorFlags & ODBF_INCLUSIVEVERIFY ) && ( nPassedTests != nAttemptedTests ) ) )
	{
		ClearObject();
	}

	// Otherwise, see if we're in a delay for clearing the object... and apply that instead
	if( m_fActiveClearDelay != OD_INVALID_CLEAR_DELAY )
	{
		m_fActiveClearDelay -= fFrameTime;

		if( m_fActiveClearDelay <= 0.0f )
		{
			ClearObject();
		}
	}
}

// **************************************************************************** //

void ObjectDetector::UpdateObjectSpatialData()
{
	if( m_pTrackedLink )
	{
		GetObjectSpatialData( m_pTrackedLink, m_vTrackedSpatialPosition, m_vTrackedSpatialDimensions );
	}
}

// **************************************************************************** //

HOBJECT ObjectDetector::AcquireObject( bool bFromPrevious )
{
	// Make sure the object to the currently tracked link is still valid
	if( m_pTrackedLink && !m_pTrackedLink->m_hObject.GetData() )
	{
		ReleaseLink( *m_pTrackedLink );
		ClearObject();
	}

	// If we don't have any acquire flags set... then don't do anything
	if( !( m_nBehaviorFlags & ODBF_ACQUIREMASK ) )
	{
		return NULL;
	}

	// Keep track of the links that fit the previous, best, and current requirements
	ObjectDetectorLink* pPrevTracked = ( bFromPrevious ? m_pTrackedLink : NULL );
	ObjectDetectorLink* pCurrTracked = NULL;
	ObjectDetectorLink* pBestTracked = NULL;

	// Requirement ranges and other data tracking variables
	float fPrevRR = 0.0f;
	float fCurrRR = 1000000.0f;
	float fBestRR = 1000000.0f;
	float fTempRR, fActiveRR;
	uint32 nAttemptedTests, nPassedTests;
	bool bContinue;

	// Get the previously tracked link data... add up the RR values even if the
	// tests don't pass.  This way we get a relative value to compare to regardless
	// of whether it can be acquired again during this attempt.
	if( pPrevTracked )
	{
		LTVector vPos, vDims;
		GetObjectSpatialData( pPrevTracked, vPos, vDims );

		if( m_nBehaviorFlags & ODBF_ACQUIREFORWARD )
		{
			TestParamsForward( pPrevTracked, vPos, vDims, fTempRR );
			fPrevRR += fTempRR;
		}

		if( m_nBehaviorFlags & ODBF_ACQUIREDIRECTION )
		{
			TestParamsDirection( pPrevTracked, vPos, vDims, fTempRR );
			fPrevRR += fTempRR;
		}

		if( m_nBehaviorFlags & ODBF_ACQUIREFOV )
		{
			TestParamsFOV( pPrevTracked, vPos, vDims, fTempRR );
			fPrevRR += fTempRR;
		}

		if( m_nBehaviorFlags & ODBF_ACQUIRESPHERE )
		{
			TestParamsSphere( pPrevTracked, vPos, vDims, fTempRR );
			fPrevRR += fTempRR;
		}

		if( m_nBehaviorFlags & ODBF_ACQUIRECYLINDER )
		{
			TestParamsCylinder( pPrevTracked, vPos, vDims, fTempRR );
			fPrevRR += fTempRR;
		}

		if( m_nBehaviorFlags & ODBF_ACQUIRECUSTOM )
		{
			TestParamsCustom( pPrevTracked, fTempRR );
			fPrevRR += fTempRR;
		}
	}

	// Go through each registered object
	ObjectDetectorLink* pLink = m_iRootLink.m_pNext;

	while( pLink )
	{
		LTVector vPos, vDims;
		GetObjectSpatialData( pLink, vPos, vDims );

		// Ignore the previous tracked link
		if( pLink == pPrevTracked )
		{
			pLink = pLink->m_pNext;
			continue;
		}

		// If this link has invalid object data... release it
		if( !pLink->m_hObject.GetData() )
		{
			ObjectDetectorLink* pRemove = pLink;
			pLink = pLink->m_pNext;
			ReleaseLink( *pRemove );
			continue;
		}

		// Check our user flag verification
		if( m_nUserFlagVerification )
		{
			uint32 nUserFlags;
			g_pLTClient->Common()->GetObjectFlags( pLink->m_hObject, OFT_User, nUserFlags );

			if( ( nUserFlags & m_nUserFlagVerification ) != m_nUserFlagVerification )
			{
				continue;
			}
		}

		// Zero out our temporary requirement range
		fActiveRR = 0.0f;
		nAttemptedTests = 0;
		nPassedTests = 0;

		// Check all the necessary params
		if( m_nBehaviorFlags & ODBF_ACQUIREFORWARD )
		{
			++nAttemptedTests;

			if( TestParamsForward( pLink, vPos, vDims, fTempRR ) )
			{
				fActiveRR += fTempRR;
				++nPassedTests;
			}
		}

		if( m_nBehaviorFlags & ODBF_ACQUIREDIRECTION )
		{
			++nAttemptedTests;

			if( TestParamsDirection( pLink, vPos, vDims, fTempRR ) )
			{
				fActiveRR += fTempRR;
				++nPassedTests;
			}
		}

		if( m_nBehaviorFlags & ODBF_ACQUIREFOV )
		{
			++nAttemptedTests;

			if( TestParamsFOV( pLink, vPos, vDims, fTempRR ) )
			{
				fActiveRR += fTempRR;
				++nPassedTests;
			}
		}

		if( m_nBehaviorFlags & ODBF_ACQUIRESPHERE )
		{
			++nAttemptedTests;

			if( TestParamsSphere( pLink, vPos, vDims, fTempRR ) )
			{
				fActiveRR += fTempRR;
				++nPassedTests;
			}
		}

		if( m_nBehaviorFlags & ODBF_ACQUIRECYLINDER )
		{
			++nAttemptedTests;

			if( TestParamsCylinder( pLink, vPos, vDims, fTempRR ) )
			{
				fActiveRR += fTempRR;
				++nPassedTests;
			}
		}

		if( m_nBehaviorFlags & ODBF_ACQUIRECUSTOM )
		{
			++nAttemptedTests;

			if( TestParamsCustom( pLink, fTempRR ) )
			{
				fActiveRR += fTempRR;
				++nPassedTests;
			}
		}

		// Make sure we passed the required tests...
		if( m_nBehaviorFlags & ODBF_INCLUSIVEACQUIRE )
		{
			bContinue = ( nPassedTests == nAttemptedTests );
		}
		else
		{
			bContinue = ( nPassedTests > 0 );
		}

		if( bContinue )
		{
			// Make sure we have a line of site to this object
			if( m_nBehaviorFlags & ODBF_ACQUIRELINEOFSITE )
			{
				bContinue = TestLineOfSite( pLink );
			}

			// If we passed the line of site test...
			if( bContinue )
			{
				// If the active test is better than our best... track it!
				if( fActiveRR < fBestRR )
				{
					fBestRR = fActiveRR;
					pBestTracked = pLink;
				}

				// If the active test is after our previous, but better than the current... track it too!
				if( ( fActiveRR > fPrevRR ) && ( fActiveRR < fCurrRR ) )
				{
					fCurrRR = fActiveRR;
					pCurrTracked = pLink;
				}
			}
		}

		// Move on to the next object
		pLink = pLink->m_pNext;
	}

	// Reset our verification timers
	memcpy( m_fActiveVerifyFailureDelays, m_fVerifyFailureDelays, sizeof( float ) * ODBF_TESTSAVAILABLE );

	// Set our tracked link to the proper one
	if( pCurrTracked )
	{
		SetLink( pCurrTracked );
	}
	else if( pBestTracked )
	{
		SetLink( pBestTracked );
	}
	else if( !pPrevTracked )
	{
		ClearObject();
	}

	return GetObject();
}

// **************************************************************************** //

HOBJECT ObjectDetector::GetObject()
{
	if( !m_pTrackedLink )
	{
		return NULL;
	}

	return m_pTrackedLink->m_hObject;
}

// **************************************************************************** //

void* ObjectDetector::GetObjectUserData()
{
	if( !m_pTrackedLink )
	{
		return NULL;
	}

	return m_pTrackedLink->m_pUserData;
}

// **************************************************************************** //

LTVector ObjectDetector::GetObjectSpatialPosition()
{
	return m_vTrackedSpatialPosition;
}

// **************************************************************************** //

LTVector ObjectDetector::GetObjectSpatialDimensions()
{
	return m_vTrackedSpatialDimensions;
}

// **************************************************************************** //

bool ObjectDetector::SetObject( HOBJECT hObject )
{
	ObjectDetectorLink* pLink = m_iRootLink.m_pNext;
	ClearObject();

	// Find this object in the registered list
	while( pLink )
	{
		if( pLink->m_hObject == hObject )
		{
			SetLink( pLink );
			break;
		}

		pLink = pLink->m_pNext;
	}

	return ( m_pTrackedLink != NULL );
}

// **************************************************************************** //

bool ObjectDetector::SetLink( ObjectDetectorLink* pLink )
{
	// Make sure this link is registered with this detector
	if( pLink->m_pDetector != this )
	{
		return false;
	}

	// Make sure we don't already have an object
	ClearObject();

	// Keep track of this one!
	m_pTrackedLink = pLink;

	// Get the spatial data for this link
	GetObjectSpatialData( m_pTrackedLink, m_vTrackedSpatialPosition, m_vTrackedSpatialDimensions );

	// Turn on the user flag if we have one we're managing
	if( m_pTrackedLink && m_nUserFlagVerification )
	{
		g_pLTClient->Common()->SetObjectFlags( m_pTrackedLink->m_hObject, OFT_User, m_nUserFlagVerification, m_nUserFlagVerification );
	}

	return true;
}

// **************************************************************************** //

void ObjectDetector::ClearObject( float fDelay )
{
	if( ( fDelay > 0.0f ) && ( m_pTrackedLink != NULL ) )
	{
		m_fActiveClearDelay = fDelay;
	}
	else
	{
		// Turn off the user flag if we have one we're managing
		if( m_pTrackedLink && m_nUserFlagVerification )
		{
			g_pLTClient->Common()->SetObjectFlags( m_pTrackedLink->m_hObject, OFT_User, 0, m_nUserFlagVerification );
		}

		m_fActiveClearDelay = OD_INVALID_CLEAR_DELAY;
		m_pTrackedLink = NULL;
	}
}

// **************************************************************************** //

bool ObjectDetector::ClearingObject()
{
	return ( m_fActiveClearDelay != OD_INVALID_CLEAR_DELAY );
}

// **************************************************************************** //

bool ObjectDetector::TestLineOfSite( ObjectDetectorLink* pLink )
{
	// If we have an override line of site test... use that instead
	if( m_pLineOfSiteFn )
	{
		return ( *m_pLineOfSiteFn )( m_tTransform.m_vPos, pLink, m_pLineOfSiteUserData );
	}

	// Do a basic line of site test... allowing only world geometry to block the view
	IntersectQuery iQuery;
	IntersectInfo iInfo;

	iQuery.m_Flags		= IGNORE_NONSOLID;
	iQuery.m_FilterFn	= NULL;
	iQuery.m_pUserData	= NULL;
	iQuery.m_From		= m_tTransform.m_vPos;
	g_pLTClient->GetObjectPos( pLink->m_hObject, &iQuery.m_To );

	return !g_pLTClient->IntersectSegment( iQuery, &iInfo );
}

// **************************************************************************** //

void ObjectDetector::GetObjectSpatialData( ObjectDetectorLink* pLink, LTVector& vPos, LTVector& vDims )
{
	// Use the override if we have one...
	if( m_pSpatialFn )
	{
		( *m_pSpatialFn )( pLink, vPos, vDims, m_pSpatialUserData );
		return;
	}

	// Otherwise, just get the object center position
	g_pLTClient->GetObjectPos( pLink->m_hObject, &vPos );
	g_pLTClient->Physics()->GetObjectDims( pLink->m_hObject, &vDims );
}

// **************************************************************************** //

bool ObjectDetector::TestParamsForward( ObjectDetectorLink* pLink, LTVector& vPos, LTVector& vDims, float& fMetRR )
{
	// Make sure we have a valid forward parameter set...
	if( m_fParamsForward == 0.0f )
	{
		fMetRR = 1.0f;
		return false;
	}

	// Create a line segment that we want to test against this object
	LTVector vPt1 = m_tTransform.m_vPos;
	LTVector vPt2 = ( m_tTransform.m_vPos + ( m_tTransform.m_rRot.Forward() * m_fParamsForward ) );

	LTRect3f rAABB;

	// Now get at the AABB of the object
	rAABB.m_vMin = ( vPos - vDims );
	rAABB.m_vMax = ( vPos + vDims );

	if( !LTIntersect::AABB_Segment( rAABB, vPt1, vPt2, fMetRR ) )
	{
		// Failure! But get a requirement range anyway
		LTVector vClosestPt;
		LTIntersect::Point_Line_DistSqr( vPos, vPt1, vPt2, vClosestPt, fMetRR );

		return false;
	}

	// An intersect time of zero means that the collision occured from within
	// the AABB... which we want to ignore.
	if( fMetRR <= 0.0f )
	{
		return false;
	}

	return true;
}

// **************************************************************************** //

bool ObjectDetector::TestParamsDirection( ObjectDetectorLink* pLink, LTVector& vPos, LTVector& vDims, float& fMetRR )
{
	// Make sure we have a valid vector parameter set...
	if( ( m_vParamsDirection.x == 0.0f ) && ( m_vParamsDirection.y == 0.0f ) && ( m_vParamsDirection.z == 0.0f ) )
	{
		fMetRR = 1.0f;
		return false;
	}

	// Create a line segment that we want to test against this object
	LTVector vPt1 = m_tTransform.m_vPos;
	LTVector vPt2 = ( m_tTransform.m_vPos + m_vParamsDirection );

	LTRect3f rAABB;

	// Now get at the AABB of the object
	rAABB.m_vMin = ( vPos - vDims );
	rAABB.m_vMax = ( vPos + vDims );

	if( !LTIntersect::AABB_Segment( rAABB, vPt1, vPt2, fMetRR ) )
	{
		// Failure! But get a requirement range anyway
		LTVector vClosestPt;
		LTIntersect::Point_Line_DistSqr( vPos, vPt1, vPt2, vClosestPt, fMetRR );

		return false;
	}

	// An intersect time of zero means that the collision occured from within
	// the AABB... which we want to ignore.
	if( fMetRR <= 0.0f )
	{
		return false;
	}

	return true;
}

// **************************************************************************** //

bool ObjectDetector::TestParamsFOV( ObjectDetectorLink* pLink, LTVector& vPos, LTVector& vDims, float& fMetRR )
{
	// Make sure we've got some valid parameters
	if( ( m_vParamsFOV.z < 0.0f ) || ( m_vParamsFOV.w <= m_vParamsFOV.z ) )
	{
		fMetRR = 1.0f;
		return false;
	}

	// Get the object position in the space of the source
	LTVector vRelPos;

	vRelPos = ( vPos - m_tTransform.m_vPos );
	vRelPos = ( ~m_tTransform.m_rRot ).RotateVector( vRelPos );

	// Get the normalized axis vectors flat along the FOV
	LTVector vForward( 0.0f, 0.0f, 1.0f );
	LTVector vFlatX( vRelPos.x, 0.0f, vRelPos.z );
	LTVector vFlatY( 0.0f, vRelPos.y, vRelPos.z );

	vFlatX.Normalize();
	vFlatY.Normalize();

	// Check the range
	float fMagSqr = vRelPos.MagSqr();
	float fXCos = vForward.Dot( vFlatX );
	float fYCos = vForward.Dot( vFlatY );

	// Calculate the requirement range values
	float fMagRR = ( ( fMagSqr - m_vParamsFOV.z ) / ( m_vParamsFOV.w - m_vParamsFOV.z ) );
	float fFOVXRR = ( ( fXCos - 1.0f ) / ( m_vParamsFOV.x - 1.0f ) );
	float fFOVYRR = ( ( fYCos - 1.0f ) / ( m_vParamsFOV.y - 1.0f ) );

	// Apply the weights to the various parameters
	fMetRR = ( fMagRR * m_vParamsFOVWeights.x ) + ( fFOVXRR * m_vParamsFOVWeights.y ) + ( fFOVYRR * m_vParamsFOVWeights.z );

	// Return true if it was contained by the FOV
	return ( ( fMagSqr >= m_vParamsFOV.z ) && ( fMagSqr < m_vParamsFOV.w ) && ( fXCos >= m_vParamsFOV.x ) && ( fYCos >= m_vParamsFOV.y ) );
}

// **************************************************************************** //

bool ObjectDetector::TestParamsSphere( ObjectDetectorLink* pLink, LTVector& vPos, LTVector& vDims, float& fMetRR )
{
	// Make sure we've got some valid parameters
	if( ( m_vParamsSphere.x <= 0.0f ) || ( m_vParamsSphere.y <= m_vParamsSphere.x ) )
	{
		fMetRR = 1.0f;
		return false;
	}

	// Get the object position in the space of the source
	LTVector vRelPos;

	vRelPos = ( vPos - m_tTransform.m_vPos );

	// Calculate the magnitude and requirement range
	float fMagSqr = vRelPos.MagSqr();
	fMetRR = ( ( fMagSqr - m_vParamsSphere.x ) / ( m_vParamsSphere.y - m_vParamsSphere.x ) );

	return ( ( fMagSqr >= m_vParamsSphere.x ) && ( fMagSqr <= m_vParamsSphere.y ) );
}

// **************************************************************************** //

bool ObjectDetector::TestParamsCylinder( ObjectDetectorLink* pLink, LTVector& vPos, LTVector& vDims, float& fMetRR )
{
	// Make sure we've got some valid parameters
	if( ( m_vParamsCylinder.x <= 0.0f ) || ( m_vParamsCylinder.y <= m_vParamsCylinder.x ) || ( m_vParamsCylinder.z <= 0.0f ) )
	{
		fMetRR = 1.0f;
		return false;
	}

	// Get the object position in the space of the source
	LTVector vRelPos;

	vRelPos = ( vPos - m_tTransform.m_vPos );
	vRelPos = ( ~m_tTransform.m_rRot ).RotateVector( vRelPos );

	// Calculate the requirement range values
	float fHeightDelta = fabs( vRelPos.y );
	vRelPos.y = 0.0f;

	float fMagSqr = vRelPos.MagSqr();
	float fMagRR = ( ( fMagSqr - m_vParamsCylinder.x ) / ( m_vParamsCylinder.y - m_vParamsCylinder.x ) );
	float fHeightRR = ( fHeightDelta / m_vParamsCylinder.z );

	// Weigh the magnitude higher than the angles
	fMetRR = ( fMagRR * 0.5f ) + ( fHeightRR * 0.5f );

	// Return true if it was contained by the cylinder
	return ( ( fMagSqr >= m_vParamsCylinder.x ) && ( fMagSqr <= m_vParamsCylinder.y ) && ( fHeightDelta <= m_vParamsCylinder.z ) );
}

// **************************************************************************** //

bool ObjectDetector::TestParamsCustom( ObjectDetectorLink* pLink, float& fMetRR )
{
	// Make sure we've got some valid parameters
	if( !m_pCustomTestFn )
	{
		fMetRR = 1.0f;
		return false;
	}

	bool bPassed = ( *m_pCustomTestFn )( pLink, fMetRR, m_pCustomTestUserData );

	// Make sure it's clamped
	fMetRR = LTCLAMP( fMetRR, 0.0f, 1.0f );

	return bPassed;
}
