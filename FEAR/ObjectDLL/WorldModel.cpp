// ----------------------------------------------------------------------- //
//
// MODULE  : WorldModel.cpp
//
// PURPOSE : WorlModel implementation
//
// CREATED : 5/9/01
//
// (c) 2001-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "Stdafx.h"
	#include "ObjectMsgs.h"
	#include "ParsedMsg.h"
	#include "WorldModel.h"
	#include "Character.h"
	#include "ClientWeaponSFX.h"
	#include "Attachments.h"
	#include "EngineLODPropUtil.h"
	#include "PrefetchUtilities.h"
	#include "ServerSaveLoadMgr.h"

LINKFROM_MODULE( WorldModel );

//
// Globals...
//

	const char* c_aPhysicsShapes[] =
	{
		"Mesh",
		"Hull",
		"OBB",
		"Sphere",
		"None"
	};

// function for gathering prefetch resources
static void CWorldModelPrefetch(const char* pszObjectName, IObjectResourceGatherer* pInterface);

//
// Add Props...
//

BEGIN_CLASS( WorldModel )

	ADD_DESTRUCTIBLE_WORLD_MODEL_AGGREGATE( PF_GROUP(1), 0 )

	// Override default values...

	ADD_BOOLPROP_FLAG( NeverDestroy, true, PF_GROUP(1), "Toggles whether the object can be destroyed.")

	// Add Object flag props...
	
	ADD_VISIBLE_FLAG( true, 0 )
	ADD_SOLID_FLAG( true, 0 )
	ADD_RAYHIT_FLAG( true, 0 )
	ADD_GRAVITY_FLAG( false, 0 )

	ADD_BOOLPROP_FLAG( Translucent, false, 0, "Indicates whether or not this world model is translucent.")
	ADD_BOOLPROP_FLAG( TranslucentLight, true, 0, "Indicates whether or not this world model should use lighting when it is translucent or if it should just be treated as full bright.")

	ADD_BOOLPROP_FLAG( CastShadow, true, 0, "If true the WorldModel will initially cast a shadow at the LOD specified with the ShadowLOD property.  If false the WorldModel will not initially cast a shadow but will save the value of the ShadowLOD property for later use with the CASTSHADOW message." )
	ADD_STRINGPROP_FLAG( ShadowLOD, "Low", PF_STATICLIST, "This value indicates at which detail levels the shadow for this world model will be visible. For example at Low, this will always show up, at Medium it will only show up in medium and high, and Never will cause the shadow to never be shown.")
	ADD_BOOLPROP_FLAG( BoxPhysics, false, 0, "In the engine, WorldModels have two possible physics models. In the first, the player can walk on and touch every curve and corner of their surface. In the second, the player can only interact with a bounding box that surrounds all the brushes in the WorldModel, just like the box you would get if you selected them in WorldEdit. For geometry with a simple rectangular shape, this is preferred because it's cheaper to calculate. However, for a lot of objects, it's limiting. If you need a player to be able to shoot through the bars in a prison door, you will need BoxPhysics set to FALSE.")
	ADD_BOOLPROP_FLAG( IsKeyframed, false, 0, "Is the WorldModel moved by a keyframer? If it is you must set this flag to true...or life will end as we know it.")
	ADD_BOOLPROP_FLAG( RigidBody, false, 0, "Should this world model be simulated with rigid body physics?")
	ADD_BOOLPROP_FLAG( MPClientOnlyRigidBody, true, 0, "In multiplayer games, should this world model be simulated with rigid body physics only on the client?  This flag is ignored if RigidBody is false." )
	ADD_BOOLPROP_FLAG( StartHidden, false, 0, "If this flag is true the WorldModel will be created as if it were sent a HIDDEN 1 message.")

	ADD_REALPROP_FLAG( Alpha, 1.0f, 0, "Specifies the alpha of the WorldModel.") // DO NOT REMOVE THIS!!!!  Pre-Processor looks at this value.

	//Physics properties
	ADD_STRINGPROP_FLAG( PhysicsShape, "None", PF_STATICLIST, "Indicates what shape will be used to represent this world model in the physics simulation")
	ADD_REALPROP(MassKg, 1.0f, "Indicates the mass of this world model in kilograms")
	ADD_REALPROP(DensityG, 1.1f, "Indicates the density of this object measured in grams per cubic centimeter, with 1.0 being the density of normal water.")
  
	// Add attachment props...

	ADD_STRINGPROP_FLAG( Attachments, "", 0, "Using this field you can list Objects, Models, and WorldModels that will move with the WorldModel object while maintaining their relative position and rotation. This is commonly used to add Props like door knobs to doors. List all Objects you want attached seperated by a semicolon.  Ex. Prop1; RotatingWorldModel2; Prop2; WorldModel1")	// List of objects to attach
	ADD_BOOLPROP_FLAG( RemoveAttachments, true, 0, "If this property is set to true, attachments will be removed (i.e., disappear) when the WorldModel is removed (destroyed).  If this property is set to false, the attachments will receive 'destroy' messages and will be destroyed (if they are set up to respond to the destroy message).")
	ADD_VECTORPROP_VAL_FLAG( AttachDir, 0.0f, 200.0f, 0.0f, 0, "This is the direction (and magnitude) away from the center of the WorldModel a ray is cast that looks for an object to attach to the WorldModel (when the WorldModel receives an Attach message).")

	// Add a disturbance group...

	PROP_DEFINEGROUP(Disturbances, PF_GROUP(6), "Properties related to AI disturbances.")
		ADD_LONGINTPROP_FLAG(DestroyAlarmLevel, 0, PF_GROUP(6), "How alarming it is to the AI when this object is detroyed. Corresponds to thresholds in AIBrains in aibutes.txt.")
		ADD_REALPROP_FLAG(StimulusRadius, 0.0f, PF_GROUP(6), "How far away this object is noticeable to AI when disturbed.")

	ADD_BOOLPROP_FLAG( InheritActivationData, true, 0, "If True this WorldModel will inherit all activation data from the WorldModel it is attached to.  If this WorldModel is not attached to another WorldModel then this property has no effect.  If this WorldModel has an ActivationType specified and InheritActivationData is True the ActivationType will be ignored and the ActivationType of the WorldModel this one is attached to will be used instead." )
	ADD_BOOLPROP_FLAG( PlayerInteract, true, 0, "If true the WorldModel will interact with player.  Otherwise it will be non-solid to player." )

	// Add prefetching props...
	ADD_PREFETCH_RESOURCE_PROPS()

