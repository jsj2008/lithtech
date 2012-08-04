// ----------------------------------------------------------------------- //
//
// MODULE  : Explosion.cpp
//
// PURPOSE : Explosion - Definition
//
// CREATED : 11/25/97
//
// ----------------------------------------------------------------------- //

#include "Explosion.h"
#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "RiotObjectUtilities.h"
#include "WeaponFXTypes.h"

#define EXPLOSION_UPDATE_DELTA			0.01f
#define EXPLOSION_DAMAGE_UPDATE_DELTA	0.1f

BEGIN_CLASS(Explosion)
	ADD_REALPROP(Duration, 1.5f)
	ADD_REALPROP_FLAG(DamageRadius, 200.0f, PF_RADIUS)
	ADD_REALPROP(MaxDamage, 50.0f)
END_CLASS_DEFAULT(Explosion, BaseClass, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Explosion()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Explosion::Explosion() : BaseClass()
{
	m_fDamageRadius			= 200.0f;
	m_fMaxDamage			= 50.0f;
	m_fDuration				= 1.5f;
	m_fLastDamageTime		= 0.0f;
	m_fStartTime			= 0.0f;
	m_fDamageScaleUDur		= m_fDuration * .5f;
	m_fDamageScaleDDur		= m_fDuration * .5f;

	m_eDamageType			= DT_UNSPECIFIED;
	m_hFiredFrom			= DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Setup()
//
//	PURPOSE:	Setup the Explosion data members (for non DEdit created
//				explosions)
//
// ----------------------------------------------------------------------- //

void Explosion::Setup(DFLOAT fDuration, DFLOAT fDamageRadius, DFLOAT fMaxDamage) 
{
	if (!g_pServerDE) return;

	m_fDamageRadius	= fDamageRadius;
	m_fMaxDamage	= fMaxDamage;
	m_fDuration		= fDuration;

	m_fDamageScaleUDur = m_fDuration * .5f;
	m_fDamageScaleDDur = m_fDuration * .5f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Setup()
//
//	PURPOSE:	Setup the Explosion data members (for non DEdit created
//				explosions)
//
// ----------------------------------------------------------------------- //

void Explosion::Setup(CProjectile* pProjectile)
{
	if (!g_pServerDE || !pProjectile) return;

	m_fDamageRadius	= pProjectile->GetExplosionRadius();
	m_fMaxDamage	= pProjectile->GetExplosionDamage();
	m_fDuration		= pProjectile->GetExplosionDuration();
	m_eDamageType	= pProjectile->GetDamageType();
	m_hFiredFrom	= pProjectile->GetFiredFrom();

	g_pServerDE->CreateInterObjectLink(m_hObject, m_hFiredFrom);

	m_fDamageScaleUDur = m_fDuration * .5f;
	m_fDamageScaleDDur = m_fDuration * .5f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD Explosion::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HandleTrigger(hSender, hRead);
			break;
		}
		default : break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Explosion::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_INITIALUPDATE:
		{
			if ((int)fData != INITIALUPDATE_SAVEGAME)
			{
				m_fStartTime = g_pServerDE->GetTime();
			}
			break;
		}

		case MID_PRECREATE:
		{
			ReadProp((ObjectCreateStruct*)pData);
			break;
		}

		case MID_LINKBROKEN :
		{
			HOBJECT hLink = (HOBJECT)pData;
			if (hLink)
			{
				if (hLink == m_hFiredFrom)
				{
					m_hFiredFrom = DNULL;
				}
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

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::ReadProp()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void Explosion::ReadProp(ObjectCreateStruct *pData)
{
	g_pServerDE->GetPropReal("DamageRadius", &m_fDamageRadius);
	g_pServerDE->GetPropReal("MaxDamage", &m_fMaxDamage);
	g_pServerDE->GetPropReal("Duration", &m_fDuration);

	m_fDamageScaleUDur	= m_fDuration * .5f;
	m_fDamageScaleDDur	= m_fDuration * .5f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::HandleTrigger
//
//	PURPOSE:	Handle trigger message.
// 
// ----------------------------------------------------------------------- //

void Explosion::HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead)
{
	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
	if (!hMsg) return;

	char* pMsg = g_pServerDE->GetStringData(hMsg);
	if (!pMsg) return;

	// See if we should make big boom...

	if (_stricmp(pMsg, "TRIGGER") == 0)
	{
		GoBoom();
	}

	g_pServerDE->FreeString(hMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::GoBoom
//
//	PURPOSE:	Do the big bang thang
// 
// ----------------------------------------------------------------------- //

void Explosion::GoBoom()
{
	if (!g_pServerDE) return;
	g_pServerDE->SetNextUpdate(m_hObject, EXPLOSION_UPDATE_DELTA);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Update()
//
//	PURPOSE:	Update the explosion
//
// ----------------------------------------------------------------------- //

void Explosion::Update()
{
	if (!g_pServerDE) return;

	g_pServerDE->SetNextUpdate(m_hObject, 0.01f);

	DFLOAT fTime = g_pServerDE->GetTime();

	if (fTime > m_fStartTime + m_fDuration)
	{
		g_pServerDE->RemoveObject(m_hObject);
		return;
	}

	UpdateDamage();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::UpdateDamage()
//
//	PURPOSE:	Update damage
//
// ----------------------------------------------------------------------- //

void Explosion::UpdateDamage()
{
	if (m_fDamageRadius <= 0.0f) return;

	DFLOAT fTime = g_pServerDE->GetTime();
	DFLOAT fDeltaTime = fTime - m_fStartTime;
	DFLOAT fDamageDeltaTime = fTime - m_fLastDamageTime;

	if (fDamageDeltaTime >= EXPLOSION_DAMAGE_UPDATE_DELTA)
	{
		m_fLastDamageTime = fTime;
		DFLOAT fMinRadius = m_fDamageRadius*.25f;
		DFLOAT fRange	  = m_fDamageRadius - fMinRadius;
		DFLOAT fRadius	  = 0.0f;
		DFLOAT fDamage	  = m_fMaxDamage - (m_fMaxDamage/m_fDuration)*fDeltaTime;

		if (fDeltaTime <= m_fDamageScaleUDur)
		{
			fRadius = fMinRadius + (fDeltaTime * fRange / m_fDamageScaleUDur);
		}
		else
		{	
			DFLOAT fNewDeltaTime = fDeltaTime - m_fDamageScaleUDur;
			fRadius = m_fDamageRadius - (fNewDeltaTime * fRange / m_fDamageScaleDDur);
		}

		DVector vPos;
		g_pServerDE->GetObjectPos(m_hObject, &vPos);

		HOBJECT hObj = m_hFiredFrom ? m_hFiredFrom : m_hObject;
		DamageObjectsInRadius(hObj, this, vPos, fRadius, fDamage, m_eDamageType);	
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Explosion::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hFiredFrom);

	pServerDE->WriteToMessageFloat(hWrite, m_fDamageRadius);
	pServerDE->WriteToMessageFloat(hWrite, m_fMaxDamage);
	pServerDE->WriteToMessageFloat(hWrite, m_fDuration);
	pServerDE->WriteToMessageFloat(hWrite, m_fDamageScaleUDur);
	pServerDE->WriteToMessageFloat(hWrite, m_fDamageScaleDDur);
	pServerDE->WriteToMessageFloat(hWrite, m_fLastDamageTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fStartTime);
	pServerDE->WriteToMessageByte(hWrite, m_bFirstUpdate);
	pServerDE->WriteToMessageByte(hWrite, m_eDamageType);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Explosion::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hFiredFrom);

	m_fDamageRadius		= pServerDE->ReadFromMessageFloat(hRead);
	m_fMaxDamage		= pServerDE->ReadFromMessageFloat(hRead);
	m_fDuration			= pServerDE->ReadFromMessageFloat(hRead);
	m_fDamageScaleUDur	= pServerDE->ReadFromMessageFloat(hRead);
	m_fDamageScaleDDur	= pServerDE->ReadFromMessageFloat(hRead);
	m_fLastDamageTime	= pServerDE->ReadFromMessageFloat(hRead);
	m_fStartTime		= pServerDE->ReadFromMessageFloat(hRead);
	m_bFirstUpdate		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_eDamageType		= (DamageType) pServerDE->ReadFromMessageByte(hRead);
}