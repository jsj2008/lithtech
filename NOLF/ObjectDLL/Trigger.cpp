// ----------------------------------------------------------------------- //
//
// MODULE  : Trigger.cpp
//
// PURPOSE : Trigger - Implementation
//
// CREATED : 10/6/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Trigger.h"
#include "iltserver.h"
#include "MsgIds.h"
#include "PlayerObj.h"
#include "gameservershell.h"
#include "SoundMgr.h"
#include "ObjectMsgs.h"
#include <stdio.h>

BEGIN_CLASS(Trigger)
	ADD_VECTORPROP_VAL_FLAG(Dims, 16.0f, 16.0f, 16.0f, PF_DIMS)
	ADD_LONGINTPROP(NumberOfActivations, 1)
	ADD_REALPROP(SendDelay, 0.0f)
	ADD_REALPROP(TriggerDelay, 0.0)

	PROP_DEFINEGROUP(Targets, PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName1, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName1, "TRIGGER", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName2, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName2, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName3, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName3, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName4, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName4, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName5, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName5, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName6, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName6, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName7, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName7, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName8, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName8, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName9, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName9, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName10, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName10, "", PF_GROUP1)

	ADD_BOOLPROP(TriggerTouch, 0)
	ADD_STRINGPROP(MessageTouch, "")
	ADD_BOOLPROP(PlayerTriggerable, 1)
	ADD_BOOLPROP(AITriggerable, 0)
	ADD_STRINGPROP(AITriggerName, "")
	ADD_BOOLPROP(BodyTriggerable, 0)
	ADD_STRINGPROP(BodyTriggerName, "")
    ADD_BOOLPROP(WeightedTrigger, LTFALSE)
	ADD_REALPROP(Message1Weight, .5)
    ADD_BOOLPROP(TimedTrigger, LTFALSE)
	ADD_REALPROP(MinTriggerTime, 0.0f)
	ADD_REALPROP(MaxTriggerTime, 10.0f)
	ADD_LONGINTPROP(ActivationCount, 1)
	ADD_BOOLPROP(Locked, 0)
	ADD_STRINGPROP_FLAG(ActivationSound, "", PF_FILENAME)
	ADD_REALPROP_FLAG(SoundRadius, 200.0f, PF_RADIUS)
	ADD_STRINGPROP_FLAG(AttachToObject, "", PF_OBJECTLINK)
	ADD_LONGINTPROP(PlayerTeamFilter, 0)
END_CLASS_DEFAULT(Trigger, GameBase, NULL, NULL)


// Static global variables...

static char *g_szLock    = "LOCK";
static char *g_szUnLock  = "UNLOCK";
static char *g_szTrigger = "TRIGGER";

