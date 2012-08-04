// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponFX.h
//
// PURPOSE : Weapon special fx class - Definition
//
// CREATED : 2/22/98
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_FX_H__
#define __WEAPON_FX_H__

#include "SpecialFX.h"
#include "SurfaceTypes.h"
#include "TracerFX.h"
#include "SmokeFX.h"
#include "ContainerCodes.h"
#include "WeaponDefs.h"
#include "ModelFuncs.h"
#include "RiotSettings.h"

struct WCREATESTRUCT : public SFXCREATESTRUCT
{
	WCREATESTRUCT::WCREATESTRUCT();

	uint8		nWeaponId;
	uint8		nSurfaceType;
	uint8		nIgnoreFX;
	LTVector		vFirePos;
	LTVector		vPos;
	LTRotation	rRot;
	uint8		nShooterId;
	LTBOOL		bLocal;
};

inline WCREATESTRUCT::WCREATESTRUCT()
{
	memset(this, 0, sizeof(WCREATESTRUCT));
	nShooterId = -1;
}


class CWeaponFX : public CSpecialFX
{
	public :

		CWeaponFX() : CSpecialFX() 
		{
			m_nWeaponId		= GUN_NONE;
			m_eSurfaceType	= ST_UNKNOWN;
			m_nFX			= 0;
			m_nIgnoreFX		= 0;
			m_fDamage		= 0;
			m_nShooterId	= -1;
			m_nLocalId		= -1;
			m_bLocal		= LTFALSE;

			VEC_INIT(m_vFirePos);
			VEC_INIT(m_vPos);
			VEC_INIT(m_vDir);
			VEC_INIT(m_vSurfaceNormal);
			VEC_SET(m_vLightColor, 1.0f, 1.0f, 1.0f);

			m_rRotation.Init();
			m_rSurfaceRot.Init();
			m_rDirRot.Init();

			m_eSize			= MS_NORMAL;
			m_eCode			= CC_NONE;
			m_eFirePosCode	= CC_NONE;

			m_bExplosionWeapon = LTFALSE;
			m_fFireDistance	   = 100.0f;

			m_nDetailLevel	= RS_HIGH;
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL CreateObject(ILTClient* pClientDE);
		virtual LTBOOL Update() { return LTFALSE; }

	protected :
	
		RiotWeaponId	m_nWeaponId;		// Id of weapon fired
		SurfaceType		m_eSurfaceType;		// Surface hit by bullet
		uint8			m_nFX;				// FX to create
		uint8			m_nIgnoreFX;		// FX to ignore
		LTFLOAT			m_fDamage;			// Damage done by bullet
		LTRotation		m_rRotation;		// Norval of surface (as rotation)
		LTVector			m_vFirePos;			// Position bullet was fired from
		LTVector			m_vPos;				// Impact pos

		uint8			m_nDetailLevel;		// Current detail level setting

		ModelSize		m_eSize;			// Size of weapon fx
		ContainerCode	m_eCode;			// Container effect is in
		ContainerCode	m_eFirePosCode;		// Container fire pos is in

		LTBOOL			m_bExplosionWeapon;	// Did the weapon go boom?
		LTFLOAT			m_fFireDistance;	// Distance from fire pos to pos
		LTVector			m_vDir;				// Direction from fire pos to pos
		LTVector			m_vSurfaceNormal;	// Normal of surface of impact
		LTRotation		m_rSurfaceRot;		// Rotation based on m_vSurfaceNormal
		LTRotation		m_rDirRot;			// Rotation based on m_vDir

		uint8			m_nShooterId;		// Client id of the shooter
		uint8			m_nLocalId;			// Local client id
		LTBOOL			m_bLocal;			// Is this a local fx (only done on this client?)

		LTVector			m_vLightColor;		// Impact light color

		void CreateMark();
		void CreateSparks();
		void CreateSmoke();
		void CreateTracer();
		void CreateBulletTrail(LTVector* pvStartPos);
		void TintScreen();

		void CreateWeaponSpecificFX();
		void CreateBeamFX();
		void CreateMuzzleFX();
		void CreateShell();
		void CreateMuzzleLight();
		void PlayImpactSound();
		void PlayFireSound();

		char* GetSparkTexture(RiotWeaponId nWeaponId, SurfaceType eSurfType,
							  LTVector* pvColor1, LTVector* pvColor2);
		void SetTracerValues(RiotWeaponId nWeaponId, TRCREATESTRUCT* pTR);
		void SetupSmoke(RiotWeaponId nWeaponId, SurfaceType eSurfType, SMCREATESTRUCT* pSM);
		void CreateLightFX();
		void CreateVectorBloodFX(LTVector & vVelMin, LTVector & vVelMax, LTFLOAT fRange);
		void CreateLowVectorBloodFX(LTVector & vVelMin, LTVector & vVelMax, LTFLOAT fRange);
		void CreateMeLTVectorBloodFX(LTVector & vVelMin, LTVector & vVelMax, LTFLOAT fRange);
		void CreateBlastMark();

		LTBOOL IsBulletTrailWeapon(RiotWeaponId nWeaponId);

		LTBOOL DetermineDamageFX(LTBOOL & bOnlyDoVectorDamageFX, 
								LTBOOL & bDoVectorDamageFX, 
								LTBOOL & bOnlyDoProjDamageFX,
								LTBOOL & bCreateSparks);

		void CreatePulseRifleFX();
		void CreateSpiderFX();
		void CreateBullgutFX();
		void CreateSniperRifleFX();
		void CreateJuggernautFX();
		void CreateShredderFX();
		void CreateRedRiotFX();
		void CreateColt45FX();
		void CreateShotgunFX();
		void CreateAssaultRifleFX();
		void CreateEnergyGrenadeFX();
		void CreateKatoGrenadeFX();
		void CreateMac10FX();
		void CreateTOWFX();
		void CreateTantoFX();

		void CreateMedPulseRifleFX();
		void CreateMedSpiderFX();
		void CreateMedBullgutFX();
		void CreateMedSniperRifleFX();
		void CreateMedJuggernautFX();
		void CreateMedShredderFX();
		void CreateMedRedRiotFX();
		void CreateMedColt45FX();
		void CreateMedShotgunFX();
		void CreateMedAssaultRifleFX();
		void CreateMedEnergyGrenadeFX();
		void CreateMedKatoGrenadeFX();
		void CreateMedMac10FX();
		void CreateMedTOWFX();
		void CreateMedTantoFX();

		void CreateLowPulseRifleFX();
		void CreateLowSpiderFX();
		void CreateLowBullgutFX();
		void CreateLowSniperRifleFX();
		void CreateLowJuggernautFX();
		void CreateLowShredderFX();
		void CreateLowRedRiotFX();
		void CreateLowColt45FX();
		void CreateLowShotgunFX();
		void CreateLowAssaultRifleFX();
		void CreateLowEnergyGrenadeFX();
		void CreateLowKatoGrenadeFX();
		void CreateLowMac10FX();
		void CreateLowTOWFX();
		void CreateLowTantoFX();

		void CreateEnergyBatonFX()	{}
		void CreateEnergyBladeFX()	{}
		void CreateKatanaFX()		{}
		void CreateMonoKnifeFX()	{}
		void CreateLaserCannonFX()	{}

		void CreateRedRiotBeam();
		void CreateJuggernautBeam();
		void CreateShredderBeam();
		void CreateLaserCannonBeam();
};

#endif // __WEAPON_FX_H__