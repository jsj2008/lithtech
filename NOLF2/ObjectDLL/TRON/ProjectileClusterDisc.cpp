// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileClusterDisc.cpp
//
// PURPOSE : Projectile disc class - implementation
//
// CREATED : 4/15/2002
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "FXButeMgr.h"
#include "ProjectileDisc.h"
#include "ProjectileClusterDisc.h"
#include "WeaponFireInfo.h"
#include "Weapons.h"
#include "Weapon.h"

//
// LithTech Macros
//
BEGIN_CLASS(CClusterDisc)
END_CLASS_DEFAULT_FLAGS(CClusterDisc, CDisc, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClusterDisc::CClusterDisc()
//
//	PURPOSE:	basic constructor
//
// ----------------------------------------------------------------------- //

CClusterDisc::CClusterDisc()
	: CDisc()
	, m_lShardReleaseAngle()
{
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClusterDisc::~CClusterDisc()
//
//	PURPOSE:	boring destructor
//
// ----------------------------------------------------------------------- //

CClusterDisc::~CClusterDisc()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::Setup()
//
//	PURPOSE:	Specialize the projectile disc setup (called externally)...
//
// ----------------------------------------------------------------------- //

LTBOOL CClusterDisc::Setup( CWeapon const *pWeapon, WeaponFireInfo const &info )
{
	// setup the base class
	LTBOOL bResult = CDisc::Setup( pWeapon, info );

	// determine the angle (with no perturb) between shards in the spread
	float fYawBetweenShards = 2.0f * info.fHorizontalSpread / static_cast< float >( info.nNumberOfShards );
//	fYawBetweenShards = DegreesToRadians( 180.0f );
	//float fYawBetweenShards = DegreesToRadians( 180.0f );

	// start this angle at the "left" shard, we will
	// increment it as we work our way to the "right"
	float fOverallAngle = ( -info.fHorizontalSpread + fYawBetweenShards / 2.0f );

	// for every shard, create an angle for it
	for ( int i = 0; i < info.nNumberOfShards; ++i )
	{
		ClusterDiscAngle a;

		// determine yaw
		a.fYaw = fOverallAngle;
		fOverallAngle += fYawBetweenShards;

		// hardcode pitch for now
		a.fPitch = GetRandom( -info.fVerticalSpread, info.fVerticalSpread );
//		a.fPitch = DegreesToRadians( GetRandom( -info.fVerticalSpread, info.fVerticalSpread ) );

		m_lShardReleaseAngle.push_back( a );
	}

	// check the results
	ClusterAngleList::const_iterator citer;
	for( citer = m_lShardReleaseAngle.begin(), i = 0;
	     citer != m_lShardReleaseAngle.end();
	     ++citer, ++i )
	{
		g_pLTServer->DebugOut( "Shard %d:"
		                       " %6.4f %6.4f,"
		                       " %6.4f %6.4f\n",
		                       i,
		                       citer->fYaw, citer->fPitch,
		                       RadiansToDegrees( citer->fYaw ), RadiansToDegrees( citer->fPitch ) );
	}

	return bResult;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClusterDisc::HandleTouch()
//
//	PURPOSE:	Filter out touching with shards, then handle as normal
//
// ----------------------------------------------------------------------- //

void CClusterDisc::HandleTouch( HOBJECT hObj )
{
	//
	// Setup information about this object
	//

	// get this object's projectile data
	PROJECTILEFX const *pClusterProjectileFX = m_pAmmoData->pProjectileFX;
	ASSERT( 0 != pClusterProjectileFX );

	// get this object's projectile class data
	CLUSTERDISCCLASSDATA *pClusterData =
		dynamic_cast< CLUSTERDISCCLASSDATA* >( pClusterProjectileFX->pClassData );
	ASSERT( 0 != pClusterData );

	//
	// Find what types of shards this object produces
	//

	// get the shard's weapon type
	WEAPON const *pShardWeaponData = g_pWeaponMgr->GetWeapon( pClusterData->szShardWeaponName );
	ASSERT( 0 != pShardWeaponData );

	// get the shard's weapon's ammo type
	AMMO const *pShardAmmoData = g_pWeaponMgr->GetAmmo( pShardWeaponData->nDefaultAmmoId );
	ASSERT( 0 != pShardAmmoData );

	// get the shard's weapon's ammo's projectile type
	PROJECTILEFX const *pShardProjectileFX = pShardAmmoData->pProjectileFX;
	ASSERT( 0 != pShardProjectileFX );

	// get the shard's weapon's ammo's projectile class name
	char const *szShardClassName = pShardProjectileFX->szClass;
	ASSERT( 0 != szShardClassName );

	//
	// Setup information about the object we've hit
	//

	// get the other object's class
	HCLASS hOtherObjectClass = g_pLTServer->GetObjectClass( hObj );
	ASSERT( 0 != hOtherObjectClass );

	// get the other object's class name
	LTRESULT ltResult;
	char szOtherObjectClassName[ 4096 ];
	ltResult = g_pLTServer->GetClassName( hOtherObjectClass, szOtherObjectClassName, 4096 );
	ASSERT( LT_OK == ltResult );

	//
	// NOW, the previous amout of work of this bloated piece of crap
	// is all in place so we can test if this object is a shard type
	// of the original object, and if so, do NOT HandleTouch.
	//


	// find the name of the shard's projectile class
	if ( 0 != stricmp( szShardClassName, szOtherObjectClassName ) )
	{
		// the don't match, handle as normal
		CDisc::HandleTouch( hObj );
	}
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClusterDisc::EnterReturnMode()
//
//	PURPOSE:	Handle the cluter bursting and the main part returning
//
// ----------------------------------------------------------------------- //

void CClusterDisc::EnterReturnMode( bool bImpact )
{
	if ( !bImpact )
	{
		//
		// The disc is NOT entereing return mode due to an impact, so create
		// the shards.  This is here because the shards had a tendancy to
		// get created outside the world (not very good).
		//

		// get the projectile data
		PROJECTILEFX const *pProjectileFX = m_pAmmoData->pProjectileFX;
		ASSERT( 0 != pProjectileFX );

		// get the projectile class data
		CLUSTERDISCCLASSDATA *pClusterData =
			dynamic_cast< CLUSTERDISCCLASSDATA* >( pProjectileFX->pClassData );
		ASSERT( 0 != pClusterData );

		// get and normalize the forward velocity vector
		LTVector vNormalizedVelocity;
		g_pPhysicsLT->GetVelocity( m_hObject, &vNormalizedVelocity );
		vNormalizedVelocity.Normalize();

		// get a matrix to transform from disc space to world space
		LTRotation rotDiscRot;
		g_pLTServer->GetObjectRotation( m_hObject, &rotDiscRot );

		// create a rotation using the forward velocity and the current "up" vector...
		LTRotation rotMasterRotation( vNormalizedVelocity, rotDiscRot.Up() );

		// so we can generate a disc space to world space matrix
		LTMatrix mTrans;
		rotMasterRotation.ConvertToMatrix( mTrans );

		// create the shards
		ClusterAngleList::const_iterator citer;
		for( citer = m_lShardReleaseAngle.begin();
			 citer != m_lShardReleaseAngle.end();
			 ++citer )
		{
			// create the forward vector for this shard
			// NOTE: the DetermineShardForwardVector uses a method
			// particular to the needs of this function, so
			// the results it returns may be strange.  Check
			// the function itself for an explanation
			LTVector vShardForward;
			DetermineShardForwardVector( &vShardForward, citer->fYaw, citer->fPitch );

			// transform the vector into world space
			vShardForward = mTrans * vShardForward;

			// launch shard
			LaunchShard( vShardForward );
		}
	}

	// test the rotation
	CDisc::EnterReturnMode( bImpact );
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClusterDisc::DetermineShardForwardVector()
//
//	PURPOSE:	Given a yaw, a pitch, and a magnitude, generate a "forward" vector
//
// ----------------------------------------------------------------------- //

void CClusterDisc::DetermineShardForwardVector( LTVector *pvForward,
                                                float fYaw,
                                                float fPitch,
                                                float fMagnitude )
{
	//
	// I'm using a modified version of spherical coordinates.
	// Typically phi is measured from the vertical axis (the
	// Y axis in this case, because it's a Y-up world), but
	// I'm going to measure it from the horizontal plane.
	// Furthur, theta typically is measured from X, but
	// because of the transforms involved, I'm measuring
	// it from the Z.
	// Examples:  (remember, Y-up world)
	//    ( r, theta, phi )
	//    ( 1, 0, 0 )     ->  +Z axis
	//    ( 1, 0, 90 )    ->  +Y axis
	//    ( 1, 90, 0 )    ->  +X axis
	//    ( 1, -90, -45 ) ->  along line Y = X, pointing down
	//

	ASSERT( ( -MATH_HALFPI < fPitch ) && ( MATH_HALFPI > fPitch ) );

	pvForward->x = fMagnitude * ltcosf( fPitch ) * ltsinf( fYaw );
	pvForward->y = fMagnitude * ltsinf( fPitch );
	pvForward->z = fMagnitude * ltcosf( fPitch ) * ltcosf( fYaw );
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClusterDisc::LaunchShard()
//
//	PURPOSE:	Take the forward vector and launch a shard in that direction
//
// ----------------------------------------------------------------------- //

void CClusterDisc::LaunchShard( LTVector const &vForward )
{
	WeaponFireInfo fireInfo;

	// the shard has the same "owner" as the disc
	fireInfo.hFiredFrom = GetFiredFrom();

	// no test object (this is more for AIs)
	fireInfo.hTestObj = 0;

	// set the direction of the shard
	fireInfo.vPath = vForward;

	// get the position to start the shard
	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );

	// set the flash and fire position to the same as the disc
	fireInfo.vFirePos = vPos;
	fireInfo.vFlashPos = vPos;

	// set the timestame
	fireInfo.nFireTimestamp = static_cast< uint32 >( g_pLTServer->GetTime() * 1000.0f );

	// type of disc tracking
	fireInfo.nDiscTrackingType = MPROJ_DISC_TRACKING_CONTROL_LINE;

	// control vector to help with controlling the disc
	fireInfo.vControlDirection = vForward;

	// control location to help with controlling the disc
	fireInfo.vControlPosition = vPos;

	// an object needed if the disc is homing in on an object
	fireInfo.hObjectTarget = 0;

	// Optional socket for the disc to return to.
	fireInfo.hSocket = 0;

	fireInfo.fDiscReleaseAngle = 0;

	//
	// Create the projectile
	//

	// NOTE: because of the interdependancies of the code, you
	// cannot fire a projectile without a weapon, which also means
	// that you have to have a CWeapons too.  It works, but BLAH!

	// projectile info for the cluster
	PROJECTILEFX const *pProjectileFX = m_pAmmoData->pProjectileFX;
	ASSERT( 0 != pProjectileFX );

	// get the cluster class data
	CLUSTERDISCCLASSDATA *pClusterData =
		dynamic_cast< CLUSTERDISCCLASSDATA* >( pProjectileFX->pClassData );
	ASSERT( 0 != pClusterData );

	// get the shard's weapon struct
	WEAPON const *pWeaponData = g_pWeaponMgr->GetWeapon( pClusterData->szShardWeaponName );
	ASSERT( 0 != pWeaponData );
	if ( 0 == pWeaponData )
	{
		return;
	}

	// here's that weapons structure (blah!)
	CWeapons weapons;

	// init the weapons with the projectile data (blah!)
	weapons.Init( m_hObject );

	// add the shard as the only weapon it will contain (blah!)
	weapons.ObtainWeapon( pWeaponData->nId );

	// change the weapon to the shard (blah!)
	weapons.ChangeWeapon( pWeaponData->nId );

	// get the CWeapon pointer from the weapons thingy (blah!)
	CWeapon* pWeapon = weapons.GetCurWeapon();
	if (!pWeapon) return;

	// set the ammo using the default ammo (blah!)
	weapons.SetAmmo( pWeapon->GetAmmoId() );

	// fire the weapon (blah!)
	pWeapon->Fire( fireInfo );
}
