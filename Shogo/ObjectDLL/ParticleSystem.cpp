// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleSystem.cpp
//
// PURPOSE : ParticleSystem - Implementation
//
// CREATED : 10/23/97
//
// ----------------------------------------------------------------------- //

#include "ParticleSystem.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"
#include "ClientServerShared.h"
#include "generic_msg_de.h"

BEGIN_CLASS(ParticleSystem)
	ADD_BOOLPROP(StartOn, DTRUE)
	ADD_LONGINTPROP(ParticleFlags, 0)
	ADD_REALPROP(BurstWait, 0.0f)
	ADD_REALPROP(ParticlesPerSecond, 0.0f)
	ADD_REALPROP_FLAG(EmissionRadius, 0.0f,PF_RADIUS)
	ADD_REALPROP(MinimumVelocity, 0.0f)
	ADD_REALPROP(MaximumVelocity, 0.0f)
	ADD_REALPROP(VelocityOffset, 0.0f)
	ADD_REALPROP(ParticleLifetime, 5.0f)
	ADD_REALPROP(ParticleRadius, 1000.0f)
	ADD_REALPROP(ParticleGravity, -500.0f)
	ADD_REALPROP(RotationVelocity, 0.0f)
	ADD_STRINGPROP(TextureName, "SpecialFX\\ParticleTextures\\particle.dtx")
	ADD_COLORPROP(Color1, 255.0f, 255.0f, 255.0f) 
	ADD_COLORPROP(Color2, 255.0f, 0.0f, 0.0f) 
END_CLASS_DEFAULT_FLAGS(ParticleSystem, CClientSFX, NULL, NULL, CF_ALWAYSLOAD)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ParticleSystem::ParticleSystem
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

