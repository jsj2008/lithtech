// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerNodeGoto.cpp
//
// PURPOSE : Implementation of PlayerNodeGoto object
//
// CREATED : 09/12/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
// 

#include "Stdafx.h"
#include "PlayerNodeGoto.h"

LINKFROM_MODULE( PlayerNodeGoto );

//
// Object Properties...
//

#define PLAYER_NODE_DIMS	16.0f

BEGIN_CLASS( PlayerNodeGoto )

	ADD_BOOLPROP( FaceForwardPitch, true, "Set this to true if you want the Player's camera to align the pitch with the forward of the node when they actually arrive at the node." )
	ADD_BOOLPROP( FaceForwardYaw, true, "Set this to true if you want the Player's camera to align the yaw with the forward of the node when they actually arrive at the node."  )
	ADD_BOOLPROP( AlignPitch, true, "Set this to true if you want the Player's camera to align the pitch in the direction of the node before actually moving towards the node." )
	ADD_BOOLPROP( AlignYaw, true, "Set this to true if you want the Player's camera to align the yaw in the direction of the node before actually moving toward the node." )
	ADD_COMMANDPROP_FLAG( ArrivalCommand, "", PF_NOTIFYCHANGE, "Command that gets processed once the player has arrived at the node and is facing the correct direction." )
	ADD_VECTORPROP_VAL_FLAG( Dims, PLAYER_NODE_DIMS, PLAYER_NODE_DIMS, PLAYER_NODE_DIMS,	PF_HIDDEN | PF_DIMS, "Give dimensions to the node so it's easier to view how the player will be situated within the level")

END_CLASS_FLAGS( PlayerNodeGoto, GameBase, 0, "Specifies a node used for the player message GOTO." )

//
// Object Messages...
//

CMDMGR_BEGIN_REGISTER_CLASS( PlayerNodeGoto )
CMDMGR_END_REGISTER_CLASS( PlayerNodeGoto, GameBase )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	PlayerNodeGoto::PlayerNodeGoto
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

PlayerNodeGoto::PlayerNodeGoto( )
:	GameBase			( OT_NORMAL ),
	m_sArrivalCommand	( ),
	m_bFaceForwardPitch	( true ),
	m_bFaceForwardYaw	( true ),
	m_bAlignPitch		( true ),
	m_bAlignYaw			( true )
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	PlayerNodeGoto::~PlayerNodeGoto
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

PlayerNodeGoto::~PlayerNodeGoto( )
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	PlayerNodeGoto::EngineMessageFn
//
//  PURPOSE:	Handle a message recieved from the engine...
//
// ----------------------------------------------------------------------- //

uint32 PlayerNodeGoto::EngineMessageFn( uint32 dwMsgId, void *pData, float fData )
{
	switch( dwMsgId )
	{
		case MID_PRECREATE:
		{
			// Let the GameBase handle the message first...
			uint32 dwRet = GameBase::EngineMessageFn( dwMsgId, pData, fData );

			ObjectCreateStruct	*pOCS = (ObjectCreateStruct*)pData;

			if( pOCS && (PRECREATE_SAVEGAME != fData) )
			{
				ReadProps( &pOCS->m_cProperties );
			}

			// Important!! - We already sent the message to the GameBase so DONT do it again.
			return dwRet;
		}
		break;

		case MID_OBJECTCREATED:
		{
			// Make sure the object is sent to the clients but never update on the server...
			g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAG_FORCECLIENTUPDATE );
			SetNextUpdate( UPDATE_NEVER );
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

	return GameBase::EngineMessageFn( dwMsgId, pData, fData );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	PlayerNodeGoto::ReadProps
//
//  PURPOSE:	Read in the property values set for the node...
//
// ----------------------------------------------------------------------- //

void PlayerNodeGoto::ReadProps( const GenericPropList *pProps )
{
	if( !pProps )
		return;
	
	m_bFaceForwardPitch	= pProps->GetBool( "FaceForwardPitch", m_bFaceForwardPitch );
	m_bFaceForwardYaw	= pProps->GetBool( "FaceForwardYaw", m_bFaceForwardYaw );
	m_bAlignPitch		= pProps->GetBool( "AlignPitch", m_bAlignPitch );
	m_bAlignYaw			= pProps->GetBool( "AlignYaw", m_bAlignYaw );

	m_sArrivalCommand	= pProps->GetCommand( "ArrivalCommand", m_sArrivalCommand.c_str( ));
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	PlayerNodeGoto::HandlePlayerArrival
//
//  PURPOSE:	Handle a player arriving at the node...
//
// ----------------------------------------------------------------------- //

void PlayerNodeGoto::HandlePlayerArrival( HOBJECT hPlayer )
{
	if( !m_sArrivalCommand.empty( ))
	{
		g_pCmdMgr->QueueCommand( m_sArrivalCommand.c_str( ), hPlayer, hPlayer );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	PlayerNodeGoto::WriteNodeData
//
//  PURPOSE:	Write out the node's data to a message that will be sent to the client...
//
// ----------------------------------------------------------------------- //

void PlayerNodeGoto::WriteNodeData( ILTMessage_Write &rMsg ) const
{
	rMsg.Writebool( m_bFaceForwardPitch );
	rMsg.Writebool( m_bFaceForwardYaw );
	rMsg.Writebool( m_bAlignPitch );
	rMsg.Writebool( m_bAlignYaw );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	PlayerNodeGoto::OnSave
//
//  PURPOSE:	Save the object data...
//
// ----------------------------------------------------------------------- //

void PlayerNodeGoto::OnSave( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg )
		return;

	SAVE_STDSTRING( m_sArrivalCommand );
	SAVE_bool( m_bFaceForwardPitch );
	SAVE_bool( m_bFaceForwardYaw );
	SAVE_bool( m_bAlignPitch );
	SAVE_bool( m_bAlignYaw );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	PlayerNodeGoto::OnLoad
//
//  PURPOSE:	Load the object data...
//
// ----------------------------------------------------------------------- //

void PlayerNodeGoto::OnLoad( ILTMessage_Read *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg )
		return;

	LOAD_STDSTRING( m_sArrivalCommand );
	LOAD_bool( m_bFaceForwardPitch );
	LOAD_bool( m_bFaceForwardYaw );
	LOAD_bool( m_bAlignPitch );
	LOAD_bool( m_bAlignYaw );
}
