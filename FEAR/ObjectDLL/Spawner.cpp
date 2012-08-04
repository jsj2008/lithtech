// ----------------------------------------------------------------------- //
//
// MODULE  : Spawner.cpp
//
// PURPOSE : Spawner class - implementation
//
// CREATED : 12/31/97
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "Spawner.h"
#include "ServerSoundMgr.h"
#include "ObjectTemplateMgr.h"

LINKFROM_MODULE( Spawner );

BEGIN_CLASS(Spawner)
	ADD_STRINGPROP_FLAG( Target, "", PF_OBJECTLINK, "This field defines what object template is to be spawned in when the Spawner receives a 'spawn' command." )
	ADD_STRINGPROP( SpawnSound, "", "The path to any .wav file may be entered here. This sound file will be played when the Spawner object is triggered." )
	ADD_REALPROP( SoundRadius, 500.0f, "This is the radius of the sound falloff measured in WorldEdit units." )
	ADD_COMMANDPROP_FLAG( InitialCommand, "", PF_NOTIFYCHANGE, "Command run after object is spawned.  The default target of the command is the new object.")
END_CLASS_FLAGS_PLUGIN(Spawner, GameBase, 0, CSpawnerPlugin, "This object allows the creation of other objects.  A default spawn can be performed using the properties listed in the DefaultSpawn string or an object can be spawned using a template object specified in the Target property.")


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpawnObject
//
//	PURPOSE:	The external function for spawning an object based on a property string
//
// ----------------------------------------------------------------------- //

