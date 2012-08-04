// ----------------------------------------------------------------------- //
//
// MODULE  : RadarObject.cpp
//
// PURPOSE : The RadarObject implementation
//
// CREATED : 6/6/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "MsgIds.h"
	#include "ParsedMsg.h"
	#include "RadarObject.h"
	#include "VersionMgr.h"


LINKFROM_MODULE( RadarObject );

BEGIN_CLASS( RadarObject )

	ADD_STRINGPROP_FLAG( Type, "", PF_STATICLIST )
	ADD_BOOLPROP( StartOn, LTTRUE )
	ADD_STRINGPROP_FLAG( Target, "", PF_OBJECTLINK )
	ADD_STRINGPROP_FLAG(Team, "AllTeams", PF_STATICLIST)

END_CLASS_DEFAULT_FLAGS_PLUGIN( RadarObject, GameBase, NULL, NULL, 0, CRadarObjectPlugin )

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( RadarObject )

	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )
	CMDMGR_ADD_MSG( TEAM, 2, NULL, "TEAM <0, 1, -1>" )
	
CMDMGR_END_REGISTER_CLASS( RadarObject, GameBase )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGadgetTargetPlugin::PreHook_EditStringList
//
//  PURPOSE:	Populate the Type list
//
// ----------------------------------------------------------------------- //

