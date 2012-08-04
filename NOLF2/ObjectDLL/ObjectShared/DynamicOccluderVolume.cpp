// ----------------------------------------------------------------------- //
//
// MODULE  : DynamicOccluderVolume.cpp
//
// PURPOSE : When the client-camera is inside the DyanmicOccluderVolume
//			 all occluders associated with the volume are enabled.  When
//			 the client-camera leaves the volume, the occluders are disabled. 
//
// CREATED : 4/17/02
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
	#include "DynamicOccluderVolume.h"
	#include "ContainerCodes.h"

LINKFROM_MODULE( DynamicOccluderVolume );

//
// Add Properties...
//

BEGIN_CLASS( DynamicOccluderVolume )

	ADD_VISIBLE_FLAG( LTFALSE, PF_HIDDEN )
	ADD_SOLID_FLAG( LTFALSE, PF_HIDDEN )
	ADD_RAYHIT_FLAG( LTFALSE, PF_HIDDEN )
	ADD_GRAVITY_FLAG( LTFALSE, PF_HIDDEN )

	ADD_STRINGPROP_FLAG(OccluderName1, "", 0)
	ADD_STRINGPROP_FLAG(OccluderName2, "", 0)
	ADD_STRINGPROP_FLAG(OccluderName3, "", 0)
	ADD_STRINGPROP_FLAG(OccluderName4, "", 0)
	ADD_STRINGPROP_FLAG(OccluderName5, "", 0)
	ADD_STRINGPROP_FLAG(OccluderName6, "", 0)
	ADD_STRINGPROP_FLAG(OccluderName7, "", 0)
	ADD_STRINGPROP_FLAG(OccluderName8, "", 0)
	ADD_STRINGPROP_FLAG(OccluderName9, "", 0)
	ADD_STRINGPROP_FLAG(OccluderName10, "", 0)

	ADD_STRINGPROP_FLAG(RenderGroup1, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup2, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup3, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup4, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup5, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup6, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup7, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup8, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup9, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup10, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup11, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup12, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup13, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup14, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup15, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup16, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup17, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup18, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup19, "", 0)
	ADD_STRINGPROP_FLAG(RenderGroup20, "", 0)

END_CLASS_DEFAULT_FLAGS( DynamicOccluderVolume, GameBase, NULL, NULL, CF_WORLDMODEL )


CMDMGR_BEGIN_REGISTER_CLASS( DynamicOccluderVolume )
CMDMGR_END_REGISTER_CLASS( DynamicOccluderVolume, GameBase )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DynamicOccluderVolume::DynamicOccluderVolume
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

