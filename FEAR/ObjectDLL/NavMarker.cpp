// ----------------------------------------------------------------------- //
//
// MODULE  : NavMarker.cpp
//
// PURPOSE : implementation of NavMarker object
//
// CREATED : 11/05/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "NavMarker.h"
#include "MsgIDs.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "ClientServerShared.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "NavMarkerTypeDB.h"
#include "Spawner.h"
#include "StringUtilities.h"
#include "GameModeMgr.h"
#include "TeamMgr.h"
#include "PlayerObj.h"

LINKFROM_MODULE( NavMarker );

extern CGameServerShell* g_pGameServerShell;


// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
//
//	CLASS:		NavMarker
//
//	PURPOSE:	Invisible object the client HUD may track.
//
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_HAVMARKER CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_HAVMARKER 0

#endif

BEGIN_CLASS(NavMarker)
	ADD_SOLID_FLAG(0, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG( Target, "", PF_OBJECTLINK, "Object to use as the location of the marker." )
	ADD_STRINGPROP_FLAG(Type, "Default", PF_STATICLIST, "This is a dropdown that allows you to set what type of marker this is.")
	ADD_STRINGPROP_FLAG(Team, "NoTeam", PF_STATICLIST, "This is a dropdown that allows you to set which team can see the navigation marker.")
	ADD_STRINGIDPROP_FLAG(Text, "", PF_NOTIFYCHANGE, "This is an optional string ID for text to display with the marker.")
	ADD_BOOLPROP_FLAG(Broadcast, 1, PF_HIDDEN, "Is this NavMarker associated with a team broadcast.")
END_CLASS_FLAGS_PLUGIN(NavMarker, GameBase, CF_HIDDEN_HAVMARKER, CNavMarkerPlugin, "A Navigation Marker that will appear on the player's HUD.")


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateTypeMsg
//
//  PURPOSE:	Make sure the type message is valid.
//
// ----------------------------------------------------------------------- //

static bool ValidateTypeMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return false;

	if( cpMsgParams.m_nArgs < 2 )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateTypeMsg()" );
			pInterface->CPrint( "    MSG - TYPE - No type parameter listed." );
		}

		return false;
	}

	HRECORD hType = g_pNavMarkerTypeDB->GetRecord(cpMsgParams.m_Args[1]);
	if (hType)
		return true;

	pInterface->ShowDebugWindow( true );
	pInterface->CPrint( "ERROR! - ValidateTypeMsg()" );
	pInterface->CPrint( "    MSG - TYPE - Unnkown NavMarkerType %s.", cpMsgParams.m_Args[1] );
	return false;
}