LTRESULT CRadarObjectPlugin::PreHook_EditStringList( const char* szRezPath, 
													 const char* szPropName,
													 char** aszStrings,
													 uint32* pcStrings,
													 const uint32 cMaxStrings,
													 const uint32 cMaxStringLength )
{
	if( !_stricmp( "Type", szPropName ))
	{
		if( m_RadarTypeMgrPlugin.PreHook_EditStringList( szRezPath, szPropName,
														 aszStrings, pcStrings, 
														 cMaxStrings, cMaxStringLength ) == LT_OK )
		{
			return LT_OK;
		}
	}
	else if( _stricmp( "Team", szPropName ) == 0 )
	{
		char szTeam[32] = {0};

		_ASSERT(cMaxStrings > (*pcStrings) + 1);
		strcpy( aszStrings[(*pcStrings)++], "AllTeams" );
		
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

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RadarObject::RadarObject
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

RadarObject::RadarObject()
:	GameBase			( OT_NORMAL ),
	m_hTarget			( LTNULL )
{
	MakeTransitionable();
	m_nTeamId = INVALID_TEAM;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RadarObject::~RadarObject
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

RadarObject::~RadarObject()
{
	
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RadarObject::EngineMessageFn
//
//  PURPOSE:	Handel messages from the engine
//
// ----------------------------------------------------------------------- //

uint32 RadarObject::EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData )
{
	switch( messageID )
	{
		case MID_PRECREATE:
		{
			// Let the GameBase handle the message first

			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			if( fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP )
			{
				ObjectCreateStruct *pStruct = (ObjectCreateStruct*)pData;
				ReadProps( pStruct );
			}

			// Important!! - We already sent the message to the GameBase so DONT do it again.

			return dwRet;
		}
		break;

		case MID_OBJECTCREATED:
		{
			if( fData != OBJECTCREATED_SAVEGAME )
			{
				ObjectCreated();
			}
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_ALLOBJECTSCREATED:
		{
			if( m_ROCS.bOn )
				AssignTarget();
		}
		break;

		case MID_SAVEOBJECT:
		{
			OnSave( (ILTMessage_Write*)pData );
		}
		break;

		case MID_LOADOBJECT:
		{
			OnLoad( (ILTMessage_Read*)pData );

			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			CreateSpecialFX();
			
			return dwRet;
		}
		break;
			
	}

	return GameBase::EngineMessageFn( messageID, pData, fData );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RadarObject::ReadProps
//
//  PURPOSE:	Read the property values...
//
// ----------------------------------------------------------------------- //

void RadarObject::ReadProps( ObjectCreateStruct *pOCS )
{
	if( !pOCS || !g_pRadarTypeMgr ) return;

	GenericProp gProp;

	// Get the Radar Type record...

	RADARTYPE *pType;

	if( g_pLTServer->GetPropGeneric( "Type", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
		{
			pType = g_pRadarTypeMgr->GetRadarType( gProp.m_String );
		}
	}

	if( pType )
	{
		m_ROCS.nRadarTypeId = pType->nId;
	}

	if( g_pLTServer->GetPropGeneric( "StartOn", &gProp ) == LT_OK )
	{
		m_ROCS.bOn = gProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "Target", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
		{
			m_sTargetName = gProp.m_String;
		}
	}

	if( IsTeamGameType() )
	{
		if( g_pLTServer->GetPropGeneric( "Team", &gProp ) == LT_OK )
		{
			if( gProp.m_String[0] )
			{
				// The team string should be TeamN, when N is the team id.
				char const szTeam[] = "Team";
				int nLen = strlen( szTeam );
				if( !_strnicmp( gProp.m_String, szTeam, nLen ))
				{
					uint32 nTeamId = atoi( &gProp.m_String[ nLen ] );
					if( nTeamId < MAX_TEAMS )
					{
						m_nTeamId = nTeamId;
						m_ROCS.nTeamId = nTeamId;
					}
				}
			}
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RadarObject::ObjectCreated
//
//	PURPOSE:	Initial update
//
// ----------------------------------------------------------------------- //

void RadarObject::ObjectCreated()
{
	// We need to force the clients to see this object so the special fx object
	// associated with this object will not get removed...

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAGMASK_ALL );
	
	SetNextUpdate( UPDATE_NEVER );
	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RadarObject::CreateSpecialFX()
//
//	PURPOSE:	Add client-side special fx
//
// ----------------------------------------------------------------------- //

void RadarObject::CreateSpecialFX( bool bUpdateClients /* = false  */ )
{
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( SFX_RADAROBJECT_ID );
		m_ROCS.Write( cMsg );
		g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read() );
	}

	if( bUpdateClients )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_RADAROBJECT_ID );
		cMsg.WriteObject( m_hObject );
		m_ROCS.Write( cMsg );
		g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RadarObject::OnTrigger
//
//  PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool RadarObject::OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken s_cTok_On( "ON" );
	static CParsedMsg::CToken s_cTok_Off( "OFF" );
	static CParsedMsg::CToken s_cTok_Team( "TEAM" );

	if( cMsg.GetArg(0) == s_cTok_On )
	{
		if( !m_ROCS.bOn )
		{
			m_ROCS.bOn = true;
			
			// Setup the target once we turn on...

			AssignTarget();
			
			CreateSpecialFX( true );
		}
	}
	else if( cMsg.GetArg(0) == s_cTok_Off )
	{
		if( m_ROCS.bOn )
		{
			m_ROCS.bOn = false;
			CreateSpecialFX( true );
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_Team)
	{
		if( cMsg.GetArgCount( ) > 1 )
		{
			uint32 nTeamId = atoi( cMsg.GetArg( 1 ));
			if( nTeamId < MAX_TEAMS )
			{
				SetTeamId( nTeamId );
			}
			else
			{
				SetTeamId( INVALID_TEAM );
			}

			return true;
		}
	}
	else
		return GameBase::OnTrigger( hSender, cMsg );

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RadarObject::OnSave
//
//  PURPOSE:	Save the Radar object...
//
// ----------------------------------------------------------------------- //

bool RadarObject::OnSave( ILTMessage_Write *pMsg )
{
	if( !pMsg ) return false;

	m_ROCS.Write( pMsg );

	SAVE_CHARSTRING( m_sTargetName.c_str() );
	SAVE_HOBJECT( m_hTarget );
	SAVE_BYTE( m_nTeamId );

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RadarObject::OnSave
//
//  PURPOSE:	Load the Radar object...
//
// ----------------------------------------------------------------------- //

bool RadarObject::OnLoad( ILTMessage_Read *pMsg )
{
	if( !pMsg ) return false;

	m_ROCS.Read( pMsg );

	char szTemp[128];
	LOAD_CHARSTRING( szTemp, ARRAY_LEN(szTemp) );
	m_sTargetName = szTemp;

	LOAD_HOBJECT( m_hTarget );

	if( g_pVersionMgr->GetCurrentSaveVersion( ) < CVersionMgr::kSaveVersion__1_3 )
	{
	}
	else
	{
		LOAD_BYTE( m_nTeamId );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RadarObject::Update
//
//  PURPOSE:	Update the radar object...
//
// ----------------------------------------------------------------------- //

void RadarObject::Update()
{
	// Only update if we have a target object...

	if( !m_hTarget )
	{
		// So if there isn't one get rid of the radar object...

		g_pLTServer->RemoveObject( m_hObject );
	}

	// Move the radar object to the target position...

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hTarget, &vPos );
	g_pLTServer->SetObjectPos( m_hObject, &vPos );

	SetNextUpdate( UPDATE_NEXT_FRAME );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RadarObject::AssignTarget
//
//  PURPOSE:	Set the target object...
//
// ----------------------------------------------------------------------- //

void RadarObject::AssignTarget()
{
	if( m_hTarget )
		return;

	// Get the target object if one was specified...
	
	if( !m_sTargetName.empty() )
	{
		HOBJECT hObj = LTNULL;
		if( LT_OK != FindNamedObject( m_sTargetName.c_str(), hObj ))
		{
			m_hTarget = LTNULL;
			g_pLTServer->RemoveObject( m_hObject );
			
			return;
		}

		m_hTarget = hObj;

		// Need to make the radar transitionable if it's target is...

		GameBase *pTarget = dynamic_cast<GameBase*>(g_pLTServer->HandleToObject( m_hTarget ));
		if( pTarget )
		{
			if( pTarget->CanTransition() )
			{
				// We need to set touch notify so the object will be added to containers, like transition areas...

				g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_TOUCH_NOTIFY, FLAG_TOUCH_NOTIFY );
			}
		}

		SetNextUpdate( UPDATE_NEXT_FRAME );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RadarObject::SetTeamId
//
//  PURPOSE:	Set the teamid.
//
// ----------------------------------------------------------------------- //

void RadarObject::SetTeamId( uint8 nTeamId )
{
	m_nTeamId = nTeamId;
	m_ROCS.nTeamId = nTeamId;

	CreateSpecialFX( true );
}
