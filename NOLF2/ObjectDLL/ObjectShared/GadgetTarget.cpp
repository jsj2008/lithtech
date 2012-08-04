// ----------------------------------------------------------------------- //
//
// MODULE  : GadgetTarget.cpp
//
// PURPOSE : GadgetTarget implementation
//
// CREATED : 8/27/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "ObjectMsgs.h"
	#include "ParsedMsg.h"
	#include "MsgIDs.h"
	#include "PlayerObj.h"
	#include "Spawner.h"
	#include "GadgetTargetTypes.h"
	#include "GadgetTarget.h"
	#include "ServerSoundMgr.h"

LINKFROM_MODULE( GadgetTarget );

//
// Defines...
//

	#define EAVESDROPBUG_WEAPON_NAME	"Eavesdropping Bug"

// KLS NOTE: Need to bute these!
	#define GT_GREEN_LIGHT				"FX\\Test\\Flares\\CamGreen.spr"
	#define GT_RED_LIGHT				"FX\\Test\\Flares\\CamRed.spr"
	#define GT_LIGHT_SCALE				0.075

//
// Globals\Statics...
//

	const char c_szGADGET[] = "GADGET";
	const char c_szSTOPPED[] = "STOPPED";
	const char c_szACTIVATE[] = "ACTIVATE";
	const char c_szCANACTIVATE[] = "CANACTIVATE";	
	const char c_szON[] = "ON";	
	const char c_szOFF[] = "OFF";
	const char c_szDisable[] = "DISABLE";


