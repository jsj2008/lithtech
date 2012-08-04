// ----------------------------------------------------------------------- //
//
// MODULE  : Trigger.h
//
// PURPOSE : Trigger - Implementation
//
// CREATED : 10/6/97
//
// ----------------------------------------------------------------------- //

#include "Trigger.h"
#include "cpp_server_de.h"
#include "RiotObjectUtilities.h"
#include "InventoryTypes.h"
#include "RiotMsgIds.h"
#include "PlayerObj.h"
#include <stdio.h>

BEGIN_CLASS(Trigger)
	ADD_ACTIVATION_AGGREGATE()
	ADD_VECTORPROP_VAL_FLAG(Dims, 16.0f, 16.0f, 16.0f, PF_DIMS) 
	ADD_REALPROP(SendDelay, 0.0f)
	ADD_REALPROP(TriggerDelay, 0.0)

	ADD_STRINGPROP(TargetName1, "")
	ADD_STRINGPROP(MessageName1, "")
	ADD_STRINGPROP(TargetName2, "")
	ADD_STRINGPROP(MessageName2, "")
	ADD_STRINGPROP(TargetName3, "")
	ADD_STRINGPROP(MessageName3, "")
	ADD_STRINGPROP(TargetName4, "")
	ADD_STRINGPROP(MessageName4, "")

	PROP_DEFINEGROUP(AdditionalTargets, PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName5, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(MessageName5, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName6, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(MessageName6, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName7, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(MessageName7, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName8, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(MessageName8, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName9, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(MessageName9, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName10, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(MessageName10, "", PF_GROUP1)

	ADD_BOOLPROP(TriggerTouch, 0)
	ADD_STRINGPROP(MessageTouch, "")
	ADD_BOOLPROP(PlayerTriggerable, 1)
	ADD_BOOLPROP(AITriggerable, 0)
	ADD_STRINGPROP(AITriggerName, "")
	ADD_BOOLPROP(WeightedTrigger, DFALSE)
	ADD_REALPROP(Message1Weight, .5)
	ADD_BOOLPROP(TimedTrigger, DFALSE)
	ADD_REALPROP(MinTriggerTime, 0.0f)
	ADD_REALPROP(MaxTriggerTime, 10.0f)
	ADD_LONGINTPROP(ActivationCount, 1)
	ADD_BOOLPROP(Locked, 0)
	ADD_STRINGPROP(UnlockKey, "")
	ADD_REALPROP(AccessDeniedMsg, 0)
	ADD_REALPROP(AccessGrantedMsg, 0)
	ADD_STRINGPROP(AccessDeniedSound, "")
	ADD_STRINGPROP(AccessGrantedSound, "")
	ADD_STRINGPROP(ActivationSound, "")
	ADD_REALPROP_FLAG(SoundRadius, 200.0f, PF_RADIUS)
	ADD_STRINGPROP(AttachToObject, "")
END_CLASS_DEFAULT(Trigger, BaseClass, NULL, NULL)


// Static global variables...

static char *g_szLock    = "LOCK"; 
static char *g_szUnLock  = "UNLOCK";
static char *g_szTrigger = "TRIGGER";

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Trigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Trigger::Trigger() : BaseClass()
{
	AddAggregate(&m_activation);

	VEC_SET( m_vDims, 5.0f, 5.0f, 5.0f );
	m_fTriggerDelay	= 0.0f;

	for (int i=0; i < MAX_NUM_MESSAGES; i++)
	{
		m_hstrTargetName[i] = DNULL;
		m_hstrMessageName[i] = DNULL;
	}

	m_bTriggerTouch			= DFALSE;
	m_bTouchNotifyActivation= DFALSE;
	m_hTouchObject			= DNULL;
	m_hstrMessageTouch		= DNULL;
	m_hstrActivationSound	= DNULL;
	m_hstrAttachToObject	= DNULL;
	m_bAttached				= DFALSE;
	m_fSoundRadius			= 200.0f;
	m_bActive				= DTRUE;
	m_bPlayerTriggerable	= DTRUE;
	m_bAITriggerable		= DFALSE;
	m_bLocked				= DFALSE;
	m_hstrUnlockKey			= DNULL;
	m_fAccessDeniedMsg		= 0.0f;
	m_fAccessGrantedMsg		= 0.0f;
	m_hstrAccessDeniedSound	= DNULL;
	m_hstrAccessGrantedSound= DNULL;
	m_hstrAIName			= DNULL;
	m_bDelayingActivate		= DFALSE;
	m_fStartDelayTime		= 0.0f;
	m_fSendDelay			= 0.0f;

	m_fLastTouchTime		= 0.0f;

	m_nCurrentActivation	= 0;
	m_nActivationCount		= 1;

	m_bWeightedTrigger		= DFALSE;
	m_fMessage1Weight		= .5f;

	m_bTimedTrigger			= DFALSE;
	m_fMinTriggerTime		= 0.0f;
	m_fMaxTriggerTime		= 1.0f;

	m_fNextTriggerTime		= 0.0f;

	m_hBoundingBox			= DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::SetTargetName1()
//
//	PURPOSE:	Set target name 1
//
// ----------------------------------------------------------------------- //

void Trigger::SetTargetName1(char* pName)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pName) return;
	
	if (m_hstrTargetName[0]) pServerDE->FreeString(m_hstrTargetName[0]);
	m_hstrTargetName[0] = pServerDE->CreateString(pName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::SetMessageName1()
//
//	PURPOSE:	Set target name 1
//
// ----------------------------------------------------------------------- //

void Trigger::SetMessageName1(char* pMsg)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pMsg) return;
	
	if (m_hstrMessageName[0]) pServerDE->FreeString(m_hstrMessageName[0]);
	m_hstrMessageName[0] = pServerDE->CreateString(pMsg);
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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	for (int i=0; i < MAX_NUM_MESSAGES; i++)
	{
		if (m_hstrTargetName[i])
		{
			pServerDE->FreeString(m_hstrTargetName[i]);
		}
		if (m_hstrMessageName[i])
		{
			pServerDE->FreeString(m_hstrMessageName[i]);
		}
	}

	if (m_hstrMessageTouch)
	{
		pServerDE->FreeString(m_hstrMessageTouch);
	}

	if (m_hstrActivationSound)
	{
		pServerDE->FreeString(m_hstrActivationSound);
	}

	if (m_hstrAttachToObject)
	{
		pServerDE->FreeString(m_hstrAttachToObject);
	}

	if (m_hstrUnlockKey)
	{
		pServerDE->FreeString(m_hstrUnlockKey);
	}

	if (m_hstrAccessDeniedSound) 
	{
		pServerDE->FreeString(m_hstrAccessDeniedSound);
	}

	if (m_hstrAccessGrantedSound)
	{
		pServerDE->FreeString(m_hstrAccessGrantedSound);
	}

	if (m_hstrAIName)
	{
		pServerDE->FreeString(m_hstrAIName);
	}

	if (m_hBoundingBox)
	{
		pServerDE->RemoveObject(m_hBoundingBox);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Trigger::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (!Update())
			{
				CServerDE* pServerDE = BaseClass::GetServerDE();
				if (!pServerDE) return 0;

				pServerDE->RemoveObject(m_hObject);		
			}
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			ObjectTouch((HOBJECT)pData);
		}
		break;

		case MID_PRECREATE:
		{
			ObjectCreateStruct *pStruct = (ObjectCreateStruct *)pData;
			if (!pStruct) return 0;

			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp(pStruct);
			}

			pStruct->m_UserData = USRFLG_IGNORE_PROJECTILES;
			pStruct->m_fDeactivationTime = TRIGGER_DEACTIVATION_TIME;
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			CacheFiles();
		}
		break;

		case MID_LINKBROKEN :
		{
			HOBJECT hLink = (HOBJECT)pData;
			if (hLink == m_hTouchObject)
			{
				m_hTouchObject = DNULL;
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

		case MID_PARENTATTACHMENTREMOVED :
		{
			// Go away if our parent is removed...

			CServerDE* pServerDE = BaseClass::GetServerDE();
			if (pServerDE)
			{
				pServerDE->RemoveObject(m_hObject);
			}
		}
		break;

		default : break;
	}


	return BaseClass::EngineMessageFn(messageID, pData, fData);
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
		case MID_INVENTORYQUERYRESPONSE :
			HandleInventoryQueryResponse(hSender, hRead);
		break;

		case MID_TRIGGER :
			HandleTriggerMsg(hSender, hRead);
		break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
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
	if (!pServerDE) return;

	HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);
	char* pMsg   = pServerDE->GetStringData(hMsg);

	// See if we should trigger the trigger...

	if (_stricmp(pMsg, g_szTrigger) == 0)
	{
		DoTrigger(hSender, DFALSE);
	}
	else if (_stricmp(pMsg, g_szLock) == 0)  // See if we should lock the trigger...
	{
		m_bLocked = DTRUE;
	}
	else if (_stricmp(pMsg, g_szUnLock) == 0) // See if we should unlock the trigger...
	{
		m_bLocked = DFALSE;
	}

	pServerDE->FreeString(hMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::HandleInventoryQueryResponse
//
//	PURPOSE:	Handle the INVENTORYQUERYRESPONSE message
//
// ----------------------------------------------------------------------- //

void Trigger::HandleInventoryQueryResponse(HOBJECT hSender, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DBYTE itemType = pServerDE->ReadFromMessageByte (hRead);
	DBYTE itemSubType = pServerDE->ReadFromMessageByte (hRead);
	HSTRING itemName = pServerDE->ReadFromMessageHString (hRead);
	DBYTE bHaveItem = pServerDE->ReadFromMessageByte (hRead);
	
	HCLIENT hClient = NULL;

	if (itemType == IT_KEY && m_hstrUnlockKey && pServerDE->CompareStrings(itemName, m_hstrUnlockKey))
	{
		pServerDE->FreeString(itemName);
		itemName = DNULL;

		// get a handle to the client

		if (IsPlayer (hSender))
		{
			CPlayerObj* pPlayer = (CPlayerObj*)pServerDE->HandleToObject (hSender);
			if (!pPlayer) return;
			hClient = pPlayer->GetClient();
		}
		else
		{
			return;
		}

		// unlock if necessary and send access granted or denied messages

		if (bHaveItem)
		{
			Unlock();
			
			if (m_fAccessGrantedMsg != 0.0f)
			{
				// send text message
				HSTRING hstr = pServerDE->FormatString ((int)m_fAccessGrantedMsg);
				HMESSAGEWRITE hWrite = pServerDE->StartMessage (hClient, MID_COMMAND_SHOWGAMEMSG);
				pServerDE->WriteToMessageString (hWrite, pServerDE->GetStringData (hstr));
				pServerDE->EndMessage (hWrite);
				pServerDE->FreeString (hstr);
			}

			if (m_hstrAccessGrantedSound)
			{
				char *pSound = pServerDE->GetStringData(m_hstrAccessGrantedSound);
				if (!pSound) return;

				PlaySoundFromObject(hSender, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_HIGH );
			}
		}
		else
		{
			if (m_fAccessDeniedMsg != 0.0f)
			{
				// send text message
				HSTRING hstr = pServerDE->FormatString ((int)m_fAccessDeniedMsg);
				if (hstr)
				{
					HMESSAGEWRITE hWrite = pServerDE->StartMessage (hClient, MID_COMMAND_SHOWGAMEMSG);
					pServerDE->WriteToMessageString (hWrite, pServerDE->GetStringData (hstr));
					pServerDE->EndMessage (hWrite);
					pServerDE->FreeString (hstr);
				}
			}
			
			if (m_hstrAccessDeniedSound)
			{
				char *pSound = pServerDE->GetStringData(m_hstrAccessDeniedSound);
				if (!pSound) return;

				PlaySoundFromObject(hSender, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_HIGH);
			}
		}
	}

	if (itemName) pServerDE->FreeString(itemName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL Trigger::ReadProp(ObjectCreateStruct *pData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pData) return DFALSE;

	const int nMaxFilesize = 500;
	char buf[nMaxFilesize + 1];

	char propName[50];
	for (int i=0; i < MAX_NUM_MESSAGES; i++)
	{
		sprintf(propName, "TargetName%d", i+1);
		if (pServerDE->GetPropString(propName, buf, nMaxFilesize) == DE_OK)
		{
			m_hstrTargetName[i] = pServerDE->CreateString(buf);
		}

		sprintf(propName, "MessageName%d", i+1);
		if (pServerDE->GetPropString(propName, buf, nMaxFilesize) == DE_OK)
		{
			m_hstrMessageName[i] = pServerDE->CreateString(buf);
		}
	}

	pServerDE->GetPropBool("TriggerTouch", &m_bTriggerTouch);

	buf[0] = '\0';
	pServerDE->GetPropString("MessageTouch", buf, nMaxFilesize);
	if (buf[0] && strlen(buf)) m_hstrMessageTouch = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("ActivationSound", buf, nMaxFilesize);
	if (buf[0] && strlen(buf)) m_hstrActivationSound = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("AttachToObject", buf, nMaxFilesize);
	if (buf[0] && strlen(buf)) m_hstrAttachToObject = pServerDE->CreateString(buf);

	pServerDE->GetPropVector("Dims", &m_vDims);
	pServerDE->GetPropReal("TriggerDelay", &m_fTriggerDelay);
	pServerDE->GetPropReal("SendDelay", &m_fSendDelay);
	pServerDE->GetPropReal("SoundRadius", &m_fSoundRadius);
	pServerDE->GetPropBool("PlayerTriggerable", &m_bPlayerTriggerable);
	pServerDE->GetPropBool("AITriggerable", &m_bAITriggerable);
	pServerDE->GetPropBool("Locked", &m_bLocked);

	buf[0] = '\0';
	pServerDE->GetPropString("UnlockKey", buf, nMaxFilesize);
	if (buf[0] && strlen(buf)) m_hstrUnlockKey = pServerDE->CreateString(buf);

	pServerDE->GetPropReal("SoundRadius", &m_fSoundRadius);
	pServerDE->GetPropReal("AccessDeniedMsg", &m_fAccessDeniedMsg);
	pServerDE->GetPropReal("AccessGrantedMsg", &m_fAccessGrantedMsg);

	buf[0] = '\0';
	pServerDE->GetPropString("AccessDeniedSound", buf, nMaxFilesize);
	if (buf[0] && strlen(buf)) m_hstrAccessDeniedSound = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("AccessGrantedSound", buf, nMaxFilesize);
	if (buf[0] && strlen(buf)) m_hstrAccessGrantedSound = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("AITriggerName", buf, nMaxFilesize);
	if (buf[0] && strlen(buf)) m_hstrAIName = pServerDE->CreateString(buf);

	long nLongVal;
	pServerDE->GetPropLongInt("ActivationCount", &nLongVal);
	m_nActivationCount = nLongVal;

	pServerDE->GetPropBool("WeightedTrigger", &m_bWeightedTrigger);
	pServerDE->GetPropReal("Message1Weight", &m_fMessage1Weight);

	pServerDE->GetPropBool("TimedTrigger", &m_bTimedTrigger);
	pServerDE->GetPropReal("MinTriggerTime", &m_fMinTriggerTime);
	pServerDE->GetPropReal("MaxTriggerTime", &m_fMaxTriggerTime);

	m_fNextTriggerTime = pServerDE->GetTime() + m_fMinTriggerTime;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::ObjectTouch
//
//	PURPOSE:	Handle object touch
//
// ----------------------------------------------------------------------- //

void Trigger::ObjectTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HCLASS hClassBaseCharacter = pServerDE->GetClass("CBaseCharacter");
	HCLASS hClassObj		   = pServerDE->GetObjectClass(hObj);
	HCLASS hClassAI			   = pServerDE->GetClass("BaseAI");


	// Only AI and players can trigger things...

	if (!pServerDE->IsKindOf(hClassObj, hClassBaseCharacter))
	{
		return;
	}


	// If we're AI, make sure we can activate this trigger...

	if (m_bAITriggerable)
	{
		if ( pServerDE->IsKindOf(hClassObj, hClassAI) )
		{
			if (m_hstrAIName) // See if only a specific AI can trigger it...
			{
				char* pAIName  = pServerDE->GetStringData(m_hstrAIName);
				char* pObjName = pServerDE->GetObjectName(hObj);

				if (pAIName && pObjName)
				{
					if ( stricmp(pAIName, pObjName) != 0 )
					{
						return;
					}
				} 
			}
		}
	}
	else  // Not AI triggerable
	{
		if ( pServerDE->IsKindOf(hClassObj, hClassAI) )
		{
			return;
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


	// Okay ready to trigger.  Make sure we've waited long enough before triggering...

	DFLOAT fTime = pServerDE->GetTime();

	if (fTime >= m_fLastTouchTime + m_fTriggerDelay)
	{
		m_fLastTouchTime = fTime;
		DoTrigger(hObj, DTRUE);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::DoTrigger
//
//	PURPOSE:	Determine if we can be triggered, and if so do it...
//
// ----------------------------------------------------------------------- //

void Trigger::DoTrigger(HOBJECT hObj, DBOOL bTouchNotify)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	m_bTouchNotifyActivation = bTouchNotify;
	m_hTouchObject = hObj;

	// Make sure we don't create links with ourself...

	if (m_hObject != m_hTouchObject)
	{
		pServerDE->CreateInterObjectLink(m_hObject, m_hTouchObject);
	}

	if (m_bActive)
	{
		// if we're locked, send a message to the object that triggered us, giving
		// that object a chance to unlock us (only if there is a way to unlock us)

		if (m_bLocked)
		{
			if (m_hstrUnlockKey)
			{
				HMESSAGEWRITE hWrite = pServerDE->StartMessageToObject (this, hObj, MID_INVENTORYITEMQUERY);
				pServerDE->WriteToMessageByte (hWrite, IT_KEY);
				pServerDE->WriteToMessageByte (hWrite, 0);
				pServerDE->WriteToMessageHString (hWrite, m_hstrUnlockKey);
				pServerDE->EndMessage (hWrite);
			}
			return;
		}
		
		// activate the trigger

		RequestActivate();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::InitialUpdate
//
//	PURPOSE:	Initial update
//
// ----------------------------------------------------------------------- //

DBOOL Trigger::InitialUpdate()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->SetObjectDims(m_hObject, &m_vDims);
	pServerDE->SetObjectFlags(m_hObject, FLAG_TOUCH_NOTIFY | FLAG_GOTHRUWORLD);
	pServerDE->SetObjectUserFlags(m_hObject, USRFLG_IGNORE_PROJECTILES);

	// If I'm not a timed trigger, my object touch notification 
	// will trigger new updates until then, I don't care...
	
	if (m_bTimedTrigger || m_hstrAttachToObject)
	{
		pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);
		pServerDE->SetDeactivationTime(m_hObject, 0.0f);
	}
	else
	{
		pServerDE->SetNextUpdate(m_hObject, 0.0f);
		pServerDE->SetDeactivationTime(m_hObject, TRIGGER_DEACTIVATION_TIME);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Update
//
//	PURPOSE:	Handle Update
//
// ----------------------------------------------------------------------- //

DBOOL Trigger::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;


	// Handle timed trigger...

	if (m_bTimedTrigger)
	{
		DFLOAT fTime = pServerDE->GetTime();
		if (fTime > m_fNextTriggerTime)
		{
			m_fNextTriggerTime = fTime + GetRandom(m_fMinTriggerTime, m_fMaxTriggerTime);
			DoTrigger(DNULL, DFALSE);
		}
	}

	
	// Attach the trigger to the object...

	if (m_hstrAttachToObject && !m_bAttached)
	{
		AttachToObject();
		m_bAttached = DTRUE;
	}



	if (m_bDelayingActivate)
	{
		UpdateDelayingActivate();
	}
	else
	{
		m_bActive = DTRUE;

		// If not a timed trigger, my object touch notification will trigger 
		// new updates until then, I don't care.
		
		if (m_bTimedTrigger)
		{
			pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);
			pServerDE->SetDeactivationTime(m_hObject, 0.0f);
		}
		else
		{
			pServerDE->SetNextUpdate(m_hObject, 0.0f);
			pServerDE->SetDeactivationTime(m_hObject, TRIGGER_DEACTIVATION_TIME);
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Unlock
//
//	PURPOSE:	Unlock the trigger and trigger it
//
// ----------------------------------------------------------------------- //

void Trigger::Unlock()
{
	m_bLocked = DFALSE;
	RequestActivate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::RequestActivate
//
//	PURPOSE:	Request activation of the trigger
//
// ----------------------------------------------------------------------- //

void Trigger::RequestActivate()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	if (m_bActive)
	{
		// We might not want to activate right away (if SendDelay was > 0)
		// Let Update() determine when to actually activate the trigger...

		pServerDE->SetNextUpdate(m_hObject, 0.001f);
		pServerDE->SetDeactivationTime(m_hObject, 0.0f);
		m_fStartDelayTime = pServerDE->GetTime();

		m_bDelayingActivate = DTRUE;
		m_bActive			= DFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::UpdateDelayingActivate
//
//	PURPOSE:	Update the delaying (and possibly activate) the trigger
//
// ----------------------------------------------------------------------- //

void Trigger::UpdateDelayingActivate()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_bDelayingActivate) return;
	
	DFLOAT fTime = pServerDE->GetTime();

	if (fTime >= m_fStartDelayTime + m_fSendDelay)
	{
		Activate();
		m_bDelayingActivate = DFALSE;
	}
	else
	{
		pServerDE->SetNextUpdate(m_hObject, 0.001f);
		pServerDE->SetDeactivationTime(m_hObject, 0.0f);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Activate
//
//	PURPOSE:	Activate the trigger.
//
// ----------------------------------------------------------------------- //

void Trigger::Activate()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;


	// Make us wait a bit before we can be triggered again...

	if (m_bTimedTrigger)
	{
		pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);
		pServerDE->SetDeactivationTime(m_hObject, 0.0f);
	}
	else
	{
		pServerDE->SetNextUpdate(m_hObject, m_fTriggerDelay);
		pServerDE->SetDeactivationTime(m_hObject, 0.0f);
	}
	
	
	// If this is a counter trigger, determine if we can activate or not...

	if (++m_nCurrentActivation < m_nActivationCount)
	{
		return;
	}
	else
	{
		m_nCurrentActivation = 0;
	}


	if (m_hstrActivationSound)
	{
		char* pSound = pServerDE->GetStringData(m_hstrActivationSound);
		if (pSound && pSound[0] != '\0')
		{
			PlaySoundFromObject(m_hObject, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_HIGH);
		}
	}

	DBOOL bTriggerMsg1 = DTRUE;
	DBOOL bTriggerMsg2 = DTRUE;

	if (m_bWeightedTrigger)
	{
		bTriggerMsg1 = (GetRandom(0.0f, 1.0f) < m_fMessage1Weight ? DTRUE : DFALSE);
		bTriggerMsg2 = !bTriggerMsg1;
	}

	for (int i=0; i < MAX_NUM_MESSAGES; i++)
	{
		DBOOL bOkayToSend = DTRUE;

		if (i == 0 && !bTriggerMsg1) bOkayToSend = DFALSE;
		else if (i == 1 && !bTriggerMsg2) bOkayToSend = DFALSE;

		if (bOkayToSend && m_hstrTargetName[i] && m_hstrMessageName[i])
		{
			SendTriggerMsgToObjects(this, m_hstrTargetName[i], m_hstrMessageName[i]);
		}
	}

	if (m_bTouchNotifyActivation && m_hTouchObject && m_bTriggerTouch && m_hstrMessageTouch)
	{
		SendTriggerMsgToObject(this, m_hTouchObject, m_hstrMessageTouch);
		m_bTouchNotifyActivation = DFALSE;
		m_hTouchObject = DNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::ToggleBoundingBoxes()
//
//	PURPOSE:	Toggle bounding boxes on/off
//
// ----------------------------------------------------------------------- //
	
void Trigger::ToggleBoundingBoxes()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hBoundingBox)
	{
		pServerDE->RemoveObject(m_hBoundingBox);
		m_hBoundingBox = DNULL;
	}
	else
	{
		CreateBoundingBox();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::CreateBoundingBox()
//
//	PURPOSE:	Create a bounding box
//
// ----------------------------------------------------------------------- //
	
void Trigger::CreateBoundingBox()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);
	VEC_COPY(theStruct.m_Pos, vPos);

	SAFE_STRCPY(theStruct.m_Filename, "Models\\Props\\1x1_square.abc");
	// strcpy(theStruct.m_SkinName, "SpecialFX\\smoke.dtx");
	theStruct.m_Flags = FLAG_VISIBLE;

	HCLASS hClass = pServerDE->GetClass("Model");
	LPBASECLASS pModel = pServerDE->CreateObject(hClass, &theStruct);

	if (pModel)
	{
		m_hBoundingBox = pModel->m_hObject;

		DVector vDims;
		pServerDE->GetObjectDims(m_hObject, &vDims);

		DVector vScale;
		VEC_DIVSCALAR(vScale, vDims, 0.5f);
		pServerDE->ScaleObject(m_hBoundingBox, &vScale);
	}

	pServerDE->SetObjectColor(m_hBoundingBox, GetRandom(0.5f, 1.0f), 
							  GetRandom(0.5f, 1.0f), GetRandom(0.5f, 1.0f), 1.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::AttachToObject()
//
//	PURPOSE:	Attach the trigger to an object
//
// ----------------------------------------------------------------------- //
	
void Trigger::AttachToObject()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hstrAttachToObject) return;

	char* pObjName = pServerDE->GetStringData(m_hstrAttachToObject);
	if (!pObjName) return;


	// Find object to attach to...

	HOBJECT hObj = DNULL;
	
	ObjectList*	pList = pServerDE->FindNamedObjects(pObjName);
	if (!pList) return;

	if (pList->m_pFirstLink)
	{
		hObj = pList->m_pFirstLink->m_hObject;
	}

	if (!hObj) return;
	
	DVector vOffset;
	VEC_INIT(vOffset);

	DRotation rOffset;
	ROT_INIT(rOffset);

	HATTACHMENT hAttachment;
	pServerDE->CreateAttachment(hObj, m_hObject, DNULL, &vOffset, &rOffset, &hAttachment);
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

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hTouchObject);
	pServerDE->WriteToMessageByte(hWrite, m_bAttached);
	pServerDE->WriteToMessageByte(hWrite, m_bActive);
	pServerDE->WriteToMessageByte(hWrite, m_bTriggerTouch);
	pServerDE->WriteToMessageByte(hWrite, m_bTouchNotifyActivation);
	pServerDE->WriteToMessageByte(hWrite, m_bPlayerTriggerable);
	pServerDE->WriteToMessageByte(hWrite, m_bAITriggerable);
	pServerDE->WriteToMessageByte(hWrite, m_bLocked);
	pServerDE->WriteToMessageByte(hWrite, m_bDelayingActivate);
	pServerDE->WriteToMessageByte(hWrite, m_bWeightedTrigger);
	pServerDE->WriteToMessageByte(hWrite, m_bTimedTrigger);

	pServerDE->WriteToMessageFloat(hWrite, m_fAccessDeniedMsg);
	pServerDE->WriteToMessageFloat(hWrite, m_fAccessGrantedMsg);
	pServerDE->WriteToMessageFloat(hWrite, m_fStartDelayTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fSendDelay);
	pServerDE->WriteToMessageFloat(hWrite, m_fLastTouchTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fMessage1Weight);
	pServerDE->WriteToMessageFloat(hWrite, m_fMinTriggerTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fMaxTriggerTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fNextTriggerTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fTriggerDelay);
	pServerDE->WriteToMessageFloat(hWrite, m_fSoundRadius);

	pServerDE->WriteToMessageDWord(hWrite, m_nActivationCount);
	pServerDE->WriteToMessageDWord(hWrite, m_nCurrentActivation);

	pServerDE->WriteToMessageHString(hWrite, m_hstrAccessDeniedSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrAccessGrantedSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrUnlockKey);
	pServerDE->WriteToMessageHString(hWrite, m_hstrAIName);
	pServerDE->WriteToMessageHString(hWrite, m_hstrMessageTouch);
	pServerDE->WriteToMessageHString(hWrite, m_hstrActivationSound);

	for (int i=0; i < MAX_NUM_MESSAGES; i++)
	{
		pServerDE->WriteToMessageHString(hWrite, m_hstrTargetName[i]);
		pServerDE->WriteToMessageHString(hWrite, m_hstrMessageName[i]);
	}
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

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hTouchObject);

	m_bAttached					= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bActive					= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bTriggerTouch				= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bTouchNotifyActivation	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bPlayerTriggerable		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bAITriggerable			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bLocked					= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bDelayingActivate			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bWeightedTrigger			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bTimedTrigger				= (DBOOL) pServerDE->ReadFromMessageByte(hRead);

	m_fAccessDeniedMsg			= pServerDE->ReadFromMessageFloat(hRead);
	m_fAccessGrantedMsg			= pServerDE->ReadFromMessageFloat(hRead);
	m_fStartDelayTime			= pServerDE->ReadFromMessageFloat(hRead);
	m_fSendDelay				= pServerDE->ReadFromMessageFloat(hRead);
	m_fLastTouchTime			= pServerDE->ReadFromMessageFloat(hRead);
	m_fMessage1Weight			= pServerDE->ReadFromMessageFloat(hRead);
	m_fMinTriggerTime			= pServerDE->ReadFromMessageFloat(hRead);
	m_fMaxTriggerTime			= pServerDE->ReadFromMessageFloat(hRead);
	m_fNextTriggerTime			= pServerDE->ReadFromMessageFloat(hRead);
	m_fTriggerDelay				= pServerDE->ReadFromMessageFloat(hRead);
	m_fSoundRadius				= pServerDE->ReadFromMessageFloat(hRead);

	m_nActivationCount			= pServerDE->ReadFromMessageDWord(hRead);
	m_nCurrentActivation		= pServerDE->ReadFromMessageDWord(hRead);

	m_hstrAccessDeniedSound		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrAccessGrantedSound	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrUnlockKey				= pServerDE->ReadFromMessageHString(hRead);
	m_hstrAIName				= pServerDE->ReadFromMessageHString(hRead);
	m_hstrMessageTouch			= pServerDE->ReadFromMessageHString(hRead);
	m_hstrActivationSound		= pServerDE->ReadFromMessageHString(hRead);

	for (int i=0; i < MAX_NUM_MESSAGES; i++)
	{
		m_hstrTargetName[i]		= pServerDE->ReadFromMessageHString(hRead);
		m_hstrMessageName[i]	= pServerDE->ReadFromMessageHString(hRead);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::CacheFiles
//
//	PURPOSE:	Cache resources used by this object
//
// ----------------------------------------------------------------------- //

void Trigger::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pFile = DNULL;
	if (m_hstrAccessDeniedSound)
	{
		pFile = pServerDE->GetStringData(m_hstrAccessDeniedSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrAccessGrantedSound)
	{
		pFile = pServerDE->GetStringData(m_hstrAccessGrantedSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrActivationSound)
	{
		pFile = pServerDE->GetStringData(m_hstrActivationSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}
}

