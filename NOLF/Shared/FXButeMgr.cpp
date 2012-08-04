// ----------------------------------------------------------------------- //
//
// MODULE  : FXButeMgr.cpp
//
// PURPOSE : FXButeMgr implementation - Controls attributes of special fx
//
// CREATED : 12/08/98
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "FXButeMgr.h"
#include "WeaponFXTypes.h"
#include "CommonUtilities.h"
#include "SurfaceFunctions.h"

#ifdef _CLIENTBUILD
#include "..\ClientShellDLL\ParticleShowerFX.h"
#include "..\ClientShellDLL\PolyDebrisFX.h"
#include "..\ClientShellDLL\ParticleExplosionFX.h"
#include "..\ClientShellDLL\GameClientShell.h"
#include "..\ClientShellDLL\InterfaceMgr.h"
#include "..\ClientShellDLL\BeamFX.h"

extern CGameClientShell* g_pGameClientShell;
#endif

#define FXBMGR_IMPACTFX_TAG				"ImpactFX"
#define FXBMGR_IMPACTFX_NAME			"Name"
#define FXBMGR_IMPACTFX_SOUND			"Sound"
#define FXBMGR_IMPACTFX_SOUNDRADIUS		"SoundRadius"
#define FXBMGR_IMPACTFX_AISOUNDRADIUS	"AISoundRadius"
#define FXBMGR_IMPACTFX_AIIGNORESURFACE	"AIIgnoreSurface"
#define FXBMGR_IMPACTFX_CREATEMARK		"CreateMark"
#define FXBMGR_IMPACTFX_CREATESMOKE		"CreateSmoke"
#define FXBMGR_IMPACTFX_IMPACTONSKY		"ImpactOnSky"
#define FXBMGR_IMPACTFX_IGNOREFLESH		"IgnoreFlesh"
#define FXBMGR_IMPACTFX_IGNORELIQUID	"IgnoreLiquid"
#define FXBMGR_IMPACTFX_DOSURFACEFX		"DoSurfaceFX"
#define FXBMGR_IMPACTFX_SCREENTINT		"ScreenTint"
#define FXBMGR_IMPACTFX_BLASTMARK		"BlastMark"
#define FXBMGR_IMPACTFX_PUSHERNAME		"PusherName"
#define FXBMGR_IMPACTFX_MARK			"Mark"
#define FXBMGR_IMPACTFX_MARKSCALE		"MarkScale"
#define FXBMGR_IMPACTFX_TINTCOLOR		"TintColor"
#define FXBMGR_IMPACTFX_TINTRAMPUP		"TintRampUp"
#define FXBMGR_IMPACTFX_TINTRAMPDOWN	"TintRampDown"
#define FXBMGR_IMPACTFX_TINTMAXTIME		"TintMaxTime"
#define FXBMGR_IMPACTFX_BLASTCOLOR		"BlastColor"
#define FXBMGR_IMPACTFX_BLASTTIMEMIN	"BlastTimeMin"
#define FXBMGR_IMPACTFX_BLASTTIMEMAX	"BlastTimeMax"
#define FXBMGR_IMPACTFX_BLASTFADEMIN	"BlastFadeMin"
#define FXBMGR_IMPACTFX_BLASTFADEMAX	"BlastFadeMax"
#define FXBMGR_IMPACTFX_DEBRISNAME		"DebrisName"
#define FXBMGR_IMPACTFX_SCALENAME		"ScaleName"
#define FXBMGR_IMPACTFX_PEXPLNAME		"PExplName"
#define FXBMGR_IMPACTFX_DLIGHTNAME		"DLightName"
#define FXBMGR_IMPACTFX_PDEBRISNAME		"PolyDebrisName"
#define FXBMGR_IMPACTFX_PSHOWERNAME		"PShowerName"

#define FXBMGR_FIREFX_TAG				"FireFX"
#define FXBMGR_FIREFX_NAME				"Name"
#define FXBMGR_FIREFX_MUZZLESMOKE		"MuzzleSmoke"
#define FXBMGR_FIREFX_EJECTSHELLS		"EjectShells"
#define FXBMGR_FIREFX_MUZZLELIGHT		"MuzzleLight"
#define FXBMGR_FIREFX_FIRESOUND			"FireSound"
#define FXBMGR_FIREFX_EXITMARK			"ExitMark"
#define FXBMGR_FIREFX_EXITDEBRIS		"ExitDebris"
#define FXBMGR_FIREFX_SHELLMODEL		"ShellModel"
#define FXBMGR_FIREFX_SHELLSKIN			"ShellSkin"
#define FXBMGR_FIREFX_SHELLSCALE		"ShellScale"
#define FXBMGR_FIREFX_BEAMFXNAME		"BeamName"

#define FXBMGR_PROJECTILEFX_TAG			"ProjectileFX"
#define FXBMGR_PROJECTILEFX_NAME		"Name"
#define FXBMGR_PROJECTILEFX_SMOKETRAIL	"SmokeTrail"
#define FXBMGR_PROJECTILEFX_FLARE		"Flare"
#define FXBMGR_PROJECTILEFX_LIGHT		"Light"
#define FXBMGR_PROJECTILEFX_FLYSOUND	"FlySound"
#define FXBMGR_PROJECTILEFX_CLASS		"Class"
#define FXBMGR_PROJECTILEFX_CLASSDATA	"ClassData"
#define FXBMGR_PROJECTILEFX_MODEL		"Model"
#define FXBMGR_PROJECTILEFX_MODELSCALE	"ModelScale"
#define FXBMGR_PROJECTILEFX_SKIN		"Skin"
#define FXBMGR_PROJECTILEFX_SOUND		"Sound"
#define FXBMGR_PROJECTILEFX_SOUNDRADIUS	"SoundRadius"
#define FXBMGR_PROJECTILEFX_FLARESPRITE	"FlareSprite"
#define FXBMGR_PROJECTILEFX_FLARESCALE	"FlareScale"
#define FXBMGR_PROJECTILEFX_LIGHTCOLOR	"LightColor"
#define FXBMGR_PROJECTILEFX_LIGHTRADIUS	"LightRadius"
#define FXBMGR_PROJECTILEFX_GRAVITY		"Gravity"
#define FXBMGR_PROJECTILEFX_LIFETIME	"Lifetime"
#define FXBMGR_PROJECTILEFX_VELOCITY	"Velocity"
#define FXBMGR_PROJECTILEFX_ALTVELOCITY	"AltVelocity"
#define FXBMGR_PROJECTILEFX_SMOKETRAILTYPE	"SmokeTrailType"

#define FXBMGR_PROXCLASS_TAG			"ProxClassData"
#define FXBMGR_PROXCLASS_ACTRADIUS		"ActivateRadius"
#define FXBMGR_PROXCLASS_ARMDELAY		"ArmDelay"
#define FXBMGR_PROXCLASS_ARMSND			"ArmSound"
#define FXBMGR_PROXCLASS_ARMSNDRADIUS	"ArmSndRadius"
#define FXBMGR_PROXCLASS_ACTDELAY		"ActivateDelay"
#define FXBMGR_PROXCLASS_ACTSND			"ActivateSound"
#define FXBMGR_PROXCLASS_ACTSNDRADIUS	"ActivateSndRadius"

#define FXBMGR_SCALEFX_TAG				"ScaleFX"
#define FXBMGR_PSHOWERFX_TAG			"PShowerFX"
#define FXBMGR_POLYDEBRISFX_TAG			"PolyDebrisFX"

#define FXBMGR_PEXPLFX_TAG				"PExplFX"
#define	FXBMGR_PEXPLFX_NAME				"Name"
#define	FXBMGR_PEXPLFX_FILE				"File"
#define	FXBMGR_PEXPLFX_POSOFFSET		"PosOffset"
#define	FXBMGR_PEXPLFX_NUMPERPUFF		"NumPerPuff"
#define	FXBMGR_PEXPLFX_NUMEMITTERS		"NumEmitters"
#define	FXBMGR_PEXPLFX_NUMSTEPS			"NumSteps"
#define	FXBMGR_PEXPLFX_CREATEDEBRIS		"CreateDebris"
#define	FXBMGR_PEXPLFX_ROTATEDEBRIS		"RotateDebris"
#define	FXBMGR_PEXPLFX_IGNOREWIND		"IgnoreWind"
#define	FXBMGR_PEXPLFX_DOBUBBLES		"DoBubbles"
#define	FXBMGR_PEXPLFX_COLOR1			"Color1"
#define	FXBMGR_PEXPLFX_COLOR2			"Color2"
#define	FXBMGR_PEXPLFX_MINVEL			"MinVel"
#define	FXBMGR_PEXPLFX_MAXVEL			"MaxVel"
#define	FXBMGR_PEXPLFX_MINDRIFTVEL		"MinDriftVel"
#define	FXBMGR_PEXPLFX_MAXDRIFTVEL		"MaxDriftVel"
#define	FXBMGR_PEXPLFX_LIFETIME			"LifeTime"
#define	FXBMGR_PEXPLFX_FADETIME			"FadeTime"
#define	FXBMGR_PEXPLFX_OFFSETTIME		"OffsetTime"
#define	FXBMGR_PEXPLFX_RADIUS			"Radius"
#define	FXBMGR_PEXPLFX_GRAVITY			"Gravity"
#define	FXBMGR_PEXPLFX_ADDITIVE			"Additive"
#define	FXBMGR_PEXPLFX_MULTIPLY			"Multiply"

#define FXBMGR_DLIGHTFX_TAG				"DLightFX"
#define	FXBMGR_DLIGHTFX_NAME			"Name"
#define	FXBMGR_DLIGHTFX_COLOR			"Color"
#define	FXBMGR_DLIGHTFX_MINRADIUS		"MinRadius"
#define	FXBMGR_DLIGHTFX_MAXRADIUS		"MaxRadius"
#define	FXBMGR_DLIGHTFX_MINTIME			"MinTime"
#define	FXBMGR_DLIGHTFX_MAXTIME			"MaxTime"
#define	FXBMGR_DLIGHTFX_RAMPUPTIME		"RampUpTime"
#define	FXBMGR_DLIGHTFX_RAMPDOWNTIME	"RampDownTime"

#define FXBMGR_SOUNDFX_TAG				"SoundFX"
#define	FXBMGR_SOUNDFX_NAME				"Name"
#define	FXBMGR_SOUNDFX_FILE				"File"
#define	FXBMGR_SOUNDFX_LOOP				"Loop"
#define	FXBMGR_SOUNDFX_RADIUS			"Radius"
#define	FXBMGR_SOUNDFX_PITCHSHIFT		"PitchShift"

#define FXBMGR_PUSHERFX_TAG				"PusherFX"
#define	FXBMGR_PUSHERFX_NAME			"Name"
#define	FXBMGR_PUSHERFX_RADIUS			"Radius"
#define	FXBMGR_PUSHERFX_STARTDELAY		"StartDelay"
#define	FXBMGR_PUSHERFX_DURATION		"Duration"
#define	FXBMGR_PUSHERFX_STRENGTH		"Strength"

#define FXBMGR_PVFX_TAG					"PVFX"
#define FXBMGR_PVFX_NAME				"Name"
#define FXBMGR_PVFX_SOCKET				"Socket"
#define FXBMGR_PVFX_SCALENAME			"ScaleName"
#define FXBMGR_PVFX_DLIGHTNAME			"DLightName"
#define FXBMGR_PVFX_SOUNDNAME			"SoundName"

