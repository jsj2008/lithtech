// ----------------------------------------------------------------------- //
//
// MODULE  : SurfaceMgr.cpp
//
// PURPOSE : SurfaceMgr - Implementation
//
// CREATED : 07/07/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SurfaceMgr.h"
#include "CommonUtilities.h"
#include "FXButeMgr.h"

#define SRFMGR_GLOBAL_TAG                       "Global"

#define SRFMGR_SURFACE_TAG						"Surface"

#define SRFMGR_SURFACE_NAME						"Name"
#define SRFMGR_SURFACE_ID						"Id"
#define SRFMGR_SURFACE_SHOWSMARK				"ShowsMark"
#define SRFMGR_SURFACE_CANSEETHROUGH			"CanSeeThrough"
#define SRFMGR_SURFACE_CANSHOOTTHROUGH			"CanShootThrough"
#define SRFMGR_SURFACE_SHOWBREATH				"ShowBreath"
#define SRFMGR_SURFACE_MAXSHOOTTHROUGHPERTURB	"MaxShootThroughPerturb"
#define SRFMGR_SURFACE_MAXSHOOTTHROUGHTHICKNESS	"MaxShootThroughThickness"
#define SRFMGR_SURFACE_BULLETHOLESPR			"BulletHoleSpr"
#define SRFMGR_SURFACE_BULLETHOLEMINSCALE		"BulletHoleMinScale"
#define SRFMGR_SURFACE_BULLETHOLEMAXSCALE		"BulletHoleMaxScale"
#define SRFMGR_SURFACE_BULLETRANGEDAMPEN		"BulletRangeDampen"
#define SRFMGR_SURFACE_BULLETDAMAGEDAMPEN		"BulletDamageDampen"
#define SRFMGR_SURFACE_BULLETIMPACTSND			"BulletImpactSnd"
#define SRFMGR_SURFACE_PROJIMPACTSND			"ProjectileImpactSnd"
#define SRFMGR_SURFACE_MELEEIMPACTSND			"MeleeImpactSnd"
#define SRFMGR_SURFACE_SHELLSNDRADIUS			"ShellSndRadius"
#define SRFMGR_SURFACE_SHELLIMPACTSND			"ShellImpactSnd"
#define SRFMGR_SURFACE_GRENADESNDRADIUS			"GrenadeSndRadius"
#define SRFMGR_SURFACE_GRENADEIMPACTSND			"GrenadeImpactSnd"
#define SRFMGR_SURFACE_RTFOOTPRINTSPR			"RtFootPrintSpr"
#define SRFMGR_SURFACE_LTFOOTPRINTSPR			"LtFootPrintSpr"
#define SRFMGR_SURFACE_FOOTPRINTSCALE			"FootPrintScale"
#define SRFMGR_SURFACE_FOOTPRINTLIFETIME		"FootPrintLifetime"
#define SRFMGR_SURFACE_MOTOVELMULTIPLIER		"MotorcycleVelMult"
#define SRFMGR_SURFACE_SNOWVELMULTIPLIER		"SnowmobileVelMult"
#define SRFMGR_SURFACE_RTFOOTSND				"RtFootSnd"
#define SRFMGR_SURFACE_LTFOOTSND				"LtFootSnd"
#define SRFMGR_SURFACE_MOTORCYCLESND			"MotorcycleSnd"
#define SRFMGR_SURFACE_SNOWMOBILESND			"SnowmobileSnd"
#define SRFMGR_SURFACE_BODYFALLSND				"BodyFallSnd"
#define SRFMGR_SURFACE_BODYFALLSNDRADIUS		"BodyFallSndRadius"
#define SRFMGR_SURFACE_BODYLEDGEFALLSND			"BodyLedgeFallSnd"
#define SRFMGR_SURFACE_BODYLEDGEFALLSNDRADIUS	"BodyLedgeFallSndRadius"
#define SRFMGR_SURFACE_ACTIVATIONSND			"ActivationSnd"
#define SRFMGR_SURFACE_ACTIVATIONSNDRADIUS		"ActivationSndRadius"
#define SRFMGR_SURFACE_DEATHNOISEMOD			"DeathNoiseMod"
#define SRFMGR_SURFACE_MOVENOISEMOD				"MoveNoiseMod"
#define SRFMGR_SURFACE_IMPACTNOISEMOD			"ImpactNoiseMod"
#define SRFMGR_SURFACE_HARDNESS					"Hardness"
#define SRFMGR_SURFACE_MAGNETIC					"Magnetic"
#define SRFMGR_SURFACE_IMPACTSCALENAME			"ImpactScaleName"
#define SRFMGR_SURFACE_IMPACTPSHOWERNAME		"ImpactPShowerName"
#define SRFMGR_SURFACE_IMPACTPOLYDEBRISNAME		"ImpactPolyDebrisName"
#define SRFMGR_SURFACE_UWIMPACTPSHOWERNAME		"UWImpactPShowerName"
#define SRFMGR_SURFACE_EXITSCALENAME			"ExitScaleName"
#define SRFMGR_SURFACE_EXITPSHOWERNAME			"ExitPShowerName"
#define SRFMGR_SURFACE_EXITPOLYDEBRISNAME		"ExitPolyDebrisName"
#define SRFMGR_SURFACE_UWEXITPSHOWERNAME		"UWExitPShowerName"

