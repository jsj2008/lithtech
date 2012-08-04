// ----------------------------------------------------------------------- //
//
// MODULE  : GameBase.cpp
//
// PURPOSE : Game base object class implementation
//
// CREATED : 10/8/99
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "GameBase.h"
#include "ObjectMsgs.h"
#include "CommandMgr.h"
#include "VersionMgr.h"
#include "TransitionAggregate.h"
#include "ClientServerShared.h"
#include "ParsedMsg.h"
#include "ObjectTemplateMgr.h"
#include "PlayerObj.h"
#include "EngineLODPropUtil.h"

static CParsedMsg::CToken s_cTok_1("1");
static CParsedMsg::CToken s_cTok_True("TRUE");
static CParsedMsg::CToken s_cTok_0("0");
static CParsedMsg::CToken s_cTok_False("FALSE");

LINKFROM_MODULE( GameBase );


BEGIN_CLASS(GameBase)
	ADD_BOOLPROP(Template, false, "Indicates that this object will not exist at runtime, and may be used as an object template for Spawner objects")
	ADD_BOOLPROP(IsGore, false, "Indicates that this object is gory and should not exist for low-violence clients." )
	ADD_STRINGPROP_FLAG( ObjectLOD, "Low", PF_STATICLIST, "This value indicates at which detail levels the object will exist. For example at Low, this object will always exist, at Medium it will only exist in medium and high detail, and Never will cause the object to never exist.")
END_CLASS_FLAGS_PLUGIN(GameBase, BaseClass, CF_HIDDEN, CGameBasePlugin, "This is the Base class for all game objects.  All objects in the game are derived from this class.")

//
// Plugin class implementation...
//

LTRESULT CGameBasePlugin::PreHook_EditStringList( const char *szRezPath, 
												  const char *szPropName,
												  char **aszStrings,
												  uint32 *pcStrings,
												  const uint32 cMaxStrings,
												  const uint32 cMaxStringLength )
{
	if( LTStrIEquals( szPropName, "ObjectLOD" ) )
	{
		return CEngineLODPropUtil::AddLODStrings(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
	}

	return LT_UNSUPPORTED;
}


static CBankedList<CTransitionAggregate> s_bankCTransAggs;

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateMsgBool
//
//  PURPOSE:	Validation message for both any boolean type messages...
//
// ----------------------------------------------------------------------- //

bool ValidateMsgBool( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( (LTStrIEquals( cpMsgParams.m_Args[1], "1" )) ||
		(LTStrIEquals( cpMsgParams.m_Args[1], "TRUE" )) ||
		(LTStrIEquals( cpMsgParams.m_Args[1], "0" )) ||
		(LTStrIEquals( cpMsgParams.m_Args[1], "FALSE" )) )
	{
		return true;
	}
	
	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->CPrint( "ERROR! - ValidateMsgBool()" );
		pInterface->CPrint( "    MSG - %s - 2nd argument '%s' is not a valid bool value.", LTStrUpr(cpMsgParams.m_Args[0]), cpMsgParams.m_Args[1] );
	}
	
	return false;
}

