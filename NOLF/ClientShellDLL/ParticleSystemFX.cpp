// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleSystemFX.cpp
//
// PURPOSE : ParticleSystem special FX - Implementation
//
// CREATED : 10/24/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ParticleSystemFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "GameClientShell.h"
#include "VarTrack.h"

#define MAX_PARTICLES_PER_SECOND 5000
#define MAX_PS_VIEW_DIST_SQR	(10000*10000)	// Max global distance to add particles

extern CGameClientShell*	g_pGameClientShell;

extern LTVector g_vWorldWindVel;
static VarTrack	s_cvarTweak;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleSystemFX::CParticleSystemFX
//
//	PURPOSE:	Construct
//
// ----------------------------------------------------------------------- //

CParticleSystemFX::CParticleSystemFX() : CBaseParticleSystemFX()
{
    m_bFirstUpdate          = LTTRUE;
	m_fLastTime				= 0.0f;
	m_fNextUpdate			= 0.01f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleSystemFX::Init
//
//	PURPOSE:	Init the particle system fx
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleSystemFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CBaseParticleSystemFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	PSCREATESTRUCT ps;

	ps.hServerObj = hServObj;
    g_pLTClient->ReadFromMessageVector(hMessage, &(ps.vColor1));
    g_pLTClient->ReadFromMessageVector(hMessage, &(ps.vColor2));
    g_pLTClient->ReadFromMessageVector(hMessage, &(ps.vDims));
    g_pLTClient->ReadFromMessageVector(hMessage, &(ps.vMinVel));
    g_pLTClient->ReadFromMessageVector(hMessage, &(ps.vMaxVel));
    ps.dwFlags             = (uint32) g_pLTClient->ReadFromMessageFloat(hMessage);
    ps.fBurstWait          = g_pLTClient->ReadFromMessageFloat(hMessage);
    ps.fBurstWaitMin       = g_pLTClient->ReadFromMessageFloat(hMessage);
    ps.fBurstWaitMax       = g_pLTClient->ReadFromMessageFloat(hMessage);
    ps.fParticlesPerSecond = g_pLTClient->ReadFromMessageFloat(hMessage);
    ps.fParticleLifetime   = g_pLTClient->ReadFromMessageFloat(hMessage);
    ps.fParticleRadius     = g_pLTClient->ReadFromMessageFloat(hMessage);
    ps.fGravity            = g_pLTClient->ReadFromMessageFloat(hMessage);
    ps.fRotationVelocity   = g_pLTClient->ReadFromMessageFloat(hMessage);
    ps.fViewDist           = g_pLTClient->ReadFromMessageFloat(hMessage);
    ps.hstrTextureName     = g_pLTClient->ReadFromMessageHString(hMessage);

	return Init(&ps);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleSystemFX::Init
//
//	PURPOSE:	Init the particle system
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleSystemFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return LTFALSE;

	// Set up our creation struct...

	PSCREATESTRUCT* pPS = (PSCREATESTRUCT*)psfxCreateStruct;
	m_cs = *pPS;

	// Set our (parent's) flags...

	m_dwFlags  = m_cs.dwFlags;
	m_fRadius  = m_cs.fParticleRadius;
	m_fGravity = m_cs.fGravity;
	m_vPos	   = m_cs.vPos;

	// Set our max viewable distance...

	m_fMaxViewDistSqr = m_cs.fViewDist*m_cs.fViewDist;
	m_fMaxViewDistSqr = m_fMaxViewDistSqr > MAX_PS_VIEW_DIST_SQR ? MAX_PS_VIEW_DIST_SQR : m_fMaxViewDistSqr;


	m_vMinOffset = -m_cs.vDims;
	m_vMaxOffset = m_cs.vDims;

	// Adjust velocities based on global wind values...

	m_cs.vMinVel += g_vWorldWindVel;
	m_cs.vMaxVel += g_vWorldWindVel;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleSystemFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleSystemFX::CreateObject(ILTClient *pClientDE)
{
    if (!pClientDE ) return LTFALSE;

	if (m_cs.hstrTextureName)
	{
		m_pTextureName = pClientDE->GetStringData(m_cs.hstrTextureName);
	}

    LTBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);

	if (bRet && m_hObject && m_hServerObject)
	{
        LTRotation rRot;
		pClientDE->GetObjectRotation(m_hServerObject, &rRot);
		pClientDE->SetObjectRotation(m_hObject, &rRot);

        uint32 dwUserFlags;
		pClientDE->GetObjectUserFlags(m_hServerObject, &dwUserFlags);
		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
            uint32 dwFlags = pClientDE->GetObjectFlags(m_hObject);
			pClientDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE);
		}
	}

    s_cvarTweak.Init(g_pLTClient, "TweakParticles", NULL, 0.0f);

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleSystemFX::Update
//
//	PURPOSE:	Update the particle system
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleSystemFX::Update()
{
    if (!m_hObject || !m_pClientDE || m_bWantRemove) return LTFALSE;

    LTFLOAT fTime = m_pClientDE->GetTime();

	// Hide/show the particle system if necessary...

	if (m_hServerObject)
	{
        uint32 dwUserFlags;
		m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

        uint32 dwFlags = m_pClientDE->GetObjectFlags(m_hObject);

		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			// Once last puff as disappeared, hide the system (no new puffs
			// will be added...)

			if (dwFlags & FLAG_VISIBLE)
			{
				if (fTime > m_fLastTime + m_cs.fParticleLifetime)
				{
					m_pClientDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE);
				}
			}
			else
			{
				m_fLastTime = fTime;
			}

            return LTTRUE;
		}
		else
		{
			m_pClientDE->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);
		}
	}

	// Debugging aid...

	if (s_cvarTweak.GetFloat() > 0)
	{
		TweakSystem();
	}


	if (m_bFirstUpdate)
	{
		m_fLastTime = fTime;
        m_bFirstUpdate = LTFALSE;
	}


	// Make sure it is time to update...

	if (fTime < m_fLastTime + m_fNextUpdate)
	{
        return LTTRUE;
	}


	// Ok, how many to add this frame....(make sure time delta is no more than
	// 15 frames/sec...

	float fTimeDelta = fTime - m_fLastTime;
	fTimeDelta = fTimeDelta > 0.0666f ? 0.0666f : fTimeDelta;
	int nToAdd = (int) floor(m_cs.fParticlesPerSecond * fTimeDelta);
    nToAdd = LTMIN(nToAdd, (int)(MAX_PARTICLES_PER_SECOND * fTimeDelta));

	nToAdd = GetNumParticles(nToAdd);

	m_pClientDE->AddParticles(m_hObject, nToAdd,
		&m_vMinOffset, &m_vMaxOffset,			// Position offset
		&(m_cs.vMinVel), &(m_cs.vMaxVel),		// Velocity
		&(m_cs.vColor1), &(m_cs.vColor2),		// Color
		m_cs.fParticleLifetime, m_cs.fParticleLifetime);


	// Determine when next update should occur...

	if (m_cs.fBurstWait > 0.001f)
	{
		m_fNextUpdate = m_cs.fBurstWait * GetRandom(m_cs.fBurstWaitMin, m_cs.fBurstWaitMax);
	}
	else
	{
		m_fNextUpdate = 0.001f;
	}


	// Rotate the particle system...

	if (m_cs.fRotationVelocity != 0.0f)
	{
        LTRotation rRot;
		m_pClientDE->GetObjectRotation(m_hObject, &rRot);
		m_pClientDE->EulerRotateY(&rRot, g_pGameClientShell->GetFrameTime() * m_cs.fRotationVelocity);
		m_pClientDE->SetObjectRotation(m_hObject, &rRot);
	}

	m_fLastTime = fTime;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleSystemFX::TweakSystem
