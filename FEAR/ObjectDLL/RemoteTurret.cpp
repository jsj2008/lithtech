// ----------------------------------------------------------------------- //
//
// MODULE  : RemoteTurret.cpp
//
// PURPOSE : RemoteTurret create a weapon to be used by a player through messages...
//
// CREATED : 07/26/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "Stdafx.h"
#include "RemoteTurret.h"
#include "PlayerObj.h"

LINKFROM_MODULE( RemoteTurret )

// Hide this object in Dark.
#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_REMOTETURRET CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	// JSC this has to be set to a value or else the parameter will be invalid
	#define CF_HIDDEN_REMOTETURRET 0

#endif

BEGIN_CLASS( RemoteTurret )
	
	// Default remote turrets to get damaged and destroyed...
	ADD_BOOLPROP_FLAG( CanDamage, true, PF_GROUP(1), "Toggles whether the object can be damaged.")
	ADD_BOOLPROP_FLAG( NeverDestroy, false, PF_GROUP(1), "Toggles whether the object can be destroyed.")

	ADD_STRINGPROP_FLAG( NextRemoteTurret, "", PF_OBJECTLINK, "Specifies the name of another RemoteTurret object that will be activated once the current RemoteTurret is deactivated.  This is used to cycle through several RemoteTurret objects.  If there is no object listed or the object is not a RemoteTurret the curretn turret will deactivated without going to another RemoteTurret object." )
	ADD_BOOLPROP_FLAG( MoveToFloor, false, PF_HIDDEN, "If true the object is moved to the floor when created in the game." )

	ADD_PREFETCH_RESOURCE_PROPS()
END_CLASS_FLAGS_PLUGIN_PREFETCH( RemoteTurret, Turret, CF_HIDDEN_REMOTETURRET, TurretPlugin, DefaultPrefetch<Turret>, "Places a player controlled remote turret within the level." )


// Register with the CommandMgr...

CMDMGR_BEGIN_REGISTER_CLASS( RemoteTurret )

	ADD_MESSAGE( ON, 1, NULL, MSG_HANDLER( RemoteTurret, HandleOnMsg ), "ON", "Turns the turret on and sets the active player as the operator of the turret.", "msg RemoteTurret ON" )

CMDMGR_END_REGISTER_CLASS( RemoteTurret, Turret )

//
// RemoteTurret class implementation...
//

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RemoteTurret::RemoteTurret
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

RemoteTurret::RemoteTurret( )
:	Turret					( ),
	m_sNextRemoteTurretName	( ),
	m_hNextRemoteTurret		( NULL ),
	m_hPrevRemoteTurret		( NULL )	
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RemoteTurret::~RemoteTurret
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

RemoteTurret::~RemoteTurret( )
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RemoteTurret::EngineMessageFn
//
//	PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //

uint32 RemoteTurret::EngineMessageFn( uint32 messageID, void *pData, float fData )
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			uint32 dwRet = Turret::EngineMessageFn( messageID, pData, fData );

			if( fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP )
			{
				if( !ReadProp( &((ObjectCreateStruct*)pData)->m_cProperties ) )
					return 0;
			}

			return dwRet;
		}
		break;

		case MID_ALLOBJECTSCREATED:
		{
			LinkRemoteTurrets( );
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
		}
		break;

		default : 
		break;
	}

	return Turret::EngineMessageFn( messageID, pData, fData );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RemoteTurret::ReadProp
//
//	PURPOSE:	Read in the properties of the object... 
//
// ----------------------------------------------------------------------- //

