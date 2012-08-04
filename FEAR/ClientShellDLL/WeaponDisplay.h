// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponDisplay.h
//
// PURPOSE : Definition of custom weapon display class
//
// CREATED : 09/13/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPONDISPLAY_H__
#define __WEAPONDISPLAY_H__

#include "LayoutDB.h"
#include "LTPoly.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		WeaponDisplayDB
//
//	PURPOSE:	Database for accessing character display db info.
//
// ----------------------------------------------------------------------- //
BEGIN_DATABASE_CATEGORY( WeaponDisplayDB, "Interface/WeaponDisplay" )
END_DATABASE_CATEGORY( );

// ----------------------------------------------------------------------- //
//
//	CLASS:		WeaponDisplayDB
//
//	PURPOSE:	Database for accessing character display db info.
//
// ----------------------------------------------------------------------- //
BEGIN_DATABASE_CATEGORY( WeaponDisplayLayoutDB, "Interface/WeaponDisplayLayout" )
END_DATABASE_CATEGORY( );


class CWeaponModelData;


class WeaponDisplayInterf
{
public:

	WeaponDisplayInterf();
	virtual ~WeaponDisplayInterf();

	//create a weapon display for the specified weapon, using the supplied database record
	static WeaponDisplayInterf* CreateDisplay(CWeaponModelData* pParent, HRECORD hDisplay );

	//set up the display
	virtual bool Init(CWeaponModelData* pParent, HRECORD hDisplay) = 0;

	//clean up
	virtual void Term() = 0;

	//re-initialize the display
	virtual bool Reset() = 0;

	//main update function
	virtual void UpdateDisplay(bool bFiredWeapon) = 0;

	//rendering
	virtual void Render() = 0;

	//reset the display material on the weapon.  the default implementation is provided
	//since it should not be a required part of the interface - overridden in WeaponDisplay
	virtual bool SetMaterial() { return true; }
};

class WeaponDisplay : public WeaponDisplayInterf
{
public:
	WeaponDisplay();
	virtual ~WeaponDisplay() {	Term( ); }

	virtual bool Init(CWeaponModelData* pParent, HRECORD hDisplay );
	virtual bool Reset();
	virtual void Term( );
	virtual void UpdateDisplay(bool bFiredWeapon);
	virtual void Render();
	virtual bool SetMaterial();

protected:
	//this handles creation of the render target
	virtual bool CreateRenderTarget();

	//this will release the associated render target
	virtual bool ReleaseRenderTarget();

	// set up info
	CWeaponModelData*	m_pParent;
	HRECORD			m_hDisplayRecord;
	HRECORD			m_hLayoutRecord;

	//our render target that we will be displaying
	HRENDERTARGET	m_hRenderTarget;
	HMATERIAL		m_hMaterial;
	uint32			m_nMaterialIndex;
	bool			m_bRenderTargetBound;
	float			m_fUpdateInterval;
	double			m_fLastUpdate;
	LTVector2n		m_v2nSize;

	// draw prim elements to render ammo display
	CLTGUIString	m_Text;
	HMATERIAL		m_hBackground;
	LTPoly_GTTS4	m_Background;

	//current ammo count (used to limit re-rendering)
	int32			m_nCount;

	// camera info
	std::string		m_sCameraSocket;
	LTVector2		m_vCameraFov;

	//render target shared across all displays
	static HRENDERTARGET	s_hRenderTarget;
	static uint32			s_nRenderTargetRefCount;
	static LTVector2n		s_v2nRenderTargetSize;


};

class WeaponDisplayAmmoBarV : public WeaponDisplay
{
public:
	WeaponDisplayAmmoBarV();
	virtual ~WeaponDisplayAmmoBarV() {	Term( ); }

	//set up the display
	virtual bool Init(CWeaponModelData* pParent, HRECORD hDisplay );

	//updates the render target
	virtual void UpdateDisplay(bool bFiredWeapon);

	//does the actual render
	virtual void Render();

protected:
	uint32				m_cFullColor;
	uint32				m_cEmptyColor;
	LTPoly_G4			m_Poly[2];
	int32				m_nMaxCount;
	LTRect2f			m_rfRect;
};

class WeaponDisplayRecharge : public WeaponDisplay
{
public:
	WeaponDisplayRecharge();
	virtual ~WeaponDisplayRecharge() {	Term( ); }

	//set up the display
	virtual bool Init(CWeaponModelData* pParent, HRECORD hDisplay );

	//updates the render target
	virtual void UpdateDisplay(bool bFiredWeapon);

	//does the actual render
	virtual void Render();
protected:

	LTPoly_GT4			m_Poly[3];
	LTRect2f			m_rfRect;
	TextureReference	m_Textures[4];

	float				m_fRecharge;
	StopWatchTimer		m_RechargeTimer;
	bool				m_bReady;

};

class WeaponDisplay3DProgressBar : public WeaponDisplayInterf
{
public:
	WeaponDisplay3DProgressBar();
	virtual ~WeaponDisplay3DProgressBar() { Term(); }

	virtual bool		Init(CWeaponModelData* pParent, HRECORD hDisplay );
	virtual void		Term();
	virtual bool		Reset();

	virtual void		UpdateDisplay(bool bFiredWeapon);
	virtual void		Render();

protected:

	// interface to initialize the material list data
	virtual bool		InitFirstMaterialIndex()	= 0;
	virtual bool		InitNumMaterials()			= 0;
	virtual bool		InitMaterialListAttribute()	= 0;

	// must be clamped to [0,1]
	virtual float		GetProgress()				= 0;

	uint32				m_nFirstMaterialIndex;
	uint32				m_nNumMaterials;
	HATTRIBUTE			m_hMaterialListAtt;
	uint32				m_nLastProgressIndex;

	std::vector<std::string> m_DefaultMaterials;
	std::vector<std::string> m_ActiveMaterials;

	CWeaponModelData*	m_pParent;
	HRECORD				m_hDisplayRecord;
};


class WeaponDisplaySpectrometer : public WeaponDisplay3DProgressBar
{
public:

	WeaponDisplaySpectrometer();
	virtual ~WeaponDisplaySpectrometer() { Term(); }

	virtual bool		InitFirstMaterialIndex();
	virtual bool		InitNumMaterials();
	virtual bool		InitMaterialListAttribute();
	virtual float		GetProgress();
	virtual void		Render();
};

class WeaponDisplayRenderTargetOverlay : public WeaponDisplay
{
public:

	WeaponDisplayRenderTargetOverlay();
	virtual ~WeaponDisplayRenderTargetOverlay() { Term(); }

	virtual bool		Init(CWeaponModelData* pParent, HRECORD hDisplay );
	virtual void		Term();
	virtual void		Render();

protected:

	virtual bool		CreateRenderTarget();
	virtual bool		ReleaseRenderTarget();

	CClientFXLink		m_linkClientFX;		// link to overlay effect
	CClientFXMgr*		m_pClientFXMgr;		// custom overlay manager
};

#endif  // __WEAPONDISPLAY_H__
