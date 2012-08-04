
#ifndef  _WeaponFireInfo_h_INCLUDED_
#define  _WeaponFireInfo_h_INCLUDED_


#include "MsgIDs.h"


struct WeaponFireInfo
{
	WeaponFireInfo()
		: hFiredFrom( LTNULL )
		, hTestObj( LTNULL )
		, vPath()
		, vFirePos()
		, vFlashPos()
		, nSeed( 0 )
		, bAltFire( LTFALSE )
		, fPerturbU( 1.0f )	// 0.0 - 1.0
		, fPerturbR( 1.0f )	// 0.0 - 1.0
		, bOverrideVelocity( LTFALSE )
		, fOverrideVelocity( 0.0f )
		, nFireTimestamp( 0 )
		, hClientObjImpact( INVALID_HOBJECT )
		, nDiscTrackingType( 0 )
		, vPointTarget()
		, vControlDirection()
		, vControlPosition()
		, hObjectTarget( 0 )
		, hSocket( INVALID_MODEL_SOCKET )
		, fDiscReleaseAngle( 0.0f )
		, fHorizontalSpread( 0.0f )
		, fHorizontalPerturb( 0.0f )
		, fVerticalSpread( 0.0f )
		, nNumberOfShards( 0 )
	{
	}

	HOBJECT     hFiredFrom;
	HOBJECT     hTestObj;
	LTVector    vPath;
	LTVector    vFirePos;
	LTVector    vFlashPos;
	uint8       nSeed;
	LTBOOL      bAltFire;
	LTFLOAT     fPerturbU;
	LTFLOAT     fPerturbR;
	LTBOOL      bOverrideVelocity;
	LTFLOAT     fOverrideVelocity;
	uint32      nFireTimestamp;

	LTVector	vClientObjImpactPos;
	HOBJECT		hClientObjImpact;

	//
	// Disc Information
	//

	// type of disc tracking
	uint8       nDiscTrackingType;

	// point needed if the disc is homing in on a point
	LTVector    vPointTarget;

	// control vector to help with controlling the disc
	LTVector    vControlDirection;

	// control location to help with controlling the disc
	LTVector    vControlPosition;

	// an object needed if the disc is homing in on an object
	HOBJECT     hObjectTarget;

	// Optional socket for the disc to return to.
	HMODELSOCKET hSocket;

	// When the disc is released, instead of sending
	// it straight forward, throw it off center at this
	// angle (specified in RADIANS).  Positive values
	// mean "right", negative values mean "left".
	// NOTE: Only used by the player.
	LTFLOAT     fDiscReleaseAngle;

	//
	// Cluster information
	//

	// the shard emission horizontal spread
	float fHorizontalSpread;

	// the shard emission horizontal perturb
	float fHorizontalPerturb;

	// the shard emission vertical spread
	float fVerticalSpread;

	// the number of shards
	uint8 nNumberOfShards;
};



#endif //_WeaponFireInfo_h_INCLUDED_
