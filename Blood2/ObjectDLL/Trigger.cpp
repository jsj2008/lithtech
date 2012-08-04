// ----------------------------------------------------------------------- //
//
// MODULE  : Trigger.cpp
//
// PURPOSE : Trigger - Implementation
//
// CREATED : 10/6/97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include <string.h>
#include "Trigger.h"
#include "generic_msg_de.h"
#include "ObjectUtilities.h"
#include "ClientServerShared.h"
#include "PlayerObj.h"
#include <mbstring.h>

// Static variables...
static char *g_szLock    = "LOCK"; 
static char *g_szUnLock  = "UNLOCK";
static char *g_szTrigger = "TRIGGER";
static char *g_szRemove  = "REMOVE";
static char *g_szReset	 = "RESET";

extern CPlayerObj* g_pPlayerObj;
/*
	Trigger helper functions
*/

inline void SendTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, DBOOL bRemoveObject, HSTRING hMsg)
{
	if (!g_pServerDE) return;

	if (bRemoveObject)
	{
		g_pServerDE->SetObjectUserFlags(hObj, 0);	// Quick hack to fix airship door load/saves
		g_pServerDE->RemoveObject(hObj);
	}
	else
	{
		HMESSAGEWRITE hMessage;

		hMessage = g_pServerDE->StartMessageToObject(pSender, hObj, MID_TRIGGER);
		g_pServerDE->WriteToMessageHString(hMessage, hMsg);
		g_pServerDE->EndMessage(hMessage);
	}
}


void SendTriggerMsgToObjects(LPBASECLASS pSender, HSTRING hName, HSTRING hMsg)
{
	if (!g_pServerDE) return;

	DBOOL bRemoveObject = DFALSE;

	char* pMsg  = g_pServerDE->GetStringData(hMsg);
	char* pName = g_pServerDE->GetStringData(hName);

	if (!pMsg || !pName || !pName[0]) return;

	// See if it's a remove trigger
	if (_mbsicmp((const unsigned char*)pMsg, (const unsigned char*)g_szRemove) == 0)
	{
		bRemoveObject = DTRUE;
	}

	ObjectList*	pList = g_pServerDE->FindNamedObjects(pName);
	if (!pList) return;

	ObjectLink* pLink = pList->m_pFirstLink;
	while(pLink)
	{
		if (pLink)
		{
			SendTriggerMsgToObject(pSender, pLink->m_hObject, bRemoveObject, hMsg);
		}

		pLink = pLink->m_pNext;
	}
	
	g_pServerDE->RelinquishList(pList);
}


// Sends a trigger message to all objects of a class
void SendTriggerMsgToClass(LPBASECLASS pSender, HCLASS hClass, HSTRING hMsg)
{
	if (!g_pServerDE) return;

	DBOOL bRemoveObject = DFALSE;

	char* pMsg = g_pServerDE->GetStringData(hMsg);

	if (!pMsg || !hClass ) return;

	// See if it's a remove trigger
	if (_mbsicmp((const unsigned char*)pMsg, (const unsigned char*)g_szRemove) == 0)
	{
		bRemoveObject = DTRUE;
	}

	// Loop through active objects
	HOBJECT hObj = g_pServerDE->GetNextObject(DNULL);

	while (hObj)
	{
		if (g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hObj), hClass))
		{
			SendTriggerMsgToObject(pSender, hObj, bRemoveObject, hMsg);
		}

		hObj = g_pServerDE->GetNextObject(hObj);
	}

	// Now loop through inactive objects
	hObj = g_pServerDE->GetNextInactiveObject(DNULL);
	while (hObj)
	{
		if (g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hObj), hClass))
		{
			SendTriggerMsgToObject(pSender, hObj, bRemoveObject, hMsg);
		}

		hObj = g_pServerDE->GetNextInactiveObject(hObj);
	}
}


