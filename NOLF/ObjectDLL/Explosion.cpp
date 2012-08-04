// ----------------------------------------------------------------------- //
//
// MODULE  : Explosion.cpp
//
// PURPOSE : Explosion - Definition
//
// CREATED : 11/25/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Explosion.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "WeaponFXTypes.h"
#include "ObjectMsgs.h"
#include "DamageTypes.h"
#include "SharedFXStructs.h"
#include "SFXMsgIds.h"
#include "Character.h"

#define MIN_RADIUS_PERCENT				0.25f

BEGIN_CLASS(Explosion)

	ADD_STRINGPROP_FLAG(ImpactFXName, "", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(DamageType, "EXPLODE", PF_STATICLIST)
	ADD_REALPROP_FLAG(DamageRadius, 200.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(MaxDamage, 200.0f, 0)
    ADD_BOOLPROP_FLAG(RemoveWhenDone, LTTRUE, 0)

END_CLASS_DEFAULT_FLAGS_PLUGIN(Explosion, GameBase, NULL, NULL, 0, CExplosionPlugin)

LTBOOL ExplosionFilterFn(HOBJECT hObj, void *pUserData)
{
	uint32 dwFlags = g_pLTServer->GetObjectFlags(hObj);
	if (!(dwFlags & FLAG_SOLID))
	{
		return LTFALSE;
	}
	else if (IsMainWorld(hObj) || (OT_WORLDMODEL == g_pLTServer->GetObjectType(hObj)))
	{
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Explosion()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Explosion::Explosion() : GameBase()
{
	m_fDamageRadius			= 200.0f;
	m_fMaxDamage			= 200.0f;
	m_eDamageType			= DT_UNSPECIFIED;

	// For now these aren't used with DEdit created Explosions...

	m_fProgDamage			= 0.0f;
	m_fProgDamageDuration	= 0.0f;
	m_fProgDamageRadius		= 0.0f;
	m_fProgDamageLifetime	= 0.0f;
	m_eProgDamageType		= DT_UNSPECIFIED;
	m_hFiredFrom			= LTNULL;

    m_bRemoveWhenDone       = LTTRUE;

	m_vPos.Init();

	m_nImpactFXId			= FXBMGR_INVALID_ID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Setup()
//
//	PURPOSE:	Setup the Explosion
//
// ----------------------------------------------------------------------- //

void Explosion::Setup(HOBJECT hFiredFrom, uint8 nAmmoId)
{
	if (!hFiredFrom) return;

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
	if (!pAmmo) return;

    m_bRemoveWhenDone = LTTRUE;

    m_fDamageRadius = (LTFLOAT) pAmmo->nAreaDamageRadius;
    m_fMaxDamage    = (LTFLOAT) pAmmo->nAreaDamage;
	m_eDamageType	= pAmmo->eAreaDamageType;

    m_fProgDamageRadius     = (LTFLOAT) pAmmo->fProgDamageRadius;
	m_fProgDamageLifetime	= pAmmo->fProgDamageLifetime;
	m_fProgDamage			= pAmmo->fProgDamage;
	m_fProgDamageDuration	= pAmmo->fProgDamageDuration;
	m_eProgDamageType		= pAmmo->eProgDamageType;

	Start(hFiredFrom);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Start()
//
//	PURPOSE:	Start the Explosion
//
// ----------------------------------------------------------------------- //

void Explosion::Start(HOBJECT hFiredFrom)
{
	if (!m_hObject) return;

    g_pLTServer->GetObjectPos(m_hObject, &m_vPos);

	// Do special fx (on the client) if applicable...

	if (m_nImpactFXId != FXBMGR_INVALID_ID)
	{
        LTRotation rRot;
        g_pLTServer->GetObjectRotation(m_hObject, &rRot);

		EXPLOSIONCREATESTRUCT cs;
		cs.nImpactFX	 = m_nImpactFXId;
		cs.rRot			 = rRot;
		cs.vPos			 = m_vPos;
		cs.fDamageRadius = m_fDamageRadius;

        HMESSAGEWRITE hMessage = g_pLTServer->StartInstantSpecialEffectMessage(&m_vPos);
        g_pLTServer->WriteToMessageByte(hMessage, SFX_EXPLOSION_ID);
        cs.Write(g_pLTServer, hMessage);
        g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
	}

	m_hFiredFrom = hFiredFrom;

	if (m_hFiredFrom)
	{
		g_pLTServer->CreateInterObjectLink(m_hObject, m_hFiredFrom);
	}


	// Do Area damage to the objects caught in the blast...

	if (m_fDamageRadius > 0.0f && m_fMaxDamage > 0.0f)
	{
		AreaDamageObjectsInSphere();
	}

	// Progressively damage the objects caught in the blast...

	if (m_fProgDamageRadius > 0.0f)
	{
		ProgDamageObjectsInSphere();
	}

	if (m_fProgDamageLifetime > 0.0f && m_fProgDamageRadius > 0.0f)
	{
		// Process the progressive damage every frame...

		m_ProgDamageTimer.Start(m_fProgDamageLifetime);
        SetNextUpdate(0.001f);
	}
	else
	{
		if (m_bRemoveWhenDone)
		{
            g_pLTServer->RemoveObject(m_hObject);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::DoDamage()
//
//	PURPOSE:	Do the damage...
//
// ----------------------------------------------------------------------- //

void Explosion::Update()
{
	// Do progressive damage to the objects caught in the blast...

	ProgDamageObjectsInSphere();

	if (m_ProgDamageTimer.Stopped())
	{
		if (m_bRemoveWhenDone)
		{
            g_pLTServer->RemoveObject(m_hObject);
		}

		if (m_hFiredFrom)
		{
			g_pLTServer->BreakInterObjectLink(m_hObject, m_hFiredFrom);
		}
	}
	else
	{
        SetNextUpdate(0.001f);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Explosion::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE :
		{
			Update();
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				SetNextUpdate(0.0f);
			}
		}
		break;

		case MID_PRECREATE :
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp();
			}

			CacheFiles();
		}
		break;

		case MID_LINKBROKEN :
		{
			HOBJECT hLink = (HOBJECT)pData;
			if (hLink == m_hFiredFrom)
			{
                m_hFiredFrom = LTNULL;
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((HMESSAGEWRITE)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 Explosion::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
 		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);

			if ((stricmp(szMsg, "START") == 0) || (stricmp(szMsg, "ON") == 0))
			{
				Start();
			}
		}

		default : break;
	}

	return GameBase::ObjectMessageFn (hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::ReadProp
//
//	PURPOSE:	Read object properties
//
// ----------------------------------------------------------------------- //

void Explosion::ReadProp()
{
	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("DamageRadius", &genProp) == LT_OK)
	{
		m_fDamageRadius = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("MaxDamage", &genProp) == LT_OK)
	{
		m_fMaxDamage = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("RemoveWhenDone", &genProp) == LT_OK)
	{
		m_bRemoveWhenDone = genProp.m_Bool;
	}

	g_pFXButeMgr->ReadImpactFXProp("ImpactFXName", m_nImpactFXId);

    if (g_pLTServer->GetPropGeneric("DamageType", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_eDamageType = StringToDamageType((const char*)genProp.m_String);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::AreaDamageObject()
//
//	PURPOSE:	Damage the object...
//
// ----------------------------------------------------------------------- //

void Explosion::AreaDamageObject(HOBJECT hObj)
{
	if (!hObj) return;

	HOBJECT hDamager = m_hFiredFrom ? m_hFiredFrom : m_hObject;

    LTVector vObjPos;
    g_pLTServer->GetObjectPos(hObj, &vObjPos);

    LTVector vDir = vObjPos - m_vPos;
    LTFLOAT fDist = vDir.Mag();

	if (fDist <= m_fDamageRadius)
	{
		// Make sure that Characters don't take damage if another object
		// is blocking them from the explosion...
	
		LTBOOL bIntersect1 = LTFALSE;
		LTBOOL bIntersect2 = LTFALSE;

		if (IsCharacter(hObj) || IsCharacterHitBox(hObj))
		{
			// To do this test, do an intersect segment both directions
			// (from the object to the explosion and from the explosion
			// to the object).  This will ensure that neither point
			// is inside a wall and that nothing is blocking the damage...

			IntersectInfo iInfo;
			IntersectQuery qInfo;

			qInfo.m_Flags	  = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
			qInfo.m_FilterFn  = ExplosionFilterFn;

			qInfo.m_From = m_vPos + vDir/fDist;
			qInfo.m_To   = vObjPos;
			
			bIntersect1 = g_pLTServer->IntersectSegment(&qInfo, &iInfo);

			qInfo.m_From = vObjPos;
			qInfo.m_To   = m_vPos + vDir/fDist;

			bIntersect2 = g_pLTServer->IntersectSegment(&qInfo, &iInfo);
		}

		if (!bIntersect1 && !bIntersect2)
		{
			DamageStruct damage;
			damage.hDamager = hDamager;
			damage.vDir		= vDir;

            LTFLOAT fMinRadius  = m_fDamageRadius * MIN_RADIUS_PERCENT;
            LTFLOAT fRange      = m_fDamageRadius - fMinRadius;

			// Scale damage if necessary...

            LTFLOAT fMultiplier = 1.0f;

			if (fDist > fMinRadius)
			{
                LTFLOAT fPercent = (fDist - fMinRadius) / (m_fDamageRadius - fMinRadius);
				fPercent = fPercent > 1.0f ? 1.0f : (fPercent < 0.0f ? 0.0f : fPercent);

				fMultiplier = (1.0f - fPercent);
			}

			damage.eType	= m_eDamageType;
			damage.fDamage	= m_fMaxDamage;

			damage.fDamage *= fMultiplier;

			damage.DoDamage(this, hObj, m_hObject);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::ProgDamageObject()
//
//	PURPOSE:	Damage the object...
//
// ----------------------------------------------------------------------- //

void Explosion::ProgDamageObject(HOBJECT hObj)
{
	if (!hObj) return;

	HOBJECT hDamager = m_hFiredFrom ? m_hFiredFrom : m_hObject;

    LTVector vObjPos;
    g_pLTServer->GetObjectPos(hObj, &vObjPos);

    LTVector vDir = vObjPos - m_vPos;
    LTFLOAT fDist = vDir.Mag();

	if (fDist <= m_fProgDamageRadius)
	{
		// Make sure that Characters don't take damage if another object
		// is blocking them from the explosion...
	
		LTBOOL bIntersect1 = LTFALSE;
		LTBOOL bIntersect2 = LTFALSE;

		if (IsCharacter(hObj) || IsCharacterHitBox(hObj))
		{
			// To do this test, do an intersect segment both directions
			// (from the object to the explosion and from the explosion
			// to the object).  This will ensure that neither point
			// is inside a wall and that nothing is blocking the damage...

			IntersectInfo iInfo;
			IntersectQuery qInfo;

			qInfo.m_Flags	  = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
			qInfo.m_FilterFn  = ExplosionFilterFn;

			qInfo.m_From = m_vPos + vDir/fDist;
			qInfo.m_To   = vObjPos;
			
			bIntersect1 = g_pLTServer->IntersectSegment(&qInfo, &iInfo);

			qInfo.m_From = vObjPos;
			qInfo.m_To   = m_vPos + vDir/fDist;

			bIntersect2 = g_pLTServer->IntersectSegment(&qInfo, &iInfo);
		}

		if (!bIntersect1 && !bIntersect2)
		{
			DamageStruct damage;
			damage.hDamager		= hDamager;
			damage.vDir			= vDir;
			damage.eType		= m_eProgDamageType;
			damage.fDuration	= m_fProgDamageDuration;
			damage.fDamage		= m_fProgDamage;
			damage.hContainer	= m_hObject;

			damage.DoDamage(this, hObj, m_hObject);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::ProgDamageObjectsInSphere()
//
//	PURPOSE:	Progressively damage all the objects in our radius
//
// ----------------------------------------------------------------------- //

void Explosion::ProgDamageObjectsInSphere()
{
    ObjectList* pList = g_pLTServer->FindObjectsTouchingSphere(&m_vPos,
		m_fProgDamageRadius);
	if (!pList) return;

	ObjectLink* pLink = pList->m_pFirstLink;
	while (pLink)
	{
		ProgDamageObject(pLink->m_hObject);
		pLink = pLink->m_pNext;
	}

    g_pLTServer->RelinquishList(pList);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::AreaDamageObjectsInSphere()
//
//	PURPOSE:	Area damage all the objects in our radius
//
// ----------------------------------------------------------------------- //

void Explosion::AreaDamageObjectsInSphere()
{
    ObjectList* pList = g_pLTServer->FindObjectsTouchingSphere(&m_vPos, m_fDamageRadius);
	if (!pList) return;

	ObjectLink* pLink = pList->m_pFirstLink;
	while (pLink)
	{
		AreaDamageObject(pLink->m_hObject);
		pLink = pLink->m_pNext;
	}

    g_pLTServer->RelinquishList(pList);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::GetBoundingBoxColor()
//
//	PURPOSE:	Get the color of the bounding box
//
// ----------------------------------------------------------------------- //

LTVector Explosion::GetBoundingBoxColor()
{
    return LTVector(0, 0, 1);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::CacheFiles()
//
//	PURPOSE:	Cache impact fx resources
//
// ----------------------------------------------------------------------- //

void Explosion::CacheFiles()
{
	// Only cache if necessary...

	if (m_nImpactFXId == FXBMGR_INVALID_ID) return;

	IMPACTFX* pImpactFX = g_pFXButeMgr->GetImpactFX(m_nImpactFXId);
	if (pImpactFX)
	{
		pImpactFX->Cache(g_pFXButeMgr);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Explosion::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

    g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hFiredFrom);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vPos);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fDamageRadius);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fMaxDamage);
    g_pLTServer->WriteToMessageByte(hWrite, m_eDamageType);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fProgDamage);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fProgDamageRadius);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fProgDamageDuration);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fProgDamageLifetime);
    g_pLTServer->WriteToMessageByte(hWrite, m_eProgDamageType);
    g_pLTServer->WriteToMessageByte(hWrite, m_bRemoveWhenDone);
    g_pLTServer->WriteToMessageByte(hWrite, m_nImpactFXId);

	m_ProgDamageTimer.Save(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Explosion::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hFiredFrom);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vPos);

    m_fDamageRadius         = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fMaxDamage            = g_pLTServer->ReadFromMessageFloat(hRead);
    m_eDamageType           = (DamageType) g_pLTServer->ReadFromMessageByte(hRead);
    m_fProgDamage           = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fProgDamageRadius     = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fProgDamageDuration   = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fProgDamageLifetime   = g_pLTServer->ReadFromMessageFloat(hRead);
    m_eProgDamageType       = (DamageType) g_pLTServer->ReadFromMessageByte(hRead);
    m_bRemoveWhenDone       = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_nImpactFXId           = g_pLTServer->ReadFromMessageByte(hRead);

	m_ProgDamageTimer.Load(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //
LTRESULT CExplosionPlugin::PreHook_EditStringList(const char* szRezPath,
												 const char* szPropName,
												 char** aszStrings,
                                                 uint32* pcStrings,
                                                 const uint32 cMaxStrings,
                                                 const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	if (_strcmpi("ImpactFXName", szPropName) == 0)
	{
		m_FXButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		if (!m_FXButeMgrPlugin.PopulateStringList(aszStrings, pcStrings,
			 cMaxStrings, cMaxStringLength)) return LT_UNSUPPORTED;

		return LT_OK;
	}
	else if (_strcmpi("DamageType", szPropName) == 0)
	{
	   if (!aszStrings || !pcStrings) return LT_UNSUPPORTED;
		_ASSERT(aszStrings && pcStrings);

		// Add an entry for each supported damage type

		for (int i=0; i < c_nDTInfoArraySize; i++)
		{
			if (!DTInfoArray[i].bGadget)
			{
				_ASSERT(cMaxStrings > (*pcStrings) + 1);

				uint32 dwNameLen = strlen(DTInfoArray[i].pName);

				if (dwNameLen < cMaxStringLength &&
					((*pcStrings) + 1) < cMaxStrings)
				{
					strcpy(aszStrings[(*pcStrings)++], DTInfoArray[i].pName);
				}
			}
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
