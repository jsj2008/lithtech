// ----------------------------------------------------------------------- //
//
// MODULE  : DamageFXMgr.cpp
//
// PURPOSE : Damage FX Manager class - Implementation
//
// CREATED : 1/20/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DamageFXMgr.h"
#include "VarTrack.h"
#include "SoundMgr.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

#define DM_BLEEDING_TAG					"Bleeding"
#define DM_POISON_TAG					"Poison"
#define DM_STUN_TAG						"Stun"
#define	DM_SLEEP_TAG					"Sleeping"
#define	DM_BURN_TAG						"Burn"
#define	DM_ELECTRO_TAG					"Electrocute"
#define	DM_CHOKE_TAG					"Choke"
#define	DM_SPRINKLES_TAG				"Sprinkles"

#define	DM_BLEEDING_SOUND				"Sound"

#define	DM_POISON_FOVMAX				"FOVMax"
#define	DM_POISON_FOVX_SPD				"FOVXSpeed"
#define	DM_POISON_FOVY_SPD				"FOVYSpeed"
#define	DM_POISON_COLOR_SPD				"ColorSpeed"
#define	DM_POISON_COLOR_R				"ColorR"
#define	DM_POISON_COLOR_G				"ColorG"
#define	DM_POISON_COLOR_B				"ColorB"
#define	DM_POISON_ROT_SPD				"RotSpeed"
#define	DM_POISON_ROT_MAX				"RotMax"
#define	DM_POISON_SOUND					"Sound"

#define	DM_STUN_SPD						"Speed"
#define	DM_STUN_MAX						"MaxBrightness"
#define	DM_STUN_MIN						"MinBrightness"
#define	DM_STUN_SOUND					"Sound"

#define	DM_SLEEP_SOUND					"StartSound"
#define	DM_SLEEP_LOOP					"LoopSound"
#define	DM_SLEEP_FOVMAX					"FOVMax"
#define	DM_SLEEP_FOV_SPD				"FOVSpeed"
#define	DM_SLEEP_SPD					"DarkenSpeed"

#define	DM_BURN_SOUND					"Sound"
#define	DM_BURN_COLOR					"Color"
#define	DM_BURN_SPD1					"Speed1"
#define	DM_BURN_SPD2					"Speed2"

#define	DM_CHOKE_SOUND					"Sound"

#define	DM_ELECTRO_SOUND				"Sound"
#define	DM_ELECTRO_COLOR				"Color"
#define	DM_ELECTRO_SPD1					"Speed1"
#define	DM_ELECTRO_SPD2					"Speed2"

#define	DM_SPRINKLES_FILENAME			"Filename"
#define	DM_SPRINKLES_SKINNAME			"SkinName"
#define	DM_SPRINKLES_COUNT				"Count"
#define	DM_SPRINKLES_SPEED				"Speed"
#define	DM_SPRINKLES_SIZE				"Size"
#define	DM_SPRINKLES_SPAWN				"SpawnRadius"
#define	DM_SPRINKLES_COLORMAX			"ColorMax"
#define	DM_SPRINKLES_COLORMIN			"ColorMin"
#define	DM_SPRINKLES_ANGLES				"AnglesVel"


VarTrack	g_vtPoisonFOVMax;
VarTrack	g_vtPoisonFOVXSpeed;
VarTrack	g_vtPoisonFOVYSpeed;
VarTrack	g_vtPoisonColorSpeed;
VarTrack	g_vtPoisonColorR;
VarTrack	g_vtPoisonColorG;
VarTrack	g_vtPoisonColorB;
VarTrack	g_vtPoisonRotSpeed;
VarTrack	g_vtPoisonRotMax;

VarTrack	g_vtStunSpeed;
VarTrack	g_vtStunMax;
VarTrack	g_vtStunMin;

VarTrack	g_vtSleepFOVMax;
VarTrack	g_vtSleepFOVSpeed;
VarTrack	g_vtSleepDarkenSpeed;

VarTrack	g_vtBurnSpeed1;
VarTrack	g_vtBurnSpeed2;

VarTrack	g_vtShockSpeed1;
VarTrack	g_vtShockSpeed2;

VarTrack	g_vtTestBleedingFX;
VarTrack	g_vtTestPoisonFX;
VarTrack	g_vtTestStunFX;
VarTrack	g_vtTestSleepingFX;
VarTrack	g_vtTestBurnFX;
VarTrack	g_vtTestChokeFX;
VarTrack	g_vtTestElectrocuteFX;
VarTrack	g_vtEnableDamageFX;

LTFLOAT  m_fSleepFOVOffset = 0.0f;
LTFLOAT  m_fSleepFOVDir = 1.0f;
LTFLOAT  m_fSleepDark   = 0.0f;
LTVector m_vDark;

LTFLOAT  m_fBurn1 = 0.0f;
LTFLOAT  m_fBurn2 = 0.0f;
LTFLOAT  m_fBurnDir1 = 1.0f;
LTFLOAT  m_fBurnDir2 = 1.0f;

LTFLOAT  m_fShock1 = 0.0f;
LTFLOAT  m_fShock2 = 0.0f;
LTFLOAT  m_fShockDir1 = 1.0f;
LTFLOAT  m_fShockDir2 = 1.0f;

static char s_aTagName[30];

extern VarTrack g_vtFOVXNormal;
extern VarTrack g_vtFOVYNormal;

