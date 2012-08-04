// ----------------------------------------------------------------------- //
//
// MODULE  : LightningFX.cpp
//
// PURPOSE : Lightning special FX - Implementation
//
// CREATED : 4/15/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LightningFX.h"
#include "iltclient.h"
#include "SFXMgr.h"
#include "iltcustomdraw.h"
#include "GameClientShell.h"
#include "DynamicLightFX.h"
#include "GameButes.h"
#include "VarTrack.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightningFX::Init
//
//	PURPOSE:	Init the lightning fx
//
// ----------------------------------------------------------------------- //

LTBOOL CLightningFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	// Read in the init info from the message...

	LFXCREATESTRUCT lcs;

	lcs.hServerObj = hServObj;
	lcs.lfx.hServerObj = hServObj;
    g_pLTClient->ReadFromMessageVector(hMessage, &(lcs.lfx.vStartPos));
    g_pLTClient->ReadFromMessageVector(hMessage, &(lcs.lfx.vEndPos));
    g_pLTClient->ReadFromMessageVector(hMessage, &(lcs.vLightColor));

    g_pLTClient->ReadFromMessageVector(hMessage, &(lcs.lfx.vInnerColorStart));
    g_pLTClient->ReadFromMessageVector(hMessage, &(lcs.lfx.vInnerColorEnd));
    g_pLTClient->ReadFromMessageVector(hMessage, &(lcs.lfx.vOuterColorStart));
    g_pLTClient->ReadFromMessageVector(hMessage, &(lcs.lfx.vOuterColorEnd));

    lcs.lfx.fAlphaStart     = g_pLTClient->ReadFromMessageFloat(hMessage);
    lcs.lfx.fAlphaEnd       = g_pLTClient->ReadFromMessageFloat(hMessage);

    lcs.lfx.fMinWidth       = g_pLTClient->ReadFromMessageFloat(hMessage);
    lcs.lfx.fMaxWidth       = g_pLTClient->ReadFromMessageFloat(hMessage);
    lcs.lfx.fLifeTime       = g_pLTClient->ReadFromMessageFloat(hMessage);
    lcs.lfx.fAlphaLifeTime  = g_pLTClient->ReadFromMessageFloat(hMessage);
    lcs.fMinDelayTime		= g_pLTClient->ReadFromMessageFloat(hMessage);
    lcs.fMaxDelayTime		= g_pLTClient->ReadFromMessageFloat(hMessage);
    lcs.lfx.fPerturb        = g_pLTClient->ReadFromMessageFloat(hMessage);
    lcs.fLightRadius        = g_pLTClient->ReadFromMessageFloat(hMessage);
    lcs.fSoundRadius        = g_pLTClient->ReadFromMessageFloat(hMessage);
    lcs.lfx.nWidthStyle     = g_pLTClient->ReadFromMessageByte(hMessage);
    lcs.lfx.nNumSegments    = g_pLTClient->ReadFromMessageByte(hMessage);
    lcs.bOneTimeOnly        = (LTBOOL) g_pLTClient->ReadFromMessageByte(hMessage);
    lcs.bDynamicLight       = (LTBOOL) g_pLTClient->ReadFromMessageByte(hMessage);
    lcs.bPlaySound          = (LTBOOL) g_pLTClient->ReadFromMessageByte(hMessage);
    lcs.lfx.bAdditive       = (LTBOOL) g_pLTClient->ReadFromMessageByte(hMessage);
    lcs.lfx.bMultiply       = (LTBOOL) g_pLTClient->ReadFromMessageByte(hMessage);

	m_hstrTexture			= g_pLTClient->ReadFromMessageHString(hMessage);

	return Init(&lcs);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightningFX::Init
//
//	PURPOSE:	Init the lightning fx
//
// ----------------------------------------------------------------------- //

