
#include "stdafx.h"
#include "SprinklesFX.h"
#include "FXButeMgr.h"
#include "ClientServerShared.h"
#include "commonutilities.h"


class ModelObjControl : public FXObjControl
{
public:

	void	DebugPrint(FXType *pType, FXObj *pObj)
	{
	}

    LTVector GetObjPos(FXType *pType, FXObj *pObj)
	{
        LTVector ret;
        g_pLTClient->GetObjectPos(pObj->m_hModel, &ret);
		return ret;
	}

    void    SetObjPos(FXType *pType, FXObj *pObj, LTVector pos)
	{
        g_pLTClient->SetObjectPos(pObj->m_hModel, &pos);
	}

	void	SetObjAlpha(FXObj *pObj, float alpha)
	{
		float r, g, b, a;

        g_pLTClient->GetObjectColor(pObj->m_hModel, &r, &g, &b, &a);
        g_pLTClient->SetObjectColor(pObj->m_hModel, r, g, b, alpha);
	}

    void    SetObjColor(FXObj *pObj, LTVector &color)
	{
		float r, g, b, a;

        g_pLTClient->GetObjectColor(pObj->m_hModel, &r, &g, &b, &a);
        g_pLTClient->SetObjectColor(pObj->m_hModel,
			color.x * MATH_ONE_OVER_255,
			color.y * MATH_ONE_OVER_255,
			color.z * MATH_ONE_OVER_255,
			a);
	}

	void	ShowObj(FXObj *pObj)
	{
        g_pLTClient->SetObjectFlags(pObj->m_hModel,
            g_pLTClient->GetObjectFlags(pObj->m_hModel) | FLAG_VISIBLE);
	}

	void	HideObj(FXObj *pObj)
	{
        g_pLTClient->SetObjectFlags(pObj->m_hModel,
            g_pLTClient->GetObjectFlags(pObj->m_hModel) & ~FLAG_VISIBLE);
	}

} g_ModelObjControl;


class ParticleObjControl : public FXObjControl
{
public:

	void	DebugPrint(FXType *pType, FXObj *pObj)
	{
        LTVector vParticlePos, vSystemPos;
        g_pLTClient->GetObjectPos(pType->m_hObject, &vSystemPos);
        LTVector vPos = vSystemPos + GetObjPos(pType, pObj);

        g_pLTClient->CPrint("World Pos: %.2f, %.2f, %.2f", vPos.x, vPos.y, vPos.z);
        g_pLTClient->CPrint("Color: %.2f, %.2f, %.2f", pObj->m_pParticle->m_Color.x,
			pObj->m_pParticle->m_Color.y, pObj->m_pParticle->m_Color.z);
        g_pLTClient->CPrint("Alpha: %.2f", pObj->m_pParticle->m_Alpha);
	}

    LTVector GetObjPos(FXType *pType, FXObj *pObj)
	{
        LTVector ret;
        g_pLTClient->GetParticlePos(pType->m_hObject, pObj->m_pParticle, &ret);
		return ret;
	}

    void    SetObjPos(FXType *pType, FXObj *pObj, LTVector pos)
	{
        g_pLTClient->SetParticlePos(pType->m_hObject, pObj->m_pParticle, &pos);
	}

    void    SetObjColor(FXObj *pObj, LTVector &color)
	{
		pObj->m_pParticle->m_Color = color;
	}

	void	SetObjAlpha(FXObj *pObj, float alpha)
	{
		pObj->m_pParticle->m_Alpha = alpha;
	}

	void	ShowObj(FXObj *pObj)
	{
	}

	void	HideObj(FXObj *pObj)
	{
		pObj->m_pParticle->m_Alpha = 0.0f;
	}

} g_ParticleObjControl;




// -------------------------------------------------------------------------------- //
// FXType
// -------------------------------------------------------------------------------- //

FXType::FXType()
{
	m_hObject = NULL;
	m_pObjects = NULL;
	m_nObjects = 0;

	dl_TieOff(&m_ActiveList);
	dl_TieOff(&m_InactiveList);
}


FXType::~FXType()
{
    uint32 i;

	if(m_pObjects)
	{
		if(m_ObjType == FXObj_Model)
		{
			for(i=0; i < m_nObjects; i++)
			{
                g_pLTClient->DeleteObject(m_pObjects[i].m_hModel);
			}
		}

		debug_deletea(m_pObjects);
		m_pObjects = NULL;
	}

	if(m_hObject)
	{
        g_pLTClient->DeleteObject(m_hObject);
		m_hObject = NULL;
	}
}


// -------------------------------------------------------------------------------- //
// SprinklesFX
// -------------------------------------------------------------------------------- //

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSteamFX::Init
//
//	PURPOSE:	Init the particle system fx
//
// ----------------------------------------------------------------------- //