bool RemoteTurret::ReadProp( const GenericPropList *pProps )
{
	m_sNextRemoteTurretName = pProps->GetString( "NextRemoteTurret", "" );

	m_bMoveToFloor = false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RemoteTurret::Update
//
//	PURPOSE:	Handle a MID_UPDATE message from the engine....
//
// ----------------------------------------------------------------------- //

void RemoteTurret::Update( )
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RemoteTurret::PreCreateSpecialFX
//
//	PURPOSE:	Update the client data before sending...
//
// ----------------------------------------------------------------------- //

void RemoteTurret::PreCreateSpecialFX( TURRETCREATESTRUCT &rTurretCS )
{
	// Don't allow clients to activate remote turrets...
	rTurretCS.m_bRemoteActivation = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RemoteTurret::HandleOnMsg
//
//	PURPOSE:	Handle an ON message....
//
// ----------------------------------------------------------------------- //

void RemoteTurret::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( !IsInUse( ))
	{
		// Use the originating player of the message for activation...
		HOBJECT hPlayer = g_pGameServerShell->GetActivePlayer( );
		RemoteActivate( hPlayer );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RemoteTurret::HandleOnMsg
//
//	PURPOSE:	Handle any cleanup required when the turret gets destroyed...
//
// ----------------------------------------------------------------------- //

void RemoteTurret::OnDeath( )
{
	Turret::OnDeath( );

	// Reset any links that may be broken when the turret gets destroyed...
	if( m_hNextRemoteTurret )
	{
		RemoteTurret *pNext = dynamic_cast<RemoteTurret*>(g_pLTServer->HandleToObject( m_hNextRemoteTurret ));
		if( pNext )
		{
			pNext->SetPreviousRemoteTurret( m_hPrevRemoteTurret );
		}
	}

	if( m_hPrevRemoteTurret )
	{
		RemoteTurret *pPrev = dynamic_cast<RemoteTurret*>(g_pLTServer->HandleToObject( m_hPrevRemoteTurret ));
		if( pPrev )
		{
			pPrev->SetNextRemoteTurret( m_hNextRemoteTurret );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RemoteTurret::RemoteActivate
//
//	PURPOSE:	Activte the remote turret...
//
// ----------------------------------------------------------------------- //

void RemoteTurret::RemoteActivate( HOBJECT hPlayer )
{
	CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( hPlayer ));
	if( !pPlayer )
		return;

	// Send message to the activating client...
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_TURRET_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( kTurretFXMsg_RemoteActivate );
	g_pLTServer->SendToClient( cMsg.Read( ), pPlayer->GetClient( ), MESSAGE_GUARANTEED );

	Activate( hPlayer );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RemoteTurret::HandleOnMsg
//
//	PURPOSE:	Establis the previous and next remote turret links...
//
// ----------------------------------------------------------------------- //

void RemoteTurret::LinkRemoteTurrets( )
{
	HOBJECT hNext = INVALID_HOBJECT;
	HOBJECT hPrev = INVALID_HOBJECT;

	// Get the handle for the next remote turret, if there is one...
	if( !m_sNextRemoteTurretName.empty( ))
	{
		if( FindNamedObject( m_sNextRemoteTurretName.c_str( ), hNext, false ) != LT_OK )
		{
			LTASSERT_PARAM1( 0, "Could not find object named \"%s\"", m_sNextRemoteTurretName.c_str( ));
			hNext = NULL;
		}

		// Make sure the object listed is a RemoteTurret...
		if( hNext && IsKindOf( hNext, "RemoteTurret" ))
		{
			RemoteTurret *pRemoteTurret = dynamic_cast<RemoteTurret*>(g_pLTServer->HandleToObject( hNext ));
			if( pRemoteTurret )
			{
				pRemoteTurret->SetPreviousRemoteTurret( m_hObject );
			}
		}
		else
		{
			LTASSERT_PARAM1( 0, "Object \"%s\" is not a RemoteTurret.", m_sNextRemoteTurretName.c_str( ));
			hNext = NULL;
		}
	}

	m_hNextRemoteTurret = hNext;
	m_hPrevRemoteTurret = hPrev;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RemoteTurret::SetPreviousRemoteTurret
//
//	PURPOSE:	Allow another remote turret to set the previous link... 
//
// ----------------------------------------------------------------------- //

void RemoteTurret::SetPreviousRemoteTurret( HOBJECT hPrev )
{
	LTASSERT( (hPrev == NULL) || IsKindOf( hPrev, "RemoteTurret" ), "Trying to set a previous RemoteTurret link that is not of the correct type." );

	m_hPrevRemoteTurret = hPrev;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RemoteTurret::SetPreviousRemoteTurret
//
//	PURPOSE:	Allow another remote turret to set the next link... 
//
// ----------------------------------------------------------------------- //

void RemoteTurret::SetNextRemoteTurret( HOBJECT hNext )
{
	LTASSERT( (hNext == NULL) || IsKindOf( hNext, "RemoteTurret" ), "Trying to set a previous RemoteTurret link that is not of the correct type." );

	m_hNextRemoteTurret = hNext;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RemoteTurret::Deactivate
//
//	PURPOSE:	Deactivate the turret...
//
// ----------------------------------------------------------------------- //

void RemoteTurret::Deactivate( )
{
	if( !IsInUse( ))
		return;

	// Remember the operating object after for use after deactivation...
	HOBJECT hOperatingObject = m_hOperatingObject;

	// If there exists a link to another RemoteTurret, activate it...
	if( m_hNextRemoteTurret )
	{
		CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( hOperatingObject ));
		if( !pPlayer )
			return;

		// Send message to the activating client...
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_TURRET_ID );
		cMsg.WriteObject( m_hObject );
		cMsg.Writeuint8( kTurretFXMsg_SwitchToTurret );
		cMsg.WriteObject( m_hNextRemoteTurret );
		g_pLTServer->SendToClient( cMsg.Read( ), pPlayer->GetClient( ), MESSAGE_GUARANTEED );

		m_hOperatingObject = NULL;
	}

	Turret::Deactivate( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RemoteTurret::Save
//
//	PURPOSE:	Save the object...
//
// ----------------------------------------------------------------------- //

void RemoteTurret::Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg )
		return;

	SAVE_STDSTRING( m_sNextRemoteTurretName );
	SAVE_HOBJECT( m_hNextRemoteTurret );
	SAVE_HOBJECT( m_hPrevRemoteTurret );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RemoteTurret::Load
//
//	PURPOSE:	Load the object...
//
// ----------------------------------------------------------------------- //

void RemoteTurret::Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags )
{
	if( !pMsg )
		return;

	LOAD_STDSTRING( m_sNextRemoteTurretName );
	LOAD_HOBJECT( m_hNextRemoteTurret );
	LOAD_HOBJECT( m_hPrevRemoteTurret );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RemoteTurret::PostLoadActivate
//
//	PURPOSE:	Handle reactivating after loading a saved game...
//
// ----------------------------------------------------------------------- //

void RemoteTurret::PostLoadActivate( HOBJECT hOperatingObject )
{
	if( hOperatingObject )
	{
		CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( hOperatingObject ));
		if( !pPlayer )
			return;

		// Send message to the activating client...
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_TURRET_ID );
		cMsg.WriteObject( m_hObject );
		cMsg.Writeuint8( kTurretFXMsg_RemoteActivate );
		g_pLTServer->SendToClient( cMsg.Read( ), pPlayer->GetClient( ), MESSAGE_GUARANTEED );

		// Activating turret...
		m_hOperatingObject = hOperatingObject;
		CreateSpecialFX( true );
	}
}

// EOF