DynamicOccluderVolume::DynamicOccluderVolume()
:	GameBase( OT_CONTAINER ),
	m_nNumOccluders	( 0 ),
	m_nNumRenderGroups ( 0 )
{
	uint32 i;
	for( i = 0; i < kMaxOccluders; ++i )
	{
		m_nOccluderIds[i] = 0;
	}

	for( i = 0; i < kMaxRenderGroups; ++i )
	{
		m_nRenderGroups[i] = 0;
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DynamicOccluderVolume::~DynamicOccluderVolume
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

DynamicOccluderVolume::~DynamicOccluderVolume()
{
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DynamicOccluderVolume::EngineMessageFn
//
//  PURPOSE:	Handel messages from the engine
//
// ----------------------------------------------------------------------- //

uint32 DynamicOccluderVolume::EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData )
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

			SetNextUpdate( UPDATE_NEVER );
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
//  ROUTINE:	DynamicOccluderVolume::ReadProps
//
//  PURPOSE:	Read in the property values...
//
// ----------------------------------------------------------------------- //

void DynamicOccluderVolume::ReadProps( ObjectCreateStruct *pOCS )
{
	ASSERT( pOCS != LTNULL );	

	GenericProp	gProp;
	char szPropName[64] = {0};

	uint32 i;
	for( i = 0; i < kMaxOccluders; ++i )
	{
		sprintf( szPropName, "OccluderName%i", i+1 );
		if( g_pLTServer->GetPropGeneric( szPropName, &gProp ) == LT_OK )
		{
			if( gProp.m_String[0] )
			{
				uint32 nId = 0;
				if (LT_OK == g_pLTServer->GetOccluderID((const char*)gProp.m_String, &nId))
				{
					m_nOccluderIds[m_nNumOccluders] = nId;
					m_nNumOccluders++;
				}
			}
		}
	}

	for( i = 0; i < kMaxRenderGroups; ++i )
	{
		sprintf( szPropName, "RenderGroup%i", i+1 );
		if( g_pLTServer->GetPropGeneric( szPropName, &gProp ) == LT_OK )
		{
			if( gProp.m_String[0] )
			{
				uint32 nId = atoi(gProp.m_String);
				m_nRenderGroups[m_nNumRenderGroups] = nId;
				m_nNumRenderGroups++;
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DynamicOccluderVolume::PostReadProp
//
//  PURPOSE:	Initialize data after the property values are read
//
// ----------------------------------------------------------------------- //

void DynamicOccluderVolume::PostReadProp( ObjectCreateStruct *pOCS )
{
	ASSERT( pOCS != LTNULL );

	SAFE_STRCPY( pOCS->m_Filename, pOCS->m_Name );
	pOCS->m_ObjectType		= OT_CONTAINER;
	pOCS->m_SkinName[0]		= '\0';
	pOCS->m_Flags			|= FLAG_CONTAINER | FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE;
	pOCS->m_ContainerCode	= (uint16)CC_DYNAMIC_OCCLUDER_VOLUME;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DynamicOccluderVolume::UpdateFXMessage
//
//  PURPOSE:	Setup and send the SpecialFX message to the clients...
//
// ----------------------------------------------------------------------- //

void DynamicOccluderVolume::UpdateFXMessage( LTBOOL bSendToClients )
{
	if( bSendToClients )
	{
		// Send the message to all connected clients...

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_DYNAMIC_OCCLUDER_ID );
		cMsg.WriteObject( m_hObject );
		WriteFXMessage( cMsg );

		g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}

	// Update the message for new clients...

	CAutoMessage cMsg;
	cMsg.Writeuint8( SFX_DYNAMIC_OCCLUDER_ID );
	WriteFXMessage( cMsg );

	g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read() );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DynamicOccluderVolume::WriteFXMessage
//
//  PURPOSE:	Fill the message with the FX info...
//
// ----------------------------------------------------------------------- //

void DynamicOccluderVolume::WriteFXMessage( ILTMessage_Write &cMsg )
{
	cMsg.Writeuint8( m_nNumOccluders );

	uint32 i;
	for ( i=0; i < m_nNumOccluders; i++)
	{
		cMsg.Writeuint32( m_nOccluderIds[i] );
	}

	cMsg.Writeuint8( m_nNumRenderGroups );

	for ( i=0; i < m_nNumRenderGroups; i++)
	{
		cMsg.Writeuint8( m_nRenderGroups[i] );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DynamicOccluderVolume::Save
//
//  PURPOSE:	Save the object...
//
// ----------------------------------------------------------------------- //

void DynamicOccluderVolume::Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	ASSERT( pMsg != LTNULL );

	SAVE_BYTE( m_nNumOccluders );

	uint32 i;
	for ( i=0; i < m_nNumOccluders; i++)
	{
		SAVE_INT( m_nOccluderIds[i] );
	}

	SAVE_BYTE( m_nNumRenderGroups );

	for ( i=0; i < m_nNumRenderGroups; i++)
	{
		SAVE_INT( m_nRenderGroups[i] );
	}

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DynamicOccluderVolume::Load
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

void DynamicOccluderVolume::Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags )
{
	ASSERT( pMsg != LTNULL );

	LOAD_BYTE( m_nNumOccluders );

	uint32 i;
	for ( i=0; i < m_nNumOccluders; i++)
	{
		LOAD_INT(m_nOccluderIds[i]);
	}

	LOAD_BYTE( m_nNumRenderGroups );

	for ( i=0; i < m_nNumRenderGroups; i++)
	{
		LOAD_INT(m_nRenderGroups[i]);
	}

}