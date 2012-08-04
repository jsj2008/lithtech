// ----------------------------------------------------------------------- //
//
// MODULE  : DynamicSectorVolume.cpp
//
// PURPOSE : When the client-camera is inside the DyanmicSectorVolume
//			 all sectors associated with the volume are enabled.  When
//			 the client-camera leaves the volume, the sectors are disabled. 
//
// CREATED : 4/17/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes..
//

	#include "Stdafx.h"
	#include "MsgIDs.h"
	#include "ObjectMsgs.h"
	#include "ParsedMsg.h"
	#include "DynamicSectorVolume.h"
	#include "ContainerCodes.h"

LINKFROM_MODULE( DynamicSectorVolume );

//
// Add Properties...
//

BEGIN_CLASS( DynamicSectorVolume )

	ADD_VISIBLE_FLAG( false, PF_HIDDEN )
	ADD_SOLID_FLAG( false, PF_HIDDEN )
	ADD_RAYHIT_FLAG( false, PF_HIDDEN )
	ADD_GRAVITY_FLAG( false, PF_HIDDEN )

	ADD_STRINGPROP_FLAG(SectorName1, "", 0, "Specifies a sector to disable when standing in this volume")
	ADD_STRINGPROP_FLAG(SectorName2, "", 0, "Specifies a sector to disable when standing in this volume")
	ADD_STRINGPROP_FLAG(SectorName3, "", 0, "Specifies a sector to disable when standing in this volume")
	ADD_STRINGPROP_FLAG(SectorName4, "", 0, "Specifies a sector to disable when standing in this volume")
	ADD_STRINGPROP_FLAG(SectorName5, "", 0, "Specifies a sector to disable when standing in this volume")
	ADD_STRINGPROP_FLAG(SectorName6, "", 0, "Specifies a sector to disable when standing in this volume")
	ADD_STRINGPROP_FLAG(SectorName7, "", 0, "Specifies a sector to disable when standing in this volume")
	ADD_STRINGPROP_FLAG(SectorName8, "", 0, "Specifies a sector to disable when standing in this volume")
	ADD_STRINGPROP_FLAG(SectorName9, "", 0, "Specifies a sector to disable when standing in this volume")
	ADD_STRINGPROP_FLAG(SectorName10, "", 0, "Specifies a sector to disable when standing in this volume")

END_CLASS_FLAGS( DynamicSectorVolume, GameBase, CF_WORLDMODEL, "Provides a mechanism for disabling visibility through sectors when standing in this volume" )


CMDMGR_BEGIN_REGISTER_CLASS( DynamicSectorVolume )
CMDMGR_END_REGISTER_CLASS( DynamicSectorVolume, GameBase )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DynamicSectorVolume::DynamicSectorVolume
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

DynamicSectorVolume::DynamicSectorVolume()
:	GameBase( OT_CONTAINER ),
	m_nNumSectors	( 0 )
{
	for( uint32 i = 0; i < kMaxSectors; ++i )
	{
		m_nSectorIds[i] = 0;
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DynamicSectorVolume::~DynamicSectorVolume
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

DynamicSectorVolume::~DynamicSectorVolume()
{
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DynamicSectorVolume::EngineMessageFn
//
//  PURPOSE:	Handel messages from the engine
//
// ----------------------------------------------------------------------- //

uint32 DynamicSectorVolume::EngineMessageFn( uint32 messageID, void *pData, float fData )
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
//  ROUTINE:	DynamicSectorVolume::ReadProps
//
//  PURPOSE:	Read in the property values...
//
// ----------------------------------------------------------------------- //

void DynamicSectorVolume::ReadProps( const GenericPropList *pProps )
{
	ASSERT( pProps != NULL );	

	char szPropName[32] = {0};
	const char *pszSector = NULL;

	for( uint32 nSector = 1; nSector < kMaxSectors; ++nSector )
	{
		LTSNPrintF( szPropName, ARRAY_LEN(szPropName), "SectorName%i", nSector );
		pszSector = pProps->GetString( szPropName, "" );

		if( pszSector && pszSector[0] )
		{
			uint32 nId = 0;
			if( LT_OK == g_pLTServer->GetSectorID( pszSector, &nId ))
			{
				m_nSectorIds[m_nNumSectors] = nId;
				m_nNumSectors++;
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DynamicSectorVolume::PostReadProp
//
//  PURPOSE:	Initialize data after the property values are read
//
// ----------------------------------------------------------------------- //

void DynamicSectorVolume::PostReadProp( ObjectCreateStruct *pOCS )
{
	ASSERT( pOCS != NULL );

	pOCS->SetFileName( pOCS->m_Name );
	pOCS->m_ObjectType		= OT_CONTAINER;
	pOCS->m_Flags			|= FLAG_CONTAINER | FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE;
	pOCS->m_ContainerCode	= (uint16)CC_DYNAMIC_SECTOR_VOLUME;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DynamicSectorVolume::UpdateFXMessage
//
//  PURPOSE:	Setup and send the SpecialFX message to the clients...
//
// ----------------------------------------------------------------------- //

void DynamicSectorVolume::UpdateFXMessage( bool bSendToClients )
{
	if( bSendToClients )
	{
		// Send the message to all connected clients...

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_DYNAMIC_SECTOR_ID );
		cMsg.WriteObject( m_hObject );
		WriteFXMessage( cMsg );

		g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED);
	}

	// Update the message for new clients...

	CAutoMessage cMsg;
	cMsg.Writeuint8( SFX_DYNAMIC_SECTOR_ID );
	WriteFXMessage( cMsg );

	g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read() );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DynamicSectorVolume::WriteFXMessage
//
//  PURPOSE:	Fill the message with the FX info...
//
// ----------------------------------------------------------------------- //

void DynamicSectorVolume::WriteFXMessage( ILTMessage_Write &cMsg )
{
	cMsg.Writeuint8( m_nNumSectors );

	uint32 i;
	for ( i=0; i < m_nNumSectors; i++)
	{
		cMsg.Writeuint32( m_nSectorIds[i] );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DynamicSectorVolume::Save
//
//  PURPOSE:	Save the object...
//
// ----------------------------------------------------------------------- //

void DynamicSectorVolume::Save( ILTMessage_Write *pMsg, uint32 /*dwSaveFlags*/ )
{
	ASSERT( pMsg != NULL );

	SAVE_BYTE( m_nNumSectors );

	uint32 i;
	for ( i=0; i < m_nNumSectors; i++)
	{
		SAVE_INT( m_nSectorIds[i] );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DynamicSectorVolume::Load
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

void DynamicSectorVolume::Load( ILTMessage_Read *pMsg, uint32 /*dwLoadFlags*/ )
{
	ASSERT( pMsg != NULL );

	LOAD_BYTE( m_nNumSectors );

	uint32 i;
	for ( i=0; i < m_nNumSectors; i++)
	{
		LOAD_INT(m_nSectorIds[i]);
	}
}
