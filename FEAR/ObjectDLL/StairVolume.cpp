// ----------------------------------------------------------------------- //
//
// MODULE  : StairVolume.h
//
// PURPOSE : Game object that defines stair areas
//
// CREATED : 10/07/04
//
// (c) 2003-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "StairVolume.h"
#include "ContainerCodes.h"

// ----------------------------------------------------------------------- //

LINKFROM_MODULE( StairVolume );

// ----------------------------------------------------------------------- //

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_StairVolume 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_StairVolume CF_HIDDEN

#endif

BEGIN_CLASS( StairVolume )

END_CLASS_FLAGS( StairVolume, GameBase, CF_WORLDMODEL | CF_HIDDEN_StairVolume, "Game object that defines stair areas." )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StairVolume::StairVolume()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

StairVolume::StairVolume() : GameBase( OT_CONTAINER )
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StairVolume::~StairVolume()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

StairVolume::~StairVolume()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StairVolume::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 StairVolume::EngineMessageFn( uint32 nMsgID, void* pData, float fData )
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
			}

			pOCS->m_Flags |= FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE;
			pOCS->m_Flags &= ~FLAG_RAYHIT;
			pOCS->SetFileName( pOCS->m_Name );
			pOCS->m_ContainerCode = ( uint16 )CC_STAIRS;

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
//	ROUTINE:	StairVolume::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 StairVolume::ObjectMessageFn( HOBJECT hSender, ILTMessage_Read* pMsg )
{
	return GameBase::ObjectMessageFn( hSender, pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StairVolume::ReadProp()
//
//	PURPOSE:	Read properties from object data
//
// ----------------------------------------------------------------------- //

void StairVolume::ReadProps( const GenericPropList *pProps )
{
	if( !pProps ) return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StairVolume::InitialUpdate()
//
//	PURPOSE:	Sets up the object based on member variables.
//
// ----------------------------------------------------------------------- //

void StairVolume::OnObjectCreated()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StairVolume::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void StairVolume::Save( ILTMessage_Write* pMsg, uint32 nFlags )
{
	if( !pMsg ) return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StairVolume::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void StairVolume::Load( ILTMessage_Read* pMsg, uint32 nFlags )
{
	if( !pMsg ) return;
}