#define FXBMGR_PARTMUZZLEFX_TAG			"ParticleMuzzleFX"
#define FXBMGR_PARTMUZZLEFX_NAME		"Name"
#define FXBMGR_PARTMUZZLEFX_FILE		"File"
#define FXBMGR_PARTMUZZLEFX_LENGTH		"Length"
#define FXBMGR_PARTMUZZLEFX_DURATION	"Duration"
#define FXBMGR_PARTMUZZLEFX_RADIUS		"Radius"
#define FXBMGR_PARTMUZZLEFX_MAXSCALE	"MaxScale"
#define FXBMGR_PARTMUZZLEFX_NUMBER		"NumParticles"
#define FXBMGR_PARTMUZZLEFX_COLOR1		"Color1"
#define FXBMGR_PARTMUZZLEFX_COLOR2		"Color2"
#define	FXBMGR_PARTMUZZLEFX_ADDITIVE	"Additive"
#define	FXBMGR_PARTMUZZLEFX_MULTIPLY	"Multiply"

#define FXBMGR_MUZZLEFX_TAG				"MuzzleFX"
#define FXBMGR_MUZZLEFX_DURATION		"Duration"
#define FXBMGR_MUZZLEFX_NAME			"Name"
#define FXBMGR_MUZZLEFX_PMUZZLEFXNAME	"PMuzzleFXName"
#define FXBMGR_MUZZLEFX_SCALEFXNAME		"ScaleFXName"
#define FXBMGR_MUZZLEFX_DLIGHTFXNAME	"DLightFXName"

#define FXBMGR_TRACERFX_TAG				"TracerFX"
#define FXBMGR_TRACERFX_NAME			"Name"
#define FXBMGR_TRACERFX_TEXTURE			"Texture"
#define FXBMGR_TRACERFX_FREQUENCY		"Frequency"
#define FXBMGR_TRACERFX_VELOCITY		"Velocity"
#define FXBMGR_TRACERFX_WIDTH			"Width"
#define FXBMGR_TRACERFX_INITIALALPHA	"InitialAlpha"
#define FXBMGR_TRACERFX_FINALALPHA		"FinalAlpha"
#define FXBMGR_TRACERFX_COLOR			"Color"

#define FXBMGR_BEAMFX_TAG				"BeamFX"
#define FXBMGR_BEAMFX_NAME				"Name"
#define FXBMGR_BEAMFX_TEXTURE			"Texture"
#define FXBMGR_BEAMFX_DURATION			"Duration"
#define FXBMGR_BEAMFX_WIDTH				"Width"
#define FXBMGR_BEAMFX_INITIALALPHA		"InitialAlpha"
#define FXBMGR_BEAMFX_FINALALPHA		"FinalAlpha"
#define FXBMGR_BEAMFX_COLOR				"Color"
#define FXBMGR_BEAMFX_ALIGNUP			"AlignUp"
#define FXBMGR_BEAMFX_ALIGNFLAT			"AlignFlat"


#define FXBMGR_MIN_BLASTMARK_RADIUS		100.0f

static char s_aTagName[30];
static char s_aAttName[100];
static char s_FileBuffer[MAX_CS_FILENAME_LEN];

CFXButeMgr* g_pFXButeMgr = LTNULL;


#ifndef _CLIENTBUILD
// Plugin statics

CFXButeMgr CFXButeMgrPlugin::sm_FXButeMgr;

