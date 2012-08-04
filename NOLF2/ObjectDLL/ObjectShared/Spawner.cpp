// ----------------------------------------------------------------------- //
//
// MODULE  : Spawner.cpp
//
// PURPOSE : Spawner class - implementation
//
// CREATED : 12/31/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Spawner.h"
#include "iltserver.h"
#include "MsgIds.h"
#include "SoundMgr.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "ServerSoundMgr.h"
#include "ObjectTemplateMgr.h"

LINKFROM_MODULE( Spawner );

BEGIN_CLASS(Spawner)
	ADD_STRINGPROP( DefaultSpawn, "" )
	ADD_STRINGPROP( Target, "" )
	ADD_STRINGPROP( SpawnSound, "" )
	ADD_REALPROP( SoundRadius, 500.0f )
	ADD_STRINGPROP_FLAG( InitialCommand, "", PF_NOTIFYCHANGE)
END_CLASS_DEFAULT_FLAGS_PLUGIN(Spawner, GameBase, NULL, NULL, 0, CSpawnerPlugin)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpawnObject
//
//	PURPOSE:	The external function for spawning an object based on a property string
//
// ----------------------------------------------------------------------- //

BaseClass *SpawnObject(char const* pszSpawn, const LTVector& vPos, const LTRotation& rRot)
{
	if (!g_pLTServer || !pszSpawn) return LTNULL;
	
	// Make a local copy, since we change it.
	char szSpawn[2048];
	SAFE_STRCPY( szSpawn, pszSpawn );
		
	// Pull the class name out of the spawn string...
	char* pszClassName = strtok(szSpawn, " ");
	if (!pszClassName) return LTNULL;
	
	HCLASS hClass = g_pLTServer->GetClass(pszClassName);
	if (!hClass) return LTNULL;
	
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);
	theStruct.m_Pos = vPos;
	theStruct.m_Rotation = rRot;
	
	// Set the string to be the rest of the string...
	char* pszProps = strtok(NULL, "");
	
   	// Allocate an object...
	return (BaseClass *)g_pLTServer->CreateObjectProps(hClass, &theStruct, pszProps);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpawnObjectTemplate
//
//	PURPOSE:	Spawns an object based on an object template
//
// ----------------------------------------------------------------------- //

