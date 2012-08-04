// ----------------------------------------------------------------------- //
//
// MODULE  : CScanner.cpp
//
// PURPOSE : Implementation of CScanner class
//
// CREATED : 6/7/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Scanner.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "gameservershell.h"
#include "SoundMgr.h"
#include "PlayerObj.h"
#include "CharacterMgr.h"
#include "SurfaceFunctions.h"
#include "VolumeBrushTypes.h"
#include "ObjectMsgs.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CScanner
//
//	PURPOSE:	Scans for players
//
// ----------------------------------------------------------------------- //
BEGIN_CLASS(CScanner)

	ADD_STRINGPROP_FLAG(Filename, "", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedFilename, "", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedSkin, "", PF_FILENAME)
	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, PF_HIDDEN)
	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_SOLID_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(MoveToFloor, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Translucency, 1.0f, PF_HIDDEN)

	ADD_REALPROP_FLAG(HitPoints, 1.0f, PF_GROUP1)
	ADD_REALPROP_FLAG(MaxHitPoints, 10.0f, PF_GROUP1)
	ADD_REALPROP_FLAG(Armor, 0.0f, PF_GROUP1)
	ADD_REALPROP_FLAG(MaxArmor, 0.0f, PF_GROUP1)

	ADD_REALPROP(FOV, 45.0)
	ADD_REALPROP_FLAG(VisualRange, 1000.0, PF_RADIUS)

	ADD_STRINGPROP_FLAG(SpotTarget, "", PF_OBJECTLINK)
	ADD_STRINGPROP(SpotMessage, "")
    ADD_LONGINTPROP(PlayerTeamFilter, 0)

END_CLASS_DEFAULT_FLAGS(CScanner, Prop, NULL, NULL, CF_HIDDEN)

// Filter functions

LTBOOL CScanner::DefaultFilterFn(HOBJECT hObj, void *pUserData)
{
    if (!hObj || !g_pLTServer) return LTFALSE;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hObj));
	if ( pSurf && pSurf->bCanSeeThrough )
	{
        return LTFALSE;
	}

    HCLASS hBody = g_pLTServer->GetClass("Body");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hBody))
	{
        return LTFALSE;
	}

	return LiquidFilterFn(hObj, pUserData);
}

