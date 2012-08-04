// ----------------------------------------------------------------------- //
//
// MODULE  : Mine.h
//
// PURPOSE : Mine - Definition
//
// CREATED : 2/26/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __MINE_H__
#define __MINE_H__

#include "Explosion.h"
#include "SharedFXStructs.h"

LINKTO_MODULE( Mine );

class Mine : public Explosion
{
	public :

 		Mine();

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);

        virtual LTVector GetBoundingBoxColor();

        LTFLOAT  m_fActivateDelay;
        LTVector m_vDims;

		MINECREATESTRUCT m_fxcs;

		LTBOOL m_bMoveToFloor;

	private :

		void ReadProp();
		void TouchNotify(HOBJECT hObj);
		void Update();

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
};

#endif // __MINE_H__