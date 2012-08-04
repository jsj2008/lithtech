// ----------------------------------------------------------------------- //
//
// MODULE  : BodyProp.cpp
//
// PURPOSE : Model BodyProp - Definition
//
// CREATED : 6/2/98
//
// ----------------------------------------------------------------------- //

#include "BodyProp.h"
#include "cpp_server_de.h"
#include "WeaponFXTypes.h"
#include "ClientServerShared.h"
#include "SFXMsgIds.h"
#include "ClientDeathSFX.h"
#include "RiotServerShell.h"
#include <stdio.h>

extern CRiotServerShell* g_pRiotServerShellDE;

BEGIN_CLASS(BodyProp)
	ADD_LONGINTPROP(DeathType, 0)
END_CLASS_DEFAULT(BodyProp, Prop, NULL, NULL)

#define UPDATE_DELTA				0.1f
#define VAPORIZE_SKIN				"SpriteTextures\\sprite(2)\\ex51127b.dtx"
#define FREEZE_TIME					5.0f
#define VAPORIZE_TIME				2.0f
#define HITPTS_ADJUST_FACTOR		0.1f
//#define MULTIPLAYER_BODY_LIFETIME	120.0f	// Two minutes

static CVarTrack g_BodyLifetimeTrack;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::BodyProp()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

BodyProp::BodyProp() : Prop()
{
	VEC_INIT(m_vColor);
	VEC_SET(m_vDeathDir, 0.0f, -1.0f, 0.0f);

	m_eDeathType	= CD_NORMAL;
	m_bFirstUpdate	= DTRUE;
	m_fStartTime	= 0.0f;
	m_fAdjustFactor = HITPTS_ADJUST_FACTOR;
	m_eDamageType	= DT_UNSPECIFIED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD BodyProp::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return 0;

	switch(messageID)
	{
		case MID_UPDATE:
		{
			// DON'T call Prop::EngineMessageFn, object might get removed...
			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			Update();

			return dwRet;
		}
		break;

		case MID_PRECREATE:
		{
			if (fData == 1.0f || fData == 2.0f)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			DDWORD dwRet = Prop::EngineMessageFn(messageID, pData, fData);
			
			pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);

			DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hObject);
			pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags | USRFLG_NIGHT_INFRARED);
			
			if(!g_BodyLifetimeTrack.IsInitted())
			{
				g_BodyLifetimeTrack.Init(pServerDE, "BodyLifetime", NULL, 10.0f);
			}

			return dwRet;
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

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::PropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void BodyProp::ReadProp(ObjectCreateStruct *)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	long nLong;
	pServerDE->GetPropLongInt("DeathType", &nLong);
	m_eDeathType = (CharacterDeath)nLong;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::Setup()
//
//	PURPOSE:	Setup the object
//
// ----------------------------------------------------------------------- //

