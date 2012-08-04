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

class LaserTrigger : public Trigger
{
	public :

		LaserTrigger();
		~LaserTrigger();

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual void  HandleTriggerMsg(HOBJECT hSender, const char* szMsg);

	private :

        LTBOOL ReadProp(ObjectCreateStruct *pData);
		void InitialUpdate(int nInfo);

        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		LTCREATESTRUCT	m_ltcs;
};

#endif // __LASER_TRIGGER_H__