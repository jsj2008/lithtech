// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileFX.h
//
// PURPOSE : Projectile special fx class - Definition
//
// CREATED : 7/6/98
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILE_FX_H__
#define __PROJECTILE_FX_H__

#include "SpecialFX.h"
#include "WeaponDefs.h"
#include "client_physics.h"


struct PROJECTILECREATESTRUCT : public SFXCREATESTRUCT
{
	PROJECTILECREATESTRUCT::PROJECTILECREATESTRUCT();

	uint8	nWeaponId;
	uint8	nShooterId;
	LTBOOL	bLocal;
};

inline PROJECTILECREATESTRUCT::PROJECTILECREATESTRUCT()
{
	memset(this, 0, sizeof(PROJECTILECREATESTRUCT));
	nShooterId = -1;
}


class CProjectileFX : public CSpecialFX
{
	public :

		CProjectileFX() : CSpecialFX() 
		{
			m_nWeaponId		= GUN_NONE;
			m_eSize			= MS_NORMAL;
			m_nFX			= 0;

			m_pSmokeTrail	= LTNULL;
			m_hFlare		= LTNULL;
			m_hLight		= LTNULL;
			m_hProjectile	= LTNULL;
			m_hFlyingSound	= LTNULL;

			m_nShooterId	= -1;
			m_bLocal		= LTFALSE;

			m_bFirstSnake	= LTTRUE;
			m_fSnakeUpVel	= 0.0f;
			m_fSnakeDir		= 1.0f;

			m_fStartTime	= 0.0f;
			m_bDetonated	= LTFALSE;
		}

		~CProjectileFX()
		{
			RemoveFX();
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL CreateObject(ILTClient* pClientDE);
		virtual LTBOOL Update();

		void HandleTouch(CollisionInfo *pInfo, LTFLOAT forceMag);

	protected :
	
		RiotWeaponId	m_nWeaponId;		// Id of weapon fired
		ModelSize		m_eSize;			// Size of projectile
		uint8			m_nFX;				// FX associated with projectile
		uint8			m_nShooterId;		// Client Id of shooter
		LTBOOL			m_bLocal;			// Did local client create this fx

		MovingObject	m_mover;			// Mover (m_bLocal only)

		void CreateSmokeTrail(LTVector & vPos, LTRotation & rRot);
		void CreateFlare(LTVector & vPos, LTRotation & rRot);
		void CreateLight(LTVector & vPos, LTRotation & rRot);
		void CreateProjectile(LTVector & vPos, LTRotation & rRot);
		void CreateFlyingSound(LTVector & vPos, LTRotation & rRot);
		
		void RemoveFX();

		LTBOOL MoveServerObj();
		void  Detonate(CollisionInfo* pInfo);

		CSpecialFX*			m_pSmokeTrail;		// Smoke trail fx
		HLOCALOBJ			m_hFlare;			// Flare fx
		HLOCALOBJ			m_hLight;			// Light fx
		HLOCALOBJ			m_hProjectile;		// The Model/sprite
		HLTSOUND			m_hFlyingSound;		// Sound of the projectile

		// Special anime snaking variables...

		LTBOOL			m_bFirstSnake;
		LTFLOAT			m_fSnakeUpVel;
		LTFLOAT			m_fSnakeDir;

		LTFLOAT			m_fStartTime;

		LTVector			m_vFirePos;
		LTVector			m_vPath;

		LTBOOL			m_bDetonated;

};

#endif // __PROJECTILE_FX_H__