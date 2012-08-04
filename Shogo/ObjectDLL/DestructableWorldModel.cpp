// ----------------------------------------------------------------------- //
//
// MODULE  : DestructableWorldModel.cpp
//
// PURPOSE : DestructableWorldModel aggregate
//
// CREATED : 4/23/98
//
// ----------------------------------------------------------------------- //

// Includes...
#include "DestructableWorldModel.h"
#include "RiotObjectUtilities.h"
#include "WorldModelDebris.h"
#include "DebrisFuncs.h"
#include "Weapons.h"
#include "Spawner.h"
#include <stdio.h>
	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableWorldModel::CDestructableWorldModel()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CDestructableWorldModel::CDestructableWorldModel() : CDestructable()
{
	m_fMass		= INFINITE_MASS;
	m_fHitPts	= 100.0f;
	m_fArmor	= 100.0f;

	m_hstrDestroySound	= DNULL;
	m_fSoundRadius		= 1500.0f;
	m_bCreatedDebris	= DFALSE;

	m_nMinNumDebris		= 10;
	m_nMaxNumDebris		= 20;

	m_eDebrisType		= DBT_METAL_BIG;
	m_eSurfaceType		= ST_METAL;

	// Explosion stuff..

	m_bCreateExplosion		= DFALSE;
	m_nExplosionWeaponId	= GUN_BULLGUT_ID;
	m_eExplosionSize		= MS_NORMAL;
	m_bFireAlongForward		= DFALSE;
	m_fDamageFactor			= 1.0f;

	m_hstrSpawn				= DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableWorldModel::~CDestructableWorldModel()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CDestructableWorldModel::~CDestructableWorldModel()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if (m_hstrDestroySound)
	{
		pServerDE->FreeString(m_hstrDestroySound);
	}

	if (m_hstrSpawn)
	{
		pServerDE->FreeString(m_hstrSpawn);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableWorldModel::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //
DDWORD CDestructableWorldModel::EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT fData)
{
	switch (messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == 1.0f)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			int nInfo = (int)fData;
			if (nInfo != INITIALUPDATE_SAVEGAME)
			{
				SetMass(m_fMass);
				SetMaxHitPoints(m_fHitPts);
				SetHitPoints(m_fHitPts);
				SetMaxArmorPoints(m_fArmor);
				SetArmorPoints(m_fArmor);

				DDWORD dwUsrFlgs = g_pServerDE->GetObjectUserFlags(m_hObject);
				g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlgs | SurfaceToUserFlag(m_eSurfaceType));
			}
			CacheFiles( );
			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
		}
		break;

		default : break;
	}

	return CDestructable::EngineMessageFn(pObject, messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableWorldModel::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD CDestructableWorldModel::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	DDWORD dwRet = CDestructable::ObjectMessageFn(pObject, hSender, messageID, hRead);

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			if (IsDead() && !m_bCreatedDebris)
			{
				CServerDE* pServerDE = BaseClass::GetServerDE();
				if (!pServerDE) break;

				SpawnItem();
				CreateDebris();
				CreateWorldModelDebris();
				DoExplosion();

				m_bCreatedDebris = DTRUE;
				pServerDE->RemoveObject(m_hObject);
			}
		}

		default : break;
	}

	return dwRet;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableWorldModel::ReadProp()
//
//	PURPOSE:	Reads CDestructableWorldModel properties
//
// --------------------------------------------------------------------------- //

