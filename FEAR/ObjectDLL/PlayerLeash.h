// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerLeash.h
//
// PURPOSE : Game object that defines a leash for the player
//
// CREATED : 10/11/04
//
// (c) 2003-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYERLEASH_H__
#define __PLAYERLEASH_H__

// ----------------------------------------------------------------------- //

#include "GameBase.h"

// ----------------------------------------------------------------------- //

LINKTO_MODULE( PlayerLeash );

// ----------------------------------------------------------------------- //

class PlayerLeash : public GameBase
{
	public:

		PlayerLeash();
		virtual ~PlayerLeash();


	protected:

		uint32			EngineMessageFn( uint32 nMsgID, void* pData, float fData );
		uint32			ObjectMessageFn( HOBJECT hSender, ILTMessage_Read* pMsg );


	private:

		void			ReadProps( const GenericPropList *pProps );
		void			OnObjectCreated();

		void			Save( ILTMessage_Write* pMsg, uint32 nFlags );
		void			Load( ILTMessage_Read* pMsg, uint32 nFlags );

		// Message Handlers...
		DECLARE_MSG_HANDLER( PlayerLeash, HandleOnMessage );
		DECLARE_MSG_HANDLER( PlayerLeash, HandleOffMessage );


	protected:

		float			m_fInnerRadius;
		float			m_fOuterRadius;
		LTVector		m_vPos;
};

// ----------------------------------------------------------------------- //

#endif//__PLAYERLEASH_H__

