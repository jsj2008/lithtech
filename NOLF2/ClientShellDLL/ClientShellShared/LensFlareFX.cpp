// ----------------------------------------------------------------------- //
//
// MODULE  : LensFlareFX.cpp
//
// PURPOSE : LensFlare FX - Implementation
//
// CREATED : 5/9/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LensFlareFX.h"
#include "GameClientShell.h"
#include "VarTrack.h"
#include "CMoveMgr.h"

extern CGameClientShell* g_pGameClientShell;

VarTrack	g_cvarMinFlareAngle;
VarTrack	g_cvarMinFlareSprAlpha;
VarTrack	g_cvarMaxFlareSprAlpha;
VarTrack	g_cvarMinFlareSprScale;
VarTrack	g_cvarMaxFlareSprScale;

VarTrack	g_cvarBlindMinScale;
VarTrack	g_cvarBlindMaxScale;
VarTrack	g_cvarBlindObjectAngle;
VarTrack	g_cvarBlindCameraAngle;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLensFlareFX::Init
//
//	PURPOSE:	Init the lens flare fx
//
// ----------------------------------------------------------------------- //

LTBOOL CLensFlareFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
    if (!CSpecialFX::Init(hServObj, pMsg)) return LTFALSE;
    if (!pMsg) return LTFALSE;

	LENSFLARECREATESTRUCT lens;

	lens.InitFromMessage(lens, pMsg);
	lens.hServerObj			= hServObj;

	return Init(&lens);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LENSFLARECREATESTRUCT::InitFromMessage
//
//	PURPOSE:	Init the lens flare fx create struct based on the message
//
// ----------------------------------------------------------------------- //

