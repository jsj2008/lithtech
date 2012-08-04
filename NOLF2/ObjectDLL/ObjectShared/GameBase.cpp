// ----------------------------------------------------------------------- //
//
// MODULE  : GameBase.cpp
//
// PURPOSE : Game base object class implementation
//
// CREATED : 10/8/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GameBase.h"
#include "CVarTrack.h"
#include "ObjectMsgs.h"
#include "CommandMgr.h"
#include "VersionMgr.h"
#include "TransitionAggregate.h"
#include "ClientServerShared.h"
#include "ParsedMsg.h"
#include "ObjectTemplateMgr.h"
#include "PlayerObj.h"

extern CVarTrack g_ShowDimsTrack;

CVarTrack	g_vtDimsAlpha;

LINKFROM_MODULE( GameBase );


BEGIN_CLASS(GameBase)
	ADD_BOOLPROP(Template, LTFALSE)
	PROP_DEFINEGROUP(GameType, PF_GROUP(16))
		ADD_BOOLPROP_FLAG(SinglePlayer, 1, PF_GROUP(16))
		ADD_BOOLPROP_FLAG(Cooperative, 1, PF_GROUP(16))
		ADD_BOOLPROP_FLAG(Deathmatch, 1, PF_GROUP(16))
		ADD_BOOLPROP_FLAG(TeamDeathmatch, 1, PF_GROUP(16))
		ADD_BOOLPROP_FLAG(DoomsDay, 1, PF_GROUP(16))
END_CLASS_DEFAULT_FLAGS(GameBase, BaseClass, NULL, NULL, CF_HIDDEN)


static CBankedList<CTransitionAggregate> s_bankCTransAggs;

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateMsgAttachDetach
//
//  PURPOSE:	Validation message for both ATTACH and DETACH messages
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateMsgVisibleSolid( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	char *pBoolValue = cpMsgParams.m_Args[1];

	if( (!_stricmp(cpMsgParams.m_Args[1], "1")) ||
		(!_stricmp(cpMsgParams.m_Args[1], "TRUE")) ||
		(!_stricmp(cpMsgParams.m_Args[1], "0")) ||
		(!_stricmp(cpMsgParams.m_Args[1], "FALSE")) )
	{
		return LTTRUE;
	}
	
	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->CPrint( "ERROR! - ValidateMsgVisibleSolid()" );
		pInterface->CPrint( "    MSG - %s - 2nd argument '%s' is not a valid bool value.", _strupr(cpMsgParams.m_Args[0]), cpMsgParams.m_Args[1] );
	}
	
	return LTFALSE;
}

//
// Register the calss with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( GameBase )

	CMDMGR_ADD_MSG( VISIBLE, 2, ValidateMsgVisibleSolid, "VISIBLE <bool>" )
	CMDMGR_ADD_MSG( SOLID, 2, ValidateMsgVisibleSolid, "SOLID <bool>" )
	CMDMGR_ADD_MSG( HIDDEN, 2, ValidateMsgVisibleSolid, "HIDDEN <bool>" )
	CMDMGR_ADD_MSG( REMOVE, 1, NULL, "REMOVE" )

CMDMGR_END_REGISTER_CLASS( GameBase, BaseClass )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::GameBase()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

