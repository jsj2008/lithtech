// ----------------------------------------------------------------------- //
//
// MODULE  : RiotProjectile.cpp
//
// PURPOSE : Riot Projectile classs - implementation
//
// CREATED : 10/3/97
//
// ----------------------------------------------------------------------- //

#include "RiotProjectiles.h"
#include "RiotObjectUtilities.h"
#include "DamageTypes.h"
#include "SurfaceTypes.h"
#include "SurfaceFunctions.h"

#define RP_FIRE_ANI_STR		"Fire"
#define RP_FLIGHT_ANI_STR	"Flight"
#define RP_IMPACT_ANI_STR	"Impact"


BEGIN_CLASS(CRedRiotProjectile)
END_CLASS_DEFAULT_FLAGS(CRedRiotProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRedRiotProjectile::CRedRiotProjectile
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRedRiotProjectile::CRedRiotProjectile() : CProjectile()
{
	m_fExplosionDuration	= 2.0f;
	m_eDamageType			= DT_EXPLODE;
	m_pProjectileFilename	= "Models\\PV_Weapons\\RedRiot_projectile.abc";
	m_pProjectileSkin		= "Skins\\Weapons\\RedRiot_Projectile_a.dtx";
}


BEGIN_CLASS(CPulseRifleProjectile)
END_CLASS_DEFAULT_FLAGS(CPulseRifleProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPulseRifleProjectile::CPulseRifleProjectile
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPulseRifleProjectile::CPulseRifleProjectile() : CProjectile()
{
	m_fExplosionDuration	= .5f;
	m_eDamageType			= DT_ENERGY;
	m_pProjectileFilename	= "Sprites\\PulseRifle.spr";

	m_dwCantDamageTypes		= DT_ENERGY;

	VEC_SET(m_vDims, 1.0f, 1.0f, 1.0f);
	VEC_SET(m_vScale, 0.15f, 0.15f, 1.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPulseRifleProjectile::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CPulseRifleProjectile::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			// SetType(OT_SPRITE);
		}
		break;

		case MID_INITIALUPDATE:
		{
			CServerDE* pServerDE = BaseClass::GetServerDE();
			if (!pServerDE) return 0;

			pServerDE->SetObjectColor(m_hObject, 1.0f, 1.0f, 1.0f, 1.0f);
			break;
		}
	}

	return CProjectile::EngineMessageFn(messageID, pData, lData);
}

BEGIN_CLASS(CBullgutProjectile)
END_CLASS_DEFAULT_FLAGS(CBullgutProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBullgutProjectile::CBullgutProjectile
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CBullgutProjectile::CBullgutProjectile() : CProjectile()
{
	m_fExplosionDuration	= 1.5f;	
	m_eDamageType			= DT_EXPLODE;
	m_dwCantDamageTypes		= DT_EXPLODE;
	m_bSnakingOn			= DTRUE;
	VEC_SET(m_vScale, 15.0f, 15.0f, 15.0f);
	m_pProjectileFilename	= "Models\\PV_Weapons\\Bullgut_Projectile.abc";
	m_pProjectileSkin		= "Skins\\Weapons\\Bullgut_Projectile_a.dtx";
}

BEGIN_CLASS(CJuggernautProjectile)
END_CLASS_DEFAULT_FLAGS(CJuggernautProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CJuggernautProjectile::CJuggernautProjectile
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CJuggernautProjectile::CJuggernautProjectile() : CProjectile()
{
	m_fExplosionDuration	= 1.0f;
	m_eDamageType			= DT_EXPLODE;
	m_pProjectileFilename	= "Models\\PV_Weapons\\Bullgut_Projectile.abc";
	m_pProjectileSkin		= "Skins\\Weapons\\Bullgut_Projectile_a.dtx";
}

BEGIN_CLASS(CTOWProjectile)
END_CLASS_DEFAULT_FLAGS(CTOWProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTOWProjectile::CTOWProjectile
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTOWProjectile::CTOWProjectile() : CProjectile()
{
	m_fExplosionDuration	= 1.0f;
	m_eDamageType			= DT_EXPLODE;
	m_dwCantDamageTypes		= DT_EXPLODE;
	m_bSnakingOn			= DFALSE;
	VEC_SET(m_vScale, 15.0f, 15.0f, 15.0f);
	m_pProjectileFilename	= "Models\\PV_Weapons\\Bullgut_Projectile.abc";
	m_pProjectileSkin		= "Skins\\Weapons\\Bullgut_Projectile_a.dtx";
}

BEGIN_CLASS(CGrenadeProjectile)
END_CLASS_DEFAULT_FLAGS(CGrenadeProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenadeProjectile::CGrenadeProjectile
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CGrenadeProjectile::CGrenadeProjectile() : CProjectile()
{
	m_dwFlags				|= FLAG_GRAVITY | FLAG_SHADOW;

	m_fExplosionDuration	= 1.0f;
	m_eDamageType			= DT_EXPLODE;
	m_pProjectileFilename	= "Sprites\\grenade1.spr";
}


BEGIN_CLASS(CKatoGrenadeProjectile)
END_CLASS_DEFAULT_FLAGS(CKatoGrenadeProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CKatoGrenadeProjectile::CKatoGrenadeProjectile
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CKatoGrenadeProjectile::CKatoGrenadeProjectile() : CGrenadeProjectile()
{
	m_dwFlags				&= ~FLAG_NOSLIDING;  // We wanna slide :)

	m_fExplosionDuration	= 1.0f;
	m_eDamageType			= DT_EXPLODE;
	m_pProjectileFilename	= "Sprites\\grenade2.spr";

	VEC_SET(m_vDims, 10.0f, 10.0f, 10.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CKatoGrenadeProjectile::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CKatoGrenadeProjectile::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			SetType(OT_SPRITE);
		}
		break;

		default : break;
	}

	return CProjectile::EngineMessageFn(messageID, pData, lData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CKatoGrenadeProjectile::HandleImpact
//
//	PURPOSE:	Handle hitting something
//
// ----------------------------------------------------------------------- //

void CKatoGrenadeProjectile::HandleImpact(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hObj) return;

	HCLASS hClassObj   = pServerDE->GetObjectClass(hObj);
	HCLASS hClassWorld = pServerDE->GetObjectClass(pServerDE->GetWorldObject());

	// Don't impact on the world...

	int nType = pServerDE->GetObjectType(hObj);
	if (pServerDE->IsKindOf(hClassObj, hClassWorld) || nType == OT_WORLDMODEL)
	{
		DoBounce();
		return;
	}
	
	// Do normal impact...

	CProjectile::HandleImpact(hObj);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CKatoGrenadeProjectile::DoBounce
//
//	PURPOSE:	Handle bouncing off world
//
// ----------------------------------------------------------------------- //

void CKatoGrenadeProjectile::DoBounce()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;
	DVector vPos;

	CollisionInfo info;
	pServerDE->GetLastCollision(&info);


	DVector vVel;
	pServerDE->GetVelocity(m_hObject, &vVel);

	//DFLOAT fDot = VEC_DOT(vVel, info.m_Plane.m_Normal);
	//VEC_ADDSCALED(vVel, vVel, info.m_Plane.m_Normal, -fDot);
	VEC_ADD( vVel, vVel, info.m_vStopVel );
	pServerDE->SetVelocity(m_hObject, &vVel);

	// Play a bounce sound...

	SurfaceType eType = GetSurfaceType(info.m_hPoly);
	char* pSound = GetBounceSound(eType, GUN_KATOGRENADE_ID);

	if (pSound)
	{
		pServerDE->GetObjectPos( m_hObject, &vPos );
		PlaySoundFromPos( &vPos, pSound, 1000.0f, SOUNDPRIORITY_MISC_MEDIUM );
	}
}


BEGIN_CLASS(CStickyGrenadeProjectile)
END_CLASS_DEFAULT_FLAGS(CStickyGrenadeProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CStickyGrenadeProjectile::CStickyGrenadeProjectile
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CStickyGrenadeProjectile::CStickyGrenadeProjectile() : CProjectile()
{
	m_dwFlags				|= FLAG_SHADOW;

	m_fExplosionDuration	= 1.0f;
	m_eDamageType			= DT_EXPLODE;

	m_pProjectileFilename	= "Models\\PV_Weapons\\SpiderProjectile.abc";
	m_pProjectileSkin		= "Skins\\Weapons\\SpiderProjectile_a.dtx";

	m_bAttached				= DFALSE;
	m_pThudSound			= "Sounds\\Weapons\\Spider\\Thud.wav";
	m_hHostObj				= DNULL;

	VEC_SET(m_vDims, 5.0f, 5.0f, 3.0f);
	VEC_SET(m_vScale, 3.0f, 3.0f, 1.0f);

	m_dwFireAni				= INVALID_ANI;
	m_dwFlightAni			= INVALID_ANI;
	m_dwImpactAni			= INVALID_ANI;

	VEC_INIT(m_vPosOffset);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CStickyGrenadeProjectile::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CStickyGrenadeProjectile::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			SetType(OT_MODEL);
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData == INITIALUPDATE_SAVEGAME) break;

			CServerDE* pServerDE = BaseClass::GetServerDE();
			if (!pServerDE) break;

			// Set up anis...

			m_dwFireAni		= pServerDE->GetAnimIndex(m_hObject, RP_FIRE_ANI_STR);
			m_dwFlightAni	= pServerDE->GetAnimIndex(m_hObject, RP_FLIGHT_ANI_STR);
			m_dwImpactAni	= pServerDE->GetAnimIndex(m_hObject, RP_IMPACT_ANI_STR);

			// Start the projecitle fire animation...

			if (m_dwFireAni != INVALID_ANI)
			{
				pServerDE->SetModelLooping(m_hObject, DFALSE);
				pServerDE->SetModelAnimation(m_hObject, m_dwFireAni);
			}
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_LINKBROKEN :
		{
			HOBJECT hLink = (HOBJECT)pData;
			if (hLink == m_hHostObj)
			{
				m_hHostObj = DNULL;
			}
		}
		break;

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

	return CProjectile::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CStickyGrenadeProjectile::HandleImpact
//
//	PURPOSE:	Handle hitting something
//
// ----------------------------------------------------------------------- //

void CStickyGrenadeProjectile::HandleImpact(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || m_bAttached) return;
	DVector vPos;

	m_bAttached = DTRUE;

	// Set the impact ani...

	if (m_dwImpactAni != INVALID_ANI)
	{
		pServerDE->SetModelLooping(m_hObject, DTRUE);
		pServerDE->SetModelAnimation(m_hObject, m_dwImpactAni);
	}

	
	// If we are attached to a character, hide the grenade...

	if (hObj)
	{
		HCLASS hClassObj   = pServerDE->GetObjectClass(hObj);
		HCLASS hClassWorld = pServerDE->GetObjectClass(pServerDE->GetWorldObject());

		DVector vMyPos, vHostPos;
		VEC_INIT(vHostPos);

		if (pServerDE->IsKindOf(hClassObj, hClassWorld))
		{
			// Align with surface normal...
			
			CollisionInfo info;
			pServerDE->GetLastCollision(&info);

			DVector vNormal;
			VEC_NEGATE(vNormal, info.m_Plane.m_Normal);

			DRotation rRot;
			pServerDE->AlignRotation(&rRot, &vNormal, DNULL);
			pServerDE->SetObjectRotation(m_hObject, &rRot);
		}
		else 
		{
			// Keep track of the object we're attached to...
			
			m_hHostObj = hObj;
			pServerDE->CreateInterObjectLink(m_hObject, m_hHostObj);

			pServerDE->GetObjectPos(m_hHostObj, &vHostPos);
		}

		// Keep track of our position relative to our host (or the world)...

		pServerDE->GetObjectPos(m_hObject, &vMyPos);
		VEC_SUB(m_vPosOffset, vMyPos, vHostPos);
	}


	// Play thud sound...

	if (m_pThudSound)
	{
		pServerDE->GetObjectPos( m_hObject, &vPos );
		PlaySoundFromPos( &vPos, m_pThudSound, 1000.0f, SOUNDPRIORITY_MISC_MEDIUM);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CStickyGrenadeProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

void CStickyGrenadeProjectile::Update()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;


	if (m_bAttached)
	{
		DVector vZero;
		VEC_INIT(vZero);

		pServerDE->SetVelocity(m_hObject, &vZero);
		pServerDE->SetAcceleration(m_hObject, &vZero);

		// Make sure we stay connected to our host...

		DVector vPos;
		VEC_INIT(vPos);

		if (m_hHostObj)
		{
			pServerDE->GetObjectPos(m_hHostObj, &vPos);
		}

		VEC_ADD(vPos, vPos, m_vPosOffset);
		pServerDE->SetObjectPos(m_hObject, &vPos);
	}
	else
	{
		// See if we need to update the ani...

		DDWORD dwState  = pServerDE->GetModelPlaybackState(m_hObject);
		DDWORD dwAni	= pServerDE->GetModelAnimation(m_hObject);

		if (m_dwFlightAni && dwAni != m_dwFlightAni)
		{
			if (dwState & MS_PLAYDONE)
			{
				pServerDE->SetModelLooping(m_hObject, DTRUE);
				pServerDE->SetModelAnimation(m_hObject, m_dwFlightAni);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CStickyGrenadeProjectile::RemoveObject()
//
//	PURPOSE:	Remove the object, and do clean up (isle 4)
//
// ----------------------------------------------------------------------- //

void CStickyGrenadeProjectile::RemoveObject()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	CProjectile::RemoveObject();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CStickyGrenadeProjectile::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CStickyGrenadeProjectile::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hHostObj);
	pServerDE->WriteToMessageVector(hWrite, &m_vPosOffset);
	pServerDE->WriteToMessageByte(hWrite, m_bAttached);
	pServerDE->WriteToMessageDWord(hWrite, m_dwFireAni);
	pServerDE->WriteToMessageDWord(hWrite, m_dwFlightAni);
	pServerDE->WriteToMessageDWord(hWrite, m_dwImpactAni);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CStickyGrenadeProjectile::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CStickyGrenadeProjectile::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hHostObj);
	pServerDE->ReadFromMessageVector(hRead, &m_vPosOffset);
	m_bAttached		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_dwFireAni		= pServerDE->ReadFromMessageDWord(hRead);
	m_dwFlightAni	= pServerDE->ReadFromMessageDWord(hRead);
	m_dwImpactAni	= pServerDE->ReadFromMessageDWord(hRead);
}