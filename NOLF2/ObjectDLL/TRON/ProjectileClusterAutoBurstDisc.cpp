//----------------------------------------------------------------------------
//              
//	MODULE:		ProjectileClusterAutoBurstDisc.cpp
//              
//	PURPOSE:	Auto bursting Disc implementation
//              
//	CREATED:	25.04.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	
//
//	TODO:	Remove the IsAI check and replace with a system to allow the AI
//			to handle weapons in a cleaner way.
//
//			Add a min distance to the bute file so that the disc won't burst
//			immediately.
//              
//              
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#include "ProjectileClusterAutoBurstDisc.h"		
#include "FXButeMgr.h"
#include "WeaponFireInfo.h"
#include "AIUtils.h"

// Forward declarations

// Globals

// Statics

//
// LithTech Macros
//
BEGIN_CLASS(CClusterAutoBurstDisc)
END_CLASS_DEFAULT_FLAGS(CClusterAutoBurstDisc, CClusterDisc, NULL, NULL, CF_HIDDEN)


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CClusterAutoBurstDisc::CClusterAutoBurstDisc()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
CClusterAutoBurstDisc::CClusterAutoBurstDisc() : 
						m_flAutoBurstDistance( 0 ),
						m_flMinTraveledDistance( 0 )
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CClusterAutoBurstDisc::~CClusterAutoBurstDisc()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ CClusterAutoBurstDisc::~CClusterAutoBurstDisc()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CClusterAutoBurstDisc::Setup()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ LTBOOL CClusterAutoBurstDisc::Setup(CWeapon const* pWeaponInfo, WeaponFireInfo const& FireInfo )
{
	UBER_ASSERT( 0 != pWeaponInfo, "No AmmoData specified" );

	WeaponFireInfo HackedInfo = FireInfo;

	// THIS IS A HACK!  This hack allows AI to use this weapon for now
	// Later on, we will have a weapon handling class for the AI, for 
	// specifying AI specific information without the AI needing to know
	// about it.
	if ( IsAI(HackedInfo.hFiredFrom) )
	{
		HackedInfo.fHorizontalSpread	= DegreesToRadians(20.0f);
		HackedInfo.fHorizontalPerturb	= DegreesToRadians(20.0f);
		HackedInfo.fVerticalSpread		= DegreesToRadians(10.0f);
		HackedInfo.nNumberOfShards		= 10;
	}

	LTBOOL bResult = CClusterDisc::Setup(pWeaponInfo, HackedInfo);
	UBER_ASSERT( 0 != m_pAmmoData, "No AmmoData specified" );

	PROJECTILEFX const *pClusterProjectileFX = m_pAmmoData->pProjectileFX;
	UBER_ASSERT( 0 != pClusterProjectileFX, "No pProjectileFX specified" );

	CLUSTERAUTOBURSTDISCCLASSDATA* pClusterAutoBurstData =
		dynamic_cast< CLUSTERAUTOBURSTDISCCLASSDATA* >( pClusterProjectileFX->pClassData );
	UBER_ASSERT( pClusterAutoBurstData, "Failed to convert classdata to AutoBurstData" );

	if ( pClusterAutoBurstData )
	{
		m_flAutoBurstDistance = GetRandom(pClusterAutoBurstData->fMinBurstDistance, pClusterAutoBurstData->fMaxBurstDistance);
		m_flMinTraveledDistance = 0;
	}

	return ( bResult );
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CClusterAutoBurstDisc::UpdateDisc()
//              
//	PURPOSE:	Do the base classes update, and extend it with a check for 
//				distance to a target which may cause entering return mode.
//				Only do the check after traveling some ammount of distance,
//				and
//              
//----------------------------------------------------------------------------
/*virtual*/ bool CClusterAutoBurstDisc::UpdateDisc()
{
	// 
	if ( !IsDiscReturning() /* && DistanceTraveled() > m_flMinTraveledDistance */)
	{
		IntersectQuery	IQuery;
		IntersectInfo	IInfo;
		LTVector vVelocity;

		g_pLTServer->GetObjectPos( m_hObject, &IQuery.m_From );
		g_pPhysicsLT->GetVelocity( m_hObject, &vVelocity );

		IQuery.m_To = IQuery.m_From + vVelocity.Unit() * m_flAutoBurstDistance;
		IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;

		if( g_pLTServer->IntersectSegment( &IQuery, &IInfo ) )
		{
			if ( IInfo.m_hObject != m_hFiredFrom )
			{
				EnterReturnMode(false);
			}
		}
	}

	return ( CClusterDisc::UpdateDisc() );
}
