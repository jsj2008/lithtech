// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponFX.h
//
// PURPOSE : Weapon special fx class - Definition
//
// CREATED : 2/22/98
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_FX_H__
#define __WEAPON_FX_H__

#include "SpecialFX.h"
#include "SurfaceDB.h"
#include "ContainerCodes.h"
#include "GameSettings.h"

class CPolyGridFX;

struct WCREATESTRUCT : public SFXCREATESTRUCT
{
	WCREATESTRUCT();

	HOBJECT		hFiredFrom;
	HOBJECT		hObjectHit;
	HWEAPON		hWeapon;
	HAMMO		hAmmo;
	uint8		nSurfaceType;
	uint16		wIgnoreFX;
	LTVector	vFirePos;
	bool		bFXAtFlashSocket;
	LTVector	vPos;
	LTVector	vSurfaceNormal;
	uint8		nShooterId;
	HMODELNODE	hNodeHit;
	bool		bLeftHandWeapon;
};

inline WCREATESTRUCT::WCREATESTRUCT()
{
	hFiredFrom		= NULL;
	hObjectHit		= NULL;
	hWeapon			= NULL;
	hAmmo			= NULL;
	nSurfaceType	= 0;
	wIgnoreFX		= 0;
	vFirePos.Init();
	bFXAtFlashSocket= true;
	vPos.Init();
	vSurfaceNormal.Init();
	nShooterId		= -1;
	hNodeHit		= INVALID_MODEL_NODE;
	bLeftHandWeapon = false;
}


class CWeaponFX : public CSpecialFX
{
	public :

		CWeaponFX() : CSpecialFX()
		{
			m_hFiredFrom	= NULL;
			m_eSurfaceType	= ST_UNKNOWN;
			m_eExitSurface	= ST_UNKNOWN;
			m_wImpactFX		= 0;
			m_wFireFX		= 0;
			m_wIgnoreFX		= 0;
			m_fInstDamage	= 0;
			m_fAreaDamage	= 0;
			m_nShooterId	= INVALID_CLIENT;
			m_nLocalId		= INVALID_CLIENT;
			m_bLocalClientFired = false;
			m_bFXAtFlashSocket	= true;
			m_bLeftHandWeapon	= false;

			m_vFirePos.Init();
			m_vOriginalFirePos.Init( );
			m_vPos.Init();
			m_vDir.Init();
			m_vSurfaceNormal.Init();
			m_vExitPos.Init();
			m_vExitNormal.Init();

			m_rSurfaceRot.Init();
			m_rDirRot.Init();

			m_eExitCode		= CC_NO_CONTAINER;
			m_eCode			= CC_NO_CONTAINER;
			m_eFirePosCode	= CC_NO_CONTAINER;
			m_fFireDistance	= 100.0f;
			m_nDetailLevel	= RS_HIGH;

			m_hWeapon		= NULL;
			m_hAmmo			= NULL;

			m_bFailedToFindBreach = false;
		}

		virtual bool Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
		virtual bool Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual bool CreateObject(ILTClient* pClientDE);
		virtual bool Update() { return false; }

		virtual uint32 GetSFXID() { return SFX_WEAPON_ID; }

	protected :

		LTObjRef		m_hFiredFrom;		// Who fired the weapon
		LTObjRef		m_hObjectHit;		// The object we hit if any
		SurfaceType		m_eSurfaceType;		// Surface hit by bullet
		SurfaceType		m_eExitSurface;		// Surface bullet is exiting
		uint16			m_wFireFX;			// Fire FX to create
		uint16			m_wImpactFX;		// Impact FX to create
		uint16			m_wIgnoreFX;		// Fire FX to ignore
		float			m_fInstDamage;		// Instantaneous damage (vector)
		float			m_fAreaDamage;		// Area damage (explosion)
		LTVector		m_vFirePos;			// Position bullet was fired from
		LTVector		m_vOriginalFirePos;	// m_vFirePos may be modified if the FX is supposed to come from the Flash
											// socket on the weapon.  This is the original fire position and may be used to develop
											// the original direction vector of the fire path...
		bool			m_bFXAtFlashSocket;	// Play FX at the position of the flash socket.
		bool			m_bLeftHandWeapon;  // Play FX for left hand weapon
		LTVector		m_vPos;				// Impact pos
		LTVector		m_vExitPos;			// Bullet exit pos
		LTVector		m_vExitNormal;		// Exit surface normal