#endif // _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CFXButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CFXButeMgr::CFXButeMgr()
{
    m_ProjectileFXList.Init(LTTRUE);
    m_ProjClassDataList.Init(LTTRUE);
    m_ImpactFXList.Init(LTTRUE);
    m_FireFXList.Init(LTTRUE);
    m_ScaleFXList.Init(LTTRUE);
    m_PShowerFXList.Init(LTTRUE);
    m_PolyDebrisFXList.Init(LTTRUE);
    m_PExplFXList.Init(LTTRUE);
    m_DLightFXList.Init(LTTRUE);
    m_SoundFXList.Init(LTTRUE);
    m_PusherFXList.Init(LTTRUE);
    m_PVFXList.Init(LTTRUE);
    m_PartMuzzleFXList.Init(LTTRUE);
    m_MuzzleFXList.Init(LTTRUE);
    m_TracerFXList.Init(LTTRUE);
    m_BeamFXList.Init(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::~CFXButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CFXButeMgr::~CFXButeMgr()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
    if (g_pFXButeMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(pInterface, szAttributeFile)) return LTFALSE;


	// Set up global pointer...

	g_pFXButeMgr = this;


	// Build our lists...

	BuildScaleFXList(m_ScaleFXList, m_buteMgr, FXBMGR_SCALEFX_TAG);
	BuildPShowerFXList(m_PShowerFXList, m_buteMgr, FXBMGR_PSHOWERFX_TAG);
	BuildPolyDebrisFXList(m_PolyDebrisFXList, m_buteMgr, FXBMGR_POLYDEBRISFX_TAG);


	// Read in the properties for each projectile class data record...
	// NOTE: This must be done before the ProjectileFX records are
	// read in...

	// Read in the properties for the ProxClassData record...

	if (m_buteMgr.Exist(FXBMGR_PROXCLASS_TAG))
	{
		PROJECTILECLASSDATA* pPCD = debug_new(PROXCLASSDATA);

		if (pPCD && pPCD->Init(m_buteMgr, FXBMGR_PROXCLASS_TAG))
		{
			m_ProjClassDataList.AddTail(pPCD);
		}
	}


	// Read in the properties for each projectile fx type...

	int nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_PROJECTILEFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		PROJECTILEFX* pPFX = debug_new(PROJECTILEFX);

		if (pPFX && pPFX->Init(m_buteMgr, s_aTagName))
		{
			pPFX->nId = nNum;
			m_ProjectileFXList.AddTail(pPFX);
		}
		else
		{
			debug_delete(pPFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_PROJECTILEFX_TAG, nNum);
	}


	// Read in the properties for each beam fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_BEAMFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		BEAMFX* pFX = debug_new(BEAMFX);

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_BeamFXList.AddTail(pFX);
		}
		else
		{
			debug_delete(pFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_BEAMFX_TAG, nNum);
	}


	// Read in the properties for each fire fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_FIREFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		FIREFX* pFFX = debug_new(FIREFX);

		if (pFFX && pFFX->Init(m_buteMgr, s_aTagName))
		{
			pFFX->nId = nNum;
			m_FireFXList.AddTail(pFFX);
		}
		else
		{
			debug_delete(pFFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_FIREFX_TAG, nNum);
	}


	// Read in the properties for each particle explosion fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_PEXPLFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		PEXPLFX* pPEFX = debug_new(PEXPLFX);

		if (pPEFX && pPEFX->Init(m_buteMgr, s_aTagName))
		{
			pPEFX->nId = nNum;
			m_PExplFXList.AddTail(pPEFX);
		}
		else
		{
			debug_delete(pPEFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_PEXPLFX_TAG, nNum);
	}


	// Read in the properties for each dynamic light fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_DLIGHTFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		DLIGHTFX* pDLightFX = debug_new(DLIGHTFX);

		if (pDLightFX && pDLightFX->Init(m_buteMgr, s_aTagName))
		{
			pDLightFX->nId = nNum;
			m_DLightFXList.AddTail(pDLightFX);
		}
		else
		{
			debug_delete(pDLightFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_DLIGHTFX_TAG, nNum);
	}


	// Read in the properties for each sound light fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_SOUNDFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		SOUNDFX* pFX = debug_new(SOUNDFX);

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_SoundFXList.AddTail(pFX);
		}
		else
		{
			debug_delete(pFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_SOUNDFX_TAG, nNum);
	}


	// Read in the properties for each pusher fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_PUSHERFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		PUSHERFX* pFX = debug_new(PUSHERFX);

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_PusherFXList.AddTail(pFX);
		}
		else
		{
			debug_delete(pFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_PUSHERFX_TAG, nNum);
	}


	// Read in the properties for each impact fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_IMPACTFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		IMPACTFX* pIFX = debug_new(IMPACTFX);

		if (pIFX && pIFX->Init(m_buteMgr, s_aTagName))
		{
			pIFX->nId = nNum;
			m_ImpactFXList.AddTail(pIFX);
		}
		else
		{
			debug_delete(pIFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_IMPACTFX_TAG, nNum);
	}


	// Read in the properties for each pv fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_PVFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		PVFX* pFX = debug_new(PVFX);

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_PVFXList.AddTail(pFX);
		}
		else
		{
			debug_delete(pFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_PVFX_TAG, nNum);
	}


	// Read in the properties for each particle muzzle fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_PARTMUZZLEFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		CParticleMuzzleFX* pFX = debug_new(CParticleMuzzleFX);

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_PartMuzzleFXList.AddTail(pFX);
		}
		else
		{
			debug_delete(pFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_PARTMUZZLEFX_TAG, nNum);
	}


	// Read in the properties for each muzzle fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_MUZZLEFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		CMuzzleFX* pFX = debug_new(CMuzzleFX);

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_MuzzleFXList.AddTail(pFX);
		}
		else
		{
			debug_delete(pFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_MUZZLEFX_TAG, nNum);
	}


	// Read in the properties for each tracer fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_TRACERFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		TRACERFX* pFX = debug_new(TRACERFX);

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_TracerFXList.AddTail(pFX);
		}
		else
		{
			debug_delete(pFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_TRACERFX_TAG, nNum);
	}


	// Free up the bute mgr's memory...

	m_buteMgr.Term();


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CFXButeMgr::Term()
{
    g_pFXButeMgr = LTNULL;

	m_ProjectileFXList.Clear();
	m_ProjClassDataList.Clear();
	m_ImpactFXList.Clear();
	m_FireFXList.Clear();
	m_ScaleFXList.Clear();
	m_PShowerFXList.Clear();
	m_PolyDebrisFXList.Clear();
	m_PExplFXList.Clear();
	m_DLightFXList.Clear();
	m_PVFXList.Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::Reload()
//
//	PURPOSE:	Reload data from the bute file
//
// ----------------------------------------------------------------------- //

void CFXButeMgr::Reload(ILTCSBase *pInterface)
{
	Term();
    Init(pInterface);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetScaleFX
//
//	PURPOSE:	Get the specified scale fx struct
//
// ----------------------------------------------------------------------- //

CScaleFX* CFXButeMgr::GetScaleFX(int nScaleFXId)
{
    CScaleFX** pCur  = LTNULL;

	pCur = m_ScaleFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nScaleFXId)
		{
			return *pCur;
		}

		pCur = m_ScaleFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetScaleFX
//
//	PURPOSE:	Get the specified scale fx struct
//
// ----------------------------------------------------------------------- //

CScaleFX* CFXButeMgr::GetScaleFX(char* pName)
{
    if (!pName) return LTNULL;

    CScaleFX** pCur  = LTNULL;

	pCur = m_ScaleFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_ScaleFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPShowerFX
//
//	PURPOSE:	Get the specified PShower fx struct
//
// ----------------------------------------------------------------------- //

CPShowerFX* CFXButeMgr::GetPShowerFX(int nPShowerFXId)
{
    if (nPShowerFXId < 0 || nPShowerFXId > m_PShowerFXList.GetLength()) return LTNULL;

	CPShowerFX** pCur = m_PShowerFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nPShowerFXId)
		{
			return *pCur;
		}

		pCur = m_PShowerFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPShowerFX
//
//	PURPOSE:	Get the specified PShower fx struct
//
// ----------------------------------------------------------------------- //

CPShowerFX* CFXButeMgr::GetPShowerFX(char* pName)
{
    if (!pName) return LTNULL;

	CPShowerFX** pCur = m_PShowerFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_PShowerFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPolyDebrisFX
//
//	PURPOSE:	Get the specified PolyDebris fx struct
//
// ----------------------------------------------------------------------- //

CPolyDebrisFX* CFXButeMgr::GetPolyDebrisFX(int nPolyDebrisFXId)
{
    if (nPolyDebrisFXId < 0 || nPolyDebrisFXId > m_PolyDebrisFXList.GetLength()) return LTNULL;

	CPolyDebrisFX** pCur = m_PolyDebrisFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nPolyDebrisFXId)
		{
			return *pCur;
		}

		pCur = m_PolyDebrisFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPolyDebrisFX
//
//	PURPOSE:	Get the specified PolyDebris fx struct
//
// ----------------------------------------------------------------------- //

CPolyDebrisFX* CFXButeMgr::GetPolyDebrisFX(char* pName)
{
    if (!pName) return LTNULL;

	CPolyDebrisFX** pCur = m_PolyDebrisFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_PolyDebrisFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetProjectileFX
//
//	PURPOSE:	Get the specified projectile fx struct
//
// ----------------------------------------------------------------------- //

PROJECTILEFX* CFXButeMgr::GetProjectileFX(int nProjectileFXId)
{
    PROJECTILEFX** pCur  = LTNULL;

	pCur = m_ProjectileFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nProjectileFXId)
		{
			return *pCur;
		}

		pCur = m_ProjectileFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetProjectileFX
//
//	PURPOSE:	Get the specified projectile fx struct
//
// ----------------------------------------------------------------------- //

PROJECTILEFX* CFXButeMgr::GetProjectileFX(char* pName)
{
    if (!pName) return LTNULL;

    PROJECTILEFX** pCur  = LTNULL;

	pCur = m_ProjectileFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_ProjectileFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetProjectileClassData
//
//	PURPOSE:	Get the specified projectile class data struct
//
// ----------------------------------------------------------------------- //

PROJECTILECLASSDATA* CFXButeMgr::GetProjectileClassData(char* pName)
{
    if (!pName) return LTNULL;

    PROJECTILECLASSDATA** pCur  = LTNULL;

	pCur = m_ProjClassDataList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_ProjClassDataList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetImpactFX
//
//	PURPOSE:	Get the specified impact fx struct
//
// ----------------------------------------------------------------------- //

IMPACTFX* CFXButeMgr::GetImpactFX(int nImpactFXId)
{
    IMPACTFX** pCur  = LTNULL;

	pCur = m_ImpactFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nImpactFXId)
		{
			return *pCur;
		}

		pCur = m_ImpactFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetImpactFX
//
//	PURPOSE:	Get the specified impact fx struct
//
// ----------------------------------------------------------------------- //

IMPACTFX* CFXButeMgr::GetImpactFX(char* pName)
{
    if (!pName) return LTNULL;

    IMPACTFX** pCur  = LTNULL;

	pCur = m_ImpactFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_ImpactFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetFireFX
//
//	PURPOSE:	Get the specified fire fx struct
//
// ----------------------------------------------------------------------- //

FIREFX* CFXButeMgr::GetFireFX(int nFireFXId)
{
    FIREFX** pCur  = LTNULL;

	pCur = m_FireFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nFireFXId)
		{
			return *pCur;
		}

		pCur = m_FireFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetFireFX
//
//	PURPOSE:	Get the specified fire fx struct
//
// ----------------------------------------------------------------------- //

FIREFX* CFXButeMgr::GetFireFX(char* pName)
{
    if (!pName) return LTNULL;

    FIREFX** pCur  = LTNULL;

	pCur = m_FireFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_FireFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPExplFX
//
//	PURPOSE:	Get the specified PExpl fx struct
//
// ----------------------------------------------------------------------- //

PEXPLFX* CFXButeMgr::GetPExplFX(int nPExpFXId)
{
    PEXPLFX** pCur  = LTNULL;

	pCur = m_PExplFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nPExpFXId)
		{
			return *pCur;
		}

		pCur = m_PExplFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPExplFX
//
//	PURPOSE:	Get the specified PExpl fx struct
//
// ----------------------------------------------------------------------- //

PEXPLFX* CFXButeMgr::GetPExplFX(char* pName)
{
    if (!pName) return LTNULL;

    PEXPLFX** pCur  = LTNULL;

	pCur = m_PExplFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_PExplFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetDLightFX
//
//	PURPOSE:	Get the specified DLIGHT fx struct
//
// ----------------------------------------------------------------------- //

DLIGHTFX* CFXButeMgr::GetDLightFX(int nDLightFXId)
{
    DLIGHTFX** pCur  = LTNULL;

	pCur = m_DLightFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nDLightFXId)
		{
			return *pCur;
		}

		pCur = m_DLightFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetDLightFX
//
//	PURPOSE:	Get the specified DLIGHT fx struct
//
// ----------------------------------------------------------------------- //

DLIGHTFX* CFXButeMgr::GetDLightFX(char* pName)
{
    if (!pName) return LTNULL;

    DLIGHTFX** pCur  = LTNULL;

	pCur = m_DLightFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_DLightFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetSoundFX
//
//	PURPOSE:	Get the specified SOUNDFX struct
//
// ----------------------------------------------------------------------- //

SOUNDFX* CFXButeMgr::GetSoundFX(int nSoundFXId)
{
    SOUNDFX** pCur  = LTNULL;

	pCur = m_SoundFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nSoundFXId)
		{
			return *pCur;
		}

		pCur = m_SoundFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetSoundFX
//
//	PURPOSE:	Get the specified SOUNDFX struct
//
// ----------------------------------------------------------------------- //

SOUNDFX* CFXButeMgr::GetSoundFX(char* pName)
{
    if (!pName) return LTNULL;

    SOUNDFX** pCur  = LTNULL;

	pCur = m_SoundFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_SoundFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPusherFX
//
//	PURPOSE:	Get the specified PUSHERFX struct
//
// ----------------------------------------------------------------------- //

PUSHERFX* CFXButeMgr::GetPusherFX(int nSoundFXId)
{
    PUSHERFX** pCur  = LTNULL;

	pCur = m_PusherFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nSoundFXId)
		{
			return *pCur;
		}

		pCur = m_PusherFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPusherFX
//
//	PURPOSE:	Get the specified PUSHERFX struct
//
// ----------------------------------------------------------------------- //

PUSHERFX* CFXButeMgr::GetPusherFX(char* pName)
{
    if (!pName) return LTNULL;

    PUSHERFX** pCur  = LTNULL;

	pCur = m_PusherFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_PusherFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPVFX
//
//	PURPOSE:	Get the specified pv fx struct
//
// ----------------------------------------------------------------------- //

PVFX* CFXButeMgr::GetPVFX(int nPVFXId)
{
    PVFX** pCur = LTNULL;

	pCur = m_PVFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nPVFXId)
		{
			return *pCur;
		}

		pCur = m_PVFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPVFX
//
//	PURPOSE:	Get the specified pv fx struct
//
// ----------------------------------------------------------------------- //

PVFX* CFXButeMgr::GetPVFX(char* pName)
{
    if (!pName) return LTNULL;

    PVFX** pCur  = LTNULL;

	pCur = m_PVFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_PVFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetParticleMuzzleFX
//
//	PURPOSE:	Get the specified particle muzzle fx struct
//
// ----------------------------------------------------------------------- //

CParticleMuzzleFX* CFXButeMgr::GetParticleMuzzleFX(int nPMFXId)
{
    CParticleMuzzleFX** pCur = LTNULL;

	pCur = m_PartMuzzleFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nPMFXId)
		{
			return *pCur;
		}

		pCur = m_PartMuzzleFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetParticleMuzzleFX
//
//	PURPOSE:	Get the specified particle muzzle fx struct
//
// ----------------------------------------------------------------------- //

CParticleMuzzleFX* CFXButeMgr::GetParticleMuzzleFX(char* pName)
{
    if (!pName) return LTNULL;

    CParticleMuzzleFX** pCur  = LTNULL;

	pCur = m_PartMuzzleFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_PartMuzzleFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetMuzzleFX
//
//	PURPOSE:	Get the specified muzzle fx struct
//
// ----------------------------------------------------------------------- //

CMuzzleFX* CFXButeMgr::GetMuzzleFX(int nMuzzleFXId)
{
    CMuzzleFX** pCur = LTNULL;

	pCur = m_MuzzleFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nMuzzleFXId)
		{
			return *pCur;
		}

		pCur = m_MuzzleFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetMuzzleFX
//
//	PURPOSE:	Get the specified muzzle fx struct
//
// ----------------------------------------------------------------------- //

CMuzzleFX* CFXButeMgr::GetMuzzleFX(char* pName)
{
    if (!pName) return LTNULL;

    CMuzzleFX** pCur  = LTNULL;

	pCur = m_MuzzleFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_MuzzleFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetTracerFX
//
//	PURPOSE:	Get the specified tracer fx struct
//
// ----------------------------------------------------------------------- //

TRACERFX* CFXButeMgr::GetTracerFX(int nTracerFXId)
{
    TRACERFX** pCur = LTNULL;

	pCur = m_TracerFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nTracerFXId)
		{
			return *pCur;
		}

		pCur = m_TracerFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetTracerFX
//
//	PURPOSE:	Get the specified tracer fx struct
//
// ----------------------------------------------------------------------- //

TRACERFX* CFXButeMgr::GetTracerFX(char* pName)
{
    if (!pName) return LTNULL;

    TRACERFX** pCur  = LTNULL;

	pCur = m_TracerFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_TracerFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetBeamFX
//
//	PURPOSE:	Get the specified beam fx struct
//
// ----------------------------------------------------------------------- //

BEAMFX* CFXButeMgr::GetBeamFX(int nBeamFXId)
{
    BEAMFX** pCur = LTNULL;

	pCur = m_BeamFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nBeamFXId)
		{
			return *pCur;
		}

		pCur = m_BeamFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetBeamFX
//
//	PURPOSE:	Get the specified beam fx struct
//
// ----------------------------------------------------------------------- //

BEAMFX* CFXButeMgr::GetBeamFX(char* pName)
{
    if (!pName) return LTNULL;

    BEAMFX** pCur  = LTNULL;

	pCur = m_BeamFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_BeamFXList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}




/////////////////////////////////////////////////////////////////////////////
//
//	P E X P L  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PEXPLFX::PEXPLFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PEXPLFX::PEXPLFX()
{
	nId				= FXBMGR_INVALID_ID;

	szName[0]		= '\0';
	szFile[0]		= '\0';

	nNumPerPuff		= 0;
	nNumEmitters	= 0;
	nNumSteps		= 0;
    bCreateDebris   = LTFALSE;
    bRotateDebris   = LTFALSE;
    bIgnoreWind     = LTFALSE;
    bDoBubbles      = LTFALSE;
    bAdditive       = LTFALSE;
    bMultiply       = LTFALSE;
	fLifeTime		= 0.0f;
	fFadeTime		= 0.0f;
	fOffsetTime		= 0.0f;
	fRadius			= 0.0f;
	fGravity		= 0.0f;

	vPosOffset.Init();
	vColor1.Init();
	vColor2.Init();
	vMinVel.Init();
	vMaxVel.Init();
	vMinDriftVel.Init();
	vMaxDriftVel.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PEXPLFX::Init
//
//	PURPOSE:	Build the particle explosion struct
//
// ----------------------------------------------------------------------- //

LTBOOL PEXPLFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	nNumPerPuff		= buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_NUMPERPUFF);
	nNumEmitters	= buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_NUMEMITTERS);
	nNumSteps		= buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_NUMSTEPS);

    bCreateDebris   = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_CREATEDEBRIS);
    bRotateDebris   = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_ROTATEDEBRIS);
    bIgnoreWind     = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_IGNOREWIND);
    bDoBubbles      = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_DOBUBBLES);
    bAdditive       = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_ADDITIVE);
    bMultiply       = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_MULTIPLY);

    fLifeTime       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PEXPLFX_LIFETIME);
    fFadeTime       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PEXPLFX_FADETIME);
    fOffsetTime     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PEXPLFX_OFFSETTIME);
    fRadius         = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PEXPLFX_RADIUS);
    fGravity        = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PEXPLFX_GRAVITY);

	vPosOffset		= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_POSOFFSET);
	vColor1			= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_COLOR1);
	vColor2			= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_COLOR2);
	vMinVel			= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_MINVEL);
	vMaxVel			= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_MAXVEL);
	vMinDriftVel	= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_MINDRIFTVEL);
	vMaxDriftVel	= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_MAXDRIFTVEL);

	CString str = buteMgr.GetString(aTagName, FXBMGR_PEXPLFX_FILE);
	if (!str.IsEmpty())
	{
		strncpy(szFile, (char*)(LPCSTR)str, ARRAY_LEN(szFile));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PEXPLFX_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PEXPLFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the particle
//				explosion struct
//
// ----------------------------------------------------------------------- //

void PEXPLFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
	if (!pFXButeMgr) return;

	if (szFile[0])
	{
		if (strstr(szFile, ".spr"))
		{
            g_pLTServer->CacheFile(FT_SPRITE, szFile);
		}
		else
		{
            g_pLTServer->CacheFile(FT_TEXTURE, szFile);
		}
	}
#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//	D L I G H T  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DLIGHTFX::DLIGHTFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

DLIGHTFX::DLIGHTFX()
{
	nId		= FXBMGR_INVALID_ID;

	szName[0]		= '\0';

	vColor.Init();

	fMinRadius		= 0.0f;
	fMaxRadius		= 0.0f;
	fMinTime		= 0.0f;
	fMaxTime		= 0.0f;
	fRampUpTime		= 0.0f;
	fRampDownTime	= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DLIGHTFX::Init
//
//	PURPOSE:	Build the dynamic light struct
//
// ----------------------------------------------------------------------- //

LTBOOL DLIGHTFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

    fMinRadius      = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_MINRADIUS);
    fMaxRadius      = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_MAXRADIUS);
    fMinTime        = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_MINTIME);
    fMaxTime        = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_MAXTIME);
    fRampUpTime     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_RAMPUPTIME);
    fRampDownTime   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_RAMPDOWNTIME);

	vColor			= buteMgr.GetVector(aTagName, FXBMGR_DLIGHTFX_COLOR);
	vColor /= 255.0f;

	CString str = buteMgr.GetString(aTagName, FXBMGR_DLIGHTFX_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DLIGHTFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the dynamic
//				light struct
//
// ----------------------------------------------------------------------- //

void DLIGHTFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
#endif
}



