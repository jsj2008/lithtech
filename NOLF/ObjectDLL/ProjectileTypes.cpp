// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileTypes.cpp
//
// PURPOSE : Projectile classs - implementation
//
// CREATED : 10/3/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ProjectileTypes.h"
#include "ServerUtilities.h"
#include "DamageTypes.h"
#include "SurfaceFunctions.h"
#include "SoundMgr.h"
#include "iltphysics.h"
#include "CVarTrack.h"
#include "Spawner.h"
#include "DebrisFuncs.h"
#include "Character.h"
#include "PlayerObj.h"
#include "CharacterHitBox.h"
#include "Body.h"

#define DEFAULT_GRENADE_DAMPEN_PERCENT	0.35f
#define DEFAULT_GRENADE_MIN_VELOCITY	10.0f

static CVarTrack g_vtGrenadeDampenPercent;
static CVarTrack g_vtGrenadeMinVelMag;

static CVarTrack g_vtProxGrenadeArmDelay;
static CVarTrack g_vtProxGrenadeDetonateDelay;

static CVarTrack g_vtSpearStickPercentage;

extern CVarTrack g_vtNetFriendlyFire;

CTList<CGrenade*> g_lstGrenades;

BEGIN_CLASS(CGrenade)
END_CLASS_DEFAULT_FLAGS(CGrenade, CProjectile, NULL, NULL, CF_HIDDEN)

BEGIN_CLASS(CLipstickProx)
END_CLASS_DEFAULT_FLAGS(CLipstickProx, CGrenade, NULL, NULL, CF_HIDDEN)

BEGIN_CLASS(CLipstickImpact)
END_CLASS_DEFAULT_FLAGS(CLipstickImpact, CGrenade, NULL, NULL, CF_HIDDEN)

BEGIN_CLASS(CLipstickTimed)
END_CLASS_DEFAULT_FLAGS(CLipstickTimed, CGrenade, NULL, NULL, CF_HIDDEN)

BEGIN_CLASS(CSpear)
END_CLASS_DEFAULT_FLAGS(CSpear, CProjectile, NULL, NULL, CF_HIDDEN)

