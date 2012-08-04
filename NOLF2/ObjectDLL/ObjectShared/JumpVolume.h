// ----------------------------------------------------------------------- //
//
// MODULE  : JumpVolume.h
//
// PURPOSE : A JumpVolume for increasing velocity of an object
//
// CREATED : 1/24/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
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

		uint32 EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData );

		virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		virtual void ReadProps( ObjectCreateStruct *pOCS );
		virtual void PostReadProp( ObjectCreateStruct *pOCS );
		virtual void Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		virtual void Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

		virtual void UpdateFXMessage( LTBOOL bSendToClients );
		virtual void WriteFXMessage( ILTMessage_Write &cMsg );


	protected : // Members...

		LTFLOAT		m_fSpeed;
		LTVector	m_vDir;
};

#endif // __JUMP_VOLUME_H__