// ----------------------------------------------------------------------- //
//
// MODULE  : Camera.cpp
//
// PURPOSE : Camera implementation
//
// CREATED : 5/20/98
//
// ----------------------------------------------------------------------- //

#include "Camera.h"
#include "cpp_server_de.h"
#include "ClientServerShared.h"
#include "generic_msg_de.h"
#include "SFXMsgIds.h"

BEGIN_CLASS(Camera)
	ADD_REALPROP(ActiveTime, -1.0f)
	ADD_BOOLPROP(AllowPlayerMovement, DFALSE)
	ADD_BOOLPROP(OneTime, DTRUE)
	ADD_LONGINTPROP(Type, CT_CINEMATIC)
	ADD_BOOLPROP(StartActive, DFALSE)
	ADD_BOOLPROP(IsListener, DFALSE)
END_CLASS_DEFAULT(Camera, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::Camera()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Camera::Camera() : BaseClass(OT_NORMAL)
{
	m_bAllowPlayerMovement	= DFALSE;
	m_fActiveTime			= 1.0f;
	m_bOneTime				= DTRUE;
	m_nCameraType			= CT_CINEMATIC;
	m_bStartActive			= DFALSE;
	m_fTurnOffTime			= 0.0f;
	m_bIsListener			= DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Camera::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct) 
			{
				pStruct->m_Flags |= FLAG_FORCECLIENTUPDATE | FLAG_FULLPOSITIONRES;
			}

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp(pStruct);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			InitialUpdate((int)fData);
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

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

DDWORD Camera::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return 0;

	switch (messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);
			TriggerMsg(hSender, hMsg);
			pServerDE->FreeString(hMsg);
			break;
		}
	}
	
	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL Camera::ReadProp(ObjectCreateStruct *pData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pData) return DFALSE;

	GenericProp genProp;
	if (pServerDE->GetPropGeneric("AllowPlayerMovement", &genProp) == DE_OK)
	{
		m_bAllowPlayerMovement = genProp.m_Bool;
	}

	if (pServerDE->GetPropGeneric("ActiveTime", &genProp) == DE_OK)
	{
		m_fActiveTime = genProp.m_Float;
	}

	if (pServerDE->GetPropGeneric("OneTime", &genProp) == DE_OK)
	{
		m_bOneTime = genProp.m_Bool;
	}

	if (pServerDE->GetPropGeneric("Type", &genProp) == DE_OK)
	{
		m_nCameraType = (DBYTE)genProp.m_Long;
	}

	if (pServerDE->GetPropGeneric("StartActive", &genProp) == DE_OK)
	{
		m_bStartActive = genProp.m_Bool;
	}

	if (pServerDE->GetPropGeneric("IsListener", &genProp) == DE_OK)
	{
		m_bIsListener = genProp.m_Bool;
	}


	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::InitialUpdate
//
//	PURPOSE:	Initialize the object
//
// ----------------------------------------------------------------------- //

void Camera::InitialUpdate(int nInfo)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (nInfo == INITIALUPDATE_SAVEGAME) return;

	if (m_bStartActive)
	{
		DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hObject);
		dwUsrFlags |= USRFLG_CAMERA_LIVE;
		pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);
		
		m_fTurnOffTime = pServerDE->GetTime() + m_fActiveTime;

		pServerDE->SetNextUpdate(m_hObject, 0.001f);
	}

	// Tell the clients about us...

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_CAMERA_ID);
	pServerDE->WriteToMessageByte(hMessage, (DBYTE)m_bAllowPlayerMovement);
	pServerDE->WriteToMessageByte(hMessage, (DBYTE)m_nCameraType);
	pServerDE->WriteToMessageByte(hMessage, (DBYTE)m_bIsListener);
	pServerDE->EndMessage(hMessage);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::TriggerMsg()
//
//	PURPOSE:	Trigger function to turn camera on/off
//
// --------------------------------------------------------------------------- //

void Camera::TriggerMsg(HOBJECT hSender, HSTRING hMsg)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pMsg = pServerDE->GetStringData(hMsg);
	if (!pMsg) return;

	DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hObject);

	if (_stricmp(pMsg, "ON") == 0)
	{
		dwUsrFlags |= USRFLG_CAMERA_LIVE;
		if (m_fActiveTime > 0.0f) 
		{
			m_fTurnOffTime = pServerDE->GetTime() + m_fActiveTime;
			pServerDE->SetNextUpdate(m_hObject, 0.001f);
		}
	}
	else if (_stricmp(pMsg, "OFF") == 0)
	{
		dwUsrFlags &= ~USRFLG_CAMERA_LIVE;
		pServerDE->SetNextUpdate(m_hObject, 0.0f);

		if (m_bOneTime)
		{
			pServerDE->RemoveObject(m_hObject);
		}
	}

	pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::Update()
//
//	PURPOSE:	Turn camera off
//
// --------------------------------------------------------------------------- //

void Camera::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Tell all de-active objects I can see to become active...

	pServerDE->PingObjects(m_hObject);


	if (m_fActiveTime > 0.0f && pServerDE->GetTime() > m_fTurnOffTime)
	{	
		// Turn camera off...

		DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hObject);
		pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags & ~USRFLG_CAMERA_LIVE);

		if (m_bOneTime)
		{
			pServerDE->RemoveObject(m_hObject);
		}
	}
	else
	{
		pServerDE->SetNextUpdate(m_hObject, 0.001f);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Camera::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageByte(hWrite, m_bIsListener);
	pServerDE->WriteToMessageByte(hWrite, m_bAllowPlayerMovement);
	pServerDE->WriteToMessageByte(hWrite, m_bOneTime);
	pServerDE->WriteToMessageByte(hWrite, m_nCameraType);
	pServerDE->WriteToMessageByte(hWrite, m_bStartActive);
	pServerDE->WriteToMessageFloat(hWrite, m_fActiveTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fTurnOffTime);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Camera::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_bIsListener			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bAllowPlayerMovement	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bOneTime				= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_nCameraType			= pServerDE->ReadFromMessageByte(hRead);
	m_bStartActive			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_fActiveTime			= pServerDE->ReadFromMessageFloat(hRead);
	m_fTurnOffTime			= pServerDE->ReadFromMessageFloat(hRead);
}