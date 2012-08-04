// ----------------------------------------------------------------------- //
//
// MODULE  : CinematicTrigger.cpp
//
// PURPOSE : CinematicTrigger - Implementation
//
// CREATED : 2/17/99
//
// COMMENT : Copyright (c) 1999-2000, Monolith Productions, Inc.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>
#include "CinematicTrigger.h"
#include "Camera.h"
#include "KeyFramer.h"
#include "DialogueWindow.h"
#include "AI.h"
#include "PlayerObj.h"
#include "Character.h"
#include "ObjectMsgs.h"

#define ADD_DIALOGUE_PROP(num, group) \
	ADD_REALPROP_FLAG(Delay##num##, 0.0f, group)\
	ADD_STRINGPROP_FLAG(Dialogue##num##, 0, group)\
	ADD_STRINGPROP_FLAG(WhoPlaysDialogue##num##, "", group)\
	ADD_STRINGPROP_FLAG(StartDialogueTriggerTarget##num##, "", group)\
	ADD_STRINGPROP_FLAG(StartDialogueTriggerMsg##num##, "", group)\
	ADD_STRINGPROP_FLAG(WhoPlaysDecisions##num##, "", group)\
	ADD_STRINGPROP_FLAG(Decisions##num##, "", group)\
	ADD_STRINGPROP_FLAG(Replies##num##, "", group)\
	ADD_STRINGPROP_FLAG(RepliesTriggerTarget##num##, "", group)\
	ADD_STRINGPROP_FLAG(RepliesTriggerMsg##num##, "", group)\
    ADD_BOOLPROP_FLAG(Window##num##, LTFALSE, group)


BEGIN_CLASS(CinematicTrigger)
    ADD_BOOLPROP(CanSkip, LTTRUE)
    ADD_BOOLPROP(OneTimeOnly, LTTRUE)
    ADD_BOOLPROP(StartOn, LTFALSE)
	ADD_BOOLPROP(RemoveBadAI, LTFALSE)
	ADD_STRINGPROP(CleanUpTriggerTarget, "")
	ADD_STRINGPROP(CleanUpTriggerMsg, "")
	ADD_STRINGPROP(DialogueStartTriggerTarget, "")
	ADD_STRINGPROP(DialogueStartTriggerMsg, "")
	ADD_STRINGPROP(DialogueDoneTriggerTarget, "")
	ADD_STRINGPROP(DialogueDoneTriggerMsg, "")
    ADD_BOOLPROP(AllWindows, LTFALSE)
	ADD_BOOLPROP(LeaveCameraOnWhenDone, LTFALSE)

	PROP_DEFINEGROUP(Dialogue, PF_GROUP1)
		ADD_DIALOGUE_PROP(1, PF_GROUP1)
		ADD_DIALOGUE_PROP(2, PF_GROUP1)
		ADD_DIALOGUE_PROP(3, PF_GROUP1)
		ADD_DIALOGUE_PROP(4, PF_GROUP1)
		ADD_DIALOGUE_PROP(5, PF_GROUP1)
		ADD_DIALOGUE_PROP(6, PF_GROUP1)
		ADD_DIALOGUE_PROP(7, PF_GROUP1)
		ADD_DIALOGUE_PROP(8, PF_GROUP1)
		ADD_DIALOGUE_PROP(9, PF_GROUP1)
		ADD_DIALOGUE_PROP(10, PF_GROUP1)
		ADD_DIALOGUE_PROP(11, PF_GROUP1)
		ADD_DIALOGUE_PROP(12, PF_GROUP1)
		ADD_DIALOGUE_PROP(13, PF_GROUP1)
		ADD_DIALOGUE_PROP(14, PF_GROUP1)
		ADD_DIALOGUE_PROP(15, PF_GROUP1)
		ADD_DIALOGUE_PROP(16, PF_GROUP1)
		ADD_DIALOGUE_PROP(17, PF_GROUP1)
		ADD_DIALOGUE_PROP(18, PF_GROUP1)
		ADD_DIALOGUE_PROP(19, PF_GROUP1)
		ADD_DIALOGUE_PROP(20, PF_GROUP1)

	PROP_DEFINEGROUP(Camera, PF_GROUP2)
        ADD_BOOLPROP_FLAG(CreateCamera, LTTRUE, PF_GROUP2)
		ADD_CAMERA_PROPERTIES(PF_GROUP2)
        ADD_BOOLPROP_FLAG(IsListener, LTTRUE, PF_GROUP2)
        ADD_BOOLPROP_FLAG(OneTime, LTFALSE, PF_GROUP2 | PF_HIDDEN)

	PROP_DEFINEGROUP(KeyFramer, PF_GROUP3)
        ADD_BOOLPROP_FLAG(CreateKeyFramer, LTFALSE, PF_GROUP3)
		ADD_KEYFRAMER_PROPERTIES(PF_GROUP3)
		ADD_STRINGPROP_FLAG(ObjectName, "", PF_GROUP3 | PF_HIDDEN)
END_CLASS_DEFAULT(CinematicTrigger, BaseClass, NULL, NULL)

extern char g_tokenSpace[];

static const char* c_szCameraName		= "CAM";
static const char* c_szKeyframerName	= "KF";
static uint32 s_nKFId = 0;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::CinematicTrigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CinematicTrigger::CinematicTrigger()
{
	// Increment the id we'll assign to the camera and keyframer objects
	// we create...

	s_nKFId++;

	for (int i=0;i < MAX_CT_MESSAGES; i++)
	{
		m_fDelay[i]					= 0.0f;
        m_hstrDialogue[i]           = LTNULL;
        m_hstrWhoPlaysDialogue[i]   = LTNULL;
        m_hstrTargetName[i]         = LTNULL;
        m_hstrMessageName[i]        = LTNULL;
		m_hstrWhoPlaysDecisions[i]	= LTNULL;
        m_hstrDecisions[i]          = LTNULL;
        m_hstrReplies[i]            = LTNULL;
        m_hstrRepliesTarget[i]      = LTNULL;
        m_hstrRepliesMsg[i]         = LTNULL;
		m_bWindow[i]				= LTFALSE;
	}

    m_bCanSkip          = LTTRUE;
    m_bOn               = LTFALSE;
    m_bNotified         = LTFALSE;
	m_nCurMessage		= 0;
    m_bAllWindows       = LTFALSE;
	m_bLeaveCameraOn	= LTFALSE;
	m_byDecision		= 0;
	m_byLastReply		= 0;


    m_hCurSpeaker       = LTNULL;
    m_hLastSpeaker      = LTNULL;
    m_hCamera           = LTNULL;
    m_hKeyFramer        = LTNULL;

	m_bRemoveBadAI		= LTFALSE;
    m_bStartOn          = LTFALSE;
    m_bOneTimeOnly      = LTTRUE;
    m_bCreateCamera     = LTFALSE;
    m_bCreateKeyFramer  = LTFALSE;

	m_fNextDialogueStart = -1.0f;

    m_hstrCleanUpTriggerTarget  = LTNULL;
    m_hstrCleanUpTriggerMsg     = LTNULL;

    m_bDialogueDone             = LTFALSE;
    m_hstrDialogueDoneTarget    = LTNULL;
    m_hstrDialogueDoneMsg       = LTNULL;
    m_hstrDialogueStartTarget   = LTNULL;
    m_hstrDialogueStartMsg      = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::~CinematicTrigger()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

CinematicTrigger::~CinematicTrigger()
{
    if (!g_pLTServer) return;

	if (m_hCamera)
	{
        g_pLTServer->RemoveObject(m_hCamera);
        m_hCamera = LTNULL;
	}

	if (m_hKeyFramer)
	{
        g_pLTServer->RemoveObject(m_hKeyFramer);
        m_hKeyFramer = LTNULL;
	}

	for (int i=0; i < MAX_CT_MESSAGES; i++)
	{
		if (m_hstrWhoPlaysDialogue[i])
		{
            g_pLTServer->FreeString(m_hstrWhoPlaysDialogue[i]);
		}

		if (m_hstrTargetName[i])
		{
            g_pLTServer->FreeString(m_hstrTargetName[i]);
		}

		if (m_hstrMessageName[i])
		{
            g_pLTServer->FreeString(m_hstrMessageName[i]);
		}

		if (m_hstrWhoPlaysDecisions[i])
		{
            g_pLTServer->FreeString(m_hstrWhoPlaysDecisions[i]);
		}

		if (m_hstrDecisions[i])
		{
            g_pLTServer->FreeString(m_hstrDecisions[i]);
		}

		if (m_hstrReplies[i])
		{
            g_pLTServer->FreeString(m_hstrReplies[i]);
		}

		if (m_hstrRepliesTarget[i])
		{
            g_pLTServer->FreeString(m_hstrRepliesTarget[i]);
		}

		if (m_hstrRepliesMsg[i])
		{
            g_pLTServer->FreeString(m_hstrRepliesMsg[i]);
		}
	}

	if (m_hstrCleanUpTriggerTarget)
	{
        g_pLTServer->FreeString(m_hstrCleanUpTriggerTarget);
	}

	if (m_hstrCleanUpTriggerMsg)
	{
        g_pLTServer->FreeString(m_hstrCleanUpTriggerMsg);
	}

	if (m_hstrDialogueDoneTarget)
	{
        g_pLTServer->FreeString(m_hstrDialogueDoneTarget);
	}

	if (m_hstrDialogueDoneMsg)
	{
        g_pLTServer->FreeString(m_hstrDialogueDoneMsg);
	}

	if (m_hstrDialogueStartTarget)
	{
        g_pLTServer->FreeString(m_hstrDialogueStartTarget);
	}

	if (m_hstrDialogueStartMsg)
	{
        g_pLTServer->FreeString(m_hstrDialogueStartMsg);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL CinematicTrigger::ReadProp(ObjectCreateStruct *pStruct)
{
    if (!g_pLTServer) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("CreateCamera", &genProp) == LT_OK)
	{
		m_bCreateCamera = genProp.m_Bool;
		if (m_bCreateCamera)
		{
			CreateCamera(pStruct);
		}
	}

    if (g_pLTServer->GetPropGeneric("CreateKeyFramer", &genProp) == LT_OK)
	{
		m_bCreateKeyFramer = genProp.m_Bool;
		if (m_bCreateKeyFramer)
		{
			CreateKeyFramer(pStruct);
		}
	}

    if (g_pLTServer->GetPropGeneric("AllWindows", &genProp) == LT_OK)
	{
		m_bAllWindows = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("LeaveCameraOnWhenDone", &genProp) == LT_OK)
	{
		m_bLeaveCameraOn = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("CanSkip", &genProp) == LT_OK)
	{
		m_bCanSkip = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("StartOn", &genProp) == LT_OK)
	{
		m_bStartOn = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("RemoveBadAI", &genProp) == LT_OK)
	{
		m_bRemoveBadAI = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("OneTimeOnly", &genProp) == LT_OK)
	{
		m_bOneTimeOnly = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("CleanUpTriggerTarget", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
             m_hstrCleanUpTriggerTarget = g_pLTServer->CreateString(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("CleanUpTriggerMsg", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
             m_hstrCleanUpTriggerMsg = g_pLTServer->CreateString(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("DialogueDoneTriggerTarget", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
             m_hstrDialogueDoneTarget = g_pLTServer->CreateString(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("DialogueDoneTriggerMsg", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
             m_hstrDialogueDoneMsg = g_pLTServer->CreateString(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("DialogueStartTriggerTarget", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
             m_hstrDialogueStartTarget = g_pLTServer->CreateString(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("DialogueStartTriggerMsg", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
             m_hstrDialogueStartMsg = g_pLTServer->CreateString(genProp.m_String);
	}

	for (int i=0; i < MAX_CT_MESSAGES; i++)
	{
		char key[40];

		sprintf(key, "Delay%d", i+1);
        if (g_pLTServer->GetPropGeneric(key, &genProp) == LT_OK)
		{
			m_fDelay[i] = genProp.m_Float;
		}

		sprintf(key, "Dialogue%d", i+1);
        if (g_pLTServer->GetPropGeneric(key, &genProp) == LT_OK)
		{
			if(genProp.m_String[0])
                m_hstrDialogue[i] = g_pLTServer->CreateString(genProp.m_String);
		}

		sprintf(key, "WhoPlaysDialogue%d", i+1);
        if (g_pLTServer->GetPropGeneric(key, &genProp) == LT_OK)
		{
			if (genProp.m_String[0])
                m_hstrWhoPlaysDialogue[i] = g_pLTServer->CreateString(genProp.m_String);
		}

		sprintf(key, "StartDialogueTriggerTarget%d", i+1);
        if (g_pLTServer->GetPropGeneric(key, &genProp) == LT_OK)
		{
			if (genProp.m_String[0])
                 m_hstrTargetName[i] = g_pLTServer->CreateString(genProp.m_String);
		}

		sprintf(key, "StartDialogueTriggerMsg%d", i+1);
        if (g_pLTServer->GetPropGeneric(key, &genProp) == LT_OK)
		{
			if (genProp.m_String[0])
                 m_hstrMessageName[i] = g_pLTServer->CreateString(genProp.m_String);
		}

		sprintf(key, "WhoPlaysDecisions%d", i+1);
        if (g_pLTServer->GetPropGeneric(key, &genProp) == LT_OK)
		{
			if (genProp.m_String[0])
                 m_hstrWhoPlaysDecisions[i] = g_pLTServer->CreateString(genProp.m_String);
		}

		sprintf(key, "Decisions%d", i+1);
        if (g_pLTServer->GetPropGeneric(key, &genProp) == LT_OK)
		{
			if (genProp.m_String[0])
                 m_hstrDecisions[i] = g_pLTServer->CreateString(genProp.m_String);
		}

		sprintf(key, "Replies%d", i+1);
        if (g_pLTServer->GetPropGeneric(key, &genProp) == LT_OK)
		{
			if (genProp.m_String[0])
                 m_hstrReplies[i] = g_pLTServer->CreateString(genProp.m_String);
		}

		sprintf(key, "RepliesTriggerTarget%d", i+1);
        if (g_pLTServer->GetPropGeneric(key, &genProp) == LT_OK)
		{
			if (genProp.m_String[0])
                 m_hstrRepliesTarget[i] = g_pLTServer->CreateString(genProp.m_String);
		}

		sprintf(key, "RepliesTriggerMsg%d", i+1);
        if (g_pLTServer->GetPropGeneric(key, &genProp) == LT_OK)
		{
			if (genProp.m_String[0])
                 m_hstrRepliesMsg[i] = g_pLTServer->CreateString(genProp.m_String);
		}

		if(m_bAllWindows)
		{
			m_bWindow[i] = TRUE;
		}
		else
		{
			sprintf(key, "Window%d", i+1);
            if (g_pLTServer->GetPropGeneric(key, &genProp) == LT_OK)
			{
				m_bWindow[i] = genProp.m_Bool;
			}
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::Update
//
//	PURPOSE:	Handle Update
//
// ----------------------------------------------------------------------- //

LTBOOL CinematicTrigger::Update()
{
    if (!g_pLTServer) return LTFALSE;

    SetNextUpdate(m_hObject, 0.001f);

    LTBOOL bDialogueDone = !m_bDialogueDone ? !UpdateDialogue() : m_bDialogueDone;

	// If we created a camera and it is done end the cinematic...

	if (m_bCreateCamera)
	{
		if (!m_hCamera)
		{
			HandleOff();
		}
		else
		{
            uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hCamera);
			if ( !(dwUsrFlags & USRFLG_CAMERA_LIVE) )
			{
				HandleOff();
			}
		}
	}

	// If the dialog is done and the keyframer is finished end the cinematic...

	if (bDialogueDone)
	{
		if (!m_bDialogueDone)
		{
            m_bDialogueDone = LTTRUE;

			// Send trigger...

			if (m_hstrDialogueDoneTarget && m_hstrDialogueDoneMsg)
			{
                SendTriggerMsgToObjects(this, g_pLTServer->GetStringData(m_hstrDialogueDoneTarget), g_pLTServer->GetStringData(m_hstrDialogueDoneMsg));
			}
		}

		if (m_hKeyFramer)
		{
            KeyFramer* pKeyFramer = (KeyFramer*)g_pLTServer->HandleToObject(m_hKeyFramer);
			if (!pKeyFramer || pKeyFramer->m_bFinished)
			{
				HandleOff();
			}
		}
		else
		{
			if (!m_hCamera)
			{
				HandleOff();
			}
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::UpdateDialogue
//
//	PURPOSE:	Handle update the dialogue.  Return false when
//				dialogue is done playing.
//
// ----------------------------------------------------------------------- //

LTBOOL CinematicTrigger::UpdateDialogue()
{
    if (!g_pLTServer) return LTFALSE;

	// If we haven't yet done so, let all the cinematic participants know
	// they're under CinematicTrigger control

	if ( !m_bNotified )
	{
		for ( uint32 iWho = 0 ; iWho < MAX_CT_MESSAGES; iWho++ )
		{
			if ( m_hstrWhoPlaysDialogue[iWho] )
			{
				HOBJECT hWho;
				if ( LT_OK == FindNamedObject(m_hstrWhoPlaysDialogue[iWho], hWho) )
				{
					if ( IsKindOf(hWho, "CAI") )
					{
						CAI* pAI = (CAI*)g_pLTServer->HandleToObject(hWho);
						pAI->LinkCinematicTrigger(m_hObject);
					}
				}
			}
		}

        m_bNotified = LTTRUE;
	}

	// Now update...

    LTFLOAT fTime = g_pLTServer->GetTime();

	// See if we are playing a dialogue...
	BOOL bDone = FALSE;

	if (m_hCurSpeaker)
	{
		// If sound is done, stop it and wait for new sound...
        CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(m_hCurSpeaker);
		if (pChar)
		{
			bDone = !pChar->IsPlayingDialogue();
		}
	}

	if (bDone)
	{
		// Send message for our last reply (since the dialog is now done)...

		if (m_byLastReply)
		{
			SendReplyMessage(m_byLastReply);
		}

		m_byLastReply = m_byDecision;
		m_byDecision  = 0;

		// Clear our speaker...

        g_pLTServer->BreakInterObjectLink(m_hObject, m_hCurSpeaker);
        m_hCurSpeaker = LTNULL;

		if(m_byLastReply)
		{
			return StartDialogue(m_byLastReply);
		}

		m_nCurMessage++;

		if (m_nCurMessage < MAX_CT_MESSAGES)
		{
			m_fNextDialogueStart = fTime + m_fDelay[m_nCurMessage];
		}
		else
		{
            return LTFALSE;
		}
	}



	if (!m_hCurSpeaker)
	{
		// See if we're done...

		if (m_nCurMessage >= MAX_CT_MESSAGES || !m_hstrDialogue[m_nCurMessage])
		{
            return LTFALSE;
		}

		// Start next sound...

		if (m_fNextDialogueStart >= 0.0f && m_fNextDialogueStart <= fTime)
		{
			return StartDialogue();
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CinematicTrigger::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (!Update())
			{
                g_pLTServer->RemoveObject(m_hObject);
                return LTFALSE;
			}
		}
		break;

		case MID_PRECREATE:
		{
            uint32 dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			if ((DWORD)fData == PRECREATE_WORLDFILE || (DWORD)fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				if (m_bStartOn)
				{
					HandleOn();
				}
				else
				{
					// Wait for a trigger message...

                    SetNextUpdate(m_hObject, 0.0f);
				}
			}
		}
		break;

		case MID_LINKBROKEN :
		{
			HandleLinkBroken((HOBJECT)pData);
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

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 CinematicTrigger::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER :
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			TriggerMsg(hSender, szMsg);
		}
		break;

		default : break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::TriggerMsg()
//
//	PURPOSE:	Process cinematic trigger messages
//
// --------------------------------------------------------------------------- //

void CinematicTrigger::TriggerMsg(HOBJECT hSender, const char *pMsg)
{
	if (!pMsg) return;

    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon) return;

	// ConParse does not destroy szMsg, so this is safe
	ConParse parse;
	parse.Init((char*)pMsg);

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			if (_stricmp(parse.m_Args[0], "ON") == 0)
			{
				HandleOn();
			}
			else if (_stricmp(parse.m_Args[0], "OFF") == 0)
			{
				HandleOff();
			}
			else if (_stricmp(parse.m_Args[0], "SKIP") == 0)
			{
				if (m_bCanSkip)
				{
					HandleOff();
				}
			}
			else if (_stricmp(parse.m_Args[0], "CAMERA") == 0)
			{
				if (m_hCamera && parse.m_nArgs > 1 && parse.m_Args[1])
				{
					// Copy all arguments to g_tokenspace...

					sprintf(g_tokenSpace, "%s", parse.m_Args[1]);
					for (int i=2; i < parse.m_nArgs && parse.m_Args[i]; i++)
					{
						strcat(g_tokenSpace, " ");
						strcat(g_tokenSpace, parse.m_Args[i]);
					}

					// Send camera the message...

					SendTriggerMsgToObject(this, m_hCamera, FALSE, g_tokenSpace);
				}
			}
			else if (_stricmp(parse.m_Args[0], "KEYFRAMER") == 0)
			{
				if (m_hKeyFramer && parse.m_nArgs > 1 && parse.m_Args[1])
				{
					// Copy all arguments to g_tokenspace...

					sprintf(g_tokenSpace, "%s", parse.m_Args[1]);
					for (int i=2; i < parse.m_nArgs && parse.m_Args[i]; i++)
					{
						strcat(g_tokenSpace, " ");
						strcat(g_tokenSpace, parse.m_Args[i]);
					}

					// Send keyframer the message...

					SendTriggerMsgToObject(this, m_hKeyFramer, FALSE, g_tokenSpace);
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::CreateCamera
//
//	PURPOSE:	Create the camera object
//
// ----------------------------------------------------------------------- //

void CinematicTrigger::CreateCamera(ObjectCreateStruct *pStruct)
{
	if (m_hCamera || !pStruct) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	VEC_COPY(theStruct.m_Pos, pStruct->m_Pos);
    theStruct.m_Rotation = pStruct->m_Rotation;

	// Give it the same name as us so the keyframer can move us around...

	sprintf(theStruct.m_Name, "%s_%s%d", pStruct->m_Name, c_szCameraName, s_nKFId);

    HCLASS hClass = g_pLTServer->GetClass("Camera");
    Camera* pCamera = (Camera*) g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pCamera) return;

	pCamera->ReadProps();

	m_hCamera = pCamera->m_hObject;
    g_pLTServer->CreateInterObjectLink(m_hObject, m_hCamera);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::CreateKeyFramer
//
//	PURPOSE:	Create the key framer object
//
// ----------------------------------------------------------------------- //

void CinematicTrigger::CreateKeyFramer(ObjectCreateStruct *pStruct)
{
	if (m_hKeyFramer || !pStruct) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	VEC_COPY(theStruct.m_Pos, pStruct->m_Pos);
    theStruct.m_Rotation = pStruct->m_Rotation;

	sprintf(theStruct.m_Name, "%s_%s%d", pStruct->m_Name, c_szKeyframerName, s_nKFId);

    HCLASS hClass = g_pLTServer->GetClass("KeyFramer");
    KeyFramer* pKeyFramer = (KeyFramer*) g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pKeyFramer) return;

	pKeyFramer->ReadProps();

	m_hKeyFramer = pKeyFramer->m_hObject;
    g_pLTServer->CreateInterObjectLink(m_hObject, m_hKeyFramer);

	// Make the keyframer's ObjectName that of the camera...

	char szCamName[255];
	sprintf(szCamName, "%s_%s%d", pStruct->m_Name, c_szCameraName, s_nKFId);
	pKeyFramer->SetObjectName(szCamName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::HandleOn
//
//	PURPOSE:	Handle turning on the cinematic trigger
//
// ----------------------------------------------------------------------- //

void CinematicTrigger::HandleOn()
{
    m_bOn = LTTRUE;

	if (m_bRemoveBadAI)
	{
		RemoveBadAI();
	}

	// Turn on the camera...

	if (m_hCamera)
	{
		SendTriggerMsgToObject(this, m_hCamera, FALSE, "ON");
	}

	// Turn on the keyframer...

	if (m_hKeyFramer)
	{
		SendTriggerMsgToObject(this, m_hKeyFramer, FALSE, "ON");
	}


	// Make sure we're starting fresh...

	m_nCurMessage	= 0;
	m_byDecision	= 0;
	m_byLastReply	= 0;
    m_bDialogueDone = LTFALSE;
    m_fNextDialogueStart = g_pLTServer->GetTime() + m_fDelay[0];

    SetNextUpdate(m_hObject, 0.001f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::HandleOff
//
//	PURPOSE:	Handle turning off the cinematic trigger
//
// ----------------------------------------------------------------------- //

void CinematicTrigger::HandleOff()
{
	if (!m_bOn) return;

	for ( uint32 iWho = 0 ; iWho < MAX_CT_MESSAGES; iWho++ )
	{
		if ( m_hstrWhoPlaysDialogue[iWho] )
		{
			HOBJECT hWho;
			if ( LT_OK == FindNamedObject(m_hstrWhoPlaysDialogue[iWho], hWho) )
			{
				if ( IsKindOf(hWho, "CAI") )
				{
					CAI* pAI = (CAI*)g_pLTServer->HandleToObject(hWho);
					pAI->UnlinkCinematicTrigger(m_hObject);
				}
			}
		}
	}

    m_bOn = LTFALSE;

	// If we have a current speaker, make sure he is done talking...

	if (m_hCurSpeaker)
	{
        CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(m_hCurSpeaker);
		if (pChar)
		{
			pChar->StopDialogue();
		}

		// Clear our speaker...

        g_pLTServer->BreakInterObjectLink(m_hObject, m_hCurSpeaker);
        m_hCurSpeaker = LTNULL;
	}

	// Clear out our last speaker

	if (m_hLastSpeaker)
	{
        CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(m_hLastSpeaker);
		if (pChar)
		{
			pChar->StopDialogue(TRUE);
		}
        g_pLTServer->BreakInterObjectLink(m_hObject, m_hLastSpeaker);
        m_hLastSpeaker = LTNULL;
	}


	// Send the clean up trigger message...

	if (m_hstrCleanUpTriggerTarget && m_hstrCleanUpTriggerMsg)
	{
        SendTriggerMsgToObjects(this, g_pLTServer->GetStringData( m_hstrCleanUpTriggerTarget ), g_pLTServer->GetStringData( m_hstrCleanUpTriggerMsg ));
	}

	// Turn off the camera...

	if (m_hCamera && !m_bLeaveCameraOn)
	{
		SendTriggerMsgToObject(this, m_hCamera, FALSE, "OFF");
	}

	// Turn off the keyframer...

	if (m_hKeyFramer)
	{
		SendTriggerMsgToObject(this, m_hKeyFramer, FALSE, "OFF");
	}

    SetNextUpdate(m_hObject, 0.0f);

	if (m_bOneTimeOnly)
	{
		// Can't get rid of object if we're leaving the camera on ;)...

		if (!m_hCamera || !m_bLeaveCameraOn)
		{
			g_pLTServer->RemoveObject(m_hObject);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::HandleLinkBroken
//
//	PURPOSE:	Handle broken link messages
//
// ----------------------------------------------------------------------- //

void CinematicTrigger::HandleLinkBroken(HOBJECT hLink)
{
	if (hLink)
	{
		if (hLink == m_hCurSpeaker)
		{
            m_hCurSpeaker = LTNULL;
		}
		if (hLink == m_hCamera)
		{
            m_hCamera = LTNULL;
		}
		if (hLink == m_hKeyFramer)
		{
            m_hKeyFramer = LTNULL;
		}
		if (hLink == m_hLastSpeaker)
		{
            m_hLastSpeaker = LTNULL;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::SendMessage
//
//	PURPOSE:	Sends the CinematicTrigger message
//
// ----------------------------------------------------------------------- //

void CinematicTrigger::SendMessage()
{
	if (m_nCurMessage < MAX_CT_MESSAGES)
	{
		if (m_hstrTargetName[m_nCurMessage] && m_hstrMessageName[m_nCurMessage])
		{
            SendTriggerMsgToObjects(this, g_pLTServer->GetStringData( m_hstrTargetName[m_nCurMessage] ), g_pLTServer->GetStringData( m_hstrMessageName[m_nCurMessage] ));
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::SendReplyMessage
//
//	PURPOSE:	Sends message based on reply
//
// ----------------------------------------------------------------------- //

void CinematicTrigger::SendReplyMessage(int nReply)
{
	if (m_nCurMessage >= MAX_CT_MESSAGES) return;

    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon) return;

	CString	csTarget;
	CString csMsg;
	ConParse parse;

    char* pMsg = g_pLTServer->GetStringData(m_hstrRepliesTarget[m_nCurMessage]);
	if(!pMsg) return;

	parse.Init(pMsg);

    int i;
    for(i=0;i<nReply;i++)
	{
		if(pCommon->Parse(&parse) == LT_OK)
		{
			if(i == (nReply - 1))
			{
				if (parse.m_nArgs > 0 && parse.m_Args[0])
				{
					csTarget = parse.m_Args[0];
				}
				break;
			}
		}
	}

    pMsg = g_pLTServer->GetStringData(m_hstrRepliesMsg[m_nCurMessage]);
	if(!pMsg) return;

	parse.Init(pMsg);

	for(i=0;i<nReply;i++)
	{
		if(pCommon->Parse(&parse) == LT_OK)
		{
			if(i == (nReply - 1))
			{
				if (parse.m_nArgs > 0 && parse.m_Args[0])
				{
					csMsg = parse.m_Args[0];
				}
				break;
			}
		}
	}

	if (!csTarget.IsEmpty() && !csMsg.IsEmpty())
	{
		SendTriggerMsgToObjects(this, (char *)(LPCSTR)csTarget, (char *)(LPCSTR)csMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::StartDialogue
//
//	PURPOSE:	Start the appropriate Dialogue (return true if a Dialogue is played)
//
// ----------------------------------------------------------------------- //

LTBOOL CinematicTrigger::StartDialogue(int nDecision)
{
	if(m_nCurMessage >= MAX_CT_MESSAGES)
        return LTFALSE;

	CString csDialogue;
	CString	csTarget;
	CString csChar;
    uint8 byMood = 0;

	if(nDecision)
	{
        char* pMsg = g_pLTServer->GetStringData(m_hstrReplies[m_nCurMessage]);
		if(!pMsg) return FALSE;

        ILTCommon* pCommon = g_pLTServer->Common();
		if (!pCommon) return FALSE;

		ConParse parse;
		parse.Init(pMsg);

		for(int i=0;i<nDecision;i++)
		{
			if(pCommon->Parse(&parse) == LT_OK)
			{
				if (parse.m_nArgs < 2)
				{
					if (parse.m_nArgs < 1 || stricmp(parse.m_Args[0], "NONE") != 0)
					{
                        g_pLTServer->CPrint("Cinematic Trigger - ERROR - Not enough replies for the amount of decisions!");
						TRACE("Cinematic Trigger - ERROR - Not enough replies for the amount of decisions!\n");
						return FALSE;
					}
				}
				else if(i == (nDecision - 1))
				{
					csTarget = parse.m_Args[0];
					if(csTarget.Compare("Cinematic") == 0)
					{
						//HandleOff(); - Should be done by reply message?

						// Force the dialogue to be done...
						m_nCurMessage = MAX_CT_MESSAGES;

						// We have another cinematic to play, turn it on...
						SendTriggerMsgToObjects(this, parse.m_Args[1], "ON");
						return FALSE;
					}
					else
					{
						// Look at the second argument to determine if we have a
						// character override
						if((parse.m_Args[1][0] < '0') && (parse.m_Args[1][0] > '9'))
						{
							// We have a character override
							csChar = parse.m_Args[1];
							csDialogue = parse.m_Args[2];
							// Check for a mood
							if(parse.m_nArgs == 4)
							{
								// char charoverride id mood
								byMood = atoi(parse.m_Args[3]);
							}
						}
						else
						{
							// We don't have a character override
							csDialogue = parse.m_Args[1];
							// check for a mood
							if(parse.m_nArgs == 3)
							{
								// char id mood
								byMood = atoi(parse.m_Args[2]);
							}
						}
					}
				}
			}
		}
	}
	else
	{
		// Send the message here

		SendMessage();

		// Check for an initial message

		if (m_nCurMessage == 0)
		{
			if (m_hstrDialogueStartTarget && m_hstrDialogueStartMsg)
			{
                SendTriggerMsgToObjects(this, g_pLTServer->GetStringData(m_hstrDialogueStartTarget), g_pLTServer->GetStringData(m_hstrDialogueStartMsg));
			}
		}

        csDialogue = g_pLTServer->GetStringData(m_hstrDialogue[m_nCurMessage]);
        csTarget = g_pLTServer->GetStringData(m_hstrWhoPlaysDialogue[m_nCurMessage]);
	}

	if (csDialogue.IsEmpty())
	{
		// Make sure the reply messages are sent, even if we aren't going to
		// do any dialogue...

		if (nDecision)
		{
			SendReplyMessage(nDecision);
		}

        return LTFALSE;
	}

	const char *szCharOverride = NULL;
	if(csChar.IsEmpty())
	{
		int nSpace = csTarget.Find(' ');
		if(nSpace != -1)
		{
			// There is a space in the name, therefore we have a character override
			csChar = csTarget.Right(csTarget.GetLength()-nSpace-1);
			csTarget = csTarget.Left(nSpace);
			if(!csChar.IsEmpty())
				szCharOverride = csChar;
		}
	}
	else
	{
		szCharOverride = csChar;
	}


    HOBJECT hObj = !csTarget.IsEmpty() ? PlayedBy((char *)(LPCSTR)csTarget) : LTNULL;
	if (hObj)
	{
        CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(hObj);
		if (pChar)
		{
			char *szDecisions = NULL;
			if (!nDecision && m_hstrDecisions[m_nCurMessage])
            {
				szDecisions = g_pLTServer->GetStringData(m_hstrDecisions[m_nCurMessage]);
			}

			// See if there's a windowed message next
			int nNext = m_nCurMessage + 1;
			//BOOL bStayOpen = (szDecisions || ((nNext < MAX_CT_MESSAGES) && m_hstrDialogue[nNext] && m_bWindow[nNext] && (m_fDelay[nNext] == 0.0f)));
			//BOOL bWindow = m_bWindow[m_nCurMessage];

            BOOL bStayOpen = LTFALSE;
			BOOL bWindow = (BOOL)szDecisions;

			pChar->PlayDialogue((char *)(LPCSTR)csDialogue,this,bWindow,bStayOpen,szCharOverride,szDecisions,byMood);

			SetupLinks(hObj);

            return LTTRUE;
		}
	}
    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::SetupLinks
//
//	PURPOSE:	Setup links to our speakers
//
// ----------------------------------------------------------------------- //

void CinematicTrigger::SetupLinks(HOBJECT hObj)
{
	if (m_hCurSpeaker && (m_hCurSpeaker != hObj))
	{
        g_pLTServer->BreakInterObjectLink(m_hObject, m_hCurSpeaker);
	}

	m_hCurSpeaker = hObj;

	if (hObj)
	{
        g_pLTServer->CreateInterObjectLink(m_hObject, m_hCurSpeaker);
	}

	if (m_hLastSpeaker && (m_hLastSpeaker != hObj))
	{
        g_pLTServer->BreakInterObjectLink(m_hObject, m_hLastSpeaker);
	}

	m_hLastSpeaker = hObj;

	if (hObj)
	{
        g_pLTServer->CreateInterObjectLink(m_hObject, m_hLastSpeaker);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::PlayedBy
//
//	PURPOSE:	Get the object that plays the Dialogue
//
// ----------------------------------------------------------------------- //

HOBJECT CinematicTrigger::PlayedBy(char *pszName)
{
    if (!g_pLTServer || m_nCurMessage >= MAX_CT_MESSAGES) return m_hObject;
    if (!pszName) return LTNULL;

	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
    g_pLTServer->FindNamedObjects(pszName,objArray);

	int numObjects = objArray.NumObjects();

	for (int i = 0; i < numObjects; i++)
	{
		HOBJECT hObject = objArray.GetObject(i);

		if (IsActor(hObject))
		{
			return hObject;
		}
	}

    return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::DialogueFinished
//
//	PURPOSE:	Handle dialogue being finished
//
// ----------------------------------------------------------------------- //

LTBOOL CinematicTrigger::DialogueFinished(uint8 byDecision, uint32 dwDecisionID)
{
    if (!m_hCurSpeaker) return LTFALSE;

    CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(m_hCurSpeaker);
    if(!pChar) return LTFALSE;

	pChar->StopDialogue();

	// Set our current decision...

	if (byDecision)
	{
		m_byDecision = byDecision;
	}
	else
	{
        return LTTRUE;
	}


	// Determine what character should play the decision dialogue...

	HOBJECT hObj = FindWhoPlaysDecision(byDecision);

	if (hObj)
	{
		SetupLinks(hObj);
        pChar = (CCharacter*)g_pLTServer->HandleToObject(hObj);

		if (pChar)
		{
			pChar->PlayDialogue(dwDecisionID, this);
            return LTTRUE;
		}
	}

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::FindWhoPlaysDecision
//
//	PURPOSE:	Find the character that should play the decision dialogue
//
// ----------------------------------------------------------------------- //

HOBJECT CinematicTrigger::FindWhoPlaysDecision(uint8 byDecision)
{
	char* pCharName = DEFAULT_PLAYERNAME;

	if (byDecision > 0)
	{
		char* pMsg = g_pLTServer->GetStringData(m_hstrWhoPlaysDecisions[m_nCurMessage]);
		if (pMsg)
		{
	 		ConParse parse;
			parse.Init(pMsg);

			// Since the names will be seperated by semi-colons we need
			// to parse until we get to the correct name...

			LTBOOL bFound = LTTRUE;
			for (int i=1; i <= byDecision; i++)
			{
				if (g_pLTServer->Common()->Parse(&parse) != LT_OK)
				{
					bFound = LTFALSE;
					break;
				}
			}

			// Parse should be on the correct name now...

			if (bFound && parse.m_nArgs > 0)
			{
				if (parse.m_Args[0])
				{
					pCharName = parse.m_Args[0];
				}
			}
		}
	}

	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
    g_pLTServer->FindNamedObjects(pCharName, objArray);

	if (objArray.NumObjects())
	{
		return objArray.GetObject(0);
	}

	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CinematicTrigger::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hCurSpeaker);
	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hLastSpeaker);
	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hCamera);
	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hKeyFramer);

	pServerDE->WriteToMessageByte(hWrite, m_nCurMessage);

	pServerDE->WriteToMessageByte(hWrite, m_bOn);
	pServerDE->WriteToMessageByte(hWrite, m_bNotified);
	pServerDE->WriteToMessageByte(hWrite, m_bCreateCamera);
	pServerDE->WriteToMessageByte(hWrite, m_bCreateKeyFramer);
	pServerDE->WriteToMessageByte(hWrite, m_bStartOn);
	pServerDE->WriteToMessageByte(hWrite, m_bRemoveBadAI);
	pServerDE->WriteToMessageByte(hWrite, m_bOneTimeOnly);
	pServerDE->WriteToMessageByte(hWrite, m_bCanSkip);
	pServerDE->WriteToMessageByte(hWrite, m_bAllWindows);
	pServerDE->WriteToMessageByte(hWrite, m_bDialogueDone);
	pServerDE->WriteToMessageByte(hWrite, m_bLeaveCameraOn);

	pServerDE->WriteToMessageFloat(hWrite, m_fNextDialogueStart);

	for (int i=0; i < MAX_CT_MESSAGES; i++)
	{
		pServerDE->WriteToMessageFloat(hWrite, m_fDelay[i]);
		pServerDE->WriteToMessageHString(hWrite, m_hstrDialogue[i]);
		pServerDE->WriteToMessageHString(hWrite, m_hstrWhoPlaysDialogue[i]);
		pServerDE->WriteToMessageHString(hWrite, m_hstrTargetName[i]);
		pServerDE->WriteToMessageHString(hWrite, m_hstrMessageName[i]);
		pServerDE->WriteToMessageHString(hWrite, m_hstrWhoPlaysDecisions[i]);
		pServerDE->WriteToMessageHString(hWrite, m_hstrDecisions[i]);
		pServerDE->WriteToMessageHString(hWrite, m_hstrReplies[i]);
		pServerDE->WriteToMessageHString(hWrite, m_hstrRepliesTarget[i]);
		pServerDE->WriteToMessageHString(hWrite, m_hstrRepliesMsg[i]);
		pServerDE->WriteToMessageByte(hWrite, m_bWindow[i]);
	}

	pServerDE->WriteToMessageHString(hWrite, m_hstrCleanUpTriggerTarget);
	pServerDE->WriteToMessageHString(hWrite, m_hstrCleanUpTriggerMsg);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDialogueDoneTarget);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDialogueDoneMsg);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDialogueStartTarget);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDialogueStartMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CinematicTrigger::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hCurSpeaker);
	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hLastSpeaker);
	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hCamera);
	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hKeyFramer);

	m_nCurMessage		= pServerDE->ReadFromMessageByte(hRead);

    m_bOn               = (LTBOOL) pServerDE->ReadFromMessageByte(hRead);
    m_bNotified         = (LTBOOL) pServerDE->ReadFromMessageByte(hRead);
    m_bCreateCamera     = (LTBOOL) pServerDE->ReadFromMessageByte(hRead);
    m_bCreateKeyFramer  = (LTBOOL) pServerDE->ReadFromMessageByte(hRead);
    m_bStartOn          = (LTBOOL) pServerDE->ReadFromMessageByte(hRead);
    m_bRemoveBadAI      = (LTBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bOneTimeOnly      = (LTBOOL) pServerDE->ReadFromMessageByte(hRead);
    m_bCanSkip          = (LTBOOL) pServerDE->ReadFromMessageByte(hRead);
    m_bAllWindows       = (LTBOOL) pServerDE->ReadFromMessageByte(hRead);
    m_bDialogueDone     = (LTBOOL) pServerDE->ReadFromMessageByte(hRead);
    m_bLeaveCameraOn    = (LTBOOL) pServerDE->ReadFromMessageByte(hRead);

	m_fNextDialogueStart	= pServerDE->ReadFromMessageFloat(hRead);

	for (int i=0; i < MAX_CT_MESSAGES; i++)
	{
		m_fDelay[i]					= pServerDE->ReadFromMessageFloat(hRead);
		m_hstrDialogue[i]			= pServerDE->ReadFromMessageHString(hRead);
		m_hstrWhoPlaysDialogue[i]	= pServerDE->ReadFromMessageHString(hRead);
		m_hstrTargetName[i]			= pServerDE->ReadFromMessageHString(hRead);
		m_hstrMessageName[i]		= pServerDE->ReadFromMessageHString(hRead);
		m_hstrWhoPlaysDecisions[i]	= pServerDE->ReadFromMessageHString(hRead);
		m_hstrDecisions[i]			= pServerDE->ReadFromMessageHString(hRead);
		m_hstrReplies[i]			= pServerDE->ReadFromMessageHString(hRead);
		m_hstrRepliesTarget[i]		= pServerDE->ReadFromMessageHString(hRead);
		m_hstrRepliesMsg[i]			= pServerDE->ReadFromMessageHString(hRead);
		m_bWindow[i]				= pServerDE->ReadFromMessageByte(hRead);
	}

	m_hstrCleanUpTriggerTarget		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrCleanUpTriggerMsg			= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDialogueDoneTarget		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDialogueDoneMsg			= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDialogueStartTarget		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDialogueStartMsg			= pServerDE->ReadFromMessageHString(hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::IsActor
//
//	PURPOSE:	Is the object an actor
//
// ----------------------------------------------------------------------- //

LTBOOL CinematicTrigger::IsActor(HOBJECT hObject)
{
	return IsCharacter(hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::RemoveBadAI
//
//	PURPOSE:	Remove all the Bad AI in the game
//
// ----------------------------------------------------------------------- //

void CinematicTrigger::RemoveBadAI()
{
    HOBJECT hObj   = g_pLTServer->GetNextObject(LTNULL);
    HCLASS  hClass = g_pLTServer->GetClass("CAI");

	// Remove all the ai objects...

    LTBOOL bRemove = LTFALSE;

    HOBJECT hRemoveObj = LTNULL;
	while (hObj)
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
			hRemoveObj = hObj;
		}

        hObj = g_pLTServer->GetNextObject(hObj);

		if (hRemoveObj)
		{
            CCharacter* pChar = (CCharacter*) g_pLTServer->HandleToObject(hRemoveObj);
			if (pChar && pChar->GetCharacterClass() == BAD)
			{
				SendTriggerMsgToObject(this, pChar->m_hObject, LTFALSE, "REMOVE");
			}

            hRemoveObj = LTNULL;
		}
	}


    hObj = g_pLTServer->GetNextInactiveObject(LTNULL);
    hRemoveObj = LTNULL;
	while (hObj)
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
			hRemoveObj = hObj;
		}

        hObj = g_pLTServer->GetNextInactiveObject(hObj);

		if (hRemoveObj)
		{
            CCharacter* pChar = (CCharacter*) g_pLTServer->HandleToObject(hRemoveObj);
			if (pChar && pChar->GetCharacterClass() == BAD)
			{
				SendTriggerMsgToObject(this, pChar->m_hObject, LTFALSE, "REMOVE");
			}

            hRemoveObj = LTNULL;
		}
	}
}