// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleSystem.cpp
//
// PURPOSE : ParticleSystem - Implementation
//
// CREATED : 10/23/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ParticleSystem.h"
#include "iltserver.h"
#include "SFXMsgIds.h"
#include "ClientServerShared.h"
#include "ObjectMsgs.h"

BEGIN_CLASS(ParticleSystem)
    ADD_BOOLPROP(StartOn, LTTRUE)
	ADD_LONGINTPROP(ParticleFlags, 0)
	ADD_VECTORPROP_VAL_FLAG(Dims, 50.0f, 50.0f, 50.0f, PF_DIMS)
	ADD_VECTORPROP_VAL(MinVelocity, 0.0f, 0.0f, 0.0f)
	ADD_VECTORPROP_VAL(MaxVelocity, 0.0f, 0.0f, 0.0f)
	ADD_REALPROP(BurstWait, 0.01f)
	ADD_REALPROP(BurstWaitMin, 0.01f)
	ADD_REALPROP(BurstWaitMax, 1.0f)
	ADD_REALPROP(ParticlesPerSecond, 0.0f)
	ADD_REALPROP(ParticleLifetime, 5.0f)
	ADD_REALPROP(ParticleRadius, 1000.0f)
	ADD_REALPROP(ParticleGravity, -500.0f)
	ADD_REALPROP(RotationVelocity, 0.0f)
	ADD_REALPROP(MaxViewDistance, 5000.0f)
	ADD_COLORPROP(Color1, 255.0f, 255.0f, 255.0f)
	ADD_COLORPROP(Color2, 255.0f, 0.0f, 0.0f)
	ADD_STRINGPROP(TextureName, "SFX\\Particle\\particle.dtx")
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
	m_fBurstWait			= 0.01f;
	m_fBurstWaitMin			= 0.01f;
	m_fBurstWaitMax			= 1.0f;
	m_fParticlesPerSecond	= 0.0f;
	m_fParticleLifetime		= 0.0f;
	m_fRadius				= 1000.0f;
	m_fGravity				= -500.0f;
	m_fViewDist				= 5000.0f;
	m_fRotationVelocity		= 0.0f;
    m_hstrTextureName       = LTNULL;
    m_bOn                   = LTTRUE;

	VEC_INIT(m_vColor1);
	VEC_INIT(m_vColor2);
	VEC_INIT(m_vMinVel);
	VEC_INIT(m_vMaxVel);
	m_vDims.x = m_vDims.y = m_vDims.z = 50.0f;
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
        g_pLTServer->FreeString(m_hstrTextureName);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ParticleSystem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 ParticleSystem::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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

	return CClientSFX::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ParticleSystem::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

uint32 ParticleSystem::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
    ILTServer* pLTServer = GetServerDE();
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			HandleMsg(hSender, szMsg);
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