BEGIN_CLASS(CCoin)
END_CLASS_DEFAULT_FLAGS(CCoin, CGrenade, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::CGrenade
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CGrenade::CGrenade() : CProjectile()
{
	// Uncomment these lines to use normal, non-point collide physics.
	// (point collide physics is faster and should be used whenever
	// possible).
	//m_dwFlags &= ~FLAG_POINTCOLLIDE;
	//m_dwFlags |= FLAG_SOLID | FLAG_CLIENTNONSOLID;

	m_vDims.Init(5.0f, 5.0f, 5.0f);

    m_bSpinGrenade  = !IsMultiplayerGame();
    m_bUpdating     = LTTRUE;

	m_fPitch	= 0.0f;
	m_fYaw		= 0.0f;
	m_fRoll		= 0.0f;
	m_fPitchVel	= 0.0f;
	m_fYawVel	= 0.0f;
	m_fRollVel	= 0.0f;

	m_cBounces = 0;

    m_hBounceSnd = LTNULL;
	m_eContainerCode = CC_NO_CONTAINER;
	m_eLastHitSurface = ST_UNKNOWN;

	m_bRotatedToRest = LTFALSE;

	ResetRotationVel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::~CGrenade
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CGrenade::~CGrenade()
{
	// Just to be safe...
	g_lstGrenades.Remove(this);

	if (m_hBounceSnd)
	{
        g_pLTServer->KillSound(m_hBounceSnd);
        m_hBounceSnd = LTNULL;
	}
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::Detonate
//
//	PURPOSE:	Go boom
//
// ----------------------------------------------------------------------- //

void CGrenade::Detonate(HOBJECT hObj)
{
	// We're blowing up, take us out of the grenade list
	g_lstGrenades.Remove(this);

	CProjectile::Detonate(hObj);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::HandleImpact
//
//	PURPOSE:	Handle bouncing off of things
//
// ----------------------------------------------------------------------- //

void CGrenade::HandleImpact(HOBJECT hObj)
{
	if (!g_vtGrenadeDampenPercent.IsInitted())
	{
        g_vtGrenadeDampenPercent.Init(g_pLTServer, "GrenadeDampenPercent", LTNULL, DEFAULT_GRENADE_DAMPEN_PERCENT);
	}

	if (!g_vtGrenadeMinVelMag.IsInitted())
	{
        g_vtGrenadeMinVelMag.Init(g_pLTServer, "GrenadeMinVelMag", LTNULL, DEFAULT_GRENADE_MIN_VELOCITY);
	}


    LTVector vVel;
    g_pLTServer->GetVelocity(m_hObject, &vVel);


	// See if we are impacting on liquid...

    LTBOOL bEnteringLiquid = LTFALSE;
    uint16 code;
    if (g_pLTServer->GetContainerCode(hObj, &code))
	{
		if (IsLiquid((ContainerCode)code))
		{
            bEnteringLiquid = LTTRUE;
		}
	}


	CollisionInfo info;
    g_pLTServer->GetLastCollision(&info);


	// Do the bounce, if the object we hit isn't liquid...

	if (!bEnteringLiquid)
	{
		vVel += (info.m_vStopVel * 2.0f);
	}


	// Dampen the grenade's new velocity based on the surface type...

    LTFLOAT fDampenPercent = g_vtGrenadeDampenPercent.GetFloat();

	m_eLastHitSurface = GetSurfaceType(info);
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eLastHitSurface);
	if (pSurf)
	{
		// Play a bounce sound (based on the surface type) if one isn't
		// already playing...

		if ( ShouldPlayBounceSound(pSurf) )
		{
			// Only play one sound at a time...

			if (m_hBounceSnd)
			{
                g_pLTServer->KillSound(m_hBounceSnd);
                m_hBounceSnd = LTNULL;
			}

            uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_TIME;

			int nVolume	= IsLiquid(m_eContainerCode) ? 50 : 100;

            LTVector vPos;
            g_pLTServer->GetObjectPos(m_hObject, &vPos);

			m_hBounceSnd = g_pServerSoundMgr->PlaySoundFromPos(vPos, (char*)GetBounceSound(pSurf),
				pSurf->fGrenadeSndRadius, SOUNDPRIORITY_MISC_MEDIUM, dwFlags, nVolume);
		}

		fDampenPercent = (1.0f - pSurf->fHardness);
	}

	fDampenPercent = fDampenPercent > 1.0f ? 1.0f : (fDampenPercent < 0.0f ? 0.0f : fDampenPercent);

	vVel *= (1.0f - fDampenPercent);


	// See if we should come to a rest...

    LTVector vTest = vVel;
	vTest.y = 0.0f;

	if (vTest.Mag() < g_vtGrenadeMinVelMag.GetFloat())
	{
		// If we're on the ground (or an object), stop movement...

		CollisionInfo standingInfo;
        g_pLTServer->GetStandingOn(m_hObject, &standingInfo);

		CollisionInfo* pInfo = standingInfo.m_hObject ? &standingInfo : &info;

		if (pInfo->m_hObject)
		{
			// Don't stop on walls...

			if (pInfo->m_Plane.m_Normal.y > 0.75f)
			{
				vVel.Init();

				// Turn off gravity, solid, and touch notify....

                uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
				dwFlags &= ~(FLAG_GRAVITY | FLAG_TOUCH_NOTIFY | FLAG_SOLID);

                g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

				// Rotate to rest...

				RotateToRest();
			}
		}
	}


	// Reset rotation velocities due to the bounce...

	ResetRotationVel(&vVel, pSurf);


	// We need to subtact this out because the engine will add it back in,
	// kind of a kludge but necessary...

	vVel -= info.m_vStopVel;


    g_pLTServer->SetVelocity(m_hObject, &vVel);

	m_cBounces++;
}

// ----------------------------------------------------------------------- //

LTBOOL CGrenade::ShouldPlayBounceSound(SURFACE* pSurface)
{
	return !!pSurface->szGrenadeImpactSnd[0];
}

// ----------------------------------------------------------------------- //

const char* CGrenade::GetBounceSound(SURFACE* pSurface)
{
	return pSurface->szGrenadeImpactSnd;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::UpdateGrenade()
//
//	PURPOSE:	Update the grenade...
//
// ----------------------------------------------------------------------- //

void CGrenade::UpdateGrenade()
{
	// Update my container code...

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);
    m_eContainerCode = GetContainerCode(g_pLTServer, vPos);


	// Update grenade spin...

	if (m_bSpinGrenade && m_bUpdating)
	{
		if (m_fPitchVel != 0 || m_fYawVel != 0 || m_fRollVel != 0)
		{
            LTFLOAT fDeltaTime = g_pLTServer->GetFrameTime();

			m_fPitch += m_fPitchVel * fDeltaTime;
			m_fYaw   += m_fYawVel * fDeltaTime;
			m_fRoll  += m_fRollVel * fDeltaTime;

            LTRotation rRot;
            g_pLTServer->SetupEuler(&rRot, m_fPitch, m_fYaw, m_fRoll);
            g_pLTServer->SetObjectRotation(m_hObject, &rRot);
		}
	}


	// See if the bounce sound is done playing...

	if (m_hBounceSnd)
	{
        LTBOOL bIsDone;
        if (g_pLTServer->IsSoundDone(m_hBounceSnd, &bIsDone) != LT_OK || bIsDone)
		{
            g_pLTServer->KillSound(m_hBounceSnd);
            m_hBounceSnd = LTNULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::RotateToRest()
//
//	PURPOSE:	Rotate the grenade to its rest position...
//
// ----------------------------------------------------------------------- //

void CGrenade::RotateToRest()
{
	// Record this grenade for AI's to be wary of

	if ( !m_bRotatedToRest )
	{
		g_lstGrenades.Add(this);
	}

	// At rest

	m_bRotatedToRest = LTTRUE;

    m_bUpdating = LTFALSE;
	
	// Spin it if necessary

	if (m_bSpinGrenade)
	{
		LTRotation rRot;
		g_pLTServer->SetupEuler(&rRot, 0.0f, m_fYaw, 0.0f);
		g_pLTServer->SetObjectRotation(m_hObject, &rRot);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::ResetRotationVel()
//
//	PURPOSE:	Update the grenade...
//
// ----------------------------------------------------------------------- //

void CGrenade::ResetRotationVel(LTVector* pvNewVel, SURFACE* pSurf)
{
	if (!m_bSpinGrenade) return;

	// (TO DO: base this on new velocity and surface hardness?)

    LTFLOAT fVal  = MATH_CIRCLE/2.0f;
    LTFLOAT fVal2 = MATH_CIRCLE;
	m_fPitchVel	 = GetRandom(-fVal, fVal);
	m_fYawVel	 = GetRandom(-fVal2, fVal2);
	m_fRollVel   = GetRandom(-fVal2, fVal2);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CGrenade::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			UpdateGrenade();
		}
		break;

		default : break;
	}

	return CProjectile::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CGrenade::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	CProjectile::Save(hWrite, dwSaveFlags);

    g_pLTServer->WriteToMessageByte(hWrite, m_bSpinGrenade);
    g_pLTServer->WriteToMessageByte(hWrite, m_bUpdating);
    g_pLTServer->WriteToMessageByte(hWrite, m_eContainerCode);
    g_pLTServer->WriteToMessageByte(hWrite, m_eLastHitSurface);

    g_pLTServer->WriteToMessageFloat(hWrite, m_fPitch);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fYaw);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fRoll);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fPitchVel);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fYawVel);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fRollVel);
	SAVE_BOOL(m_bRotatedToRest);
	SAVE_INT(m_cBounces);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CGrenade::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	CProjectile::Load(hRead, dwLoadFlags);

    m_bSpinGrenade      = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bUpdating         = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_eContainerCode    = (ContainerCode) g_pLTServer->ReadFromMessageByte(hRead);
    m_eLastHitSurface   = (SurfaceType) g_pLTServer->ReadFromMessageByte(hRead);

    m_fPitch    = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fYaw      = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fRoll     = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fPitchVel = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fYawVel   = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fRollVel  = g_pLTServer->ReadFromMessageFloat(hRead);
	LOAD_BOOL(m_bRotatedToRest);
	LOAD_INT(m_cBounces);

	if ( m_bRotatedToRest )
	{
		g_lstGrenades.Add(this);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLipstickProx::Setup()
//
//	PURPOSE:	Setup the grenade...
//
// ----------------------------------------------------------------------- //

void CLipstickProx::Setup(CWeapon *pWeapon, WFireInfo & info)
{
	CGrenade::Setup(pWeapon, info);

	if (!m_pAmmoData || !m_pAmmoData->pProjectileFX ||
		!m_pAmmoData->pProjectileFX->pClassData) return;

	m_pClassData = (PROXCLASSDATA*)m_pAmmoData->pProjectileFX->pClassData;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLipstickProx::UpdateGrenade()
//
//	PURPOSE:	Update the grenade...
//
// ----------------------------------------------------------------------- //

void CLipstickProx::UpdateGrenade()
{
	CGrenade::UpdateGrenade();

	// If we're doing normal (in-air) updates, don't do any more
	// processing...

	if (m_bUpdating || !m_pClassData) return;


	// Make sure we aren't moving anymore...
		
	LTVector vVel(0, 0, 0);
	g_pLTServer->SetVelocity(m_hObject, &vVel);


	// Waiting to go boom...

	if (m_bActivated && m_DetonateTime.Stopped())
	{
        Detonate(LTNULL);
		return;
	}


	// See if it is time to arm yet...

	if (!m_bArmed && m_ArmTime.Stopped())
	{
        m_bArmed = LTTRUE;

		// Play armed sound...

		if (m_pClassData->szArmSound[0])
		{
			int nVolume = IsLiquid(m_eContainerCode) ? 50 : 100;

            LTVector vPos;
            g_pLTServer->GetObjectPos(m_hObject, &vPos);

			g_pServerSoundMgr->PlaySoundFromPos(vPos, m_pClassData->szArmSound,
                (LTFLOAT)m_pClassData->nArmSndRadius, SOUNDPRIORITY_MISC_MEDIUM,
				0, nVolume);
		}
	}


	// Is there anything close enough to cause us to go boom?

	if (!m_bActivated && m_bArmed)
	{
		if (!m_pAmmoData) return;

        LTFLOAT fRadius = (LTFLOAT) m_pClassData->nActivateRadius;

		// NEED TO FIGURE OUT A BETTER WAY TO DO THIS!!!

        LTVector vPos;
        g_pLTServer->GetObjectPos(m_hObject, &vPos);
        ObjectList* pList = g_pLTServer->FindObjectsTouchingSphere(&vPos, fRadius);
		if (!pList) return;

		ObjectLink* pLink = pList->m_pFirstLink;
		while (pLink)
		{
			if (g_pGameServerShell->GetGameType() == COOPERATIVE_ASSAULT && IsPlayer(pLink->m_hObject) && IsPlayer(m_hFiredFrom) && g_vtNetFriendlyFire.GetFloat() < 1.0f)
			{
				CPlayerObj* pPlayer1 = (CPlayerObj*) g_pLTServer->HandleToObject(m_hFiredFrom);
				CPlayerObj* pPlayer2 = (CPlayerObj*) g_pLTServer->HandleToObject(pLink->m_hObject);
				if (pPlayer1 != pPlayer2 && pPlayer1->GetTeamID() == pPlayer2->GetTeamID())
				{
					//go to next obj
					pLink = pLink->m_pNext;
					continue;
				}
			}
			if (IsCharacter(pLink->m_hObject))
			{
                m_bActivated = LTTRUE;

                LTFLOAT fDelay = g_vtProxGrenadeDetonateDelay.GetFloat() < 0.0f ?
					m_pClassData->fActivateDelay : g_vtProxGrenadeDetonateDelay.GetFloat();

				m_DetonateTime.Start(fDelay);

				// Play activation sound...

				if (m_pClassData->szActivateSound[0])
				{
					int nVolume = IsLiquid(m_eContainerCode) ? 50 : 100;

                    LTVector vPos;
                    g_pLTServer->GetObjectPos(m_hObject, &vPos);

					g_pServerSoundMgr->PlaySoundFromPos(vPos, m_pClassData->szActivateSound,
                        (LTFLOAT) m_pClassData->nActivateSndRadius,
						SOUNDPRIORITY_MISC_MEDIUM, 0, nVolume);
				}

				break;
			}

			pLink = pLink->m_pNext;
		}
        g_pLTServer->RelinquishList(pList);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLipstickProx::HandleImpact
//
//	PURPOSE:	Handle bouncing off of things
//
// ----------------------------------------------------------------------- //

void CLipstickProx::HandleImpact(HOBJECT hObj)
{
	CGrenade::HandleImpact(hObj);

	if (!g_vtProxGrenadeArmDelay.IsInitted())
	{
        g_vtProxGrenadeArmDelay.Init(g_pLTServer, "ProxArmDelay", LTNULL, -1.0f);
	}

	if (!g_vtProxGrenadeDetonateDelay.IsInitted())
	{
        g_vtProxGrenadeDetonateDelay.Init(g_pLTServer, "ProxDetonateDelay", LTNULL, -1.0f);
	}

	// See if we should stick to the object we just hit...

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eLastHitSurface);
	if (pSurf)
	{
		// Does this surface support magnatism?  If so, stick...

		if (pSurf->bMagnetic)
		{
			// Need to set velocity to 0.0f but account for stoping vel
			// being added back in...

			CollisionInfo info;
            g_pLTServer->GetLastCollision(&info);

			LTVector vVel(0, 0, 0);
			vVel -= info.m_vStopVel;
			g_pLTServer->SetVelocity(m_hObject, &vVel);

			m_vSurfaceNormal.Init(0, 1, 0);
			m_vSurfaceNormal = info.m_Plane.m_Normal;


			// Turn off gravity, solid, and touch notify....
			// And turn on go-thru-world so it doesn't reflect from the ending position

            uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
			dwFlags &= ~(FLAG_GRAVITY | FLAG_TOUCH_NOTIFY | FLAG_SOLID);
			dwFlags |= FLAG_GOTHRUWORLD;
            g_pLTServer->SetObjectFlags(m_hObject, dwFlags);


			// Rotate to rest...

			RotateToRest();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLipstickProx::RotateToRest()
//
//	PURPOSE:	Rotate the grenade to its rest position...
//
// ----------------------------------------------------------------------- //

void CLipstickProx::RotateToRest()
{
	CGrenade::RotateToRest();
/* Update 1.002
    LTRotation rRot;
    g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	// Okay, rotated based on the surface normal we're on...

    g_pLTServer->AlignRotation(&rRot, &m_vSurfaceNormal, LTNULL);
    g_pLTServer->SetObjectRotation(m_hObject, &rRot);
*/
	// Arm the grenade after a few...

    LTFLOAT fDelay = g_vtProxGrenadeArmDelay.GetFloat() < 0.0f ?
			m_pClassData->fArmDelay : g_vtProxGrenadeArmDelay.GetFloat();

	m_ArmTime.Start(fDelay);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLipstickProx::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CLipstickProx::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	CGrenade::Save(hWrite, dwSaveFlags);

	m_DetonateTime.Save(hWrite);
	m_ArmTime.Save(hWrite);

    g_pLTServer->WriteToMessageVector(hWrite, &m_vSurfaceNormal);
    g_pLTServer->WriteToMessageByte(hWrite, m_bArmed);
    g_pLTServer->WriteToMessageByte(hWrite, m_bActivated);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLipstickProx::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CLipstickProx::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	CGrenade::Load(hRead, dwLoadFlags);

	m_DetonateTime.Load(hRead);
	m_ArmTime.Load(hRead);

    g_pLTServer->ReadFromMessageVector(hRead, &m_vSurfaceNormal);

    m_bArmed        = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bActivated    = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);

    m_pClassData = LTNULL;
	if (m_pAmmoData && m_pAmmoData->pProjectileFX)
	{
		m_pClassData = (PROXCLASSDATA*)m_pAmmoData->pProjectileFX->pClassData;
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpear::CSpear
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CSpear::CSpear() : CProjectile()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpear::~CSpear
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CSpear::~CSpear()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpear::HandleImpact
//
//	PURPOSE:	Handle bouncing off of things
//
// ----------------------------------------------------------------------- //

void CSpear::HandleImpact(HOBJECT hObj)
{
	if (!g_vtSpearStickPercentage.IsInitted())
	{
        g_vtSpearStickPercentage.Init(g_pLTServer, "SpearStickPercent", LTNULL, 0.9f);
	}

	if (!m_pAmmoData || !m_pAmmoData->pProjectileFX)
	{
		CProjectile::HandleImpact(hObj);
		return;
	}


	CollisionInfo info;
    g_pLTServer->GetLastCollision(&info);

    LTVector vPos, vVel, vCurVel, vP0, vP1;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

    LTRotation rRot;
    g_pLTServer->GetObjectRotation(m_hObject, &rRot);


	// Should we break the spear?

	enum SpearAction
	{
		eSpearActionBreak,
		eSpearActionStickWorld,
		eSpearActionStickAI,
		eSpearActionStickPlayer,
		eSpearActionStickBody
	};

	SpearAction eSpearAction = eSpearActionBreak;

	// Randomly break even if we could sometimes stick...

	if (GetRandom(0.0, 1.0f) > g_vtSpearStickPercentage.GetFloat())
	{
		eSpearAction = eSpearActionBreak;
	}
	else if (IsMainWorld(hObj))
	{
 		// Calculate where we really hit the world...

		g_pLTServer->GetVelocity(m_hObject, &vVel);

		vP1 = vPos;
        vCurVel = vVel * g_pLTServer->GetFrameTime();
		vP0 = vP1 - vCurVel;
		vP1 += vCurVel;

        LTFLOAT fDot1 = VEC_DOT(info.m_Plane.m_Normal, vP0) - info.m_Plane.m_Dist;
        LTFLOAT fDot2 = VEC_DOT(info.m_Plane.m_Normal, vP1) - info.m_Plane.m_Dist;

		if (fDot1 < 0.0f && fDot2 < 0.0f || fDot1 > 0.0f && fDot2 > 0.0f)
		{
			vPos = vP1;
		}
		else
		{
            LTFLOAT fPercent = -fDot1 / (fDot2 - fDot1);
			VEC_LERP(vPos, vP0, vP1, fPercent);
		}

		// Set our new "real" pos...

        g_pLTServer->SetObjectPos(m_hObject, &vPos);

		eSpearAction = eSpearActionStickWorld;
	}
	else if (IsMoveable(hObj))
	{
		if (IsAI(hObj))
		{
			// Attach to a AI
			eSpearAction = eSpearActionStickAI;
		}
		else if (IsPlayer(hObj))
		{
			// Attach to a Player
			eSpearAction = eSpearActionStickPlayer;
		}
		else if (IsBody(hObj))
		{
			// Attach to a body
 			eSpearAction = eSpearActionStickBody;
		}
		else
		{
			// Could probably come up with a way to attach to moveable
			// non-character objects (like doors), but it is much easier
			// to just break it ;)...

            eSpearAction = eSpearActionBreak;
		}
	}

	// If the surface is too hard, the spear will just break when
	// it hits it...

	SurfaceType eSurf = GetSurfaceType(info);
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurf);
	if ((eSpearActionBreak == eSpearAction) || ((eSpearActionStickWorld == eSpearAction) && pSurf && pSurf->fHardness > 0.5))
	{
		// Create spear debris...

		DEBRIS* pDebris = g_pDebrisMgr->GetDebris(m_pAmmoData->szName);
		if (pDebris)
		{
			vVel.Norm();
            LTVector vNegVel = -vVel;
            CreatePropDebris(vPos, vNegVel, pDebris->nId);
		}

		CProjectile::HandleImpact(hObj);
		return;
	}

	// Create the Spear powerup...

	char szSpawn[512];
	sprintf(szSpawn, "AmmoBox AmmoType1 %s;AmmoCount1 1;Filename %s;Skin %s",
		m_pAmmoData->szName, m_pAmmoData->pProjectileFX->szModel,
		m_pAmmoData->pProjectileFX->szSkin);

	LTVector vScale = m_pAmmoData->pProjectileFX->vModelScale;

	// Make sure the spear sticks out a little ways...

	vVel.Norm();
	vPos -= (vVel * vScale.z/2.0f);

	if (eSpearActionStickWorld == eSpearAction)
	{
		g_pLTServer->AlignRotation(&rRot, &vVel, LTNULL);
	}

	BaseClass* pClass = SpawnObject(szSpawn, LTVector(-10000,-10000,-10000), rRot);

	if (pClass)
	{
		g_pLTServer->ScaleObject(pClass->m_hObject, &vScale);

		LTVector vDims;
		g_pLTServer->GetObjectDims(pClass->m_hObject, &vDims);
		vDims.x *= vScale.x;
		vDims.y *= vScale.y;
		vDims.z *= vScale.z;

		g_pLTServer->SetObjectDims(pClass->m_hObject, &vDims);

		// We don't want other projectiles to impact on us...

		//uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(pClass->m_hObject);
		//dwUsrFlags |= USRFLG_IGNORE_PROJECTILES;
		//g_pLTServer->SetObjectUserFlags(pClass->m_hObject, dwUsrFlags);


		if ( eSpearActionStickAI == eSpearAction || eSpearActionStickPlayer == eSpearAction )
		{
			g_pLTServer->SetObjectUserFlags(pClass->m_hObject, g_pLTServer->GetObjectUserFlags(pClass->m_hObject) & ~USRFLG_GLOW);
			g_pLTServer->SetObjectFlags(pClass->m_hObject, g_pLTServer->GetObjectFlags(pClass->m_hObject) & ~FLAG_TOUCH_NOTIFY);

			if ( eSpearActionStickPlayer == eSpearAction )
			{
				g_pLTServer->SetObjectUserFlags(pClass->m_hObject, g_pLTServer->GetObjectUserFlags(pClass->m_hObject) | USRFLG_ATTACH_HIDE1SHOW3);
			}

			// Attach it to the character

			CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(hObj);
			pCharacter->AddSpear(pClass->m_hObject, pCharacter->GetModelNodeLastHit(), rRot);
		}
		else if ( eSpearActionStickBody == eSpearAction )
		{
			g_pLTServer->SetObjectUserFlags(pClass->m_hObject, g_pLTServer->GetObjectUserFlags(pClass->m_hObject) & ~USRFLG_GLOW);
			g_pLTServer->SetObjectFlags(pClass->m_hObject, g_pLTServer->GetObjectFlags(pClass->m_hObject) & ~FLAG_TOUCH_NOTIFY);

			// Attach it to the body

			Body* pBody = (Body*)g_pLTServer->HandleToObject(hObj);
			pBody->AddSpear(pClass->m_hObject, rRot);
		}
		else // ( eSpearActionStickWorld == eSpearAction )
		{
			// Move it to the right position in the world
			g_pLTServer->SetObjectPos(pClass->m_hObject, &vPos);
		}
	}

	CProjectile::HandleImpact(hObj);
}

// ----------------------------------------------------------------------- //

LTBOOL CCoin::ShouldPlayBounceSound(SURFACE* pSurface)
{
	return (m_cBounces == 0);
}

// ----------------------------------------------------------------------- //

const char* CCoin::GetBounceSound(SURFACE* pSurface)
{
	return "snd\\event\\dropcoin.wav";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCoin::RotateToRest()
//
//	PURPOSE:	Rotate the grenade to its rest position...
//
// ----------------------------------------------------------------------- //

void CCoin::RotateToRest()
{
	if ( !m_bRotatedToRest )
	{
		char szSpawn[1024];
		sprintf(szSpawn, "WeaponItem Gravity 0;AmmoAmount 1;WeaponType Coin;AmmoType Coin");

		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		vPos.y += 2.0f; // This offsets us from the floor a bit so we don't pop through when WeaponItem sets its dims.

		LTRotation rRot;
		rRot.Init();

		BaseClass* pObj = SpawnObject(szSpawn, vPos, rRot);

		if ( pObj && pObj->m_hObject )
		{
			g_pLTServer->SetAcceleration(pObj->m_hObject, &LTVector(0,0,0));
			g_pLTServer->SetVelocity(pObj->m_hObject, &LTVector(0,0,0));
		}

		g_pLTServer->SetObjectFlags(m_hObject, g_pLTServer->GetObjectFlags(m_hObject)&~FLAG_VISIBLE);
	}

	CGrenade::RotateToRest();

	if ( IsCharacter(m_hFiredFrom) )
	{
		LTVector vPosition;
        g_pLTServer->GetObjectPos(m_hObject, &vPosition);

        CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(m_hFiredFrom);

		CharCoinInfo cinfo;
        cinfo.fTime = g_pLTServer->GetTime();
		cinfo.eSurfaceType = m_eLastHitSurface;
		cinfo.vPosition = vPosition;

		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eLastHitSurface);
		_ASSERT(pSurf);
		if (pSurf)
		{
			cinfo.fVolume = pSurf->fMovementNoiseModifier;
		}
		else
		{
			cinfo.fVolume = 1.0f;
		}

		pCharacter->SetLastCoinInfo(&cinfo);
	}
}