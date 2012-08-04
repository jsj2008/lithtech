// ----------------------------------------------------------------------- //
//
// MODULE  : Group.cpp
//
// PURPOSE : Group - Implementation
//
// CREATED : 12/21/99
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "Group.h"
#include "iltmessage.h"
#include "CommonUtilities.h"
#include "ServerUtilities.h"
#include "ParsedMsg.h"

#define MAX_GROUP_TARGETS	50

LINKFROM_MODULE( Group );

BEGIN_CLASS(Group)
	ADD_STRINGPROP_FLAG(Object1, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object2, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object3, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object4, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object5, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object6, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object7, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object8, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object9, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object10, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object11, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object12, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object13, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object14, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object15, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object16, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object17, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object18, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object19, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object20, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object21, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object22, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object23, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object24, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object25, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object26, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object27, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object28, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object29, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object30, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object31, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object32, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object33, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object34, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object35, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object36, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object37, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object38, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object39, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object40, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object41, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object42, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object43, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object44, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object45, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object46, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object47, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object48, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object49, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
	ADD_STRINGPROP_FLAG(Object50, "", PF_OBJECTLINK, "This is the name of an object that will receive the command string that is sent to the Group object.")
END_CLASS_FLAGS(Group, GameBase, 0, "Group objects are used to deliver a single message to multiple objects at once. A message received by a group object will be delivered to every object listed within it.")


//
// Register this class with the command mgr plugin but flag it to ignore all messages sent to it.
// This eliminates unwanted spam in the WorldEdit debug window.
//

CMDMGR_BEGIN_REGISTER_CLASS( Group )
CMDMGR_END_REGISTER_CLASS_HANDLER( Group, GameBase, CMDMGR_CF_MSGIGNORE, MSG_HANDLER( Group, HandleAllMsgs ))


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Group::Group
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Group::Group() :
	GameBase		( OT_NORMAL ),
	m_saObjectNames	( )
{

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

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Group::EngineMessageFn
//
//  PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //

uint32 Group::EngineMessageFn( uint32 messageID, void *pvData, float fData )
{
	switch( messageID )
	{
		case MID_PRECREATE:
		{
			if( fData == PRECREATE_WORLDFILE )
			{
				ObjectCreateStruct *pOCS = (ObjectCreateStruct*)pvData;
				ReadProp( &pOCS->m_cProperties );

				// Ensure the object will never be sent to the client.
				if( pOCS )
					pOCS->m_Flags |= FLAG_NOTINWORLDTREE;
			}
		}
		break;

		case MID_OBJECTCREATED:
		{
			SetNextUpdate( UPDATE_NEVER );
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save( (ILTMessage_Write*)pvData );
		}
		break;

		case MID_LOADOBJECT:
		{
			Load( (ILTMessage_Read*)pvData );
		}
		break;
	};

	return GameBase::EngineMessageFn( messageID, pvData, fData );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Group::PreCreate
//
//	PURPOSE:	Handle pre create
//
// ----------------------------------------------------------------------- //

bool Group::ReadProp(const GenericPropList *pProps)
{
	char szObject[128] = {0};

	m_saObjectNames.reserve( MAX_GROUP_TARGETS );

	for( uint32 nTarget = 0; nTarget < MAX_GROUP_TARGETS; ++nTarget )
	{
		LTSNPrintF( szObject, ARRAY_LEN(szObject), "Object%d", nTarget + 1 );
		
		const char *pszTargetName = pProps->GetString( szObject, "" );	
		if( pszTargetName && pszTargetName[0] )
		{
			m_saObjectNames.push_back( pszTargetName );
		}
	}

	// Shrink-to-fit...
	StringArray( m_saObjectNames ).swap( m_saObjectNames );

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
	uint32 nSize;
	LOAD_DWORD( nSize );

	m_saObjectNames.clear();
	m_saObjectNames.resize( nSize );

	for( uint32 i = 0; i < nSize; ++i )
	{
		LOAD_STDSTRING( m_saObjectNames[i] );
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
	SAVE_DWORD( m_saObjectNames.size() );

	StringArray::iterator iter;
	for( iter = m_saObjectNames.begin(); iter != m_saObjectNames.end(); ++iter )
	{
		SAVE_STDSTRING( (*iter) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Group::HandleAllMsgs
//
//	PURPOSE:	Handle sending off messages to our objects
//
// ----------------------------------------------------------------------- //

void Group::HandleAllMsgs( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	char szMsgArgs[256];
	crParsedMsg.ReCreateMsg( szMsgArgs, ARRAY_LEN(szMsgArgs), 0 );

	if( !szMsgArgs[0] )
		return;
		
	for( StringArray::iterator iter = m_saObjectNames.begin(); iter != m_saObjectNames.end(); ++iter )
	{
		if( !(*iter).empty() )
		{
			const char* pszObjectName = iter->c_str();

			// This needs to be format string characters +
			// LTARRAYSIZE(szMsgArgs) + MaxObjectName characters long.
			char szMsg[10 + LTARRAYSIZE(szMsgArgs) + MAX_CS_FILENAME_LEN]; 
			LTSNPrintF(szMsg, LTARRAYSIZE(szMsg), "msg %s (%s);", pszObjectName, szMsgArgs );
			g_pCmdMgr->QueueCommand( szMsg, hSender, NULL );
		}
	}
}
