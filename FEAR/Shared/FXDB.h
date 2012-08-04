// ----------------------------------------------------------------------- //
//
// MODULE  : FXDB.h
//
// PURPOSE : Definition of the effects database.
//
// CREATED : 02/09/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __FXDB_H__
#define __FXDB_H__


//
// Includes...
//

#include "GameDatabaseMgr.h"
#include "resourceextensions.h"
#include "ContainerCodes.h"


//
// Defines...
//


//ImpactFX
const char* const FXDB_rPusher =			"Pusher";
const char* const FXDB_nAISoundRadius =		"AISoundRadius";
const char* const FXDB_bAIIgnoreSurface =	"AIIgnoreSurface";
const char* const FXDB_nAIAlarmLevel =		"AIAlarmLevel";
const char* const FXDB_sAIStimulusType =	"AIStimulusType";

//pusher FX
const char* const FXDB_fRadius =			"Radius";
const char* const FXDB_fStartDelay =		"StartDelay";
const char* const FXDB_fDuration =			"Duration";
const char* const FXDB_fStrength =			"Strength";

//tracer FX
const char* const FXDB_nFrequency =			"Frequency";
const char* const FXDB_sFXName =			"FXName";

//fire FX
const char* const FXDB_sShellModel =		"ShellModel";
const char* const FXDB_sShellMaterial =		"ShellMaterial";
const char* const FXDB_fShellScale =		"ShellScale";
const char* const FXDB_vShellEjectMinVel =	"ShellEjectMinVelocity";
const char* const FXDB_vShellEjectMaxVel =	"ShellEjectMaxVelocity";
const char* const FXDB_sBeamFXName =		"BeamFXName";
const char* const FXDB_sUWBeamFXName =		"UWBeamFXName";

//projectile FX
const char* const FXDB_bFlySound =			"FlySound";
const char* const FXDB_nSoundRadius =		"SoundRadius";
const char* const FXDB_sStimulus =			"Stimulus";
const char* const FXDB_sClass =				"Class";
const char* const FXDB_sClassData =			"ClassData";
const char* const FXDB_sModel =				"Model";
const char* const FXDB_sMaterial =			"Material";
const char* const FXDB_fModelScale =		"ModelScale";
const char* const FXDB_sSound =				"Sound";
const char* const FXDB_bGravity =			"Gravity";
const char* const FXDB_fLifetime =			"Lifetime";
const char* const FXDB_nVelocity =			"Velocity";
const char* const FXDB_fFireOffset =		"FireOffset";
const char* const FXDB_fFireElevation =		"FireElevation";
const char* const FXDB_bCanHitSameKind =	"CanHitSameKind";
const char* const FXDB_bFXLoop =			"FXLoop";
const char* const FXDB_bFXSmoothShutdown =	"FXSmoothShutdown";
const char* const FXDB_sFXSocket =			"FXSocket";
const char* const FXDB_sFXTarget =			"FXTarget";
const char* const FXDB_sAnimation =			"Animation";
const char* const FXDB_bRandomRoll =		"RandomRoll";
const char* const FXDB_bDamagedByOwner =	"DamagedByOwner";
const char* const FXDB_sDamageMask =		"DamageMask";
const char* const FXDB_rCollisionProperty =	"CollisionProperty";

//spear class data
const char* const FXDB_fStickPercent =		"StickPercent";
const char* const FXDB_bCanWallStick =		"CanWallStick";
const char* const FXDB_vDimsScale =			"DimsScale";

//proximity class data
const char* const FXDB_sArmAnimation =		"ArmAnimation";
const char* const FXDB_sArmedAnimation =	"ArmedAnimation";
const char* const FXDB_sActivateAnimation =	"ActivateAnimation";
const char* const FXDB_sArmedFX =			"ArmedFX";
const char* const FXDB_fActivationRadius =	"ActivationRadius";
const char* const FXDB_fPopUpVelocity =		"PopUpVelocity";

