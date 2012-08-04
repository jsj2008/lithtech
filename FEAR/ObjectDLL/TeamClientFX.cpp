// ----------------------------------------------------------------------- //
//
// MODULE  : TeamClientFX.cpp
//
// PURPOSE : Places team specific clientfx in a level.
//
// CREATED : 05/25/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "Stdafx.h"
#include "TeamClientFX.h"
#include "GameModeMgr.h"

LINKFROM_MODULE( TeamClientFX )

// Plugin class for hooking into the level editor for displaying entries in listboxes and displaying the model...
class TeamClientFXPlugin : public IObjectPlugin
{
public: // Methods...

	virtual LTRESULT PreHook_EditStringList( 
		const char *szRezPath,
		const char *szPropName,
		char **aszStrings,
		uint32 *pcStrings,
		const uint32 cMaxStrings,
		const uint32 cMaxStringLen );

	virtual LTRESULT PreHook_PropChanged( 
		const	char		*szObjName,
		const	char		*szPropName,
		const	int			nPropType,
		const	GenericProp	&gpPropValue,
		ILTPreInterface		*pInterface,
		const	char		*szModifiers );

private: // Members...

	CCommandMgrPlugin			m_CommandMgrPlugin;
};


BEGIN_CLASS( TeamClientFX )
	ADD_STRINGPROP_FLAG(TeamClientFX, "", PF_STATICLIST, "This is a dropdown that allows you to set which TeamClientFX record to use from the database.")
	ADD_STRINGPROP_FLAG(Team, "NoTeam", PF_STATICLIST, "This is a dropdown that allows you to set which team this object belongs to.")
END_CLASS_FLAGS_PLUGIN( TeamClientFX, GameBase, 0, TeamClientFXPlugin, "Places a TeamClientFX within the level." )

// Register with the CommandMgr...

CMDMGR_BEGIN_REGISTER_CLASS( TeamClientFX )
CMDMGR_END_REGISTER_CLASS( TeamClientFX, GameBase )


LTRESULT TeamClientFXPlugin::PreHook_EditStringList( const char *szRezPath,
											   const char *szPropName,
											   char **aszStrings,
											   uint32 *pcStrings,
											   const uint32 cMaxStrings,
											   const uint32 cMaxStringLen )
{
	if( !aszStrings || !pcStrings )
	{
		LTERROR( "Invalid input parameters" );
		return LT_UNSUPPORTED;
	}

	if( LTStrEquals( szPropName, "TeamClientFX" ))
	{
		// Fill the first string in the list with a <none> selection...
		LTStrCpy( aszStrings[(*pcStrings)++], "", cMaxStringLen );

		// Add an entry for each flagbase.
		uint8 nNumRecords = DATABASE_CATEGORY( TeamClientFX ).GetNumRecords();
		for( uint8 nRecordIndex = 0; nRecordIndex < nNumRecords; ++nRecordIndex )
		{
			LTASSERT( cMaxStrings > (*pcStrings) + 1, "Too many items's to fit in the list.  Enlarge list size?" );

			HRECORD hRecord = DATABASE_CATEGORY( TeamClientFX ).GetRecordByIndex( nRecordIndex );
			if( !hRecord )
				continue;

			const char *pszRecordName = DATABASE_CATEGORY( TeamClientFX ).GetRecordName( hRecord );
			if( !pszRecordName )
				continue;

			if( (LTStrLen( pszRecordName ) < cMaxStringLen) && ((*pcStrings) + 1 < cMaxStrings) )
			{
				LTStrCpy( aszStrings[(*pcStrings)++], pszRecordName, cMaxStringLen );
			}
		}

		// Sort the list so things are easier to find.  Skip the first item, since it's the <none> selection.
		qsort( aszStrings + 1, *pcStrings - 1, sizeof(char *), CaseInsensitiveCompare );

		return LT_OK;
	}

	// Handle team...
	if( LTStrIEquals( "Team", szPropName ))
	{
		TeamPopulateEditStringList( aszStrings, pcStrings, cMaxStrings, cMaxStringLen );
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

LTRESULT TeamClientFXPlugin::PreHook_PropChanged( const char *szObjName, 
											const char *szPropName,
											const int nPropType,
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


//
// TeamClientFX class implementation...
//

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TeamClientFX::TeamClientFX
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

TeamClientFX::TeamClientFX( ) : GameBase( OT_MODEL )
{
	m_nTeamId = INVALID_TEAM;
	m_hTeamClientFXRec = NULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TeamClientFX::~TeamClientFX
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

TeamClientFX::~TeamClientFX( )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TeamClientFX::EngineMessageFn
//
//	PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //
uint32 TeamClientFX::EngineMessageFn( uint32 messageID, void *pData, float fData )
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;

			if( fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP )
			{
				if( !ReadProp( &pStruct->m_cProperties ))
					return 0;
			}

			if( !PostReadProp( pStruct ))
				return 0;

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			return dwRet;
		}
		break;

		default : 
		break;
	}

	return GameBase::EngineMessageFn( messageID, pData, fData );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TeamClientFX::ReadProp
//
//	PURPOSE:	Read in the properties of the object... 
//
// ----------------------------------------------------------------------- //
bool TeamClientFX::ReadProp( const GenericPropList *pProps )
{
	char const* pszTeamClientFX = pProps->GetString( "TeamClientFX", "" );
	if( LTStrEmpty( pszTeamClientFX ))
		return false;
	m_hTeamClientFXRec = DATABASE_CATEGORY( TeamClientFX ).GetRecordByName( pszTeamClientFX );
	if( !m_hTeamClientFXRec )
		return false;

	if( GameModeMgr::Instance( ).m_grbUseTeams )
	{
		const char *pszTeam = pProps->GetString( "Team", "" );
		m_nTeamId = TeamStringToTeamId( pszTeam );
	}
	else
	{
		// Not useful without a team mode.
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TeamClientFX::PostReadProp
//
//	PURPOSE:	Configure the ObjectCreateStruct for creating the object
//
// ----------------------------------------------------------------------- //
bool TeamClientFX::PostReadProp( ObjectCreateStruct *pStruct )
{
	pStruct->m_Flags |= FLAG_FORCECLIENTUPDATE;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TeamClientFX::InitialUpdate
//
//	PURPOSE:	Handle a MID_INITIALUPDATE message from the engine....
//
// ----------------------------------------------------------------------- //
void TeamClientFX::InitialUpdate( )
{
	CreateSpecialFX( false );
	SetNextUpdate( UPDATE_NEVER );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TeamClientFX::CreateSpecialFX
//
//	PURPOSE:	Send relevant information to clients...
//
// ----------------------------------------------------------------------- //

void TeamClientFX::CreateSpecialFX( bool bUpdateClients )
{
	TEAMCLIENTFXCREATESTRUCT cs;
	cs.m_hTeamClientFXRec = m_hTeamClientFXRec;
	cs.m_nTeamId = m_nTeamId;

	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( SFX_TEAMCLIENTFX_ID );
		cs.Write( cMsg );
		g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read( ));
	}
}
