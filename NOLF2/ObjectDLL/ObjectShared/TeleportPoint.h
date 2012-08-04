// ----------------------------------------------------------------------- //
//
// MODULE  : TeleportPoint.h
//
// PURPOSE : TeleportPoint - Definition
//
// CREATED : 4/21/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TELEPORT_POINT_H__
#define __TELEPORT_POINT_H__

#include "ltengineobjects.h"

LINKTO_MODULE( TeleportPoint );

class TeleportPoint : public BaseClass
{
	public :

        LTVector GetPitchYawRoll() const { return m_vPitchYawRoll; }

		LTBOOL IsMoveToFloor() const { return m_bMoveToFloor; }

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);

	private :

        LTVector	m_vPitchYawRoll;        // Pitch, yaw, and roll of point
		LTBOOL		m_bMoveToFloor;			// Move to floor when teleporting here?

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
};

#endif // __TELEPORT_POINT_H__