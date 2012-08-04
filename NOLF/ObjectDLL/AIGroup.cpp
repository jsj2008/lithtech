// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIGroup.h"
#include "AI.h"
#include "AISense.h"
#include "DeathScene.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "CommandMgr.h"
#include "CVarTrack.h"

// Console var trackers

BEGIN_CLASS(AIGroup)

	ADD_STRINGPROP(Name, "")

	ADD_STRINGPROP_FLAG(Member1, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Member2, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Member3, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Member4, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Member5, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Member6, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Member7, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Member8, "", PF_OBJECTLINK)

END_CLASS_DEFAULT_FLAGS(AIGroup, BaseClass, NULL, NULL, 0)

// Statics

const static char s_szMessageRemove[] = "REMOVE";
const static char s_szMessageSend[] = "SEND ";
const static int s_nMessageSendLen = strlen(s_szMessageSend);
const static char s_szMessageAdd[] = "ADD ";
const static int s_nMessageAddLen = strlen(s_szMessageAdd);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIGroup::AIGroup()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

AIGroup::AIGroup() : BaseClass()
{
    m_hstrName = LTNULL;

	{for ( int iMember = 0 ; iMember < kMaxMembers ; iMember++ )
	{
        m_ahstrMembers[iMember] = LTNULL;
        m_ahMembers[iMember] = LTNULL;
	}}

	m_cMembers = 0;

    m_bFirstUpdate = LTTRUE;

	m_pAISenseRecorder = FACTORY_NEW(CAISenseRecorder);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIGroup::~AIGroup()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

AIGroup::~AIGroup()
{
	FREE_HSTRING(m_hstrName);

	for ( int iMember = 0 ; iMember < kMaxMembers ; iMember++ )
	{
		FREE_HSTRING(m_ahstrMembers[iMember]);

		if ( m_ahMembers[iMember] )
		{
			Unlink(m_ahMembers[iMember]);
            m_ahMembers[iMember] = LTNULL;
		}
	}

	FACTORY_DELETE(m_pAISenseRecorder);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIGroup::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void AIGroup::Save(HMESSAGEWRITE hWrite)
{
	m_pAISenseRecorder->Save(hWrite);

	SAVE_BOOL(m_bFirstUpdate);

	SAVE_HSTRING(m_hstrName);

	{for ( int iMember = 0 ; iMember < kMaxMembers ; iMember++ )
	{
		SAVE_HSTRING(m_ahstrMembers[iMember]);
		SAVE_HOBJECT(m_ahMembers[iMember]);
	}}

	SAVE_INT(m_cMembers);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIGroup::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void AIGroup::Load(HMESSAGEREAD hRead)
{
	m_pAISenseRecorder->Load(hRead);

	LOAD_BOOL(m_bFirstUpdate);

	LOAD_HSTRING(m_hstrName);

	{for ( int iMember = 0 ; iMember < kMaxMembers ; iMember++ )
	{
		LOAD_HSTRING(m_ahstrMembers[iMember]);
		LOAD_HOBJECT(m_ahMembers[iMember]);

		if ( m_ahMembers[iMember] )
		{
            CAI* pAI = (CAI*)g_pLTServer->HandleToObject(m_ahMembers[iMember]);
			pAI->SetGroup(this);
		}
	}}

	LOAD_INT(m_cMembers);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIGroup::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 AIGroup::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
            g_pLTServer->SetNextUpdate(m_hObject, c_fUpdateDelta);
		}
		break;

		case MID_UPDATE:
		{
			Update();

            g_pLTServer->SetNextUpdate(m_hObject, c_fUpdateDelta);
		}
		break;

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
			Save((HMESSAGEWRITE)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData);
		}
		break;

		case MID_LINKBROKEN :
		{
			HandleBrokenLink((HOBJECT)pData);
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIGroup::HandleBrokenLink
//
//	PURPOSE:	Handles a link to a member or threat being broken
//
// ----------------------------------------------------------------------- //

void AIGroup::HandleBrokenLink(HOBJECT hObject)
{
	m_pAISenseRecorder->HandleBrokenLink(hObject);

	for ( int iMember = 0 ; iMember < kMaxMembers ; iMember++ )
	{
		if ( m_ahMembers[iMember] == hObject )
		{
			// This member is going away

            m_ahMembers[iMember] = LTNULL;
			m_cMembers--;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIGroup::HandleSense
//
//	PURPOSE:	Handles being told about a sense by an AI
//
// ----------------------------------------------------------------------- //

void AIGroup::HandleSense(CAI* pAI, CAISense* pAISense)
{
	if ( m_pAISenseRecorder->IsRecorded(pAISense) )
	{
		// We already know about it. Ignore it.
	}
	else
	{
		m_pAISenseRecorder->Record(pAISense);

		for ( int iMember = 0 ; iMember < kMaxMembers ; iMember++ )
		{
            CAI* pAIMember = m_ahMembers[iMember] ? (CAI*)g_pLTServer->HandleToObject(m_ahMembers[iMember]) : LTNULL;

			if ( !pAIMember )
			{
				continue;
			}
			else if ( pAIMember == pAI )
			{
				// If it's the AI telling us about it, he can do his individual reaction.

				HSTRING hstrReaction = pAIMember->GetIndividualSenseReaction(pAISense);
                const char* szReaction = g_pLTServer->GetStringData(hstrReaction);

#ifndef _FINAL
                g_pLTServer->CPrint("%s doing individual reaction \"%s\" to %s", g_pLTServer->GetStringData(m_ahstrMembers[iMember]), szReaction, pAISense->GetName());
#endif

                pAIMember->DoReaction(hstrReaction, pAISense, LTTRUE);
			}
			else
			{
				// DON'T do the group reaction if this AI is already alert

				if ( pAIMember->IsAlert() ) continue;

				// Otherwise, the AI should do his group reaction.

				HSTRING hstrReaction = pAIMember->GetGroupSenseReaction(pAISense);
                const char* szReaction = g_pLTServer->GetStringData(hstrReaction);

#ifndef _FINAL
                g_pLTServer->CPrint("%s doing group reaction \"%s\" to %s", g_pLTServer->GetStringData(m_ahstrMembers[iMember]), szReaction, pAISense->GetName());
#endif

                pAIMember->DoReaction(hstrReaction, pAISense, LTFALSE);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIGroup::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 AIGroup::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_TRIGGER:
		{
            const char* szMessage = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			TriggerMsg(hSender, szMessage);
		}
		break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	AIGroup::TriggerMsg()
//
//	PURPOSE:	Handler for AIGroup trigger messages.
//
// --------------------------------------------------------------------------- //

void AIGroup::TriggerMsg(HOBJECT hSender, const char* szMessage)
{
	if ( !_strnicmp(szMessage, s_szMessageSend, s_nMessageSendLen) )
	{
		SendMessageToAllMembers(&szMessage[s_nMessageSendLen]);
	}
	else if ( !_strnicmp(szMessage, s_szMessageAdd, s_nMessageAddLen) )
	{
		HOBJECT hObject;
		if ( LT_OK == FindNamedObject(&szMessage[s_nMessageAddLen], hObject) )
		{
			if ( !IsKindOf(hObject, "CAI") ) return;
			if ( kMaxMembers == m_cMembers ) return;

            CAI* pAI = (CAI*)g_pLTServer->HandleToObject(hObject);
			pAI->SetGroup(this);

			m_ahMembers[m_cMembers++] = pAI->GetObject();
			Link(pAI->GetObject());
		}
	}
	else if ( !_stricmp(szMessage, s_szMessageRemove) )
	{
		SendMessageToAllMembers(s_szMessageRemove);
        g_pLTServer->RemoveObject(m_hObject);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	AIGroup::SendMessageToAllMembers()
//
//	PURPOSE:	Sends a messsage to all group members
//
// --------------------------------------------------------------------------- //

void AIGroup::SendMessageToAllMembers(const char* szMessage)
{
	for ( int iMember = 0 ; iMember < kMaxMembers ; iMember++ )
	{
		if ( m_ahMembers[iMember] )
		{
			SendMixedTriggerMsgToObject(this, m_ahMembers[iMember], LTFALSE, szMessage);
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	AIGroup::SendMessageToMember()
//
//	PURPOSE:	Sends a messsage to a group member
//
// --------------------------------------------------------------------------- //

void AIGroup::SendMessageToMember(const char* szMessage, HOBJECT hMember)
{
	if (!hMember) return;

	SendMixedTriggerMsgToObject(this, hMember, LTFALSE, szMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIGroup::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL AIGroup::ReadProp(ObjectCreateStruct *pInfo)
{
    if (!pInfo) return LTFALSE;

	GenericProp genProp;

    if ( g_pLTServer->GetPropGeneric( "Name", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrName = g_pLTServer->CreateString( genProp.m_String );

	{for ( int iMember = 0 ; iMember < kMaxMembers ; iMember++ )
	{
		char szProp[64];
		sprintf(szProp, "Member%d", iMember+1);

        if ( g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK )
			if ( genProp.m_String[0] )
                m_ahstrMembers[iMember] = g_pLTServer->CreateString( genProp.m_String );
	}}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIGroup::FirstUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void AIGroup::FirstUpdate()
{
	CharacterClass cc;
	LTBOOL bFirst = LTTRUE;

	for ( int iMember = 0 ; iMember < kMaxMembers ; iMember++ )
	{
		if ( m_ahstrMembers[iMember] )
		{
			HOBJECT hObject;
			if ( LT_OK == FindNamedObject(m_ahstrMembers[iMember], hObject) )
			{
				if ( !IsKindOf(hObject, "CAI") )
				{
					g_pLTServer->CPrint("AIGroup ''%s'': %s is not an AI!!!", g_pLTServer->GetObjectName(m_hObject), g_pLTServer->GetStringData(m_ahstrMembers[iMember]));
					continue;
				}

                CAI* pAI = (CAI*)g_pLTServer->HandleToObject(hObject);
				pAI->SetGroup(this);

				if ( bFirst )
				{
					cc = pAI->GetCharacterClass();
					bFirst = LTFALSE;
				}
				else
				{
					if ( cc != pAI->GetCharacterClass() )
					{
						g_pLTServer->CPrint("AIGroup ''%s'': alignments are inconsistent", g_pLTServer->GetObjectName(m_hObject));
					}
				}

				m_ahMembers[m_cMembers++] = pAI->GetObject();
				Link(pAI->GetObject());
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIGroup::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

void AIGroup::Update()
{
	if ( m_bFirstUpdate )
	{
		FirstUpdate();
        m_bFirstUpdate = LTFALSE;
	}

	m_pAISenseRecorder->Update();
}
