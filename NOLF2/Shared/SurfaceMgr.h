// ----------------------------------------------------------------------- //
//
// MODULE  : SurfaceMgr.h
//
// PURPOSE : Definition of surface mgr
//
// CREATED : 7/06/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SURFACE_MGR_H__
#define __SURFACE_MGR_H__

#include "GameButeMgr.h"
#include "TemplateList.h"
#include "SurfaceDefs.h"

class CSurfaceMgr;
struct CScaleFX;
struct CPShowerFX;
struct CPolyDebrisFX;

extern CSurfaceMgr* g_pSurfaceMgr;

#define SRFMGR_DEFAULT_FILE		"Attributes\\Surface.txt"

#define SRF_MAX_FILE_PATH			64
#define SRF_MAX_NAME_LENGTH			32
#define SRF_MAX_FOOTSTEP_SNDS		2
#define SRF_MAX_SNOWMOBILE_SNDS		2
#define SRF_MAX_IMPACT_SNDS			2
#define SRF_MAX_SHELL_SNDS			1
#define SRF_MAX_IMPACT_SCALEFX		5
#define SRF_MAX_IMPACT_PSHOWERFX	5
#define SRF_MAX_IMPACT_POLYDEBRISFX	5
#define SRF_MAX_EXIT_SCALEFX		5
#define SRF_MAX_EXIT_PSHOWERFX		5
#define SRF_MAX_EXIT_POLYDEBRISFX	5

struct SURFACE
{
	SURFACE();

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
	void	Cache(CSurfaceMgr* pSurfaceMgr);

	char	szName[SRF_MAX_NAME_LENGTH];

	SurfaceType	eType;

    LTBOOL   bCanSeeThrough;

	// Shoot through info...

    LTBOOL   bCanShootThrough;
	int		nMaxShootThroughPerturb;
	int		nMaxShootThroughThickness;

	// Show cold breath when standing on this surface...

    LTBOOL   bShowBreath;

	// Bullet info...

    LTBOOL  bShowsMark;
	char	szBulletHoleSpr[SRF_MAX_FILE_PATH];
	float	fBulletHoleMinScale;
	float	fBulletHoleMaxScale;
	float	fBulletRangeDampen;
	float	fBulletDamageDampen;
	char	szBulletImpactSnds[SRF_MAX_IMPACT_SNDS][SRF_MAX_FILE_PATH];
	char	szProjectileImpactSnds[SRF_MAX_IMPACT_SNDS][SRF_MAX_FILE_PATH];
	char	szMeleeImpactSnds[SRF_MAX_IMPACT_SNDS][SRF_MAX_FILE_PATH];

	// Shell info...

	float	fShellSndRadius;
	char	szShellImpactSnds[SRF_MAX_SHELL_SNDS][SRF_MAX_FILE_PATH];

	// Movement info...

	char	szRtFootPrintSpr[SRF_MAX_FILE_PATH];
	char	szLtFootPrintSpr[SRF_MAX_FILE_PATH];
	char	szRtFootStepSnds[SRF_MAX_FOOTSTEP_SNDS][SRF_MAX_FILE_PATH];
	char	szLtFootStepSnds[SRF_MAX_FOOTSTEP_SNDS][SRF_MAX_FILE_PATH];
	char	szSnowmobileSnds[SRF_MAX_SNOWMOBILE_SNDS][SRF_MAX_FILE_PATH];

	LTFLOAT	fSnowVelMult;

    LTFLOAT  fFootPrintLifetime;
    LTVector vFootPrintScale;

	// Death noise info...

	char	szBodyFallSnd[SRF_MAX_FILE_PATH];
	float	fBodyFallSndRadius;
	char	szBodyLedgeFallSnd[SRF_MAX_FILE_PATH];
	float	fBodyLedgeFallSndRadius;
	float	fDeathNoiseModifier;
	float	fMovementNoiseModifier;
	float	fImpactNoiseModifier;

	// Object Impact info...

	float	fHardness;
    LTBOOL   bMagnetic;

	// Activation info...

	char	szActivationSnd[SRF_MAX_FILE_PATH];
	float	fActivationSndRadius;

	// Grenade impact info...

	char	szGrenadeImpactSnd[SRF_MAX_FILE_PATH];
	float	fGrenadeSndRadius;


	// Name of FxED created FX to use for Surface Effects...

	// Normal Impact FX
	char	szImpactFXName[SRF_MAX_NAME_LENGTH];

	// Underwater Impact FX
	char	szUWImpactFXName[SRF_MAX_NAME_LENGTH];

	// Normal Exit FX
	char	szExitFXName[SRF_MAX_NAME_LENGTH];

	// Underwater Exit FX
	char	szUWExitFXName[SRF_MAX_NAME_LENGTH];
	
	// Snowmobile Impact FX
	char	szSnowmobileImpactFXName[SRF_MAX_NAME_LENGTH];

};

typedef CTList<SURFACE*> SurfaceList;


class CSurfaceMgr : public CGameButeMgr
{
	public :

		CSurfaceMgr();
		~CSurfaceMgr();

		void			CacheAll();
        void            Reload() { Term(); m_buteMgr.Term(); Init(); }

        LTBOOL           Init(const char* szAttributeFile=SRFMGR_DEFAULT_FILE);
		void			Term();

		SURFACE*		GetSurface(SurfaceType eId);
		SURFACE*		GetSurface(const char* pName);

		int				GetNumSurface() const { return m_SurfaceList.GetLength(); }

		// Helper functions...
        CScaleFX*        GetScaleFX(int nScaleFXId) const;
        CPShowerFX*      GetPShowerFX(int nPShowerFXId) const;
        CPolyDebrisFX*   GetPolyDebrisFX(int nPolyDebrisFXId) const;

		SurfaceList*	GetSurfaceList()	{ return &m_SurfaceList; }

	private :

		SURFACE*		GetDefaultSurface();
		SurfaceList		m_SurfaceList;
};


////////////////////////////////////////////////////////////////////////////
//
// CSurfaceMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CSurfaceMgr
//
////////////////////////////////////////////////////////////////////////////
#ifndef __PSX2
#ifndef _CLIENTBUILD

#include "iobjectplugin.h"

class CSurfaceMgrPlugin : public IObjectPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
            uint32* pcStrings,
            const uint32 cMaxStrings,
            const uint32 cMaxStringLength);

        LTBOOL PopulateStringList(char** aszStrings, uint32* pcStrings,
            const uint32 cMaxStrings, const uint32 cMaxStringLength);

	protected :

		static CSurfaceMgr		sm_SurfaceMgr;
};

#endif // _CLIENTBUILD
#endif // __PSX2

#endif // __SURFACE_FUNCTIONS_H__