//
// Register the calss with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( GameBase )

	ADD_MESSAGE( VISIBLE,		2,	ValidateMsgBool,	MSG_HANDLER( GameBase, HandleVisibleMsg ),	"VISIBLE &lt;bool&gt;", "Toggles whether the object is visible.", "msg &lt;ObjectName&gt; (VISIBLE 1)&lt;BR&gt;msg &lt;ObjectName&gt; (VISIBLE 0)" )
	ADD_MESSAGE( SOLID,			2,	ValidateMsgBool,	MSG_HANDLER( GameBase, HandleSolidMsg ),	"SOLID &lt;bool&gt;", "Toggles whether the object is solid.", "msg &lt;ObjectName&gt; (SOLID 1)&lt;BR&gt;msg &lt;ObjectName&gt; (SOLID 0)" )
	ADD_MESSAGE( HIDDEN,		2,	ValidateMsgBool,	MSG_HANDLER( GameBase, HandleHiddenMsg ),	"HIDDEN &lt;bool&gt;", "Toggles whether the object is visible, solid, and rayhit.", "msg &lt;ObjectName&gt; (HIDDEN 1)&lt;BR&gt;msg &lt;ObjectName&gt; (HIDDEN 0)" )
	ADD_MESSAGE( REMOVE,		1,	NULL,				MSG_HANDLER( GameBase, HandleRemoveMsg ),	"REMOVE", "Removes the object from the game permanently.", "msg &lt;ObjectName&gt; REMOVE" )
	ADD_MESSAGE( CASTSHADOW,	2,	ValidateMsgBool,	MSG_HANDLER( GameBase, HandleCastShadowMsg ),	"CASTSHADOW &lt;bool&gt;", "Toggles whether the object casts a shadow.", "msg &lt;ObjectName&gt; (CASTSHADOW 1)&lt;BR&gt;msg &lt;ObjectName&gt; (CASTSHADOW 0)" )
	ADD_MESSAGE( SETPOS,		4,	NULL,				MSG_HANDLER( GameBase, HandleMoveMsg ),		"SETPOS &lt;X&gt; &lt;Y&gt; &lt;Z&gt;", "DEBUGGING ONLY.  Sets the position of an object.", "msg &lt;ObjectName&gt; (MOVETOPOS 50.5 100 75)"  )
	ADD_MESSAGE( MOVETOPOS,		4,	NULL,				MSG_HANDLER( GameBase, HandleMoveMsg ),		"MOVETOPOS &lt;X&gt; &lt;Y&gt; &lt;Z&gt;", "DEBUGGING ONLY.  Moves to the position, with collisions.", "msg &lt;ObjectName&gt; (MOVETOPOS 50.5 100 75)" )
	ADD_MESSAGE( SETROTATION,	4,	NULL,				MSG_HANDLER( GameBase, HandleSetRotationMsg ), "SETROTATION &lt;yaw&gt; &lt;pitch&gt; &lt;roll&gt;", "DEBUGGING ONLY.  Rotates object to euler angles.", "msg &lt;ObjectName&gt; (SETROTATION 90.0 0 0)" )
	ADD_MESSAGE( COPYXFORM,		2,	NULL,				MSG_HANDLER( GameBase, HandleCopyXFormMsg ), "COPYXFORM &lt;object&gt;", "DEBUGGING ONLY.  Copies trasform of &lt;object&gt; to this object.", "msg &lt;ObjectName&gt; (COPYXFORM ThePoint)" )

CMDMGR_END_REGISTER_CLASS( GameBase, BaseClass )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::GameBase()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

GameBase::GameBase(uint8 nType)
:	BaseClass				( nType ),
	m_dwOriginalFlags		( 0 ),
	m_pTransAgg				( NULL ),
	m_eOriginalShadowLOD	( eEngineLOD_Never )
{
	
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

	ASSERT( m_pTransAgg == NULL );

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
	m_pTransAgg = NULL;
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
//	ROUTINE:	GameBase::HandleVisibleMsg
//
//	PURPOSE:	Handle a VISIBLE message...
//
// ----------------------------------------------------------------------- //

void GameBase::HandleVisibleMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	if( !m_dwOriginalFlags )
	{
		m_dwOriginalFlags = dwFlags;
	}

	if ((crParsedMsg.GetArg(1) == s_cTok_1) ||
		(crParsedMsg.GetArg(1) == s_cTok_True))
	{
		dwFlags |= FLAG_VISIBLE;
	}
	else
	{
		if ((crParsedMsg.GetArg(1) == s_cTok_0) ||
			(crParsedMsg.GetArg(1) == s_cTok_False))
		{
			dwFlags &= ~FLAG_VISIBLE;
		}
	}

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, dwFlags, FLAGMASK_ALL);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::HandleSolidMsg
//
//	PURPOSE:	Handle a SOLID message...
//
// ----------------------------------------------------------------------- //

