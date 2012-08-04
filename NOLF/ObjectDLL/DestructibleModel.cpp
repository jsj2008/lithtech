// ----------------------------------------------------------------------- //
//
// MODULE  : DestructibleModel.cpp
//
// PURPOSE : DestructibleModel aggregate
//
// CREATED : 4/23/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DestructibleModel.h"
#include "ClientServerShared.h"
#include "ServerUtilities.h"
#include "WorldModelDebris.h"
#include "DebrisFuncs.h"
#include "Weapons.h"
#include "Spawner.h"
#include "ObjectMsgs.h"
#include "Globals.h"
#include "DebrisMgr.h"
#include <stdio.h>

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModelPlugin::PreHook_EditStringList()
//
//	PURPOSE:	Fill in property string list
//
// ----------------------------------------------------------------------- //
LTRESULT CDestructibleModelPlugin::PreHook_EditStringList(const char* szRezPath,
	const char* szPropName, char** aszStrings, uint32* pcStrings,
	const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	// DestructibleModel property lists here...

	if (m_DebrisPlugin.PreHook_EditStringList(szRezPath, szPropName,
		aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
	{
		return LT_OK;
	}
	else if (_strcmpi("SurfaceOverride", szPropName) == 0)
	{
		m_SurfaceMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		if (m_SurfaceMgrPlugin.PopulateStringList(aszStrings, pcStrings,
			 cMaxStrings, cMaxStringLength))
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::CDestructibleModel()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CDestructibleModel::CDestructibleModel() : CDestructible()
{
	m_bCreatedDebris	= LTFALSE;
	m_nDebrisId			= DEBRISMGR_INVALID_ID;

	// Explosion stuff..

	m_bCreateExplosion		= LTFALSE;
	m_nExplosionWeaponId	= 0;
	m_bFireAlongForward		= LTFALSE;
	m_fDamageFactor			= 1.0f;

	m_hstrSpawn				= LTNULL;

	m_bRemoveOnDeath		= LTTRUE;

	m_hstrSurfaceOverride	= LTNULL;

	m_dwOriginalFlags		= 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::~CDestructibleModel()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CDestructibleModel::~CDestructibleModel()
{
	FREE_HSTRING(m_hstrSpawn);
	FREE_HSTRING(m_hstrSurfaceOverride);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

uint32 CDestructibleModel::EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT fData)
{
	switch (messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			uint32 dwRet = CDestructible::EngineMessageFn(pObject, messageID, pData, fData);
			int nInfo = (int)fData;
			if (nInfo != INITIALUPDATE_SAVEGAME)
			{
				SetSurfaceType();
			}

			CacheFiles();
			return dwRet;
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

	return CDestructible::EngineMessageFn(pObject, messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::SetSurfaceType
//
//	PURPOSE:	Set the object's surface type...
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::SetSurfaceType()
{
	uint32 dwUsrFlgs = g_pLTServer->GetObjectUserFlags(m_hObject);

	uint32 dwSurfUsrFlgs = 0;
	SurfaceType eSurfType = ST_UNKNOWN;

	// See if this object is a world model...

	if (g_pLTServer->GetObjectType(m_hObject) == OT_WORLDMODEL)
	{
        LTBOOL bDoBruteForce = LTTRUE;

		// See if we have a surface override...

		if (m_hstrSurfaceOverride)
		{
			char* pSurfName = g_pLTServer->GetStringData(m_hstrSurfaceOverride);
			if (pSurfName)
			{
				SURFACE* pSurf = g_pSurfaceMgr->GetSurface(pSurfName);
				if (pSurf && pSurf->eType != ST_UNKNOWN)
				{
					eSurfType = pSurf->eType;
					bDoBruteForce = LTFALSE;
				}
			}
		}


		// Determine our surface...the hard way...

		if (bDoBruteForce)
		{
			IntersectQuery qInfo;
			IntersectInfo iInfo;

			LTVector vPos, vDims, vF, vU, vR;
			LTRotation rRot;

			g_pLTServer->GetObjectPos(m_hObject, &vPos);
			g_pLTServer->GetObjectRotation(m_hObject, &rRot);
			g_pLTServer->GetRotationVectors(&rRot, &vU, &vR, &vF);
			g_pLTServer->GetObjectDims(m_hObject, &vDims);

			LTFLOAT fMaxDims = vDims.x;
			fMaxDims = Max(fMaxDims, vDims.y);
			fMaxDims = Max(fMaxDims, vDims.z);

			qInfo.m_From = vPos + (vF * (fMaxDims + 1));
			qInfo.m_To   = vPos;

			qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

			SurfaceType eType = ST_UNKNOWN;

			if (g_pLTServer->IntersectSegment(&qInfo, &iInfo))
			{
				if (iInfo.m_hObject == m_hObject)
				{
					eSurfType = GetSurfaceType(iInfo);
				}
			}
		}
	}
	else
	{
		DEBRIS* pDebris = g_pDebrisMgr->GetDebris(m_nDebrisId);
		if (pDebris)
		{
			eSurfType = pDebris->eSurfaceType;
		}
	}

	dwSurfUsrFlgs = SurfaceToUserFlag(eSurfType);
	g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlgs | dwSurfUsrFlgs);

    m_dwOriginalFlags = g_pLTServer->GetObjectFlags(m_hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CDestructibleModel::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	uint32 dwRet = CDestructible::ObjectMessageFn(pObject, hSender, messageID, hRead);

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			if (IsDead() && !m_bCreatedDebris)
			{
                ILTServer* pServerDE = BaseClass::GetServerDE();
				if (!pServerDE) break;

				SpawnItem();
				CreateDebris();
				CreateWorldModelDebris();

				if (m_bCreateExplosion)
				{
					DoExplosion();
				}

				m_bCreatedDebris = LTTRUE;

				if (m_bRemoveOnDeath)
				{
					pServerDE->RemoveObject(m_hObject);
				}
			}
		}
		break;

		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);

			// ConParse does not destroy szMsg, so this is safe
			ConParse parse;
			parse.Init((char*)szMsg);

			while (g_pLTServer->Common()->Parse(&parse) == LT_OK)
			{
				if (parse.m_nArgs > 0 && parse.m_Args[0])
				{
					if (!IsDead())
					{
						if (_stricmp(parse.m_Args[0], "FIRE") == 0)
						{
							char* pTargetName = parse.m_nArgs > 1 ? parse.m_Args[1] : LTNULL;
							DoExplosion(pTargetName);
						}
						else if (_stricmp(parse.m_Args[0], "HIDDEN") == 0)
						{
							uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);

							if (parse.m_nArgs > 1 && parse.m_Args[1])
							{
								if ((_stricmp(parse.m_Args[1], "1") == 0) ||
									(_stricmp(parse.m_Args[1], "TRUE") == 0))
								{
									dwFlags = 0;

									m_bSaveCanDamage = GetCanDamage();
									m_bSaveNeverDestroy = GetNeverDestroy();

									SetCanDamage(LTFALSE);
									SetNeverDestroy(LTTRUE);
								}
								else
								{
									if ((_stricmp(parse.m_Args[1], "0") == 0) ||
										(_stricmp(parse.m_Args[1], "FALSE") == 0))
									{
										dwFlags = m_dwOriginalFlags;
										dwFlags |= FLAG_VISIBLE;

										SetCanDamage(m_bSaveCanDamage);
										SetNeverDestroy(m_bSaveNeverDestroy);
									}
								}

								g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
							}
						}
					}
				}
			}
		}
		break;

		default : break;
	}

	return dwRet;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::ReadProp()
//
//	PURPOSE:	Reads CDestructibleModel properties
//
// --------------------------------------------------------------------------- //

LTBOOL CDestructibleModel::ReadProp(ObjectCreateStruct *)
{
    ILTServer* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return LTFALSE;

	GetDebrisProperties(m_nDebrisId);

	GenericProp genProp;
    if (g_pLTServer->GetPropGeneric("CreateExplosion", &genProp) == LT_OK)
	{
		m_bCreateExplosion = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("WeaponId", &genProp) == LT_OK)
	{
        m_nExplosionWeaponId = (uint8)genProp.m_Long;
	}

    if (g_pLTServer->GetPropGeneric("FireAlongForward", &genProp) == LT_OK)
	{
		m_bFireAlongForward = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("Spawn", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			 m_hstrSpawn = g_pLTServer->CreateString(genProp.m_String);
		}
	}


    if (g_pLTServer->GetPropGeneric("SurfaceOverride", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			 m_hstrSurfaceOverride = g_pLTServer->CreateString(genProp.m_String);
		}
	}


    if (g_pLTServer->GetPropGeneric("DamageFactor", &genProp) == LT_OK)
	{
		m_fDamageFactor = genProp.m_Float;
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::CreateWorldModelDebris()
//
//	PURPOSE:	Create world model debris...
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::CreateWorldModelDebris()
{
    ILTServer* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	char* pName = pServerDE->GetObjectName(m_hObject);
	if (!pName || !pName[0]) return;


	// Find all the debris objects...

	int nNum = 0;

	char strKey[128]; memset(strKey, 0, 128);
	char strNum[18];  memset(strNum, 0, 18);

	HCLASS hWMDebris = pServerDE->GetClass("WorldModelDebris");

	while (1)
	{
		// Create the keyname string...

		sprintf(strKey, "%sDebris%d", pName, nNum);

		// Find any debris with that name...

		ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
		pServerDE->FindNamedObjects(strKey, objArray);

		int numObjects = objArray.NumObjects();
		if (!numObjects) return;

		for (int i = 0; i < numObjects; i++)
		{
			HOBJECT hObject = objArray.GetObject(i);

			if (pServerDE->IsKindOf(pServerDE->GetObjectClass(hObject), hWMDebris))
			{
				WorldModelDebris* pDebris = (WorldModelDebris*)pServerDE->HandleToObject(hObject);
				if (!pDebris) break;

				LTVector vVel, vRotPeriods;
				vVel.Init(GetRandom(-200.0f, 200.0f),
					GetRandom(100.0f, 300.0f), GetRandom(-200.0f, 200.0f));

				vRotPeriods.Init(GetRandom(-1.0f, 1.0f),
					GetRandom(-1.0f, 1.0f), GetRandom(-1.0f, 1.0f));

				pDebris->Start(&vRotPeriods, &vVel);
			}
		}

		// Increment the counter...

		nNum++;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::CreateDebris()
//
//	PURPOSE:	Create debris...
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::CreateDebris()
{
	DEBRIS* pDebris = g_pDebrisMgr->GetDebris(m_nDebrisId);
	if (pDebris)
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

        LTVector vDeathDir = GetDeathDir();
        ::CreatePropDebris(vPos, vDeathDir, m_nDebrisId);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::DoExplosion()
//
//	PURPOSE:	Handle doing explosion
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::DoExplosion(char* pTargetName)
{
	CWeapons weapons;
	weapons.Init(m_hObject);
	weapons.ObtainWeapon(m_nExplosionWeaponId);
	weapons.ChangeWeapon(m_nExplosionWeaponId);

	CWeapon* pWeapon = weapons.GetCurWeapon();
	if (!pWeapon) return;

	weapons.SetAmmo(pWeapon->GetAmmoId());

	pWeapon->SetDamageFactor(m_fDamageFactor);

	LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	LTVector vU, vR, vF, vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pLTServer->GetRotationVectors(&rRot, &vU, &vR, &vF);

	// Just blow up in place if we're not supposed to fire along
	// forward vector and we don't have a target...

	if (!m_bFireAlongForward)
	{
		pWeapon->SetLifetime(0.0f);
		VEC_SET(vF, 0.0f, -1.0f, 0.0f);  // Fire down
	}

	// See if we have a target...If so, point at it.

	if (pTargetName)
	{
		ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
		g_pLTServer->FindNamedObjects(pTargetName, objArray);

		if (objArray.NumObjects())
		{
			LTVector vObjPos;
			g_pLTServer->GetObjectPos(objArray.GetObject(0), &vObjPos);
			vF = vObjPos - vPos;
			vF.Norm();

			g_pLTServer->AlignRotation(&rRot, &vF, LTNULL);
			g_pLTServer->RotateObject(m_hObject, &rRot);
		}
	}

	WFireInfo fireInfo;
	fireInfo.hFiredFrom	= m_hObject;
	fireInfo.vPath		= vF;
	fireInfo.vFirePos	= vPos;
	fireInfo.vFlashPos	= vPos;

	pWeapon->Fire(fireInfo);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::SpawnItem()
//
//	PURPOSE:	Spawn an item
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::SpawnItem()
{
	if (!m_hstrSpawn) return;

	LTVector vPos;
	LTRotation rRot;
	char szSpawn[256];

	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	strncpy(szSpawn, g_pLTServer->GetStringData(m_hstrSpawn), ARRAY_LEN(szSpawn));
	szSpawn[255] = '\0';
	SpawnObject(szSpawn, vPos, rRot);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::CacheFiles
//
//	PURPOSE:	Caches files used by object.
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::CacheFiles()
{
	// Don't cache if the world already loaded...

	DEBRIS* pDebris = g_pDebrisMgr->GetDebris(m_nDebrisId);
	if (pDebris)
	{
		pDebris->Cache(g_pDebrisMgr);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!g_pLTServer || !hWrite) return;

	SAVE_HSTRING(m_hstrSurfaceOverride);
	g_pLTServer->WriteToMessageHString(hWrite, m_hstrSpawn);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fDamageFactor);
	g_pLTServer->WriteToMessageByte(hWrite, m_bCreatedDebris);
	g_pLTServer->WriteToMessageByte(hWrite, m_nDebrisId);
	g_pLTServer->WriteToMessageByte(hWrite, m_bFireAlongForward);
	g_pLTServer->WriteToMessageByte(hWrite, m_bCreateExplosion);
	g_pLTServer->WriteToMessageByte(hWrite, m_nExplosionWeaponId);
	g_pLTServer->WriteToMessageByte(hWrite, m_bSaveCanDamage);
	g_pLTServer->WriteToMessageByte(hWrite, m_bSaveNeverDestroy);
	g_pLTServer->WriteToMessageDWord(hWrite, m_dwOriginalFlags);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!g_pLTServer || !hRead) return;

	LOAD_HSTRING(m_hstrSurfaceOverride);
	m_hstrSpawn				= g_pLTServer->ReadFromMessageHString(hRead);
	m_fDamageFactor			= g_pLTServer->ReadFromMessageFloat(hRead);
    m_bCreatedDebris        = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
	m_nDebrisId				= g_pLTServer->ReadFromMessageByte(hRead);
    m_bFireAlongForward     = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bCreateExplosion      = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
	m_nExplosionWeaponId	= g_pLTServer->ReadFromMessageByte(hRead);
	m_bSaveCanDamage		= (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
	m_bSaveNeverDestroy		= (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
	m_dwOriginalFlags		= g_pLTServer->ReadFromMessageDWord(hRead);
}