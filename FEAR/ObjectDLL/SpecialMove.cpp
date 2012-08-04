// ----------------------------------------------------------------------- //
//
// MODULE  : SpecialMove.cpp
//
// PURPOSE : Implementation of SpecialMove object
//
// CREATED : 02/07/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "SpecialMove.h"
#include "AnimationPropStrings.h"

#define SPECIALMOVE_PREFIX "ACT_"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool UnsupportedMsg
//
//  PURPOSE:	Message handler for unsupported messages
//
// ----------------------------------------------------------------------- //

static bool UnsupportedMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->CPrint( "WARNING! - this MSG is not supported by the SpecialMove object:" );
	}

	return false;
}

extern bool ValidateMsgBool( ILTPreInterface *pInterface, ConParse &cpMsgParams );

LINKFROM_MODULE( SpecialMove );

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_SpecialMove 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_SpecialMove CF_HIDDEN

#endif

BEGIN_CLASS( SpecialMove )
	ADD_BOOLPROP_FLAG(On, true, 0, "Determines if this object starts out enabled or not.")
	ADD_BOOLPROP_FLAG(Radial, false, 0, "If true, the player will be aligned to the object, and the distance check will be radial.  Otherwise, the player will positioned at the closest point along the plane of the object and that distance will be used.")
	ADD_STRINGPROP_FLAG(Animation, "", PF_STATICLIST, "This dropdown menu allows you to choose a specific animation to be played when the player presses his action button.")
	ADD_REALPROP_FLAG(ActivateDist, 50.0f, PF_RADIUS, "The player must be this distance from the object in order to activate it (+/- some value specified in the database: Shared/SpecialMove/ActivateBuffer)")
	ADD_STRINGPROP_FLAG(ActivationType, "Default", PF_STATICLIST, "A list of different activatable types used for player interaction.")
	ADD_COMMANDPROP_FLAG(LookedAtCommand, "", PF_NOTIFYCHANGE, "Command sent when the object is looked at.")
	ADD_COMMANDPROP_FLAG(ActivateCommand, "", PF_NOTIFYCHANGE, "Command sent when the object is activated.")
	ADD_COMMANDPROP_FLAG(ReleaseCommand, "", PF_NOTIFYCHANGE, "Command sent when the animation is finished.")
END_CLASS_FLAGS_PLUGIN( SpecialMove, GameBase, CF_WORLDMODEL | CF_HIDDEN_SpecialMove, SpecialMovePlugin, "Used to play context specific animations in response to player input.  IOW: Your basic climb-on-box tech." )

CMDMGR_BEGIN_REGISTER_CLASS( SpecialMove )
	ADD_MESSAGE( ACTIVATE,		1,	NULL, MSG_HANDLER( SpecialMove, HandleActivateMsg ), "ACTIVATE", "Used to activate via game events (rather than waiting for player input).", "msg <ObjectName> ACTIVATE" )
	ADD_MESSAGE( ON,			1,	NULL, MSG_HANDLER( SpecialMove, HandleOnMsg ), "ON", "Used to enabled activation of this object by the player.", "msg <ObjectName> ON" )
	ADD_MESSAGE( OFF,			1,	NULL, MSG_HANDLER( SpecialMove, HandleOffMsg ), "OFF", "Used to disable activation of this object by the player.", "msg <ObjectName> OFF" )
	ADD_MESSAGE( VISIBLE,		2,	UnsupportedMsg,	NULL,	"VISIBLE", "Unsupported by SpecialMove objects.", "" )
	ADD_MESSAGE( SOLID,			2,	UnsupportedMsg,	NULL,	"SOLID <bool>", "Unsupported by SpecialMove objects.", "" )
	ADD_MESSAGE( HIDDEN,		2,	UnsupportedMsg,	NULL,	"HIDDEN <bool>", "Unsupported by SpecialMove objects.", "" )
	ADD_MESSAGE( CASTSHADOW,	2,	UnsupportedMsg,	NULL,	"CASTSHADOW", "Unsupported by SpecialMove objects.", "" )
	ADD_MESSAGE( SETPOS,		4,	UnsupportedMsg,	NULL,	"SETPOS", "Unsupported by SpecialMove objects.", ""  )
	ADD_MESSAGE( MOVETOPOS,		4,	UnsupportedMsg,	NULL,	"MOVETOPOS", "Unsupported by SpecialMove objects.", "" )
	ADD_MESSAGE( SETROTATION,	4,	UnsupportedMsg,	NULL,	"SETROTATION", "Unsupported by SpecialMove objects.", "" )
CMDMGR_END_REGISTER_CLASS( SpecialMove, GameBase )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpecialMove::SpecialMove
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