void GameBase::HandleSolidMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	if( !m_dwOriginalFlags )
	{
		m_dwOriginalFlags = dwFlags;
	}

	if ((crParsedMsg.GetArg(1) == s_cTok_1) ||
		(crParsedMsg.GetArg(1) == s_cTok_True))
	{
		dwFlags |= FLAG_SOLID;

		if (m_dwOriginalFlags & FLAG_RAYHIT)
		{
			dwFlags |= FLAG_RAYHIT;
		}
	}
	else
	{
		if ((crParsedMsg.GetArg(1) == s_cTok_0) ||
			(crParsedMsg.GetArg(1) == s_cTok_False))
		{
			dwFlags &= ~FLAG_SOLID;
			dwFlags &= ~FLAG_RAYHIT;
		}
	}

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, dwFlags, FLAGMASK_ALL);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::HandleHiddenMsg
//
//	PURPOSE:	Handle a HIDDEN message...
//
// ----------------------------------------------------------------------- //

void GameBase::HandleHiddenMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	if( !m_dwOriginalFlags )
	{
		m_dwOriginalFlags = dwFlags;
	}

	EEngineLOD eShadowLOD;
	g_pLTServer->GetObjectShadowLOD( m_hObject, eShadowLOD );
	if( eShadowLOD != eEngineLOD_Never )
	{
		m_eOriginalShadowLOD = eShadowLOD;
	}
	
	if ((crParsedMsg.GetArg(1) == s_cTok_1) ||
		(crParsedMsg.GetArg(1) == s_cTok_True))
	{
		dwFlags &= ~FLAG_SOLID;
		dwFlags &= ~FLAG_RAYHIT;
		dwFlags &= ~FLAG_VISIBLE;

		eShadowLOD = eEngineLOD_Never;
	}
	else
	{
		if ((crParsedMsg.GetArg(1) == s_cTok_0) ||
			(crParsedMsg.GetArg(1) == s_cTok_False))
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

			eShadowLOD = m_eOriginalShadowLOD;
		}
	}

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, dwFlags, FLAGMASK_ALL);
	g_pLTServer->SetObjectShadowLOD( m_hObject, eShadowLOD );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::HandleRemoveMsg
//
//	PURPOSE:	Handle a REMOVE message...
//
// ----------------------------------------------------------------------- //

