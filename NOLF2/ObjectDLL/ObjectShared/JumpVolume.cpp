// ----------------------------------------------------------------------- //
//
// MODULE  : JumpVolume.cpp
//
// PURPOSE : A JumpVolume for increasing velocity of an object
//
// CREATED : 1/24/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes..
//

	#include "stdafx.h"
	#include "MsgIds.h"
	#include "ObjectMsgs.h"
	#include "ParsedMsg.h"
	#include "JumpVolume.h"
	#include "ContainerCodes.h"

LINKFROM_MODULE( JumpVolume );

//
// Add Properties...
//

BEGIN_CLASS( JumpVolume )

	ADD_VISIBLE_FLAG( LTFALSE, PF_HIDDEN )
	ADD_SOLID_FLAG( LTFALSE, PF_HIDDEN )
	ADD_RAYHIT_FLAG( LTFALSE, PF_HIDDEN )
	ADD_GRAVITY_FLAG( LTFALSE, PF_HIDDEN )

	ADD_REALPROP_FLAG( Velocity, 1000.0f, 0 )

END_CLASS_DEFAULT_FLAGS( JumpVolume, GameBase, NULL, NULL, CF_WORLDMODEL )


CMDMGR_BEGIN_REGISTER_CLASS( JumpVolume )
	
	CMDMGR_ADD_MSG( VELOCITY, 2, NULL, "VELOCITY <speed>" )

CMDMGR_END_REGISTER_CLASS( JumpVolume, GameBase )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	JumpVolume::JumpVolume
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

JumpVolume::JumpVolume()
:	GameBase		( OT_CONTAINER ),
	m_fSpeed		( 0.0f ),
	m_vDir			( 0.0f, 0.0f, 0.0f )
{
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	JumpVolume::~JumpVolume
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

JumpVolume::~JumpVolume()
{
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	JumpVolume::EngineMessageFn
//
//  PURPOSE:	Handel messages from the engine
//
// ----------------------------------------------------------------------- //

uint32 JumpVolume::EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData )
{
	switch( messageID )
	{
		case MID_PRECREATE :
		{
			// Let the GameBase handle it first...

			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			ObjectCreateStruct *pOCS = (ObjectCreateStruct*)pData;

			if( pOCS )
			{
				if( PRECREATE_WORLDFILE == fData )
				{
					m_vDir = pOCS->m_Rotation.Forward();
					pOCS->m_Rotation.Identity();
					
					ReadProps( pOCS );
				}

				PostReadProp( pOCS );
			}

			return dwRet;
		}
		break;

		case MID_OBJECTCREATED :
		{
			if( OBJECTCREATED_SAVEGAME != fData )
			{
				g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, USRFLG_VISIBLE, USRFLG_VISIBLE );

				UpdateFXMessage( LTFALSE );
			}

			SetNextUpdate(UPDATE_NEVER);
		}
		break;

		case MID_SAVEOBJECT :
		{
			Save( (ILTMessage_Write*)pData, (uint32)fData );
		}
		break;

		case MID_LOADOBJECT :
		{
			Load( (ILTMessage_Read*)pData, (uint32)fData );
		}
		break;
	}


	return GameBase::EngineMessageFn( messageID, pData, fData );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	JumpVolume::OnTrigger
//
//  PURPOSE:	Handel recieving a trigger msg from another object
//
// ----------------------------------------------------------------------- //

bool JumpVolume::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Velocity("VELOCITY");

	if( cMsg.GetArg(0) == s_cTok_Velocity )
	{
		if( cMsg.GetArgCount() >= 2 )
		{
			m_fSpeed = (LTFLOAT)atof( cMsg.GetArg(1) );
			UpdateFXMessage( LTTRUE );
		}
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	JumpVolume::ReadProps
//
//  PURPOSE:	Read in the property values...
//
// ----------------------------------------------------------------------- //


void JumpVolume::ReadProps( ObjectCreateStruct *pOCS )
{
	ASSERT( pOCS != LTNULL );	

	GenericProp	gProp;

	if( g_pLTServer->GetPropGeneric( "Velocity", &gProp ) == LT_OK )
	{
		m_fSpeed = gProp.m_Float;
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	JumpVolume::PostReadProp
//
//  PURPOSE:	Initialize data after the property values are read
//
// ----------------------------------------------------------------------- //

void JumpVolume::PostReadProp( ObjectCreateStruct *pOCS )
{
	ASSERT( pOCS != LTNULL );

	SAFE_STRCPY( pOCS->m_Filename, pOCS->m_Name );
	pOCS->m_ObjectType		= OT_CONTAINER;
	pOCS->m_SkinName[0]		= '\0';
	pOCS->m_Flags			|= FLAG_CONTAINER | FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE;
	pOCS->m_ContainerCode	= (uint16)CC_JUMP_VOLUME;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	JumpVolume::UpdateFXMessage
//
//  PURPOSE:	Setup and send the SpecialFX message to the clients...
//
// ----------------------------------------------------------------------- //

void JumpVolume::UpdateFXMessage( LTBOOL bSendToClients )
{
	if( bSendToClients )
	{
		// Send the message to all connected clients...

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_JUMPVOLUME_ID );
		cMsg.WriteObject( m_hObject );
		WriteFXMessage( cMsg );

		g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );
	}

	// Update the message for new clients...

	CAutoMessage cMsg;
	cMsg.Writeuint8( SFX_JUMPVOLUME_ID );
	WriteFXMessage( cMsg );

	g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read() );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	JumpVolume::WriteFXMessage
//
//  PURPOSE:	Fill the message with the FX info...
//
// ----------------------------------------------------------------------- //

void JumpVolume::WriteFXMessage( ILTMessage_Write &cMsg )
{
	LTVector vVel = m_vDir * m_fSpeed;

	cMsg.WriteLTVector( vVel );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	JumpVolume::Save
//
//  PURPOSE:	Save the object...
//
// ----------------------------------------------------------------------- //

void JumpVolume::Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	ASSERT( pMsg != LTNULL );

	SAVE_FLOAT( m_fSpeed );
	SAVE_VECTOR( m_vDir );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	JumpVolume::Load
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

void JumpVolume::Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags )
{
	ASSERT( pMsg != LTNULL );

	LOAD_FLOAT( m_fSpeed );
	LOAD_VECTOR( m_vDir );
}