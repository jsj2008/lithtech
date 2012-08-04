// ----------------------------------------------------------------------- //
//
// MODULE  : KeyframeToRigidBody.cpp
//
// PURPOSE : KeyframeToRigidBody - Implementation
//
// CREATED : 04/14/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "KeyframeToRigidBody.h"

#define MAX_KTRB_OBJECTS	50

LINKFROM_MODULE( KeyframeToRigidBody );

BEGIN_CLASS(KeyframeToRigidBody)
	ADD_COMMANDPROP_FLAG(Command, "", PF_NOTIFYCHANGE, "This is the command string that is sent on the next frame after the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object1, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object2, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object3, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object4, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object5, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object6, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object7, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object8, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object9, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object10, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object11, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object12, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object13, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object14, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object15, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object16, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object17, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object18, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object19, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object20, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object21, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object22, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object23, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object24, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object25, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object26, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object27, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object28, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object29, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object30, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object31, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object32, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object33, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object34, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object35, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object36, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object37, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object38, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object39, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object40, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object41, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object42, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object43, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object44, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object45, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object46, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object47, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object48, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object49, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
	ADD_STRINGPROP_FLAG(Object50, "", PF_OBJECTLINK, "This is the name of an object that will become rigid body when the KeyframeToRigidBody object is turned on.")
END_CLASS_FLAGS(KeyframeToRigidBody, GameBase, 0, "KeyframeToRigidBody objects are used to turn multiple objects at once to rigid body when it receives the ON message.  On the next frame of processing (after a physics update has occured) the command specified will then be sent.")

CMDMGR_BEGIN_REGISTER_CLASS( KeyframeToRigidBody )
	ADD_MESSAGE( ON, 1,	NULL,	MSG_HANDLER( KeyframeToRigidBody, HandleOnMsg ),	"ON", "Turn all the objects specified to Rigid Body, wait a frame, then send our command.", "msg KeyframeToRigidBody ON" )
CMDMGR_END_REGISTER_CLASS( KeyframeToRigidBody, GameBase )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyframeToRigidBody::KeyframeToRigidBody
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

KeyframeToRigidBody::KeyframeToRigidBody() :
	GameBase		( OT_NORMAL ),
	m_saObjectNames	( ),
	m_fCurrentFrameTime ( 0.0f )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyframeToRigidBody::~KeyframeToRigidBody
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

KeyframeToRigidBody::~KeyframeToRigidBody()
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	KeyframeToRigidBody::EngineMessageFn
//
//  PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //

uint32 KeyframeToRigidBody::EngineMessageFn( uint32 messageID, void *pvData, float fData )
{
	switch( messageID )
	{
		case MID_UPDATE:
		{
			// Make sure we wait a frame before sending our command...
			if ( m_fCurrentFrameTime < g_pLTServer->GetTime() )
			{
				// Send the command...
				if ( !m_sCommand.empty() )
				{
					g_pCmdMgr->QueueCommand( m_sCommand.c_str(), m_hObject, m_hObject );
				}

				SetNextUpdate( UPDATE_NEVER );
			}
			else
			{
				SetNextUpdate( UPDATE_NEXT_FRAME );
			}
		}
		break;

		case MID_PRECREATE:
		{
			if( fData == PRECREATE_WORLDFILE )
			{
				ObjectCreateStruct *pOCS = (ObjectCreateStruct*)pvData;
				ReadProp( &pOCS->m_cProperties );

				// Ensure the object will never be sent to the client.
				if( pOCS )
				{
					pOCS->m_Flags |= FLAG_NOTINWORLDTREE;
				}
			}
		}
		break;

		case MID_OBJECTCREATED:
		{
			if( fData != OBJECTCREATED_SAVEGAME )
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
	}

	return GameBase::EngineMessageFn( messageID, pvData, fData );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyframeToRigidBody::PreCreate
//
//	PURPOSE:	Handle pre create
//
// ----------------------------------------------------------------------- //

bool KeyframeToRigidBody::ReadProp(const GenericPropList *pProps)
{
	char szObject[128] = {0};

	m_sCommand = pProps->GetCommand( "Command", m_sCommand.c_str( ) );

	m_saObjectNames.reserve( MAX_KTRB_OBJECTS );
	for( uint32 nTarget = 0; nTarget < MAX_KTRB_OBJECTS; ++nTarget )
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
//	ROUTINE:	KeyframeToRigidBody::Load
//
//	PURPOSE:	Handle loading object
//
// ----------------------------------------------------------------------- //

void KeyframeToRigidBody::Load(ILTMessage_Read *pMsg)
{
	uint32 nSize;
	LOAD_DWORD( nSize );

	m_saObjectNames.clear();
	m_saObjectNames.resize( nSize );

	for( uint32 i = 0; i < nSize; ++i )
	{
		LOAD_STDSTRING( m_saObjectNames[i] );
	}

	LOAD_DOUBLE( m_fCurrentFrameTime );
	LOAD_STDSTRING( m_sCommand );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyframeToRigidBody::Save
//
//	PURPOSE:	Handle saving object
//
// ----------------------------------------------------------------------- //

void KeyframeToRigidBody::Save(ILTMessage_Write *pMsg)
{
	SAVE_DWORD( m_saObjectNames.size() );

	StringArray::iterator iter;
	for( iter = m_saObjectNames.begin(); iter != m_saObjectNames.end(); ++iter )
	{
		SAVE_STDSTRING( (*iter) );
	}

	SAVE_DOUBLE( m_fCurrentFrameTime );
	SAVE_STDSTRING( m_sCommand );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyframeToRigidBody::HandleOnMsg
//
//	PURPOSE:	Handle our on message
//
// ----------------------------------------------------------------------- //

void KeyframeToRigidBody::HandleOnMsg( HOBJECT hSender, const CParsedMsg& /* crParsedMsg */ )
{
	std::string sCmd;

	// Turn all the objects rigid body...
	StringArray::iterator iter;
	for( iter = m_saObjectNames.begin(); iter != m_saObjectNames.end(); ++iter )
	{
		if( !(*iter).empty() )
		{
			sCmd = "msg " + *iter + " (RIGIDBODY 1);";
			g_pCmdMgr->QueueCommand( sCmd.c_str(), hSender, NULL );
		}
	}

	// Turn on updates so we can process our command next frame
	// It is VERY important to process the command next frame as the whole point
	// of this object is give the physics simulation an opportunity to update before
	// processing our command.
	if ( !m_sCommand.empty() )
	{
		m_fCurrentFrameTime = g_pLTServer->GetTime();
		SetNextUpdate( UPDATE_NEXT_FRAME );
	}
}
