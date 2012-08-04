// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectivesTrigger.cpp
//
// PURPOSE : ObjectivesTrigger - Implementation
//
// CREATED : 10/4/98
//
// ----------------------------------------------------------------------- //

#include "ObjectivesTrigger.h"
#include "cpp_server_de.h"
#include "SharedDefs.h"
#include "BloodServerShell.h"
#include "PlayerObj.h"
#include "ObjectUtilities.h"


BEGIN_CLASS(ObjectivesTrigger)
	ADD_LONGINTPROP(ResourceNum, 0)
	ADD_STRINGPROP(TitleToDisplay, "Objective #1:")
	ADD_STRINGPROP(TextToDisplay, "Use the '|' character to force a line break...")
	ADD_STRINGPROP(Sound, "Sounds\\Thunder.wav")

	ADD_STRINGPROP_FLAG(ActivationSound, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(SoundRadius, 200.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(TargetName1, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(MessageName1, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(MessageDelay, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(TargetName2, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(MessageName2, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(MessageDelay2, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(TargetName3, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(MessageName3, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(MessageDelay3, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(TargetName4, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(MessageName4, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(MessageDelay4, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(TargetName5, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(MessageName5, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(MessageDelay5, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(TargetName6, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(MessageName6, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(MessageDelay6, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(TargetName7, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(MessageName7, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(MessageDelay7, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(TargetName8, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(MessageName8, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(MessageDelay8, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(TargetName9, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(MessageName9, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(MessageDelay9, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(TargetName10, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(MessageName10, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(MessageDelay10, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP(TouchActivate, DFALSE)
	ADD_BOOLPROP(PlayerActivate, DTRUE)
	ADD_BOOLPROP(AIActivate, DFALSE)
	ADD_BOOLPROP(ObjectActivate, DFALSE)
	ADD_BOOLPROP(TriggerRelayActivate, DFALSE)
END_CLASS_DEFAULT(ObjectivesTrigger, Trigger, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectivesTrigger::ObjectivesTrigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

ObjectivesTrigger::ObjectivesTrigger() : Trigger()
{
	m_hstrTitle = DNULL;
	m_hstrText = DNULL;
	m_hstrSound = DNULL;
	m_nResource = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectivesTrigger::~ObjectivesTrigger()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

ObjectivesTrigger::~ObjectivesTrigger()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrTitle)
		pServerDE->FreeString(m_hstrTitle);

	if (m_hstrText)
		pServerDE->FreeString(m_hstrText);

	if (m_hstrSound)
		pServerDE->FreeString(m_hstrSound);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectivesTrigger::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD ObjectivesTrigger::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
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


	return Trigger::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectivesTrigger::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL ObjectivesTrigger::ReadProp(ObjectCreateStruct *pData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pData) return DFALSE;

	GenericProp genProp;

	if(pServerDE->GetPropGeneric("ResourceNum", &genProp) == DE_OK)
		m_nResource = genProp.m_Long;

	if(pServerDE->GetPropGeneric("TitleToDisplay", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) 
			m_hstrTitle = pServerDE->CreateString(genProp.m_String);
	}

	if(pServerDE->GetPropGeneric("TextToDisplay", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) 
			m_hstrText = pServerDE->CreateString(genProp.m_String);
	}

	if(pServerDE->GetPropGeneric("Sound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) 
			m_hstrSound = pServerDE->CreateString(genProp.m_String);
	}
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectivesTrigger::SendMessages
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void ObjectivesTrigger::SendMessages()
{
	CServerDE* pServerDE = GetServerDE();
	if(!pServerDE) return;

	if(m_hstrTitle && m_hstrText)
	{
		HMESSAGEWRITE hMsg = pServerDE->StartMessage(DNULL, SMSG_OBJECTIVES);
		pServerDE->WriteToMessageDWord(hMsg, m_nResource);
		pServerDE->WriteToMessageHString(hMsg, m_hstrTitle);
		pServerDE->WriteToMessageHString(hMsg, m_hstrText);
		pServerDE->EndMessage(hMsg);

		if(m_hstrSound)
			PlaySoundLocal(pServerDE->GetStringData(m_hstrSound), SOUNDPRIORITY_MISC_HIGH);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectivesTrigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ObjectivesTrigger::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageDWord(hWrite, m_nResource);
	pServerDE->WriteToMessageHString(hWrite, m_hstrTitle);
	pServerDE->WriteToMessageHString(hWrite, m_hstrText);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectivesTrigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ObjectivesTrigger::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_nResource		= pServerDE->ReadFromMessageDWord(hRead);
	m_hstrTitle		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrText		= pServerDE->ReadFromMessageHString(hRead);
}