BEGIN_CLASS( GadgetTarget )
	
	// Override base-class properties...

	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP(1), PF_HIDDEN)
	
	ADD_STRINGPROP_FLAG(Filename, "", PF_HIDDEN | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, "", PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, PF_HIDDEN)
	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_SOLID_FLAG(1, PF_HIDDEN)
	ADD_SHADOW_FLAG(0, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(MoveToFloor, LTTRUE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)
	ADD_COLORPROP_FLAG(ObjectColor, 255.0f, 255.0f, 255.0f, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Additive, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Multiply, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(RayHit, LTTRUE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(DetailLevel, "HIGH", PF_STATICLIST | PF_HIDDEN)
	
	ADD_BOOLPROP_FLAG(CanTransition, LTFALSE, PF_HIDDEN)


	// New Props...

	ADD_STRINGPROP_FLAG( GadgetTargetName, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS )
	ADD_LONGINTPROP_FLAG( CodeID, 0000, 0 ) 
	ADD_REALPROP_FLAG( MinTime, 0.0f, 0 )
	ADD_REALPROP_FLAG( MaxTime, 0.0f, 0 )
	ADD_STRINGPROP_FLAG( Command, "", PF_NOTIFYCHANGE )
	ADD_STRINGPROP_FLAG( PowerOffCommand, "", PF_NOTIFYCHANGE )
    ADD_BOOLPROP_FLAG( StartOn, LTTRUE, 0, )
	ADD_STRINGPROP_FLAG( Team, "NoTeam", PF_STATICLIST )

END_CLASS_DEFAULT_FLAGS_PLUGIN( GadgetTarget, Prop, NULL, NULL, 0, CGadgetTargetPlugin )

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( GadgetTarget )

	CMDMGR_ADD_MSG( GADGET, 2, NULL, "GADGET <ammo id>" )
	CMDMGR_ADD_MSG( CANACTIVATE, 1, NULL, "CANACTIVATE" )
	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )
	CMDMGR_ADD_MSG( DISABLE, 1, NULL, "DISABLE" )
	CMDMGR_ADD_MSG( TEAM, 2, NULL, "TEAM <0, 1, -1>" )

CMDMGR_END_REGISTER_CLASS( GadgetTarget, Prop )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGadgetTargetPlugin::PreHook_EditStringList
//
//  PURPOSE:	Populate the Type list
//
// ----------------------------------------------------------------------- //

LTRESULT CGadgetTargetPlugin::PreHook_EditStringList(
													const char* szRezPath,
													const char* szPropName,
													char** aszStrings,
													uint32* pcStrings,
													const uint32 cMaxStrings,
													const uint32 cMaxStringLength )
{

	if( LT_OK == CPropPlugin::PreHook_EditStringList(szRezPath, szPropName,
		aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}
	else if( !_stricmp("GadgetTargetName", szPropName) )
	{
		// Fill the list with our Gadget Target names...

		m_GadgetTargetMgrPlugin.SetFilterOutTypes( GT_TYPE_TO_FLAG( eBombable ) );

		if( m_GadgetTargetMgrPlugin.PreHook_EditStringList( szRezPath, szPropName, aszStrings,
			pcStrings, cMaxStrings, cMaxStringLength ) == LT_OK )
		{
			return LT_OK;
		}
	}
	else if( !_stricmp( "Team", szPropName ) )
	{
		char szTeam[32] = {0};

		_ASSERT(cMaxStrings > (*pcStrings) + 1);
		strcpy( aszStrings[(*pcStrings)++], "NoTeam" );
		
		for( int i = 0; i < MAX_TEAMS; ++i )
		{
			_ASSERT(cMaxStrings > (*pcStrings) + 1);

			sprintf( szTeam, "Team%i", i );
			strcpy( aszStrings[(*pcStrings)++], szTeam );
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGadgetTargetPlugin::PreHook_EditStringList
//
//  PURPOSE:	Set the model & dims
//
// ----------------------------------------------------------------------- //

LTRESULT CGadgetTargetPlugin::PreHook_Dims(
											const char* szRezPath,
											const char* szPropValue,
											char* szModelFilenameBuf,
											int	  nModelFilenameBufLen,
											LTVector & vDims )
{
	if (m_GadgetTargetMgrPlugin.PreHook_Dims(szRezPath, szPropValue,
		szModelFilenameBuf, nModelFilenameBufLen, vDims) == LT_OK)
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGadgetTargetPlugin::PreHook_PropChanged
//
//  PURPOSE:	Check properties that change...
//
// ----------------------------------------------------------------------- //

LTRESULT CGadgetTargetPlugin::PreHook_PropChanged( const char *szObjName, 
												   const char *szPropName,
												   const int nPropType,
												   const GenericProp &gpPropValue,
												   ILTPreInterface *pInterface,
												   const char *szModifiers )
{
	// Send the prop change to our base to see if it will handle it..

	if( CPropPlugin::PreHook_PropChanged( szObjName,
										  szPropName,
										  nPropType,
										  gpPropValue,
										  pInterface,
										  szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	// Only our command gets change notification so just send it to the command mgr...

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


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GadgetTarget::GadgetTarget
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

GadgetTarget::GadgetTarget( )
:	Prop					( ),
	m_pGTInfo				( LTNULL ),
	m_fMinTime				( 0.0f ),
	m_fMaxTime				( 0.0f ),
	m_fDisableTime			( 0.0f ),
	m_fTotalTime			( 0.0f ),
	m_fSoundRadius			( 400.0f ),
	m_hstrDisablingSnd		( LTNULL ),
	m_hstrDisabledSnd		( LTNULL ),
	m_hstrDisabledCmd		( LTNULL ),
	m_hstrPowerOffCmd		( LTNULL ),
	m_dwGadgetUsrFlgs		( 0 ),
	m_hSound				( LTNULL ),
	m_bDisabled				( LTFALSE ),
	m_bDisableRequested		( LTFALSE ),
	m_dwCodeID				( 0 ),
	m_bRemoveWhenDisabled	( LTFALSE ),
	m_bInfiniteDisables		( LTFALSE ),
	m_bInfiniteActivates	( LTFALSE ),
	m_bOn					( LTTRUE ),
	m_bRestoreFlagsOnLoad	( LTFALSE ),
	m_eLightPos				( eLightPos1 ),
	m_nGTID					( GTMGR_INVALID_ID ),
	m_nTeamID				( INVALID_TEAM )
{
	m_hAttachedModel.SetReceiver( *this );
    m_hLight.SetReceiver( *this );
	
	// Do not allow gadget targets to transition...

	DestroyTransitionAggregate();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GadgetTarget::~GadgetTarget
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

GadgetTarget::~GadgetTarget( )
{
	FREE_HSTRING( m_hstrDisablingSnd );
	FREE_HSTRING( m_hstrDisabledSnd );
	FREE_HSTRING( m_hstrDisabledCmd );
	FREE_HSTRING( m_hstrPowerOffCmd );

	// If we are playing a sound... Kill it!!

	if( m_hSound )
	{
		g_pLTServer->SoundMgr()->KillSound( m_hSound );
	}

	if( m_hAttachedModel )
	{
		// Remove the model...

		HATTACHMENT hAttachment;
		if( g_pLTServer->FindAttachment( m_hObject, m_hAttachedModel, &hAttachment ) == LT_OK )
		{
			g_pLTServer->RemoveAttachment( hAttachment );
		}

		g_pLTServer->RemoveObject( m_hAttachedModel );
		m_hAttachedModel = LTNULL;
	}

	if (m_hLight)
	{
		// Remove the light...

		HATTACHMENT hAttachment;
        if (g_pLTServer->FindAttachment(m_hObject, m_hLight, &hAttachment) == LT_OK)
		{
            g_pLTServer->RemoveAttachment(hAttachment);
		}

        g_pLTServer->RemoveObject(m_hLight);
        m_hLight = LTNULL;
	}
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GadgetTarget::EngineMessageFn
//
//  PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 GadgetTarget::EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData )
{
	switch( messageID )
	{
		case MID_PRECREATE :
		{
			uint32 dwRet = Prop::EngineMessageFn( messageID, pData, fData );

			if( fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP )
			{
				ObjectCreateStruct *pStruct = (ObjectCreateStruct*)pData;
				ReadProp( pStruct );
			}

			return dwRet;
		}
		break;

		case MID_OBJECTCREATED :
		{
			if( fData != OBJECTCREATED_SAVEGAME )
			{
				OnObjectCreated( );
			}
		}
		break;

		case MID_SAVEOBJECT :
		{
			OnSave( (ILTMessage_Write*)pData ); 
		}
		break;

		case MID_LOADOBJECT :
		{
			OnLoad( (ILTMessage_Read*)pData );
		}
		break;

		default : break;
	}

	return Prop::EngineMessageFn( messageID, pData, fData );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GadgetTarget::ReadProp
//
//  PURPOSE:	get the properties
//
// ----------------------------------------------------------------------- //

LTBOOL GadgetTarget::ReadProp( ObjectCreateStruct *pStruct )
{
	if( !pStruct ) return LTFALSE;

	GenericProp gProp;

	// Get the GadgetTarget record...

	GADGETTARGET	*pGadgetTarget = LTNULL;
	
	if( g_pLTServer->GetPropGeneric( "GadgetTargetName", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
		{
			pGadgetTarget = g_pGadgetTargetMgr->GetGadgetTarget( gProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "StartOn", &gProp ) == LT_OK )
	{
		// Do we start on

		m_bOn = gProp.m_Bool;
	}

	// Model

	if( !pGadgetTarget || !pGadgetTarget->szFileName[0] ) return LTFALSE;

	m_nGTID = pGadgetTarget->nId;

	SAFE_STRCPY( pStruct->m_Filename, pGadgetTarget->szFileName );

	// Skins...

	pGadgetTarget->blrGTSkinReader.CopyList(0, pStruct->m_SkinNames[0], MAX_CS_FILENAME_LEN+1);

	// Render Styles

	pGadgetTarget->blrGTRenderStyleReader.CopyList( 0, pStruct->m_RenderStyleNames[0], MAX_CS_FILENAME_LEN + 1 );

	m_vObjectColor	= pGadgetTarget->vObjectColor;
	m_fAlpha		= pGadgetTarget->fAlpha;
	m_bMoveToFloor	= pGadgetTarget->bMoveToFloor;
	m_vScale		= pGadgetTarget->vScale;

	m_dwFlags |= ( pGadgetTarget->bSolid ? FLAG_SOLID : 0 );
	m_dwFlags |= ( pGadgetTarget->bVisible ? FLAG_VISIBLE : 0 );
	m_dwFlags |= ( pGadgetTarget->bShadow ? FLAG_SHADOW : 0 );
	m_dwFlags |= FLAG_TOUCH_NOTIFY;

	if( pGadgetTarget->nHitPts >= 0 )
	{
		m_damage.SetMaxHitPoints( (LTFLOAT)pGadgetTarget->nHitPts );
		m_damage.SetHitPoints( (LTFLOAT)pGadgetTarget->nHitPts );
		
		// Gadget targets should not have any armor...
		
		m_damage.SetMaxArmorPoints( 0.0f );
		m_damage.SetArmorPoints( 0.0f );
	}

	// Set the debris type...

	if( pGadgetTarget->szDebrisType[0] )
	{
		DEBRIS *pDebris = g_pDebrisMgr->GetDebris( pGadgetTarget->szDebrisType );
		if( pDebris )
		{
			m_damage.m_nDebrisId = pDebris->nId;
		}
	}
	else
	{
		m_damage.m_nDebrisId = DEBRISMGR_INVALID_ID;
	}

	// Make sure the object is set up properly...

	pStruct->m_Flags = m_dwFlags;
	pStruct->m_Flags2 = m_dwFlags2;

	// Get the type of GadgetTarget

	if( IsValidGadgetTargetType( pGadgetTarget->nType ))
	{
		m_pGTInfo = &GTInfoArray[pGadgetTarget->nType];
		if( !m_pGTInfo ) return LTFALSE;
	}

	// Sounds...

	if( pGadgetTarget->szDisablingSnd[0] )
	{
		m_hstrDisablingSnd = g_pLTServer->CreateString( pGadgetTarget->szDisablingSnd );
	}

	if( pGadgetTarget->szDisabledSnd[0] )
	{
		m_hstrDisabledSnd = g_pLTServer->CreateString( pGadgetTarget->szDisabledSnd );
	}

	m_fSoundRadius	= pGadgetTarget->fSoundRadius;
	m_fMinTime		= pGadgetTarget->fMinTime;
	m_fMaxTime		= pGadgetTarget->fMaxTime;
	m_dwCodeID		= pGadgetTarget->dwCodeID;
	
	m_bRemoveWhenDisabled	= pGadgetTarget->bRemoveWhenDisabled;
	m_bInfiniteDisables		= pGadgetTarget->bInfiniteDisables;
	m_bInfiniteActivates	= pGadgetTarget->bInfiniteActivates;

	// Finish reading the rest of the props...

	if( g_pLTServer->GetPropGeneric( "MinTime", &gProp ) == LT_OK )
	{
		// Do we want to override the default value?

		if( gProp.m_Float > 0.0f )
		{
			m_fMinTime = gProp.m_Float;
		}
	}
	
	if( g_pLTServer->GetPropGeneric( "MaxTime", &gProp ) == LT_OK )
	{
		// Do we want to override the default value?

		if( gProp.m_Float > 0.0f )
		{
			m_fMaxTime = gProp.m_Float;
		}
	}

	if( g_pLTServer->GetPropGeneric( "Command", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
		{
			m_hstrDisabledCmd = g_pLTServer->CreateString( gProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "PowerOffCommand", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
		{
			m_hstrPowerOffCmd = g_pLTServer->CreateString( gProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "CodeID", &gProp ) == LT_OK )
	{
		// Do we want to override the default value?

		if( gProp.m_Long > 0 )
		{
			m_dwCodeID = (uint32)gProp.m_Long;
		}
	}

	// Get the team the GadgetTarget belongs to, but only when in a team game...
	
	if( IsTeamGameType() )
	{
		if( g_pLTServer->GetPropGeneric( "Team", &gProp ) == LT_OK )
		{
			if( gProp.m_String[0] )
			{
				char szTeam[32] = {0};
				for( int i = 0; i < MAX_TEAMS; ++i )
				{
					sprintf( szTeam, "Team%i", i );
					if( !_stricmp( gProp.m_String, szTeam ))
					{
						m_nTeamID = i;
					}
				}
			}
		}
	}
	else
	{
		m_nTeamID = INVALID_TEAM;
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GadgetTarget::OnObjectCreated
//
//  PURPOSE:	Handel some initialization after the object is created
//
// ----------------------------------------------------------------------- //

LTBOOL GadgetTarget::OnObjectCreated( )
{
	SetNextUpdate( UPDATE_NEVER );
	
	if( !m_pGTInfo ) return LTFALSE;

	DamageFlags nCanDamageFlags = 0;

	m_damage.SetCantDamageFlags( ~nCanDamageFlags );
	m_damage.SetCanDamage( LTFALSE );
	m_damage.SetNeverDestroy( LTTRUE );

	// See what affects us...

	if( m_pGTInfo->m_bCanShoot )
	{
		nCanDamageFlags |= DamageTypeToFlag( DT_BULLET ) | DamageTypeToFlag( DT_EXPLODE );
		m_damage.SetCanDamage( LTTRUE );
		m_damage.SetNeverDestroy( LTFALSE );
		m_damage.ClearCantDamageFlags( nCanDamageFlags );
	}

	if( m_pGTInfo->m_bCanLockPick )
	{
		m_dwGadgetUsrFlgs |= USRFLG_GADGET_LOCK_PICK;
	}

	if( m_pGTInfo->m_bCanWeld )
	{
		m_dwGadgetUsrFlgs |= USRFLG_GADGET_WELDER;
	}

	if( m_pGTInfo->m_bCanCodeBreak )
	{
		m_dwGadgetUsrFlgs |= USRFLG_GADGET_CODE_DECIPHERER;
	}

	if( m_pGTInfo->m_eTargetType == ePhotographable )
	{
		m_dwGadgetUsrFlgs |= USRFLG_GADGET_CAMERA;
	}
	else if( m_pGTInfo->m_eTargetType == eTelephone )
	{
		m_dwGadgetUsrFlgs |= USRFLG_GADGET_EAVESDROPBUG;
	}
	else if( m_pGTInfo->m_eTargetType == eBombable )
	{
		m_dwGadgetUsrFlgs |= (USRFLG_GADGET_BOMBABLE | USRFLG_GLOW);
	}
	else if( m_pGTInfo->m_eTargetType == eInvisibleInk )
	{
		m_dwGadgetUsrFlgs |= USRFLG_GADGET_INVISIBLE_INK;
	}

	m_dwUsrFlgs |= m_dwGadgetUsrFlgs;

	// Set the time it takes to disable us...

	if( m_fMaxTime < m_fMinTime )
	{
		m_fMaxTime = m_fMinTime;
	}

	// If it's a photographable target, the time should be instant

	m_fTotalTime = m_fDisableTime = (m_dwGadgetUsrFlgs & USRFLG_GADGET_CAMERA ? 0.0f : GetRandom( m_fMinTime, m_fMaxTime ));

	CreateSpecialFX( false );

	// See if we want to create the light...

	GADGETTARGET *pGadgetTarget = g_pGadgetTargetMgr->GetGadgetTarget( m_nGTID );
	if (pGadgetTarget && pGadgetTarget->nLightType != 0)
	{
		CreateLight();
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GadgetTarget::ObjectMessageFn
//
//  PURPOSE:	Handle messages from other objects
//
// ----------------------------------------------------------------------- //

uint32 GadgetTarget::ObjectMessageFn( HOBJECT hSender, ILTMessage_Read *pMsg )
{
	if( !g_pLTServer || !m_pGTInfo ) return 0;

	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();
	switch( messageID )
	{
		case MID_DAMAGE :
		{
			if( m_bDisabled ) break;

			uint32 dwRet = 0;

			if( m_pGTInfo->m_bCanShoot )
			{
				DamageStruct damage;
				damage.InitFromMessage( pMsg );

				dwRet = Prop::ObjectMessageFn( hSender, pMsg );

				if( m_damage.IsDead() )
				{
					SetupDisabledState( LTTRUE );
				}
			}

			return dwRet;

		}
		break;

		default : break;
	}

	return Prop::ObjectMessageFn( hSender, pMsg );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GadgetTarget::OnTrigger
//
//  PURPOSE:	Handle a trigger message...
//
// ----------------------------------------------------------------------- //

bool GadgetTarget::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Gadget(c_szGADGET);
	static CParsedMsg::CToken s_cTok_Stopped(c_szSTOPPED);
	static CParsedMsg::CToken s_cTok_Activate(c_szACTIVATE);
	static CParsedMsg::CToken s_cTok_CanActivate(c_szCANACTIVATE);
	static CParsedMsg::CToken s_cTok_On(c_szON);
	static CParsedMsg::CToken s_cTok_Off(c_szOFF);
	static CParsedMsg::CToken s_cTok_Disable(c_szDisable);
	static CParsedMsg::CToken s_cTok_Team( "TEAM" );
	
	if( cMsg.GetArg(0) == s_cTok_Gadget )
	{
		HandleGadgetMsg( cMsg, hSender );
	}
	else if( cMsg.GetArg(0) == s_cTok_Stopped )
	{
		HandleStoppedMsg( cMsg );
	}
	else if( cMsg.GetArg(0) == s_cTok_Activate )
	{
		HandleActivateMsg( hSender);
	}
	else if( cMsg.GetArg(0) == s_cTok_CanActivate )
	{
		HandleCanActivateMsg();
	}
	else if( cMsg.GetArg(0) == s_cTok_On )
	{
		TurnOn();
	}
	else if( cMsg.GetArg(0) == s_cTok_Off )
	{
		TurnOn(false);
	}
	else if( cMsg.GetArg(0) == s_cTok_Disable )
	{
		SetupDisabledState();
	}
	else if (cMsg.GetArg(0) == s_cTok_Team)
	{
		if( cMsg.GetArgCount( ) > 1 )
		{
			uint32 nTeamId = atoi( cMsg.GetArg( 1 ));
			if( nTeamId < MAX_TEAMS )
			{
				m_nTeamID = nTeamId;
			}
			else
			{
				m_nTeamID = INVALID_TEAM;
			}

			CreateSpecialFX( true );
			return true;
		}
	}
	else
		return Prop::OnTrigger(hSender, cMsg);

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GadgetTarget::HandleGadgetMsg
//
//  PURPOSE:	Handle a gadget message...
//
// ----------------------------------------------------------------------- //

void GadgetTarget::HandleGadgetMsg( const CParsedMsg &cMsg, HOBJECT hSender )
{
	if( cMsg.GetArgCount() < 2 ) return;

	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( atol(cMsg.GetArg(1)) );
	if( !pAmmo ) return;

	DamageType	eType = pAmmo->eInstDamageType;
	if( ( (m_dwGadgetUsrFlgs & USRFLG_GADGET_LOCK_PICK)			&& eType == DT_GADGET_LOCK_PICK)		||
		( (m_dwGadgetUsrFlgs & USRFLG_GADGET_WELDER)			&& eType == DT_GADGET_WELDER)			||
		( (m_dwGadgetUsrFlgs & USRFLG_GADGET_CODE_DECIPHERER)	&& eType == DT_GADGET_CODE_DECIPHERER)	||
		( (m_dwGadgetUsrFlgs & USRFLG_GADGET_CAMERA)			&& eType == DT_GADGET_CAMERA)			||
		( (m_dwGadgetUsrFlgs & USRFLG_GADGET_BOMBABLE)			&& eType == DT_GADGET_TIME_BOMB)		||
		( (m_dwGadgetUsrFlgs & USRFLG_GADGET_INVISIBLE_INK)		&& eType == DT_GADGET_INK_REAGENT)		||
		( (m_dwGadgetUsrFlgs & USRFLG_GADGET_EAVESDROPBUG)		&& eType == DT_GADGET_EAVESDROPBUG)	)
	{

		// If we're turned off, send our "power off" command...

		if (m_bOn)
		{
			SetupDisablingState( hSender );
		}
		else
		{
			if( m_hstrPowerOffCmd )
			{
				const char* szCmd = g_pLTServer->GetStringData( m_hstrPowerOffCmd );
				if( szCmd )
				{
					g_pCmdMgr->Process( szCmd, m_hObject, m_hObject );
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GadgetTarget::HandleStoppedMsg
//
//  PURPOSE:	Handle stopping the disabling
//
// ----------------------------------------------------------------------- //

void GadgetTarget::HandleStoppedMsg( const CParsedMsg &cMsg )
{
	if( cMsg.GetArgCount() < 2 ) return;

	LTFLOAT	fTime = (LTFLOAT)atof( cMsg.GetArg(1) );

	// Stop the disabling sound if one was playing
	
	if( m_hSound )
	{
		g_pLTServer->SoundMgr()->KillSound( m_hSound );
        m_hSound = LTNULL;
	}

	//when we load, we don't want our flags to be reset
	m_bRestoreFlagsOnLoad = LTFALSE;

	if( fTime > 0.0f )
	{
		// The client stopped disabling us

		if( !m_pGTInfo->m_bResetTime )
		{
			// Save how much time is left

			m_fDisableTime = fTime;	
		}

		// Clients can try to disable us again...

		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, m_dwGadgetUsrFlgs, m_dwGadgetUsrFlgs );
	}
	else
	{
		SetupDisabledState( );

		if( m_bInfiniteDisables )
		{
			// Reset the gadget target so it can be disabled again...
			
			m_bDisabled = LTFALSE;
			m_bDisableRequested = LTFALSE;
			m_fDisableTime = m_fTotalTime;	

			// Clients can try to disable us again...
			
			g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, m_dwGadgetUsrFlgs, m_dwGadgetUsrFlgs );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GadgetTarget::HandleActivateMsg
//
//  PURPOSE:	Handle an activation command
//
// ----------------------------------------------------------------------- //

void GadgetTarget::HandleActivateMsg( HOBJECT hSender )
{
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_User, dwFlags );

	// If we're turned off, send our "power off" command...
	
	if (!m_bOn)
	{
		if( m_hstrPowerOffCmd )
		{
			const char* szCmd = g_pLTServer->GetStringData( m_hstrPowerOffCmd );
			if( szCmd )
			{
				g_pCmdMgr->Process( szCmd, m_hObject, m_hObject );
			}
		}

		return;
	}


	if( dwFlags & USRFLG_CAN_ACTIVATE )
	{
		// Become disabled...

		SetupDisabledState();

		if( m_bInfiniteActivates )
		{
			// Reset the gadget target so it can be disabled again...
			
			m_bDisabled = LTFALSE;
			m_bDisableRequested = LTFALSE;
		}
		else
		{
			// We should no longer be activateble
			
			g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE );
		}
	}	
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GadgetTarget::HandleCanActivateMsg
//
//  PURPOSE:	Handle setting up the object to be activated
//
// ----------------------------------------------------------------------- //

void GadgetTarget::HandleCanActivateMsg( )
{
	if( !m_bDisabled )
	{
		// Set us up so we can now be activated but not disabled by gadgets...

		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, m_dwGadgetUsrFlgs | USRFLG_CAN_ACTIVATE );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GadgetTarget::TurnOn
//
//  PURPOSE:	Turn the object on / off
//
// ----------------------------------------------------------------------- //

void GadgetTarget::TurnOn(bool bOn)
{
	m_bOn = bOn;

	CreateSpecialFX( true );

	TurnLightOn(bOn);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GadgetTarget::CreateSpecialFX
//
//	PURPOSE:	Add client-side special fx
//
// ----------------------------------------------------------------------- //

void GadgetTarget::CreateSpecialFX( bool bUpdateClients )
{
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(SFX_GADGETTARGET_ID);
		cMsg.Writeuint8(m_pGTInfo->m_eTargetType);
		cMsg.Writebool(!!m_bOn);
		cMsg.Writebool(!!m_bOn);
		cMsg.Writeuint8( m_nTeamID );
		g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
	}

	if( bUpdateClients )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_GADGETTARGET_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(m_pGTInfo->m_eTargetType);
		cMsg.Writebool(!!m_bOn);
		cMsg.Writebool(!!m_bOn);
		cMsg.Writeuint8( m_nTeamID );
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GadgetTarget::SetUpDisablingState
//
//  PURPOSE:	Get the gadget target into a disabling state...
//
// ----------------------------------------------------------------------- //

void GadgetTarget::SetupDisablingState( HOBJECT hSender )
{
	if( !IsPlayer( hSender ) ) return;

	CPlayerObj *pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( hSender );
	if( !pPlayer ) return;
	
	// Don't let clients try to disable us if we already are being disabled...

	m_bRestoreFlagsOnLoad = LTTRUE;
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, 0, m_dwGadgetUsrFlgs );

	// Play the disabling sound...

	if( m_hstrDisablingSnd )
	{
		const char *szSound = g_pLTServer->GetStringData( m_hstrDisablingSnd );
		if( szSound )
		{
			LTVector vPos;
			g_pLTServer->GetObjectPos( m_hObject, &vPos );

			uint32 dwFlags = (PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);
			m_hSound = g_pServerSoundMgr->PlaySoundFromPos( vPos, szSound, m_fSoundRadius, SOUNDPRIORITY_MISC_LOW, dwFlags );

		}
	}

	// Send a message to the client letting it know how long it should take
	// and whether or not it should show the bar...

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_GADGETTARGET );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( m_pGTInfo->m_eTargetType );
	cMsg.Writefloat( m_fTotalTime );
	cMsg.Writefloat( m_fDisableTime );
	cMsg.Writeuint32( m_dwCodeID );
	g_pLTServer->SendToClient( cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED );

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GadgetTarget::SetupDisabledState
//
//  PURPOSE:	The gadget target is disabled so do the disabled stuff...
//
// ----------------------------------------------------------------------- //

void GadgetTarget::SetupDisabledState( LTBOOL bDestroyed  )
{
	// Check to see if we already recieved a disable request... 
	// Since we can't set the actual disabled flag untill after the command is processed
	// we need to guard against infinite loops of level design craziness like doorknobs messaging
	// each other to disable.

	if( m_bDisableRequested )
		return;

	m_bDisableRequested = LTTRUE;

	// Send the disabled command...
	
	if( m_hstrDisabledCmd )
	{
        const char* szCmd = g_pLTServer->GetStringData( m_hstrDisabledCmd );
		if( szCmd )
		{
			g_pCmdMgr->Process( szCmd, m_hObject, m_hObject );
		}
	}

	// GadgetTarget is disabled.  Make sure we set this after we process our
	// command incase a level designer decides to do something crazy like having
	// our command send us a CanActivate message...not that our wonderful level
	// designers would ever do something like that...huh JOHN?!?!

	m_bDisabled = LTTRUE;
	
	// We are no longer able to be disabled...
	m_bRestoreFlagsOnLoad = LTFALSE;
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, 0, m_dwGadgetUsrFlgs );

	// Play the disabled sound...

	if( m_hstrDisabledSnd )
	{
		const char *szSound = g_pLTServer->GetStringData( m_hstrDisabledSnd );
		if( szSound )
		{
			LTVector vPos;
			g_pLTServer->GetObjectPos( m_hObject, &vPos );

			uint32 dwFlags = (PLAYSOUND_GETHANDLE);
			m_hSound = g_pServerSoundMgr->PlaySoundFromPos( vPos, szSound, m_fSoundRadius, SOUNDPRIORITY_MISC_LOW, dwFlags );

		}
	}
	
	// We only want to create the debirs once...

	if( !bDestroyed )
	{
		DEBRIS* pDebris = g_pDebrisMgr->GetDebris( m_damage.m_nDebrisId );
		if( pDebris )
		{
			LTVector vPos;
			g_pLTServer->GetObjectPos( m_hObject, &vPos );

			CreatePropDebris(vPos, LTVector( 0, 1, 0 ), pDebris->nId);
		}
	}

	if( m_bRemoveWhenDisabled )
	{
		g_pLTServer->RemoveObject( m_hObject );
	}

	// Handle any special case disabling here...

	SpecialDisabledHandling( );
	
	// TODO:
	// Maybe do some disabled FX, play an animation...
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GadgetTarget::SpecialDisabledHandling
//
//  PURPOSE:	A place to do any special cases we may have for the different gadget targets...
//
// ----------------------------------------------------------------------- //

void GadgetTarget::SpecialDisabledHandling( )
{
	// Make sure we are disabled...

	if( !m_bDisabled ) return;


	// Special case for the eavesdrop target...

	if( m_dwGadgetUsrFlgs & USRFLG_GADGET_EAVESDROPBUG )
	{
		// Create the bug model, and attach it to the telephone...

		LTVector vPos;
		g_pLTServer->GetObjectPos( m_hObject, &vPos );

		LTRotation rRot;
		g_pLTServer->GetObjectRotation( m_hObject, &rRot );

		ObjectCreateStruct ocsBug;

		ocsBug.m_Pos = vPos;
		ocsBug.m_Rotation = rRot;

		WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon( EAVESDROPBUG_WEAPON_NAME );
		if( !pWeapon )
			return;

		SAFE_STRCPY( ocsBug.m_Filename, pWeapon->szHHModel );
		pWeapon->blrHHSkins.CopyList( 0, ocsBug.m_SkinNames[0], MAX_CS_FILENAME_LEN+1 ); 

		ocsBug.m_Scale = pWeapon->vHHScale;
		ocsBug.m_Flags = FLAG_VISIBLE;
		ocsBug.m_ObjectType = OT_MODEL;

		HCLASS hClass = g_pLTServer->GetClass( "BaseClass" );
		LPBASECLASS pModel = g_pLTServer->CreateObject( hClass, &ocsBug );
		if( !pModel || !pModel->m_hObject )
			return;

		m_hAttachedModel = pModel->m_hObject;
		::SetNextUpdate(m_hAttachedModel, UPDATE_NEVER);
		
		// Set the Models's animation...

		HMODELANIM hAni = g_pLTServer->GetAnimIndex( m_hAttachedModel, "World");
		if( hAni != INVALID_MODEL_ANIM )
		{
			g_pLTServer->SetModelLooping( m_hAttachedModel, LTTRUE );
			g_pLTServer->SetModelAnimation( m_hAttachedModel, hAni );
		}

		// Attach the model to the phone...

		LTVector vOffset( 0, 0, 0);
		LTRotation rOffset( 0, 0, 0, 1);

		HATTACHMENT hAttachment;
		if( LT_OK != g_pLTServer->CreateAttachment( m_hObject, m_hAttachedModel, "Bug", &vOffset, &rOffset, &hAttachment ))
		{
			g_pLTServer->RemoveObject( m_hAttachedModel );
			m_hAttachedModel = LTNULL;
		}
	}

	// See if we want to move the light...

	GADGETTARGET *pGadgetTarget = g_pGadgetTargetMgr->GetGadgetTarget( m_nGTID );
	if (pGadgetTarget && pGadgetTarget->nLightType != 0)
	{
		SetLightPosition(eLightPos2);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GadgetTarget::CreateLight()
//
//	PURPOSE:	Create the light on the gadget target
//
// ----------------------------------------------------------------------- //

void GadgetTarget::CreateLight()
{
	if ( GTMGR_INVALID_ID == m_nGTID ) return;

	GADGETTARGET *pGadgetTarget = g_pGadgetTargetMgr->GetGadgetTarget( m_nGTID );
	if (!pGadgetTarget || pGadgetTarget->nLightType == 0) return;

	// Create the light

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);
	g_pLTServer->GetObjectPos( m_hObject, &(theStruct.m_Pos) );

	// See if we should create a sprite or a model...

	if (pGadgetTarget->nLightType == 1)
	{
		theStruct.m_Flags2 = FLAG2_ADDITIVE;
		theStruct.m_Flags = FLAG_SPRITEBIAS;
		theStruct.m_ObjectType = OT_SPRITE;
	}
	else if (pGadgetTarget->nLightType == 2)
	{
		theStruct.m_Flags2 = FLAG2_FORCETRANSLUCENT;
		theStruct.m_ObjectType = OT_MODEL;
	}

	theStruct.m_Flags |= FLAG_VISIBLE;

	
	// Always start on light 1

	SAFE_STRCPY( theStruct.m_Filename, pGadgetTarget->szLight1FileName );
	pGadgetTarget->blrGTLight1SkinReader.CopyList(0, theStruct.m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
	pGadgetTarget->blrGTLightRenderStyleReader.CopyList( 0, theStruct.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN + 1 );

	m_eLightPos = eLightPos1;

    HCLASS hClass = g_pLTServer->GetClass("BaseClass");
    LPBASECLASS pObj = g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pObj) return;

	m_hLight = pObj->m_hObject;
	::SetNextUpdate(m_hLight, UPDATE_NEVER);

    g_pLTServer->ScaleObject(m_hLight, &pGadgetTarget->vLightScale);


	// Attach the light to the the object...

    LTVector vOffset(0, 0, 0);
    LTRotation rOffset;

	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, m_hLight, pGadgetTarget->szLight1SocketName,
											     &vOffset, &rOffset, &hAttachment);
    if (dRes != LT_OK)
	{
		g_pLTServer->CPrint("GadgetTarget::CreateLight() CreateAttachment() Failed!");
		g_pLTServer->RemoveObject(m_hLight);
        m_hLight = LTNULL;
		return;
	}

	TurnLightOn(m_bOn ? true : false);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GadgetTarget::SetLightPosition()
//
//	PURPOSE:	Set the light's position
//
// ----------------------------------------------------------------------- //

void GadgetTarget::SetLightPosition(LightPosition eLightPos)
{
	if (!m_hLight || m_eLightPos == eLightPos) return;

	if ( GTMGR_INVALID_ID == m_nGTID ) return;

	GADGETTARGET *pGadgetTarget = g_pGadgetTargetMgr->GetGadgetTarget( m_nGTID );
	if (!pGadgetTarget || pGadgetTarget->nLightType == 0) return;

	m_eLightPos = eLightPos;

	ObjectCreateStruct createstruct;
	createstruct.Clear();

	char* pSocket = LTNULL;

	switch (m_eLightPos)
	{
		case eLightPos1 :
		{
			pSocket = pGadgetTarget->szLight1SocketName;
			SAFE_STRCPY(createstruct.m_Filename, pGadgetTarget->szLight1FileName);
			pGadgetTarget->blrGTLight1SkinReader.CopyList(0, createstruct.m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
		}
		break;
		
		case eLightPos2 :
		default :
		{
			pSocket = pGadgetTarget->szLight2SocketName;
			SAFE_STRCPY(createstruct.m_Filename, pGadgetTarget->szLight2FileName);
			pGadgetTarget->blrGTLight2SkinReader.CopyList(0, createstruct.m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
		}
		break;
	}

	g_pCommonLT->SetObjectFilenames(m_hLight, &createstruct);


	// Make sure the light is using the correct socket...

	HATTACHMENT hAttachment;
    if (g_pLTServer->FindAttachment(m_hObject, m_hLight, &hAttachment) == LT_OK)
	{
		g_pLTServer->RemoveAttachment(hAttachment);
	}

    LTVector vOffset(0, 0, 0);
    LTRotation rOffset;

    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, m_hLight, pSocket,
											     &vOffset, &rOffset, &hAttachment);
    if (dRes != LT_OK)
	{
		g_pLTServer->CPrint("GadgetTarget::SetLightPosition() CreateAttachment() Failed!");
        g_pLTServer->RemoveObject(m_hLight);
        m_hLight = LTNULL;
		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GadgetTarget::TurnLightOn()
//
//	PURPOSE:	Turn the light on/off
//
// ----------------------------------------------------------------------- //

void GadgetTarget::TurnLightOn(bool bOn)
{
	if (!m_hLight) return;

	g_pCommonLT->SetObjectFlags(m_hLight, OFT_Flags, bOn ? FLAG_VISIBLE : 0, FLAG_VISIBLE);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GadgetTarget::OnSave
//
//  PURPOSE:	Save the GadgetTarget object...
//
// ----------------------------------------------------------------------- //

LTBOOL GadgetTarget::OnSave( ILTMessage_Write *pMsg )
{
	if( !pMsg || !m_pGTInfo ) return LTFALSE;

	SAVE_BYTE	( m_pGTInfo->m_eTargetType );
	SAVE_FLOAT	( m_fMinTime );
	SAVE_FLOAT	( m_fMaxTime );
	SAVE_FLOAT	( m_fDisableTime );
	SAVE_FLOAT	( m_fTotalTime );
	SAVE_FLOAT	( m_fSoundRadius );
	SAVE_HSTRING( m_hstrDisablingSnd );
	SAVE_HSTRING( m_hstrDisabledSnd );
	SAVE_HSTRING( m_hstrDisabledCmd );
	SAVE_HSTRING( m_hstrPowerOffCmd );
	SAVE_DWORD	( m_dwGadgetUsrFlgs );
	SAVE_BOOL	( m_bDisabled );
	SAVE_BOOL	( m_bDisableRequested );
	SAVE_DWORD	( m_dwCodeID );
	SAVE_BOOL	( m_bRemoveWhenDisabled );
	SAVE_BOOL	( m_bInfiniteDisables );
	SAVE_BOOL	( m_bInfiniteActivates );
	SAVE_BOOL	( m_bOn );
	SAVE_BOOL	( m_bRestoreFlagsOnLoad );
	SAVE_HOBJECT( m_hAttachedModel );
    SAVE_HOBJECT( m_hLight );
    SAVE_BYTE	((uint8)m_eLightPos);
	SAVE_DWORD	( m_nGTID );

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_User, dwFlags );

	SAVE_bool( !!(dwFlags & USRFLG_CAN_ACTIVATE) );

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GadgetTarget::OnLoad
//
//  PURPOSE:	Load the GadgetTarget object...
//
// ----------------------------------------------------------------------- //

LTBOOL GadgetTarget::OnLoad( ILTMessage_Read *pMsg )
{
	if( !pMsg ) return LTFALSE;

	GadgetTargetType	eType;

	LOAD_BYTE_CAST( eType, GadgetTargetType );
	LOAD_FLOAT	( m_fMinTime );
	LOAD_FLOAT	( m_fMaxTime );
	LOAD_FLOAT	( m_fDisableTime );
	LOAD_FLOAT	( m_fTotalTime );
	LOAD_FLOAT	( m_fSoundRadius );
	LOAD_HSTRING( m_hstrDisablingSnd );
	LOAD_HSTRING( m_hstrDisabledSnd );
	LOAD_HSTRING( m_hstrDisabledCmd );
	LOAD_HSTRING( m_hstrPowerOffCmd );
	LOAD_DWORD	( m_dwGadgetUsrFlgs );
	
	LOAD_BOOL	( m_bDisabled );
	LOAD_BOOL	( m_bDisableRequested );
	LOAD_DWORD	( m_dwCodeID );
	LOAD_BOOL	( m_bRemoveWhenDisabled );
	LOAD_BOOL	( m_bInfiniteDisables );
	LOAD_BOOL	( m_bInfiniteActivates );
	LOAD_BOOL	( m_bOn );
	LOAD_BOOL	( m_bRestoreFlagsOnLoad );
	LOAD_HOBJECT( m_hAttachedModel );

    LOAD_HOBJECT( m_hLight );
    LOAD_BYTE_CAST(m_eLightPos, LightPosition);
	LOAD_DWORD	( m_nGTID );

	bool bCanActivate;
	LOAD_bool( bCanActivate );

	// Set our flags so we can be disabled or activated...

	if( (!m_bDisabled || m_bInfiniteDisables) && m_bRestoreFlagsOnLoad )
	{
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, m_dwGadgetUsrFlgs, m_dwGadgetUsrFlgs );
	}

	if( bCanActivate )
	{
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE );
	}

	// Load the Gadget Target Info 

	m_pGTInfo = &GTInfoArray[eType];
	if( !m_pGTInfo ) return LTFALSE;
	
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GadgetTarget::OnLinkBroken
//
//	PURPOSE:	Handle attached object getting removed.
//
// ----------------------------------------------------------------------- //

void GadgetTarget::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if( pRef == &m_hAttachedModel )
	{
		HATTACHMENT hAttachment;
		if ( LT_OK == g_pLTServer->FindAttachment( m_hObject, hObj, &hAttachment) )
		{
			g_pLTServer->RemoveAttachment(hAttachment);
		}
	}
	else if( pRef == &m_hLight )
	{
		HATTACHMENT hAttachment;
		if ( LT_OK == g_pLTServer->FindAttachment( m_hObject, hObj, &hAttachment) )
		{
			g_pLTServer->RemoveAttachment(hAttachment);
		}
	}

	Prop::OnLinkBroken( pRef, hObj );
}

