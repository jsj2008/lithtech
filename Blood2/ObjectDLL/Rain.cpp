// ----------------------------------------------------------------------- //
//
// MODULE  : Rain.cpp
//
// PURPOSE : Rain - Implementation
//
// CREATED : 10/23/97
//
// ----------------------------------------------------------------------- //

#include "Rain.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"
#include "generic_msg_de.h"
#include "ClientServerShared.h"

void BPrint(char*);

// Static global variables..
static char *g_szTriggerOn = "TRIGGER"; 
static char *g_szTriggerOff = "TRIGGEROFF";



BEGIN_CLASS(Rain)
	ADD_REALPROP(Density, 0.0f)
	ADD_VECTORPROP_FLAG(Dims, PF_DIMS)
	ADD_REALPROP(Lifetime, 1.0f)
	ADD_VECTORPROP(Direction)
	ADD_BOOLPROP(AddGravity, DTRUE)
	ADD_REALPROP(ParticleScale, 1.0f)
	ADD_REALPROP(Spread, 0.0f)
	ADD_BOOLPROP(Triggered, DFALSE)
	ADD_COLORPROP(Color1, 200.0f, 255.0f, 255.0f)
	ADD_COLORPROP(Color2, 40.0f, 50.0f, 50.0f)
	ADD_REALPROP(TimeLimit, 0.0f)
	ADD_REALPROP(Pulse, 0.0f)
END_CLASS_DEFAULT(Rain, CClientSFX, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rain::Rain
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

Rain::Rain() : CClientSFX()
{ 
	m_dwFlags  = 0;
	m_fDensity = 0.0f;
	VEC_INIT(m_vDims);
	m_fLifetime = 1.0f;
	m_bGravity = DTRUE;
	m_fParticleScale = 1.0f;
	m_bTriggered = DFALSE;
	VEC_INIT(m_vDirection);
	VEC_SET(m_vColor1, 200.0f, 255.0f, 255.0f);
	VEC_SET(m_vColor2, 40.0f, 50.0f, 50.0f);
	m_fTimeLimit = 0.0f;
	m_fPulse = 0.0f;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Rain::Setup
// DESCRIPTION	: 
// RETURN TYPE	: void 
// PARAMS		: DFLOAT fDensity
// PARAMS		: DVector vDims
// PARAMS		: DFLOAT fLifeTime
// PARAMS		: DVector vDir
// PARAMS		: DBOOL bGravity
// PARAMS		: DFLOAT fScale
// PARAMS		: DFLOAT fSpread
// PARAMS		: DVector vColor1
// PARAMS		: DVector vColor2
// ----------------------------------------------------------------------- //

void Rain::Setup(DFLOAT fDensity, DVector vDims, DFLOAT fLifeTime, DVector vDir, DBOOL bGravity, DFLOAT fScale, 
				 DFLOAT fSpread, DVector vColor1, DVector vColor2, DFLOAT fTimeLimit, DFLOAT fPulse)
{
	m_fDensity = fDensity;
	VEC_COPY(m_vDims,vDims);
	m_fLifetime = fLifeTime;
	m_bGravity = bGravity;
	m_fParticleScale = fScale;
	m_fSpread = fSpread;
	VEC_COPY(m_vDirection,vDir);
	VEC_COPY(m_vColor1,vColor1);
	VEC_COPY(m_vColor2,vColor2);
	m_fTimeLimit = fTimeLimit;
	m_fPulse = fPulse;

	SendEffectMessage();

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rain::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Rain::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
				ReadProp((ObjectCreateStruct *)pData);
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
				InitialUpdate((DVector *)pData);
			break;
		}

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;

		//SCHLEGZ 5/3/98 11:29:45 PM: if parent is gone, remove this
		case MID_PARENTATTACHMENTREMOVED:
			GetServerDE()->RemoveObject(m_hObject);
			break;

		default : break;
	}

	return CClientSFX::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Rain::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

DDWORD Rain::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	switch (messageID)
	{
		case MID_TRIGGER:
			HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);
			TriggerMsg(hSender, hMsg);
			pServerDE->FreeString(hMsg);
			break;
	}
	
	return CClientSFX::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rain::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL Rain::ReadProp(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return DFALSE;

	pServerDE->GetPropVector("Dims", &m_vDims);
	pServerDE->GetPropReal("Density", &m_fDensity);
	pServerDE->GetPropReal("Lifetime", &m_fLifetime);
	pServerDE->GetPropVector("Direction", &m_vDirection);
	pServerDE->GetPropBool("AddGravity", &m_bGravity);
	pServerDE->GetPropReal("ParticleScale", &m_fParticleScale);
	pServerDE->GetPropReal("Spread", &m_fSpread);
	pServerDE->GetPropBool("Triggered", &m_bTriggered);
	pServerDE->GetPropVector("Color1", &m_vColor1);
	pServerDE->GetPropVector("Color2", &m_vColor2);
	pServerDE->GetPropReal("TimeLimit", &m_fTimeLimit);
	pServerDE->GetPropReal("Pulse", &m_fPulse);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rain::InitialUpdate
//
//	PURPOSE:	Handle initial Update
//
// ----------------------------------------------------------------------- //

void Rain::InitialUpdate(DVector *pMovement)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

	// Mark this object as savable
	DDWORD dwFlags = pServerDE->GetObjectUserFlags(m_hObject);
	dwFlags |= USRFLG_SAVEABLE;
	pServerDE->SetObjectUserFlags(m_hObject, dwFlags);

	// Tell the clients about the Rain
	if (!m_bTriggered)
		SendEffectMessage();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rain::SendEffectMessage
//
//	PURPOSE:	Sends a message to the client to start a rain effect
//
// ----------------------------------------------------------------------- //

void Rain::SendEffectMessage()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_RAIN_ID);
/*
	if(m_fTimeLimit)	//SCHLEGZ 5/3/98 11:36:00 PM: remove object if the server object disappears (HACK!!)
		pServerDE->WriteToMessageByte(hMessage, DFALSE);
	else
//pServerDE->WriteToMessageByte(hMessage, DTRUE);
*/
	pServerDE->WriteToMessageFloat(hMessage, (DFLOAT)m_dwFlags);
	pServerDE->WriteToMessageFloat(hMessage, m_fDensity);
	pServerDE->WriteToMessageFloat(hMessage, m_fLifetime);
	pServerDE->WriteToMessageByte(hMessage, (DBYTE)m_bGravity);
	pServerDE->WriteToMessageFloat(hMessage, m_fParticleScale);
	pServerDE->WriteToMessageFloat(hMessage, m_fSpread);
	pServerDE->WriteToMessageVector(hMessage, &m_vDims);
	pServerDE->WriteToMessageVector(hMessage, &m_vDirection);
	pServerDE->WriteToMessageVector(hMessage, &m_vColor1);
	pServerDE->WriteToMessageVector(hMessage, &m_vColor2);
	pServerDE->WriteToMessageFloat(hMessage, m_fTimeLimit);
	pServerDE->WriteToMessageFloat(hMessage, m_fPulse);
	pServerDE->EndMessage(hMessage);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Rain::TriggerMsg()