//remote class data
const char* const FXDB_sOverrideFX =		"OverrideFX";



struct IFXCS
{
	IFXCS()	:	
		vPos			( 0.0f, 0.0f, 0.0f ),
		vDir			( 0.0f, 0.0f, 0.0f ),
		vSurfNormal		( 0.0f, 0.0f, 0.0f ),
		rSurfRot		( 0.0f, 0.0f, 0.0f, 1.0f ),
		eSurfType		( ST_UNKNOWN ),
		eCode			( CC_NO_CONTAINER ),
		bPlaySound		( false )
	{

	}

	LTVector		vPos;           // Position of FX
	LTVector		vDir;           // Direction for scale fx
	LTVector		vSurfNormal;    // Normal of surface of impact
	LTRotation		rSurfRot;       // Rotation aligned with surface normal
	SurfaceType		eSurfType;		// Type of surface impacting on
	ContainerCode	eCode;			// Container code fx is currently in
	bool			bPlaySound;     // Should the sound be played
};


class CFXDB;
extern CFXDB* g_pFXDB;

class CFXDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( CFXDB );

public :	// Methods...

	bool	Init( const char *szDatabaseFile = DB_Default_File );
	void	Term() {};

	//------------------------------------------------------
	// Access to data
#if defined(_CLIENTBUILD)
	void CreateImpactFX(HRECORD hImpactFX, IFXCS & cs);
	void CreatePusherFX(HRECORD hPusherFX, const LTVector &vPos);
#endif // _CLIENTBUILD

	HRECORD		GetImpactFX(uint32 nImpactFXId);
	HRECORD		GetImpactFX(const char *pName );

#ifdef _SERVERBUILD
	uint32		GetNumImpactFX() const;
	bool		ReadImpactFXProp( const GenericPropList *pProps, const char *pszPropName, uint32 & nImpactFXId);
#endif // _SERVERBUILD

	HRECORD		GetTracerFX(const char* pName);

	HRECORD		GetFireFX(const char* pName);
	uint16		GetFireFlags(HRECORD hFire);

	HRECORD		GetProjectileFX(const char* pName);
	uint16		GetProjectileFlags(HRECORD hProjectile);
	uint32		GetProjectileObjectFlags(HRECORD hProjectile);
	uint32		GetProjectileFXFlags(HRECORD hProjectile);

	HRECORD		GetSpearClassData(HRECORD hProjectile);
	HRECORD		GetProximityClassData(HRECORD hProjectile);
	HRECORD		GetRemoteClassData(HRECORD hProjectile);

private	:	// Members...

	HCATEGORY	m_hImpactCat;
	HCATEGORY	m_hTracerCat;
	HCATEGORY	m_hFireCat;
	HCATEGORY	m_hProjectileCat;

	HCATEGORY	m_hSpearClassDataCat;
	HCATEGORY	m_hProximityClassDataCat;
	HCATEGORY	m_hRemoteClassDataCat;

};


////////////////////////////////////////////////////////////////////////////
//
// CFXDBPlugin is used to help facilitate populating the WorldEdit object
// properties that use FXDB
//
////////////////////////////////////////////////////////////////////////////
#ifdef _SERVERBUILD

#include "iobjectplugin.h"


class CFXDBPlugin : public IObjectPlugin
{
private:

	CFXDBPlugin();
	CFXDBPlugin( const CFXDBPlugin &other );
	CFXDBPlugin& operator=( const CFXDBPlugin &other );
	~CFXDBPlugin();


public:

	NO_INLINE static CFXDBPlugin& Instance() { static CFXDBPlugin sPlugin; return sPlugin; }


	static bool PopulateStringList(char** aszStrings, uint32* pcStrings,
		const uint32 cMaxStrings, const uint32 cMaxStringLength);


};


#endif // _SERVERBUILD


#endif  // __FXDB_H__