CMDMGR_BEGIN_REGISTER_CLASS( NavMarker )

	ADD_MESSAGE( ON, 1, NULL, MSG_HANDLER( NavMarker, HandleOnMsg), "ON", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( OFF, 1, NULL, MSG_HANDLER( NavMarker, HandleOffMsg), "OFF", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( TARGET, 2, NULL, MSG_HANDLER( NavMarker, HandleTargetMsg), "TARGET <objectname>", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( TYPE, 2, ValidateTypeMsg, MSG_HANDLER( NavMarker, HandleTypeMsg), "TYPE <typename>", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( TEXT, 2, NULL, MSG_HANDLER( NavMarker, HandleTextMsg), "TEXT <string id>", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( TEAM, 2, NULL, MSG_HANDLER( NavMarker, HandleTeamMsg), "TEAM <0, 1, -1>", "TODO:CMDDESC", "TODO:CMDEXP" )

CMDMGR_END_REGISTER_CLASS( NavMarker, GameBase )



LTRESULT CNavMarkerPlugin::PreHook_PropChanged( const char *szObjName,
												 const char *szPropName,
											     const int nPropType,
											     const GenericProp &gpPropValue,
											     ILTPreInterface *pInterface,
												 const	char *szModifiers )
{
	if( LT_OK == m_StringEditMgrPlugin.PreHook_PropChanged( szObjName,
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

LTRESULT CNavMarkerPlugin::PreHook_EditStringList(const char* szRezPath,
												   const char* szPropName,
												   char** aszStrings,
												   uint32* pcStrings,
												   const uint32 cMaxStrings,
												   const uint32 cMaxStringLength)
{
	if( LTStrICmp( "Team", szPropName ) == 0 )
	{
		TeamPopulateEditStringList( aszStrings, pcStrings, cMaxStrings, cMaxStringLength );
		return LT_OK;
	}

	if( LTStrICmp( "Type", szPropName ) == 0 )
	{
		CNavMarkerTypeDBPlugin::Instance().PopulateStringList(aszStrings,pcStrings,cMaxStrings,cMaxStringLength);
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NavMarker::NavMarker()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

NavMarker::NavMarker() : GameBase(OT_NORMAL), m_hTarget(NULL)
{
	MakeTransitionable();
	m_hTarget.SetReceiver( *this );
	m_nTeamId = INVALID_TEAM;
	m_nStringId = INVALID_STRINGEDIT_INDEX;
	m_hType = NULL;
	m_bIsActive = false;
	m_fLifeTime = 0.0f;
	m_bBroadcast = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NavMarker::~NavMarker()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //
NavMarker::~NavMarker()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NavMarker::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 NavMarker::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			ObjectCreateStruct* pInfo = (ObjectCreateStruct*)pData;
	
			if ((fData == PRECREATE_WORLDFILE) || (fData == PRECREATE_STRINGPROP))
			{
				ReadProp(&pInfo->m_cProperties);

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

		case MID_ALLOBJECTSCREATED:
		{
			AssignTarget();

			CreateSpecialFX( true );
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

		case MID_SAVESPECIALEFFECTMESSAGE:
		{
			SaveSFXMessage( static_cast<ILTMessage_Write*>( pData ), static_cast<uint32>( fData ) );
		}
		break;

		case MID_LOADSPECIALEFFECTMESSAGE:
		{
			LoadSFXMessage( static_cast<ILTMessage_Read*>( pData ), static_cast<uint32>( fData ) );
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NavMarker::HandleOnMsg
//
//	PURPOSE:	Handle a ON message...
//
// ----------------------------------------------------------------------- //

void NavMarker::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_bIsActive = true;
	CreateSpecialFX(true);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NavMarker::HandleOffMsg
//
//	PURPOSE:	Handle a OFF message...
//
// ----------------------------------------------------------------------- //

void NavMarker::HandleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_bIsActive = false;
	CreateSpecialFX(true);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NavMarker::HandleTargetMsg
//
//	PURPOSE:	Handle a TARGET message...
//
// ----------------------------------------------------------------------- //

void NavMarker::HandleTargetMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_sTargetName = crParsedMsg.GetArg( 1 );
	m_hTarget = NULL;
	AssignTarget();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NavMarker::HandleTypeMsg
//
//	PURPOSE:	Handle a TYPE message...
//
// ----------------------------------------------------------------------- //

void NavMarker::HandleTypeMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	std::string sType = crParsedMsg.GetArg( 1 ).c_str();

	// Find the named type
	HRECORD hType = g_pNavMarkerTypeDB->GetRecord(sType.c_str());

	if (hType)
	{
		m_hType = hType;
		CreateSpecialFX(true);
	}
	else
	{
		g_pLTServer->CPrint("ERROR in NavMarker::HandleTypeMsg() : Unknown type \"%s\"", sType.c_str());
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NavMarker::HandleTextMsg
//
//	PURPOSE:	Handle a TEXT message...
//
// ----------------------------------------------------------------------- //

void NavMarker::HandleTextMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_nStringId = (uint32)IndexFromStringID( crParsedMsg.GetArg(1).c_str() );
	CreateSpecialFX(true);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NavMarker::HandleTeamMsg
//
//	PURPOSE:	Handle a TEAM message...
//
// ----------------------------------------------------------------------- //

void NavMarker::HandleTeamMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	uint32 nTeamId = atoi( crParsedMsg.GetArg( 1 ));
	if( nTeamId < MAX_TEAMS )
	{
		ASSERT( nTeamId == ( uint8 )nTeamId );
		m_nTeamId = ( uint8 )LTMIN( nTeamId, 255 );
		CreateSpecialFX(true);
	}
	else
	{
		m_nTeamId = INVALID_TEAM;
		CreateSpecialFX(true);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NavMarker::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool NavMarker::ReadProp(const GenericPropList *pProps)
{
	if( !pProps )
		return false;

	m_sTargetName = pProps->GetString( "Target", "" );
	std::string sType = pProps->GetString( "Type", "" );
	m_hType = g_pNavMarkerTypeDB->GetRecord(sType.c_str());

	// Get the team this object belongs to.
	if( GameModeMgr::Instance( ).m_grbUseTeams )
	{
		m_nTeamId = TeamStringToTeamId( pProps->GetString( "Team", "" ) );
	}
	else
	{
		m_nTeamId = INVALID_TEAM;
	}

	m_nStringId = IndexFromStringID( pProps->GetStringID("Text", 0) );

	m_bBroadcast = pProps->GetBool("Broadcast",false);

	m_fLifeTime = g_pNavMarkerTypeDB->GetLifetime(m_hType);

	return true;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NavMarker::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

bool NavMarker::InitialUpdate()
{
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );
	if (!m_hTarget)
	{
		g_pLTServer->SetNetFlags(m_hObject, NETFLAG_NOROTCHANGES);
	}
	else
	{
		g_pLTServer->SetNetFlags(m_hObject, NETFLAG_NOPOSCHANGES|NETFLAG_NOROTCHANGES);
	}
	
	m_LifeTimeTimer.SetEngineTimer( SimulationTimer::Instance( ));
	if (m_fLifeTime > 0.0f)
	{
		m_LifeTimeTimer.Start(m_fLifeTime);
		SetNextUpdate(m_fLifeTime);
	}
	else
	{
		SetNextUpdate(UPDATE_NEVER);
	}

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NavMarker::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

bool NavMarker::Update()
{
	if (m_LifeTimeTimer.IsStarted())
	{
		if (m_LifeTimeTimer.IsTimedOut())
		{
			g_pLTServer->RemoveObject(m_hObject);
		}
		else
		{
			SetNextUpdate((float)m_LifeTimeTimer.GetTimeLeft());
		}
	}
	else
	{
		SetNextUpdate(UPDATE_NEVER);
	}

   CreateSpecialFX(true);

   return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NavMarker::CreateSpecialFX
//
//	PURPOSE:	Send the special fx message for this object
//
// ----------------------------------------------------------------------- //

void NavMarker::CreateSpecialFX( bool bUpdateClients /* = false  */ )
{
	HOBJECT hTarget = m_hTarget;
	if (!hTarget)
	{
		hTarget = m_hObject;
	}

	uint8 dwClientID = INVALID_TEAM;
	if( IsPlayer( m_hTarget ))
	{
		CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_hTarget ));
		if( pPlayer )
		{
			dwClientID = (uint8)g_pLTServer->GetClientID( pPlayer->GetClient( ));
		}
	}

	NAVMARKERCREATESTRUCT navMarkerCreate;
	navMarkerCreate.m_bIsActive = m_bIsActive;
	navMarkerCreate.m_hTarget = m_hTarget;
	navMarkerCreate.m_nClientID = dwClientID;
	navMarkerCreate.m_nTeamId = m_nTeamId;
	navMarkerCreate.m_hType = m_hType;
	navMarkerCreate.m_nStringId = m_nStringId;
	navMarkerCreate.m_bBroadcast = m_bBroadcast;
	navMarkerCreate.m_bInstant = false;

	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(SFX_NAVMARKER_ID);
		navMarkerCreate.Write( cMsg );
		g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
	}

	if (bUpdateClients)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_NAVMARKER_ID );
		cMsg.WriteObject( m_hObject );
		navMarkerCreate.Write( cMsg );
		g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NavMarker::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void NavMarker::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_CHARSTRING( m_sTargetName.c_str() );
	SAVE_BYTE(m_nTeamId);
	SAVE_CHARSTRING(StringIDFromIndex(m_nStringId));
	SAVE_HRECORD(m_hType);
	SAVE_bool(m_bIsActive);
	SAVE_FLOAT(m_fLifeTime);
	SAVE_bool(m_bBroadcast);
	m_LifeTimeTimer.Save(*pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NavMarker::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void NavMarker::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	char szTemp[128];
	LOAD_CHARSTRING( szTemp, ARRAY_LEN(szTemp) );
	m_sTargetName = szTemp;
	LOAD_BYTE(m_nTeamId);
	LOAD_CHARSTRING( szTemp, ARRAY_LEN(szTemp) );
	m_nStringId = IndexFromStringID(szTemp);
	LOAD_HRECORD(m_hType,g_pNavMarkerTypeDB->GetCategory());
	LOAD_bool(m_bIsActive);
	LOAD_FLOAT(m_fLifeTime);
	LOAD_bool(m_bBroadcast);
	m_LifeTimeTimer.Load(*pMsg);

	AssignTarget();
	SetNextUpdate( UPDATE_NEXT_FRAME );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NavMarker::SaveSFXMessage
//
//	PURPOSE:	Save the object special effect message
//
// ----------------------------------------------------------------------- //

void NavMarker::SaveSFXMessage( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg )
		return;

	CAutoMessage cGetMsg;
	g_pLTServer->GetObjectSFXMessage( m_hObject, cGetMsg );
	CLTMsgRef_Read pSFXMsg = cGetMsg.Read( );

	if( pSFXMsg->Size( ) == 0 )
		return;

	pMsg->Writeuint8( pSFXMsg->Readuint8( ) );

	NAVMARKERCREATESTRUCT NavMarkerCS;
	NavMarkerCS.Read( pSFXMsg );
	NavMarkerCS.Write( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NavMarker::LoadSFXMessage
//
//	PURPOSE:	Load the object special effect message
//
// ----------------------------------------------------------------------- //

void NavMarker::LoadSFXMessage( ILTMessage_Read *pMsg, uint32 dwLoadFlags )
{
	if( !pMsg || pMsg->Size( ) == 0 )
		return;

	CAutoMessage cSFXMsg;

	cSFXMsg.Writeuint8( pMsg->Readuint8( ) );

	NAVMARKERCREATESTRUCT NavMarkerCS;
	NavMarkerCS.Read( pMsg );
	NavMarkerCS.Write( cSFXMsg );
	g_pLTServer->SetObjectSFXMessage( m_hObject, cSFXMsg.Read( ) );
}
	
// ----------------------------------------------------------------------- //
//
//  ROUTINE:	NavMarker::AssignTarget
//
//  PURPOSE:	Set the target object...
//
// ----------------------------------------------------------------------- //

void NavMarker::AssignTarget()
{
	// Get the target object if one was specified...
	
	if( !m_sTargetName.empty() )
	{
		HOBJECT hObj = NULL;
		if( (LTStrICmp(m_sTargetName.c_str(),"NONE") == 0) || (LT_OK != FindNamedObject( m_sTargetName.c_str(), hObj )))
		{
			m_hTarget = NULL;
			SetNextUpdate( UPDATE_NEXT_FRAME );
			return;
		}

		m_hTarget = hObj;

		// Need to make the magnet transitionable if it's target is...

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
//  ROUTINE:	NavMarker::OnLinkBroken
//
//  PURPOSE:	An object has broken its link with us
//
// ----------------------------------------------------------------------- //

void NavMarker::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if (pRef == &m_hTarget)
	{
		m_bIsActive = false;
		CreateSpecialFX(true);
	}
	
	GameBase::OnLinkBroken(pRef, hObj);
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	NavMarkerCreator::SpawnMarker
//
//  PURPOSE:	Spawn a new NavMarker object based on our data;
//
// ----------------------------------------------------------------------- //

NavMarker*  NavMarkerCreator::SpawnMarker() const
{
	if( m_bInstant )
	{
		uint8 dwClientID = INVALID_TEAM;
		CPlayerObj *pPlayer = CPlayerObj::DynamicCast( m_hTarget );
		if( pPlayer )
		{
			dwClientID = (uint8)g_pLTServer->GetClientID( pPlayer->GetClient( ));
		}

		NAVMARKERCREATESTRUCT navMarkerCreate;
		navMarkerCreate.m_bIsActive = true;
		navMarkerCreate.m_hTarget = m_hTarget;
		navMarkerCreate.m_nClientID = dwClientID;
		navMarkerCreate.m_nTeamId = m_nTeamId;
		navMarkerCreate.m_hType = m_hType;
		navMarkerCreate.m_nStringId = m_nStringId;
		navMarkerCreate.m_bBroadcast = m_bBroadcast;
		navMarkerCreate.m_bInstant = true;
		navMarkerCreate.m_vPos = m_vPos;

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_INSTANTNAVMARKER );
		navMarkerCreate.Write( cMsg );
		g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );

		return NULL;
	}
	else
	{
		char szSpawn[1024];
		char szTeam[8] = "None";
		if (m_nTeamId < MAX_TEAMS)
		{
			LTSNPrintF(szTeam,LTARRAYSIZE(szTeam),"Team%d",m_nTeamId);
		}

		if (IsPlayer(m_hTarget))
		{
			LTSNPrintF(szSpawn, LTARRAYSIZE(szSpawn),
				"NavMarker Type %s; Team %s; Broadcast %d",
				g_pNavMarkerTypeDB->GetRecordName(m_hType),
				szTeam,
				(m_bBroadcast?1:0));
		}
		else
		{
			LTSNPrintF(szSpawn, LTARRAYSIZE(szSpawn),
				"NavMarker Target %s; Type %s; Team %s; Broadcast %d",
				GetObjectName(m_hTarget),
				g_pNavMarkerTypeDB->GetRecordName(m_hType),
				szTeam,
				(m_bBroadcast?1:0));
	}
	
		NavMarker* pNM = dynamic_cast<NavMarker*>(SpawnObject(szSpawn, m_vPos, LTRotation()));
		pNM->Activate(true);

		if( m_hTarget )
			pNM->SetTargetObject( m_hTarget );

		return pNM;
	}
}