/////////////////////////////////////////////////////////////////////////////
//
//	I M P A C T  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IMPACTFX::IMPACTFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

IMPACTFX::IMPACTFX()
{
	nId		= FXBMGR_INVALID_ID;

	szName[0]	= '\0';
	szSound[0]	= '\0';
	szMark[0]	= '\0';

	nSoundRadius	= 0;
	nAISoundRadius	= 0;
	bAIIgnoreSurface = LTFALSE;
	nFlags			= 0;
	fMarkScale		= 0.0f;
	fTintRampUp		= 0.0f;
	fTintRampDown	= 0.0f;
	fTintMaxTime	= 0.0f;
	fBlastTimeMin	= 0.0f;
	fBlastTimeMax	= 0.0f;
	fBlastFadeMin	= 0.0f;
	fBlastFadeMax	= 0.0f;
    bDoSurfaceFX    = LTFALSE;
	bIgnoreFlesh	= LTFALSE;
	bIgnoreLiquid	= LTFALSE;

	vTintColor.Init();
	vBlastColor.Init();

	pPusherFX		= LTNULL;

	nNumDebrisFXTypes = 0;
    int i;
    for (i=0; i < IMPACT_MAX_DEBRISFX_TYPES; i++)
	{
		aDebrisFXTypes[i] = FXBMGR_INVALID_ID;
	}

	nNumScaleFXTypes = 0;
	for (i=0; i < IMPACT_MAX_SCALEFX_TYPES; i++)
	{
		aScaleFXTypes[i] = FXBMGR_INVALID_ID;
	}

	nNumPExplFX = 0;
	for (i=0; i < IMPACT_MAX_PEXPLFX_TYPES; i++)
	{
		aPExplFXTypes[i] = FXBMGR_INVALID_ID;
	}

	nNumDLightFX = 0;
	for (i=0; i < IMPACT_MAX_DLIGHTFX_TYPES; i++)
	{
		aDLightFXTypes[i] = FXBMGR_INVALID_ID;
	}

	nNumPolyDebrisFX = 0;
	for (i=0; i < IMPACT_MAX_PDEBRISFX_TYPES; i++)
	{
		aPolyDebrisFXTypes[i] = FXBMGR_INVALID_ID;
	}

	nNumPShowerFX = 0;
	for (i=0; i < IMPACT_MAX_PSHOWERFX; i++)
	{
		aPShowerFXIds[i] = FXBMGR_INVALID_ID;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IMPACTFX::Init
//
//	PURPOSE:	Build the impact fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL IMPACTFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_IMPACTFX_SOUND);
	if (!str.IsEmpty())
	{
		strncpy(szSound, (char*)(LPCSTR)str, ARRAY_LEN(szSound));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_IMPACTFX_MARK);
	if (!str.IsEmpty())
	{
		strncpy(szMark, (char*)(LPCSTR)str, ARRAY_LEN(szMark));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_IMPACTFX_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_IMPACTFX_PUSHERNAME);
	if (!str.IsEmpty())
	{
		pPusherFX = g_pFXButeMgr->GetPusherFX((char*)(LPCSTR)str);
	}

	nSoundRadius	= buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_SOUNDRADIUS);
	nAISoundRadius	= buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_AISOUNDRADIUS);
	bAIIgnoreSurface= buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_AIIGNORESURFACE);

    fMarkScale      = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_MARKSCALE);
    fTintRampUp     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_TINTRAMPUP);
    fTintRampDown   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_TINTRAMPDOWN);
    fTintMaxTime    = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_TINTMAXTIME);
    fBlastTimeMin   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_BLASTTIMEMIN);
    fBlastTimeMax   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_BLASTTIMEMAX);
    fBlastFadeMin   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_BLASTFADEMIN);
    fBlastFadeMax   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_BLASTFADEMAX);

	vTintColor		= buteMgr.GetVector(aTagName, FXBMGR_IMPACTFX_TINTCOLOR);
	vTintColor /= 255.0f;

	vBlastColor		= buteMgr.GetVector(aTagName, FXBMGR_IMPACTFX_BLASTCOLOR);
	vBlastColor /= 255.0f;

    bDoSurfaceFX    = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_DOSURFACEFX);
    bIgnoreFlesh    = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_IGNOREFLESH);
    bIgnoreLiquid   = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_IGNORELIQUID);

	// Build our debris fx types id array...

	nNumDebrisFXTypes = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_DEBRISNAME, nNumDebrisFXTypes);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumDebrisFXTypes < IMPACT_MAX_DEBRISFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			DEBRIS* pDebrisFX = g_pDebrisMgr->GetDebris((char*)(LPCSTR)str);
			if (pDebrisFX)
			{
				aDebrisFXTypes[nNumDebrisFXTypes] = pDebrisFX->nId;
			}
		}

		nNumDebrisFXTypes++;
		sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_DEBRISNAME, nNumDebrisFXTypes);
	}

	// Build our scale fx types id array...

	nNumScaleFXTypes = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_SCALENAME, nNumScaleFXTypes);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumScaleFXTypes < IMPACT_MAX_SCALEFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX((char*)(LPCSTR)str);
			if (pScaleFX)
			{
				aScaleFXTypes[nNumScaleFXTypes] = pScaleFX->nId;
			}
		}

		nNumScaleFXTypes++;
		sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_SCALENAME, nNumScaleFXTypes);
	}

	// Build our particle explosion fx types id array...

	nNumPExplFX = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_PEXPLNAME, nNumPExplFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumPExplFX < IMPACT_MAX_PEXPLFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			PEXPLFX* pPExplFX = g_pFXButeMgr->GetPExplFX((char*)(LPCSTR)str);
			if (pPExplFX)
			{
				aPExplFXTypes[nNumPExplFX] = pPExplFX->nId;
			}
		}

		nNumPExplFX++;
		sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_PEXPLNAME, nNumPExplFX);
	}

	// Build our dynamic light fx types id array...

	nNumDLightFX = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_DLIGHTNAME, nNumDLightFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumDLightFX < IMPACT_MAX_DLIGHTFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			DLIGHTFX* pDLightFX = g_pFXButeMgr->GetDLightFX((char*)(LPCSTR)str);
			if (pDLightFX)
			{
				aDLightFXTypes[nNumDLightFX] = pDLightFX->nId;
			}
		}

		nNumDLightFX++;
		sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_DLIGHTNAME, nNumDLightFX);
	}


	// Build our poly debris fx types id array...

	nNumPolyDebrisFX = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_PDEBRISNAME, nNumPolyDebrisFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumPolyDebrisFX < IMPACT_MAX_PDEBRISFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			CPolyDebrisFX* pPDebrisFX = g_pFXButeMgr->GetPolyDebrisFX((char*)(LPCSTR)str);
			if (pPDebrisFX)
			{
				aPolyDebrisFXTypes[nNumPolyDebrisFX] = pPDebrisFX->nId;
			}
		}

		nNumPolyDebrisFX++;
		sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_PDEBRISNAME, nNumPolyDebrisFX);
	}


	// Build our particle shower fx types id array...

	nNumPShowerFX = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_PSHOWERNAME, nNumPShowerFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumPShowerFX < IMPACT_MAX_PSHOWERFX)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			CPShowerFX* pPShowerFX = g_pFXButeMgr->GetPShowerFX((char*)(LPCSTR)str);
			if (pPShowerFX)
			{
				aPShowerFXIds[nNumPShowerFX] = pPShowerFX->nId;
			}
		}

		nNumPShowerFX++;
		sprintf(s_aAttName, "%s%d", FXBMGR_IMPACTFX_PSHOWERNAME, nNumPShowerFX);
	}


	nFlags = 0;

	if (buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_CREATEMARK))
	{
		nFlags |= WFX_MARK;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_SCREENTINT))
	{
		nFlags |= WFX_TINTSCREEN;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_BLASTMARK))
	{
		nFlags |= WFX_BLASTMARK;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_IMPACTONSKY))
	{
		nFlags |= WFX_IMPACTONSKY;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IMPACTFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the impact
//				fx struct
//
// ----------------------------------------------------------------------- //

void IMPACTFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	if (pPusherFX)
	{
		pPusherFX->Cache(pFXButeMgr);
	}

	if (szSound[0] && strstr(szSound, ".wav"))
	{
        g_pLTServer->CacheFile(FT_SOUND, szSound);
	}

	if (szMark[0] && strstr(szMark, ".spr"))
	{
        g_pLTServer->CacheFile(FT_SPRITE, szMark);
	}

    int i;
    for (i=0; i < nNumDebrisFXTypes; i++)
	{
		DEBRIS* pDebrisFX = g_pDebrisMgr->GetDebris(aDebrisFXTypes[i]);
		if (pDebrisFX)
		{
			pDebrisFX->Cache(g_pDebrisMgr);
		}
	}


	for (i=0; i < nNumScaleFXTypes; i++)
	{
		CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX(aScaleFXTypes[i]);
		if (pScaleFX)
		{
			pScaleFX->Cache();
		}
	}

	for (i=0; i < nNumPExplFX; i++)
	{
		PEXPLFX* pPExplFX = g_pFXButeMgr->GetPExplFX(aPExplFXTypes[i]);
		if (pPExplFX)
		{
			pPExplFX->Cache(pFXButeMgr);
		}
	}

	for (i=0; i < nNumDLightFX; i++)
	{
		DLIGHTFX* pDLightFX = g_pFXButeMgr->GetDLightFX(aDLightFXTypes[i]);
		if (pDLightFX)
		{
			pDLightFX->Cache(pFXButeMgr);
		}
	}

	for (i=0; i < nNumPolyDebrisFX; i++)
	{
		CPolyDebrisFX* pPolyDebrisFX = g_pFXButeMgr->GetPolyDebrisFX(aPolyDebrisFXTypes[i]);
		if (pPolyDebrisFX)
		{
			pPolyDebrisFX->Cache();
		}
	}

	for (i=0; i < nNumPShowerFX; i++)
	{
		CPShowerFX* pPShowerFX = g_pFXButeMgr->GetPShowerFX(aPShowerFXIds[i]);
		if (pPShowerFX)
		{
			pPShowerFX->Cache();
		}
	}

#endif
}




