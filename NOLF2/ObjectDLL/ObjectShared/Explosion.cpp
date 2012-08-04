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
#include "ParsedMsg.h"
#include "DamageTypes.h"
#include "SharedFXStructs.h"
#include "SFXMsgIds.h"
#include "Character.h"

#define MIN_RADIUS_PERCENT				0.25f

LINKFROM_MODULE( Explosion );

#pragma force_active on
BEGIN_CLASS(Explosion)

	ADD_STRINGPROP_FLAG(ImpactFXName, "", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(DamageType, "EXPLODE", PF_STATICLIST)
	ADD_REALPROP_FLAG(DamageRadius, 200.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(MaxDamage, 200.0f, 0)
    ADD_BOOLPROP_FLAG(RemoveWhenDone, LTTRUE, 0)

END_CLASS_DEFAULT_FLAGS_PLUGIN(Explosion, GameBase, NULL, NULL, 0, CExplosionPlugin)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( Explosion )

	CMDMGR_ADD_MSG( START,	1,	NULL,	"START" )
	CMDMGR_ADD_MSG( ON,		1,	NULL,	"ON" )

CMDMGR_END_REGISTER_CLASS( Explosion, GameBase )


bool ExplosionFilterFn(HOBJECT hObj, void *pUserData)
{
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_Flags, dwFlags);
	if (!(dwFlags & FLAG_SOLID))
	{
		return false;
	}
	else if (IsMainWorld(hObj) || (OT_WORLDMODEL == GetObjectType(hObj)))
	{
		return true;
	}

	return false;
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

	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
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

		CAutoMessage cMsg;
		cMsg.Writeuint8(SFX_EXPLOSION_ID);
        cs.Write(cMsg);
		g_pLTServer->SendSFXMessage(cMsg.Read(), m_vPos, 0);
	}

	m_hFiredFrom = hFiredFrom;

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
        SetNextUpdate(UPDATE_NEXT_FRAME);
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

		m_hFiredFrom = 0;
	}
	else
	{
        SetNextUpdate(UPDATE_NEXT_FRAME);
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
				SetNextUpdate(UPDATE_NEVER);
			}
		}
		break;

		case MID_PRECREATE :
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp();
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool Explosion::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Start("START");
	static CParsedMsg::CToken s_cTok_On("ON");

	if ((cMsg.GetArg(0) == s_cTok_Start) || (cMsg.GetArg(0) == s_cTok_On))
	{
		Start();
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
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
//	ROUTINE:	Explosion::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Explosion::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

    SAVE_HOBJECT(m_hFiredFrom);
	SAVE_VECTOR(m_vPos);
    SAVE_FLOAT(m_fDamageRadius);
    SAVE_FLOAT(m_fMaxDamage);
    SAVE_BYTE(m_eDamageType);
    SAVE_FLOAT(m_fProgDamage);
    SAVE_FLOAT(m_fProgDamageRadius);
    SAVE_FLOAT(m_fProgDamageDuration);
    SAVE_FLOAT(m_fProgDamageLifetime);
    SAVE_BYTE(m_eProgDamageType);
    SAVE_BOOL(m_bRemoveWhenDone);
    SAVE_BYTE(m_nImpactFXId);

	m_ProgDamageTimer.Save(pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Explosion::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_HOBJECT(m_hFiredFrom);

	LOAD_VECTOR(m_vPos);
    LOAD_FLOAT(m_fDamageRadius);
    LOAD_FLOAT(m_fMaxDamage);
    LOAD_BYTE_CAST(m_eDamageType, DamageType);
    LOAD_FLOAT(m_fProgDamage);
    LOAD_FLOAT(m_fProgDamageRadius);
    LOAD_FLOAT(m_fProgDamageDuration);
    LOAD_FLOAT(m_fProgDamageLifetime);
    LOAD_BYTE_CAST(m_eProgDamageType, DamageType);
    LOAD_BOOL(m_bRemoveWhenDone);
    LOAD_BYTE(m_nImpactFXId);

	m_ProgDamageTimer.Load(pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //
#ifndef __PSX2
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

		for (int i=0; i < kNumDamageTypes; i++)
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
#endif