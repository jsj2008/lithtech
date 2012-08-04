// ----------------------------------------------------------------------- //
//
// MODULE  : DialogTrigger.cpp
//
// PURPOSE : DialogTrigger implementation
//
// CREATED : 1/26/98
//
// ----------------------------------------------------------------------- //

#include "DialogTrigger.h"
#include "PlayerObj.h"
#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "RiotObjectUtilities.h"

BEGIN_CLASS(DialogTrigger)
	ADD_ACTIVATION_AGGREGATE()
	ADD_VECTORPROP_VAL_FLAG(Dims, 5.0f, 5.0f, 5.0f, PF_DIMS) 
	ADD_REALPROP(String1ID, 0.0f)
	ADD_STRINGPROP(Target1, "")
	ADD_STRINGPROP(Message1, "")
	ADD_REALPROP(String2ID, 0.0f)
	ADD_STRINGPROP(Target2, "")
	ADD_STRINGPROP(Message2, "")
	ADD_REALPROP(String3ID, 0.0f)
	ADD_STRINGPROP(Target3, "")
	ADD_STRINGPROP(Message3, "")
	ADD_REALPROP(SendDelay, 0.0f)
END_CLASS_DEFAULT(DialogTrigger, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DialogTrigger::DialogTrigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

DialogTrigger::DialogTrigger() : BaseClass ()
{
	AddAggregate(&m_activation);
	
	VEC_SET( m_vDims, 5.0f, 5.0f, 5.0f );
	
	m_hPlayerObject = DNULL;
	m_bTriggered	= DFALSE;

	memset (m_nStringID, 0, MAX_MESSAGES_NUM * sizeof (DDWORD));
	memset (m_hTarget, 0, MAX_MESSAGES_NUM * sizeof (HSTRING));
	memset (m_hMessage, 0, MAX_MESSAGES_NUM * sizeof (HSTRING));
	
	m_fSendDelay = 0.0f;
	m_bFirstUpdate = DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DialogTrigger::~DialogTrigger()
//
//	PURPOSE:	Free object
//
// ----------------------------------------------------------------------- //

DialogTrigger::~DialogTrigger()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	for (int i = 0; i < MAX_MESSAGES_NUM; i++)
	{
		if (m_hTarget[i]) pServerDE->FreeString (m_hTarget[i]);
		if (m_hMessage[i]) pServerDE->FreeString (m_hMessage[i]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DialogTrigger::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

DDWORD DialogTrigger::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
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
//	ROUTINE:	DialogTrigger::EngineMessageFn
//
//	PURPOSE:	Handle engineEngineMessageFn messages
//
// ----------------------------------------------------------------------- //

DDWORD DialogTrigger::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	switch (messageID)
	{
		case MID_INITIALUPDATE:
		{
			if ((int)fData != INITIALUPDATE_SAVEGAME)
			{
				pServerDE->SetObjectDims(m_hObject, &m_vDims);
				pServerDE->SetObjectFlags(m_hObject, FLAG_TOUCH_NOTIFY);
				pServerDE->SetNextUpdate(m_hObject, 0.1f);
			}
		}
		break;

		case MID_UPDATE:
		{
			if (m_bFirstUpdate)
			{
				m_bFirstUpdate = DFALSE;

				pServerDE->SetNextUpdate (m_hObject, 0.0f);
			}

			if (m_hPlayerObject)
			{
				ShowDialog();
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
			if ((int)fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_LINKBROKEN :
		{
			HOBJECT hLink = (HOBJECT)pData;
			if (hLink)
			{
				if (hLink == m_hPlayerObject)
				{
					m_hPlayerObject = DNULL;
				}
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
//	ROUTINE:	DialogTrigger::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

DBOOL DialogTrigger::ReadProp(ObjectCreateStruct *pData)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !pData) return DFALSE;

	DFLOAT fRealVal;
	char buf[MAX_CS_FILENAME_LEN];

	pServerDE->GetPropVector("Dims", &m_vDims);

	pServerDE->GetPropReal("String1ID", &fRealVal);
	m_nStringID[0] = (DDWORD) fRealVal;

	pServerDE->GetPropReal("String2ID", &fRealVal);
	m_nStringID[1] = (DDWORD) fRealVal;

	pServerDE->GetPropReal("String3ID", &fRealVal);
	m_nStringID[2] = (DDWORD) fRealVal;

	buf[0] = '\0';
	pServerDE->GetPropString("Target1", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hTarget[0] = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("Target2", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hTarget[1] = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("Target3", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hTarget[2] = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("Message1", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hMessage[0] = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("Message2", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hMessage[1] = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("Message3", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hMessage[2] = pServerDE->CreateString(buf);

	pServerDE->GetPropReal("SendDelay", &m_fSendDelay);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DialogTrigger::Trigger
//
//	PURPOSE:	Remotely trigger the dialog trigger
//
// ----------------------------------------------------------------------- //

void DialogTrigger::Trigger()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Find the first (only) player...

	HOBJECT hObj   = pServerDE->GetNextObject(DNULL);
	HCLASS  hClass = g_pServerDE->GetClass("CPlayerObj");

	while (hObj)
	{
		if (pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hObj), hClass))
		{
			ObjectTouch(hObj);
			break;
		}

		hObj = g_pServerDE->GetNextObject(hObj);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DialogTrigger::ObjectTouch
//
//	PURPOSE:	Handle object touch
//
// ----------------------------------------------------------------------- //

void DialogTrigger::ObjectTouch (HOBJECT hObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hObj) return;

	// Ignore everything but the player object

	if (!IsPlayer (hObj)) return;

	// if there's a send delay, start the timer
	// otherwise, send the dialog message now

	if (m_hPlayerObject)
	{
		pServerDE->BreakInterObjectLink(m_hObject, m_hPlayerObject);
	}

	m_hPlayerObject = hObj;
	pServerDE->CreateInterObjectLink(m_hObject, m_hPlayerObject);

	m_bTriggered = DTRUE;

	if (m_fSendDelay)
	{
		pServerDE->SetNextUpdate (m_hObject, m_fSendDelay);
	}
	else
	{
		ShowDialog();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DialogTrigger::ShowDialog
//
//	PURPOSE:	Sends a dialog message to the client of the player that
//				triggered us
//
// ----------------------------------------------------------------------- //

void DialogTrigger::ShowDialog()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hPlayerObject) return;
	
	// first count how many sets we have...

	int nItems = 0;
	for (int i = 0; i < MAX_MESSAGES_NUM; i++)
	{
		if (m_nStringID[i] && m_hTarget[i] && m_hMessage[i]) nItems++;
	}

	CPlayerObj* pPlayer = (CPlayerObj*)pServerDE->HandleToObject(m_hPlayerObject);
	if (!pPlayer) return;

	HCLIENT hClient = pPlayer->GetClient();
	HMESSAGEWRITE hMessage = pServerDE->StartMessage(hClient, MID_COMMAND_SHOWDLG);
	DDWORD nObjectHandle = (DDWORD) m_hObject;
	pServerDE->WriteToMessageByte (hMessage, (DBYTE)nObjectHandle);
	pServerDE->WriteToMessageByte (hMessage, (DBYTE)(nObjectHandle >> 8));
	pServerDE->WriteToMessageByte (hMessage, (DBYTE)(nObjectHandle >> 16));
	pServerDE->WriteToMessageByte (hMessage, (DBYTE)(nObjectHandle >> 24));
	pServerDE->WriteToMessageFloat (hMessage, (float) nItems);

	for (int i = 0; i < MAX_MESSAGES_NUM; i++)
	{
		if (m_nStringID[i] && m_hTarget[i] && m_hMessage[i])
		{
			pServerDE->WriteToMessageDWord (hMessage, m_nStringID[i]);
		}
	}
	pServerDE->EndMessage (hMessage);

	if (m_hPlayerObject)
	{
		pServerDE->BreakInterObjectLink(m_hObject, m_hPlayerObject);
		m_hPlayerObject = DNULL;
	}

	pServerDE->SetNextUpdate (m_hObject, 0.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DialogTrigger::Trigger
//
//	PURPOSE:	Sends a trigger to the given selection
//
// ----------------------------------------------------------------------- //

void DialogTrigger::Trigger (int nSelection)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	int nCurrentItem = -1;
	int i;
	for (i = 0; i < MAX_MESSAGES_NUM; i++)
	{
		if (m_nStringID[i] && m_hTarget[i] && m_hMessage[i]) nCurrentItem++;
		if (nCurrentItem == nSelection) break;
	}
	if (i == MAX_MESSAGES_NUM) return;


	// Find the target object...

	char* pData = pServerDE->GetStringData(m_hTarget[nCurrentItem]);
	if (!pData) return;

	HOBJECT hObj = DNULL;
	ObjectList* pList = pServerDE->FindNamedObjects (pData);
	if (!pList) return;

	if (pList->m_nInList > 0)
	{
		hObj = pList->m_pFirstLink->m_hObject;
	}
	pServerDE->RelinquishList (pList);

	if (hObj)
	{
		SendTriggerMsgToObject (this, hObj, m_hMessage[nCurrentItem]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DialogTrigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void DialogTrigger::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;
	
	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hPlayerObject);

	pServerDE->WriteToMessageByte(hWrite, m_bTriggered);
	pServerDE->WriteToMessageByte(hWrite, m_bFirstUpdate);
	pServerDE->WriteToMessageFloat(hWrite, m_fSendDelay);
	pServerDE->WriteToMessageVector(hWrite, &m_vDims);

	for (int i=0; i < MAX_MESSAGES_NUM; i++)
	{
		pServerDE->WriteToMessageDWord(hWrite, m_nStringID[i]);
		pServerDE->WriteToMessageHString(hWrite, m_hTarget[i]);
		pServerDE->WriteToMessageHString(hWrite, m_hMessage[i]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DialogTrigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void DialogTrigger::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hPlayerObject);

	m_bTriggered	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bFirstUpdate	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_fSendDelay	= pServerDE->ReadFromMessageFloat(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vDims);

	for (int i=0; i < MAX_MESSAGES_NUM; i++)
	{
		m_nStringID[i]	= pServerDE->ReadFromMessageDWord(hRead);
		m_hTarget[i]	= pServerDE->ReadFromMessageHString(hRead);
		m_hMessage[i]	= pServerDE->ReadFromMessageHString(hRead);
	}
}
