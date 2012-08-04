// ----------------------------------------------------------------------- //
//
// MODULE  : SurfaceDB.h
//
// PURPOSE : Definition of Surface Database
//
// CREATED : 03/04/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SURFACEDB_H__
#define __SURFACEDB_H__



//
// Includes...
//

#include "GameDatabaseMgr.h"
#include "SurfaceDefs.h"
#include "CommonUtilities.h"

//
// Defines...
//
#define SrfDB_MaxSurfaces 256

const char* const SrfDB_Srf_nId =						"Id";
const char* const SrfDB_Srf_bShowsFX =					"ShowsFX";
const char* const SrfDB_Srf_bCanSeeThrough =			"CanSeeThrough";
const char* const SrfDB_Srf_bCanShootThrough =			"CanShootThrough";
const char* const SrfDB_Srf_rCanShootThroughAmmoDisallow = "CanShootThroughAmmoDisallow";
const char* const SrfDB_Srf_nMaxShootThroughPerturb =	"MaxShootThroughPerturb";
const char* const SrfDB_Srf_nMaxShootThroughThickness =	"MaxShootThroughThickness";
const char* const SrfDB_Srf_fBulletRangeDampen =		"BulletRangeDampen";
const char* const SrfDB_Srf_fBulletDamageDampen =		"BulletDamageDampen";
const char* const SrfDB_Srf_rShellBounceSnd =			"ShellBounceSnd";
const char* const SrfDB_Srf_rGrenadeBounceSnd =			"GrenadeBounceSnd";
const char* const SrfDB_Srf_sRtFootPrintFX =			"RtFootPrintFX";
const char* const SrfDB_Srf_sLtFootPrintFX =			"LtFootPrintFX";
const char* const SrfDB_Srf_nFootPrintLifetime =		"FootPrintLifetime";
const char* const SrfDB_Srf_rRtPlayerFootSnd =			"RtPlayerFootSnd";
const char* const SrfDB_Srf_rLtPlayerFootSnd =			"LtPlayerFootSnd";
const char* const SrfDB_Srf_rRt3rdPlayerFootSnd =		"Rt3rdPlayerFootSnd";
const char* const SrfDB_Srf_rLt3rdPlayerFootSnd =		"Lt3rdPlayerFootSnd";
const char* const SrfDB_Srf_rRtDefaultFootSnd =			"RtDefaultFootSnd";
const char* const SrfDB_Srf_rLtDefaultFootSnd =			"LtDefaultFootSnd";
const char* const SrfDB_Srf_rBodyFallSnd =				"BodyFallSnd";
const char* const SrfDB_Srf_rBodyLedgeFallSnd =			"BodyLedgeFallSnd";
const char* const SrfDB_Srf_rActivationSnd =			"ActivationSnd";
const char* const SrfDB_Srf_fDeathNoiseMod =			"DeathNoiseMod";
const char* const SrfDB_Srf_fMoveNoiseMod =				"MoveNoiseMod";
const char* const SrfDB_Srf_fImpactNoiseMod =			"ImpactNoiseMod";
const char* const SrfDB_Srf_fHardness =					"Hardness";
const char* const SrfDB_Srf_rWeaponFX =					"WeaponFX";
const char* const SrfDB_Srf_fOcclusionDampening =		"OcclusionDampening";
const char* const SrfDB_Srf_fOcclusionLFRatio =			"OcclusionLFRatio";
const char* const SrfDB_Srf_rCollisionProperty =		"CollisionProperty";
const char* const SrfDB_Srf_bCanWallStick =				"CanWallStick";
const char* const SrfDB_Srf_LandingSoundList =			"LandingSnd";
const char* const SrfDB_Srf_sLandingSoundBute =			"LandingSound";
const char* const SrfDB_Srf_nLandingSoundRange =		"LandingRange";

const char* const SrfDB_Imp_sMarkFX =					"MarkFX";
const char* const SrfDB_Imp_sNormalFX =					"NormalFX";
const char* const SrfDB_Imp_sOutgoingFX =				"OutgoingFX";
const char* const SrfDB_Imp_sToViewerFX =				"ToViewerFX";
const char* const SrfDB_Imp_fToViewerRadius =			"ToViewerRadius";
const char* const SrfDB_Imp_sToSourceFX =				"ToSourceFX";
const char* const SrfDB_Imp_sOnSourceFX =				"OnSourceFX";
const char* const SrfDB_Imp_sProtrudingFX =				"ProtrudingFX";
const char* const SrfDB_Imp_sUW_NormalFX =				"UW_NormalFX";
const char* const SrfDB_Imp_sUW_OutgoingFX =			"UW_OutgoingFX";
const char* const SrfDB_Imp_sUW_ToViewerFX =			"UW_ToViewerFX";
const char* const SrfDB_Imp_fUW_ToViewerRadius =		"UW_ToViewerRadius";
const char* const SrfDB_Imp_sUW_ToSourceFX =			"UW_ToSourceFX";
const char* const SrfDB_Imp_sUW_OnSourceFX =			"UW_OnSourceFX";
const char* const SrfDB_Imp_sUW_ProtrudingFX =			"UW_ProtrudingFX";

typedef HRECORD	HSURFACE;
typedef HRECORD	HSRF_IMPACT;


class CSurfaceDB;
extern CSurfaceDB* g_pSurfaceDB;

class CSurfaceDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( CSurfaceDB );

public :	// Methods...

	bool	Init( const char *szDatabaseFile = DB_Default_File );
	void	Term();

	HSURFACE GetSurface(SurfaceType eId);
	HSURFACE GetSurface(const char* pName);

	SurfaceType GetSurfaceType(HSURFACE hSurface);

	HSRF_IMPACT GetSurfaceImpactFX(HRECORD hSurface, const char* pszWeaponFXName);

	// Landing sound support
	HATTRIBUTE GetLandingSoundStruct(HRECORD hSurface);
	int32 GetLandingSoundStructRange(HATTRIBUTE hStruct, int32 index);
	HRECORD GetLandingSoundStructSound(HATTRIBUTE hStruct, int32 index);


	//WorldEdit plugin support functions
	uint32		GetNumSurfaces() const;
	HSURFACE	GetSurfaceByIndex(uint32 nIndex);

private	:	// Members...
	HCATEGORY	m_hSurfaceCat;
	HCATEGORY	m_hWeaponFXCat;
	HCATEGORY	m_hImpactCat;

	typedef std::vector<HRECORD, LTAllocator<HRECORD, LT_MEM_TYPE_GAMECODE> > HRecordArray;
	HRecordArray	m_vecSurfaces;

};

////////////////////////////////////////////////////////////////////////////
//
// CFXDBPlugin is used to help facilitate populating the WorldEdit object
// properties that use FXDB
//
////////////////////////////////////////////////////////////////////////////
#ifndef _CLIENTBUILD

#include "iobjectplugin.h"


class CSurfaceDBPlugin : public IObjectPlugin
{
private:

	CSurfaceDBPlugin();
	CSurfaceDBPlugin( const CSurfaceDBPlugin &other );
	CSurfaceDBPlugin& operator=( const CSurfaceDBPlugin &other );
	~CSurfaceDBPlugin();


public:

	NO_INLINE static CSurfaceDBPlugin& Instance() { static CSurfaceDBPlugin sPlugin; return sPlugin; }

	static bool PopulateStringList(char** aszStrings, uint32* pcStrings,
		const uint32 cMaxStrings, const uint32 cMaxStringLength);

};


#endif // _CLIENTBUILD




#endif  // __SURFACEDB_H__

