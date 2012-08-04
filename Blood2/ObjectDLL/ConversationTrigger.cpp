// ----------------------------------------------------------------------- //
//
// MODULE  : ConversationTrigger.cpp
//
// PURPOSE : ConversationTrigger - Implementation
//
// CREATED : 5/26/98
//
// ----------------------------------------------------------------------- //

#include "ConversationTrigger.h"
#include "cpp_server_de.h"
#include "SharedDefs.h"
#include "BloodServerShell.h"
#include "PlayerObj.h"
#include "ObjectUtilities.h"
#include "ConversationStrings.h"


BEGIN_CLASS(ConversationTrigger)
	ADD_STRINGPROP(WaveFile, "default.wav")
	ADD_STRINGPROP(WaveFile2, "")
	ADD_STRINGPROP(WaveFile3, "")
	ADD_STRINGPROP(WaveFile4, "")
	ADD_STRINGPROP(WaveFile5, "")
	ADD_STRINGPROP(WaveFile6, "")
	ADD_STRINGPROP(WaveFile7, "")
	ADD_STRINGPROP(WaveFile8, "")

	ADD_LONGINTPROP(CharacterType, 0)
	ADD_BOOLPROP(RepeatLastMessage, DFALSE)
	ADD_BOOLPROP(LoopToFirstMessage, DFALSE)

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
	ADD_BOOLPROP_FLAG(TypeActionMode, DFALSE, PF_GROUP6)
END_CLASS_DEFAULT(ConversationTrigger, Trigger, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ConversationTrigger::ConversationTrigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

ConversationTrigger::ConversationTrigger() : Trigger()
{
	for (int i=0; i < CT_MAXMESSAGES; i++)
	{
		m_hstrSoundFile[i] = DNULL;
	}
	m_nCharacterType = 0;

	m_nNumMessages = 0;
	m_nCurrentMessage = 0;

	m_bRepeatLastMessage = DFALSE;
	m_bLoopMessages		 = DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ConversationTrigger::~ConversationTrigger()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

ConversationTrigger::~ConversationTrigger()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	for (int i=0; i < CT_MAXMESSAGES; i++)
	{
		if (m_hstrSoundFile[i])
			pServerDE->FreeString(m_hstrSoundFile[i]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ConversationTrigger::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD ConversationTrigger::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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
//	ROUTINE:	ConversationTrigger::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL ConversationTrigger::ReadProp(ObjectCreateStruct *pData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pData) return DFALSE;

	GenericProp genProp;

	for (int i=0; i < CT_MAXMESSAGES; i++)
	{
		char name[20];
		_mbscpy((unsigned char*)name, (const unsigned char*)"WaveFile");
		if (i > 0)
			_itoa(i+1, &name[_mbstrlen(name)], 10);

		if (pServerDE->GetPropGeneric(name, &genProp) == DE_OK)
		{
			if (genProp.m_String[0]) 
				m_hstrSoundFile[i] = pServerDE->CreateString(genProp.m_String);
			else
				break;
		}

		m_nNumMessages++;
	}

	if (pServerDE->GetPropGeneric("RepeatLastMessage", &genProp) == DE_OK)
	{
		m_bRepeatLastMessage = genProp.m_Bool;
	}

	if (pServerDE->GetPropGeneric("LoopToFirstMessage", &genProp) == DE_OK)
	{
		m_bLoopMessages = genProp.m_Bool;
	}

	if (pServerDE->GetPropGeneric("CharacterType", &genProp) == DE_OK)
	{
		m_nCharacterType = (char)genProp.m_Long;
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ConversationTrigger::SendMessages
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void ConversationTrigger::SendMessages()
{
/*	if (m_hLastSender && IsPlayer(m_hLastSender))
	{
		CServerDE* pServerDE = GetServerDE();
		if (!pServerDE) return;
	
		CPlayerObj* pPlayer = (CPlayerObj*)pServerDE->HandleToObject(m_hLastSender);
		if (!pPlayer) return;

		HCLIENT hClient = pPlayer->GetClient();

		HMESSAGEWRITE hMsg = pServerDE->StartMessage(hClient, SMSG_CONVERSATION);
		pServerDE->WriteToMessageHString(hMsg, m_hstrSoundFile[0]);
		pServerDE->WriteToMessageByte(hMsg, m_nCharacterType);
		pServerDE->EndMessage(hMsg);
	}
*/
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || m_nCurrentMessage >= m_nNumMessages ) return;

	char* pText = FilenameToText(m_hstrSoundFile[m_nCurrentMessage]);
	if (!pText) return;

	HSTRING hstrText = pServerDE->CreateString(pText);

	if (hstrText && m_hstrSoundFile[m_nCurrentMessage])
	{
		HMESSAGEWRITE hMsg = pServerDE->StartMessage(DNULL, SMSG_CONVERSATION);
		pServerDE->WriteToMessageHString(hMsg, m_hstrSoundFile[m_nCurrentMessage]);
		pServerDE->WriteToMessageHString(hMsg, hstrText);
		pServerDE->WriteToMessageByte(hMsg, m_nCharacterType);
		pServerDE->EndMessage(hMsg);

		m_nCurrentMessage++;
		if (m_nCurrentMessage == m_nNumMessages)
		{
			// Check repeat flags:  RepeatLastMessage causes the last sound to repeat
			// over and over. LoopMessages causes it to start over at the first.
			// Neither means that the trigger will no longer play a message.
			if (m_bRepeatLastMessage) 
				m_nCurrentMessage--;
			else if (m_bLoopMessages)
				m_nCurrentMessage = 0;
		}
	}

	if (hstrText)
	{
		pServerDE->FreeString(hstrText);
	}
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ConversationTrigger::CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void ConversationTrigger::CacheFiles()
{
/* TODO: Don't cache anything, want to use PLAYSOUND_STREAMING.
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pFile = DNULL;

	for (int i=0; i < CT_MAXMESSAGES; i++)
	{
		if (m_hstrSoundFile[i])
		{
			pFile = pServerDE->GetStringData(m_hstrSoundFile[i]);
			if (pFile)
			{
				 pServerDE->CacheFile(FT_SOUNDLOCAL, pFile);
			}
		}
	}
*/
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ConversationTrigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ConversationTrigger::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	for (int i=0; i < CT_MAXMESSAGES; i++)
	{
		pServerDE->WriteToMessageHString(hWrite, m_hstrSoundFile[i]);
	}
	pServerDE->WriteToMessageByte(hWrite, m_nCharacterType);
	pServerDE->WriteToMessageByte(hWrite, m_nNumMessages);
	pServerDE->WriteToMessageByte(hWrite, m_nCurrentMessage);

	pServerDE->WriteToMessageByte(hWrite, m_bRepeatLastMessage);
	pServerDE->WriteToMessageByte(hWrite, m_bLoopMessages);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ConversationTrigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ConversationTrigger::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	for (int i=0; i < CT_MAXMESSAGES; i++)
	{
		m_hstrSoundFile[i]	= pServerDE->ReadFromMessageHString(hRead);
	}
	m_nCharacterType	= pServerDE->ReadFromMessageByte(hRead);
	m_nNumMessages		= pServerDE->ReadFromMessageByte(hRead);
	m_nCurrentMessage	= pServerDE->ReadFromMessageByte(hRead);

	m_bRepeatLastMessage = pServerDE->ReadFromMessageByte(hRead);
	m_bLoopMessages		 = pServerDE->ReadFromMessageByte(hRead);
}