void BodyProp::Setup(CBaseCharacter* pChar)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pChar || !pChar->m_hObject) return;

	m_eSurfaceType = pChar->IsMecha() ? ST_MECHA : ST_FLESH;
	m_eDeathType   = pChar->GetDeathType();
	m_nModelId     = pChar->GetModelId();
	m_eSize		   = pChar->GetModelSize();
	m_cc		   = pChar->GetCharacterClass();

	m_Damage.SetCanDamage(pChar->CanDamageBody());
	m_Damage.SetApplyDamagePhysics(pChar->CanDamageBody());

	switch (GetModelType(m_nModelId, m_eSize))
	{
		case MT_MECHA:
		{
			m_fAdjustFactor /= 5.0f;
		}
		break;

		case MT_VEHICLE:
		{
			m_eDeathType = CD_GIB;
			m_fAdjustFactor /= 2.0f;
		}
		break;
	}

	m_fLife	= -1.0f;

	CDestructable* pDest = pChar->GetDestructable();
	if (pDest)
	{
		m_eDamageType = pDest->GetDeathType();

		VEC_COPY(m_vDeathDir, pDest->GetDeathDir());
		VEC_NORM(m_vDeathDir);
		VEC_MULSCALAR(m_vDeathDir, m_vDeathDir, 1.0f + (pDest->GetDeathDamage() / pDest->GetMaxHitPoints()));
	}

	DFLOAT fHitPts = pDest->GetMaxHitPoints() * m_fAdjustFactor;
	m_Damage.Reset(fHitPts, 0.0f);
	m_Damage.SetHitPoints(fHitPts);
	m_Damage.SetMaxHitPoints(fHitPts);
	m_Damage.SetArmorPoints(0.0f);
	m_Damage.SetMaxArmorPoints(0.0f);

	DDWORD dwFlags = pServerDE->GetObjectFlags(pChar->m_hObject) & ~FLAG_SOLID & 
		~FLAG_ANIMTRANSITION & ~FLAG_GOTHRUWORLD;
	dwFlags |= FLAG_GRAVITY; // Make sure we've got gravity.

	DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hObject) | USRFLG_NIGHT_INFRARED;

	// Set our surface type...

	dwUsrFlags |= SurfaceToUserFlag(m_eSurfaceType);


	// Make sure model doesn't slide all over the place...

	pServerDE->SetFrictionCoefficient(m_hObject, 500.0f);

	DVector vDims;
	pServerDE->GetObjectDims(pChar->m_hObject, &vDims);

	// Set the dims.  If we can't set the dims that big, set them
	// as big as possible...

	if (pServerDE->SetObjectDims2(m_hObject, &vDims) == DE_ERROR)
	{
		pServerDE->SetObjectDims2(m_hObject, &vDims);
	}

	DFLOAT r, g, b, a;
	pServerDE->GetObjectColor(pChar->m_hObject, &r, &g, &b, &a);
	pServerDE->SetObjectColor(m_hObject, r, g, b, a);

	DVector vScale;
	pServerDE->GetObjectScale(pChar->m_hObject, &vScale);
	pServerDE->ScaleObject(m_hObject, &vScale);

	if (m_eDeathType == CD_FREEZE)
	{
		dwUsrFlags |= USRFLG_MODELADD;
		dwFlags	   |= FLAG_SOLID;
	}
	else if (m_eDeathType == CD_GIB)
	{
		CreateGibs();

		pServerDE->RemoveObject(m_hObject);
		return;
	}
	else if (m_eDeathType == CD_VAPORIZE)
	{
		m_Damage.SetCanDamage(DFALSE);			// Will fade away, don't gib...
		m_Damage.SetApplyDamagePhysics(DFALSE);	// Don't move either...

		m_fLife	    = 60.0f;
		dwUsrFlags |= USRFLG_MODELADD;

		// Pack our model add color into the upper 3 bytes of the user data...

		VEC_SET(m_vColor, 255.0f, 255.0f, 255.0f);

		DBYTE nR = (DBYTE)m_vColor.x;
		DBYTE nG = (DBYTE)m_vColor.y;
		DBYTE nB = (DBYTE)m_vColor.z;

		dwUsrFlags = ((nR<<24) | (nG<<16) | (nB<<8) | (dwUsrFlags & 0x000F));

		// Set the model skin to the vaporize skin...

		char pFilename[MAX_CS_FILENAME_LEN+1];
		char pSkin[MAX_CS_FILENAME_LEN+1];
		pServerDE->GetModelFilenames(m_hObject, pFilename, MAX_CS_FILENAME_LEN, pSkin, MAX_CS_FILENAME_LEN);
		pServerDE->SetModelFilenames(m_hObject, pFilename, VAPORIZE_SKIN);
	}

	HMODELANIM hAni = pServerDE->GetModelAnimation(pChar->m_hObject);
	pServerDE->SetModelAnimation(m_hObject, hAni);
	pServerDE->SetModelLooping(m_hObject, DFALSE);

	pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);
	pServerDE->SetObjectFlags(m_hObject, dwFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::SubClassSetup()
//
//	PURPOSE:	Do setup
//
// ----------------------------------------------------------------------- //

void BodyProp::SubClassSetup()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

void BodyProp::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	DFLOAT fTime = pServerDE->GetTime();
	
	if (m_bFirstUpdate)
	{
		m_fStartTime   = fTime;
		m_bFirstUpdate = DFALSE;

		if (m_eDeathType == CD_GIB)
		{
			pServerDE->SetNextUpdate(m_hObject, 0.0f);
			return;
		}
	}

	DBOOL bKeepUpdating = DFALSE;

	switch (m_eDeathType)
	{
		case CD_NORMAL :
			bKeepUpdating = UpdateNormalDeath(fTime); 
		break;

		case CD_FREEZE :
			bKeepUpdating = UpdateFreezeDeath(fTime);
		break;

		case CD_VAPORIZE :
			bKeepUpdating = UpdateVaporizeDeath(fTime);
		break;

		default :
		break;
	}

	if (bKeepUpdating)
	{
		pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);
	}
	else
	{
		pServerDE->SetNextUpdate(m_hObject, 0.0f);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::UpdateNormalDeath()
//
//	PURPOSE:	Update normal death
//
// ----------------------------------------------------------------------- //

DBOOL BodyProp::UpdateNormalDeath(DFLOAT fTime)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || !g_pRiotServerShellDE) return DFALSE;

	if (g_pRiotServerShellDE->GetGameType() == SINGLE)
	{
		return DFALSE;  // Leave body around forever in single player...
	}
	else if (fTime >= m_fStartTime + g_BodyLifetimeTrack.GetFloat(10.0f))
	{
		pServerDE->RemoveObject(m_hObject);
		return DFALSE;
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::UpdateFreezeDeath()
//
//	PURPOSE:	Update freeze death
//
// ----------------------------------------------------------------------- //

DBOOL BodyProp::UpdateFreezeDeath(DFLOAT fTime)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;
	
	DBOOL bRet = DTRUE;

	if (fTime >= m_fStartTime + FREEZE_TIME)
	{
		m_vColor.z = 255.0f;
		bRet = DFALSE;
	}
	else
	{
		m_vColor.z = 255.0f * (fTime - m_fStartTime) / FREEZE_TIME;
		m_vColor.z = m_vColor.z > 255.0f ? 255.0f : m_vColor.z;
	}

	// Pack our color into the upper 3 bytes of the user data...

	DBYTE r = (DBYTE)m_vColor.x;
	DBYTE g = (DBYTE)m_vColor.y;
	DBYTE b = (DBYTE)m_vColor.z;

	DDWORD dwData = pServerDE->GetObjectUserFlags(m_hObject);
	dwData = ((r<<24) | (g<<16) | (b<<8) | (dwData & 0x000F));
	pServerDE->SetObjectUserFlags(m_hObject, dwData);


	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::UpdateVaporizeDeath()
//
//	PURPOSE:	Update vaporize death
//
// ----------------------------------------------------------------------- //

DBOOL BodyProp::UpdateVaporizeDeath(DFLOAT fTime)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;
	
	DBOOL bRet = DTRUE;

	DFLOAT r, g, b, a;
	pServerDE->GetObjectColor(m_hObject, &r, &g, &b, &a);

	if (fTime >= m_fStartTime + VAPORIZE_TIME)
	{
		a = 0.0f;
		
		// Since the model is still drawning (even though it is now
		// invisible...If it has a shadow leave it around for a while
		// (so it looks like the shadow was burned onto the ground)...

		if ( !(pServerDE->GetObjectFlags(m_hObject) & FLAG_SHADOW) )
		{
			pServerDE->RemoveObject(m_hObject);
			return DFALSE;

		}
		else if (fTime >= m_fStartTime + 30.0f)
		{
			pServerDE->RemoveObject(m_hObject);
			return DFALSE;
		}
	}
	else
	{
		a = 1.0f - (fTime - m_fStartTime) / VAPORIZE_TIME;
		a = a < 0.0f ? 0.0f : a;
	}

	pServerDE->SetObjectColor(m_hObject, r, g, b, a);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::CreateGibs()
//
//	PURPOSE:	Create the gibs props
//
// ----------------------------------------------------------------------- //

void BodyProp::CreateGibs()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

	CLIENTDEATHFX fxStruct;

	fxStruct.nModelId	= m_nModelId;
	fxStruct.nSize		= m_eSize;
	fxStruct.nDeathType	= m_eDeathType;
	fxStruct.nCharClass	= m_cc;

	VEC_COPY(fxStruct.vPos, vPos);
	VEC_COPY(fxStruct.vDeathDir, m_vDeathDir);

	CreateClientDeathFX(fxStruct);


	// If we are gibbing a MCA or Vehicle, create an explosion...

	DBOOL bCreateExplosion = DFALSE;

	switch (GetModelType(m_nModelId, m_eSize))
	{
		case MT_MECHA:
		{
			if (m_eDamageType != DT_MELEE)
			{
				bCreateExplosion = DTRUE;
				m_fDamageFactor  = 1.0f;
			}
		}
		break;

		case MT_VEHICLE:
		{
			bCreateExplosion = DTRUE;
			m_fDamageFactor  = 0.25f;
		}
		break;

		default : break;
	}

	if (bCreateExplosion)
	{
		m_bFireAlongForward  = DFALSE;
		m_nExplosionWeaponId = GUN_BULLGUT_ID;
		m_bCreateExplosion   = DTRUE;
		DoExplosion();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::CreateGibs()
//
//	PURPOSE:	See if the prop is dead, if so...gib it.
//
// ----------------------------------------------------------------------- //

void BodyProp::Damage( )
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	VEC_COPY(m_vDeathDir, m_Damage.GetDeathDir());
	VEC_NORM(m_vDeathDir);
	VEC_MULSCALAR(m_vDeathDir, m_vDeathDir, 1.0f + (m_Damage.GetDeathDamage() * m_fAdjustFactor / m_Damage.GetMaxHitPoints()));

	CreateGibs();
	pServerDE->RemoveObject(m_hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void BodyProp::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageFloat(hWrite, m_fAdjustFactor);
	pServerDE->WriteToMessageFloat(hWrite, m_fStartTime);
	pServerDE->WriteToMessageByte(hWrite, m_nModelId);
	pServerDE->WriteToMessageByte(hWrite, m_bFirstUpdate);
	pServerDE->WriteToMessageByte(hWrite, m_eDeathType);
	pServerDE->WriteToMessageByte(hWrite, m_eSize);
	pServerDE->WriteToMessageByte(hWrite, m_cc);
	pServerDE->WriteToMessageVector(hWrite, &m_vColor);
	pServerDE->WriteToMessageVector(hWrite, &m_vDeathDir);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BodyProp::Load
//
//	PURPOSE:	BodyProp the object
//
// ----------------------------------------------------------------------- //

void BodyProp::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_fAdjustFactor = pServerDE->ReadFromMessageFloat(hRead);
	m_fStartTime	= pServerDE->ReadFromMessageFloat(hRead);
	m_nModelId		= pServerDE->ReadFromMessageByte(hRead);
	m_bFirstUpdate	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_eDeathType	= (CharacterDeath) pServerDE->ReadFromMessageByte(hRead);
	m_eSize			= (ModelSize) pServerDE->ReadFromMessageByte(hRead);
	m_cc			= (CharacterClass) pServerDE->ReadFromMessageByte(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vColor);
	pServerDE->ReadFromMessageVector(hRead, &m_vDeathDir);
}