GameBase::GameBase(uint8 nType) : BaseClass(nType)
{
    m_hDimsBox.SetReceiver( *this );
	m_dwOriginalFlags	= 0;

	m_pTransAgg			= LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::~GameBase()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

GameBase::~GameBase()
{
	DestroyTransitionAggregate();

	RemoveBoundingBox();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GameBase::MakeTransitionable
//
//  PURPOSE:	Setup the transition aggregate...
//
// ----------------------------------------------------------------------- //

void GameBase::MakeTransitionable( )
{
	// Only want one per object

	ASSERT( m_pTransAgg == LTNULL );

	// Create it and add it...

	m_pTransAgg = s_bankCTransAggs.New();
	if( !m_pTransAgg )
	{
		g_pLTServer->CPrint( "new TransAgg FAILED!!!" );
		return;
	}
	AddAggregate( (LPAGGREGATE)m_pTransAgg );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GameBase::DestroyTransitionAggregate
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

void GameBase::DestroyTransitionAggregate( )
{
	if( !m_pTransAgg ) return;

#ifndef _FINAL
	CTransitionAggregate *pIAgg = dynamic_cast<CTransitionAggregate*>(m_pTransAgg);
	if( !pIAgg )
	{
		g_pLTServer->CPrint( "NOT a CTransitionAggregate!!!" );
		return;
	}
#endif

	RemoveAggregate( (LPAGGREGATE)m_pTransAgg );
	s_bankCTransAggs.Delete( m_pTransAgg );
	m_pTransAgg = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GameBase::AddToObjectList
//
//  PURPOSE:	Add ourselves to the list...
//
// ----------------------------------------------------------------------- //

void GameBase::AddToObjectList( ObjectList *pObjList, eObjListControl eControl /*= eObjListNODuplicates*/  )
{
	if( !pObjList ) return;

	// Just add us...

	AddObjectToList( pObjList, m_hObject, eControl );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 GameBase::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
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

			// Make sure other people can read it...

			pMsg->SeekTo(0);
		}
		break;

		default : break;
	}

	return BaseClass::ObjectMessageFn(hSender, pMsg);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::TriggerMsg()
//
//	PURPOSE:	Route trigger messages through the standard processing method
//
// --------------------------------------------------------------------------- //

void GameBase::TriggerMsg(HOBJECT hSender, const char* szMsg)
{
	if (!szMsg) return;

	// ConParse does not destroy szMsg, so this is safe
	ConParse parse;
	parse.Init((char*)szMsg);

	while (g_pCommonLT->Parse(&parse) == LT_OK)
	{
		// Don't parse empty messages
		if (!parse.m_nArgs || !parse.m_Args[0])
			continue;

		CParsedMsg cCurMsg(parse.m_nArgs, parse.m_Args);
		OnTrigger(hSender, cCurMsg);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::OnTrigger()
//
//	PURPOSE:	Process GameBase trigger messages 
//
// --------------------------------------------------------------------------- //

bool GameBase::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_1("1");
	static CParsedMsg::CToken s_cTok_True("TRUE");
	static CParsedMsg::CToken s_cTok_0("0");
	static CParsedMsg::CToken s_cTok_False("FALSE");
	static CParsedMsg::CToken s_cTok_Visible("VISIBLE");
	static CParsedMsg::CToken s_cTok_Solid("SOLID");
	static CParsedMsg::CToken s_cTok_Hidden("HIDDEN");
	static CParsedMsg::CToken s_cTok_Remove("REMOVE");

    uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	if (!m_dwOriginalFlags)
	{
		m_dwOriginalFlags = dwFlags;
	}

	if (cMsg.GetArg(0) == s_cTok_Visible)
	{
		if (cMsg.GetArgCount() > 1)
		{
			if ((cMsg.GetArg(1) == s_cTok_1) ||
				(cMsg.GetArg(1) == s_cTok_True))
			{
				dwFlags |= FLAG_VISIBLE;
			}
			else
			{
				if ((cMsg.GetArg(1) == s_cTok_0) ||
					(cMsg.GetArg(1) == s_cTok_False))
				{
					dwFlags &= ~FLAG_VISIBLE;
				}
			}

			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, dwFlags, FLAGMASK_ALL);
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_Solid)
	{
		if (cMsg.GetArgCount() > 1)
		{
			if ((cMsg.GetArg(1) == s_cTok_1) ||
				(cMsg.GetArg(1) == s_cTok_True))
			{
				dwFlags |= FLAG_SOLID;

				if (m_dwOriginalFlags & FLAG_RAYHIT)
				{
					dwFlags |= FLAG_RAYHIT;
				}
			}
			else
			{
				if ((cMsg.GetArg(1) == s_cTok_0) ||
					(cMsg.GetArg(1) == s_cTok_False))
				{
					dwFlags &= ~FLAG_SOLID;
					dwFlags &= ~FLAG_RAYHIT;
				}
			}

			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, dwFlags, FLAGMASK_ALL);
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_Hidden)
	{
		if (cMsg.GetArgCount() > 1)
		{
			if ((cMsg.GetArg(1) == s_cTok_1) ||
				(cMsg.GetArg(1) == s_cTok_True))
			{
				dwFlags &= ~FLAG_SOLID;
				dwFlags &= ~FLAG_RAYHIT;
				dwFlags &= ~FLAG_VISIBLE;
			}
			else
			{
				if ((cMsg.GetArg(1) == s_cTok_0) ||
					(cMsg.GetArg(1) == s_cTok_False))
				{
					// Not all objects should have solid set to true.
					// (e.g. AIs should never set solid true)

					if( m_dwOriginalFlags & FLAG_SOLID )
					{
						dwFlags |= FLAG_SOLID;
					}

					if( m_dwOriginalFlags & FLAG_VISIBLE )
					{
						dwFlags |= FLAG_VISIBLE;
					}

					if (m_dwOriginalFlags & FLAG_RAYHIT)
					{
						dwFlags |= FLAG_RAYHIT;
					}
				}
			}

			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, dwFlags, FLAGMASK_ALL);
		}
	}
	else if ( cMsg.GetArg(0) == s_cTok_Remove )
	{
		CPlayerObj* pPlayerObj = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( m_hObject ));
		if( pPlayerObj )
		{
			ASSERT( !"GameBase::OnTrigger:  Tried to remove player object with trigger." );
			g_pLTServer->CPrint( "Tried to remove player object with trigger" );
			return false;
		}

		g_pLTServer->RemoveObject( m_hObject );
	}
	else
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::CreateBoundingBox()
//
//	PURPOSE:	Create a bounding box
//
// ----------------------------------------------------------------------- //

void GameBase::CreateBoundingBox()
{
	if (m_hDimsBox) return;

	if (!g_vtDimsAlpha.IsInitted())
	{
        g_vtDimsAlpha.Init(g_pLTServer, "DimsAlpha", LTNULL, 1.0f);
	}

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	theStruct.m_Pos = vPos;

	SAFE_STRCPY(theStruct.m_Filename, "Models\\1x1_square.ltb");
	SAFE_STRCPY(theStruct.m_SkinName, "Models\\1x1_square.dtx");

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT | FLAG_GOTHRUWORLD;
	theStruct.m_ObjectType = OT_MODEL;

    HCLASS hClass = g_pLTServer->GetClass("BaseClass");
    LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);

	if (pModel)
	{
		m_hDimsBox = pModel->m_hObject;

		// Don't eat ticks please...
		::SetNextUpdate(m_hDimsBox, UPDATE_NEVER);

        LTVector vDims;
		g_pPhysicsLT->GetObjectDims(m_hObject, &vDims);

        LTVector vScale;
		VEC_DIVSCALAR(vScale, vDims, 0.5f);
        g_pLTServer->ScaleObject(m_hDimsBox, &vScale);
	}


    LTVector vOffset;
	vOffset.Init();
    LTRotation rOffset;

	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, m_hDimsBox, LTNULL,
											     &vOffset, &rOffset, &hAttachment);
    if (dRes != LT_OK)
	{
        g_pLTServer->RemoveObject(m_hDimsBox);
        m_hDimsBox = LTNULL;
	}

    LTVector vColor = GetBoundingBoxColor();

    g_pLTServer->SetObjectColor(m_hDimsBox, vColor.x, vColor.y, vColor.z, g_vtDimsAlpha.GetFloat());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::RemoveBoundingBox()
//
//	PURPOSE:	Remove the bounding box
//
// ----------------------------------------------------------------------- //

void GameBase::RemoveBoundingBox()
{
	if (m_hDimsBox)
	{
		HATTACHMENT hAttachment;
        if (g_pLTServer->FindAttachment(m_hObject, m_hDimsBox, &hAttachment) == LT_OK)
		{
            g_pLTServer->RemoveAttachment(hAttachment);
		}

        g_pLTServer->RemoveObject(m_hDimsBox);
        m_hDimsBox = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 GameBase::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
        case MID_ACTIVATING:
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_GAMEBASE_ACTIVE, USRFLG_GAMEBASE_ACTIVE);
		}
		break;

		case MID_DEACTIVATING:
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_GAMEBASE_ACTIVE);
		}
		break;

        case MID_PRECREATE:
		{
            uint32 dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP || nInfo == PRECREATE_NORMAL)
			{
				ObjectCreateStruct* pocs = (ObjectCreateStruct*)pData;
				if( !ReadProp( pocs ))
					return 0;
			}

			return dwRet;
		}
		break;

		case MID_MODELSTRINGKEY:
		{
            uint32 dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			// Let the CmdMgr take a crack at it.

			ArgList* pArgList = (ArgList*)pData;

			char szBuffer[256];
			sprintf(szBuffer, "");

			for ( int iArg = 0 ; iArg < pArgList->argc ; iArg++ )
			{
				strcat(szBuffer, pArgList->argv[iArg]);
				strcat(szBuffer, " ");
			}

			szBuffer[strlen(szBuffer)-1] = 0;

			g_pCmdMgr->Process(szBuffer, m_hObject, m_hObject);

			return dwRet;
		}
		break;
		
		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData);
		}
		break;

		default:
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::GetBoundingBoxColor()
//
//	PURPOSE:	Get the color of the bounding box
//
// ----------------------------------------------------------------------- //

