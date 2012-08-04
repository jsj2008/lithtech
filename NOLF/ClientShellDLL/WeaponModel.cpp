// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponModel.cpp
//
// PURPOSE : Generic client-side WeaponModel wrapper class - Implementation
//
// CREATED : 9/27/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeaponModel.h"
#include "ClientUtilities.h"
#include "GameClientShell.h"
#include "ShellCasingFX.h"
#include "SFXMsgIds.h"
#include "GameSettings.h"
#include "MsgIds.h"
#include "WeaponFX.h"
#include "ProjectileFX.h"
#include "ClientServerShared.h"
#include "ClientWeaponUtils.h"
#include "iltphysics.h"
#include "PlayerStats.h"
#include "WeaponFXTypes.h"
#include "SurfaceFunctions.h"
#include "iltcustomdraw.h"
#include "VarTrack.h"
#include "CharacterFX.h"

extern CGameClientShell* g_pGameClientShell;
extern LTBOOL g_bInfiniteAmmo;

#define INFINITE_AMMO_AMOUNT	1000
#define INVALID_ANI				((HMODELANIM)-1)
#define DEFAULT_ANI				((HMODELANIM)0)

#define	WEAPON_KEY_SUNGLASS		"SUNGLASSES_KEY"

static uint8 g_nRandomWeaponSeed;

// Used with camera disabler gadget...

static char* s_pCamDisPieces[] =
{
	"dis1", 0
};

// Used with code decipher gadget...

static char* s_pCodeDecPieces[] =
{
	"Dec1", 0
};

namespace
{
	VarTrack	g_vtFastTurnRate;
	VarTrack	g_vtPerturbRotationEffect;
	VarTrack	g_vtPerturbIncreaseSpeed;
	VarTrack	g_vtPerturbDecreaseSpeed;
	VarTrack	g_vtPerturbWalkPercent;
	VarTrack	g_vtCameraShutterSpeed;

    LTFLOAT		m_fLastPitch = 0.0f;
    LTFLOAT		m_fLastYaw = 0.0f;