DBOOL CDestructableWorldModel::ReadProp(ObjectCreateStruct *)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->GetPropReal("Mass", &m_fMass);
	pServerDE->GetPropReal("HitPoints", &m_fHitPts);
	pServerDE->GetPropReal("Armor", &m_fArmor);

	char buf[MAX_CS_FILENAME_LEN];
	buf[0] = '\0';
	if (pServerDE->GetPropString("DestroySound", buf, MAX_CS_FILENAME_LEN) == DE_OK)
	{
		if (buf[0] && strlen(buf)) m_hstrDestroySound = pServerDE->CreateString(buf);
	}

	pServerDE->GetPropReal("SoundRadius", &m_fSoundRadius);
		
	long nLong;
	pServerDE->GetPropLongInt("DebrisType", &nLong);
	m_eDebrisType = (DebrisType)nLong;

	m_eSurfaceType = GetDebrisSurfaceType(m_eDebrisType);

	GenericProp genProp;
	if (g_pServerDE->GetPropGeneric( "CreateExplosion", &genProp ) == DE_OK)
		m_bCreateExplosion = genProp.m_Bool;

	if (g_pServerDE->GetPropGeneric( "WeaponId", &genProp ) == DE_OK)
		m_nExplosionWeaponId = (DBYTE)genProp.m_Long;

	if (g_pServerDE->GetPropGeneric( "ExplosionSize", &genProp ) == DE_OK)
		m_eExplosionSize = (ModelSize)genProp.m_Long;
	
	if (g_pServerDE->GetPropGeneric( "FireAlongForward", &genProp ) == DE_OK)
		m_bFireAlongForward = genProp.m_Bool;

	if (g_pServerDE->GetPropGeneric( "MinNumDebris", &genProp ) == DE_OK)
		m_nMinNumDebris = (DBYTE)genProp.m_Long;

	if (g_pServerDE->GetPropGeneric( "MaxNumDebris", &genProp ) == DE_OK)
		m_nMaxNumDebris = (DBYTE)genProp.m_Long;

	if (g_pServerDE->GetPropGeneric("Spawn", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrSpawn = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric( "DamageFactor", &genProp ) == DE_OK)
		m_fDamageFactor = genProp.m_Float;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableWorldModel::CreateWorldModelDebris()
//
//	PURPOSE:	Create world model debris...
//
// ----------------------------------------------------------------------- //

void CDestructableWorldModel::CreateWorldModelDebris()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
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

		ObjectList* pTempKeyList = pServerDE->FindNamedObjects(strKey);
		ObjectLink* pLink = pTempKeyList->m_pFirstLink;
		if (!pLink) return;
		
		while (pLink)
		{
			if (pServerDE->IsKindOf(pServerDE->GetObjectClass(pLink->m_hObject), hWMDebris))
			{
				WorldModelDebris* pDebris = (WorldModelDebris*)pServerDE->HandleToObject(pLink->m_hObject);
				if (!pDebris) break;

				DVector vVel, vRotPeriods;
				VEC_SET(vVel, GetRandom(-200.0f, 200.0f), 
							  GetRandom(100.0f, 300.0f),
							  GetRandom(-200.0f, 200.0f) );

				VEC_SET(vRotPeriods, GetRandom(-1.0f, 1.0f),
						GetRandom(-1.0f, 1.0f), GetRandom(-1.0f, 1.0f));

				pDebris->Start(&vRotPeriods, &vVel);
			}

			pLink = pLink->m_pNext;
		}

		pServerDE->RelinquishList (pTempKeyList);
		
		// Increment the counter...

		nNum++;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableWorldModel::CreateDebris()
//
//	PURPOSE:	Create debris...
//
// ----------------------------------------------------------------------- //

void CDestructableWorldModel::CreateDebris()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || m_nMaxNumDebris <= 0) return;

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

	if (m_hstrDestroySound)
	{
		char* pSound = pServerDE->GetStringData(m_hstrDestroySound);
		if (pSound) PlaySoundFromPos(&vPos, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_HIGH );
	}	

	DFLOAT fDimsMag = GetRandom(10.0f, 20.0f);
	::CreatePropDebris(vPos, fDimsMag, GetDeathDir(),
					   m_eDebrisType, m_nMinNumDebris, m_nMaxNumDebris);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableWorldModel::DoExplosion()
//
//	PURPOSE:	Handle doing explosion
//
// ----------------------------------------------------------------------- //

