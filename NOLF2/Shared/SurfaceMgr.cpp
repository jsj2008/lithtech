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
#define SRFMGR_SURFACE_SNOWVELMULTIPLIER		"SnowmobileVelMult"
#define SRFMGR_SURFACE_RTFOOTSND				"RtFootSnd"
#define SRFMGR_SURFACE_LTFOOTSND				"LtFootSnd"
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
#define	SRFMGR_SURFACE_IMPACTFXNAME				"ImpactFXName"
#define	SRFMGR_SURFACE_UWIMPACTFXNAME			"UWImpactFXName"
#define	SRFMGR_SURFACE_EXITFXNAME				"ExitFXName"
#define	SRFMGR_SURFACE_UWEXITFXNAME				"UWExitFXName"
#define SRFMGR_SURFACE_SNOWMOBILEIMPACTFXNAME	"SnowmobileImpactFXName"

// Global pointer to surface mgr...

CSurfaceMgr*    g_pSurfaceMgr = LTNULL;

static char s_aTagName[30];
static char s_aAttName[100];

#ifndef __PSX2
#ifndef _CLIENTBUILD

// Plugin statics

CSurfaceMgr CSurfaceMgrPlugin::sm_SurfaceMgr;

#endif // _CLIENTBUILD
#endif // __PSX2

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

LTBOOL CSurfaceMgr::Init(const char* szAttributeFile)
{
    if (g_pSurfaceMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile)) return LTFALSE;

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
//  ROUTINE:	CSurfaceMgr::GetDefaultSurface
//
//  PURPOSE:	Get a known surface if the GetSurface() methods fail to find one.
//
// ----------------------------------------------------------------------- //

