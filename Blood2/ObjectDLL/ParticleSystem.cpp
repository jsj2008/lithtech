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


BEGIN_CLASS(ParticleSystem)
	ADD_BOOLPROP(Static, 1)
	ADD_LONGINTPROP(ParticleFlags, 0)
	ADD_REALPROP(BurstWait, 0.0f)
	ADD_REALPROP(ParticlesPerSecond, 0.0f)
	ADD_REALPROP_FLAG(EmissionRadius, 0.0f,PF_RADIUS)
	ADD_REALPROP(MinimumVelocity, 0.0f)
	ADD_REALPROP(MaximumVelocity, 0.0f)
	ADD_REALPROP(VelocityOffset, 0.0f)
	ADD_REALPROP(ParticleLifetime, 5.0f)
	ADD_REALPROP(ParticleRadius, 1000.0f)
	ADD_REALPROP(Gravity, -500.0f)
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
	m_bStatic				= DTRUE;

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
			if (fData == 1.0f || fData == 2.0f)
			{
				ReadProp((ObjectCreateStruct *)pData);
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			InitialUpdate((DVector *)pData);
			break;
		}

		case MID_UPDATE:
		{
			CServerDE* pServerDE = GetServerDE();
			if (pServerDE)
			{
				pServerDE->RemoveObject(m_hObject);
			}
			break;
		}

		default : break;
	}

	return CClientSFX::EngineMessageFn(messageID, pData, fData);
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

	if (g_pServerDE->GetPropGeneric("Static", &genProp) == DE_OK)
		m_bStatic = genProp.m_Bool;

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

	if (g_pServerDE->GetPropGeneric("Gravity", &genProp) == DE_OK)
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

void ParticleSystem::InitialUpdate(DVector *pMovement)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Tell the clients about the ParticleSystem, and remove thyself...

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
//	pServerDE->WriteToMessageByte(hMessage, SFX_PARTICLESYSTEM_ID | (( m_bStatic ) ? SFX_STATIC : 0 ));
	pServerDE->WriteToMessageByte(hMessage, SFX_PARTICLESYSTEM_ID);
//pServerDE->WriteToMessageByte(hMessage, m_bStatic);
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
