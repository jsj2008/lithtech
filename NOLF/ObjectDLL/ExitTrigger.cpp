// ----------------------------------------------------------------------- //
//
// MODULE  : ExitTrigger.h
//
// PURPOSE : ExitTrigger - Implementation
//
// CREATED : 10/6/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ExitTrigger.h"
#include "iltserver.h"
#include "GameServerShell.h"
#include "ServerUtilities.h"
#include "PlayerObj.h"
#include "MsgIDs.h"

extern CGameServerShell* g_pGameServerShell;

BEGIN_CLASS(ExitTrigger)
	ADD_REALPROP_FLAG(FadeOutTime, 0.0f, 0)
	ADD_REALPROP_FLAG(TriggerDelay, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumberOfActivations, 1, PF_HIDDEN)
	ADD_REALPROP_FLAG(SendDelay, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(ActivationSound, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(SoundRadius, 200.0f, PF_HIDDEN)
	PROP_DEFINEGROUP(Targets, PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName1, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName1, "", PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName2, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName2, "", PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName3, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName3, "", PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName4, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName4, "", PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName5, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName5, "", PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName6, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName6, "", PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName7, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName7, "", PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName8, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName8, "", PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName9, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName9, "", PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName10, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName10, "", PF_GROUP1|PF_HIDDEN)
	ADD_BOOLPROP_FLAG(TriggerTouch, 0 , PF_HIDDEN)
	ADD_STRINGPROP_FLAG(MessageTouch, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(AITriggerName, "", PF_HIDDEN)
	ADD_BOOLPROP_FLAG(PlayerTriggerable, 1, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(AITriggerable, 0, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(WeightedTrigger, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Message1Weight, .5, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(TimedTrigger, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(MinTriggerTime, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(MaxTriggerTime, 10.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(ActivationCount, 1, PF_HIDDEN)
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
	m_fFadeOutTime	= 0.0f;
	m_dwFlags		&= ~FLAG_TOUCH_NOTIFY;  // Trigger activate only
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
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 ExitTrigger::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

            uint32 dwRet = Trigger::EngineMessageFn(messageID, pData, fData);

			PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}

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

LTBOOL ExitTrigger::ReadProp(ObjectCreateStruct *pData)
{
    if (!pData) return LTFALSE;

	GenericProp genProp;
    if (g_pLTServer->GetPropGeneric("FadeOutTime", &genProp) == LT_OK)
	{
		m_fFadeOutTime = genProp.m_Float;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::PostPropRead
//
//	PURPOSE:	Handle extra initialization
//
// ----------------------------------------------------------------------- //

void ExitTrigger::PostPropRead(ObjectCreateStruct *pStruct)
{
    m_bPlayerTriggerable = LTTRUE;
    m_bAITriggerable     = LTFALSE;
    m_hstrAIName         = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::Activate
//
//	PURPOSE:	Handle being activated
//
// ----------------------------------------------------------------------- //

LTBOOL ExitTrigger::Activate()
{
    if (!Trigger::Activate()) return LTFALSE;

	if (m_fFadeOutTime > 0.05f && g_pGameServerShell->GetGameType() == SINGLE)
	{
		// Tell the client to fade the screen and exit...

        HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_PLAYER_INFOCHANGE);
        g_pLTServer->WriteToMessageByte(hMessage, IC_FADE_SCREEN_ID);
        g_pLTServer->WriteToMessageByte(hMessage, 0);  // Fade out == 0
        g_pLTServer->WriteToMessageByte(hMessage, 1);  // Exit level == 1
        g_pLTServer->WriteToMessageFloat(hMessage, m_fFadeOutTime);
        g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);

		g_pGameServerShell->ExitLevel(LTFALSE);
	}
	else
	{
		g_pGameServerShell->ExitLevel(LTTRUE);
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ExitTrigger::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageFloat(hWrite, m_fFadeOutTime);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ExitTrigger::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

    m_fFadeOutTime = g_pLTServer->ReadFromMessageFloat(hRead);
}