CDamageFXMgr::CDamageFXMgr()
{
    m_bBleeding = LTFALSE;
    m_bPoison = LTFALSE;
    m_bStun = LTFALSE;
    m_bSleeping = LTFALSE;
    m_bBurn = LTFALSE;
    m_bChoke = LTFALSE;
    m_bElectrocute = LTFALSE;

    m_bBleedingFade = LTFALSE;
    m_bPoisonFade = LTFALSE;
    m_bStunFade = LTFALSE;
    m_bSleepingFade = LTFALSE;
    m_bBurnFade = LTFALSE;
    m_bChokeFade = LTFALSE;
    m_bElectrocuteFade = LTFALSE;

	m_vPoisonColor.Init(0.0f, 0.0f, 0.0f);
	m_vStunColor.Init(0.0f, 0.0f, 0.0f);
	m_vBurnColor.Init(0.0f, 0.0f, 0.0f);
	m_vElectrocuteColor.Init(0.0f, 0.0f, 0.0f);

	m_fFOVXOffset = 0.0f;
	m_fFOVYOffset = 0.0f;
	m_fFOVXDir = 1.0f;
	m_fFOVYDir = 1.0f;
	m_fColorRDir = 1.0f;
	m_fColorGDir = 1.0f;
	m_fColorBDir = 1.0f;
	m_fRotDir = 1.0f;
	m_fOffsetRot = 0.0f;
	m_fMaxRot = 1.0f;
	m_fMinRot = -1.0f;
	m_fMoveMult = 0.5f;
	m_fStunDir = 1.0f;

	m_fSleepFOVOffset = 0.0f;
	m_fSleepFOVDir = 1.0f;
	m_fSleepDark   = 0.0f;

    m_hBleedingSound = LTNULL;
    m_hPoisonSound = LTNULL;
    m_hStunSound = LTNULL;
    m_hSleepingSound = LTNULL;
    m_pSprinkles = LTNULL;
    m_hBurnSound = LTNULL;
    m_hChokeSound = LTNULL;
    m_hElectrocuteSound = LTNULL;

	m_nNumSprinkles = 0;
}

CDamageFXMgr::~CDamageFXMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDamageFXMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CDamageFXMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
    if (!szAttributeFile) return LTFALSE;
    if (!Parse(pInterface, szAttributeFile)) return LTFALSE;

    g_vtPoisonFOVMax.Init(g_pLTClient, "PoisonFOVMax", NULL, GetFloat(DM_POISON_TAG,DM_POISON_FOVMAX));
    g_vtPoisonFOVXSpeed.Init(g_pLTClient, "PoisonFOVXSpeed", NULL, GetFloat(DM_POISON_TAG,DM_POISON_FOVX_SPD));
    g_vtPoisonFOVYSpeed.Init(g_pLTClient, "PoisonFOVYSpeed", NULL, GetFloat(DM_POISON_TAG,DM_POISON_FOVY_SPD));
    g_vtPoisonColorSpeed.Init(g_pLTClient, "PoisonColorSpeed", NULL, GetFloat(DM_POISON_TAG,DM_POISON_COLOR_SPD));
    g_vtPoisonColorR.Init(g_pLTClient, "PoisonColorR", NULL, GetFloat(DM_POISON_TAG,DM_POISON_COLOR_R));
    g_vtPoisonColorG.Init(g_pLTClient, "PoisonColorG", NULL, GetFloat(DM_POISON_TAG,DM_POISON_COLOR_G));
    g_vtPoisonColorB.Init(g_pLTClient, "PoisonColorB", NULL, GetFloat(DM_POISON_TAG,DM_POISON_COLOR_B));
    g_vtPoisonRotSpeed.Init(g_pLTClient, "PoisonRotSpeed", NULL, GetFloat(DM_POISON_TAG,DM_POISON_ROT_SPD));
    g_vtPoisonRotMax.Init(g_pLTClient, "PoisonRotMax", NULL, GetFloat(DM_POISON_TAG,DM_POISON_ROT_MAX));

    g_vtStunSpeed.Init(g_pLTClient, "StunSpeed", NULL, GetFloat(DM_STUN_TAG,DM_STUN_SPD));
    g_vtStunMax.Init(g_pLTClient, "StunMax", NULL, GetFloat(DM_STUN_TAG,DM_STUN_MAX));
    g_vtStunMin.Init(g_pLTClient, "StunMin", NULL, GetFloat(DM_STUN_TAG,DM_STUN_MIN));

    g_vtSleepFOVMax.Init(g_pLTClient, "SleepFOVMax", NULL, GetFloat(DM_SLEEP_TAG,DM_SLEEP_FOVMAX));
    g_vtSleepFOVSpeed.Init(g_pLTClient, "SleepFOVSpeed", NULL, GetFloat(DM_SLEEP_TAG,DM_SLEEP_FOV_SPD));
    g_vtSleepDarkenSpeed.Init(g_pLTClient, "SleepDarkenSpeed", NULL, GetFloat(DM_SLEEP_TAG,DM_SLEEP_SPD));

    g_vtBurnSpeed1.Init(g_pLTClient, "BurnSpeed1", NULL, GetFloat(DM_BURN_TAG,DM_BURN_SPD1));
    g_vtBurnSpeed2.Init(g_pLTClient, "BurnSpeed2", NULL, GetFloat(DM_BURN_TAG,DM_BURN_SPD2));

    g_vtShockSpeed1.Init(g_pLTClient, "ShockSpeed1", NULL, GetFloat(DM_ELECTRO_TAG,DM_ELECTRO_SPD1));
    g_vtShockSpeed2.Init(g_pLTClient, "ShockSpeed2", NULL, GetFloat(DM_ELECTRO_TAG,DM_ELECTRO_SPD2));

    g_vtTestBleedingFX.Init(g_pLTClient, "TestBleedingFX", NULL, 0.0f);
    g_vtTestPoisonFX.Init(g_pLTClient, "TestPoisonFX", NULL, 0.0f);
    g_vtTestStunFX.Init(g_pLTClient, "TestStunFX", NULL, 0.0f);
    g_vtTestSleepingFX.Init(g_pLTClient, "TestSleepingFX", NULL, 0.0f);
    g_vtTestBurnFX.Init(g_pLTClient, "TestBurnFX", NULL, 0.0f);
    g_vtTestChokeFX.Init(g_pLTClient, "TestChokeFX", NULL, 0.0f);
    g_vtTestElectrocuteFX.Init(g_pLTClient, "TestElectrocuteFX", NULL, 0.0f);
    g_vtEnableDamageFX.Init(g_pLTClient, "EnableDamageFX", NULL, 1.0f);

	m_vBurnColor = GetVector(DM_BURN_TAG,DM_BURN_COLOR);
	m_vBurnColor *= MATH_ONE_OVER_255;
	m_vElectrocuteColor = GetVector(DM_ELECTRO_TAG,DM_ELECTRO_COLOR);
	m_vElectrocuteColor *= MATH_ONE_OVER_255;

	m_fMaxRot = g_vtPoisonRotMax.GetFloat();
	m_fMinRot = -m_fMaxRot;

	sprintf(s_aTagName, "%s0", DM_SPRINKLES_TAG);
	while (m_buteMgr.Exist(s_aTagName) && m_nNumSprinkles < MAX_SPRINKLE_TYPES)
	{
		m_nNumSprinkles++;
		sprintf(s_aTagName, "%s%d", DM_SPRINKLES_TAG, m_nNumSprinkles);
	}


    return LTTRUE;
}


