// ----------------------------------------------------------------------- //
//
// MODULE  : StoryMode.cpp
//
// PURPOSE : Implementation of the StoryMode object
//
// CREATED : 10/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "StoryMode.h"
#include "MsgIDs.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"

LINKFROM_MODULE( StoryMode );


BEGIN_CLASS(StoryMode)
	ADD_COMMANDPROP_FLAG(CleanUpCommand, "", PF_NOTIFYCHANGE, "A command that will be executed whenever the StoryMode is stopped by the OFF command.")
	ADD_COMMANDPROP_FLAG(AbortCommand, "", PF_NOTIFYCHANGE, "A command that will always executed whenever the StoryMode is stopped by the user cancelling.")
	ADD_BOOLPROP(RemoveWhenDone, true, "By default the StoryMode object is removed when it is turned off.  If you want to reuse a StoryMode set the RemoveWhenDone property to FALSE.")
	ADD_BOOLPROP(CanSkip, true, "If set to false, the player will not be able to skip past this story.")
END_CLASS(StoryMode, GameBase, "An object to handle turning on and off story mode")


CMDMGR_BEGIN_REGISTER_CLASS( StoryMode )

	ADD_MESSAGE( ON,		1,	NULL,	MSG_HANDLER( StoryMode, HandleOnMsg ),		"ON ", "Places the player into StoryMode.", "msg StoryMode ON" )
	ADD_MESSAGE( OFF,		1,	NULL,	MSG_HANDLER( StoryMode, HandleOffMsg ),		"OFF", "Takes the player out of story mode, sends the CleanUpCommand if one was specified, and removes the object if RemoveWhenDone is set to true.", "msg StoryMode OFF" )

CMDMGR_END_REGISTER_CLASS( StoryMode, GameBase )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StoryMode::StoryMode()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

StoryMode::StoryMode()
:	GameBase			( OT_NORMAL ),
	m_bOn				( false ),
	m_bCanSkip		( true ),
	m_sCleanUpCmd		( ),
	m_sAbortCmd		( ),
	m_bRemoveWhenDone	( true )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StoryMode::~StoryMode()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

StoryMode::~StoryMode()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StoryMode::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 StoryMode::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
	case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp(&((ObjectCreateStruct*)pData)->m_cProperties);
			}
		}
		break;

	case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
		}
		break;

	case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData);
		}
		break;

	case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData);

			uint32 dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);


			return dwRet;
		}
		break;

	default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StoryMode::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void StoryMode::ReadProp(const GenericPropList *pProps)
{
	m_sCleanUpCmd		= pProps->GetCommand( "CleanUpCommand", "" );
	m_sAbortCmd			= pProps->GetCommand( "AbortCommand", "" );
	m_bRemoveWhenDone	= pProps->GetBool( "RemoveWhenDone", m_bRemoveWhenDone );
	m_bCanSkip			= pProps->GetBool( "CanSkip", m_bCanSkip );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StoryMode::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void StoryMode::InitialUpdate()
{
	// Must be triggered on...

	// Make sure our sfx message is told to the client.
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAG_FORCECLIENTUPDATE );

	SetNextUpdate(UPDATE_NEVER);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	StoryMode::HandleOnMsg()
//
//	PURPOSE:	Handle a ON message...
//
// --------------------------------------------------------------------------- //

void StoryMode::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_bOn = true;

	CPlayerObj* pPlayer = g_pCharacterMgr->FindPlayer();
	if( pPlayer )
	{
		pPlayer->StartStoryMode(m_hObject, m_bCanSkip, false);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	StoryMode::HandleOffMsg()
//
//	PURPOSE:	Handle a OFF message...
//
// --------------------------------------------------------------------------- //

void StoryMode::HandleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if (!m_bOn)
		return;
	CPlayerObj* pPlayer = g_pCharacterMgr->FindPlayer();
	if( pPlayer )
	{
		bool bEnded = pPlayer->EndStoryMode();
		if (bEnded)
		{
			End(false);
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	StoryMode::End()
//
//	PURPOSE:	End story mode...
//
// --------------------------------------------------------------------------- //

void StoryMode::End(bool bAbort)
{
	if (!m_bOn)
		return;
	m_bOn = false;
	if (bAbort)
	{
		if( !m_sAbortCmd.empty() )
		{
			g_pCmdMgr->QueueCommand( m_sAbortCmd.c_str(), m_hObject, m_hObject );
		}
	}
	else
	{
		if( !m_sCleanUpCmd.empty() )
		{
			g_pCmdMgr->QueueCommand( m_sCleanUpCmd.c_str(), m_hObject, m_hObject );
		}
	}

	if (m_bRemoveWhenDone)
	{
		g_pLTServer->RemoveObject(m_hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StoryMode::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void StoryMode::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	SAVE_STDSTRING( m_sCleanUpCmd );
	SAVE_STDSTRING( m_sAbortCmd );
	SAVE_BOOL(m_bRemoveWhenDone);
	SAVE_BOOL(m_bOn);
	SAVE_BOOL(m_bCanSkip);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StoryMode::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void StoryMode::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	LOAD_STDSTRING( m_sCleanUpCmd );
	LOAD_STDSTRING( m_sAbortCmd );
	LOAD_BOOL(m_bRemoveWhenDone);
	LOAD_BOOL(m_bOn);
	LOAD_BOOL(m_bCanSkip);

}