#define UPDATE_DELTA					0.1f
#define TRIGGER_DEACTIVATION_TIME		0.001f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Trigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Trigger::Trigger() : GameBase()
{
	m_vDims.Init(5.0f, 5.0f, 5.0f);
	m_fTriggerDelay	= 0.0f;

	for (int i=0; i < MAX_NUM_MESSAGES; i++)
	{
        m_hstrTargetName[i] = LTNULL;
        m_hstrMessageName[i] = LTNULL;
	}

    m_bTriggerTouch         = LTFALSE;
    m_bTouchNotifyActivation= LTFALSE;
    m_hTouchObject          = LTNULL;
    m_hstrMessageTouch      = LTNULL;
    m_hstrActivationSound   = LTNULL;
    m_hstrAttachToObject    = LTNULL;
    m_bAttached             = LTFALSE;
	m_fSoundRadius			= 200.0f;
    m_bActive               = LTTRUE;
    m_bPlayerTriggerable    = LTTRUE;
	m_nPlayerTeamFilter		= 0;
    m_bAITriggerable        = LTFALSE;
    m_bLocked               = LTFALSE;
    m_hstrAIName            = LTNULL;
    m_bBodyTriggerable      = LTFALSE;
    m_hstrBodyName          = LTNULL;
    m_bDelayingActivate     = LTFALSE;
	m_fStartDelayTime		= 0.0f;
	m_fSendDelay			= 0.0f;

	m_fLastTouchTime		= 0.0f;

	m_nCurrentActivation	= 0;
	m_nActivationCount		= 1;

	m_nNumActivations		= 1;
	m_nNumTimesActivated	= 0;

    m_bWeightedTrigger      = LTFALSE;
	m_fMessage1Weight		= .5f;

    m_bTimedTrigger         = LTFALSE;
	m_fMinTriggerTime		= 0.0f;
	m_fMaxTriggerTime		= 1.0f;

	m_fNextTriggerTime		= 0.0f;

	m_dwFlags				= (FLAG_TOUCH_NOTIFY | FLAG_GOTHRUWORLD);
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
	if (!pName) return;

    if (m_hstrTargetName[0]) g_pLTServer->FreeString(m_hstrTargetName[0]);
    m_hstrTargetName[0] = g_pLTServer->CreateString(pName);
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
	if (!pMsg) return;

    if (m_hstrMessageName[0]) g_pLTServer->FreeString(m_hstrMessageName[0]);
    m_hstrMessageName[0] = g_pLTServer->CreateString(pMsg);
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
	for (int i=0; i < MAX_NUM_MESSAGES; i++)
	{
		if (m_hstrTargetName[i])
		{
            g_pLTServer->FreeString(m_hstrTargetName[i]);
		}
		if (m_hstrMessageName[i])
		{
            g_pLTServer->FreeString(m_hstrMessageName[i]);
		}
	}

	if (m_hstrMessageTouch)
	{
        g_pLTServer->FreeString(m_hstrMessageTouch);
	}

	if (m_hstrActivationSound)
	{
        g_pLTServer->FreeString(m_hstrActivationSound);
	}

	if (m_hstrAttachToObject)
	{
        g_pLTServer->FreeString(m_hstrAttachToObject);
	}

	if (m_hstrAIName)
	{
        g_pLTServer->FreeString(m_hstrAIName);
	}

	if (m_hstrBodyName)
	{
        g_pLTServer->FreeString(m_hstrBodyName);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Trigger::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (!Update())
			{
                g_pLTServer->RemoveObject(m_hObject);
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
                m_hTouchObject = LTNULL;
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((HMESSAGEWRITE)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint32)fData);
		}
		break;

		case MID_PARENTATTACHMENTREMOVED :
		{
			// Go away if our parent is removed...

            g_pLTServer->RemoveObject(m_hObject);
		}
		break;

		default : break;
	}


	return GameBase::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 Trigger::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER :
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			HandleTriggerMsg(hSender, szMsg);
		break;
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::HandleTriggerMsg
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

