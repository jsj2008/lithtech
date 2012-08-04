// ----------------------------------------------------------------------- //
//
// MODULE  : LaserTrigger.h
//
// PURPOSE : LaserTrigger - Definition
//
// CREATED : 4/30/98
//
// ----------------------------------------------------------------------- //

#ifndef __LASER_TRIGGER_H__
#define __LASER_TRIGGER_H__

#include "Trigger.h"
#include "SharedFXStructs.h"

LINKTO_MODULE( LaserTrigger );


class LaserTrigger : public Trigger
{
	public :

		LaserTrigger();
		~LaserTrigger();

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

        LTBOOL ReadProp(ObjectCreateStruct *pData);
		void InitialUpdate(int nInfo);

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		LTCREATESTRUCT	m_ltcs;
};

#endif // __LASER_TRIGGER_H__