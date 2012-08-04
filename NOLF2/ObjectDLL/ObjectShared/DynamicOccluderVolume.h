// ----------------------------------------------------------------------- //
//
// MODULE  : DynamicOccluderVolume.h
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

#ifndef __DYNAMIC_OCCLUDER_VOLUME_H__
#define __DYNAMIC_OCCLUDER_VOLUME_H__

//
// Includes...
//

	#include "GameBase.h"

LINKTO_MODULE( DynamicOccluderVolume );


class DynamicOccluderVolume : public GameBase
{
	public :	// Methods...

		DynamicOccluderVolume();
		~DynamicOccluderVolume();

	protected :	// Methods...

		uint32 EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData );

	private : // Members...

		void ReadProps( ObjectCreateStruct *pOCS );
		void PostReadProp( ObjectCreateStruct *pOCS );
		void Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		void Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

		void UpdateFXMessage( LTBOOL bSendToClients );
		void WriteFXMessage( ILTMessage_Write &cMsg );

		enum constants { kMaxOccluders=10,
						 kMaxRenderGroups=20};

		uint32	m_nOccluderIds[kMaxOccluders];
		uint8	m_nNumOccluders;

		uint32	m_nRenderGroups[kMaxRenderGroups];
		uint8	m_nNumRenderGroups;
};

#endif // __DYNAMIC_OCCLUDER_VOLUME_H__