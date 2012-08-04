// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponModel.h
//
// PURPOSE : Generic client-side WeaponModel wrapper class - Definition
//
// CREATED : 9/27/97
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_MODEL_H__
#define __WEAPON_MODEL_H__

#include "clientheaders.h"
#include "WeaponDefs.h"
#include "SurfaceTypes.h"

class CWeaponModel
{
	public :

		CWeaponModel();
		virtual ~CWeaponModel();

		LTBOOL Create(ILTClient* pClientDE, uint8 nWeaponId);
		void ChangeWeapon(uint8 nCommandId);
		void ChangeToPrevWeapon();	
		void ChangeToNextWeapon();	
		void ToggleVehicleMode();

		LTBOOL IsDemoWeapon(uint8 nCommandId);

		WeaponState UpdateWeaponModel(LTRotation rRot, LTVector vPos, LTBOOL bFire);
		void HandleStateChange(ILTMessage_Read* hMessage);

		void Reset();
		HLOCALOBJ GetHandle() const { return m_hObject; }

		void UpdateBob(LTFLOAT fWidth, LTFLOAT fHeight);

		void SetVisible(LTBOOL bVis=LTTRUE);
		void SetZoom(LTBOOL b) { m_bZoomView = b; }

		LTVector GetFlashPos() const { return m_vFlashPos; }
		LTVector GetModelPos() const;

		LTVector GetOffset()			const { return m_vOffset; }
		void SetOffset(LTVector v)	{ VEC_COPY(m_vOffset, v); }

		LTVector GetMuzzleOffset()			const { return m_vMuzzleOffset; }
		void SetMuzzleOffset(LTVector v)		{ VEC_COPY(m_vMuzzleOffset, v); }

		RiotWeaponId GetId() const { return m_nWeaponId; }

		void OnModelKey(HLOCALOBJ hObj, ArgList *pArgs);

	protected : 

		void CreateFlash();
		void CreateModel();
		void RemoveModel();
		void StartFlash();
		void UpdateFlash(WeaponState eState);
		
		WeaponState Fire();
		WeaponState UpdateModelState(LTBOOL bFire);

		void	SendFireMsg();
		void	UpdateFiring();
		void	UpdateNonFiring();
		LTBOOL	PlaySelectAnimation();
		LTBOOL	PlayDeselectAnimation();
		LTBOOL	PlayStartFireAnimation();
		LTBOOL	PlayFireAnimation();
		LTBOOL	PlayReloadAnimation();
		LTBOOL	PlayStopFireAnimation();
		LTBOOL	PlayIdleAnimation();
		void	InitAnimations();

		void	Deselect();
		void	Select();

		void	ClientFire(LTVector & vPath, LTVector & vFirePos);
		void	DoProjectile();
		void	DoVector();

		HLOCALOBJ CreateServerObj();

		void	AddImpact(HLOCALOBJ hObj, LTVector & vInpactPoint, 
						  LTVector & vNormal, SurfaceType eType);
		void	HandleVectorImpact(IntersectQuery & qInfo, IntersectInfo & iInfo);

		
		void	HandleInternalWeaponChange(uint8 nWeaponId);
		void	DoWeaponChange(uint8 nWeaponId);
		LTBOOL	CanChangeToWeapon(uint8 nCommandId);
		void	AutoSelectWeapon();
		uint8	PrevWeapon(uint8 nPlayerMode);
		uint8	NextWeapon(uint8 nPlayerMode);
		uint8	DemoPrevWeapon(uint8 nPlayerMode);
		uint8	DemoNextWeapon(uint8 nPlayerMode);


		ILTClient*	m_pClientDE;	// The ILTClient
		HLOCALOBJ	m_hObject;		// Handle of WeaponModel model

		RiotWeaponId m_nWeaponId;

		LTVector		m_vOffset;
		LTVector		m_vMuzzleOffset;

		LTVector		m_vFlashPos;
		HLOCALOBJ	m_hFlashObject;		// Muzzle flash object
		LTFLOAT		m_fFlashStartTime;	// When did flash start

		// Bobbin' and Swayin' - Blood 2 style ;)
		LTFLOAT		m_fBobHeight;
		LTFLOAT		m_fBobWidth;

	
		
		LTFLOAT		m_fLastIdleTime;
		LTFLOAT		m_fTimeBetweenIdles;
		LTBOOL		m_bFire;
		int			m_nAmmoInClip;
		WeaponState m_eState;			// What are we currently doing
		WeaponState	m_eLastWeaponState;

		HMODELANIM	m_nSelectAni;		// Select weapon
		HMODELANIM	m_nDeselectAni;		// Deselect weapon
		HMODELANIM	m_nStartFireAni;	// Start firing
		HMODELANIM	m_nFireAni;			// Fire
		HMODELANIM	m_nFireAni2;		// Fire2
		HMODELANIM	m_nFireZoomAni;		// Fire when zoomed in
		HMODELANIM	m_nLastFireAni;		// What fire ani was played last
		HMODELANIM	m_nStopFireAni;		// Stop firing
		HMODELANIM	m_nIdleAni1;		// Idle one
		HMODELANIM	m_nIdleAni2;		// Idle two
		HMODELANIM	m_nReloadAni;		// Reload weapon

		LTVector			m_vPath;		// Path of current vector/projectile
		LTVector			m_vFirePos;		// Fire position of current vector/projectile
		LTVector			m_vEndPos;		// Impact location of current vector
		uint8			m_nIgnoreFX;	// FX to ignore for current vector/projectile
		ProjectileType	m_eType;		// Type of weapon
		LTBOOL			m_bZoomView;	// Is the weapon zoomed?

		uint8			m_nRequestedWeaponId;	// Id of weapon to select
		LTBOOL			m_bWeaponDeselected;	// Did we just deselect the weapon
};

#endif // __WEAPON_MODEL_H__