/////////////////////////////////////////////////////////////////////////////
//
//	F I R E  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FIREFX::FIREFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FIREFX::FIREFX()
{
	nId		= FXBMGR_INVALID_ID;

	szName[0]		= '\0';
	szShellModel[0] = '\0';
	szShellSkin[0]	= '\0';

	nFlags = 0;
	vShellScale.Init(1, 1, 1);

	nNumBeamFX = 0;

	for (int i=0; i < FIRE_MAX_BEAMFX; i++)
	{
		pBeamFX[i] = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FIREFX::Init
//
//	PURPOSE:	Build the fire fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL FIREFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_FIREFX_SHELLMODEL);
	if (!str.IsEmpty())
	{
		strncpy(szShellModel, (char*)(LPCSTR)str, ARRAY_LEN(szShellModel));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_FIREFX_SHELLSKIN);
	if (!str.IsEmpty())
	{
		strncpy(szShellSkin, (char*)(LPCSTR)str, ARRAY_LEN(szShellSkin));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_FIREFX_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	vShellScale = buteMgr.GetVector(aTagName, FXBMGR_FIREFX_SHELLSCALE);

	nFlags = 0;

	if (buteMgr.GetInt(aTagName, FXBMGR_FIREFX_MUZZLESMOKE))
	{
		nFlags |= WFX_MUZZLE;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_FIREFX_EJECTSHELLS))
	{
		nFlags |= WFX_SHELL;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_FIREFX_MUZZLELIGHT))
	{
		nFlags |= WFX_LIGHT;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_FIREFX_FIRESOUND))
	{
		nFlags |= WFX_FIRESOUND;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_FIREFX_EXITMARK))
	{
		nFlags |= WFX_EXITMARK;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_FIREFX_EXITDEBRIS))
	{
		nFlags |= WFX_EXITDEBRIS;
	}

	// Build our beam fx array...

	nNumBeamFX = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_FIREFX_BEAMFXNAME, nNumBeamFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumBeamFX < IMPACT_MAX_PDEBRISFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			BEAMFX* pFX = g_pFXButeMgr->GetBeamFX((char*)(LPCSTR)str);
			if (pFX)
			{
				pBeamFX[nNumBeamFX] = pFX;
			}
		}

		nNumBeamFX++;
		sprintf(s_aAttName, "%s%d", FXBMGR_FIREFX_BEAMFXNAME, nNumBeamFX);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FIREFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the fire
//				fx struct
//
// ----------------------------------------------------------------------- //

void FIREFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	if (szShellModel[0])
	{
        g_pLTServer->CacheFile(FT_MODEL, szShellModel);
	}

	if (szShellSkin[0])
	{
        g_pLTServer->CacheFile(FT_TEXTURE, szShellSkin);
	}

	for (int i=0; i < nNumBeamFX; i++)
	{
		if (pBeamFX[i])
		{
			pBeamFX[i]->Cache(g_pFXButeMgr);
		}
	}

#endif
}





/////////////////////////////////////////////////////////////////////////////
//
//	P R O J E C T I L E  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROJECTILEFX::PROJECTILEFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PROJECTILEFX::PROJECTILEFX()
{
	nId		= FXBMGR_INVALID_ID;

	szName[0]			= '\0';
	szFlareSprite[0]	= '\0';
	szSound[0]			= '\0';
	szClass[0]			= '\0';
	szModel[0]			= '\0';
	szSkin[0]			= '\0';

	nSmokeTrailType = 0;
	nAltVelocity	= 0;
	nVelocity		= 0;
	nFlags			= 0;
	nLightRadius	= 0;
	nSoundRadius	= 0;
	fLifeTime		= 0.0f;
	fFlareScale		= 0.0f;
	dwObjectFlags	= 0;

    pClassData      = LTNULL;

	vLightColor.Init();
	vModelScale.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROJECTILEFX::Init
//
//	PURPOSE:	Build the projectile fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL PROJECTILEFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_FLARESPRITE);
	if (!str.IsEmpty())
	{
		strncpy(szFlareSprite, (char*)(LPCSTR)str, ARRAY_LEN(szFlareSprite));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_SOUND);
	if (!str.IsEmpty())
	{
		strncpy(szSound, (char*)(LPCSTR)str, ARRAY_LEN(szSound));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_CLASS);
	if (!str.IsEmpty())
	{
		strncpy(szClass, (char*)(LPCSTR)str, ARRAY_LEN(szClass));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_CLASSDATA);
	if (!str.IsEmpty())
	{
		pClassData = g_pFXButeMgr->GetProjectileClassData((char*)(LPCSTR)str);
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_MODEL);
	if (!str.IsEmpty())
	{
		strncpy(szModel, (char*)(LPCSTR)str, ARRAY_LEN(szModel));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_SKIN);
	if (!str.IsEmpty())
	{
		strncpy(szSkin, (char*)(LPCSTR)str, ARRAY_LEN(szSkin));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	nSmokeTrailType	= buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_SMOKETRAILTYPE);
	nAltVelocity	= buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_ALTVELOCITY);
	nVelocity		= buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_VELOCITY);
	nLightRadius	= buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_LIGHTRADIUS);
	nSoundRadius	= buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_SOUNDRADIUS);
    fLifeTime       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PROJECTILEFX_LIFETIME);
    fFlareScale     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PROJECTILEFX_FLARESCALE);

	vLightColor	= buteMgr.GetVector(aTagName, FXBMGR_PROJECTILEFX_LIGHTCOLOR);
	vLightColor /= 255.0f;

	vModelScale	= buteMgr.GetVector(aTagName, FXBMGR_PROJECTILEFX_MODELSCALE);

	dwObjectFlags = 0;

	if (buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_GRAVITY))
	{
		dwObjectFlags |= FLAG_GRAVITY;
	}

	nFlags = 0;

	if (buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_SMOKETRAIL))
	{
		nFlags |= PFX_SMOKETRAIL;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_FLARE))
	{
		nFlags |= PFX_FLARE;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_LIGHT))
	{
		nFlags |= PFX_LIGHT;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_FLYSOUND))
	{
		nFlags |= PFX_FLYSOUND;
	}


	switch (nSmokeTrailType)
	{
		case 2 :
			nSmokeTrailType = PT_SMOKE_LONG;
		break;

		case 3 :
			nSmokeTrailType = PT_SMOKE_BLACK;
		break;

		case 1 :
		default :
			nSmokeTrailType = PT_SMOKE;
		break;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROJECTILEFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the projectile
//				fx struct
//
// ----------------------------------------------------------------------- //

void PROJECTILEFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	if (szFlareSprite[0])
	{
        g_pLTServer->CacheFile(FT_SPRITE, szFlareSprite);
	}

	if (szSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szSound);
	}

	if (szModel[0])
	{
        g_pLTServer->CacheFile(FT_MODEL, szModel);
	}

	if (szSkin[0])
	{
        g_pLTServer->CacheFile(FT_TEXTURE, szSkin);
	}

	if (pClassData)
	{
		pClassData->Cache(g_pFXButeMgr);
	}
#endif
}




/////////////////////////////////////////////////////////////////////////////
//
//	P R O J E C T I L E  C L A S S  D A T A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROJECTILECLASSDATA::Init
//
//	PURPOSE:	Build the projectile class data struct
//
// ----------------------------------------------------------------------- //

LTBOOL PROJECTILECLASSDATA::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	strncpy(szName, aTagName, ARRAY_LEN(szName));

    return LTTRUE;
}

/////////////////////////////////////////////////////////////////////////////
//
//	P R O X  C L A S S  D A T A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROXCLASSDATA::PROXCLASSDATA
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PROXCLASSDATA::PROXCLASSDATA()
{
	nActivateRadius		= 0;
	nArmSndRadius		= 0;
	nActivateSndRadius	= 0;

	fArmDelay		= 0.0f;
	fActivateDelay	= 0.0f;

	szArmSound[0]		= '\0';
	szActivateSound[0]	= '\0';
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROXCLASSDATA::Init
//
//	PURPOSE:	Build the prox class data class data struct
//
// ----------------------------------------------------------------------- //

LTBOOL PROXCLASSDATA::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;
    if (!PROJECTILECLASSDATA::Init(buteMgr, aTagName)) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_PROXCLASS_ARMSND);
	if (!str.IsEmpty())
	{
		strncpy(szArmSound, (char*)(LPCSTR)str, ARRAY_LEN(szArmSound));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PROXCLASS_ACTSND);
	if (!str.IsEmpty())
	{
		strncpy(szActivateSound, (char*)(LPCSTR)str, ARRAY_LEN(szActivateSound));
	}

	nActivateRadius		= buteMgr.GetInt(aTagName, FXBMGR_PROXCLASS_ACTRADIUS);
	nArmSndRadius		= buteMgr.GetInt(aTagName, FXBMGR_PROXCLASS_ARMSNDRADIUS);
	nActivateSndRadius	= buteMgr.GetInt(aTagName, FXBMGR_PROXCLASS_ACTSNDRADIUS);

    fArmDelay       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PROXCLASS_ARMDELAY);
    fActivateDelay  = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PROXCLASS_ACTDELAY);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROXCLASSDATA::Cache