LTBOOL SprinklesFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	SPRINKLESCREATESTRUCT scs;

	scs.hServerObj = hServObj;
    scs.Read(g_pLTClient, hMessage);

    LTBOOL result = Init(&scs);
    for(uint32 i = 0; i < MAX_SPRINKLE_TYPES; i++)
    {
        if (scs.m_Types[i].m_hFilename)
        {
            g_pLTClient->FreeString(scs.m_Types[i].m_hFilename);
        }

        if (scs.m_Types[i].m_hSkinName)
        {
            g_pLTClient->FreeString(scs.m_Types[i].m_hSkinName);
        }
    }
    return result;
}


LTBOOL SprinklesFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	SPRINKLESCREATESTRUCT *pStruct;
    uint32 i, j, count;
	float particleSize;
	FXType *pType;
	ObjectCreateStruct ocs;
    LTVector vZero, v255;
	char tempFilename[256];
	FXObj *pFXObj;


	if(!CSpecialFX::Init(psfxCreateStruct))
        return LTFALSE;

    uint32 dwWidth, dwHeight;
    HSURFACE hScreen = g_pLTClient->GetScreenSurface();
    g_pLTClient->GetSurfaceDims(hScreen, &dwWidth, &dwHeight);
    if (dwWidth < 1) return LTFALSE;

    LTVector vServObjPos;
    g_pLTClient->GetObjectPos(m_hServerObject, &vServObjPos);

	pStruct = (SPRINKLESCREATESTRUCT*)psfxCreateStruct;

	if(pStruct->m_nTypes > MAX_SPRINKLE_TYPES)
	{
        g_pLTClient->CPrint("SprinklesFX::Init, too many FX types (%d)", pStruct->m_nTypes);
        return LTFALSE;
	}

	vZero.Init();
	v255.Init(255.0f, 255.0f, 255.0f);

	m_nFXTypes = pStruct->m_nTypes;
	for(i=0; i < m_nFXTypes; i++)
	{
		pType = &m_FXTypes[i];
		SPRINKLETYPECREATESTRUCT *pConstruct = &(pStruct->m_Types[i]);

		ocs.Clear();
		ocs.m_Flags = FLAG_VISIBLE;

        char *pTemp = g_pLTClient->GetStringData(pConstruct->m_hFilename);
		strncpy(tempFilename,pTemp,ARRAY_LEN(tempFilename));

        pTemp = g_pLTClient->GetStringData(pConstruct->m_hSkinName);
		strncpy(ocs.m_SkinName,pTemp,ARRAY_LEN(ocs.m_SkinName));

		strupr(tempFilename);
		if(strstr(tempFilename, ".ABC"))
		{
			pType->m_ObjType = FXObj_Model;
			pType->m_pControl = &g_ModelObjControl;
			ocs.m_ObjectType = OT_MODEL;
			ocs.m_Flags |= FLAG_NOLIGHT;
			SAFE_STRCPY(ocs.m_Filename, tempFilename);
		}
		else
		{
			pType->m_ObjType = FXObj_Particle;
			pType->m_pControl = &g_ParticleObjControl;
			ocs.m_ObjectType = OT_PARTICLESYSTEM;
			ocs.m_Flags |= FLAG_FOGDISABLE; // | FLAG_UPDATEUNSEEN;
			ocs.m_Pos = vServObjPos;
		}

		count = pConstruct->m_Count;
		pType->m_Speed = pConstruct->m_Speed;
		particleSize = pConstruct->m_Size;
        particleSize /= ((LTFLOAT)dwWidth);

		pType->m_SpawnRadius = pConstruct->m_SpawnRadius;
		pType->m_AnglesVel = pConstruct->m_AnglesVel;

		pType->m_ColorMin = pConstruct->m_ColorMin;

		pType->m_ColorMax = pConstruct->m_ColorMax;

		// Create the particle system?
		if(pType->m_ObjType == FXObj_Particle)
		{
            pType->m_hObject = g_pLTClient->CreateObject(&ocs);
			if(!pType->m_hObject)
				continue;

            g_pLTClient->SetupParticleSystem(pType->m_hObject, tempFilename,
				0.0f, PS_DUMB, particleSize);
		}

		// Create the objects.
		pType->m_pObjects = debug_newa(FXObj, count);
		if(pType->m_pObjects)
		{
			pType->m_nObjects = count;

			for(j=0; j < pType->m_nObjects; j++)
			{
				pFXObj = &pType->m_pObjects[j];

				// Init stuff.
				pFXObj->m_Velocity.Init();
				pFXObj->m_Link.m_pData = pFXObj;
				dl_Insert(&pType->m_InactiveList, &pFXObj->m_Link);

				if(pType->m_ObjType == FXObj_Particle)
				{
                    pFXObj->m_pParticle = g_pLTClient->AddParticle(
						pType->m_hObject, &vZero, &vZero, &v255, 0.0f);
				}
				else
				{
                    pFXObj->m_hModel = g_pLTClient->CreateObject(&ocs);
				}

				if(pFXObj->IsValid())
				{
					pType->m_pControl->HideObj(pFXObj);
				}
			}
		}
	}

    return LTTRUE;
}