SpecialMove::SpecialMove(): GameBase(OT_WORLDMODEL)
, m_eAnimation			(kAP_None)
, m_fActivateDist		(0.0f)
, m_bOn					(true)
, m_bRadial				(false)
, m_sActivateCommand	("")
, m_sReleaseCommand		("")
, m_sLookedAtCommand	("")
{
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpecialMove::~SpecialMove
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

SpecialMove::~SpecialMove()
{
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpecialMove::EngineMessageFn
//
//  PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //

uint32 SpecialMove::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch (messageID)
	{
	case MID_PRECREATE:
		{
			uint32 nRes = GameBase::EngineMessageFn(messageID, pData, fData);
			ObjectCreateStruct	*pOCS = (ObjectCreateStruct*)pData;
			ReadProp(&pOCS->m_cProperties);
			PostReadProp(pOCS);
			return nRes;
		}
		break;

	case MID_OBJECTCREATED:
		{
			if( OBJECTCREATED_SAVEGAME != fData )
			{
				// Link up the activation handler
				m_ActivateTypeHandler.Init( m_hObject );
			}

			uint32 nRes = GameBase::EngineMessageFn(messageID, pData, fData);

			int nInfo = (int)fData;
			if (nInfo != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			return nRes;
		}
		break;

	case MID_SAVEOBJECT:
		{
			Save( ( ILTMessage_Write* )pData, ( uint32 )fData );
			break;
		}

	case MID_LOADOBJECT:
		{
			Load( ( ILTMessage_Read* )pData, ( uint32 )fData );
			break;
		}
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecialMove::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 SpecialMove::ObjectMessageFn( HOBJECT hSender, ILTMessage_Read* pMsg )
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();
	switch (messageID)
	{
	case MID_SFX_MESSAGE:
		{
			uint8 nSfxId = pMsg->Readuint8();
			HandleSfxMessage(hSender, pMsg, nSfxId);
		}
		break;
	}

	return GameBase::ObjectMessageFn( hSender, pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecialMove::HandleSfxMessage
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void SpecialMove::HandleSfxMessage( HOBJECT hSender, ILTMessage_Read *pMsg, uint8 nSfxId )
{
	switch( nSfxId )
	{
	case SPECIALMOVEFX_ACTIVATED:
		{
			OnActivated();
		}
		break;

	case SPECIALMOVEFX_RELEASED:
		{
			OnReleased();
		}
		break;

	case SPECIALMOVEFX_LOOKEDAT:
		{
			OnLookedAt();
		}
		break;
	}
}

void SpecialMove::OnActivated()
{
	if( !m_sActivateCommand.empty() )
	{
		g_pCmdMgr->QueueCommand( m_sActivateCommand.c_str(), m_hObject, NULL );
	}
}
void SpecialMove::OnReleased()
{
	if( !m_sReleaseCommand.empty() )
	{
		g_pCmdMgr->QueueCommand( m_sReleaseCommand.c_str(), m_hObject, NULL );
	}
}
void SpecialMove::OnLookedAt()
{
	if( !m_sLookedAtCommand.empty() )
	{
		g_pCmdMgr->QueueCommand( m_sLookedAtCommand.c_str(), m_hObject, NULL );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecialMove::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void SpecialMove::ReadProp(const GenericPropList *pProps)
{
	if (!pProps)
		return;

	const char* pszAnimation = pProps->GetString("Animation", NULL);
	if (!LTStrEmpty(pszAnimation))
	{
		m_eAnimation = AnimPropUtils::Enum(pszAnimation);
	}

	m_fActivateDist = pProps->GetReal("ActivateDist", m_fActivateDist);

	const char* pszActivateType = pProps->GetString("ActivationType", NULL);
	if (!LTStrEmpty(pszActivateType))
	{
		m_ActivateTypeHandler.SetActivateType(pszActivateType);
	}

	m_bOn = pProps->GetBool("On", true);
	m_bRadial = pProps->GetBool("Radial", false);

	m_sActivateCommand = pProps->GetCommand("ActivateCommand", "");
	m_sReleaseCommand = pProps->GetCommand("ReleaseCommand", "");
	m_sLookedAtCommand = pProps->GetCommand("LookedAtCommand", "");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecialMove::PostReadProp
//
//	PURPOSE:	Update the ObjectCreateStruct when creating the object
//
// ----------------------------------------------------------------------- //

void SpecialMove::PostReadProp(ObjectCreateStruct *pStruct)
{
	pStruct->m_Flags |= FLAG_FORCECLIENTUPDATE | FLAG_RAYHIT;
	pStruct->SetFileName(pStruct->m_Name);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecialMove::InitialUpdate
//
//	PURPOSE:	Setup the object.
//
// ----------------------------------------------------------------------- //

void SpecialMove::InitialUpdate()
{
	g_pLTServer->SetObjectShadowLOD(m_hObject, eEngineLOD_Never);

	m_ActivateTypeHandler.Init( m_hObject );
	m_ActivateTypeHandler.SetState( ACTIVATETYPE::eOn, false );
	m_ActivateTypeHandler.SetDisabled( false, false );
}

uint32 SpecialMove::OnAllObjectsCreated()
{
	CAutoMessage cMsg;
	WriteSFXMsg(cMsg);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());

	return GameBase::OnAllObjectsCreated();
}

void SpecialMove::WriteSFXMsg(CAutoMessage& cMsg)
{
	// Set our special effect message.
	cMsg.Writeuint8(GetSFXID());
	cMsg.Writeuint32(m_eAnimation);
	cMsg.Writefloat(m_fActivateDist);
	cMsg.Writebool(m_bOn);
	cMsg.Writebool(m_bRadial);

	// Piggyback our Activate data.
	m_ActivateTypeHandler.WriteActivateTypeMsg(cMsg);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandleActivateMsg
//
//  PURPOSE:	Handle a ACTIVATE message...
//
// ----------------------------------------------------------------------- //

void SpecialMove::HandleActivateMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg  )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( GetSFXID() );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( SPECIALMOVEFX_ACTIVATE );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandleOnMsg
//
//  PURPOSE:	Handle a ON message...
//
// ----------------------------------------------------------------------- //

void SpecialMove::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg  )
{
	SetEnabled(true);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ActiveWorldModel::HandleOffMsg
//
//  PURPOSE:	Handle a OFF message...
//
// ----------------------------------------------------------------------- //

void SpecialMove::HandleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg  )
{
	SetEnabled(false);
}

void SpecialMove::SetEnabled(bool bOn)
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( GetSFXID() );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( bOn ? SPECIALMOVEFX_ON : SPECIALMOVEFX_OFF );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecialMove::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void SpecialMove::Save( ILTMessage_Write* pMsg, uint32 nFlags )
{
	if( !pMsg ) return;

	SAVE_INT(m_eAnimation);
	SAVE_FLOAT(m_fActivateDist);
	SAVE_BOOL(m_bOn);
	SAVE_BOOL(m_bRadial);
	SAVE_STDSTRING(m_sActivateCommand);
	SAVE_STDSTRING(m_sReleaseCommand);
	SAVE_STDSTRING(m_sLookedAtCommand);
	m_ActivateTypeHandler.Save( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ForensicObject::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void SpecialMove::Load( ILTMessage_Read* pMsg, uint32 nFlags )
{
	if( !pMsg ) return;

	LOAD_INT_CAST(m_eAnimation, EnumAnimProp);
	LOAD_FLOAT(m_fActivateDist);
	LOAD_BOOL(m_bOn);
	LOAD_BOOL(m_bRadial);
	LOAD_STDSTRING(m_sActivateCommand);
	LOAD_STDSTRING(m_sReleaseCommand);
	LOAD_STDSTRING(m_sLookedAtCommand);
	m_ActivateTypeHandler.Load( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecialMovePlugin::PreHook_EditStringList
//
//	PURPOSE:	build string lists for world edit dropdowns
//
// ----------------------------------------------------------------------- //

SpecialMovePlugin::SpecialMovePlugin()
{
}

SpecialMovePlugin::~SpecialMovePlugin()
{
}

LTRESULT SpecialMovePlugin::PreHook_EditStringList(	const char* szRezPath,
													const char* szPropName,
													char** aszStrings,
													uint32* pcStrings,
													const uint32 cMaxStrings,
													const uint32 cMaxStringLength)
{
	if (LTStrIEquals("Animation", szPropName))
	{
		for (int i = 0; i < AnimPropUtils::Count(); i++)
		{
			const char* pszAnim = AnimPropUtils::String(EnumAnimProp(i));
			if (LTSubStrIEquals(pszAnim, SPECIALMOVE_PREFIX, LTARRAYSIZE(SPECIALMOVE_PREFIX)-1))
			{
				LTStrCpy(aszStrings[(*pcStrings)++], pszAnim, cMaxStringLength);
			}
		}
		return LT_OK;
	}
	else if (LTStrIEquals("ActivationType", szPropName))
	{
		if( CategoryPlugin::Instance().PopulateStringList( DATABASE_CATEGORY( Activate ).GetCategory(), 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength ))
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CForensicObjectPlugin::PreHook_PropChanged
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

LTRESULT SpecialMovePlugin::PreHook_PropChanged(	const char *szObjName, 
													const char *szPropName,
													const int nPropType,
													const GenericProp &gpPropValue,
													ILTPreInterface *pInterface,
													const char *szModifiers )
{
	// Only our commands are marked for change notification so just send it to the CommandMgr..

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