END_CLASS_FLAGS_PLUGIN_PREFETCH( WorldModel, GameBase, CF_WORLDMODEL, CWorldModelPlugin, PrefetchWorldModel, "WorldModels are used to specify dynamic geomotry in the world that may be keyframed, have physics properties, be destroyed or interact in other ways with the player or other objects in the world." )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateMsgAttachDetach
//
//  PURPOSE:	Validation message for both ATTACH and DETACH messages
//
// ----------------------------------------------------------------------- //

static bool ValidateMsgAttachDetach( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	char szMsgName[16] = {0};

	LTStrCpy( szMsgName, cpMsgParams.m_Args[0], LTARRAYSIZE(szMsgName));
	
	if( cpMsgParams.m_nArgs == 1 )
	{
		return true;
	}

	int	i = 1;
	char *pObjName = cpMsgParams.m_Args[1];

	while( pObjName )
	{
		if( !CCommandMgrPlugin::DoesObjectExist( pInterface, pObjName ))
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - ValidateMsgAttach()" );
				pInterface->CPrint( "    MSG - %s - Could not find object '%s'!", LTStrUpr(szMsgName), pObjName );
			}
			
			return false;
		}

		pObjName = (cpMsgParams.m_nArgs > ++i && cpMsgParams.m_Args[i]) ? cpMsgParams.m_Args[i] : NULL;
	}

	return true;
}

static bool ValidateMsgShadowLOD(ILTPreInterface *pInterface, ConParse &cpMsgParams)
{
	char szMsgName[16] = {0};

	LTStrCpy( szMsgName, cpMsgParams.m_Args[0], LTARRAYSIZE(szMsgName));

	if (cpMsgParams.m_nArgs != 2)
	{
		if ( CCommandMgrPlugin::s_bShowMsgErrors)
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateMsgShadowLOD()" );
			pInterface->CPrint( "    MSG - %s - Does not have an LOD specified", LTStrUpr(szMsgName) );
		}
		return false;
	}

	if ( eEngineLOD_NumLODTypes == CEngineLODPropUtil::StringToLOD(cpMsgParams.m_Args[1], eEngineLOD_NumLODTypes) )
	{
		if ( CCommandMgrPlugin::s_bShowMsgErrors)
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateMsgShadowLOD()" );
			pInterface->CPrint( "    MSG - %s - Invalid LOD specified.", LTStrUpr(szMsgName) );
		}

		return false;
	}

	return true;
}

static bool ValidateMsgRigidBody( ILTPreInterface *pInterface, ConParse &cpMsgParams )
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
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - ValidateMsgRigidBody()" );
		pInterface->CPrint( "    MSG - %s - 2nd argument '%s' is not a valid bool value.", LTStrUpr(cpMsgParams.m_Args[0]), cpMsgParams.m_Args[1] );
	}

	return false;
}

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( WorldModel )
	
  	ADD_MESSAGE( ATTACH, -1, ValidateMsgAttachDetach, MSG_HANDLER( WorldModel, HandleAttachMsg ), "ATTACH <object name(s)>", "Attaches the object specified in the message<BR><BR>If no object is specified it attaches the first object found in the direction of the AttachDir vector or within the dimensions of the world model if AttachDir is <0,0,0>", "To attach a WorldModel named BOX to a WorldModel named BALL, the command would look like:<BR><BR>msg Ball (attach box)" )
  	ADD_MESSAGE( DETACH, -1, ValidateMsgAttachDetach, MSG_HANDLER( WorldModel, HandleDetachMsg ), "DETACH <object name(s)>", "Detach a specific object attached with the ATTACH message.", "To detach a WorldModel named BOX from a WorldModel named BALL, the command would look like:<BR><BR>msg BALL (DETACH BOX)" )
  	ADD_MESSAGE( ATTACHCLASS, 2, NULL, MSG_HANDLER( WorldModel, HandleAttachClassMsg ), "ATTACHCLASS <class name>", "This will attach the first object of the specified class found in the direction of AttachDir or within the dimensions of the world model if AttachDir is <0,0,0>", "To attach any AI to the WorldModel the command would look like:<BR><BR>msg WorldModel (ATTACHCLASS CAI)" )
  	ADD_MESSAGE( DESTROY, 1, NULL, MSG_HANDLER( WorldModel, HandleDestroyMsg ), "DESTROY", "This command destroys an object in the game as if it had been destroyed trough the loss of all of its hit points. It will play any effects and sounds set up to play at the time of its destruction", "msg WorldModel DESTROY" )
	ADD_MESSAGE( NEVERDESTROY, 2, NULL,	MSG_HANDLER( WorldModel, EmptyHandler ), "NEVERDESTROY <1 or 0>", "This message allows you to set the NeverDestroy flag through commands.  This message does not actually destroy the object but allows destruction if NeverDestroy was set to true initially.", "msg WorldModel (NEVERDESTROY 1)" )
  	ADD_MESSAGE( ACTIVATE, 1, NULL, MSG_HANDLER( WorldModel, HandleActivateMsg ), "ACTIVATE", "This is an internal message used by game code to specify the WorldModel should activate.  This message should not be sent through a commnad specified in the editor.", "DO NOT USE" )
	ADD_MESSAGE( RIGIDBODY, 2, ValidateMsgRigidBody, MSG_HANDLER( WorldModel, HandleRigidBodyMsg ), "RIGIDBODY <bool>", "This command will set the RigidBody flag on the object making it physically simulated (1) or not (0)", "msg WorldModel RIGIDBODY 1" )
	ADD_MESSAGE( SCALEALPHA, 2, NULL, MSG_HANDLER( WorldModel, HandleScaleAlphaMsg ), "SCALEALPHA <0..1>", "This command will scale the alpha of this world model by the specified value as long as it is flagged as being translucent", "msg WorldModel ScaleAlpha 0.7" )
	