SURFACE* CSurfaceMgr::GetDefaultSurface()
{
	SURFACE** pCur  = LTNULL;

	pCur = m_SurfaceList.GetItem( TLIT_FIRST );
	while( pCur )
	{
		if( *pCur && (*pCur)->eType == ST_UNKNOWN )
		{
			return *pCur;
		}

		pCur = m_SurfaceList.GetItem( TLIT_NEXT );
	}

	// NO Default!!! return NULL
    return LTNULL;
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

	// Couldn't find the surface... Use a default!

	char szError[64] = { 0 };
	sprintf( szError, "Could not find Surface Type: %i", eType );
	GBM_DisplayError( szError );

	return GetDefaultSurface();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceMgr::GetSurface
//
//	PURPOSE:	Get the specified surface
//
// ----------------------------------------------------------------------- //

SURFACE* CSurfaceMgr::GetSurface(const char* pName)
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

    // Couldn't find the surface... Use a default!

	char szError[64] = { 0 };
	sprintf( szError, "Could not find Surface Type: %s", pName );
	GBM_DisplayError( szError );

	return GetDefaultSurface();
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

CScaleFX* CSurfaceMgr::GetScaleFX(int nScaleFXId) const
{
	return g_pFXButeMgr->GetScaleFX(nScaleFXId);
}

CPShowerFX* CSurfaceMgr::GetPShowerFX(int nPShowerFXId) const
{
	return g_pFXButeMgr->GetPShowerFX(nPShowerFXId);
}

CPolyDebrisFX* CSurfaceMgr::GetPolyDebrisFX(int nPolyDebrisFXId) const
{
	return g_pFXButeMgr->GetPolyDebrisFX(nPolyDebrisFXId);
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
	fSnowVelMult			= 1.0f;

	szActivationSnd[0]		= '\0';
	fActivationSndRadius	= 0;

	szGrenadeImpactSnd[0]	= '\0';
	fGrenadeSndRadius		= 0;

	fHardness				= 0.0f;
    bMagnetic               = LTFALSE;

	szImpactFXName[0]		= '\0';
	szUWImpactFXName[0]		= '\0';

	szExitFXName[0]			= '\0';
	szUWExitFXName[0]		= '\0';

	szSnowmobileImpactFXName[0]	= '\0';
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
    fSnowVelMult			= (LTFLOAT) buteMgr.GetDouble(aTagName, SRFMGR_SURFACE_SNOWVELMULTIPLIER);
    bMagnetic               = (LTBOOL) buteMgr.GetInt(aTagName, SRFMGR_SURFACE_MAGNETIC);

	vFootPrintScale			= buteMgr.GetVector(aTagName, SRFMGR_SURFACE_FOOTPRINTSCALE);

	buteMgr.GetString(aTagName, SRFMGR_SURFACE_NAME, szName, ARRAY_LEN(szName));

	buteMgr.GetString(aTagName, SRFMGR_SURFACE_BULLETHOLESPR, szBulletHoleSpr, ARRAY_LEN(szBulletHoleSpr));

	buteMgr.GetString(aTagName, SRFMGR_SURFACE_RTFOOTPRINTSPR, szRtFootPrintSpr, ARRAY_LEN(szRtFootPrintSpr));

	buteMgr.GetString(aTagName, SRFMGR_SURFACE_LTFOOTPRINTSPR, szLtFootPrintSpr, ARRAY_LEN(szLtFootPrintSpr));

    int i;
    for (i=1; i <= SRF_MAX_FOOTSTEP_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_RTFOOTSND, i);
		buteMgr.GetString(aTagName, s_aAttName, szRtFootStepSnds[i-1], ARRAY_LEN(szRtFootStepSnds[i-1]));
	}

	for (i=1; i <= SRF_MAX_FOOTSTEP_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_LTFOOTSND, i);
		buteMgr.GetString(aTagName, s_aAttName, szLtFootStepSnds[i-1], ARRAY_LEN(szLtFootStepSnds[i-1]));
	}

	for (i=1; i <= SRF_MAX_SNOWMOBILE_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_SNOWMOBILESND, i);
		buteMgr.GetString(aTagName, s_aAttName, szSnowmobileSnds[i-1], ARRAY_LEN(szSnowmobileSnds[i-1]));
	}

	buteMgr.GetString(aTagName, SRFMGR_SURFACE_BODYFALLSND, szBodyFallSnd, ARRAY_LEN(szBodyFallSnd));

	buteMgr.GetString(aTagName, SRFMGR_SURFACE_BODYLEDGEFALLSND, szBodyLedgeFallSnd, ARRAY_LEN(szBodyLedgeFallSnd));

	buteMgr.GetString(aTagName, SRFMGR_SURFACE_ACTIVATIONSND, szActivationSnd, ARRAY_LEN(szActivationSnd));

	buteMgr.GetString(aTagName, SRFMGR_SURFACE_GRENADEIMPACTSND, szGrenadeImpactSnd, ARRAY_LEN(szGrenadeImpactSnd));

	for (i=1; i <= SRF_MAX_IMPACT_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_BULLETIMPACTSND, i);
		buteMgr.GetString(aTagName, s_aAttName, szBulletImpactSnds[i-1], ARRAY_LEN(szBulletImpactSnds[i-1]));
	}

	for (i=1; i <= SRF_MAX_IMPACT_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_PROJIMPACTSND, i);
		buteMgr.GetString(aTagName, s_aAttName, szProjectileImpactSnds[i-1], ARRAY_LEN(szProjectileImpactSnds[i-1]));
	}

	for (i=1; i <= SRF_MAX_IMPACT_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_MELEEIMPACTSND, i);
		buteMgr.GetString(aTagName, s_aAttName, szMeleeImpactSnds[i-1], ARRAY_LEN(szMeleeImpactSnds[i-1]));
	}

	for (i=1; i <= SRF_MAX_SHELL_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", SRFMGR_SURFACE_SHELLIMPACTSND, i);
		buteMgr.GetString(aTagName, s_aAttName, szShellImpactSnds[i-1], ARRAY_LEN(szShellImpactSnds[i-1]));

	}


	// Read the name of our FxED created fx for Surface Impacts...

	buteMgr.GetString( aTagName, SRFMGR_SURFACE_IMPACTFXNAME, szImpactFXName, ARRAY_LEN( szImpactFXName ) );

	buteMgr.GetString( aTagName, SRFMGR_SURFACE_UWIMPACTFXNAME, szUWImpactFXName, ARRAY_LEN( szUWImpactFXName ));

	buteMgr.GetString( aTagName, SRFMGR_SURFACE_EXITFXNAME, szExitFXName, ARRAY_LEN( szExitFXName ));

	buteMgr.GetString( aTagName, SRFMGR_SURFACE_UWEXITFXNAME, szUWExitFXName, ARRAY_LEN( szUWExitFXName ));

	buteMgr.GetString( aTagName, SRFMGR_SURFACE_SNOWMOBILEIMPACTFXNAME, szSnowmobileImpactFXName, ARRAY_LEN( szSnowmobileImpactFXName ));

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

#endif
}



#ifndef __PSX2
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
        sm_SurfaceMgr.Init(szFile);
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
#endif // __PSX2