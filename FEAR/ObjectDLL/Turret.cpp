// ----------------------------------------------------------------------- //
//
// MODULE  : Turret.cpp
//
// PURPOSE : Turrets create a weapon to be used by a player through activation...
//
// CREATED : 07/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "Stdafx.h"
#include "Turret.h"
#include "Arsenal.h"
#include "Weapon.h"
#include "PlayerObj.h"
#include "ObjectMsgs.h"
#include "PrefetchUtilities.h"
#include "DatabaseUtils.h"
#include "LTEulerAngles.h"

LINKFROM_MODULE( Turret )

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_TURRET CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	// JSC this has to be set to a value or else the parameter will be invalid
	#define CF_HIDDEN_TURRET 0

#endif

BEGIN_CLASS( Turret )
	ADD_STRINGPROP_FLAG( Filename, "", PF_HIDDEN | PF_MODEL, "This hidden property is needed in order to get the model visible within WorldEdit." )
	ADD_STRINGPROP_FLAG( TurretType, SELECTION_NONE, PF_STATICLIST | PF_DIMS | PF_LOCALDIMS, "Record within the game database to use as the template for this turret." )
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE( PF_GROUP(1), 0 )
	// Default normal turrets to not get damaged or destroyed...
	ADD_BOOLPROP_FLAG( CanDamage, false, PF_GROUP(1), "Toggles whether the object can be damaged.")
	ADD_BOOLPROP_FLAG( NeverDestroy, true, PF_GROUP(1), "Toggles whether the object can be destroyed.")
	ADD_COMMANDPROP_FLAG( ActivateCommand, "", PF_NOTIFYCHANGE, "Command sent the when the Turret is activated." )
	ADD_COMMANDPROP_FLAG( DeactivateCommand, "", PF_NOTIFYCHANGE, "Command sent the when Turret is deactivated." )
	ADD_BOOLPROP( MoveToFloor, true, "If true the object is moved to the floor when created in the game." )
	ADD_PREFETCH_RESOURCE_PROPS()
END_CLASS_FLAGS_PLUGIN_PREFETCH( Turret, GameBase, CF_HIDDEN_TURRET, TurretPlugin, DefaultPrefetch<Turret>, "Places a player controlled turret within the level."  )

extern bool ValidateMsgBool( ILTPreInterface *pInterface, ConParse &cpMsgParams );

// Register with the CommandMgr...

CMDMGR_BEGIN_REGISTER_CLASS( Turret )

	ADD_MESSAGE( ACTIVATE_TURRET, 1, NULL, MSG_HANDLER( Turret, HandleActivateMsg ), "ACTIVATE_TURRET", "Allows Players to activate a turret.  <b>NOTE:</b> This should only be called through game code.", "DO NOT USE" )
	ADD_MESSAGE( DEACTIVATE, 1, NULL, MSG_HANDLER( Turret, HandleDeactivateMsg ), "DEACTIVATE", "Deactivates a turret", "msg Turret01 DEACTIVATE" )
	ADD_MESSAGE( HIDDEN, 2,	ValidateMsgBool, MSG_HANDLER( Turret, HandleHiddenMsg ),	"HIDDEN <bool>", "Toggles whether the object is visible, solid, and rayhit.", "msg <ObjectName> (HIDDEN 1)<BR>msg <ObjectName> (HIDDEN 0)" )
	ADD_MESSAGE( REMOVE, 1,	NULL, MSG_HANDLER( Turret, HandleRemoveMsg ),	"REMOVE", "Removes the object from the game permanently.", "msg <ObjectName> REMOVE" )

CMDMGR_END_REGISTER_CLASS( Turret, GameBase )

//
// TurretPlugin class implementation...
//