ParticleSystem::ParticleSystem() : CClientSFX()
{ 
	m_dwFlags				= 0;
	m_fBurstWait			= 0.0f;
	m_fParticlesPerSecond	= 0.0f;
	m_fEmissionRadius		= 0.0f;
	m_fMinimumVelocity		= 0.0f;
	m_fMaximumVelocity		= 0.0f;
	m_fVelocityOffset		= 0.0f;
	m_fParticleLifetime		= 0.0f;
	m_fRadius				= 1000.0f;
	m_fGravity				= -500.0f;
	m_fRotationVelocity		= 0.0f;
	m_hstrTextureName		= DNULL;
	m_bOn					= DTRUE;

	VEC_INIT(m_vColor1);
	VEC_INIT(m_vColor2);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ParticleSystem::ParticleSystem
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

ParticleSystem::~ParticleSystem()
{ 
	if (m_hstrTextureName)
	{
		g_pServerDE->FreeString(m_hstrTextureName);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ParticleSystem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD ParticleSystem::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct *)pData;
			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp(pStruct);
			}
			
			pStruct->m_Flags = FLAG_FULLPOSITIONRES | FLAG_FORCECLIENTUPDATE;

			break;
		}

		case MID_INITIALUPDATE:
		{
			InitialUpdate((int)fData);
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

	return CClientSFX::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ParticleSystem::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

DDWORD ParticleSystem::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);
			HandleMsg(hSender, hMsg);
			pServerDE->FreeString(hMsg);
		}
		break;

		default : break;
	}
	
	return CClientSFX::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ParticleSystem::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL ParticleSystem::ReadProp(ObjectCreateStruct *)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	GenericProp genProp;

	if (g_pServerDE->GetPropGeneric("StartOn", &genProp) == DE_OK)
		m_bOn = genProp.m_Bool;

	if (g_pServerDE->GetPropGeneric("ParticleFlags", &genProp) == DE_OK)
		m_dwFlags = genProp.m_Long;

	if (g_pServerDE->GetPropGeneric("BurstWait", &genProp) == DE_OK)
		m_fBurstWait = genProp.m_Float;

	if (g_pServerDE->GetPropGeneric("ParticlesPerSecond", &genProp) == DE_OK)
		m_fParticlesPerSecond = genProp.m_Float;

	if (g_pServerDE->GetPropGeneric("EmissionRadius", &genProp) == DE_OK)
		m_fEmissionRadius = genProp.m_Float;

	if (g_pServerDE->GetPropGeneric("MinimumVelocity", &genProp) == DE_OK)
		m_fMinimumVelocity = genProp.m_Float;

	if (g_pServerDE->GetPropGeneric("MaximumVelocity", &genProp) == DE_OK)
		m_fMaximumVelocity = genProp.m_Float;

	if (g_pServerDE->GetPropGeneric("VelocityOffset", &genProp) == DE_OK)
		m_fVelocityOffset = genProp.m_Float;

	if (g_pServerDE->GetPropGeneric("ParticleLifetime", &genProp) == DE_OK)
		m_fParticleLifetime = genProp.m_Float;

	if (g_pServerDE->GetPropGeneric("Color1", &genProp) == DE_OK)
		VEC_COPY(m_vColor1, genProp.m_Color);

	if (g_pServerDE->GetPropGeneric("Color2", &genProp) == DE_OK)
		VEC_COPY(m_vColor2, genProp.m_Color);

	if (g_pServerDE->GetPropGeneric("ParticleRadius", &genProp) == DE_OK)
		m_fRadius = genProp.m_Float;

	if (g_pServerDE->GetPropGeneric("ParticleGravity", &genProp) == DE_OK)
		m_fGravity = genProp.m_Float;

	if (g_pServerDE->GetPropGeneric("RotationVelocity", &genProp) == DE_OK)
		m_fRotationVelocity = genProp.m_Float;

	if (g_pServerDE->GetPropGeneric("TextureName", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) m_hstrTextureName = pServerDE->CreateString(genProp.m_String);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ParticleSystem::InitialUpdate
//
//	PURPOSE:	Handle initial Update
//
// ----------------------------------------------------------------------- //

void ParticleSystem::InitialUpdate(int nInfo)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (nInfo == INITIALUPDATE_SAVEGAME) return;


	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

	DDWORD dwUserFlags = m_bOn ? USRFLG_VISIBLE : 0;
	pServerDE->SetObjectUserFlags(m_hObject, dwUserFlags);

	// Tell the clients about the ParticleSystem, and remove thyself...

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_PARTICLESYSTEM_ID);
	pServerDE->WriteToMessageVector(hMessage, &m_vColor1);
	pServerDE->WriteToMessageVector(hMessage, &m_vColor2);
	pServerDE->WriteToMessageFloat(hMessage, (DFLOAT)m_dwFlags);
	pServerDE->WriteToMessageFloat(hMessage, m_fBurstWait);
	pServerDE->WriteToMessageFloat(hMessage, m_fParticlesPerSecond);
	pServerDE->WriteToMessageFloat(hMessage, m_fEmissionRadius);
	pServerDE->WriteToMessageFloat(hMessage, m_fMinimumVelocity);
	pServerDE->WriteToMessageFloat(hMessage, m_fMaximumVelocity);
	pServerDE->WriteToMessageFloat(hMessage, m_fVelocityOffset);
	pServerDE->WriteToMessageFloat(hMessage, m_fParticleLifetime);
	pServerDE->WriteToMessageFloat(hMessage, m_fRadius);
	pServerDE->WriteToMessageFloat(hMessage, m_fGravity);
	pServerDE->WriteToMessageFloat(hMessage, m_fRotationVelocity);
	pServerDE->WriteToMessageHString(hMessage, m_hstrTextureName);
	pServerDE->EndMessage(hMessage);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ParticleSystem::HandleMsg()
//
//	PURPOSE:	Handle trigger messages
//
// --------------------------------------------------------------------------- //

void ParticleSystem::HandleMsg(HOBJECT hSender, HSTRING hMsg)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pMsg = pServerDE->GetStringData(hMsg);
	if (!pMsg) return;

	if (_stricmp(pMsg, "ON") == 0 && !m_bOn)
	{
		DDWORD dwUserFlags = USRFLG_VISIBLE;
		pServerDE->SetObjectUserFlags(m_hObject, dwUserFlags);
		m_bOn = DTRUE;
	}
	else if (_stricmp(pMsg, "OFF") == 0 && m_bOn)
	{
		DDWORD dwUserFlags = 0;
		pServerDE->SetObjectUserFlags(m_hObject, dwUserFlags);
		m_bOn = DFALSE;
	}
	else if (_stricmp(pMsg, "REMOVE") == 0)
	{
		pServerDE->RemoveObject(m_hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ParticleSystem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ParticleSystem::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageByte(hWrite, m_bOn);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ParticleSystem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ParticleSystem::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_bOn	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ParticleSystem::CacheFiles
//
//	PURPOSE:	Cache resources used by this object
//
// ----------------------------------------------------------------------- //

void ParticleSystem::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrTextureName)
	{
		char* pFile = pServerDE->GetStringData(m_hstrTextureName);
		if (pFile && pFile[0])
		{
			pServerDE->CacheFile(FT_TEXTURE, pFile);
		}
	}
}