// Global pointer to surface mgr...

CSurfaceMgr*    g_pSurfaceMgr = LTNULL;

static char s_aTagName[30];
static char s_aAttName[100];

#ifndef _CLIENTBUILD

// Plugin statics

CSurfaceMgr CSurfaceMgrPlugin::sm_SurfaceMgr;

#endif // _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgr::CSurfaceMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CSurfaceMgr::CSurfaceMgr()
{
    m_SurfaceList.Init(LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgr::~CSurfaceMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CSurfaceMgr::~CSurfaceMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CSurfaceMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
    if (g_pSurfaceMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(pInterface, szAttributeFile)) return LTFALSE;

	g_pSurfaceMgr = this;


	// Save all the global info...



	// Read in the properties for each surface...

	int nNum = 0;
	sprintf(s_aTagName, "%s%d", SRFMGR_SURFACE_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		SURFACE* pSurf = debug_new(SURFACE);

		if (pSurf && pSurf->Init(m_buteMgr, s_aTagName))
		{
			m_SurfaceList.AddTail(pSurf);
		}
		else
		{
			debug_delete(pSurf);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", SRFMGR_SURFACE_TAG, nNum);
	}


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgr::GetSurface
//
//	PURPOSE:	Get the specified surface
//
// ----------------------------------------------------------------------- //

SURFACE* CSurfaceMgr::GetSurface(SurfaceType eType)
{
    SURFACE** pCur  = LTNULL;

	pCur = m_SurfaceList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->eType == eType)
		{
			return *pCur;
		}

		pCur = m_SurfaceList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgr::GetSurface
//
//	PURPOSE:	Get the specified surface
//
// ----------------------------------------------------------------------- //

SURFACE* CSurfaceMgr::GetSurface(char* pName)
{
    if (!pName) return LTNULL;

    SURFACE** pCur  = LTNULL;

	pCur = m_SurfaceList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_SurfaceList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CSurfaceMgr::Term()
{
    g_pSurfaceMgr = LTNULL;

	m_SurfaceList.Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgr::CacheAll
//
//	PURPOSE:	Cache all the surface related resources
//
// ----------------------------------------------------------------------- //

void CSurfaceMgr::CacheAll()
{
#ifndef _CLIENTBUILD  // Server-side only

	// Cache all the surfaces...

    SURFACE** pCurSurf  = LTNULL;
	pCurSurf = m_SurfaceList.GetItem(TLIT_FIRST);

	while (pCurSurf)
	{
		if (*pCurSurf)
		{
			(*pCurSurf)->Cache(this);
		}

		pCurSurf = m_SurfaceList.GetItem(TLIT_NEXT);
	}

#endif
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SURFACE::SURFACE
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SURFACE::SURFACE()
{
	eType						= ST_UNKNOWN;
    bShowsMark                  = LTFALSE;
    bCanSeeThrough              = LTFALSE;
    bCanSeeThrough              = LTFALSE;
	nMaxShootThroughPerturb		= 0;
	nMaxShootThroughThickness	= 0;
	szBulletHoleSpr[0]			= '\0';
	fBulletHoleMinScale			= 1.0f;
	fBulletHoleMaxScale			= 1.0f;
	fShellSndRadius				= 0.0f;

	vFootPrintScale.Init(1, 1, 1);

    int i;
    for (i=0; i < SRF_MAX_IMPACT_SNDS; i++)
	{
		szBulletImpactSnds[i][0]	= '\0';
	}

	for (i=0; i < SRF_MAX_IMPACT_SNDS; i++)
	{
		szProjectileImpactSnds[i][0]	= '\0';
	}

	for (i=0; i < SRF_MAX_IMPACT_SNDS; i++)
	{
		szMeleeImpactSnds[i][0]	= '\0';
	}

	for (i=0; i < SRF_MAX_SHELL_SNDS; i++)
	{
		szShellImpactSnds[i][0]	= '\0';
	}

	szRtFootPrintSpr[0]	= '\0';
	szLtFootPrintSpr[0]	= '\0';

	for (i=0; i < SRF_MAX_FOOTSTEP_SNDS; i++)
	{
		szRtFootStepSnds[i][0]	= '\0';
		szLtFootStepSnds[i][0]	= '\0';
	}

	for (i=0; i < SRF_MAX_MOTORCYCLE_SNDS; i++)
	{
		szMotorcycleSnds[i][0]	= '\0';
	}

	for (i=0; i < SRF_MAX_SNOWMOBILE_SNDS; i++)
	{
		szSnowmobileSnds[i][0]	= '\0';
	}

	szName[0]				= '\0';
	szBodyFallSnd[0]		= '\0';
	fBodyFallSndRadius		= 800.0f;
	szBodyLedgeFallSnd[0]	= '\0';
	fBodyLedgeFallSndRadius	= 1200.0f;
	fDeathNoiseModifier		= 1.0f;
	fMovementNoiseModifier	= 1.0f;
	fImpactNoiseModifier	= 1.0f;
	fFootPrintLifetime		= 1.0f;
	fMotoVelMult			= 1.0f;
	fSnowVelMult			= 1.0f;

	szActivationSnd[0]		= '\0';
	fActivationSndRadius	= 0;

	szGrenadeImpactSnd[0]	= '\0';
	fGrenadeSndRadius		= 0;

	fHardness				= 0.0f;
    bMagnetic               = LTFALSE;

	nNumImpactScaleFX			= 0;
	aImpactScaleFXIds[0]		= -1;

	nNumImpactPShowerFX			= 0;
	aImpactPShowerFXIds[0]		= -1;

	nNumImpactPolyDebrisFX		= 0;
	aImpactPolyDebrisFXIds[0]	= -1;

	nNumUWImpactPShowerFX		= 0;
	aUWImpactPShowerFXIds[0]	= -1;


	nNumExitScaleFX			= 0;
	aExitScaleFXIds[0]		= -1;

	nNumExitPShowerFX		= 0;
	aExitPShowerFXIds[0]	= -1;

	nNumExitPolyDebrisFX	= 0;
	aExitPolyDebrisFXIds[0]	= -1;

	nNumUWExitPShowerFX		= 0;
	aUWExitPShowerFXIds[0]	= -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SURFACE::Init
//
//	PURPOSE:	Build the surface struct
//
// ----------------------------------------------------------------------- //

LTBOOL SURFACE::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	eType					= (SurfaceType) buteMgr.GetInt(aTagName, SRFMGR_SURFACE_ID);
    bShowsMark              = (LTBOOL) buteMgr.GetInt(aTagName, SRFMGR_SURFACE_SHOWSMARK);
    bCanSeeThrough          = (LTBOOL) buteMgr.GetInt(aTagName, SRFMGR_SURFACE_CANSEETHROUGH);
    bCanShootThrough        = (LTBOOL) buteMgr.GetInt(aTagName, SRFMGR_SURFACE_CANSHOOTTHROUGH);
    bShowBreath             = (LTBOOL) buteMgr.GetInt(aTagName, SRFMGR_SURFACE_SHOWBREATH);
	nMaxShootThroughPerturb	= buteMgr.GetInt(aTagName, SRFMGR_SURFACE_MAXSHOOTTHROUGHPERTURB);
	nMaxShootThroughThickness= buteMgr.GetInt(aTagName, SRFMGR_SURFACE_MAXSHOOTTHROUGHTHICKNESS);
    fDeathNoiseModifier     = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_DEATHNOISEMOD);
    fMovementNoiseModifier  = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_MOVENOISEMOD);
    fImpactNoiseModifier    = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_IMPACTNOISEMOD);
    fBodyFallSndRadius      = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_BODYFALLSNDRADIUS);
    fBodyLedgeFallSndRadius = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_BODYLEDGEFALLSNDRADIUS);
    fBulletHoleMinScale     = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_BULLETHOLEMINSCALE);
    fBulletHoleMaxScale     = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_BULLETHOLEMAXSCALE);
    fBulletRangeDampen      = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_BULLETRANGEDAMPEN);
    fBulletDamageDampen     = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_BULLETDAMAGEDAMPEN);
    fActivationSndRadius    = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_ACTIVATIONSNDRADIUS);
    fShellSndRadius         = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_SHELLSNDRADIUS);
    fGrenadeSndRadius       = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_GRENADESNDRADIUS);
    fHardness               = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_HARDNESS);
    fFootPrintLifetime      = (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_FOOTPRINTLIFETIME);
    fMotoVelMult			= (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_MOTOVELMULTIPLIER);
    fSnowVelMult			= (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_SNOWVELMULTIPLIER);
    bMagnetic               = (LTBOOL) buteMgr.GetInt(aTagName, SRFMGR_SURFACE_MAGNETIC);

	vFootPrintScale			= buteMgr.GetVector(aTagName, SRFMGR_SURFACE_FOOTPRINTSCALE);

	CString str = buteMgr.GetString(aTagName, SRFMGR_SURFACE_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, SRFMGR_SURFACE_BULLETHOLESPR);
	if (!str.IsEmpty())
	{
		strncpy(szBulletHoleSpr, (char*)(LPCSTR)str, ARRAY_LEN(szBulletHoleSpr));
	}

	str = buteMgr.GetString(aTagName, SRFMGR_SURFACE_RTFOOTPRINTSPR);
	if (!str.IsEmpty())
	{
		strncpy(szRtFootPrintSpr, (char*)(LPCSTR)str, ARRAY_LEN(szRtFootPrintSpr));
	}

	str = buteMgr.GetString(aTagName, SRFMGR_SURFACE_LTFOOTPRINTSPR);
	if (!str.IsEmpty())
	{
		strncpy(szLtFootPrintSpr, (char*)(LPCSTR)str, ARRAY_LEN(szLtFootPrintSpr));
	}

    int i;
    for (i=1; i <= SRF_MAX_FOOTSTEP_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_RTFOOTSND, i);
		str = buteMgr.GetString(aTagName, s_aAttName);

		if (!str.IsEmpty())
		{
			strncpy(szRtFootStepSnds[i-1], (char*)(LPCSTR)str, ARRAY_LEN(szRtFootStepSnds[i-1]));
		}
	}

	for (i=1; i <= SRF_MAX_FOOTSTEP_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_LTFOOTSND, i);
		str = buteMgr.GetString(aTagName, s_aAttName);

		if (!str.IsEmpty())
		{
			strncpy(szLtFootStepSnds[i-1], (char*)(LPCSTR)str, ARRAY_LEN(szLtFootStepSnds[i-1]));
		}
	}

	for (i=1; i <= SRF_MAX_MOTORCYCLE_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_MOTORCYCLESND, i);
		str = buteMgr.GetString(aTagName, s_aAttName);

		if (!str.IsEmpty())
		{
			strncpy(szMotorcycleSnds[i-1], (char*)(LPCSTR)str, ARRAY_LEN(szMotorcycleSnds[i-1]));
		}
	}

	for (i=1; i <= SRF_MAX_SNOWMOBILE_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_SNOWMOBILESND, i);
		str = buteMgr.GetString(aTagName, s_aAttName);

		if (!str.IsEmpty())
		{
			strncpy(szSnowmobileSnds[i-1], (char*)(LPCSTR)str, ARRAY_LEN(szSnowmobileSnds[i-1]));
		}
	}

	str = buteMgr.GetString(aTagName, SRFMGR_SURFACE_BODYFALLSND);
	if (!str.IsEmpty())
	{
		strncpy(szBodyFallSnd, (char*)(LPCSTR)str, ARRAY_LEN(szBodyFallSnd));
	}

	str = buteMgr.GetString(aTagName, SRFMGR_SURFACE_BODYLEDGEFALLSND);
	if (!str.IsEmpty())
	{
		strncpy(szBodyLedgeFallSnd, (char*)(LPCSTR)str, ARRAY_LEN(szBodyLedgeFallSnd));
	}

	str = buteMgr.GetString(aTagName, SRFMGR_SURFACE_ACTIVATIONSND);
	if (!str.IsEmpty())
	{
		strncpy(szActivationSnd, (char*)(LPCSTR)str, ARRAY_LEN(szActivationSnd));
	}

	str = buteMgr.GetString(aTagName, SRFMGR_SURFACE_GRENADEIMPACTSND);
	if (!str.IsEmpty())
	{
		strncpy(szGrenadeImpactSnd, (char*)(LPCSTR)str, ARRAY_LEN(szGrenadeImpactSnd));
	}

	for (i=1; i <= SRF_MAX_IMPACT_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_BULLETIMPACTSND, i);
		str = buteMgr.GetString(aTagName, s_aAttName);

		if (!str.IsEmpty())
		{
			strncpy(szBulletImpactSnds[i-1], (char*)(LPCSTR)str, ARRAY_LEN(szBulletImpactSnds[i-1]));
		}
	}

	for (i=1; i <= SRF_MAX_IMPACT_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_PROJIMPACTSND, i);
		str = buteMgr.GetString(aTagName, s_aAttName);

		if (!str.IsEmpty())
		{
			strncpy(szProjectileImpactSnds[i-1], (char*)(LPCSTR)str, ARRAY_LEN(szProjectileImpactSnds[i-1]));
		}
	}

	for (i=1; i <= SRF_MAX_IMPACT_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_MELEEIMPACTSND, i);
		str = buteMgr.GetString(aTagName, s_aAttName);

		if (!str.IsEmpty())
		{
			strncpy(szMeleeImpactSnds[i-1], (char*)(LPCSTR)str, ARRAY_LEN(szMeleeImpactSnds[i-1]));
		}
	}

	for (i=1; i <= SRF_MAX_SHELL_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_SHELLIMPACTSND, i);
		str = buteMgr.GetString(aTagName, s_aAttName);

		if (!str.IsEmpty())
		{
			strncpy(szShellImpactSnds[i-1], (char*)(LPCSTR)str, ARRAY_LEN(szShellImpactSnds[i-1]));
		}
	}

	// Impact specific scale fx...

	// Build our impact scale fx id list...

	nNumImpactScaleFX = 0;
	sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_IMPACTSCALENAME, nNumImpactScaleFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumImpactScaleFX < SRF_MAX_IMPACT_SCALEFX)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX((char*)(LPCSTR)str);
			if (pScaleFX)
			{
				aImpactScaleFXIds[nNumImpactScaleFX] = pScaleFX->nId;
			}
		}

		nNumImpactScaleFX++;
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_IMPACTSCALENAME, nNumImpactScaleFX);
	}

	// Build our impact particle shower fx id list...

	nNumImpactPShowerFX = 0;
	sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_IMPACTPSHOWERNAME, nNumImpactPShowerFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumImpactPShowerFX < SRF_MAX_IMPACT_PSHOWERFX)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			CPShowerFX* pPShowerFX = g_pFXButeMgr->GetPShowerFX((char*)(LPCSTR)str);
			if (pPShowerFX)
			{
				aImpactPShowerFXIds[nNumImpactPShowerFX] = pPShowerFX->nId;
			}
		}

		nNumImpactPShowerFX++;
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_IMPACTPSHOWERNAME, nNumImpactPShowerFX);
	}

	// Build our impact poly debris fx id list...

	nNumImpactPolyDebrisFX = 0;
	sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_IMPACTPOLYDEBRISNAME, nNumImpactPolyDebrisFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumImpactPolyDebrisFX < SRF_MAX_IMPACT_PSHOWERFX)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			CPolyDebrisFX* pPolyDebrisFX = g_pFXButeMgr->GetPolyDebrisFX((char*)(LPCSTR)str);
			if (pPolyDebrisFX)
			{
				aImpactPolyDebrisFXIds[nNumImpactPolyDebrisFX] = pPolyDebrisFX->nId;
			}
		}

		nNumImpactPolyDebrisFX++;
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_IMPACTPOLYDEBRISNAME, nNumImpactPolyDebrisFX);
	}

	// Build our under water impact particle shower fx id list...


	nNumUWImpactPShowerFX = 0;
	sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_UWIMPACTPSHOWERNAME, nNumUWImpactPShowerFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumUWImpactPShowerFX < SRF_MAX_IMPACT_PSHOWERFX)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			CPShowerFX* pPShowerFX = g_pFXButeMgr->GetPShowerFX((char*)(LPCSTR)str);
			if (pPShowerFX)
			{
				aUWImpactPShowerFXIds[nNumUWImpactPShowerFX] = pPShowerFX->nId;
			}
		}

		nNumUWImpactPShowerFX++;
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_UWIMPACTPSHOWERNAME, nNumUWImpactPShowerFX);
	}



	// Exit specific scale fx...

	// Build our exit scale fx id list...

	nNumExitScaleFX = 0;
	sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_EXITSCALENAME, nNumExitScaleFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumExitScaleFX < SRF_MAX_IMPACT_SCALEFX)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX((char*)(LPCSTR)str);
			if (pScaleFX)
			{
				aExitScaleFXIds[nNumExitScaleFX]  = pScaleFX->nId;
			}
		}

		nNumExitScaleFX++;
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_EXITSCALENAME, nNumExitScaleFX);
	}

	// Build our exit particle shower fx id list...

	nNumExitPShowerFX = 0;
	sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_EXITPSHOWERNAME, nNumExitPShowerFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumExitPShowerFX < SRF_MAX_IMPACT_PSHOWERFX)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			CPShowerFX* pPShowerFX = g_pFXButeMgr->GetPShowerFX((char*)(LPCSTR)str);
			if (pPShowerFX)
			{
				aExitPShowerFXIds[nNumExitPShowerFX] = pPShowerFX->nId;
			}
		}

		nNumExitPShowerFX++;
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_EXITPSHOWERNAME, nNumExitPShowerFX);
	}

	// Build our exit poly debris fx id list...

	nNumExitPolyDebrisFX = 0;
	sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_EXITPOLYDEBRISNAME, nNumExitPolyDebrisFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumExitPolyDebrisFX < SRF_MAX_IMPACT_PSHOWERFX)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			CPolyDebrisFX* pPolyDebrisFX = g_pFXButeMgr->GetPolyDebrisFX((char*)(LPCSTR)str);
			if (pPolyDebrisFX)
			{
				aExitPolyDebrisFXIds[nNumExitPolyDebrisFX] = pPolyDebrisFX->nId;
			}
		}

		nNumExitPolyDebrisFX++;
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_EXITPOLYDEBRISNAME, nNumExitPolyDebrisFX);
	}

	// Build our under water exit particle shower fx id list...

	nNumUWExitPShowerFX = 0;
	sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_UWIMPACTPSHOWERNAME, nNumUWExitPShowerFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumUWExitPShowerFX < SRF_MAX_IMPACT_PSHOWERFX)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			CPShowerFX* pPShowerFX = g_pFXButeMgr->GetPShowerFX((char*)(LPCSTR)str);
			if (pPShowerFX)
			{
				aUWExitPShowerFXIds[nNumUWExitPShowerFX] = pPShowerFX->nId;
			}
		}

		nNumUWExitPShowerFX++;
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_UWIMPACTPSHOWERNAME, nNumUWExitPShowerFX);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SURFACE::Cache
//
//	PURPOSE:	Cache all the resources associated with the surface
//
// ----------------------------------------------------------------------- //