LTRESULT TurretPlugin::PreHook_EditStringList( const char *szRezPath,
											   const char *szPropName,
											   char **aszStrings,
											   uint32 *pcStrings,
											   const uint32 cMaxStrings,
											   const uint32 cMaxStringLen )
{
	if( !aszStrings || !pcStrings || !g_pWeaponDB )
	{
		LTERROR( "Invalid input parameters" );
		return LT_UNSUPPORTED;
	}

	if( LTStrEquals( szPropName, "TurretType" ))
	{
		// Fill the first string in the list with a <none> selection...
		LTStrCpy( aszStrings[(*pcStrings)++], SELECTION_NONE, cMaxStringLen );

		// Add an entry for each turret type...

		uint8 nNumTurrets = g_pWeaponDB->GetNumTurrets( );
		for( uint8 nTurret = 0; nTurret < nNumTurrets; ++nTurret )
		{
			LTASSERT( cMaxStrings > (*pcStrings) + 1, "Too many turret to fit in the list.  Enlarge list size?" );

			HTURRET hTurret = g_pWeaponDB->GetTurretRecord( nTurret );
			if( !hTurret )
				continue;

			const char *pszTurretName = g_pWeaponDB->GetRecordName( hTurret );
			if( !pszTurretName )
				continue;

			if( (LTStrLen( pszTurretName ) < cMaxStringLen) && ((*pcStrings) + 1 < cMaxStrings) )
			{
				LTStrCpy( aszStrings[(*pcStrings)++], pszTurretName, cMaxStringLen );
			}

		}

		// Sort the list so turret types are easier to find...
		qsort( aszStrings, *pcStrings, sizeof(char *), CaseInsensitiveCompare );

		return LT_OK;
	}

	if( m_DestructibleModelPlugin.PreHook_EditStringList( szRezPath, szPropName, 
														  aszStrings, pcStrings, 
														  cMaxStrings, cMaxStringLen ) == LT_OK)
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

LTRESULT TurretPlugin::PreHook_Dims( const char* szRezPath,
									const char* szPropName, 
									 const char* szPropValue,
									 char* szModelFilenameBuf,
									 int nModelFilenameBufLen,
									 LTVector &vDims,
									 const char* pszObjName, 
									 ILTPreInterface *pInterface)
{
	if( !szModelFilenameBuf || nModelFilenameBufLen < 1 )
		return LT_UNSUPPORTED;

	szModelFilenameBuf[0] = '\0';

	HTURRET hTurret = g_pWeaponDB->GetTurretRecord( szPropValue );
	if( !hTurret )
		return LT_UNSUPPORTED;

	const char *pszBaseModel = g_pWeaponDB->GetString( hTurret, WDB_TURRET_sBaseModel );
	if( !pszBaseModel )
		return LT_UNSUPPORTED;

	LTStrCpy( szModelFilenameBuf, pszBaseModel, nModelFilenameBufLen );
	
	return LT_OK;
}

LTRESULT TurretPlugin::PreHook_PropChanged( const char *szObjName, 
											const char *szPropName,
											const int nPropType,
											const GenericProp &gpPropValue,
											ILTPreInterface *pInterface,
											const char *szModifiers )
{
	// Since we don't have any props that need notification, just pass it to the Destructible model...

	if( LT_OK == m_DestructibleModelPlugin.PreHook_PropChanged( szObjName, szPropName, nPropType, gpPropValue, pInterface, szModifiers ))
	{
		return LT_OK;
	}

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


//
// Turret class implementation...
//

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Turret::Turret
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

Turret::Turret( )
:	GameBase				( OT_MODEL ),
	m_hTurret				( NULL ),
	m_hOperatingObject		( NULL ),
	m_sActivateCommand		( ),
	m_sDeactivateCommand	( ),
	m_bMoveToFloor			( true ),
	m_nCurDamageState		( 0 ),
	m_bPostLoadActivate		( false )
{
	AddAggregate( &m_Arsenal );
	AddAggregate( &m_Damage );

	m_hOperatingObject.SetReceiver( *this );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Turret::~Turret
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

Turret::~Turret( )
{
	Deactivate( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::EngineMessageFn
//
//	PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //

uint32 Turret::EngineMessageFn( uint32 messageID, void *pData, float fData )
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if( fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP )
			{
				ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
				if( !ReadProp( &pStruct->m_cProperties ))
					return 0;
				PostReadProp( pStruct );
			}
		}
		break;

		case MID_OBJECTCREATED:
		{
			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			if (fData != OBJECTCREATED_SAVEGAME)
			{
				InitialUpdate();
			}

			return dwRet;
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save( (ILTMessage_Write*)pData, (uint32)fData );
		}
		break;

		case MID_LOADOBJECT:
		{
			Load( (ILTMessage_Read*)pData, (uint32)fData );
		}
		break;

		case MID_UPDATE:
		{
			Update( );

			if( m_bPostLoadActivate )
			{
				m_bPostLoadActivate = false;
				HOBJECT hOperatingObject = m_hOperatingObject;
				m_hOperatingObject = INVALID_HOBJECT;

				PostLoadActivate( hOperatingObject );
			}
		}
		break;

		case MID_SAVESPECIALEFFECTMESSAGE:
		{
			SaveSFXMessage( static_cast<ILTMessage_Write*>( pData ), static_cast<uint32>( fData ) );
		}
		break;

		case MID_LOADSPECIALEFFECTMESSAGE:
		{
			LoadSFXMessage( static_cast<ILTMessage_Read*>( pData ), static_cast<uint32>( fData ) );
		}
		break;

		default : 
		break;
	}

	return GameBase::EngineMessageFn( messageID, pData, fData );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::ObjectMessageFn
//
//	PURPOSE:	Handle messages from other objects...
//
// ----------------------------------------------------------------------- //

uint32 Turret::ObjectMessageFn( HOBJECT hSender, ILTMessage_Read *pMsg )
{
	// Reset message to begining...
	pMsg->SeekTo( 0 );

	uint32 dwMsgId = pMsg->Readuint32( );
	switch( dwMsgId )
	{
		case MID_DAMAGE	:
		{
			uint32 dwMsgPos = pMsg->Tell( );

			// Process base first so the dead flag gets set...
			uint32 dwRet = GameBase::ObjectMessageFn( hSender, pMsg );

			if( m_Damage.IsDead( ))
			{
				OnDeath( );
			}
			else
			{
				// Determine the percentage of health left on the turret...
				// This will be used to create certain ClientFX based on damage states...
				float fHealthPercent = 0.0f;
				if( m_Damage.GetMaxHitPoints( ) > 0.0f )
				{
					fHealthPercent = m_Damage.GetHitPoints( ) / m_Damage.GetMaxHitPoints( );
				}

				HATTRIBUTE hDamageStateStruct = g_pWeaponDB->GetAttribute( m_hTurret, WDB_TURRET_DamageState );
				uint32 nNumDamageStates = g_pWeaponDB->GetNumValues( hDamageStateStruct );

				uint32 nDamageState;
				for( nDamageState = 0; nDamageState < nNumDamageStates; ++nDamageState )
				{
					// Run through the damage states and determine which ones to display...
					HATTRIBUTE hAttrib = g_pWeaponDB->GetStructAttribute( hDamageStateStruct, nDamageState, WDB_TURRET_fHealthPercent );
					float fDamageStatePercent = g_pWeaponDB->GetFloat( hAttrib );

					if( nDamageState > m_nCurDamageState &&
						fDamageStatePercent >= fHealthPercent )
					{
						m_nCurDamageState = nDamageState;
					}
				}

				// Send damage msg to operating client...
				CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_hOperatingObject ));
				if( pPlayer )
				{
					CAutoMessage cMsg;
					cMsg.Writeuint8( MID_SFX_MESSAGE );
					cMsg.Writeuint8( SFX_TURRET_ID );
					cMsg.WriteObject( m_hObject );
					cMsg.Writeuint8( kTurretFXMsg_Damage );
					cMsg.Writeuint32( m_nCurDamageState );
					g_pLTServer->SendToClient( cMsg.Read( ), pPlayer->GetClient( ), MESSAGE_GUARANTEED );
				}
			}

			return dwRet;
		}
		break;
	}

	return GameBase::ObjectMessageFn( hSender, pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::OnDeath
//
//	PURPOSE:	Handle any cleanup required when the turret gets destroyed...
//
// ----------------------------------------------------------------------- //

void Turret::OnDeath( )
{
	if( !m_swtDestroyedDeactivationDelay.IsStarted( ))
	{
		uint32 dwDelay = g_pWeaponDB->GetInt32( m_hTurret, WDB_TURRET_tmDestroyedDeactivationDelay );
		m_swtDestroyedDeactivationDelay.Start( dwDelay * 0.001f );	
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::ReadProp
//
//	PURPOSE:	Read in the properties of the object... 
//
// ----------------------------------------------------------------------- //

bool Turret::ReadProp( const GenericPropList *pProps )
{
	const char *pszTurretType = pProps->GetString( "TurretType", "" );
	m_hTurret = g_pWeaponDB->GetTurretRecord( pszTurretType );
	if( !m_hTurret )
	{
		LTERROR( "Invalid turret type." );
		return false;
	}

	m_sActivateCommand		= pProps->GetCommand( "ActivateCommand", "" );
	m_sDeactivateCommand	= pProps->GetCommand( "DeactivateCommand", "" );

	m_bMoveToFloor = pProps->GetBool( "MoveToFloor", m_bMoveToFloor );
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::PostReadProp
//
//	PURPOSE:	Configure the ObjectCreateStruct for creating the object
//
// ----------------------------------------------------------------------- //

void Turret::PostReadProp( ObjectCreateStruct *pStruct )
{
	const char *pszBaseModel = g_pWeaponDB->GetString( m_hTurret, WDB_TURRET_sBaseModel );
	if( pszBaseModel )
		LTStrCpy( pStruct->m_Filename, pszBaseModel, LTARRAYSIZE(pStruct->m_Filename) );

	// Make sure to grab all the material files...
	g_pWeaponDB->CopyStringValues( m_hTurret, WDB_TURRET_sBaseMaterial, pStruct->m_Materials[0],
		LTARRAYSIZE(pStruct->m_Materials), LTARRAYSIZE(pStruct->m_Materials[0]) );

	pStruct->m_ObjectType = OT_MODEL;

	if( g_pWeaponDB->GetBool( m_hTurret, WDB_TURRET_bHideBase ))
	{
		// Make the turret base invisible and non-solid but allow raycasts so 
		// the turret may still be damaged...
		pStruct->m_Flags |= FLAG_FORCECLIENTUPDATE | FLAG_RAYHIT;
	}
	else
	{
		pStruct->m_Flags |= FLAG_VISIBLE | FLAG_SOLID | FLAG_RAYHIT;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::InitialUpdate
//
//	PURPOSE:	Handle a MID_INITIALUPDATE message from the engine....
//
// ----------------------------------------------------------------------- //

void Turret::InitialUpdate( )
{
	if( !CreateWeapon( ))
		return;

	// Set the base model diminsions...

	LTVector vDims;
	HMODELANIM hAnimBase = INVALID_MODEL_ANIM;
	g_pModelLT->GetCurAnim( m_hObject, MAIN_TRACKER, hAnimBase );
	g_pModelLT->GetModelAnimUserDims (m_hObject, hAnimBase, &vDims);
	g_pPhysicsLT->SetObjectDims( m_hObject, &vDims, 0 );

	if( g_pWeaponDB->GetBool( m_hTurret, WDB_TURRET_bHideBase ))
		g_pLTServer->SetObjectShadowLOD( m_hObject, eEngineLOD_Never );

	// Make sure object starts on floor if the flag is set...
	if( m_bMoveToFloor )
	{
		MoveObjectToFloor( m_hObject );
	}

	// Do not remove the turret on death since the deactivation of the turret will be delayed...
	// The turret will be removed after the delay...
	m_Damage.m_DestructibleModelFlags = m_Damage.m_DestructibleModelFlags & ~CDestructibleModel::kDestructibleModelFlag_RemoveOnDeath;
	m_swtDestroyedDeactivationDelay.SetEngineTimer( SimulationTimer::Instance( ));

	CreateSpecialFX( false );

	SetNextUpdate( UPDATE_NEXT_FRAME );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::Update
//
//	PURPOSE:	Handle a MID_UPDATE message from the engine....
//
// ----------------------------------------------------------------------- //

void Turret::Update( )
{
	if( m_hOperatingObject && IsPlayer( m_hOperatingObject ))
	{
		if( m_swtDestroyedDeactivationDelay.IsStarted( ) && m_swtDestroyedDeactivationDelay.IsTimedOut( ))
		{
			Deactivate( );

			// Remove the turret weapon since the turret is getting removed as well...
			m_Arsenal.RemoveAllActiveWeapons( );

			// Remove ourselves now that we're dead...
			g_pLTServer->RemoveObject( m_hObject );

			return;
		}
		
		CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_hOperatingObject ));
		if( !pPlayer || !pPlayer->IsAlive( ))
		{
			Deactivate( );
			return;
		}
		
		CWeapon *pPlayerWeapon = pPlayer->GetArsenal( )->GetCurWeapon( );
		if( pPlayerWeapon )
			pPlayerWeapon->HideWeapon( true );

		LTRigidTransform tView;
		pPlayer->GetTrueViewTransform( tView );

		CWeapon *pTurretWeapon = m_Arsenal.GetCurWeapon( );
		if( pTurretWeapon )
		{
			HMODELSOCKET hPivot = INVALID_MODEL_SOCKET;
			g_pModelLT->GetSocket( m_hObject, "Pivot", hPivot );

			LTTransform tPivot;
			g_pModelLT->GetSocketTransform( m_hObject, hPivot, tPivot, true );

			HOBJECT hWeapon = pTurretWeapon->GetModelObject( );

			LTRigidTransform rtCur;
			g_pLTServer->GetObjectTransform( hWeapon, &rtCur );

			// Update the position if it's changed.
			if( !rtCur.m_vPos.NearlyEquals( tPivot.m_vPos, 0.0001f ) || !rtCur.m_rRot.NearlyEquals( tView.m_rRot, 0.00001f ))
			{
				g_pLTServer->SetObjectTransform( hWeapon, LTRigidTransform(tPivot.m_vPos, tView.m_rRot) );
			}
		}

	}
	else
	{
		CWeapon *pTurretWeapon = m_Arsenal.GetCurWeapon( );
		if( pTurretWeapon )
		{
			HMODELSOCKET hPivot = INVALID_MODEL_SOCKET;
			g_pModelLT->GetSocket( m_hObject, "Pivot", hPivot );

			LTTransform tPivot;
			g_pModelLT->GetSocketTransform( m_hObject, hPivot, tPivot, true );

			HOBJECT hWeapon = pTurretWeapon->GetModelObject( );

			LTRigidTransform rtCur;
			g_pLTServer->GetObjectTransform( hWeapon, &rtCur );

			// Update the position if it's changed.
			if( !rtCur.m_vPos.NearlyEquals( tPivot.m_vPos, 0.0001f ) || !rtCur.m_rRot.NearlyEquals( tPivot.m_rRot, 0.00001f ))
			{
				g_pLTServer->SetObjectTransform( hWeapon, tPivot );
			}
		}
	}

	SetNextUpdate( UPDATE_NEXT_FRAME );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::OnLinkBroken
//
//	PURPOSE:	Create the actual weapon used for the turret...
//
// ----------------------------------------------------------------------- //

void Turret::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if( &m_hOperatingObject == pRef )
	{
		Deactivate( );
	}

	GameBase::OnLinkBroken( pRef, hObj );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::CreateWeapon
//
//	PURPOSE:	Create the actual weapon used for the turret...
//
// ----------------------------------------------------------------------- //

bool Turret::CreateWeapon( )
{
	HWEAPON hWeapon = g_pWeaponDB->GetRecordLink( m_hTurret, WDB_TURRET_rWeapon );
	if( !hWeapon )
		return false;

	const char *pszWeapon = g_pWeaponDB->GetRecordName( hWeapon );

	// Create a weapon at the pivot point...
	CActiveWeapon* pActiveWeapon = m_Arsenal.ActivateWeapon( pszWeapon, "Pivot" );
	if( pActiveWeapon == NULL )
		return false;

	// Make it our current weapon...
	if( !m_Arsenal.ChangeWeapon( pActiveWeapon ))
		return false;

	CWeapon *pWeapon = m_Arsenal.GetCurWeapon( );
	if( !pWeapon )
		return false;
	
	// Give us plenty of ammo..
	m_Arsenal.AddAmmo( pWeapon->GetAmmoRecord(), 10000 );

	HATTACHMENT hAttachment = NULL;
	g_pLTServer->FindAttachment( m_hObject, pWeapon->GetModelObject( ), &hAttachment );

    if( hAttachment )
	{
		// Undo the attachment so the position and rotation can be manually set...
		g_pLTServer->RemoveAttachment( hAttachment );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::CreateSpecialFX
//
//	PURPOSE:	Send relevant information to clients...
//
// ----------------------------------------------------------------------- //

void Turret::CreateSpecialFX( bool bUpdateClients )
{
	CWeapon *pWeapon = m_Arsenal.GetCurWeapon( );

	TURRETCREATESTRUCT csTurret;
	csTurret.m_hTurret = m_hTurret;
	csTurret.m_hOperatingObject = m_hOperatingObject;
	csTurret.m_hTurretWeapon = (pWeapon ? pWeapon->GetModelObject( ) : NULL );
	csTurret.m_bRemoteActivation = false;
	csTurret.m_nDamageState = m_nCurDamageState;	
	
	// Give derived classes a chance at updating the client data...
	PreCreateSpecialFX( csTurret );

	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( SFX_TURRET_ID );
		csTurret.Write( cMsg );
		g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read( ));
	}
	

	if( bUpdateClients )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_TURRET_ID );
		cMsg.WriteObject( m_hObject );
		cMsg.Writeuint8( kTurretFXMsg_All );
		csTurret.Write( cMsg );
		g_pLTServer->SendToClient( cMsg.Read( ), NULL, MESSAGE_GUARANTEED );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::HandleActivateMsg
//
//	PURPOSE:	Handles an ACTIVATE message...
//
// ----------------------------------------------------------------------- //

void Turret::HandleActivateMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( IsInUse( ) )
	{	
		if( m_hOperatingObject == hSender )
		{
			Deactivate( );
		}
		else
		{
			// Can't activate an occupied turret...
			return;
		}
	}
	else
	{
		Activate( hSender );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::HandleDeactivateMsg
//
//	PURPOSE:	Handles an DEACTIVATE message...
//
// ----------------------------------------------------------------------- //

void Turret::HandleDeactivateMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( IsInUse( ))
	{
		Deactivate( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::HandleHiddenMsg
//
//	PURPOSE:	Handles an HIDDEN message...
//
// ----------------------------------------------------------------------- //

void Turret::HandleHiddenMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	static CParsedMsg::CToken s_cTok_1("1");
	static CParsedMsg::CToken s_cTok_True("TRUE");
	static CParsedMsg::CToken s_cTok_0("0");
	static CParsedMsg::CToken s_cTok_False("FALSE");

	// Hide the base model as usual...
	GameBase::HandleHiddenMsg( hSender, crParsedMsg );

	// Now control the hidden state of the weapon model...
	if( (crParsedMsg.GetArg(1) == s_cTok_1) ||
		(crParsedMsg.GetArg(1) == s_cTok_True) )
	{
		m_Arsenal.HideWeapons( true );
	}
	else if( (crParsedMsg.GetArg(1) == s_cTok_0) ||
			 (crParsedMsg.GetArg(1) == s_cTok_False) )
	{
		m_Arsenal.HideWeapons( false );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::HandleRemoveMsg
//
//	PURPOSE:	Handles an REMOVE message...
//
// ----------------------------------------------------------------------- //

void Turret::HandleRemoveMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Make sure the player is off the turret before removing...
	Deactivate( );

	// Remove the turret weapon since the turret is getting removed as well...
	m_Arsenal.RemoveAllActiveWeapons( );

	GameBase::HandleRemoveMsg( hSender, crParsedMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::Activate
//
//	PURPOSE:	Activate the turret...
//
// ----------------------------------------------------------------------- //

void Turret::Activate( HOBJECT hSender )
{
	if( IsInUse( ) || m_Damage.IsDead( ))
		return;

	if( m_swtDestroyedDeactivationDelay.IsStarted( ))
	{
		g_pLTServer->CPrint( "Activated with deactivation delay" );
	}

	// Activating turret...
	m_hOperatingObject = hSender;

	CreateSpecialFX( true );

	if( IsPlayer( m_hOperatingObject ))
	{
		// Change the players weapon to the turret weapon...
		CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_hOperatingObject ));
		pPlayer->SetOperatingTurret( *this, true );
	}

	// Process any activation command we may have...
	if( !m_sActivateCommand.empty( ))
	{
		g_pCmdMgr->QueueCommand( m_sActivateCommand.c_str( ), hSender, m_hObject );
	}

	SetNextUpdate( UPDATE_NEXT_FRAME );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::Deactivate
//
//	PURPOSE:	Deactivate the turret...
//
// ----------------------------------------------------------------------- //

void Turret::Deactivate( )
{
	if( !IsInUse( ))
	{
		DebugCPrint(1, "Deactivation failed!");
		return;
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_TURRET_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( kTurretFXMsg_Deactivate );
	g_pLTServer->SendToClient( cMsg.Read( ), NULL, MESSAGE_GUARANTEED );

	if( IsPlayer( m_hOperatingObject ))
	{
		// Change the players weapon to the turret weapon...
		CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_hOperatingObject ));
		pPlayer->SetOperatingTurret( *this, false );
	}

	// Process any deactivation command we may have...
	if( !m_sDeactivateCommand.empty( ))
	{
		g_pCmdMgr->QueueCommand( m_sDeactivateCommand.c_str( ), m_hOperatingObject, m_hObject );
	}

	m_hOperatingObject = NULL;

	CreateSpecialFX( false );

	// [RP] NOTE: We still need to continually update to manually keep accurate positions.
	//		Using attachments was causing issues of not being able to activate.  Need to look
	//		further into this.  Once this issue is resolved we should not need to update if 
	//		the turret is deactivates.
	SetNextUpdate( /*UPDATE_NEVER*/ UPDATE_NEXT_FRAME );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::Save
//
//	PURPOSE:	Save the object...
//
// ----------------------------------------------------------------------- //

void Turret::Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg )
		return;

	m_swtDestroyedDeactivationDelay.Save( *pMsg );

	SAVE_HRECORD( m_hTurret );
	SAVE_HOBJECT( m_hOperatingObject );
	SAVE_STDSTRING( m_sActivateCommand );
	SAVE_STDSTRING( m_sDeactivateCommand );
	SAVE_DWORD( m_nCurDamageState );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::Load
//
//	PURPOSE:	Load the object...
//
// ----------------------------------------------------------------------- //

void Turret::Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags )
{
	if( !pMsg )
		return;

	m_swtDestroyedDeactivationDelay.Load( *pMsg );

	LOAD_HRECORD( m_hTurret, g_pWeaponDB->GetTurretsCategory( ));
	LOAD_HOBJECT( m_hOperatingObject );
	LOAD_STDSTRING( m_sActivateCommand );
	LOAD_STDSTRING( m_sDeactivateCommand );
	LOAD_DWORD( m_nCurDamageState );
	
	if( m_hOperatingObject )
		m_bPostLoadActivate = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::SaveSFXMessage
//
//	PURPOSE:	Save the object special effect message
//
// ----------------------------------------------------------------------- //

void Turret::SaveSFXMessage( ILTMessage_Write *pMsg, uint32 dwFlags )
{
	if( !pMsg )
		return;

	CAutoMessage cGetMsg;
	g_pLTServer->GetObjectSFXMessage( m_hObject, cGetMsg );
	CLTMsgRef_Read pSFXMsg = cGetMsg.Read( );

	if( pSFXMsg->Size( ) == 0 )
		return;

	pMsg->Writeuint8( pSFXMsg->Readuint8( ) );

	TURRETCREATESTRUCT TurretCS;
	TurretCS.Read( pSFXMsg );
	TurretCS.Write( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::LoadSFXMessage
//
//	PURPOSE:	Load the object special effect message.
//
// ----------------------------------------------------------------------- //

void Turret::LoadSFXMessage( ILTMessage_Read *pMsg, uint32 dwFlags )
{
	if( !pMsg )
		return;

	CAutoMessage cSFXMsg;

	cSFXMsg.Writeuint8( pMsg->Readuint8( ) );

	TURRETCREATESTRUCT TurretCS;
	TurretCS.Read( pMsg );
	TurretCS.Write( cSFXMsg );
	g_pLTServer->SetObjectSFXMessage( m_hObject, cSFXMsg.Read( ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::PostLoadActivate
//
//	PURPOSE:	Handle reactivating after loading a saved game...
//
// ----------------------------------------------------------------------- //

void Turret::PostLoadActivate( HOBJECT hOperatingObject )
{
	if( hOperatingObject )
	{
		// Activating turret...
		m_hOperatingObject = hOperatingObject;
		CreateSpecialFX( true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Turret::GetPrefetchResourceList
//
//	PURPOSE:	Determines the list of all needed resources
//
// ----------------------------------------------------------------------- //

void Turret::GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources )
{
	// get the turret record
	char szTurretType[MAX_PATH];
	pInterface->GetPropString(pszObjectName, "TurretType", szTurretType, LTARRAYSIZE(szTurretType), NULL);
	
	if (!LTStrEmpty(szTurretType))
	{
		HRECORD hTurretRecord = g_pWeaponDB->GetTurretRecord(szTurretType);

		// get the base model and material
		HATTRIBUTE hBaseModelAttribute = g_pLTDatabase->GetAttribute(hTurretRecord, WDB_TURRET_sBaseModel);
		const char* pszBaseModel = g_pLTDatabase->GetString(hBaseModelAttribute, 0, NULL);
		if (pszBaseModel)
		{
			Resources.push_back(pszBaseModel);
		}

		HATTRIBUTE hBaseMaterialAttribute = g_pLTDatabase->GetAttribute(hTurretRecord, WDB_TURRET_sBaseMaterial);
		const char* pszBaseMaterial = g_pLTDatabase->GetString(hBaseMaterialAttribute, 0, NULL);
		if (pszBaseMaterial)
		{
			Resources.push_back(pszBaseMaterial);
		}

		// get the client FX resources
		GetClientFXResources(Resources, hTurretRecord, WDB_TURRET_sLoopFX);
		GetClientFXResources(Resources, hTurretRecord, WDB_TURRET_sDamageFX);

		HATTRIBUTE hDamageStateStruct = g_pLTDatabase->GetAttribute(hTurretRecord, WDB_TURRET_DamageState);
		uint32 nNumberOfDamageStates = g_pLTDatabase->GetNumValues(hDamageStateStruct);

		for (uint32 nDamageStateIndex = 0; nDamageStateIndex < nNumberOfDamageStates; ++nDamageStateIndex)
		{
			HATTRIBUTE hClientFX = CGameDatabaseReader::GetStructAttribute(hDamageStateStruct, nDamageStateIndex, WDB_TURRET_fxClientFX);
			const char* pszClientFX = g_pLTDatabase->GetString(hClientFX, 0, NULL);
			GetClientFXResources(Resources, pszClientFX);
		}

		// get the weapon record
		HATTRIBUTE hTurretWeaponLink = g_pLTDatabase->GetAttribute(hTurretRecord, WDB_TURRET_rWeapon);
		HRECORD hTurretWeapon = g_pLTDatabase->GetRecordLink(hTurretWeaponLink, 0, NULL);
		if (hTurretWeapon)
		{
			// get the default attribute
			HATTRIBUTE hDefaultAttribute = g_pLTDatabase->GetAttribute(hTurretWeapon, "Default");
            HRECORD hDefaultRecord = g_pLTDatabase->GetRecordLink(hDefaultAttribute, 0, NULL);

			// get everything under here
			GetRecordResources(Resources, hDefaultRecord, true);
		}
	}
}

// EOF