//
//	PURPOSE:	Cache all the resources associated with the prox class
//				data struct
//
// ----------------------------------------------------------------------- //

void PROXCLASSDATA::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	PROJECTILECLASSDATA::Cache(pFXButeMgr);

	if (szArmSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szArmSound);
	}

	if (szActivateSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szActivateSound);
	}

#endif
}





/////////////////////////////////////////////////////////////////////////////
//
//	P V  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PVFX::PVFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PVFX::PVFX()
{
	nId		= FXBMGR_INVALID_ID;

	szName[0]	= '\0';
	szSocket[0] = '\0';

	nNumScaleFXTypes = 0;
    int i;
    for (i=0; i < PV_MAX_SCALEFX_TYPES; i++)
	{
		aScaleFXTypes[i] = FXBMGR_INVALID_ID;
	}

	nNumDLightFX = 0;
	for (i=0; i < PV_MAX_DLIGHTFX_TYPES; i++)
	{
		aDLightFXTypes[i] = FXBMGR_INVALID_ID;
	}

	nNumSoundFX = 0;
	for (i=0; i < PV_MAX_DLIGHTFX_TYPES; i++)
	{
		aSoundFXTypes[i] = FXBMGR_INVALID_ID;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PVFX::Init
//
//	PURPOSE:	Build the pv fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL PVFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_PVFX_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PVFX_SOCKET);
	if (!str.IsEmpty())
	{
		strncpy(szSocket, (char*)(LPCSTR)str, ARRAY_LEN(szSocket));
	}


	// Build our scale fx types id array...

	nNumScaleFXTypes = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_PVFX_SCALENAME, nNumScaleFXTypes);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumScaleFXTypes < PV_MAX_SCALEFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX((char*)(LPCSTR)str);
			if (pScaleFX)
			{
				aScaleFXTypes[nNumScaleFXTypes] = pScaleFX->nId;
			}
		}

		nNumScaleFXTypes++;
		sprintf(s_aAttName, "%s%d", FXBMGR_PVFX_SCALENAME, nNumScaleFXTypes);
	}


	// Build our dynamic light fx types id array...

	nNumDLightFX = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_PVFX_DLIGHTNAME, nNumDLightFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumDLightFX < PV_MAX_DLIGHTFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			DLIGHTFX* pDLightFX = g_pFXButeMgr->GetDLightFX((char*)(LPCSTR)str);
			if (pDLightFX)
			{
				aDLightFXTypes[nNumDLightFX] = pDLightFX->nId;
			}
		}

		nNumDLightFX++;
		sprintf(s_aAttName, "%s%d", FXBMGR_PVFX_DLIGHTNAME, nNumDLightFX);
	}

	// Build our sound fx types id array...

	nNumSoundFX = 0;
	sprintf(s_aAttName, "%s%d", FXBMGR_PVFX_SOUNDNAME, nNumSoundFX);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumSoundFX < PV_MAX_SOUNDFX_TYPES)
	{
		str = buteMgr.GetString(aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			SOUNDFX* pFX = g_pFXButeMgr->GetSoundFX((char*)(LPCSTR)str);
			if (pFX)
			{
				aSoundFXTypes[nNumSoundFX] = pFX->nId;
			}
		}

		nNumSoundFX++;
		sprintf(s_aAttName, "%s%d", FXBMGR_PVFX_SOUNDNAME, nNumSoundFX);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PVFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the pv
//				fx struct
//
// ----------------------------------------------------------------------- //

void PVFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
	if (!pFXButeMgr) return;

    int i;
    for (i=0; i < nNumScaleFXTypes; i++)
	{
		CScaleFX* pScaleFX = pFXButeMgr->GetScaleFX(aScaleFXTypes[i]);
		if (pScaleFX)
		{
			pScaleFX->Cache();
		}
	}

	for (i=0; i < nNumDLightFX; i++)
	{
		DLIGHTFX* pDLightFX = pFXButeMgr->GetDLightFX(aDLightFXTypes[i]);
		if (pDLightFX)
		{
			pDLightFX->Cache(pFXButeMgr);
		}
	}

	for (i=0; i < nNumSoundFX; i++)
	{
		SOUNDFX* pFX = pFXButeMgr->GetSoundFX(aSoundFXTypes[i]);
		if (pFX)
		{
			pFX->Cache(pFXButeMgr);
		}
	}
#endif
}



/////////////////////////////////////////////////////////////////////////////
//
//	P A R T I C L E  M U Z Z L E  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleMuzzleFX::CParticleMuzzleFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CParticleMuzzleFX::CParticleMuzzleFX()
{
	nId				= FXBMGR_INVALID_ID;
	szName[0]		= '\0';
	szFile[0]		= '\0';
	fLength			= 0.0f;
	fDuration		= 0.0f;
	fRadius			= 0.0f;
	fMaxScale		= 0.0f;
	nNumParticles	= 0;

    bAdditive       = LTFALSE;
    bMultiply       = LTFALSE;

	vColor1.Init();
	vColor2.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleMuzzleFX::Init
//
//	PURPOSE:	Build the particle muzzle fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleMuzzleFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_PARTMUZZLEFX_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_PARTMUZZLEFX_FILE);
	if (!str.IsEmpty())
	{
		strncpy(szFile, (char*)(LPCSTR)str, ARRAY_LEN(szFile));
	}

    fLength         = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_LENGTH);
    fDuration       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_DURATION);
    fRadius         = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_RADIUS);
    fMaxScale       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_MAXSCALE);
	nNumParticles	= buteMgr.GetInt(aTagName, FXBMGR_PARTMUZZLEFX_NUMBER);

    bAdditive       = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PARTMUZZLEFX_ADDITIVE);
    bMultiply       = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PARTMUZZLEFX_MULTIPLY);

	vColor1			= buteMgr.GetVector(aTagName, FXBMGR_PARTMUZZLEFX_COLOR1);
	vColor2			= buteMgr.GetVector(aTagName, FXBMGR_PARTMUZZLEFX_COLOR2);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleMuzzleFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the particle
//				muzzle fx struct
//
// ----------------------------------------------------------------------- //

void CParticleMuzzleFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
	if (!pFXButeMgr) return;

	if (szFile[0])
	{
		if (strstr(szFile, ".spr"))
		{
            g_pLTServer->CacheFile(FT_SPRITE, szFile);
		}
		else
		{
            g_pLTServer->CacheFile(FT_TEXTURE, szFile);
		}
	}

#endif
}




/////////////////////////////////////////////////////////////////////////////
//
//	M U Z Z L E  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFX::CMuzzleFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMuzzleFX::CMuzzleFX()
{
	nId				= FXBMGR_INVALID_ID;
	szName[0]		= '\0';
	fDuration		= 0.0f;

    pPMuzzleFX      = LTNULL;
    pScaleFX        = LTNULL;
    pDLightFX       = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFX::Init
//
//	PURPOSE:	Build the muzzle fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL CMuzzleFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_MUZZLEFX_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_MUZZLEFX_PMUZZLEFXNAME);
	if (!str.IsEmpty())
	{
		pPMuzzleFX = g_pFXButeMgr->GetParticleMuzzleFX((char*)(LPCSTR)str);
	}

	str = buteMgr.GetString(aTagName, FXBMGR_MUZZLEFX_SCALEFXNAME);
	if (!str.IsEmpty())
	{
		pScaleFX = g_pFXButeMgr->GetScaleFX((char*)(LPCSTR)str);
	}

	str = buteMgr.GetString(aTagName, FXBMGR_MUZZLEFX_DLIGHTFXNAME);
	if (!str.IsEmpty())
	{
		pDLightFX = g_pFXButeMgr->GetDLightFX((char*)(LPCSTR)str);
	}

    fDuration = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_MUZZLEFX_DURATION);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the muzzle fx struct
//
// ----------------------------------------------------------------------- //

void CMuzzleFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
	if (!pFXButeMgr) return;

	if (pPMuzzleFX)
	{
		pPMuzzleFX->Cache(pFXButeMgr);
	}

	if (pScaleFX)
	{
		pScaleFX->Cache();
	}

	if (pDLightFX)
	{
		pDLightFX->Cache(pFXButeMgr);
	}
#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//	T R A C E R  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TRACERFX::TRACERFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

TRACERFX::TRACERFX()
{
	nId				= FXBMGR_INVALID_ID;
	szName[0]		= '\0';
	szTexture[0]	= '\0';

	nFrequency		= 1;
	fVelocity		= 0.0f;
	fWidth			= 0.0f;
	fInitialAlpha	= 0.0f;
	fFinalAlpha		= 0.0f;
	vColor.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TRACERFX::Init
//
//	PURPOSE:	Build the tracer fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL TRACERFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_TRACERFX_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_TRACERFX_TEXTURE);
	if (!str.IsEmpty())
	{
		strncpy(szTexture, (char*)(LPCSTR)str, ARRAY_LEN(szTexture));
	}

	nFrequency		= buteMgr.GetInt(aTagName, FXBMGR_TRACERFX_FREQUENCY);
    fVelocity       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_TRACERFX_VELOCITY);
    fWidth          = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_TRACERFX_WIDTH);
    fInitialAlpha   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_TRACERFX_INITIALALPHA);
    fFinalAlpha     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_TRACERFX_FINALALPHA);
	vColor			= buteMgr.GetVector(aTagName, FXBMGR_TRACERFX_COLOR);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TRACERFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the tracer fx struct
//
// ----------------------------------------------------------------------- //

void TRACERFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
	if (szTexture[0])
	{
		g_pLTServer->CacheFile(FT_TEXTURE, szTexture);
	}
#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//  B E A M  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BEAMFX::BEAMFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

BEAMFX::BEAMFX()
{
	nId				= FXBMGR_INVALID_ID;
	szName[0]		= '\0';
	szTexture[0]	= '\0';

	fDuration		= 0.0f;
	fWidth			= 0.0f;
	fInitialAlpha	= 0.0f;
	fFinalAlpha		= 0.0f;
	vColor.Init();
	bAlignUp		= LTFALSE;
	bAlignFlat		= LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BEAMFX::Init
//
//	PURPOSE:	Build the beam fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL BEAMFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_BEAMFX_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_BEAMFX_TEXTURE);
	if (!str.IsEmpty())
	{
		strncpy(szTexture, (char*)(LPCSTR)str, ARRAY_LEN(szTexture));
	}

    fDuration       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_BEAMFX_DURATION);
    fWidth          = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_BEAMFX_WIDTH);
    fInitialAlpha   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_BEAMFX_INITIALALPHA);
    fFinalAlpha     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_BEAMFX_FINALALPHA);
	vColor			= buteMgr.GetVector(aTagName, FXBMGR_BEAMFX_COLOR);
    bAlignUp        = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_BEAMFX_ALIGNUP);
    bAlignFlat      = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_BEAMFX_ALIGNFLAT);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BEAMFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the beam fx struct
//
// ----------------------------------------------------------------------- //

void BEAMFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
	if (szTexture[0])
	{
		g_pLTServer->CacheFile(FT_TEXTURE, szTexture);
	}
