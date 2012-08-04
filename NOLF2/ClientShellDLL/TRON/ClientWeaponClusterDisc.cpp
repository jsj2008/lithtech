// ----------------------------------------------------------------------- //
//`
// MODULE  : ClientWeaponClusterDisc.cpp
//
// PURPOSE : Tron specific client-side cluster version of the disc
//
// CREATED : 4/12/02
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponClusterDisc.h"
#include "FXButeMgr.h"
#include "RatingMgr.h"

CClientWeaponClusterDisc::CClientWeaponClusterDisc()
	: CClientWeaponDisc()
{
}



CClientWeaponClusterDisc::~CClientWeaponClusterDisc()
{
}



void CClientWeaponClusterDisc::AddExtraFireMessageInfo( bool bFire, ILTMessage_Write *pMsg )
{
	// first add the standard disc extra info
	CClientWeaponDisc::AddExtraFireMessageInfo( bFire, pMsg );

	// get the cluster specific data
	CLUSTERDISCCLASSDATA const *pClusterData = dynamic_cast< CLUSTERDISCCLASSDATA const * >( m_pAmmo->pProjectileFX->pClassData );

	// write the horizontal spread
	float fHorizontalSpread; 
	fHorizontalSpread = InterpolateRating< float >(
			Rating_Accuracy,
			pClusterData->fShardHorizontalSpreadMin,
			pClusterData->fShardHorizontalSpreadMax,
			pClusterData->fShardHorizontalSpreadSurge
		);
	pMsg->Writeuint16( CompressAngleToShort( fHorizontalSpread ) );

	// write the horizontal perturb
	float fHorizontalPerturb;
	fHorizontalPerturb = InterpolateRating< float >(
			Rating_Accuracy,
			pClusterData->fShardHorizontalPerturbMin,
			pClusterData->fShardHorizontalPerturbMax,
			pClusterData->fShardHorizontalPerturbSurge
		);
	pMsg->Writeuint8( CompressAngleToByte( fHorizontalPerturb ) );

	// write the vertical spread
	float fVerticalSpread;
	fVerticalSpread = InterpolateRating< float >(
			Rating_Accuracy,
			pClusterData->fShardVerticalSpreadMin,
			pClusterData->fShardVerticalSpreadMax,
			pClusterData->fShardVerticalSpreadSurge
		);
	pMsg->Writeuint8( CompressAngleToByte( fVerticalSpread ) );

	//
	// number of shards
	//

	// get the base number of shards
	int nNumberOfShards;
	nNumberOfShards = InterpolateRating< int >(
			Rating_Accuracy,
			pClusterData->nShardsTotalMin,
			pClusterData->nShardsTotalMax,
			pClusterData->nShardsTotalSurge
		);

	// get the perturb
	int nNumberOfShardsPreturb;
	nNumberOfShardsPreturb = InterpolateRating< int >(
			Rating_Accuracy,
			pClusterData->nShardsTotalPerturbMin,
			pClusterData->nShardsTotalPerturbMax,
			pClusterData->nShardsTotalPerturbSurge
		);

	// get the total number of shards
	int nTotalNumberOfShards = GetRandom( 
			( nNumberOfShards - nNumberOfShardsPreturb ),
			( nNumberOfShards + nNumberOfShardsPreturb )
		);
	ASSERT( ( 0 <= nTotalNumberOfShards ) && ( UCHAR_MAX > nTotalNumberOfShards ) );

	// write the total number of shards to the message
	pMsg->Writeuint8( static_cast< uint8 >( nTotalNumberOfShards ) );
}