LTBOOL CLightningFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	LFXCREATESTRUCT* pLFX = (LFXCREATESTRUCT*)psfxCreateStruct;

	m_cs = *pLFX;

	if (m_hstrTexture)
	{
		m_cs.lfx.pTexture = g_pLTClient->GetStringData(m_hstrTexture);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightningFX::CreateObject
//
//	PURPOSE:	Create object associated the object
//
// ----------------------------------------------------------------------- //

LTBOOL CLightningFX::CreateObject(ILTClient *pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

	// Validate our init info...

    if (m_cs.lfx.nNumSegments < 1) return LTFALSE;


	// Get the thunder sound path if we need to play the sound...

	if (m_cs.bPlaySound)
	{
		m_csThunderStr = g_pClientButeMgr->GetWeatherAttributeString(WEATHER_BUTE_THUNDERSOUND);
	}


	// Set up the lightning...

	return Setup();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightningFX::Setup
//
//	PURPOSE:	Setup the line used to draw lightning
//
// ----------------------------------------------------------------------- //

LTBOOL CLightningFX::Setup()
{
	// Figure out the position based on the object's current pos...

	if (m_hServerObject)
	{
		LTVector vPos;
		g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

		m_cs.lfx.vStartPos = vPos;
	}

	// Create our poly-line for the lightning...

	if (m_Line.HasBeenDrawn())
	{
		m_Line.ReInit(&m_cs.lfx);
	}
	else
	{
		m_Line.Init(&m_cs.lfx);
		m_Line.CreateObject(g_pLTClient);
	}

	// Figure out when to start/stop...

    LTFLOAT fTime = g_pLTClient->GetTime();
	m_fStartTime = fTime + GetRandom(m_cs.fMinDelayTime, m_cs.fMaxDelayTime);
	m_fEndTime	 = m_fStartTime + m_cs.lfx.fLifeTime;

	// Calculate our mid point...

    LTVector vPos = m_cs.lfx.vStartPos;
    LTVector vDir = (m_cs.lfx.vEndPos - m_cs.lfx.vStartPos);
	float fDist  = vDir.Mag();
	float fTotalDist = fDist;

	vDir.Norm();

	m_vMidPos = (m_cs.lfx.vStartPos + (vDir * fDist/2.0f));

 	// Create the dynamic light if necessary...

	if (m_cs.bDynamicLight && !m_hLight)
	{
		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);

		createStruct.m_ObjectType = OT_LIGHT;
		createStruct.m_Flags = FLAG_DONTLIGHTBACKFACING;
		createStruct.m_Pos = m_vMidPos;

		m_hLight = m_pClientDE->CreateObject(&createStruct);
        if (!m_hLight) return LTFALSE;

		m_pClientDE->SetLightColor(m_hLight, m_cs.vLightColor.x/255.0f,
			m_cs.vLightColor.y/255.0f, m_cs.vLightColor.z/255.0f);
		m_pClientDE->SetLightRadius(m_hLight, m_cs.fLightRadius);
	}


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightningFX::HandleFirstTime
//
//	PURPOSE:	Handle the first time drawing...
//
// ----------------------------------------------------------------------- //

void CLightningFX::HandleFirstTime()
{
	if (m_hLight)
	{
        uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hLight);
        g_pLTClient->SetObjectFlags(m_hLight, dwFlags | FLAG_VISIBLE);
	}

	if (m_cs.bPlaySound)
	{
		float fWaitTime = 0.0f;
        m_fPlaySoundTime = g_pLTClient->GetTime() + fWaitTime;
        m_bPlayedSound = LTFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightningFX::UpdateSound
//
//	PURPOSE:	Handle playing the sound
//
// ----------------------------------------------------------------------- //

void CLightningFX::UpdateSound()
{
	if (m_bPlayedSound || !m_cs.bPlaySound || (m_csThunderStr.GetLength() < 1)) return;

    if (m_fPlaySoundTime <= g_pLTClient->GetTime())
	{
        m_bPlayedSound = LTTRUE;

		g_pClientSoundMgr->PlaySoundFromPos(m_vMidPos, (char *)(LPCSTR)m_csThunderStr,
			m_cs.fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightningFX::Update
//
//	PURPOSE:	Update the lightning
//
// ----------------------------------------------------------------------- //

LTBOOL CLightningFX::Update()
{
    if (m_bWantRemove) return LTFALSE;

    LTFLOAT fTime = g_pLTClient->GetTime();

	// Hide/show lightning if necessary...

	if (m_hServerObject)
	{
        uint32 dwUserFlags;
        g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			m_Line.SetFlags(m_Line.GetFlags() & ~FLAG_VISIBLE);

			if (m_hLight)
			{
				uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hLight);
				g_pLTClient->SetObjectFlags(m_hLight, dwFlags & ~FLAG_VISIBLE);
			}

            return LTTRUE;
		}
		else
		{
			m_Line.SetFlags(m_Line.GetFlags() | FLAG_VISIBLE);
		}
	}

	m_Line.Update();

	// See if it is time to act...

	if (fTime > m_fEndTime)
	{
		if (m_cs.bOneTimeOnly)
		{
            return LTFALSE;
		}
		else
		{
			Setup();
            m_bFirstTime = LTTRUE;
		}
	}

	if (fTime < m_fStartTime)
	{
		m_Line.SetFlags(m_Line.GetFlags() & ~FLAG_VISIBLE);

		if (m_hLight)
		{
            uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hLight);
            g_pLTClient->SetObjectFlags(m_hLight, dwFlags & ~FLAG_VISIBLE);
		}

        return LTTRUE;  // not yet...
	}
	else
	{
		m_Line.SetFlags(m_Line.GetFlags() | FLAG_VISIBLE);
	}


	// Do first time stuff...

	if (m_bFirstTime)
	{
        m_bFirstTime = LTFALSE;
		HandleFirstTime();
	}
	else
	{
		if (m_hLight)
		{
            uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hLight);
            g_pLTClient->SetObjectFlags(m_hLight, dwFlags & ~FLAG_VISIBLE);
		}
	}

	// Update playing the sound...

	UpdateSound();

    return LTTRUE;
}

