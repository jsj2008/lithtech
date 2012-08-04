// ----------------------------------------------------------------------- //
//
// MODULE  : JumpVolume.h
//
// PURPOSE : A JumpVolume for increasing velocity of an object
//
// CREATED : 1/24/02
//
// (c) 2002-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __JUMP_VOLUME_H__
#define __JUMP_VOLUME_H__

//
// Includes...
//

	#include "GameBase.h"

LINKTO_MODULE( JumpVolume );


class JumpVolume : public GameBase
{
	public :	// Methods...

		JumpVolume();
		~JumpVolume();


	protected :	// Methods...

		uint32 EngineMessageFn( uint32 messageID, void *pData, float fData );

		virtual void ReadProps( const GenericPropList *pProps );
		virtual void PostReadProp( ObjectCreateStruct *pOCS );
		virtual void Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		virtual void Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

		virtual void UpdateFXMessage( bool bSendToClients );
		virtual void WriteFXMessage( ILTMessage_Write &cMsg );


	protected : // Members...

		float		m_fSpeed;
		LTVector	m_vDir;


		// Message Handlers...

		DECLARE_MSG_HANDLER( JumpVolume, HandleVelocityMsg );
};

#endif // __JUMP_VOLUME_H__
