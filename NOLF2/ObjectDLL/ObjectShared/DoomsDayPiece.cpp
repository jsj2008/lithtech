// ----------------------------------------------------------------------- //
//
// MODULE  : DoomsDayPiece.cpp
//
// PURPOSE : The object used for collecting DoomsDay pieces 
//
// CREATED : 12/13/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "ParsedMsg.h"
	#include "PlayerObj.h"
	#include "DoomsDayDevice.h"
	#include "DoomsDayPiece.h"

//
// Globals...
//
	
	

LINKFROM_MODULE( DoomsDayPiece );

BEGIN_CLASS( DoomsDayPiece )

	ADD_STRINGPROP_FLAG(Type, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS)

END_CLASS_DEFAULT_FLAGS_PLUGIN( DoomsDayPiece, PropType, NULL, NULL, 0, CDoomsDayPiecePlugin )


// Register with the CommandMgr...

CMDMGR_BEGIN_REGISTER_CLASS( DoomsDayPiece )
CMDMGR_END_REGISTER_CLASS( DoomsDayPiece, PropType )



LTRESULT CDoomsDayPiecePlugin::PreHook_EditStringList(
	const char *szRezPath,
	const char *szPropName,
	char **aszStrings,
	uint32 *pcStrings,
	const uint32 cMaxStrings,
	const uint32 cMaxStringLen )
{
	if( _strcmpi("Type", szPropName) == 0 )
	{
		// Fill the list with our piece types...

		for( int i = 0; i < kDoomsDay_MAXTYPES; i++ )
		{
			strcpy( aszStrings[(*pcStrings)++], c_aDDPieceTypes[i].m_pszPropType );
		}
		
		return LT_OK;
	}
	else if( LT_OK == CPropTypePlugin::PreHook_EditStringList( szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLen ))
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

LTRESULT CDoomsDayPiecePlugin::PreHook_Dims( 
	const char *szRezPath,
	const char *szPropValue,
	char *szModelFilenameBuf,
	int nModelFilenameBufLen,
	LTVector &vDims )
{
	if( LT_OK == CPropTypePlugin::PreHook_Dims( szRezPath, szPropValue, szModelFilenameBuf, nModelFilenameBufLen, vDims ))
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DoomsDayPiece::DoomsDayPiece
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

DoomsDayPiece::DoomsDayPiece( )
:	PropType		( ),
	m_hDevice		( LTNULL ),
	m_vOriginalPos	( 0.0f, 0.0f, 0.0f ),
	m_bCanBeCarried	( true )
{
	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DoomsDayPiece::DoomsDayPiece
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

DoomsDayPiece::~DoomsDayPiece( )
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayPiece::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 DoomsDayPiece::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if( fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP )
			{
				ReadProp( (ObjectCreateStruct*)pData );
			}
		}
		break;
		
		case MID_INITIALUPDATE:
		{
			// Cache the position to be used for respawning the piece when dropped...

			g_pLTServer->GetObjectPos( m_hObject, &m_vOriginalPos );

			CreateSFXMessage(false,INVALID_TEAM);


		}
		break;
		
		case MID_UPDATE:
		{
			uint32 dwRet = PropType::EngineMessageFn(messageID, pData, fData);
			
			Update( );

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

		default : break;
	}

	return PropType::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayPiece::PropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

bool DoomsDayPiece::ReadProp( ObjectCreateStruct *pOCS )
{
	if( !pOCS )
		return false;

	GenericProp GenProp;

	if( g_pLTServer->GetPropGeneric( "Type", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			for( int i = 0; i < kDoomsDay_MAXTYPES; ++i )
			{
				if( !_stricmp( GenProp.m_String, c_aDDPieceTypes[i].m_pszPropType ) )
				{
					m_nDDPieceType = (DDPieceType)i;
					break;
				}
			}	
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayPiece::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

bool DoomsDayPiece::OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken	s_cTok_Carry( "CARRY" );
	static CParsedMsg::CToken	s_cTok_Drop( "DROP" );

	if( cMsg.GetArg( 0 ) == s_cTok_Carry )
	{
		if( !OnCarry( hSender ))
			return false;

		m_bCanBeCarried = false;
	}
	else if( cMsg.GetArg( 0 ) == s_cTok_Drop )
	{
		if( !OnDrop( hSender ))
			return false;

		m_bCanBeCarried = true;
	}
	
	return PropType::OnTrigger( hSender, cMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayPiece::OnCarry
//
//	PURPOSE:	Handle the Carry message...
//
// ----------------------------------------------------------------------- //

bool DoomsDayPiece::OnCarry( HOBJECT hSender )
{
	if( !m_bCanBeCarried )
		return false;
	
	if( !IsPlayer( hSender ))
		return false;

	CPlayerObj *pPlayer = dynamic_cast< CPlayerObj* >(g_pLTServer->HandleToObject( hSender ));
	if( !pPlayer )
		return false;

	if( !m_DelayTimer.Stopped() )
		return false;



	uint8 nDDAction = MID_DOOMSDAY_PIECE_PICKEDUP;
	
	if( m_hDevice )
	{
		// Removing the piece from a device...

		DoomsDayDevice *pDevice = dynamic_cast< DoomsDayDevice* >(g_pLTServer->HandleToObject( m_hDevice ));
		if( pDevice )
		{
			// Don't allow the player to remove pieces from it's own device....

			if( pPlayer->GetTeamID() == pDevice->GetTeamID() )
				return false;

			if( !pDevice->RemoveDoomsDayPiece( this, pPlayer ) )
				return false;

			m_hDevice = LTNULL;

			nDDAction = MID_DOOMSDAY_PIECE_STOLEN;
		}
	}

	// Send a message letting the players know which team currently has the piece...

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_DOOMSDAY_MESSAGE );
	cMsg.Writeuint8( nDDAction );
	cMsg.Writeuint8( GetDoomsDayPieceType() );
	cMsg.Writeuint8( pPlayer->GetTeamID() );
	g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );

	// Go non-solid so the character can move while carrying it...

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, 0, FLAG_SOLID | FLAG_RAYHIT );
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, USRFLG_ATTACH_HIDE1SHOW3, USRFLG_ATTACH_HIDE1SHOW3);

	ShowObjectAttachments( m_hObject, false );
	
	// Set this object as the players carried object...
	
	pPlayer->SetCarriedObject( m_hObject );
	
	// Make sure it's not faded at all...

	float r, g, b, a;
	g_pLTServer->GetObjectColor( m_hObject, &r, &g, &b, &a );
	g_pLTServer->SetObjectColor( m_hObject, r, g, b, 1.0f );

	m_bFading = false;

	// Stop the respawn counter since the piece is being carried again...

	m_RespawnTimer.Stop();
	m_DelayTimer.Stop();
	SetNextUpdate( UPDATE_NEVER );

	CreateSFXMessage(true,pPlayer->GetTeamID());

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayPiece::OnDrop
//
//	PURPOSE:	Handle the Drop message...
//
// ----------------------------------------------------------------------- //

bool DoomsDayPiece::OnDrop( HOBJECT hSender )
{
	// Make sure the piece can be picked up again...

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_RAYHIT , FLAG_RAYHIT );
	
	// See if we are being dropped near a DoomsDayDevice...

	if( !IsPlayer( hSender ))
		return false;

	CPlayerObj *pPlayer = dynamic_cast< CPlayerObj* >(g_pLTServer->HandleToObject( hSender ));
	if( !pPlayer )
		return false;



	LTVector vDropPos;
	g_pLTServer->GetObjectPos( hSender, &vDropPos );

	LTVector vDevicePos;

	DoomsDayDevice::DoomsDayDeviceList::const_iterator iter = DoomsDayDevice::GetDoomsDayDeviceList().begin();
	while( iter != DoomsDayDevice::GetDoomsDayDeviceList().end() )
	{
		g_pLTServer->GetObjectPos( (*iter)->m_hObject, &vDevicePos );

		if( ( vDropPos.Dist( vDevicePos ) <= (*iter)->GetDropZoneRadius() ) &&
			( (*iter)->GetTeamID() == pPlayer->GetTeamID() ))
		{
			// Dropping close to a device...
			
			if( (*iter)->AddDoomsDayPiece( this, pPlayer ) )
			{
				// Let the piece know it's now part of a device...

				m_hDevice = (*iter)->m_hObject;

				// Send a message letting the players know which team currently has the piece...
				if(( *iter )->GetDoomsDayPieceList( ).size( ) < kDoomsDay_MAXTYPES )
				{
					CAutoMessage cMsg;
					cMsg.Writeuint8( MID_DOOMSDAY_MESSAGE );
					cMsg.Writeuint8( MID_DOOMSDAY_PIECE_PLACED );
					cMsg.Writeuint8( GetDoomsDayPieceType() );
					cMsg.Writeuint8( pPlayer->GetTeamID() );
					g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );

					CreateSFXMessage(false,pPlayer->GetTeamID());
				}
				
				return true;
			}
		}

		++iter;
	}

	// Send a message letting the players know which team currently has the piece...

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_DOOMSDAY_MESSAGE );
	cMsg.Writeuint8( MID_DOOMSDAY_PIECE_DROPPED );
	cMsg.Writeuint8( GetDoomsDayPieceType() );
	cMsg.Writeuint8( pPlayer->GetTeamID() );
	g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );
	
	CreateSFXMessage(false,INVALID_TEAM);

	// Not close enough to a device so just drop where we are...

	g_pLTServer->SetObjectPos( m_hObject, &vDropPos );

	// Go solid again if originally solid...

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, m_dwFlags, m_dwFlags );

	HOBJECT hFilter[] = { m_hObject, hSender, LTNULL };
	MoveObjectToFloor( m_hObject, hFilter );
	
	// Make sure the piece is playing it's base animation...

	HMODELANIM hAnim = 0;
	g_pModelLT->GetAnimIndex( m_hObject, "base", hAnim );
	g_pModelLT->SetCurAnim( m_hObject, MAIN_TRACKER, hAnim );
	g_pModelLT->SetLooping( m_hObject, MAIN_TRACKER, false );
	g_pModelLT->ResetAnim( m_hObject, MAIN_TRACKER );

	// Make sure it's not faded at all...

	float r, g, b, a;
	g_pLTServer->GetObjectColor( m_hObject, &r, &g, &b, &a );
	g_pLTServer->SetObjectColor( m_hObject, r, g, b, 1.0f );

	m_bFading = false;

	// Begin our respawn timer...

	m_RespawnTimer.Start( g_pServerButeMgr->GetDoomsDayDeviceRespawnTime() );
	m_DelayTimer.Start(0.5f);

	SetNextUpdate( UPDATE_NEXT_FRAME );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayPiece::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void DoomsDayPiece::Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg )
		return;

	m_RespawnTimer.Save( pMsg );
	SAVE_BYTE( m_nDDPieceType );
	SAVE_HOBJECT( m_hDevice );
	SAVE_VECTOR( m_vOriginalPos );
	SAVE_bool( m_bCanBeCarried );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayPiece::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void DoomsDayPiece::Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags )
{
	if( !pMsg )
		return;

	m_RespawnTimer.Load( pMsg );
	LOAD_BYTE_CAST( m_nDDPieceType, DDPieceType );
	LOAD_HOBJECT( m_hDevice );
	LOAD_VECTOR( m_vOriginalPos );
	LOAD_bool( m_bCanBeCarried );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayPiece::IsHeavy
//
//	PURPOSE:	Checks if piece is heavy type.
//
// ----------------------------------------------------------------------- //

bool DoomsDayPiece::IsHeavy( ) const
{
	return c_aDDPieceTypes[m_nDDPieceType].m_bHeavy;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayPiece::Update
//
//	PURPOSE:	Frame Update...
//
// ----------------------------------------------------------------------- //

void DoomsDayPiece::Update()
{
	if( m_RespawnTimer.Stopped() )
	{
		Respawn( );

		SetNextUpdate( UPDATE_NEVER );
	}
	else if( m_RespawnTimer.On() )
	{
	
		if( m_RespawnTimer.GetCountdownTime() <= 10.0f )
		{
			float r, g, b, fAlpha;
			g_pLTServer->GetObjectColor( m_hObject, &r, &g, &b, &fAlpha );

			if( fAlpha >= 0.9f )
			{
				float fDuration = m_RespawnTimer.GetCountdownTime() / 10.0f;
				StartFade( fDuration, 0.0f, 0.0f, false );
			}
			else if( fAlpha <= 0.0f )
			{
				float fDuration = m_RespawnTimer.GetCountdownTime() / 10.0f;
				StartFade( fDuration, 0.0f, 1.0f, false );
			}
		}

		SetNextUpdate( UPDATE_NEXT_FRAME );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoomsDayPiece::Respawn
//
//	PURPOSE:	Place the piece back at it's original position...
//
// ----------------------------------------------------------------------- //

void DoomsDayPiece::Respawn( )
{


	// Set the position to the origianl position...

	g_pLTServer->SetObjectPos( m_hObject, &m_vOriginalPos );

	// Make sure it's not faded at all...

	float r, g, b, a;
	g_pLTServer->GetObjectColor( m_hObject, &r, &g, &b, &a );
	g_pLTServer->SetObjectColor( m_hObject, r, g, b, 1.0f );

	m_bFading = false;

	// Move it to the floor...

	HOBJECT hFilter[] = { m_hObject, LTNULL };
	MoveObjectToFloor( m_hObject, hFilter );

	// Let the clients know it respawned...

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_DOOMSDAY_MESSAGE );
	cMsg.Writeuint8( MID_DOOMSDAY_PIECE_RESPAWNED );
	cMsg.Writeuint8( GetDoomsDayPieceType() );
	g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );
}


void DoomsDayPiece::CreateSFXMessage(bool bCarried, uint8 nTeam)
{
	bool bPlanted = !!(m_hDevice);

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_DOOMSDAYPIECE_ID);
	cMsg.Writeuint8(m_nDDPieceType);
	cMsg.Writebool(bCarried);
	cMsg.Writeuint8(nTeam);
	cMsg.Writebool( bPlanted );
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());


	cMsg.Reset();
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_DOOMSDAYPIECE_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(m_nDDPieceType);
	cMsg.Writebool(bCarried);
	cMsg.Writeuint8(nTeam);
	cMsg.Writebool( bPlanted );
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

}