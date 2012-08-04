// ----------------------------------------------------------------------- //
//
// MODULE  : DynamicSectorVolume.h
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

#ifndef __DYNAMIC_SECTOR_VOLUME_H__
#define __DYNAMIC_SECTOR_VOLUME_H__

//
// Includes...
//

	#include "GameBase.h"

LINKTO_MODULE( DynamicSectorVolume );


class DynamicSectorVolume : public GameBase
{
	public :	// Methods...

		DynamicSectorVolume();
		~DynamicSectorVolume();

	protected :	// Methods...

		uint32 EngineMessageFn( uint32 messageID, void *pData, float fData );

	private : // Members...

		void ReadProps( const GenericPropList *pProps );
		void PostReadProp( ObjectCreateStruct *pOCS );
		void Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		void Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

		void UpdateFXMessage( bool bSendToClients );
		void WriteFXMessage( ILTMessage_Write &cMsg );

		enum constants { kMaxSectors=10};

		uint32	m_nSectorIds[kMaxSectors];
		uint8	m_nNumSectors;
};

#endif // __DYNAMIC_SECTOR_VOLUME_H__
