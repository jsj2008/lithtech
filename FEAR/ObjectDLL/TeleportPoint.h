// ----------------------------------------------------------------------- //
//
// MODULE  : TeleportPoint.h
//
// PURPOSE : TeleportPoint - Definition
//
// CREATED : 4/21/99
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TELEPORT_POINT_H__
#define __TELEPORT_POINT_H__

#include "GameBase.h"

LINKTO_MODULE( TeleportPoint );

class TeleportPoint : public GameBase
{
	public :

		bool IsMoveToFloor() const { return m_bMoveToFloor; }

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, float lData);

	private :

		bool		m_bMoveToFloor;			// Move to floor when teleporting here?

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
};

#endif // __TELEPORT_POINT_H__
