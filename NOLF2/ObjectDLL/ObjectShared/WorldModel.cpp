// ----------------------------------------------------------------------- //
//
// MODULE  : WorldModel.cpp
//
// PURPOSE : WorlModel implementation
//
// CREATED : 5/9/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "ObjectMsgs.h"
	#include "ParsedMsg.h"
	#include "WorldModel.h"
	#include "ClientWeaponSFX.h"
	#include "ServerMark.h"
	#include "WeaponMgr.h"
	#include "Attachments.h"
	#include "FXButeMgr.h"

LINKFROM_MODULE( WorldModel );

//
// Globals...
//

	const char* c_aBlendModes[] =
	{
		"None",
		"Additive",
		"Translucent",
	};

#define MAX_MARKS_PER_OBJECT	40


//
// Add Props...
//

BEGIN_CLASS( WorldModel )

	ADD_DESTRUCTIBLE_MODEL_AGGREGATE( PF_GROUP(1), 0 )

	// Override default values...

	ADD_BOOLPROP_FLAG( NeverDestroy, LTTRUE, PF_GROUP(1) )

	// Add Object flag props...
	
	ADD_VISIBLE_FLAG( LTTRUE, 0 )
	ADD_SOLID_FLAG( LTTRUE, 0 )
	ADD_RAYHIT_FLAG( LTTRUE, 0 )
	ADD_GRAVITY_FLAG( LTFALSE, 0 )

	ADD_STRINGPROP_FLAG( BlendMode, "None", PF_STATICLIST )

	ADD_BOOLPROP_FLAG( BlockLight, LTTRUE, 0 ) // Used by pre-processor
	ADD_BOOLPROP_FLAG( BoxPhysics, LTTRUE, 0 )
	ADD_BOOLPROP_FLAG( FogDisable, LTFALSE, 0 )
	ADD_BOOLPROP_FLAG( IsKeyframed, LTFALSE, 0 )
	ADD_BOOLPROP_FLAG( StartHidden, LTFALSE, 0 )

	ADD_REALPROP_FLAG( Alpha, 1.0f, 0) // DO NOT REMOVE THIS!!!!  Pre-Processor looks at this value.
  
	// Add attachment props...

	ADD_STRINGPROP_FLAG( Attachments, "", 0 )	// List of objects to attach
	ADD_BOOLPROP_FLAG( RemoveAttachments, LTTRUE, 0 )
	ADD_VECTORPROP_VAL_FLAG( AttachDir, 0.0f, 200.0f, 0.0f, 0 )

	// Add a disturbance group...

	PROP_DEFINEGROUP(Disturbances, PF_GROUP(6))
		ADD_LONGINTPROP_FLAG(DestroyAlarmLevel, 0, PF_GROUP(6))
		ADD_REALPROP_FLAG(StimulusRadius, 0.0f, PF_GROUP(6))

END_CLASS_DEFAULT_FLAGS_PLUGIN( WorldModel, GameBase, NULL, NULL, CF_WORLDMODEL, CWorldModelPlugin )



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateMsgAttachDetach
//
//  PURPOSE:	Validation message for both ATTACH and DETACH messages
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateMsgAttachDetach( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	char szMsgName[16] = {0};

	SAFE_STRCPY( szMsgName, cpMsgParams.m_Args[0]);
	
	if( cpMsgParams.m_nArgs == 1 )
	{
		return LTTRUE;
	}

	int	i = 1;
	char *pObjName = cpMsgParams.m_Args[1];

	while( pObjName )
	{
		if( LT_NOTFOUND == pInterface->FindObject( pObjName ) )
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - ValidateMsgAttach()" );
				pInterface->CPrint( "    MSG - %s - Could not find object '%s'!", _strupr(szMsgName), pObjName );
			}
			
			return LTFALSE;
		}

		pObjName = (cpMsgParams.m_nArgs > ++i && cpMsgParams.m_Args[i]) ? cpMsgParams.m_Args[i] : LTNULL;
	}

	return LTTRUE;
}


