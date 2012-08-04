// ----------------------------------------------------------------------- //
//
// MODULE  : GameStartPoint.cpp
//
// PURPOSE : GameStartPoint - Definition
//
// CREATED : 9/30/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GameStartPoint.h"
#include "iltserver.h"
#include "GameServerShell.h"
#include "ServerUtilities.h"
#include "SurfaceFunctions.h"
#include "WeaponMgr.h"
#include "PlayerObj.h"
#include "ParsedMsg.h"

LINKFROM_MODULE( GameStartPoint );

extern CGameServerShell* g_pGameServerShell;

uint32 GameStartPoint::m_dwCounter = 0;

#pragma force_active on
BEGIN_CLASS(GameStartPoint)
	ADD_STRINGPROP_FLAG(ModelTemplate, "", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(PhysicsModel, "", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(Command, "", PF_NOTIFYCHANGE)
	ADD_BOOLPROP_FLAG(Locked, LTFALSE, 0)
	ADD_BOOLPROP_FLAG(SendCommandOnRespawn, LTFALSE, 0)
	ADD_STRINGPROP_FLAG(Team, "NoTeam", PF_STATICLIST)
	ADD_BOOLPROP_FLAG( StartPoint, true, 0 )
	ADD_BOOLPROP_FLAG( SpawnPoint, true, 0 )
END_CLASS_DEFAULT_FLAGS_PLUGIN(GameStartPoint, GameBase, NULL, NULL, CF_ALWAYSLOAD, CGameStartPointPlugin)
#pragma force_active off


CMDMGR_BEGIN_REGISTER_CLASS( GameStartPoint )

	CMDMGR_ADD_MSG( LOCK, 1, NULL, "LOCK" )
	CMDMGR_ADD_MSG( UNLOCK, 1, NULL, "UNLOCK" )
	CMDMGR_ADD_MSG( TEAM, 2, NULL, "TEAM <0, 1, -1>" )

CMDMGR_END_REGISTER_CLASS( GameStartPoint, GameBase )

GameStartPoint::StartPointList GameStartPoint::m_lstStartPoints;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::GameStartPoint
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

GameStartPoint::GameStartPoint() : GameBase()
{
	m_eModelId				= eModelIdInvalid;
	m_ePPhysicsModel		= PPM_NORMAL;
    m_hstrName              = LTNULL;
    m_hstrCommand		    = LTNULL;
	m_vPitchYawRoll.Init();
	m_bLocked				= false;
	m_bSendCommandOnRespawn	= false;
	m_nTeamID				= INVALID_TEAM;
	m_bStartPoint			= true;
	m_bSpawnPoint			= true;
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
	if (m_hstrName)
	{
        g_pLTServer->FreeString(m_hstrName);
	}

	if (m_hstrCommand)
	{
        g_pLTServer->FreeString(m_hstrCommand);
	}

	m_dwCounter = 0;

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

uint32 GameStartPoint::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
					ReadProp(pStruct);
				}
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				SetNextUpdate(UPDATE_NEVER);
			}

			// Once created add this start point to the global list.

			m_lstStartPoints.push_back( this );

			//choose a negative last use time to randomize the order of use
			m_fLastUse = GetRandom(-1000.0f,-10.0f);
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

LTBOOL GameStartPoint::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;

   	// Determine which template to use...

    if (g_pLTServer->GetPropGeneric("ModelTemplate", &genProp) == LT_OK)
	{
		m_eModelId = g_pModelButeMgr->GetModelId(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("PhysicsModel", &genProp) == LT_OK)
	{
		m_ePPhysicsModel = GetPlayerPhysicsModelFromPropertyName(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("Name", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrName = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("Command", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrCommand = g_pLTServer->CreateString(genProp.m_String);
		}
	}


    g_pLTServer->GetPropRotationEuler("Rotation", &m_vPitchYawRoll);

	if( g_pLTServer->GetPropGeneric( "Locked", &genProp ) == LT_OK )
	{
		m_bLocked = genProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "SendCommandOnRespawn", &genProp ) == LT_OK )
	{
		m_bSendCommandOnRespawn = genProp.m_Bool;
	}

	// Get the team the startpoint belongs to, but only when in a team game...
	
	if( IsTeamGameType() )
	{
		if( g_pLTServer->GetPropGeneric( "Team", &genProp ) == LT_OK )
		{
			if( genProp.m_String[0] )
			{
				char szTeam[32] = {0};
				for( int i = 0; i < MAX_TEAMS; ++i )
				{
					sprintf( szTeam, "Team%i", i );
					if( !_stricmp( genProp.m_String, szTeam ))
					{
						m_nTeamID = i;
					}
				}
			}
		}
	}
	else
	{
		m_nTeamID = INVALID_TEAM;
	}

	if( g_pLTServer->GetPropGeneric( "StartPoint", &genProp ) == LT_OK )
	{
		m_bStartPoint = genProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "SpawnPoint", &genProp ) == LT_OK )
	{
		m_bSpawnPoint = genProp.m_Bool;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::OnTrigger()
//
//	PURPOSE:	Process GameStartPoint trigger messages 
//
// ----------------------------------------------------------------------- //

bool GameStartPoint::OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken s_cTok_Lock( "LOCK" );
	static CParsedMsg::CToken s_cTok_UnLock( "UNLOCK" );
	static CParsedMsg::CToken s_cTok_Team( "TEAM" );

	if( cMsg.GetArg(0) == s_cTok_Lock )
	{
		m_bLocked = true;
	}
	else if( cMsg.GetArg(0) == s_cTok_UnLock )
	{
		m_bLocked = false;
	}
	else if (cMsg.GetArg(0) == s_cTok_Team)
	{
		if( cMsg.GetArgCount( ) > 1 )
		{
			uint32 nTeamId = atoi( cMsg.GetArg( 1 ));
			if( nTeamId < MAX_TEAMS )
			{
				m_nTeamID = nTeamId;
			}
			else
			{
				m_nTeamID = INVALID_TEAM;
			}

			return true;
		}
	}
	else
	{
		return false;
	}

	return true;
}


#ifndef __PSX2
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

	if ( !_strcmpi("ModelTemplate", szPropName) )
	{
		m_ModelButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings,	cMaxStringLength);

		uint32 cModels = g_pModelButeMgr->GetNumModels();
		_ASSERT(cMaxStrings >= cModels);
		for ( uint32 iModel = 0 ; iModel < cModels ; iModel++ )
		{
			// Only list model templates in DEdit that have been
			// set in modelbutes.txt to AIOnly = FALSE
			if( !g_pModelButeMgr->IsModelAIOnly((ModelId)iModel) )
			{
				strcpy(aszStrings[(*pcStrings)++], g_pModelButeMgr->GetModelName((ModelId)iModel));
			}
		}

		return LT_OK;
	}

	// Handle physics model...

    if (_stricmp("PhysicsModel", szPropName) == 0)
	{
		for (int i=PPM_FIRST; i < PPM_NUM_MODELS; i++)
		{
			_ASSERT(cMaxStrings > (*pcStrings) + 1);
			if (cMaxStrings > (*pcStrings) + 1)
			{
				strcpy(aszStrings[(*pcStrings)++], GetPropertyNameFromPlayerPhysicsModel((PlayerPhysicsModel)i));
			}
		}

		return LT_OK;
	}

	// Handle team...

	if( _stricmp( "Team", szPropName ) == 0 )
	{
		char szTeam[32] = {0};

		_ASSERT(cMaxStrings > (*pcStrings) + 1);
		strcpy( aszStrings[(*pcStrings)++], "NoTeam" );
		
		for( int i = 0; i < MAX_TEAMS; ++i )
		{
			_ASSERT(cMaxStrings > (*pcStrings) + 1);

			sprintf( szTeam, "Team%i", i );
			strcpy( aszStrings[(*pcStrings)++], szTeam );
		}

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

#endif // !__PSX2