LTVector GameBase::GetBoundingBoxColor()
{
    LTVector vColor(1, 1, 1);
	switch (GetType())
	{
		case OT_MODEL :
			 vColor.Init(1, 0, 0);
		break;

		case OT_WORLDMODEL :
			 vColor.Init(0, 0, 1);
		break;

		case OT_NORMAL :
		default :
			 vColor.Init(1, 1, 1);
		break;
	}

	return vColor;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::UpdateBoundingBox()
//
//	PURPOSE:	Update bounding box
//
// ----------------------------------------------------------------------- //

void GameBase::UpdateBoundingBox()
{
	int nVal = (int)g_ShowDimsTrack.GetFloat();

	if (nVal < 4)
	{
		switch (GetType())
		{
			case OT_WORLDMODEL :
			{
				if (nVal != 1)
				{
					RemoveBoundingBox();
					return;
				}
			}
			break;

			case OT_MODEL :
			{
				if (nVal != 2)
				{
					RemoveBoundingBox();
					return;
				}
			}
			break;

			case OT_NORMAL :
			{
				if (nVal != 3)
				{
					RemoveBoundingBox();
					return;
				}
			}
			break;

			default :
			break;
		}
	}

	CreateBoundingBox();

	if (m_hDimsBox)
	{
        LTVector vDims, vScale;
		g_pPhysicsLT->GetObjectDims(m_hObject, &vDims);
		vScale = (vDims * 2.0);
        g_pLTServer->ScaleObject(m_hDimsBox, &vScale);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::SetNextUpdate()
//
//	PURPOSE:	Allows objects to set their next update time and at
//				time same time update autodeactivation
//
// ----------------------------------------------------------------------- //

void GameBase::SetNextUpdate(LTFLOAT fDelta, eUpdateControl eControl)
{
	if (!m_hObject) return;

	fDelta = fDelta <= 0.0f ? 0.0f : fDelta;
    g_pLTServer->SetNextUpdate(m_hObject, fDelta);

	if (eControl == eControlDeactivation)
	{
		if (fDelta == 0.0f)
		{
			g_pLTServer->SetObjectState(m_hObject, OBJSTATE_INACTIVE);
		}
		else
		{
			g_pLTServer->SetObjectState(m_hObject, OBJSTATE_ACTIVE);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void GameBase::Save(ILTMessage_Write *pMsg)
{
	SAVE_DWORD(g_pVersionMgr->GetSaveVersion());
	SAVE_DWORD(m_dwOriginalFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void GameBase::Load(ILTMessage_Read *pMsg)
{
	// This old version didn't save the version number at the top.  We need
	// to use an engine value to determine the version.
	uint32 nSaveFileVersion = 0;
	g_pLTServer->GetSaveFileVersion( nSaveFileVersion );
	if( nSaveFileVersion == 2001 )
	{
		LOAD_DWORD(m_dwOriginalFlags);
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
		LOAD_DWORD(m_dwOriginalFlags);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::OnLinkBroken
//
//	PURPOSE:	Handle attached object getting removed.
//
// ----------------------------------------------------------------------- //

void GameBase::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if( pRef == &m_hDimsBox )
	{
		HATTACHMENT hAttachment;
		if ( LT_OK == g_pLTServer->FindAttachment( m_hObject, hObj, &hAttachment) )
		{
			g_pLTServer->RemoveAttachment(hAttachment);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool GameBase::ReadProp( ObjectCreateStruct* pStruct )
{
	if( !pStruct )
		return false;

	// Check for a template object
	if( pStruct->m_cProperties.GetPropBool("Template", false))
	{
		// Save the template
		g_pGameServerShell->GetObjectTemplates()->AddTemplate( pStruct );

		// Don't actually create the object
		return false;
	}
	if ( !( pStruct->m_Name[0] ))
	{
        static int s_nUniqueId = 0;
		sprintf( pStruct->m_Name, "noname%d", s_nUniqueId++);
	}

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