//
// Register the calss with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( WorldModel )
	
	CMDMGR_ADD_MSG( ATTACH, -1, ValidateMsgAttachDetach, "ATTACH <object name(s)>" )
	CMDMGR_ADD_MSG( DETACH, -1, ValidateMsgAttachDetach, "DETACH <object name(s)>" )
	CMDMGR_ADD_MSG( ATTACHCLASS, 2, NULL, "ATTACHCLASS <class name>" )
	CMDMGR_ADD_MSG( DESTROY, 1, NULL, "DESTROY" )

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

	if( m_DamageModelPlugin.PreHook_EditStringList( szRezPath,
													szPropName,
													aszStrings,
													pcStrings,
													cMaxStrings,
													cMaxStringLength ) == LT_OK )
	{
		return LT_OK;
	}

	// See if we can handle the list...

	if( !_stricmp( szPropName, "BlendMode" ) )
	{
		// Fill the list with our blend modes...

		for( int i = 0; i <= WM_BLEND_MAXMODES; i++ )
		{
			strcpy( aszStrings[(*pcStrings)++], c_aBlendModes[i] );
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

	if( LT_OK == m_DamageModelPlugin.PreHook_PropChanged( szObjName,
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
	m_bIsKeyframed			( false ),
	m_bStartHidden			( false ),
	m_hstrAttachments		( LTNULL ),
	m_bRemoveAttachments	( LTTRUE ),
	m_vAttachDir			( 0.0f, 1.0f, 0.0f ),
	m_hackInitialRot		( 0.0f, 0.0f, 0.0f, 1.0f ),
	m_fStimRadius			( 0.0f ),
	m_nDestroyAlarmLevel	( 0 ),
	m_hActivateParent		( LTNULL ),
	m_bCanActivate			( false )
{
	AddAggregate( &m_DamageModel );

	m_hAttachDirObj.SetReceiver( *this );
	
	m_ActivateTypeHandler.Init( this );
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
	FREE_HSTRING( m_hstrAttachments );
	
	RemoveAtachments();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::EngineMessageFn
//
//  PURPOSE:	Handel messages from the engine
//
// ----------------------------------------------------------------------- //

uint32 WorldModel::EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData )
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
// tagRP: HACK - Save off the rotation then get rid of it to deal with the double rotation problem
					m_hackInitialRot = pOCS->m_Rotation;
					pOCS->m_Rotation.Identity();
// end HACK
					ReadProps( pOCS );
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

void WorldModel::ReadProps( ObjectCreateStruct *pOCS )
{
	ASSERT( pOCS != NULL );

	uint8		nBlendMode = 0;
	GenericProp	genProp;

	// Get the blend mode...

	if( g_pLTServer->GetPropGeneric( "BlendMode", &genProp ) == LT_OK )
	{
		if( genProp.m_String[0] )
		{
			for( int i = 0; i <= WM_BLEND_MAXMODES; i++ )
			{
				if( !_stricmp( genProp.m_String, c_aBlendModes[i] ))
				{
					nBlendMode = i;
					break;
				}
			}
		}
	}

	switch( nBlendMode )
	{
		case WM_BLEND_ADDITIVE:		pOCS->m_Flags2 |= FLAG2_ADDITIVE | FLAG2_FORCETRANSLUCENT; break;
		case WM_BLEND_TRANSLUCENT:	pOCS->m_Flags2 |= FLAG2_FORCETRANSLUCENT; break;
	}

	if( g_pLTServer->GetPropGeneric( "BoxPhysics", &genProp ) == LT_OK )
	{
		pOCS->m_Flags |= (genProp.m_Bool ? FLAG_BOXPHYSICS : 0 );
	}

	if( g_pLTServer->GetPropGeneric( "FogDisable", &genProp ) == LT_OK )
	{
		pOCS->m_Flags |= (genProp.m_Bool ? FLAG_FOGDISABLE : 0 );
	}

	if( g_pLTServer->GetPropGeneric( "IsKeyframed", &genProp ) == LT_OK )
	{
		m_bIsKeyframed = genProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "StartHidden", &genProp ) == LT_OK )
	{
		m_bStartHidden = genProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "Attachments", &genProp ) == LT_OK )
	{
		if( genProp.m_String[0] )
		{
			m_hstrAttachments = g_pLTServer->CreateString( genProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "RemoveAttachments", &genProp ) == LT_OK )
	{
		m_bRemoveAttachments = genProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "AttachDir", &genProp ) == LT_OK )
	{
		m_vAttachDir = genProp.m_Vec;
	}

	// Read the Disturbance stuff...

	if( g_pLTServer->GetPropGeneric( "DestroyAlarmLevel", &genProp ) == LT_OK )
	{
		m_nDestroyAlarmLevel = (uint32)genProp.m_Long;
	}
	
	if( g_pLTServer->GetPropGeneric( "StimulusRadius", &genProp ) == LT_OK )
	{
		m_fStimRadius = genProp.m_Float;
	}

	m_DamageModel.SetDestroyedStimulus(m_fStimRadius, m_nDestroyAlarmLevel);

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

	SAFE_STRCPY( pOCS->m_Filename, pOCS->m_Name );
	pOCS->m_SkinName[0] = '\0';
	pOCS->m_Flags |= FLAG_FULLPOSITIONRES | FLAG_DONTFOLLOWSTANDING | FLAG_GOTHRUWORLD;

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

	if( m_bIsKeyframed || (m_DamageModel.GetCanDamage() && !m_DamageModel.GetNeverDestroy()) )
	{
		dwUserFlags |= USRFLG_MOVEABLE;
	}

	if( dwFlags & FLAG_VISIBLE )
	{
		dwUserFlags |= USRFLG_VISIBLE;
	}

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, dwUserFlags, USRFLG_MOVEABLE | USRFLG_VISIBLE );

	if( m_bStartHidden )
	{
		SendTriggerMsgToObject( this, m_hObject, LTFALSE, "Hidden 1" );
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
	// Save if we are currently set up to be activated...

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_User, dwFlags );

	m_bCanActivate = !!(dwFlags & USRFLG_CAN_ACTIVATE);

	// Check for any objects to attach 

	if( !m_hstrAttachments ) return;

	const char *pAttachments = g_pLTServer->GetStringData( m_hstrAttachments );
	if( !pAttachments ) return;

	ConParse	parse( pAttachments );
		
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

	FREE_HSTRING( m_hstrAttachments );
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
	if( !hObj ) return LTNULL;

	// Make sure the object is detached first.
	DetachObject( hObj );

	LTVector	vPos, vParentPos;
	LTRotation	rRot, rParentRot;

	// Get our position and rotation

	g_pLTServer->GetObjectPos( m_hObject, &vParentPos );
	g_pLTServer->GetObjectRotation( m_hObject, &rParentRot );
	
	LTMatrix mRot;
	rParentRot.ConvertToMatrix( mRot );

	// Get the attachment object pos / rot

	g_pLTServer->GetObjectPos( hObj, &vPos );
	g_pLTServer->GetObjectRotation( hObj, &rRot );

	// Calculate the offsets...

	LTVector	vPosOffset( ~mRot * (vPos - vParentPos) );
	LTRotation	rRotOffset = rRot * ~rParentRot;

	// Attach it...

	HATTACHMENT	hAttachment;
	LTRESULT	LTRes = g_pLTServer->CreateAttachment( m_hObject, hObj, LTNULL, 
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
		if( pWorldModel )
		{
			// Send any activate messages the WorldModel recevies to us...

			pWorldModel->SetActivateParent( m_hObject );

			// Add the WorldModel to our list of objects to inherit our ActivateType...

			m_ActivateTypeHandler.InheritObject( hObj );
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

			pWorldModel->SetActivateParent( LTNULL );

			// Remove the WorldModel from our list of object to inherit our ActivateType...

			m_ActivateTypeHandler.DisownObject( hObj );
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
			g_pPhysicsLT->SetVelocity( hObj, &vVel );
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

		if( m_bRemoveAttachments )
		{
			g_pLTServer->RemoveObject( hObj );
		}
		else // Let the object destroy itself...
		{
			DamageStruct	damage;

			damage.eType	= DT_EXPLODE;
			damage.fDamage	= damage.kInfiniteDamage;
			damage.hDamager	= m_hObject;
			damage.vDir.Init( 0.0f, 1.0f, 0.0f );

			damage.DoDamage( this, hObj );
		}
	}

	// Clear our other lists.
	m_AttachMsgObjList.clear( );
	m_MarkList.clear( );
	m_hAttachDirObj = NULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::OnUpdate
//
//  PURPOSE:	Update the WorldModel
//
// ----------------------------------------------------------------------- //

void WorldModel::OnUpdate( const LTFLOAT &fCurTime )
{
	// Do nothing... we should never get here

	_ASSERT( LTFALSE );
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

	for( iter = m_MarkList.begin( ); iter != m_MarkList.end( ); iter++ )
	{
		LTObjRefNotifier& mark = *iter;
		if( pRef == &mark )
		{
			m_MarkList.erase( iter );
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
//  ROUTINE:	WorldModel::OnTrigger
//
//  PURPOSE:	Handel recieving a trigger msg from another object
//
// ----------------------------------------------------------------------- //

bool WorldModel::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Attach(KEY_ATTACH);
	static CParsedMsg::CToken s_cTok_Detach(KEY_DETACH);
	static CParsedMsg::CToken s_cTok_AttachClass("ATTACHCLASS");
	static CParsedMsg::CToken s_cTok_Destroy( "DESTROY" );
	static CParsedMsg::CToken s_cTok_Activate( "ACTIVATE" );

	if( cMsg.GetArg(0) == s_cTok_Attach && cMsg.GetArgCount() == 1 )
	{
		HandleAttachMsg( cMsg, "Prop" );
	}
	else if( cMsg.GetArg(0) == s_cTok_Attach && cMsg.GetArgCount() > 1 )
	{
		HandleAttachMsg( cMsg, LTNULL );
	}
	else if( cMsg.GetArg(0) == s_cTok_Detach )
	{
		HandleDetachMsg( cMsg );
	}
	else if( cMsg.GetArg(0) == s_cTok_AttachClass && cMsg.GetArgCount() == 2 )
	{
		HandleAttachMsg( cMsg, cMsg.GetArg(1).c_str() );
	}
	else if( cMsg.GetArg(0) == s_cTok_Destroy )
	{
		// Allow the WorldModel to be destroyed and then let the aggregate destroy it...
		
		m_DamageModel.SetNeverDestroy( LTFALSE );
	}
	else if( cMsg.GetArg(0) == s_cTok_Activate )
	{
		HandleActivateMsg( hSender, cMsg );
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::HandleAttachMsg
//
//  PURPOSE:	Handles an attach command
//
// ----------------------------------------------------------------------- //

void WorldModel::HandleAttachMsg( const CParsedMsg &cMsg, const char *szClass )
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

			if( g_pLTServer->IntersectSegment( &IQuery, &IInfo ) )
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
			LTVector	vPos;
			LTRotation	rRot;
			LTMatrix	mMat;

			g_pLTServer->GetObjectPos( m_hObject, &vPos );
			g_pLTServer->GetObjectRotation( m_hObject, &rRot );
			rRot.ConvertToMatrix( mMat );

			LTVector vMin, vMax, vDims;

			g_pPhysicsLT->GetObjectDims( m_hObject, &vDims );

			vMin = mMat * (vPos - vDims);
			vMax = mMat * (vPos + vDims);

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

void WorldModel::HandleDetachMsg( const CParsedMsg &cMsg )
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
//  ROUTINE:	WorldModel::HandleActivateMsg
//
//  PURPOSE:	Relay any recieved activate message to our activate parent...
//
// ----------------------------------------------------------------------- //

void WorldModel::HandleActivateMsg( HOBJECT hSender, const CParsedMsg &cMsg )
{
	if( !m_hActivateParent )
		return;

	SendTriggerMsgToObject( g_pLTServer->HandleToObject( hSender ), m_hActivateParent, LTTRUE, "ACTIVATE" );
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

	SAVE_bool( m_bIsKeyframed );
	SAVE_bool( m_bStartHidden );
	SAVE_HSTRING( m_hstrAttachments );
	SAVE_BOOL( m_bRemoveAttachments );
	SAVE_VECTOR( m_vAttachDir );
	SAVE_HOBJECT( m_hAttachDirObj );
	SAVE_ROTATION( m_hackInitialRot );
	SAVE_FLOAT( m_fStimRadius );
	SAVE_HOBJECT( m_hActivateParent );
	SAVE_bool( m_bCanActivate );

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

	// Then the Mark List
	SAVE_BYTE( m_MarkList.size( ));
	for( iter = m_MarkList.begin( ); iter != m_MarkList.end( ); iter++ )
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

	LOAD_bool( m_bIsKeyframed );
	LOAD_bool( m_bStartHidden );
	LOAD_HSTRING( m_hstrAttachments );
	LOAD_BOOL( m_bRemoveAttachments );
	LOAD_VECTOR( m_vAttachDir );
	LOAD_HOBJECT( m_hAttachDirObj );
	LOAD_ROTATION( m_hackInitialRot );
	LOAD_FLOAT( m_fStimRadius );
	LOAD_HOBJECT( m_hActivateParent );
	LOAD_bool( m_bCanActivate );

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

	// Then the Mark list
	LOAD_BYTE( nObjsInList );
	m_MarkList.clear( );
	if( nObjsInList > 0 )
	{
		LTObjRefNotifier ref( *this );
		for( uint8 i = 0; i < nObjsInList; ++i )
		{
			LOAD_HOBJECT( ref );
			if( ref )
			{
				m_MarkList.push_back( ref );
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::GetWorldModelRotation
//
//  PURPOSE:	Get around the double rotation bug
//
// ----------------------------------------------------------------------- //

LTRotation WorldModel::GetWorldModelRotation( )
{
	LTRotation rRot;

	g_pLTServer->GetObjectRotation( m_hObject, &rRot );

	return (m_hackInitialRot * rRot );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::SetWorldModelRotation
//
//  PURPOSE:	Get around the double rotation bug
//
// ----------------------------------------------------------------------- //

void WorldModel::SetWorldModelRotation( LTRotation &rNewRot )
{
	LTRotation rRot = rNewRot * ~m_hackInitialRot;

	g_pLTServer->SetObjectRotation( m_hObject, &rRot );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModel::CreateServerMark
//
//	PURPOSE:	Create a server side mark
//
// ----------------------------------------------------------------------- //

bool WorldModel::CreateServerMark(CLIENTWEAPONFX & theStruct)
{
	// See if we should create a mark, or simply move one of the GameBase's
	// marks.

	// If the GameBase has the max number of marks or this mark is very close
	// to a pre-existing mark, just move that mark to the new position.

    HOBJECT hMoveObj = LTNULL;
    HOBJECT hFarObj  = LTNULL;

    uint32 nNumMarks = m_MarkList.size( );

    LTFLOAT  fClosestMarkDist  = REGION_DIAMETER;
    LTFLOAT  fFarthestMarkDist = 0.0f;
    uint8   nNumInRegion      = 0;
    LTVector vPos;

	for( ObjRefNotifierList::iterator iter = m_MarkList.begin( ); iter != m_MarkList.end( ); iter++ )
	{
		HOBJECT hObj = *iter;
		if( !hObj )
			continue;

		HATTACHMENT hAttachment = NULL;
        if (LT_OK == g_pLTServer->FindAttachment( m_hObject, hObj, &hAttachment))
		{
			LTransform transform;
            g_pCommonLT->GetAttachmentTransform(hAttachment, transform, LTTRUE);

			vPos = transform.m_Pos;
		}

        LTFLOAT fDist = VEC_DISTSQR(vPos, theStruct.vPos);
		if (fDist < REGION_DIAMETER)
		{
			if (fDist < fClosestMarkDist)
			{
				fClosestMarkDist = fDist;
				hMoveObj = hObj;
			}

			if (++nNumInRegion > MAX_MARKS_IN_REGION)
			{
				// Just move this mark to the correct pos...
				hMoveObj = hMoveObj ? hMoveObj : hObj;
				break;
			}
		}

		if (fDist > fFarthestMarkDist)
		{
			fFarthestMarkDist = fDist;
			hFarObj = hObj;
		}
	}

	// If we've got the max number of marks on this object, just move
	// the closest one to the new position...

	if (nNumMarks >= MAX_MARKS_PER_OBJECT)
	{
		if( !hMoveObj )
		{
			if( hFarObj )
			{
				hMoveObj = hFarObj;
			}
			else
			{
				HOBJECT hFirstMark = *m_MarkList.begin( );
				hMoveObj = hFirstMark;
			}
		}
	}
	else
	{
        hMoveObj = LTNULL; // Need to create one...
	}

	// Re-setup the object to move it...

	if (hMoveObj && IsKindOf(hMoveObj, "CServerMark"))
	{
        CServerMark* pMoveMark = (CServerMark*) g_pLTServer->HandleToObject(hMoveObj);
		if (!pMoveMark)
			return false;

		// Since this mark is already attached to us, remove the attachment
		DetachObject( pMoveMark->m_hObject );

		if( !AttachServerMark( *pMoveMark, (CLIENTWEAPONFX)theStruct))
		{
			g_pLTServer->RemoveObject( pMoveMark->m_hObject );
			RemoveMarkFromList( pMoveMark->m_hObject );
			pMoveMark = NULL;
			return false;
		}

		return true;
	}


	// Okay, no luck, need to create a new mark...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

    LTFLOAT fScaleAdjust = 1.0f;
	if (!GetImpactSprite((SurfaceType)theStruct.nSurfaceType, fScaleAdjust,
		theStruct.nAmmoId, createStruct.m_Filename, ARRAY_LEN(createStruct.m_Filename)))
	{
		return false;
	}

	createStruct.m_ObjectType = OT_SPRITE;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT | FLAG_ROTATEABLESPRITE;
	createStruct.m_Pos = theStruct.vPos;

	createStruct.m_Rotation = LTRotation(theStruct.vSurfaceNormal, LTVector(0.0f, 1.0f, 0.0f));

	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(theStruct.nAmmoId);
	if( !pAmmo ) 
		return false;

    static HCLASS hClass = g_pLTServer->GetClass("CServerMark");
    CServerMark* pMark = (CServerMark*) g_pLTServer->CreateObject(hClass, &createStruct);
	if (!pMark)
		return false;


	// Randomly adjust the mark's scale to add a bit o spice...

	if (pAmmo->pImpactFX)
	{
        LTFLOAT fScale = fScaleAdjust * pAmmo->pImpactFX->fMarkScale;

        LTVector vScale(fScale, fScale, fScale);
        g_pLTServer->ScaleObject(pMark->m_hObject, &vScale);
	}

	if( !AttachServerMark( *pMark, (CLIENTWEAPONFX)theStruct))
	{
		g_pLTServer->RemoveObject( pMark->m_hObject );
		pMark = NULL;
		return false;
	}

	AddMarkToList( pMark->m_hObject );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModel::AttachServerMark()
//
//	PURPOSE:	Attach the mark to its parent
//
// ----------------------------------------------------------------------- //

bool WorldModel::AttachServerMark( CServerMark& mark, CLIENTWEAPONFX & theStruct)
{
	LTransform globalTransform, parentTransform, localTransform;
    ILTTransform *pTransformLT;
    LTVector vParentPos, vOffset;
    LTRotation rParentRot, rRot;
    LTRotation rOffset;

	pTransformLT = g_pLTServer->GetTransformLT();

	// Attach the mark to the parent object...

	// Figure out what the rotation we want is.
    rOffset.Init();
	rRot = LTRotation(theStruct.vSurfaceNormal, LTVector(0.0f, 1.0f, 0.0f));


	// MD
	// Ok, now we have the transform in global space but attachments are specified in
	// local space (so they can move as the object moves and rotates).

	// Set the global LTransform.
	pTransformLT->Set(globalTransform, theStruct.vPos, rRot);

	// Get the object's transform.
	g_pLTServer->GetObjectPos( m_hObject, &vParentPos);
	g_pLTServer->GetObjectRotation( m_hObject, &rParentRot);
	parentTransform.m_Pos = vParentPos;
	parentTransform.m_Rot = rParentRot;
	
	parentTransform.m_Scale.Init(1,1,1);
	globalTransform.m_Scale.Init(1,1,1);

	// Get the offset.
	pTransformLT->Difference(localTransform, globalTransform, parentTransform);
	vOffset = localTransform.m_Pos;
	rOffset = localTransform.m_Rot;


	HATTACHMENT hAttachment = NULL;
    LTRESULT dRes = g_pLTServer->CreateAttachment( m_hObject, mark.m_hObject, LTNULL,
											     &vOffset, &rOffset, &hAttachment);
    if (dRes != LT_OK)
	{
		return false;
	}

	// Add to the attachment list.
	LTObjRefNotifier ref( *this );
	ref = mark.m_hObject;
	m_AttachmentList.push_back( ref );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModel::AddMarkToList
//
//	PURPOSE:	Add Mark to list of marks.
//
// ----------------------------------------------------------------------- //

void WorldModel::AddMarkToList( HOBJECT hObj )
{
	// Make sure its only on there once.
	RemoveMarkFromList( hObj );

	LTObjRefNotifier ref( *this );
	ref = hObj;
	m_MarkList.push_back( ref );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModel::RemoveMarkFromList
//
//	PURPOSE:	Remove mark from list of marks.
//
// ----------------------------------------------------------------------- //

void WorldModel::RemoveMarkFromList( HOBJECT hObj )
{
	for( ObjRefNotifierList::iterator iter = m_MarkList.begin( ); iter != m_MarkList.end( ); iter++ )
	{
		if( *iter == hObj )
		{
			m_MarkList.erase( iter );
			break;
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

		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, (m_bCanActivate ? USRFLG_CAN_ACTIVATE : 0), USRFLG_CAN_ACTIVATE );
		return;
	}

	// Check our parents user flags to see if it can be activated...

	uint32 dwParentFlags;
	g_pCommonLT->GetObjectFlags( m_hActivateParent, OFT_User, dwParentFlags );

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_User, dwFlags );

	// Save whether we were activateble or not...

	m_bCanActivate = !!(dwFlags & USRFLG_CAN_ACTIVATE);

	// Set our can activate flag the same as our parents...

	dwFlags = 0;
	if( dwParentFlags & USRFLG_CAN_ACTIVATE )
		dwFlags = USRFLG_CAN_ACTIVATE;

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, dwFlags, USRFLG_CAN_ACTIVATE );

}