LTBOOL LENSFLARECREATESTRUCT::InitFromMessage(LENSFLARECREATESTRUCT & lens,
											 ILTMessage_Read *pMsg)
{
    if (!pMsg) return LTFALSE;

    lens.bInSkyBox          = (LTBOOL) pMsg->Readuint8();
    lens.bCreateSprite      = (LTBOOL) pMsg->Readuint8();
    lens.bSpriteOnly        = (LTBOOL) pMsg->Readuint8();
    lens.bUseObjectAngle    = (LTBOOL) pMsg->Readuint8();
    lens.bSpriteAdditive    = (LTBOOL) pMsg->Readuint8();
    lens.fSpriteOffset      = pMsg->Readfloat();
    lens.fMinAngle          = pMsg->Readfloat();
    lens.fMinSpriteAlpha    = pMsg->Readfloat();
    lens.fMaxSpriteAlpha    = pMsg->Readfloat();
    lens.fMinSpriteScale    = pMsg->Readfloat();
    lens.fMaxSpriteScale    = pMsg->Readfloat();
    lens.hstrSpriteFile     = pMsg->ReadHString();
    lens.bBlindingFlare     = (LTBOOL) pMsg->Readuint8();
    lens.fBlindObjectAngle  = pMsg->Readfloat();
    lens.fBlindCameraAngle  = pMsg->Readfloat();
    lens.fMinBlindScale     = pMsg->Readfloat();
    lens.fMaxBlindScale     = pMsg->Readfloat();

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLensFlareFX::Init
//
//	PURPOSE:	Init the lens flare fx
//
// ----------------------------------------------------------------------- //

LTBOOL CLensFlareFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	m_cs = *((LENSFLARECREATESTRUCT*)psfxCreateStruct);

    g_cvarMinFlareAngle.Init(g_pLTClient, "FlareMin", NULL, -1.0f);
    g_cvarMinFlareSprAlpha.Init(g_pLTClient, "FlareSprAlphaMin", NULL, -1.0f);
    g_cvarMaxFlareSprAlpha.Init(g_pLTClient, "FlareSprAlphaMax", NULL, -1.0f);
    g_cvarMinFlareSprScale.Init(g_pLTClient, "FlareSprScaleMin", NULL, -1.0f);
    g_cvarMaxFlareSprScale.Init(g_pLTClient, "FlareSprScaleMax", NULL, -1.0f);

    g_cvarBlindMinScale.Init(g_pLTClient, "FlareBlindScaleMin", NULL, -1.0f);
    g_cvarBlindMaxScale.Init(g_pLTClient, "FlareBlindScaleMax", NULL, -1.0f);
    g_cvarBlindObjectAngle.Init(g_pLTClient, "FlareBlindObjAngle", NULL, -1.0f);
    g_cvarBlindCameraAngle.Init(g_pLTClient, "FlareBlindCamAngle", NULL, -1.0f);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLensFlareFX::CreateObject
//
//	PURPOSE:	Create the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CLensFlareFX::CreateObject(ILTClient* pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return LTFALSE;

    if (!m_cs.bCreateSprite) return LTTRUE;   // Don't want a sprite
    if (!m_cs.hstrSpriteFile) return LTFALSE; // Wanted a sprite, but no filename

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	// Create the lens flare sprite...


	g_pLTClient->GetObjectPos(m_hServerObject, &(createStruct.m_Pos));
	createStruct.m_ObjectType = OT_SPRITE;

	const char* pFilename = m_pClientDE->GetStringData(m_cs.hstrSpriteFile);
    if (!pFilename) return LTFALSE;

	SAFE_STRCPY(createStruct.m_Filename, pFilename);
	createStruct.m_Flags = FLAG_NOLIGHT | FLAG_SPRITEBIAS;

	if (m_cs.bSpriteAdditive)
	{
		createStruct.m_Flags2 = FLAG2_ADDITIVE;
	}

	m_hFlare = m_pClientDE->CreateObject(&createStruct);
    if (!m_hFlare) return LTFALSE;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLensFlareFX::Update
//
//	PURPOSE:	Update the lens flare fx
//
// ----------------------------------------------------------------------- //

LTBOOL CLensFlareFX::Update()
{
    if (!m_pClientDE || !m_hServerObject || m_bWantRemove) return LTFALSE;

	// Hide/show the flare object if necessary...

	if (m_hServerObject)
	{
        uint32 dwUserFlags;
		g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_User, dwUserFlags);

		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			if (m_hFlare)
			{
				g_pCommonLT->SetObjectFlags(m_hFlare, OFT_Flags, 0, FLAG_VISIBLE);
			}

            return LTTRUE;
		}
		else
		{
			if (m_hFlare)
			{
				g_pCommonLT->SetObjectFlags(m_hFlare, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
			}
		}
	}


	// Only show the flare if the camera is looking at it...

	HLOCALOBJ hCamera = g_pPlayerMgr->GetCamera();
    if (!hCamera) return LTFALSE;

    LTVector vPos, vCamPos;
	g_pLTClient->GetObjectPos(hCamera, &vCamPos);
	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

	// If our server object is in the sky box, convert its position to
	// real world coordinates...

	if (m_cs.bInSkyBox)
	{
		SkyDef sky;
		m_pClientDE->GetSkyDef(&sky);

        LTVector vSkyCenter = (sky.m_Min + ((sky.m_Max - sky.m_Min) / 2.0f));
		vPos = vCamPos + (vPos - vSkyCenter);
	}


    LTRotation rCamRot, rObjRot;
	g_pLTClient->GetObjectRotation(hCamera, &rCamRot);
	g_pLTClient->GetObjectRotation(m_hServerObject, &rObjRot);

    LTVector vF, vObjF;
	vF = rCamRot.Forward();
	vObjF = rObjRot.Forward();

    LTVector vDir = vPos - vCamPos;
	vDir.Normalize();

    LTFLOAT fCameraAngle = vDir.Dot(vF);
	fCameraAngle = fCameraAngle < 0.0f ? 0.0f : fCameraAngle;
	fCameraAngle *= 90.0f; // Change to degrees

    LTFLOAT fObjectAngle = -vDir.Dot(vObjF);
	fObjectAngle = fObjectAngle < 0.0f ? 0.0f : fObjectAngle;
	fObjectAngle *= 90.0f; // Change to degrees

    LTFLOAT fSpriteAngle = m_cs.bUseObjectAngle ? fObjectAngle : fCameraAngle;
    LTFLOAT fMinAngle    = g_cvarMinFlareAngle.GetFloat() < 0.0f ? m_cs.fMinAngle : g_cvarMinFlareAngle.GetFloat();

	// Show or hide the server object, if necessary...

	if (!m_cs.bSpriteOnly)
	{
		if (fCameraAngle < (90.0f - fMinAngle))
		{
			g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags, 0, FLAG_VISIBLE);
		}
		else
		{
			// Set Server object's color...

			g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);

            LTFLOAT r, g, b, a;
            LTFLOAT fVal = (fCameraAngle + fMinAngle - 90.0f)/fMinAngle;
			r = g = b = a = fVal;

			m_pClientDE->SetObjectColor(m_hServerObject, r, g, b, a);
		}
	}

	// Handle Scale sprite...

	if (m_hFlare)
	{
		if (fSpriteAngle < (90.0f - fMinAngle))
		{
			g_pCommonLT->SetObjectFlags(m_hFlare, OFT_Flags, 0, FLAG_VISIBLE);
		}
		else
		{
			if (m_cs.bBlindingFlare)
			{
				// Doesn't look quite as good, but will clip on 
				// world/objects...

				g_pCommonLT->SetObjectFlags(m_hFlare, OFT_Flags, FLAG_SPRITEBIAS | FLAG_VISIBLE, FLAG_SPRITE_NOZ | FLAG_SPRITEBIAS | FLAG_VISIBLE);
			}
			else
			{
				g_pCommonLT->SetObjectFlags(m_hFlare, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
			}

            LTFLOAT fVal = (fSpriteAngle + fMinAngle - 90.0f)/fMinAngle;

			// Calculate new alpha...

            LTFLOAT fMinAlpha = g_cvarMinFlareSprAlpha.GetFloat() < 0.0 ? m_cs.fMinSpriteAlpha : g_cvarMinFlareSprAlpha.GetFloat();
            LTFLOAT fMaxAlpha = g_cvarMaxFlareSprAlpha.GetFloat() < 0.0 ? m_cs.fMaxSpriteAlpha : g_cvarMaxFlareSprAlpha.GetFloat();
            LTFLOAT fAlphaRange = fMaxAlpha - fMinAlpha;

            LTFLOAT r, g, b, a;
			m_pClientDE->GetObjectColor(m_hFlare, &r, &g, &b, &a);

			a = fMinAlpha + (fVal * fAlphaRange);
			m_pClientDE->SetObjectColor(m_hFlare, r, g, b, a);

			// Calculate new scale...

            LTFLOAT fMinScale = g_cvarMinFlareSprScale.GetFloat() < 0.0 ? m_cs.fMinSpriteScale : g_cvarMinFlareSprScale.GetFloat();
            LTFLOAT fMaxScale = g_cvarMaxFlareSprScale.GetFloat() < 0.0 ? m_cs.fMaxSpriteScale : g_cvarMaxFlareSprScale.GetFloat();
            LTFLOAT fScaleRange = fMaxScale - fMinScale;

            LTFLOAT fScale = fMinScale + (fVal * fScaleRange);
            LTVector vScale(fScale, fScale, fScale);
			m_pClientDE->SetObjectScale(m_hFlare, &vScale);


			// Make sure the flare is in the correct position...

			vPos += (vObjF * m_cs.fSpriteOffset);
			g_pLTClient->SetObjectPos(m_hFlare, &vPos);

			// Don't do any more processing if the alpha is 0...

            if (a < 0.001f) return LTTRUE;


			// See if we should make a "bliding" flare, and if so see if the
			// camera is looking directly at the flare...

			if (m_cs.bBlindingFlare)
			{
                LTFLOAT fBlindObjAngle = g_cvarBlindObjectAngle.GetFloat() < 0.0 ? m_cs.fBlindObjectAngle : g_cvarBlindObjectAngle.GetFloat();
                LTFLOAT fBlindCamAngle = g_cvarBlindCameraAngle.GetFloat() < 0.0 ? m_cs.fBlindCameraAngle : g_cvarBlindCameraAngle.GetFloat();

				if ((fObjectAngle > (90.0f - fBlindObjAngle)) &&
					(fCameraAngle > (90.0f - fBlindCamAngle)))
				{
					// Update the no-z flare if possible...

					// Make sure there is a clear path from the flare to the camera...

					HLOCALOBJ hPlayerObj = m_pClientDE->GetClientObject();
                    HOBJECT hFilterList[] = {hPlayerObj, g_pPlayerMgr->GetMoveMgr()->GetObject(), LTNULL};

					IntersectInfo iInfo;
					IntersectQuery qInfo;
					qInfo.m_Flags		= INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
					qInfo.m_FilterFn	= ObjListFilterFn;
                    qInfo.m_pUserData   = g_pPlayerMgr->IsFirstPerson() ? hFilterList : LTNULL;
					qInfo.m_From		= vPos;
					qInfo.m_To			= vCamPos;

                    if (!g_pLTClient->IntersectSegment(&qInfo, &iInfo))
					{
						// Calculate new flare scale...

						fVal = (fObjectAngle + fBlindObjAngle - 90.0f)/fBlindObjAngle;

                        LTFLOAT fMaxBlindScale = g_cvarBlindMaxScale.GetFloat() < 0.0 ? m_cs.fMaxBlindScale : g_cvarBlindMaxScale.GetFloat();
                        LTFLOAT fMinBlindScale = g_cvarBlindMinScale.GetFloat() < 0.0 ? m_cs.fMinBlindScale : g_cvarBlindMinScale.GetFloat();

						fScaleRange =  fMaxBlindScale - fMinBlindScale;
						fScale = fMinBlindScale + (fVal * fScaleRange);
						vScale.Init(fScale, fScale, fScale);
						m_pClientDE->SetObjectScale(m_hFlare, &vScale);

						// This looks better, but will show through 
						// world/objects so we only do this if there
						// isn't anything in the way...

						g_pCommonLT->SetObjectFlags(m_hFlare, OFT_Flags, FLAG_SPRITE_NOZ, FLAG_SPRITE_NOZ | FLAG_SPRITEBIAS);
					}

				}
			}
		}
	}

    return LTTRUE;
}