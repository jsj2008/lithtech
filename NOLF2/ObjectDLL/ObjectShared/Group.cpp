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
#include "ParsedMsg.h"

LINKFROM_MODULE( Group );

#pragma force_active on
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
END_CLASS_DEFAULT_FLAGS(Group, GameBaseLite, NULL, NULL, CF_CLASSONLY)
#pragma force_active off

//
// Register this class with the command mgr plugin but flag it to ignore all messages sent to it.
// This eliminates unwanted spam in the DEdit debug window.
//

CMDMGR_BEGIN_REGISTER_CLASS( Group )
CMDMGR_END_REGISTER_CLASS_FLAGS( Group, GameBase, CMDMGR_CF_MSGIGNORE )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Group::Group
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Group::Group() :
	GameBaseLite(false),
	m_nNumTargets(0)
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
	for (uint32 i=0; i < m_nNumTargets; i++)
	{
		if (m_hstrObjectNames[i])
		{
            g_pLTServer->FreeString(m_hstrObjectNames[i]);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Group::PreCreate
//
//	PURPOSE:	Handle pre create
//
// ----------------------------------------------------------------------- //

bool Group::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!GameBaseLite::ReadProp(pStruct))
		return false;

	char buf[100];
	GenericProp gProp;

	m_nNumTargets = 0;
	for (int i=0; i < MAX_GROUP_TARGETS; i++)
	{
		sprintf(buf, "Object%d", i+1);
        if (g_pLTServer->GetPropGeneric(buf, &gProp) == LT_OK)
		{
			if (gProp.m_String[0])
			{
                m_hstrObjectNames[i] = g_pLTServer->CreateString(gProp.m_String);
				m_nNumTargets = i + 1;
			}
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Group::Load
//
//	PURPOSE:	Handle loading object
//
// ----------------------------------------------------------------------- //

void Group::Load(ILTMessage_Read *pMsg)
{
	GameBaseLite::Load( pMsg );

	LOAD_DWORD(m_nNumTargets);

	for (uint32 i=0; i < m_nNumTargets; i++)
	{
		LOAD_HSTRING(m_hstrObjectNames[i]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Group::Save
//
//	PURPOSE:	Handle saving object
//
// ----------------------------------------------------------------------- //

void Group::Save(ILTMessage_Write *pMsg)
{
	GameBaseLite::Save( pMsg );
	
	SAVE_DWORD(m_nNumTargets);

	for (uint32 i=0; i < m_nNumTargets; i++)
	{
		SAVE_HSTRING(m_hstrObjectNames[i]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Group::HandleTrigger
//
//	PURPOSE:	Handle sending off messages to our objects
//
// ----------------------------------------------------------------------- //

bool Group::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	// Put the message back in a string
	char aMsgBuff[256];
	cMsg.ReCreateMsg(aMsgBuff, sizeof(aMsgBuff), 0);

	const char* pName;
	for (uint32 i=0; i < m_nNumTargets; i++)
	{
		if (m_hstrObjectNames[i])
		{
            pName = g_pLTServer->GetStringData(m_hstrObjectNames[i]);
			if (pName && pName[0])
			{
				SendTriggerMsgToObjects(this, pName, aMsgBuff);
			}
		}
	}

	return true;
}