BEGIN_CLASS(Trigger)
	ADD_VECTORPROP_FLAG(Dims, PF_DIMS)
	ADD_REALPROP(ResetTime, 0.0f)
	ADD_STRINGPROP(ActivationSound, "")
	ADD_REALPROP(SoundRadius, 200.0f)
	ADD_STRINGPROP(TargetName1, "")
	ADD_STRINGPROP(MessageName1, "")
	ADD_REALPROP(MessageDelay, 0.0f)
	ADD_STRINGPROP(TargetName2, "")
	ADD_STRINGPROP(MessageName2, "")
	ADD_REALPROP(MessageDelay2, 0.0f)
	ADD_STRINGPROP(TargetName3, "")
	ADD_STRINGPROP(MessageName3, "")
	ADD_REALPROP(MessageDelay3, 0.0f)
	ADD_STRINGPROP(TargetName4, "")
	ADD_STRINGPROP(MessageName4, "")
	ADD_REALPROP(MessageDelay4, 0.0f)
	PROP_DEFINEGROUP(AdditionalTargets, PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName5, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(MessageName5, "", PF_GROUP1)
		ADD_REALPROP_FLAG(MessageDelay5, 0.0f, PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName6, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(MessageName6, "", PF_GROUP1)
		ADD_REALPROP_FLAG(MessageDelay6, 0.0f, PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName7, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(MessageName7, "", PF_GROUP1)
		ADD_REALPROP_FLAG(MessageDelay7, 0.0f, PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName8, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(MessageName8, "", PF_GROUP1)
		ADD_REALPROP_FLAG(MessageDelay8, 0.0f, PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName9, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(MessageName9, "", PF_GROUP1)
		ADD_REALPROP_FLAG(MessageDelay9, 0.0f, PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName10, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(MessageName10, "", PF_GROUP1)
		ADD_REALPROP_FLAG(MessageDelay10, 0.0f, PF_GROUP1)

	ADD_BOOLPROP(TouchActivate, DTRUE)
	ADD_BOOLPROP(PlayerActivate, DTRUE)
	ADD_BOOLPROP(AIActivate, DTRUE)
	ADD_BOOLPROP(ObjectActivate, DFALSE)
	ADD_BOOLPROP(TriggerRelayActivate, DTRUE)
	ADD_BOOLPROP(NamedObjectActivate, DFALSE)
	ADD_STRINGPROP(ActivationObjectName, "")
	
	ADD_LONGINTPROP(ActivationCount, 1)
	ADD_BOOLPROP(Locked, DFALSE)
//	ADD_STRINGPROP(LockedMsg, "")
	ADD_STRINGPROP(LockedSound, "")
//	ADD_STRINGPROP(UnlockedMsg, "")
	ADD_STRINGPROP(UnlockedSound, "")
	ADD_STRINGPROP(UnlockKeyName, "Key")
END_CLASS_DEFAULT(Trigger, B2BaseClass, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Trigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Trigger::Trigger() : B2BaseClass(OT_NORMAL)
{
	m_fTriggerResetTime		= 0.0f;

	for (int i=0;i < MAX_TRIGGER_MESSAGES; i++)
	{
		m_hstrTargetName[i]		= DNULL;
		m_hstrMessageName[i]	= DNULL;
		m_fTriggerMessageDelay[i] = 0.0f;
		m_bDelay[i]				= DFALSE;
	}
	m_fMaxTriggerMessageDelay = 0.0f;
	m_fDelayStartTime		= 0.0f;

	m_hstrActivationSound	= DNULL;
	m_fSoundRadius			= 200.0f;
	m_bActive				= DTRUE;
//	m_bDelay				= DFALSE;

	m_bTouchActivate		= DTRUE;
	m_bPlayerActivate		= DTRUE;
	m_bAIActivate			= DTRUE;
	m_bObjectActivate		= DFALSE;
	m_bTriggerRelayActivate = DTRUE;
	m_bNamedObjectActivate	= DFALSE;
	m_hstrActivationObjectName = DNULL;
	VEC_INIT(m_vDims);
	m_hLastSender			= DNULL;

	m_bLocked				= DFALSE;
	m_hstrLockedMsg			= DNULL;
	m_hstrLockedSound		= DNULL;
	m_hstrUnlockedMsg		= DNULL;
	m_hstrUnlockedSound		= DNULL;
	m_hstrKeyName			= DNULL;

	m_nCurrentActivation	= 0;
	m_nActivationCount		= 1;

	m_bSending				= DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::~Trigger()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Trigger::~Trigger()
{
	if (!g_pServerDE) return;

	for (int i=0;i < MAX_TRIGGER_MESSAGES; i++)
	{
		if (m_hstrTargetName[i])
		{
			g_pServerDE->FreeString(m_hstrTargetName[i]);
		}

		if (m_hstrMessageName[i])
		{
			g_pServerDE->FreeString(m_hstrMessageName[i]);
		}
	}

	if (m_hstrActivationObjectName)
	{
		g_pServerDE->FreeString(m_hstrActivationObjectName);
	}

	if (m_hstrLockedMsg)
		g_pServerDE->FreeString(m_hstrLockedMsg);

	if (m_hstrLockedSound)
		g_pServerDE->FreeString(m_hstrLockedSound);

	if (m_hstrUnlockedMsg)
		g_pServerDE->FreeString(m_hstrUnlockedMsg);

	if (m_hstrUnlockedSound)
		g_pServerDE->FreeString(m_hstrUnlockedSound);

	if (m_hstrKeyName)
		g_pServerDE->FreeString(m_hstrKeyName);

	if( m_hstrActivationSound )
		g_pServerDE->FreeString( m_hstrActivationSound );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL Trigger::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!g_pServerDE) return DFALSE;

	GenericProp genProp;

	for (int i=0; i < MAX_TRIGGER_MESSAGES; i++)
	{
		char key[40];

		sprintf(key, "TargetName%d", i+1);
		if (g_pServerDE->GetPropGeneric(key, &genProp) == DE_OK)
		{
			if (genProp.m_String[0])
				 m_hstrTargetName[i] = g_pServerDE->CreateString(genProp.m_String);
		}

		sprintf(key, "MessageName%d", i+1);
		if (g_pServerDE->GetPropGeneric(key, &genProp) == DE_OK)
		{
			if (genProp.m_String[0])
				 m_hstrMessageName[i] = g_pServerDE->CreateString(genProp.m_String);
		}

		if (i+1 > 1)
			sprintf(key, "MessageDelay%d", i+1);
		else
			sprintf(key, "MessageDelay");		// for backwards compatibility
		if (g_pServerDE->GetPropGeneric(key, &genProp) == DE_OK)
		{
			m_fTriggerMessageDelay[i] = genProp.m_Float;

			// Keep track of the max
			if (m_fTriggerMessageDelay[i] > m_fMaxTriggerMessageDelay)
				m_fMaxTriggerMessageDelay = m_fTriggerMessageDelay[i];
		}
	}

	if (g_pServerDE->GetPropGeneric("ActivationSound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrActivationSound = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("ActivationObjectName", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrActivationObjectName = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("Dims", &genProp) == DE_OK)
	{
		VEC_COPY(m_vDims, genProp.m_Vec);
	}

	if (g_pServerDE->GetPropGeneric("ResetTime", &genProp) == DE_OK)
	{
		m_fTriggerResetTime = genProp.m_Float;
	}

//	if (g_pServerDE->GetPropGeneric("MessageDelay", &genProp) == DE_OK)
//	{
//		m_fTriggerMessageDelay = genProp.m_Float;
//	}

	if (g_pServerDE->GetPropGeneric("TouchActivate", &genProp) == DE_OK)
	{
		m_bTouchActivate = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("PlayerActivate", &genProp) == DE_OK)
	{
		m_bPlayerActivate = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("AIActivate", &genProp) == DE_OK)
	{
		m_bAIActivate = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("TriggerRelayActivate", &genProp) == DE_OK)
	{
		m_bTriggerRelayActivate = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("ObjectActivate", &genProp) == DE_OK)
	{
		m_bObjectActivate = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("NamedObjectActivate", &genProp) == DE_OK)
	{
		m_bNamedObjectActivate = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("ActivationObjectName", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrActivationObjectName = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("Locked", &genProp) == DE_OK)
		m_bLocked = genProp.m_Bool;

	if (g_pServerDE->GetPropGeneric("LockedMsg", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrLockedMsg = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("LockedSound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrLockedSound = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("UnlockedMsg", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrUnlockedMsg = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("UnlockedSound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrUnlockedSound = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("UnlockKeyName", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrKeyName = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("ActivationCount", &genProp) == DE_OK)
		m_nActivationCount = genProp.m_Long;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::ObjectTouch
//
//	PURPOSE:	Handle object touch
//
// ----------------------------------------------------------------------- //

void Trigger::ObjectTouch (HOBJECT hObj)
{
	if (!g_pServerDE) return;
  
	if (m_bActive && m_bTouchActivate && ValidateSender(hObj))
	{
		m_hLastSender = hObj;

		// If this is a counter trigger, determine if we can activate or not...

		if (++m_nCurrentActivation < m_nActivationCount)
		{
			return;
		}
		else
		{
			m_nCurrentActivation = 0;
		}

		PlayActivationSound();

		// Make us wait a bit before we can be triggered again...

		if (m_fMaxTriggerMessageDelay)
		{
			m_fDelayStartTime = g_pServerDE->GetTime();

			g_pServerDE->SetNextUpdate(m_hObject, 0.01f);
			for (int i = 0; i < MAX_TRIGGER_MESSAGES; i++)
			{
				if (m_fTriggerMessageDelay[i])
					m_bDelay[i] = DTRUE;
				else if (m_hstrTargetName[i] && m_hstrMessageName[i])	// Else send the message now
					SendMessage(i);
			}
//			m_bDelay = DTRUE;
		}
		else
		{
			SendMessages();
			g_pServerDE->SetNextUpdate(m_hObject, m_fTriggerResetTime);
		}

		m_bActive = DFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::InitialUpdate
//
//	PURPOSE:	Initial update
//
// ----------------------------------------------------------------------- //

DBOOL Trigger::InitialUpdate(DVector *position)
{
	if (!g_pServerDE) return DFALSE;

	g_pServerDE->SetObjectDims(m_hObject, &m_vDims);
	g_pServerDE->SetObjectFlags(m_hObject, FLAG_TOUCH_NOTIFY);

	// My object touch notification will trigger new updates
	// until then, I don't care...
	
	// Mark this object as savable
	DDWORD dwFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
	dwFlags |= USRFLG_SAVEABLE;
	g_pServerDE->SetObjectUserFlags(m_hObject, dwFlags);

	g_pServerDE->SetNextUpdate(m_hObject, 0.0f);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Update
//
//	PURPOSE:	Handle Update
//
// ----------------------------------------------------------------------- //

DBOOL Trigger::Update(DVector *position)
{
	if (!g_pServerDE) return DFALSE;

	DBOOL bDelayed = DFALSE;

	// Trigger was delayed, send now
	for (int i=0; i < MAX_TRIGGER_MESSAGES; i++)
	{
		if (m_bDelay[i])
		{
			bDelayed = DTRUE;
		}
	}

	// Not waiting for any more delays, reset the trigger
	if (!bDelayed)
	{
		m_bActive = DTRUE;

		// My object touch notification will trigger new updates
		// until then, I don't care.
		g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
		return DTRUE;
	}
	else
	{
		DFLOAT fTime = g_pServerDE->GetTime();
		bDelayed = DFALSE;

		for (int i=0; i < MAX_TRIGGER_MESSAGES; i++)
		{
			if (m_bDelay[i])
			{
				if (fTime >= m_fDelayStartTime + m_fTriggerMessageDelay[i])
				{
					SendMessage(i);
					m_bDelay[i] = DFALSE;
				}
				else 
					bDelayed = DTRUE;	// Keep track of messages not sent, for NextUpdate
			}
		}

		if (bDelayed)
		{
			g_pServerDE->SetNextUpdate(m_hObject, 0.01f);
		}
		else	// No more messages, start reset time
		{
			g_pServerDE->SetNextUpdate(m_hObject, m_fTriggerResetTime);
		}
	}

	return DTRUE;
}


// ------------------------------------------------------------ //
//
//	ROUTINE:	Trigger::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //
DDWORD Trigger::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (!Update((DVector *)pData))
			{
				g_pServerDE->RemoveObject(m_hObject);
				return DFALSE;
			}

			break;
		}

		case MID_TOUCHNOTIFY:
		{
			ObjectTouch((HOBJECT)pData);
			break;
		}

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
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

		default : break;
	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

DDWORD Trigger::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER :
		{
			HandleTriggerMsg(hSender, hRead);
		}
		break;

		case MID_KEYQUERYRESPONSE:
		{
			HandleKeyQueryResponse(hSender, hRead);
		}
		break;
	}

	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::HandleKeyQueryResponse
//
//	PURPOSE:	Handle the MID_KEYQUERYRESPONSE message
//
// ----------------------------------------------------------------------- //

void Trigger::HandleKeyQueryResponse(HOBJECT hSender, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HSTRING hItemName = g_pServerDE->ReadFromMessageHString(hRead);
	DBOOL	bHaveItem = (DBOOL)g_pServerDE->ReadFromMessageByte(hRead);

	// Check the key & locked status
	if (m_bLocked && bHaveItem && g_pServerDE->CompareStringsUpper(hItemName, m_hstrKeyName))
	{
		pServerDE->FreeString(hItemName);
		// unlock if necessary and send access granted or denied messages
		m_bLocked = DFALSE;
		
		if (m_hstrUnlockedMsg)
		{
			// send text message
		}
/*
		if (m_hstrUnlockedSound)
		{
			char *pSound = pServerDE->GetStringData(m_hstrUnlockedSound);
			if (!pSound) return;

			PlaySoundFromObject(hSender, pSound, 200.0f, SOUNDTYPE_MISC, SOUNDPRIORITY_HIGH);
		}
*/
	}
	else
	{
		pServerDE->FreeString(hItemName);
		if (m_hstrLockedMsg)
		{
			// send text message
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::HandleTriggerMsg
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

void Trigger::HandleTriggerMsg(HOBJECT hSender, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_bActive) return;

	HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);
	char *pMsg = pServerDE->GetStringData(hMsg);
	
	if (pMsg)
	{
		if (_mbsicmp((const unsigned char*)pMsg, (const unsigned char*)g_szTrigger) == 0 && ValidateSender(hSender))
		{
			// If this is a counter trigger, determine if we can activate or not...

			if (++m_nCurrentActivation < m_nActivationCount)
			{
				pServerDE->FreeString(hMsg);
				return;
			}
			else
			{
				m_nCurrentActivation = 0;
			}

			PlayActivationSound();
	
			m_hLastSender = hSender;
			if (m_fMaxTriggerMessageDelay)
			{
				m_fDelayStartTime = g_pServerDE->GetTime();
	
				g_pServerDE->SetNextUpdate(m_hObject, 0.01f);
				for (int i = 0; i < MAX_TRIGGER_MESSAGES; i++)
				{
					if (m_fTriggerMessageDelay[i])
						m_bDelay[i] = DTRUE;
					else if (m_hstrTargetName[i] && m_hstrMessageName[i])	// Else send the message now
						SendMessage(i);
				}
			}
			else
			{
				SendMessages();
				g_pServerDE->SetNextUpdate(m_hObject, m_fTriggerResetTime);
			}

			m_bActive = DFALSE;
		}
		// reset the trigger: Set active, and clear the next update if any
		else if (_mbsicmp((const unsigned char*)pMsg, (const unsigned char*)g_szReset) == 0)
		{
			m_bActive = DTRUE;
			g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
		}
		else if (_mbsicmp((const unsigned char*)pMsg, (const unsigned char*)g_szLock) == 0)
		{
			m_bLocked = DTRUE;
			g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
		}
		else if (_mbsicmp((const unsigned char*)pMsg, (const unsigned char*)g_szUnLock) == 0)
		{
			if (m_hstrUnlockedSound && m_bLocked)
			{
				char *pSound = pServerDE->GetStringData(m_hstrUnlockedSound);
				if (!pSound)
				{
					g_pServerDE->FreeString( hMsg );
					return;
				}
				
				if (GetGameType() == GAMETYPE_ACTION && g_pPlayerObj)
				{
					HSTRING hstr = g_pServerDE->FormatString(IDS_GENERAL_UNLOCKED);
					g_pPlayerObj->SendConsoleMessage(g_pServerDE->GetStringData(hstr));
					g_pServerDE->FreeString(hstr);
//					g_pPlayerObj->SendConsoleMessage("Unlocked!");
				}
				else
				{
					PlaySoundFromObject(hSender, pSound, 200.0f, SOUNDPRIORITY_MISC_HIGH);
				}
			}
			m_bLocked = DFALSE;
			g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
		}
	}
	pServerDE->FreeString(hMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::SendMessage
//
//	PURPOSE:	Sends the trigger message for slot nSlot
//
// ----------------------------------------------------------------------- //
void Trigger::SendMessage(int nSlot)
{
	if (m_hstrTargetName[nSlot] && m_hstrMessageName[nSlot])
	{
		SendTriggerMsgToObjects(this, m_hstrTargetName[nSlot], m_hstrMessageName[nSlot]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::SendMessages
//
//	PURPOSE:	Sends the trigger messages
//
// ----------------------------------------------------------------------- //

void Trigger::SendMessages()
{
	if (m_bSending) return;
	m_bSending = DTRUE;

	for (int i=0; i < MAX_TRIGGER_MESSAGES; i++)
	{
		if (m_hstrTargetName[i] && m_hstrMessageName[i])
		{
			SendTriggerMsgToObjects(this, m_hstrTargetName[i], m_hstrMessageName[i]);
		}
	}

	m_bSending = DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::PlayActivationSound
//
//	PURPOSE:	Plays the activation sound, if any
//
// ----------------------------------------------------------------------- //
void Trigger::PlayActivationSound()
{
	if (!m_hstrActivationSound || !g_pServerDE) return;

	char* pSound = g_pServerDE->GetStringData(m_hstrActivationSound);
	if (pSound && pSound[0] != '\0')
	{
		PlaySoundFromObject(m_hObject, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::ValidateSender
//
//	PURPOSE:	Determines if the object triggering us is allowed to.
//
// ----------------------------------------------------------------------- //
DBOOL Trigger::ValidateSender(HOBJECT hObj)
{
	DBOOL	bRetVal = DFALSE;
	HCLASS	hAIObj		= g_pServerDE->GetClass("CBaseCharacter");
	HCLASS  hPlayerObj	= g_pServerDE->GetClass("CPlayerObj");
	HCLASS	hTriggerObj	= g_pServerDE->GetClass("Trigger");
	HCLASS	hObjClass	= g_pServerDE->GetObjectClass(hObj);

	if (!hObj) return bRetVal;

	// first check for a trigger relay
	if (m_bTriggerRelayActivate && g_pServerDE->IsKindOf(hObjClass, hTriggerObj))
	{
		bRetVal = DTRUE;
	}
	else if (g_pServerDE->IsKindOf(hObjClass, hPlayerObj))
	{
		if (m_bPlayerActivate)
		{
			if (m_bLocked && m_hstrKeyName)
			{
				// See if he has the key we need to unlock.. Trigger should become unlocked.
				HMESSAGEWRITE hMessage = g_pServerDE->StartMessageToObject((LPBASECLASS)this, hObj, MID_KEYQUERY);
				g_pServerDE->WriteToMessageHString(hMessage, m_hstrKeyName);
				g_pServerDE->EndMessage(hMessage);
			}
			if (!m_bLocked)
				bRetVal = DTRUE;
			else
			{
				if (m_hstrLockedSound && IsPlayer(hObj))
				{
					if (GetGameType() == GAMETYPE_ACTION && g_pPlayerObj)
					{
						HSTRING hstr = g_pServerDE->FormatString(IDS_GENERAL_LOCKED);
						g_pPlayerObj->SendConsoleMessage(g_pServerDE->GetStringData(hstr));
						g_pServerDE->FreeString(hstr);
//						g_pPlayerObj->SendConsoleMessage("Locked!");
					}
					else
					{
						char *pSound = g_pServerDE->GetStringData(m_hstrLockedSound);
						PlaySoundFromObject(hObj, pSound, 200.0f, SOUNDPRIORITY_MISC_HIGH);
					}
				}
			}
		}
	}
	else if (g_pServerDE->IsKindOf(hObjClass, hAIObj))
	{
		if (m_bAIActivate)
			bRetVal = DTRUE;
	}
	else if (m_bObjectActivate)
	{
		bRetVal = DTRUE;
	}

	// If object is triggerable, see if only ActivationObjectName can trigger it.
	if (bRetVal && m_bNamedObjectActivate && m_hstrActivationObjectName)
	{
		char *pName = g_pServerDE->GetObjectName(hObj);

		if (!pName || _mbsicmp((const unsigned char*)pName, (const unsigned char*)g_pServerDE->GetStringData(m_hstrActivationObjectName)) != 0)
			bRetVal = DFALSE;
	}

	return bRetVal;
}


void Trigger::Setup(DBOOL bActive, char *pszTarget, char *pszMessage, DBOOL bTouchActivate,
					DBOOL bPlayerActivate, DBOOL bAIActivate, DBOOL bObjectActivate, 
					DBOOL bTriggerRelayActivate, DBOOL bNamedObjectActivate, 
					HSTRING hstrActivationObjectName, DBOOL bLocked,
					HSTRING hstrLockedMsg, HSTRING hstrLockedSound, HSTRING hstrUnlockedMsg, 
					HSTRING hstrUnlockedSound, HSTRING hstrKeyName)
{
	if (!g_pServerDE) return;

	SetActive(bActive);
	SetTargetName(0, pszTarget);
	SetMessageName(0, pszMessage);

	m_bTouchActivate		= bTouchActivate;
	m_bPlayerActivate		= bPlayerActivate;
	m_bAIActivate			= bAIActivate;
	m_bObjectActivate		= bObjectActivate;
	m_bTriggerRelayActivate = bTriggerRelayActivate;
	m_bNamedObjectActivate	= bNamedObjectActivate;
	m_bLocked				= bLocked;

	if (hstrActivationObjectName)
	{
		if (m_hstrActivationObjectName)
		{
			g_pServerDE->FreeString(m_hstrActivationObjectName);
		}
		m_hstrActivationObjectName = g_pServerDE->CopyString(hstrActivationObjectName);
	}

	if (hstrLockedMsg)
	{
		if (m_hstrLockedMsg)
		{
			g_pServerDE->FreeString(m_hstrLockedMsg);
		}
		m_hstrLockedMsg = g_pServerDE->CopyString(hstrLockedMsg);
	}

	if (hstrLockedSound)
	{
		if (m_hstrLockedSound)
		{
			g_pServerDE->FreeString(m_hstrLockedSound);
		}
		m_hstrLockedSound = g_pServerDE->CopyString(hstrLockedSound);
	}

	if (hstrUnlockedMsg)
	{
		if (m_hstrUnlockedMsg)
		{
			g_pServerDE->FreeString(m_hstrUnlockedMsg);
		}
		m_hstrUnlockedMsg = g_pServerDE->CopyString(hstrUnlockedMsg);
	}

	if (hstrUnlockedSound)
	{
		if (m_hstrUnlockedSound)
		{
			g_pServerDE->FreeString(m_hstrUnlockedSound);
		}
		m_hstrUnlockedSound = g_pServerDE->CopyString(hstrUnlockedSound);
	}

	if (hstrKeyName)
	{
		if (m_hstrKeyName)
		{
			g_pServerDE->FreeString(m_hstrKeyName);
		}
		m_hstrKeyName = g_pServerDE->CopyString(hstrKeyName);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Trigger::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	DFLOAT fTime = pServerDE->GetTime();

	pServerDE->WriteToMessageByte(hWrite, m_bActive);
	pServerDE->WriteToMessageVector(hWrite, &m_vDims);
	pServerDE->WriteToMessageFloat(hWrite, m_fTriggerResetTime);

	pServerDE->WriteToMessageHString(hWrite, m_hstrActivationSound);
	pServerDE->WriteToMessageFloat(hWrite, m_fSoundRadius);

	for (int i=0; i < MAX_TRIGGER_MESSAGES; i++)
	{
		pServerDE->WriteToMessageHString(hWrite, m_hstrTargetName[i]);
		pServerDE->WriteToMessageHString(hWrite, m_hstrMessageName[i]);
		pServerDE->WriteToMessageFloat(hWrite, m_fTriggerMessageDelay[i]);
		pServerDE->WriteToMessageByte(hWrite, m_bDelay[i]);
	}
	pServerDE->WriteToMessageFloat(hWrite, m_fMaxTriggerMessageDelay);
	pServerDE->WriteToMessageFloat(hWrite, m_fDelayStartTime - fTime);

	pServerDE->WriteToMessageByte(hWrite, m_bTouchActivate);
	pServerDE->WriteToMessageByte(hWrite, m_bPlayerActivate);
	pServerDE->WriteToMessageByte(hWrite, m_bAIActivate);
	pServerDE->WriteToMessageByte(hWrite, m_bObjectActivate);
	pServerDE->WriteToMessageByte(hWrite, m_bTriggerRelayActivate);
	pServerDE->WriteToMessageByte(hWrite, m_bNamedObjectActivate);
	pServerDE->WriteToMessageByte(hWrite, m_bLocked);

	pServerDE->WriteToMessageDWord(hWrite, m_nActivationCount);
	pServerDE->WriteToMessageDWord(hWrite, m_nCurrentActivation);

	pServerDE->WriteToMessageHString(hWrite, m_hstrLockedMsg);
	pServerDE->WriteToMessageHString(hWrite, m_hstrLockedSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrUnlockedMsg);
	pServerDE->WriteToMessageHString(hWrite, m_hstrUnlockedSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrKeyName);

	pServerDE->WriteToMessageHString(hWrite, m_hstrActivationObjectName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Trigger::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	DFLOAT fTime = pServerDE->GetTime();

	m_bActive				= pServerDE->ReadFromMessageByte(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vDims);
	m_fTriggerResetTime		= pServerDE->ReadFromMessageFloat(hRead);

	m_hstrActivationSound	= pServerDE->ReadFromMessageHString(hRead);
	m_fSoundRadius			= pServerDE->ReadFromMessageFloat(hRead);

	for (int i=0; i < MAX_TRIGGER_MESSAGES; i++)
	{
		m_hstrTargetName[i]		= pServerDE->ReadFromMessageHString(hRead);
		m_hstrMessageName[i]	= pServerDE->ReadFromMessageHString(hRead);
		m_fTriggerMessageDelay[i]	= pServerDE->ReadFromMessageFloat(hRead);
		m_bDelay[i]			= pServerDE->ReadFromMessageByte(hRead);
	}
	m_fMaxTriggerMessageDelay	= pServerDE->ReadFromMessageFloat(hRead);
	m_fDelayStartTime		= pServerDE->ReadFromMessageFloat(hRead) + fTime;

	m_bTouchActivate		= pServerDE->ReadFromMessageByte(hRead);
	m_bPlayerActivate		= pServerDE->ReadFromMessageByte(hRead);
	m_bAIActivate			= pServerDE->ReadFromMessageByte(hRead);
	m_bObjectActivate		= pServerDE->ReadFromMessageByte(hRead);
	m_bTriggerRelayActivate = pServerDE->ReadFromMessageByte(hRead);
	m_bNamedObjectActivate	= pServerDE->ReadFromMessageByte(hRead);
	m_bLocked				= pServerDE->ReadFromMessageByte(hRead);

	m_nActivationCount		= pServerDE->ReadFromMessageDWord(hRead);
	m_nCurrentActivation	= pServerDE->ReadFromMessageDWord(hRead);

	m_hstrLockedMsg			= pServerDE->ReadFromMessageHString(hRead);
	m_hstrLockedSound		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrUnlockedMsg		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrUnlockedSound		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrKeyName			= pServerDE->ReadFromMessageHString(hRead);

	m_hstrActivationObjectName	= pServerDE->ReadFromMessageHString(hRead);
}



BEGIN_CLASS(BigTrigger)
END_CLASS_DEFAULT(BigTrigger, Trigger, NULL, NULL)



BEGIN_CLASS(ToggleTrigger)
	ADD_STRINGPROP(OffMessageName1, "")
	ADD_STRINGPROP(OffMessageName2, "")
	ADD_STRINGPROP(OffMessageName3, "")
	ADD_STRINGPROP(OffMessageName4, "")
	ADD_STRINGPROP(OffMessageName5, "")
	ADD_STRINGPROP(OffMessageName6, "")
	ADD_STRINGPROP(OffMessageName7, "")
	ADD_STRINGPROP(OffMessageName8, "")
	ADD_STRINGPROP(OffMessageName9, "")
	ADD_STRINGPROP(OffMessageName10, "")
	ADD_BOOLPROP(StartOn, DFALSE)
END_CLASS_DEFAULT(ToggleTrigger, Trigger, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ToggleTrigger::ToggleTrigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

ToggleTrigger::ToggleTrigger()
{
	for (int i=0;i < MAX_TRIGGER_MESSAGES; i++)
	{
		m_hstrOffMessageName[i]	= DNULL;
	}

	m_bOn = DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ToggleTrigger::~ToggleTrigger()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

ToggleTrigger::~ToggleTrigger()
{
	if (!g_pServerDE) return;

	for (int i=0;i < MAX_TRIGGER_MESSAGES; i++)
	{
		if (m_hstrOffMessageName[i])
		{
			g_pServerDE->FreeString(m_hstrOffMessageName[i]);
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ToggleTrigger::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL ToggleTrigger::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!g_pServerDE) return DFALSE;

	char buf[MAX_CS_FILENAME_LEN];

	for (int i=0; i < MAX_TRIGGER_MESSAGES; i++)
	{
		char key[40];

		sprintf(key, "OffMessageName%d", i+1);
		buf[0] = '\0';
		g_pServerDE->GetPropString(key, buf, MAX_CS_FILENAME_LEN);
		if (buf[0]) m_hstrOffMessageName[i] = g_pServerDE->CreateString(buf);
	}

	g_pServerDE->GetPropBool("StartOn", &m_bOn);

	return DTRUE;
}


// ------------------------------------------------------------ //
//
//	ROUTINE:	ToggleTrigger::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //
DDWORD ToggleTrigger::EngineMessageFn(DDWORD messageID, void *pData, float fData)
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
//	ROUTINE:	ToggleTrigger::SendMessages
//
//	PURPOSE:	Sends the trigger messages
//
// ----------------------------------------------------------------------- //
void ToggleTrigger::SendMessages()
{
	g_pServerDE->SetNextUpdate(m_hObject, m_fTriggerResetTime);

	for (int i=0; i < MAX_TRIGGER_MESSAGES; i++)
	{
		HSTRING hstrMessage = m_bOn ? m_hstrOffMessageName[i] : m_hstrMessageName[i];

		if (m_hstrTargetName[i] && hstrMessage)
		{
			SendTriggerMsgToObjects(this, m_hstrTargetName[i], hstrMessage);
		}

	}
	m_bOn = !m_bOn;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ToggleTrigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ToggleTrigger::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	for (int i=0; i < MAX_TRIGGER_MESSAGES; i++)
	{
		pServerDE->WriteToMessageHString(hWrite, m_hstrOffMessageName[i]);
	}

	pServerDE->WriteToMessageByte(hWrite, m_bOn);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ToggleTrigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ToggleTrigger::Load(HMESSAGEREAD hRead, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	for (int i=0; i < MAX_TRIGGER_MESSAGES; i++)
	{
		m_hstrOffMessageName[i]	= pServerDE->ReadFromMessageHString(hRead);
	}

	m_bOn = pServerDE->ReadFromMessageByte(hRead);
}



