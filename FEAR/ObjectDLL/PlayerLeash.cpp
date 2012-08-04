// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerLeash.h
//
// PURPOSE : Game object that defines a leash for the player
//
// CREATED : 10/11/04
//
// (c) 2003-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "PlayerLeash.h"

// ----------------------------------------------------------------------- //

LINKFROM_MODULE( PlayerLeash );

// ----------------------------------------------------------------------- //

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_PlayerLeash 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_PlayerLeash CF_HIDDEN

#endif

BEGIN_CLASS( PlayerLeash )

	ADD_REALPROP_FLAG( InnerRadius, 125.0f, PF_RADIUS, "The inner leash radius, where velocity will start to be constrained." )
	ADD_REALPROP_FLAG( OuterRadius, 350.0f, PF_RADIUS, "The outer leash radius, where velocity will be zeroed out." )

END_CLASS_FLAGS( PlayerLeash, GameBase, CF_HIDDEN_PlayerLeash, "An object that determines an X/Z radial leash, to constrain the player to an area of the world." )

// ----------------------------------------------------------------------- //

CMDMGR_BEGIN_REGISTER_CLASS( PlayerLeash )

ADD_MESSAGE( ON, 1, NULL, MSG_HANDLER( PlayerLeash, HandleOnMessage ), "ON", "TODO:CMDDESC", "TODO:CMDEXP" )
ADD_MESSAGE( OFF, 1, NULL, MSG_HANDLER( PlayerLeash, HandleOffMessage ), "OFF", "TODO:CMDDESC", "TODO:CMDEXP" )

CMDMGR_END_REGISTER_CLASS( PlayerLeash, GameBase )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLeash::PlayerLeash()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PlayerLeash::PlayerLeash() : GameBase( OT_NORMAL )
{
	m_fInnerRadius = 0.0f;
	m_fOuterRadius = 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLeash::~PlayerLeash()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

PlayerLeash::~PlayerLeash()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLeash::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 PlayerLeash::EngineMessageFn( uint32 nMsgID, void* pData, float fData )
{
	switch( nMsgID )
	{
		case MID_PRECREATE:
		{
			// Let the GameBase handle the message first
			uint32 dwRet = GameBase::EngineMessageFn( nMsgID, pData, fData );
			ObjectCreateStruct* pOCS = ( ObjectCreateStruct* )pData;

			if( fData == PRECREATE_WORLDFILE )
			{
				ReadProps( &pOCS->m_cProperties );
				m_vPos = pOCS->m_Pos;

				pOCS->SetFileName( pOCS->m_Name );
				pOCS->m_Flags |= FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE;
				pOCS->m_Flags &= ~FLAG_RAYHIT;
			}

			// Important!! - We already sent the message to the GameBase so DONT do it again.
			return dwRet;
		}

		case MID_OBJECTCREATED:
		{
			if( fData != OBJECTCREATED_SAVEGAME )
			{
				OnObjectCreated();
			}

			break;
		}

		case MID_SAVEOBJECT:
		{
			Save( ( ILTMessage_Write* )pData, ( uint32 )fData );
			break;
		}

		case MID_LOADOBJECT:
		{
			Load( ( ILTMessage_Read* )pData, ( uint32 )fData );
			break;
		}
	}


	return GameBase::EngineMessageFn( nMsgID, pData, fData );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLeash::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 PlayerLeash::ObjectMessageFn( HOBJECT hSender, ILTMessage_Read* pMsg )
{
	return GameBase::ObjectMessageFn( hSender, pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLeash::ReadProp()
//
//	PURPOSE:	Read properties from object data
//
// ----------------------------------------------------------------------- //

void PlayerLeash::ReadProps( const GenericPropList *pProps )
{
	if( !pProps ) return;

	m_fInnerRadius = LTMAX( 0.0f, pProps->GetReal( "InnerRadius", 0.0f ) );
	m_fOuterRadius = LTMAX( 0.0f, pProps->GetReal( "OuterRadius", 0.0f ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLeash::InitialUpdate()
//
//	PURPOSE:	Sets up the object based on member variables.
//
// ----------------------------------------------------------------------- //

void PlayerLeash::OnObjectCreated()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLeash::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void PlayerLeash::Save( ILTMessage_Write* pMsg, uint32 nFlags )
{
	if( !pMsg ) return;

	SAVE_FLOAT( m_fInnerRadius );
	SAVE_FLOAT( m_fOuterRadius );
	SAVE_VECTOR( m_vPos );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLeash::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PlayerLeash::Load( ILTMessage_Read* pMsg, uint32 nFlags )
{
	if( !pMsg ) return;

	LOAD_FLOAT( m_fInnerRadius );
	LOAD_FLOAT( m_fOuterRadius );
	LOAD_VECTOR( m_vPos );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLeash::HandleOnMessage
//
//	PURPOSE:	Handle a message to turn the leash on...
//
// ----------------------------------------------------------------------- //

void PlayerLeash::HandleOnMessage( HOBJECT hSender, const CParsedMsg& crParsedMsg )
{
	CAutoMessage cMsg;

	cMsg.Writeuint8( MID_PLAYER_LEASH );
	cMsg.Writefloat( m_fOuterRadius );
	cMsg.Writefloat( m_fInnerRadius );
	cMsg.WriteLTVector( m_vPos );

	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLeash::HandleOffMessage
//
//	PURPOSE:	Handle a message to turn the leash off...
//
// ----------------------------------------------------------------------- //

void PlayerLeash::HandleOffMessage( HOBJECT hSender, const CParsedMsg& crParsedMsg )
{
	CAutoMessage cMsg;

	cMsg.Writeuint8( MID_PLAYER_LEASH );
	cMsg.Writefloat( 0.0f );

	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
}