	int			m_nCurTracer = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CWeaponModel()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

CWeaponModel::CWeaponModel()
{
	m_nWeaponId			= WMGR_INVALID_ID;
	m_nHolsterWeaponId	= WMGR_INVALID_ID;
	m_nAmmoId			= WMGR_INVALID_ID;
    m_hObject           = LTNULL;

    m_hSilencerModel    = LTNULL;
    m_hLaserModel       = LTNULL;
    m_hScopeModel       = LTNULL;

	m_hBreachSocket		= INVALID_MODEL_SOCKET;
	m_hLaserSocket		= INVALID_MODEL_SOCKET;
	m_hSilencerSocket	= INVALID_MODEL_SOCKET;
	m_hScopeSocket		= INVALID_MODEL_SOCKET;

    m_bHaveSilencer     = LTFALSE;
    m_bHaveLaser        = LTFALSE;
    m_bHaveScope        = LTFALSE;

	m_fBobHeight		= 0.0f;
	m_fBobWidth			= 0.0f;
	m_fFlashStartTime	= 0.0f;

	m_vFlashPos.Init();
	m_vFlashOffset.Init();

	m_fNextIdleTime			= 0.0f;
	m_nAmmoInClip			= 0;
	m_nNewAmmoInClip		= 0;
    m_bFire                 = LTFALSE;
	m_eState				= W_IDLE;
	m_eLastWeaponState		= W_IDLE;
	m_eLastFireType			= FT_NORMAL_FIRE;
    m_bCanSetLastFire       = LTTRUE;

    m_bUsingAltFireAnis         = LTFALSE;
    m_bFireKeyDownLastUpdate    = LTFALSE;

	m_nSelectAni			= INVALID_ANI;
	m_nDeselectAni			= INVALID_ANI;
	m_nReloadAni			= INVALID_ANI;

	m_nAltSelectAni			= INVALID_ANI;
	m_nAltDeselectAni		= INVALID_ANI;
	m_nAltDeselect2Ani		= INVALID_ANI;
	m_nAltReloadAni			= INVALID_ANI;

    int i;
    for (i=0; i < WM_MAX_FIRE_ANIS; i++)
	{
		m_nFireAnis[i] = INVALID_ANI;
	}

	for (i=0; i < WM_MAX_IDLE_ANIS; i++)
	{
		m_nIdleAnis[i] = INVALID_ANI;
	}

	for (i=0; i < WM_MAX_ALTFIRE_ANIS; i++)
	{
		m_nAltFireAnis[i] = INVALID_ANI;
	}

	for (i=0; i < WM_MAX_ALTIDLE_ANIS; i++)
	{
		m_nAltIdleAnis[i] = INVALID_ANI;
	}

	m_vPath.Init();
	m_vFirePos.Init();
	m_vEndPos.Init();

	m_wIgnoreFX				= 0;
	m_nRequestedWeaponId	= WMGR_INVALID_ID;
    m_bWeaponDeselected     = LTFALSE;

    m_pWeapon   = LTNULL;
    m_pAmmo     = LTNULL;

	m_fMovementPerturb		= 0.0f;

    m_bDisabled             = LTFALSE;
    m_bVisible              = LTTRUE;

	m_rCamRot.Init();
	m_vCamPos.Init();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CWeaponModel()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CWeaponModel::~CWeaponModel()
{
	if (m_hObject)
	{
        g_pLTClient->DeleteObject(m_hObject);
        m_hObject = LTNULL;
	}

	RemoveMods();

	m_PVFXMgr.Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::Init()
//
//	PURPOSE:	Initialize perturb variables
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::Init()
{
    g_vtFastTurnRate.Init(g_pLTClient, "FastTurnRate", NULL, 2.3f);

    LTFLOAT fTemp = g_pLayoutMgr->GetPerturbRotationEffect();
    g_vtPerturbRotationEffect.Init(g_pLTClient, "PerturbRotationEffect", NULL, fTemp);

	fTemp = g_pLayoutMgr->GetPerturbIncreaseSpeed();
    g_vtPerturbIncreaseSpeed.Init(g_pLTClient, "PerturbIncreaseSpeed", NULL, fTemp);

	fTemp = g_pLayoutMgr->GetPerturbDecreaseSpeed();
    g_vtPerturbDecreaseSpeed.Init(g_pLTClient, "PerturbDecreaseSpeed", NULL, fTemp);

	fTemp = g_pLayoutMgr->GetPerturbWalkPercent();
    g_vtPerturbWalkPercent.Init(g_pLTClient, "PerturbWalkPercent", NULL, fTemp);

   g_vtCameraShutterSpeed.Init(g_pLTClient, "CameraShutterSpeed", NULL, 0.3f);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::Create()
//
//	PURPOSE:	Create the WeaponModel model
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::Create(ILTClient* pClientDE, uint8 nWeaponId, uint8 nAmmoId,
                           uint32 dwAmmo)
{
    if (!pClientDE) return LTFALSE;

	m_nWeaponId	= nWeaponId;
	m_pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
    if (!m_pWeapon) return LTFALSE;

	m_nAmmoId = nAmmoId;
	m_pAmmo	= g_pWeaponMgr->GetAmmo(nAmmoId);
    if (!m_pAmmo) return LTFALSE;

	// Important to update this before we access PlayerStats...

	g_pInterfaceMgr->UpdateWeaponStats(nWeaponId, nAmmoId, dwAmmo);

	ResetWeaponData();
	CreateModel();
	CreateFlash();
	CreateMods();

	m_PVFXMgr.Init(m_hObject, m_pWeapon);

	InitAnimations();

	Select();	// Select the weapon

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::ResetWeaponData
//
//	PURPOSE:	Reset weapon specific data
//
// ----------------------------------------------------------------------- //

void CWeaponModel::ResetWeaponData()
{
	m_hBreachSocket		= INVALID_MODEL_SOCKET;
	m_hLaserSocket		= INVALID_MODEL_SOCKET;
	m_hSilencerSocket	= INVALID_MODEL_SOCKET;
	m_hScopeSocket		= INVALID_MODEL_SOCKET;

    m_bHaveSilencer     = LTFALSE;
    m_bHaveLaser        = LTFALSE;
    m_bHaveScope        = LTFALSE;

	m_fBobHeight		= 0.0f;
	m_fBobWidth			= 0.0f;
	m_fFlashStartTime	= 0.0f;

	m_vFlashPos.Init();
	m_vFlashOffset.Init();

	m_nAmmoInClip			= 0;
	m_nNewAmmoInClip		= 0;
    m_bFire                 = LTFALSE;
	m_eState				= W_IDLE;
	m_eLastWeaponState		= W_IDLE;
	m_eLastFireType			= FT_NORMAL_FIRE;
    m_bCanSetLastFire       = LTTRUE;

	m_nSelectAni			= INVALID_ANI;
	m_nDeselectAni			= INVALID_ANI;
	m_nReloadAni			= INVALID_ANI;

	m_nAltSelectAni			= INVALID_ANI;
	m_nAltDeselectAni		= INVALID_ANI;
	m_nAltDeselect2Ani		= INVALID_ANI;
	m_nAltReloadAni			= INVALID_ANI;

    int i;
    for (i=0; i < WM_MAX_FIRE_ANIS; i++)
	{
		m_nFireAnis[i] = INVALID_ANI;
	}

	for (i=0; i < WM_MAX_IDLE_ANIS; i++)
	{
		m_nIdleAnis[i] = INVALID_ANI;
	}

	for (i=0; i < WM_MAX_ALTFIRE_ANIS; i++)
	{
		m_nAltFireAnis[i] = INVALID_ANI;
	}

	for (i=0; i < WM_MAX_ALTIDLE_ANIS; i++)
	{
		m_nAltIdleAnis[i] = INVALID_ANI;
	}

	m_vPath.Init();
	m_vFirePos.Init();
	m_vEndPos.Init();

	m_wIgnoreFX				= 0;
	m_nRequestedWeaponId	= WMGR_INVALID_ID;
    m_bWeaponDeselected     = LTFALSE;

	m_nAmmoInClip		= 0;
	m_nNewAmmoInClip	= 0;

    m_bHaveSilencer     = LTFALSE;
    m_bHaveLaser        = LTFALSE;
    m_bHaveScope        = LTFALSE;

    m_bUsingAltFireAnis         = LTFALSE;
    m_bFireKeyDownLastUpdate    = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::InitAnimations
//
//	PURPOSE:	Set the animations
//
// ----------------------------------------------------------------------- //

void CWeaponModel::InitAnimations(LTBOOL bAllowSelectOverride)
{
	if (!m_hObject) return;

    m_nSelectAni    = g_pLTClient->GetAnimIndex(m_hObject, "Select");
    m_nDeselectAni  = g_pLTClient->GetAnimIndex(m_hObject, "Deselect");
    m_nReloadAni    = g_pLTClient->GetAnimIndex(m_hObject, "Reload");

    m_nAltSelectAni     = g_pLTClient->GetAnimIndex(m_hObject, "AltSelect");
    m_nAltDeselectAni   = g_pLTClient->GetAnimIndex(m_hObject, "AltDeselect");
    m_nAltDeselect2Ani  = g_pLTClient->GetAnimIndex(m_hObject, "AltDeselect2");
    m_nAltReloadAni     = g_pLTClient->GetAnimIndex(m_hObject, "AltReload");

	char buf[30];

    int i;
    for (i=0; i < WM_MAX_IDLE_ANIS; i++)
	{
		sprintf(buf, "Idle_%d", i);
        m_nIdleAnis[i] = g_pLTClient->GetAnimIndex(m_hObject, buf);
    }


	for (i=0; i < WM_MAX_FIRE_ANIS; i++)
	{
		if (i > 0)
		{
			sprintf(buf, "Fire%d", i);
		}
		else
		{
			sprintf(buf, "Fire");
		}

        m_nFireAnis[i] = g_pLTClient->GetAnimIndex(m_hObject, buf);
	}

	for (i=0; i < WM_MAX_ALTIDLE_ANIS; i++)
	{
		sprintf(buf, "AltIdle_%d", i);
        m_nAltIdleAnis[i] = g_pLTClient->GetAnimIndex(m_hObject, buf);
	}

	for (i=0; i < WM_MAX_ALTFIRE_ANIS; i++)
	{
		if (i > 0)
		{
			sprintf(buf, "AltFire%d", i);
		}
		else
		{
			sprintf(buf, "AltFire");
		}

        m_nAltFireAnis[i] = g_pLTClient->GetAnimIndex(m_hObject, buf);
	}


	// See if there are Ammo-override animations...
	if (m_pAmmo->pAniOverrides)
	{
		// Set new animations...

		SetAmmoOverrideAnis(bAllowSelectOverride);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::SetAmmoOverrideAnis
//
//	PURPOSE:	Set the ammo specific override animations...
//
// ----------------------------------------------------------------------- //

void CWeaponModel::SetAmmoOverrideAnis(LTBOOL bAllowSelectOverride)
{
	if (!m_hObject || !m_pAmmo || ! m_pAmmo->pAniOverrides) return;

	if (bAllowSelectOverride && m_pAmmo->pAniOverrides->szSelectAni[0])
	{
        m_nSelectAni = g_pLTClient->GetAnimIndex(m_hObject, m_pAmmo->pAniOverrides->szSelectAni);
	}

	if (m_pAmmo->pAniOverrides->szDeselectAni[0])
	{
        m_nDeselectAni = g_pLTClient->GetAnimIndex(m_hObject, m_pAmmo->pAniOverrides->szDeselectAni);
	}

	if (m_pAmmo->pAniOverrides->szReloadAni[0])
	{
        m_nReloadAni = g_pLTClient->GetAnimIndex(m_hObject, m_pAmmo->pAniOverrides->szReloadAni);
	}

    int i;
    for (i=0; i < WM_MAX_IDLE_ANIS; i++)
	{
		if (i < m_pAmmo->pAniOverrides->nNumIdleAnis)
		{
			if (m_pAmmo->pAniOverrides->szIdleAnis[i][0])
			{
                m_nIdleAnis[i] = g_pLTClient->GetAnimIndex(m_hObject, m_pAmmo->pAniOverrides->szIdleAnis[i]);
			}
		}
		else
		{
			m_nIdleAnis[i] = INVALID_ANI;
		}
	}


	for (i=0; i < WM_MAX_FIRE_ANIS; i++)
	{
		if (i < m_pAmmo->pAniOverrides->nNumFireAnis)
		{
			if (m_pAmmo->pAniOverrides->szFireAnis[i][0])
			{
                m_nFireAnis[i] = g_pLTClient->GetAnimIndex(m_hObject, m_pAmmo->pAniOverrides->szFireAnis[i]);
			}
		}
		else
		{
			m_nFireAnis[i] = INVALID_ANI;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::Reset()
//
//	PURPOSE:	Reset the model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::Reset()
{
	if (!m_hObject) return;

	RemoveModel();

	m_nWeaponId				= WMGR_INVALID_ID;
	m_nHolsterWeaponId		= WMGR_INVALID_ID;
	m_nAmmoInClip			= 0;
    m_bFire                 = LTFALSE;
	m_nRequestedWeaponId	= WMGR_INVALID_ID;
    m_bWeaponDeselected     = LTFALSE;

    m_bUsingAltFireAnis         = LTFALSE;
    m_bFireKeyDownLastUpdate    = LTFALSE;

	m_PVFXMgr.Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::RemoveModel()
//
//	PURPOSE:	Remove our model data member
//
// ----------------------------------------------------------------------- //

void CWeaponModel::RemoveModel()
{
	if (!m_hObject) return;

	RemoveMods();

    g_pLTClient->DeleteObject(m_hObject);
    m_hObject   = LTNULL;

	m_MuzzleFlash.Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::RemoveMods()
//
//	PURPOSE:	Remove our mod data members
//
// ----------------------------------------------------------------------- //

void CWeaponModel::RemoveMods()
{
	// Remove the silencer model...

	if (m_hSilencerModel)
	{
        g_pLTClient->DeleteObject(m_hSilencerModel);
        m_hSilencerModel  = LTNULL;
		m_hSilencerSocket = INVALID_MODEL_SOCKET;
        m_bHaveSilencer   = LTFALSE;
	}

	// Remove the laser model...

	if (m_hLaserModel)
	{
        g_pLTClient->DeleteObject(m_hLaserModel);
        m_hLaserModel  = LTNULL;
		m_hLaserSocket = INVALID_MODEL_SOCKET;
        m_bHaveLaser   = LTFALSE;

		m_LaserBeam.TurnOff();
	}

	// Remove the scope model...

	if (m_hScopeModel)
	{
        g_pLTClient->DeleteObject(m_hScopeModel);
        m_hScopeModel  = LTNULL;
		m_hScopeSocket = INVALID_MODEL_SOCKET;
        m_bHaveScope   = LTFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateWeaponModel()
//
//	PURPOSE:	Update the WeaponModel state
//
// ----------------------------------------------------------------------- //

WeaponState CWeaponModel::UpdateWeaponModel(LTRotation rCamRot, LTVector vCamPos,
                                            LTBOOL bFire, FireType eFireType)
{
	if (!m_hObject) return W_IDLE;

	// Store current camera pos/rot...

	m_rCamRot = rCamRot;
	m_vCamPos = vCamPos;

	// See if we are disabled...If so don't allow any weapon stuff...

	if (m_bDisabled)
	{
		// Make sure we all the pvfxmgr to hide its stuff...
		m_PVFXMgr.Update();

		return W_IDLE;
	}


	if (bFire && m_bCanSetLastFire)
	{
		m_eLastFireType	= eFireType;
	}

	// See if we just started/stopped firing...

	if (!m_bFireKeyDownLastUpdate && bFire)
	{
		HandleFireKeyDown();
	}
	else if (m_bFireKeyDownLastUpdate && !bFire)
	{
		HandleFireKeyUp();
	}

	// Check for special zip cord case...

	LTBOOL bFireOverride = bFire;
	if (bFire && m_pAmmo->eInstDamageType == DT_GADGET_ZIPCORD)
	{
		bFireOverride = HandleZipCordFire();
	}

	// Need to set these after HandleZipCordFire...It uses the
	// m_bFireKeyDownLastUpdate...

	m_bFireKeyDownLastUpdate = bFire;
	m_eLastWeaponState		 = GetState();

	bFire = bFireOverride;

	// Selecting Alt-fire does not fire the weapon if we are using
	// alt fire animations...

	if (m_bUsingAltFireAnis && m_eLastFireType == FT_ALT_FIRE)
	{
        bFire = LTFALSE;
	}


	// Update the state of the model...

	WeaponState eState = UpdateModelState(bFire);


	// Compute offset for WeaponModel and move the model to the
	// correct position (this is all now camera-relative)

    LTVector vOffset         = GetWeaponOffset();
    LTVector vMuzzleOffset   = GetMuzzleOffset();
    LTVector vRecoil         = m_pWeapon->vRecoil;

    LTVector vNewPos(0, 0, 0);
    LTRotation rNewRot;
	rNewRot.Init();
    g_pLTClient->SetObjectRotation(m_hObject, &rNewRot);

    LTVector vU, vR, vF;
    g_pLTClient->GetRotationVectors(&rNewRot, &vU, &vR, &vF);

    LTVector vCamU, vCamR, vCamF;
    g_pLTClient->GetRotationVectors(&m_rCamRot, &vCamU, &vCamR, &vCamF);

    vNewPos += vR * (vOffset.x + m_fBobWidth);
    vNewPos += vU * (vOffset.y + m_fBobHeight);
    vNewPos += vF * vOffset.z;

	LTVector vTemp(0, 0, 0);
	m_vFlashOffset.Init();

	m_vFlashOffset.x = (vOffset.x + vMuzzleOffset.x + m_fBobWidth);
	m_vFlashOffset.y = (vOffset.y + vMuzzleOffset.y + m_fBobHeight);
	m_vFlashOffset.z = (vOffset.z + vMuzzleOffset.z);

	vTemp = vCamR  * m_vFlashOffset.x;
	vTemp += vCamU * m_vFlashOffset.y;
	vTemp += vCamF * m_vFlashOffset.z;

	if (FiredWeapon(eState))
	{
        LTFLOAT xRand = GetRandom(-vRecoil.x, vRecoil.x);
        LTFLOAT yRand = GetRandom(-vRecoil.y, vRecoil.y);
        LTFLOAT zRand = GetRandom(-vRecoil.z, vRecoil.z);

		vNewPos += vU * yRand;
		vNewPos += vR * xRand;
		vNewPos += vF * zRand;

		m_vFlashOffset.x += yRand;
		m_vFlashOffset.y += xRand;
		m_vFlashOffset.z += zRand;

		vTemp = vCamR  * m_vFlashOffset.x;
		vTemp += vCamU * m_vFlashOffset.y;
		vTemp += vCamF * m_vFlashOffset.z;

        g_pLTClient->SetObjectPos(m_hObject, &vNewPos, LTTRUE);

		if (!m_bHaveSilencer)
		{
			StartFlash();
		}

		// Send message to server telling player to fire...

		SendFireMsg();
	}
	else
	{
        g_pLTClient->SetObjectPos(m_hObject, &vNewPos, LTTRUE);
	}


	m_vFlashPos = m_vCamPos + vTemp;


	// Update the muzzle flash...

	if (!m_bHaveSilencer)
	{
		UpdateFlash(eState);
	}


	// Update the mods...

	UpdateMods();


	// Update any player-view fx...

	m_PVFXMgr.Update();


	UpdateMovementPerturb();


	// AutoSelectWeapon

	if (GetState() == W_FIRING_NOAMMO)
	{
		AutoSelectWeapon();
	}

	return eState;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateBob()
//
//	PURPOSE:	Update WeaponModel bob
//
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateBob(LTFLOAT fWidth, LTFLOAT fHeight)
{
	m_fBobWidth  = fWidth;
	m_fBobHeight = fHeight;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateMods()
//
//	PURPOSE:	Update WeaponModel bob
//
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateMods()
{
	if (!m_hObject) return;

    LTVector vPos = GetModelPos();

    LTRotation rRot;
    uint32 dwFlags;

	// Update the silencer...

	if (m_hSilencerModel)
	{
		if (m_bHaveSilencer && m_hSilencerSocket != INVALID_MODEL_SOCKET)
		{
			LTransform transform;
            if (g_pModelLT->GetSocketTransform(m_hObject, m_hSilencerSocket, transform, LTTRUE) == LT_OK)
			{
				g_pTransLT->Get(transform, vPos, rRot);
                g_pLTClient->SetObjectPos(m_hSilencerModel, &vPos, LTTRUE);
                g_pLTClient->SetObjectRotation(m_hSilencerModel, &rRot);
			}
		}
		else
		{
			// Keep the model close to us...

            g_pLTClient->SetObjectPos(m_hSilencerModel, &vPos, LTTRUE);

			// Hide model...

            dwFlags = g_pLTClient->GetObjectFlags(m_hSilencerModel);
            g_pLTClient->SetObjectFlags(m_hSilencerModel, dwFlags & ~FLAG_VISIBLE);
		}
	}


	// Update the laser...

	if (m_hLaserModel)
	{
		if (m_bHaveLaser && m_hLaserSocket != INVALID_MODEL_SOCKET)
		{
			LTransform transform;
            if (g_pModelLT->GetSocketTransform(m_hObject, m_hLaserSocket, transform, LTTRUE) == LT_OK)
			{
				g_pTransLT->Get(transform, vPos, rRot);
                g_pLTClient->SetObjectPos(m_hLaserModel, &vPos, LTTRUE);
                g_pLTClient->SetObjectRotation(m_hLaserModel, &rRot);

				// Update the laser beam...

                m_LaserBeam.Update(vPos, &rRot, LTFALSE);
			}
		}
		else
		{
			// Keep the model close to us...

            g_pLTClient->SetObjectPos(m_hLaserModel, &vPos, LTTRUE);

			// Hide model...

            dwFlags = g_pLTClient->GetObjectFlags(m_hLaserModel);
            g_pLTClient->SetObjectFlags(m_hLaserModel, dwFlags & ~FLAG_VISIBLE);

			m_LaserBeam.TurnOff();
		}
	}


	// Update the scope...

	if (m_hScopeModel)
	{
		if (m_bHaveScope && m_hScopeSocket != INVALID_MODEL_SOCKET)
		{
			LTransform transform;
            if (g_pModelLT->GetSocketTransform(m_hObject, m_hScopeSocket, transform, LTTRUE) == LT_OK)
			{
				g_pTransLT->Get(transform, vPos, rRot);
                g_pLTClient->SetObjectPos(m_hScopeModel, &vPos, LTTRUE);
                g_pLTClient->SetObjectRotation(m_hScopeModel, &rRot);
			}
		}
		else
		{
			// Keep the model close to us...

            g_pLTClient->SetObjectPos(m_hScopeModel, &vPos, LTTRUE);

			// Hide model...

            dwFlags = g_pLTClient->GetObjectFlags(m_hScopeModel);
            g_pLTClient->SetObjectFlags(m_hScopeModel, dwFlags & ~FLAG_VISIBLE);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::Disable()
//
//	PURPOSE:	Disable/Enable the weapon
//
// ----------------------------------------------------------------------- //

void CWeaponModel::Disable(LTBOOL bDisable)
{
    LTBOOL bOldVisibility = m_bVisible;

	// Let the client shell handle the weapon being disabled...

	g_pGameClientShell->HandleWeaponDisable(bDisable);

	// Handle Gadget Disable...

	GadgetDisable(bDisable);

	if (bDisable)
	{
		// Force weapon invisible...

        SetVisible(LTFALSE);

		// Reset our data member for when the weapon is re-enabled...

		m_bVisible = bOldVisibility;

		// Must set this AFTER call to SetVisible()

        m_bDisabled = LTTRUE;
	}
	else
	{
		// Must set this BEFORE the call to SetVisible()

        m_bDisabled = LTFALSE;

		// Set the visibility back to whatever it was...

		SetVisible(m_bVisible);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GadgetDisable()
//
//	PURPOSE:	Disable/Enable the gadget
//
// ----------------------------------------------------------------------- //

void CWeaponModel::GadgetDisable(LTBOOL bDisable)
{
	if (!m_pAmmo) return;

	// Handle sunglasses gadget...

	if (m_pAmmo->eType == GADGET)
	{
		if (bDisable)
		{
			if (m_pAmmo->eInstDamageType == DT_GADGET_CAMERA)
			{
				g_pInterfaceMgr->SetSunglassMode(SUN_NONE);
			}
			else if (m_pAmmo->eInstDamageType == DT_GADGET_MINE_DETECTOR)
			{
				g_pInterfaceMgr->SetSunglassMode(SUN_NONE);
			}
			else if (m_pAmmo->eInstDamageType == DT_GADGET_INFRA_RED)
			{
				g_pInterfaceMgr->SetSunglassMode(SUN_NONE);
			}
			else
			{
				g_pInterfaceMgr->SetSunglassMode(SUN_NONE);
			}
		}
		else  // Enable...
		{
			if (m_pAmmo->eInstDamageType == DT_GADGET_CAMERA)
			{
				g_pInterfaceMgr->SetSunglassMode(SUN_CAMERA);
			}
			else if (m_pAmmo->eInstDamageType == DT_GADGET_MINE_DETECTOR)
			{
				g_pInterfaceMgr->SetSunglassMode(SUN_MINES);
			}
			else if (m_pAmmo->eInstDamageType == DT_GADGET_INFRA_RED)
			{
				g_pInterfaceMgr->SetSunglassMode(SUN_IR);
			}
			else
			{
				g_pInterfaceMgr->SetSunglassMode(SUN_NONE);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::SetVisible()
//
//	PURPOSE:	Hide/Show the weapon model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::SetVisible(LTBOOL bVis)
{
	if (!m_hObject) return;

	// Set the visible/invisible data member even if we are disabled.
	// The Disabled() function will make sure the weapon is visible/invisible
	// if SetVisible() was called while the weapon was disabled...

	m_bVisible = bVis;

	if (m_bDisabled) return;


	// Hide/Show weapon model...

    uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hObject);
	if (bVis)
	{
		dwFlags |= FLAG_VISIBLE;
	}
	else
	{
		dwFlags &= ~FLAG_VISIBLE;
	}

    g_pLTClient->SetObjectFlags(m_hObject, dwFlags);


	// Always hide the flash (it will be shown when needed)...

	m_MuzzleFlash.Hide();


	// Hide/Show silencer...

	if (m_hSilencerModel)
	{
        dwFlags = g_pLTClient->GetObjectFlags(m_hSilencerModel);
		if (bVis && m_bHaveSilencer)
		{
			dwFlags |= FLAG_VISIBLE;
		}
		else
		{
			dwFlags &= ~FLAG_VISIBLE;
		}

        g_pLTClient->SetObjectFlags(m_hSilencerModel, dwFlags);
	}


	// Hide/Show laser...

	if (m_hLaserModel)
	{
        dwFlags = g_pLTClient->GetObjectFlags(m_hLaserModel);
		if (bVis && m_bHaveLaser)
		{
			dwFlags |= FLAG_VISIBLE;
		}
		else
		{
			dwFlags &= ~FLAG_VISIBLE;
		}

        g_pLTClient->SetObjectFlags(m_hLaserModel, dwFlags);
	}

	if (bVis && m_bHaveLaser)
	{
		m_LaserBeam.TurnOn();
	}
	else
	{
		m_LaserBeam.TurnOff();
	}


	// Hide/Show scope...

	if (m_hScopeModel)
	{
        dwFlags = g_pLTClient->GetObjectFlags(m_hScopeModel);
		if (bVis && m_bHaveScope)
		{
			dwFlags |= FLAG_VISIBLE;
		}
		else
		{
			dwFlags &= ~FLAG_VISIBLE;
		}

        g_pLTClient->SetObjectFlags(m_hScopeModel, dwFlags);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CreateFlash
//
//	PURPOSE:	Create the muzzle flash
//
// ----------------------------------------------------------------------- //

void CWeaponModel::CreateFlash()
{
	if (!m_pWeapon) return;

	// Setup Breach socket (if it exists)...

	m_hBreachSocket = INVALID_MODEL_SOCKET;
	if (m_hObject)
	{
		if (g_pModelLT->GetSocket(m_hObject, "Breach", m_hBreachSocket) != LT_OK)
		{
			m_hBreachSocket = INVALID_MODEL_SOCKET;
		}
	}

	MUZZLEFLASHCREATESTRUCT mf;

    mf.bPlayerView  = LTTRUE;
	mf.hParent		= m_hObject;
	mf.pWeapon		= m_pWeapon;
	mf.vPos			= GetModelPos();
	mf.rRot			= GetModelRot();

	m_MuzzleFlash.Setup(mf);
	m_MuzzleFlash.Hide();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CreateModel
//
//	PURPOSE:	Create the weapon model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::CreateModel()
{
	if (!m_pWeapon) return;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	SAFE_STRCPY(createStruct.m_Filename, m_pWeapon->szPVModel);
	SAFE_STRCPY(createStruct.m_SkinNames[0], m_pWeapon->szPVSkin);

    // Figure out what hand skin to use...

	ModelStyle eModelStyle = eModelStyleDefault;
	CCharacterFX* pCharFX = g_pGameClientShell->GetMoveMgr()->GetCharacterFX();
	if (pCharFX)
	{
		eModelStyle = pCharFX->GetModelStyle();
	}

	if (g_pGameClientShell->GetGameType() == SINGLE || g_pGameClientShell->GetMoveMgr()->IsPlayerModel())
	{
		SAFE_STRCPY(createStruct.m_SkinNames[1], g_pModelButeMgr->GetHandsSkinFilename(eModelStyle));
	}
	else
	{
		SAFE_STRCPY(createStruct.m_SkinNames[1], "guns\\skins_pv\\MultiHands_pv.dtx");
	}

	// Check for special case of hands for the main weapon skin...

	if (strcmp(m_pWeapon->szPVSkin, "Hands") == 0)
	{
		if ( g_pGameClientShell->GetGameType() == SINGLE )
		{
			SAFE_STRCPY(createStruct.m_SkinNames[0], g_pModelButeMgr->GetHandsSkinFilename(eModelStyle));
		}
		else
		{
			SAFE_STRCPY(createStruct.m_SkinNames[0], "guns\\skins_pv\\MultiHands_pv.dtx");
		}

		// Okay, here is a nice 11th hour hack for you...We want to make sure the
		// player is using the space hands model for the space station mission...so,
		// check to see if we're on the space station...

		if (g_pGameClientShell->GetCurrentMission() == 19)
		{
			SAFE_STRCPY(createStruct.m_Filename, "guns\\models_pv\\SpaceChop_pv.abc");
		}
	}


	m_hObject = CreateModelObject(m_hObject, createStruct);
	if (!m_hObject) return;

	DoSpecialCreateModel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::DoSpecialCreateModel()
//
//	PURPOSE:	Do special case create model processing...
//
// ----------------------------------------------------------------------- //

void CWeaponModel::DoSpecialCreateModel()
{
	// Currently we need to check for gadget special cases...

	LTFLOAT r, g, b, a;
	g_pLTClient->GetObjectColor(m_hObject, &r, &g, &b, &a);
	a = 1.0f;

	if (m_pAmmo->eType == GADGET)
	{
		// If we're out of ammo, hide the necessary pieces...

		if (IsOutOfAmmo(m_nWeaponId))
		{
			// Hide the necessary pieces...

            SpecialShowPieces(LTFALSE);
		}

		// See if we should set the alpha (sunglasses)...

		if ((m_pAmmo->eInstDamageType == DT_GADGET_CAMERA) ||
		   (m_pAmmo->eInstDamageType == DT_GADGET_MINE_DETECTOR) ||
		   (m_pAmmo->eInstDamageType == DT_GADGET_INFRA_RED))
		{
			a = 0.99f;
		}
	}

	g_pLTClient->SetObjectColor(m_hObject, r, g, b, a);


	// Just do a SetupModel()...This will handle other special cases...

	SetupModel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CreateModelObject
//
//	PURPOSE:	Create a weaponmodel model object
//
// ----------------------------------------------------------------------- //

HOBJECT CWeaponModel::CreateModelObject(HOBJECT hOldObj, ObjectCreateStruct & createStruct)
{
    if (!m_pWeapon) return LTNULL;

	HOBJECT hObj = hOldObj;

	if (!hObj)
	{
		createStruct.m_ObjectType = OT_MODEL;
        createStruct.m_Flags     |= FLAG_VISIBLE | FLAG_REALLYCLOSE;
		createStruct.m_Flags2	 |= FLAG2_PORTALINVISIBLE | FLAG2_DYNAMICDIRLIGHT;

        hObj = g_pLTClient->CreateObject(&createStruct);
        if (!hObj) return LTNULL;

        //g_pLTClient->SetObjectColor(hObj, 1.0f, 1.0f, 1.0f, 1.0f);
	}

    g_pLTClient->Common()->SetObjectFilenames(hObj, &createStruct);

    uint32 dwFlags = g_pLTClient->GetObjectFlags(hObj);

	if (m_pWeapon->bEnvironmentMap)
	{
        g_pLTClient->SetObjectFlags(hObj, dwFlags | FLAG_ENVIRONMENTMAP);
	}
	else
	{
        g_pLTClient->SetObjectFlags(hObj, dwFlags & ~FLAG_ENVIRONMENTMAP);
	}

    uint32 dwCFlags = g_pLTClient->GetObjectClientFlags(hObj);
    g_pLTClient->SetObjectClientFlags(hObj, dwCFlags | CF_NOTIFYMODELKEYS);

    g_pLTClient->SetModelLooping(hObj, LTFALSE);
    g_pLTClient->SetModelAnimation(hObj, 0);

	return hObj;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CreateMods
//
//	PURPOSE:	Create any necessary mods
//
// ----------------------------------------------------------------------- //

void CWeaponModel::CreateMods()
{
	if (!m_pWeapon) return;

	// Create the available mods...

	CreateSilencer();
	CreateLaser();
	CreateScope();

	// Put the mods in their starting pos/rot...

	UpdateMods();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CreateSilencer
//
//	PURPOSE:	Create the silencer model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::CreateSilencer()
{
	if (!m_pWeapon) return;

	m_hSilencerSocket = INVALID_MODEL_SOCKET;

	// Make sure we have the silencer...

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	MOD* pMod = g_pWeaponMgr->GetMod((ModType)pStats->GetSilencer());

	if (!pMod || !pMod->szSocket[0] || !pStats->HaveMod(pMod->nId))
	{
		if (m_hSilencerModel)
		{
            uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hSilencerModel);
            g_pLTClient->SetObjectFlags(m_hSilencerModel, dwFlags & ~FLAG_VISIBLE);
		}

		return;
	}



	// Make sure we have a socket for the silencer...

	if (m_hObject)
	{
		if (g_pModelLT->GetSocket(m_hObject, pMod->szSocket, m_hSilencerSocket) != LT_OK)
		{
			if (m_hSilencerModel)
			{
                uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hSilencerModel);
                g_pLTClient->SetObjectFlags(m_hSilencerModel, dwFlags & ~FLAG_VISIBLE);
			}

			return;
		}
	}


	// Okay create/setup the model...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	SAFE_STRCPY(createStruct.m_Filename, pMod->szAttachModel);
	SAFE_STRCPY(createStruct.m_SkinNames[0], pMod->szAttachSkin);

	m_hSilencerModel = CreateModelObject(m_hSilencerModel, createStruct);

	if (m_hSilencerModel)
	{
        uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hSilencerModel);
        g_pLTClient->SetObjectFlags(m_hSilencerModel, dwFlags | FLAG_VISIBLE);
        g_pLTClient->SetObjectScale(m_hSilencerModel, &(pMod->vAttachScale));
	}

    m_bHaveSilencer = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CreateLaser
//
//	PURPOSE:	Create the laser model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::CreateLaser()
{
	if (!m_pWeapon) return;

    uint32 dwFlags;

	m_hLaserSocket = INVALID_MODEL_SOCKET;

	// Make sure we have the laser...

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	MOD* pMod = g_pWeaponMgr->GetMod((ModType)pStats->GetLaser());

	if (!pMod || !pMod->szSocket[0] || !pStats->HaveMod(pMod->nId))
	{
        uint32 dwFlags;
		if (m_hLaserModel)
		{
            dwFlags = g_pLTClient->GetObjectFlags(m_hLaserModel);
            g_pLTClient->SetObjectFlags(m_hLaserModel, dwFlags & ~FLAG_VISIBLE);
		}

		m_LaserBeam.TurnOff();
		return;
	}


	// Make sure we have a socket for the laser...

	if (m_hObject)
	{
		if (g_pModelLT->GetSocket(m_hObject, pMod->szSocket, m_hLaserSocket) != LT_OK)
		{
			if (m_hLaserModel)
			{
                dwFlags = g_pLTClient->GetObjectFlags(m_hLaserModel);
                g_pLTClient->SetObjectFlags(m_hLaserModel, dwFlags & ~FLAG_VISIBLE);
			}

			m_LaserBeam.TurnOff();
			return;
		}
	}


	// Okay create/setup the model...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	SAFE_STRCPY(createStruct.m_Filename, pMod->szAttachModel);
	SAFE_STRCPY(createStruct.m_SkinNames[0], pMod->szAttachSkin);

	m_hLaserModel = CreateModelObject(m_hLaserModel, createStruct);

	if (m_hLaserModel)
	{
        dwFlags = g_pLTClient->GetObjectFlags(m_hLaserModel);
        g_pLTClient->SetObjectFlags(m_hLaserModel, dwFlags | FLAG_VISIBLE);
        g_pLTClient->SetObjectScale(m_hLaserModel, &(pMod->vAttachScale));
	}

	m_LaserBeam.TurnOn();

    m_bHaveLaser = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CreateScope
//
//	PURPOSE:	Create the scope model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::CreateScope()
{
	if (!m_pWeapon) return;

	m_hScopeSocket = INVALID_MODEL_SOCKET;

	// Make sure we have the scope...

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();

	MOD* pMod = g_pWeaponMgr->GetMod((ModType)pStats->GetScope());
	if (!pMod || !pMod->szSocket[0] || !pStats->HaveMod(pMod->nId))
	{
		if (m_hScopeModel)
		{
            uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hScopeModel);
            g_pLTClient->SetObjectFlags(m_hScopeModel, dwFlags & ~FLAG_VISIBLE);
		}

		return;
	}



	// Make sure we have a socket for the scope...

	if (m_hObject)
	{
		if (g_pModelLT->GetSocket(m_hObject, pMod->szSocket, m_hScopeSocket) != LT_OK)
		{
			if (m_hScopeModel)
			{
                uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hScopeModel);
                g_pLTClient->SetObjectFlags(m_hScopeModel, dwFlags & ~FLAG_VISIBLE);
			}

			return;
		}
	}


	// Okay create/setup the model...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	SAFE_STRCPY(createStruct.m_Filename, pMod->szAttachModel);
	SAFE_STRCPY(createStruct.m_SkinNames[0], pMod->szAttachSkin);

	m_hScopeModel = CreateModelObject(m_hScopeModel, createStruct);

	if (m_hScopeModel)
	{
        uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hScopeModel);
        g_pLTClient->SetObjectFlags(m_hScopeModel, dwFlags | FLAG_VISIBLE);
        g_pLTClient->SetObjectScale(m_hScopeModel, &(pMod->vAttachScale));
	}

    m_bHaveScope = LTTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateFlash()
//
//	PURPOSE:	Update muzzle flash state
//
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateFlash(WeaponState eState)
{
    uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hObject);

	if (!(dwFlags & FLAG_VISIBLE) || !m_pWeapon->pPVMuzzleFX)
	{
		m_MuzzleFlash.Hide();
		return;
	}

    LTFLOAT fCurTime = g_pLTClient->GetTime();
    LTFLOAT fFlashDuration = m_pWeapon->pPVMuzzleFX->fDuration;

	if ( fCurTime >= m_fFlashStartTime + fFlashDuration ||
		 g_pGameClientShell->GetPlayerState() != PS_ALIVE ||
		 IsLiquid(g_pGameClientShell->GetCurContainerCode()) )
	{
		m_MuzzleFlash.Hide();
	}
	else
	{
		// Align the flash object to the direction the model is facing...

  		m_MuzzleFlash.Show();
		m_MuzzleFlash.SetPos(m_vFlashPos, m_vFlashOffset);
		m_MuzzleFlash.SetRot(GetModelRot());
		m_MuzzleFlash.Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::StartFlash()
//
//	PURPOSE:	Start the muzzle flash
//
// ----------------------------------------------------------------------- //

void CWeaponModel::StartFlash()
{

    m_fFlashStartTime = g_pLTClient->GetTime();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetModelPos()
//
//	PURPOSE:	Get the position of the weapon model
//
// ----------------------------------------------------------------------- //

LTVector CWeaponModel::GetModelPos() const
{
    LTVector vPos;
	vPos.Init();

	if (m_hObject)
	{
        g_pLTClient->GetObjectPos(m_hObject, &vPos);
		vPos += m_vCamPos;
	}

	return vPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetModelRot()
//
//	PURPOSE:	Get the rotation of the weapon model
//
// ----------------------------------------------------------------------- //

LTRotation CWeaponModel::GetModelRot() const
{
	return m_rCamRot;

    LTRotation rRot;
	rRot.Init();

	if (m_hObject)
	{
        g_pLTClient->GetObjectRotation(m_hObject, &rRot);
		//rRot += m_rCamRot;
	}

	return rRot;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::StringKey()
//
//	PURPOSE:	Handle animation command
//
// ----------------------------------------------------------------------- //

void CWeaponModel::OnModelKey(HLOCALOBJ hObj, ArgList* pArgList)
{
	if (!hObj || (hObj != m_hObject) || !pArgList || !pArgList->argv || pArgList->argc == 0) return;
	if (!m_pWeapon) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	if (stricmp(pKey, WEAPON_KEY_FIRE) == 0)
	{
		// Only allow fire keys if it is a fire animation...

		if (m_hObject)
		{
			uint32 dwAni = g_pLTClient->GetModelAnimation(m_hObject);
			if (IsFireAni(dwAni))
			{
				m_bFire = LTTRUE;
			}
		}
	}
	else if (stricmp(pKey, WEAPON_KEY_SOUND) == 0)
	{
		if (pArgList->argc > 1 && pArgList->argv[1])
		{
            char* pBuf = LTNULL;

			PlayerSoundId nId = (PlayerSoundId)atoi(pArgList->argv[1]);
			switch (nId)
			{
				case PSI_RELOAD:
				case PSI_RELOAD2:
				case PSI_RELOAD3:
				{
					pBuf = m_pWeapon->szReloadSounds[nId - PSI_RELOAD];
				}
				break;
				case PSI_SELECT:
					pBuf = m_pWeapon->szSelectSound;
				break;
				case PSI_DESELECT:
					pBuf = m_pWeapon->szDeselectSound;
				break;

				case PSI_INVALID:
				default : break;
			}

			if (pBuf && pBuf[0])
			{
				g_pClientSoundMgr->PlaySoundLocal(pBuf, SOUNDPRIORITY_PLAYER_HIGH);

				// Send message to Server so that other client's can hear this sound...

                uint32 dwId;
                g_pLTClient->GetLocalClientID(&dwId);

                HMESSAGEWRITE hWrite = g_pLTClient->StartMessage(MID_WEAPON_SOUND);
                g_pLTClient->WriteToMessageByte(hWrite, nId);
                g_pLTClient->WriteToMessageByte(hWrite, m_nWeaponId);
                g_pLTClient->WriteToMessageByte(hWrite, (uint8)dwId);
                g_pLTClient->WriteToMessageVector(hWrite, &m_vFlashPos);
                g_pLTClient->EndMessage2(hWrite, MESSAGE_NAGGLEFAST);
			}
		}
	}
	else if (stricmp(pKey, WEAPON_KEY_FX) == 0)
	{
		m_PVFXMgr.HandleFXKey(pArgList);
	}
	else if (stricmp(pKey, WEAPON_KEY_FIREFX) == 0)
	{
		// Only allow fire keys if it is a fire animation...

		if (m_hObject)
		{
			uint32 dwAni = g_pLTClient->GetModelAnimation(m_hObject);
			if (IsFireAni(dwAni))
			{
		        m_bFire = LTTRUE;
			}
		}

		m_PVFXMgr.HandleFXKey(pArgList);
	}
	else if (stricmp(pKey, WEAPON_KEY_SUNGLASS) == 0)
	{
		HandleSunglassMode();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::HandleSunglassWeaponKey()
//
//	PURPOSE:	Handle changing sunglass modes...
//
// ----------------------------------------------------------------------- //

void CWeaponModel::HandleSunglassMode()
{
	// Handle sunglasses gadget...

	if (m_pAmmo->eType == GADGET)
	{
		if (GetState() == W_DESELECT)
		{
			g_pInterfaceMgr->SetSunglassMode(SUN_NONE);
		}
		else if (m_pAmmo->eInstDamageType == DT_GADGET_CAMERA)
		{
			g_pInterfaceMgr->SetSunglassMode(SUN_CAMERA);
		}
		else if (m_pAmmo->eInstDamageType == DT_GADGET_MINE_DETECTOR)
		{
			g_pInterfaceMgr->SetSunglassMode(SUN_MINES);
		}
		else if (m_pAmmo->eInstDamageType == DT_GADGET_INFRA_RED)
		{
			g_pInterfaceMgr->SetSunglassMode(SUN_IR);
		}
		else
		{
			g_pInterfaceMgr->SetSunglassMode(SUN_NONE);
		}
	}
	else
	{
		g_pInterfaceMgr->SetSunglassMode(SUN_NONE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateModelState
//
//  PURPOSE:    Update the model's state (fire if bFire == LTTRUE)
//
// ----------------------------------------------------------------------- //

WeaponState CWeaponModel::UpdateModelState(LTBOOL bFire)
{
	WeaponState eRet = W_IDLE;

	// Determine what we should be doing...

	if (bFire)
	{
		UpdateFiring();
	}
	else
	{
		UpdateNonFiring();
	}


	if (m_bFire)
	{
		eRet = Fire((m_pAmmo->eType != GADGET));
	}


	// See if we just finished deselecting the weapon...

	if (m_bWeaponDeselected)
	{
        m_bWeaponDeselected = LTFALSE;

		// Change weapons if we're not chaing between normal and
		// alt-fire modes...

		if (m_nRequestedWeaponId != m_nWeaponId)
		{
			HandleInternalWeaponChange(m_nRequestedWeaponId);
		}
	}

	return eRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::Fire
//
//	PURPOSE:	Handle firing the weapon
//
// ----------------------------------------------------------------------- //

WeaponState CWeaponModel::Fire(LTBOOL bUpdateAmmo)
{

	WeaponState eRet = W_IDLE;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!pStats) return W_IDLE;

    LTBOOL bInfiniteAmmo = (g_bInfiniteAmmo || m_pWeapon->bInfiniteAmmo);
	int nAmmo = bInfiniteAmmo ? INFINITE_AMMO_AMOUNT : pStats->GetAmmoCount(m_nAmmoId);

	// If this weapon uses ammo, make sure we have ammo...

	if (nAmmo > 0)
	{
		eRet = W_FIRED;

		if (bUpdateAmmo)
		{
			DecrementAmmo();
		}
	}
	else  // NO AMMO
	{
		SetState(W_FIRING_NOAMMO);

		// Play dry-fire sound...

		if (m_pWeapon->szDryFireSound[0])
		{
			g_pClientSoundMgr->PlaySoundLocal(m_pWeapon->szDryFireSound, SOUNDPRIORITY_PLAYER_HIGH);
		}


		// Send message to Server so that other client's can hear this sound...

        uint32 dwId;
        g_pLTClient->GetLocalClientID(&dwId);

        HMESSAGEWRITE hWrite = g_pLTClient->StartMessage(MID_WEAPON_SOUND);
        g_pLTClient->WriteToMessageByte(hWrite, PSI_DRY_FIRE);
        g_pLTClient->WriteToMessageByte(hWrite, m_nWeaponId);
        g_pLTClient->WriteToMessageByte(hWrite, (uint8)dwId);
        g_pLTClient->WriteToMessageVector(hWrite, &m_vFlashPos);
        g_pLTClient->EndMessage2(hWrite, MESSAGE_NAGGLEFAST);
	}

    m_bFire = LTFALSE;

	return eRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::DecrementAmmo
//
//	PURPOSE:	Decrement the weapon's ammo count
//
// ----------------------------------------------------------------------- //

void CWeaponModel::DecrementAmmo()
{
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!pStats) return;

    LTBOOL bInfiniteAmmo = (g_bInfiniteAmmo || m_pWeapon->bInfiniteAmmo);
	int nAmmo = bInfiniteAmmo ? INFINITE_AMMO_AMOUNT : pStats->GetAmmoCount(m_nAmmoId);

	int nShotsPerClip = m_pWeapon->nShotsPerClip;

	if (m_nAmmoInClip > 0)
	{
		if (nShotsPerClip > 0)
		{
			m_nAmmoInClip--;
		}

		if (!bInfiniteAmmo)
		{
			nAmmo--;

			// Update our stats.  This will ensure that our stats are always
			// accurate (even in multiplayer)...

			pStats->UpdateAmmo(m_nWeaponId, m_nAmmoId, nAmmo, LTFALSE, LTFALSE);
		}
	}

	// Check to see if we need to reload...

	if (nShotsPerClip > 0)
	{
		if (m_nAmmoInClip <= 0)
		{
            ReloadClip(LTTRUE, nAmmo);
		}
	}


// TESTING
	// If we're out of ammo, set the appropriate state...
	if (pStats->GetAmmoCount(m_nAmmoId) <= 0)
	{
		SetState(W_FIRING_NOAMMO);
	}
// TESTING

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::ReloadClip
//
//	PURPOSE:	Fill the clip
//
// ----------------------------------------------------------------------- //

void CWeaponModel::ReloadClip(LTBOOL bPlayReload, int nNewAmmo, LTBOOL bForce)
{
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!pStats) return;


	// Handle gadget reload...

	GadgetReload();


	int nAmmoCount = pStats->GetAmmoCount(m_nAmmoId);
	int nAmmo = nNewAmmo >= 0 ? nNewAmmo : nAmmoCount;
	int nShotsPerClip = m_pWeapon->nShotsPerClip;


	// Update the player's stats...

	g_pInterfaceMgr->UpdateWeaponStats(m_nWeaponId, m_nAmmoId, nAmmo);


	// Make sure we can reload the clip...

	if (!bForce)
	{
		// Already reloading...

		if (m_hObject && (GetState() == W_RELOADING))
		{
			return;
		}

		// Clip is full...

		if (m_nAmmoInClip == nShotsPerClip || m_nAmmoInClip == nAmmoCount)
		{
			return;
		}
	}

	if (nAmmo > 0 && nShotsPerClip > 0)
	{
		m_nNewAmmoInClip = nAmmo < nShotsPerClip ? nAmmo : nShotsPerClip;

		if (bPlayReload && GetReloadAni() != INVALID_ANI)
		{
			SetState(W_RELOADING);
			return;
		}
		else
		{
			// This will get set after the reloading animation is done if
			// we are playing a reload animation...

			m_nAmmoInClip = m_nNewAmmoInClip;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateFiring
//
//	PURPOSE:	Update the animation state of the model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateFiring()
{
    m_bCanSetLastFire = LTTRUE;

	if (GetState() == W_RELOADING)
	{
		if (!PlayReloadAnimation())
		{
			SetState(W_FIRING);
		}
	}
	if (GetState() == W_IDLE)
	{
		SetState(W_FIRING);
	}
	if (GetState() == W_SELECT)
	{
		if (!PlaySelectAnimation())
		{
			SetState(W_FIRING);
		}
	}
	if (GetState() == W_DESELECT)
	{
		if (!PlayDeselectAnimation())
		{
			SetState(W_FIRING);
		}
	}
	if (GetState() == W_FIRING || GetState() == W_FIRING_NOAMMO)
	{
		if (PlayFireAnimation())
		{
            m_bCanSetLastFire = LTFALSE;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateNonFiring
//
//	PURPOSE:	Update the non-firing animation state of the model
//
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateNonFiring()
{
    m_bCanSetLastFire = LTTRUE;

	if (GetState() == W_FIRING)
	{
        if (!PlayFireAnimation(LTFALSE))
		{
			SetState(W_IDLE);
		}
		else
		{
            m_bCanSetLastFire = LTFALSE;
		}
	}
	if (GetState() == W_FIRING_NOAMMO)
	{
		SetState(W_IDLE);
	}
	if (GetState() == W_RELOADING)
	{
		if (!PlayReloadAnimation())
		{
			SetState(W_IDLE);
		}
	}
	if (GetState() == W_SELECT)
	{
		if (!PlaySelectAnimation())
		{
			SetState(W_IDLE);
		}
	}
	if (GetState() == W_DESELECT)
	{
		if (!PlayDeselectAnimation())
		{
            m_bWeaponDeselected = LTTRUE;
			SetState(W_IDLE);
		}
	}
	if (GetState() == W_IDLE)
	{
		PlayIdleAnimation();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlaySelectAnimation()
//
//	PURPOSE:	Set model to select animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlaySelectAnimation()
{
    uint32 dwSelectAni = GetSelectAni();

    if (!m_hObject || dwSelectAni == INVALID_ANI) return LTFALSE;

    uint32 dwAni    = g_pLTClient->GetModelAnimation(m_hObject);
    uint32 dwState  = g_pLTClient->GetModelPlaybackState(m_hObject);

    LTBOOL bIsSelectAni = IsSelectAni(dwAni);
	if (bIsSelectAni && (dwState & MS_PLAYDONE))
	{
        return LTFALSE;
	}
	else if (!bIsSelectAni)
	{
        g_pLTClient->SetModelLooping(m_hObject, LTFALSE);
        g_pLTClient->SetModelAnimation(m_hObject, dwSelectAni);
/*
		// Tell the server we're playing the select animation...

        HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
        g_pLTClient->WriteToMessageByte(hMessage, CP_WEAPON_STATUS);
        g_pLTClient->WriteToMessageByte(hMessage, WS_SELECT);
        g_pLTClient->EndMessage(hMessage);
*/
	}

    return LTTRUE;  // Animation playing
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayDeselectAnimation()
//
//	PURPOSE:	Set model to deselect animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayDeselectAnimation()
{
    uint32 dwDeselectAni = GetDeselectAni();

	if (!m_hObject || dwDeselectAni == INVALID_ANI)
	{
        m_bWeaponDeselected = LTTRUE;
        return LTFALSE;
	}

    uint32 dwAni    = g_pLTClient->GetModelAnimation(m_hObject);
    uint32 dwState  = g_pLTClient->GetModelPlaybackState(m_hObject);

    LTBOOL bIsDeselectAni = IsDeselectAni(dwAni);

	if (bIsDeselectAni && (dwState & MS_PLAYDONE))
	{
        m_bWeaponDeselected = LTTRUE;
        return LTFALSE;
	}

    return LTTRUE;  // Animation playing
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayFireAnimation()
//
//	PURPOSE:	Set model to firing animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayFireAnimation(LTBOOL bResetAni)
{
    if (!m_hObject) return LTFALSE;

	// Can only set the last fire type if a fire animation isn't playing
	// (i.e., we'll assume this function will return false)...

    uint32 dwAni    = g_pLTClient->GetModelAnimation(m_hObject);
    uint32 dwState  = g_pLTClient->GetModelPlaybackState(m_hObject);

    LTBOOL bIsFireAni = IsFireAni(dwAni);

	if (!bIsFireAni || (dwState & MS_PLAYDONE))
	{
		if (bResetAni)
		{
            uint32 dwFireAni = GetFireAni(m_eLastFireType);
            if (dwFireAni == INVALID_ANI) return LTFALSE;

            g_pLTClient->SetModelLooping(m_hObject, LTFALSE);
            g_pLTClient->SetModelAnimation(m_hObject, dwFireAni);
            g_pLTClient->ResetModelAnimation(m_hObject);  // Start from beginning
		}

		if (bIsFireAni && (dwState & MS_PLAYDONE))
		{
			DoSpecialEndFire();
            return LTFALSE;
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::DoSpecialFire()
//
//	PURPOSE:	Do special case fire processing
//
// ----------------------------------------------------------------------- //

void CWeaponModel::DoSpecialFire()
{
	// Currently we need to check for gadget special cases...

	if (m_pAmmo->eType == GADGET)
	{
		// Hide the necessary pieces...

        SpecialShowPieces(LTFALSE);

		// Decrement ammo count here...

		DecrementAmmo();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::DoSpecialEndFire()
//
//	PURPOSE:	Do special case end of fire animation processing
//
// ----------------------------------------------------------------------- //

void CWeaponModel::DoSpecialEndFire()
{
	// Currently we need to check for gadget special cases...

	if (m_pAmmo->eType == GADGET)
	{
		// Unhide any hidden pieces...

        SpecialShowPieces(LTTRUE);

		// If we're out of ammo, switch weapons...

		if (IsOutOfAmmo(m_nWeaponId))
		{
			AutoSelectWeapon();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::SpecialShowPieces()
//
//	PURPOSE:	Special case showing nodes
//
// ----------------------------------------------------------------------- //

void CWeaponModel::SpecialShowPieces(LTBOOL bShow, LTBOOL bForce)
{
	// Currently we need to check for gadget special cases...

	if (m_pAmmo->eType == GADGET)
	{
		// If we're out of ammo, keep hidden...

		if (bShow && !bForce)
		{
			CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
			if (pStats && pStats->GetAmmoCount(m_nAmmoId) < 1)
			{
                bShow = LTFALSE; // Hide the necessary pieces...
			}
		}

        char** s_pPieceArray = LTNULL;

		DamageType eType = m_pAmmo->eInstDamageType;
		if (eType == DT_GADGET_CAMERA_DISABLER)
		{
			s_pPieceArray = s_pCamDisPieces;
		}
		else if (eType == DT_GADGET_CODE_DECIPHERER)
		{
			s_pPieceArray = s_pCodeDecPieces;
		}

        ILTModel* pModelLT = g_pLTClient->GetModelLT();
        HMODELPIECE hPiece = LTNULL;

		for (int i=0; s_pPieceArray && s_pPieceArray[i]; i++)
		{
			if (pModelLT->GetPiece(m_hObject, s_pPieceArray[i], hPiece) == LT_OK)
			{
				pModelLT->SetPieceHideStatus(m_hObject, hPiece, !bShow);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayReloadAnimation()
//
//	PURPOSE:	Set model to reloading animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayReloadAnimation()
{
    uint32 dwReloadAni = GetReloadAni();

    if (!m_hObject || dwReloadAni == INVALID_ANI) return LTFALSE;

    uint32 dwAni    = g_pLTClient->GetModelAnimation(m_hObject);
    uint32 dwState  = g_pLTClient->GetModelPlaybackState(m_hObject);

    LTBOOL bCanPlay  = (!IsFireAni(dwAni) || g_pLTClient->GetModelLooping(m_hObject) || (dwState & MS_PLAYDONE));

    LTBOOL bIsReloadAni = IsReloadAni(dwAni);

	if (bIsReloadAni && (dwState & MS_PLAYDONE))
	{
		// Set ammo in clip amount...

		m_nAmmoInClip = m_nNewAmmoInClip;

		// Update the player's stats...

		CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
		if (pStats)
		{
			int nAmmo = pStats->GetAmmoCount(m_nAmmoId);
			g_pInterfaceMgr->UpdateWeaponStats(m_nWeaponId, m_nAmmoId, nAmmo);
		}

        return LTFALSE;
	}
	else if (!bIsReloadAni && bCanPlay)
	{
        g_pLTClient->SetModelLooping(m_hObject, LTFALSE);
        g_pLTClient->SetModelAnimation(m_hObject, dwReloadAni);

		// Tell the server we're playing the reload ani...

        HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
        g_pLTClient->WriteToMessageByte(hMessage, CP_WEAPON_STATUS);
        g_pLTClient->WriteToMessageByte(hMessage, WS_RELOADING);
        g_pLTClient->EndMessage(hMessage);
	}

    return LTTRUE;  // Animation playing
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PlayIdleAnimation()
//
//	PURPOSE:	Set model to Idle animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::PlayIdleAnimation()
{
    if (!m_hObject || g_pGameClientShell->IsZoomed()) return LTFALSE;

    LTBOOL bCurAniDone = !!(g_pLTClient->GetModelPlaybackState(m_hObject) & MS_PLAYDONE);

	// Make sure idle animation is done if one is currently playing...

    uint32 dwAni = g_pLTClient->GetModelAnimation(m_hObject);
	if (IsIdleAni(dwAni))
	{
		if (!bCurAniDone)
		{
            return LTTRUE;
		}
	}


	// See if the player is moving...Don't do normal idles when player is
	// moving...

    LTBOOL bMoving = LTFALSE;
	if (g_pGameClientShell->GetMoveMgr()->GetVelMagnitude() > 0.1f)
	{
		bMoving = !!(g_pGameClientShell->GetPlayerFlags() & BC_CFLG_MOVING);
	}


	// Play idle if it is time...(and not moving)...

    LTFLOAT fTime = g_pLTClient->GetTime();

    LTBOOL bPlayIdle = LTFALSE;

	if (fTime > m_fNextIdleTime && bCurAniDone)
	{
		bPlayIdle = !bMoving;
		m_fNextIdleTime	= GetNextIdleTime();
	}

    uint32 nSubleIdleAni = GetSubtleIdleAni();

	if (bPlayIdle)
	{
        uint32 nAni = GetIdleAni();

		if (nAni == INVALID_ANI)
		{
			nAni = DEFAULT_ANI;
		}

        g_pLTClient->SetModelLooping(m_hObject, LTFALSE);
        g_pLTClient->SetModelAnimation(m_hObject, nAni);

        return LTTRUE;
	}
	else if (nSubleIdleAni != INVALID_ANI)
	{
		// Play subtle idle...

		if (dwAni != nSubleIdleAni || bCurAniDone)
		{
            g_pLTClient->SetModelLooping(m_hObject, LTFALSE /*LTTRUE*/);
            g_pLTClient->SetModelAnimation(m_hObject, nSubleIdleAni);
            g_pLTClient->ResetModelAnimation(m_hObject);
		}

        return LTTRUE;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::Select()
//
//	PURPOSE:	Select the weapon
//
// ----------------------------------------------------------------------- //

void CWeaponModel::Select()
{
	SetState(W_SELECT);

    ReloadClip(LTFALSE);

    uint32 dwSelectAni = GetSelectAni();

	if (m_hObject && dwSelectAni != INVALID_ANI)
	{
        uint32 dwAni = g_pLTClient->GetModelAnimation(m_hObject);

		if (!IsSelectAni(dwAni))
		{
            g_pLTClient->SetModelLooping(m_hObject, LTFALSE);
            g_pLTClient->SetModelAnimation(m_hObject, dwSelectAni);
            g_pLTClient->ResetModelAnimation(m_hObject);
		}

		// Tell the server we're playing the select animation...

        HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
        g_pLTClient->WriteToMessageByte(hMessage, CP_WEAPON_STATUS);
        g_pLTClient->WriteToMessageByte(hMessage, WS_SELECT);
        g_pLTClient->EndMessage(hMessage);
	}

	// Make sure the zipcord is off (incase it was on)...

	g_pGameClientShell->GetMoveMgr()->TurnOffZipCord();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GadgetReload()
//
//	PURPOSE:	Handle gadget reload
//
// ----------------------------------------------------------------------- //

void CWeaponModel::GadgetReload()
{
	// Handle sunglasses gadget...

	if (m_pAmmo->eType == GADGET)
	{
		if (GetState() == W_SELECT)
		{
			// This will be handled by HandleSunglassMode() if we're selecting
			// the gadget...
			g_pInterfaceMgr->SetSunglassMode(SUN_NONE);
			return;
		}

		if (m_pAmmo->eInstDamageType == DT_GADGET_CAMERA)
		{
			g_pInterfaceMgr->SetSunglassMode(SUN_CAMERA);
		}
		else if (m_pAmmo->eInstDamageType == DT_GADGET_MINE_DETECTOR)
		{
			g_pInterfaceMgr->SetSunglassMode(SUN_MINES);
		}
		else if (m_pAmmo->eInstDamageType == DT_GADGET_INFRA_RED)
		{
			g_pInterfaceMgr->SetSunglassMode(SUN_IR);
		}
		else
		{
			g_pInterfaceMgr->SetSunglassMode(SUN_NONE);
		}
	}
	else
	{
		g_pInterfaceMgr->SetSunglassMode(SUN_NONE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::Deselect()
//
//	PURPOSE:	Deselect the weapon
//
// ----------------------------------------------------------------------- //

void CWeaponModel::Deselect()
{
    uint32 dwDeselectAni = GetDeselectAni();
    LTBOOL bPlayDeselectAni = (m_hObject && dwDeselectAni != INVALID_ANI);

	// Special case for gadgets...

	if (m_pAmmo->eType == GADGET && IsOutOfAmmo(m_nWeaponId))
	{
        bPlayDeselectAni = LTFALSE;
	}

	if (bPlayDeselectAni)
	{
        uint32 dwAni = g_pLTClient->GetModelAnimation(m_hObject);

		SetState(W_DESELECT);

		if (!IsDeselectAni(dwAni))
		{
            g_pLTClient->SetModelLooping(m_hObject, LTFALSE);
            g_pLTClient->SetModelAnimation(m_hObject, dwDeselectAni);
            g_pLTClient->ResetModelAnimation(m_hObject);
		}

		// Tell the server we're playing the deselect animation...

        HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
        g_pLTClient->WriteToMessageByte(hMessage, CP_WEAPON_STATUS);
        g_pLTClient->WriteToMessageByte(hMessage, WS_DESELECT);
        g_pLTClient->EndMessage(hMessage);
	}
	else
	{
        m_bWeaponDeselected = LTTRUE;
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::SendFireMsg
//
//	PURPOSE:	Send fire message to server
//
// ----------------------------------------------------------------------- //

void CWeaponModel::SendFireMsg()
{
	if (!m_hObject) return;


	// Special case, check for gadget modes that don't actually support
	// firing...

	if ((m_pAmmo->eInstDamageType == DT_GADGET_MINE_DETECTOR) ||
		(m_pAmmo->eInstDamageType == DT_GADGET_INFRA_RED))
	{
		return;
	}


	LTVector vU, vR, vF, vFirePos;
    LTFLOAT fPerturb = m_fMovementPerturb;

	if (!g_pGameClientShell->IsMultiplayerGame())
	{
		CPlayerSummaryMgr *pPSummary = g_pGameClientShell->GetPlayerSummary();
        LTFLOAT  fPerturbX = pPSummary->m_PlayerRank.fPerturbMultiplier;
		fPerturb *= fPerturbX;
	}

	if (!GetFireInfo(vU, vR, vF, vFirePos)) return;


	// Make sure we always ignore the fire sounds...

	m_wIgnoreFX = WFX_FIRESOUND | WFX_ALTFIRESND;

	if (!m_bHaveSilencer)
	{
		m_wIgnoreFX |= WFX_SILENCED;
	}

	// Calculate a random seed...(srand uses this value so it can't be 1, since
	// that has a special meaning for srand)

    uint8 nRandomSeed = GetRandom(2, 255);

	g_nRandomWeaponSeed = nRandomSeed;


	// Create a client-side projectile for every vector...

	WeaponPath wp;
	wp.nWeaponId = m_nWeaponId;
	wp.vU		 = vU;
	wp.vR		 = vR;
	wp.fPerturbR = fPerturb;
	wp.fPerturbU = wp.fPerturbR;

	for (int i=0; i < m_pWeapon->nVectorsPerRound; i++)
	{
		wp.vPath = vF;

		//srand(g_nRandomWeaponSeed);
		//g_nRandomWeaponSeed = GetRandom(2, 255);

		g_pWeaponMgr->CalculateWeaponPath(wp);

        // g_pLTClient->CPrint("Client Fire Path (%d): %.2f, %.2f, %.2f",
		// g_nRandomWeaponSeed, wp.vPath.x, wp.vPath.y, wp.vPath.z);

		// Do client-side firing...

		ClientFire(wp.vPath, vFirePos);
	}


	// Play Fire sound...

    uint8 nFireType = GetLastSndFireType();

	PlayerSoundId eSoundId = PSI_FIRE;
	if (nFireType == PSI_SILENCED_FIRE)
	{
		eSoundId = PSI_SILENCED_FIRE;
	}
	else if (nFireType == PSI_ALT_FIRE)
	{
		eSoundId = PSI_ALT_FIRE;
	}

	LTVector vPos(0, 0, 0);
	PlayWeaponSound(m_pWeapon, vPos, eSoundId, LTTRUE);


	// Send Fire message to server...

	if (m_pAmmo->eType != GADGET)
	{
        HMESSAGEWRITE hWrite = g_pLTClient->StartMessage(MID_WEAPON_FIRE);
        g_pLTClient->WriteToMessageVector(hWrite, &m_vFlashPos);
        g_pLTClient->WriteToMessageVector(hWrite, &vFirePos);
        g_pLTClient->WriteToMessageVector(hWrite, &vF);
        g_pLTClient->WriteToMessageByte(hWrite, nRandomSeed);
        g_pLTClient->WriteToMessageByte(hWrite, m_nWeaponId);
        g_pLTClient->WriteToMessageByte(hWrite, m_nAmmoId);
        g_pLTClient->WriteToMessageByte(hWrite, (LTBOOL) (m_eLastFireType == FT_ALT_FIRE));
        g_pLTClient->WriteToMessageByte(hWrite, (uint8) (fPerturb * 255.0f));
		g_pLTClient->WriteToMessageDWord(hWrite, (int) (g_pLTClient->GetTime() * 1000.0f));
        g_pLTClient->EndMessage2(hWrite, MESSAGE_NAGGLEFAST);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetFireInfo
//
//	PURPOSE:	Get the fire pos/rot
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::GetFireInfo(LTVector & vU, LTVector & vR, LTVector & vF,
                                LTVector & vFirePos)
{
	// Get the fire position / direction from the camera (so it lines
	// up correctly with the crosshairs...

    LTRotation rRot;
	if (g_pGameClientShell->IsFirstPerson() &&
		!g_pGameClientShell->IsUsingExternalCamera())
	{
		HOBJECT hCamera = g_pGameClientShell->GetCamera();
        if (!hCamera) return LTFALSE;

		g_pLTClient->GetObjectPos(hCamera, &vFirePos);
		g_pLTClient->GetObjectRotation(hCamera, &rRot);
	    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);
	}
	else
	{
		vFirePos = GetModelPos();
		rRot	 = GetModelRot();
		g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetLastSndFireType
//
//	PURPOSE:	Get the last fire snd type
//
// ----------------------------------------------------------------------- //

uint8 CWeaponModel::GetLastSndFireType()
{
	// Determine the fire snd type...

    uint8 nFireType = PSI_FIRE;

	if (m_bHaveSilencer)
	{
		nFireType = PSI_SILENCED_FIRE;
	}
	else if (m_eLastFireType == FT_ALT_FIRE)
	{
		nFireType = PSI_ALT_FIRE;
	}

	return nFireType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::ClientFire
//
//	PURPOSE:	Do client-side weapon firing
//
// ----------------------------------------------------------------------- //

void CWeaponModel::ClientFire(LTVector & vPath, LTVector & vFirePos)
{
	m_vPath		= vPath;
	m_vFirePos	= vFirePos;

	// Always process gadget firing...

	if (m_pAmmo->eType == GADGET)
	{
		DoGadget();
		return;
	}

	// Only process the rest of these if this is a multiplayer game...

	if (!g_pGameClientShell->IsMultiplayerGame()) return;

	switch (m_pAmmo->eType)
	{
		case PROJECTILE :
		{
			DoProjectile();
		}
		break;

		case VECTOR :
		{
			DoVector();
		}
		break;

		default :
		{
            g_pLTClient->CPrint("ERROR in CWeaponModel::ClientFire().  Invalid Ammo Type!");
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::DoProjectile
//
//	PURPOSE:	Do client-side projectile
//
// ----------------------------------------------------------------------- //

void CWeaponModel::DoProjectile()
{
	// All projectiles are server-side (for now)...
	return;

	if (!m_hObject) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	PROJECTILECREATESTRUCT projectile;

    uint32 dwId;
    g_pLTClient->GetLocalClientID(&dwId);

	projectile.hServerObj = CreateServerObj();
	projectile.nWeaponId  = m_nWeaponId;
	projectile.nAmmoId	  = m_nAmmoId;
    projectile.nShooterId = (uint8)dwId;
    projectile.bLocal     = LTTRUE;
	projectile.bAltFire	  = !!(m_eLastFireType == FT_ALT_FIRE);


	psfxMgr->CreateSFX(SFX_PROJECTILE_ID, &projectile);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CreateServerObj
//
//	PURPOSE:	Create a "server" object used by the projectile sfx
//
// ----------------------------------------------------------------------- //

HLOCALOBJ CWeaponModel::CreateServerObj()
{
    if (!m_hObject) return LTNULL;

    LTRotation rRot;
    g_pLTClient->AlignRotation(&rRot, &m_vPath, LTNULL);

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

    uint32 dwFlags = FLAG_POINTCOLLIDE | FLAG_NOSLIDING | FLAG_TOUCH_NOTIFY;
	dwFlags |= m_pAmmo->pProjectileFX ? m_pAmmo->pProjectileFX->dwObjectFlags : 0;

	createStruct.m_ObjectType = OT_NORMAL;
	createStruct.m_Flags = dwFlags;
	createStruct.m_Pos = m_vFirePos;
	createStruct.m_Rotation = rRot;

    HLOCALOBJ hObj = g_pLTClient->CreateObject(&createStruct);

    g_pLTClient->Physics()->SetForceIgnoreLimit(hObj, 0.0f);

	return hObj;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::DoGadget
//
//	PURPOSE:	Do client-side gadget
//
// ----------------------------------------------------------------------- //

void CWeaponModel::DoGadget()
{
	if (m_pAmmo->eInstDamageType == DT_GADGET_POODLE)
	{
        LTVector vImpactPoint(0, 0, 0);
        HandleGadgetImpact(LTNULL, vImpactPoint);
	}
	else
	{
		// Do Camera shutter fx...

		if (m_pAmmo->eInstDamageType == DT_GADGET_CAMERA)
		{
			g_pInterfaceMgr->StartScreenFadeIn(g_vtCameraShutterSpeed.GetFloat());
		}

		DoVector();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::DoVector
//
//	PURPOSE:	Do client-side vector
//
// ----------------------------------------------------------------------- //

void CWeaponModel::DoVector()
{
	if (!m_hObject) return;

	IntersectInfo iInfo;
	IntersectQuery qInfo;
	qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

    LTVector vTemp;
	VEC_MULSCALAR(vTemp, m_vPath, m_pWeapon->nRange);
	VEC_ADD(m_vEndPos, m_vFirePos, vTemp);

    HOBJECT hFilterList[] = {g_pLTClient->GetClientObject(),
        g_pGameClientShell->GetMoveMgr()->GetObject(), LTNULL};

	qInfo.m_FilterFn  = ObjListFilterFn;
	qInfo.m_pUserData = hFilterList;

	qInfo.m_From = m_vFirePos;
	qInfo.m_To = m_vEndPos;

    if (g_pLTClient->IntersectSegment(&qInfo, &iInfo))
	{
		HandleVectorImpact(qInfo, iInfo);
	}
	else
	{
        LTVector vUp;
		vUp.Init(0.0f, 1.0f, 0.0f);
        AddImpact(LTNULL, m_vEndPos, vUp, ST_SKY);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::AddImpact
//
//	PURPOSE:	Add the weapon impact
//
// ----------------------------------------------------------------------- //

void CWeaponModel::AddImpact(HLOCALOBJ hObj, LTVector & vImpactPoint,
                             LTVector & vNormal, SurfaceType eType)
{
	// Handle gadget special case...

	if (m_pAmmo->eType == GADGET)
	{
		HandleGadgetImpact(hObj, vImpactPoint);
		return;  // No impact fx for gadgets...
	}

	// See if we should do tracers or not...

	if (m_pAmmo->pTracerFX)
	{
		++m_nCurTracer;
		if ((m_nCurTracer % m_pAmmo->pTracerFX->nFrequency) != 0)
		{
			m_wIgnoreFX |= WFX_TRACER;
		}
	}
	else
	{
		m_wIgnoreFX |= WFX_TRACER;
	}

	::AddLocalImpactFX(hObj, m_vFlashPos, vImpactPoint, vNormal, eType,
					   m_vPath, m_nWeaponId, m_nAmmoId, m_wIgnoreFX);

	// If we do multiple calls to AddLocalImpact, make sure we only do some
	// effects once :)

	m_wIgnoreFX |= WFX_SILENCED | WFX_SHELL | WFX_LIGHT | WFX_MUZZLE | WFX_TRACER;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleVectorImpact
//
//	PURPOSE:	Handle a vector hitting something
//
// ----------------------------------------------------------------------- //

void CWeaponModel::HandleVectorImpact(IntersectQuery & qInfo, IntersectInfo & iInfo)
{
	// Get the surface type (check the poly first)...

	SurfaceType eType = GetSurfaceType(iInfo.m_hPoly);

	if (eType == ST_UNKNOWN)
	{
		eType = GetSurfaceType(iInfo.m_hObject);
	}

	AddImpact(iInfo.m_hObject, iInfo.m_Point, iInfo.m_Plane.m_Normal, eType);

	// If we hit liquid, cast another ray that will go through the water...

	if (eType == ST_LIQUID)
	{
		qInfo.m_FilterFn = AttackerLiquidFilterFn;

        if (g_pLTClient->IntersectSegment(&qInfo, &iInfo))
		{
			// Get the surface type (check the poly first)...

			SurfaceType eType = GetSurfaceType(iInfo.m_hPoly);

			if (eType == ST_UNKNOWN)
			{
				eType = GetSurfaceType(iInfo.m_hObject);
			}

			AddImpact(iInfo.m_hObject, iInfo.m_Point, iInfo.m_Plane.m_Normal, eType);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleGadgetImpact
//
//	PURPOSE:	Handle a gadget vector hitting an object
//
// ----------------------------------------------------------------------- //

void CWeaponModel::HandleGadgetImpact(HOBJECT hObj, LTVector vImpactPoint)
{
	// If the gadget can activate this type of object, Tell the server
	// that the gadget was activated on this object...

    LTVector vU, vR, vF, vFirePos;
	if (!GetFireInfo(vU, vR, vF, vFirePos)) return;

    uint32 dwUserFlags = 0;
	if (hObj)
	{
        g_pLTClient->GetObjectUserFlags(hObj, &dwUserFlags);
	}

	DamageType eType = m_pAmmo->eInstDamageType;

    LTBOOL bTestDamageType = LTTRUE;

	// Make sure the object isn't a character object (gadget and character
	// user flags overlap) unless this is a character specific gadget...

	if (dwUserFlags & USRFLG_CHARACTER)
	{
		// Only the lighter is used with character models currently...

		if (eType == DT_GADGET_LIGHTER)
		{
			// See if we're oriented correctly with the character
			// to use the lighter...

            LTVector vPos, vObjU, vObjR, vObjF;
			vPos = GetModelPos();

            LTVector vDir;
			vDir = vPos - vImpactPoint;
			vDir.Norm();

            LTRotation rRot;// = GetModelRot();
			g_pLTClient->GetObjectRotation(hObj, &rRot);
            g_pLTClient->GetRotationVectors(&rRot, &vObjU, &vObjR, &vObjF);

            LTFLOAT fMul = VEC_DOT(vDir, vObjF);
			if (fMul <= 0.5f) return;

			// Everything's cool...Light that baby...

            bTestDamageType = LTFALSE;
		}
		else
		{
			return;
		}
	}


	// Test the damage type if necessary...

	if (bTestDamageType)
	{
		if (eType == DT_GADGET_CAMERA_DISABLER)
		{
			// Make sure the object can be disabled...

			if (!(dwUserFlags & USRFLG_GADGET_CAMERA_DISABLER)) return;
		}
		else if (eType == DT_GADGET_CODE_DECIPHERER)
		{
			// Make sure the object can be deciphered...

			if (!(dwUserFlags & USRFLG_GADGET_CODE_DECIPHERER)) return;
		}
		else if (eType == DT_GADGET_LOCK_PICK)
		{
			// Make sure the object is "pickable"...

			if (!(dwUserFlags & USRFLG_GADGET_LOCK_PICK)) return;
		}
		else if (eType == DT_GADGET_LIGHTER)
		{
			// Make sure the object is "lightable"...

			if (!(dwUserFlags & USRFLG_GADGET_LIGHTER)) return;
		}
		else if (eType == DT_GADGET_WELDER)
		{
			// Make sure the object is "weldable"...

			if (!(dwUserFlags & USRFLG_GADGET_WELDER)) return;
		}
		else if (eType == DT_GADGET_CAMERA)
		{
			// Make sure the object is something we can photograph...

			if (dwUserFlags & USRFLG_GADGET_INTELLIGENCE)
			{
				// Make sure we're in camera range...

				if (!g_pGameClientShell->InCameraGadgetRange(hObj)) return;
			}
			else
			{
				return;
			}
		}
		else if (eType == DT_GADGET_ZIPCORD)
		{
			// Make sure the object is something we can zipcord to...

			if (dwUserFlags & USRFLG_GADGET_ZIPCORD)
			{
				// Tell the MoveMgr we're zipcording...

				if (!g_pGameClientShell->GetMoveMgr()->IsZipCordOn())
				{
					g_pGameClientShell->GetMoveMgr()->TurnOnZipCord(hObj);
				}
			}
			else
			{
				return;
			}
		}
		else if (eType != DT_GADGET_POODLE)
		{
			return;
		}
	}


    HMESSAGEWRITE hWrite = g_pLTClient->StartMessage(MID_WEAPON_FIRE);
    g_pLTClient->WriteToMessageVector(hWrite, &m_vFlashPos);
    g_pLTClient->WriteToMessageVector(hWrite, &vFirePos);
    g_pLTClient->WriteToMessageVector(hWrite, &vF);
    g_pLTClient->WriteToMessageByte(hWrite, 0);
    g_pLTClient->WriteToMessageByte(hWrite, m_nWeaponId);
    g_pLTClient->WriteToMessageByte(hWrite, m_nAmmoId);
    g_pLTClient->WriteToMessageByte(hWrite, (LTBOOL) (m_eLastFireType == FT_ALT_FIRE));
    g_pLTClient->WriteToMessageByte(hWrite, (uint8) (m_fMovementPerturb * 255.0f));
	g_pLTClient->WriteToMessageDWord(hWrite, (int) (g_pLTClient->GetTime() * 1000.0f));
    g_pLTClient->WriteToMessageObject(hWrite, hObj);
    g_pLTClient->EndMessage2(hWrite, MESSAGE_NAGGLEFAST);


	// Do any special processing...

	DoSpecialFire();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CanChangeToWeapon()
//
//	PURPOSE:	See if we can change to this weapon
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::CanChangeToWeapon(uint8 nCommandId)
{
	if (g_pGameClientShell->IsPlayerDead() ||
        g_pGameClientShell->IsSpectatorMode()) return LTFALSE;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
    if (!pStats) return LTFALSE;

    uint8 nWeaponId = g_pWeaponMgr->GetWeaponId(nCommandId);


	// Make sure this is a valid weapon for us to switch to...

    if (!pStats->HaveWeapon(nWeaponId)) return LTFALSE;



	// If this weapon has no ammo, let user know...

	if (IsOutOfAmmo(nWeaponId))
	{
		g_pInterfaceMgr->UpdatePlayerStats(IC_OUTOFAMMO_ID, nWeaponId, 0, 0.0f);
        return LTFALSE;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsOutOfAmmo()
//
//	PURPOSE:	Do we have any ammo for this weapon
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsOutOfAmmo(uint8 nWeaponId)
{
	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
    if (!pWeapon) return LTTRUE;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
    if (!pStats) return LTTRUE;

	if (pWeapon->bInfiniteAmmo)
	{
        return LTFALSE;
	}
	else
	{
		for (int i=0; i < pWeapon->nNumAmmoTypes; i++)
		{
			if (pStats->GetAmmoCount(pWeapon->aAmmoTypes[i]) > 0)
			{
                return LTFALSE;
			}
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetFirstAvailableAmmoType()
//
//	PURPOSE:	Get the fire available ammo type for this weapon.
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::GetFirstAvailableAmmoType(uint8 nWeaponId, int & nAmmoType)
{
	nAmmoType = WMGR_INVALID_ID;

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
    if (!pWeapon) return LTFALSE;

	// If we don't always have ammo, return an ammo type that we have (if
	// possible)...

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
    if (!pStats) return LTFALSE;

	for (int i=0; i < pWeapon->nNumAmmoTypes; i++)
	{
		if (pStats->GetAmmoCount(pWeapon->aAmmoTypes[i]) > 0)
		{
			nAmmoType = pWeapon->aAmmoTypes[i];
            return LTTRUE;
		}
	}

	// If we get to here (which we shouldn't), just use the default ammo
	// type if this weapon uses infinite ammo...

	if (pWeapon->bInfiniteAmmo)
	{
		nAmmoType = pWeapon->nDefaultAmmoType;
        return LTTRUE;
	}

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetBestAvailableAmmoType()
//
//	PURPOSE:	Get the best available ammo type for this weapon.
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::GetBestAvailableAmmoType(uint8 nWeaponId, int & nAmmoType)
{
	nAmmoType = WMGR_INVALID_ID;

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
    if (!pWeapon) return LTFALSE;

	// If this is a gadget, return the first ammo type...

	if (pWeapon->IsAGadget())
	{
		return GetFirstAvailableAmmoType(nWeaponId, nAmmoType);
	}
	

	// If we don't always have ammo, return an ammo type that we have (if
	// possible)...

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
    if (!pStats) return LTFALSE;

	int nAmmoBest = WMGR_INVALID_ID;
	LTFLOAT fMaxPriority = -1.0f;

	for (int i=0; i < pWeapon->nNumAmmoTypes; i++)
	{
		if (pStats->GetAmmoCount(pWeapon->aAmmoTypes[i]) > 0)
		{
			int nAmmo = pWeapon->aAmmoTypes[i];
			AMMO* pAmmo = g_pWeaponMgr->GetAmmo(nAmmo);
			if ( pAmmo->fPriority > fMaxPriority )
			{
				nAmmoBest = nAmmo;
				fMaxPriority = pAmmo->fPriority;
			}
		}
	}

	if ( nAmmoBest != WMGR_INVALID_ID )
	{
		nAmmoType = nAmmoBest;
		return LTTRUE;
	}

	// If we get to here (which we shouldn't), just use the default ammo
	// type if this weapon uses infinite ammo...

	if (pWeapon->bInfiniteAmmo)
	{
		nAmmoType = pWeapon->nDefaultAmmoType;
        return LTTRUE;
	}

	nAmmoType = WMGR_INVALID_ID;
    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::ChangeWeapon()
//
//	PURPOSE:	Change to a different weapon
//
// ----------------------------------------------------------------------- //

void CWeaponModel::ChangeWeapon(uint8 nCommandId, LTBOOL bCanDeselect)
{
	if (!CanChangeToWeapon(nCommandId)) return;

	// Don't do anything if we are trying to change to the same weapon...

    LTBOOL bDeselectWeapon = (m_nWeaponId != WMGR_INVALID_ID);
    uint8 nWeaponId = g_pWeaponMgr->GetWeaponId(nCommandId);

	if (nWeaponId == m_nWeaponId)
	{
		return;
	}

	if (g_pInterfaceMgr->IsChoosingWeapon() || g_pInterfaceMgr->IsChoosingAmmo())
		g_pInterfaceMgr->CloseChoosers();

	// Handle deselection of current weapon...

	if (bDeselectWeapon && bCanDeselect)
	{
		// Need to wait for deselection animation to finish...Save the
		// new weapon id...

		m_nRequestedWeaponId = nWeaponId;

		Deselect();
	}
	else
	{
		HandleInternalWeaponChange(nWeaponId);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::HandleInternalWeaponChange()
//
//	PURPOSE:	Change to a different weapon
//
// ----------------------------------------------------------------------- //

void CWeaponModel::HandleInternalWeaponChange(uint8 nWeaponId)
{
	if (g_pGameClientShell->IsPlayerDead() ||
		g_pGameClientShell->IsSpectatorMode()) return;


	// Change to the weapon...

	DoWeaponChange(nWeaponId);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::DoWeaponChange()
//
//	PURPOSE:	Do the actual weapon change.  This isn't part of
//				HandleInternalWeaponChange() so that it can be called when
//				loading the player.
//
// ----------------------------------------------------------------------- //

void CWeaponModel::DoWeaponChange(uint8 nWeaponId)
{
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!pStats) return;

	DoSpecialWeaponChange();

	if (pStats->HaveWeapon(nWeaponId))
	{
		int nAmmoId = WMGR_INVALID_ID;
		if (GetBestAvailableAmmoType(nWeaponId, nAmmoId) && (nAmmoId != WMGR_INVALID_ID))
		{
			g_pGameClientShell->ChangeWeapon(nWeaponId, nAmmoId, pStats->GetAmmoCount(nAmmoId));
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::DoSpecialWeaponChange()
//
//	PURPOSE:	Do special case weapon change processing...
//
// ----------------------------------------------------------------------- //

void CWeaponModel::DoSpecialWeaponChange()
{
	// Currently we need to check for gadget special cases...

	if (m_pAmmo->eType == GADGET)
	{
		// Show the necessary pieces (so they aren't hidden on the new
		// weapon model)...

        SpecialShowPieces(LTTRUE, LTTRUE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::AutoSelectWeapon()
//
//	PURPOSE:	Determine what weapon to switch to, and switch
//
// ----------------------------------------------------------------------- //

void CWeaponModel::AutoSelectWeapon()
{
	// Try and auto-select a new ammo type before auto selecting a new weapon...

	int nAmmoId = WMGR_INVALID_ID;
	if (GetBestAvailableAmmoType(m_nWeaponId, nAmmoId) && (nAmmoId != WMGR_INVALID_ID))
	{
		// Set our ammo type...

		m_nAmmoId	= nAmmoId;
		m_pAmmo		= g_pWeaponMgr->GetAmmo(m_nAmmoId);

		// Reload our clip...

        ReloadClip(LTTRUE);
		return;
	}

	// Okay, need to change, find the next weapon/gadget that will
	// do damage...

	ChangeToNextRealWeapon();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::ChangeToNextRealWeapon()
//
//	PURPOSE:	Change to the next weapon/gadget that does damage.
//				(used by the auto weapon switching)
//
// ----------------------------------------------------------------------- //

void CWeaponModel::ChangeToNextRealWeapon()
{
	if (!m_pWeapon) return;

	// If we're supposed to hide the weapon when it is empty (i.e.,
	// it doesn't make sense to see it, like for the poodle, or lipsticks)
	// then we don't want to play the deselect animation...

	LTBOOL bCanDeselect = !m_pWeapon->bHideWhenEmpty;

	// Find the next weapon that does damage...

	uint8 nCurrWeaponId = m_nWeaponId;

	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!pStats) return;

	int nMinWeapon = g_pWeaponMgr->GetFirstWeaponCommandId();
	int nMaxWeapon = g_pWeaponMgr->GetLastWeaponCommandId();
	int nOriginalWeapon = g_pWeaponMgr->GetCommandId(nCurrWeaponId);

	int nWeaponCommand = nOriginalWeapon + 1;
	if (nWeaponCommand > nMaxWeapon) nWeaponCommand = nMinWeapon;
	int nWeaponIndex = g_pWeaponMgr->GetWeaponId(nWeaponCommand);

	uint8 nMeleeId = WMGR_INVALID_ID;

	while (1)
	{
		if (pStats->HaveWeapon(nWeaponIndex) && !IsOutOfAmmo(nWeaponIndex))
		{
			WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponIndex);
			if (pWeapon)
			{
				AMMO* pAmmo = g_pWeaponMgr->GetAmmo(pWeapon->nDefaultAmmoType);
				if (pAmmo)
				{
					if (pAmmo->eInstDamageType == DT_MELEE)
					{
						nMeleeId = nWeaponIndex;
					}
					else if (!IsGadgetAmmo(pAmmo))
					{
						ChangeWeapon(g_pWeaponMgr->GetCommandId(nWeaponIndex), bCanDeselect);
						return;
					}
				}
			}
		}

		nWeaponCommand++;

		if (nWeaponCommand > nMaxWeapon) nWeaponCommand = nMinWeapon;

		if (nWeaponCommand == nOriginalWeapon) break;

		nWeaponIndex = g_pWeaponMgr->GetWeaponId(nWeaponCommand);
	}

	// Check to see if we should change to the melee weapon...

	if (nMeleeId != WMGR_INVALID_ID)
	{
		ChangeWeapon(g_pWeaponMgr->GetCommandId(nMeleeId), bCanDeselect);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsGadgetAmmo()
//
//	PURPOSE:	Is the ammo used by a gadget
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsGadgetAmmo(AMMO* pAmmo)
{
	if (!pAmmo) return LTFALSE;

	if (IsGadgetType(pAmmo->eInstDamageType)) return LTTRUE;

	// Special case

	if (strcmpi(pAmmo->szName, "Coin") == 0) return LTTRUE;

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::PrevWeapon()
//
//	PURPOSE:	Determine what the previous weapon is
//
// ----------------------------------------------------------------------- //

uint8 CWeaponModel::PrevWeapon(uint8 nCurrWeaponId)
{
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!pStats) return -1;

	if (!g_pWeaponMgr->IsValidWeapon(nCurrWeaponId))
	{
		nCurrWeaponId = m_nWeaponId;
	}

	if (!pStats->HaveWeapon(nCurrWeaponId)) return -1;

	int nMinWeapon = g_pWeaponMgr->GetFirstWeaponCommandId();
	int nMaxWeapon = g_pWeaponMgr->GetLastWeaponCommandId();
	int nOriginalWeapon = g_pWeaponMgr->GetCommandId(nCurrWeaponId);

	int nWeapon = nOriginalWeapon - 1;
	if (nWeapon < nMinWeapon) nWeapon = nMaxWeapon;
	int nWeaponIndex = g_pWeaponMgr->GetWeaponId(nWeapon);

	while (!pStats->HaveWeapon(nWeaponIndex) || IsOutOfAmmo(nWeaponIndex))
	{
		nWeapon--;

		if (nWeapon < nMinWeapon) nWeapon = nMaxWeapon;

		if (nWeapon == nOriginalWeapon) break;

		nWeaponIndex = g_pWeaponMgr->GetWeaponId(nWeapon);
	}

    return (uint8)g_pWeaponMgr->GetWeaponId(nWeapon);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::NextWeapon()
//
//	PURPOSE:	Determine what the next weapon is
//
// ----------------------------------------------------------------------- //

uint8 CWeaponModel::NextWeapon(uint8 nCurrWeaponId)
{
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!pStats) return -1;

	if (!g_pWeaponMgr->IsValidWeapon(nCurrWeaponId))
	{
		nCurrWeaponId = m_nWeaponId;
	}

	if (!pStats->HaveWeapon(nCurrWeaponId)) return -1;

	int nMinWeapon = g_pWeaponMgr->GetFirstWeaponCommandId();
	int nMaxWeapon = g_pWeaponMgr->GetLastWeaponCommandId();
	int nOriginalWeapon = g_pWeaponMgr->GetCommandId(nCurrWeaponId);

	int nWeapon = nOriginalWeapon + 1;
	if (nWeapon > nMaxWeapon) nWeapon = nMinWeapon;
	int nWeaponIndex = g_pWeaponMgr->GetWeaponId(nWeapon);

	while (!pStats->HaveWeapon(nWeaponIndex) || IsOutOfAmmo(nWeaponIndex))
	{
		nWeapon++;

		if (nWeapon > nMaxWeapon) nWeapon = nMinWeapon;

		if (nWeapon == nOriginalWeapon) break;

		nWeaponIndex = g_pWeaponMgr->GetWeaponId(nWeapon);
	}

    return (uint8)g_pWeaponMgr->GetWeaponId(nWeapon);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::NextAmmo()
//
//	PURPOSE:	Determine the next ammo type
//
// ----------------------------------------------------------------------- //

uint8 CWeaponModel::NextAmmo(uint8 nCurrAmmoId)
{
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!pStats || !m_pWeapon) return -1;

	if (!g_pWeaponMgr->IsValidAmmoType(nCurrAmmoId))
	{
		nCurrAmmoId = m_nAmmoId;
	}

	int nNewAmmoId = nCurrAmmoId;
	int nOriginalAmmoIndex = 0;
	int nCurAmmoIndex = 0;
	int nAmmoCount = 0;

	for (int i=0; i < m_pWeapon->nNumAmmoTypes; i++)
	{
		if (nCurrAmmoId == m_pWeapon->aAmmoTypes[i])
		{
			nOriginalAmmoIndex = i;
			nCurAmmoIndex = i;
			break;
		}
	}

	while (1)
	{
		nCurAmmoIndex++;

		if (nCurAmmoIndex >= m_pWeapon->nNumAmmoTypes) nCurAmmoIndex = 0;
		if (nCurAmmoIndex == nOriginalAmmoIndex) break;

		nAmmoCount = pStats->GetAmmoCount(m_pWeapon->aAmmoTypes[nCurAmmoIndex]);
		if (nAmmoCount > 0)
		{
			nNewAmmoId = m_pWeapon->aAmmoTypes[nCurAmmoIndex];
			break;
		}
	}

	return nNewAmmoId;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::ChangeAmmo()
//
//	PURPOSE:	Change to the specified ammo type
//
// ----------------------------------------------------------------------- //

void CWeaponModel::ChangeAmmo(uint8 nNewAmmoId, LTBOOL bForce)
{
	// Update the player's stats...

	if (GetState() == W_RELOADING && !bForce) return;

	
	// Make sure this is an ammo type our current weapon can use...

	if (!m_pWeapon) return;
	for (int i=0; i < m_pWeapon->nNumAmmoTypes; i++)
	{
		if (nNewAmmoId == m_pWeapon->aAmmoTypes[i])
		{
			break;
		}
	}
	if (i == m_pWeapon->nNumAmmoTypes) return;  // Not a valid ammo type


	if (g_pWeaponMgr->IsValidAmmoType(nNewAmmoId) && nNewAmmoId != m_nAmmoId)
	{
		m_nAmmoId	= nNewAmmoId;
		m_pAmmo		= g_pWeaponMgr->GetAmmo(m_nAmmoId);

		// Make sure we reset the anis...

		InitAnimations(LTTRUE);

		if (m_pAmmo->pAniOverrides)
		{
			// If we're not using the defaults play the new select ani...

			Select();
		}
		else
		{
			// Do normal reload...

            ReloadClip(LTTRUE, -1, LTTRUE);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetWeaponOffset()
//
//	PURPOSE:	Set the weapon offset
//
// ----------------------------------------------------------------------- //

LTVector CWeaponModel::GetWeaponOffset()
{
    LTVector vRet;
	vRet.Init();

	if (!m_pWeapon) return vRet;
	return m_pWeapon->vPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::SetWeaponOffset()
//
//	PURPOSE:	Set the weapon offset
//
// ----------------------------------------------------------------------- //

void CWeaponModel::SetWeaponOffset(LTVector vPos)
{
	WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(m_nWeaponId);

	if (!m_pWeapon) return;
	m_pWeapon->vPos = vPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetMuzzleOffset()
//
//	PURPOSE:	Set the weapon muzzle offset
//
// ----------------------------------------------------------------------- //

LTVector CWeaponModel::GetMuzzleOffset()
{
    LTVector vRet;
	vRet.Init();

	if (!m_pWeapon) return vRet;
	return m_pWeapon->vMuzzlePos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::SetMuzzleOffset()
//
//	PURPOSE:	Set the muzzle offset
//
// ----------------------------------------------------------------------- //

void CWeaponModel::SetMuzzleOffset(LTVector vPos)
{
	if (!m_pWeapon) return;
	m_pWeapon->vMuzzlePos = vPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetShellEjectPos()
//
//	PURPOSE:	Get the shell eject pos
//
// ----------------------------------------------------------------------- //

LTVector CWeaponModel::GetShellEjectPos(LTVector & vOriginalPos)
{
    LTVector vPos = vOriginalPos;

	if (m_hObject && m_hBreachSocket != INVALID_MODEL_SOCKET)
	{
		LTransform transform;
        if (g_pModelLT->GetSocketTransform(m_hObject, m_hBreachSocket, transform, LTTRUE) == LT_OK)
		{
			g_pTransLT->GetPos(transform, vPos);
		}
	}

	return vPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetNextIdleTime()
//
//	PURPOSE:	Determine the next time we should play an idle animation
//
// ----------------------------------------------------------------------- //

LTFLOAT CWeaponModel::GetNextIdleTime()
{
    return g_pLTClient->GetTime() + GetRandom(WEAPON_MIN_IDLE_TIME, WEAPON_MAX_IDLE_TIME);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetFireAni()
//
//	PURPOSE:	Get the fire animation based on the fire type
//
// ----------------------------------------------------------------------- //

uint32 CWeaponModel::GetFireAni(FireType eFireType)
{
	int nNumValid = 0;

	if ((eFireType == FT_ALT_FIRE && !CanUseAltFireAnis()) ||
		(m_bUsingAltFireAnis && eFireType == FT_NORMAL_FIRE))
	{
        uint32 dwValidAltFireAnis[WM_MAX_ALTFIRE_ANIS];

		for (int i=0; i < WM_MAX_ALTFIRE_ANIS; i++)
		{
			if (m_nAltFireAnis[i] != INVALID_ANI)
			{
				dwValidAltFireAnis[nNumValid] = m_nAltFireAnis[i];
				nNumValid++;
			}
		}

		if (nNumValid > 0)
		{
			return dwValidAltFireAnis[GetRandom(0, nNumValid-1)];
		}
	}
	else if (eFireType == FT_NORMAL_FIRE)
	{
        uint32 dwValidFireAnis[WM_MAX_FIRE_ANIS];

		for (int i=0; i < WM_MAX_FIRE_ANIS; i++)
		{
			if (m_nFireAnis[i] != INVALID_ANI)
			{
				dwValidFireAnis[nNumValid] = m_nFireAnis[i];
				nNumValid++;
			}
		}

		if (nNumValid > 0)
		{
			return dwValidFireAnis[GetRandom(0, nNumValid-1)];
		}
	}

	return INVALID_ANI;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetIdleAni()
//
//	PURPOSE:	Get an idle animation
//
// ----------------------------------------------------------------------- //

uint32 CWeaponModel::GetIdleAni()
{
	int nNumValid = 0;

	if (m_bUsingAltFireAnis)
	{
        uint32 dwValidAltIdleAnis[WM_MAX_ALTIDLE_ANIS];

		// Note that we skip the first ani, this is reserved for
		// the subtle idle ani...

		for (int i=1; i < WM_MAX_ALTIDLE_ANIS; i++)
		{
			if (m_nAltIdleAnis[i] != INVALID_ANI)
			{
				dwValidAltIdleAnis[nNumValid] = m_nAltIdleAnis[i];
				nNumValid++;
			}
		}

		if (nNumValid > 0)
		{
			return dwValidAltIdleAnis[GetRandom(0, nNumValid-1)];
		}
	}
	else  // Normal idle anis
	{
        uint32 dwValidIdleAnis[WM_MAX_IDLE_ANIS];

		// Note that we skip the first ani, this is reserved for
		// the subtle idle ani...

		for (int i=1; i < WM_MAX_IDLE_ANIS; i++)
		{
			if (m_nIdleAnis[i] != INVALID_ANI)
			{
				dwValidIdleAnis[nNumValid] = m_nIdleAnis[i];
				nNumValid++;
			}
		}

		if (nNumValid > 0)
		{
			return dwValidIdleAnis[GetRandom(0, nNumValid-1)];
		}
	}


	return INVALID_ANI;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetSubtleIdleAni()
//
//	PURPOSE:	Get a sutble idle animation
//
// ----------------------------------------------------------------------- //

uint32 CWeaponModel::GetSubtleIdleAni()
{
	return m_bUsingAltFireAnis ? m_nAltIdleAnis[0] : m_nIdleAnis[0];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetSelectAni()
//
//	PURPOSE:	Get a select animation
//
// ----------------------------------------------------------------------- //

uint32 CWeaponModel::GetSelectAni()
{
	return m_bUsingAltFireAnis ? m_nAltSelectAni : m_nSelectAni;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetReloadAni()
//
//	PURPOSE:	Get a reload animation
//
// ----------------------------------------------------------------------- //

uint32 CWeaponModel::GetReloadAni()
{
	return m_bUsingAltFireAnis ? m_nAltReloadAni : m_nReloadAni;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::GetDeselectAni()
//
//	PURPOSE:	Get a deselect animation
//
// ----------------------------------------------------------------------- //

uint32 CWeaponModel::GetDeselectAni()
{
    uint32 dwAni = INVALID_ANI;


	if (m_bUsingAltFireAnis)
	{
		// If we're actually changing weapons make sure we use the
		// currect AltDeselect animation...

		if (m_nRequestedWeaponId != WMGR_INVALID_ID &&
			m_nRequestedWeaponId != m_nWeaponId)
		{
			dwAni = m_nAltDeselect2Ani;
		}
		else
		{
			dwAni = m_nAltDeselectAni;
		}
	}
	else
	{
		dwAni = m_nDeselectAni;
	}

	return dwAni;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsFireAni()
//
//	PURPOSE:	Is the passed in animation a fire animation
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsFireAni(uint32 dwAni)
{
    if (dwAni == INVALID_ANI) return LTFALSE;

    int i;
    for (i=0; i < WM_MAX_FIRE_ANIS; i++)
	{
		if (m_nFireAnis[i] == dwAni)
		{
            return LTTRUE;
		}
	}

	for (i=0; i < WM_MAX_ALTFIRE_ANIS; i++)
	{
		if (m_nAltFireAnis[i] == dwAni)
		{
            return LTTRUE;
		}
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsIdleAni()
//
//	PURPOSE:	Is the passed in animation an idle animation (NOTE this
//              will return LTFALSE if the passed in animation is a subtle
//				idle animation).
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsIdleAni(uint32 dwAni)
{
    if (dwAni == INVALID_ANI) return LTFALSE;

    int i;
    for (i=1; i < WM_MAX_IDLE_ANIS; i++)
	{
		if (m_nIdleAnis[i] == dwAni)
		{
            return LTTRUE;
		}
	}

	for (i=1; i < WM_MAX_ALTIDLE_ANIS; i++)
	{
		if (m_nAltIdleAnis[i] == dwAni)
		{
            return LTTRUE;
		}
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsDeselectAni()
//
//	PURPOSE:	Is this a valid deselect ani
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsDeselectAni(uint32 dwAni)
{
    if (dwAni == INVALID_ANI) return LTFALSE;

	if (dwAni == m_nDeselectAni ||
		dwAni == m_nAltDeselectAni ||
		dwAni == m_nAltDeselect2Ani)
	{
        return LTTRUE;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsSelectAni()
//
//	PURPOSE:	Is this a valid Select ani
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsSelectAni(uint32 dwAni)
{
    if (dwAni == INVALID_ANI) return LTFALSE;

	if (dwAni == m_nSelectAni || dwAni == m_nAltSelectAni)
	{
        return LTTRUE;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::IsReloadAni()
//
//	PURPOSE:	Is this a valid Reload ani
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::IsReloadAni(uint32 dwAni)
{
    if (dwAni == INVALID_ANI) return LTFALSE;

	if (dwAni == m_nReloadAni || dwAni == m_nAltReloadAni)
	{
        return LTTRUE;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::CanUseAltFireAnis()
//
//	PURPOSE:	Can we use alt-fire anis?
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::CanUseAltFireAnis()
{
	return (m_nAltSelectAni != INVALID_ANI);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::HandleFireKeyDown()
//
//	PURPOSE:	Handle fire key down
//
// ----------------------------------------------------------------------- //

void CWeaponModel::HandleFireKeyDown()
{
	// Only handle alt-fire case on weapons that have special
	// Alt-fire animations...

	if (m_eLastFireType != FT_ALT_FIRE || !CanUseAltFireAnis()) return;

	// If we aren't playing the select, deselect, or fire ani, it is
	// okay to toggle using Alt-Fire Anis on/off...

    uint32 dwAni = g_pLTClient->GetModelAnimation(m_hObject);

	if (IsSelectAni(dwAni) || IsDeselectAni(dwAni) || IsFireAni(dwAni))
	{
		return;
	}


	// Toggle use of Alt-Fire Anis on/off...

	// Alright we need to either select or deselect the alt-fire
	// aspect of the weapon.  This is a bit tricky since the
	// select/deselect code depends on the current value of
	// m_bUsingAltFireAnis, and we want to change that value here.
	//
	// So, for the select case (i.e., m_bUsingAltFireAni == TRUE AFTER
	// it is toggled), we'll go ahead and toggle it first...).
	//
	// However, for the deselect case (i.e., m_bUsingAltFireAni
	// == TRUE BEFORE it is toggled), we'll toggle it after we
	// call Deslect...


	// See if we need to call Select...

	if (!m_bUsingAltFireAnis)
	{
		// Toggle so Select knows the right ani to play...

		m_bUsingAltFireAnis = !m_bUsingAltFireAnis;

		Select();
	}
	else
	{
		// Call deselect, then toggle...

		Deselect();

		m_bUsingAltFireAnis = !m_bUsingAltFireAnis;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::HandleFireKeyUp()
//
//	PURPOSE:	Handle fire key up
//
// ----------------------------------------------------------------------- //

void CWeaponModel::HandleFireKeyUp()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::SetState()
//
//	PURPOSE:	Set our m_eState data member
//
// ----------------------------------------------------------------------- //

WeaponState CWeaponModel::SetState(WeaponState eNewState)
{
	WeaponState eOldState = m_eState;

	m_eState = eNewState;

	if (GetState() == W_IDLE)
	{
		// Earliest we can play a non-subtle idle ani...

		m_fNextIdleTime	= GetNextIdleTime();
	}

	return eOldState;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::UpdateMovementPerturb()
//
//	PURPOSE:	Update our weapon perturb value
//
// ----------------------------------------------------------------------- //

void CWeaponModel::UpdateMovementPerturb()
{
	// Make sure the weapon has perturb...

	if (m_pWeapon && m_pWeapon->nMaxPerturb == 0)
	{
		m_fMovementPerturb = 0.0f;
		return;
	}

    LTFLOAT fDelta = g_pGameClientShell->GetFrameTime();
    LTFLOAT fMove = g_pGameClientShell->GetMoveMgr()->GetMovementPercent();
	if (fMove > 1.0f)
	{
		fMove = 1.0f;
	}

	//if walking
	if (!(g_pGameClientShell->GetMoveMgr()->GetControlFlags() & BC_CFLG_RUN) || g_pGameClientShell->IsZoomed())
	{
		fMove *= g_vtPerturbWalkPercent.GetFloat();
	}

	if (g_pGameClientShell->GetDamageFXMgr()->IsPoisoned() || g_pGameClientShell->GetDamageFXMgr()->IsStunned())
	{
		fMove = 1.0f;
	}


    LTVector vPlayerRot;
	g_pGameClientShell->GetPlayerPitchYawRoll(vPlayerRot);
    LTFLOAT fPitchDiff = (LTFLOAT)fabs(vPlayerRot.x - m_fLastPitch);
    LTFLOAT fYawDiff = (LTFLOAT)fabs(vPlayerRot.y - m_fLastYaw);
	m_fLastPitch = vPlayerRot.x;
	m_fLastYaw = vPlayerRot.y;
    LTFLOAT fRot = g_vtPerturbRotationEffect.GetFloat() * (fPitchDiff + fYawDiff) / (2.0f + g_vtFastTurnRate.GetFloat() * fDelta);
	if (fRot > 1.0f)
		fRot = 1.0f;

    LTFLOAT fAdj = Max(fRot,fMove);
    LTFLOAT fDiff = (LTFLOAT)fabs(fAdj - m_fMovementPerturb);
	if (fAdj >  m_fMovementPerturb)
	{
		fDelta *= g_vtPerturbIncreaseSpeed.GetFloat();
		m_fMovementPerturb += Min(fDelta,fDiff);
	}
	else if (fAdj <  m_fMovementPerturb)
	{
		fDelta *= g_vtPerturbDecreaseSpeed.GetFloat();
		m_fMovementPerturb -= Min(fDelta,fDiff);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::ToggleHolster()
//
//	PURPOSE:	Holster or unholster our weapon
//
// ----------------------------------------------------------------------- //

void CWeaponModel::ToggleHolster(LTBOOL bPlayDeselect)
{
	// if weapon isn't hand
	if (!IsMeleeWeapon())
	{
		m_nHolsterWeaponId = m_nWeaponId;
		ChangeWeapon(g_pWeaponMgr->GetCommandId(MeleeWeapon()), bPlayDeselect);
	}
	else
	{
		ChangeWeapon(g_pWeaponMgr->GetCommandId(m_nHolsterWeaponId), bPlayDeselect);
		m_nHolsterWeaponId = m_nWeaponId;
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::SetHolster()
//
//	PURPOSE:	Set what weapon is in our holster
//
// ----------------------------------------------------------------------- //

void CWeaponModel::SetHolster(uint8 nWeaponId)
{
	if (!g_pWeaponMgr->IsValidWeapon(nWeaponId)) return;
	m_nHolsterWeaponId = nWeaponId;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::MeleeWeapon()
//
//	PURPOSE:	Determine what the current melee weapon is
//
// ----------------------------------------------------------------------- //

uint8 CWeaponModel::MeleeWeapon()
{
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!pStats) return -1;

	int nWeapon = g_pWeaponMgr->GetFirstWeaponCommandId();
	int nMaxWeapon = g_pWeaponMgr->GetLastWeaponCommandId();
	int nMelee = -1;

	while (nWeapon <= nMaxWeapon && nMelee < 0)
	{
		int nWeaponIndex = g_pWeaponMgr->GetWeaponId(nWeapon);
		if (pStats->HaveWeapon(nWeaponIndex))
		{
			WEAPON *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponIndex);
			int nAmmoId = pWeapon->nDefaultAmmoType;
			AMMO *pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
			if (pAmmo && pAmmo->eInstDamageType == DT_MELEE)
				nMelee = nWeapon;
		}
		nWeapon++;
	}

    return (uint8)g_pWeaponMgr->GetWeaponId(nMelee);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::HandleZipCordFire()
//
//	PURPOSE:	Handle the zip-cord firing...
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponModel::HandleZipCordFire()
{
	CPlayerStats* pStats = g_pGameClientShell->GetPlayerStats();
	if (!pStats) return LTFALSE;

	// If we're currently zip-cording, turn it off..

	if (g_pGameClientShell->GetMoveMgr()->IsZipCordOn())
	{
		if (!m_bFireKeyDownLastUpdate)
		{
			g_pGameClientShell->GetMoveMgr()->TurnOffZipCord();
		}

		return LTFALSE;
	}
	else if (!pStats->DrawingActivateGadget())
	{
		// We're not targeting a zip hook so this fire message
		// really didn't count...

		if (!m_bFireKeyDownLastUpdate)
		{
			// Play "sorry try again" sound...

			char* pSound = "Guns\\snd\\zipcord\\notarget.wav";
			g_pClientSoundMgr->PlaySoundLocal(pSound,
				SOUNDPRIORITY_PLAYER_HIGH, PLAYSOUND_CLIENT);
		}

		return LTFALSE;
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModel::SetupModel
//
//	PURPOSE:	Make sure the player-view model is using the correct
//				textures (based on the player's style).
//
// ----------------------------------------------------------------------- //

void CWeaponModel::SetupModel()
{
	if (!m_hObject || !m_pWeapon) return;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	SAFE_STRCPY(createStruct.m_Filename, m_pWeapon->szPVModel);
	SAFE_STRCPY(createStruct.m_SkinNames[0], m_pWeapon->szPVSkin);

    // Figure out what hand skin to use...

	ModelStyle eModelStyle = eModelStyleDefault;
	CCharacterFX* pCharFX = g_pGameClientShell->GetMoveMgr()->GetCharacterFX();
	if (pCharFX)
	{
		eModelStyle = pCharFX->GetModelStyle();
	}

	if (g_pGameClientShell->GetGameType() == SINGLE || g_pGameClientShell->GetMoveMgr()->IsPlayerModel())
	{
		SAFE_STRCPY(createStruct.m_SkinNames[1], g_pModelButeMgr->GetHandsSkinFilename(eModelStyle));
	}
	else
	{
		SAFE_STRCPY(createStruct.m_SkinNames[1], "guns\\skins_pv\\MultiHands_pv.dtx");
	}

	// Check for special case of hands for the main weapon skin...

	if (strcmp(m_pWeapon->szPVSkin, "Hands") == 0)
	{
		if ( g_pGameClientShell->GetGameType() == SINGLE )
		{
			SAFE_STRCPY(createStruct.m_SkinNames[0], g_pModelButeMgr->GetHandsSkinFilename(eModelStyle));
		}
		else
		{
			SAFE_STRCPY(createStruct.m_SkinNames[0], "guns\\skins_pv\\MultiHands_pv.dtx");
		}

		// Okay, here is a nice 11th hour hack for you...We want to make sure the
		// player is using the space hands model for the space station mission...so,
		// check to see if we're on the space station...

		if (g_pGameClientShell->GetCurrentMission() == 19)
		{
			SAFE_STRCPY(createStruct.m_Filename, "guns\\models_pv\\SpaceChop_pv.abc");
		}
	}


	// Set the filenames...

    g_pLTClient->Common()->SetObjectFilenames(m_hObject, &createStruct);
}