//
//	PURPOSE:	Handler for rain trigger messages.
//
// --------------------------------------------------------------------------- //

void Rain::TriggerMsg(HOBJECT hSender, HSTRING hMsg)
{
	CServerDE* pServerDE = GetServerDE();
	HSTRING hstr;

	hstr = pServerDE->CreateString(g_szTriggerOn);
	if (pServerDE->CompareStringsUpper(hMsg, hstr))
		SendEffectMessage();
	pServerDE->FreeString(hstr);
}

	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rain::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Rain::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageDWord(hWrite, m_dwFlags);
	pServerDE->WriteToMessageFloat(hWrite, m_fDensity);
	pServerDE->WriteToMessageFloat(hWrite, m_fLifetime);
	pServerDE->WriteToMessageByte(hWrite, m_bGravity);
	pServerDE->WriteToMessageFloat(hWrite, m_fParticleScale);
	pServerDE->WriteToMessageFloat(hWrite, m_fSpread);
	pServerDE->WriteToMessageByte(hWrite, m_bTriggered);
	pServerDE->WriteToMessageVector(hWrite, &m_vDims);
	pServerDE->WriteToMessageVector(hWrite, &m_vDirection);
	pServerDE->WriteToMessageVector(hWrite, &m_vColor1);
	pServerDE->WriteToMessageVector(hWrite, &m_vColor2);
	pServerDE->WriteToMessageFloat(hWrite, m_fTimeLimit);
	pServerDE->WriteToMessageFloat(hWrite, m_fPulse);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rain::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Rain::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_dwFlags		= pServerDE->ReadFromMessageDWord(hRead);
	m_fDensity		= pServerDE->ReadFromMessageFloat(hRead);
	m_fLifetime		= pServerDE->ReadFromMessageFloat(hRead);
	m_bGravity		= pServerDE->ReadFromMessageByte(hRead);
	m_fParticleScale= pServerDE->ReadFromMessageFloat(hRead);
	m_fSpread		= pServerDE->ReadFromMessageFloat(hRead);
	m_bTriggered	= pServerDE->ReadFromMessageByte(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vDims);
	pServerDE->ReadFromMessageVector(hRead, &m_vDirection);
	pServerDE->ReadFromMessageVector(hRead, &m_vColor1);
	pServerDE->ReadFromMessageVector(hRead, &m_vColor2);
	m_fTimeLimit	= pServerDE->ReadFromMessageFloat(hRead);	
	m_fPulse		= pServerDE->ReadFromMessageFloat(hRead);	

	// Tell the clients about the Rain
	if (!m_bTriggered)
		SendEffectMessage();
}



