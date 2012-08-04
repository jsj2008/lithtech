// ----------------------------------------------------------------------- //
//
// MODULE  : StoryTrigger.cpp
//
// PURPOSE : CStoryTrigger implementation
//
// CREATED : 12/8/97
//
// ----------------------------------------------------------------------- //

#include "StoryTrigger.h"
#include "cpp_server_de.h"
#include "generic_msg_de.h"

BEGIN_CLASS(CStoryTrigger)
END_CLASS_DEFAULT_FLAGS(CStoryTrigger, BaseClass, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CStoryTrigger::CStoryTrigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CStoryTrigger::CStoryTrigger()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CStoryTrigger::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

DDWORD CStoryTrigger::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER :
			HandleTriggerMsg(hSender, hRead);
		break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CStoryTrigger::EngineMessageFn
//
//	PURPOSE:	Handle engineEngineMessageFn messages
//
// ----------------------------------------------------------------------- //

DDWORD CStoryTrigger::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData)
{
	DDWORD dwRet;

	switch (messageID)
	{
		case MID_PRECREATE:
		{
			dwRet = BaseClass::EngineMessageFn(messageID, pData, lData);
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				SAFE_STRCPY(pStruct->m_Name, "Story");
			}

			return dwRet;
			break;
		}
		
		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, lData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CStoryTrigger::HandleTriggerMsg
//
//	PURPOSE:	Handle object trigger messages
//
// ----------------------------------------------------------------------- //

void CStoryTrigger::HandleTriggerMsg(HOBJECT hSender, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hSender || !hRead) return;

	HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);
	if (!hMsg) return;

	char* pMsg = pServerDE->GetStringData(hMsg);
	if (pMsg)
	{
		// Add the msg to the game console...

		pServerDE->RunGameConString(pMsg);
	}

	pServerDE->FreeString(hMsg);
}