void CDamageFXMgr::Update()
{
	if (g_vtEnableDamageFX.GetFloat() < 1.0f)
	{
		g_vtTestBleedingFX.SetFloat(0.0f);
		g_vtTestPoisonFX.SetFloat(0.0f);
		g_vtTestStunFX.SetFloat(0.0f);
		g_vtTestSleepingFX.SetFloat(0.0f);
		g_vtTestBurnFX.SetFloat(0.0f);
		g_vtTestChokeFX.SetFloat(0.0f);
		g_vtTestElectrocuteFX.SetFloat(0.0f);

		Clear();
		return;
	}

	if (g_pInterfaceMgr->GetGameState() != GS_PLAYING || g_pGameClientShell->IsUsingExternalCamera())
	{
		Clear();
		return;
	}


	//do stuff here
	if (m_bPoison || m_bPoisonFade)
		UpdatePoisonFX();
	if (m_bStun || m_bStunFade)
		UpdateStunFX();
	if (m_bSleeping || m_bSleepingFade)
		UpdateSleepingFX();
	if (m_bBurn || m_bBurnFade)
		UpdateBurnFX();
	if (m_bChoke || m_bChokeFade)
		UpdateChokeFX();
	if (m_bElectrocute || m_bElectrocuteFade)
		UpdateElectrocuteFX();


}

void CDamageFXMgr::Clear()
{
    StopBleedingFX(LTFALSE);
    StopPoisonFX(LTFALSE);
    StopStunFX(LTFALSE);
    StopSleepingFX(LTFALSE);
    StopBurnFX(LTFALSE);
    StopElectrocuteFX(LTFALSE);
}

void CDamageFXMgr::StartBleedingFX()
{
	if (!m_bBleeding)
	{
        uint32 dwFlags = PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
		char szTemp[128] = "";
		GetString(DM_BLEEDING_TAG,DM_BLEEDING_SOUND,szTemp,sizeof(szTemp));

		m_hBleedingSound = g_pClientSoundMgr->PlaySoundLocal(szTemp, SOUNDPRIORITY_PLAYER_LOW, dwFlags);
        m_bBleeding = LTTRUE;
	}
}

void CDamageFXMgr::StopBleedingFX(LTBOOL bFade)
{
	if (m_bBleeding)
	{
		if (m_hBleedingSound)
		{
			g_pLTClient->KillSound(m_hBleedingSound);
			m_hBleedingSound = LTNULL;
		}
        m_bBleeding = LTFALSE;
	}
}


void CDamageFXMgr::StartPoisonFX()
{
	if (!m_bPoison)
	{
		m_fFOVXOffset = 0.0f;
		m_fFOVYOffset = 0.0f;
		CreateSprinkles();
        uint32 dwFlags = PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
		char szTemp[128] = "";
		GetString(DM_POISON_TAG,DM_POISON_SOUND,szTemp,sizeof(szTemp));

		m_hPoisonSound = g_pClientSoundMgr->PlaySoundLocal(szTemp, SOUNDPRIORITY_PLAYER_LOW, dwFlags);
	}
    m_bPoison = LTTRUE;
}

void CDamageFXMgr::StopPoisonFX(LTBOOL bFade)
{
	if (m_bPoison)
	{
        m_bPoison = LTFALSE;
		m_bPoisonFade = bFade;
		if (m_hPoisonSound)
		{
			g_pLTClient->KillSound(m_hPoisonSound);
			m_hPoisonSound = LTNULL;
		}

		DestroySprinkles();
		if (!bFade)
			UpdatePoisonFX();
	}
}