		uint8			m_nDetailLevel;		// Current detail level setting

		ContainerCode	m_eCode;			// Container effect is in
		ContainerCode	m_eFirePosCode;		// Container fire pos is in
		ContainerCode	m_eExitCode;		// Container exit pos is in

		float			m_fFireDistance;	// Distance from fire pos to pos
		LTVector		m_vDir;				// Direction from fire pos to pos
		LTVector		m_vSurfaceNormal;	// Normal of surface of impact
		LTRotation		m_rSurfaceRot;		// Normal of surface (as rotation)
		LTRotation		m_rDirRot;			// Rotation based on m_vDir

		uint8			m_nShooterId;		// Client id of the shooter
		uint8			m_nLocalId;			// Local client id
		bool			m_bLocalClientFired;// Did the local client fire the weapon?

		LTVector		m_vLightColor;		// Impact light color

		HWEAPON			m_hWeapon;			// Weapon fired
		HAMMO			m_hAmmo;			// Ammo used

		HMODELNODE		m_hNodeHit;			// The node that was impacted if the object was a model
		ModelsDB::HNODE m_hModelDBNode;		// The ModelsDB node of m_hNodeHit

		bool			m_bFailedToFindBreach;

		void	SetupExitInfo();

		void	CreateExitMark();
		void	CreateMark(const LTVector &vPos, const LTVector &vNorm, const LTRotation &rRot, SurfaceType eType);
		void	CreateTracer();

		void	CreateWeaponImpactFX();
		void	CreateWeaponBeamFX();
		void	CreateSurfaceSpecificFX();
		void	CreateSurfaceSpecificImpactFX(HSURFACE hSurf, HSRF_IMPACT hImpact, bool bUnderWater);
		void	CreateSurfaceSpecificImpactFX_Normal(HSURFACE hSurf, HSRF_IMPACT hImpact, bool bUnderWater);
		bool	CreateSurfaceSpecificImpactFX_NormalNode(ModelsDB::HNODE hNode, HAMMODATA hAmmoData, bool bUnderWater);
		void	CreateSurfaceSpecificImpactFX_Outgoing(HSURFACE hSurf, HSRF_IMPACT hImpact, bool bUnderWater);
		void	CreateSurfaceSpecificImpactFX_ToViewer(HSURFACE hSurf, HSRF_IMPACT hImpact, bool bUnderWater);
		void	CreateSurfaceSpecificImpactFX_ToSource(HSURFACE hSurf, HSRF_IMPACT hImpact, bool bUnderWater);
		void	CreateSurfaceSpecificImpactFX_OnSource(HSURFACE hSurf, HSRF_IMPACT hImpact, bool bUnderWater);
		void	CreateSurfaceSpecificImpactFX_Protruding( HSURFACE hSurf, HSRF_IMPACT hImpact, bool bUnderWater );
		void	CreateMuzzleFX();
		void	CreateShell();
		void	CreatePhysicsImpulseForce( );
		void	PlayFireSound();
		void	PlayBulletFlyBySound();
		void	PlayImpactDing();

		void	PerturbImpactedPolygrid();
		bool	PathIntersectsPolygrid(const CPolyGridFX* pPGrid, LTVector & vIntersectPos);
		
		void	ApplyModelDecal();

		bool	IsBulletTrailWeapon();

		LTVector CalcFirePos(const LTVector &vFirePos, bool bLeftHand);
		LTVector CalcBreachPos(const LTVector &vBreachPos, bool bLeftHand);
};

#endif // __WEAPON_FX_H__