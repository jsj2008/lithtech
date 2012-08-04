// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectiveTrigger.cpp
//
// PURPOSE : ObjectiveTrigger implementation
//
// CREATED : 3/22/98
//
// ----------------------------------------------------------------------- //

#include "ObjectiveTrigger.h"
#include "PlayerObj.h"
#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "RiotObjectUtilities.h"

BEGIN_CLASS(ObjectiveTrigger)
	ADD_ACTIVATION_AGGREGATE()
	ADD_VECTORPROP_VAL_FLAG(Dims, 5.0f, 5.0f, 5.0f, PF_DIMS) 
	ADD_BOOLPROP(PlayerTriggerable, 0)
	ADD_BOOLPROP(AITriggerable, 0)
	ADD_STRINGPROP(AITriggerName, "")
	ADD_STRINGPROP(AddObjectives, "")
	ADD_STRINGPROP(RemoveObjectives, "")
	ADD_STRINGPROP(CompleteObjectives, "")
END_CLASS_DEFAULT(ObjectiveTrigger, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveTrigger::ObjectiveTrigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

ObjectiveTrigger::ObjectiveTrigger() : BaseClass ()
{
	AddAggregate(&m_activation);
	
	VEC_SET( m_vDims, 5.0f, 5.0f, 5.0f );
	
	m_bPlayerTriggerable = DFALSE;

	m_bAITriggerable = DFALSE;
	m_hstrAIName = DNULL;
		
	m_bTriggered = DFALSE;

	m_hAddObjectives = DNULL;
	m_hRemoveObjectives = DNULL;
	m_hCompletedObjectives = DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveTrigger::~ObjectiveTrigger()
//
//	PURPOSE:	Free object
//
// ----------------------------------------------------------------------- //

ObjectiveTrigger::~ObjectiveTrigger()
{
	CServerDE* pServerDE = GetServerDE();

	if (m_hstrAIName) pServerDE->FreeString (m_hstrAIName);
	if (m_hAddObjectives) pServerDE->FreeString (m_hAddObjectives);
	if (m_hRemoveObjectives) pServerDE->FreeString (m_hRemoveObjectives);
	if (m_hCompletedObjectives) pServerDE->FreeString (m_hCompletedObjectives);

	if (!pServerDE) return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveTrigger::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

DDWORD ObjectiveTrigger::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			if (!m_bTriggered) Trigger();
		}
		break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveTrigger::EngineMessageFn
//
//	PURPOSE:	Handle engineEngineMessageFn messages
//
// ----------------------------------------------------------------------- //

DDWORD ObjectiveTrigger::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	switch (messageID)
	{
		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				pServerDE->SetObjectDims(m_hObject, &m_vDims);
				pServerDE->SetObjectFlags(m_hObject, FLAG_TOUCH_NOTIFY);
				pServerDE->SetObjectUserFlags(m_hObject, USRFLG_IGNORE_PROJECTILES);
				pServerDE->SetNextUpdate(m_hObject, 0.0f);
			}
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			if (!m_bTriggered) ObjectTouch((HOBJECT)pData);
		}
		break;

		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (!pStruct) return 0;

			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp(pStruct);
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
//	ROUTINE:	ObjectiveTrigger::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

DBOOL ObjectiveTrigger::ReadProp(ObjectCreateStruct *pData)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !pData) return DFALSE;

	char buf[MAX_CS_FILENAME_LEN];

	pServerDE->GetPropVector("Dims", &m_vDims);

	pServerDE->GetPropBool("PlayerTriggerable", &m_bPlayerTriggerable);

	pServerDE->GetPropBool("AITriggerable", &m_bAITriggerable);
	
	buf[0] = '\0';
	pServerDE->GetPropString("AITriggerName", buf, MAX_CS_FILENAME_LEN);
	if (buf[0] && strlen(buf)) m_hstrAIName = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("AddObjectives", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hAddObjectives = pServerDE->CreateString(buf);
	
	buf[0] = '\0';
	pServerDE->GetPropString("RemoveObjectives", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hRemoveObjectives = pServerDE->CreateString(buf);
	
	buf[0] = '\0';
	pServerDE->GetPropString("CompleteObjectives", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hCompletedObjectives = pServerDE->CreateString(buf);
	
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveTrigger::ObjectTouch
//
//	PURPOSE:	Handle object touch
//
// ----------------------------------------------------------------------- //

void ObjectiveTrigger::ObjectTouch (HOBJECT hObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// ignore everything but characters derived from BaseCharacter

	if (!IsBaseCharacter (hObj)) return;

	HCLASS hClassObj = pServerDE->GetObjectClass(hObj);
	
	// If we're AI, make sure we can activate this trigger...

	if (!m_bAITriggerable)
	{
		HCLASS hClassAI = pServerDE->GetClass("BaseAI");
		if ( pServerDE->IsKindOf(hClassObj, hClassAI) )
		{
			return;
		}
	}
	else if (m_hstrAIName) // See if only a specific AI can trigger it...
	{
		char* pAIName  = pServerDE->GetStringData(m_hstrAIName);
		char* pObjName = pServerDE->GetObjectName(hObj);

		if (pAIName && pObjName)
		{
			if ( stricmp(pAIName, pObjName) != 0)
			{
				return;
			}
		}
	}
	
	// If we're the player, make sure we can activate this trigger...

	if (!m_bPlayerTriggerable)
	{
		HCLASS hClassPlayer = pServerDE->GetClass("CPlayerObj");

		if ( pServerDE->IsKindOf(hClassObj, hClassPlayer) )
		{
			return;
		}
	}
	
	// send the mission objective message now

	Trigger();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveTrigger::Trigger
//
//	PURPOSE:	Sends a transmission message to the client of the player
//				that triggered us
//
// ----------------------------------------------------------------------- //

void ObjectiveTrigger::Trigger()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	m_bTriggered = DTRUE;

	HMESSAGEWRITE hMessage = pServerDE->StartMessage(DNULL, MID_COMMAND_OBJECTIVE);
	pServerDE->WriteToMessageHString (hMessage, m_hAddObjectives);
	pServerDE->WriteToMessageHString (hMessage, m_hRemoveObjectives);
	pServerDE->WriteToMessageHString (hMessage, m_hCompletedObjectives);
	pServerDE->EndMessage (hMessage);

	pServerDE->RemoveObject(m_hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveTrigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ObjectiveTrigger::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageByte(hWrite, m_bPlayerTriggerable);
	pServerDE->WriteToMessageByte(hWrite, m_bAITriggerable);
	pServerDE->WriteToMessageByte(hWrite, m_bTriggered);
	pServerDE->WriteToMessageHString(hWrite, m_hstrAIName);
	pServerDE->WriteToMessageHString(hWrite, m_hAddObjectives);
	pServerDE->WriteToMessageHString(hWrite, m_hRemoveObjectives);
	pServerDE->WriteToMessageHString(hWrite, m_hCompletedObjectives);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveTrigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ObjectiveTrigger::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_bPlayerTriggerable	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bAITriggerable		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bTriggered			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_hstrAIName			= pServerDE->ReadFromMessageHString(hRead);
	m_hAddObjectives		= pServerDE->ReadFromMessageHString(hRead);
	m_hRemoveObjectives		= pServerDE->ReadFromMessageHString(hRead);
	m_hCompletedObjectives	= pServerDE->ReadFromMessageHString(hRead);
}