void GameBase::HandleRemoveMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Do not allow the removal of a player with a message...

	CPlayerObj* pPlayerObj = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( m_hObject ));
	if( pPlayerObj )
	{
		LTERROR( "GameBase::HandleRemoveMsg:  Tried to remove player object with a message." );
		g_pLTServer->CPrint( "Tried to remove player object with a message" );
		return;
	}

	g_pLTServer->RemoveObject( m_hObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::HandleCastShadowMsg
//
//	PURPOSE:	Handle a CASTSHADOW message...
//
// ----------------------------------------------------------------------- //

void GameBase::HandleCastShadowMsg(  HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	EEngineLOD eShadowLOD;
	g_pLTServer->GetObjectShadowLOD( m_hObject, eShadowLOD );
	if( eShadowLOD != eEngineLOD_Never )
	{
		m_eOriginalShadowLOD = eShadowLOD;
	}

	if ((crParsedMsg.GetArg(1) == s_cTok_1) ||
		(crParsedMsg.GetArg(1) == s_cTok_True))
	{
		eShadowLOD = m_eOriginalShadowLOD;
	}
	else
	{
		if ((crParsedMsg.GetArg(1) == s_cTok_0) ||
			(crParsedMsg.GetArg(1) == s_cTok_False))
		{
			eShadowLOD = eEngineLOD_Never;
		}
	}

	g_pLTServer->SetObjectShadowLOD( m_hObject, eShadowLOD );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::HandleMoveMsg
//
//	PURPOSE:	Handle a SETPOS and MOVETOPOS message...
//
// ----------------------------------------------------------------------- //

void GameBase::HandleMoveMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Make sure we have all the arguments.
	if( crParsedMsg.GetArgCount( ) < 4 )
		return;

	static CParsedMsg::CToken s_cTok_SetPos( "SETPOS" );
	bool bSetPos = ( crParsedMsg.GetArg( 0 ) == s_cTok_SetPos );

	// Get the position they specified.  This is in WorldEdit space.
	LTVector vPos;
	vPos.x = ( float )atof( crParsedMsg.GetArg( 1 ));
	vPos.y = ( float )atof( crParsedMsg.GetArg( 2 ));
	vPos.z = ( float )atof( crParsedMsg.GetArg( 3 ));

	// Transform the position to runtime space.
	LTVector vOffset;
	g_pLTServer->GetSourceWorldOffset(vOffset);
	vPos -= vOffset;

	if( bSetPos )
		g_pLTServer->SetObjectPos( m_hObject, vPos );
	else
		g_pLTServer->Physics()->MoveObject( m_hObject, vPos, 0 );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::HandleSetRotationMsg
//
//	PURPOSE:	Handle a SETROTATION message...
//
// ----------------------------------------------------------------------- //

void GameBase::HandleSetRotationMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Make sure we have all the arguments.
	if( crParsedMsg.GetArgCount( ) < 4 )
		return;

	// Get the euler angles.  Convert to radians.
	LTVector vEulerAngle;
	vEulerAngle.y = (( float )atof( crParsedMsg.GetArg( 1 )) * MATH_PI / 180.0f );
	vEulerAngle.x = (( float )atof( crParsedMsg.GetArg( 2 )) * MATH_PI / 180.0f );
	vEulerAngle.z = (( float )atof( crParsedMsg.GetArg( 3 )) * MATH_PI / 180.0f );

	LTRotation rRot( vEulerAngle.x, vEulerAngle.y, vEulerAngle.z );
	g_pLTServer->SetObjectRotation( m_hObject, rRot );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::HandleCopyXFormMsg
//
//	PURPOSE:	Handle a COPYXFORM message...
//
// ----------------------------------------------------------------------- //

void GameBase::HandleCopyXFormMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Make sure we have all the arguments.
	if( crParsedMsg.GetArgCount( ) < 2 )
		return;

	// Get the target object.
	HOBJECT hTarget = NULL;
	FindNamedObject( crParsedMsg.GetArg( 1 ), hTarget, false );
	if( !hTarget )
		return;

	// Set the position for this object.
	LTVector vPos;
	g_pLTServer->GetObjectPos( hTarget, &vPos );
	g_pLTServer->SetObjectPos( m_hObject, vPos );

	// Set the rotation for this object.
	LTRotation rRot;
	g_pLTServer->GetObjectRotation( hTarget, &rRot );
	g_pLTServer->SetObjectRotation( m_hObject, rRot );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 GameBase::EngineMessageFn(uint32 messageID, void *pData, float fData)
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

		case MID_OBJECTCREATED:
		{
			if( fData != OBJECTCREATED_SAVEGAME )
			{
				ObjectCreated( reinterpret_cast<const GenericPropList*>(pData) );
			}
		}
		break;

		case MID_MODELSTRINGKEY:
		{
			HandleModelString( (ArgList*)pData );
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
//	ROUTINE:	GameBase::SetNextUpdate()
//
//	PURPOSE:	Allows objects to set their next update time and at
//				time same time update autodeactivation
//
// ----------------------------------------------------------------------- //

void GameBase::SetNextUpdate(float fDelta, eUpdateControl eControl)
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
	SAVE_BYTE( m_eOriginalShadowLOD );
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
	uint32 nSaveVersion;
	LOAD_DWORD(nSaveVersion);
	g_pVersionMgr->SetCurrentSaveVersion( nSaveVersion );
	LOAD_DWORD(m_dwOriginalFlags);
	LOAD_BYTE_CAST( m_eOriginalShadowLOD, EEngineLOD );
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
	if( pStruct->m_cProperties.GetBool("Template", false))
	{
		// Save the template
		g_pGameServerShell->GetObjectTemplates()->AddTemplate( pStruct );

		// Don't actually create the object
		return false;
	}
	if ( !( pStruct->m_Name[0] ))
	{
		LTSNPrintF( pStruct->m_Name, ARRAY_LEN(pStruct->m_Name), "noname%d", g_pGameServerShell->GetUniqueObjectID() );
		g_pGameServerShell->IncrementUniqueObjectID();
	}

	// After the auto save has been created dynamically created objects that should not exist in the level 
	// should just never be created...
	if( g_pGameServerShell->GetSwitchingWorldsState( ) == eSwitchingWorldsStateFinished )
	{
		uint32 nUserFlags = 0;
		if( pStruct->m_cProperties.GetBool( "IsGore", false ))
		{
			nUserFlags |= USRFLG_GORE;
		}

		EEngineLOD eObjectLOD = CEngineLODPropUtil::StringToLOD( pStruct->m_cProperties.GetString( "ObjectLOD", "Low" ), eEngineLOD_Low );
		nUserFlags |= ObjectLODToUserFlag( eObjectLOD );

		// Don't bother creating the object if the client settings won't allow it...
		if( g_pGameServerShell->ShouldRemoveBasedOnClientSettings( nUserFlags ))
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::ObjectCreated
//
//	PURPOSE:	Handle processing properties after the object has been created...
//
// ----------------------------------------------------------------------- //

void GameBase::ObjectCreated( const GenericPropList *pPropList )
{
	if( !pPropList )
		return;

	// Single player games may have objects removed based on detail and gore...
	if( !IsMultiplayerGameServer( ))
	{
		uint32 nUserFlags = 0;
		if( pPropList->GetBool( "IsGore", false ))
		{
			nUserFlags |= USRFLG_GORE;
		}

		EEngineLOD eObjectLOD = CEngineLODPropUtil::StringToLOD( pPropList->GetString( "ObjectLOD", "Low" ), eEngineLOD_Low );
		nUserFlags |= ObjectLODToUserFlag( eObjectLOD );
		
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, nUserFlags, USRFLG_GORE | USRFLG_OBJECTLODMASK );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::HandleModelString
//
//	PURPOSE:	Handle recieving a modelstring from the models animation...
//
// ----------------------------------------------------------------------- //

void GameBase::HandleModelString( ArgList* pArgList )
{
	static CParsedMsg::CToken s_cTok_KEY_COMMAND( "CMD" );

	if( !pArgList || !pArgList->argv || pArgList->argc == 0 )
		return;

	char* pKey = pArgList->argv[0];
	if( !pKey )
		return;

	CParsedMsg::CToken tok( pKey );

	if( tok == s_cTok_KEY_COMMAND )
	{
		
		char szBuffer[256] = {0};
		bool bAddParen = false;

		// Rebuild the command string and queue it up...

		LTSNPrintF( szBuffer, LTARRAYSIZE( szBuffer ), "%s", pArgList->argv[1] );
		for( int nArg = 2; nArg < pArgList->argc; ++nArg )
		{
			bAddParen = false;
			strcat(szBuffer, " ");
			if( strstr( pArgList->argv[nArg], " " ))
			{
				// Enclose the argument in parens if it's broken up into multiple arguments...
				strcat( szBuffer, "(" );
				bAddParen = true;
			}

			strcat( szBuffer, pArgList->argv[nArg] );

			if( bAddParen )
			{
				// Tailing paren if needed...
				strcat( szBuffer, ")" );
			}
		}

		g_pCmdMgr->QueueCommand( szBuffer, m_hObject, m_hObject );
	}
}
