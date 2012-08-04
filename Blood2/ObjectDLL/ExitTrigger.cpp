// ----------------------------------------------------------------------- //
//
// MODULE  : ExitTrigger.cpp
//
// PURPOSE : ExitTrigger - Implementation
//
// CREATED : 3/24/98
//
// ----------------------------------------------------------------------- //

#include "ExitTrigger.h"
#include "cpp_server_de.h"
#include "SharedDefs.h"
#include "BloodServerShell.h"
#include "PlayerObj.h"
#include "ObjectUtilities.h"
#include "ClientServerShared.h"


BEGIN_CLASS(ExitTrigger)
	ADD_STRINGPROP(NextWorld, "")
	ADD_STRINGPROP(StartPointName, "start")
	ADD_BOOLPROP(EndEpisode, DFALSE)
	ADD_BOOLPROP(SubWorld, DFALSE)
	ADD_REALPROP_FLAG(ResetTime, 0.0f, PF_HIDDEN)
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
	ADD_BOOLPROP(TriggerRelayActivate, DTRUE)
END_CLASS_DEFAULT(ExitTrigger, Trigger, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::ExitTrigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

ExitTrigger::ExitTrigger() : Trigger()
{
	m_hstrNextWorld			= DNULL;
	m_hstrStartPointName	= DNULL;
	m_nExitType				= ENDWORLD_ENDOFWORLD;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::~ExitTrigger()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

ExitTrigger::~ExitTrigger()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrNextWorld)
	{
		pServerDE->FreeString(m_hstrNextWorld);
	}
	if (m_hstrStartPointName)
	{
		pServerDE->FreeString(m_hstrStartPointName);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD ExitTrigger::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;

		default : break;
	}


	return Trigger::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL ExitTrigger::ReadProp(ObjectCreateStruct *pData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pData) return DFALSE;

	char buf[MAX_CS_FILENAME_LEN];

	buf[0] = '\0';
	pServerDE->GetPropString("NextWorld", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrNextWorld = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("StartPointName", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrStartPointName = pServerDE->CreateString(buf);

	DBOOL bEndOfEpisode = DFALSE;
	pServerDE->GetPropBool("EndOfEpisode", &bEndOfEpisode);

	DBOOL bSubWorld = DFALSE;
	pServerDE->GetPropBool("SubWorld", &bSubWorld);

	if (bEndOfEpisode)
		m_nExitType = ENDWORLD_ENDOFEPISODE;
	else if (bSubWorld)
		m_nExitType = ENDWORLD_ENDOFSUBWORLD;
	else
		m_nExitType = ENDWORLD_ENDOFWORLD;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::SendMessages
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void ExitTrigger::SendMessages()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (g_pBloodServerShell && m_hstrStartPointName)
	{
		g_pBloodServerShell->SetStartPointName(m_hstrStartPointName);
	}

	HMESSAGEWRITE hMsg = pServerDE->StartMessage(DNULL, SMSG_EXITWORLD);
	pServerDE->WriteToMessageHString(hMsg, m_hstrNextWorld);
	pServerDE->WriteToMessageByte(hMsg, m_nExitType);
	pServerDE->EndMessage(hMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ExitTrigger::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageHString(hWrite, m_hstrNextWorld);
	pServerDE->WriteToMessageHString(hWrite, m_hstrStartPointName);
	pServerDE->WriteToMessageByte(hWrite, m_nExitType);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ExitTrigger::Load(HMESSAGEREAD hRead, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_hstrNextWorld			= pServerDE->ReadFromMessageHString(hRead);
	m_hstrStartPointName	= pServerDE->ReadFromMessageHString(hRead);
	m_nExitType				 = pServerDE->ReadFromMessageByte(hRead);
}

