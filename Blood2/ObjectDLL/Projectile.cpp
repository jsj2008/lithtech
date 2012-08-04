// ----------------------------------------------------------------------- //
//
// MODULE  : Projectile.cpp
//
// PURPOSE : Projectile class - implementation
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "Projectile.h"
#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "Impacts.h"
#include "generic_msg_de.h"
#include "ClientSmokeTrail.h"
#include "ObjectUtilities.h"
#include "Explosion.h"
#include "ClientSparksSFX.h"
#include "ClientWeaponSFX.h"
#include "ClientExplosionSFX.h"
#include "ClientServerShared.h"
#include "SfxMsgIds.h"
#include <mbstring.h>
#include "SoundTypes.h"

void BPrint(char*);

BEGIN_CLASS(CProjectile)
END_CLASS_DEFAULT_FLAGS(CProjectile, BaseClass, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::CProjectile
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CProjectile::CProjectile(DBYTE nType) : BaseClass(nType)
{
	AddAggregate(&m_damage);
	m_fVelocity				= 0.0f;
	m_fDamage				= 0.0f;
	m_nRadius				= 0;
	m_fLifeTime				= 10.0f;

	VEC_SET(m_vInitScale, 1.0f, 1.0f, 1.0f);
	m_hFiredFrom			= DNULL;
	m_nDamageType			= DAMAGE_TYPE_NORMAL;

	m_fStartTime			= 0.0f;
	m_bExplode				= DFALSE;
	m_bClientFX				= DTRUE;
	m_hHitObject			= DNULL;

	m_fImpactDuration		= 1.0f;
	m_fImpactScaleMin		= 1.0f;
	m_fImpactScaleMax		= 1.0f;
	m_bShockwave			= DFALSE;
	m_bSparks				= DFALSE;
	m_bExplosion			= DFALSE;
	m_fShockwaveDuration	= 0.0f;
	m_hstrShockwaveFilename	= DNULL;
	VEC_SET(m_vShockwaveScaleMin, 0.1f, 0.1f, 1.0f);
	VEC_SET(m_vShockwaveScaleMax, 1.0f, 1.0f, 1.0f);

	m_hLight				= NULL;

	m_bSmokeTrail			= DFALSE;
	m_hSmokeTrail			= DNULL;

	m_pProjectileFilename	= DNULL;
	m_pProjectileSkin		= DNULL;

	m_hSound				= DNULL;

	m_dwClientID			= 0;

	VEC_SET(m_vDims, 1, 1, 1);
	VEC_SET(m_vLightColor, 1, 1, 1);

	m_dwFlags = FLAG_VISIBLE | FLAG_TOUCH_NOTIFY | FLAG_REMOVEIFOUTSIDE | FLAG_FORCECLIENTUPDATE;

	if (nType == OT_MODEL)
		m_dwFlags |= FLAG_SHADOW | FLAG_MODELGOURAUDSHADE | FLAG_RAYHIT;

	// Particle trail values
	VEC_SET(m_vTrailOffset, 0.0f, 0.0f, 0.0f);
	m_fTrailScale			= 1.0f;
	m_dwTrailScaleFlags		= 0;
	m_dwTrailFXID			= OBJFX_NONE;
	m_dwTrailFXFlags		= 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::~CProjectile
//
//	PURPOSE:	Desctructor
//
// ----------------------------------------------------------------------- //

CProjectile::~CProjectile()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if (m_hLight)
		pServerDE->RemoveObject(m_hLight);

	if (m_hSmokeTrail) 
		pServerDE->RemoveObject(m_hSmokeTrail);

	if (m_hstrShockwaveFilename)
		pServerDE->FreeString(m_hstrShockwaveFilename);

	if (m_hSound)
		{ pServerDE->KillSound(m_hSound); m_hSound = DNULL; }
}
	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CProjectile::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (!Update((DVector *)pData))
			{
				CServerDE* pServerDE = BaseClass::GetServerDE();
				if (!pServerDE) return 0;

				pServerDE->RemoveObject(m_hObject);		
			}
			break;
		}

		case MID_PRECREATE:
		{
			ObjectCreateStruct *ocs = (ObjectCreateStruct*)pData;
			m_dwClientID = ocs->m_UserData;
			ocs = DNULL;

			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			if (fData != PRECREATE_SAVEGAME)
				PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
				InitialUpdate((DVector *)pData);
			break;
		}

		case MID_TOUCHNOTIFY:
		{
			HandleTouch((HOBJECT)pData);
			break;
		}

		case MID_LINKBROKEN:
		{
			HOBJECT hObj = (HOBJECT)pData;
			BreakLink(hObj);
			break;
		}

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;

		default : break;
	}


	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::BreakLink
