// ----------------------------------------------------------------------- //
//
// MODULE  : GameStartPoint.cpp
//
// PURPOSE : GameStartPoint - Definition
//
// CREATED : 9/30/97
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

extern CGameServerShell* g_pGameServerShell;

uint32 GameStartPoint::m_dwCounter = 0;

namespace
{
	const char aszTeamNames[3][16] = { "Any Team", "Team 1", "Team 2" };
}

BEGIN_CLASS(GameStartPoint)
	ADD_STRINGPROP_FLAG(GameType, "", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(PlayerModel, "", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(PhysicsModel, "", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(Team, (char *)aszTeamNames[0], PF_STATICLIST)
	ADD_STRINGPROP(TriggerTarget, "")
	ADD_STRINGPROP(TriggerMessage, "")
END_CLASS_DEFAULT_FLAGS_PLUGIN(GameStartPoint, GameBase, NULL, NULL, CF_ALWAYSLOAD, CGameStartPointPlugin)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::GameStartPoint
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

GameStartPoint::GameStartPoint() : GameBase()
{
	m_eGameType				= SINGLE;
	m_ePlayerModelStyle		= eModelStyleInvalid;
	m_ePPhysicsModel		= PPM_NORMAL;
    m_hstrName              = LTNULL;
    m_hstrTriggerTarget     = LTNULL;
    m_hstrTriggerMessage    = LTNULL;
	m_vPitchYawRoll.Init();
	m_byTeam				= TEAM_AUTO;
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

	if (m_hstrTriggerTarget)
	{
        g_pLTServer->FreeString(m_hstrTriggerTarget);
	}

	if (m_hstrTriggerMessage)
	{
        g_pLTServer->FreeString(m_hstrTriggerMessage);
	}

	m_dwCounter = 0;
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

				SAFE_STRCPY(pStruct->m_Name, "GameStartPoint");
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				SetNextUpdate(0.0f);
			}

			CacheFiles();
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

    if (g_pLTServer->GetPropGeneric("GameType", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			for (int i=0; i < g_knNumGameTypes; i++)
			{
				if (_stricmp(genProp.m_String, GameTypeToString((GameType)i)) == 0)
				{
					m_eGameType = (GameType) i;
					break;
				}
			}
		}
	}

	// Determine what player model style to use...

    if (g_pLTServer->GetPropGeneric("PlayerModel", &genProp) == LT_OK)
	{
		m_ePlayerModelStyle = g_pModelButeMgr->GetModelStyleFromProperty(genProp.m_String);
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

    if (g_pLTServer->GetPropGeneric("TriggerTarget", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrTriggerTarget = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("TriggerMessage", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrTriggerMessage = g_pLTServer->CreateString(genProp.m_String);
		}
	}


    g_pLTServer->GetPropRotationEuler("Rotation", &m_vPitchYawRoll);

    if (g_pLTServer->GetPropGeneric("Team", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			for (int i=0; i < 3; i++)
			{
				if (_stricmp(genProp.m_String, aszTeamNames[i]) == 0)
				{
					m_byTeam = i;
					break;
				}
			}
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::CacheFiles
//
//	PURPOSE:	Cache files associated with this object
//
// ----------------------------------------------------------------------- //

void GameStartPoint::CacheFiles()
{
	if (!m_hObject) return;

	// Okay, determine if this start point is used in the current game
	// (i.e., Single player are only used in single player, etc.)

	if (m_eGameType != g_pGameServerShell->GetGameType()) return;

	if (m_dwCounter == 0)
	{
		CachePlayerFiles();
		CacheSurfaceFiles();
		m_dwCounter++;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::CachePlayerFiles
//
//	PURPOSE:	Cache files associated with the current player mode.
//
// ----------------------------------------------------------------------- //

void GameStartPoint::CachePlayerFiles()
{
	// Cache the player models/skins...

	ModelId nId = g_pModelButeMgr->GetModelId(DEFAULT_PLAYERNAME);
	char* pFilename = (char*) g_pModelButeMgr->GetModelFilename(nId, m_ePlayerModelStyle);
    if (pFilename) g_pLTServer->CacheFile(FT_MODEL, pFilename);

	char* pSkin = (char*) g_pModelButeMgr->GetBodySkinFilename(nId, m_ePlayerModelStyle);
    if (pSkin) g_pLTServer->CacheFile(FT_TEXTURE, pSkin);

	pSkin = (char*) g_pModelButeMgr->GetHeadSkinFilename(nId, m_ePlayerModelStyle);
    if (pSkin) g_pLTServer->CacheFile(FT_TEXTURE, pSkin);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::CacheSurfaceFiles()
//
//	PURPOSE:	Cache files associated with surface flags
//
// ----------------------------------------------------------------------- //

void GameStartPoint::CacheSurfaceFiles()
{
    if (!g_pLTServer || !g_pWeaponMgr) return;

	StartTimingCounter();

	g_pSurfaceMgr->CacheAll();

	EndTimingCounter("GameStartPoint::CacheSurfaceFiles()");
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

    if (_stricmp("GameType", szPropName) == 0)
	{
		for (int i=0; i < g_knNumGameTypes; i++)
		{
			_ASSERT(cMaxStrings > (*pcStrings) + 1);
			if (cMaxStrings > (*pcStrings) + 1)
			{
				strcpy(aszStrings[(*pcStrings)++], GameTypeToString((GameType)i));
			}
		}

		return LT_OK;
	}


	// Handle Player model...

    if (_stricmp("PlayerModel", szPropName) == 0)
	{
		m_ModelStylePlugin.PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings,	cMaxStringLength);

		m_ModelStylePlugin.PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

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

    if (_stricmp("Team", szPropName) == 0)
	{
		for (int i=0; i < 3; i++)
		{
			_ASSERT(cMaxStrings > (*pcStrings) + 1);
			if (cMaxStrings > (*pcStrings) + 1)
			{
				strcpy(aszStrings[(*pcStrings)++], aszTeamNames[i]);
			}
		}

		return LT_OK;
	}


	return LT_UNSUPPORTED;
}
