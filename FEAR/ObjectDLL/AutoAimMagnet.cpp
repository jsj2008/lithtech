// ----------------------------------------------------------------------- //
//
// MODULE  : AutoAimMagnet.cpp
//
// PURPOSE : Invisible object that be used as an potential target by the auto-aim system
//
// CREATED : 3/11/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AutoAimMagnet.h"
#include "MsgIDs.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "ClientServerShared.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "GameModeMgr.h"
#include "TeamMgr.h"

LINKFROM_MODULE( AutoAimMagnet );

extern CGameServerShell* g_pGameServerShell;


// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
//
//	CLASS:		AutoAimMagnet
//
//	PURPOSE:	Invisible object that be used as an potential target by the auto-aim system
//
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AutoAimMagnet 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AutoAimMagnet CF_HIDDEN

#endif

BEGIN_CLASS(AutoAimMagnet)
	ADD_SOLID_FLAG(0, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG( Target, "", PF_OBJECTLINK, "Object to use as the auto-aim target." )
	//place holder in case it's needed later
	ADD_STRINGPROP_FLAG(Team, "NoTeam", PF_HIDDEN | PF_STATICLIST, "This is a dropdown that allows you to set which team can see the auto-aim magnet.")
END_CLASS_FLAGS_PLUGIN(AutoAimMagnet, GameBase, CF_HIDDEN_AutoAimMagnet, CAutoAimMagnetPlugin, "This object is created to specify an target that will draw the reticle for autoaim.")


CMDMGR_BEGIN_REGISTER_CLASS( AutoAimMagnet )

	ADD_MESSAGE( TEAM, 2, NULL, MSG_HANDLER( AutoAimMagnet, HandleTeamMsg), "TEAM <0, 1, -1>", "Tells the object which team will have their targeting reticule attracted to it.", "msg AutoAimMagnet (TEAM 1)" )
	ADD_MESSAGE( ON, 1, NULL, MSG_HANDLER( AutoAimMagnet, HandleOnMsg), "ON", "Tells the object to attract the auto aim targeting reticule.", "msg AutoAimMagnet ON" )
	ADD_MESSAGE( OFF, 1, NULL, MSG_HANDLER( AutoAimMagnet, HandleOffMsg), "OFF", "Tells the object not to attract the auto aim targeting reticule.", "msg AutoAimMagnet OFF" )
	ADD_MESSAGE( TARGET, 2, NULL, MSG_HANDLER( AutoAimMagnet, HandleTargetMsg), "TARGET <objectname>", "Tells the AutoAimMagnet object to center it�s self on the object specified in the message.", "To tell an AutoAimMagnet object named AutoAimMagnet to target an object named AI01 the command would look like:<BR><BR>msg AutoAimMagnet (TARGET AI01)" )

CMDMGR_END_REGISTER_CLASS( AutoAimMagnet, GameBase )



LTRESULT CAutoAimMagnetPlugin::PreHook_EditStringList(const char* szRezPath,
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


	return LT_UNSUPPORTED;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AutoAimMagnet::AutoAimMagnet()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

AutoAimMagnet::AutoAimMagnet() : GameBase(OT_NORMAL), m_hTarget(NULL)
{
	MakeTransitionable();
	m_hTarget.SetReceiver( *this );
	m_nTeamId = INVALID_TEAM;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AutoAimMagnet::~AutoAimMagnet()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //
AutoAimMagnet::~AutoAimMagnet()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AutoAimMagnet::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 AutoAimMagnet::EngineMessageFn(uint32 messageID, void *pData, float fData)
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
	
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp(&pInfo->m_cProperties);
			}
			else if (fData == PRECREATE_STRINGPROP)
			{
				ReadProp(&pInfo->m_cProperties);

				// Show ourself...

				pInfo->m_Flags |= FLAG_VISIBLE;
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
			
			uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);
			
			return dwRet;
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AutoAimMagnet::HandleTeamMsg
//
//	PURPOSE:	Handle a TEAM message...
//
// ----------------------------------------------------------------------- //

void AutoAimMagnet::HandleTeamMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	uint32 nTeamId = atoi( crParsedMsg.GetArg( 1 ));
	if( nTeamId < MAX_TEAMS )
	{
		SetTeamId( nTeamId );
	}
	else
	{
		SetTeamId( INVALID_TEAM );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AutoAimMagnet::HandleOnMsg
//
//	PURPOSE:	Handle a ON message...
//
// ----------------------------------------------------------------------- //

void AutoAimMagnet::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( m_hObject )
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AutoAimMagnet::HandleOffMsg
//
//	PURPOSE:	Handle a OFF message...
//
// ----------------------------------------------------------------------- //

void AutoAimMagnet::HandleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( m_hObject )
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_VISIBLE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AutoAimMagnet::HandleTargetMsg
//
//	PURPOSE:	Handle a TARGET message...
//
// ----------------------------------------------------------------------- //

void AutoAimMagnet::HandleTargetMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_sTargetName = crParsedMsg.GetArg( 1 );
	m_hTarget = NULL;
	AssignTarget();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AutoAimMagnet::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool AutoAimMagnet::ReadProp(const GenericPropList *pProps)
{
	if( !pProps )
		return false;

	m_sTargetName = pProps->GetString( "Target", "" );

	// Get the team this object belongs to.
	if( GameModeMgr::Instance( ).m_grbUseTeams )
	{
		m_nTeamId = TeamStringToTeamId( pProps->GetString( "Team", "" ) );
	}
	else
	{
		m_nTeamId = INVALID_TEAM;
	}

	return true;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AutoAimMagnet::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

bool AutoAimMagnet::InitialUpdate()
{
    SetNextUpdate(UPDATE_NEVER);

//	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAG_FORCECLIENTUPDATE );
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );

	CreateSpecialFX();

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AutoAimMagnet::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

bool AutoAimMagnet::Update()
{
   SetNextUpdate(UPDATE_NEVER);

   CreateSpecialFX(true);

   return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AutoAimMagnet::CreateSpecialFX
//
//	PURPOSE:	Send the special fx message for this object
//
// ----------------------------------------------------------------------- //

void AutoAimMagnet::CreateSpecialFX( bool bUpdateClients /* = false  */ )
{
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(SFX_AIMMAGNET_ID);
		cMsg.Writeuint8(m_nTeamId);
		cMsg.WriteObject(m_hTarget);
		g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
	}

	if (bUpdateClients)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_AIMMAGNET_ID );
		cMsg.WriteObject( m_hObject );
		cMsg.Writeuint8( m_nTeamId );
		cMsg.WriteObject(m_hTarget);
		g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AutoAimMagnet::SetTeamId
//
//	PURPOSE:	Set the teamid for this AutoAimMagnet...
//
// ----------------------------------------------------------------------- //

void AutoAimMagnet::SetTeamId( uint8 nTeamId )
{
	m_nTeamId = nTeamId;


	CreateSpecialFX(true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AutoAimMagnet::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void AutoAimMagnet::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_BYTE(m_nTeamId);
	SAVE_CHARSTRING( m_sTargetName.c_str() );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AutoAimMagnet::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void AutoAimMagnet::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_BYTE(m_nTeamId);
	char szTemp[128];
	LOAD_CHARSTRING( szTemp, ARRAY_LEN(szTemp) );
	m_sTargetName = szTemp;

	AssignTarget();
}

	
// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AutoAimMagnet::AssignTarget
//
//  PURPOSE:	Set the target object...
//
// ----------------------------------------------------------------------- //

void AutoAimMagnet::AssignTarget()
{
	if( m_hTarget )
		return;

	// Get the target object if one was specified...
	
	if( !m_sTargetName.empty() )
	{
		HOBJECT hObj = NULL;
		if( LT_OK != FindNamedObject( m_sTargetName.c_str(), hObj ))
		{
			m_hTarget = NULL;
			g_pLTServer->RemoveObject( m_hObject );
			
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
//  ROUTINE:	AutoAimMagnet::OnLinkBroken
//
//  PURPOSE:	An object has broken its link with us
//
// ----------------------------------------------------------------------- //

void AutoAimMagnet::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if (pRef == &m_hTarget)
		CreateSpecialFX(true);

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, 0, FLAG_VISIBLE );
	
	GameBase::OnLinkBroken(pRef, hObj);
}
