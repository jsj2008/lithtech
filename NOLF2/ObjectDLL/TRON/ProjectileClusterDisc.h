// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileClusterDisc.h
//
// PURPOSE : TRON Disc cluster class - definition
//
// CREATED : 04/15/01
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include <vector>


#include "ProjectileDisc.h"


class CClusterDisc : public CDisc
{
public:
	// destructor
	~CClusterDisc();

	// constructor
	CClusterDisc();

protected:
	// setup the disc (called externally)
	virtual LTBOOL      Setup( CWeapon const*, WeaponFireInfo const& );

	// handles any time a projectile touches something
	virtual void        HandleTouch( HOBJECT hObj );

	// code to handle state changing information on ReturnMode
	virtual void        EnterReturnMode( bool bImpact );

	// helper function to create a vector given yaw and pitch
	void                DetermineShardForwardVector( LTVector *pvForward,
	                                                 float fYaw,
	                                                 float fPitch,
	                                                 float fMagnitude = 1.0f );

	// launch a shard
	void                LaunchShard( LTVector const &vForward );

	struct ClusterDiscAngle
	{
		float fYaw;
		float fPitch;
	};

	typedef std::vector< ClusterDiscAngle > ClusterAngleList;

	ClusterAngleList m_lShardReleaseAngle;

	// number of shards to emit
	int                 nShardsTotal;

};