void Trigger::HandleTriggerMsg(HOBJECT hSender, const char* szMsg)
{
	// See if we should trigger the trigger...

	if (_stricmp(szMsg, g_szTrigger) == 0)
	{
        DoTrigger(hSender, LTFALSE);
	}
	else if (_stricmp(szMsg, g_szLock) == 0)  // See if we should lock the trigger...
	{
        m_bLocked = LTTRUE;
	}
	else if (_stricmp(szMsg, g_szUnLock) == 0) // See if we should unlock the trigger...
	{
        m_bLocked = LTFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL Trigger::ReadProp(ObjectCreateStruct *pData)
{
    if (!pData) return LTFALSE;

	const int nMaxFilesize = 500;
	char buf[nMaxFilesize + 1];
	buf[0] = '\0';

	char propName[50];
	for (int i=0; i < MAX_NUM_MESSAGES; i++)
	{
		sprintf(propName, "TargetName%d", i+1);
		buf[0] = '\0';
        if (g_pLTServer->GetPropString(propName, buf, nMaxFilesize) == LT_OK)
		{
			if (buf[0] && strlen(buf))
			{
                m_hstrTargetName[i] = g_pLTServer->CreateString(buf);
			}
		}

		sprintf(propName, "MessageName%d", i+1);
		buf[0] = '\0';
        if (g_pLTServer->GetPropString(propName, buf, nMaxFilesize) == LT_OK)
		{
			if (buf[0] && strlen(buf))
			{
                m_hstrMessageName[i] = g_pLTServer->CreateString(buf);
			}
		}
	}

    g_pLTServer->GetPropBool("TriggerTouch", &m_bTriggerTouch);

	buf[0] = '\0';
    g_pLTServer->GetPropString("MessageTouch", buf, nMaxFilesize);
    if (buf[0] && strlen(buf)) m_hstrMessageTouch = g_pLTServer->CreateString(buf);

	buf[0] = '\0';
    g_pLTServer->GetPropString("ActivationSound", buf, nMaxFilesize);
    if (buf[0] && strlen(buf)) m_hstrActivationSound = g_pLTServer->CreateString(buf);

	buf[0] = '\0';
    g_pLTServer->GetPropString("AttachToObject", buf, nMaxFilesize);
    if (buf[0] && strlen(buf)) m_hstrAttachToObject = g_pLTServer->CreateString(buf);

    g_pLTServer->GetPropVector("Dims", &m_vDims);
    g_pLTServer->GetPropReal("TriggerDelay", &m_fTriggerDelay);
    g_pLTServer->GetPropReal("SendDelay", &m_fSendDelay);
    g_pLTServer->GetPropReal("SoundRadius", &m_fSoundRadius);
    g_pLTServer->GetPropBool("PlayerTriggerable", &m_bPlayerTriggerable);
    g_pLTServer->GetPropBool("AITriggerable", &m_bAITriggerable);
    g_pLTServer->GetPropBool("Locked", &m_bLocked);

    g_pLTServer->GetPropReal("SoundRadius", &m_fSoundRadius);

	buf[0] = '\0';
    g_pLTServer->GetPropString("AITriggerName", buf, nMaxFilesize);
    if (buf[0] && strlen(buf)) m_hstrAIName = g_pLTServer->CreateString(buf);

    g_pLTServer->GetPropBool("BodyTriggerable", &m_bBodyTriggerable);
	buf[0] = '\0';
    g_pLTServer->GetPropString("BodyTriggerName", buf, nMaxFilesize);
    if (buf[0] && strlen(buf)) m_hstrBodyName = g_pLTServer->CreateString(buf);

	long nLongVal;
    g_pLTServer->GetPropLongInt("ActivationCount", &nLongVal);
	m_nActivationCount = nLongVal;

    g_pLTServer->GetPropLongInt("NumberOfActivations", &nLongVal);
	m_nNumActivations = nLongVal;

    g_pLTServer->GetPropBool("WeightedTrigger", &m_bWeightedTrigger);
    g_pLTServer->GetPropReal("Message1Weight", &m_fMessage1Weight);

    g_pLTServer->GetPropBool("TimedTrigger", &m_bTimedTrigger);
    g_pLTServer->GetPropReal("MinTriggerTime", &m_fMinTriggerTime);
    g_pLTServer->GetPropReal("MaxTriggerTime", &m_fMaxTriggerTime);

    g_pLTServer->GetPropLongInt("PlayerTeamFilter", &nLongVal);
	m_nPlayerTeamFilter = nLongVal;

    m_fNextTriggerTime = g_pLTServer->GetTime() + m_fMinTriggerTime;

    return LTTRUE;
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
    HCLASS hClassCharacter = g_pLTServer->GetClass("CCharacter");
    HCLASS hClassObj           = g_pLTServer->GetObjectClass(hObj);
    HCLASS hClassAI            = g_pLTServer->GetClass("CAI");
    HCLASS hClassBody            = g_pLTServer->GetClass("Body");


	// Only AI and players and bodies can trigger things...

    if (!g_pLTServer->IsKindOf(hClassObj, hClassCharacter) && !g_pLTServer->IsKindOf(hClassObj, hClassBody))
	{
		return;
	}


	// If we're AI, make sure we can activate this trigger...

	if (m_bAITriggerable)
	{
        if ( g_pLTServer->IsKindOf(hClassObj, hClassAI) )
		{
			if (m_hstrAIName) // See if only a specific AI can trigger it...
			{
                char* pAIName  = g_pLTServer->GetStringData(m_hstrAIName);
                char* pObjName = g_pLTServer->GetObjectName(hObj);

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
        if ( g_pLTServer->IsKindOf(hClassObj, hClassAI) )
		{
			return;
		}
	}

	// If we're Body, make sure we can activate this trigger...

	if (m_bBodyTriggerable)
	{
        if ( g_pLTServer->IsKindOf(hClassObj, hClassBody) )
		{
			if (m_hstrBodyName) // See if only a specific Body can trigger it...
			{
                char* pBodyName  = g_pLTServer->GetStringData(m_hstrBodyName);
                char* pObjName = g_pLTServer->GetObjectName(hObj);

				if (pBodyName && pObjName)
				{
					if ( stricmp(pBodyName, pObjName) != 0 )
					{
						return;
					}
				}
			}
		}
	}
	else  // Not Body triggerable
	{
        if ( g_pLTServer->IsKindOf(hClassObj, hClassBody) )
		{
			return;
		}
	}


	// If we're the player, make sure we can activate this trigger...
	if (m_bPlayerTriggerable)
	{
		if (IsPlayer(hObj))
		{
            CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hObj);
			if (!pPlayer) return;

			// if this is a team game, check if the player is on the right team
			if (g_pGameServerShell->GetGameType() == COOPERATIVE_ASSAULT && m_nPlayerTeamFilter)
			{
				if (pPlayer->GetTeamID() != m_nPlayerTeamFilter)
					return;
			}

		}

	}
	else //not player triggerable
	{
		if ( IsPlayer(hObj) )
		{
			return;
		}
	}


	// Okay ready to trigger.  Make sure we've waited long enough before triggering...

    LTFLOAT fTime = g_pLTServer->GetTime();

	if (fTime >= m_fLastTouchTime + m_fTriggerDelay)
	{
		m_fLastTouchTime = fTime;
        DoTrigger(hObj, LTTRUE);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::DoTrigger
//
//	PURPOSE:	Determine if we can be triggered, and if so do it...
//
// ----------------------------------------------------------------------- //

void Trigger::DoTrigger(HOBJECT hObj, LTBOOL bTouchNotify)
{
	m_bTouchNotifyActivation = bTouchNotify;
	m_hTouchObject = hObj;

	// Make sure we don't create links with ourself...

	if (m_hObject != m_hTouchObject)
	{
        g_pLTServer->CreateInterObjectLink(m_hObject, m_hTouchObject);
	}

	if (m_bActive)
	{
		if (!m_bLocked)
		{
			RequestActivate();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::InitialUpdate
//
//	PURPOSE:	Initial update
//
// ----------------------------------------------------------------------- //

LTBOOL Trigger::InitialUpdate()
{
    g_pLTServer->SetObjectDims(m_hObject, &m_vDims);
    g_pLTServer->SetObjectFlags(m_hObject, m_dwFlags);
    g_pLTServer->SetObjectUserFlags(m_hObject, USRFLG_IGNORE_PROJECTILES);

	// If I'm not a timed trigger, my object touch notification
	// will trigger new updates until then, I don't care...

	if (m_bTimedTrigger || m_hstrAttachToObject)
	{
        SetNextUpdate(UPDATE_DELTA);
	}
	else
	{
        SetNextUpdate(0.0f);
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Update
//
//	PURPOSE:	Handle Update
//
// ----------------------------------------------------------------------- //

LTBOOL Trigger::Update()
{
	// Handle timed trigger...

	if (m_bTimedTrigger)
	{
        LTFLOAT fTime = g_pLTServer->GetTime();
		if (fTime > m_fNextTriggerTime)
		{
			m_fNextTriggerTime = fTime + GetRandom(m_fMinTriggerTime, m_fMaxTriggerTime);
            DoTrigger(LTNULL, LTFALSE);
		}
	}


	// Attach the trigger to the object...

	if (m_hstrAttachToObject && !m_bAttached)
	{
		AttachToObject();
        m_bAttached = LTTRUE;
	}



	if (m_bDelayingActivate)
	{
		UpdateDelayingActivate();
	}
	else
	{
        m_bActive = LTTRUE;

		// If not a timed trigger, my object touch notification will trigger
		// new updates until then, I don't care.

		if (m_bTimedTrigger)
		{
            SetNextUpdate(UPDATE_DELTA);
		}
		else
		{
            SetNextUpdate(0.0f);
		}
	}

    return LTTRUE;
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
    m_bLocked = LTFALSE;
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
	if (m_bActive)
	{
		// We might not want to activate right away (if SendDelay was > 0)
		// Let Update() determine when to actually activate the trigger...

        SetNextUpdate(0.001f);
        m_fStartDelayTime = g_pLTServer->GetTime();

        m_bDelayingActivate = LTTRUE;
        m_bActive           = LTFALSE;
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
	if (!m_bDelayingActivate) return;

    LTFLOAT fTime = g_pLTServer->GetTime();

	if (fTime >= m_fStartDelayTime + m_fSendDelay)
	{
		Activate();
        m_bDelayingActivate = LTFALSE;
        m_bActive           = LTTRUE;
	}
	else
	{
        SetNextUpdate(0.001f);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Activate
//
//	PURPOSE:	Activate the trigger.
//
// ----------------------------------------------------------------------- //

LTBOOL Trigger::Activate()
{
	// Make us wait a bit before we can be triggered again...

	if (m_bTimedTrigger)
	{
        SetNextUpdate(UPDATE_DELTA);
 	}
	else
	{
        SetNextUpdate(m_fTriggerDelay);
	}


	// If this is a counter trigger, determine if we can activate or not...

	if (++m_nCurrentActivation < m_nActivationCount)
	{
        return LTFALSE;
	}
	else
	{
		m_nCurrentActivation = 0;
	}


	// Only allow the object to be activated the number of specified times...

	if (m_nNumActivations > 0)
	{
		if (m_nNumTimesActivated >= m_nNumActivations)
		{
            return LTFALSE;
		}

		m_nNumTimesActivated++;
	}


	if (m_hstrActivationSound)
	{
        char* pSound = g_pLTServer->GetStringData(m_hstrActivationSound);
		if (pSound && pSound[0] != '\0')
		{
			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);
            g_pServerSoundMgr->PlaySoundFromPos(vPos, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_HIGH);
        }
	}

    LTBOOL bTriggerMsg1 = LTTRUE;
    LTBOOL bTriggerMsg2 = LTTRUE;

	if (m_bWeightedTrigger)
	{
        bTriggerMsg1 = (GetRandom(0.0f, 1.0f) < m_fMessage1Weight ? LTTRUE : LTFALSE);
		bTriggerMsg2 = !bTriggerMsg1;
	}

	for (int i=0; i < MAX_NUM_MESSAGES; i++)
	{
        LTBOOL bOkayToSend = LTTRUE;

        if (i == 0 && !bTriggerMsg1) bOkayToSend = LTFALSE;
        else if (i == 1 && !bTriggerMsg2) bOkayToSend = LTFALSE;

		if (bOkayToSend && m_hstrTargetName[i] && m_hstrMessageName[i])
		{
			SendTriggerMsgToObjects(this, m_hstrTargetName[i], m_hstrMessageName[i]);
		}
	}

	if (m_bTouchNotifyActivation && m_hTouchObject && m_bTriggerTouch && m_hstrMessageTouch)
	{
		SendTriggerMsgToObject(this, m_hTouchObject, m_hstrMessageTouch);
        m_bTouchNotifyActivation = LTFALSE;
        m_hTouchObject = LTNULL;
	}

    return LTTRUE;
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
	if (!m_hstrAttachToObject) return;

    char* pObjName = g_pLTServer->GetStringData(m_hstrAttachToObject);
	if (!pObjName) return;

	// Find object to attach to...

	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
    g_pLTServer->FindNamedObjects(pObjName, objArray);

	if (!objArray.NumObjects()) return;

	HOBJECT hObj = objArray.GetObject(0);

	if (!hObj) return;

    LTVector vOffset(0, 0, 0);

    LTRotation rOffset;
	rOffset.Init();

	HATTACHMENT hAttachment;
    g_pLTServer->CreateAttachment(hObj, m_hObject, LTNULL, &vOffset, &rOffset, &hAttachment);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Trigger::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

    g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hTouchObject);
    g_pLTServer->WriteToMessageByte(hWrite, m_bAttached);
    g_pLTServer->WriteToMessageByte(hWrite, m_bActive);
    g_pLTServer->WriteToMessageByte(hWrite, m_bTriggerTouch);
    g_pLTServer->WriteToMessageByte(hWrite, m_bTouchNotifyActivation);
    g_pLTServer->WriteToMessageByte(hWrite, m_bPlayerTriggerable);
    g_pLTServer->WriteToMessageByte(hWrite, m_bAITriggerable);
    g_pLTServer->WriteToMessageByte(hWrite, m_bBodyTriggerable);
    g_pLTServer->WriteToMessageByte(hWrite, m_bLocked);
    g_pLTServer->WriteToMessageByte(hWrite, m_bDelayingActivate);
    g_pLTServer->WriteToMessageByte(hWrite, m_bWeightedTrigger);
    g_pLTServer->WriteToMessageByte(hWrite, m_bTimedTrigger);

    g_pLTServer->WriteToMessageFloat(hWrite, m_fStartDelayTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fSendDelay);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fLastTouchTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fMessage1Weight);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fMinTriggerTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fMaxTriggerTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fNextTriggerTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fTriggerDelay);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fSoundRadius);
    g_pLTServer->WriteToMessageFloat(hWrite, (LTFLOAT)m_nNumActivations);

    g_pLTServer->WriteToMessageDWord(hWrite, m_nNumTimesActivated);
    g_pLTServer->WriteToMessageDWord(hWrite, m_nActivationCount);
    g_pLTServer->WriteToMessageDWord(hWrite, m_nCurrentActivation);
    g_pLTServer->WriteToMessageDWord(hWrite, m_nPlayerTeamFilter);


    g_pLTServer->WriteToMessageHString(hWrite, m_hstrAIName);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrBodyName);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrMessageTouch);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrActivationSound);

	for (int i=0; i < MAX_NUM_MESSAGES; i++)
	{
        g_pLTServer->WriteToMessageHString(hWrite, m_hstrTargetName[i]);
        g_pLTServer->WriteToMessageHString(hWrite, m_hstrMessageName[i]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Trigger::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hTouchObject);

    m_bAttached                 = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bActive                   = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bTriggerTouch             = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bTouchNotifyActivation    = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bPlayerTriggerable        = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bAITriggerable            = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bBodyTriggerable          = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bLocked                   = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bDelayingActivate         = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bWeightedTrigger          = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bTimedTrigger             = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);

    m_fStartDelayTime           = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fSendDelay                = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fLastTouchTime            = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fMessage1Weight           = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fMinTriggerTime           = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fMaxTriggerTime           = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fNextTriggerTime          = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fTriggerDelay             = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fSoundRadius              = g_pLTServer->ReadFromMessageFloat(hRead);
    m_nNumActivations           = (int) g_pLTServer->ReadFromMessageFloat(hRead);

    m_nNumTimesActivated        = g_pLTServer->ReadFromMessageDWord(hRead);
    m_nActivationCount          = g_pLTServer->ReadFromMessageDWord(hRead);
    m_nCurrentActivation        = g_pLTServer->ReadFromMessageDWord(hRead);
    m_nPlayerTeamFilter         = g_pLTServer->ReadFromMessageDWord(hRead);

    m_hstrAIName                = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrBodyName              = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrMessageTouch          = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrActivationSound       = g_pLTServer->ReadFromMessageHString(hRead);

	for (int i=0; i < MAX_NUM_MESSAGES; i++)
	{
        m_hstrTargetName[i]     = g_pLTServer->ReadFromMessageHString(hRead);
        m_hstrMessageName[i]    = g_pLTServer->ReadFromMessageHString(hRead);
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
	if (m_hstrActivationSound)
	{
        char* pFile = g_pLTServer->GetStringData(m_hstrActivationSound);
		if (pFile)
		{
            g_pLTServer->CacheFile(FT_SOUND, pFile);
		}
	}
}