LTBOOL ParticleSystem::ReadProp(ObjectCreateStruct *)
{
    ILTServer* pLTServer = GetServerDE();
    if (!pLTServer) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("StartOn", &genProp) == LT_OK)
		m_bOn = genProp.m_Bool;

    if (g_pLTServer->GetPropGeneric("ParticleFlags", &genProp) == LT_OK)
		m_dwFlags = genProp.m_Long;

    if (g_pLTServer->GetPropGeneric("BurstWait", &genProp) == LT_OK)
		m_fBurstWait = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("BurstWaitMin", &genProp) == LT_OK)
		m_fBurstWaitMin = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("BurstWaitMax", &genProp) == LT_OK)
		m_fBurstWaitMax = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("ParticlesPerSecond", &genProp) == LT_OK)
		m_fParticlesPerSecond = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("Dims", &genProp) == LT_OK)
		m_vDims = genProp.m_Vec;

    if (g_pLTServer->GetPropGeneric("MinVelocity", &genProp) == LT_OK)
		m_vMinVel = genProp.m_Vec;

    if (g_pLTServer->GetPropGeneric("MaxVelocity", &genProp) == LT_OK)
		m_vMaxVel = genProp.m_Vec;

    if (g_pLTServer->GetPropGeneric("ParticleLifetime", &genProp) == LT_OK)
		m_fParticleLifetime = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("Color1", &genProp) == LT_OK)
		VEC_COPY(m_vColor1, genProp.m_Color);

    if (g_pLTServer->GetPropGeneric("Color2", &genProp) == LT_OK)
		VEC_COPY(m_vColor2, genProp.m_Color);

    if (g_pLTServer->GetPropGeneric("ParticleRadius", &genProp) == LT_OK)
		m_fRadius = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("ParticleGravity", &genProp) == LT_OK)
		m_fGravity = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("RotationVelocity", &genProp) == LT_OK)
		m_fRotationVelocity = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("TextureName", &genProp) == LT_OK)
	{
        if (genProp.m_String[0]) m_hstrTextureName = pLTServer->CreateString(genProp.m_String);
	}

    g_pLTServer->GetPropReal("MaxViewDistance", &m_fViewDist);

    return LTTRUE;
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
    ILTServer* pLTServer = GetServerDE();
    if (!pLTServer) return;

	if (nInfo == INITIALUPDATE_SAVEGAME) return;


    LTVector vPos;
    pLTServer->GetObjectPos(m_hObject, &vPos);

    uint32 dwUserFlags = m_bOn ? USRFLG_VISIBLE : 0;
    pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);

	// Tell the clients about the ParticleSystem, and remove thyself...

    HMESSAGEWRITE hMessage = pLTServer->StartSpecialEffectMessage(this);
    pLTServer->WriteToMessageByte(hMessage, SFX_PARTICLESYSTEM_ID);
    pLTServer->WriteToMessageVector(hMessage, &m_vColor1);
    pLTServer->WriteToMessageVector(hMessage, &m_vColor2);
    pLTServer->WriteToMessageVector(hMessage, &m_vDims);
    pLTServer->WriteToMessageVector(hMessage, &m_vMinVel);
    pLTServer->WriteToMessageVector(hMessage, &m_vMaxVel);
    pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)m_dwFlags);
    pLTServer->WriteToMessageFloat(hMessage, m_fBurstWait);
    pLTServer->WriteToMessageFloat(hMessage, m_fBurstWaitMin);
    pLTServer->WriteToMessageFloat(hMessage, m_fBurstWaitMax);
    pLTServer->WriteToMessageFloat(hMessage, m_fParticlesPerSecond);
    pLTServer->WriteToMessageFloat(hMessage, m_fParticleLifetime);
    pLTServer->WriteToMessageFloat(hMessage, m_fRadius);
    pLTServer->WriteToMessageFloat(hMessage, m_fGravity);
    pLTServer->WriteToMessageFloat(hMessage, m_fRotationVelocity);
    pLTServer->WriteToMessageFloat(hMessage, m_fViewDist);
    pLTServer->WriteToMessageHString(hMessage, m_hstrTextureName);
    pLTServer->EndMessage(hMessage);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ParticleSystem::HandleMsg()
//
//	PURPOSE:	Handle trigger messages
//
// --------------------------------------------------------------------------- //

void ParticleSystem::HandleMsg(HOBJECT hSender, const char* szMsg)
{
	if (_stricmp(szMsg, "ON") == 0 && !m_bOn)
	{
        uint32 dwUserFlags = USRFLG_VISIBLE;
        g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);
        m_bOn = LTTRUE;
	}
	else if (_stricmp(szMsg, "OFF") == 0 && m_bOn)
	{
        uint32 dwUserFlags = 0;
        g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);
        m_bOn = LTFALSE;
	}
	else if (_stricmp(szMsg, "REMOVE") == 0)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ParticleSystem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ParticleSystem::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
    ILTServer* pLTServer = GetServerDE();
    if (!pLTServer || !hWrite) return;

    pLTServer->WriteToMessageByte(hWrite, m_bOn);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ParticleSystem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ParticleSystem::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
    ILTServer* pLTServer = GetServerDE();
    if (!pLTServer || !hRead) return;

    m_bOn   = (LTBOOL) pLTServer->ReadFromMessageByte(hRead);
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
    ILTServer* pLTServer = GetServerDE();
    if (!pLTServer) return;

	if (m_hstrTextureName)
	{
        char* pFile = pLTServer->GetStringData(m_hstrTextureName);
		if (pFile && pFile[0])
		{
            pLTServer->CacheFile(FT_TEXTURE, pFile);
		}
	}
}