#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//  S O U N D  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SOUNDFX::SOUNDFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SOUNDFX::SOUNDFX()
{
	nId				= FXBMGR_INVALID_ID;
	szName[0]		= '\0';
	szFile[0]		= '\0';

	fRadius			= 0.0f;
	fPitchShift		= 1.0f;
	bLoop			= LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SOUNDFX::Init
//
//	PURPOSE:	Build the sound fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL SOUNDFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_SOUNDFX_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, FXBMGR_SOUNDFX_FILE);
	if (!str.IsEmpty())
	{
		strncpy(szFile, (char*)(LPCSTR)str, ARRAY_LEN(szFile));
	}

    fRadius     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_SOUNDFX_RADIUS);
    fPitchShift	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_SOUNDFX_PITCHSHIFT);
    bLoop		= (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_SOUNDFX_LOOP);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SOUNDFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the sound fx struct
//
// ----------------------------------------------------------------------- //

void SOUNDFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	if (szFile[0] && strstr(szFile, ".wav"))
	{
        g_pLTServer->CacheFile(FT_SOUND, szFile);
	}

#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//  P U S H E R  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PUSHERFX::PUSHERFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PUSHERFX::PUSHERFX()
{
	nId				= FXBMGR_INVALID_ID;
	szName[0]		= '\0';

	fRadius			= 0.0f;
	fStartDelay		= 0.0f;
	fDuration		= 0.0f;
	fStrength		= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PUSHERFX::Init
//
//	PURPOSE:	Build the pusher fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL PUSHERFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	CString str = buteMgr.GetString(aTagName, FXBMGR_PUSHERFX_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

    fRadius		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PUSHERFX_RADIUS);
    fStartDelay = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PUSHERFX_STARTDELAY);
    fDuration	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PUSHERFX_DURATION);
    fStrength	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PUSHERFX_STRENGTH);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PUSHERFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the pusher fx struct
//
// ----------------------------------------------------------------------- //

void PUSHERFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
#endif
}
















/////////////////////////////////////////////////////////////////////////////
//
//	C L I E N T - S I D E  U T I L I T Y  F U N C T I O N S
//
/////////////////////////////////////////////////////////////////////////////

#if defined(_CLIENTBUILD)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreateScaleFX()
//
//	PURPOSE:	Create a scale fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreateScaleFX(CScaleFX* pScaleFX, LTVector vPos,
                                     LTVector vDir, LTVector* pvSurfaceNormal,
                                     LTRotation* prRot, CBaseScaleFX* pFX)
{
    if (!pScaleFX || !pScaleFX->szFile[0]) return LTNULL;

	// Create scale fx...

	BSCREATESTRUCT scale;
	scale.dwFlags = FLAG_VISIBLE | FLAG_NOLIGHT; // | FLAG_NOGLOBALLIGHTSCALE;

	if (prRot)
	{
		scale.rRot = *prRot;
	}

	// Set up fx flags...

	if (pScaleFX->eType == SCALEFX_SPRITE)
	{
		if (pScaleFX->bAlignToSurface && pvSurfaceNormal)
		{
			scale.dwFlags |= FLAG_ROTATEABLESPRITE;
            g_pLTClient->AlignRotation(&(scale.rRot), pvSurfaceNormal, LTNULL);
		}
		else if (!pScaleFX->bReallyClose)
		{
			// We want FLAG_REALLYCLOSE to override the default
			// FLAG_SPRITEBIAS...

			scale.dwFlags |= FLAG_SPRITEBIAS;
		}

		if (pScaleFX->bNoZ)
		{
			scale.dwFlags |= FLAG_SPRITE_NOZ;
		}

		if (pScaleFX->bRotate)
		{
			scale.dwFlags |= FLAG_ROTATEABLESPRITE;
		}
	}
	else
	{
		scale.bChromakey = pScaleFX->bChromakey;
	}

	if (pScaleFX->bReallyClose)
	{
		scale.dwFlags |= FLAG_REALLYCLOSE;

		// Get the position into camera-relative space...

		HOBJECT hCam = LTNULL;
		if (g_pInterfaceMgr->UseInterfaceCamera())
			hCam = g_pGameClientShell->GetInterfaceCamera();
		else
			hCam = g_pGameClientShell->GetCamera();

        LTVector vOffset;
        g_pLTClient->GetObjectPos(hCam, &vOffset);

		vPos -= vOffset;
	}

	// Adjust the position based on the offsets...

	scale.vPos = vPos + (vDir * pScaleFX->fDirOffset);

	if (pScaleFX->fDirROffset || pScaleFX->fDirUOffset)
	{
        LTRotation rTempRot;
		rTempRot.Init();
        LTVector vUp(0, 1, 0), vR, vF, vU;
        g_pLTClient->AlignRotation(&rTempRot, &vDir, &vUp);
        g_pLTClient->GetRotationVectors(&rTempRot, &vU, &vR, &vF);

		scale.vPos += (vR * pScaleFX->fDirROffset);
		scale.vPos += (vU * pScaleFX->fDirUOffset);
	}

	scale.pFilename			= pScaleFX->szFile;
	scale.pSkin				= pScaleFX->szSkin;
	scale.vVel				= pScaleFX->vVel;
	scale.vInitialScale		= pScaleFX->vInitialScale;
	scale.vFinalScale		= pScaleFX->vFinalScale;
	scale.vInitialColor		= pScaleFX->vInitialColor;
	scale.vFinalColor		= pScaleFX->vFinalColor;
	scale.bUseUserColors	= pScaleFX->bUseColors;
	scale.fLifeTime			= pScaleFX->fLifeTime;
	scale.fInitialAlpha		= pScaleFX->fInitialAlpha;
	scale.fFinalAlpha		= pScaleFX->fFinalAlpha;
	scale.bLoop				= pScaleFX->bLoop;
	scale.fDelayTime		= pScaleFX->fDelayTime;
	scale.bAdditive			= pScaleFX->bAdditive;
	scale.bMultiply			= pScaleFX->bMultiply;
	scale.nType				= (pScaleFX->eType == SCALEFX_MODEL) ? OT_MODEL : OT_SPRITE;
    scale.bUseUserColors    = LTTRUE;
	scale.bFaceCamera		= pScaleFX->bFaceCamera;
	scale.nRotationAxis		= pScaleFX->nRotationAxis;
	scale.bRotate			= pScaleFX->bRotate;
	scale.fMinRotateVel		= pScaleFX->fMinRotVel;
	scale.fMaxRotateVel		= pScaleFX->fMaxRotVel;

	if (!pFX)
	{
		CSpecialFX* pNewFX = g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_SCALE_ID, &scale);
		if (pNewFX)
		{
			pNewFX->Update();
		}

		return pNewFX;
	}
	else
	{
		pFX->Init(&scale);
        pFX->CreateObject(g_pLTClient);
	}

	return pFX;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreatePShowerFX()
//
//	PURPOSE:	Create a particle shower fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreatePShowerFX(CPShowerFX* pPShowerFX, LTVector vPos,
                                        LTVector vDir, LTVector vSurfaceNormal)
{
    if (!pPShowerFX || !pPShowerFX->szTexture[0]) return LTNULL;

	// Create particle shower fx...

	PARTICLESHOWERCREATESTRUCT ps;

	ps.vPos				= vPos + (vDir * pPShowerFX->fDirOffset);
	ps.vDir				= (vSurfaceNormal * GetRandom(pPShowerFX->fMinVel, pPShowerFX->fMaxVel));
	ps.vColor1			= pPShowerFX->vColor1;
	ps.vColor2			= pPShowerFX->vColor2;
	ps.pTexture			= pPShowerFX->szTexture;
	ps.nParticles		= GetRandom(pPShowerFX->nMinParticles, pPShowerFX->nMaxParticles);
	ps.fDuration		= GetRandom(pPShowerFX->fMinDuration, pPShowerFX->fMaxDuration);
	ps.fEmissionRadius	= pPShowerFX->fEmissionRadius;
	ps.fRadius			= pPShowerFX->fRadius;
	ps.fGravity			= pPShowerFX->fGravity;
	ps.bAdditive		= pPShowerFX->bAdditive;
	ps.bMultiply		= pPShowerFX->bMultiply;

	return g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_PARTICLESHOWER_ID, &ps);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreatePolyDebrisFX()
//
//	PURPOSE:	Create a poly debris fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreatePolyDebrisFX(CPolyDebrisFX* pPolyDebrisFX, LTVector vPos,
                                          LTVector vDir, LTVector vSurfaceNormal)
{
    if (!pPolyDebrisFX) return LTNULL;

	POLYDEBRISCREATESTRUCT pdebris;

	pdebris.vNormal			= vSurfaceNormal;
	pdebris.vPos			= vPos;
	pdebris.vDir			= vDir;
	pdebris.PolyDebrisFX	= *pPolyDebrisFX;

	return g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_POLYDEBRIS_ID, &pdebris);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreateBeamFX()
//
//	PURPOSE:	Create a beam fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreateBeamFX(BEAMFX* pBeamFX, LTVector vStartPos,
                                     LTVector vEndPos)
{
    if (!pBeamFX) return LTNULL;

	BEAMCREATESTRUCT beam;

	beam.vStartPos		= vStartPos;
	beam.vEndPos		= vEndPos;
	beam.pBeamFX		= pBeamFX;

	return g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_BEAM_ID, &beam);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreateSoundFX()
