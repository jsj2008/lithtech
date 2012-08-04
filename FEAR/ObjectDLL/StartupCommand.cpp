// ----------------------------------------------------------------------- //
//
// MODULE  : StartupCommand.cpp
//
// PURPOSE : The StartupCommand implementation
//
// CREATED : 11/27/01
//
// (c) 2001-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "Stdafx.h"
	#include "StartupCommand.h"
	#include "VersionMgr.h"
	#include "ServerMissionMgr.h"

LINKFROM_MODULE( StartupCommand );

BEGIN_CLASS( StartupCommand )

	ADD_COMMANDPROP_FLAG( Command, "", PF_NOTIFYCHANGE, "Command to run when the level starts." )
	ADD_STRINGPROP_FLAG( RoundCondition, "All", PF_STATICLIST, "Startup command only fires when condition met of all, odd or even rounds.")
	ADD_BOOLPROP( SafeExecute, true, "Execute command safely.  Set to false if target of command needs to be modified before first update." )

END_CLASS_FLAGS_PLUGIN( StartupCommand, GameBase, 0, CStartupCommandPlugin, "StartupCommand objects are used to send a command immediately when the level loads for the first time." )

CMDMGR_BEGIN_REGISTER_CLASS( StartupCommand )
CMDMGR_END_REGISTER_CLASS( StartupCommand, GameBase )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CStartupCommandPlugin::PreHook_PropChanged
//
//  PURPOSE:	Make sure the Command is valid
//
// ----------------------------------------------------------------------- //

LTRESULT CStartupCommandPlugin::PreHook_PropChanged( const char *szObjName,
													 const char *szPropName,
												     const int nPropType,
												     const GenericProp &gpPropValue,
												     ILTPreInterface *pInterface,
													 const	char *szModifiers )
{
	if( LT_OK == m_CmdMgrPlugin.PreHook_PropChanged( szObjName,
														 szPropName,
														 nPropType, 
														 gpPropValue,
														 pInterface,
														 szModifiers ))
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CStartupCommandPlugin::PreHook_EditStringList
//
//  PURPOSE:	Edit string list on drop downs.
//
// ----------------------------------------------------------------------- //

LTRESULT CStartupCommandPlugin::PreHook_EditStringList(const char* szRezPath,
												   const char* szPropName,
												   char** aszStrings,
												   uint32* pcStrings,
												   const uint32 cMaxStrings,
												   const uint32 cMaxStringLength)
{

	if( LTStrIEquals( "RoundCondition", szPropName ))
	{
		ASSERT(cMaxStrings > (*pcStrings) + 3);
		LTStrCpy( aszStrings[(*pcStrings)++], "All", cMaxStringLength );
		LTStrCpy( aszStrings[(*pcStrings)++], "Odd", cMaxStringLength );
		LTStrCpy( aszStrings[(*pcStrings)++], "Even", cMaxStringLength );
		
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	StartupCommand::StartupCommand
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

StartupCommand::StartupCommand( ) :	GameBase( OT_NORMAL )
{
	m_eRoundCondition = kRoundCondition_All;
	m_bSafeExecute = true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	StartupCommand::~StartupCommand
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

StartupCommand::~StartupCommand( )
{
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	StartupCommand::EngineMessageFn
//
//  PURPOSE:	Handle message from the engine...
//
// ----------------------------------------------------------------------- //

uint32 StartupCommand::EngineMessageFn( uint32 messageID, void *pData, float fData )
{
	switch( messageID )
	{
		case MID_PRECREATE :
		{
			// Let the GameBase handle the message first

			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			ObjectCreateStruct	*pOCS = (ObjectCreateStruct*)pData;

			if( pOCS )
			{
				if( PRECREATE_WORLDFILE == fData )
				{
					ReadProps( &pOCS->m_cProperties );
				}
			}

			// Important!! - We already sent the message to the GameBase so DONT do it again.

			return dwRet;
		}
		break;

		case MID_OBJECTCREATED :
		{
			// Wait untill the first update to send the command.  This way any object that relies
			// on MID_ALLOBJECTSCREATED to finish their setup will have done so. 

			SetNextUpdate( UPDATE_NEXT_FRAME );
		}
		break;

		case MID_ALLOBJECTSCREATED:
		{
			// Check if they couldn't wait for a safe execution in MID_UPDATE.
			// This is sometimes needed if the target of the command is something
			// that has to be modified before the first update.
			if( !m_bSafeExecute )
			{
				SendCommand( );
			}
		}
		break;

		case MID_UPDATE :
		{
			SendCommand( );
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn( messageID, pData, fData );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	StartupCommand::ReadProps
//
//  PURPOSE:	Read in the property values...
//
// ----------------------------------------------------------------------- //

void StartupCommand::ReadProps( const GenericPropList *pProps )
{
	ASSERT( pProps != NULL );

	m_sStartupCmd = pProps->GetCommand( "Command", "" );

	const char *pszRoundCondition = pProps->GetString( "RoundCondition", "All" );
	if( pszRoundCondition && pszRoundCondition[0] )
	{
		if( LTStrIEquals( pszRoundCondition, "Odd" ))
		{
			m_eRoundCondition = kRoundCondition_Odd;
		}
		else if( LTStrIEquals( pszRoundCondition, "Even" ))
		{
			m_eRoundCondition = kRoundCondition_Even;
		}
		else
		{
			m_eRoundCondition = kRoundCondition_All;
		}
	}

	m_bSafeExecute = pProps->GetBool( "SafeExecute", m_bSafeExecute );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StartupCommand::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void StartupCommand::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	SAVE_CHARSTRING( m_sStartupCmd.c_str( ));
	SAVE_BYTE( m_eRoundCondition );
	SAVE_bool( m_bSafeExecute );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StartupCommand::Load
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void StartupCommand::Load(ILTMessage_Read *pMsg, uint32 dwSaveFlags)
{
	char szString[1024];
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sStartupCmd = szString;

	LOAD_BYTE_CAST( m_eRoundCondition, RoundCondition );
	LOAD_bool( m_bSafeExecute );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StartupCommand::SendCommand
//
//	PURPOSE:	Sends the command.
//
// ----------------------------------------------------------------------- //

void StartupCommand::SendCommand( )
{
	// Check if we satisfy the round condition.
	// Round 0 is considered odd, since it's the "1st" round.
	bool bSatisfiesRoundCondition = false;
	switch( m_eRoundCondition )
	{
		case kRoundCondition_All:
			bSatisfiesRoundCondition = true;
			break;
		case kRoundCondition_Odd:
			bSatisfiesRoundCondition = !( g_pServerMissionMgr->GetCurrentRound( ) & 0x1 );
			break;
		case kRoundCondition_Even:
			bSatisfiesRoundCondition = ( g_pServerMissionMgr->GetCurrentRound( ) & 0x1 );
			break;
	}

	if( bSatisfiesRoundCondition )
	{
		// Process the command and then remove us...

		if( !m_sStartupCmd.empty( ))
		{
			g_pCmdMgr->QueueCommand( m_sStartupCmd.c_str( ), m_hObject, m_hObject );
		}
	}

	g_pLTServer->RemoveObject( m_hObject );
}