//
//	PURPOSE:	Breaks a link of an object
//
// ----------------------------------------------------------------------- //

void CProjectile::BreakLink(HOBJECT hObj)
{
	if(m_hHitObject == hObj)
		m_hHitObject = DNULL;
	else if(m_hFiredFrom == hObj)
		m_hFiredFrom = DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Init
//
//	PURPOSE:	Initialize a projectile
//
// ----------------------------------------------------------------------- //

void CProjectile::Setup(DVector *vDir, DBYTE nType, DFLOAT fDamage, DFLOAT fVelocity,
						int nRadius, HOBJECT hFiredFrom)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	VEC_COPY(m_vDir, *vDir);
	m_fDamage		= fDamage;
	m_fVelocity		= fVelocity;
	m_nRadius		= nRadius;

	m_hFiredFrom	= hFiredFrom;
	pServerDE->CreateInterObjectLink( m_hObject, m_hFiredFrom );

	m_nWeaponType	= nType;

	DVector vVel;
	VEC_MULSCALAR(vVel, m_vDir, m_fVelocity);

	// Set the rotation of the projectile...
	DRotation rRot;
	pServerDE->AlignRotation(&rRot, &m_vDir, DNULL);
	pServerDE->SetObjectRotation(m_hObject, &rRot);

	// And away we go...
	pServerDE->SetVelocity(m_hObject, &vVel);
	pServerDE->SetObjectFlags(m_hObject, m_dwFlags);


	// Add smoke trail
	if (m_bSmokeTrail) AddSmokeTrail(vVel);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void CProjectile::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	if (m_pProjectileFilename) 
		_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)m_pProjectileFilename);
	else 
		_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)"Models\\Ammo\\Grenade.abc");
	
	if (m_pProjectileSkin) 
		_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)m_pProjectileSkin);
	else 
		_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)"Skins\\Ammo\\Grenade.dtx");

	pStruct->m_NextUpdate = 0.001f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::InitialUpdate()
//
//	PURPOSE:	Do first update
//
// ----------------------------------------------------------------------- //

DBOOL CProjectile::InitialUpdate(DVector*)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);
	m_fStartTime = pServerDE->GetTime();

	pServerDE->SetObjectDims(m_hObject, &m_vDims);

	pServerDE->SetForceIgnoreLimit(m_hObject, 0.0f);
	pServerDE->SetFrictionCoefficient(m_hObject, 18.0);
	pServerDE->ScaleObject(m_hObject, &m_vInitScale);

	// Initialize damage
	m_damage.Init(m_hObject);
	m_damage.SetHitPoints(5.0f);		// A minor number of  hitpoints
	m_damage.SetApplyDamagePhysics(DFALSE);

	// Mark this object as savable
	DDWORD dwFlags = pServerDE->GetObjectUserFlags(m_hObject);
	dwFlags |= USRFLG_SAVEABLE | USRFLG_SINGULARITY_ATTRACT;
	pServerDE->SetObjectUserFlags(m_hObject, dwFlags);

	// Create a particle trail
	if(m_dwTrailFXID)
	{
		HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
		pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);

		pServerDE->WriteToMessageObject(hMessage, m_hObject);
		pServerDE->WriteToMessageVector(hMessage, &m_vTrailOffset);
		pServerDE->WriteToMessageFloat(hMessage, m_fTrailScale);
		pServerDE->WriteToMessageDWord(hMessage, m_dwTrailScaleFlags);
		pServerDE->WriteToMessageDWord(hMessage, m_dwTrailFXID);
		pServerDE->WriteToMessageDWord(hMessage, m_dwTrailFXFlags);

		pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