LTBOOL SprinklesFX::Update()
{
    uint32 i;
	FXType *pType;
	FXObj *pFXObj;
    LTLink *pCur, *pNext;
    LTVector playerPos, curPos, newColor;
	HOBJECT hObj;
	float frameTime, alphaVal;
    LTRotation rot;


    hObj = g_pLTClient->GetClientObject();
	if(!hObj)
        return LTTRUE;

    g_pLTClient->GetObjectPos(hObj, &playerPos);
    frameTime = g_pGameClientShell->GetFrameTime();

	for(i=0; i < m_nFXTypes; i++)
	{
		pType = &m_FXTypes[i];

		if(!pType->m_nObjects)
			continue;

		// Spawn in objects that were outside.
		for(pCur=pType->m_InactiveList.m_pNext; pCur != &pType->m_InactiveList; pCur=pNext)
		{
			pNext = pCur->m_pNext;

			if(GetRandom(0, 10) < 3)
				continue;

			pFXObj = (FXObj*)pCur->m_pData;
			if(!pFXObj->IsValid())
				continue;

			dl_Remove(pCur);
			dl_Insert(&pType->m_ActiveList, pCur);

			// Give it a new position and velocity.
			curPos.x = playerPos.x + GetRandom(-pType->m_SpawnRadius, pType->m_SpawnRadius);
			curPos.y = playerPos.y + GetRandom(-pType->m_SpawnRadius, pType->m_SpawnRadius);
			curPos.z = playerPos.z + GetRandom(-pType->m_SpawnRadius, pType->m_SpawnRadius);

			// Need to subtract off the particle system's position...

			if (pType->m_ObjType == FXObj_Particle)
			{
                LTVector vSystemPos;
                g_pLTClient->GetObjectPos(pType->m_hObject, &vSystemPos);

				curPos -= vSystemPos;
			}

			pType->m_pControl->SetObjPos(pType, pFXObj, curPos);
			pType->m_pControl->ShowObj(pFXObj);

			pFXObj->m_Velocity.Init(
				GetRandom(-pType->m_Speed, pType->m_Speed),
				GetRandom(-pType->m_Speed, pType->m_Speed),
				GetRandom(-pType->m_Speed, pType->m_Speed));

			// Random starting angles.
			pFXObj->m_Angles.x = GetRandom(0.0f, MATH_CIRCLE);
			pFXObj->m_Angles.y = GetRandom(0.0f, MATH_CIRCLE);
			pFXObj->m_Angles.z = GetRandom(0.0f, MATH_CIRCLE);

			// Get velocity.
			pFXObj->m_AnglesVel.x = GetRandom(-pType->m_AnglesVel.x, pType->m_AnglesVel.x);
			pFXObj->m_AnglesVel.y = GetRandom(-pType->m_AnglesVel.y, pType->m_AnglesVel.y);
			pFXObj->m_AnglesVel.z = GetRandom(-pType->m_AnglesVel.z, pType->m_AnglesVel.z);

			// Get color.
			newColor = pType->m_ColorMin + (pType->m_ColorMax - pType->m_ColorMin) * GetRandom(0.0f, 1.0f);
			pType->m_pControl->SetObjColor(pFXObj, newColor);
		}

		// Move each object along its velocity.
		for(pCur=pType->m_ActiveList.m_pNext; pCur != &pType->m_ActiveList; pCur=pNext)
		{
			pNext = pCur->m_pNext;

			pFXObj = (FXObj*)pCur->m_pData;
			if(!pFXObj->IsValid())
				continue;

			curPos = pType->m_pControl->GetObjPos(pType, pFXObj);
			curPos += pFXObj->m_Velocity * frameTime;
			pType->m_pControl->SetObjPos(pType, pFXObj, curPos);

			// Set its angles.
			if(pType->m_ObjType == FXObj_Model)
			{
				pFXObj->m_Angles += pFXObj->m_AnglesVel * frameTime;
                g_pLTClient->SetupEuler(&rot, VEC_EXPAND(pFXObj->m_Angles));
                g_pLTClient->SetObjectRotation(pFXObj->m_hModel, &rot);
			}
			else if (pType->m_ObjType == FXObj_Particle)
			{
				// Add system pos back on to calculate alpha...
                LTVector vSystemPos;
                g_pLTClient->GetObjectPos(pType->m_hObject, &vSystemPos);

				curPos += vSystemPos;
			}

			// Set its alpha.
			alphaVal = 1.0f - ((curPos - playerPos).Mag() / pType->m_SpawnRadius);

			if(alphaVal < 0.0f)
			{
				pType->m_pControl->HideObj(pFXObj);
				dl_Remove(pCur);
				dl_Insert(&pType->m_InactiveList, pCur);
			}
			else
			{
				pType->m_pControl->SetObjAlpha(pFXObj, alphaVal);
			}
		}
	}

    return LTTRUE;
}