//
//	PURPOSE:	Tweak the particle system
//
// ----------------------------------------------------------------------- //

void CParticleSystemFX::TweakSystem()
{
    LTFLOAT fIncValue = 0.01f;
    LTBOOL bChanged = LTFALSE;

    LTVector vScale;
	vScale.Init();

    uint32 dwPlayerFlags = g_pGameClientShell->GetPlayerFlags();

	// Move faster if running...

	if (dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = .5f;
	}

	// Move Red up/down...

	if ((dwPlayerFlags & BC_CFLG_FORWARD) || (dwPlayerFlags & BC_CFLG_REVERSE))
	{

		//m_cs.vMinVel
		//m_cs.vMaxVel
	//m_cs.vColor1
	//m_cs.vColor2

        bChanged = LTTRUE;
	}


	// Add/Subtract number of particles per second

	if ((dwPlayerFlags & BC_CFLG_STRAFE_RIGHT) || (dwPlayerFlags & BC_CFLG_STRAFE_LEFT))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_STRAFE_RIGHT ? fIncValue : -fIncValue;
        m_cs.fParticlesPerSecond += (LTFLOAT)(fIncValue * 101.0);

		m_cs.fParticlesPerSecond = m_cs.fParticlesPerSecond < 0.0f ? 0.0f :
			(m_cs.fParticlesPerSecond > MAX_PARTICLES_PER_SECOND ? MAX_PARTICLES_PER_SECOND : m_cs.fParticlesPerSecond);

        bChanged = LTTRUE;
	}


	// Lower/Raise burst wait...

	if ((dwPlayerFlags & BC_CFLG_JUMP) || (dwPlayerFlags & BC_CFLG_DUCK))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_DUCK ? -fIncValue : fIncValue;

        bChanged = LTTRUE;
	}


	if (bChanged)
	{
		g_pGameClientShell->CSPrint("Particles per second: %.2f", m_cs.fParticlesPerSecond);
	}
}