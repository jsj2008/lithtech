// ----------------------------------------------------------------------- //
//
// MODULE  : GameStartPoint.cpp
//
// PURPOSE : GameStartPoint - Definition
//
// CREATED : 9/30/97
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "GameStartPoint.h"
#include "iltserver.h"
#include "GameServerShell.h"
#include "ServerUtilities.h"
#include "SurfaceFunctions.h"
#include "PlayerObj.h"
#include "ParsedMsg.h"
#include "GameModeMgr.h"
#include "TeamMgr.h"

LINKFROM_MODULE( GameStartPoint );

BEGIN_CLASS(GameStartPoint)
	ADD_STRINGPROP_FLAG(ModelTemplate, "", PF_STATICLIST, "This is a dropdown menu that allows you to choose the model player model that will be loaded at the beginning of the level.")
	ADD_STRINGPROP_FLAG(PhysicsModel, "", PF_STATICLIST, "This dropdown menu allows you to choose what physics model is loaded at the beginning of the level.")
	ADD_COMMANDPROP_FLAG(Command, "", PF_NOTIFYCHANGE, "This is a Command string that will be processed when the player spawns in at this start point.")
	ADD_BOOLPROP_FLAG(Locked, false, 0, "This flag is used to not allow players to spawn at this start point.  Sending the GameStartPoint object LOCK and UNLOCK messages will allow or disallow players from spawning at this point.")
	ADD_BOOLPROP_FLAG(SendCommandOnRespawn, false, 0, "If this flag is set to true the Command string will be sent any time the player spawns at this point otherwise it will only be sent the first time.")
	ADD_STRINGPROP_FLAG(Team, "NoTeam", PF_STATICLIST, "This is a dropdown that allows you to set which team can spawn at this start point while playing a team game.")
	ADD_BOOLPROP_FLAG( StartPoint, true, 0, "If this flag is true players will be allowed to spawn in at this start point at the very begining of the level.  If this flag is false players won't be able to use this start point for their very first spawn into a level." )
	ADD_BOOLPROP_FLAG( SpawnPoint, true, 0, "If this flag is true players will be allowed to respawn at this start point at any point during the game after the first spawn.  If this flag is false players will not be able use this start point for respawning." )
END_CLASS_FLAGS_PLUGIN(GameStartPoint, GameBase, 0, CGameStartPointPlugin, "Defines points that the player can start at in a level.")



CMDMGR_BEGIN_REGISTER_CLASS( GameStartPoint )

	ADD_MESSAGE( LOCK,		1,	NULL,	MSG_HANDLER( GameStartPoint, HandleLockMsg ),	"LOCK", "Locks the GameStarPoint so no player can enter the world using this GameStarPoint", "msg GameStarPoint LOCK" )
	ADD_MESSAGE( UNLOCK,	1,	NULL,	MSG_HANDLER( GameStartPoint, HandleUnlockMsg ),	"UNLOCK", "Unlocks the GameStarPoint", "msg GameStarPoint UNLOCK" )
	ADD_MESSAGE( TEAM,		2,	NULL,	MSG_HANDLER( GameStartPoint, HandleTeamMsg ),	"TEAM <0, 1, -1>", "Specifies which team can spawn at this start point", "msg GameStartPoint (TEAM 1)" )

CMDMGR_END_REGISTER_CLASS( GameStartPoint, GameBase )

GameStartPoint::StartPointList GameStartPoint::m_lstStartPoints;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::GameStartPoint
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