CMDMGR_END_REGISTER_CLASS( WorldModel, GameBase )


//
// Plugin class implementation...
//

LTRESULT CWorldModelPlugin::PreHook_EditStringList( const char *szRezPath, 
												    const char *szPropName,
													char **aszStrings,
													uint32 *pcStrings,
													const uint32 cMaxStrings,
													const uint32 cMaxStringLength )
{
	// Pass it on down to the DestructibleModel plugin

	if( m_DamageWorldModelPlugin.PreHook_EditStringList( szRezPath,
													szPropName,
													aszStrings,
													pcStrings,
													cMaxStrings,
													cMaxStringLength ) == LT_OK )
	{
		return LT_OK;
	}

	// See if we can handle the list...
	if( LTStrIEquals( szPropName, "ShadowLOD" ) )
	{
		return CEngineLODPropUtil::AddLODStrings(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
	}

	if( LTStrIEquals( szPropName, "PhysicsShape" ) )
	{
		// Fill the list with our blend modes...
		for( int i = 0; i < LTARRAYSIZE(c_aPhysicsShapes); i++ )
		{
			LTStrCpy( aszStrings[(*pcStrings)++], c_aPhysicsShapes[i], cMaxStringLength );
		}
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

LTRESULT CWorldModelPlugin::PreHook_PropChanged( const char *szObjName, 
												 const char *szPropName,
												 const int nPropType, 
												 const GenericProp &gpPropValue,
												 ILTPreInterface *pInterface,
												 const char *szModifiers )
{
	// See if the Destructible model will handel the props...

	if( LT_OK == m_DamageWorldModelPlugin.PreHook_PropChanged( szObjName,
														  szPropName,
														  nPropType,
														  gpPropValue,
														  pInterface,
														  szModifiers ))
	{
		return LT_OK;
	}

	// TODO:  Check the objects in the Attachments prop...
 

	return LT_UNSUPPORTED;
}


//
// WorldModel class implementation...
// 

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::WorldModel
//
//  PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

WorldModel::WorldModel( )
:	GameBase				( OT_WORLDMODEL ),
	m_sAttachments			( ),
	m_vAttachDir			( 0.0f, 1.0f, 0.0f ),
	m_hActivateParent		( NULL ),
	m_fAlphaScale			( 1.0f )
{
	m_WorldModelFlags = ( kWorldModelFlag_RemoveAttachments | kWorldModelFlag_CastShadow | 
		kWorldModelFlag_InheritActivationData | kWorldModelFlag_PlayerInteract );
	AddAggregate( &m_DamageWorldModel );

	m_hAttachDirObj.SetReceiver( *this );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::~WorldModel
//
//  PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

WorldModel::~WorldModel( )
{
	// Removing attachments when loading levels is unnecessary since they will be removed anyways...
	if( !g_pServerSaveLoadMgr->IsLoadingLevel( ))
		RemoveAtachments();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::EngineMessageFn
//
//  PURPOSE:	Handel messages from the engine
//
// ----------------------------------------------------------------------- //

uint32 WorldModel::EngineMessageFn( uint32 messageID, void *pData, float fData )
{
	switch( messageID )
	{
		case MID_PRECREATE:
		{
			// Let the GameBase handle the message first

			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			ObjectCreateStruct	*pOCS = (ObjectCreateStruct*)pData;

			if( pOCS )
			{
				if( PRECREATE_WORLDFILE == fData )
				{
					ReadProps( &pOCS->m_cProperties );
				}

				if( PRECREATE_SAVEGAME != fData )
				{
					// Init some data if it's not a saved game

					PostReadProp( pOCS );
				}
	
			}

			// Important!! - We already sent the message to the GameBase so DONT do it again.

			return dwRet;
		}
		break;

		case MID_OBJECTCREATED:
		{
			if( OBJECTCREATED_SAVEGAME != fData )
			{
				OnObjectCreated();
			}
		}
		break;

		case MID_ALLOBJECTSCREATED:
		{
			OnEveryObjectCreated();
		}
		break;

		case MID_UPDATE:
		{
			OnUpdate( g_pLTServer->GetTime() );
		}
		break;

		case MID_SAVEOBJECT:
		{
			OnSave( (ILTMessage_Write*)pData, (uint32)fData );
		}
		break;

		case MID_LOADOBJECT:
		{
			OnLoad( (ILTMessage_Read*)pData, (uint32)fData );
		}
		break;
	}

	return GameBase::EngineMessageFn( messageID, pData, fData );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::ReadProps
//
//  PURPOSE:	Read in property values 
//
// ----------------------------------------------------------------------- //

void WorldModel::ReadProps( const GenericPropList *pProps )
{
	ASSERT( pProps != NULL );

	m_fAlpha				= pProps->GetReal( "Alpha", 1.0f );
	m_WorldModelFlags = ( m_WorldModelFlags & ~kWorldModelFlag_IsKeyframed ) | ( pProps->GetBool( "IsKeyframed", false ) ? kWorldModelFlag_IsKeyframed : 0 );
	m_WorldModelFlags = ( m_WorldModelFlags & ~kWorldModelFlag_StartHidden ) | ( pProps->GetBool( "StartHidden", false ) ? kWorldModelFlag_StartHidden : 0 );
	m_sAttachments			= pProps->GetString( "Attachments", "" );
	m_WorldModelFlags = ( m_WorldModelFlags & ~kWorldModelFlag_RemoveAttachments ) | ( pProps->GetBool( "RemoveAttachments", true ) ? kWorldModelFlag_RemoveAttachments : 0 );
	m_vAttachDir			= pProps->GetVector( "AttachDir", LTVector( 0.0f, 200.0f, 0.0f ));
	m_eOriginalShadowLOD	= CEngineLODPropUtil::StringToLOD(pProps->GetString("ShadowLOD", "Low"));
	m_WorldModelFlags = ( m_WorldModelFlags & ~kWorldModelFlag_CastShadow ) | ( pProps->GetBool( "CastShadow", true ) ? kWorldModelFlag_CastShadow : 0 );
	m_WorldModelFlags = ( m_WorldModelFlags & ~kWorldModelFlag_InheritActivationData ) | ( pProps->GetBool( "InheritActivationData", true ) ? kWorldModelFlag_InheritActivationData : 0 );
	m_WorldModelFlags = ( m_WorldModelFlags & ~kWorldModelFlag_PlayerInteract ) | ( pProps->GetBool( "PlayerInteract", true ) ? kWorldModelFlag_PlayerInteract : 0 );

	// Read the Disturbance stuff...
	
	uint32 nDestroyAlarmLevel	= (uint32)pProps->GetLongInt( "DestroyAlarmLevel", 0 );
	float fStimRadius			= pProps->GetReal( "StimulusRadius", 0.0f );
	
	m_DamageWorldModel.SetDestroyedStimulus( fStimRadius, nDestroyAlarmLevel );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::PostReadProp
//
//  PURPOSE:	Initialize data after the property values are read
//
// ----------------------------------------------------------------------- //

void WorldModel::PostReadProp( ObjectCreateStruct *pOCS )
{
	ASSERT( pOCS != NULL );

	const GenericPropList *pProps = &pOCS->m_cProperties;

	pOCS->m_Flags			|= (pProps->GetBool( "BoxPhysics", true ) ? FLAG_BOXPHYSICS : 0 );
	pOCS->m_Flags			|= (pProps->GetBool( "TranslucentLight", true ) ? 0 : FLAG_NOLIGHT );
	pOCS->m_Flags2			|= (pProps->GetBool( "RigidBody", false) ? FLAG2_RIGIDBODY : 0 );
	
	if( IsMultiplayerGameServer( ))
	{
		// For multiplayer games make sure the object is only simulated on the clients...
		if( (pOCS->m_Flags2 & FLAG2_RIGIDBODY) && pProps->GetBool( "MPClientOnlyRigidBody", true ))
		{
			pOCS->m_Flags2	|= FLAG2_CLIENTRIGIDBODY;
			pOCS->m_Flags2	&= ~FLAG2_RIGIDBODY;
		}
	}
	
	//determine if this world model is translucent or not
	if(pOCS->m_cProperties.GetBool("Translucent", false))
		pOCS->m_Flags2 |= FLAG2_FORCETRANSLUCENT; 

	pOCS->SetFileName( pOCS->m_Name );
	pOCS->m_Flags |= FLAG_FULLPOSITIONRES | FLAG_DONTFOLLOWSTANDING | FLAG_GOTHRUWORLD | FLAG_REMOVEIFOUTSIDE;

	// Dont go through world if gravity is set

	if( pOCS->m_Flags & FLAG_GRAVITY )
	{
		pOCS->m_Flags &= ~FLAG_GOTHRUWORLD;
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::OnObjectCreated
//
//  PURPOSE:	Our object is now created... Init some more data
//
// ----------------------------------------------------------------------- //

void WorldModel::OnObjectCreated( )
{
	uint32	dwFlags;
	uint32	dwUserFlags = 0;

	g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags, dwFlags );

	// Set movable flag if we can be destroyed

	if(( m_WorldModelFlags & kWorldModelFlag_IsKeyframed ) || (m_DamageWorldModel.GetCanDamage() && !m_DamageWorldModel.GetNeverDestroy()) )
	{
		dwUserFlags |= USRFLG_MOVEABLE;
	}

	if( dwFlags & FLAG_VISIBLE )
	{
		dwUserFlags |= USRFLG_VISIBLE;
	}

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, dwUserFlags, USRFLG_MOVEABLE | USRFLG_VISIBLE );

	//setup our shadow for this LOD
	g_pLTServer->SetObjectShadowLOD( m_hObject, m_eOriginalShadowLOD );

	// Turn shadow off if needed...
	if( !( m_WorldModelFlags & kWorldModelFlag_CastShadow ))
	{
		g_pCmdMgr->QueueMessage( this, this, "CASTSHADOW 0" );
	}

	if( m_WorldModelFlags & kWorldModelFlag_StartHidden )
	{
		g_pCmdMgr->QueueMessage( this, this, "HIDDEN 1" );
	}

	// Link up the activation handler
	m_ActivateTypeHandler.Init( m_hObject );

	// Update the object's alpha
	UpdateAlpha();

	bool bMultiplayerNonSolid = false;
	if( IsMultiplayerGameServer( ))
	{
		uint32 dwFlags2 = 0;
		g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags2, dwFlags2 );

		if( dwFlags2 & FLAG2_CLIENTRIGIDBODY )
		{
			// Multiplayer rigidbody worldmodels should not have solid objects but should have solid rigidbodies...
			g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAG_SOLID | FLAG_FORCECLIENTUPDATE );
			g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, USRFLG_CLIENT_RIGIDBODY_ONLY, USRFLG_CLIENT_RIGIDBODY_ONLY );
			bMultiplayerNonSolid = true;
			HPHYSICSRIGIDBODY hRigidBody = INVALID_PHYSICS_RIGID_BODY;
			if( g_pLTServer->PhysicsSim( )->GetWorldModelRigidBody( m_hObject, hRigidBody ) == LT_OK )
			{
				g_pLTServer->PhysicsSim( )->SetRigidBodySolid( hRigidBody, true );
				g_pLTServer->PhysicsSim( )->PinRigidBody( hRigidBody, true );
				g_pLTServer->PhysicsSim( )->SetRigidBodyCollisionGroup( hRigidBody, PhysicsUtilities::ePhysicsGroup_UserMultiplayer );

				g_pLTServer->PhysicsSim( )->ReleaseRigidBody( hRigidBody );
			}
		}
	}

	// Check if this worldmodel shouldn't be interactable with player.  Don't override what the multiplayer check did though.
	if( !bMultiplayerNonSolid && !( m_WorldModelFlags & kWorldModelFlag_PlayerInteract ))
	{
		uint32 nFlags = 0;
		g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags, nFlags );
		if( nFlags & FLAG_SOLID )
		{
			HPHYSICSRIGIDBODY hRigidBody = INVALID_PHYSICS_RIGID_BODY;
			g_pLTServer->PhysicsSim( )->GetWorldModelRigidBody( m_hObject, hRigidBody );

			bool bRigidBodySolid = false;
			if( hRigidBody != INVALID_PHYSICS_RIGID_BODY )
			{
				g_pLTServer->PhysicsSim( )->GetRigidBodySolid( hRigidBody, bRigidBodySolid );
			}

			// Rigidbody should not have solid objects but should have solid rigidbodies.
			g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAG_SOLID | FLAG_FORCECLIENTUPDATE );

			if( hRigidBody != INVALID_PHYSICS_RIGID_BODY )
			{
				if( bRigidBodySolid )
				{
					g_pLTServer->PhysicsSim( )->SetRigidBodySolid( hRigidBody, true );
					// Put it into the playerrb group so the playerrb can't actually interact with it.
					g_pLTServer->PhysicsSim( )->SetRigidBodyCollisionGroup( hRigidBody, PhysicsUtilities::ePhysicsGroup_UserPlayerRigidBody );
				}
				g_pLTServer->PhysicsSim( )->ReleaseRigidBody( hRigidBody );
				hRigidBody = NULL;
			}
		}
	}

	// Don't update us yet
	SetNextUpdate( UPDATE_NEVER );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::OnAllObjectsCreated
//
//  PURPOSE:	Called once per object after every object in the world is loaded
//
// ----------------------------------------------------------------------- //

void WorldModel::OnEveryObjectCreated( )
{
	//if we've already been initialized, re-initializing will stomp existing data
	//	so we'll bail early to prevent save/load issues
	if( m_WorldModelFlags & kWorldModelFlag_Initialized )
	{
		return;
	}
	m_WorldModelFlags |= kWorldModelFlag_Initialized;

	// Save if we are currently set up to be activated...
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_User, dwFlags );

	m_WorldModelFlags = ( m_WorldModelFlags & ~kWorldModelFlag_CanActivate ) | (( dwFlags & USRFLG_CAN_ACTIVATE ) ? kWorldModelFlag_CanActivate : 0 );

	// Check for any objects to attach 

	if( m_sAttachments.empty() ) return;

	ConParse	parse( m_sAttachments.c_str() );
		
	while( g_pCommonLT->Parse( &parse ) == LT_OK )
	{
		// We have an object we want to attach... find it

		if( parse.m_nArgs > 0 && parse.m_Args[0] )
		{
			ObjArray<HOBJECT, MAX_OBJECT_ARRAY_SIZE>	objArray;
			g_pLTServer->FindNamedObjects( parse.m_Args[0], objArray );

			for( uint i = 0; i < objArray.NumObjects(); i++ )
			{
				 AttachObject( objArray.GetObject( i ) );
			}
		}
	}

	// Once they are added free the string because we don't want to re-add the objects 
	// everytime this objects gets loaded.  Our attachment list will take care of everything.

	std::string().swap( m_sAttachments );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::AttachObject
//
//  PURPOSE:	Attach the given object to us
//
// ----------------------------------------------------------------------- //

HOBJECT WorldModel::AttachObject( HOBJECT hObj )
{
	if( !hObj || (hObj == m_hObject))
		return NULL;

	// Make sure the object is detached first.
	DetachObject( hObj );

	LTVector	vPos, vParentPos;
	LTRotation	rRot, rParentRot;

	// Get our position and rotation

	g_pLTServer->GetObjectPos( m_hObject, &vParentPos );
	g_pLTServer->GetObjectRotation( m_hObject, &rParentRot );
	
	// Get the attachment object pos / rot

	g_pLTServer->GetObjectPos( hObj, &vPos );
	g_pLTServer->GetObjectRotation( hObj, &rRot );

	LTRotation rInvParentRot = ~rParentRot;

	// Calculate the offsets...

	LTVector	vPosOffset = rInvParentRot.RotateVector( vPos - vParentPos );
	LTRotation	rRotOffset = rInvParentRot * rRot;

	// Attach it...

	HATTACHMENT	hAttachment;
	LTRESULT	LTRes = g_pLTServer->CreateAttachment( m_hObject, hObj, NULL, 
													   &vPosOffset, &rRotOffset, &hAttachment );
	if( LTRes != LT_OK )
		return NULL;

	LTObjRefNotifier ref( *this );
	ref = hObj;
	m_AttachmentList.push_back( ref );

	// If the object is a WorldModel set it's activate parent...

	if( IsWorldModel( hObj ))
	{
		WorldModel *pWorldModel = dynamic_cast<WorldModel*>(g_pLTServer->HandleToObject( hObj ));
		if( pWorldModel && ( pWorldModel->m_WorldModelFlags & kWorldModelFlag_InheritActivationData ))
		{
			// Send any activate messages the WorldModel recevies to us...

			pWorldModel->SetActivateParent( m_hObject );

			// Add the WorldModel to our list of objects to inherit our ActivateType...

			m_ActivateTypeHandler.InheritObject( hObj );
		}
	}

	// Notify characters that they are attached to something.

	if( IsCharacter( hObj ) )
	{
		CCharacter* pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject( hObj ));
		if( pChar )
		{
			pChar->AttachToObject( m_hObject );
		}
	}

	return hObj;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::DetachObject
//
//  PURPOSE:	Remove the attachment from us
//
// ----------------------------------------------------------------------- //

void WorldModel::DetachObject( HOBJECT hObj )
{
	if( !hObj ) return;

	// Remove the object from our list of attachments.  Should only be in here once.
	for( ObjRefNotifierList::iterator iter = m_AttachmentList.begin( ); iter != m_AttachmentList.end( ); iter++ )
	{
		if( hObj == *iter )
		{
			// Remove it from our list.
			m_AttachmentList.erase( iter );
			break;
		}
	}
	

	HATTACHMENT	hAttachment = NULL;
	if( g_pLTServer->FindAttachment( m_hObject, hObj, &hAttachment ) != LT_OK )
		return;

	g_pLTServer->RemoveAttachment( hAttachment );

	// If the object is a WorldModel set it's activate parent...

	if( IsWorldModel( hObj ))
	{
		WorldModel *pWorldModel = dynamic_cast<WorldModel*>(g_pLTServer->HandleToObject( hObj ));
		if( pWorldModel )
		{
			// No longer send any activate messages the WorldModel recevies to us...

			pWorldModel->SetActivateParent( NULL );

			// Remove the WorldModel from our list of object to inherit our ActivateType...

			m_ActivateTypeHandler.DisownObject( hObj );
		}
	}

	// Notify characters that they are detached from something.

	if( IsCharacter( hObj ) )
	{
		CCharacter* pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject( hObj ));
		if( pChar )
		{
			pChar->DetachFromObject( m_hObject );
		}
	}

	// Don't just let the object float 

	uint32	dwFlags;
	g_pCommonLT->GetObjectFlags( hObj, OFT_Flags, dwFlags );

	if( (IsKindOf(hObj, "Prop" )) && (dwFlags & FLAG_GRAVITY) )
	{
		LTVector	vVel;

		g_pPhysicsLT->GetVelocity( hObj, &vVel );

		if( vVel.y > -0.1f )
		{
			vVel.y -= 10.0f;
			g_pPhysicsLT->SetVelocity( hObj, vVel );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::RemoveAtachments
//
//  PURPOSE:	Remove all of our attachments
//
// ----------------------------------------------------------------------- //

void WorldModel::RemoveAtachments( )
{
	// Remove all the objects attached to us.
	while( m_AttachmentList.size( ))
	{
		HOBJECT hObj = *m_AttachmentList.begin( );
		if( !hObj )
		{	
			m_AttachmentList.erase( m_AttachmentList.begin( ));
			continue;
		}

		// This will remove it from our attachment list.
		DetachObject( hObj );

		if( m_WorldModelFlags & kWorldModelFlag_RemoveAttachments )
		{
			g_pLTServer->RemoveObject( hObj );
		}
		else // Let the object destroy itself...
		{
			DamageStruct	damage;

			damage.eType	= DT_EXPLODE;
			damage.fDamage	= damage.kInfiniteDamage;
			damage.hDamager	= m_hObject;

			damage.DoDamage( m_hObject, hObj );
		}
	}

	// Clear our other lists.
	m_AttachMsgObjList.clear( );
	m_hAttachDirObj = NULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::OnUpdate
//
//  PURPOSE:	called to update the alpha of this object. This will not 
//				perform it unless it is a translucent world model
//
// ----------------------------------------------------------------------- //
void WorldModel::UpdateAlpha()
{
	//make sure we are a translucent world model first
	uint32 nFlags2;
	g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags2, nFlags2);

	if((m_fAlpha < 1.0f) || (nFlags2 & FLAG2_FORCETRANSLUCENT))
	{
		// Update the object's alpha
		LTVector4 vOldColor;
		g_pLTServer->GetObjectColor( m_hObject, &vOldColor.x, &vOldColor.y, &vOldColor.z, &vOldColor.w );
		g_pLTServer->SetObjectColor( m_hObject, vOldColor.x, vOldColor.y, vOldColor.z, m_fAlpha * m_fAlphaScale );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::OnUpdate
//
//  PURPOSE:	Update the WorldModel
//
// ----------------------------------------------------------------------- //

void WorldModel::OnUpdate( const double &fCurTime )
{
	// Do nothing... we should never get here

	LTASSERT( false, "TODO: Add description here");
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::OnLinkBroken
//
//  PURPOSE:	An object has broken its link with us
//
// ----------------------------------------------------------------------- //

void WorldModel::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	// Make sure the object is detached.
	DetachObject( hObj );

	// Since we're matching addresses of LTObjRefNotifiers from here down, we
	// can return as soon as we find a match.  We don't even need to call our baseclass.
	ObjRefNotifierList::iterator iter;

	// Remove the object from our list of attachments.  DetachObject won't be able
	// to catch it since the LTObjRefNotifier will already be nulled out.
	for( iter = m_AttachmentList.begin( ); iter != m_AttachmentList.end( ); iter++ )
	{
		if( pRef == &( *iter ))
		{
			// Remove it from our list.
			m_AttachmentList.erase( iter );
			return;
		}
	}

	for( iter = m_AttachMsgObjList.begin( ); iter != m_AttachMsgObjList.end( ); iter++ )
	{
		LTObjRefNotifier& attachmsgobj = *iter;
		if( pRef == &attachmsgobj )
		{
			m_AttachMsgObjList.erase( iter );
			return;
		}
	}

	GameBase::OnLinkBroken( pRef, hObj );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::HandleAttachMsg
//
//  PURPOSE:	Handles an ATTACH message...
//
// ----------------------------------------------------------------------- //

void WorldModel::HandleAttachMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() == 1 )
	{
		AttachFromMsg( crParsedMsg, "Prop" );
	}
	else if( crParsedMsg.GetArgCount() > 1 )
	{
		AttachFromMsg( crParsedMsg, NULL );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::HandleAttachClassMsg
//
//  PURPOSE:	Handles an ATTACHCLASS message...
//
// ----------------------------------------------------------------------- //

void WorldModel::HandleAttachClassMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	AttachFromMsg( crParsedMsg, crParsedMsg.GetArg(1).c_str() );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::AttachFromMsg
//
//  PURPOSE:	Handles an attach command
//
// ----------------------------------------------------------------------- //

void WorldModel::AttachFromMsg( const CParsedMsg &cMsg, const char *szClass )
{
	if( cMsg.GetArgCount() >= 2 && !szClass )
	{
		// Look for the specific objects we want to attach

		for( uint i = 1; i < cMsg.GetArgCount( ); i++ )
		{
			const char *pObjName = cMsg.GetArg(i).c_str();
			if( !pObjName )
				break;

			ObjArray<HOBJECT, 1> objArray;
			g_pLTServer->FindNamedObjects( const_cast<char*>(pObjName), objArray );

			if( objArray.NumObjects() )
			{
				AttachObject( objArray.GetObject( 0 ) );
			}
		}
	}
	else
	{
		if( m_vAttachDir.MagSqr() > 1.0f )
		{
			// Grab an object in the direction of the attach vector and attach it

			IntersectQuery	IQuery;
			IntersectInfo	IInfo;

			g_pLTServer->GetObjectPos( m_hObject, &IQuery.m_From );
			IQuery.m_To = IQuery.m_From + m_vAttachDir;

			IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;

			if( g_pLTServer->IntersectSegment( IQuery, &IInfo ) )
			{
				// Only attach objects of a specific class type...

				if( IInfo.m_hObject && IsKindOf( IInfo.m_hObject, szClass ))
				{
					if( m_hAttachDirObj )
						DetachObject( m_hAttachDirObj );

					m_hAttachDirObj = IInfo.m_hObject;
					AttachObject( m_hAttachDirObj );
				}
			}
		}
		else
		{
			LTRigidTransform tTrans;
			g_pLTServer->GetObjectTransform( m_hObject, &tTrans );

			LTVector vMin, vMax, vDims;

			g_pPhysicsLT->GetObjectDims( m_hObject, &vDims );

			vMin = tTrans.m_rRot * (tTrans.m_vPos - vDims);
			vMax = tTrans.m_rRot * (tTrans.m_vPos + vDims);

			ObjectList *pObjList = g_pLTServer->GetBoxIntersecters( &vMin, &vMax );
			if( !pObjList )
				return;

			HOBJECT hObj;

			ObjectLink *pLink = pObjList->m_pFirstLink;
			while( pLink )
			{
				hObj = pLink->m_hObject;
				if( hObj && IsKindOf( hObj, szClass ))
				{
					AttachObject( hObj );

					// add it to our list of objects attached via the ATTACH message...
					LTObjRefNotifier ref( *this );
					ref = hObj;
					m_AttachMsgObjList.push_back( ref );
				}
				pLink = pLink->m_pNext;
			}

			g_pLTServer->RelinquishList( pObjList );
			pObjList = NULL;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::HandleDetachMsg
//
//  PURPOSE:	Handles a detach command
//
// ----------------------------------------------------------------------- //

void WorldModel::HandleDetachMsg( HOBJECT hSender, const CParsedMsg &cMsg )
{
	if( cMsg.GetArgCount() >= 2 )
	{
		// Look for the specific objects we want to detach
		for( uint i = 1; i < cMsg.GetArgCount( ); i++ )
		{
			const char *pObjName = cMsg.GetArg(i).c_str();
			if( !pObjName )
				break;

			ObjArray<HOBJECT, 1> objArray;
			g_pLTServer->FindNamedObjects( const_cast<char *>(pObjName), objArray );

			if( objArray.NumObjects() )
			{
				DetachObject( objArray.GetObject( 0 ) );
			}
		}
	}
	else
	{
		// Remove an Object we attached using the AttachDir...

		if( m_hAttachDirObj )
		{
			DetachObject( m_hAttachDirObj );
			m_hAttachDirObj = NULL;
		}
		else
		{
			// Objects attached via ATTACH ....
			while( m_AttachMsgObjList.size( ))
			{
				ObjRefNotifierList::iterator msgiter = m_AttachMsgObjList.begin( );
				HOBJECT hObj = *msgiter;
				if( hObj )
					DetachObject( hObj );

				m_AttachMsgObjList.erase( msgiter );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::HandleDestroyMsg
//
//  PURPOSE:	Relay any recieved activate message to our activate parent...
//
// ----------------------------------------------------------------------- //

void WorldModel::HandleDestroyMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Allow the WorldModel to be destroyed and then let the aggregate destroy it...

	m_DamageWorldModel.SetNeverDestroy( false );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::HandleActivateMsg
//
//  PURPOSE:	Relay any recieved activate message to our activate parent...
//
// ----------------------------------------------------------------------- //

void WorldModel::HandleActivateMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( !m_hActivateParent )
		return;

	g_pCmdMgr->QueueMessage( g_pLTServer->HandleToObject( hSender ),
							 g_pLTServer->HandleToObject( m_hActivateParent ),
							 "ACTIVATE" );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::EmptyHandler
//
//  PURPOSE:	This exists just to have a function to call for a message
//				which was added solely for the purpose of exposing the
//				message to WorldEdit.  The message (NEVERDESTROY) needs to 
//				be exposed because our Destructible aggregate handles it.
//
// ----------------------------------------------------------------------- //

void WorldModel::EmptyHandler( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModel::HandleRigidBodyMsg
//
//	PURPOSE:	Handle a RIGIDBODY message...
//
// ----------------------------------------------------------------------- //

void WorldModel::HandleRigidBodyMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	static CParsedMsg::CToken s_cTok_1("1");
	static CParsedMsg::CToken s_cTok_True("TRUE");
	static CParsedMsg::CToken s_cTok_0("0");
	static CParsedMsg::CToken s_cTok_False("FALSE");

	if ((crParsedMsg.GetArg(1) == s_cTok_1) ||
		(crParsedMsg.GetArg(1) == s_cTok_True))
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags2, FLAG2_RIGIDBODY, FLAG2_RIGIDBODY);
	}
	else
	{
		if ((crParsedMsg.GetArg(1) == s_cTok_0) ||
			(crParsedMsg.GetArg(1) == s_cTok_False))
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags2, 0, FLAG2_RIGIDBODY);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModel::HandleScaleAlphaMsg
//
//	PURPOSE:	Handle a SCALEALPHS message...
//
// ----------------------------------------------------------------------- //

void WorldModel::HandleScaleAlphaMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_fAlphaScale = (float)LTStrToDouble(crParsedMsg.GetArg(1).c_str());
	UpdateAlpha();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::OnSave
//
//  PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void WorldModel::OnSave( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	ASSERT( pMsg != NULL );
	
	m_ActivateTypeHandler.Save( pMsg );

	SAVE_BYTE( m_WorldModelFlags );
	SAVE_STDSTRING( m_sAttachments );
	SAVE_VECTOR( m_vAttachDir );
	SAVE_HOBJECT( m_hAttachDirObj );
	SAVE_HOBJECT( m_hActivateParent );
	SAVE_FLOAT( m_fAlpha );
	SAVE_FLOAT( m_fAlphaScale );

	// Save the object lists last...
	ObjRefNotifierList::iterator iter;

	// First the Attachment List
	SAVE_BYTE( m_AttachmentList.size( ));
	for( iter = m_AttachmentList.begin( ); iter != m_AttachmentList.end( ); iter++ )
	{
		SAVE_HOBJECT( *iter );
	}
	
	SAVE_BYTE( m_AttachMsgObjList.size( ));
	for( iter = m_AttachMsgObjList.begin( ); iter != m_AttachMsgObjList.end( ); iter++ )
	{
		SAVE_HOBJECT( *iter );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::OnLoad
//
//  PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void WorldModel::OnLoad( ILTMessage_Read *pMsg, uint32 dwSaveFlags )
{
	ASSERT( pMsg != NULL );

	m_ActivateTypeHandler.Load( pMsg );

	LOAD_BYTE( m_WorldModelFlags );
	LOAD_STDSTRING( m_sAttachments );
	LOAD_VECTOR( m_vAttachDir );
	LOAD_HOBJECT( m_hAttachDirObj );
	LOAD_HOBJECT( m_hActivateParent );
	LOAD_FLOAT( m_fAlpha );
	LOAD_FLOAT( m_fAlphaScale );
	
	// Load the object lists last...
	uint8	nObjsInList = 0;

	// First the Attachment list
	LOAD_BYTE( nObjsInList );
	m_AttachmentList.clear( );
	if( nObjsInList > 0 )
	{
		LTObjRefNotifier ref( *this );
		for( uint8 i = 0; i < nObjsInList; ++i )
		{
			LOAD_HOBJECT( ref );
			if( ref )
			{
				m_AttachmentList.push_back( ref );
			}
		}
	}

	// Then the other attachment list
	LOAD_BYTE( nObjsInList );
	m_AttachMsgObjList.clear( );
	if( nObjsInList > 0 )
	{
		LTObjRefNotifier ref( *this );
		for( uint8 i = 0; i < nObjsInList; ++i )
		{
			LOAD_HOBJECT( ref );
			if( ref )
			{
				m_AttachMsgObjList.push_back( ref );
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModel::SetActivateParent
//
//	PURPOSE:	Assign our active parent and set our flags appropriately...
//
// ----------------------------------------------------------------------- //

void WorldModel::SetActivateParent( HOBJECT hParent )
{
	// Assign our parent..

	m_hActivateParent = hParent;

	if( !m_hActivateParent )
	{
		// Reset our can activate flag after we lose our parent...

		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, (( m_WorldModelFlags & kWorldModelFlag_CanActivate ) ? USRFLG_CAN_ACTIVATE : 0), USRFLG_CAN_ACTIVATE );
		return;
	}

	// Check our parents user flags to see if it can be activated...

	uint32 dwParentFlags;
	g_pCommonLT->GetObjectFlags( m_hActivateParent, OFT_User, dwParentFlags );

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_User, dwFlags );

	// Save whether we were activateble or not...

	m_WorldModelFlags = ( m_WorldModelFlags & ~kWorldModelFlag_CanActivate ) | ((dwFlags & USRFLG_CAN_ACTIVATE) ? kWorldModelFlag_CanActivate : 0 );

	// Set our can activate flag the same as our parents...
	if( dwParentFlags & USRFLG_CAN_ACTIVATE )
		dwFlags |= USRFLG_CAN_ACTIVATE;

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, dwFlags, USRFLG_CAN_ACTIVATE );

}
