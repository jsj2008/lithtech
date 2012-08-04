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

#ifndef __STAIRVOLUME_H__
#define __STAIRVOLUME_H__

// ----------------------------------------------------------------------- //

#include "GameBase.h"

// ----------------------------------------------------------------------- //

LINKTO_MODULE( StairVolume );

// ----------------------------------------------------------------------- //

class StairVolume : public GameBase
{
	public:

		StairVolume();
		virtual ~StairVolume();


	protected:

		uint32		EngineMessageFn( uint32 nMsgID, void* pData, float fData );
		uint32		ObjectMessageFn( HOBJECT hSender, ILTMessage_Read* pMsg );


	private:

		void		ReadProps( const GenericPropList *pProps );
		void		OnObjectCreated();

		void		Save( ILTMessage_Write* pMsg, uint32 nFlags );
		void		Load( ILTMessage_Read* pMsg, uint32 nFlags );

};

// ----------------------------------------------------------------------- //

#endif//__STAIRVOLUME_H__