LTBOOL CScanner::BodyFilterFn(HOBJECT hObj, void *pUserData)
{
    if (!hObj || !g_pLTServer) return LTFALSE;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hObj));
	if ( pSurf && pSurf->bCanSeeThrough )
	{
        return LTFALSE;
	}

	return LiquidFilterFn(hObj, pUserData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::CScanner()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CScanner::CScanner() : Prop()
{
	m_fFOV				= 0.0f;
	m_fVisualRange		= 1000.0f;
    m_nPlayerTeamFilter = 0;

	m_vInitialPitchYawRoll.Init(0, 0, 0);

    m_hstrDestroyedFilename = LTNULL;
    m_hstrDestroyedSkin = LTNULL;

    m_hstrSpotMessage   = LTNULL;
    m_hstrSpotTarget    = LTNULL;

    m_hLastDetectedEnemy = LTNULL;
	m_vLastDetectedDeathPos.Init(0, 0, 0);

	// We do not want our object removed when we die

    m_damage.m_bRemoveOnDeath = LTFALSE;

    m_bCanProcessDetection = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::~CScanner()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

CScanner::~CScanner()
{
	FREE_HSTRING(m_hstrDestroyedFilename);
	FREE_HSTRING(m_hstrDestroyedSkin);
	FREE_HSTRING(m_hstrSpotMessage);
	FREE_HSTRING(m_hstrSpotTarget);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CScanner::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch (messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
				ReadProp(pStruct);

				// Don't stand on the player...

				pStruct->m_Flags |= FLAG_DONTFOLLOWSTANDING;
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
			}

			CacheFiles();
		}
		break;

		case MID_LINKBROKEN :
		{
			HOBJECT hLink = (HOBJECT)pData;
			if (hLink)
			{
				if (hLink == m_hLastDetectedEnemy)
				{
                    m_hLastDetectedEnemy = LTNULL;
				}
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData);
		}
		break;

		default : break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL CScanner::ReadProp(ObjectCreateStruct *pInfo)
{
    if (!pInfo) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("FOV", &genProp) == LT_OK)
	{
		m_fFOV = genProp.m_Float;

		// Change FOV into the value we're going to compare in the dot product
		// for fov

		m_fFOV = (float)sin(DEG2RAD(90.0f-m_fFOV/2.0f));
	}

    if (g_pLTServer->GetPropGeneric("VisualRange", &genProp) == LT_OK)
	{
		m_fVisualRange = genProp.m_Float;

		// All values compared against VisualRange will be squared, so square
		// it too

		m_fVisualRange = m_fVisualRange*m_fVisualRange;
	}

    if (g_pLTServer->GetPropGeneric("DestroyedFilename", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrDestroyedFilename = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("DestroyedSkin", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrDestroyedSkin = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("SpotMessage", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrSpotMessage = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("SpotTarget", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrSpotTarget = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("PlayerTeamFilter", &genProp) == LT_OK)
	{
		m_nPlayerTeamFilter = (uint8) genProp.m_Long;
	}


    LTVector vAngles;
    if (g_pLTServer->GetPropRotationEuler("Rotation", &vAngles) == LT_OK)
	{
		m_vInitialPitchYawRoll = vAngles;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::UpdateDetect()
//
//	PURPOSE:	Checks to see if we can see anything
//
// ----------------------------------------------------------------------- //

CScanner::DetectState CScanner::UpdateDetect()
{
	CCharacter* pChar   = g_pCharacterMgr->LookForEnemy(this);
	CDeathScene* pScene = g_pCharacterMgr->LookForDeathScene(this);

	if (pChar || pScene)
	{


		// Set the focus timer if it hasn't been set...(i.e., the
		// first time we see a "situation")...

        LTBOOL bFocus = !!(GetFocusTime() > 0.0f);

		if (bFocus && !m_FocusTimer.GetDuration())
		{
			m_FocusTimer.Start(GetFocusTime());
		}
		else if (!bFocus || m_FocusTimer.Stopped())
		{
			// Only process detection once (unless it is reset
			// someplace else...)...

			if (m_bCanProcessDetection)
			{
				if (pChar)
				{
					SetLastDetectedEnemy(pChar->m_hObject);
				}
				else if (pScene)
				{
					SetLastDetectedDeathPos(pScene->GetPosition());
				}

				SendTriggerMsgToObjects(this, m_hstrSpotTarget, m_hstrSpotMessage);

                m_bCanProcessDetection = LTFALSE;
			}

			return DS_DETECTED;
		}

		return DS_FOCUSING;
	}
	else
	{
		// If the focus timer has stopped (i.e., we tried to focus on
		// a character but he moved before we detected him, stop the
		// timer (which will reset the duration)...

		if (m_FocusTimer.Stopped())
		{
			m_FocusTimer.Stop();
		}
		else
		{
			return DS_FOCUSING;
		}
	}

	return DS_CLEAR;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::SetDestroyedModel()
//
//	PURPOSE:	Set our model to the destroyed version
//
// ----------------------------------------------------------------------- //

void CScanner::SetDestroyedModel()
{
	if (!m_hstrDestroyedFilename || !m_hstrDestroyedSkin) return;

    char* szDestroyedFilename = g_pLTServer->GetStringData(m_hstrDestroyedFilename);
    char* szDestroyedSkin     = g_pLTServer->GetStringData(m_hstrDestroyedSkin);

    LTRESULT hResult = g_pLTServer->SetObjectFilenames(m_hObject, szDestroyedFilename, szDestroyedSkin);
    _ASSERT(hResult == LT_OK);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::SetLastDetectedEnemy()
//
//	PURPOSE:	Set our last detected enemy
//
// ----------------------------------------------------------------------- //

void CScanner::SetLastDetectedEnemy(HOBJECT hObj)
{
	if (m_hLastDetectedEnemy)
	{
        g_pLTServer->BreakInterObjectLink(m_hObject, m_hLastDetectedEnemy);
	}

	m_hLastDetectedEnemy = hObj;

	if (m_hLastDetectedEnemy)
	{
        g_pLTServer->CreateInterObjectLink(m_hObject, m_hLastDetectedEnemy);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::CanSeeObject()
//
//	PURPOSE:	Is this object visible to us?
//
// ----------------------------------------------------------------------- //

LTBOOL CScanner::CanSeeObject(ObjectFilterFn ofn, HOBJECT hObject)
{
	_ASSERT(hObject);
    if (!hObject) return LTFALSE;

	if (g_pGameServerShell->GetGameType() == COOPERATIVE_ASSAULT && m_nPlayerTeamFilter && IsPlayer(hObject))
	{
		CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hObject);
		if (pPlayer->GetTeamID() != m_nPlayerTeamFilter)
			return LTFALSE;
	}


    LTVector vPos;
    g_pLTServer->GetObjectPos(hObject, &vPos);

    LTVector vDir;
	vDir = vPos - GetScanPosition();

	if (VEC_MAGSQR(vDir) >= m_fVisualRange)
	{
        return LTFALSE;
	}

	vDir.Norm();

    LTRotation rRot = GetScanRotation();

    LTVector vUp, vRight, vForward;
    g_pLTServer->GetRotationVectors(&rRot, &vUp, &vRight, &vForward);

    LTFLOAT fDp = vDir.Dot(vForward);

	if (fDp < m_fFOV)
	{
        return LTFALSE;
	}

	// See if we can see the position in question

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, GetScanPosition());
	VEC_COPY(IQuery.m_To, vPos);
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = ofn;

    if (g_pLTServer->IntersectSegment(&IQuery, &IInfo))
	{
		if (IInfo.m_hObject == hObject)
		{
            return LTTRUE;
		}
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::CanSeePos()
//
//	PURPOSE:	Is this position visible to us?
//
// ----------------------------------------------------------------------- //

LTBOOL CScanner::CanSeePos(ObjectFilterFn ofn, const LTVector& vPos)
{
    LTVector vDir;
	vDir = vPos - GetScanPosition();

	if (VEC_MAGSQR(vDir) >= m_fVisualRange)
	{
        return LTFALSE;
	}

	vDir.Norm();

    LTRotation rRot = GetScanRotation();

    LTVector vUp, vRight, vForward;
    g_pLTServer->GetRotationVectors(&rRot, &vUp, &vRight, &vForward);

    LTFLOAT fDp = vDir.Dot(vForward);

	if (fDp < m_fFOV)
	{
        return LTFALSE;
	}

	// See if we can see the position in question

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, GetScanPosition());
	VEC_COPY(IQuery.m_To, vPos);
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = ofn;

    if (!g_pLTServer->IntersectSegment(&IQuery, &IInfo))
	{
        return LTTRUE;
	}

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CScanner::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageByte(hWrite, m_bCanProcessDetection);

    g_pLTServer->WriteToMessageFloat(hWrite, m_fFOV);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fVisualRange);

    g_pLTServer->WriteToMessageVector(hWrite, &m_vInitialPitchYawRoll);

    g_pLTServer->WriteToMessageHString(hWrite, m_hstrDestroyedFilename);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrDestroyedSkin);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrSpotMessage);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrSpotTarget);

	m_FocusTimer.Save(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CScanner::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

    m_bCanProcessDetection = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);

    m_fFOV              = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fVisualRange      = g_pLTServer->ReadFromMessageFloat(hRead);

    g_pLTServer->ReadFromMessageVector(hRead, &m_vInitialPitchYawRoll);

    m_hstrDestroyedFilename = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrDestroyedSkin     = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrSpotMessage       = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrSpotTarget        = g_pLTServer->ReadFromMessageHString(hRead);

	m_FocusTimer.Load(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScanner::CacheFiles
//
//	PURPOSE:	Cache whatever resources this object uses
//
// ----------------------------------------------------------------------- //

void CScanner::CacheFiles()
{
	if (m_hstrDestroyedFilename)
	{
        char* pFile = g_pLTServer->GetStringData(m_hstrDestroyedFilename);
		if (pFile && pFile[0])
		{
            g_pLTServer->CacheFile(FT_MODEL, pFile);
		}
	}

	if (m_hstrDestroyedSkin)
	{
        char* pFile = g_pLTServer->GetStringData(m_hstrDestroyedSkin);
		if (pFile && pFile[0])
		{
            g_pLTServer->CacheFile(FT_TEXTURE, pFile);
		}
	}
}