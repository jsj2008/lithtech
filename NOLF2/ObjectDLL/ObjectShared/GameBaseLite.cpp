// ----------------------------------------------------------------------- //
//
// MODULE  : GameBaseLite.cpp
//
// PURPOSE : "Lite" game base object class definition  (For game objects without an HOBJECT)
//
// CREATED : 7/12/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GameBaseLite.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "VersionMgr.h"

LINKFROM_MODULE( GameBaseLite );

BEGIN_CLASS(GameBaseLite)
	PROP_DEFINEGROUP(GameType, PF_GROUP(16))
		ADD_BOOLPROP_FLAG(SinglePlayer, 1, PF_GROUP(16))
		ADD_BOOLPROP_FLAG(Cooperative, 1, PF_GROUP(16))
		ADD_BOOLPROP_FLAG(Deathmatch, 1, PF_GROUP(16))
		ADD_BOOLPROP_FLAG(TeamDeathmatch, 1, PF_GROUP(16))
		ADD_BOOLPROP_FLAG(DoomsDay, 1, PF_GROUP(16))
END_CLASS_DEFAULT_FLAGS(GameBaseLite, BaseClass, NULL, NULL, CF_HIDDEN | CF_CLASSONLY)

GameBaseLite::GameBaseLite(bool bStartActive) :
	BaseClass(),
	m_bActive(bStartActive),
	m_hClass(0),
	m_nSerializeID(0),
	m_bRegistered(false)
{
	// Clear out the invalid data from BaseClass
	m_hObject = 0;
	m_nType = OT_NORMAL;
}

GameBaseLite::~GameBaseLite()
{
	ASSERT(!g_pGameServerShell->GetLiteObjectMgr()->HasObject(this));
}

uint32 GameBaseLite::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
    if (!g_pLTServer) return 0;

	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();
	switch(messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)pMsg->Readuint32();
			TriggerMsg(hSender, szMsg);
		}
		break;

		default : break;
	}

	return BaseClass::ObjectMessageFn(hSender, pMsg);
}

void GameBaseLite::Save(ILTMessage_Write *pMsg)
{
	SAVE_DWORD(g_pVersionMgr->GetSaveVersion());
	SAVE_bool(m_bActive);
}

void GameBaseLite::Load(ILTMessage_Read *pMsg)
{
	// This old version didn't save the version number at the top.  We need
	// to use an engine value to determine the version.
	uint32 nSaveFileVersion = 0;
	g_pLTServer->GetSaveFileVersion( nSaveFileVersion );
	if( nSaveFileVersion == 2001 )
	{
		LOAD_bool(m_bActive);
		uint32 nSaveVersion;
		LOAD_DWORD(nSaveVersion);
		g_pVersionMgr->SetCurrentSaveVersion( nSaveVersion );
		HSTRING hDummy = NULL;
		LOAD_HSTRING(hDummy);
		g_pLTServer->FreeString( hDummy );
		hDummy = NULL;
	}
	else
	{
		uint32 nSaveVersion;
		LOAD_DWORD(nSaveVersion);
		g_pVersionMgr->SetCurrentSaveVersion( nSaveVersion );
		LOAD_bool(m_bActive);
	}
}

void GameBaseLite::InitialUpdate(float fUpdateType)
{
	// If someone's telling us to update, we're obviously registered.  (Needed for proper load behavior.)
	m_bRegistered = true;
}

void GameBaseLite::Activate()
{
	if (m_bActive)
		return;

	m_bActive = true;

	// Tell the ObjectLiteMgr to activate us
	g_pGameServerShell->GetLiteObjectMgr()->ActivateObject(this);
}

void GameBaseLite::Deactivate()
{
	if (!m_bActive)
		return;

	m_bActive = false;
	
	// Tell the ObjectLiteMgr to de-activate us
	g_pGameServerShell->GetLiteObjectMgr()->DeactivateObject(this);
}

void GameBaseLite::SetName(const char *pName)
{
	if (m_bRegistered)
	{
		m_bRegistered = false;
		g_pGameServerShell->GetLiteObjectMgr()->RenameObject(this, pName);
		m_bRegistered = true;
		return;
	}
	m_sName = pName;
}

bool GameBaseLite::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Remove("REMOVE");

	if (cMsg.GetArg(0) == s_cTok_Remove)
	{
		// Remove ourselves from the ObjectLiteMgr
		g_pGameServerShell->GetLiteObjectMgr()->RemoveObject(this);
	}
	else
	{
		return false;
	}

	return true;
}

void GameBaseLite::TriggerMsg(HOBJECT hSender, const char* pMsg)
{
	if (!pMsg) return;

	ConParse parse;
	parse.Init(pMsg);

	while (g_pCommonLT->Parse(&parse) == LT_OK)
	{
		// Don't parse empty messages
		if (!parse.m_nArgs || !parse.m_Args[0])
			continue;

		CParsedMsg cCurMsg(parse.m_nArgs, parse.m_Args);
		OnTrigger(hSender, cCurMsg);
	}
}

uint32 GameBaseLite::OnPrecreate(ObjectCreateStruct* pOCS, float precreateType)
{
	if (!BaseClass::OnPrecreate(pOCS, precreateType))
		return 0;

	// Save the class
	SetClass(pOCS->m_hClass);

	// Save the name
	SetName(pOCS->m_Name);
	ASSERT(*GetName() && "Creating lite object without a name!");

	// Read our properties
	if (!ReadProp(pOCS))
		return 0;

	// Add ourselves to the ObjectLiteMgr
	g_pGameServerShell->GetLiteObjectMgr()->AddObject(this);
	m_bRegistered = true;

	return 1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBaseLite::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool GameBaseLite::ReadProp( ObjectCreateStruct* pStruct )
{
	if( !pStruct )
		return false;

	// Read properties based on gametype.
	switch( g_pGameServerShell->GetGameType( ))
	{
		case eGameTypeSingle:
		{
			if( !pStruct->m_cProperties.GetPropBool( "SinglePlayer", true ))
				return false;
		}
		break;
		case eGameTypeCooperative:
		{
			if( !pStruct->m_cProperties.GetPropBool( "Cooperative", true ))
				return false;
		}
		break;
		case eGameTypeDeathmatch:
		{
			if( !pStruct->m_cProperties.GetPropBool( "Deathmatch", true ))
				return false;
		}
		break;
		case eGameTypeTeamDeathmatch:
		{
			if( !pStruct->m_cProperties.GetPropBool( "TeamDeathmatch", true ))
				return false;
		}
		break;
		case eGameTypeDoomsDay:
		{
			if( !pStruct->m_cProperties.GetPropBool( "DoomsDay", true ))
				return false;
		}	
		break;
	}

	return true;
}
