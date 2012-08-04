// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileTypes.h
//
// PURPOSE : ProjectileTypes class - definition
//
// CREATED : 10/3/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILE_TYPES_H__
#define __PROJECTILE_TYPES_H__

#include "Projectile.h"
#include "Timer.h"

struct PROXCLASSDATA;
struct KITTYCLASSDATA;
struct SPAWNCLASSDATA;
struct BEARTRAPCLASSDATA;
struct BANANACLASSDATA;
struct SPEARCLASSDATA;

enum ContainerCode;
enum EnumAIStimulusID;

LINKTO_MODULE( ProjectileTypes );

extern CTList<class CGrenade*> g_lstGrenades;

class CGrenade : public CProjectile
{
	public :

		CGrenade();
		~CGrenade();

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual void HandleImpact(HOBJECT hObj);
		virtual void UpdateGrenade();
        virtual void ResetRotationVel(LTVector* pvNewVel=LTNULL, SURFACE* pSurf=LTNULL);
		virtual void RotateToRest();
		virtual void Detonate(HOBJECT hObj);

        virtual void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        virtual void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		virtual LTBOOL ShouldPlayBounceSound(SURFACE* pSurface);
		virtual const char* GetBounceSound(SURFACE* pSurface);

        LTBOOL   m_bSpinGrenade;     // Should the grenade spin
        LTBOOL   m_bUpdating;        // Are we updating the grenade

		ContainerCode m_eContainerCode;
		SurfaceType	  m_eLastHitSurface;

        LTFLOAT  m_fPitch;
        LTFLOAT  m_fYaw;
        LTFLOAT  m_fRoll;
        LTFLOAT  m_fPitchVel;
        LTFLOAT  m_fYawVel;
        LTFLOAT  m_fRollVel;
		LTBOOL	 m_bRotatedToRest;
		uint32	 m_cBounces;
		LTBOOL	 m_bAddToGrenadeList;

		// Don't need to save this...

        HLTSOUND    m_hBounceSnd;   // Handle to current bounce sound
};


class CLipstickProx : public CGrenade
{
	public :

		CLipstickProx() : CGrenade()
		{
			m_vSurfaceNormal.Init();
            m_bArmed     = LTFALSE;
            m_bActivated = LTFALSE;
            m_pClassData = LTNULL;
		}

		virtual LTBOOL	Setup(CWeapon const* pWeapon, WeaponFireInfo const& info);

	protected :

		virtual void UpdateGrenade();
		virtual void HandleImpact(HOBJECT hObj);
		virtual void RotateToRest();

        virtual void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        virtual void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

        LTVector m_vSurfaceNormal;
        LTBOOL   m_bArmed;
        LTBOOL   m_bActivated;
		CTimer	m_DetonateTime;
		CTimer  m_ArmTime;

		PROXCLASSDATA* m_pClassData;
};


class CGrenadeImpact : public CGrenade
{
	public :

		CGrenadeImpact() : CGrenade() {}

	protected :

		virtual void HandleImpact(HOBJECT hObj)
		{
			CProjectile::HandleImpact(hObj);
		}
};

class CSpear : public CProjectile
{
	public :

		CSpear();
		~CSpear();
		
		virtual LTBOOL	Setup( const CWeapon *pWeapon, const WeaponFireInfo &info );

	protected :

		virtual void HandleImpact(HOBJECT hObj);

	protected :

		SPEARCLASSDATA	*m_pClassData;
};

class CCameraDisabler : public CProjectile
{
	public :

		CCameraDisabler();
		~CCameraDisabler();

	protected :

		virtual void HandleImpact(HOBJECT hObj);
};

class CCoin : public CGrenade
{
	public :

	protected :

		virtual LTBOOL ShouldPlayBounceSound(SURFACE* pSurface);
		virtual const char* GetBounceSound(SURFACE* pSurface);

		virtual LTBOOL CanSetLastFireInfo() { return LTFALSE; }
		virtual void RotateToRest();
};

class CKitty : public CProjectile
{
	public:

		CKitty();
		~CKitty();

		virtual LTBOOL	Setup( const CWeapon *pWeapon, const WeaponFireInfo & info );

		virtual LTBOOL	CreateDangerStimulus(HOBJECT hFiredFrom);

	protected:
		
		uint32 EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData );

		virtual void	HandleImpact( HOBJECT hObj );
		
		virtual	void	UpdateKitty( );
		virtual	void	UpdateMovement( );
		virtual void	Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		virtual void	Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

		virtual bool	ShouldDetonate( );
		virtual	void	CheckActivation( );

	protected:

		CTimer		m_DetonateTimer;
		CTimer		m_ArmTimer;
		
		bool		m_bArmed;
		bool		m_bActivated;

		LTObjRef	m_hActivator;		// The object the activated the Kitty.
		LTObjRef	m_hSpawnedKitty;

		EnumAIStimulusID	m_eStimID;

		HLTSOUND	m_hArmSound;

		KITTYCLASSDATA *m_pClassData;
};

class CBearTrap : CProjectile
{
	public :

		CBearTrap();
		~CBearTrap();

		virtual LTBOOL	Setup( const CWeapon *pWeapon, const WeaponFireInfo &info );

	protected :

		uint32 EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData );

		virtual void	HandleImpact( HOBJECT hObj );
		virtual void	Detonate( HOBJECT hObj );

		virtual void	UpdateTrap( );
		virtual void	Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		virtual void	Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

	protected :

		CTimer		m_ArmTimer;
		bool		m_bArmed;
		LTObjRef	m_hSpawnedBearTrap;

		BEARTRAPCLASSDATA	*m_pClassData;
};

class CBanana : CGrenade
{
	public :

		CBanana();
		~CBanana();

		virtual LTBOOL	Setup( const CWeapon *pWeapon, const WeaponFireInfo &info );

	protected:
		
		uint32 EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData );
		
		virtual void	HandleImpact( HOBJECT hObj );
		virtual void	Detonate( HOBJECT hObj );

		virtual LTBOOL ShouldPlayBounceSound(SURFACE* pSurface);
		virtual const char* GetBounceSound(SURFACE* pSurface);

		virtual LTBOOL	CanSetLastFireInfo() { return LTFALSE; }
		virtual void	RotateToRest();

		virtual void	UpdateBanana( );
		virtual void	Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		virtual void	Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

	protected :

		CTimer		m_ArmTimer;
		bool		m_bArmed;
		LTObjRef	m_hSpawnedBanana;
		bool		m_bMovedToSpawnedPos;

		BANANACLASSDATA	*m_pClassData;
};

// NOT FINISHED
class CProjectileSpawner : public CProjectile
{
	public :	

		CProjectileSpawner();
		~CProjectileSpawner();

		virtual LTBOOL	Setup(CWeapon const* pWeapon, WeaponFireInfo const& info);

	protected :

		virtual void HandleImpact(HOBJECT hObj);

		SPAWNCLASSDATA *m_pClassData;
};

#endif //  __PROJECTILE_TYPES_H__
