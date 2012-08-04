// ----------------------------------------------------------------------- //
//
// MODULE  : PVFXMgr.cpp
//
// PURPOSE : Player-view fx manager - Implementation
//
// CREATED : 12/13/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PVFXMgr.h"
#include "CommonUtilities.h"
#include "GameClientShell.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVFXMgr::CPVFXMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPVFXMgr::CPVFXMgr()
{
    m_hModelObject = LTNULL;

	// Set up arrays...
    int i;
    for (i=0; i < PVFX_MAX_SCALE_FX; i++)
	{
		m_ScaleFX[i].pFX = &(m_prvtScaleFX[i]);
	}

	for (i=0; i < PVFX_MAX_DLIGHT_FX; i++)
	{
		m_DLightFX[i].pFX = &(m_prvtLightFX[i]);
	}

	for (i=0; i < PVFX_MAX_SOUND_FX; i++)
	{
		m_SoundFX[i].pFX = &(m_prvtSoundFX[i]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVFXMgr::~CPVFXMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CPVFXMgr::~CPVFXMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVFXMgr::Init()
//
//	PURPOSE:	Initialize data
//
// ----------------------------------------------------------------------- //

LTBOOL CPVFXMgr::Init(HOBJECT hModelObj, WEAPON* pWeapon)
{
    if (!pWeapon || !hModelObj) return LTFALSE;

	m_hModelObject = hModelObj;

	// Disable all current fx...

	DisableAllFX();

	// Set up our fx...

	HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;
	for (int i=0; i < pWeapon->nNumPVFXTypes; i++)
	{
		PVFX* pFX = g_pFXButeMgr->GetPVFX(pWeapon->aPVFXTypes[i]);
		if (pFX)
		{
			// Add the sounds...

            int j;
            for (j=0; j < pFX->nNumSoundFX; j++)
			{
				AddSoundFX(g_pFXButeMgr->GetSoundFX(pFX->aSoundFXTypes[j]),
					pFX->szName);
			}

			if (pFX->szSocket[0])
			{
				if (g_pModelLT->GetSocket(m_hModelObject, pFX->szSocket, hSocket) != LT_OK)
				{
					g_pLTClient->CPrint("ERROR: CPVFXMgr::Init() %s is an Invalid socket!", pFX->szSocket);
					continue;
				}

				// Add the scale types...

				for (j=0; j < pFX->nNumScaleFXTypes; j++)
				{
					AddScaleFX(g_pFXButeMgr->GetScaleFX(pFX->aScaleFXTypes[j]),
						hSocket, pFX->szName);
				}

				// Add the dynamic lights...

				for (j=0; j < pFX->nNumDLightFX; j++)
				{
					AddDLightFX(g_pFXButeMgr->GetDLightFX(pFX->aDLightFXTypes[j]),
						hSocket, pFX->szName);
				}
			}
		}
	}


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVFXMgr::DisableAllFX()
//
//	PURPOSE:	Disable all fx
//
// ----------------------------------------------------------------------- //

void CPVFXMgr::DisableAllFX()
{
    HOBJECT hFX = LTNULL;

    int i;
    for (i=0; i < PVFX_MAX_SCALE_FX; i++)
	{
		hFX = m_ScaleFX[i].pFX->GetObject();
		if (hFX)
		{
            uint32 dwFlags = g_pLTClient->GetObjectFlags(hFX);
            g_pLTClient->SetObjectFlags(hFX, dwFlags & ~FLAG_VISIBLE);
		}

        m_ScaleFX[i].bOn = LTFALSE;
        m_ScaleFX[i].bUsed = LTFALSE;
		m_ScaleFX[i].hSocket = INVALID_MODEL_SOCKET;
        m_ScaleFX[i].pName = LTNULL;
	}

	for (i=0; i < PVFX_MAX_DLIGHT_FX; i++)
	{
		hFX = m_DLightFX[i].pFX->GetObject();
		if (hFX)
		{
            uint32 dwFlags = g_pLTClient->GetObjectFlags(hFX);
            g_pLTClient->SetObjectFlags(hFX, dwFlags & ~FLAG_VISIBLE);
		}

        m_DLightFX[i].bOn = LTFALSE;
        m_DLightFX[i].bUsed = LTFALSE;
		m_DLightFX[i].hSocket = INVALID_MODEL_SOCKET;
        m_DLightFX[i].pName = LTNULL;
	}

	for (i=0; i < PVFX_MAX_SOUND_FX; i++)
	{
		m_SoundFX[i].Term();
        m_SoundFX[i].bOn = LTFALSE;
        m_SoundFX[i].bUsed = LTFALSE;
		m_SoundFX[i].hSocket = INVALID_MODEL_SOCKET;
        m_SoundFX[i].pName = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVFXMgr::AddScaleFX()
//
//	PURPOSE:	Add ScaleFX
//
// ----------------------------------------------------------------------- //

void CPVFXMgr::AddScaleFX(CScaleFX* pScaleFX, HMODELSOCKET hSocket, char* pName)
{
	if (!pScaleFX || !m_hModelObject || hSocket == INVALID_MODEL_SOCKET) return;

    LTVector vPos;
    LTRotation rRot;
	LTransform transform;
    if (g_pModelLT->GetSocketTransform(m_hModelObject, hSocket, transform, LTTRUE) == LT_OK)
	{
		g_pTransLT->Get(transform, vPos, rRot);
	}
	else
	{
		return;
	}


	// Find a slot for the scale fx...

	for (int i=0; i < PVFX_MAX_SCALE_FX; i++)
	{
		if (!m_ScaleFX[i].bUsed)
		{
			m_ScaleFX[i].pFX = g_pFXButeMgr->CreateScaleFX(pScaleFX, vPos,
                LTVector(0, 0, 0), LTNULL, LTNULL, (CBaseScaleFX *)m_ScaleFX[i].pFX);

            m_ScaleFX[i].bOn = LTFALSE;
            m_ScaleFX[i].bUsed = LTTRUE;
			m_ScaleFX[i].hSocket = hSocket;
			m_ScaleFX[i].pName = pName;

			// Hide fx...

			HOBJECT hFX = m_ScaleFX[i].pFX->GetObject();
			if (hFX)
			{
                uint32 dwFlags = g_pLTClient->GetObjectFlags(hFX);
                g_pLTClient->SetObjectFlags(hFX, dwFlags & ~FLAG_VISIBLE);

				// Hide object in portals...

                uint32 dwFlags2;
                g_pLTClient->Common()->GetObjectFlags(hFX, OFT_Flags2, dwFlags2);
				dwFlags2 |= FLAG2_PORTALINVISIBLE;
                g_pLTClient->Common()->SetObjectFlags(hFX, OFT_Flags2, dwFlags2);
			}

			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVFXMgr::AddDLightFX()
//
//	PURPOSE:	Add DLightFX
//
// ----------------------------------------------------------------------- //

void CPVFXMgr::AddDLightFX(DLIGHTFX* pDLightFX, HMODELSOCKET hSocket, char* pName)
{
	if (!pDLightFX || !m_hModelObject || hSocket == INVALID_MODEL_SOCKET) return;

    LTVector vPos;
    LTRotation rRot;
	LTransform transform;
    if (g_pModelLT->GetSocketTransform(m_hModelObject, hSocket, transform, LTTRUE) == LT_OK)
	{
		g_pTransLT->Get(transform, vPos, rRot);
	}
	else
	{
		return;
	}

	for (int i=0; i < PVFX_MAX_DLIGHT_FX; i++)
	{
		if (!m_DLightFX[i].bUsed)
		{
			m_DLightFX[i].pFX = g_pFXButeMgr->CreateDLightFX(pDLightFX, vPos,
				(CDynamicLightFX*)m_DLightFX[i].pFX);

            m_DLightFX[i].bOn = LTFALSE;
            m_DLightFX[i].bUsed = LTTRUE;
			m_DLightFX[i].hSocket = hSocket;
			m_DLightFX[i].pName = pName;

			// Hide fx...

			HOBJECT hFX = m_DLightFX[i].pFX->GetObject();
			if (hFX)
			{
                uint32 dwFlags = g_pLTClient->GetObjectFlags(hFX);
                g_pLTClient->SetObjectFlags(hFX, dwFlags & ~FLAG_VISIBLE);
			}

			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVFXMgr::AddSoundFX()
//
//	PURPOSE:	Add SoundFX
//
// ----------------------------------------------------------------------- //

void CPVFXMgr::AddSoundFX(SOUNDFX* pSoundFX, char* pName)
{
	if (!pSoundFX || !m_hModelObject) return;

	for (int i=0; i < PVFX_MAX_SOUND_FX; i++)
	{
		if (!m_SoundFX[i].bUsed)
		{
			m_SoundFX[i].pFX = g_pFXButeMgr->CreateSoundFX(pSoundFX,
				LTVector(0, 0, 0), (CSoundFX*)m_SoundFX[i].pFX);

            m_SoundFX[i].bOn = LTFALSE;
            m_SoundFX[i].bUsed = LTTRUE;
			m_SoundFX[i].pName = pName;

			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVFXMgr::Term()
//
//	PURPOSE:	Term
//
// ----------------------------------------------------------------------- //

void CPVFXMgr::Term()
{
	// Term all our special fx objects...NOTE:  We don't set the pFX pointers
	// to NULL because these pointers point into our m_prvtXXX static special
	// fx arrays (i.e., these pointers are always valid)...

    int i;
    for (i=0; i < PVFX_MAX_SCALE_FX; i++)
	{
		m_ScaleFX[i].Term();
	}

	for (i=0; i < PVFX_MAX_DLIGHT_FX; i++)
	{
		m_DLightFX[i].Term();
	}

	for (i=0; i < PVFX_MAX_SOUND_FX; i++)
	{
		m_SoundFX[i].Term();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVFXMgr::HandleFXKey()
//
//	PURPOSE:	Handle fx model key
//
// ----------------------------------------------------------------------- //

void CPVFXMgr::HandleFXKey(ArgList* pArgList)
{
	if (!pArgList) return;
	if (pArgList->argc < 3 || !pArgList->argv[1] || !pArgList->argv[2]) return;

	char* pFXName = pArgList->argv[1];
	char* pFXState = pArgList->argv[2];

	// Turn on/off the necessary fx...

	TurnOn(pFXName, (stricmp(pFXState, "ON") == 0));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVFXMgr::Update()
//
//	PURPOSE:	Update our fx
//
// ----------------------------------------------------------------------- //

void CPVFXMgr::Update()
{
	if (!m_hModelObject) return;

	LTransform transform;
    LTVector vPos;
    LTRotation rRot;
	HOBJECT hFX;

    g_pLTClient->GetObjectPos(m_hModelObject, &vPos);

	// Hide all the fx if not 1st person...

    LTBOOL bForceHide = LTFALSE;

	if (!g_pGameClientShell->IsFirstPerson() ||
		g_pGameClientShell->IsUsingExternalCamera() ||
		g_pGameClientShell->GetWeaponModel()->IsDisabled())
	{
        bForceHide = LTTRUE;
	}


	// Update all the used scale fx...

    int i;
    for (i=0; i < PVFX_MAX_SCALE_FX; i++)
	{
		if (m_ScaleFX[i].bUsed && m_ScaleFX[i].hSocket != INVALID_MODEL_SOCKET)
		{
			if (g_pModelLT->GetSocketTransform(m_hModelObject,
                m_ScaleFX[i].hSocket, transform, LTTRUE) == LT_OK)
			{
				g_pTransLT->Get(transform, vPos, rRot);
			}

			hFX = m_ScaleFX[i].pFX->GetObject();
			if (!hFX) continue;

            g_pLTClient->SetObjectPos(hFX, &vPos, LTTRUE);
            g_pLTClient->SetObjectRotation(hFX, &rRot);

			m_ScaleFX[i].pFX->Update();

			// Show/Hide object if on/off...

            uint32 dwFlags = g_pLTClient->GetObjectFlags(hFX);
			if (m_ScaleFX[i].bOn && !bForceHide)
			{
				dwFlags |= FLAG_VISIBLE;
			}
			else
			{
				dwFlags &= ~FLAG_VISIBLE;
			}
            g_pLTClient->SetObjectFlags(hFX, dwFlags);
		}
	}

	// Update all the used light fx...

	for (i=0; i < PVFX_MAX_DLIGHT_FX; i++)
	{
		if (m_DLightFX[i].bUsed && m_DLightFX[i].hSocket != INVALID_MODEL_SOCKET)
		{
			if (g_pModelLT->GetSocketTransform(m_hModelObject,
                m_DLightFX[i].hSocket, transform, LTTRUE) == LT_OK)
			{
				g_pTransLT->Get(transform, vPos, rRot);
			}

			hFX = m_DLightFX[i].pFX->GetObject();
			if (!hFX) continue;

			// Add on the camera's position...

			LTVector vOffset;
			g_pLTClient->GetObjectPos(g_pGameClientShell->GetCamera(), &vOffset);

			vPos += vOffset;
            g_pLTClient->SetObjectPos(hFX, &vPos, LTTRUE);


			m_DLightFX[i].pFX->Update();

			// Show/Hide object if on/off...

            uint32 dwFlags = g_pLTClient->GetObjectFlags(hFX);
			if (m_DLightFX[i].bOn && !bForceHide)
			{
				dwFlags |= FLAG_VISIBLE;
			}
			else
			{
				dwFlags &= ~FLAG_VISIBLE;
			}
            g_pLTClient->SetObjectFlags(hFX, dwFlags);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVFXMgr::TurnOn()
//
//	PURPOSE:	Turn on/off the specified fx
//
// ----------------------------------------------------------------------- //

void CPVFXMgr::TurnOn(char* pFXName, LTBOOL bOn)
{
	if (!pFXName || !pFXName[0]) return;

	// Hide/Show all scale fx...
    int i;
    for (i=0; i < PVFX_MAX_SCALE_FX; i++)
	{
		if (m_ScaleFX[i].pName && m_ScaleFX[i].bUsed)
		{
			if (stricmp(m_ScaleFX[i].pName, pFXName) == 0)
			{
				m_ScaleFX[i].bOn = bOn;
			}
		}
	}


	// Hide/show all light fx...

	for (i=0; i < PVFX_MAX_DLIGHT_FX; i++)
	{
		if (m_DLightFX[i].pName && m_DLightFX[i].bUsed)
		{
			if (stricmp(m_DLightFX[i].pName, pFXName) == 0)
			{
				m_DLightFX[i].bOn = bOn;
			}
		}
	}


	// Play all the sound fx...

	for (i=0; i < PVFX_MAX_SOUND_FX; i++)
	{
		if (m_SoundFX[i].pName && m_SoundFX[i].bUsed && m_SoundFX[i].pFX)
		{
			if (stricmp(m_SoundFX[i].pName, pFXName) == 0)
			{
				if (bOn)
				{
					((CSoundFX*)m_SoundFX[i].pFX)->Play();
				}
				else
				{
					((CSoundFX*)m_SoundFX[i].pFX)->Stop();
				}
			}
		}
	}
}