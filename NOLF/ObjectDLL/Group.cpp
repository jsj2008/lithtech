// ----------------------------------------------------------------------- //
//
// MODULE  : Group.cpp
//
// PURPOSE : Group - Implementation
//
// CREATED : 12/21/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Group.h"
#include "iltmessage.h"
#include "commonutilities.h"
#include "serverutilities.h"
#include "ObjectMsgs.h"

BEGIN_CLASS(Group)
	ADD_STRINGPROP_FLAG(Object1, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object2, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object3, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object4, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object5, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object6, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object7, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object8, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object9, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object10, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object11, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object12, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object13, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object14, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object15, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object16, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object17, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object18, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object19, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object20, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object21, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object22, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object23, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object24, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object25, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object26, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object27, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object28, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object29, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object30, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object31, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object32, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object33, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object34, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object35, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object36, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object37, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object38, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object39, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object40, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object41, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object42, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object43, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object44, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object45, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object46, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object47, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object48, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object49, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Object50, "", PF_OBJECTLINK)
END_CLASS_DEFAULT(Group, BaseClass, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Group::Group
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Group::Group()
{
	for (int i=0; i < MAX_GROUP_TARGETS; i++)
	{
        m_hstrObjectNames[i] = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Group::~Group
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Group::~Group()
{
	for (int i=0; i < MAX_GROUP_TARGETS; i++)
	{
		if (m_hstrObjectNames[i])
		{
            g_pLTServer->FreeString(m_hstrObjectNames[i]);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Group::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DWORD Group::EngineMessageFn(DWORD messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP || fData == PRECREATE_NORMAL)
			{
				PreCreate((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				SetNextUpdate(m_hObject, 0.0f);
			}
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DWORD)fData);
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DWORD)fData);
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Group::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DWORD Group::ObjectMessageFn(HOBJECT hSender, DWORD messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			HandleTrigger(hSender, szMsg);
		}
		break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Group::PreCreate
//
//	PURPOSE:	Handle pre create
//
// ----------------------------------------------------------------------- //

void Group::PreCreate(ObjectCreateStruct *pStruct)
{
	char buf[100];
	GenericProp gProp;

	for (int i=0; i < MAX_GROUP_TARGETS; i++)
	{
		sprintf(buf, "Object%d", i+1);
        if (g_pLTServer->GetPropGeneric(buf, &gProp) == LT_OK)
		{
			if (gProp.m_String[0])
			{
                m_hstrObjectNames[i] = g_pLTServer->CreateString(gProp.m_String);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Group::Load
//
//	PURPOSE:	Handle loading object
//
// ----------------------------------------------------------------------- //

void Group::Load(HMESSAGEREAD hRead, DWORD dwSaveFlags)
{
	if (!hRead) return;

	for (int i=0; i < MAX_GROUP_TARGETS; i++)
	{
		m_hstrObjectNames[i] = hRead->ReadHString();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Group::Save
//
//	PURPOSE:	Handle saving object
//
// ----------------------------------------------------------------------- //

void Group::Save(HMESSAGEWRITE hWrite, DWORD dwSaveFlags)
{
	if (!hWrite) return;

	for (int i=0; i < MAX_GROUP_TARGETS; i++)
	{
		hWrite->WriteHString(m_hstrObjectNames[i]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Group::HandleTrigger
//
//	PURPOSE:	Handle sending off messages to our objects
//
// ----------------------------------------------------------------------- //

void Group::HandleTrigger(HOBJECT hSender, const char *szMsg)
{
	if (!szMsg || !szMsg[0]) return;

	char* pName;
	for (int i=0; i < MAX_GROUP_TARGETS; i++)
	{
		if (m_hstrObjectNames[i])
		{
            pName = g_pLTServer->GetStringData(m_hstrObjectNames[i]);
			if (pName && pName[0])
			{
				SendTriggerMsgToObjects(this, pName, szMsg);
			}
		}
	}
}