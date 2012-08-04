// ----------------------------------------------------------------------- //
//
// MODULE  : JumpVolume.cpp
//
// PURPOSE : A JumpVolume for increasing velocity of an object
//
// CREATED : 1/24/02
//
// (c) 2002-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes..
//

	#include "Stdafx.h"
	#include "MsgIDs.h"
	#include "ObjectMsgs.h"
	#include "ParsedMsg.h"
	#include "JumpVolume.h"
	#include "ContainerCodes.h"

LINKFROM_MODULE( JumpVolume );

//
// Add Properties...
//

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_JumpVolume 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_JumpVolume CF_HIDDEN

#endif

BEGIN_CLASS( JumpVolume )

	ADD_VISIBLE_FLAG( false, PF_HIDDEN )
	ADD_SOLID_FLAG( false, PF_HIDDEN )
	ADD_RAYHIT_FLAG( false, PF_HIDDEN )
	ADD_GRAVITY_FLAG( false, PF_HIDDEN )

	ADD_REALPROP_FLAG( Velocity, 1000.0f, 0, "TODO:PROPDESC" )

END_CLASS_FLAGS( JumpVolume, GameBase, CF_WORLDMODEL | CF_HIDDEN_JumpVolume, "A JumpVolume for increasing velocity of an object" )


CMDMGR_BEGIN_REGISTER_CLASS( JumpVolume )
	
	ADD_MESSAGE( VELOCITY, 2, NULL, MSG_HANDLER( JumpVolume, HandleVelocityMsg ), "VELOCITY <speed>", "TODO:CMDDESC", "TODO:CMDEXP" )

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

uint32 JumpVolume::EngineMessageFn( uint32 messageID, void *pData, float fData )
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
					
					ReadProps( &pOCS->m_cProperties );
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

				UpdateFXMessage( false );
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
//  ROUTINE:	JumpVolume::HandleVelocityMsg
//
//  PURPOSE:	Handle a VELOCITY message...
//
// ----------------------------------------------------------------------- //

void JumpVolume::HandleVelocityMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_fSpeed = (float)atof( crParsedMsg.GetArg(1) );
	UpdateFXMessage( true );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	JumpVolume::ReadProps
//
//  PURPOSE:	Read in the property values...
//
// ----------------------------------------------------------------------- //


void JumpVolume::ReadProps( const GenericPropList *pProps )
{
	LTASSERT( pProps != NULL, "NULL ObjectCreateStruct!" );	

	m_fSpeed = pProps->GetReal( "Velocity", m_fSpeed );
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
	ASSERT( pOCS != NULL );

	pOCS->SetFileName( pOCS->m_Name );
	pOCS->m_ObjectType		= OT_CONTAINER;
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

void JumpVolume::UpdateFXMessage( bool bSendToClients )
{
	if( bSendToClients )
	{
		// Send the message to all connected clients...

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_JUMPVOLUME_ID );
		cMsg.WriteObject( m_hObject );
		WriteFXMessage( cMsg );

		g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
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
	ASSERT( pMsg != NULL );

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
	ASSERT( pMsg != NULL );

	LOAD_FLOAT( m_fSpeed );
	LOAD_VECTOR( m_vDir );
}