GameStartPoint::GameStartPoint()
:	GameBase				( OT_NORMAL ),
	m_hModel				( NULL ),
	m_ePPhysicsModel		( PPM_NORMAL ),
	m_sName					( ),
	m_sCommand				( ),
	m_bLocked				( false ),
	m_bSendCommandOnRespawn	( false ),
	m_nTeamID				( INVALID_TEAM ),
	m_bStartPoint			( true ),
	m_bSpawnPoint			( true )
{
	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::~GameStartPoint
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

GameStartPoint::~GameStartPoint()
{
	// Take the start point off the list...
	
	StartPointList::iterator iter = m_lstStartPoints.begin();
	while( iter != m_lstStartPoints.end() )
	{
		if( *iter == this )
		{
			m_lstStartPoints.erase( iter );
			break;
		}

		++iter;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 GameStartPoint::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
    uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct *pStruct = (ObjectCreateStruct*)pData;

			if (pStruct)
			{
				if (fData == PRECREATE_WORLDFILE)
				{
					ReadProp(&pStruct->m_cProperties);
				}
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				SetNextUpdate(UPDATE_NEVER);

				//choose a negative last use time to randomize the order of use
				m_fLastUse = GetRandom(-1000.0f,-10.0f);
			}

			// Once created add this start point to the global list.

			m_lstStartPoints.push_back( this );

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

	return dwRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool GameStartPoint::ReadProp(const GenericPropList *pProps)
{
	// Determine which template to use...

	const char *pszModelTemplate = pProps->GetString( "ModelTemplate", "" );
	m_hModel = g_pModelsDB->GetModelByRecordName( pszModelTemplate );

	const char *pszPhysicsModel = pProps->GetString( "PhysicsModel", "" );
	m_ePPhysicsModel = GetPlayerPhysicsModelFromPropertyName( pszPhysicsModel );
	

	m_sName					= pProps->GetString( "Name", "" );
	m_sCommand				= pProps->GetCommand( "Command", "" );

	m_bLocked				= pProps->GetBool( "Locked", m_bLocked );
	m_bSendCommandOnRespawn	= pProps->GetBool( "SendCommandOnRespawn", m_bSendCommandOnRespawn );
	

	// Get the team the startpoint belongs to, but only when in a team game...
	
	if( GameModeMgr::Instance( ).m_grbUseTeams )
	{
		const char *pszTeam = pProps->GetString( "Team", "" );
		m_nTeamID = TeamStringToTeamId( pszTeam );
	}
	else
	{
		m_nTeamID = INVALID_TEAM;
	}

	m_bStartPoint = pProps->GetBool( "StartPoint", m_bStartPoint );
	m_bSpawnPoint = pProps->GetBool( "SpawnPoint", m_bSpawnPoint );

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::HandleLockMsg()
//
//	PURPOSE:	Handle a LOCK message...
//
// ----------------------------------------------------------------------- //

void GameStartPoint::HandleLockMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_bLocked = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::HandleUnlockMsg()
//
//	PURPOSE:	Handle a UNLOCK message...
//
// ----------------------------------------------------------------------- //

void GameStartPoint::HandleUnlockMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_bLocked = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::HandleTeamMsg()
//
//	PURPOSE:	Handle a TEAM message...
//
// ----------------------------------------------------------------------- //

void GameStartPoint::HandleTeamMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	int nTeamId = atoi( crParsedMsg.GetArg( 1 ));
	if( nTeamId < MAX_TEAMS )
	{
		ASSERT( nTeamId == ( uint8 )nTeamId );
		m_nTeamID = ( uint8 )LTCLAMP( nTeamId, 0, 255 );
	}
	else
	{
		m_nTeamID = INVALID_TEAM;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void GameStartPoint::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_CHARSTRING( g_pModelsDB->GetRecordName( m_hModel ));
	SAVE_STDSTRING( m_sName );
	SAVE_bool( m_bLocked );
	SAVE_bool( m_bSendCommandOnRespawn );
	SAVE_INT( m_ePPhysicsModel );
	SAVE_STDSTRING( m_sCommand );
	SAVE_DOUBLE( m_fLastUse );
	SAVE_BYTE( m_nTeamID );
	SAVE_bool( m_bStartPoint );
	SAVE_bool( m_bSpawnPoint );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void GameStartPoint::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	char szValue[256];
	LOAD_CHARSTRING( szValue, LTARRAYSIZE( szValue ));
	m_hModel = g_pModelsDB->GetModelByRecordName( szValue );
	LOAD_STDSTRING( m_sName );
	LOAD_bool( m_bLocked );
	LOAD_bool( m_bSendCommandOnRespawn );
	LOAD_INT_CAST( m_ePPhysicsModel, PlayerPhysicsModel );
	LOAD_STDSTRING( m_sCommand );
	LOAD_DOUBLE( m_fLastUse );
	LOAD_BYTE( m_nTeamID );
	LOAD_bool( m_bStartPoint );
	LOAD_bool( m_bSpawnPoint );
}


LTRESULT CGameStartPointPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	// Handle Template...

	if( LTStrIEquals( "ModelTemplate", szPropName ))
	{
		uint32 cModels = g_pModelsDB->GetNumModels();
		LTASSERT(cMaxStrings >= cModels, "TODO: Add description here");
		for ( uint32 iModel = 0 ; iModel < cModels ; iModel++ )
		{
			// exit out early if we can't hold any more strings
			if( *pcStrings >= cMaxStrings )
				return LT_OK;

			// Only list model templates in WorldEdit that have been
			// set in modelbutes.txt to AIOnly = FALSE
			ModelsDB::HMODEL hModel = g_pModelsDB->GetModel( iModel );
			if( g_pModelsDB->ShowGameStartPoint( hModel ))
			{
				LTStrCpy( aszStrings[(*pcStrings)++], g_pModelsDB->GetRecordName( hModel ), cMaxStringLength );
			}
		}

		return LT_OK;
	}

	// Handle physics model...

    if( LTStrIEquals( "PhysicsModel", szPropName ))
	{
		for (int i=PPM_FIRST; i < PPM_NUM_MODELS; i++)
		{
			LTASSERT(cMaxStrings > (*pcStrings) + 1, "TODO: Add description here");
			if( *pcStrings < cMaxStrings )
			{
				LTStrCpy( aszStrings[(*pcStrings)++], GetPropertyNameFromPlayerPhysicsModel((PlayerPhysicsModel)i), cMaxStringLength );
			}
			else
			{
				// exit out early if we can't hold any more strings
				return LT_OK;
			}
		}

		return LT_OK;
	}

	// Handle team...
	if( LTStrIEquals( "Team", szPropName ))
	{
		TeamPopulateEditStringList( aszStrings, pcStrings, cMaxStrings, cMaxStringLength );
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

LTRESULT CGameStartPointPlugin::PreHook_PropChanged( const char *szObjName,
													 const char *szPropName, 
													 const int  nPropType, 
													 const GenericProp &gpPropValue,
													 ILTPreInterface *pInterface,
													 const char *szModifiers )
{
	// Only our command is marked for change notification so just send it to the CommandMgr..

	if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
												szPropName, 
												nPropType, 
												gpPropValue,
												pInterface,
												szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