BaseClass *SpawnObjectTemplate(const char *pszSpawn, const char *pName, const LTVector& vPos, const LTRotation& rRot)
{
    if (!g_pLTServer || !pszSpawn) return LTNULL;

	// Pull the class name out of the spawn string...
	const ObjectCreateStruct *pOCS = g_pGameServerShell->GetObjectTemplates()->FindTemplate(pszSpawn);
	if (!pOCS) return LTNULL;

	ObjectCreateStruct theStruct = *pOCS;

    HCLASS hClass = theStruct.m_hClass;
    if (!hClass) return LTNULL;

	theStruct.m_Pos = vPos;
	theStruct.m_Rotation = rRot;

	// Change the name of the object, we're assuming that these already exist and more room
	// doesn't need to be allocated for them
	SAFE_STRCPY(theStruct.m_Name, pName);
	theStruct.m_cProperties.AddProp("Name", GenericProp(pName, LT_PT_STRING));

	// Change the position and rotation
	theStruct.m_cProperties.AddProp("Pos", GenericProp(vPos, LT_PT_VECTOR));
	theStruct.m_cProperties.AddProp("Rotation", GenericProp(rRot, LT_PT_ROTATION));

	// Allocate an object...
	// Note : This has to use the CreateObjectProps function for purposes of backwards
	// compatibility.  Most of the game code assumes that if it's getting a PRECREATE_NORMAL
	// message that it doesn't have any properties available.
    return (BaseClass *)g_pLTServer->CreateObjectProps(hClass, &theStruct, "");
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateMsgDefaultFrom
//
//  PURPOSE:	Validates the message for DEFAULTFROM
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateMsgDefaultFrom( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( LT_NOTFOUND == pInterface->FindObject( cpMsgParams.m_Args[1] ) )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateMsgDefaultFrom()" );
			pInterface->CPrint( "    MSG - DEFAULTFROM - Could not find object '%s'!", cpMsgParams.m_Args[1] );
		}
		
		return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateMsgSpawn
//
//  PURPOSE:	Validates the message for SPAWN
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateMsgSpawn( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	// If a name was specified try to add the object to the dynamic object list...
	
	if( cpMsgParams.m_nArgs == 2 )
	{
		if( pInterface->FindObject( cpMsgParams.m_Args[1] ) == LT_OK )
		{
			// <sigh> Trying to spawn in an object with a name of an object that is already in the level...

			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - ValidateMsgSpawn()" );
				pInterface->CPrint( "    MSG - SPAWN - Object '%s' already exists and is of type '%s'!", cpMsgParams.m_Args[1], pInterface->GetObjectClass( cpMsgParams.m_Args[1] ));
			}
			
			return LTFALSE;	
		}

		CCommandMgrPlugin::DYNAMIC_OBJECT obj;
		obj.m_sName = cpMsgParams.m_Args[1];


		char *pObj = CCommandMgrPlugin::GetCurrentObjectName();

		// Find the target template object and get it's class name...

		GenericProp gProp;
		if( pInterface->GetPropGeneric( pObj, "Target", &gProp ) == LT_OK )
		{
			if( gProp.m_String[0] )
			{
				const char *pClassName = pInterface->GetObjectClass( gProp.m_String );
				if( pClassName )
				{
					obj.m_sClassName = pClassName;
				}
				else
				{
					if( CCommandMgrPlugin::s_bShowMsgErrors )
					{
						pInterface->ShowDebugWindow( LTTRUE );
						pInterface->CPrint( "ERROR! - ValidateMsgSpawn()" );
						pInterface->CPrint( "    MSG - SPAWN - Target object '%s' does not exist!", gProp.m_String );
					}
					
					return LTFALSE;	
				}

				if( !obj.m_sClassName.empty() && !obj.m_sName.empty() )
				{
					CCommandMgrPlugin::AddDynamicObject( obj );
				}
			}
		}	
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateMsgSpawnFrom
//
//  PURPOSE:	Validates the message for SPAWNFROM
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateMsgSpawnFrom( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( LT_NOTFOUND == pInterface->FindObject( cpMsgParams.m_Args[1] ) )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateMsgSpawnFrom()" );
			pInterface->CPrint( "    MSG - SPAWNFROM - Could not find object '%s'!", cpMsgParams.m_Args[1] );
		}
		
		return LTFALSE;
	}

	// If a name was specified try to add the object to the dynamic object list...
	
	if( cpMsgParams.m_nArgs == 3 )
	{
		if( pInterface->FindObject( cpMsgParams.m_Args[2] ) == LT_OK )
		{
			// <sigh> Trying to spawn in an object with a name of an object that is already in the level...

			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - ValidateMsgSpawnFrom()" );
				pInterface->CPrint( "    MSG - SPAWNFROM - Object '%s' already exists and is of type '%s'!", cpMsgParams.m_Args[2], pInterface->GetObjectClass( cpMsgParams.m_Args[2] ));
			}
			
			return LTFALSE;		
		}

		CCommandMgrPlugin::DYNAMIC_OBJECT obj;
		obj.m_sName = cpMsgParams.m_Args[2];

		char *pObj = CCommandMgrPlugin::GetCurrentObjectName();

		// Find the target template object and get it's class name...

		GenericProp gProp;
		if( pInterface->GetPropGeneric( pObj, "Target", &gProp ) == LT_OK )
		{
			if( gProp.m_String[0] )
			{
				obj.m_sClassName = pInterface->GetObjectClass( gProp.m_String );

				if( !obj.m_sClassName.empty() && !obj.m_sName.empty() )
				{
					CCommandMgrPlugin::AddDynamicObject( obj );
				}
			}
		}	
	}

	return LTTRUE;
}