void CDamageFXMgr::StartStunFX()
{
	if (!m_bStun)
	{
        uint32 dwFlags = PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
		char szTemp[128] = "";
		GetString(DM_STUN_TAG,DM_STUN_SOUND,szTemp,sizeof(szTemp));

		m_hStunSound = g_pClientSoundMgr->PlaySoundLocal(szTemp, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
        m_bStun = LTTRUE;
	}
}

void CDamageFXMgr::StopStunFX(LTBOOL bFade)
{
	if (m_bStun)
	{
        m_bStun = LTFALSE;
		if (m_hStunSound)
		{
			g_pLTClient->KillSound(m_hStunSound);
			m_hStunSound = LTNULL;
		}

		m_bStunFade = bFade;
		if (!bFade)
			UpdateStunFX();
	}
}


void CDamageFXMgr::StartSleepingFX()
{
	if (m_bSleeping) return;
	if (!m_bSleepingFade)
	{
		m_fSleepDark = 0.0f;
		m_fSleepFOVOffset = 0.0f;
		VEC_SET(m_vDark,0.0f,0.0f,0.0f);
	}

	if (!m_hSleepingSound)
	{
        uint32 dwFlags = PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
		char szTemp[128] = "";
		GetString(DM_SLEEP_TAG,DM_SLEEP_LOOP,szTemp,sizeof(szTemp));
		m_hSleepingSound = g_pClientSoundMgr->PlaySoundLocal(szTemp, SOUNDPRIORITY_PLAYER_LOW, dwFlags);

		GetString(DM_SLEEP_TAG,DM_SLEEP_SOUND,szTemp,sizeof(szTemp));
		g_pClientSoundMgr->PlaySoundLocal(szTemp, SOUNDPRIORITY_PLAYER_MEDIUM);
	}

	// Make sure we can't move when sleeping...
	g_pGameClientShell->GetMoveMgr()->AllowMovement(LTFALSE);
    g_pLTClient->SetInputState(LTFALSE);

    m_bSleeping = LTTRUE;
}

void CDamageFXMgr::StopSleepingFX(LTBOOL bFade)
{
	if (m_bSleeping)
	{
		// Okay, we can move now...
		g_pGameClientShell->GetMoveMgr()->AllowMovement(LTTRUE);
        g_pLTClient->SetInputState(LTTRUE);

        m_bSleeping = LTFALSE;
		m_bSleepingFade = bFade;
		if (m_hSleepingSound)
		{
			g_pLTClient->KillSound(m_hSleepingSound);
			m_hSleepingSound = LTNULL;
		}
		if (!bFade)
			UpdateSleepingFX();
	}
}

void CDamageFXMgr::StartBurnFX()
{
	if (!m_bBurn)
	{
        m_bBurn = LTTRUE;
		m_fBurn1 = 0.0f;
		m_fBurn2 = 0.0f;
		m_fBurnDir1 = 1.0f;
		m_fBurnDir2 = 1.0f;
        uint32 dwFlags = PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
		char szTemp[128] = "";
		GetString(DM_BURN_TAG,DM_BURN_SOUND,szTemp,sizeof(szTemp));
		m_hBurnSound = g_pClientSoundMgr->PlaySoundLocal(szTemp, SOUNDPRIORITY_PLAYER_LOW, dwFlags);
	}

}

void CDamageFXMgr::StopBurnFX(LTBOOL bFade)
{
	if (m_bBurn)
	{
        m_bBurn = LTFALSE;
		m_bBurnFade = bFade;
		if (m_hBurnSound)
		{
			g_pLTClient->KillSound(m_hBurnSound);
			m_hBurnSound = LTNULL;
		}

		if (!bFade)
			UpdateBurnFX();
	}
}

void CDamageFXMgr::StartElectrocuteFX()
{

	if (!m_bElectrocute)
	{
        m_bElectrocute = LTTRUE;
		m_fShock1 = 0.0f;
		m_fShock2 = 0.0f;
		m_fShockDir1 = 1.0f;
		m_fShockDir2 = 1.0f;
        uint32 dwFlags = PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
		char szTemp[128] = "";
		GetString(DM_ELECTRO_TAG,DM_ELECTRO_SOUND,szTemp,sizeof(szTemp));
		m_hElectrocuteSound = g_pClientSoundMgr->PlaySoundLocal(szTemp, SOUNDPRIORITY_PLAYER_LOW, dwFlags);
	}
}

void CDamageFXMgr::StopElectrocuteFX(LTBOOL bFade)
{
	if (m_bElectrocute)
	{
        m_bElectrocute = LTFALSE;
		m_bElectrocuteFade = bFade;
		if (m_hElectrocuteSound)
		{
			g_pLTClient->KillSound(m_hElectrocuteSound);
			m_hElectrocuteSound = LTNULL;
		}

		if (!bFade)
			UpdateElectrocuteFX();
	}
}

void CDamageFXMgr::StartChokeFX()
{
	if (!m_bChoke)
	{
        m_bChoke = LTTRUE;
        uint32 dwFlags = PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
		char szTemp[128] = "";
		GetString(DM_CHOKE_TAG,DM_CHOKE_SOUND,szTemp,sizeof(szTemp));
		m_hChokeSound = g_pClientSoundMgr->PlaySoundLocal(szTemp, SOUNDPRIORITY_PLAYER_LOW, dwFlags);
	}
}

void CDamageFXMgr::StopChokeFX(LTBOOL bFade)
{
	if (m_bChoke)
	{
        m_bChoke = LTFALSE;
		m_bChokeFade = bFade;
		if (m_hChokeSound)
		{
			g_pLTClient->KillSound(m_hChokeSound);
			m_hChokeSound = LTNULL;
		}

		if (!bFade)
			UpdateChokeFX();
	}
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdatePoisonFX
//
//	PURPOSE:	UpdatePoisonFX
//
// --------------------------------------------------------------------------- //

void CDamageFXMgr::UpdatePoisonFX()
{
	HLOCALOBJ hCamera = g_pGameClientShell->GetCamera();
    LTFLOAT fMove = g_pGameClientShell->GetMoveMgr()->GetMovementPercent();

    LTFLOAT fFovX = DEG2RAD(g_vtFOVXNormal.GetFloat());
    LTFLOAT fFovY = DEG2RAD(g_vtFOVYNormal.GetFloat());

    LTFLOAT fFOVMax = g_vtPoisonFOVMax.GetFloat();
    LTFLOAT fRMax = g_vtPoisonColorR.GetFloat();
    LTFLOAT fGMax = g_vtPoisonColorG.GetFloat();
    LTFLOAT fBMax = g_vtPoisonColorB.GetFloat();
    g_pLTClient->GetCameraFOV(hCamera, &fFovX, &fFovY);

	fFovX -= m_fFOVXOffset * m_fMoveMult;
	fFovY -= m_fFOVYOffset * m_fMoveMult;

	m_fMoveMult = 0.3f + (fMove * 0.7f);

    LTRotation rot;
	g_pGameClientShell->GetCameraRotation(&rot);
    LTVector vU, vF, vR;
    g_pLTClient->GetRotationVectors(&rot, &vU, &vR, &vF);


	LTFLOAT fFrameTime = g_pGameClientShell->GetFrameTime();

	if (m_bPoison)
	{
        LTFLOAT fXSpeed = g_vtPoisonFOVXSpeed.GetFloat() * fFrameTime * m_fFOVXDir;
        LTFLOAT fYSpeed = g_vtPoisonFOVYSpeed.GetFloat() * fFrameTime * m_fFOVYDir;
        LTFLOAT fRSpeed = g_vtPoisonColorSpeed.GetFloat() * fFrameTime * m_fColorRDir;
        LTFLOAT fGSpeed = g_vtPoisonColorSpeed.GetFloat() * fFrameTime * m_fColorGDir;
        LTFLOAT fBSpeed = g_vtPoisonColorSpeed.GetFloat() * fFrameTime * m_fColorBDir;
        LTFLOAT fRotSpeed = g_vtPoisonRotSpeed.GetFloat() * fFrameTime * m_fRotDir;

		m_fFOVXOffset += fXSpeed;
		if (m_fFOVXOffset > fFOVMax)
		{
			m_fFOVXOffset = fFOVMax;
			m_fFOVXDir = -m_fFOVXDir;
		}
		else if (m_fFOVXOffset < -fFOVMax)
		{
			m_fFOVXOffset = -fFOVMax;
			m_fFOVXDir = -m_fFOVXDir;
		}


		m_fFOVYOffset += fYSpeed;
		if (m_fFOVYOffset > fFOVMax)
		{
			m_fFOVYOffset = fFOVMax;
			m_fFOVYDir = -m_fFOVYDir;
		}
		else if (m_fFOVYOffset < -fFOVMax)
		{
			m_fFOVYOffset = -fFOVMax;
			m_fFOVYDir = -m_fFOVYDir;
		}


		m_vPoisonColor.x += fRSpeed;
		if (m_vPoisonColor.x > fRMax)
		{
			m_vPoisonColor.x = fRMax;
			m_fColorRDir = -1.0f;
		}
		else if (m_vPoisonColor.x < 0.0f)
		{
			m_vPoisonColor.x = 0.0f;
			m_fColorRDir = 1.0f;
		}

		m_vPoisonColor.y += fGSpeed;
		if (m_vPoisonColor.y > fGMax)
		{
			m_vPoisonColor.y = fGMax;
			m_fColorGDir = -1.0f;
		}
		else if (m_vPoisonColor.y < 0.0f)
		{
			m_vPoisonColor.y = 0.0f;
			m_fColorGDir = 1.0f;
		}

		m_vPoisonColor.z += fBSpeed;
		if (m_vPoisonColor.z > fBMax)
		{
			m_vPoisonColor.z = fBMax;
			m_fColorBDir = -1.0f;
		}
		else if (m_vPoisonColor.z < 0.0f)
		{
			m_vPoisonColor.z = 0.0f;
			m_fColorBDir = 1.0f;
		}

		// ROTATION
		m_fOffsetRot += fRotSpeed;
		if(m_fOffsetRot >= m_fMaxRot)
		{
			m_fOffsetRot = m_fMaxRot;
			m_fRotDir = -1.0f;
			m_fMinRot = -g_vtPoisonRotMax.GetFloat() * GetRandom(0.5f, 1.0f);
		}
		else if (m_fOffsetRot <= m_fMinRot)
		{
			m_fOffsetRot = m_fMinRot;
			m_fRotDir = 1.0f;
			m_fMaxRot = g_vtPoisonRotMax.GetFloat() * GetRandom(0.5f, 1.0f);
		}

	}
	else if (m_bPoisonFade)
	{
        LTFLOAT fXSpeed = g_vtPoisonFOVXSpeed.GetFloat() * fFrameTime;
        LTFLOAT fYSpeed = g_vtPoisonFOVYSpeed.GetFloat() * fFrameTime;
        LTFLOAT fCSpeed = g_vtPoisonColorSpeed.GetFloat() * fFrameTime;
        LTFLOAT fRotSpeed = g_vtPoisonRotSpeed.GetFloat() * fFrameTime;
        LTBOOL bDone = LTTRUE;
		if (m_fFOVXOffset < -fXSpeed)
		{
			m_fFOVXOffset += fXSpeed;
            bDone = LTFALSE;
		}
		else if (m_fFOVXOffset > fXSpeed)
		{
			m_fFOVXOffset -= fXSpeed;
            bDone = LTFALSE;
		}
		else
		{
			m_fFOVXOffset = 0.0f;
		}

		if (m_fFOVYOffset < -fYSpeed)
		{
			m_fFOVYOffset += fYSpeed;
            bDone = LTFALSE;
		}
		else if (m_fFOVYOffset > fYSpeed)
		{
			m_fFOVYOffset -= fYSpeed;
            bDone = LTFALSE;
		}
		else
		{
			m_fFOVYOffset = 0.0f;
		}

		if (m_vPoisonColor.x > fCSpeed)
		{
			m_vPoisonColor.x -= fCSpeed;
            bDone = LTFALSE;
		}
		else
		{
			m_vPoisonColor.x = 0.0f;
		}
		if (m_vPoisonColor.y > fCSpeed)
		{
			m_vPoisonColor.y -= fCSpeed;
            bDone = LTFALSE;
		}
		else
		{
			m_vPoisonColor.y = 0.0f;
		}

		if (m_vPoisonColor.z > fCSpeed)
		{
			m_vPoisonColor.z -= fCSpeed;
            bDone = LTFALSE;
		}
		else
		{
			m_vPoisonColor.z = 0.0f;
		}

		if (m_fOffsetRot < -fRotSpeed)
		{
			m_fOffsetRot += fRotSpeed;
            bDone = LTFALSE;
		}
		else if (m_fOffsetRot > fRotSpeed)
		{
			m_fOffsetRot -= fRotSpeed;
            bDone = LTFALSE;
		}
		else
		{
			m_fOffsetRot = 0.0f;
		}



		if (bDone)
            m_bPoisonFade = LTFALSE;

	}
	else //neither active or fading, so shut down
	{
		fFovX = DEG2RAD(g_vtFOVXNormal.GetFloat());
		fFovY = DEG2RAD(g_vtFOVYNormal.GetFloat());
		m_fFOVXOffset = 0.0f;
		m_fFOVYOffset = 0.0f;
		m_vPoisonColor.x = 0.0f;
		m_vPoisonColor.y = 0.0f;
		m_vPoisonColor.z = 0.0f;
		m_fOffsetRot = 0.0f;
	}

	if (m_pSprinkles)
		m_pSprinkles->Update();

	fFovX += m_fFOVXOffset * m_fMoveMult;
	fFovY += m_fFOVYOffset * m_fMoveMult;
    LTVector vCol = m_vPoisonColor * m_fMoveMult;

	g_pGameClientShell->GetScreenTintMgr()->Set(TINT_POISON,&vCol);

    g_pLTClient->SetCameraFOV(hCamera, fFovX, fFovY);
    g_pLTClient->RotateAroundAxis(&rot, &vF, m_fOffsetRot * m_fMoveMult);
    g_pLTClient->SetObjectRotation(hCamera, &rot);

	return;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdateStunFX
//
//	PURPOSE:	UpdateStunFX
//
// --------------------------------------------------------------------------- //

void CDamageFXMgr::UpdateStunFX()
{
	HLOCALOBJ hCamera = g_pGameClientShell->GetCamera();

	if (m_bStun)
	{
        LTFLOAT fStunMax = g_vtStunMax.GetFloat();
        LTFLOAT fStunMin = 0.0f; //g_vtStunMin.GetFloat();

        LTFLOAT fSpeed = g_vtStunSpeed.GetFloat() * g_pGameClientShell->GetFrameTime();// * m_fStunDir;


		m_vStunColor.x += fSpeed;
		if (m_vStunColor.x > fStunMax)
		{
			m_vStunColor.x = fStunMax;
//			m_fStunDir = -GetRandom(0.5f,1.5f);
		}
//		else if (m_vStunColor.x < fStunMin)
//		{
//			m_vStunColor.x = fStunMin;
//			m_fStunDir = GetRandom(0.5f,1.5f);
//		}

		m_vStunColor.y = m_vStunColor.x;
		m_vStunColor.z= m_vStunColor.x;

	}
	else if (m_bStunFade)
	{
        LTFLOAT fSpeed = g_vtStunSpeed.GetFloat() * g_pGameClientShell->GetFrameTime();
        LTBOOL bDone = LTTRUE;

		if (m_vStunColor.x > fSpeed)
		{
			m_vStunColor.x -= fSpeed;
            bDone = LTFALSE;
		}
		else
		{
			m_vStunColor.x = 0.0f;
		}
		m_vStunColor.y = m_vStunColor.x;
		m_vStunColor.z= m_vStunColor.x;
		if (bDone)
            m_bStunFade = LTFALSE;

	}
	else
	{
		m_vStunColor.x = 0.0f;
		m_vStunColor.y = 0.0f;
		m_vStunColor.z = 0.0f;
	}

	g_pGameClientShell->GetScreenTintMgr()->Set(TINT_STUN,&m_vStunColor);

	return;
}
// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdateSleepingFX
//
//	PURPOSE:	UpdateSleepingFX
//
// --------------------------------------------------------------------------- //

void CDamageFXMgr::UpdateSleepingFX()
{
	HLOCALOBJ hCamera = g_pGameClientShell->GetCamera();

    LTFLOAT fFovX = DEG2RAD(g_vtFOVXNormal.GetFloat());
    LTFLOAT fFovY = DEG2RAD(g_vtFOVYNormal.GetFloat());

    LTFLOAT fFOVMax = g_vtSleepFOVMax.GetFloat();
    g_pLTClient->GetCameraFOV(hCamera, &fFovX, &fFovY);

	fFovX -= m_fSleepFOVOffset;
	fFovY -= m_fSleepFOVOffset;

	g_pGameClientShell->GetLightScaleMgr()->ClearLightScale(&m_vDark,LightEffectDamage);

	if (m_bSleeping)
	{
        m_fSleepDark += g_vtSleepDarkenSpeed.GetFloat() * g_pGameClientShell->GetFrameTime();
		if (m_fSleepDark > 1.0f)
			m_fSleepDark = 1.0f;

        LTFLOAT fSpeed = g_vtSleepFOVSpeed.GetFloat() * g_pGameClientShell->GetFrameTime() * m_fSleepFOVDir;

		m_fSleepFOVOffset += fSpeed;
		if (m_fSleepFOVOffset > fFOVMax)
		{
			m_fSleepFOVOffset = fFOVMax;
			m_fSleepFOVDir = -1.0f;
		}
		else if (m_fSleepFOVOffset < -fFOVMax)
		{
			m_fSleepFOVOffset = -fFOVMax;
			m_fSleepFOVDir = 1.0f;
		}


	}
	else if (m_bSleepingFade)
	{
        LTBOOL bDone = LTTRUE;
        LTFLOAT fDark = g_vtSleepDarkenSpeed.GetFloat() * g_pGameClientShell->GetFrameTime();
		if (m_fSleepDark > fDark)
		{
			m_fSleepDark -=	fDark;
            bDone = LTFALSE;
		}
		else
			m_fSleepDark = 0.0f;


        LTFLOAT fSpeed = g_vtSleepFOVSpeed.GetFloat() * g_pGameClientShell->GetFrameTime();
		if (m_fSleepFOVOffset < -fSpeed)
		{
			m_fSleepFOVOffset += fSpeed;
            bDone = LTFALSE;
		}
		else if (m_fSleepFOVOffset > fSpeed)
		{
			m_fSleepFOVOffset -= fSpeed;
            bDone = LTFALSE;
		}
		else
		{
			m_fSleepFOVOffset = 0.0f;
		}


		if (bDone)
            m_bSleepingFade = LTFALSE;

	}
	else //neither active or fading, so shut down
	{
		fFovX = DEG2RAD(g_vtFOVXNormal.GetFloat());
		fFovY = DEG2RAD(g_vtFOVYNormal.GetFloat());
		m_fSleepDark = 0.0f;
		m_fSleepFOVOffset = 0.0f;
	}

	VEC_SET(m_vDark,(1.0f-m_fSleepDark),(1.0f-m_fSleepDark),(1.0f-m_fSleepDark));
	if (m_fSleepDark > 0.0f)
	{
		g_pGameClientShell->GetLightScaleMgr()->SetLightScale(&m_vDark,LightEffectDamage);
	}
	fFovX += m_fSleepFOVOffset;
	fFovY += m_fSleepFOVOffset;

    g_pLTClient->SetCameraFOV(hCamera, fFovX, fFovY);

	return;
}
// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdateChokeFX
//
//	PURPOSE:	UpdateChokeFX
//
// --------------------------------------------------------------------------- //

void CDamageFXMgr::UpdateChokeFX()
{
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdateBurnFX
//
//	PURPOSE:	UpdateBurnFX
//
// --------------------------------------------------------------------------- //

void CDamageFXMgr::UpdateBurnFX()
{
    LTVector vCurrBurn;
	vCurrBurn.Init(0.0f,0.0f,0.0f);
	if (m_bBurn)
	{
        LTFLOAT fSpeed1 = g_vtBurnSpeed1.GetFloat() * g_pGameClientShell->GetFrameTime() * m_fBurnDir1;
        LTFLOAT fSpeed2 = g_vtBurnSpeed2.GetFloat() * g_pGameClientShell->GetFrameTime() * m_fBurnDir2;
		m_fBurn1 += fSpeed1;
		if (m_fBurn1 > 1.0f)
		{
			m_fBurn1 = 1.0f;
			m_fBurnDir1 = -1.0f;
		}
		else if (m_fBurn1 < 0.0f)
		{
			m_fBurn1 = 0.0f;
			m_fBurnDir1 = 1.0f;
		}

		m_fBurn2 += fSpeed2;
		if (m_fBurn2 > 1.0f)
		{
			m_fBurn2 = 1.0f;
			m_fBurnDir2 = -1.0f;
		}
		else if (m_fBurn2 < 0.0f)
		{
			m_fBurn2 = 0.0f;
			m_fBurnDir2 = 1.0f;
		}

	}
	else if (m_bBurnFade)
	{
        LTBOOL bDone = LTTRUE;
        LTFLOAT fSpeed1 = g_vtBurnSpeed1.GetFloat() * g_pGameClientShell->GetFrameTime();
        LTFLOAT fSpeed2 = g_vtBurnSpeed2.GetFloat() * g_pGameClientShell->GetFrameTime();
		if (m_fBurn1 > fSpeed1)
		{
			m_fBurn1 -= fSpeed1;
            bDone = LTFALSE;
		}
		else
		{
			m_fBurn1 = 0.0f;
		}
		if (m_fBurn2 > fSpeed2)
		{
			m_fBurn2 -= fSpeed2;
            bDone = LTFALSE;
		}
		else
		{
			m_fBurn2 = 0.0f;
		}
		if (bDone)
            m_bBurnFade = LTFALSE;

	}
	else
	{
		m_fBurn1 = 0.0f;
		m_fBurn2 = 0.0f;
	}
	vCurrBurn.x = m_vBurnColor.x * m_fBurn1 * m_fBurn2;
	vCurrBurn.y = m_vBurnColor.y * m_fBurn1 * m_fBurn2;
	vCurrBurn.z = m_vBurnColor.z * m_fBurn1 * m_fBurn2;

	g_pGameClientShell->GetScreenTintMgr()->Set(TINT_BURN,&vCurrBurn);

	return;
}
// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdateElectrocuteFX
//
//	PURPOSE:	UpdateElectrocuteFX
//
// --------------------------------------------------------------------------- //

void CDamageFXMgr::UpdateElectrocuteFX()
{
    LTVector vCurrShock;
	vCurrShock.Init(0.0f,0.0f,0.0f);
	if (m_bElectrocute)
	{
        LTFLOAT fSpeed1 = g_vtShockSpeed1.GetFloat() * g_pGameClientShell->GetFrameTime() * m_fShockDir1;
        LTFLOAT fSpeed2 = g_vtShockSpeed2.GetFloat() * g_pGameClientShell->GetFrameTime() * m_fShockDir2;
		m_fShock1 += fSpeed1;
		if (m_fShock1 > 1.0f)
		{
			m_fShock1 = 1.0f;
			m_fShockDir1 = -1.0f;
		}
		else if (m_fShock1 < 0.0f)
		{
			m_fShock1 = 0.0f;
			m_fShockDir1 = 1.0f;
		}

		m_fShock2 += fSpeed2;
		if (m_fShock2 > 1.0f)
		{
			m_fShock2 = 1.0f;
			m_fShockDir2 = -1.0f;
		}
		else if (m_fShock2 < 0.0f)
		{
			m_fShock2 = 0.0f;
			m_fShockDir2 = 1.0f;
		}

	}
	else if (m_bElectrocuteFade)
	{
        LTBOOL bDone = LTTRUE;
        LTFLOAT fSpeed1 = g_vtShockSpeed1.GetFloat() * g_pGameClientShell->GetFrameTime();
        LTFLOAT fSpeed2 = g_vtShockSpeed2.GetFloat() * g_pGameClientShell->GetFrameTime();
		if (m_fShock1 > fSpeed1)
		{
			m_fShock1 -= fSpeed1;
            bDone = LTFALSE;
		}
		else
		{
			m_fShock1 = 0.0f;
		}
		if (m_fShock2 > fSpeed2)
		{
			m_fShock2 -= fSpeed2;
            bDone = LTFALSE;
		}
		else
		{
			m_fShock2 = 0.0f;
		}
		if (bDone)
            m_bElectrocuteFade = LTFALSE;

	}
	else
	{
		m_fShock1 = 0.0f;
		m_fShock2 = 0.0f;
	}
	vCurrShock.x = m_vElectrocuteColor.x * m_fShock1 * m_fShock2;
	vCurrShock.y = m_vElectrocuteColor.y * m_fShock1 * m_fShock2;
	vCurrShock.z = m_vElectrocuteColor.z * m_fShock1 * m_fShock2;


	g_pGameClientShell->GetScreenTintMgr()->Set(TINT_ELECTROCUTE,&vCurrShock);

	return;
}


void CDamageFXMgr::CreateSprinkles()
{
	if (!m_pSprinkles)
	{
		m_pSprinkles = debug_new(SprinklesFX);
		SPRINKLESCREATESTRUCT scs;

		scs.m_nTypes = m_nNumSprinkles;

		for (int i = 0; i < m_nNumSprinkles; i++)
		{
			char szTemp[128] = "";
			sprintf(s_aTagName, "%s%d", DM_SPRINKLES_TAG,i);

			GetString(s_aTagName,DM_SPRINKLES_FILENAME,szTemp,sizeof(szTemp));
            scs.m_Types[i].m_hFilename      = g_pLTClient->CreateString(szTemp);


            szTemp[0] = LTNULL;
			GetString(s_aTagName,DM_SPRINKLES_SKINNAME,szTemp,sizeof(szTemp));
            scs.m_Types[i].m_hSkinName      = g_pLTClient->CreateString(szTemp);

			scs.m_Types[i].m_Count			= GetDWord(s_aTagName,DM_SPRINKLES_COUNT);
			scs.m_Types[i].m_Speed			= GetFloat(s_aTagName,DM_SPRINKLES_SPEED);
			scs.m_Types[i].m_Size			= GetFloat(s_aTagName,DM_SPRINKLES_SIZE);
			scs.m_Types[i].m_SpawnRadius	= GetFloat(s_aTagName,DM_SPRINKLES_SPAWN);
			scs.m_Types[i].m_ColorMax		= GetVector(s_aTagName,DM_SPRINKLES_COLORMAX);
			scs.m_Types[i].m_ColorMin		= GetVector(s_aTagName,DM_SPRINKLES_COLORMIN);
			scs.m_Types[i].m_AnglesVel		= GetVector(s_aTagName,DM_SPRINKLES_ANGLES);
		}

		m_pSprinkles->Init(&scs);

	}
}

void CDamageFXMgr::DestroySprinkles()
{
	if (m_pSprinkles)
	{
		debug_delete(m_pSprinkles);
        m_pSprinkles = LTNULL;
	}
}



// ------------------------------------------------------------------------//
//
//	Private Helper functions
//
// ------------------------------------------------------------------------//

LTBOOL CDamageFXMgr::GetBool(char *pTag,char *pAttribute)
{
    return (LTBOOL) m_buteMgr.GetInt(pTag,pAttribute, 0);
}

LTFLOAT CDamageFXMgr::GetFloat(char *pTag,char *pAttribute)
{
    return (LTFLOAT)m_buteMgr.GetDouble(pTag, pAttribute, 0.0f);
}

int	CDamageFXMgr::GetInt(char *pTag,char *pAttribute)
{
	return m_buteMgr.GetInt(pTag, pAttribute, 0);
}

LTIntPt CDamageFXMgr::GetPoint(char *pTag,char *pAttribute)
{
    CPoint zero(0,0);
    CPoint tmp = m_buteMgr.GetPoint(pTag, pAttribute, zero);
    LTIntPt pt(tmp.x,tmp.y);
	return pt;
}

uint32 CDamageFXMgr::GetDWord(char *pTag,char *pAttribute)
{
    return (uint32)m_buteMgr.GetInt(pTag, pAttribute, 0);
}

void CDamageFXMgr::GetString(char *pTag,char *pAttribute,char *pBuf, int nBufLen)
{
	CString str = "";
	str = m_buteMgr.GetString(pTag, pAttribute);
	strncpy(pBuf, (char*)(LPCSTR)str, nBufLen);
}

LTVector CDamageFXMgr::GetVector(char *pTag,char *pAttribute)
{
	CAVector vRet(0.0,0.0,0.0);
	return m_buteMgr.GetVector(pTag, pAttribute, vRet);
}