//
//	PURPOSE:	Create a sound fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreateSoundFX(SOUNDFX* pSoundFX, LTVector vPos,
                                     CSoundFX* pFX)
{
    if (!pSoundFX) return LTNULL;

	SNDCREATESTRUCT snd;

	snd.bLocal		= vPos.Equals(LTVector(0,0,0)) ? LTTRUE : LTFALSE;
	snd.bLoop		= pSoundFX->bLoop;
	snd.fPitchShift	= pSoundFX->fPitchShift;
	snd.fRadius		= pSoundFX->fRadius;
	snd.pSndName	= pSoundFX->szFile;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
    if (!psfxMgr) return LTNULL;

	if (pFX)
	{
		pFX->Init(&snd);
        pFX->CreateObject(g_pLTClient);

		return pFX;
	}

	return psfxMgr->CreateSFX(SFX_SOUND_ID, &snd);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreatePusherFX()
//
//	PURPOSE:	Create a pusher fx
//
// ----------------------------------------------------------------------- //

void CFXButeMgr::CreatePusherFX(PUSHERFX* pPusherFX, LTVector vPos)
{
    if (!pPusherFX) return;

	g_pGameClientShell->GetMoveMgr()->AddPusher(vPos, pPusherFX->fRadius,
		pPusherFX->fStartDelay, pPusherFX->fDuration, pPusherFX->fStrength);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreatePExplFX()
//
//	PURPOSE:	Create a paritlce explosion specific fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreatePExplFX(PEXPLFX* pPExplFX, LTRotation rSurfaceRot,
                                     SurfaceType eSurfaceType, LTVector vPos,
									 ContainerCode eCode)
{
    if (!pPExplFX) return LTNULL;

	// Create a particle explosion...

	PESCREATESTRUCT pe;
    pe.rSurfaceRot = rSurfaceRot;

	pe.nSurfaceType		= eSurfaceType;
	pe.vPos				= vPos + pPExplFX->vPosOffset;
	pe.bCreateDebris	= pPExplFX->bCreateDebris;
	pe.bRotateDebris	= pPExplFX->bRotateDebris;
	pe.bIgnoreWind		= pPExplFX->bIgnoreWind;
	pe.vColor1			= pPExplFX->vColor1;
	pe.vColor2			= pPExplFX->vColor2;
	pe.vMinVel			= pPExplFX->vMinVel;
	pe.vMaxVel			= pPExplFX->vMaxVel;
	pe.vMinDriftVel		= pPExplFX->vMinDriftVel;
	pe.vMaxDriftVel		= pPExplFX->vMaxDriftVel;
	pe.fLifeTime		= pPExplFX->fLifeTime;
	pe.fFadeTime		= pPExplFX->fFadeTime;
	pe.fOffsetTime		= pPExplFX->fOffsetTime;
	pe.fRadius			= pPExplFX->fRadius;
	pe.fGravity			= pPExplFX->fGravity;
	pe.nNumPerPuff		= pPExplFX->nNumPerPuff;
	pe.nNumEmitters		= pPExplFX->nNumEmitters;
	pe.nNumSteps		= pPExplFX->nNumSteps;
	pe.pFilename		= pPExplFX->szFile;
	pe.bAdditive		= pPExplFX->bAdditive;
	pe.bMultiply		= pPExplFX->bMultiply;

	if (IsLiquid(eCode) && pPExplFX->bDoBubbles)
	{
		GetLiquidColorRange(eCode, &pe.vColor1, &pe.vColor2);
		pe.pFilename = DEFAULT_BUBBLE_TEXTURE;
	}

	CSpecialFX* pFX = g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_PARTICLEEXPLOSION_ID, &pe);
	if (pFX) pFX->Update();

	return pFX;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreateDLightFX()
//
//	PURPOSE:	Create a dynamic light specific fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreateDLightFX(DLIGHTFX* pDLightFX, LTVector vPos,
									   CDynamicLightFX* pFX)
{
    if (!pDLightFX) return LTNULL;

	DLCREATESTRUCT dl;
	dl.vPos			 = vPos;
	dl.vColor		 = pDLightFX->vColor;
	dl.fMinRadius    = pDLightFX->fMinRadius;
	dl.fMaxRadius	 = pDLightFX->fMaxRadius;
	dl.fRampUpTime	 = pDLightFX->fRampUpTime;
	dl.fMaxTime		 = pDLightFX->fMaxTime;
	dl.fMinTime		 = pDLightFX->fMinTime;
	dl.fRampDownTime = pDLightFX->fRampDownTime;
	dl.dwFlags		 = FLAG_VISIBLE | FLAG_DONTLIGHTBACKFACING;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
    if (!psfxMgr) return LTNULL;

	if (pFX)
	{
		pFX->Init(&dl);
        pFX->CreateObject(g_pLTClient);

		return pFX;
	}

	return psfxMgr->CreateSFX(SFX_DYNAMICLIGHT_ID, &dl);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreateImpactFX()
//
//	PURPOSE:	Create the specified impact fx
//
// ----------------------------------------------------------------------- //

void CFXButeMgr::CreateImpactFX(IMPACTFX* pImpactFX, IFXCS & cs)
{
	// Sanity checks...

	if (!pImpactFX) return;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
	if (!pSettings) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	int nDetailLevel = pSettings->SpecialFXSetting();

	// Add a pusher if necessary...

	CreatePusherFX(pImpactFX->pPusherFX, cs.vPos);

	// Play the impact sound if appropriate...

	if (cs.bPlaySound && pImpactFX->szSound[0])
	{
        LTFLOAT fSndRadius = (LTFLOAT) pImpactFX->nSoundRadius;

		g_pClientSoundMgr->PlaySoundFromPos(cs.vPos, pImpactFX->szSound, fSndRadius,
				SOUNDPRIORITY_MISC_MEDIUM);
	}


	// See if we should ignore certain surfaces...

	if (cs.eSurfType == ST_FLESH && pImpactFX->bIgnoreFlesh) return;
	if (cs.eSurfType == ST_LIQUID && pImpactFX->bIgnoreLiquid) return;


	// Create any debris fx...

    int i;
    for (i=0; i < pImpactFX->nNumDebrisFXTypes; i++)
	{
		DEBRISCREATESTRUCT debris;
		debris.rRot			= cs.rSurfRot;
		debris.vPos			= cs.vPos;
		debris.nDebrisId	= pImpactFX->aDebrisFXTypes[i];

		psfxMgr->CreateSFX(SFX_DEBRIS_ID, &debris);
	}

	// Create any scale fx...

	for (i=0; i < pImpactFX->nNumScaleFXTypes; i++)
	{
		CScaleFX* pScaleFX = GetScaleFX(pImpactFX->aScaleFXTypes[i]);
		if (pScaleFX)
		{
			CreateScaleFX(pScaleFX, cs.vPos, cs.vDir, &(cs.vSurfNormal), &(cs.rSurfRot));
		}
	}

	// Create the particle explosion fx...

	for (i=0; i < pImpactFX->nNumPExplFX; i++)
	{
		PEXPLFX* pPExplFX = GetPExplFX(pImpactFX->aPExplFXTypes[i]);
		if (pPExplFX)
		{
			CreatePExplFX(pPExplFX, cs.rSurfRot, cs.eSurfType, cs.vPos, cs.eCode);
		}
	}

	// Create the poly debris fx...

	for (i=0; i < pImpactFX->nNumPolyDebrisFX; i++)
	{
		CPolyDebrisFX* pPolyDebrisFX = GetPolyDebrisFX(pImpactFX->aPolyDebrisFXTypes[i]);
		if (pPolyDebrisFX)
		{
			CreatePolyDebrisFX(pPolyDebrisFX, cs.vPos, cs.vDir, cs.vSurfNormal);
		}
	}

	// Create the particle shower fx...

	for (i=0; i < pImpactFX->nNumPShowerFX; i++)
	{
		CPShowerFX* pPShowerFX = GetPShowerFX(pImpactFX->aPShowerFXIds[i]);
		if (pPShowerFX)
		{
			CreatePShowerFX(pPShowerFX, cs.vPos, cs.vDir, cs.vSurfNormal);
		}
	}

	// Create the dynamic light fx...

	if (nDetailLevel != RS_LOW)
	{
		for (int i=0; i < pImpactFX->nNumDLightFX; i++)
		{
			DLIGHTFX* pDLightFX = GetDLightFX(pImpactFX->aDLightFXTypes[i]);
			if (pDLightFX)
			{
				CreateDLightFX(pDLightFX, cs.vPos);
			}
		}
	}

	// Create blast mark if appropriate...

	if ((pImpactFX->nFlags & WFX_BLASTMARK) && ShowsMark(cs.eSurfType) &&
		(nDetailLevel == RS_HIGH) && cs.fBlastRadius >= FXBMGR_MIN_BLASTMARK_RADIUS)
	{
		// Create a dynamic light for the blast mark...

		DLCREATESTRUCT dl;

		dl.vPos			 = cs.vPos;
		dl.vColor		 = pImpactFX->vBlastColor;
		dl.fMinRadius    = cs.fBlastRadius / 2.0f;
		dl.fMaxRadius	 = cs.fBlastRadius / 4.0f;
		dl.fRampUpTime	 = 0.0f;
		dl.fMaxTime		 = GetRandom(pImpactFX->fBlastTimeMin, pImpactFX->fBlastTimeMax);
		dl.fMinTime		 = 0.0f;
		dl.fRampDownTime = GetRandom(pImpactFX->fBlastFadeMin, pImpactFX->fBlastFadeMax);
		dl.dwFlags		 = FLAG_VISIBLE | FLAG_ONLYLIGHTWORLD | FLAG_DONTLIGHTBACKFACING;

		psfxMgr->CreateSFX(SFX_DYNAMICLIGHT_ID, &dl);
	}

	// Tint screen if appropriate...

	if ((pImpactFX->nFlags & WFX_TINTSCREEN))
	{
        LTVector vTintColor  = pImpactFX->vTintColor;
        LTFLOAT fRampUp      = pImpactFX->fTintRampUp;
        LTFLOAT fRampDown    = pImpactFX->fTintRampDown;
        LTFLOAT fTintTime    = pImpactFX->fTintMaxTime;

		g_pGameClientShell->FlashScreen(vTintColor, cs.vPos, cs.fTintRange,
			fRampUp, fTintTime, fRampDown);

		// If close enough, shake the screen...

        HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if (hPlayerObj)
		{
            LTVector vPlayerPos, vDir;
            g_pLTClient->GetObjectPos(hPlayerObj, &vPlayerPos);

			vDir = vPlayerPos - cs.vPos;
            LTFLOAT fDist = vDir.Mag();

            LTFLOAT fRadius = cs.fBlastRadius;

			if (fDist < fRadius * 2.0f)
			{
                LTFLOAT fVal = fDist < 1.0f ? 3.0f : fRadius / fDist;
				fVal = fVal > 3.0f ? 3.0f : fVal;

                LTVector vShake(fVal, fVal, fVal);
				g_pGameClientShell->ShakeScreen(vShake);
			}
		}
	}
}
#endif

#ifndef _CLIENTBUILD

/////////////////////////////////////////////////////////////////////////////
//
//	S E R V E R - S I D E  U T I L I T Y  F U N C T I O N S
//
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::ReadImpactFXProp
//
//	PURPOSE:	Read in the impact fx properties
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgr::ReadImpactFXProp(char* pPropName, uint8 & nImpactFXId)
{
    if (!pPropName || !pPropName[0]) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric(pPropName, &genProp) == LT_OK)
	{
		// Get the impact fx

		IMPACTFX* pImpactFX = GetImpactFX(genProp.m_String);
		if (pImpactFX)
		{
			nImpactFXId = pImpactFX->nId;
		}

        return LTTRUE;
	}

    return LTFALSE;
}

////////////////////////////////////////////////////////////////////////////
//
// CFXButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use FXButeMgr
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT CFXButeMgrPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (!g_pFXButeMgr)
	{
		// Make sure debris mgr is inited...

		m_DebrisMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
			aszStrings,	pcStrings, cMaxStrings, cMaxStringLength);

		// This will set the g_pFXButeMgr...Since this could also be
		// set elsewhere, just check for the global bute mgr...

		char szFile[256];
		sprintf(szFile, "%s\\%s", szRezPath, FXBMGR_DEFAULT_FILE);
        sm_FXButeMgr.SetInRezFile(LTFALSE);
        sm_FXButeMgr.Init(g_pLTServer, szFile);
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgrPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgrPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
    if (!aszStrings || !pcStrings) return LTFALSE;
	_ASSERT(aszStrings && pcStrings);

	// Add an entry for each impact fx

	int nNumImpactFX = g_pFXButeMgr->GetNumImpactFX();
	_ASSERT(nNumImpactFX > 0);

    IMPACTFX* pImpactFX = LTNULL;

	for (int i=0; i < nNumImpactFX; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pImpactFX = g_pFXButeMgr->GetImpactFX(i);
        uint32 dwImpactFXNameLen = strlen(pImpactFX->szName);

		if (pImpactFX && pImpactFX->szName[0] &&
			dwImpactFXNameLen < cMaxStringLength &&
			((*pcStrings) + 1) < cMaxStrings)
		{
			strcpy(aszStrings[(*pcStrings)++], pImpactFX->szName);
		}
	}

    return LTTRUE;
}

#endif // _CLIENTBUILD