CMDMGR_BEGIN_REGISTER_CLASS( Spawner )

	CMDMGR_ADD_MSG( DEFAULT, 1, NULL, "DEFAULT" )
	CMDMGR_ADD_MSG( DEFAULTFROM, 2, ValidateMsgDefaultFrom, "DEFAULTFROM <object name>")
	CMDMGR_ADD_MSG_ARG_RANGE( SPAWN, 1, 2, ValidateMsgSpawn, "SPAWN" )
	CMDMGR_ADD_MSG_ARG_RANGE( SPAWNFROM, 2, 3, ValidateMsgSpawnFrom, "SPAWNFROM <object name>")

CMDMGR_END_REGISTER_CLASS( Spawner, BaseClass )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::Spawner
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

Spawner::Spawner() : GameBase(OT_NORMAL)
{
	m_fSoundRadius = 500.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::~Spawner
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Spawner::~Spawner()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Spawner::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);

			break;
		}
		case MID_INITIALUPDATE:
		{
			InitialUpdate();
			break;
		}

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

		default : break;
	}


	return GameBase::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::InitialUpdate
//
//	PURPOSE:	Do initial updating
//
// ----------------------------------------------------------------------- //

LTBOOL Spawner::InitialUpdate()
{
	// Don't eat ticks please...
    SetNextUpdate(UPDATE_NEVER);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::ReadProp
//
//	PURPOSE:	Reads properties from level info
//
// ----------------------------------------------------------------------- //

LTBOOL Spawner::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

    if ( g_pLTServer->GetPropGeneric( "DefaultSpawn", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_sDefaultSpawn = genProp.m_String;

    if ( g_pLTServer->GetPropGeneric( "Target", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_sTarget = genProp.m_String;

    if ( g_pLTServer->GetPropGeneric( "SpawnSound", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_sSpawnSound = genProp.m_String;

    if ( g_pLTServer->GetPropGeneric( "InitialCommand", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_sInitialCommand = genProp.m_String;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::PostPropRead
//
//	PURPOSE:	Modifies properties after reading from level info.
//
// ----------------------------------------------------------------------- //

LTBOOL Spawner::PostPropRead( ObjectCreateStruct *pStruct )
{
    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool Spawner::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Spawn("spawn");
	static CParsedMsg::CToken s_cTok_SpawnFrom("spawnfrom");
	static CParsedMsg::CToken s_cTok_Default("default");
	static CParsedMsg::CToken s_cTok_DefaultFrom("defaultfrom");

	char		szSpawn[1024] = "";
	LTVector	vPos;
    LTRotation	rRot;
	const char *pName = "";

	g_pLTServer->GetObjectPos( m_hObject, &vPos );
	g_pLTServer->GetObjectRotation( m_hObject, &rRot );

	bool bSpawnTemplate = false;

	if( cMsg.GetArg(0) == s_cTok_Spawn )
	{
		// Spawn the target object from this spawners position...
	    
		SAFE_STRCPY(szSpawn, m_sTarget.c_str( ));

		// Rename the object if desired
		if( cMsg.GetArgCount() > 1 )
		{
			pName = cMsg.GetArg(1);
		}

		bSpawnTemplate = true;
	}
	else if( cMsg.GetArg(0) == s_cTok_SpawnFrom )
	{
		// Spawn the target object from the position of the specified object...
	
		SAFE_STRCPY(szSpawn, m_sTarget.c_str( ));

		ObjArray<HOBJECT, 1> objArray;
		g_pLTServer->FindNamedObjects( const_cast<char*>(cMsg.GetArg(1).c_str()), objArray );

		if( objArray.NumObjects() == 1 )
		{
			g_pLTServer->GetObjectPos( objArray.GetObject(0), &vPos );
			g_pLTServer->GetObjectRotation( objArray.GetObject(0), &rRot );
		}

		// Rename the object if desired
		if( cMsg.GetArgCount() > 2 )
		{
			pName = cMsg.GetArg(2);
		}

		bSpawnTemplate = true;
	}
	else if( cMsg.GetArg(0) == s_cTok_Default )
	{
		// Spawn the default object from this spawners position...
		
		if( m_sDefaultSpawn.length( ))
		{
			SAFE_STRCPY(szSpawn, m_sDefaultSpawn.c_str( ));
		}
	}
	else if( cMsg.GetArg(0) == s_cTok_DefaultFrom )
	{
		// Spawn the default object from the position of the specified object...

		if( m_sDefaultSpawn.length( ))
		{
			SAFE_STRCPY(szSpawn, m_sDefaultSpawn.c_str( ));
			
			ObjArray<HOBJECT, 1> objArray;
			g_pLTServer->FindNamedObjects( const_cast<char*>(cMsg.GetArg(1).c_str()), objArray );

			if( objArray.NumObjects() == 1 )
			{
				g_pLTServer->GetObjectPos( objArray.GetObject(0), &vPos );
				g_pLTServer->GetObjectRotation( objArray.GetObject(0), &rRot );
			}
		}
	}
	else
	{
  		// Spawn an object from this spawners pos using the properties specified in the message...
  
  		cMsg.ReCreateMsg(szSpawn, sizeof(szSpawn), 0);
	}


	// Play spawn sound...
	if ( m_sSpawnSound.length( ))
	{
        g_pServerSoundMgr->PlaySoundFromPos(vPos, m_sSpawnSound.c_str( ), m_fSoundRadius, SOUNDPRIORITY_MISC_LOW);
    }

	BaseClass* pBaseClass = NULL;
	if (bSpawnTemplate)
		pBaseClass = SpawnObjectTemplate( szSpawn, pName, vPos, rRot );
	else
		pBaseClass = SpawnObject( szSpawn, vPos, rRot );

	// Check if we succeeded in creating someone.
	if( pBaseClass )
	{
		// Send the initial command if specified.
		if( m_sInitialCommand.length( ))
		{
			char const* pszCmd = m_sInitialCommand.c_str( );
			if( g_pCmdMgr->IsValidCmd( pszCmd ))
			{
				g_pCmdMgr->Process( pszCmd, m_hObject, pBaseClass->m_hObject );
			}
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Spawner::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_FLOAT(m_fSoundRadius);
	SAVE_CHARSTRING(m_sSpawnSound.c_str( ));
	SAVE_CHARSTRING(m_sDefaultSpawn.c_str( ));
	SAVE_CHARSTRING(m_sTarget.c_str( ));
	SAVE_CHARSTRING(m_sInitialCommand.c_str( ));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Spawner::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	char szString[1024];
	LOAD_FLOAT(m_fSoundRadius);
	LOAD_CHARSTRING(szString, ARRAY_LEN(szString));
	m_sSpawnSound = szString;
	LOAD_CHARSTRING(szString, ARRAY_LEN(szString));
	m_sDefaultSpawn = szString;
	LOAD_CHARSTRING(szString, ARRAY_LEN(szString));
	m_sTarget = szString;
	LOAD_CHARSTRING(szString, ARRAY_LEN(szString));
	m_sInitialCommand = szString;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpawnerPlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //

LTRESULT CSpawnerPlugin::PreHook_PropChanged( const char *szObjName,
											  const char *szPropName, 
											  const int  nPropType, 
											  const GenericProp &gpPropValue,
											  ILTPreInterface *pInterface,
											  const char *szModifiers )
{
	// Check if the props are our commands and then just send it to the CommandMgr..

	if( !_stricmp( "InitialCommand", szPropName ))
	{
		if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
													szPropName, 
													nPropType, 
													gpPropValue,
													pInterface,
													szModifiers ) == LT_OK )
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}