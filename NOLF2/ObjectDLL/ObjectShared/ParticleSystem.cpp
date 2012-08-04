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
#include "ParsedMsg.h"
#include "CommandMgr.h"

LINKFROM_MODULE( ParticleSystem );

#pragma force_active on
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
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( ParticleSystem )

	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )
	CMDMGR_ADD_MSG( REMOVE, 1, NULL, "REMOVE" )

CMDMGR_END_REGISTER_CLASS( ParticleSystem, CClientSFX )

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

	return CClientSFX::EngineMessageFn(messageID, pData, fData);
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
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, m_bOn ? USRFLG_VISIBLE : 0, FLAGMASK_ALL);

	// Tell the clients about the ParticleSystem, and remove thyself...

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_PARTICLESYSTEM_ID);
    cMsg.WriteLTVector(m_vColor1);
    cMsg.WriteLTVector(m_vColor2);
    cMsg.WriteLTVector(m_vDims);
    cMsg.WriteLTVector(m_vMinVel);
    cMsg.WriteLTVector(m_vMaxVel);
    cMsg.Writeuint32(m_dwFlags);
    cMsg.Writefloat(m_fBurstWait);
    cMsg.Writefloat(m_fBurstWaitMin);
    cMsg.Writefloat(m_fBurstWaitMax);
    cMsg.Writefloat(m_fParticlesPerSecond);
    cMsg.Writefloat(m_fParticleLifetime);
    cMsg.Writefloat(m_fRadius);
    cMsg.Writefloat(m_fGravity);
    cMsg.Writefloat(m_fRotationVelocity);
    cMsg.Writefloat(m_fViewDist);
	cMsg.WriteHString(m_hstrTextureName);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ParticleSystem::OnTrigger()
//
//	PURPOSE:	Handle trigger messages
//
// --------------------------------------------------------------------------- //

bool ParticleSystem::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");
	static CParsedMsg::CToken s_cTok_Remove("REMOVE");

	if ((cMsg.GetArg(0) == s_cTok_On) && !m_bOn)
	{
        g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_VISIBLE, FLAGMASK_ALL);
        m_bOn = LTTRUE;
	}
	else if ((cMsg.GetArg(0) == s_cTok_Off) && m_bOn)
	{
        g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, FLAGMASK_ALL);
        m_bOn = LTFALSE;
	}
	else if (cMsg.GetArg(0) == s_cTok_Remove)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
	else
		return CClientSFX::OnTrigger(hSender, cMsg);

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ParticleSystem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ParticleSystem::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
    if (!pMsg) return;

	SAVE_BOOL(m_bOn);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ParticleSystem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ParticleSystem::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
    if (!pMsg) return;

	LOAD_BOOL(m_bOn);
}


