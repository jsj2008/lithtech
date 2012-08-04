
#ifndef  __WEAPON_FIRE_INFO_H__
#define  __WEAPON_FIRE_INFO_H__


#include "MsgIDs.h"

class CWeaponPath;

struct WeaponFireInfo
{
	WeaponFireInfo()
	:	hFiredFrom			( NULL ),
		hFiringWeapon		( NULL ),
		hTestObj			( NULL ),
		hNodeHit			( INVALID_MODEL_NODE ),
		vPath				( 0.0f, 0.0f, 0.0f ),
		vFirePos			( 0.0f, 0.0f, 0.0f ),
		vFlashPos			( 0.0f, 0.0f, 0.0f ),
		nSeed				( 0 ),
		nPerturbCount		( 0 ),
		bAltFire			( false ),
		fPerturb			( 1.0f ),	// 0.0 - 1.0
		bOverrideVelocity	( false ),
		fOverrideVelocity	( 0.0f ),
		nFireTimestamp		( 0 ),
		bSendWeaponRecord	( true ),
		bSendAmmoRecord		( true ),
		bGuaranteedHit		( false ),
		pWeaponPath			( NULL ),
		bLeftHandWeapon		( false )
	{
	}

	HOBJECT		hFiredFrom;
	HOBJECT		hFiringWeapon;
	HOBJECT		hTestObj;
	HMODELNODE	hNodeHit;
	LTVector	vPath;
	LTVector	vFirePos;
	LTVector	vFlashPos;
	uint8		nSeed;
	uint8		nPerturbCount;
	bool		bAltFire;
	float		fPerturb;
	bool		bOverrideVelocity;
	float		fOverrideVelocity;
	uint32		nFireTimestamp;
	bool		bSendWeaponRecord;
	bool		bSendAmmoRecord;
	bool		bGuaranteedHit;
	bool		bLeftHandWeapon;

	CWeaponPath	*pWeaponPath;
};

#endif //__WEAPON_FIRE_INFO_H__
