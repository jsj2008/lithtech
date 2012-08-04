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
#include "FXButeMgr.h"
#include "SurfaceDefs.h"

class CSurfaceMgr;
extern CSurfaceMgr* g_pSurfaceMgr;

#define SRFMGR_DEFAULT_FILE		"Attributes\\Surface.txt"

#define SRF_MAX_FILE_PATH			64
#define SRF_MAX_NAME_LENGTH			32
#define SRF_MAX_FOOTSTEP_SNDS		2
#define SRF_MAX_MOTORCYCLE_SNDS		2
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
	char	szMotorcycleSnds[SRF_MAX_MOTORCYCLE_SNDS][SRF_MAX_FILE_PATH];
	char	szSnowmobileSnds[SRF_MAX_SNOWMOBILE_SNDS][SRF_MAX_FILE_PATH];

	LTFLOAT	fMotoVelMult;
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

	// Normal impact members...

	// Impact scale fx
	int		nNumImpactScaleFX;
	int		aImpactScaleFXIds[SRF_MAX_IMPACT_SCALEFX];

	// Impact particle shower fx
	int		nNumImpactPShowerFX;
	int		aImpactPShowerFXIds[SRF_MAX_IMPACT_PSHOWERFX];

	// Impact poly debris fx
	int		nNumImpactPolyDebrisFX;
	int		aImpactPolyDebrisFXIds[SRF_MAX_IMPACT_POLYDEBRISFX];


	// Under water impact members...

	// Under water Impact particle shower fx
	int		nNumUWImpactPShowerFX;
	int		aUWImpactPShowerFXIds[SRF_MAX_IMPACT_PSHOWERFX];


	// Normal exit members...

	// Exit scale fx
	int		nNumExitScaleFX;
	int		aExitScaleFXIds[SRF_MAX_EXIT_SCALEFX];

	// Exit particle shower fx
	int		nNumExitPShowerFX;
	int		aExitPShowerFXIds[SRF_MAX_EXIT_PSHOWERFX];

	// Exit poly debris fx
	int		nNumExitPolyDebrisFX;
	int		aExitPolyDebrisFXIds[SRF_MAX_EXIT_POLYDEBRISFX];


	// Under water exit members...

	// Under water Exit particle shower fx
	int		nNumUWExitPShowerFX;
	int		aUWExitPShowerFXIds[SRF_MAX_EXIT_PSHOWERFX];
};

typedef CTList<SURFACE*> SurfaceList;


class CSurfaceMgr : public CGameButeMgr
{
	public :

		CSurfaceMgr();
		~CSurfaceMgr();

		void			CacheAll();
        void            Reload(ILTCSBase *pInterface) { Term(); m_buteMgr.Term(); Init(pInterface); }

        LTBOOL           Init(ILTCSBase *pInterface, const char* szAttributeFile=SRFMGR_DEFAULT_FILE);
		void			Term();

		SURFACE*		GetSurface(SurfaceType eId);
		SURFACE*		GetSurface(char* pName);

		int				GetNumSurface() const { return m_SurfaceList.GetLength(); }

		// Helper functions...
        CScaleFX*        GetScaleFX(int nScaleFXId) const { return g_pFXButeMgr->GetScaleFX(nScaleFXId); }
        CPShowerFX*      GetPShowerFX(int nPShowerFXId) const { return g_pFXButeMgr->GetPShowerFX(nPShowerFXId); }
        CPolyDebrisFX*   GetPolyDebrisFX(int nPolyDebrisFXId) const { return g_pFXButeMgr->GetPolyDebrisFX(nPolyDebrisFXId); }

		SurfaceList*	GetSurfaceList()	{ return &m_SurfaceList; }

	private :

		SurfaceList		m_SurfaceList;
};

////////////////////////////////////////////////////////////////////////////
//
// CSurfaceMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CSurfaceMgr
//
////////////////////////////////////////////////////////////////////////////
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

#endif // __SURFACE_FUNCTIONS_H__