void SURFACE::Cache(CSurfaceMgr* pSurfaceMgr)
{
#ifndef _CLIENTBUILD

	if (!pSurfaceMgr) return;

	// Cache sprites...

	if (szBulletHoleSpr[0])
	{
        g_pLTServer->CacheFile(FT_SPRITE, szBulletHoleSpr);
	}

	if (szRtFootPrintSpr[0])
	{
        g_pLTServer->CacheFile(FT_SPRITE, szRtFootPrintSpr);
	}

	if (szLtFootPrintSpr[0])
	{
        g_pLTServer->CacheFile(FT_SPRITE, szLtFootPrintSpr);
	}


	// Cache sounds...

    int i;
    for (i=0; i < SRF_MAX_IMPACT_SNDS; i++)
	{
		if (szBulletImpactSnds[i][0])
		{
            g_pLTServer->CacheFile(FT_SOUND, szBulletImpactSnds[i]);
		}
	}

	for (i=0; i < SRF_MAX_IMPACT_SNDS; i++)
	{
		if (szMeleeImpactSnds[i][0])
		{
            g_pLTServer->CacheFile(FT_SOUND, szMeleeImpactSnds[i]);
		}
	}

	for (i=0; i < SRF_MAX_IMPACT_SNDS; i++)
	{
		if (szProjectileImpactSnds[i][0])
		{
            g_pLTServer->CacheFile(FT_SOUND, szProjectileImpactSnds[i]);
		}
	}

	for (i=0; i < SRF_MAX_SHELL_SNDS; i++)
	{
		if (szShellImpactSnds[i][0])
		{
            g_pLTServer->CacheFile(FT_SOUND, szShellImpactSnds[i]);
		}
	}

	for (i=0; i < SRF_MAX_FOOTSTEP_SNDS; i++)
	{
		if (szRtFootStepSnds[i][0])
		{
            g_pLTServer->CacheFile(FT_SOUND, szRtFootStepSnds[i]);
		}

		if (szLtFootStepSnds[i][0])
		{
            g_pLTServer->CacheFile(FT_SOUND, szLtFootStepSnds[i]);
		}
	}

	for (i=0; i < SRF_MAX_MOTORCYCLE_SNDS; i++)
	{
		if (szMotorcycleSnds[i][0])
		{
            g_pLTServer->CacheFile(FT_SOUND, szMotorcycleSnds[i]);
		}
	}

	for (i=0; i < SRF_MAX_SNOWMOBILE_SNDS; i++)
	{
		if (szSnowmobileSnds[i][0])
		{
            g_pLTServer->CacheFile(FT_SOUND, szSnowmobileSnds[i]);
		}
	}

	if (szBodyFallSnd[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szBodyFallSnd);
	}

	if (szBodyLedgeFallSnd[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szBodyLedgeFallSnd);
	}

	if (szActivationSnd[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szActivationSnd);
	}

	if (szGrenadeImpactSnd[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szGrenadeImpactSnd);
	}

	for (i=0; i < nNumImpactScaleFX; i++)
	{
		CScaleFX* pScaleFX = pSurfaceMgr->GetScaleFX(aImpactScaleFXIds[i]);
		if (pScaleFX)
		{
			pScaleFX->Cache();
		}
	}

	for (i=0; i < nNumImpactPShowerFX; i++)
	{
		CPShowerFX* pPShowerFX = pSurfaceMgr->GetPShowerFX(aImpactPShowerFXIds[i]);
		if (pPShowerFX)
		{
			pPShowerFX->Cache();
		}
	}

	for (i=0; i < nNumImpactPolyDebrisFX; i++)
	{
		CPolyDebrisFX* pPolyDebrisFX = pSurfaceMgr->GetPolyDebrisFX(aImpactPolyDebrisFXIds[i]);
		if (pPolyDebrisFX)
		{
			pPolyDebrisFX->Cache();
		}
	}

	for (i=0; i < nNumUWImpactPShowerFX; i++)
	{
		CPShowerFX* pPShowerFX = pSurfaceMgr->GetPShowerFX(aUWImpactPShowerFXIds[i]);
		if (pPShowerFX)
		{
			pPShowerFX->Cache();
		}
	}
#endif
}