BaseClass *SpawnObject(char const* pszSpawn, const LTVector& vPos, const LTRotation& rRot)
{
	if (!g_pLTServer || !pszSpawn) return NULL;
	
	// Make a local copy, since we change it.
	char szSpawn[2048];
	LTStrCpy( szSpawn, pszSpawn, LTARRAYSIZE(szSpawn) );
		
	// Pull the class name out of the spawn string...
	char* pszClassName = strtok(szSpawn, " ");
	if (!pszClassName) return NULL;
	
	HCLASS hClass = g_pLTServer->GetClass(pszClassName);
	if (!hClass) return NULL;
	
	ObjectCreateStruct theStruct;
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
    if (!g_pLTServer || !pszSpawn) return NULL;

	// Pull the class name out of the spawn string...
	const ObjectCreateStruct *pOCS = g_pGameServerShell->GetObjectTemplates()->FindTemplate(pszSpawn);
	if (!pOCS) return NULL;

	ObjectCreateStruct theStruct = *pOCS;

    HCLASS hClass = theStruct.m_hClass;
    if (!hClass) return NULL;

	theStruct.m_Pos = vPos;
	theStruct.m_Rotation = rRot;

	// Change the name of the object, we're assuming that these already exist and more room
	// doesn't need to be allocated for them
	theStruct.SetName(pName);
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
//  ROUTINE:	bool ValidateTargetProperty
//
//  PURPOSE:	Perform general validation of the TARGET property, common 
//				to all uses.
//
// ----------------------------------------------------------------------- //

static bool ValidateTargetProperty( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	char *pObj = CCommandMgrPlugin::GetCurrentObjectName();
	const char* pszCommand = cpMsgParams.m_Args[0];

	// Failed to find a target property.  This shouldn't happen unless 
	// we change the property set for this object.

	GenericProp gTargetNameProp;
	if( pInterface->GetPropGeneric( pObj, "Target", &gTargetNameProp ) != LT_OK )
	{
		WORLDEDIT_ERROR_MSG( pInterface, cpMsgParams, "No 'Target' property." );
		return false;
	}
	
	const char* pszTargetObjectName = gTargetNameProp.GetString();

	// Fail if the target property string is empty.

	if ( LTStrEmpty( pszTargetObjectName ) )
	{
		WORLDEDIT_ERROR_MSG( pInterface, cpMsgParams, "No object specified in the Target property" );
		return false;
	}

	// Fail if the class does not exist.

	const char *pClassName = pInterface->GetObjectClass( pszTargetObjectName );
	if ( LTStrEmpty( pClassName ) )
	{
		WORLDEDIT_ERROR_MSG1( pInterface, cpMsgParams, "Target object '%s' does not exist!", pszTargetObjectName );
		return false;
	}

	// Fail if the target object does not have a template property.

	GenericProp gTargetTemplateProp;
	if( pInterface->GetPropGeneric( pszTargetObjectName, "Template", &gTargetTemplateProp ) != LT_OK )
	{
		WORLDEDIT_ERROR_MSG1( pInterface, cpMsgParams, "Target object '%s' is not a template object!", pszTargetObjectName  );
	}

	// Fail if the target object is not a template.

	if ( LT_PT_BOOL != gTargetTemplateProp.GetType() 
		|| false == gTargetTemplateProp.GetBool() )
	{
		WORLDEDIT_ERROR_MSG1( pInterface, cpMsgParams, "Target object '%s' is not a template object!", pszTargetObjectName );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateMsgSpawn
//
//  PURPOSE:	Validates the message for SPAWN
//
// ----------------------------------------------------------------------- //

static bool ValidateMsgSpawn( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	// Insure the Target property is valid before doing anything else.

	if ( !ValidateTargetProperty( pInterface, cpMsgParams ) )
	{
		return false;
	}

	// If a name was specified try to add the object to the dynamic object list...
	
	if( cpMsgParams.m_nArgs == 2 )
	{
		if( CCommandMgrPlugin::DoesObjectExist( pInterface, cpMsgParams.m_Args[1] ))
		{
			// <sigh> Trying to spawn in an object with a name of an object that is already in the level...

			WORLDEDIT_ERROR_MSG2( pInterface, cpMsgParams, "Object '%s' already exists and is of type '%s'!", cpMsgParams.m_Args[1], pInterface->GetObjectClass( cpMsgParams.m_Args[1] ) );
			return false;	
		}

		CCommandMgrPlugin::DYNAMIC_OBJECT obj;
		obj.m_sName = cpMsgParams.m_Args[1];


		char *pObj = CCommandMgrPlugin::GetCurrentObjectName();

		// Find the target template object and get it's class name...

		GenericProp gProp;
		if( pInterface->GetPropGeneric( pObj, "Target", &gProp ) == LT_OK )
		{
			obj.m_sClassName = pInterface->GetObjectClass( gProp.GetString() );
			if( !obj.m_sClassName.empty() && !obj.m_sName.empty() )
			{
				CCommandMgrPlugin::AddDynamicObject( obj );
			}
		}	
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateMsgSpawnFrom
//
//  PURPOSE:	Validates the message for SPAWNFROM
//
// ----------------------------------------------------------------------- //

static bool ValidateMsgSpawnFrom( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	// Insure the Target property is valid before doing anything else.

	if ( !ValidateTargetProperty( pInterface, cpMsgParams ) )
	{
		return false;
	}

	if( !CCommandMgrPlugin::DoesObjectExist( pInterface, cpMsgParams.m_Args[1] ))
	{
		WORLDEDIT_ERROR_MSG1( pInterface, cpMsgParams, "Could not find object '%s'!", cpMsgParams.m_Args[1] );
		return false;
	}

	// If a name was specified try to add the object to the dynamic object list...
	
	if( cpMsgParams.m_nArgs == 3 )
	{
		if( CCommandMgrPlugin::DoesObjectExist( pInterface, cpMsgParams.m_Args[2] ))
		{
			// <sigh> Trying to spawn in an object with a name of an object that is already in the level...
			WORLDEDIT_ERROR_MSG2( pInterface, cpMsgParams, "Object '%s' already exists and is of type '%s'!", cpMsgParams.m_Args[2], pInterface->GetObjectClass( cpMsgParams.m_Args[2] ) );
			return false;
		}

		CCommandMgrPlugin::DYNAMIC_OBJECT obj;
		obj.m_sName = cpMsgParams.m_Args[2];

		char *pObj = CCommandMgrPlugin::GetCurrentObjectName();

		// Find the target template object and get it's class name...

		GenericProp gProp;
		if( pInterface->GetPropGeneric( pObj, "Target", &gProp ) == LT_OK )
		{
			obj.m_sClassName = pInterface->GetObjectClass( gProp.GetString() );
			if( !obj.m_sClassName.empty() && !obj.m_sName.empty() )
			{
				CCommandMgrPlugin::AddDynamicObject( obj );
			}
		}	
	}

	return true;
}

CMDMGR_BEGIN_REGISTER_CLASS( Spawner )

	ADD_MESSAGE_ARG_RANGE( SPAWN, 1, 2, ValidateMsgSpawn, MSG_HANDLER( Spawner, HandleSpawnMsg ), "SPAWN", "Spawns the default or target object", "msg Spawner SPAWN" )
	ADD_MESSAGE_ARG_RANGE( SPAWNFROM, 2, 3, ValidateMsgSpawnFrom, MSG_HANDLER( Spawner, HandleSpawnFromMsg ), "SPAWNFROM <object name>", "Spawns the default or target object from a named object in the world", "To use a Spawner named \"Spawner\" to spawn a target object from an object named AI-01 the command would look like:<BR><BR>msg Spawner (SPAWNFROM AI-01)" )

CMDMGR_END_REGISTER_CLASS( Spawner, GameBase )


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

uint32 Spawner::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp(&((ObjectCreateStruct*)pData)->m_cProperties);
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

bool Spawner::InitialUpdate()
{
	// Don't eat ticks please...
    SetNextUpdate(UPDATE_NEVER);

    return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::ReadProp
//
//	PURPOSE:	Reads properties from level info
//
// ----------------------------------------------------------------------- //

bool Spawner::ReadProp(const GenericPropList *pProps)
{
	m_sTarget			= pProps->GetString( "Target", "" );
	m_sSpawnSound		= pProps->GetString( "SpawnSound", "" );
	m_sInitialCommand	= pProps->GetCommand( "InitialCommand", "" );

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::PostPropRead
//
//	PURPOSE:	Modifies properties after reading from level info.
//
// ----------------------------------------------------------------------- //

bool Spawner::PostPropRead( ObjectCreateStruct *pStruct )
{
    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::HandleSpawnMsg
//
//	PURPOSE:	Handle a SPAWN message...
//
// ----------------------------------------------------------------------- //

void Spawner::HandleSpawnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	LTVector	vPos;
	LTRotation	rRot;
	const char *pName = "";

	// Spawn the target object from this spawners position...

	g_pLTServer->GetObjectPos( m_hObject, &vPos );
	g_pLTServer->GetObjectRotation( m_hObject, &rRot );

	// Rename the object if desired
	if( crParsedMsg.GetArgCount() > 1 )
	{
		pName = crParsedMsg.GetArg(1);
	}

	// Spawn using a target template...
	Spawn( m_sTarget.c_str(), pName, vPos, rRot );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::HandleSpawnFromMsg
//
//	PURPOSE:	Handle a SPAWNFROM message...
//
// ----------------------------------------------------------------------- //

void Spawner::HandleSpawnFromMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	LTVector	vPos;
	LTRotation	rRot;
	const char *pName = "";

	ObjArray<HOBJECT, 1> objArray;
	g_pLTServer->FindNamedObjects( const_cast<char*>(crParsedMsg.GetArg(1).c_str()), objArray );

	// Spawn the target object from the position of the specified object...
	if( objArray.NumObjects() == 1 )
	{
		g_pLTServer->GetObjectPos( objArray.GetObject(0), &vPos );
		g_pLTServer->GetObjectRotation( objArray.GetObject(0), &rRot );
	}
	else
	{
		g_pLTServer->GetObjectPos( m_hObject, &vPos );
		g_pLTServer->GetObjectRotation( m_hObject, &rRot );
	}

	// Rename the object if desired
	if( crParsedMsg.GetArgCount() > 2 )
	{
		pName = crParsedMsg.GetArg(2);
	}

	// Spawn using a target template...
	Spawn( m_sTarget.c_str(), pName, vPos, rRot );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::Spawn
//
//	PURPOSE:	Spawn an object...
//
// ----------------------------------------------------------------------- //

BaseClass* Spawner::Spawn( const char *pszSpawnString, const char *pszObjName, const LTVector &vPos, const LTRotation &rRot )
{
	if( !pszSpawnString || !pszSpawnString[0] )
		return NULL;

	// Play spawn sound...
	if ( m_sSpawnSound.length( ))
	{
		g_pServerSoundMgr->PlaySoundFromPos(vPos, m_sSpawnSound.c_str( ), NULL, m_fSoundRadius, SOUNDPRIORITY_MISC_LOW,
			PLAYSOUND_USEOCCLUSION, SMGR_DEFAULT_VOLUME, 1.0f, -1.0f,
			DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS);
	}

	BaseClass* pBaseClass = SpawnObjectTemplate( pszSpawnString, pszObjName, vPos, rRot );

	// Check if we succeeded in creating someone.
	if( pBaseClass )
	{
		// Send the initial command if specified.
		if( !m_sInitialCommand.empty( ))
		{
			g_pCmdMgr->QueueCommand( m_sInitialCommand.c_str( ), m_hObject, pBaseClass->m_hObject );
		}

		return pBaseClass;
	}

	return NULL;
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
	SAVE_STDSTRING( m_sSpawnSound );
	SAVE_STDSTRING( m_sTarget );
	SAVE_STDSTRING( m_sInitialCommand );
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

	LOAD_FLOAT(m_fSoundRadius);
	
	LOAD_STDSTRING( m_sSpawnSound );
	LOAD_STDSTRING( m_sTarget );
	LOAD_STDSTRING( m_sInitialCommand );
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

	if( LTStrIEquals( "InitialCommand", szPropName ))
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