void CDestructableWorldModel::DoExplosion()
{
	if (!m_bCreateExplosion) return;

	CWeapons weapons;
	weapons.Init(m_hObject, m_eExplosionSize);
	weapons.SetArsenal(CWeapons::AT_AS_NEEDED);
	weapons.ObtainWeapon(m_nExplosionWeaponId);
	weapons.ChangeWeapon(m_nExplosionWeaponId);
	weapons.AddAmmo(m_nExplosionWeaponId, GetWeaponMaxAmmo(m_nExplosionWeaponId));

	CWeapon* pWeapon = weapons.GetCurWeapon();
	if (!pWeapon) return;

	pWeapon->SetDamageFactor(m_fDamageFactor);
	pWeapon->SetCanLockOnTarget(DFALSE);

	DRotation rRot;
	g_pServerDE->GetObjectRotation(m_hObject, &rRot);

	DVector vU, vR, vF, vPos;
	g_pServerDE->GetObjectPos(m_hObject, &vPos);
	g_pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	// Just blow up in place if we're not supposed to fire along
	// forward vector...

	if (!m_bFireAlongForward)
	{
		pWeapon->SetLifetime(0.0f);
		VEC_SET(vF, 0.0f, -1.0f, 0.0f);  // Fire down
	}

	pWeapon->Fire(m_hObject, vF, vPos);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableWorldModel::SpawnItem()
//
//	PURPOSE:	Spawn an item
//
// ----------------------------------------------------------------------- //

void CDestructableWorldModel::SpawnItem( )
{
	DVector vPos;
	DRotation rRot;
	char szSpawn[MAX_CS_FILENAME_LEN+1];

	if (!m_hstrSpawn) return;

	g_pServerDE->GetObjectPos(m_hObject, &vPos);
	g_pServerDE->GetObjectRotation(m_hObject, &rRot);

	strncpy( szSpawn, g_pServerDE->GetStringData(m_hstrSpawn), MAX_CS_FILENAME_LEN );
	szSpawn[MAX_CS_FILENAME_LEN] = '\0';
	SpawnObject(szSpawn, &vPos, &rRot);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableWorldModel::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CDestructableWorldModel::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!g_pServerDE || !hWrite) return;

	g_pServerDE->WriteToMessageHString(hWrite, m_hstrSpawn);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrDestroySound);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fDamageFactor);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fSoundRadius);
	g_pServerDE->WriteToMessageByte(hWrite, m_bCreatedDebris);
	g_pServerDE->WriteToMessageByte(hWrite, m_nMinNumDebris);
	g_pServerDE->WriteToMessageByte(hWrite, m_nMaxNumDebris);
	g_pServerDE->WriteToMessageByte(hWrite, m_bFireAlongForward);
	g_pServerDE->WriteToMessageByte(hWrite, m_bCreateExplosion);
	g_pServerDE->WriteToMessageByte(hWrite, m_nExplosionWeaponId);
	g_pServerDE->WriteToMessageByte(hWrite, m_eExplosionSize);
	g_pServerDE->WriteToMessageByte(hWrite, m_eDebrisType);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableWorldModel::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CDestructableWorldModel::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!g_pServerDE || !hRead) return;

	m_hstrSpawn				= g_pServerDE->ReadFromMessageHString(hRead);
	m_hstrDestroySound		= g_pServerDE->ReadFromMessageHString(hRead);
	m_fDamageFactor			= g_pServerDE->ReadFromMessageFloat(hRead);
	m_fSoundRadius			= g_pServerDE->ReadFromMessageFloat(hRead);
	m_bCreatedDebris		= (DBOOL) g_pServerDE->ReadFromMessageByte(hRead);
	m_nMinNumDebris			= g_pServerDE->ReadFromMessageByte(hRead);
	m_nMaxNumDebris			= g_pServerDE->ReadFromMessageByte(hRead);
	m_bFireAlongForward		= (DBOOL) g_pServerDE->ReadFromMessageByte(hRead);
	m_bCreateExplosion		= (DBOOL) g_pServerDE->ReadFromMessageByte(hRead);
	m_nExplosionWeaponId	= g_pServerDE->ReadFromMessageByte(hRead);
	m_eExplosionSize		= (ModelSize) g_pServerDE->ReadFromMessageByte(hRead);
	m_eDebrisType			= (DebrisType) g_pServerDE->ReadFromMessageByte(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableWorldModel::CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void CDestructableWorldModel::CacheFiles()
{
	if (!g_pServerDE) return;

	char* pFile = DNULL;

	if( !( g_pServerDE->GetServerFlags( ) & SS_CACHING ))
		return;

	if (m_hstrDestroySound)
	{
		pFile = g_pServerDE->GetStringData(m_hstrDestroySound);
		if (pFile)
		{
			 g_pServerDE->CacheFile(FT_SOUND,pFile);
		}
	}
}