#ifndef _CLIENTBUILD

////////////////////////////////////////////////////////////////////////////
//
// CSurfaceMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CSurfaceMgr
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT CSurfaceMgrPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (!g_pSurfaceMgr)
	{
		// This will set the g_pSurfaceMgr...Since this could also be
		// set elsewhere, just check for the global bute mgr...

		char szFile[256];
		sprintf(szFile, "%s\\%s", szRezPath, SRFMGR_DEFAULT_FILE);
        sm_SurfaceMgr.SetInRezFile(LTFALSE);
        sm_SurfaceMgr.Init(g_pLTServer, szFile);
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgrPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

LTBOOL CSurfaceMgrPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	_ASSERT(aszStrings && pcStrings && g_pSurfaceMgr);
    if (!aszStrings || !pcStrings || !g_pSurfaceMgr) return LTFALSE;

	// Add an entry for each surface

	SurfaceList* pList = g_pSurfaceMgr->GetSurfaceList();
    if (!pList || pList->GetLength() < 1) return LTFALSE;

	// Cache all the surfaces...

    SURFACE** pCurSurf  = LTNULL;
	pCurSurf = pList->GetItem(TLIT_FIRST);

	while (pCurSurf)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		if (*pCurSurf && (*pCurSurf)->szName[0])
		{
            uint32 dwSurfaceNameLen = strlen((*pCurSurf)->szName);

			if (dwSurfaceNameLen < cMaxStringLength && ((*pcStrings) + 1) < cMaxStrings)
			{
				strcpy(aszStrings[(*pcStrings)++], (*pCurSurf)->szName);
			}
		}

		pCurSurf = pList->GetItem(TLIT_NEXT);
	}

    return LTTRUE;
}

#endif // #ifndef _CLIENTBUILD