DBOOL CProjectile::Update(DVector *pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);

//	pServerDE->BPrint("hit points %f", m_damage.GetHitPoints());
	if (m_bExplode || m_damage.IsDead())
	{
		Explode();
		return DFALSE;
	}

	if (m_fLifeTime > 0)
	{
		DFLOAT fTime = pServerDE->GetTime();

		if(fTime > (m_fStartTime + m_fLifeTime)) 
		{
			return DFALSE;
		} 
	}

	DVector vVel, vPos;
	pServerDE->GetVelocity(m_hObject, &vVel);

	pServerDE->GetObjectPos(m_hObject, &vPos);

	if (m_hSmokeTrail)
	{
		pServerDE->SetObjectPos(m_hSmokeTrail, &vPos);
	}

	if (m_hLight)
	{
		pServerDE->SetObjectPos(m_hLight, &vPos);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CProjectile::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if (hObj == m_hFiredFrom) return; // Let it get out of our bounding box...

	// return if it hit a non solid object 
	if (hObj != pServerDE->GetWorldObject() && !(pServerDE->GetObjectFlags(hObj) & FLAG_SOLID)) // Ignore non-solid objects
		return;

	// return if it hit another of this same class
	if (pServerDE->GetObjectClass(hObj) == pServerDE->GetObjectClass(m_hObject))
		return;

	// See if we hit the sky...
	HCLASS hClassObj	= pServerDE->GetObjectClass(hObj);
	HCLASS hClassWorld  = pServerDE->GetObjectClass(pServerDE->GetWorldObject());
	if (pServerDE->IsKindOf(hClassObj, hClassWorld))
	{
		CollisionInfo info;
		pServerDE->GetLastCollision(&info);

		SurfaceType eType = GetSurfaceType(info.m_hObject, info.m_hPoly);

		if (eType == SURFTYPE_SKY)
		{
			pServerDE->RemoveObject(m_hObject);
			return;
		}
	}


	m_bExplode = DTRUE;
	m_hHitObject = hObj;
	// GK 8/4/98
	// Keep a link to this object in case it blows up for some reason (so m_hHitObject is never invalid)
	pServerDE->CreateInterObjectLink(m_hObject, hObj);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Explode()
//
//	PURPOSE:	Creates the impact and does damage
//
// ----------------------------------------------------------------------- //

void CProjectile::Explode()
{
	if (!g_pServerDE) return;

	DVector vPos;
	g_pServerDE->GetObjectPos(m_hObject, &vPos);

	AddImpact(vPos, vPos, m_hHitObject);

	if(m_nRadius) 
	{
		DamageObjectsWithinRadius();
	} 
	else if (m_hHitObject)
	{
		// Send damage message to object...
		DamageObject(m_hFiredFrom, this, m_hHitObject, m_fDamage, m_vDir, vPos, m_nDamageType); 
	}
	
	if (m_hHitObject)
	{
		g_pServerDE->BreakInterObjectLink(m_hObject, m_hHitObject);
		m_hHitObject = DFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::DamageObjectsWithinRadius()
//
//	PURPOSE:	Damage the objects within our radius
//
// ----------------------------------------------------------------------- //

void CProjectile::DamageObjectsWithinRadius()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

	DFLOAT fRadius = (DFLOAT)m_nRadius;

	DamageObjectsInRadius(m_hFiredFrom, this, vPos, fRadius, m_fDamage, m_nDamageType);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddImpact()
//
//	PURPOSE:	Add impact objects
//
// ----------------------------------------------------------------------- //

void CProjectile::AddImpact(DVector vPoint, DVector vNormal, HOBJECT hObj)
{
	// Compute a normal vector
	// Cast a ray to see what we hit
	DVector		tempPos;

	IntersectQuery iq;
	IntersectInfo  ii;
	SurfaceType eType = SURFTYPE_UNKNOWN;
	DBYTE nFX = 0;

	VEC_NORM(m_vDir);
	VEC_MULSCALAR(m_vDir, m_vDir, 2.0f);
	VEC_SUB(iq.m_From, vPoint, m_vDir);
	VEC_MULSCALAR(m_vDir, m_vDir, 10.0f);
	VEC_ADD(iq.m_To, vPoint, m_vDir);
	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = NULL;
	iq.m_pUserData = NULL;	

	if (hObj && g_pServerDE->IntersectSegment(&iq, &ii))
	{
		VEC_COPY(vNormal, ii.m_Plane.m_Normal);
		eType = GetSurfaceType(ii.m_hObject, ii.m_hPoly);

		VEC_COPY(tempPos, ii.m_Point);
	}
	else	// Fake it
	{
		VEC_NEGATE(vNormal, m_vDir);

		VEC_COPY(tempPos, vPoint);
	}

	if(m_bExplosion) 
	{
		AddExplosion(tempPos, vNormal);
		nFX |= WFX_SCREENSHAKE;
	}

	if(m_bClientFX)
	{
		CServerDE* pServerDE = BaseClass::GetServerDE();
		if (!pServerDE) return;

/*		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT(theStruct);

		VEC_COPY(theStruct.m_Pos, vPoint);

		HCLASS hClass = pServerDE->GetClass("CClientWeaponSFX");

		CClientWeaponSFX *pWeaponFX = DNULL;

		if(hClass)
			pWeaponFX = (CClientWeaponSFX*)pServerDE->CreateObject(hClass, &theStruct);
*/
		DDWORD	nFX = WFX_SCREENSHAKE;
		DDWORD	nExtras = WFX_EXTRA_DAMAGE;
		WeaponFXExtras	ext;
		ext.fDamage = m_fDamage;

//		if(pWeaponFX)
		SendWeaponSFXMessage(&vPoint, &vPoint, &vNormal, &vNormal, nFX, nExtras, &ext);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddBloodImpact()
//
//	PURPOSE:	Add blood impacts objects
//
// ----------------------------------------------------------------------- //

void CProjectile::AddBloodImpact(DVector *vPos, DVector *vNormal, DFLOAT fDamage)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

/*	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	VEC_COPY(theStruct.m_Pos, *vPos);

	HCLASS hClass = pServerDE->GetClass("CClientWeaponSFX");

	CClientWeaponSFX *pWeaponFX = DNULL;

	if(hClass)
		pWeaponFX = (CClientWeaponSFX*)pServerDE->CreateObject(hClass, &theStruct);
*/
	DDWORD	nFX = WFX_BLOODSPURT | WFX_SPARKS;
	DDWORD	nExtras = WFX_EXTRA_DAMAGE | WFX_EXTRA_SURFACETYPE;
	WeaponFXExtras	ext;
	ext.fDamage = fDamage;
	ext.nSurface = (DBYTE)SURFTYPE_FLESH;

//	if(pWeaponFX)
	SendWeaponSFXMessage(vPos, vPos, vNormal, vNormal, nFX, nExtras, &ext);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CProjectile::AddExplosion(DVector vPos, DVector vNormal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_GRENADE;
	DVector		vUp;
	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vUp);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\c4\\explosion_1.wav", 1000.0f, SOUNDPRIORITY_MISC_MEDIUM);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddSmokeTrail()
//
//	PURPOSE:	Add a client-side smoke trail
//
// ----------------------------------------------------------------------- //

void CProjectile::AddSmokeTrail(DVector vVel)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);
	VEC_COPY(theStruct.m_Pos, vPos);

	HCLASS hClass = pServerDE->GetClass("CClientSmokeTrail");

	CClientSmokeTrail* pTrail = DNULL;

	if (hClass)
	{
		pTrail = (CClientSmokeTrail*)pServerDE->CreateObject(hClass, &theStruct);
	}

	if (pTrail)
	{
		pTrail->Setup(vVel, DTRUE);
		m_hSmokeTrail = pTrail->m_hObject;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddLight()
//
//	PURPOSE:	Add a dynamic light
//
// ----------------------------------------------------------------------- //

void CProjectile::AddLight(DFLOAT fRadius, DFLOAT fRed, DFLOAT fGreen, DFLOAT fBlue)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_vLightColor.x = fRed;
	m_vLightColor.y = fGreen;
	m_vLightColor.z = fBlue;

	if (fRadius)
	{
		ObjectCreateStruct ocStruct;
		INIT_OBJECTCREATESTRUCT(ocStruct);

		ocStruct.m_Flags = FLAG_VISIBLE;
		ocStruct.m_ObjectType = OT_LIGHT; 

		HCLASS hClass = pServerDE->GetClass("BaseClass");
		LPBASECLASS	pFlash = pServerDE->CreateObject(hClass, &ocStruct);

		if (pFlash)
		{
			m_hLight = pFlash->m_hObject;
			pServerDE->SetLightRadius(m_hLight, fRadius);
			pServerDE->SetLightColor(m_hLight, m_vLightColor.x, m_vLightColor.y, m_vLightColor.z);
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddSparks
//
//	PURPOSE:	Add impact sparks
//
// ----------------------------------------------------------------------- //

void CProjectile::AddSparks(DVector vPos, DVector vNormal, DFLOAT fDamage, HOBJECT hObject)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	VEC_COPY(ocStruct.m_Pos, vPos);

	HCLASS hClass = pServerDE->GetClass("CClientSparksSFX");

	CClientSparksSFX *pSpark = DNULL;

	if (hClass)
	{
		pSpark = (CClientSparksSFX *)pServerDE->CreateObject(hClass, &ocStruct);
	}

	if (pSpark)
	{
		DVector vColor1, vColor2;
		char* pFile;
		DBYTE nSparks = (DBYTE)fDamage;

		SurfaceType eType = GetSurfaceType(hObject, DNULL);

		switch(eType)
		{
			case SURFTYPE_FLESH:
			{
				VEC_SET(vColor1, 200.0f, 0.0f, 0.0f);
				VEC_SET(vColor2, 255.0f, 0.0f, 0.0f);

				pFile = "spritetextures\\particle1.dtx";

				pSpark->Setup(&vNormal, &vColor1, &vColor2, pFile, nSparks, 0.6f, 2.0f, 500.0f);
				break;
			}
			default:
			{
				VEC_SET(vColor1, 200.0f, 200.0, 200.0f);
				VEC_SET(vColor2, 200.0f, 200.0f, 0.0f);

				pFile = "spritetextures\\particle1.dtx";

				pSpark->Setup(&vNormal, &vColor1, &vColor2, pFile, nSparks, 0.4f, 2.0f, 500.0f);
				break;
			}
		}
	}

	return;
}


/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::GetSurfaceType
//
//	PURPOSE:	Determines the surface type of a hit object
//
// ----------------------------------------------------------------------- //

SurfaceType CProjectile::GetSurfaceType(HOBJECT hObject)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hObject) return ST_UNKNOWN;
	
	SurfaceType eType = ST_UNKNOWN;

	if (pServerDE->GetWorldObject() == hObject)
	{
		eType = ST_WORLD;
	}
	else
	{
		HCLASS hObjClass  = pServerDE->GetObjectClass(hObject);
		HCLASS hCharacter = pServerDE->GetClass("CBaseCharacter");

		if (pServerDE->IsKindOf(hObjClass, hCharacter))
		{
			eType = ST_CHARACTER;
		}
	}

	return eType;
}

*/

void CProjectile::SetFlags(DDWORD dwFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_dwFlags = dwFlags;
	pServerDE->SetObjectFlags(m_hObject, m_dwFlags);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CProjectile::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	DFLOAT fTime = pServerDE->GetTime();

	pServerDE->WriteToMessageVector(hWrite, &m_vDir);
	pServerDE->WriteToMessageFloat(hWrite, m_fVelocity);
	pServerDE->WriteToMessageDWord(hWrite, m_dwFlags);

	pServerDE->WriteToMessageByte(hWrite, m_nDamageType);
//		HOBJECT			m_hFiredFrom;			// Who fired us?  Can't tell
	pServerDE->WriteToMessageDWord(hWrite, m_nRadius);
	pServerDE->WriteToMessageFloat(hWrite, m_fStartTime - fTime);
	pServerDE->WriteToMessageByte(hWrite, m_bRemoveFromWorld);
	pServerDE->WriteToMessageFloat(hWrite, m_fDamage);
	pServerDE->WriteToMessageFloat(hWrite, m_fLifeTime);
	pServerDE->WriteToMessageDWord(hWrite, m_nDamageMsgID);

//		DBOOL			m_bSmokeTrail;			// Do we generate a smoke trail

	pServerDE->WriteToMessageByte(hWrite, m_bShockwave);
	pServerDE->WriteToMessageByte(hWrite, m_bExplosion);
	pServerDE->WriteToMessageByte(hWrite, m_bSparks);
	pServerDE->WriteToMessageVector(hWrite, &m_vShockwaveScaleMin);
	pServerDE->WriteToMessageVector(hWrite, &m_vShockwaveScaleMax);
	pServerDE->WriteToMessageVector(hWrite, &m_vLightColor);
	pServerDE->WriteToMessageFloat(hWrite, m_fShockwaveDuration);
	pServerDE->WriteToMessageHString(hWrite,m_hstrShockwaveFilename);

	pServerDE->WriteToMessageVector(hWrite, &m_vDims);
	pServerDE->WriteToMessageByte(hWrite, m_bExplode);

	// Special members
	pServerDE->WriteToMessageVector(hWrite, &m_LastPos);
	pServerDE->WriteToMessageByte(hWrite, m_bArmed);
	pServerDE->WriteToMessageFloat(hWrite, m_fPitchVel);
	pServerDE->WriteToMessageFloat(hWrite, m_fYawVel);
	pServerDE->WriteToMessageFloat(hWrite, m_fPitch);
	pServerDE->WriteToMessageFloat(hWrite, m_fYaw);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CProjectile::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	DFLOAT fTime = pServerDE->GetTime();

	pServerDE->ReadFromMessageVector(hRead, &m_vDir);
	m_fVelocity				= pServerDE->ReadFromMessageFloat(hRead);
	m_dwFlags				= pServerDE->ReadFromMessageDWord(hRead);

	m_nDamageType			= pServerDE->ReadFromMessageByte(hRead);
//		HOBJECT			m_hFiredFrom;			// Who fired us?  Can't tell
	m_nRadius				= pServerDE->ReadFromMessageDWord(hRead);
	m_fStartTime			= pServerDE->ReadFromMessageFloat(hRead) + fTime;
	m_bRemoveFromWorld		= pServerDE->ReadFromMessageByte(hRead);
	m_fDamage				= pServerDE->ReadFromMessageFloat(hRead);
	m_fLifeTime				= pServerDE->ReadFromMessageFloat(hRead);
	m_nDamageMsgID			= pServerDE->ReadFromMessageDWord(hRead);

//		DBOOL			m_bSmokeTrail;			// Do we generate a smoke trail

	m_bShockwave			= pServerDE->ReadFromMessageByte(hRead);
	m_bExplosion			= pServerDE->ReadFromMessageByte(hRead);
	m_bSparks				= pServerDE->ReadFromMessageByte(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vShockwaveScaleMin);
	pServerDE->ReadFromMessageVector(hRead, &m_vShockwaveScaleMax);
	pServerDE->ReadFromMessageVector(hRead, &m_vLightColor);
	m_fShockwaveDuration	= pServerDE->ReadFromMessageFloat(hRead);
	m_hstrShockwaveFilename	= pServerDE->ReadFromMessageHString(hRead);

	pServerDE->ReadFromMessageVector(hRead, &m_vDims);
	m_bExplode				= pServerDE->ReadFromMessageByte(hRead);

	// Special members
	pServerDE->ReadFromMessageVector(hRead, &m_LastPos);
	m_bArmed	= pServerDE->ReadFromMessageByte(hRead);
	m_fPitchVel	= pServerDE->ReadFromMessageFloat(hRead);
	m_fYawVel	= pServerDE->ReadFromMessageFloat(hRead);
	m_fPitch	= pServerDE->ReadFromMessageFloat(hRead);
	m_fYaw		= pServerDE->ReadFromMessageFloat(hRead);
}



